/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:pcie_chip_gf61.c header file
 * Author: @CompanyNameTag
 */

#ifndef PCIE_CHIP_GF61_H
#define PCIE_CHIP_GF61_H

#include "pcie_host.h"

void oal_pcie_device_phy_config_by_changespeed(oal_pcie_linux_res *pcie_res, uint32_t base_addr);
void oal_pcie_link_speed_change_gf61(oal_pcie_linux_res *pcie_res);
int32_t oal_pcie_eye_param_change_gf61(oal_pcie_linux_res *pcie_res);
int32_t oal_pcie_chip_info_init_gf61(oal_pcie_linux_res *pst_pci_res, int32_t device_id);
void oal_pcie_ete_dereset(oal_pcie_linux_res *pcie_res);
#endif
