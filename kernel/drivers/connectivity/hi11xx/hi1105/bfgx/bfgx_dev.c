/**
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 *
 * Description: Create a GNSS/FM system file node and register the driver
 * Author: @CompanyNameTag
 */

/* Include Head file */
#include "bfgx_dev.h"
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/tty.h>
#include <linux/poll.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/reboot.h>
#include <linux/fs.h>
#include <linux/of_device.h>
#include <linux/version.h>
#if (((defined CONFIG_LOG_EXCEPTION) || (defined CONFIG_LOG_USERTYPE)) && \
    (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)) && \
    (!defined PLATFORM_DEBUG_ENABLE))
#include <log/log_usertype.h>
#endif

#include "board.h"
#include "plat_debug.h"
#include "plat_uart.h"
#include "plat_pm.h"
#include "bfgx_user_ctrl.h"
#include "bfgx_exception_rst.h"
#include "plat_exception_rst.h"
#include "plat_firmware.h"
#include "plat_cali.h"
#include "plat_common_clk.h"
#include "securec.h"
#include "oal_ext_if.h"
#include "bfgx_gnss.h"
#include "bfgx_sle.h"
#include "bfgx_core.h"
#include "bfgx_data_parse.h"
#include "chr_user.h"
#ifdef _PRE_CONFIG_HISI_110X_BLUEZ
#include "bfgx_bluez.h"
#endif

#ifdef BFGX_UART_DOWNLOAD_SUPPORT
#include "plat_firmware_uart.h"
#endif

#ifdef CONFIG_HI110X_GPS_REFCLK
#include "gps_refclk_src_3.h"
#endif

#define UART_TEST_DEV_ACK_TIMEOUT_MS 5000

/*
 * This references is the per-PS platform device in the arch/arm/
 * board-xx.c file.
 */

STATIC struct service_attr g_mpxx_srv_desc[BFGX_BUTT] = {
    {"BT", OAL_TRUE, BUART, SYS_CFG_OPEN_BT, SYS_CFG_CLOSE_BT, WAIT_BT_OPEN_TIME, WAIT_BT_CLOSE_TIME},
    {"FM", OAL_TRUE, BUART, SYS_CFG_OPEN_FM, SYS_CFG_CLOSE_FM, WAIT_FM_OPEN_TIME, WAIT_FM_CLOSE_TIME},
    {"GNSS", OAL_TRUE, BUART, SYS_CFG_OPEN_GNSS, SYS_CFG_CLOSE_GNSS, WAIT_GNSS_OPEN_TIME, WAIT_GNSS_CLOSE_TIME},
    {"IR", OAL_TRUE, BUART, SYS_CFG_OPEN_IR, SYS_CFG_CLOSE_IR, WAIT_IR_OPEN_TIME, WAIT_IR_CLOSE_TIME},
    {"SH", OAL_TRUE, BUART, SYS_CFG_OPEN_NFC, SYS_CFG_CLOSE_NFC, WAIT_NFC_OPEN_TIME, WAIT_NFC_CLOSE_TIME},
    {"GNSS_ME", OAL_TRUE, GUART, SYS_CFG_OPEN_GNSS, SYS_CFG_CLOSE_GNSS, WAIT_GNSS_OPEN_TIME, WAIT_GNSS_CLOSE_TIME}
};

STATIC struct bus_attr g_mpxx_bus_desc[UART_BUTT] = {
    {B_SYS, BFGX_WKUP_HOST},
    {B_SYS, G_WKUP_HOST},
};

STATIC struct service_attr g_mp16c_srv_desc[BFGX_BUTT] = {
    {"BT", OAL_TRUE, BUART, SYS_CFG_OPEN_BT, SYS_CFG_CLOSE_BT, WAIT_BT_OPEN_TIME, WAIT_BT_CLOSE_TIME},
    {"FM", OAL_TRUE, GUART, SYS_CFG_OPEN_FM, SYS_CFG_CLOSE_FM, WAIT_FM_OPEN_TIME,  WAIT_FM_CLOSE_TIME},
    {"GNSS", OAL_TRUE, GUART, SYS_CFG_OPEN_GNSS, SYS_CFG_CLOSE_GNSS, WAIT_GNSS_OPEN_TIME, WAIT_GNSS_CLOSE_TIME},
    {"IR", OAL_TRUE, BUART, SYS_CFG_OPEN_IR, SYS_CFG_CLOSE_IR, WAIT_IR_OPEN_TIME, WAIT_IR_CLOSE_TIME},
    {"SH", OAL_TRUE, BUART, SYS_CFG_OPEN_NFC, SYS_CFG_CLOSE_NFC, WAIT_NFC_OPEN_TIME, WAIT_NFC_CLOSE_TIME},
    {"SLE", OAL_TRUE, GUART, SYS_CFG_OPEN_SLE, SYS_CFG_CLOSE_SLE, WAIT_SLE_OPEN_TIME, WAIT_SLE_CLOSE_TIME}
};

STATIC struct bus_attr g_mp16c_bus_desc[UART_BUTT] = {
    {B_SYS, BFGX_WKUP_HOST},
    {G_SYS, G_WKUP_HOST},
};

STATIC struct service_attr g_gf61_srv_desc[BFGX_BUTT] = {
    {"BT", OAL_TRUE, BUART, SYS_CFG_OPEN_BT, SYS_CFG_CLOSE_BT, WAIT_BT_OPEN_TIME, WAIT_BT_CLOSE_TIME},
    {"FM", OAL_FALSE, BUART, SYS_CFG_OPEN_FM, SYS_CFG_CLOSE_FM, WAIT_FM_OPEN_TIME,  WAIT_FM_CLOSE_TIME},
    {"GNSS", OAL_FALSE, BUART, SYS_CFG_OPEN_GNSS, SYS_CFG_CLOSE_GNSS, WAIT_GNSS_OPEN_TIME, WAIT_GNSS_CLOSE_TIME},
    {"IR", OAL_FALSE, BUART, SYS_CFG_OPEN_IR, SYS_CFG_CLOSE_IR, WAIT_IR_OPEN_TIME, WAIT_IR_CLOSE_TIME},
    {"SH", OAL_TRUE, BUART, SYS_CFG_OPEN_NFC, SYS_CFG_CLOSE_NFC, WAIT_NFC_OPEN_TIME, WAIT_NFC_CLOSE_TIME},
    {"SLE", OAL_TRUE, GUART, SYS_CFG_OPEN_SLE, SYS_CFG_CLOSE_SLE, WAIT_SLE_OPEN_TIME, WAIT_SLE_CLOSE_TIME}
};

STATIC struct bus_attr g_gf61_bus_desc[UART_BUTT] = {
    {B_SYS, BFGX_WKUP_HOST},
    {G_SYS, G_WKUP_HOST},
};

struct uart_status_debug {
    uint32_t tty_tx_count;
    uint32_t tty_rx_count;
    uint32_t tty_tx_len;
    uint32_t uart_tx_count;
    uint32_t uart_rx_count;
};

static struct uart_status_debug g_uart_info;

void service_attribute_load(void)
{
    struct pm_top* top = pm_get_top();
    int32_t chip_type;

    if (top == NULL) {
        ps_print_err("%s fail get pm_top\n", __func__);
        return;
    }
    chip_type = get_mpxx_subchip_type();
    if (chip_type == BOARD_VERSION_MP16C) {
        top->srv_desc = g_mp16c_srv_desc;
        top->bus_desc= g_mp16c_bus_desc;
        ps_print_info("%s load mp16c service attribute\n", __func__);
    } else if (chip_type == BOARD_VERSION_GF61) {
        top->srv_desc = g_gf61_srv_desc;
        top->bus_desc = g_gf61_bus_desc;
        ps_print_info("%s load gf61 service attribute\n", __func__);
    } else {
        top->srv_desc = g_mpxx_srv_desc;
        top->bus_desc = g_mpxx_bus_desc;
        ps_print_info("%s load mpxx service attribute\n", __func__);
    }
}

uint8_t* service_get_name(uint8_t service)
{
    struct pm_top* top = pm_get_top();
    if (service >= BFGX_BUTT || top->srv_desc == NULL) {
        return SUCCESS;
    }
    return top->srv_desc[service].name;
}

uint8_t service_get_bus_id(uint8_t service)
{
    struct pm_top* top = pm_get_top();
    if (service >= BFGX_BUTT || top->srv_desc == NULL) {
        return SUCCESS;
    }
    return top->srv_desc[service].bus_id;
}

uint8_t service_bus_to_sys_en(uint8_t bus_id)
{
    struct pm_top* top = pm_get_top();
    if (bus_id >= UART_BUTT) {
        return SUCCESS;
    }
    return top->bus_desc[bus_id].sys_id;
}

STATIC uint8_t service_get_open_cmd(uint8_t service)
{
    struct pm_top* top = pm_get_top();
    if (service >= BFGX_BUTT || top->srv_desc == NULL) {
        return SUCCESS;
    }
    return top->srv_desc[service].open_cmd;
}

STATIC uint32_t service_get_open_timeout(uint8_t service)
{
    struct pm_top* top = pm_get_top();
    if (service >= BFGX_BUTT || top->srv_desc == NULL) {
        return SUCCESS;
    }
    return top->srv_desc[service].open_cmd_timeout;
}

STATIC uint8_t service_get_close_cmd(uint8_t service)
{
    struct pm_top* top = pm_get_top();
    if (service >= BFGX_BUTT || top->srv_desc == NULL) {
        return SUCCESS;
    }
    return top->srv_desc[service].close_cmd;
}

STATIC uint32_t service_get_close_timeout(uint8_t service)
{
    struct pm_top* top = pm_get_top();
    if (service >= BFGX_BUTT || top->srv_desc == NULL) {
        return SUCCESS;
    }
    return top->srv_desc[service].close_cmd_timeout;
}

uint32_t g_gnss_me_thread_status = DEV_THREAD_EXIT;
uint32_t g_gnss_lppe_thread_status = DEV_THREAD_EXIT;

STATIC bool g_device_log_status = false;
void ps_set_device_log_status(bool status)
{
    g_device_log_status = status;
}
bool ps_is_device_log_enable(void)
{
#if (defined PLATFORM_DEBUG_ENABLE)
    return true;
#elif (((defined CONFIG_LOG_EXCEPTION) || (defined CONFIG_LOG_USERTYPE)) && \
    (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)))
    unsigned int log_usertype = get_log_usertype();
    bool status = ((log_usertype > COMMERCIAL_USER) && (log_usertype < OVERSEA_COMMERCIAL_USER));
    status = (status || g_device_log_status);
    return status;
#else
    return g_device_log_status;
#endif
}

/*
 * Prototype    : ps_get_plat_reference
 * Description  : reference the plat's dat,This references the per-PS
 *                  platform device in the arch/arm/board-xx.c file.
 */
int32_t ps_get_plat_reference(struct ps_plat_s **plat_data)
{
    // 接口不支持，仅保留给octty后续适配
    *plat_data = NULL;

    return SUCCESS;
}

/*
 * Prototype    : ps_bfg_subsys_active
 * Description  : check bfgx subsys active or no
 */
bool ps_bfg_subsys_active(struct ps_core_s *ps_core_d, uint32_t subsys)
{
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return false;
    }

    if (subsys >= BFGX_BUTT) {
        ps_print_err("subsys id[%u] exceed limit\n", subsys);
        return false;
    }

    if (atomic_read(&ps_core_d->bfgx_info[subsys].subsys_state) == POWER_STATE_SHUTDOWN) {
        ps_print_info("subsys %s is shutdown\n", service_get_name(subsys));
        return  false;
    }

    return true;
}

/* only gnss is open and it agree to sleep */
STATIC bool ps_chk_only_gnss_and_sleep(struct ps_core_s *ps_core_d)
{
    struct pm_drv_data *pm_data = ps_core_d->pm_data;
    if ((atomic_read(&ps_core_d->bfgx_info[BFGX_BT].subsys_state) == POWER_STATE_SHUTDOWN) &&
        (atomic_read(&ps_core_d->bfgx_info[BFGX_FM].subsys_state) == POWER_STATE_SHUTDOWN) &&
        (atomic_read(&ps_core_d->bfgx_info[BFGX_IR].subsys_state) == POWER_STATE_SHUTDOWN) &&
        (atomic_read(&ps_core_d->bfgx_info[BFGX_NFC].subsys_state) == POWER_STATE_SHUTDOWN) &&
        (atomic_read(&ps_core_d->bfgx_info[BFGX_GNSS].subsys_state) == POWER_STATE_OPEN) &&
        (gnss_get_lowpower_state(pm_data) == GNSS_AGREE_SLEEP) &&
        (pm_data->bfgx_dev_state == BFGX_ACTIVE)) {
        return true;
    }
    return false;
}

bool ps_chk_tx_queue_empty(struct ps_core_s *ps_core_d)
{
    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL");
        return true;
    }

    if (skb_queue_empty(&ps_core_d->tx_high_seq) && skb_queue_empty(&ps_core_d->tx_low_seq)) {
        return true;
    }
    return false;
}

/*
 * Prototype    : ps_alloc_skb
 * Description  : allocate mem for new skb
 */
struct sk_buff *ps_alloc_skb(uint16_t len)
{
    struct sk_buff *skb = NULL;

    skb = alloc_skb(len, GFP_KERNEL);
    if (skb == NULL) {
        ps_print_warning("can't allocate mem for new skb, len=%u\n", len);
        return NULL;
    }

    skb_put(skb, len);

    return skb;
}

/*
 * Prototype    : ps_kfree_skb
 * Description  : when close a function, kfree skb
 */
void ps_kfree_skb(struct ps_core_s *ps_core_d, uint8_t type)
{
    struct sk_buff *skb = NULL;

    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL");
        return;
    }

    while ((skb = ps_skb_dequeue(ps_core_d, type))) {
        kfree_skb(skb);
    }

    switch (type) {
        case TX_HIGH_QUEUE:
            skb_queue_purge(&ps_core_d->tx_high_seq);
            break;
        case TX_LOW_QUEUE:
            skb_queue_purge(&ps_core_d->tx_low_seq);
            break;
        case RX_GNSS_QUEUE:
            skb_queue_purge(&ps_core_d->bfgx_info[BFGX_GNSS].rx_queue);
            break;
        case RX_FM_QUEUE:
            skb_queue_purge(&ps_core_d->bfgx_info[BFGX_FM].rx_queue);
            break;
        case RX_BT_QUEUE:
            skb_queue_purge(&ps_core_d->bfgx_info[BFGX_BT].rx_queue);
            break;
        case RX_DBG_QUEUE:
            skb_queue_purge(&ps_core_d->rx_dbg_seq);
            break;
        case RX_NFC_QUEUE:
            skb_queue_purge(&ps_core_d->bfgx_info[BFGX_NFC].rx_queue);
            break;
        case RX_IR_QUEUE:
            skb_queue_purge(&ps_core_d->bfgx_info[BFGX_IR].rx_queue);
            break;
        case RX_SLE_QUEUE:
            skb_queue_purge(&ps_core_d->bfgx_info[BFGX_SLE].rx_queue);
            break;
        default:
            ps_print_err("queue type is error, type=%u\n", type);
            break;
    }
    return;
}

/*
 * Prototype    : ps_restore_skbqueue
 * Description  : when err and restore skb to seq function.
 */
int32_t ps_restore_skbqueue(struct ps_core_s *ps_core_d, struct sk_buff *skb, uint8_t type)
{
    PS_PRINT_FUNCTION_NAME;

    if (unlikely((skb == NULL) || (ps_core_d == NULL))) {
        ps_print_err(" skb or ps_core_d is NULL\n");
        return -EINVAL;
    }

    switch (type) {
        case RX_GNSS_QUEUE:
            skb_queue_head(&ps_core_d->bfgx_info[BFGX_GNSS].rx_queue, skb);
            break;
        case RX_FM_QUEUE:
            skb_queue_head(&ps_core_d->bfgx_info[BFGX_FM].rx_queue, skb);
            break;
        case RX_BT_QUEUE:
            skb_queue_head(&ps_core_d->bfgx_info[BFGX_BT].rx_queue, skb);
            break;
        case RX_IR_QUEUE:
            skb_queue_head(&ps_core_d->bfgx_info[BFGX_IR].rx_queue, skb);
            break;
        case RX_NFC_QUEUE:
            skb_queue_head(&ps_core_d->bfgx_info[BFGX_NFC].rx_queue, skb);
            break;
        case RX_SLE_QUEUE:
            skb_queue_head(&ps_core_d->bfgx_info[BFGX_SLE].rx_queue, skb);
            break;
        case RX_DBG_QUEUE:
            skb_queue_head(&ps_core_d->rx_dbg_seq, skb);
            break;

        default:
            ps_print_err("queue type is error, type=%u\n", type);
            break;
    }

    return SUCCESS;
}

/* prepare to visit dev_node */
int32_t prepare_to_visit_node(struct ps_core_s *ps_core_d, int32_t *prepare_result)
{
    struct pm_drv_data *pm_data = NULL;
    uint8_t uart_ready;
    unsigned long flags;

    *prepare_result = -EFAULT;
    pm_data = ps_core_d->pm_data;
    if (unlikely(pm_data == NULL)) {
        ps_print_err("pm_data is NULL!\n");
        return -EFAULT;
    }

    /* lock wake_lock */
    pm_data->bfg_wake_lock(pm_data);

    /* try to wake up device */
    oal_spin_lock_irq_save(&pm_data->uart_state_spinlock, &flags);
    atomic_inc(&ps_core_d->node_visit_flag); /* mark someone is visiting dev node */
    uart_ready = pm_data->bfgx_uart_state_get(pm_data);
    oal_spin_unlock_irq_restore(&pm_data->uart_state_spinlock, &flags);

    if (uart_ready == UART_NOT_READY) {
        if (host_wkup_dev(pm_data) != 0) {
            ps_print_err("[%s]wkup device FAILED!\n", index2name(pm_data->index));
            atomic_dec(&ps_core_d->node_visit_flag);
            return -EIO;
        }
    }
    *prepare_result = SUCCESS;
    return SUCCESS;
}

/* we should do something before exit from visiting dev_node */
void post_to_visit_node(struct ps_core_s *ps_core_d, int32_t prepare_result)
{
    struct pm_drv_data *pm_data = NULL;

    pm_data = ps_core_d->pm_data;
    if (unlikely(pm_data == NULL)) {
        ps_print_err("pm_data is NULL!\n");
        return;
    }

    if (unlikely(prepare_result != SUCCESS)) {
        ps_print_warning("[%s]node_visit_flag[%d] is not need decrease\n", index2name(pm_data->index),
                         atomic_read(&ps_core_d->node_visit_flag));
        return;
    }

    atomic_dec(&ps_core_d->node_visit_flag);

    return;
}

int32_t alloc_seperted_rx_buf(struct ps_core_s *ps_core_d, uint8_t subsys, uint32_t len, uint8_t alloctype)
{
    struct bfgx_sepreted_rx_st *pst_sepreted_data = NULL;
    uint8_t *p_rx_buf = NULL;

    if (subsys == BFGX_BT || subsys == BFGX_SLE) {
        ps_print_dbg("%s no sepreted buf\n", service_get_name(subsys));
        return SUCCESS;
    }

    pst_sepreted_data = &ps_core_d->bfgx_info[subsys].sepreted_rx;

    if (alloctype == KZALLOC) {
        p_rx_buf = kzalloc(len, GFP_KERNEL);
    } else if (alloctype == VMALLOC) {
        p_rx_buf = vmalloc(len);
    }

    if (p_rx_buf == NULL) {
        ps_print_err("alloc failed! subsys=%d, len=%u\n", subsys, len);
        return -ENOMEM;
    }

    oal_spin_lock(&pst_sepreted_data->sepreted_rx_lock);
    pst_sepreted_data->rx_prev_seq = RX_SEQ_NULL;
    pst_sepreted_data->rx_buf_all_len = 0;
    pst_sepreted_data->rx_buf_ptr = p_rx_buf;
    pst_sepreted_data->rx_buf_org_ptr = p_rx_buf;
    oal_spin_unlock(&pst_sepreted_data->sepreted_rx_lock);

    return SUCCESS;
}

void free_seperted_rx_buf(struct ps_core_s *ps_core_d, uint8_t subsys, uint8_t alloctype)
{
    struct bfgx_sepreted_rx_st *pst_sepreted_data = NULL;
    uint8_t *buf_ptr = NULL;

    if (subsys == BFGX_BT || subsys == BFGX_SLE) {
        ps_print_info("%s no sepreted buf\n", service_get_name(subsys));
        return;
    }

    pst_sepreted_data = &ps_core_d->bfgx_info[subsys].sepreted_rx;

    buf_ptr = pst_sepreted_data->rx_buf_org_ptr;
    oal_spin_lock(&pst_sepreted_data->sepreted_rx_lock);
    pst_sepreted_data->rx_prev_seq = RX_SEQ_NULL;
    pst_sepreted_data->rx_buf_all_len = 0;
    pst_sepreted_data->rx_buf_ptr = NULL;
    pst_sepreted_data->rx_buf_org_ptr = NULL;
    oal_spin_unlock(&pst_sepreted_data->sepreted_rx_lock);
    if (buf_ptr != NULL) {
        if (alloctype == KZALLOC) {
            kfree(buf_ptr);
        } else if (alloctype == VMALLOC) {
            vfree(buf_ptr);
        }
    }

    return;
}

STATIC void bfgx_open_fail_process(struct ps_core_s *ps_core_d, uint32_t subsys, int32_t error)
{
    struct st_exception_info *pst_excp_data = get_exception_info_reference();
    if (unlikely(ps_core_d == NULL || pst_excp_data == NULL)) {
        ps_print_err("ps_core_d or pst_excp_data is NULL\n");
        return;
    }

    ps_print_info("bfgx open fail, type=[%d]\n", error);
    del_timer_sync(&ps_core_d->pm_data->bfg_timer);

    (void)ssi_dump_err_regs(SSI_ERR_BFGX_OPEN_FAIL);

    switch (error) {
        case BFGX_POWER_PULL_POWER_GPIO_FAIL:
        case BFGX_POWER_TTY_OPEN_FAIL:
        case BFGX_POWER_TTY_FLOW_ENABLE_FAIL:
            break;

        case BFGX_POWER_WIFI_DERESET_BCPU_FAIL:
        case BFGX_POWER_WIFI_ON_BOOT_UP_FAIL:
            if (plat_power_fail_info_set(SUBSYS_BFGX, subsys, get_poweron_fail_excp(subsys)) == BFGX_POWER_SUCCESS) {
                chip_reset_main(&pst_excp_data->record, pst_excp_data->excp_chip);
                plat_power_fail_process_done(get_poweron_fail_excp(subsys));
            } else {
                ps_print_err("bfgx power fail, set exception info fail\n");
            }

            ps_core_d->pm_data->bfg_power_set(ps_core_d->pm_data, subsys, BFG_POWER_GPIO_DOWN);
            break;

        case BFGX_POWER_WIFI_OFF_BOOT_UP_FAIL:
        case BFGX_POWER_DOWNLOAD_FIRMWARE_FAIL:
        case BFGX_POWER_DOWNLOAD_FIRMWARE_INTERRUPT:
            ps_core_d->pm_data->bfg_power_set(ps_core_d->pm_data, subsys, BFG_POWER_GPIO_DOWN);
            break;

        case BFGX_POWER_WAKEUP_FAIL:
        case BFGX_POWER_OPEN_CMD_FAIL:
            if (plat_power_fail_info_set(SUBSYS_BFGX, subsys, get_poweron_fail_excp(subsys)) == BFGX_POWER_SUCCESS) {
                chip_reset_main(&pst_excp_data->record, pst_excp_data->excp_chip);
                plat_power_fail_process_done(get_poweron_fail_excp(subsys));
            } else {
                ps_print_err("bfgx power fail, set exception info fail\n");
            }
            ps_core_d->pm_data->bfg_power_set(ps_core_d->pm_data, subsys, BFG_POWER_GPIO_DOWN);
            break;

        default:
            ps_print_err("error is undefined, error=[%d]\n", error);
            break;
    }
}

STATIC int32_t wait_bfgx_close_done(struct ps_core_s *ps_core_d)
{
#define WAIT_CLOSE_TIMES 100
    int32_t i;
    int32_t ret = FAILURE;
    int32_t bwkup_gpio_val = 1;
    struct ps_plat_s *ps_plat_data = NULL;

    ps_plat_data = (struct ps_plat_s *)ps_core_d->ps_plat;
    if (conn_gpio_valid(ps_plat_data->host_wkup_io_type) == OAL_FALSE) {
        msleep(BFGX_CLOSE_DELAY);
        return SUCCESS;
    }

    for (i = 0; i < WAIT_CLOSE_TIMES; i++) {
        bwkup_gpio_val = conn_get_gpio_level(ps_plat_data->host_wkup_io_type);
        if (bwkup_gpio_val == 0) {
            ret = SUCCESS;
            break;
        }
        msleep(10); // sleep 10ms
    }
    ps_print_info("[%s]bfg gpio level:%d, i=%d\n", index2name(ps_core_d->pm_data->index), bwkup_gpio_val, i);

    if (ret == FAILURE) {
        ps_uart_state_dump(ps_core_d);
        ssi_dump_device_regs(SSI_MODULE_MASK_AON | SSI_MODULE_MASK_ARM_REG | SSI_MODULE_MASK_GTRACE |
                             SSI_MODULE_MASK_BTRACE);
    }

    return ret;
}

/*
 * Prototype    : uart_bfgx_close_cmd
 * Description  : functions called by bfgn pm to close bcpu throuhg bfgx system
 */
int32_t uart_bfgx_close_cmd(struct ps_core_s *ps_core_d)
{
    int32_t ret;

    ps_print_info("%s\n", __func__);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is err\n");
        return -EINVAL;
    }

    /* 单红外dev不处理系统消息 */
    if (oal_atomic_read(&ps_core_d->ir_only) != 0) {
        return SUCCESS;
    }

     /* 下发BFGIN shutdown命令 */
    ps_print_info("[%s]uart shutdown CPU\n", index2name(ps_core_d->pm_data->index));
    ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_SHUTDOWN_SLP);
    ret = wait_bfgx_close_done(ps_core_d);

    return ret;
}

static void print_uart_status_info(uint32_t subsys)
{
    ps_print_info("[%s]print uart status info before sending open or close cmd!\n", service_get_name(subsys));
    ps_print_warning("tty tx:%x, rx:%x, tx buf len:%x\n", g_uart_info.tty_tx_count,
                     g_uart_info.tty_rx_count, g_uart_info.tty_tx_len);
    ps_print_warning("uart tx:%x    rx:%x\n", g_uart_info.uart_tx_count, g_uart_info.uart_rx_count);
}

static void save_uart_status_info(struct ps_core_s *ps_core_d)
{
    struct serial_icounter_struct icount;
    struct tty_struct *tty = NULL;

    memset_s(&g_uart_info, sizeof(struct uart_status_debug), 0, sizeof(struct uart_status_debug));
    g_uart_info.tty_tx_count = oal_atomic_read(&(ps_core_d->tty_tx_cnt));
    g_uart_info.tty_rx_count = oal_atomic_read(&(ps_core_d->tty_rx_cnt));

    tty = ps_core_d->tty;
    if (tty == NULL) {
        ps_print_warning("in exception cause tty open fail... this can be null\n");
        return;
    }

    g_uart_info.tty_tx_len = tty_chars_in_buffer(tty);

    if (tty->ops->get_icount(tty, &icount) < 0) {
        ps_print_err("get icount error\n");
        return;
    }

    g_uart_info.uart_tx_count = icount.tx;
    g_uart_info.uart_rx_count = icount.rx;
}

static int32_t bfgx_open_cmd_check(struct ps_core_s *ps_core_d, uint32_t subsys)
{
    if (unlikely(ps_core_d == NULL || ps_core_d->pm_data == NULL)) {
        ps_print_err("ps_core_d  or pm_data is null\n");
        return -EINVAL;
    }

    if (subsys >= BFGX_BUTT) {
        ps_print_err("subsys is err, subsys is [%d]\n", subsys);
        return -EINVAL;
    }

#ifdef PLATFORM_DEBUG_ENABLE
    if (is_dfr_test_en(BFGX_POWEON_FAULT, (uintptr_t)ps_core_d)) {
        ps_print_warning("[dfr test]: %s trigger powon fail\n", index2name(ps_core_d->pm_data->index));
        cancel_excp_test_en(BFGX_POWEON_FAULT, (uintptr_t)ps_core_d);
        return -EINVAL;
    }
#endif

    return SUCCESS;
}

int32_t bfgx_open_cmd_send(struct ps_core_s *ps_core_d, uint32_t subsys)
{
    unsigned long timeleft;
    struct st_bfgx_data *pst_bfgx_data = NULL;

    if (bfgx_open_cmd_check(ps_core_d, subsys) != SUCCESS) {
        return -EINVAL;
    }

    /* 单红外dev不处理系统消息 */
    if (oal_atomic_read(&ps_core_d->ir_only) != 0) {
        return SUCCESS;
    }

    if (subsys == BFGX_IR) {
        ps_tx_sys_cmd(ps_core_d, SYS_MSG, service_get_open_cmd(subsys));
        msleep(20); // sleep 20ms
        return SUCCESS;
    }

    if (subsys == BFGX_GNSS) {
        g_gnss_me_thread_status = DEV_THREAD_EXIT;
        g_gnss_lppe_thread_status = DEV_THREAD_EXIT;
    }

    pst_bfgx_data = &ps_core_d->bfgx_info[subsys];

    save_uart_status_info(ps_core_d);
    oal_reinit_completion(pst_bfgx_data->wait_opened);
    ps_tx_sys_cmd(ps_core_d, SYS_MSG, service_get_open_cmd(subsys));

    timeleft = oal_wait_for_completion_timeout(&pst_bfgx_data->wait_opened,
                                               msecs_to_jiffies(service_get_open_timeout(subsys)));
    if (!timeleft) {
        print_uart_status_info(subsys);
        ps_uart_state_dump(ps_core_d);
        ps_print_err("wait %s open ack timeout\n", service_get_name(subsys));
        ssi_dump_device_regs(SSI_MODULE_MASK_AON | SSI_MODULE_MASK_ARM_REG | SSI_MODULE_MASK_GTRACE |
                             SSI_MODULE_MASK_BTRACE);
        if (subsys == BFGX_GNSS) {
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_OPEN, CHR_PLAT_DRV_ERROR_OPEN_GNSS);
        } else if (subsys == BFGX_BT) {
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_OPEN, CHR_PLAT_DRV_ERROR_OPEN_BT);
        }
        return -ETIMEDOUT;
    }

    return SUCCESS;
}

int32_t bfgx_close_cmd_send(struct ps_core_s *ps_core_d, uint32_t subsys)
{
    unsigned long timeleft;
    struct st_bfgx_data *pst_bfgx_data = NULL;

    if (unlikely(ps_core_d == NULL || ps_core_d->pm_data == NULL)) {
        ps_print_err("ps_core_d or pm_data is null\n");
        return -EINVAL;
    }

    if (subsys >= BFGX_BUTT) {
        ps_print_err("subsys is err, subsys is [%u]\n", subsys);
        return -EINVAL;
    }

    /* 单红外dev不处理系统消息 */
    if (oal_atomic_read(&ps_core_d->ir_only) != 0) {
        return SUCCESS;
    }

#ifdef PLATFORM_DEBUG_ENABLE
    if (is_dfr_test_en(BFGX_POWEOFF_FAULT, (uintptr_t)ps_core_d)) {
        ps_print_warning("[dfr test]: %s trigger power off fail\n", index2name(ps_core_d->pm_data->index));
        cancel_excp_test_en(BFGX_POWEOFF_FAULT, (uintptr_t)ps_core_d);
        return -EINVAL;
    }
#endif

    if (subsys == BFGX_IR) {
        ps_tx_sys_cmd(ps_core_d, SYS_MSG, service_get_close_cmd(subsys));
        msleep(20); // sleep 20ms
        return SUCCESS;
    }

    pst_bfgx_data = &ps_core_d->bfgx_info[subsys];

    save_uart_status_info(ps_core_d);
    oal_reinit_completion(pst_bfgx_data->wait_closed);
    ps_tx_sys_cmd(ps_core_d, SYS_MSG, service_get_close_cmd(subsys));
    timeleft = oal_wait_for_completion_timeout(&pst_bfgx_data->wait_closed,
                                               msecs_to_jiffies(service_get_close_timeout(subsys)));
    if (!timeleft) {
        print_uart_status_info(subsys);
        ps_uart_state_dump(ps_core_d);
        ps_print_err("wait %s close ack timeout\n", service_get_name(subsys));
        ssi_dump_device_regs(SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_GTRACE | SSI_MODULE_MASK_BTRACE);
        if (subsys == BFGX_GNSS) {
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_CLOSE, CHR_PLAT_DRV_ERROR_CLOSE_GNSS);
        } else if (subsys == BFGX_BT) {
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_CLOSE, CHR_PLAT_DRV_ERROR_CLOSE_BT);
        }
        return -ETIMEDOUT;
    }

    return SUCCESS;
}

STATIC int32_t hw_bfgx_start(struct ps_core_s *ps_core_d, uint32_t subsys)
{
    struct st_bfgx_data *pst_bfgx_data = NULL;
    struct pm_drv_data *pm_data = NULL;
    int32_t error;
    int32_t prepare_result;

    pm_data = ps_core_d->pm_data;
    pst_bfgx_data = &ps_core_d->bfgx_info[subsys];

    /* 当单红外模式打开其他子系统时，需要关闭单红外才能其他子系统正常上电 */
    if ((ps_core_ir_only_mode() == OAL_TRUE) && (subsys != BFGX_IR)) {
        if (hw_bfgx_close(bfgx_get_core_by_dev_name(HW_IR_DEV_NAME), BFGX_IR) != BFGX_POWER_SUCCESS) {
            ps_print_err("ir only mode,but close ir only mode fail!\n");
            return BFGX_POWER_FAILED;
        }
    }

    error = pm_data->bfg_power_set(pm_data, subsys, BFG_POWER_GPIO_UP);
    if (error != BFGX_POWER_SUCCESS) {
        ps_print_err("[%s]set %s power on error=%d\n", index2name(pm_data->index), service_get_name(subsys), error);
        goto bfgx_power_on_fail;
    }

    if (prepare_to_visit_node(ps_core_d, &prepare_result) != BFGX_POWER_SUCCESS) {
        error = BFGX_POWER_WAKEUP_FAIL;
        goto bfgx_wakeup_fail;
    }

    /* 单红外没有低功耗 */
    if (oal_atomic_read(&ps_core_d->ir_only) == 0) {
        ps_print_info("open %s mod_timer %d\n", service_get_name(subsys),
                      BT_SLEEP_TIME + service_get_open_timeout(subsys));
        mod_timer(&pm_data->bfg_timer, jiffies + msecs_to_jiffies(BT_SLEEP_TIME + service_get_open_timeout(subsys)));
        oal_atomic_inc(&pm_data->bfg_timer_mod_cnt);
    }

    if (bfgx_open_cmd_send(ps_core_d, subsys) != BFGX_POWER_SUCCESS) {
        ps_print_err("bfgx open cmd fail\n");
        error = BFGX_POWER_OPEN_CMD_FAIL;
        goto bfgx_open_cmd_fail;
    }

    atomic_set(&pst_bfgx_data->subsys_state, POWER_STATE_OPEN);
    post_to_visit_node(ps_core_d, prepare_result);

    return BFGX_POWER_SUCCESS;

bfgx_open_cmd_fail:
    post_to_visit_node(ps_core_d, prepare_result);
bfgx_wakeup_fail:
bfgx_power_on_fail:
    bfgx_open_fail_process(ps_core_d, subsys, error);
    return BFGX_POWER_FAILED;
}

int32_t hw_bfgx_open(struct ps_core_s *ps_core_d, uint32_t subsys)
{
    int32_t ret;
    struct st_bfgx_data *pst_bfgx_data = NULL;

    ps_print_info("open %s\n", service_get_name(subsys));
    pst_bfgx_data = &ps_core_d->bfgx_info[subsys];

    if (atomic_read(&pst_bfgx_data->subsys_state) == POWER_STATE_OPEN) {
        ps_print_warning("[%s]%s has opened! It's Not necessary to send msg to device\n",
                         index2name(ps_core_d->uart_index), service_get_name(subsys));
        return BFGX_POWER_SUCCESS;
    }

    if (alloc_seperted_rx_buf(ps_core_d, subsys, g_bfgx_rx_max_frame[subsys], VMALLOC) != BFGX_POWER_SUCCESS) {
        return -ENOMEM;
    }

    ret = hw_bfgx_start(ps_core_d, subsys);
    if (oal_unlikely(ret != BFGX_POWER_SUCCESS)) {
        free_seperted_rx_buf(ps_core_d, subsys, VMALLOC);
    }

    return ret;
}

OAL_STATIC int32_t bfgx_send_close_cmd(struct ps_core_s *ps_core_d, uint32_t subsys)
{
    struct pm_drv_data *pm_data = ps_core_d->pm_data;
    int32_t prepare_result;

    if (prepare_to_visit_node(ps_core_d, &prepare_result) < 0) {
        ps_print_err("[%s] prepare work FAIL\n", index2name(pm_data->index));
        return -FAILURE;
    }

    if (bfgx_close_cmd_send(ps_core_d, subsys) < 0) {
        ps_print_err("[%s] bfgx close cmd fail\n", index2name(pm_data->index));
        post_to_visit_node(ps_core_d, prepare_result);
        return -FAILURE;
    }

    post_to_visit_node(ps_core_d, prepare_result);
    return SUCCESS;
}

int32_t hw_bfgx_close(struct ps_core_s *ps_core_d, uint32_t subsys)
{
    int32_t ret;
    struct st_bfgx_data *pst_bfgx_data = NULL;
    struct pm_drv_data *pm_data = NULL;

    ps_print_info("close %s\n", service_get_name(subsys));

    pm_data = ps_core_d->pm_data;
    pst_bfgx_data = &ps_core_d->bfgx_info[subsys];

    if (atomic_read(&pst_bfgx_data->subsys_state) == POWER_STATE_SHUTDOWN) {
        ps_print_warning("[%s]%s has closed! It's Not necessary to send msg to device\n",
                         index2name(pm_data->index), service_get_name(subsys));
        return BFGX_POWER_SUCCESS;
    }
    oal_wait_queue_wake_up_interrupt(&pst_bfgx_data->rx_wait);

    ret = bfgx_send_close_cmd(ps_core_d, subsys);
    if (ret < 0) {
        if (bfgx_other_subsys_all_shutdown(pm_data, subsys) == false) {
            ps_print_err("[%s] bring to reset work\n", index2name(pm_data->index));
            plat_exception_handler(SUBSYS_BFGX, THREAD_BT, get_poweroff_fail_excp(subsys));
        }
    }

    atomic_set(&pst_bfgx_data->subsys_state, POWER_STATE_SHUTDOWN);
    free_seperted_rx_buf(ps_core_d, subsys, VMALLOC);
    ps_kfree_skb(ps_core_d, g_bfgx_rx_queue[subsys]);

    pst_bfgx_data->rx_pkt_num = 0;
    pst_bfgx_data->tx_pkt_num = 0;

    if (ps_chk_only_gnss_and_sleep(ps_core_d) == true) {
        ps_print_info("[%s]gnss only request sleep\n", index2name(pm_data->index));
        mod_timer(&pm_data->bfg_timer, jiffies + msecs_to_jiffies(PLATFORM_SLEEP_TIME));
        oal_atomic_inc(&pm_data->bfg_timer_mod_cnt);
    }

    ret = pm_data->bfg_power_set(pm_data, subsys, BFG_POWER_GPIO_DOWN);
    if (ret) {
        ps_print_err("set %s power off err!ret = %d", service_get_name(subsys), ret);
    }

    return SUCCESS;
}

void hw_bfgx_close_for_excp(struct pm_drv_data *pm_data, uint32_t flag)
{
    uint32_t i;
    struct ps_core_s *ps_core_d = NULL;
    struct st_bfgx_data *pst_bfgx_data = NULL;
    struct st_exception_info *pst_exception_data = get_exception_info_reference();

    if (oal_unlikely(pst_exception_data == NULL)) {
        ps_print_err("[HI110X_DFR]get exception info reference is error\n");
        return;
    }

    ps_print_info("uart:%s"NEWLINE, index2name(pm_data->index));
    ps_core_d = pm_data->ps_core_data;
    bfgx_timer_clear(pm_data);
    for (i = 0; i < BFGX_BUTT; i++) {
        pst_bfgx_data = &ps_core_d->bfgx_info[i];
        if (atomic_read(&pst_bfgx_data->subsys_state) == POWER_STATE_OPEN) {
            ps_print_info("close %s "NEWLINE, pst_bfgx_data->name);
            atomic_set(&pst_bfgx_data->subsys_state, POWER_STATE_SHUTDOWN);

            free_seperted_rx_buf(ps_core_d, i, VMALLOC);
            ps_kfree_skb(ps_core_d, g_bfgx_rx_queue[i]);

            pst_bfgx_data->rx_pkt_num = 0;
            pst_bfgx_data->tx_pkt_num = 0;
            if (flag == OAL_TRUE) { // 主核, 拉下电
                pm_data->bfg_power_set(pm_data, i, BFG_POWER_GPIO_DOWN);
            } else {
                bfgx_gpio_intr_enable(pm_data, OAL_FALSE);
                release_tty_drv(ps_core_d);
                atomic_set(&pm_data->bfg_needwait_devboot_flag, NEED_SET_FLAG);
            }
        }
    }
}

// 子系统关闭则不再写数据
int32_t hw_bfgx_write_check(const char __user *buf, struct ps_core_s *ps_core_d,
                            uint32_t subsys)
{
    struct st_bfgx_data *pst_bfgx_data = NULL;
    if (unlikely((ps_core_d == NULL) || (buf == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    pst_bfgx_data = &ps_core_d->bfgx_info[subsys];
    if (atomic_read(&pst_bfgx_data->subsys_state) == POWER_STATE_SHUTDOWN) {
        ps_print_warning("%s has closed! It's Not necessary to send msg to device\n", service_get_name(subsys));
        return -EINVAL;
    }
    return SUCCESS;
}

/*
 * 函 数 名  : hw_plat_open
 * 功能描述  : 不支持NFC的情况下，平台利用该接口单独上电，用于平台功能调试
               uart:指定命令通过哪个uart下发。mp16c支持双uart独立启动
 */
int32_t hw_plat_open(uint8_t uart)
{
    int32_t ret;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_top* pm_top_data = pm_get_top();

    if (uart > UART_BUTT) {
        return -EINVAL;
    }

    if (get_mpxx_subchip_type() == BOARD_VERSION_MP16C) {
        g_mp16c_srv_desc[BFGX_NFC].bus_id = uart;
    } else if (get_mpxx_subchip_type() == BOARD_VERSION_GF61) {
        g_gf61_srv_desc[BFGX_NFC].bus_id = uart;
    } else {
        g_mpxx_srv_desc[BFGX_NFC].bus_id = BUART;
    }

    mutex_lock(&(pm_top_data->host_mutex));
    ps_core_d = ps_get_core_reference(service_get_bus_id(BFGX_NFC));
    ret = hw_bfgx_open(ps_core_d, BFGX_NFC);
    mutex_unlock(&(pm_top_data->host_mutex));
    return ret;
}

int32_t hw_plat_close(uint8_t uart)
{
    int32_t ret;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_top* pm_top_data = pm_get_top();

    if (uart > UART_BUTT) {
        return -EINVAL;
    }

    if (get_mpxx_subchip_type() == BOARD_VERSION_MP16C) {
        g_mp16c_srv_desc[BFGX_NFC].bus_id = uart;
    } else if (get_mpxx_subchip_type() == BOARD_VERSION_GF61) {
        g_gf61_srv_desc[BFGX_NFC].bus_id = uart;
    } else {
        g_mpxx_srv_desc[BFGX_NFC].bus_id = BUART;
    }

    mutex_lock(&(pm_top_data->host_mutex));
    ps_core_d = ps_get_core_reference(service_get_bus_id(BFGX_NFC));
    ret = hw_bfgx_close(ps_core_d, BFGX_NFC);
    mutex_unlock(&(pm_top_data->host_mutex));
    return ret;
}

uart_loop_cfg g_uart_loop_test_cfg = { 256, 60000, 0, 0, 0 };
uart_loop_test_struct *g_uart_loop_test_info = NULL;

STATIC void uart_loop_test_tx_buf_init(uint8_t *puc_data, uint16_t us_data_len)
{
    get_random_bytes(puc_data, us_data_len);
}

STATIC int32_t alloc_uart_loop_test(void)
{
    uint8_t *uart_loop_tx_buf = NULL;
    uint8_t *uart_loop_rx_buf = NULL;
    uint16_t pkt_len = 0;

    if (g_uart_loop_test_info == NULL) {
        g_uart_loop_test_info = (uart_loop_test_struct *)kzalloc(sizeof(uart_loop_test_struct), GFP_KERNEL);
        if (g_uart_loop_test_info == NULL) {
            ps_print_err("malloc uart_loop_test_info fail\n");
            goto malloc_test_info_fail;
        }

        pkt_len = g_uart_loop_test_cfg.pkt_len;
        if (pkt_len == 0 || pkt_len > UART_LOOP_MAX_PKT_LEN) {
            pkt_len = UART_LOOP_MAX_PKT_LEN;
            g_uart_loop_test_cfg.pkt_len = UART_LOOP_MAX_PKT_LEN;
        }

        uart_loop_tx_buf = (uint8_t *)kzalloc(pkt_len, GFP_KERNEL);
        if (uart_loop_tx_buf == NULL) {
            ps_print_err("malloc uart_loop_tx_buf fail\n");
            goto malloc_tx_buf_fail;
        }

        memset_s(uart_loop_tx_buf, pkt_len, 0xa5, pkt_len);

        uart_loop_rx_buf = (uint8_t *)kzalloc(pkt_len, GFP_KERNEL);
        if (uart_loop_rx_buf == NULL) {
            ps_print_err("malloc uart_loop_rx_buf fail\n");
            goto malloc_rx_buf_fail;
        }

        g_uart_loop_test_cfg.uart_loop_enable = 1;
        g_uart_loop_test_cfg.uart_loop_tx_random_enable = 1;

        init_completion(&g_uart_loop_test_info->set_done);
        init_completion(&g_uart_loop_test_info->loop_test_done);

        g_uart_loop_test_info->test_cfg = &g_uart_loop_test_cfg;
        g_uart_loop_test_info->tx_buf = uart_loop_tx_buf;
        g_uart_loop_test_info->rx_buf = uart_loop_rx_buf;
        g_uart_loop_test_info->rx_pkt_len = 0;

        ps_print_info("uart loop test, pkt len is [%d]\n", pkt_len);
        ps_print_info("uart loop test, loop count is [%d]\n", g_uart_loop_test_cfg.loop_count);
    }

    return SUCCESS;

malloc_rx_buf_fail:
    kfree(uart_loop_tx_buf);
malloc_tx_buf_fail:
    kfree(g_uart_loop_test_info);
    g_uart_loop_test_info = NULL;
malloc_test_info_fail:
    return -ENOMEM;
}

STATIC void free_uart_loop_test(void)
{
    if (g_uart_loop_test_info == NULL) {
        return;
    }
    ps_print_info("free uart loop test buf\n");
    g_uart_loop_test_cfg.uart_loop_enable = 0;
    kfree(g_uart_loop_test_info->rx_buf);
    kfree(g_uart_loop_test_info->tx_buf);
    kfree(g_uart_loop_test_info);
    g_uart_loop_test_info = NULL;

    return;
}

STATIC int32_t uart_loop_test_check(struct ps_core_s *ps_core_d)
{
    struct pm_drv_data *pm_data = NULL;
    struct st_bfgx_data *bfgx_data = NULL;
    int32_t prepare_result;

    if (unlikely((ps_core_d == NULL) || (ps_core_d->pm_data == NULL) ||
        (ps_core_d->pm_data->bfg_power_set == NULL))) {
        ps_print_err("ps_core_d is err\n");
        return -EINVAL;
    }

    if (ps_core_chk_bfgx_active(ps_core_d)) {
        ps_print_err("bfgx subsys must all close\n");
        return -EINVAL;
    }

    if (alloc_uart_loop_test() != BFGX_POWER_SUCCESS) {
        ps_print_err("alloc mem for uart loop test fail!\n");
        return -ENOMEM;
    }

    pm_data = ps_core_d->pm_data;
    if (pm_data->bfg_power_set(pm_data, BFGX_GNSS, BFG_POWER_GPIO_UP) !=
                               BFGX_POWER_SUCCESS) {
        ps_print_err("uart loop test, power on err!\n");
        free_uart_loop_test();
        return  -EINVAL;
    }

    if (prepare_to_visit_node(ps_core_d, &prepare_result) != BFGX_POWER_SUCCESS) {
        ps_print_err("uart loop test, prepare work fail\n");
        free_uart_loop_test();
        pm_data->bfg_power_set(pm_data, BFGX_GNSS, BFG_POWER_GPIO_DOWN);
        return  -EINVAL;
    }

    bfgx_data = &ps_core_d->bfgx_info[BFGX_GNSS];
    atomic_set(&bfgx_data->subsys_state, POWER_STATE_OPEN);

    post_to_visit_node(ps_core_d, prepare_result);

    return BFGX_POWER_SUCCESS;
}

STATIC int32_t uart_loop_test_open(struct ps_core_s *ps_core_d)
{
    struct pm_top* pm_top_data = pm_get_top();

    ps_print_info("%s\n", __func__);

    if (oal_mutex_trylock(&pm_top_data->host_mutex) == 0) {
        ps_print_err("uart loop maybe aleray in test\n");
        return -EINVAL;
    }

    if (uart_loop_test_check(ps_core_d) != BFGX_POWER_SUCCESS) {
        ps_print_err("uart loop check fail\n");
        oal_mutex_unlock(&pm_top_data->host_mutex);
        return -EINVAL;
    }

    oal_mutex_unlock(&pm_top_data->host_mutex);

    return BFGX_POWER_SUCCESS;
}

STATIC void uart_loop_test_close(struct ps_core_s *ps_core_d)
{
    struct st_bfgx_data *bfgx_data = NULL;
    struct pm_top* pm_top_data = pm_get_top();
    int32_t prepare_result;

    ps_print_info("%s", __func__);

    oal_mutex_lock(&pm_top_data->host_mutex);
    free_uart_loop_test();

    if (unlikely((ps_core_d == NULL) || (ps_core_d->pm_data == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        oal_mutex_unlock(&pm_top_data->host_mutex);
        return;
    }

    if (prepare_to_visit_node(ps_core_d, &prepare_result) != 0) {
        ps_print_err("uart loop test, prepare work fail\n");
    }

    bfgx_data = &ps_core_d->bfgx_info[BFGX_GNSS];
    atomic_set(&bfgx_data->subsys_state, POWER_STATE_SHUTDOWN);

    if (ps_core_d->pm_data->bfg_power_set(ps_core_d->pm_data, BFGX_GNSS, BFG_POWER_GPIO_DOWN) != 0) {
        ps_print_err("uart loop test, power off err!");
    }

    post_to_visit_node(ps_core_d, prepare_result);
    oal_mutex_unlock(&pm_top_data->host_mutex);
}

STATIC int32_t uart_loop_test_set(uint8_t uart, uint8_t flag)
{
    unsigned long timeleft;
    uint8_t cmd;
    int32_t prepare_result;

    struct ps_core_s *ps_core_d = ps_get_core_reference(uart);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is null\n");
        return -EINVAL;
    }

    if (flag == UART_LOOP_SET_DEVICE_DATA_HANDLER) {
        cmd = SYS_CFG_SET_UART_LOOP_HANDLER;
    } else {
        cmd = SYS_CFG_SET_UART_LOOP_FINISH;
    }

    if (prepare_to_visit_node(ps_core_d, &prepare_result) < 0) {
        ps_print_err("prepare wakeup fail\n");
        return -EFAULT;
    }

    oal_reinit_completion(g_uart_loop_test_info->set_done);
    ps_tx_sys_cmd(ps_core_d, SYS_MSG, cmd);
    timeleft = wait_for_completion_timeout(&g_uart_loop_test_info->set_done,
                                           msecs_to_jiffies(UART_TEST_DEV_ACK_TIMEOUT_MS));
    if (!timeleft) {
        ps_uart_state_dump(ps_core_d);
        post_to_visit_node(ps_core_d, prepare_result);
        ps_print_err("wait set uart loop ack timeout\n");
        return -ETIMEDOUT;
    }

    post_to_visit_node(ps_core_d, prepare_result);
    return SUCCESS;
}

STATIC int32_t uart_loop_test_send_data(struct ps_core_s *ps_core_d, uint8_t *buf, size_t count)
{
    struct sk_buff *skb = NULL;
    uint16_t tx_skb_len;
    uint16_t tx_gnss_len;
    uint8_t start = 0;

    PS_PRINT_FUNCTION_NAME;

    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    while (count > 0) {
        if (count > PS_TX_PACKET_LIMIT) {
            tx_gnss_len = PS_TX_PACKET_LIMIT;
        } else {
            tx_gnss_len = count;
        }
        /* curr tx skb total lenth */
        tx_skb_len = tx_gnss_len + (uint16_t)sizeof(struct ps_packet_head);
        tx_skb_len = tx_skb_len + (uint16_t)sizeof(struct ps_packet_end);

        skb = ps_alloc_skb(tx_skb_len);
        if (skb == NULL) {
            ps_print_err("ps alloc skb mem fail\n");
            return -EFAULT;
        }

        if (count > PS_TX_PACKET_LIMIT) {
            if (start == false) { /* this is a start gnss packet */
                ps_add_packet_head(skb->data, GNSS_FIRST_MSG, tx_skb_len);
                start = true;
            } else { /* this is a int gnss packet */
                ps_add_packet_head(skb->data, GNSS_COMMON_MSG, tx_skb_len);
            }
        } else { /* this is the last gnss packet */
            ps_add_packet_head(skb->data, GNSS_LAST_MSG, tx_skb_len);
        }

        if (memcpy_s(&skb->data[sizeof(struct ps_packet_head)], tx_skb_len - sizeof(struct ps_packet_head),
                     buf, tx_gnss_len) != EOK) {
            ps_print_err("buf is not enough\n");
        }

        /* push the skb to skb queue */
        ps_skb_enqueue(ps_core_d, skb, TX_LOW_QUEUE);
        ps_core_tx_work_add(ps_core_d);

        buf = buf + tx_gnss_len;
        count = count - tx_gnss_len;
    }

    return SUCCESS;
}

STATIC int32_t uart_loop_test_send_pkt(uint8_t uart)
{
    unsigned long timeleft;
    int32_t prepare_result;

    struct ps_core_s *ps_core_d = ps_get_core_reference(uart);
    if (unlikely((ps_core_d == NULL) || (g_uart_loop_test_info == NULL) ||
                 (g_uart_loop_test_info->tx_buf == NULL))) {
        ps_print_err("para is invalided\n");
        return -EFAULT;
    }

    /* if low queue num > MAX_NUM and don't write */
    if (oal_netbuf_list_len(&ps_core_d->tx_low_seq) > TX_LOW_QUE_MAX_NUM) {
        ps_print_err("uart loop test, tx low seq is too large [%d]\n", oal_netbuf_list_len(&ps_core_d->tx_low_seq));
        return SUCCESS;
    }

    if (prepare_to_visit_node(ps_core_d, &prepare_result) < 0) {
        ps_print_err("prepare work fail\n");
        return -EFAULT;
    }

    oal_reinit_completion(g_uart_loop_test_info->loop_test_done);

    /* to divide up packet function and tx to tty work */
    if (uart_loop_test_send_data(ps_core_d, g_uart_loop_test_info->tx_buf, g_uart_loop_test_cfg.pkt_len) < 0) {
        ps_print_err("uart loop test pkt send is err\n");
        post_to_visit_node(ps_core_d, prepare_result);
        return -EFAULT;
    }

    timeleft = wait_for_completion_timeout(&g_uart_loop_test_info->loop_test_done,
                                           msecs_to_jiffies(UART_TEST_DEV_ACK_TIMEOUT_MS));
    if (!timeleft) {
        ps_uart_state_dump(ps_core_d);
        ps_print_err("wait uart loop done timeout\n");
        post_to_visit_node(ps_core_d, prepare_result);
        return -ETIMEDOUT;
    }

    post_to_visit_node(ps_core_d, prepare_result);

    return SUCCESS;
}

STATIC int32_t uart_loop_test_data_check(uint8_t *puc_src, uint8_t *puc_dest, uint16_t us_data_len)
{
    uint16_t us_index;

    for (us_index = 0; us_index < us_data_len; us_index++) {
        if (puc_src[us_index] != puc_dest[us_index]) {
            ps_print_err("puc_src[%u] = %x, puc_dest[%u] = %x\n", us_index,
                         puc_src[us_index], us_index, puc_dest[us_index]);
            return false;
        }
    }

    return true;
}

int32_t uart_loop_test_recv_pkt(struct ps_core_s *ps_core_d, const uint8_t *buf_ptr, uint16_t pkt_len)
{
    uint16_t expect_pkt_len;
    uint8_t *rx_buf = NULL;
    uint16_t recvd_len;

    if (unlikely((ps_core_d == NULL) || (g_uart_loop_test_info == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    expect_pkt_len = g_uart_loop_test_info->test_cfg->pkt_len;
    rx_buf = g_uart_loop_test_info->rx_buf;
    recvd_len = g_uart_loop_test_info->rx_pkt_len;

    if ((uint32_t)recvd_len + (uint32_t)pkt_len <= expect_pkt_len) {
        if (memcpy_s(&rx_buf[recvd_len], expect_pkt_len - recvd_len, buf_ptr, pkt_len) != EOK) {
            ps_print_err("memcpy_s error, destlen=%d, srclen=%d\n ", expect_pkt_len - recvd_len, pkt_len);
        }
        g_uart_loop_test_info->rx_pkt_len += pkt_len;
    } else {
        ps_print_err("pkt len err! pkt_len=[%u], recvd_len=[%u], max len=[%u]\n", pkt_len, recvd_len, expect_pkt_len);
    }

    if (expect_pkt_len == g_uart_loop_test_info->rx_pkt_len) {
        if (uart_loop_test_data_check(rx_buf, g_uart_loop_test_info->tx_buf, expect_pkt_len)) {
            ps_print_info("uart loop recv pkt SUCC\n");
        }
        g_uart_loop_test_info->rx_pkt_len = 0;
        complete(&g_uart_loop_test_info->loop_test_done);
    }

    return SUCCESS;
}

STATIC void uart_loop_result(uint8_t uart, uint32_t count, uint16_t pkt_len, unsigned long long total_time)
{
    unsigned long long throughout, tx_total_len, effect;
    unsigned long baud_rate;
    struct ps_core_s *ps_core_d = ps_get_core_reference(BUART);
    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is null!\n");
        return;
    }
    /* 将传输字节数转换成bit数(B->b)以方便后文计算 */
    tx_total_len = ((unsigned long long)count) * ((unsigned long long)pkt_len);
    /* 1000000->1M波特率，10=8+2->传输1字节需加2bit的开始位和结束位，2->uart是环回，发送后需回 */
    throughout = tx_total_len * 1000000 * 10 * 2;
    if (total_time == 0) {
        ps_print_err("divisor can not be zero!\n");
        return;
    }
    do_div(throughout, total_time);
    effect = throughout;
    do_div(throughout, 8192); /* b->B->KB : x*8*1024 = 8192 */

    baud_rate = ps_get_default_baud_rate(ps_core_d);
    do_div(effect, (baud_rate / 100)); /* 除以100算出百分比效率 */

    ps_print_info("[UART Test] pkt count      [%u] pkts sent\n", count);
    ps_print_info("[UART Test] pkt len        [%u] is pkt len\n", pkt_len);
    ps_print_info("[UART Test] data lenth     [%llu]\n", tx_total_len * 2); /* uart是环回的，发1字节还会回1字节 即长度*2 */
    ps_print_info("[UART Test] used time      [%llu] us\n", total_time);
    ps_print_info("[UART Test] throughout     [%llu] KBps\n", throughout);
    ps_print_info("[UART Test] effect         [%llu]%%\n", effect);
}

int32_t uart_loop_test(struct ps_core_s *ps_core_d)
{
    uint32_t i, count;
    uint16_t pkt_len;
    unsigned long long total_time = 0;
    ktime_t start_time, end_time, trans_time;
    uint8_t *puc_buf = NULL;
    uint8_t uart = (uint8_t)ps_core_d->uart_index;

    if (uart_loop_test_open(ps_core_d) < 0) {
        goto open_fail;
    }

    if (uart_loop_test_set(uart, UART_LOOP_SET_DEVICE_DATA_HANDLER) < 0) {
        goto test_set_fail;
    }

    count = g_uart_loop_test_info->test_cfg->loop_count;
    pkt_len = g_uart_loop_test_info->test_cfg->pkt_len;
    puc_buf = g_uart_loop_test_info->tx_buf;

    for (i = 0; i < count; i++) {
        if (g_uart_loop_test_info->test_cfg->uart_loop_tx_random_enable) {
            uart_loop_test_tx_buf_init(puc_buf, pkt_len);  // 初始化tx_buf为随机数
        }

        start_time = ktime_get();

        if (uart_loop_test_send_pkt(uart) != SUCCESS) {
            ps_print_err("uart loop test fail, i=[%u]\n", i);
            goto send_pkt_fail;
        }

        end_time = ktime_get();
        trans_time = ktime_sub(end_time, start_time);
        total_time += (unsigned long long)ktime_to_us(trans_time);
    }

    if (uart_loop_test_set(uart, UART_LOOP_RESUME_DEVICE_DATA_HANDLER) < 0) {
        ps_print_err("uart loop test, resume device data handler failer\n");
    }

    uart_loop_test_close(ps_core_d);

    uart_loop_result(uart, count, pkt_len, total_time);

    return SUCCESS;

send_pkt_fail:
test_set_fail:
    uart_loop_test_close(ps_core_d);
open_fail:
    return -FAILURE;
}

MODULE_DESCRIPTION("Public serial Driver for huawei BT/FM/GNSS chips");
MODULE_LICENSE("GPL");
