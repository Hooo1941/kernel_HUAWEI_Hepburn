/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : hmac_vsp.c 的头文件
 * 作者       :
 */

#ifndef HMAC_VSP_IF_H
#define HMAC_VSP_IF_H

#include "hmac_user.h"
#include "hmac_rx_data.h"

#ifdef _PRE_WLAN_FEATURE_VSP
typedef enum {
    VSP_CMD_LINK_EN,
    VSP_CMD_TEST_EN,
    VSP_CMD_BUTT,
} hmac_vsp_debug_cmd_enum;

typedef enum {
    VSP_PARAM_MAX_TRANSMIT_DLY,
    VSP_PARAM_VSYNC_INTERVAL,
    VSP_PARAM_MAX_FEEDBACK_DLY,
    VSP_PARAM_SLICES_PER_FRAME,
    VSP_PARAM_LAYER_NUM,
    VSP_PARAM_BUTT,
} hmac_vsp_debug_param_enum;

typedef struct {
    uint8_t type;
    uint8_t link_en;
    uint8_t mode;
    uint8_t user_mac[OAL_MAC_ADDR_LEN];
    uint32_t param[VSP_PARAM_BUTT];
    uint8_t test_en;
    uint32_t test_frm_cnt;
} hmac_vsp_debug_cmd;

typedef enum {
    VSP_CMD_CFG = 0,
    VSP_CMD_START,
    VSP_CMD_STOP,
} vsp_cmd_enum;

typedef struct {
    int cmd;
    int slice_cnt;
    int layer_cnt;
    int max_payload;
    int header_len;
    int tmax;
    int tslice;
    int tfeedback;
    char sa[OAL_MAC_ADDR_LEN];
    char ra[OAL_MAC_ADDR_LEN];
    char mode; // recv side: 0, send side 1
} hmac_vsp_cmd;

#define VSP_RX_ACCEPT 0
#define VSP_RX_REFUSE 1

uint32_t hmac_vsp_cfg(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, hmac_vsp_cmd *param);
uint32_t hmac_vsp_start(hmac_user_stru *hmac_user);
uint32_t hmac_vsp_stop(hmac_user_stru *hmac_user);
uint32_t hmac_vsp_common_timeout(frw_event_mem_stru *event_mem);
uint32_t hmac_set_vsp_info_addr(frw_event_mem_stru *frw_mem);
uint32_t hmac_vsp_rx_proc(hmac_host_rx_context_stru *rx_context, oal_netbuf_stru *netbuf);
void hmac_vsp_tx_complete_process(void);
uint32_t hmac_vsp_debug_cmd_proc(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
uint32_t hmac_vsp_set_log_lvl(mac_vap_stru *mac_vap, uint32_t *params);
#endif
#endif
