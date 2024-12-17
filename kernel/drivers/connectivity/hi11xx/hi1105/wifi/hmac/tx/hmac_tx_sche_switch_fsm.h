/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : tx调度模式切换状态机
 * 作    者 : wifi
 * 创建日期 : 2021年3月31日
 */

#ifndef HMAC_TX_SCHE_SWITCH_FSM_H
#define HMAC_TX_SCHE_SWITCH_FSM_H

#include "oal_ext_if.h"

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
void hmac_tx_sche_switch_process(uint32_t tx_throughput);
#endif
uint8_t hmac_device_loop_sche_mode(void);

#endif
