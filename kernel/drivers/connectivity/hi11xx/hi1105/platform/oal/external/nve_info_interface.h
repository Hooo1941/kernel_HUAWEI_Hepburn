/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:nve_info_interface.h header file
 * Author: @CompanyNameTag
 */

#ifndef NVE_INFO_INTERFACE_H
#define NVE_INFO_INTERFACE_H

#ifdef CONFIG_ARCH_PLATFORM
#include <linux/mtd/nve_ap_kernel_interface.h>
#define external_nve_info_user opt_nve_info_user
#define external_nve_direct_access_interface nve_direct_access_interface
#else
#include <linux/mtd/hisi_nve_interface.h>
#define external_nve_info_user hisi_nve_info_user
#define external_nve_direct_access_interface hisi_nve_direct_access
#endif

#endif
