/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : HMAC PCIE动态切换 头文件
 * 作    者 :
 * 创建日期 : 2022年3月31日
 */
#include "hmac_vap.h"
#include "hmac_user.h"

void hmac_user_del_dyn_pcie_vote(mac_vap_stru *mac_vap);
void hmac_vap_down_pcie_vote(mac_vap_stru *mac_vap, mac_device_stru *mac_device);
uint32_t hmac_pcie_up_prepare_succ_handle(mac_vap_stru *mac_vap, uint8_t len, uint8_t *prepare_flag);
void hmac_pcie_switch_up_recover(uint32_t switch_ret);
uint8_t hmac_get_pcie_switch_func(void);
void hmac_process_assoc_dyn_pcie_vote(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
uint32_t hmac_pcie_callback_fsm_func(void);
void hmac_pcie_switch_vote_by_pps(uint32_t trx_total_pps);
