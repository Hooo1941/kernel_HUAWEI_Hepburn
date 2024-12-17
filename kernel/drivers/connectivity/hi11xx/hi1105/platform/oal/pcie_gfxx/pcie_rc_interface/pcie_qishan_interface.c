/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:pcie driver
 * Author: @CompanyNameTag
 */

#ifdef _PRE_PLAT_FEATURE_GF6X_PCIE
#ifdef _PRE_QISHAN_BOARD
#include "oal_hardware.h"
#include "oal_util.h"
#include "external/lpcpu_feature.h"
#include "plat_pm.h"
#include "oal_pcie_interface.h"

#ifdef _PRE_HOST_SUPPORT_PCIE_CUSTOMIZE
int pcie_kport_enumerate(uint32_t rc_idx);
int pcie_kport_pm_control(uint32_t power_ops, uint32_t rc_idx);
int pcie_kport_lp_ctrl(uint32_t rc_idx, uint32_t enable);
int pcie_kport_power_notifiy_register(uint32_t rc_idx, int (*poweron)(void* data),
    int (*poweroff)(void* data), void* data);
#endif

void oal_pcie_deregister_event(oal_pcie_linux_res *pst_pci_lres)
{
    return;
}

void oal_pcie_register_event(oal_pci_dev_stru *pst_pci_dev, oal_pcie_linux_res *pst_pci_lres)
{
    return;
}

#ifdef _PRE_HOST_PCIE_ALWAYS_ON
static void oal_pcie_ete_board_power_action(hcc_bus *hi_bus, int32_t power_option)
{
    oal_pcie_linux_res *pst_pci_res = (oal_pcie_linux_res *)hi_bus->data;

    if (oal_unlikely(pst_pci_res == NULL)) {
        pci_print_log(PCI_LOG_INFO, "pst_pci_lres is null");
        return;
    }
    /* HOST PCIE未定制化场景，PCIE需要保持建链状态，GT不下电 */
    if (pst_pci_res->dev_id == HCC_EP_GT_DEV) {
        return;
    }

    if (power_option == PCIE_POWER_ON) {
        board_wlan_gpio_power_on((void *)W_POWER);
    } else {
        board_wlan_gpio_power_off((void *)W_POWER);
    }
}
#endif

int32_t oal_pcie_pm_control(oal_pcie_linux_res *pst_pci_lres, u32 rc_idx, int power_option)
{
    int32_t ret = OAL_SUCC;
    if (power_option == PCIE_POWER_ON) {
        wlan_pm_idle_sleep_vote(DISALLOW_IDLESLEEP);
    }
#ifdef _PRE_HOST_SUPPORT_PCIE_CUSTOMIZE
    ret = pcie_kport_pm_control(power_option, rc_idx);
    pci_print_log(PCI_LOG_INFO, "host_pcie_pm_control ret=%d,dev id %d\n", ret, pst_pci_lres->dev_id);
#endif
#ifdef _PRE_HOST_PCIE_ALWAYS_ON
    ret = OAL_SUCC;
    oal_pcie_ete_board_power_action(hcc_get_bus(pst_pci_lres->dev_id), power_option);
#endif
    if ((power_option == PCIE_POWER_OFF_WITH_L3MSG) || (power_option == PCIE_POWER_OFF_WITHOUT_L3MSG)) {
        wlan_pm_idle_sleep_vote(ALLOW_IDLESLEEP);
    }
    oal_reference(ret);
    return ret;
}

int32_t oal_pcie_power_notifiy_register(u32 rc_idx, int (*poweron)(void *data),
                                        int (*poweroff)(void *data), void *data)
{
#ifdef _PRE_HOST_SUPPORT_PCIE_CUSTOMIZE
    int32_t ret = pcie_kport_power_notifiy_register(rc_idx, poweron, poweroff, data);
    pci_print_log(PCI_LOG_INFO, "host_pcie_power_notifiy_register ret=%d\n", ret);
    return ret;
#endif
    return OAL_SUCC;
}

int32_t oal_pcie_host_aspm_init(oal_pcie_linux_res *pst_pci_lres)
{
    pci_print_log(PCI_LOG_INFO, "not adapter pcie host aspm\n");
    return OAL_SUCC;
}

int32_t oal_pcie_get_mps(oal_pci_dev_stru *pst_pcie_dev)
{
    return OAL_SUCC;
}

/* Max Payload Size Supported,  must config beofre pcie access */
int32_t oal_pcie_mps_init(oal_pcie_linux_res *pst_pci_lres)
{
    return OAL_SUCC;
}

int32_t oal_pcie_enumerate(int32_t rc_idx)
{
    return OAL_SUCC;
}
#endif // _PRE_QISHAN_BOARD
#endif // _PRE_PLAT_FEATURE_GF6X_PCIE
