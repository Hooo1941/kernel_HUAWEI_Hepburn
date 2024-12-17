/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: factory test
 * Author: @CompanyNameTag
 */

/* 头文件包含 */
#include "board_mp13.h"
#include "plat_firmware.h"
#include "plat_pm.h"
#include "pcie_linux.h"
#include "factory_misc.h"

int32_t g_ft_fail_powerdown_bypass = 0;

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE

OAL_STATIC int32_t pcie_ip_test_power_on(hcc_bus *pst_bus)
{
    int32_t ret;
    ret = board_power_on(W_SYS);
    if (ret) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "power on wlan failed=%d", ret);
        return ret;
    }

    hcc_bus_power_action(pst_bus, HCC_BUS_POWER_PATCH_LOAD_PREPARE);

    ret = hcc_bus_reinit(pst_bus);
    if (ret != OAL_SUCC) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "reinit bus %s failed, ret=%d", pst_bus->name, ret);
        return ret;
    }
    return OAL_SUCC;
}

OAL_STATIC void pcie_ip_test_power_off(void)
{
    (void)board_power_off(W_SYS);
    (void)board_power_off(B_SYS);
    (void)board_power_off(G_SYS);
}

OAL_STATIC int32_t pcie_ip_test_pre(hcc_bus **old_bus)
{
    int32_t ret;
    hcc_bus *hi_bus = NULL;
    if (!bfgx_is_shutdown()) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "bfgx is open, test abort!");
        bfgx_print_subsys_state(BUART);
        return -OAL_ENODEV;
    }

    if (!wlan_is_shutdown()) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "wlan is open, test abort!");
        return -OAL_ENODEV;
    }

    *old_bus = hcc_get_bus(HCC_EP_WIFI_DEV);
    if (*old_bus == NULL) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "mpxx_bus is null, test abort!");
        return -OAL_ENODEV;
    }

    hi_bus = *old_bus;
    if (hi_bus->bus_type != HCC_BUS_PCIE) {
        /* 尝试切换到PCIE */
        ret = hcc_switch_bus(HCC_EP_WIFI_DEV, HCC_BUS_PCIE);
        if (ret) {
            oal_print_mpxx_log(MPXX_LOG_ERR, "switch to PCIe failed, ret=%d", ret);
            return -OAL_ENODEV;
        }
        oal_print_mpxx_log(MPXX_LOG_INFO, "switch to PCIe ok.");
    } else {
        old_bus = NULL;
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t pcie_ip_test_core(hcc_bus *hi_bus, int32_t test_count)
{
    int32_t ret = OAL_SUCC;
    int32_t pre_fwdownload_way = g_st_board_info.download_mode;
    g_st_board_info.download_mode = MODE_COMBO;
    /* power on wifi, need't download firmware */
    ret = pcie_ip_test_power_on(hi_bus);
    if (ret != OAL_SUCC) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "test power on failed");
        goto test_core_fail;
    }

    wlan_pm_init_dev();

    ret = mp13_check_device_ready();
    if (ret != OAL_SUCC) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "check_device_ready failed, ret=%d", ret);
        goto test_core_fail;
    }

    ret = oal_pcie_ip_factory_test(hi_bus, test_count);
    if (ret != OAL_SUCC) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "pcie_ip_factory_test failed, ret=%d", ret);
        goto test_core_fail;
    }
    g_st_board_info.download_mode = pre_fwdownload_way;
    pcie_ip_test_power_off();
    return ret;

test_core_fail:
    g_st_board_info.download_mode = pre_fwdownload_way;
    if (!g_ft_fail_powerdown_bypass) {
        pcie_ip_test_power_off();
    }
    return ret;
}

OAL_STATIC int32_t pcie_ip_test_later(hcc_bus *hi_bus, hcc_bus *old_bus)
{
    int32_t ret = OAL_SUCC;
    if (old_bus != NULL) {
        ret = hcc_switch_bus(HCC_EP_WIFI_DEV, old_bus->bus_type);
        if (ret != OAL_SUCC) {
            oal_print_mpxx_log(MPXX_LOG_ERR, "restore to bus %s failed, ret=%d", old_bus->name, ret);
            return -OAL_ENODEV;
        }
        oal_print_mpxx_log(MPXX_LOG_INFO, "resotre to bus %s ok.", old_bus->name);
    }
    return ret;
}

int32_t hi1103_pcie_ip_test(int32_t test_count)
{
    int32_t ret;
    hcc_bus *pst_bus = hcc_get_bus(HCC_EP_WIFI_DEV);
    hcc_bus *old_bus = NULL;
    struct pm_top* pm_top_data = pm_get_top();
    declare_time_cost_stru(cost);

    if (oal_pcie_mpxx_working_check() != OAL_TRUE) {
        /* 不支持PCIe,直接返回成功 */
        oal_print_mpxx_log(MPXX_LOG_WARN, "do not support PCIe!");
        return OAL_SUCC;
    }

    if (oal_mutex_trylock(&pm_top_data->host_mutex) == 0) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "get lock fail!");
        return -EINVAL;
    }

    ret = pcie_ip_test_pre(&old_bus);
    if (ret != OAL_SUCC) {
        goto init_fail;
    }

    oal_get_time_cost_start(cost);
    hcc_bus_and_wake_lock(pst_bus);

    ret = pcie_ip_test_core(pst_bus, test_count);
    if (ret != OAL_SUCC) {
        goto test_fail;
    }

    ret = pcie_ip_test_later(pst_bus, old_bus);
    if (ret != OAL_SUCC) {
        goto test_fail;
    }

    hcc_bus_and_wake_unlock(pst_bus);
    oal_mutex_unlock(&pm_top_data->host_mutex);
    oal_get_time_cost_end(cost);
    oal_calc_time_cost_sub(cost);
    oal_print_mpxx_log(MPXX_LOG_INFO, "mp13 pcie ip test %llu us", time_cost_var_sub(cost));

    return OAL_SUCC;
test_fail:
    ssi_dump_device_regs(SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_PCIE_CFG);
    hcc_bus_and_wake_unlock(pst_bus);
init_fail:
    oal_mutex_unlock(&pm_top_data->host_mutex);
    return ret;
}

EXPORT_SYMBOL(hi1103_pcie_ip_test);

OAL_STATIC uint32_t g_slt_pcie_status = 0;
OAL_STATIC hcc_bus *g_slt_old_bus = NULL;
/* for kirin slt test */
OAL_STATIC int32_t pcie_test_init(hcc_bus *hi_bus)
{
    int32_t ret;
    ret = hcc_bus_reinit(hi_bus);
    if (ret != OAL_SUCC) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "reinit bus %d failed, ret=%d", hi_bus->bus_type, ret);
        ssi_dump_device_regs(SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_PCIE_CFG);
        return ret;
    }

    wlan_pm_init_dev();

    ret = mp13_check_device_ready();
    if (ret != OAL_SUCC) {
        ssi_dump_device_regs(SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_PCIE_CFG);
        oal_print_mpxx_log(MPXX_LOG_ERR, "check_device_ready failed, ret=%d", ret);
        return ret;
    }

    ret = oal_pcie_ip_init(hi_bus);
    if (ret != OAL_SUCC) {
        ssi_dump_device_regs(SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_PCIE_CFG);
        oal_print_mpxx_log(MPXX_LOG_ERR, "oal_pcie_ip_init failed, ret=%d", ret);
        return ret;
    }

    ret = oal_pcie_ip_voltage_bias_init(hi_bus);
    if (ret != OAL_SUCC) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "voltage_bias_init failed, ret=%d", ret);
        return ret;
    }
    return OAL_SUCC;
}

int32_t hi110x_pcie_chip_poweron(void *data)
{
    int32_t ret;
    hcc_bus *pst_bus = NULL;
    hcc_bus *old_bus = NULL;

    if (oal_pcie_mpxx_working_check() != OAL_TRUE) {
        /* 不支持PCIe,直接返回成功 */
        oal_print_mpxx_log(MPXX_LOG_WARN, "do not support PCIe!");
        return -OAL_ENODEV;
    }

    ret = pcie_ip_test_pre(&old_bus);
    if (ret != OAL_SUCC) {
        return -OAL_EFAIL;
    }

    g_slt_old_bus = old_bus;

    pst_bus = hcc_get_bus(HCC_EP_WIFI_DEV);

    hcc_bus_and_wake_lock(pst_bus);
    /* power on wifi, need't download firmware */
    ret = board_power_on(W_SYS);
    if (ret) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "power on wlan failed=%d", ret);
        hcc_bus_and_wake_unlock(pst_bus);
        return ret;
    }

    hcc_bus_power_action(pst_bus, HCC_BUS_POWER_PATCH_LOAD_PREPARE);

    ret = pcie_test_init(pst_bus);
    if (ret != OAL_SUCC) {
        if (!g_ft_fail_powerdown_bypass) {
            (void)board_power_off(W_SYS);
        }
        hcc_bus_and_wake_unlock(pst_bus);
        return ret;
    }
    g_slt_pcie_status = 1;

    return 0;
}

int32_t hi110x_pcie_chip_poweroff(void *data)
{
    int32_t ret;
    hcc_bus *pst_bus;

    pst_bus = hcc_get_bus(HCC_EP_WIFI_DEV);
    if (pst_bus == NULL) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "can't find any wifi bus");
        return -OAL_ENODEV;
    }

    if (pst_bus->bus_type != HCC_BUS_PCIE) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "current bus is %s , not PCIe", pst_bus->name);
        return -OAL_ENODEV;
    }

    if (g_slt_pcie_status != 1) {
        oal_print_mpxx_log(MPXX_LOG_WARN, "pcie slt is not power on");
        return -OAL_EBUSY;
    }

    g_slt_pcie_status = 0;

    /* SLT下电之前打印链路信息 */
    hcc_bus_chip_info(pst_bus, OAL_FALSE, OAL_TRUE);

    (void)board_power_off(W_SYS);

    hcc_bus_and_wake_unlock(pst_bus);

    if (g_slt_old_bus != NULL) {
        ret = hcc_switch_bus(HCC_EP_WIFI_DEV, g_slt_old_bus->bus_type);
        if (ret) {
            oal_print_mpxx_log(MPXX_LOG_ERR, "restore to bus %s failed, ret=%d", g_slt_old_bus->name, ret);
            return -OAL_ENODEV;
        }
        oal_print_mpxx_log(MPXX_LOG_INFO, "resotre to bus %s ok.", g_slt_old_bus->name);
    }

    return 0;
}

int32_t hi110x_pcie_chip_transfer(void *ddr_address, uint32_t data_size, uint32_t direction)
{
    hcc_bus *pst_bus;

    pst_bus = hcc_get_bus(HCC_EP_WIFI_DEV);
    if (pst_bus == NULL) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "can't find any wifi bus");
        return -OAL_ENODEV;
    }

    if (pst_bus->bus_type != HCC_BUS_PCIE) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "current bus is %s , not PCIe", pst_bus->name);
        return -OAL_ENODEV;
    }

    return oal_pcie_rc_slt_chip_transfer(pst_bus, ddr_address, data_size, (int32_t)direction);
}
EXPORT_SYMBOL(hi110x_pcie_chip_poweron);
EXPORT_SYMBOL(hi110x_pcie_chip_transfer);
EXPORT_SYMBOL(hi110x_pcie_chip_poweroff);
#endif

#if (defined(CONFIG_PCIE_KIRIN_SLT_HI110X)|| defined(CONFIG_PCIE_KPORT_SLT_DEVICE)) && defined(CONFIG_HISI_DEBUG_FS)
int pcie_slt_hook_register(u32 rc_id, u32 device_type, int (*init)(void *), int (*on)(void *),
                           int (*off)(void *), int (*setup)(void *), int (*data_transfer)(void *, u32, u32));
int32_t mp13_pcie_chip_init(void *data)
{
    oal_print_mpxx_log(MPXX_LOG_INFO, "slt pcie init");
    return 0;
}

int32_t hi1103_pcie_chip_rc_slt_register(void)
{
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
    return pcie_slt_hook_register(g_kirin_rc_idx, 0x2,
                                  mp13_pcie_chip_init, hi110x_pcie_chip_poweron,
                                  hi110x_pcie_chip_poweroff, mp13_pcie_chip_init,
                                  hi110x_pcie_chip_transfer);
#else
    return 0;
#endif
}

int32_t hi1103_pcie_chip_rc_slt_unregister(void)
{
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
    return pcie_slt_hook_register(g_kirin_rc_idx, 0x2,
                                  NULL, NULL,
                                  NULL, NULL,
                                  NULL);
#else
    return 0;
#endif
}
EXPORT_SYMBOL(hi1103_pcie_chip_rc_slt_register);
EXPORT_SYMBOL(hi1103_pcie_chip_rc_slt_unregister);
#endif
