/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:External Interfaces of the pciee phy electronic eye diagram
 * Author: @CompanyNameTag
 */

#ifndef PCIE_PCS_DEYE_PATTERN_H
#define PCIE_PCS_DEYE_PATTERN_H

#include "oal_types.h"
#include "pcie_pcs_regs.h"
int32_t pcie_pcs_deye_pattern_draw(pcs_reg_ops *reg_ops, uintptr_t base_addr, uint32_t lane, uint8_t *buff,
    int32_t size);
#endif

