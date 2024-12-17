/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 配置相关实现hmac接口实现源文件
 * 作    者 :
 * 创建日期 : 2013年1月8日
 */

/* 1 头文件包含 */
#include "hmac_config.h"
#include "hmac_fsm.h"
#include "hmac_mgmt_join.h"
#ifdef _PRE_WLAN_CHBA_MGMT
#include "hmac_chba_function.h"
#include "hmac_chba_ps.h"
#include "hmac_chba_chan_switch.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CONFIG_CHBA_C

#ifdef _PRE_WLAN_CHBA_MGMT
/*
 * 功能描述  : CHBA维测日志开关接口
 * 日    期  : 2021年09月06日
 * 作    者  : wifi7
 */
uint32_t hmac_config_chba_log_level(mac_vap_stru *mac_vap, uint16_t len, uint8_t *params)
{
    uint32_t ret;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_chba_vap_stru *chba_vap_info = NULL;

    if ((mac_vap == NULL) || (params == NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_chba_log_level:ptr is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 非CHBA vap不做处理 */
    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_chba_vap_start_check(hmac_vap) == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_CHBA, "{hmac_config_chba_log_level:hmac_vap is not chba vap}");
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    chba_vap_info = hmac_get_up_chba_vap_info();
    if (chba_vap_info == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_chba_log_level:chba_vap_info is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    chba_vap_info->chba_log_level = *(uint8_t *)params;
    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
        "{hmac_config_chba_log_level: chba_log_level is %d}", chba_vap_info->chba_log_level);
    /***************************************************************************
       抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_CHBA_LOG_LEVEL, len, params);
    if (ret != OAL_SUCC) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CHBA, "{hmac_config_chba_log_level:fail to send event to dmac}");
        return ret;
    }
    return OAL_SUCC;
}
/*
 * 功能描述  : 配置CHBA低功耗吞吐门限
 * 日    期  : 2021年12月07日
 * 作    者  : wifi7
 */
uint32_t hmac_config_chba_set_ps_thres(mac_vap_stru *mac_vap, uint16_t len, uint8_t *params)
{
    hmac_vap_stru *hmac_vap = NULL;
    hmac_chba_vap_stru *chba_vap_info = NULL;
    chba_ps_thres_config_stru *ps_thres_cfg = NULL;
    hmac_chba_ps_throught_thres_stru *chba_thres = NULL;

    if ((mac_vap == NULL) || (params == NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_chba_set_ps_thres:ptr is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 非CHBA vap不做处理 */
    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_chba_vap_start_check(hmac_vap) == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_CHBA, "{hmac_config_chba_set_ps_thres:hmac_vap is not chba vap}");
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    chba_vap_info = hmac_get_up_chba_vap_info();
    if (chba_vap_info == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_chba_log_level:chba_vap_info is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ps_thres_cfg = (chba_ps_thres_config_stru *)params;
    oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CFG,
        "{hmac_config_chba_set_ps_thres: thres_type[%d], new_ps_throught_thres %d(100ms)}",
        ps_thres_cfg->thres_type, ps_thres_cfg->thres_val);
    chba_thres = hmac_chba_get_ps_throught_thres(hmac_vap);
    switch (ps_thres_cfg->thres_type) {
        case CHBA_PS_THRES_LEVEL_THREE_TO_TWO:
            chba_thres->upgrade_to_lv_two_thres = ps_thres_cfg->thres_val;
            break;
        case CHBA_PS_THRES_LEVEL_TWO_TO_ONE:
            chba_thres->upgrade_to_lv_one_thres = ps_thres_cfg->thres_val;
            break;
        case CHBA_PS_THRES_LEVEL_ONE_TO_TWO:
            chba_thres->downgrade_to_lv_two_thres = ps_thres_cfg->thres_val;
            break;
        case CHBA_PS_THRES_LEVEL_TWO_TO_THREE:
            chba_thres->downgrade_to_lv_three_thres = ps_thres_cfg->thres_val;
            break;
        default:
            oam_error_log1(0, OAM_SF_CHBA,
                "{hmac_config_chba_set_ps_thres:invalid thres_type[%d]}", ps_thres_cfg->thres_type);
            break;
    }
    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_CFG,
        "{hmac_config_chba_set_ps_thres:three_to_two[%d], two_to_one[%d], one_to_two[%d], two_to_three[%d]}",
        chba_thres->upgrade_to_lv_two_thres, chba_thres->upgrade_to_lv_one_thres,
        chba_thres->downgrade_to_lv_two_thres, chba_thres->downgrade_to_lv_three_thres);
    return OAL_SUCC;
}

/*
 * 功能描述  : CHBA整岛切信道命令接口
 * 日    期  : 2021年09月14日
 * 作    者  : wifi7
 */
uint32_t hmac_config_chba_island_chan_switch(mac_vap_stru *mac_vap, uint16_t len, uint8_t *params)
{
    hmac_vap_stru *hmac_vap = NULL;
    mac_chba_adjust_chan_info *chan_info = NULL;

    if (oal_any_null_ptr2(mac_vap, params)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 非chba vap不处理 */
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_chba_vap_start_check(hmac_vap) == OAL_FALSE) {
        return OAL_SUCC;
    }

    chan_info = (mac_chba_adjust_chan_info *)params;
    oam_warning_log3(0, OAM_SF_ANY, "CHBA: hmac_config_chba_island_chan_switch: chan[%d] bw[%d], switch_type[%d].",
        chan_info->chan_number, chan_info->bandwidth, chan_info->switch_type);

    /* 启动全岛切信道流程 */
    hmac_chba_adjust_channel_proc(hmac_vap, chan_info);
    return OAL_SUCC;
}
#endif
