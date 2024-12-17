/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description:  \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-02-29, Create file. \n
 */
#ifndef TIOT_SYS_MSG_HANDLE_H
#define TIOT_SYS_MSG_HANDLE_H

#include "tiot_controller.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

void tiot_pkt_sys_msg_handle(tiot_controller *ctrl, uint8_t code);

void tiot_pkt_sys_msg_slave_handle(tiot_controller *ctrl, uint8_t code);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif