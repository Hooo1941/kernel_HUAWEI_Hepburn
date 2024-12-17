/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:usb netbuf
 * Author: @CompanyNameTag
 */

#include "oal_usb_netbuf.h"
#include "oal_hcc_host_if.h"
#include "oal_usb_driver.h"
#include "oal_usb_linux.h"

#define OAL_USB_NETBUF_MAX_SIZE     2048
#define OAL_USB_NETBUF_RX_THRESHOLD 16

#define OAL_USB_TX_NETBUF_WAIT_TIME         5
#define OAL_USB_TX_NETBUF_HIGH_WATERLINE    256
#define OAL_USB_TX_NETBUF_LOW_WATERLINE     128
oal_wait_queue_head_stru g_tx_netbuf_wait_queue;
uint32_t g_tx_netbuf_count = 0;
uint32_t g_tx_netbuf_comp_count = 0;

static void oal_usb_tx_netbuf_complete(struct urb *urb)
{
    struct oal_usb_xfer_param *param = NULL;
    struct oal_usb_dev *usb_dev = NULL;
    oal_netbuf_stru *netbuf = NULL;
    struct hcc_handler *hcc = NULL;

    param = (struct oal_usb_xfer_param *)urb->context;
    usb_dev = param->usb_dev;
    hcc = hbus_to_hcc(usb_dev->usb_dev_container->bus);
    netbuf = (oal_netbuf_stru *)param->context;

    oal_usb_tx_complete(urb, param);
    if (usb_get_xfer_flag(param->flag, USB_XFER_DMA)) {
        dma_unmap_single(&usb_dev->udev->dev, urb->transfer_dma, urb->transfer_buffer_length, DMA_TO_DEVICE);
    }
    hcc_tx_netbuf_free(netbuf, hcc);

    g_tx_netbuf_comp_count ++;
    oal_wait_queue_wake_up_interrupt(&g_tx_netbuf_wait_queue);
}

static void oal_usb_rx_netbuf_complete(struct urb *urb)
{
    struct oal_usb_xfer_param *param = NULL;
    struct oal_usb_dev *usb_dev = NULL;
    oal_netbuf_stru *netbuf = NULL;
    struct hcc_handler *hcc = NULL;
    uint32_t free_len = urb->transfer_buffer_length - urb->actual_length;

    param = (struct oal_usb_xfer_param *)urb->context;
    usb_dev = param->usb_dev;
    hcc = hbus_to_hcc(usb_dev->usb_dev_container->bus);
    netbuf = (oal_netbuf_stru *)param->context;

    oal_usb_rx_complete(urb, param);
    if (usb_get_xfer_flag(param->flag, USB_XFER_DMA)) {
        dma_unmap_single(&usb_dev->udev->dev, urb->transfer_dma, urb->transfer_buffer_length, DMA_FROM_DEVICE);
    }
    /* 根据实际接收到的数据，刷新netbuf有效数据长度 */
    netbuf->tail -= free_len;
    netbuf->len -= free_len;
    hcc_rx_submit(hcc, netbuf);
    hcc_sched_transfer(hcc);
    oal_usb_sched_rx_thread(usb_dev);
}

static inline oal_netbuf_stru *oal_usb_rx_netbuf_alloc(uint32_t ul_size, int32_t gflag)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    return __netdev_alloc_skb(NULL, ul_size, gflag);
#else
    oal_reference(gflag);
    return oal_netbuf_alloc(ul_size, 0, 0);
#endif
}

static int32_t oal_usb_rx_netbuf_supply(struct oal_usb_dev *usb_dev)
{
    struct oal_usb_xfer_param *param = NULL;
    uint32_t busy_cnt;
    uint32_t supply_cnt;
    oal_netbuf_stru *netbuf = NULL;
    int32_t i;
    int32_t ret;

    busy_cnt = usb_dev->xfer_stat.rx_total - usb_dev->xfer_stat.rx_cmpl - usb_dev->xfer_stat.rx_miss;
    if (busy_cnt >= usb_dev->rx_threshold) {
        return OAL_SUCC;
    }
    supply_cnt = usb_dev->rx_threshold - busy_cnt;
    for (i = 0; i < supply_cnt; ++i) {
        oal_print_mpxx_log(MPXX_LOG_DBG, "supply netbuf index[%d]", i);
        netbuf = oal_usb_rx_netbuf_alloc(usb_dev->rx_buf_max_size, GFP_KERNEL);
        if (netbuf == NULL) {
            return -OAL_EFAIL;
        }
        oal_netbuf_put(netbuf, usb_dev->rx_buf_max_size);
        param = oal_usb_alloc_xfer_param(usb_dev, oal_netbuf_data(netbuf), oal_netbuf_len(netbuf), TRANSFER_NOT_TIMEOUT,
                                         HOST_USB_DMA_XFER, oal_usb_rx_netbuf_complete, netbuf);
        ret = oal_usb_bulk_rx(param);
        if (ret != OAL_SUCC) {
            oal_netbuf_free(netbuf);
            return ret;
        }
    }
    return OAL_SUCC;
}

static int32_t oal_usb_rx_netbuf_thread(void *data)
{
    struct oal_usb_dev *usb_dev = (struct oal_usb_dev *)data;
    int32_t ret;

    if (usb_dev == NULL) {
        return OAL_FAIL;
    }

    allow_signal(SIGTERM);
    oal_print_mpxx_log(MPXX_LOG_INFO, "%s thread start", oal_get_current_task_name());

    forever_loop()
    {
        if (oal_unlikely(kthread_should_stop())) {
            break;
        }
        ret = oal_wait_event_interruptible_m(usb_dev->rx_wq, oal_usb_rx_thread_condtion(&usb_dev->rx_cond));
        if (oal_unlikely(ret == -ERESTARTSYS)) {
            oal_print_mpxx_log(MPXX_LOG_INFO, "task %s was interrupted by a signal\n", oal_get_current_task_name());
            break;
        }

        if (oal_unlikely(usb_dev->udev->state != USB_STATE_CONFIGURED)) {
            oal_print_mpxx_log(MPXX_LOG_WARN, "hi thread link invaild, stop supply mem, link_state:%d",
                               usb_dev->udev->state);
        } else {
            ret = oal_usb_rx_netbuf_supply(usb_dev);
            if (ret != OAL_SUCC) {
                oal_usb_sched_rx_thread(usb_dev);
                oal_msleep(10); /* 10 supply failed, may be memleak, wait & retry */
            }
        }
    }

    oal_print_mpxx_log(MPXX_LOG_INFO, "%s thread stop", oal_get_current_task_name());
    return 0;
}

void usb_netbuf_dev_init(struct oal_usb_dev *usb_dev)
{
    usb_dev->rx_thread = oal_usb_rx_netbuf_thread;
    usb_dev->rx_threshold = OAL_USB_NETBUF_RX_THRESHOLD;
    usb_dev->rx_buf_max_size = OAL_USB_NETBUF_MAX_SIZE;
    oal_wait_queue_init_head(&g_tx_netbuf_wait_queue);
}

int32_t oal_usb_rx_netbuf(hcc_bus *bus, oal_netbuf_head_stru *head)
{
    return OAL_SUCC;
}

static int32_t oal_usb_transfer_netbuf_list(struct oal_usb_dev *usb_dev, oal_netbuf_head_stru *head)
{
    struct oal_usb_xfer_param *param = NULL;
    int32_t ret;
    uint32_t queue_len;
    oal_netbuf_stru *netbuf = NULL;
    int32_t total_cnt = 0;
    int32_t i;
    struct hcc_handler *hcc = NULL;

    /* 发送队列长度大于阈值时，等待发送队列低于阈值时再发送.(处理统计数据翻转) */
    if (((g_tx_netbuf_count >= g_tx_netbuf_comp_count) &&
            ((g_tx_netbuf_count - g_tx_netbuf_comp_count) > OAL_USB_TX_NETBUF_HIGH_WATERLINE)) ||
        ((g_tx_netbuf_count < g_tx_netbuf_comp_count) &&
            (g_tx_netbuf_count > (g_tx_netbuf_comp_count + OAL_USB_TX_NETBUF_HIGH_WATERLINE)))) {
        ret = oal_wait_event_interruptible_timeout_m(g_tx_netbuf_wait_queue,
            (((g_tx_netbuf_count >= g_tx_netbuf_comp_count) &&
                ((g_tx_netbuf_count - g_tx_netbuf_comp_count) <= OAL_USB_TX_NETBUF_LOW_WATERLINE)) ||
            ((g_tx_netbuf_count < g_tx_netbuf_comp_count) &&
                (g_tx_netbuf_count <= (g_tx_netbuf_comp_count + OAL_USB_TX_NETBUF_LOW_WATERLINE)))),
            OAL_USB_TX_NETBUF_WAIT_TIME * OAL_TIME_HZ);
        if (ret == 0) {
            oal_print_mpxx_log(MPXX_LOG_WARN, "usb wait timeout. g_tx_netbuf_count:%u, g_tx_netbuf_comp_count:%u",
                g_tx_netbuf_count, g_tx_netbuf_comp_count);
            return 0;
        } else if (ret == -ERESTARTSYS) {
            oal_print_mpxx_log(MPXX_LOG_WARN, "usb wait was interrupted by a signal");
            return 0;
        }
    }

    queue_len = oal_netbuf_list_len(head);
    hcc = hbus_to_hcc(usb_dev->usb_dev_container->bus);

    for (i = 0; i < queue_len; ++i) {
        netbuf = oal_netbuf_delist(head);
        if (oal_unlikely(netbuf == NULL)) {
            oal_print_mpxx_log(MPXX_LOG_INFO, "netbuf is null!");
            continue;
        }
        g_tx_netbuf_count ++;
        param = oal_usb_alloc_xfer_param(usb_dev, oal_netbuf_data(netbuf), oal_netbuf_len(netbuf), TRANSFER_NOT_TIMEOUT,
                                         HOST_USB_DMA_XFER, oal_usb_tx_netbuf_complete, (void *)netbuf);
        ret = oal_usb_bulk_tx(param);
        if (ret != OAL_SUCC) {
            oal_print_mpxx_log(MPXX_LOG_INFO, "tx fail");
            hcc_tx_netbuf_free(netbuf, hcc);
            --g_tx_netbuf_count;
        }
        total_cnt++;
    }

    return total_cnt;
}

int32_t oal_usb_tx_netbuf(hcc_bus *bus, oal_netbuf_head_stru *head, hcc_netbuf_queue_type qtype)
{
    struct hcc_handler *hcc = hbus_to_hcc(bus);
    struct oal_usb_dev *usb_dev = NULL;

    if (hcc == NULL) {
        oal_print_mpxx_log(MPXX_LOG_INFO, "usb dev's hcc handler is null!");
        return OAL_SUCC;
    }
    if (oal_unlikely(oal_atomic_read(&hcc->state) != HCC_ON)) {
        /* drop netbuf list, wlan close or exception */
        oal_print_mpxx_log(MPXX_LOG_INFO, "drop usb netbuflist %u", oal_netbuf_list_len(head));
        return OAL_SUCC;
    }

    usb_dev = ((struct oal_usb_dev_container *)bus->data)->usb_devices[OAL_USB_INTERFACE_BULK][OAL_USB_CHAN_1];
    if (usb_dev == NULL) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "usb device unregsister type = [%u]", OAL_USB_INTERFACE_BULK);
        return OAL_SUCC;
    }

    return oal_usb_transfer_netbuf_list(usb_dev, head);
}
