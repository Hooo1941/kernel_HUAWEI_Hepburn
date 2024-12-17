/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : hmac_uapsd.c 的头文件
 * 作    者 :
 * 创建日期 : 2013年9月18日
 */

#ifndef HMAC_PM_H
#define HMAC_PM_H

#include "mac_user.h"
#include "hmac_ext_if.h"

#include "hmac_user.h"
#include "hmac_vap.h"
void hmac_register_pm_callback(void);
extern uint8_t g_wlan_device_pm_switch;
extern uint8_t g_wlan_min_fast_ps_idle;
extern uint8_t g_wlan_max_fast_ps_idle;
extern uint8_t g_wlan_auto_ps_screen_on;
extern uint8_t g_wlan_auto_ps_screen_off;
extern uint8_t g_wlan_ps_mode;
extern uint8_t g_wlan_fast_ps_dyn_ctl;
#endif /* end of hmac_uapsd.h */
