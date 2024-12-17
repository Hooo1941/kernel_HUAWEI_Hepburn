/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : WIFI OAM LOG功能
 * 创建日期   : 2022年6月10日
 */

#include "oam_event_wifi.h"
#include "oam_event.h"
#include "wlan_types.h"
#include "oam_main.h"

int32_t oam_wifi_init(void)
{
    uint8_t uc_vap_idx;
    int8_t l_return;
    l_return = oam_subsys_register(WLAN_USER_MAX_USER_LIMIT, OAM_OUTPUT_TYPE_SDT);
    if (l_return != 0) {
        oal_io_print("oam_wifi_init: wifi system register fail\n");
        return l_return;
    }
    /* 日志全局开关默认为开 */
    oam_log_set_global_switch(OAL_SWITCH_ON);

    /* VAP级别日志设置 */
    for (uc_vap_idx = 0; uc_vap_idx < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; uc_vap_idx++) {
        /* 设置VAP日志级别 */
        oam_log_set_vap_level(uc_vap_idx, OAM_LOG_DEFAULT_LEVEL);
    }
    return OAL_SUCC;
}
