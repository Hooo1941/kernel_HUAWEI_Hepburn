/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : linux ioctl配置命令
 * 作    者 :
 * 创建日期 : 2012年12月10日
 */
#ifdef _PRE_WLAN_CHBA_MGMT
#include "mac_chba_common.h"
#include "hmac_chba_function.h"
#include "hmac_chba_common_function.h"
#include "hmac_chba_coex.h"
#include "hmac_chba_ps.h"
#include "hmac_chba_chan_switch.h"
#include "hmac_chba_sync.h"
#include "wal_linux_netdev_ops.h"
#include "wal_config.h"
#include "wal_linux_ioctl.h"

/* 信道切换打桩命令 */
uint32_t wal_hipriv_chba_chan_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param);

uint32_t wal_hipriv_chba_module_init(oal_net_device_stru *net_dev, int8_t *pc_param);

uint32_t wal_hipriv_set_coex_chan_info(oal_net_device_stru *net_dev, int8_t *pc_param);
uint32_t wal_hipriv_set_chba_battery(oal_net_device_stru *net_dev, int8_t *pc_param);

/* 设置CHBA 自动调整低功耗bitmap开关，0：关闭自动调整，1：打开自动调整 */
uint32_t wal_hipriv_chba_set_auto_bitmap(oal_net_device_stru *net_dev, int8_t *pc_param);
/* 设置CHBA user bitmap */
uint32_t wal_hipriv_chba_set_user_bitmap(oal_net_device_stru *net_dev, int8_t *pc_param);
/* 设置CHBA vap bitmap */
uint32_t wal_hipriv_chba_set_vap_bitmap(oal_net_device_stru *net_dev, int8_t *pc_param);

uint32_t wal_hipriv_chba_log_level(oal_net_device_stru *net_dev, int8_t *param);

uint32_t wal_hipriv_chba_set_ps_thres(oal_net_device_stru *net_dev, int8_t *param);

uint32_t wal_hipriv_chba_island_chan_switch(oal_net_device_stru *net_dev, int8_t *param);
/* 设置chba特性开关 */
uint32_t wal_hipriv_set_chba_feature_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
#endif

