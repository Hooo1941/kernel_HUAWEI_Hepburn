/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 多通道退化白名单探测函数定义
 * 作    者 : wifi
 * 创建日期 : 2020年11月23日
 */

#ifndef HMAC_DEGRADATION_C
#define HMAC_DEGRADATION_C

#include "hmac_degradation.h"
#include "hmac_vap.h"
#include "hmac_user.h"
#include "hmac_config.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_DEGRADATION_C

#ifdef _PRE_WLAN_FEATURE_DEGRADE_SWITCH
/*
 * 功能描述  : host调控发送能力退化功能
 * 1.日    期  : 2022.4.14
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_degradation_tx_chain_switch(mac_vap_stru *mac_vap, uint8_t degradation_enable)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_DEGRADATION_SWITCH,
        sizeof(uint8_t), (uint8_t *)&degradation_enable);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_degradation_tx_chain_disable:: failed[%d].}", ret);
    }
}
#endif /* end of _PRE_WLAN_FEATURE_DEGRADE_SWITCH */
#endif /* end of _HMAC_MCM_DEGRADATION_C */

