/**
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 *
 * Description: Create an FM system file node and register the driver
 * Author: @CompanyNameTag
 */

#include <linux/miscdevice.h>
#include <linux/reboot.h>
#include <linux/fs.h>

#include "plat_pm.h"
#include "bfgx_dev.h"
#include "bfgx_core.h"
#include "bfgx_data_parse.h"
#include "plat_exception_rst.h"
#include "hw_bfg_ps.h"
#include "plat_debug.h"
#include "bfgx_excp_dbg.h"

/* fm */
#define FM_SET_READ_TIME     1

/*
 * Prototype    : hw_fm_open
 * Description  : functions called from above fm hal,when open fm file
 * input        : "/dev/hwfm"
 * output       : return 0 --> open is ok
 *              : return !0--> open is false
 */
STATIC int32_t hw_fm_open(struct inode *inode, struct file *filp)
{
    int32_t ret;
    struct ps_core_s *ps_core_d = NULL;
    struct bfgx_dev_node *dev_node = NULL;
    struct pm_top* pm_top_data = pm_get_top();

    dev_node = oal_container_of((struct miscdevice *)filp->private_data,
                                 struct bfgx_dev_node, c);
    ps_core_d = (struct ps_core_s *)dev_node->core;
    if (ps_core_d == NULL) {
        ps_print_err("%s ps core is null", __func__);
        return -EINVAL;
    }

    mutex_lock(&(pm_top_data->host_mutex));
    if (oal_atomic_read(&dev_node->open_cnt) != 0) {
        mutex_unlock(&(pm_top_data->host_mutex));
        ps_print_err("hwfm dev already opened\n");
        return -EINVAL;
    }

    ret = hw_bfgx_open(ps_core_d, BFGX_FM);
    oal_atomic_set(&(ps_core_d->bfgx_info[BFGX_FM].read_delay), FM_READ_DEFAULT_TIME);
    if (ret == 0) {
        oal_atomic_set(&dev_node->open_cnt, 1);
    }
    mutex_unlock(&(pm_top_data->host_mutex));

    return ret;
}

/*
 * Prototype    : hw_fm_read
 * Description  : functions called from above fm hal,read count data to buf
 */
STATIC ssize_t hw_fm_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    struct sk_buff *skb = NULL;
    uint16_t count1;
    long timeout;

    PS_PRINT_FUNCTION_NAME;
    if (unlikely(buf == NULL)) {
        ps_print_err("buf is NULL\n");
        return -EINVAL;
    }

    ps_core_d = get_ps_core_from_miscdev(filp->private_data);
    if (ps_core_d == NULL) {
        ps_print_err("get ps core fail\n");
        return -EINVAL;
    }

    if (ps_core_d->bfgx_info[BFGX_FM].rx_queue.qlen == 0) { /* if don,t data, and wait timeout function */
        if (filp->f_flags & O_NONBLOCK) {                   /* if O_NONBLOCK read and return */
            return -EAGAIN;
        }
        /* timeout function;when have data,can interrupt */
        timeout = oal_wait_event_interruptible_timeout_m(ps_core_d->bfgx_info[BFGX_FM].rx_wait,
            (ps_core_d->bfgx_info[BFGX_FM].rx_queue.qlen > 0),
            (long)msecs_to_jiffies(oal_atomic_read(&(ps_core_d->bfgx_info[BFGX_FM].read_delay))));
        if (!timeout) {
            ps_print_dbg("fm read time out!\n");
            return -ETIMEDOUT;
        }
    }

    if ((skb = ps_skb_dequeue(ps_core_d, RX_FM_QUEUE)) == NULL) {
        ps_print_warning("fm read no data!\n");
        return -ETIMEDOUT;
    }

    count1 = min_t(size_t, skb->len, count);
    if (copy_to_user(buf, skb->data, count1)) {
        ps_print_err("copy_to_user is err!\n");
        ps_restore_skbqueue(ps_core_d, skb, RX_FM_QUEUE);
        return -EFAULT;
    }

    skb_pull(skb, count1);

    if (skb->len == 0) { /* curr skb data have read to user */
        kfree_skb(skb);
    } else { /* if don,t read over; restore to skb queue */
        ps_restore_skbqueue(ps_core_d, skb, RX_FM_QUEUE);
    }

    return count1;
}

/*
 * Prototype    : hw_fm_write
 * Description  : functions called from above fm hal,write count data to buf
 */
STATIC ssize_t hw_fm_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    int32_t ret = 0;
    int32_t prepare_result;

    PS_PRINT_FUNCTION_NAME;

    ps_core_d = get_ps_core_from_miscdev(filp->private_data);
    ret = hw_bfgx_write_check(buf, ps_core_d, BFGX_FM);
    if (ret < 0) {
        return ret;
    }

    /* if count is too large;and don,t tx */
    if (count > (FM_TX_MAX_FRAME - sizeof(struct ps_packet_head) - sizeof(struct ps_packet_end))) {
        ps_print_err("err:fm packet is too large!\n");
        return -EINVAL;
    }

    /* if low queue num > MAX_NUM and don't write */
    if (oal_netbuf_list_len(&ps_core_d->tx_low_seq) > TX_LOW_QUE_MAX_NUM) {
        return 0;
    }

    ret = prepare_to_visit_node(ps_core_d, &prepare_result);
    if (ret < 0) {
        ps_print_err("prepare work fail, bring to reset work\n");
        plat_exception_handler(SUBSYS_BFGX, THREAD_FM, get_wakeup_excp_by_uart_idx(ps_core_d->pm_data->index));
        return ret;
    }

    /* modify expire time of uart idle timer */
    mod_timer(&ps_core_d->pm_data->bfg_timer, jiffies + msecs_to_jiffies(BT_SLEEP_TIME));
    oal_atomic_inc(&ps_core_d->pm_data->bfg_timer_mod_cnt);

    /* to divide up packet function and tx to tty work */
    if (ps_h2d_tx_data(ps_core_d, BFGX_FM, buf, count) < 0) {
        ps_print_err("hw_fm_write is err\n");
        post_to_visit_node(ps_core_d, prepare_result);
        return -EFAULT;
    }
    ps_core_d->bfgx_info[BFGX_FM].tx_pkt_num++;

    post_to_visit_node(ps_core_d, prepare_result);

    ps_print_dbg("FM data write end\n");

    return count;
}

/*
 * Prototype    : hw_fm_ioctl
 * Description  : called by hw func from hal when open power gpio or close power gpio
 */
STATIC long hw_fm_ioctl(struct file *filp, uint32_t cmd, unsigned long arg)
{
    struct ps_core_s *ps_core_d = NULL;

    PS_PRINT_FUNCTION_NAME;

    ps_core_d = get_ps_core_from_miscdev(filp->private_data);
    if (unlikely((ps_core_d == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (cmd == FM_SET_READ_TIME) {
        if (arg < FM_MAX_READ_TIME) { /* set timeout for fm read function */
            oal_atomic_set(&(ps_core_d->bfgx_info[BFGX_FM].read_delay), arg);
        } else {
            ps_print_err("arg is too large!\n");
            return -EINVAL;
        }
    }

    return 0;
}

/*
 * Prototype    : hw_fm_release
 * Description  : called by fm func from hal when close fm inode
 * input        : have opened file handle
 * output       : return 0 --> close is ok
 *                return !0--> close is false
 */
STATIC int32_t hw_fm_release(struct inode *inode, struct file *filp)
{
    int32_t ret;
    struct ps_core_s *ps_core_d = NULL;
    struct bfgx_dev_node *dev_node = NULL;
    struct pm_top* pm_top_data = pm_get_top();

    if (unlikely((inode == NULL) || (filp == NULL) || (filp->private_data == NULL))) {
        ps_print_err("%s param is error", __func__);
        return -EINVAL;
    }

    dev_node = oal_container_of((struct miscdevice *)filp->private_data,
                                 struct bfgx_dev_node, c);
    ps_core_d = dev_node->core;
    mutex_lock(&(pm_top_data->host_mutex));
    ret = hw_bfgx_close(ps_core_d, BFGX_FM);
    oal_atomic_set(&dev_node->open_cnt, 0);
    mutex_unlock(&(pm_top_data->host_mutex));

    return ret;
}

STATIC const struct file_operations g_default_fm_fops = {
    .owner = THIS_MODULE,
    .open = hw_fm_open,
    .write = hw_fm_write,
    .read = hw_fm_read,
    .unlocked_ioctl = hw_fm_ioctl,
    .release = hw_fm_release,
};

#ifdef PLATFORM_DEBUG_ENABLE
STATIC ssize_t excp_dbg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return bfgx_excp_dbg_show_comm(buf);
}

STATIC ssize_t excp_dbg_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    return bfgx_excp_dbg_store_comm(dev, buf, count);
}
STATIC DEVICE_ATTR_RW(excp_dbg);
#endif

// for directory /sys/devices/virtual/misc/hwfm
STATIC struct attribute *g_hwfm_attrs[] = {
#ifdef PLATFORM_DEBUG_ENABLE
    &dev_attr_excp_dbg.attr,
#endif
    NULL
};
ATTRIBUTE_GROUPS(g_hwfm);

STATIC struct bfgx_subsys_driver_desc g_default_fm_driver_desc = {
    .file_ops = &g_default_fm_fops,
    .groups = g_hwfm_groups
};

struct bfgx_subsys_driver_desc *get_fm_default_drv_desc(void)
{
    return &g_default_fm_driver_desc;
}

MODULE_DESCRIPTION("Public serial Driver for huawei hwfm chips");
MODULE_LICENSE("GPL");