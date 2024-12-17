/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: Declaration Driver Entry Function
 * Author: @CompanyNameTag
 */

/* 头文件包含 */
#define MPXX_LOG_MODULE_NAME     "[MP13_BOARD]"
#define MPXX_LOG_MODULE_NAME_VAR mp13_board_loglevel
#include "board_mp13.h"

#include "chr_user.h"
#include "plat_debug.h"
#include "bfgx_dev.h"
#include "plat_pm.h"
#include "plat_uart.h"
#include "plat_cust.h"
#include "plat_firmware.h"
#include "plat_firmware_download.h"
#include "oam_dsm.h"
#include "board_gpio.h"

#ifdef BFGX_UART_DOWNLOAD_SUPPORT
#include "bfgx_data_parse.h"
#include "plat_firmware_uart.h"
#endif

#ifdef _PRE_SHARE_BUCK_SURPORT
#ifndef CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT
#include <huawei_platform/sensor/hw_comm_pmic.h>
#endif
#endif

#define BUCK_MODE_OFFSET                        14
#define BUCK_MODE_MASK                          0xC000   // bit[15:14]
#define EXT_BUCK_OFF                             0        // 无外置buck,全内置
#define EXT_BUCK_I2C_CTL                         1        // I2C控制独立外置buck
#define EXT_BUCK_GPIO_CTL                        2        // GPIO控制独立外置buck
#define EXT_BUCK_HOST_CTL                        3        // host控制共享外置buck

#define BUCK_NUMBER_OFFSET                      0
#define BUCK_NUMBER_MASK                        0xFF     // bit[7:0]
#define BUCK_NUMBER_AE                          0x0      // ANNA ELSA上用的外置buck
#define BUCK_NUMBER_ONNT                        0x2      // ONNT 上用的外置buck
#define BUCK_NUMBER_ADA                         0x3      // ADA 上控制外置BUCK

#define POWER_DOWN                              0
#define POWER_UP                                1

/* 全局变量定义 */
#ifdef PLATFORM_DEBUG_ENABLE
int32_t g_device_monitor_enable = 0;
#endif

/* 必须和芯片定义的PLAT_CUST_LEN长度相同 */
#define PLAT_CUST_LEN       0x10

typedef union {
    struct {
        uint8_t vmin_mode;
    } cfg;
    uint8_t data[PLAT_CUST_LEN];
} plat_cust_data_mp13;

/*
 * 设置Vmin模式，下次加载firmware时随plat_cust数据下发
 * factory模式下禁止修改vmin_mode
 */
STATIC void board_vmin_mode_set_mp13(uint32_t vmin_mode, uint32_t errid)
{
    plat_cust_data_mp13 *plat_cust = NULL;

    plat_cust = (plat_cust_data_mp13 *)get_plat_cust_addr();
    if (plat_cust == NULL) {
        ps_print_err("plat_cust buf not init, please check\n");
        return;
    }

    if (plat_cust->cfg.vmin_mode == VMIN_MODE_FACTORY) {
        ps_print_err("current vmin_mode is factory, not allow reset\n");
        return;
    }

    if (vmin_mode >= VMIN_MODE_BUTT) {
        plat_cust->cfg.vmin_mode = VMIN_MODE_NORMAL;
    } else {
        plat_cust->cfg.vmin_mode = vmin_mode;
    }
    ps_print_info("request vmin_mode = %u, set = %u\n", vmin_mode, plat_cust->cfg.vmin_mode);
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_PM, errid);
}

STATIC void mp13_board_error_handler(uint32_t errid)
{
    switch (errid) {
        case CHR_PLAT_DRV_ERROR_VMIN_WLAN_POWER_FAIL:
        case CHR_PLAT_DRV_ERROR_VMIN_BFGX_POWER_FAIL:
            board_vmin_mode_set_mp13(VMIN_MODE_EMERGENCY, errid);
            break;
        default:
            ps_print_info("no such vmin err type\n");
    }
}

/*
*  函 数 名  : buck_number_get
*  功能描述  : 获取不同产品的buck编号
*/
STATIC uint8_t buck_number_get(void)
{
    return  (((g_st_board_info.buck_param) & BUCK_NUMBER_MASK) >> BUCK_NUMBER_OFFSET);
}

/* 外置PMU buck配置上下电接口, 目前仅ada产品上使用, 电压使用默认值0.6v */
STATIC void ext_pmu_buck_power_ctrl(uint32_t power_state)
{
#define BUCK_POWER_VOL    600000
#define BUCK_CTRL_DELAY   50
    static int32_t power_on_cnt = 0;
    int32_t ret;
    if (buck_number_get() != BUCK_NUMBER_ADA) {
        ps_print_info("ext_pmu_buck_power_ctrl: no need to control external buck\n");
        return;
    }
    ret = regulator_set_voltage(g_st_board_info.buck_regulator, BUCK_POWER_VOL, BUCK_POWER_VOL);
    if (ret) {
        regulator_put(g_st_board_info.buck_regulator);
        ps_print_err("ext_pmu_buck_power_ctrl: fail to set regulator voltage\n");
        return;
    }

    /* 上下电时序约束：
    * 上电：MP15的BUCK使能->延时50ms->PMU BUCK使能
    * 下电：PMU的BUCK关闭->延时50ms->MP15的BUCK关闭
    */
    if (power_state) {
        power_on_cnt++;
        msleep(BUCK_CTRL_DELAY);
        ret = regulator_enable(g_st_board_info.buck_regulator);
    } else {
        power_on_cnt--;
        ret = regulator_disable(g_st_board_info.buck_regulator);
        msleep(BUCK_CTRL_DELAY);
    }
    ps_print_info("ext_pmu_buck_power_ctrl: power_state is %d, power_on_cnt is %d, result %d\n",
                  power_state, power_on_cnt, ret);
}

STATIC int32_t mp13_board_power_on(uint32_t ul_subsystem)
{
    int32_t ret = SUCC;

    if (ul_subsystem == W_SYS) {
        if (bfgx_is_shutdown()) {
            ps_print_info("wifi pull up power_on_enable gpio!\n");
            board_chip_power_on();
            if (board_sys_enable(W_SYS)) {
                ret = WIFI_POWER_BFGX_OFF_PULL_WLEN_FAIL;
            }
            ext_pmu_buck_power_ctrl(POWER_UP);
            if (mpxx_firmware_download_mode() == MODE_COMBO) {
                board_sys_enable(B_SYS);
            }
        } else {
            if (board_sys_enable(W_SYS)) {
                ret = WIFI_POWER_BFGX_ON_PULL_WLEN_FAIL;
            }
        }
    } else if (ul_subsystem == B_SYS) {
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
        if (wlan_is_shutdown()) {
            ps_print_info("bfgx pull up power_on_enable gpio!\n");
            board_chip_power_on();
            ret = board_sys_enable(W_SYS);
            board_sys_enable(B_SYS);
            ext_pmu_buck_power_ctrl(POWER_UP);
        } else {
            board_sys_enable(B_SYS);
        }
#else
        board_chip_power_on();
        board_sys_enable(B_SYS);
        ext_pmu_buck_power_ctrl(POWER_UP);
#endif
    } else {
        ps_print_err("power input system:%d error\n", ul_subsystem);
        ret = WIFI_POWER_FAIL;
    }

    return ret;
}

STATIC int32_t mp13_board_power_off(uint32_t ul_subsystem)
{
    if (ul_subsystem == W_SYS) {
        if (bfgx_is_shutdown()) {
            ps_print_info("wifi pull down power_on_enable!\n");
            ext_pmu_buck_power_ctrl(POWER_DOWN);
            board_sys_disable(W_SYS);
            board_sys_disable(B_SYS);
            board_chip_power_off();
        } else {
            board_sys_disable(W_SYS);
        }
    } else if (ul_subsystem == B_SYS) {
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
        if (wlan_is_shutdown()) {
            ps_print_info("bfgx pull down power_on_enable!\n");
            ext_pmu_buck_power_ctrl(POWER_DOWN);
            board_sys_disable(B_SYS);
            board_sys_disable(W_SYS);
            board_chip_power_off();
        } else {
            board_sys_disable(B_SYS);
        }
#else
        ext_pmu_buck_power_ctrl(POWER_DOWN);
        board_sys_disable(B_SYS);
        board_chip_power_off();
#endif
    } else {
        ps_print_err("power input system:%d error\n", ul_subsystem);
        return -EFAIL;
    }

    return SUCC;
}

STATIC int32_t mp13_board_power_reset(uint32_t ul_subsystem)
{
    int32_t ret = 0;
    ext_pmu_buck_power_ctrl(POWER_DOWN);
    board_sys_disable(B_SYS);
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    board_sys_disable(W_SYS);
#endif
    board_chip_power_off();
    board_chip_power_on();
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    ret = board_sys_enable(W_SYS);
#endif
    board_sys_enable(B_SYS);
    ext_pmu_buck_power_ctrl(POWER_UP);
    return ret;
}

int32_t mp13_bfgx_subsys_reset(excp_comm_t *node_ptr)
{
    // 维测, 判断之前的gpio状态
    ps_print_info("bfgx wkup host gpio val %d\n", conn_get_gpio_level(BFGX_WKUP_HOST));

    oal_reference(node_ptr);
    if (wlan_is_shutdown() == true) {
        ext_pmu_buck_power_ctrl(POWER_DOWN);
    }
    board_sys_disable(B_SYS);
    mdelay(BFGX_SUBSYS_RST_DELAY);
    board_sys_enable(B_SYS);
    if (wlan_is_shutdown() == true) {
        ext_pmu_buck_power_ctrl(POWER_UP);
    }
    return OAL_SUCC;
}

int32_t mp13_wifi_subsys_reset(excp_comm_t *node_ptr)
{
    int32_t ret;

    oal_reference(node_ptr);
    if (bfgx_is_shutdown() == true) {
        ext_pmu_buck_power_ctrl(POWER_DOWN);
    }
    board_sys_disable(W_SYS);
    mdelay(WIFI_SUBSYS_RST_DELAY);
    ret = board_sys_enable(W_SYS);
    if (bfgx_is_shutdown() == true) {
        ext_pmu_buck_power_ctrl(POWER_UP);
    }
    return ret;
}

#ifndef BFGX_UART_DOWNLOAD_SUPPORT
STATIC int32_t mp13_bfgx_download_by_wifi(void)
{
    int32_t ret;
    int32_t error = BFGX_POWER_SUCCESS;

    if (wlan_is_shutdown() || (mpxx_firmware_download_mode() == MODE_SEPARATE)) {
        ret = firmware_download_function(BFGX_CFG, hcc_get_bus(HCC_EP_WIFI_DEV));
        if (ret != BFGX_POWER_SUCCESS) {
            ps_print_err("bfgx download firmware fail!\n");
            error = (ret == -OAL_EINTR) ? BFGX_POWER_DOWNLOAD_FIRMWARE_INTERRUPT : BFGX_POWER_DOWNLOAD_FIRMWARE_FAIL;
            return error;
        }
        ps_print_info("bfgx download firmware succ!\n");
    } else {
        /* 此时BFGX 需要解复位BCPU */
        ps_print_info("wifi dereset bcpu\n");
        if (wlan_pm_open_bcpu() != BFGX_POWER_SUCCESS) {
            ps_print_err("wifi dereset bcpu fail!\n");
            error = BFGX_POWER_WIFI_DERESET_BCPU_FAIL;
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_OPEN, CHR_PLAT_DRV_ERROR_BFGX_PWRON_BY_WIFI);
            return error;
        }
    }
    return error;
}
#else
STATIC int32_t mp13_bfgx_download_by_uart(struct ps_core_s *ps_core_d)
{
    int32_t error;

    st_tty_recv = ps_recv_patch;
    error = ps_core_d->pm_data->download_patch(ps_core_d);
    if (error) { /* if download patch err,and close uart */
        ps_print_err(" download_patch is failed!\n");
        return error;
    }

    ps_print_suc("download_patch is successfully!\n");
    return error;
}
#endif

STATIC void judge_ir_only_mode(struct ps_core_s *ps_core_d)
{
    if (get_mpxx_subchip_type() == BOARD_VERSION_MP16) {
        return;
    }
    /* wifi & bfgx both shutdown */
    if ((wlan_is_shutdown() == true) && (bfgx_is_shutdown() == true)) {
        /* chip type is asic */
        if (mpxx_is_asic()) {
            oal_atomic_set(&ps_core_d->ir_only, 1);
        }
    }
}

STATIC int32_t mp13_bfgx_firmware_download(struct ps_core_s *ps_core_d)
{
    int32_t error;

#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    error = mp13_bfgx_download_by_wifi();
    if (wlan_is_shutdown()) {
        struct pm_top* pm_top_data = pm_get_top();
        hcc_bus_disable_state(pm_top_data->wlan_pm_info->pst_bus, OAL_BUS_STATE_ALL);
 /* eng support monitor */
#ifdef PLATFORM_DEBUG_ENABLE
        if (!g_device_monitor_enable) {
#endif
            board_sys_disable(W_SYS);
#ifdef PLATFORM_DEBUG_ENABLE
        }
#endif
    }
#else
    error = mp13_bfgx_download_by_uart(ps_core_d);
#endif

    return error;
}

STATIC int32_t mp13_bfgx_dev_power_on(struct ps_core_s *ps_core_d, uint32_t sys, uint32_t subsys)
{
    int32_t ret;
    int32_t error = BFGX_POWER_SUCCESS;
    struct pm_drv_data *pm_data = NULL;

    if ((ps_core_d == NULL) || (ps_core_d->pm_data == NULL)) {
        ps_print_err("ps_core_d is err\n");
        return BFGX_POWER_FAILED;
    }

    /* judge ir only mode */
    if (subsys == BFGX_IR) {
        judge_ir_only_mode(ps_core_d);
    }

    pm_data = ps_core_d->pm_data;
    bfgx_gpio_intr_enable(pm_data, OAL_TRUE);

    ret = mp13_board_power_on(sys);
    if (ret) {
        ps_print_err("mp13_board_power_on bfg failed, ret=%d\n", ret);
        error = BFGX_POWER_PULL_POWER_GPIO_FAIL;
        goto bfgx_power_on_fail;
    }

    if (open_tty_drv(ps_core_d) != BFGX_POWER_SUCCESS) {
        ps_print_err("open tty fail!\n");
        error = BFGX_POWER_TTY_OPEN_FAIL;
        goto bfgx_power_on_fail;
    }

    error = mp13_bfgx_firmware_download(ps_core_d);
    if (error != BFGX_POWER_SUCCESS) {
        release_tty_drv(ps_core_d);
        goto bfgx_power_on_fail;
    }

    return BFGX_POWER_SUCCESS;

bfgx_power_on_fail:
#if defined(CONFIG_HUAWEI_OHOS_DSM) || defined(CONFIG_HUAWEI_DSM)
    if (error != BFGX_POWER_DOWNLOAD_FIRMWARE_INTERRUPT) {
        hw_mpxx_dsm_client_notify(SYSTEM_TYPE_PLATFORM, DSM_MPXX_DOWNLOAD_FIRMWARE,
                                  "bcpu download firmware failed,wifi %s,ret=%d,process:%s\n",
                                  wlan_is_shutdown() ? "off" : "on", error, current->comm);
    }
#endif
    (void)mp13_board_power_off(B_SYS);
    return error;
}

STATIC int32_t mp13_bfgx_dev_power_off(struct ps_core_s *ps_core_d, uint32_t sys, uint32_t subsys)
{
    int32_t error = BFGX_POWER_SUCCESS;
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

    if (!wlan_is_shutdown()) {
        ps_print_info("wifi shutdown bcpu\n");
        if (wlan_pm_shutdown_bcpu_cmd() != SUCCESS) {
            ps_print_err("wifi shutdown bcpu fail\n");
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_CLOSE, CHR_PLAT_DRV_ERROR_WIFI_CLOSE_BCPU);
            error = BFGX_POWER_FAILED;
        }
    }

    pm_data->bfgx_dev_state = BFGX_SLEEP;
    pm_data->uart_state = UART_NOT_READY;

    (void)mp13_board_power_off(sys);

    if (subsys == BFGX_IR) {
        oal_atomic_set(&ps_core_d->ir_only, 0);
    }

    return error;
}

STATIC int32_t mp13_wlan_power_off(void)
{
    struct wlan_pm_s *wlan_pm_info = wlan_pm_get_drv();

    /* 先关闭SDIO TX通道 */
    hcc_bus_disable_state(hcc_get_bus(HCC_EP_WIFI_DEV), OAL_BUS_STATE_TX);

    /* wakeup dev,send poweroff cmd to wifi */
    if (wlan_pm_poweroff_cmd() != OAL_SUCC) {
        /* wifi self close 失败了也继续往下执行，uart关闭WCPU，异常恢复推迟到wifi下次open的时候执行 */
        declare_dft_trace_key_info("wlan_poweroff_cmd_fail", OAL_DFT_TRACE_FAIL);
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_CLOSE, CHR_PLAT_DRV_ERROR_CLOSE_WCPU);
    }
    hcc_bus_disable_state(hcc_get_bus(HCC_EP_WIFI_DEV), OAL_BUS_STATE_ALL);

    (void)mp13_board_power_off(W_SYS);

    if (wlan_pm_info != NULL) {
        wlan_pm_info->wlan_power_state = POWER_STATE_SHUTDOWN;
    }
    return SUCCESS;
}

STATIC int32_t mp13_wlan_firmware_download(void)
{
    int32_t ret;
    int32_t error;
    struct wlan_pm_s *wlan_pm_info = wlan_pm_get_drv();

    if (wlan_pm_info == NULL) {
        ps_print_err("wlan_pm_info is NULL!\n");
        return -FAILURE;
    }

    hcc_bus_power_action(hcc_get_bus(HCC_EP_WIFI_DEV), HCC_BUS_POWER_PATCH_LOAD_PREPARE);
    /* 混合加载模式下，才一起加载bfg */
    if (bfgx_is_shutdown() && (mpxx_firmware_download_mode() == MODE_COMBO)) {
        error = firmware_download_function(BFGX_AND_WIFI_CFG, hcc_get_bus(HCC_EP_WIFI_DEV));
        board_sys_disable(B_SYS);
    } else {
        error = firmware_download_function(WIFI_CFG, hcc_get_bus(HCC_EP_WIFI_DEV));
    }

    if (error != WIFI_POWER_SUCCESS) {
        ps_print_err("firmware download fail\n");
        if (error == -OAL_EINTR) {
            error = WIFI_POWER_ON_FIRMWARE_DOWNLOAD_INTERRUPT;
        } else if (error == -OAL_ENOPERMI) {
            error = WIFI_POWER_ON_FIRMWARE_FILE_OPEN_FAIL;
        } else {
            error = WIFI_POWER_BFGX_OFF_FIRMWARE_DOWNLOAD_FAIL;
        }
        return error;
    } else {
        wlan_pm_info->wlan_power_state = POWER_STATE_OPEN;
    }

    ret = hcc_bus_power_action(hcc_get_bus(HCC_EP_WIFI_DEV), HCC_BUS_POWER_PATCH_LAUCH);
    if (ret != 0) {
        declare_dft_trace_key_info("wlan_poweron HCC_BUS_POWER_PATCH_LAUCH by gpio_fail", OAL_DFT_TRACE_FAIL);
        ps_print_err("wlan_poweron HCC_BUS_POWER_PATCH_LAUCH by gpio fail ret=%d", ret);
        mp13_board_error_handler(CHR_PLAT_DRV_ERROR_VMIN_WLAN_POWER_FAIL);
        error = WIFI_POWER_BFGX_OFF_BOOT_UP_FAIL;
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_OPEN, CHR_PLAT_DRV_ERROR_WCPU_BOOTUP);
        return error;
    }

    return WIFI_POWER_SUCCESS;
}

STATIC int32_t mp13_wlan_power_on(void)
{
    int32_t ret;
    int32_t error = WIFI_POWER_SUCCESS;

    ret = mp13_board_power_on(W_SYS);
    if (ret) {
        ps_print_err("mp13_board_power_on wlan failed ret=%d\n", ret);
        return -FAILURE;
    }

    error = mp13_wlan_firmware_download();
#if defined(CONFIG_HUAWEI_OHOS_DSM) || defined(CONFIG_HUAWEI_DSM)
    if ((error != WIFI_POWER_SUCCESS) && (error != WIFI_POWER_ON_FIRMWARE_DOWNLOAD_INTERRUPT)
        && (error != WIFI_POWER_ON_FIRMWARE_FILE_OPEN_FAIL)) {
        hw_mpxx_dsm_client_notify(SYSTEM_TYPE_PLATFORM, DSM_MPXX_DOWNLOAD_FIRMWARE,
                                  "%s: failed to download firmware, bfgx %s, error=%d\n",
                                  __FUNCTION__, bfgx_is_shutdown() ? "off" : "on", error);
    }
#endif
    return error;
}


#if defined(_PRE_S4_FEATURE)
STATIC void mp13_suspend_power_gpio(void)
{
    int physical_gpio;

    physical_gpio = conn_get_gpio_number_by_type(CHIP_POWR);
    if (physical_gpio <= 0) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "power_on_enable suspend fail.\n");
        return;
    }
    oal_gpio_free(physical_gpio);

    physical_gpio = conn_get_gpio_number_by_type(BFGX_POWER);
    if (physical_gpio <= 0) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "bfgn_power_on_enable suspend fail.\n");
        return;
    }
    oal_gpio_free(physical_gpio);

#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    physical_gpio = conn_get_gpio_number_by_type(W_POWER);
    if (physical_gpio <= 0) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "wlan_power_on_enable suspend fail.\n");
        return;
    }
    oal_gpio_free(physical_gpio);
#endif
}

STATIC void mp13_suspend_wakeup_gpio(void)
{
    int physical_gpio;

    physical_gpio = conn_get_gpio_number_by_type(BFGX_WKUP_HOST);
    if (physical_gpio == 0) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "bfgn_wakeup_host suspend fail.\n");
        return;
    }
    oal_gpio_free(physical_gpio);
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    physical_gpio = conn_get_gpio_number_by_type(W_WKUP_HOST);
    if (physical_gpio == 0) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "wlan_wakeup_host suspend fail.\n");
        return;
    }
    oal_gpio_free(physical_gpio);

    physical_gpio = conn_get_gpio_number_by_type(HOST_WKUP_W);
    if (physical_gpio <= 0) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "host_wakeup_wlan suspend fail.\n");
        return;
    }
    oal_gpio_free(physical_gpio);
#endif
#ifdef _PRE_H2D_GPIO_WKUP
    physical_gpio = conn_get_gpio_number_by_type(HOST_WKUP_BFGX);
    if (physical_gpio <= 0) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "host_wakeup_bfg suspend fail.\n");
        return;
    }
    gpio_free(physical_gpio);
#endif
}

STATIC void mp13_suspend_tas_gpio(void)
{
    oal_gpio_free(g_st_board_info.rf_wifi_tas);
}

STATIC void mp13_suspend_gpio(void)
{
    mp13_suspend_tas_gpio();
    mp13_suspend_wakeup_gpio();
    mp13_suspend_power_gpio();
}

STATIC int32_t mp13_resume_gpio_reuqest_in(int32_t physical_gpio, const char *gpio_name)
{
    int32_t ret;

    oal_print_mpxx_log(MPXX_LOG_INFO, " physical_gpio is %d, gpio_name is %s.\n",
        physical_gpio, gpio_name);

    if ((physical_gpio == 0) || (gpio_name == NULL)) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "physical_gpio is %d.\n", physical_gpio);
        return BOARD_FAIL;
    }

    ret = board_gpio_request(physical_gpio, gpio_name, GPIO_DIRECTION_INPUT);
    if (ret != BOARD_SUCC) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "mp13_board_gpio(%s)_request_in fail.\n", gpio_name);
        return BOARD_FAIL;
    }

    return BOARD_SUCC;
}

STATIC int32_t mp13_resume_gpio_reuqest_out(int32_t physical_gpio, const char *gpio_name)
{
    int32_t ret;

    oal_print_mpxx_log(MPXX_LOG_INFO, " physical_gpio is %d, gpio_name is %s.\n",
        physical_gpio, gpio_name);

    if ((physical_gpio <= 0) || (gpio_name == NULL)) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "physical_gpio is %d.\n", physical_gpio);
        return BOARD_FAIL;
    }

    ret = board_gpio_request(physical_gpio, gpio_name, GPIO_DIRECTION_OUTPUT);
    if (ret != BOARD_SUCC) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "mp13_board_gpio(%s)_request_out fail.\n", gpio_name);
        return BOARD_FAIL;
    }

    return BOARD_SUCC;
}

STATIC void mp13_resume_power_gpio(void)
{
    int32_t ret;
    int32_t physical_gpio;

    physical_gpio = conn_get_gpio_number_by_type(CHIP_POWR);
    ret = mp13_resume_gpio_reuqest_out(physical_gpio, PROC_NAME_GPIO_POWEN_ON);
    if (ret != BOARD_SUCC) {
        return;
    }

    physical_gpio = conn_get_gpio_number_by_type(BFGX_POWER);
    ret = mp13_resume_gpio_reuqest_out(physical_gpio, PROC_NAME_GPIO_BFGX_POWEN_ON);
    if (ret != BOARD_SUCC) {
        return;
    }

    physical_gpio = conn_get_gpio_number_by_type(W_POWER);
    ret = mp13_resume_gpio_reuqest_out(physical_gpio, PROC_NAME_GPIO_WLAN_POWEN_ON);
    if (ret != BOARD_SUCC) {
        return;
    }

    return;
}

STATIC void mp13_resume_wakeup_gpio(void)
{
    int32_t ret;
    int32_t physical_gpio;

    physical_gpio = conn_get_gpio_number_by_type(BFGX_WKUP_HOST);
    ret = mp13_resume_gpio_reuqest_in(physical_gpio, PROC_NAME_GPIO_BFGX_WAKEUP_HOST);
    if (ret != BOARD_SUCC) {
        return;
    }

    physical_gpio = conn_get_gpio_number_by_type(W_WKUP_HOST);
    ret = mp13_resume_gpio_reuqest_in(physical_gpio, PROC_NAME_GPIO_WLAN_WAKEUP_HOST);
    if (ret != BOARD_SUCC) {
        return;
    }

    physical_gpio = conn_get_gpio_number_by_type(HOST_WKUP_W);
    ret = mp13_resume_gpio_reuqest_out(physical_gpio, PROC_NAME_GPIO_HOST_WAKEUP_WLAN);
    if (ret != BOARD_SUCC) {
        return;
    }

#ifdef _PRE_H2D_GPIO_WKUP
    physical_gpio = conn_get_gpio_number_by_type(HOST_WKUP_BFGX);
    ret = mp13_resume_gpio_reuqest_out(physical_gpio, PROC_NAME_GPIO_HOST_WAKEUP_BFG);
    if (ret != BOARD_SUCC) {
        return;
    }
#endif
    return;
}

STATIC void mp13_resume_tas_gpio(void)
{
    int32_t ret;
    if (g_st_board_info.wifi_tas_enable == WIFI_TAS_DISABLE) {
        return;
    }

    if (g_st_board_info.wifi_tas_state) {
        ret = gpio_request_one(g_st_board_info.rf_wifi_tas, GPIOF_OUT_INIT_HIGH, PROC_NAME_GPIO_WIFI_TAS);
    } else {
        ret = gpio_request_one(g_st_board_info.rf_wifi_tas, GPIOF_OUT_INIT_LOW, PROC_NAME_GPIO_WIFI_TAS);
    }
    if (ret) {
        ps_print_err("%s gpio_request failed\n", PROC_NAME_GPIO_WIFI_TAS);
        return;
    }
}

STATIC void mp13_resume_gpio(void)
{
    mp13_resume_power_gpio();
    mp13_resume_wakeup_gpio();
    mp13_resume_tas_gpio();
}
#else /* 非信创方案 */
#ifdef CONFIG_HIBERNATION
STATIC int32_t g_hibernate = OAL_FALSE;

/* S4睡眠前检查power on管脚是否拉低状态，芯片要在S4睡眠前下电，否则主芯片GPIO失效，connectivity芯片行为不可预期 */
STATIC int32_t hibernate_check(void)
{
    int32_t power_on_s4_state = 0;
    int32_t wl_en_s4_state = 0;
    int32_t bt_en_s4_state = 0;
    int32_t gle_en_s4_state = 0;

    power_on_s4_state = conn_get_gpio_level(CHIP_POWR);
    wl_en_s4_state = conn_get_gpio_level(W_POWER);
    bt_en_s4_state = conn_get_gpio_level(BFGX_POWER);
    gle_en_s4_state = conn_get_gpio_level(G_POWER);
    ps_print_info("CHIP_POWR[%d], W_EN[%d], B_EN[%d], G_EN[%d]\n",
                  power_on_s4_state, wl_en_s4_state, bt_en_s4_state, gle_en_s4_state);
    if (power_on_s4_state == GPIO_LEVEL_HIGH) {
        ps_print_err("should not s4 hibernate when chip power on");
        declare_dft_trace_key_info("s4 hibernate when powe on", OAL_DFT_TRACE_EXCEP);
        /* NOTIFY_BAD才能阻止内核pm流程 */
        return OAL_NOTIFY_BAD;
    }
    return OAL_SUCC;
}

STATIC int32_t pm_hibernate(void)
{
    struct platform_device *pdev = (struct platform_device*)(get_hi110x_board_info()->dev);
    int32_t ret = OAL_SUCC;

    if (pdev == NULL) {
        ps_print_err("pdev NULL");
        return ret;
    }
    if (g_hibernate == OAL_TRUE) {
        ps_print_err("already in hibernate state, need not process again");
        return OAL_SUCC;
    }

    ret = hibernate_check();
    if (ret != OAL_SUCC) {
        return ret;
    }
    conn_irq_hibernate();
    conn_board_gpio_remove(pdev);
    g_hibernate = OAL_TRUE;
    return ret;
}

STATIC int32_t pm_restore(void)
{
    struct platform_device *pdev = (struct platform_device*)(get_hi110x_board_info()->dev);

    if (pdev == NULL) {
        ps_print_err("pdev NULL");
        return OAL_SUCC;
    }

    if (g_hibernate == OAL_FALSE) {
        ps_print_err("NOT in hibernate state, need not restore");
        return OAL_SUCC;
    }
    conn_board_gpio_init(pdev);
    conn_irq_restore();
    g_hibernate = OAL_FALSE;
    return  OAL_SUCC;
}


STATIC int32_t mp13_pm_notify_handler(unsigned long mode, void *data)
{
    int32_t ret = OAL_SUCC;

    switch (mode) {
        case PM_SUSPEND_PREPARE:
            break;
        case PM_HIBERNATION_PREPARE:
            ret = pm_hibernate();
            break;
        case PM_POST_SUSPEND:
            break;
        case PM_POST_HIBERNATION:
            ret = pm_restore();
            break;
        default:
            break;
    }
    ps_print_info("mode[%lu][%s],ret[%d]\n", mode, PM_NOTIFY_STR, ret);
    return ret;
}
#endif
#endif

#ifdef _PRE_SHARE_BUCK_SURPORT
#ifndef CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT
/*
*  函 数 名  : buck_mode_get
*  功能描述  : 获取buck方案
*  0:  全内置buck
*  1:  I2C控制独立外置buck
*  2:  GPIO控制独立外置buck
*  3:  host控制共享外置buck电压
*/
STATIC uint8_t buck_mode_get(void)
{
    return  (((g_st_board_info.buck_param) & BUCK_MODE_MASK) >> BUCK_MODE_OFFSET);
}

STATIC void buck_pmic_cfg(uint16_t buck_control_flag, uint8_t high_vset)
{
#define BUCK_1_P15_V    1150000
#define BUCK_1_P05_V    1050000
#ifndef WBG_PMIC_REQ
#define WBG_PMIC_REQ 8
#endif
    struct hw_comm_pmic_cfg_t fp_pmic_ldo_set;
    uint8_t buck_num = buck_number_get();
    if (buck_num == BUCK_NUMBER_AE || buck_num == BUCK_NUMBER_ONNT) {
        fp_pmic_ldo_set.pmic_num = buck_num;
    } else {
        ps_print_info("share buck mode, invalid buck num cfg[%d], check dts & ini\n", buck_num);
        return;
    }
    fp_pmic_ldo_set.pmic_power_type = VOUT_BUCK_1;
    if (high_vset) {
        fp_pmic_ldo_set.pmic_power_voltage = BUCK_1_P15_V;
    } else {
        fp_pmic_ldo_set.pmic_power_voltage = BUCK_1_P05_V;
    }
    /* 打开或关闭 0/1 */
    if (buck_control_flag) {
        fp_pmic_ldo_set.pmic_power_state = 1;
    } else {
        fp_pmic_ldo_set.pmic_power_state = 0;
    }

    ps_print_info("share buck mode, buck_flag[0x%x],pmic_num[%d],pmic_power_type[%d],power_state[%d],vset[%d]\n",
                  buck_control_flag, fp_pmic_ldo_set.pmic_num, fp_pmic_ldo_set.pmic_power_type,
                  fp_pmic_ldo_set.pmic_power_state, fp_pmic_ldo_set.pmic_power_voltage);

    hw_pmic_power_cfg(WBG_PMIC_REQ, &fp_pmic_ldo_set);
}

/* 共享外置buck上下电接口 */
STATIC void buck_power_ctrl(uint8_t enable, uint8_t subsys)
{
    if (buck_mode_get() == EXT_BUCK_HOST_CTL) {
        uint8_t high_vset = OAL_TRUE;

        if (enable) {
            g_st_board_info.buck_control_flag |= (1 << subsys);
        } else {
            g_st_board_info.buck_control_flag &= ~(1 << subsys);
        }

        /* 仅平台默认上电，或者WIFI/BT打开，1.15v */
        if ((g_st_board_info.buck_control_flag == MPXX_PLAT_SUB_MASK) ||
            (g_st_board_info.buck_control_flag & MPXX_BT_SUB_MASK) ||
            (g_st_board_info.buck_control_flag & MPXX_WIFI_SUB_MASK)) {
            high_vset = OAL_TRUE;
        } else {
            high_vset = OAL_FALSE;
        }

        buck_pmic_cfg(g_st_board_info.buck_control_flag, high_vset);
        ps_print_info("share buck mode[0x%x]\n", g_st_board_info.buck_param);
    } else {
        ps_print_info("NOT share buck mode[0x%x], do nothing\n", g_st_board_info.buck_param);
    }
}
#endif
#endif

STATIC int32_t mp13_board_power_notify(uint32_t ul_subsystem, int32_t en)
{
#ifdef _PRE_SHARE_BUCK_SURPORT
#ifndef CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT
    buck_power_ctrl((uint8_t)en, (uint8_t)ul_subsystem);
#endif
#endif
    return SUCC;
}

void board_callback_init_mp13_power(void)
{
    g_st_board_info.bd_ops.bfgx_dev_power_on = mp13_bfgx_dev_power_on;
    g_st_board_info.bd_ops.bfgx_dev_power_off = mp13_bfgx_dev_power_off;
    g_st_board_info.bd_ops.wlan_power_off = mp13_wlan_power_off;
    g_st_board_info.bd_ops.wlan_power_on = mp13_wlan_power_on;
    g_st_board_info.bd_ops.board_power_on = mp13_board_power_on;
    g_st_board_info.bd_ops.board_power_off = mp13_board_power_off;
    g_st_board_info.bd_ops.board_power_reset = mp13_board_power_reset;
    g_st_board_info.bd_ops.board_power_notify = mp13_board_power_notify;
    g_st_board_info.bd_ops.board_error_handler = mp13_board_error_handler;
#if defined(_PRE_S4_FEATURE)
    g_st_board_info.bd_ops.suspend_board_gpio_in_s4 = mp13_suspend_gpio;
    g_st_board_info.bd_ops.resume_board_gpio_in_s4 = mp13_resume_gpio;
#else
#ifdef CONFIG_HIBERNATION
    g_st_board_info.bd_ops.pm_notify = mp13_pm_notify_handler;
#endif
#endif
}

STATIC void board_plat_cust_init_mp13(void)
{
    int32_t ret;
    plat_cust_data_mp13 plat_cust;
#if defined(CONFIG_FACTORY_MODE) || defined(FACTORY_VERSION)
    plat_cust.cfg.vmin_mode = VMIN_MODE_FACTORY;
#else
    plat_cust.cfg.vmin_mode = VMIN_MODE_NORMAL;
#endif
    ret = set_plat_cust_buf((uint8_t *)&plat_cust, sizeof(plat_cust_data_mp13));
    if (ret != SUCC) {
        ps_print_err("set plat cust buf fail, ret = %d\n", ret);
        return;
    }
    ps_print_info("set plat cust buf succ, vtype=%d\n", hi110x_get_release_type());
}

/* mpxx系列芯片board ops初始化 */
void board_info_init_mp13(void)
{
    board_callback_init_dts();
    board_callback_init_mp13_power();
    board_plat_cust_init_mp13();
}
