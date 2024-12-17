/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: CHBA 2.0 coex header file
 * Author: wifi
 * Create: 2021-02-06
 */

#ifndef HMAC_CHBA_COEX_MGMT_H
#define HMAC_CHBA_COEX_MGMT_H

#include "hmac_chba_common_function.h"
#include "hmac_chba_mgmt.h"
#include "wal_linux_vendor.h"
#include "wal_linux_ioctl.h"
#include "wal_config.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CHBA_COEX_H

#ifdef __cplusplus
extern "C" {
#endif
#ifdef _PRE_WLAN_CHBA_MGMT
#define HMAC_CHBA_INVALID_COEX_VAP_CNT 1
#define CHBA_DBAC_BA_RX_DEFAULT_TIMEOUT 100 /* CHBA DBAC模式下rx重排序队列超时定时器默认周期100 milliseconds */

// 上报CHBA设备的共存信道信息
typedef struct {
    hmac_info_report_type_enum report_type;
    uint8_t dev_mac_addr[WLAN_MAC_ADDR_LEN];
    uint8_t coex_chan_cnt;
    uint8_t coex_chan_lists[WLAN_5G_CHANNEL_NUM];
} hmac_chba_report_coex_chan_info;

// 上报CHBA设备的共存信道信息
typedef struct {
    hmac_info_report_type_enum report_type;
    uint8_t island_dev_cnt;
    uint8_t island_dev_lists[MAC_CHBA_MAX_ISLAND_DEVICE_NUM][WLAN_MAC_ADDR_LEN];
} hmac_chba_report_island_info;

typedef enum {
    HMAC_CHBA_COEX_CHAN_SWITCH_STA_ROAM_RPT = 0, /* 因STA漫游避免DBAC触发信道切换上报 */
    HMAC_CHBA_COEX_CHAN_SWITCH_STA_CSA_RPT = 1, /* 因dmac STA避免DBAC触发信道切换上报 */
    HMAC_CHBA_COEX_CHAN_SWITCH_CHBA_CHAN_RPT = 2, /* 因CHBA本岛切信道避免DBAC上层触发信道切换 */
    HMAC_CHBA_COEX_CHAN_SWITCH_BUTT,
} hmac_chba_coex_chan_switch_dbac_rpt_enum;

int32_t wal_ioctl_chba_set_coex_start_check(oal_net_device_stru *net_dev);
int32_t wal_ioctl_chba_set_coex_cmd_para_check(mac_chba_set_coex_chan_info *cfg_coex_chan_info);
int32_t wal_ioctl_chba_set_supp_coex_chan_list(oal_net_device_stru *net_dev, int8_t *command,
    wal_wifi_priv_cmd_stru priv_cmd);
int32_t wal_ioctl_chba_set_coex(oal_net_device_stru *net_dev, int8_t *command);
uint32_t hmac_chba_coex_info_changed_report(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param);
void hmac_chba_coex_island_info_report(mac_chba_self_conn_info_stru *self_conn_info);
uint32_t hmac_config_chba_set_coex_chan(mac_vap_stru *mac_vap, uint16_t len, uint8_t *params);
oal_bool_enum_uint8 hmac_chba_coex_chan_is_in_list(uint8_t *coex_chan_lists, uint8_t chan_idx, uint8_t chan_number);
void hmac_chba_coex_switch_chan_dbac_rpt(mac_channel_stru *mac_channel, uint8_t rpt_type);
void hmac_chba_dbac_vap_set_rx_reorder_timeout(uint8_t is_dbac_running);
oal_bool_enum_uint8 hmac_chba_coex_is_support_dbdc_dbac(mac_vap_stru **up_mac_vap, uint32_t up_vap_num);
#endif
#ifdef __cplusplus
}
#endif

#endif
