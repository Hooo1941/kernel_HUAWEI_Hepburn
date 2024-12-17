/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:pcie_interface.h header file
 * Author: @CompanyNameTag
 */

#ifndef OAL_PCIE_INTERFACE_H
#define OAL_PCIE_INTERFACE_H

#include "oal_types.h"
#include "oal_hardware.h"
#include "pcie_linux_gfxx.h"
#include "pcie_host_gfxx.h"

int32_t oal_pcie_enumerate(int32_t rc_idx);
void oal_pcie_deregister_event(oal_pcie_linux_res *pst_pci_lres);
void oal_pcie_register_event(oal_pci_dev_stru *pst_pci_dev, oal_pcie_linux_res *pst_pci_lres);
int32_t oal_pcie_pm_control(oal_pcie_linux_res *pst_pci_lres, u32 rc_idx, int power_option);
int32_t oal_pcie_power_notifiy_register(u32 rc_idx, int (*poweron)(void *data),
                                        int (*poweroff)(void *data), void *data);
int32_t oal_pcie_host_aspm_init(oal_pcie_linux_res *pst_pci_lres);
int32_t oal_pcie_get_mps(oal_pci_dev_stru *pst_pcie_dev);
int32_t oal_pcie_mps_init(oal_pcie_linux_res *pst_pci_lres);

#endif /* end for OAL_PCIE_INTERFACE_H */

