/*
 * 版权所有 (c) 华为技术有限公司 2021-2021
 * 功能说明   : nve_info_interface.h
 * 作者       :
 * 创建日期   : 2021年1月24日
 */
#ifndef __NVE_INFO_INTERFACE_H__
#define __NVE_INFO_INTERFACE_H__
#ifndef _PRE_WLAN_EXPORT
#ifdef CONFIG_ARCH_PLATFORM
#include <linux/mtd/nve_ap_kernel_interface.h>
#define external_nve_info_user opt_nve_info_user
#define external_nve_direct_access_interface nve_direct_access_interface
#else
#ifdef CONFIG_HISI_NVE
#include <linux/mtd/hisi_nve_interface.h>
#define external_nve_info_user hisi_nve_info_user
#define external_nve_direct_access_interface hisi_nve_direct_access
#else
#include <linux/mtd/hw_nve_interface.h>
#define external_nve_info_user hw_nve_info_user
#define external_nve_direct_access_interface hw_nve_direct_access
#endif
#endif
#endif
#endif
