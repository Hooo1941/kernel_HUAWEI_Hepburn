/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:pcie_pcs.c header file
 * Author: @CompanyNameTag
 */

#ifndef PCIE_PCS_H
#define PCIE_PCS_H

#include "oal_types.h"

#ifndef NEWLINE
#define NEWLINE "\n"
#endif

#define pcie_pcs_udelay(utime) oal_udelay(utime)
#define pcie_pcs_print oal_io_print
void pcie_pcs_print_split_log_info(char *str);
#endif

