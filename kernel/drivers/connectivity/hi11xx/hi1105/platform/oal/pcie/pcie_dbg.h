/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:pcie_dbg.c header file
 * Author: @CompanyNameTag
 */

#ifndef PCIE_DBG_H
#define PCIE_DBG_H

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#include <linux/kernel.h>
int32_t oal_pcie_sysfs_group_create(oal_kobject *root_obj);
void oal_pcie_sysfs_group_remove(oal_kobject *root_obj);
#endif

#endif /* end of pcie_dbg.h */
