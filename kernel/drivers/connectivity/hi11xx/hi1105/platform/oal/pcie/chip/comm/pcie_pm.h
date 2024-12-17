/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:pcie_pm.c header file
 * Author: @CompanyNameTag
 */

#ifndef PCIE_PM_HISI_H
#define PCIE_PM_HISI_H

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#include "pcie_host.h"
extern uint32_t g_pcie_pcs_tracer_enable;

static inline uint32_t pcs_trace_linkup_flag(void)
{
    return (uint32_t)((uint32_t)(g_pcie_pcs_tracer_enable >> 4) & 0xf); /* 右移4位，获取bit[7]的值 */
}

static inline uint32_t pcs_trace_speedchange_flag(void)
{
    return (uint32_t)(g_pcie_pcs_tracer_enable & 0xf);
}

int32_t oal_pcie_pm_chip_init(oal_pcie_res *pst_pci_res, int32_t device_id);
int32_t pcie_pm_chip_init_mp13(oal_pcie_res *pst_pci_res, int32_t device_id);
int32_t pcie_pm_chip_init_mp15(oal_pcie_res *pst_pci_res, int32_t device_id);
int32_t pcie_pm_chip_init_mp16(oal_pcie_res *pst_pci_res, int32_t device_id);
int32_t pcie_pm_chip_init_mp16c(oal_pcie_res *pst_pci_res, int32_t device_id);
#endif

#endif
