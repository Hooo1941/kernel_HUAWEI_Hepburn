/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : WIFI 投屏低功耗
 * 作    者 : huyuxiang
 * 创建日期 : 2023年4月6日
 */

#ifndef HMAC_LP_MIRACAST_H
#define HMAC_LP_MIRACAST_H

#include "hmac_ext_if.h"

uint32_t hmac_config_notify_lp_miracast(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
void hmac_config_control_lp_miracast(mac_vap_stru *mac_vap, uint8_t is_siso);
uint32_t hmac_config_set_lp_miracast(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
uint32_t hmac_m2s_lp_miracast_report_state(mac_vap_stru *mac_vap, uint8_t uc_len, uint8_t *param);
void hmac_config_lp_miracast_stat_mbps(uint32_t tx_throughput_mbps, uint32_t rx_throughput_mbps);
void hmac_config_down_vap_close_lp_miracast(mac_vap_stru *mac_vap);
void hmac_config_close_lp_miracast(mac_vap_stru *mac_vap);
#endif /* end of hmac_lp_miracast.h */
