/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: pcie_chip.c header file
 * Author: @CompanyNameTag
 */

#ifndef PCIE_CHIP_H
#define PCIE_CHIP_H

#include "pcie_host.h"
#include "pcie_linux.h"

typedef int32_t (*linkup_fixup_func)(int32_t);
typedef struct _linkup_fixup_ops_ {
    linkup_fixup_func link_prepare_fixup;
    linkup_fixup_func link_fail_fixup;
    linkup_fixup_func link_succ_fixup;
} linkup_fixup_ops;

typedef int32_t (*board_chip_func)(linkup_fixup_ops*);
typedef struct _board_chip_id_stru_ {
    int32_t chip_id;
    board_chip_func func;
} board_chip_id_stru;

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
int32_t oal_pcie_chip_info_init(oal_pcie_res *pst_pci_res);
#elif (defined _PRE_PLAT_FEATURE_GF6X_PCIE)
int32_t oal_pcie_chip_info_init(oal_pcie_linux_res *pst_pci_res);
#endif
int32_t oal_pcie_board_init(void);

void oal_ete_default_intr_cb(void *fun);
int32_t oal_pcie_slave_soc_config(oal_pcie_linux_res *pcie_lres,
                                  oal_pci_slave_config_type type);
#endif
