/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:pcie driver
 * Author: @CompanyNameTag
 */

#ifdef _PRE_PLAT_FEATURE_GF6X_PCIE
/* HOST未适配RC Interface或不需要适配的, 将接口定义成空函数 */
#if !(defined _PRE_QISHAN_BOARD) &&  !(defined CONFIG_ARCH_KIRIN_PCIE)

#include "oal_pcie_interface.h"

int32_t oal_pcie_enumerate(int32_t rc_idx)
{
    return OAL_SUCC;
}

void oal_pcie_deregister_event(oal_pcie_linux_res *pst_pci_lres)
{
}

void oal_pcie_register_event(oal_pci_dev_stru *pst_pci_dev, oal_pcie_linux_res *pst_pci_lres)
{
}

int32_t oal_pcie_pm_control(oal_pcie_linux_res *pst_pci_lres, u32 rc_idx, int power_option)
{
    return OAL_SUCC;
}

int32_t oal_pcie_power_notifiy_register(u32 rc_idx, int (*poweron)(void *data),
    int (*poweroff)(void *data), void *data)
{
    return OAL_SUCC;
}

int32_t oal_pcie_host_aspm_init(oal_pcie_linux_res *pst_pci_lres)
{
    return OAL_SUCC;
}

int32_t oal_pcie_get_mps(oal_pci_dev_stru *pst_pcie_dev)
{
    return OAL_SUCC;
}

int32_t oal_pcie_mps_init(oal_pcie_linux_res *pst_pci_lres)
{
    return OAL_SUCC;
}
#endif
#endif // _PRE_PLAT_FEATURE_GF6X_PCIE
