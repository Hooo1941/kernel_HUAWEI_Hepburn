/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:oal_pcie_chip_mp16c.c header file
 * Author: @CompanyNameTag
 */

#ifndef PCIE_CHIP_MP16C_H
#define PCIE_CHIP_MP16C_H

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#include "pcie_host.h"

int32_t oal_pcie_chip_info_init_mp16c(oal_pcie_res *pst_pci_res, int32_t device_id);
int32_t oal_pcie_board_init_mp16c(linkup_fixup_ops *ops);
void oal_pcie_link_speed_change_mp16c(oal_pci_dev_stru *pst_pci_dev, oal_pcie_res *pst_pci_res);
void oal_pcie_device_phy_config_by_ssi(uint32_t base_addr);
void oal_pcie_device_phy_config_by_changespeed(oal_pcie_res *pst_pci_res, uint32_t base_addr);
#endif
#endif
