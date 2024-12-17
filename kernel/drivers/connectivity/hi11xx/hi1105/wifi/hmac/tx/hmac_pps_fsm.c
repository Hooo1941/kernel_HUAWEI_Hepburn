/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : pps驱动特性状态机
 * 作    者 : wifi
 * 创建日期 : 2021年3月31日
 */

#include "hmac_pps_fsm.h"
#include "oam_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_PPS_FSM_C

void hmac_pps_fsm_handle_event(uint8_t event, hmac_pps_fsm_stru *fsm)
{
    uint8_t entry;

    for (entry = 0; entry < fsm->entry_num; entry++) {
        if (fsm->fsm_table[entry].current_state != oal_atomic_read(&fsm->current_state) ||
            fsm->fsm_table[entry].event != event) {
            continue;
        }

        if (fsm->fsm_table[entry].action && fsm->fsm_table[entry].action() != OAL_SUCC) {
            break;
        }

        oal_atomic_set(&fsm->current_state, fsm->fsm_table[entry].next_state);

        oam_warning_log4(0, 0, "{hmac_pps_fsm_handle_event::fsm[%d] event[%d] trigger state[%d] to state[%d]}",
            fsm->id, event, fsm->fsm_table[entry].current_state, fsm->fsm_table[entry].next_state);

        break;
    }
}

