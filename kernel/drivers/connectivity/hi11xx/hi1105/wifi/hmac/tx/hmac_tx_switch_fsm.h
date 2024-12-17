/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : host/device ring tx切换状态机模块
 * 作    者 : wifi
 * 创建日期 : 2021年1月4日
 */

#ifndef HMAC_TX_SWITCH_FSM_H
#define HMAC_TX_SWITCH_FSM_H

#include "oal_ext_if.h"

#define HMAC_PSM_TX_SWITCH_LOW_PPS 4500
#define HMAC_PSM_TX_SWITCH_HIGH_PPS 5500

typedef enum {
    PSM_TX_H2D_SWITCH,
    PSM_TX_D2H_SWITCH,

    PSM_TX_SWITCH_BUTT,
} hmac_psm_tx_switch_event_enum;

uint32_t hmac_get_tx_switch_fsm_state(void);
void hmac_tx_switch_fsm_init(void);
void hmac_tx_switch_fsm_handle_event(hmac_psm_tx_switch_event_enum ring_state);
void hmac_tx_fsm_quick_switch_to_device(void);
#endif
