/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: Determine the device chip type
 * Author: @CompanyNameTag
 */

#ifndef HISI__ONEIMAGE_H
#define HISI__ONEIMAGE_H

#include <linux/version.h>
#ifdef CONFIG_HWCONNECTIVITY
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 1, 0))
#include <linux/huawei/hw_connectivity.h>
#include <linux/huawei/gps/huawei_gps.h>
#else
#include <huawei_platform/connectivity/hw_connectivity.h>
#include <huawei_platform/connectivity/huawei_gps.h>
#endif
#endif

#endif
