/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : P2P 相关特性处理
 * 作    者 :
 * 创建日期 : 2014年11月25日
 */


/* 1 头文件包含 */
#include "hmac_p2p.h"
#include "hmac_mgmt_join.h"
#include "oal_util.h"
#include "hmac_ext_if.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_vap.h"
#include "mac_mib.h"
#include "hmac_rx_filter.h"
#include "hmac_fsm.h"
#include "hmac_mgmt_bss_comm.h"
#include "mac_frame.h"
#include "hmac_user.h"
#include "hmac_mgmt_ap.h"
#include "securec.h"
#include "hmac_chan_mgmt.h"
#ifdef _PRE_WLAN_FEATURE_DFS
#include "hmac_dfs.h"
#endif
#ifdef _PRE_WLAN_CHBA_MGMT
#include "hmac_chba_function.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_P2P_C
#define HMAC_P2P_GO_CSA_COUNT 5
#define HMAC_P2P_GO_VENDOR_CMD_CSA_COUNT 2

typedef struct mac_vap_state_priority {
    uint8_t uc_priority;
} mac_vap_state_priority_stru;

typedef struct hmac_input_req_priority {
    uint8_t uc_priority;
} hmac_input_req_priority_stru;

mac_vap_state_priority_stru g_mac_vap_state_priority_table[MAC_VAP_STATE_BUTT] = {
    { 0 }, /* {MAC_VAP_STATE_INIT */
    { 2 }, /* {MAC_VAP_STATE_UP */
    { 0 }, /* {MAC_VAP_STATE_PAUSE */
    /* ap 独有状态 */
    { 0 }, /* {MAC_VAP_STATE_AP_PAUSE */
    /* sta独有状态 */
    { 0 },  /* {MAC_VAP_STATE_STA_FAKE_UP */
    { 10 }, /* {MAC_VAP_STATE_STA_WAIT_SCAN */
    { 0 },  /* {MAC_VAP_STATE_STA_SCAN_COMP */
    { 10 }, /* {MAC_VAP_STATE_STA_JOIN_COMP */
    { 10 }, /* {MAC_VAP_STATE_STA_WAIT_AUTH_SEQ2 */
    { 10 }, /* {MAC_VAP_STATE_STA_WAIT_AUTH_SEQ4 */
    { 10 }, /* {MAC_VAP_STATE_STA_AUTH_COMP */
    { 10 }, /* {MAC_VAP_STATE_STA_WAIT_ASOC */
    { 10 }, /* {MAC_VAP_STATE_STA_OBSS_SCAN */
    { 10 }, /* {MAC_VAP_STATE_STA_BG_SCAN */
    { 0 },  /* MAC_VAP_STATE_STA_LISTEN */
    { 10 }  /* MAC_VAP_STATE_ROAMING */
};

hmac_input_req_priority_stru g_mac_fsm_input_type_priority_table[HMAC_FSM_INPUT_TYPE_BUTT] = {
    { 0 }, /* HMAC_FSM_INPUT_RX_MGMT */
    { 0 }, /* HMAC_FSM_INPUT_RX_DATA */
    { 0 }, /* HMAC_FSM_INPUT_TX_DATA */
    { 0 }, /* HMAC_FSM_INPUT_TIMER0_OUT */
    { 0 }, /* HMAC_FSM_INPUT_MISC */

    { 0 }, /* HMAC_FSM_INPUT_START_REQ */

    { 5 }, /* HMAC_FSM_INPUT_SCAN_REQ */
    { 5 }, /* HMAC_FSM_INPUT_JOIN_REQ */
    { 5 }, /* HMAC_FSM_INPUT_AUTH_REQ */
    { 5 }, /* HMAC_FSM_INPUT_ASOC_REQ */

    { 5 }, /* HMAC_FSM_INPUT_LISTEN_REQ */
    { 0 }  /* HMAC_FSM_INPUT_LISTEN_TIMEOUT */
};

/* 功能描述: 外部输入事件触发检查 检查其他vap连接状态 */
OAL_STATIC oal_bool_enum_uint8 hmac_enter_state_check_vap_connect_state(mac_vap_stru *mac_vap,
    hmac_fsm_input_type_enum input_req, oal_bool_enum_uint8 is_high_prio_event)
{
#ifdef _PRE_WLAN_CHBA_MGMT
    oal_bool_enum_uint8 is_high_prio_scan_event =
        ((input_req == HMAC_FSM_INPUT_SCAN_REQ) && (is_high_prio_event == OAL_TRUE));
    /* CHBA启动但是未同步上，不允许被外部输入事件打断;已同步有其他用户入网的场景，避免入网失败也不允许被外部输入事件打断 */
    if (hmac_check_chba_is_wait_sync(mac_vap, is_high_prio_scan_event) == OAL_TRUE) {
        return OAL_TRUE;
    }
#endif
    return hmac_user_is_connecting(mac_vap);
}

/*
 * 功能描述  : 检查外部输入事件是否允许执行
 * 日    期  : 2014年11月25日
 * 修改内容  : 新生成函数
 */
uint32_t hmac_check_can_enter_state(mac_vap_stru *mac_vap, hmac_fsm_input_type_enum input_req,
    oal_bool_enum_uint8 is_high_prio_event)
{
    mac_device_stru *mac_device = NULL;
    mac_vap_stru *other_vap = NULL;
    uint8_t vap_num;
    uint8_t vap_idx;

    /*
     * 2.1 检查其他vap 状态，判断输入事件优先级是否比vap 状态优先级高
     * 如果输入事件优先级高，则可以执行输入事件
     */
    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_device == NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_p2p_check_can_enter_state::mac_res_get_dev fail.device_id = %u}",
                         mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    vap_num = mac_device->uc_vap_num;

    for (vap_idx = 0; vap_idx < vap_num; vap_idx++) {
        other_vap = mac_res_get_mac_vap(mac_device->auc_vap_id[vap_idx]);
        if (other_vap == NULL) {
            oam_warning_log1(0, OAM_SF_CFG, "{hmac_p2p_check_can_enter_state::get_mac_vap fail.vap_idx = %u}",
                             mac_device->auc_vap_id[vap_idx]);
            continue;
        }

        if (other_vap->uc_vap_id == mac_vap->uc_vap_id) {
            /* 如果检测到是自己，则继续检查其他VAP 状态 */
            continue;
        }

        /* 当GO或者AP在进行关联时，不进行扫描业务 */
        if ((g_mac_vap_state_priority_table[other_vap->en_vap_state].uc_priority >
             g_mac_fsm_input_type_priority_table[input_req].uc_priority) ||
            (hmac_enter_state_check_vap_connect_state(other_vap, input_req, is_high_prio_event) != OAL_FALSE)) {
            return OAL_ERR_CODE_CONFIG_BUSY;
        }
    }
    return OAL_SUCC;
}

/*
 * 功能描述  : 添加vap时检查P2P vap的num是否符合要求
 * 日    期  : 2014年11月25日
 * 作    者  : wifi
 * 修改内容  : 新生成函数
 */
uint32_t hmac_check_p2p_vap_num(mac_device_stru *mac_device, wlan_p2p_mode_enum_uint8 p2p_mode)
{
    if (mac_device == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_check_p2p_vap_num::mac_device is NULL}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (p2p_mode == WLAN_P2P_DEV_MODE) {
        /* 判断已创建的P2P_DEVICE 个数是否达到最大值,P2P0只能创建1个、且不能与AP共存 */
        if (mac_device->st_p2p_info.uc_p2p_device_num >= WLAN_MAX_SERVICE_P2P_DEV_NUM) {
            /* 已创建的P2P_DEV个数达到最大值 */
            oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_check_vap_num::only can created 1 p2p_device[%d].}",
                mac_device->st_p2p_info.uc_p2p_device_num);
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        }

        /* 剩余资源是否足够申请一份P2P_DEV */
        if (mac_device->uc_sta_num >= g_wlan_spec_cfg->max_sta_num) {
            /* 已有AP创建 */
            oam_warning_log1(0, OAM_SF_CFG, "{hmac_check_p2p_vap_num::can not create p2p_device any more[%d].}",
                             mac_device->uc_sta_num);
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        }
    } else if (p2p_mode == WLAN_P2P_CL_MODE) {
        /* 判断已创建的P2P_CLIENT 个数是否达到最大值 */
        if (mac_device->st_p2p_info.uc_p2p_goclient_num >= g_wlan_spec_cfg->max_p2p_group_num) {
            /* 已创建的P2P_DEV个数达到最大值 */
            oam_warning_log1(0, OAM_SF_CFG, "{hmac_check_p2p_vap_num::only can created 1 p2p_GO/Client[%d].}",
                             mac_device->st_p2p_info.uc_p2p_goclient_num);
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        }

        /* P2P0是否已经创建 */
        if (mac_device->st_p2p_info.uc_p2p_device_num != WLAN_MAX_SERVICE_P2P_DEV_NUM) {
            /* 已有AP创建 */
            oam_warning_log1(0, OAM_SF_CFG, "{hmac_check_p2p_vap_num::there's no p2p0 %d,can not create p2p_CL.}",
                             mac_device->st_p2p_info.uc_p2p_device_num);
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        }
    } else if (p2p_mode == WLAN_P2P_GO_MODE) {
        /* 判断已创建的P2P_CL/P2P_GO 个数是否达到最大值 */
        if (mac_device->st_p2p_info.uc_p2p_goclient_num >= g_wlan_spec_cfg->max_p2p_group_num) {
            /* 已创建的P2P_DEV个数达到最大值 */
            oam_warning_log1(0, OAM_SF_CFG, "{hmac_check_p2p_vap_num::only can created 1 p2p_GO/Client[%d].}",
                             mac_device->st_p2p_info.uc_p2p_goclient_num);
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        }
        /* 是否有足够的AP vap模式资源供创建一个P2P_GO */
        if ((mac_device->uc_vap_num - mac_device->uc_sta_num) >= WLAN_SERVICE_AP_MAX_NUM_PER_DEVICE) {
            /* 已创建的AP个数达到最大值4 */
            oam_warning_log1(0, OAM_SF_CFG, "{hmac_check_p2p_vap_num::ap num exceeds the supported spec[%d].}",
                             mac_device->uc_vap_num - mac_device->uc_sta_num);
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        }
    } else {
        return OAL_ERR_CODE_INVALID_CONFIG; /* 入参错误，无效配置 */
    }
    return OAL_SUCC;
}

/*
 * 功能描述  : 创建P2P CL 业务VAP
 * 日    期  : 2014年12月31日
 * 作    者  : wifi
 * 修改内容  : 新生成函数
 */
uint32_t hmac_add_p2p_cl_vap(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    mac_cfg_add_vap_param_stru *vap_param = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    uint32_t ret;
    mac_device_stru *mac_dev = NULL;
    uint8_t vap_id;
    wlan_p2p_mode_enum_uint8 p2p_mode;

    vap_param = (mac_cfg_add_vap_param_stru *)param;
    mac_dev = mac_res_get_dev(mac_vap->uc_device_id);
    if (oal_unlikely(mac_dev == NULL)) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_add_p2p_cl_vap::mac_dev null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* VAP个数判断 */
    p2p_mode = vap_param->en_p2p_mode;
    ret = hmac_check_p2p_vap_num(mac_dev, p2p_mode);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_add_p2p_cl_vap::hmac_config_check_vap_num failed[%d].}", ret);
        return ret;
    }

    /* P2P CL 和P2P0 共用一个VAP 结构，创建P2P CL 时不需要申请VAP 资源，需要返回p2p0 的vap 结构 */
    vap_id = mac_dev->st_p2p_info.uc_p2p0_vap_idx;
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_add_p2p_cl_vap::mac_res_get_hmac_vap failed.vap_id:[%d].}", vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    vap_param->uc_vap_id = vap_id;
    hmac_vap->pst_net_device = vap_param->pst_net_dev;

    /* 包括'\0' */
    if (EOK != memcpy_s(hmac_vap->auc_name, OAL_IF_NAME_SIZE, vap_param->pst_net_dev->name, OAL_IF_NAME_SIZE)) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_add_p2p_cl_vap::memcpy fail!");
        return OAL_FAIL;
    }

    /* 将申请到的mac_vap空间挂到net_device priv指针上去 */
    oal_net_dev_priv(vap_param->pst_net_dev) = &hmac_vap->st_vap_base_info;
    vap_param->us_muti_user_id = hmac_vap->st_vap_base_info.us_multi_user_idx;
    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = hmac_config_send_event(&hmac_vap->st_vap_base_info, WLAN_CFGID_ADD_VAP, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        /* 异常处理，释放内存 */
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_add_p2p_cl_vap::alloc_event failed[%d].}", ret);
        return ret;
    }

    if (vap_param->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        /* 初始化us_assoc_vap_id为最大值代表ap未关联 */
        mac_vap_set_assoc_id(&hmac_vap->st_vap_base_info, g_wlan_spec_cfg->invalid_user_id);
    }
    mac_vap_set_p2p_mode(&hmac_vap->st_vap_base_info, vap_param->en_p2p_mode);
    mac_inc_p2p_num(&hmac_vap->st_vap_base_info);

    oam_info_log3(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_add_p2p_cl_vap::vap_mode[%d], p2p_mode[%d}, vap_id[%d]",
                  vap_param->en_vap_mode, vap_param->en_p2p_mode, vap_param->uc_vap_id);

    return OAL_SUCC;
}

/*
 * 功能描述  : 删除p2p cl vap
 * 日    期  : 2014年12月31日
 * 作    者  : wifi
 * 修改内容  : 新生成函数
 */
uint32_t hmac_del_p2p_cl_vap(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    hmac_vap_stru *hmac_vap = NULL;
    uint32_t ret;
    mac_device_stru *mac_device = NULL;
    uint8_t vap_id;
    mac_cfg_del_vap_param_stru *del_vap_param = (mac_cfg_del_vap_param_stru *)param;

    if (oal_unlikely(oal_any_null_ptr2(mac_vap, param))) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_del_vap::input params null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (oal_unlikely(mac_device == NULL)) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_del_vap::mac_device null.devid[%d]}",
                       mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 如果是删除P2P CL ，则不需要释放VAP 资源 */
    vap_id = mac_device->st_p2p_info.uc_p2p0_vap_idx;
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_del_vap::get_hmac_vap fail.vap_id[%d]}", vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* p2p关联过程中，开始作为cl模式，切换到GO模式时需要停用vap，置为NULL */
    oal_net_dev_priv(hmac_vap->pst_net_device) = NULL;
    hmac_vap->pst_net_device = hmac_vap->pst_p2p0_net_device;
    mac_dec_p2p_num(&hmac_vap->st_vap_base_info);
    mac_vap_set_p2p_mode(&hmac_vap->st_vap_base_info, WLAN_P2P_DEV_MODE);
    if (hmac_vap->pst_net_device == NULL || hmac_vap->pst_net_device->ieee80211_ptr == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_del_p2p_cl_vap::pst_net_device or ieee80211_ptr null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (memcpy_s(mac_mib_get_StationID(&hmac_vap->st_vap_base_info), WLAN_MAC_ADDR_LEN,
                 mac_mib_get_p2p0_dot11StationID(&hmac_vap->st_vap_base_info), WLAN_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_del_p2p_cl_vap::memcpy fail!");
        return OAL_FAIL;
    }
    // 防止下发参数p2p_mode不为p2p_cl导致dmac将sta vap删除异常
    if (del_vap_param->en_p2p_mode != WLAN_P2P_CL_MODE) {
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_del_p2p_cl_vap::current vap state[%d], p2p mode[%d], netdev iftype[%d]}", mac_vap->en_vap_state,
            mac_vap->en_p2p_mode, hmac_vap->pst_net_device->ieee80211_ptr->iftype);
        del_vap_param->en_p2p_mode = WLAN_P2P_CL_MODE;
    }
    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_DEL_VAP, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_del_p2p_cl_vap::send_event failed[%d].}", ret);
        return ret;
    }

    oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_del_p2p_cl_vap::func out.vap_mode[%d], p2p_mode[%d]}",
                     del_vap_param->en_vap_mode, del_vap_param->en_p2p_mode);
    return OAL_SUCC;
}

/*
 * 功能描述  : 监听超时处理,通知WAL 监听超时
 * 日    期  : 2014年11月25日
 * 修改内容  : 新生成函数
 */
uint32_t hmac_p2p_send_listen_expired_to_host(hmac_vap_stru *hmac_vap)
{
    mac_device_stru *mac_device = NULL;
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *event = NULL;
    oal_wireless_dev_stru *wdev = NULL;
    mac_p2p_info_stru *p2p_info = NULL;
    hmac_p2p_listen_expired_stru *p2p_listen_expired = NULL;

    mac_device = mac_res_get_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (mac_device == NULL) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P,
                         "{hmac_send_mgmt_to_host::mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    p2p_info = &mac_device->st_p2p_info;
    /* 填写上报监听超时, 上报的网络设备应该采用p2p0 */
    if (hmac_vap->pst_p2p0_net_device && hmac_vap->pst_p2p0_net_device->ieee80211_ptr) {
        wdev = hmac_vap->pst_p2p0_net_device->ieee80211_ptr;
    } else if (hmac_vap->pst_net_device && hmac_vap->pst_net_device->ieee80211_ptr) {
        wdev = hmac_vap->pst_net_device->ieee80211_ptr;
    } else {
        /*  vap已经删除，需要直接返回，wpa_supplicant不会继续处理 */
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P,
                         "{hmac_send_mgmt_to_host::vap has deleted.}");
        return OAL_FAIL;
    }

    /* 组装事件到WAL，上报监听结束 */
    event_mem = frw_event_alloc_m(sizeof(hmac_p2p_listen_expired_stru));
    if (event_mem == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P,
                       "{hmac_send_mgmt_to_host::event_mem null.}");
        return OAL_FAIL;
    }

    /* 填写事件 */
    event = frw_get_event_stru(event_mem);
    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_HOST_CTX, HMAC_HOST_CTX_EVENT_SUB_TYPE_LISTEN_EXPIRED,
                       sizeof(hmac_p2p_listen_expired_stru), FRW_EVENT_PIPELINE_STAGE_0,
                       hmac_vap->st_vap_base_info.uc_chip_id, hmac_vap->st_vap_base_info.uc_device_id,
                       hmac_vap->st_vap_base_info.uc_vap_id);

    p2p_listen_expired = (hmac_p2p_listen_expired_stru *)(event->auc_event_data);
    p2p_listen_expired->st_listen_channel = p2p_info->st_listen_channel;
    p2p_listen_expired->pst_wdev = wdev;

    /* 分发事件 */
    frw_event_dispatch_event(event_mem);
    frw_event_free_m(event_mem);

    return OAL_SUCC;
}

/*
 * 功能描述  : 监听超时处理，通知DMAC 返回home 信道
 * 日    期  : 2014年11月25日
 * 修改内容  : 新生成函数
 */
uint32_t hmac_p2p_send_listen_expired_to_device(hmac_vap_stru *hmac_vap)
{
    uint32_t ret;
    hmac_device_stru *hmac_device = NULL;

    oam_info_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P,
                  "{hmac_p2p_send_listen_expired_to_device:: listen timeout!.}");
    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = hmac_config_send_event(&(hmac_vap->st_vap_base_info), WLAN_CFGID_CFG80211_CANCEL_REMAIN_ON_CHANNEL, 0, NULL);
    /* 强制stop listen */
    hmac_device = hmac_res_get_mac_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (oal_unlikely(hmac_device == NULL)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_p2p_send_listen_expired_to_device::hmac_device[%d] null!}",
                       hmac_vap->st_vap_base_info.uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (hmac_device->st_scan_mgmt.en_is_scanning == OAL_TRUE) {
        hmac_device->st_scan_mgmt.en_is_scanning = OAL_FALSE;
    }

    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P,
            "{hmac_p2p_send_listen_expired_to_device::hmac_config_send_event failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}

/*
 * 功能描述  : 停止p2p noa,p2p oppps
 * 日    期  : 2015年03月11日
 * 修改内容  : 新生成函数
 */
void hmac_disable_p2p_pm(hmac_vap_stru *hmac_vap)
{
    mac_vap_stru *mac_vap = NULL;
    mac_cfg_p2p_ops_param_stru p2p_ops;
    mac_cfg_p2p_noa_param_stru p2p_noa;
    uint32_t ret;

    mac_vap = &(hmac_vap->st_vap_base_info);
    memset_s(&p2p_noa, sizeof(p2p_noa), 0, sizeof(p2p_noa));
    ret = hmac_config_set_p2p_ps_noa(mac_vap, sizeof(mac_cfg_p2p_noa_param_stru), (uint8_t *)&p2p_noa);
    if (ret != OAL_SUCC) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P,
                       "{hmac_disable_p2p_pm::hmac_config_set_p2p_ps_noa disable p2p NoA fail.}");
    }
    memset_s(&p2p_ops, sizeof(p2p_ops), 0, sizeof(p2p_ops));
    ret = hmac_config_set_p2p_ps_ops(mac_vap, sizeof(mac_cfg_p2p_ops_param_stru), (uint8_t *)&p2p_ops);
    if (ret != OAL_SUCC) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P,
                       "{hmac_disable_p2p_pm::hmac_config_set_p2p_ps_ops disable p2p OppPS fail.}");
    }
}

/*
 * 功能描述  : 是否是P2P GO negotiation request action帧
 * 日    期  : 2015年4月22日
 * 修改内容  : 新生成函数
 */
oal_bool_enum_uint8 hmac_is_p2p_go_neg_req_frame(const uint8_t *data)
{
    uint8_t ret = ((data[MAC_ACTION_OFFSET_CATEGORY + MAC_80211_FRAME_LEN] == MAC_ACTION_CATEGORY_PUBLIC) &&
                   (data[P2P_PUB_ACT_OUI_OFF1 + MAC_80211_FRAME_LEN] == WFA_OUI_BYTE1) &&
                   (data[P2P_PUB_ACT_OUI_OFF2 + MAC_80211_FRAME_LEN] == WFA_OUI_BYTE2) &&
                   (data[P2P_PUB_ACT_OUI_OFF3 + MAC_80211_FRAME_LEN] == WFA_OUI_BYTE3) &&
                   (data[P2P_PUB_ACT_OUI_TYPE_OFF + MAC_80211_FRAME_LEN] == WFA_P2P_V1_0) &&
                   (data[P2P_PUB_ACT_OUI_SUBTYPE_OFF + MAC_80211_FRAME_LEN] == P2P_PAF_GON_REQ));

    if (ret != OAL_FALSE) {
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
}

/*
 * 功能描述  : 判断p2p 是否为该状态
 * 日    期  : 2015年5月20日
 * 修改内容  : 新生成函数
 */
uint8_t hmac_get_p2p_status(uint32_t p2p_status, hmac_cfgp2p_status_enum_uint32 status)
{
    if (p2p_status & BIT(status)) {
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
}

/*
 * 功能描述  : 设置p2p 为对应状态
 * 日    期  : 2015年5月20日
 * 修改内容  : 新生成函数
 */
void hmac_set_p2p_status(uint32_t *p2p_status, hmac_cfgp2p_status_enum_uint32 status)
{
    *p2p_status |= ((uint32_t)BIT(status));
}

/*
 * 功能描述  : 清除p2p 对应状态
 * 日    期  : 2015年5月20日
 * 修改内容  : 新生成函数
 */
void hmac_clr_p2p_status(uint32_t *p2p_status, hmac_cfgp2p_status_enum_uint32 status)
{
    *p2p_status &= ~((uint32_t)BIT(status));
}

/*
 * 功能描述  : sta要入网，go 160M应回退80M
 * 日    期  : 2019.2.9
 * 作    者  : wifi
 * 修改内容  : 新生成函数
 */
OAL_STATIC void hmac_p2p_go_back_to_80m(mac_vap_stru *go_mac_vap)
{
    wlan_bw_cap_enum_uint8 bw_cap;
    wlan_channel_bandwidth_enum_uint8 new_80bw;

    bw_cap = mac_vap_bw_mode_to_bw(go_mac_vap->st_channel.en_bandwidth);
    if (bw_cap == WLAN_BW_CAP_160M) {
        new_80bw = mac_regdomain_get_support_bw_mode(WLAN_BW_CAP_80M,
            go_mac_vap->st_channel.uc_chan_number);
        bw_cap = mac_vap_bw_mode_to_bw(new_80bw);
        if (bw_cap == WLAN_BW_CAP_80M) {
            oam_warning_log2(0, OAM_SF_QOS, "{hmac_p2p_go_back_to_80m::GO BW change from \
                [%d] to [%d]}", go_mac_vap->st_channel.en_bandwidth, new_80bw);
#ifdef _PRE_WLAN_FEATURE_DFS
            hmac_dfs_status_set(HMAC_BACK_80M);
#endif
            go_mac_vap->st_ch_switch_info.uc_ch_switch_cnt = 1;
            go_mac_vap->st_ch_switch_info.en_csa_mode      = WLAN_CSA_MODE_TX_ENABLE;
            hmac_chan_initiate_switch_to_new_channel(go_mac_vap,
                go_mac_vap->st_channel.uc_chan_number, new_80bw);
        }
    }
}

/*
 * 功能描述  : sta要入网，go 160M回退80M处理
 * 日    期  : 2019.2.9
 * 作    者  : wifi
 * 修改内容  : 新生成函数
 */
void hmac_p2p_sta_join_go_back_to_80m_handle(mac_vap_stru *sta_mac_vap)
{
#ifdef _PRE_WLAN_FEATURE_160M
    mac_vap_stru *go_mac_vap = NULL;
    if (!is_legacy_sta(sta_mac_vap)) {
        return;
    }
    if (mac_vap_go_can_not_in_160m_check(sta_mac_vap, sta_mac_vap->st_channel.uc_chan_number) == OAL_TRUE) {
        go_mac_vap = mac_vap_find_another_up_vap_by_mac_vap(sta_mac_vap);
        if (go_mac_vap != NULL && is_p2p_go(go_mac_vap)) {
            hmac_p2p_go_back_to_80m(go_mac_vap);
        }
    }
#endif
}

OAL_STATIC uint8_t mac_csa_check_can_run_dbdc(mac_vap_stru *other_vap, mac_vap_stru *p2p_vap, uint8_t new_band,
    wlan_channel_bandwidth_enum_uint8 new_bw)
{
    /* 2G ax VAP并且5G P2P协议<ax  bw<80M, 5G vap去辅路。不满足条件不允许启dbdc */
    if (mac_vap_is_work_he_protocol(other_vap) && other_vap->st_channel.en_band == WLAN_BAND_2G &&
        new_band == WLAN_BAND_5G && (new_bw > WLAN_BAND_WIDTH_40MINUS)) {
        oam_warning_log3(p2p_vap->uc_vap_id, OAM_SF_ANY,
            "{mac_csa_need_downgrade_protocol::sta_pro=[%d] p2p_pro=[%d] p2p_bw=[%d] not allow dbdc!}",
            other_vap->en_protocol, p2p_vap->en_protocol, new_bw);
        return OAL_FALSE;
    }

    /* 2G ax P2P并且5G VAP协议<ax  bw<80M, 5G vap去辅路。不满足条件不允许启dbdc */
    if (mac_vap_is_work_he_protocol(p2p_vap) && other_vap->st_channel.en_band == WLAN_BAND_5G &&
        new_band == WLAN_BAND_2G && (other_vap->st_channel.en_bandwidth > WLAN_BAND_WIDTH_40MINUS)) {
        oam_warning_log3(p2p_vap->uc_vap_id, OAM_SF_ANY,
            "{mac_csa_need_downgrade_protocol::sta_pro=[%d] p2p_pro=[%d] sta_bw=[%d] not allow dbdc!}",
            other_vap->en_protocol, p2p_vap->en_protocol, other_vap->st_channel.en_bandwidth);
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

/*
 * 功能描述  : go csa,判断切信道是否会需要切换协议，不支持csa的时候切换协议
 * 日    期  : 2021.2.7
 * 作    者  : wifi
 * 修改内容  : 新生成函数
 */
OAL_STATIC uint8_t mac_csa_check_down_protocol(mac_vap_stru *other_vap, mac_vap_stru *p2p_vap, uint8_t new_band,
    wlan_channel_bandwidth_enum_uint8 new_bw)
{
    /* vap+p2p共存时，且p2p要跨频段时，存在协议限制 */
    if (new_band != p2p_vap->st_channel.en_band) {
        /* vap和p2p都在11ax，有一个vap需要切到辅路没法在11ax，需要降协议 */
        if (mac_vap_is_work_he_protocol(other_vap) && mac_vap_is_work_he_protocol(p2p_vap) &&
            new_band != other_vap->st_channel.en_band) {
            oam_warning_log0(p2p_vap->uc_vap_id, OAM_SF_ANY,
                "{mac_csa_need_downgrade_protocol::sta and p2p protocol 11ax! can not run dbdc through csa}");
            return OAL_TRUE;
        }

        /* 不满足dbdc能力场景 一个在vap在2G ax，另一个在5G 40M以上带宽 */
        if (mac_csa_check_can_run_dbdc(other_vap, p2p_vap, new_band, new_bw) == OAL_FALSE) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}

uint8_t hmac_go_csa_check_down_protocol(mac_vap_stru **other_vap,
    mac_vap_stru *p2p_vap, mac_channel_stru *new_channel, uint32_t up_vap_num)
{
    uint8_t ret;
    uint8_t idx;

    /* mp13、mp15有辅路协议限制（辅路不支持ax），其他芯片无 */
    if (g_wlan_spec_cfg->feature_slave_ax_is_support == OAL_TRUE) {
        return OAL_FALSE;
    }

    for (idx = 0; idx < up_vap_num; idx++) {
        if (other_vap[idx]->uc_vap_id == p2p_vap->uc_vap_id) {
            continue;
        }
        ret = mac_csa_check_down_protocol(other_vap[idx], p2p_vap, new_channel->en_band, new_channel->en_bandwidth);
        if (ret == OAL_TRUE) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}

/*
 * 功能描述 : wal层下发csa参数没填带宽，按固定带宽策略
 */
OAL_STATIC wlan_channel_bandwidth_enum_uint8 mac_csa_get_go_bandwidth(mac_vap_stru *p2p_vap,
    mac_channel_stru *new_channel, wlan_bw_cap_enum_uint8 new_bw_cap)
{
    wlan_bw_cap_enum_uint8 bw_cap = mac_vap_bw_mode_to_bw(p2p_vap->st_channel.en_bandwidth);
    if (new_bw_cap >= WLAN_BW_CAP_BUTT) {  // csa参数未填带宽
        if (new_channel->en_band == p2p_vap->st_channel.en_band) {
            return mac_regdomain_get_bw_by_channel_bw_cap(new_channel->uc_chan_number, bw_cap); // 不跨band: GO带宽保持为原来值
        } else if (new_channel->en_band == WLAN_BAND_5G) {
            return mac_regdomain_get_bw_by_channel_bw_cap(new_channel->uc_chan_number, WLAN_BW_CAP_80M); // 5G带宽默认80M
        } else {
            return WLAN_BAND_WIDTH_20M;  // 2G带宽默认20M
        }
    } else if (new_channel->en_band != p2p_vap->st_channel.en_band) {
        return mac_regdomain_get_bw_by_channel_bw_cap(new_channel->uc_chan_number, new_bw_cap);
    } else {
        return mac_regdomain_get_bw_by_channel_bw_cap(new_channel->uc_chan_number, bw_cap);
    }
}

uint32_t hmac_p2p_csa_check_channel(mac_vap_stru **up_mac_vap,
    mac_vap_stru *p2p_vap, mac_channel_stru *new_channel, uint32_t up_vap_num)
{
    uint8_t check_band;
    uint8_t idx;

    /* 切到同信道不处理 */
    if (new_channel->uc_chan_number == p2p_vap->st_channel.uc_chan_number) {
        return OAL_FAIL;
    }

    /* csa扩展 支持跨频段的定制化，不支持则不允许跨频切信道 */
    check_band = (g_optimized_feature_switch_bitmap & BIT(CUSTOM_OPTIMIZE_CSA_EXT)) ?
        new_channel->en_band : p2p_vap->st_channel.en_band;
    if (mac_is_channel_num_valid(check_band, new_channel->uc_chan_number,
        p2p_vap->st_channel.ext6g_band) != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_p2p_change_go_channel::new_channel=%u }", new_channel->uc_chan_number);
        return OAL_FAIL;
    }

    /* 切到其他up vap所在的信道不做雷达信道检查 */
    for (idx = 0; idx < up_vap_num; idx++) {
        if (up_mac_vap[idx]->uc_vap_id != p2p_vap->uc_vap_id &&
            up_mac_vap[idx]->st_channel.uc_chan_number == new_channel->uc_chan_number) {
            return OAL_SUCC;
        }
    }

    /* 不切到雷达信道 */
    if (mac_is_dfs_channel(check_band, new_channel->uc_chan_number) == OAL_TRUE) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_p2p_change_go_channel::[%d] is dfs channel, not handle}",
            new_channel->uc_chan_number);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

uint32_t hmac_p2p_change_go_channel(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    mac_channel_stru new_channel = { 0 };
    wlan_bw_cap_enum_uint8 new_bw_cap;
    mac_device_stru *mac_device = NULL;
    mac_vap_stru *up_mac_vap[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {NULL};
    uint32_t up_vap_num;

    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_device == NULL || !is_p2p_go(mac_vap)) {
        return OAL_FAIL;
    }

    new_channel.uc_chan_number = param[0];
    new_bw_cap = param[1];

    oam_warning_log3(0, OAM_SF_ANY, "{hmac_p2p_change_go_channel::up_vap_nums=%d, new_channel=%d, new_bw_cap=%d}",
        mac_device->uc_vap_num, new_channel.uc_chan_number, new_bw_cap);

    up_vap_num = mac_device_find_up_vaps(mac_device, up_mac_vap, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    if (up_vap_num == 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_p2p_change_go_channel::mac_device_find_up_vaps fail.}");
        return OAL_FAIL;
    }

    new_channel.en_band = mac_get_band_by_channel_num(new_channel.uc_chan_number);
    new_channel.en_bandwidth = mac_csa_get_go_bandwidth(mac_vap, &new_channel, new_bw_cap);
    if (mac_get_channel_idx_from_num(new_channel.en_band, new_channel.uc_chan_number,
        OAL_FALSE, &(new_channel.uc_chan_idx)) != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* 信道检查 */
    if (hmac_p2p_csa_check_channel(up_mac_vap, mac_vap, &new_channel, up_vap_num) != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* 识别原状态和目标状态，避免去dbac和非法状态 */
    if (hmac_chan_switch_get_state_trans(up_mac_vap, mac_vap, &new_channel, up_vap_num,
        MAC_MVAP_CHAN_SWITCH_GO_CSA) == MAC_MVAP_STATE_TRANS_UNSUPPORTED) {
        return OAL_FAIL;
    }

    /* 需要降协议，无法通过csa切换协议 */
    if (hmac_go_csa_check_down_protocol(up_mac_vap, mac_vap, &new_channel, up_vap_num) == OAL_TRUE) {
        return OAL_FAIL;
    }

    oam_warning_log4(0, OAM_SF_CSA, "{hmac_p2p_change_go_channel::go channel from [%d][%d] to [%d][%d]}",
        mac_vap->st_channel.uc_chan_number, mac_vap->st_channel.en_bandwidth,
        new_channel.uc_chan_number, new_channel.en_bandwidth);
    mac_vap->st_ch_switch_info.uc_ch_switch_cnt = HMAC_P2P_GO_VENDOR_CMD_CSA_COUNT;
    mac_vap->st_ch_switch_info.en_csa_mode      = WLAN_CSA_MODE_TX_ENABLE;
    hmac_chan_initiate_switch_to_new_channel(mac_vap, new_channel.uc_chan_number, new_channel.en_bandwidth);

    return OAL_SUCC;
}

mac_p2p_scenes_enum_uint8 hmac_p2p_get_scenes(mac_vap_stru *mac_vap)
{
    mac_device_stru *mac_device = NULL;
    mac_p2p_scenes_enum_uint8 value;

    if (mac_vap == NULL || is_legacy_vap(mac_vap)) {
        return AC_P2P_SCENES_BUTT;
    }

    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_device == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_P2P, "hmac_p2p_get_scenes:: mac_device is NULL");
        return AC_P2P_SCENES_BUTT;
    }

    value = mac_device->st_p2p_info.p2p_scenes;
    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "hmac_p2p_get_scenes:: p2p scene[%d].", value);
    return value;
}

void hmac_vap_join_avoid_dbac_back_to_11n_handle(mac_vap_stru *mac_vap, mac_bss_dscr_stru *bss_dscr)
{
    if (mac_vap_need_proto_downgrade(mac_vap) == OAL_FALSE ||
        bss_dscr->en_p2p_scenes != MAC_P2P_SCENES_LOW_LATENCY) {
        return;
    }

    if (mac_vap_avoid_dbac_close_vht_protocol(mac_vap) == OAL_TRUE) {
        bss_dscr->en_vht_capable = OAL_FALSE;
        mac_mib_set_VHTOptionImplemented(mac_vap, OAL_FALSE);
    } else {
        mac_mib_set_VHTOptionImplemented(mac_vap, OAL_TRUE);
    }
}

/*
 * 函 数 名  : hmac_config_cancel_remain_on_channel
 * 功能描述  : 停止保持在指定信道
 * 1.日    期  : 2014年11月22日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_p2p_cancel_remain_on_channel(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
            "hmac_config_cancel_remain_on_channel::mac_res_get_hmac_vap fail.vap_id = %u", pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN) {
        hmac_p2p_listen_timeout(pst_hmac_vap, pst_mac_vap);
    } else {
        hmac_p2p_send_listen_expired_to_host(pst_hmac_vap);
    }
    return OAL_SUCC;
}

OAL_STATIC oal_bool_enum_uint8 hmac_p2p_check_is_dispatch_scan_req(hmac_vap_stru *hmac_vap,
    hmac_device_stru *hmac_device, wlan_ieee80211_roc_type_uint8 roc_type)
{
    oal_bool_enum_uint8 is_high_prio_scan =
        ((roc_type == IEEE80211_ROC_TYPE_HIGH_PRIO_MGMT_TX) || (roc_type == IEEE80211_ROC_TYPE_HIGH_PRIO_MGMT_RX));

    if (hmac_scan_check_is_dispatch_scan_req(hmac_vap, hmac_device, is_high_prio_scan) != OAL_SUCC) {
        return OAL_FALSE;
    }
    return OAL_TRUE;
}
/*
 * 函 数 名  : hmac_config_remain_on_channel
 * 功能描述  : 保持在指定信道
 * 1.日    期  : 2014年11月22日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_p2p_set_remain_on_channel(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_remain_on_channel_param_stru *remain_on_channel = NULL;
    mac_device_stru *mac_device = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_device_stru *hmac_device = NULL;
    uint32_t ret;

    /* 1.1 判断入参 */
    if (oal_any_null_ptr2(mac_vap, puc_param)) {
        oam_error_log0(0, OAM_SF_P2P, "{hmac_config_remain_on_channel::mac_vap or puc_param null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    remain_on_channel = (hmac_remain_on_channel_param_stru *)puc_param;
    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (oal_unlikely(mac_device == NULL)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_config_remain_on_channel::mac_device[%d] null!}", mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* mac_device已经判空，此时hmac_device无需重复判空，可以直接使用 */
    hmac_device = hmac_res_get_mac_dev(mac_vap->uc_device_id);

    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_P2P, "{hmac_config_remain_on_channel::mac_res_get_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 1.2 检查是否能进入监听状态 */
    if (hmac_p2p_check_is_dispatch_scan_req(hmac_vap, hmac_device, remain_on_channel->en_roc_type) == OAL_FALSE) {
        /* 不能进入监听状态，返回设备忙 */
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_P2P, "{hmac_config_remain_on_channel::dispatch fail}");
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    /* 1.3 获取home 信道和信道类型。如果返回主信道为0，表示没有设备处于up 状态，监听后不需要返回主信道 */
    /* 保存内核下发的监听信道信息，用于监听超时或取消监听时返回 */
    mac_device->st_p2p_info.st_listen_channel = remain_on_channel->st_listen_channel;

    /* 由于p2p0和 p2p cl 共用一个VAP 结构，故在进入监听时，需要保存之前的状态，便于监听结束时返回 */
    /*  wlan0 也可能进入监听模式 */
    if (mac_vap->en_p2p_mode != WLAN_LEGACY_VAP_MODE) {
        if (mac_vap->en_vap_state != MAC_VAP_STATE_STA_LISTEN) {
            mac_device->st_p2p_info.en_last_vap_state = mac_vap->en_vap_state;
        }
        remain_on_channel->en_last_vap_state = mac_device->st_p2p_info.en_last_vap_state;
    } else {
        if (mac_vap->en_vap_state != MAC_VAP_STATE_STA_LISTEN) {
            mac_vap->en_last_vap_state = mac_vap->en_vap_state;
        }
        remain_on_channel->en_last_vap_state = mac_vap->en_last_vap_state;
    }
    oam_info_log3(mac_vap->uc_vap_id, OAM_SF_P2P, "{hmac_config_remain_on_channel:: \
        listen_channel=%d, current_channel=%d, last_state=%d}", remain_on_channel->uc_listen_channel,
        mac_vap->st_channel.uc_chan_number, remain_on_channel->en_last_vap_state);

    /* 状态机调用:  hmac_p2p_remain_on_channel */
    ret = hmac_fsm_call_func_sta(hmac_vap, HMAC_FSM_INPUT_LISTEN_REQ, (void *)(remain_on_channel));
    if (ret != OAL_SUCC) {
        /* DMAC 设置切换信道失败 */
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_P2P, "{hmac_config_remain_on_channel::fail, ret=%d}", ret);
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    oam_info_log3(mac_vap->uc_vap_id, OAM_SF_P2P, "{hmac_config_remain_on_channel::channel=%d, duration=%d, band=%d}",
        remain_on_channel->uc_listen_channel, remain_on_channel->listen_duration, remain_on_channel->en_band);
    return OAL_SUCC;
}
