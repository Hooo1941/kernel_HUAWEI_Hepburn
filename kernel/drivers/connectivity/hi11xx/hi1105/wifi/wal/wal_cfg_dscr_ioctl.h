/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 :
 * 创建日期 : 2022年11月19日
 */

#ifndef WAL_DSCR_CFG_IOCTL_H
#define WAL_DSCR_CFG_IOCTL_H
#include "mac_vap.h"
#include "oal_ext_if.h"

#ifdef WIFI_DEBUG_ENABLE

uint32_t wal_hipriv_set_protocol_debug_info(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_set_tx_pow_param(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_set_ucast_data_dscr_param(oal_net_device_stru *net_dev, int8_t *param);
uint32_t wal_hipriv_set_bcast_data_dscr_param(oal_net_device_stru *net_dev, int8_t *param);
uint32_t wal_hipriv_set_ucast_mgmt_dscr_param(oal_net_device_stru *net_dev, int8_t *param);
uint32_t wal_hipriv_set_mbcast_mgmt_dscr_param(oal_net_device_stru *net_dev, int8_t *param);
uint32_t wal_hipriv_set_ampdu_aggr_num(oal_net_device_stru *pst_net_dev, int8_t *pc_param);


#endif
#endif
