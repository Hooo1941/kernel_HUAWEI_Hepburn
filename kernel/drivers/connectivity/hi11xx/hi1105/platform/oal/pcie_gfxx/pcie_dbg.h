/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:pcie_dbg.c header file
 * Author: @CompanyNameTag
 */

#ifndef PCIE_DBG_H
#define PCIE_DBG_H

#include <linux/kernel.h>
int32_t oal_pcie_sysfs_group_create(oal_kobject *root_obj);
void oal_pcie_sysfs_group_remove(oal_kobject *root_obj);
#endif /* end of pcie_dbg.h */