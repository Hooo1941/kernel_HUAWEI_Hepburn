/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:pcie_iatu.c header file
 * Author: @CompanyNameTag
 */

#ifndef PCIE_IATU_H
#define PCIE_IATU_H

#include "oal_types.h"
#include "pcie_linux.h"

int32_t oal_pcie_bar_init(oal_pcie_linux_res *pcie_res);
void oal_pcie_bar_exit(oal_pcie_linux_res *pcie_res);
void oal_pcie_iatu_reg_dump(oal_pcie_linux_res *pci_res);
void oal_pcie_iatu_outbound_dump_by_membar(const void *outbound_addr, uint32_t index);
#endif
