/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: Creating the MP16 Product bfgx dev for External Implementation
 * Author: @CompanyNameTag
 */

#include "plat_pm.h"
#include "bfgx_bt.h"
#include "bfgx_ir.h"
#include "bfgx_gnss.h"
#include "bfgx_fm.h"
#include "bfgx_sle.h"
#include "bfgx_debug.h"
#include "board_bfgx.h"
#include "plat_debug.h"
#include "plat_uart.h"
#include "bfgx_dev.h"
#include "bfgx_excp_dbg.h"

STATIC bool bfgx_is_boot_up(struct pm_drv_data *pm_data)
{
    const int check_interval = 50;
    int retry = 20; // 1秒钟，重试20次

    while (conn_get_gpio_level(pm_data->ps_plat_data->host_wkup_io_type) != GPIO_LEVEL_HIGH) {
        if (retry-- <= 0) {
            return false;
        }
        msleep(check_interval);
    }
    return true;
}

STATIC int32_t gnss_me_open(struct ps_core_s *ps_core_d)
{
    struct st_bfgx_data *pst_bfgx_data = NULL;
    struct pm_drv_data *pm_data = NULL;

    ps_print_info("open gnss me\n");

    pst_bfgx_data = &ps_core_d->bfgx_info[BFGX_GNSS];
    if (atomic_read(&pst_bfgx_data->subsys_state) == POWER_STATE_OPEN) {
        ps_print_warning("gnss me has opened! It's Not necessary to send msg to device\n");
        return BFGX_POWER_SUCCESS;
    }
    pm_data = ps_core_d->pm_data;
    if (!bfgx_is_boot_up(pm_data)) {
        ps_print_err("gcpu is not boot up!\n");
        return BFGX_POWER_FAILED;
    }

    if (alloc_seperted_rx_buf(ps_core_d, BFGX_GNSS, GNSS_RX_MAX_FRAME, VMALLOC) != 0) {
        ps_print_err("gnss me alloc failed! len=%d\n", GNSS_RX_MAX_FRAME);
        return BFGX_POWER_FAILED;
    }

    if (open_tty_drv(ps_core_d) != BFGX_POWER_SUCCESS) {
        ps_print_err("gnss me open tty fail!\n");
        goto open_tty_fail;
    }

    oal_reinit_completion(pm_data->dev_bootok_ack_comp);
    atomic_set(&pm_data->bfg_needwait_devboot_flag, NEED_SET_FLAG);

    bfgx_gpio_intr_enable(pm_data, OAL_TRUE);

    if (bfgx_wait_bootup(ps_core_d->pm_data) != BFGX_POWER_SUCCESS) {
        ps_print_err("gnss me open wait bootup fail!\n");
        goto open_cmd_fail;
    }

    if (bfgx_open_cmd_send(ps_core_d, BFGX_GNSS) != BFGX_POWER_SUCCESS) {
        ps_print_err("bfgx open cmd fail\n");
        goto open_cmd_fail;
    }
    atomic_set(&pst_bfgx_data->subsys_state, POWER_STATE_OPEN);

    return BFGX_POWER_SUCCESS;

open_cmd_fail:
    bfgx_uart_state_set(pm_data, UART_NOT_READY);
    bfgx_dev_state_set(pm_data, BFGX_SLEEP);
    bfgx_gpio_intr_enable(pm_data, OAL_FALSE);
open_tty_fail:
    free_seperted_rx_buf(ps_core_d, BFGX_GNSS, VMALLOC);
    return BFGX_POWER_FAILED;
}

STATIC int32_t gnss_me_close(struct ps_core_s *ps_core_d)
{
    int32_t ret;
    int32_t prepare_result;
    struct st_bfgx_data *pst_bfgx_data = NULL;
    struct pm_drv_data *pm_data = NULL;

    pst_bfgx_data = &ps_core_d->bfgx_info[BFGX_GNSS];

    if (atomic_read(&pst_bfgx_data->subsys_state) == POWER_STATE_SHUTDOWN) {
        ps_print_warning("gnss me has closed! It's Not necessary to send msg to device\n");
        return BFGX_POWER_SUCCESS;
    }

    pm_data = ps_core_d->pm_data;

    ps_print_info("close gnss me\n");

    oal_wait_queue_wake_up_interrupt(&pst_bfgx_data->rx_wait);

    ret = prepare_to_visit_node(ps_core_d, &prepare_result);
    if (ret < 0) {
        /* 鍞ら啋澶辫触锛宐fgx close鏃剁殑鍞ら啋澶辫触涓嶈繘琛孌FR鎭㈠ */
        ps_print_err("[%s]prepare work FAIL\n", index2name(pm_data->index));
    }

    bfgx_timer_clear(pm_data);

    ret = bfgx_close_cmd_send(ps_core_d, BFGX_GNSS);
    if (ret < 0) {
        /* 鍙戦€乧lose鍛戒护澶辫触锛屼笉杩涜DFR锛岀户缁繘琛屼笅鐢垫祦绋嬶紝DFR鎭㈠寤惰繜鍒颁笅娆pen鏃舵垨鑰呭叾浠栦笟鍔¤繍琛屾椂杩涜 */
        ps_print_err("bfgx close cmd fail\n");
    }
    atomic_set(&pst_bfgx_data->subsys_state, POWER_STATE_SHUTDOWN);

    if (uart_bfgx_close_cmd(ps_core_d) != SUCCESS) {
        /* bfgx self close fail, go on */
        ps_print_err("gnss me self close fail\n");
    }

    bfgx_gpio_intr_enable(pm_data, OAL_FALSE);

    if (release_tty_drv(ps_core_d) != SUCCESS) {
        /* 浠ｇ爜鎵ц鍒版澶勶紝璇存槑鍏悎涓€鎵€鏈変笟鍔￠兘宸茬粡鍏抽棴锛屾棤璁簍ty鏄惁鍏抽棴鎴愬姛锛宒evice閮借涓嬬數 */
        ps_print_err("close gnss me tty is err!");
    }

    post_to_visit_node(ps_core_d, prepare_result);

    /* 涓嬬數鍗冲皢瀹屾垚锛岄渶瑕佸湪姝ゆ椂璁剧疆涓嬫涓婄數瑕佺瓑寰卍evice涓婄數鎴愬姛鐨刦lag */
    atomic_set(&pm_data->bfg_needwait_devboot_flag, NEED_SET_FLAG);

    free_seperted_rx_buf(ps_core_d, BFGX_GNSS, VMALLOC);
    ps_kfree_skb(ps_core_d, RX_GNSS_QUEUE);

    bfgx_uart_state_set(pm_data, UART_NOT_READY);
    bfgx_dev_state_set(pm_data, BFGX_SLEEP);
    bfg_wake_unlock(pm_data);

    return BFGX_POWER_SUCCESS;
}

STATIC int32_t hw_gnss_me_open(struct inode *inode, struct file *filp)
{
    int32_t ret;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_top* pm_top_data = pm_get_top();

    ps_core_d = get_ps_core_from_miscdev(filp->private_data);
    if (unlikely(ps_core_d == NULL || inode == NULL)) {
        ps_print_err("%s param is NULL\n", __func__);
        return -EINVAL;
    }

    mutex_lock(&pm_top_data->host_mutex);
    atomic_set(&ps_core_d->pm_data->gnss_sleep_flag, GNSS_NOT_AGREE_SLEEP);

    ret = gnss_me_open(ps_core_d);
    if (ret != BFGX_POWER_SUCCESS) {
        atomic_set(&ps_core_d->pm_data->gnss_sleep_flag, GNSS_AGREE_SLEEP);
    }

    mutex_unlock(&pm_top_data->host_mutex);
    return ret;
}

STATIC int32_t hw_gnss_me_release(struct inode *inode, struct file *filp)
{
    int32_t ret;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_top* pm_top_data = pm_get_top();

    ps_core_d = get_ps_core_from_miscdev(filp->private_data);
    if (unlikely(ps_core_d == NULL || inode == NULL)) {
        ps_print_err("%s param is NULL\n", __func__);
        return -EINVAL;
    }

    mutex_lock(&pm_top_data->host_mutex);

    ret = gnss_me_close(ps_core_d);

    atomic_set(&ps_core_d->pm_data->gnss_sleep_flag, GNSS_AGREE_SLEEP);
    ps_core_d->pm_data->gnss_frc_awake = 0;

    mutex_unlock(&pm_top_data->host_mutex);

    return ret;
}

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

// for directory /sys/devices/virtual/misc/hwgnss_me
STATIC struct attribute *g_hwgnss_me_attrs[] = {
#ifdef PLATFORM_DEBUG_ENABLE
    &dev_attr_excp_dbg.attr,
#endif
    NULL
};
ATTRIBUTE_GROUPS(g_hwgnss_me);

STATIC struct file_operations g_default_gnss_me_fops;
STATIC struct bfgx_subsys_driver_desc g_default_gnss_me_drv_desc;

struct bfgx_subsys_driver_desc *get_gnss_me_drv_desc(void)
{
    struct bfgx_subsys_driver_desc *drv_desc = NULL;

    drv_desc = get_gnss_default_drv_desc();
    if (memcpy_s(&g_default_gnss_me_fops, sizeof(struct file_operations),
        drv_desc->file_ops, sizeof(struct file_operations)) != EOK) {
        ps_print_err("memcpy_s error\n");
        return NULL;
    }

    g_default_gnss_me_fops.open = hw_gnss_me_open;
    g_default_gnss_me_fops.release = hw_gnss_me_release;
    g_default_gnss_me_drv_desc.file_ops = &g_default_gnss_me_fops;
    g_default_gnss_me_drv_desc.groups = g_hwgnss_me_groups;

    return &g_default_gnss_me_drv_desc;
}

STATIC struct bfgx_subsys_dev_desc g_bfgx_dev_desc[] = {
    {
        .name = HW_BT_DEV_NAME, .uart = BUART, .get_driver_desc = get_bt_default_drv_desc
    },
    {
        .name = HW_IR_DEV_NAME, .uart = BUART, .get_driver_desc = get_ir_default_drv_desc
    },
    {
        .name = HW_GNSS_DEV_NAME, .uart = BUART, .get_driver_desc = get_gnss_default_drv_desc
    },
    {
        .name = HW_GNSS_ME_DEV_NAME, .uart = GUART, .get_driver_desc = get_gnss_me_drv_desc
    },
    {
        .name = HW_FM_DEV_NAME, .uart = BUART, .get_driver_desc = get_fm_default_drv_desc
    },
    {
        .name = HW_BFGDBG_DEV_NAME, .uart = BUART, .get_driver_desc = get_bfgxdbg_default_drv_desc
    },
    {
        .name = HW_BFGDBG_DEV_NAME0, .uart = GUART, .get_driver_desc = get_bfgxdbg_default_drv_desc
    },
    {
        .name = HW_EXCP_BFGX_DEV_NAME, .uart = BUART, .get_driver_desc = get_excp_default_drv_desc,
        .private_data = &g_bcpu_memdump_cfg
    },
    {
        .name = HW_EXCP_ME_DEV_NAME, .uart = GUART, .get_driver_desc = get_excp_default_drv_desc,
        .private_data = &g_gcpu_memdump_cfg
    },
    {
        .name = HW_EXCP_WIFI_DEV_NAME, .uart = BUART, .get_driver_desc = get_excp_default_drv_desc,
        .private_data = &g_wcpu_memdump_cfg
    }
};

STATIC struct bfgx_prj_desc g_prj_desc = {
    .subsys_desc = g_bfgx_dev_desc,
    .desc_num = oal_array_size(g_bfgx_dev_desc)
};

struct bfgx_prj_desc *mp16_bfgx_prj_desc_init(void)
{
    return &g_prj_desc;
}

void mp16_prj_desc_remove(void)
{
}
