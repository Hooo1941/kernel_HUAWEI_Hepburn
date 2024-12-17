/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 配置相关实现hmac接口实现源文件
 * 作    者 :
 * 创建日期 : 2013年1月8日
 */

/* 1 头文件包含 */
#include "oam_ext_if.h"
#include "oal_schedule.h"

#include "hmac_resource.h"
#include "hmac_vap.h"
#include "hmac_config.h"

#include "securec.h"
#include "securectype.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CONFIG_QUERY_C

/*
 * 函 数 名  : hmac_config_query_sta_mngpkt_sendstat_rsp
 * 功能描述  : 响应查询sta关联时候管理帧发送状态 dmac上报的结果
 * 1.日    期  : 2018年8月5日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_query_sta_mngpkt_sendstat_rsp(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_device_stru *pst_mac_dev = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    mac_cfg_query_mngpkt_sendstat_stru *pst_param;
    pst_param = (mac_cfg_query_mngpkt_sendstat_stru *)puc_param;

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_dev == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_query_sta_mngpkt_sendstat_rsp::mac_res_get_dev failed.}");
        return OAL_FAIL;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "hmac_config_query_sta_mngpkt_sendstat_rsp: pst_hmac_vap is null ptr.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_dev->uc_auth_req_sendst = pst_param->uc_auth_req_st;
    pst_mac_dev->uc_asoc_req_sendst = pst_param->uc_asoc_req_st;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_query_rssi_rsp
 * 功能描述  : 响应查询rssi dmac上报的结果
 * 1.日    期  : 2016年4月11日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_config_query_rssi_rsp(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_cfg_query_rssi_stru *pst_param = NULL;
    hmac_user_stru *pst_hmac_user = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;

    pst_param = (mac_cfg_query_rssi_stru *)puc_param;

    pst_hmac_user = mac_res_get_hmac_user(pst_param->us_user_id);
    if (pst_hmac_user == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "hmac_config_query_rssi_rsp: pst_hmac_user is null ptr. user id:%d", pst_param->us_user_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_query_rssi_rsp: pst_hmac_vap is null ptr.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_user->c_rssi = pst_param->c_rssi;
    pst_hmac_user->c_free_power = pst_param->c_free_power;

    pst_hmac_vap->station_info_query_completed_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));
    return OAL_SUCC;
}

uint32_t hmac_config_query_tid_queue_rsp(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    mac_cfg_query_tid_queue_stru *tid_queue_info = NULL;
    hmac_user_stru *hmac_user = NULL;
    hmac_vap_stru *hmac_vap = NULL;

    tid_queue_info = (mac_cfg_query_tid_queue_stru *)param;

    hmac_user = mac_res_get_hmac_user(tid_queue_info->user_id);
    if (hmac_user == NULL) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                       "hmac_config_query_tid_queue_rsp: hmac_user is null ptr. user id:%d", tid_queue_info->user_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_query_tid_queue_rsp: hmac_vap is null ptr.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_query_tid_queue_rsp:user[%d] total_mpdu_num[%d]",
        tid_queue_info->user_id, tid_queue_info->total_mpdu_num);

    hmac_vap->station_info_query_completed_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(hmac_vap->query_wait_q));
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_DELAY_STATISTIC
/*
 * 函 数 名  : hmac_config_receive_sta_delay
 * 功能描述  : 响应dmac  上报的时延统计结果
 * 1.日    期  : 2018年9月12日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_config_receive_sta_delay(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    user_delay_info_stru *pst_param;
    char *type_name[] = { "tid_sta_delay", "air_sta_delay" };
    pst_param = (user_delay_info_stru *)puc_param;
#ifndef _PRE_LINUX_TEST
    printk(KERN_INFO "%s, 0: %d, 1~5 ms: %d, 5~10 ms: %d, 10~15 ms: %d, 15~20 ms: %d, 20~25 ms: %d, \
           25~30 ms: %d, 30~35 ms: %d, 35~40 ms: %d, >=40 ms: %d",
           type_name[pst_param->dl_measure_type % 2],
           pst_param->dl_time_array[BYTE_OFFSET_0], pst_param->dl_time_array[BYTE_OFFSET_1],
           pst_param->dl_time_array[BYTE_OFFSET_2], pst_param->dl_time_array[BYTE_OFFSET_3],
           pst_param->dl_time_array[BYTE_OFFSET_4], pst_param->dl_time_array[BYTE_OFFSET_5],
           pst_param->dl_time_array[BYTE_OFFSET_6], pst_param->dl_time_array[BYTE_OFFSET_7],
           pst_param->dl_time_array[BYTE_OFFSET_8], pst_param->dl_time_array[BYTE_OFFSET_9]);
#endif
    return OAL_SUCC;
}
#endif /* _PRE_WLAN_DELAY_STATISTIC */
/*
 * 函 数 名  : hmac_config_query_psst_rsp
 * 功能描述  : 响应查询rssi dmac上报的结果
 * 1.日    期  : 2016年4月11日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_config_query_psst_rsp(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_cfg_query_psst_stru *pst_param = NULL;
    hmac_user_stru *pst_hmac_user = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;

    pst_param = (mac_cfg_query_psst_stru *)puc_param;

    pst_hmac_user = mac_res_get_hmac_user(pst_param->us_user_id);
    if (pst_hmac_user == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "hmac_config_query_psst_rsp: pst_hmac_user is null ptr. user id:%d", pst_param->us_user_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_query_psst_rsp: pst_hmac_vap is null ptr.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_user->uc_ps_st = pst_param->uc_ps_st;

    pst_hmac_vap->station_info_query_completed_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_query_rate_rsp
 * 功能描述  : 响应查询rate dmac上报的结果
 * 1.日    期  : 2016年4月11日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_config_query_rate_rsp(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_cfg_query_rate_stru *pst_param = NULL;
    hmac_user_stru *pst_hmac_user = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;

    pst_param = (mac_cfg_query_rate_stru *)puc_param;
    pst_hmac_user = mac_res_get_hmac_user(pst_param->us_user_id);
    if (pst_hmac_user == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "hmac_config_query_rate_rsp: pst_hmac_user is null ptr. user id:%d", pst_param->us_user_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_query_rate_rsp: pst_hmac_vap is null ptr.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_user->tx_rate = pst_param->tx_rate;
    pst_hmac_user->tx_rate_min = pst_param->tx_rate_min;
    pst_hmac_user->tx_rate_max = pst_param->tx_rate_max;
    pst_hmac_user->rx_rate = pst_param->rx_rate;
    pst_hmac_user->rx_rate_min = pst_param->rx_rate_min;
    pst_hmac_user->rx_rate_max = pst_param->rx_rate_max;
#ifdef _PRE_WLAN_DFT_STAT
    pst_hmac_user->uc_cur_per = pst_param->uc_cur_per;
    pst_hmac_user->uc_bestrate_per = pst_param->uc_bestrate_per;
#endif

    pst_hmac_vap->station_info_query_completed_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));

    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_config_psm_query_rsp_proc(hmac_psm_flt_stat_query_stru *pst_hmac_psm_query,
                                                   mac_psm_query_msg *pst_query_msg)
{
    mac_psm_query_stat_stru  *pst_psm_stat = &pst_hmac_psm_query->ast_psm_stat[pst_query_msg->en_query_type];

    pst_psm_stat->query_item = pst_query_msg->st_stat.query_item;
    if (memcpy_s(pst_psm_stat->aul_val, sizeof(pst_psm_stat->aul_val), pst_query_msg->st_stat.aul_val,
        (pst_psm_stat->query_item + 1) * sizeof(uint32_t)) != EOK) {
        oam_error_log2(0, OAM_SF_CFG, "{hmac_config_psm_query_rsp_proc::memcpy fail, query type[%d] query_item[%d]}",
                       pst_query_msg->en_query_type, pst_query_msg->st_stat.query_item);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_query_psm_rsp
 * 1.日    期  : 2019年3月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_query_psm_rsp(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    hmac_device_stru        *pst_hmac_device = NULL;
    hmac_psm_flt_stat_query_stru *pst_hmac_psm_query = NULL;
    mac_psm_query_msg       *pst_query_rsp_msg = NULL;
    uint32_t query_item = 0;

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "hmac_config_query_psm_flt_stat: pst_hmac_device is null ptr. device id:%d",
            pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_hmac_psm_query = &pst_hmac_device->st_psm_flt_stat_query;
    pst_query_rsp_msg = (mac_psm_query_msg*)puc_param;

    if (OAL_SUCC == hmac_config_psm_query_rsp_proc(pst_hmac_psm_query, pst_query_rsp_msg)) {
#ifdef _PRE_WLAN_FEATURE_PSM_ABN_PKTS_STAT
        if (pst_query_rsp_msg->en_query_type == MAC_PSM_QUERY_ABNORMAL_PKTS_CNT) {
            query_item = pst_query_rsp_msg->st_stat.query_item;
            pst_hmac_psm_query->ast_psm_stat[pst_query_rsp_msg->en_query_type].aul_val[query_item] =
                pst_hmac_device->addba_req_cnt;
            pst_hmac_psm_query->ast_psm_stat[pst_query_rsp_msg->en_query_type].aul_val[query_item + 1] =
                pst_hmac_device->group_rekey_cnt;
            /* addba req count和 group rekey count 2个item */
            pst_hmac_psm_query->ast_psm_stat[pst_query_rsp_msg->en_query_type].query_item += 2;
            oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_query_psm_rsp::addba_req_cnt[%d], group_rekey_cnt[%d]}",
                             pst_hmac_device->addba_req_cnt, pst_hmac_device->group_rekey_cnt);
        }
#endif

        /* 设置wait条件为true */
        pst_hmac_psm_query->auc_complete_flag[pst_query_rsp_msg->en_query_type] = OAL_TRUE;
        oal_wait_queue_wake_up_interrupt(&(pst_hmac_device->st_psm_flt_stat_query.st_wait_queue));
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_DFT_STAT
/*
 * 函 数 名  : hmac_config_query_ani_rsp
 * 功能描述  : 响应查询ani dmac上报的结果
 * 1.日    期  : 2016年4月15日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_config_query_ani_rsp(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_cfg_query_ani_stru *pst_param;
    hmac_vap_stru *pst_hmac_vap;

    pst_param = (mac_cfg_query_ani_stru *)puc_param;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_query_ani_rsp: pst_hmac_vap is null ptr.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap->uc_device_distance = pst_param->uc_device_distance;
    pst_hmac_vap->uc_intf_state_cca = pst_param->uc_intf_state_cca;
    pst_hmac_vap->uc_intf_state_co = pst_param->uc_intf_state_co;

    pst_hmac_vap->station_info_query_completed_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));

    return OAL_SUCC;
}
#endif
/*
 * 函 数 名  : hmac_config_query_station_info
 * 功能描述  : host侧查询station info
 * 1.日    期  : 2014年11月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_query_station_info(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    dmac_query_station_info_request_event  *query_requset_event = (dmac_query_station_info_request_event *)puc_param;
    mac_user_stru *mac_user = mac_vap_get_user_by_addr(mac_vap, query_requset_event->auc_query_sta_addr);
    if (mac_user == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_query_station_info::user free, stop query.}");
        return OAL_FAIL;
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_QUERY_STATION_STATS, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_phy_stat_info::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

uint32_t hmac_config_query_tid_queue(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_send_event(pst_mac_vap, WLAN_CFGID_QUERY_TID_QUEUE, us_len, puc_param);
}

/*
 * 函 数 名  : hmac_config_query_rssi
 * 功能描述  : 查询用户RSSI
 * 1.日    期  : 2016年4月11日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_config_query_rssi(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_QUERY_RSSI, us_len, puc_param);

    return ret;
}

/*
 * 函 数 名  : hmac_config_query_psst
 * 功能描述  : 查询用户省功耗状态
 * 1.日    期  : 2016年4月11日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_config_query_psst(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_QUERY_PSST, us_len, puc_param);

    return ret;
}

/*
 * 函 数 名  : hmac_config_query_rate
 * 功能描述  : 查询dmac user tx rx phy rate
 * 1.日    期  : 2016年4月11日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_config_query_rate(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_QUERY_RATE, us_len, puc_param);

    return ret;
}

#ifdef _PRE_WLAN_DFT_STAT
/*
 * 函 数 名  : hmac_config_query_ani
 * 功能描述  : 查询dmac vap 抗干扰信息
 * 1.日    期  : 2016年4月15日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_config_query_ani(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_QUERY_ANI, us_len, puc_param);

    return ret;
}
#endif

/*
 * 函 数 名  : hmac_config_query_rssi
 * 功能描述  : 查询低功耗帧过滤统计
 * 1.日    期  : 2019年3月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_query_psm_flt_stat(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_QUERY_PSM_STAT, us_len, puc_param);

    return ret;
}
