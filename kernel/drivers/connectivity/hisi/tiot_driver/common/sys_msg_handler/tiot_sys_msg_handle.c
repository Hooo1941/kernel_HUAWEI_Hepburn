/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description:  \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-02-29, Create file. \n
 */
#include "tiot_sys_msg_handle.h"
#include "tiot_sys_msg_types.h"
#include "tiot_pm.h"
#include "tiot_board_log.h"

void tiot_pkt_sys_msg_handle(tiot_controller *ctrl, uint8_t code)
{
    if ((code == SYS_INF_GNSS_INIT) || (code == SYS_INF_GNSS_LPPE_INIT)) {
        tiot_pm_set_event(&ctrl->pm, TIOT_PM_EVENT_REPORT_WORK);
    } else if (code == SYS_INF_REQUEST_SLEEP) {
        tiot_print_info("[TIoT:pkt]recv request sleep\n");
        tiot_pm_set_event(&ctrl->pm, TIOT_PM_EVENT_REQUEST_SLEEP);
    } else {
        tiot_print_warning("[TIoT:pkt]recv unknown sys msg:0x%x\n", code);
    }
}
