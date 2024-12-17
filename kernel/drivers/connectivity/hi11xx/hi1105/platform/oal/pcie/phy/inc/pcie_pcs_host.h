/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:pcie_pcs_host header file
 * Author: @CompanyNameTag
 */

#ifndef PCIE_PCS_HOST_H
#define PCIE_PCS_HOST_H

#include "oal_types.h"
#include "pcie_host.h"
void oal_pcie_device_phy_config_single_lane(oal_pcie_res *pst_pci_res, uint32_t base_addr);
void oal_pcie_device_phy_disable_l1ss_rekey(oal_pcie_res *pst_pci_res, uint32_t base_addr);
void oal_pcie_phy_ini_config_init(void);
#endif

