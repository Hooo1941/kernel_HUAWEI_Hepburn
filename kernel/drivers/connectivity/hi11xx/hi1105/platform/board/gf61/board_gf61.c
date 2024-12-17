/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: Obtain resources
 * Author: @CompanyNameTag
 */

/* 头文件包含 */
#ifdef _PRE_CONFIG_USE_DTS
#include <linux/of.h>
#include <linux/of_gpio.h>
#endif

#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/fs.h>

#include "board.h"
#include "bfgx_dev.h"
#include "plat_pm.h"
#include "plat_uart.h"
#include "plat_firmware.h"
#include "plat_firmware_download.h"
#include "plat_pm_gt.h"
#include "plat_debug.h"
#include "oal_hcc_host_if.h"
#include "oam_ext_if.h"
#include "plat_cust.h"
#include "plat_common_clk.h"
#include "board_gf61.h"
#ifdef _PRE_PLAT_FEATURE_GF6X_PCIE
#include "pcie_linux_gfxx.h"
#endif

STATIC int32_t gf61_board_power_on(uint32_t ul_subsystem);
STATIC int32_t gf61_board_power_off(uint32_t ul_subsystem);

#define OAL_WAKEUP_RX_SEM_DALAY 1000

oal_timer_list_stru g_wifi_rx_sem_timer;
oal_timer_list_stru g_gt_rx_sem_timer;

typedef struct plat_cust_data {
    uint32_t lowpower_cfg_value;
    uint32_t clk_mode_value;
} plat_cust_data_t;

typedef struct _gpio_cfg_table_ {
    uint32_t reg_base;
    uint32_t d2h_io;
    uint32_t h2d_io;
} gpio_cfg_table;

gpio_cfg_table gf61_gpio_cfg_table[SYS_BUTT] = {
    { WLAN_GPIO_BASE_ADDR, WL_DEV2HOST_GPIO,  WL_HOST2DEV_GPIO  },
    { BT_GPIO_BASE_ADDR,   B_DEV2HOST_GPIO,   B_HOST2DEV_GPIO   },
    { SLE_GPIO_BASE_ADDR,  SLE_DEV2HOST_GPIO, SLE_HOST2DEV_GPIO },
    { GT_GPIO_BASE_ADDR,   GT_DEV2HOST_GPIO,  GT_HOST2DEV_GPIO  },
};

static bool wakeup_rx_sem_action(struct hcc_handler *hcc)
{
    hcc_bus* bus = NULL;

    if (hcc == NULL) {
        printk("week up rx sem timer, hcc is null\n");
        return false;
    }

    bus = hcc_to_bus(hcc);
    if (bus == NULL) {
        printk("week up rx sem timer, bus is null\n");
        return false;
    }
    up(&bus->rx_sema);
    return true;
}

static void wifi_wakeup_rx_sem_action(oal_timeout_func_para_t arg)
{
    struct hcc_handler *hcc = hcc_get_handler(HCC_EP_WIFI_DEV);
    if (!wakeup_rx_sem_action(hcc)) {
        return;
    }
    oal_timer_start(&g_wifi_rx_sem_timer, OAL_WAKEUP_RX_SEM_DALAY);
}

static void gt_wakeup_rx_sem_action(oal_timeout_func_para_t arg)
{
    struct hcc_handler *hcc = hcc_get_handler(HCC_EP_GT_DEV);
    if (!wakeup_rx_sem_action(hcc)) {
        return;
    }
    oal_timer_start(&g_gt_rx_sem_timer, OAL_WAKEUP_RX_SEM_DALAY);
}

static void rx_sem_timer_init(uint8_t dev_id)
{
    if (dev_id == HCC_EP_WIFI_DEV) {
        oal_timer_init(&g_wifi_rx_sem_timer, OAL_WAKEUP_RX_SEM_DALAY << 1, wifi_wakeup_rx_sem_action, 0); // 2ms
        oal_timer_add(&g_wifi_rx_sem_timer);
    } else {
        oal_timer_init(&g_gt_rx_sem_timer, OAL_WAKEUP_RX_SEM_DALAY << 1, gt_wakeup_rx_sem_action, 0); // 2ms
        oal_timer_add(&g_gt_rx_sem_timer);
    }
    printk("rx sem timer init succ\n");
}

static void rx_sem_timer_del(uint8_t dev_id)
{
    oal_timer_list_stru *timer = NULL;
    if (dev_id == HCC_EP_WIFI_DEV) {
        timer = &g_wifi_rx_sem_timer;
    } else {
        timer = &g_gt_rx_sem_timer;
    }
    oal_timer_delete_sync(timer);
    printk("rx sem delele timer devid %d\n", dev_id);
}

STATIC int32_t gf61_bfgx_download(uint32_t sys)
{
    int32_t ret;
    uint32_t which_cfg = (sys == B_SYS) ? BFGX_CFG : SLE_CFG;

    ret = firmware_download_function(which_cfg, hcc_get_bus(HCC_EP_WIFI_DEV));
    if (ret != BFGX_POWER_SUCCESS) {
        ps_print_err("bfgx download firmware fail, cfg[%d]!\n", which_cfg);
        return (ret == -OAL_EINTR) ? BFGX_POWER_DOWNLOAD_FIRMWARE_INTERRUPT : BFGX_POWER_DOWNLOAD_FIRMWARE_FAIL;
    }
    ps_print_info("bfgx download firmware succ!\n");

    return BFGX_POWER_SUCCESS;
}

/*
 * BFGX上电固定使用WIFI HCC加载，尽量与mpxx逻辑一致
 * 并且芯片上有要求，HOST USB/PCIE访问BT/GLE，WIFI必须上电
 * BFGX上电与GT并无强耦合关系，只是PCIE上电需要GT bootloader作PCIE建链初始化；不需要体现在bfgx上电函数
 * 只需要体现在PCIE建链回调函数中
 */
STATIC int32_t gf61_bfgx_dev_power_on(struct ps_core_s *ps_core_d, uint32_t sys, uint32_t subsys)
{
    int32_t ret;
    int32_t error = BFGX_POWER_SUCCESS;
    struct pm_drv_data *pm_data = NULL;

    ps_print_info("%s sys[%d] subsys[%d]\n", __func__, sys, subsys);

    if ((ps_core_d == NULL) || (ps_core_d->pm_data == NULL)) {
        ps_print_err("ps_core_d is err\n");
        return BFGX_POWER_FAILED;
    }

    pm_data = ps_core_d->pm_data;

    bfgx_gpio_intr_enable(pm_data, OAL_TRUE);

    ret = gf61_board_power_on(sys);
    if (ret) {
        ps_print_err("gf61_board_power_on bfg[%d] failed, ret=%d\n", sys, ret);
        error = BFGX_POWER_PULL_POWER_GPIO_FAIL;
        goto board_power_on_fail;
    }

    if (open_tty_drv(ps_core_d) != BFGX_POWER_SUCCESS) {
        ps_print_err("open tty fail!\n");
        error = BFGX_POWER_TTY_OPEN_FAIL;
        goto open_tty_fail;
    }

    error = gf61_bfgx_download(sys);
    if (error != BFGX_POWER_SUCCESS) {
        goto bfgx_download_fail;
    }

    goto bfgx_power_on_succ;

bfgx_download_fail:
    release_tty_drv(ps_core_d);
open_tty_fail:
    (void)gf61_board_power_off(sys);
board_power_on_fail:
    bfgx_gpio_intr_enable(pm_data, OAL_FALSE);

bfgx_power_on_succ:
#ifndef _PRE_HOST_PCIE_ALWAYS_ON
    if (wlan_is_shutdown() == OAL_TRUE) {
        board_sys_disable(W_SYS);
    }
#endif
    return error;
}

STATIC int32_t gf61_bfgx_dev_power_off(struct ps_core_s *ps_core_d, uint32_t sys, uint32_t subsys)
{
    struct pm_drv_data *pm_data = NULL;

    ps_print_info("%s sys[%d] subsys[%d]\n", __func__, sys, subsys);

    if ((ps_core_d == NULL) || (ps_core_d->pm_data == NULL)) {
        ps_print_err("ps_core_d is err\n");
        return BFGX_POWER_FAILED;
    }

    pm_data = ps_core_d->pm_data;
    bfgx_gpio_intr_enable(pm_data, OAL_FALSE);

    if (wait_bfgx_memdump_complete(ps_core_d) != EXCEPTION_SUCCESS) {
        ps_print_err("wait memdump complete failed\n");
    }

    if (release_tty_drv(ps_core_d) != SUCCESS) {
        /* 代码执行到此处，说明六合一所有业务都已经关闭，无论tty是否关闭成功，device都要下电 */
        ps_print_err("wifi off, close tty is err!");
    }

    pm_data->bfgx_dev_state = BFGX_SLEEP;
    pm_data->uart_state = UART_NOT_READY;

    (void)gf61_board_power_off(sys);

    return BFGX_POWER_SUCCESS;
}

STATIC int32_t gf61_wlan_poweron_fail_proc(int32_t error)
{
#if defined(CONFIG_HUAWEI_OHOS_DSM) || defined(CONFIG_HUAWEI_DSM)
    if (error != WIFI_POWER_ON_FIRMWARE_DOWNLOAD_INTERRUPT && error != WIFI_POWER_ON_FIRMWARE_FILE_OPEN_FAIL) {
        hw_mpxx_dsm_client_notify(SYSTEM_TYPE_PLATFORM, DSM_MPXX_DOWNLOAD_FIRMWARE,
                                  "%s: failed to download firmware, bfgx %s, error=%d\n",
                                  __FUNCTION__, bfgx_is_shutdown() ? "off" : "on", error);
    }
#endif
    return error;
}

/* WIFI上电固定使用WIFI HCC加载，尽量与mpxx逻辑一致 */
STATIC int32_t gf61_wlan_power_on(void)
{
    int32_t ret;
    int32_t error;
    struct wlan_pm_s *wlan_pm_info = wlan_pm_get_drv();
    hcc_bus *wifi_bus = NULL;
    if (wlan_pm_info == NULL) {
        ps_print_err("wlan_pm_info is NULL!\n");
        return -FAILURE;
    }

    ret = gf61_board_power_on(W_SYS);
    if (ret) {
        ps_print_err("gf61_board_power_on wlan failed ret=%d\n", ret);
        return -FAILURE;
    }
    wifi_bus = hcc_get_bus(HCC_EP_WIFI_DEV);
    hcc_bus_power_action(wifi_bus, HCC_BUS_POWER_PATCH_LOAD_PREPARE);
    error = firmware_download_function(WIFI_CFG, wifi_bus);
    if (error != WIFI_POWER_SUCCESS) {
        ps_print_err("firmware download fail, error 0x%x\n", error);
        if (error == -OAL_EINTR) {
            error = WIFI_POWER_ON_FIRMWARE_DOWNLOAD_INTERRUPT;
        } else if (error == -OAL_ENOPERMI) {
            error = WIFI_POWER_ON_FIRMWARE_FILE_OPEN_FAIL;
        } else {
            error = WIFI_POWER_BFGX_OFF_FIRMWARE_DOWNLOAD_FAIL;
        }
        return gf61_wlan_poweron_fail_proc(error);
    } else {
        wlan_pm_info->wlan_power_state = POWER_STATE_OPEN;
    }

    ret = hcc_bus_power_action(wifi_bus, HCC_BUS_POWER_PATCH_LAUCH);

    if ((is_gt_support() == OAL_FALSE) && (gt_is_shutdown() == OAL_TRUE) &&
        (g_st_board_info.wlan_download_channel == CHANNEL_PCIE)) {
        power_state_change(GT_POWER, BOARD_POWER_OFF);
    }
    if (ret != 0) {
        declare_dft_trace_key_info("wlan_poweron HCC_BUS_POWER_PATCH_LAUCH by gpio_fail", OAL_DFT_TRACE_FAIL);
        error = WIFI_POWER_BFGX_OFF_BOOT_UP_FAIL;
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_OPEN, CHR_PLAT_DRV_ERROR_WCPU_BOOTUP);
        return gf61_wlan_poweron_fail_proc(error);
    }
    return WIFI_POWER_SUCCESS;
}

STATIC int32_t gf61_wlan_power_off(void)
{
    struct wlan_pm_s *wlan_pm_info = wlan_pm_get_drv();

    /* 先关闭TX通道 */
    hcc_bus_disable_state(hcc_get_bus(HCC_EP_WIFI_DEV), OAL_BUS_STATE_TX);

    /* wakeup dev,send poweroff cmd to wifi */
    if (wlan_pm_poweroff_cmd() != OAL_SUCC) {
        /* wifi self close 失败了也继续往下执行，uart关闭WCPU，异常恢复推迟到wifi下次open的时候执行 */
        declare_dft_trace_key_info("wlan_poweroff_cmd_fail", OAL_DFT_TRACE_FAIL);
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_CLOSE, CHR_PLAT_DRV_ERROR_CLOSE_WCPU);
    }
    hcc_bus_disable_state(hcc_get_bus(HCC_EP_WIFI_DEV), OAL_BUS_STATE_ALL);

    (void)gf61_board_power_off(W_SYS);

    if (wlan_pm_info != NULL) {
        wlan_pm_info->wlan_power_state = POWER_STATE_SHUTDOWN;
    }
    return BOARD_SUCC;
}

/* GT上电固定使用GT HCC加载 */
STATIC int32_t gf61_gt_power_on(void)
{
    int32_t error;
    int32_t ret;
    hcc_bus_dev *pst_bus_dev = hcc_get_bus_dev(HCC_EP_GT_DEV);
    struct gt_pm_s *gt_pm_info = gt_pm_get_drv();
    hcc_bus *gt_bus = NULL;

    if (gt_pm_info == NULL) {
        ps_print_err("gt_pm_info is NULL!\n");
        return -FAILURE;
    }
    ret = gf61_board_power_on(GT_SYS);
    if (ret) {
        ps_print_err("gf61_board_power_on gt failed ret=%d\n", ret);
        return -FAILURE;
    }
    gt_bus = hcc_get_bus(HCC_EP_GT_DEV);
    hcc_bus_power_action(gt_bus, HCC_BUS_POWER_PATCH_LOAD_PREPARE);
    error = firmware_download_function(GT_CFG, gt_bus);
    if (error != GT_POWER_SUCCESS) {
        ps_print_err("firmware download fail, error 0x%x\n", error);
        if (error == -OAL_EINTR) {
            error = GT_POWER_ON_FIRMWARE_DOWNLOAD_INTERRUPT;
        } else if (error == -OAL_ENOPERMI) {
            error = GT_POWER_ON_FIRMWARE_FILE_OPEN_FAIL;
        } else {
            error = GT_POWER_BFGX_OFF_FIRMWARE_DOWNLOAD_FAIL;
        }
        return error;
    } else {
        gt_pm_info->gt_power_state = POWER_STATE_OPEN;
    }

    /* 配置文件中，没有配置gpio唤醒时，使用定时器唤醒。当前evb单板有配置，v0单板无对应管脚配置 */
    if (pst_bus_dev != NULL && !pst_bus_dev->is_wakeup_gpio_support) {
        ps_print_info("gt device not support gpio wakeup!\n");
        rx_sem_timer_init(HCC_EP_GT_DEV);
    } else {
        ps_print_info("gt device support gpio wakeup!\n");
        /* 当前qishan evb单板，配置的gpio管脚为409，与预分配的410管脚不符。后续更新配置后，删除该行代码 */
        rx_sem_timer_init(HCC_EP_GT_DEV);
    }

    ret = hcc_bus_power_action(gt_bus, HCC_BUS_POWER_PATCH_LAUCH);
    if (ret != 0) {
        declare_dft_trace_key_info("slb poweron HCC_BUS_POWER_PATCH_LAUCH fail", OAL_DFT_TRACE_FAIL);
        error = GT_POWER_BFGX_OFF_BOOT_UP_FAIL;
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_OPEN, CHR_PLAT_DRV_ERROR_WCPU_BOOTUP);
        return error;
    }
    return GT_POWER_SUCCESS;
}

STATIC int32_t gf61_gt_power_off(void)
{
    struct gt_pm_s *gt_pm_info = gt_pm_get_drv();
    hcc_bus_dev *pst_bus_dev = hcc_get_bus_dev(HCC_EP_GT_DEV);
    /* 先关闭TX通道 */
    hcc_bus_disable_state(hcc_get_bus(HCC_EP_GT_DEV), OAL_BUS_STATE_TX);

    /* wakeup dev,send poweroff cmd to gt */
    if (gt_pm_poweroff_cmd() != OAL_SUCC) {
        /* gt self close 失败了也继续往下执行，异常恢复推迟到gt下次open的时候执行 */
        declare_dft_trace_key_info("gt_poweroff_cmd_fail", OAL_DFT_TRACE_FAIL);
    }
    if (pst_bus_dev != NULL && !pst_bus_dev->is_wakeup_gpio_support) {
        rx_sem_timer_del(HCC_EP_GT_DEV);
    }
    hcc_bus_disable_state(hcc_get_bus(HCC_EP_GT_DEV), OAL_BUS_STATE_ALL);
    (void)gf61_board_power_off(GT_SYS);
    if (gt_pm_info != NULL) {
        gt_pm_info->gt_power_state = POWER_STATE_SHUTDOWN;
    }
    return BOARD_SUCC;
}

static char *sys_string_tbl[SYS_BUTT] = {"wifi", "bt", "sle", "gt"};

static int32_t gf61_board_check_other_sys_shutdown(uint8_t sys)
{
    switch (sys) {
        case W_SYS:
            return (bfgx_is_shutdown() == OAL_TRUE) && (sle_is_shutdown() == OAL_TRUE) &&
                    (gt_is_shutdown() == OAL_TRUE);
        case B_SYS:
            return (wlan_is_shutdown() == OAL_TRUE) && (sle_is_shutdown() == OAL_TRUE) &&
                   (gt_is_shutdown() == OAL_TRUE);
        case G_SYS:
            return (bfgx_is_shutdown() == OAL_TRUE) && (wlan_is_shutdown() == OAL_TRUE) &&
                   (gt_is_shutdown() == OAL_TRUE);
        case GT_SYS:
            return (bfgx_is_shutdown() == OAL_TRUE) && (sle_is_shutdown() == OAL_TRUE) &&
                   (wlan_is_shutdown() == OAL_TRUE);
        default:
            ps_print_err("check sys shutdown input err\n");
            return OAL_FALSE;
    }
}
static void gf61_board_power_prepare(uint8_t sys)
{
    if (gf61_board_check_other_sys_shutdown(sys) == OAL_TRUE) {
        ps_print_info("%s pull up power_on_enable gpio!\n", sys_string_tbl[sys]);
        board_chip_power_on();
    }

    /*
     * 确保WL_EN先拉高,芯片要求BFGX/GLE加载时，WL须在位
     * wifi_disable 与 bfgx/gle board poweroff中将WL_EN拉低
     */
    power_state_change(W_POWER, BOARD_POWER_ON);
    if (wlan_is_shutdown() == OAL_TRUE) {
        board_sys_enable(W_SYS);
    }
}

static void gf61_board_poweroff_system(uint8_t sys)
{
    /*
     * 在gf61_board_pcie_power_on中有将gt_en拉高，在无GT场景，关闭gt_en
     * 对于有GT的场景，在chip_poweroff或gt_power_off中将gt_en关闭,
     * 因为gt bootloader中，在GT FW没加载的情况
     * 中反复初始化PCIE问题
     */
    if ((is_gt_support() == OAL_FALSE) && (gt_is_shutdown() == OAL_TRUE) &&
        (g_st_board_info.wlan_download_channel == CHANNEL_PCIE)) {
        power_state_change(GT_POWER, BOARD_POWER_OFF);
    }
#ifndef _PRE_HOST_PCIE_ALWAYS_ON
    if (gf61_board_check_other_sys_shutdown(sys) == OAL_TRUE) {
        power_state_change(GT_POWER, BOARD_POWER_OFF);
        board_chip_power_off();
        ps_print_info("%s pull down power_on_enable!\n", sys_string_tbl[sys]);
    }
#endif
}

STATIC int32_t gf61_board_w_power_on(uint8_t sys)
{
    int32_t ret;
    if (gf61_board_check_other_sys_shutdown(sys) == OAL_TRUE) {
        ps_print_info("wifi pull up power_on_enable gpio!\n");
        board_chip_power_on();
    }
    power_state_change(W_POWER, BOARD_POWER_ON);
    ret = board_sys_enable(W_SYS);
    return ret;
}

STATIC int32_t gf61_board_gt_power_on(uint8_t sys)
{
    int32_t ret;
    if (gf61_board_check_other_sys_shutdown(sys) == OAL_TRUE) {
        ps_print_info("gt pull up power_on_enable gpio!\n");
        board_chip_power_on();
    }
#ifdef _PRE_PLAT_FEATURE_GF6X_PCIE
    // PCIE场景，双HCC时，另一条HCC处于LINK状态，Kirin的接口不会再作PCIE建链操作，需要自己上电GT
    if ((g_st_board_info.wlan_download_channel == CHANNEL_PCIE) &&
        (oal_pcie_get_link_state(HCC_EP_WIFI_DEV) >= PCI_WLAN_LINK_UP)) {
        power_state_change(GT_POWER, BOARD_POWER_ON);
    }
#endif
    ret = board_sys_enable(GT_SYS);
    return ret;
}

STATIC int32_t gf61_board_b_power_on(uint8_t sys)
{
    gf61_board_power_prepare(sys);
    board_sys_enable(B_SYS);
    return  SUCC;
}

STATIC int32_t gf61_board_g_power_on(uint8_t sys)
{
    gf61_board_power_prepare(sys);
    board_sys_enable(G_SYS);
    return SUCC;
}

typedef struct board_power_ctrl_s {
    int32_t (*power_on)(uint8_t sys);
    int32_t (*power_off)(uint8_t sys);
} board_power_ctrl;

STATIC int32_t gf61_board_w_power_off(uint8_t sys)
{
    board_sys_disable(W_SYS);
    power_state_change(W_POWER, BOARD_POWER_OFF);
    gf61_board_poweroff_system(sys);
    return SUCC;
}

STATIC int32_t gf61_board_gt_power_off(uint8_t sys)
{
    board_sys_disable(GT_SYS);
#ifndef _PRE_HOST_PCIE_ALWAYS_ON
    power_state_change(GT_POWER, BOARD_POWER_OFF);
    if (gf61_board_check_other_sys_shutdown(sys) == OAL_TRUE) {
        board_chip_power_off();
        ps_print_info("%s pull down power_on_enable!\n", sys_string_tbl[sys]);
    }
#endif
    return SUCC;
}

STATIC int32_t gf61_board_b_power_off(uint8_t sys)
{
    board_sys_disable(B_SYS);

    if (wlan_is_shutdown() == OAL_TRUE) {
        power_state_change(W_POWER, BOARD_POWER_OFF);
    }

    if (sle_is_shutdown() == OAL_TRUE) {
        board_sys_disable(G_SYS);
    }
    gf61_board_poweroff_system(sys);
    return SUCC;
}

STATIC int32_t gf61_board_g_power_off(uint8_t sys)
{
    if (bfgx_is_shutdown() == OAL_TRUE) {
        board_sys_disable(B_SYS);
    }

    if (wlan_is_shutdown() == OAL_TRUE) {
        power_state_change(W_POWER, BOARD_POWER_OFF);
    }
    gf61_board_poweroff_system(sys);
    return SUCC;
}

static board_power_ctrl g_power_ctrl[SYS_BUTT] = {
    { gf61_board_w_power_on, gf61_board_w_power_off },  // wifi
    { gf61_board_b_power_on, gf61_board_b_power_off },  // bt
    { gf61_board_g_power_on, gf61_board_g_power_off },  // sle
    { gf61_board_gt_power_on, gf61_board_gt_power_off }, // gt
};

STATIC int32_t gf61_board_power_on(uint32_t ul_subsystem)
{
    board_power_ctrl *ctrl = g_power_ctrl;
    if (ul_subsystem >= SYS_BUTT) {
        ps_print_err("power input system:%d error\n", ul_subsystem);
        return -EFAIL;
    }

    return ctrl[ul_subsystem].power_on((uint8_t)ul_subsystem);
}

STATIC int32_t gf61_board_power_off(uint32_t ul_subsystem)
{
    board_power_ctrl *ctrl = g_power_ctrl;
    if (ul_subsystem >= SYS_BUTT) {
        ps_print_err("power input system:%d error\n", ul_subsystem);
        return -EFAIL;
    }
    return ctrl[ul_subsystem].power_off((uint8_t)ul_subsystem);
}

STATIC int32_t gf61_board_power_reset(uint32_t ul_subsystem)
{
    int32_t ret;

    board_sys_disable(B_SYS);
    board_sys_disable(G_SYS);
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    board_sys_disable(W_SYS);
#endif
#ifndef _PRE_HOST_PCIE_ALWAYS_ON
    board_chip_power_off();
    board_chip_power_on();
#endif
    ps_print_info("gf61 power reset\n");
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    ret = board_sys_enable(W_SYS);
    if (ret != SUCC) {
        return ret;
    }
#endif
    ret = board_sys_enable(B_SYS);
    if (ret != SUCC) {
        return ret;
    }
    ret = board_sys_enable(G_SYS);
    if (ret != SUCC) {
        return ret;
    }
    return ret;
}

static int32_t gf61_board_gpio_request(int32_t physical_gpio, const char *gpio_name, uint32_t direction)
{
    int32_t ret;
    if (direction == GPIO_DIRECTION_INPUT) {
        ret = gpio_request_one(physical_gpio, GPIOF_IN, gpio_name);
    } else {
        ret = gpio_request_one(physical_gpio, GPIOF_OUT_INIT_HIGH, gpio_name);
    }

    if (ret != BOARD_SUCC) {
        ps_print_err("%s[%d] gpio_request failed\n", gpio_name, physical_gpio);
        return BOARD_FAIL;
    }

    return BOARD_SUCC;
}

static void gf61_chip_poweroff_with_delay(void)
{
    int32_t level = conn_get_gpio_level(CHIP_POWR);
    if (level == 0) {
        /* 已经拉低的情况下，直接返回，防止多余等待 */
        return;
    }
    conn_set_gpio_level(CHIP_POWR, GPIO_LEVEL_LOW);
    /*
     * PMU芯片的PMU-HOLD默认为复位模式，默认需要500ms才能再次操作PMU-HOLD；
     * 即使设置成PMU-HOLD关机模式，也需要500ms延时
     */
    mdelay(500); /* delay 500ms */
}

static void gf61_power_state_change(int32_t type, int32_t flag)
{
    if (flag == BOARD_POWER_ON) {
        conn_set_gpio_level(type, GPIO_LEVEL_HIGH);
        mdelay(20); /* delay 20ms */
    } else if (flag == BOARD_POWER_OFF) {
        if (type == CHIP_POWR) {
            return gf61_chip_poweroff_with_delay();
        } else {
            conn_set_gpio_level(type, GPIO_LEVEL_LOW);
            mdelay(10); /* delay 10ms */
        }
    }
}

/*
 * 只有一路PCIE，2路HCC调用的PCIE建链上下电函数一致
 */
int32_t gf61_board_pcie_power_on(int32_t data)
{
    // memdump 中，调用wifi_device_reset, 此时并不会先拉高W power
    if (data == W_POWER && conn_get_gpio_level(W_POWER) == GPIO_LEVEL_LOW) {
        power_state_change(W_POWER, BOARD_POWER_ON);
    }

    power_state_change(GT_POWER, BOARD_POWER_ON);
    board_host_wakeup_dev_set(0);
    board_host_gt_wakeup_dev_set(0);

    /* 如果总线不是pcie,不进行phy初始化; 此时bus_type未初始化,直接从ini读取 */
    if (g_st_board_info.wlan_download_channel == CHANNEL_PCIE) {
        if (conn_gpio_valid(PCIE0_RST) == OAL_TRUE) {
            /* 拉高pcie_rst */
            conn_set_gpio_level(PCIE0_RST, GPIO_LEVEL_LOW);
            mdelay(1);
            conn_set_gpio_level(PCIE0_RST, GPIO_LEVEL_HIGH);
            mdelay(10); /* delay 10ms */
            ps_print_info("pcie0_rst %d pull up\n", conn_get_gpio_number_by_type(PCIE0_RST));
        }
    }

    board_host_wakeup_dev_set(1);
    board_host_gt_wakeup_dev_set(1);
    return 0;
}

int32_t gf61_board_pcie_power_off(int32_t data)
{
    (void)data;
    if (conn_gpio_valid(PCIE0_RST) == OAL_TRUE) {
        /* 拉低pcie_rst */
        conn_set_gpio_level(PCIE0_RST, GPIO_LEVEL_LOW);
        mdelay(1);
        ps_print_info("pcie0_rst %d pull down\n", conn_get_gpio_number_by_type(PCIE0_RST));
    }
#ifndef _PRE_HOST_PCIE_ALWAYS_ON
    if (gt_is_shutdown() == OAL_TRUE) {
        power_state_change(GT_POWER, BOARD_POWER_OFF);
    }
#endif
    return 0;
}

STATIC void board_callback_init_gf61_power(void)
{
    g_st_board_info.bd_ops.bfgx_dev_power_on = gf61_bfgx_dev_power_on;
    g_st_board_info.bd_ops.bfgx_dev_power_off = gf61_bfgx_dev_power_off;
    g_st_board_info.bd_ops.wlan_power_on = gf61_wlan_power_on;
    g_st_board_info.bd_ops.wlan_power_off = gf61_wlan_power_off;
    g_st_board_info.bd_ops.gt_power_on = gf61_gt_power_on;
    g_st_board_info.bd_ops.gt_power_off = gf61_gt_power_off;
    g_st_board_info.bd_ops.board_power_on = gf61_board_power_on;
    g_st_board_info.bd_ops.board_power_off = gf61_board_power_off;
    g_st_board_info.bd_ops.board_power_reset = gf61_board_power_reset;
    g_st_board_info.bd_ops.power_state_change = gf61_power_state_change;
    g_st_board_info.bd_ops.board_gpio_request = gf61_board_gpio_request;
    g_st_board_info.is_asic = VERSION_ASIC;
    board_register_pcie_poweron(gf61_board_pcie_power_on);
    board_register_pcie_poweroff(gf61_board_pcie_power_off);
#if defined(_PRE_PRODUCT_SYLINCOM) || defined(_PRE_PRODUCT_SLCONTEST)
    g_st_board_info.is_gt_disable = 0;
#endif
}

void plat_cust_init(void)
{
    int32_t l_cfg_value;
    int32_t l_ret;
    plat_cust_data_t plat_cust;
    /* 获取ini的配置值 */
    l_ret = get_cust_conf_int32(INI_MODU_WIFI, "powermgmt_switch", &l_cfg_value);
    if (l_ret == INI_FAILED) {
        ps_print_err("get_ssi_dump_cfg: fail to get ini, keep disable\n");
        return;
    }
    plat_cust.lowpower_cfg_value = l_cfg_value;
    plat_cust.clk_mode_value = board_clk_mode();
    l_ret = set_plat_cust_buf((uint8_t *)&plat_cust, sizeof(plat_cust_data_t));
    if (l_ret != SUCC) {
        ps_print_err("set plat cust buf fail, ret = %d\n", l_ret);
        return;
    }
    ps_print_info("set plat cust buf succ\n");
}

// 兼容历史代码，gf61不编译SDIO相关的HOST代码
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_GF61_HOST)
struct oal_sdio *g_mpxx_sdio;
void oal_wifi_platform_unload_sdio(void)
{
    return;
}

int32_t oal_sdio_mpxx_working_check(void)
{
    return OAL_FALSE;
}

uint32_t oal_sdio_get_large_pkt_free_cnt(struct oal_sdio *sdio)
{
    return 0;
}
#endif

void board_info_init_gf61(void)
{
    board_callback_init_dts();
    board_callback_init_gf61_power();
    plat_cust_init();
}

static int32_t ssi_read_value32_board(uint32_t addr, uint32_t *value)
{
#ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG
    /* Shifts rightwards by 16 bits and 0xFFFF. */
    return ssi_read_value32(addr, value, (addr >> 16) & 0xffff);
#else
    return -OAL_FAIL;
#endif
}

static int32_t ssi_write_value32_board(uint32_t addr, uint32_t value)
{
#ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG
    /* Shifts rightwards by 16 bits and 0xFFFF. */
    return ssi_write_value32(addr, value);
#else
    return -OAL_FAIL;
#endif
}

/* factory test, wifi power on, do some test under bootloader mode */
STATIC void gf61_dump_gpio_regs(void)
{
    uint32_t value = 0;
    int32_t ret;
    ret = ssi_read_value32_board(GT_GPIO_BASE_ADDR + GPIO_INOUT_CONFIG_REGADDR, &value);
    if (ret) {
        return;
    }

    oal_print_mpxx_log(MPXX_LOG_INFO, "gpio reg 0x%x = 0x%x", GT_GPIO_BASE_ADDR + GPIO_INOUT_CONFIG_REGADDR, value);

    value = 0;
    ret = ssi_read_value32_board(GT_GPIO_BASE_ADDR + GPIO_LEVEL_GET_REGADDR, &value);
    if (ret) {
        return;
    }

    oal_print_mpxx_log(MPXX_LOG_INFO, "gpio reg 0x%x = 0x%x", GT_GPIO_BASE_ADDR + GPIO_LEVEL_GET_REGADDR, value);
}

STATIC int32_t gf61_ssi_set_gpio_direction(uint32_t address, uint32_t gpio_bit, uint32_t direction)
{
    uint32_t value = 0;
    int32_t ret;

    ret = ssi_read_value32_board(address, &value);
    if (ret) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "read 0x%x reg failed, ret=%d", address, ret);
        return -OAL_EFAIL;
    }

    if (direction == GPIO_DIRECTION_OUTPUT) {
        value |= gpio_bit;
    } else {
        value &= (uint16_t)(~gpio_bit);
    }

    ret = ssi_write_value32_board(address, value);
    if (ret != BOARD_SUCC) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "write 0x%x reg failed,value=0x%x, ret=%d", address, value, ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

STATIC int32_t gf61_ssi_set_gpio_value(int32_t gpio, int32_t gpio_lvl, uint32_t address, uint32_t mask)
{
    int32_t ret;
    uint32_t value = 0;
    ret = ssi_read_value32_board(address, &value);
    if (ret) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "read 0x%x reg failed, ret=%d", address, ret);
        return -OAL_EFAIL;
    }
    if (gpio_lvl == GPIO_HIGHLEVEL) {
        value |= mask;
    } else {
        value &= (~mask);
    }
    ret = ssi_write_value32_board(address, value);
    if (ret) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "write 0x%x reg failed,value=0x%x, ret=%d", address, value, ret);
        return -OAL_EFAIL;
    }

    oal_msleep(1);

    if (oal_gpio_get_value(gpio) != gpio_lvl) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "pull gpio to %d failed!", gpio_lvl);
        gf61_dump_gpio_regs();
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

STATIC int32_t gf61_ssi_check_gpio_value(int32_t gpio, int32_t gpio_lvl, uint32_t address, uint32_t check_bit)
{
    uint32_t value = 0;
    int32_t ret;

    gpio_direction_output(gpio, gpio_lvl);
    oal_msleep(1);

    ret = ssi_read_value32_board(address, &value);
    if (ret) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "read 0x%x reg failed, ret=%d", address, ret);
        return -OAL_EFAIL;
    }

    oal_print_mpxx_log(MPXX_LOG_DBG, "read 0x%x reg=0x%x", address, value);

    value &= check_bit;

    if (((gpio_lvl == GPIO_HIGHLEVEL) && (value == 0)) || ((gpio_lvl == GPIO_LOWLEVEL) && (value != 0))) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "pull gpio to %d failed!", gpio_lvl);
        gf61_dump_gpio_regs();
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

STATIC int32_t gf61_check_dev_wakeup_host(uint8_t sys, int32_t gpio_nr)
{
    int32_t i;
    int32_t ret;
    uint32_t ssi_address;
    uint32_t gpio_base;
    uint32_t gpio_mask;

    gpio_base = gf61_gpio_cfg_table[sys].reg_base;
    gpio_mask = (1 << gf61_gpio_cfg_table[sys].d2h_io);

    ssi_address = gpio_base + GPIO_INOUT_CONFIG_REGADDR;
    ret = gf61_ssi_set_gpio_direction(ssi_address, gpio_mask, GPIO_DIRECTION_OUTPUT);
    if (ret < 0) {
        return ret;
    }

    ssi_address = gpio_base + GPIO_LEVEL_CONFIG_REGADDR;
    for (i = 0; i < GPIO_TEST_TIMES; i++) {
        ret = gf61_ssi_set_gpio_value(gpio_nr, GPIO_HIGHLEVEL, ssi_address, gpio_mask);
        if (ret < 0) {
            return ret;
        }

        ret = gf61_ssi_set_gpio_value(gpio_nr, GPIO_LOWLEVEL, ssi_address, gpio_mask);
        if (ret < 0) {
            return ret;
        }
    }

    oal_print_mpxx_log(MPXX_LOG_INFO, "check sys %d d2h wakeup io succ", sys);
    return OAL_SUCC;
}

STATIC int32_t gf61_check_host_wakeup_dev(uint8_t sys, int32_t gpio_nr)
{
    int32_t i;
    int32_t ret;
    uint32_t ssi_address;
    uint32_t gpio_base;
    uint32_t gpio_mask;

    gpio_base = gf61_gpio_cfg_table[sys].reg_base;
    gpio_mask = (1 << gf61_gpio_cfg_table[sys].h2d_io);

    ssi_address = gpio_base + GPIO_INOUT_CONFIG_REGADDR;
    ret = gf61_ssi_set_gpio_direction(ssi_address, gpio_mask, GPIO_DIRECTION_INPUT);
    if (ret < 0) {
        return ret;
    }

    ssi_address = gpio_base + GPIO_LEVEL_GET_REGADDR;
    for (i = 0; i < GPIO_TEST_TIMES; i++) {
        ret = gf61_ssi_check_gpio_value(gpio_nr, GPIO_HIGHLEVEL, ssi_address, gpio_mask);
        if (ret < 0) {
            return ret;
        }

        ret = gf61_ssi_check_gpio_value(gpio_nr, GPIO_LOWLEVEL, ssi_address, gpio_mask);
        if (ret < 0) {
            return ret;
        }
    }

    oal_print_mpxx_log(MPXX_LOG_INFO, "check sys %d h2d wakeup io succ", sys);
    return OAL_SUCC;
}

STATIC int32_t gf61_test_device_io_sys(uint8_t sys, uint32_t wkup_dev, uint32_t wkup_host)
{
    int32_t ret = OAL_SUCC;
    int32_t gpio_nr;

    if (conn_gpio_valid(wkup_dev) == OAL_TRUE) {
        gpio_nr = conn_get_gpio_number_by_type(wkup_dev);
        ret = gf61_check_host_wakeup_dev(sys, gpio_nr);
        if (ret) {
            oal_print_mpxx_log(MPXX_LOG_ERR, "sys %d check_host_wakeup failed, ret=%d", sys, ret);
        }
        oal_gpio_free(gpio_nr);
    }

    if (conn_gpio_valid(wkup_host) == OAL_TRUE) {
        gpio_nr = conn_get_gpio_number_by_type(wkup_host);
        ret = gf61_check_dev_wakeup_host(sys, gpio_nr);
        if (ret) {
            oal_print_mpxx_log(MPXX_LOG_ERR, "sys %d check_wakeup_host failed, ret=%d", sys, ret);
        }
        oal_gpio_free(gpio_nr);
    }

    return ret;
}

STATIC int32_t gf61_test_device_io(void)
{
    int32_t ret;
    uint32_t ret_err = 0;

    ret = gf61_test_device_io_sys(GT_SYS, HOST_WKUP_GT, GT_WKUP_HOST);
    if (ret) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "check_gt_gpio failed, ret=%d", ret);
        ret_err = (ret_err << 1) + 1;
    }

    ret = gf61_test_device_io_sys(W_SYS, HOST_WKUP_W, W_WKUP_HOST);
    if (ret) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "check_wlan_gpio failed, ret=%d", ret);
        ret_err = (ret_err << 1) + 1;
    }

    ret = gf61_test_device_io_sys(B_SYS, HOST_WKUP_BFGX, BFGX_WKUP_HOST);
    if (ret) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "check_bt_gpio failed, ret=%d", ret);
        ret_err = (ret_err << 1) + 1;
    }

    ret = gf61_test_device_io_sys(G_SYS, HOST_WKUP_SLE, SLE_WKUP_HOST);
    if (ret) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "check_sle_gpio failed, ret=%d", ret);
        ret_err = (ret_err << 1) + 1;
    }

    if (ret_err == 0) {
        return OAL_SUCC;
    } else {
        oal_print_mpxx_log(MPXX_LOG_ERR, "check_sle_gpio failed, ret_err=0x%x", ret_err);
        return -OAL_FAIL;
    }
}

int32_t gf61_dev_io_test(void)
{
    int32_t ret;
    declare_time_cost_stru(cost);
    struct pm_top* pm_top_data = pm_get_top();

    if (oal_mutex_trylock(&pm_top_data->host_mutex) == 0) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "get lock fial!");
        return -OAL_EINVAL;
    }

    if (!bfgx_is_shutdown()) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "bfgx is open, test abort!");
        bfgx_print_subsys_state(BUART);
        oal_mutex_unlock(&pm_top_data->host_mutex);
        return -OAL_ENODEV;
    }

    if (!wlan_is_shutdown()) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "wlan is open, test abort!");
        oal_mutex_unlock(&pm_top_data->host_mutex);
        return -OAL_ENODEV;
    }

    oal_get_time_cost_start(cost);

    ret = gf61_test_device_io();
    if (ret) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "dev io test fail!");
    }

    oal_get_time_cost_end(cost);
    oal_calc_time_cost_sub(cost);
    oal_print_mpxx_log(MPXX_LOG_INFO, "gf61 device io test cost %llu us", time_cost_var_sub(cost));

    oal_mutex_unlock(&pm_top_data->host_mutex);
    return ret;
}
EXPORT_SYMBOL(gf61_dev_io_test);

#ifdef _PRE_PRODUCT_SYLINCOM
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
#endif
