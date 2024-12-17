/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:oam_dsm.c header file
 * Author: @CompanyNameTag
 */

#ifndef OAM_DSM_H
#define OAM_DSM_H

#if defined(CONFIG_HUAWEI_OHOS_DSM) || defined(CONFIG_HUAWEI_DSM)

#if defined(_PRE_PRODUCT_HI1620S_KUNPENG) || defined(_PRE_WINDOWS_SUPPORT)
#include <linux/huawei/dsm/dsm_pub.h>
#else
#include <dsm/dsm_pub.h>
#endif

#define DSM_MPXX_TCXO_ERROR        909030001
#define DSM_PCIE_ENMU_FAIL         909030002
#define DSM_MPXX_DOWNLOAD_FIRMWARE 909030033
#define DSM_BUCK_PROTECTED         909030034
#define DSM_MPXX_HALT              909030035
#define DSM_WIFI_FEMERROR          909030036
#define DSM_PCIE_SWITCH_SDIO_FAIL  909030043
#define DSM_PCIE_SWITCH_SDIO_SUCC  909030044
#define DSM_WIFI_LNAERROR          909030046
#define DSM_BT_FEMERROR            913002007

#define DSM_BUF_MAX_SIZE           256 /* DSM最大上报长度 */
void hw_mpxx_dsm_client_notify(int sub_sys, int dsm_id, const char *fmt, ...);
void hw_mpxx_register_dsm_client(void);
void hw_mpxx_unregister_dsm_client(void);

#endif // CONFIG_HUAWEI_DSM
#endif // __OAM_DSM_H__
