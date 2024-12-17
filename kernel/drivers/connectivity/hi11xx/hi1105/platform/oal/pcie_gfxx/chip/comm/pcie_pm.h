/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:pcie_pm.c header file
 * Author: @CompanyNameTag
 */

#ifndef PCIE_PM_HISI_H
#define PCIE_PM_HISI_H

#ifdef _PRE_PLAT_FEATURE_GF6X_PCIE
#include "pcie_linux.h"

uint32_t pcs_trace_linkup_flag(void);
uint32_t pcs_trace_speedchange_flag(void);
int32_t oal_pcie_pm_chip_init(oal_pcie_linux_res *pst_pci_res, int32_t device_id);
int32_t pcie_pm_chip_init_gf61(oal_pcie_linux_res *pst_pci_res, int32_t device_id);
void oal_pcie_pcs_ini_config_init(void);

#endif
#endif
