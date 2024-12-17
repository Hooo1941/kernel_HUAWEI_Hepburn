/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : hmac_uapsd.c 的头文件
 * 作    者 :
 * 创建日期 : 2013年9月18日
 */

#ifndef HMAC_LOW_LATENCY_H
#define HMAC_LOW_LATENCY_H

#include "hmac_ext_if.h"

#define irq_cmd_is_onecpu(_cmd) ((_cmd) & ((_cmd) - 1)) /* 判断_cmd中是否只有一位bit为1 */

void hmac_low_latency_wifi_enable(void);
void hmac_low_latency_wifi_disable(void);
void hmac_register_pmqos_network_latency_handler(void);
void hmac_unregister_pmqos_network_latency_handler(void);
#endif /* end of hmac_uapsd.h */
