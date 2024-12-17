/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : hmac_chan_mgmt.c
 * 作    者 :
 * 创建日期 : 2014年2月22日
 */
/* 1 头文件包含 */
#include "mac_ie.h"
#include "mac_regdomain.h"
#include "mac_device.h"
#include "wlan_types.h"
#include "hmac_mgmt_sta.h"
#include "hmac_sme_sta.h"
#include "hmac_fsm.h"
#include "hmac_dfs.h"
#include "hmac_chan_mgmt.h"
#include "mac_device.h"
#include "hmac_scan.h"
#include "frw_ext_if.h"
#include "hmac_resource.h"
#include "securec.h"
#include "mac_mib.h"
#include "wlan_chip_i.h"
#ifdef _PRE_WLAN_FEATURE_HID2D
#include "hmac_hid2d.h"
#endif
#ifdef _PRE_WLAN_CHBA_MGMT
#include "hmac_chba_mgmt.h"
#include "hmac_chba_coex.h"
#include "hmac_chba_function.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CHAN_MGMT_C

/* 2 全局变量定义 */
#define HMAC_CENTER_FREQ_2G_40M_OFFSET 2 /* 中心频点相对于主信道idx的偏移量 */
#define HMAC_AFFECTED_CH_IDX_OFFSET    5 /* 2.4GHz下，40MHz带宽所影响的信道半径，中心频点 +/- 5个信道 */

/* 信道切换状态表 */
/************************************************************************************************************
     MAC_MVAP_STATE_BUTT : 0 : DCHN | 1 :DBDC | 2 : DBAC | 3 : DBDC+DBAC | 4 :SINGLE_VAP | 5 : UNSUPPORTED
***********************************************************************************************************/
/* 切信道状态切换表 */
static mac_mvap_state_trans_enum g_chan_switch_state_trans_table[MAC_MVAP_STATE_BUTT][MAC_MVAP_STATE_BUTT] = {
    [MAC_MVAP_STATE_DCHN] = {
        MAC_MVAP_STATE_TRANS_DCHN_TO_DCHN, MAC_MVAP_STATE_TRANS_DCHN_TO_DBDC,
        MAC_MVAP_STATE_TRANS_DCHN_TO_DBAC, MAC_MVAP_STATE_TRANS_DCHN_TO_DBDC_AC,
        MAC_MVAP_STATE_TRANS_DCHN_TO_SINGLE_VAP, MAC_MVAP_STATE_TRANS_UNSUPPORTED
    }, /* 同频同信道 -> 其他状态 */
    [MAC_MVAP_STATE_DBDC] = {
        MAC_MVAP_STATE_TRANS_DBDC_TO_DCHN, MAC_MVAP_STATE_TRANS_DBDC_TO_DBDC,
        MAC_MVAP_STATE_TRANS_DBDC_TO_DBAC, MAC_MVAP_STATE_TRANS_DBDC_TO_DBDC_AC,
        MAC_MVAP_STATE_TRANS_DBDC_TO_SINGLE_VAP, MAC_MVAP_STATE_TRANS_UNSUPPORTED
    }, /* dbdc -> 其他状态 */
    [MAC_MVAP_STATE_DBAC] = {
        MAC_MVAP_STATE_TRANS_DBAC_TO_DCHN, MAC_MVAP_STATE_TRANS_DBAC_TO_DBDC,
        MAC_MVAP_STATE_TRANS_DBAC_TO_DBAC, MAC_MVAP_STATE_TRANS_DBAC_TO_DBDC_AC,
        MAC_MVAP_STATE_TRANS_DBAC_TO_SINGLE_VAP, MAC_MVAP_STATE_TRANS_UNSUPPORTED
    }, /* dbac -> 其他状态 */
    [MAC_MVAP_STATE_DBDC_DBAC] = {
        MAC_MVAP_STATE_TRANS_DBDC_AC_TO_DCHN, MAC_MVAP_STATE_TRANS_DBDC_AC_TO_DBDC,
        MAC_MVAP_STATE_TRANS_DBDC_AC_TO_DBAC, MAC_MVAP_STATE_TRANS_DBDC_AC_TO_DBDC_AC,
        MAC_MVAP_STATE_TRANS_DBDC_AC_TO_SINGLE_VAP, MAC_MVAP_STATE_TRANS_UNSUPPORTED
    }, /* dbdc_dbac -> 其他状态 */
    [MAC_MVAP_STATE_SINGLE_VAP] = {
        MAC_MVAP_STATE_TRANS_SINGLE_VAP_TO_DCHN, MAC_MVAP_STATE_TRANS_SINGLE_VAP_TO_DBDC,
        MAC_MVAP_STATE_TRANS_SINGLE_VAP_TO_DBAC, MAC_MVAP_STATE_TRANS_SINGLE_VAP_TO_DBDC_AC,
        MAC_MVAP_STATE_TRANS_SINGLE_VAP_TO_SINGLE_VAP, MAC_MVAP_STATE_TRANS_UNSUPPORTED
    }, /* 单vap状态 -> 单vap状态 */
    [MAC_MVAP_STATE_UNSUPPORTED] = {
        MAC_MVAP_STATE_TRANS_UNSUPPORTED, MAC_MVAP_STATE_TRANS_UNSUPPORTED,
        MAC_MVAP_STATE_TRANS_UNSUPPORTED, MAC_MVAP_STATE_TRANS_UNSUPPORTED,
        MAC_MVAP_STATE_TRANS_UNSUPPORTED, MAC_MVAP_STATE_TRANS_UNSUPPORTED
    }, /* 不支持的状态 -> 其他状态 */
};

/* 3 函数声明 */
/* 4 函数实现 */
static void hmac_ap_chan_switch_start_send_cali_data(mac_vap_stru *mac_vap, uint8_t channel_num,
    wlan_channel_bandwidth_enum_uint8 bandwidth)
{
    wlan_channel_band_enum_uint8 new_band;
    cali_data_req_stru cali_data_req_info;

    /* 支持跨band切换信道，ap启动CSA需要传入新的band */
    new_band = mac_get_band_by_channel_num(channel_num);

    /* 下发CSA 新信道的校准数据 */
    cali_data_req_info.mac_vap_id = mac_vap->uc_vap_id;
    cali_data_req_info.channel.en_band = new_band;
    cali_data_req_info.channel.en_bandwidth = bandwidth;
    cali_data_req_info.channel.uc_chan_number = channel_num;
    cali_data_req_info.work_cali_data_sub_type =  CALI_DATA_CSA_TYPE;

    wlan_chip_update_cur_chn_cali_data(&cali_data_req_info);
}
/*
 * 函 数 名  : hmac_chan_initiate_switch_to_new_channel
 * 功能描述  : 设置VAP信道参数，准备切换至新信道运行
 * 1.日    期  : 2014年2月22日
 *   修改内容  : 新生成函数
 */
void hmac_chan_initiate_switch_to_new_channel(mac_vap_stru *pst_mac_vap, uint8_t uc_channel,
    wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    frw_event_mem_stru *pst_event_mem = NULL;
    frw_event_stru *pst_event = NULL;
    uint32_t ret;
    dmac_set_ch_switch_info_stru *pst_ch_switch_info = NULL;
    mac_device_stru *pst_mac_device = NULL;

    /* AP准备切换信道 */
    pst_mac_vap->st_ch_switch_info.en_ch_switch_status = WLAN_CH_SWITCH_STATUS_1;
    pst_mac_vap->st_ch_switch_info.uc_announced_channel = uc_channel;
    pst_mac_vap->st_ch_switch_info.en_announced_bandwidth = en_bandwidth;

    /* 在Beacon帧中添加Channel Switch Announcement IE */
    pst_mac_vap->st_ch_switch_info.en_csa_present_in_bcn = OAL_TRUE;

    hmac_ap_chan_switch_start_send_cali_data(pst_mac_vap, uc_channel, en_bandwidth);

    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_2040,
        "{hmac_chan_initiate_switch_to_new_channel::uc_announced_channel=%d,en_announced_bandwidth=%d csa_cnt=%d}",
        uc_channel, en_bandwidth, pst_mac_vap->st_ch_switch_info.uc_ch_switch_cnt);

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                       "{hmac_chan_initiate_switch_to_new_channel::pst_mac_device null.}");
        return;
    }
    /* 申请事件内存 */
    pst_event_mem = frw_event_alloc_m(sizeof(dmac_set_ch_switch_info_stru));
    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                       "{hmac_chan_initiate_switch_to_new_channel::pst_event_mem null.}");
        return;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX, DMAC_WLAN_CTX_EVENT_SUB_TYPE_SWITCH_TO_NEW_CHAN,
                       sizeof(dmac_set_ch_switch_info_stru), FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    /* 填写事件payload */
    pst_ch_switch_info = (dmac_set_ch_switch_info_stru *)pst_event->auc_event_data;
    pst_ch_switch_info->en_ch_switch_status = WLAN_CH_SWITCH_STATUS_1;
    pst_ch_switch_info->uc_announced_channel = uc_channel;
    pst_ch_switch_info->en_announced_bandwidth = en_bandwidth;
    pst_ch_switch_info->uc_ch_switch_cnt = pst_mac_vap->st_ch_switch_info.uc_ch_switch_cnt;
    pst_ch_switch_info->en_csa_present_in_bcn = OAL_TRUE;
    pst_ch_switch_info->uc_csa_vap_cnt = pst_mac_device->uc_csa_vap_cnt;

    pst_ch_switch_info->en_csa_mode = pst_mac_vap->st_ch_switch_info.en_csa_mode;

    /* 分发事件 */
    ret = frw_event_dispatch_event(pst_event_mem);
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                       "{hmac_chan_initiate_switch_to_new_channel::frw_event_dispatch_event failed[%d].}", ret);
        frw_event_free_m(pst_event_mem);
        return;
    }

    /* 释放事件 */
    frw_event_free_m(pst_event_mem);
}

/* mp16需要识别是否为特殊DBDC */
oal_bool_enum_uint8 hmac_jugde_dual_5g_dbdc(const mac_channel_stru *chan_info,
    const mac_channel_stru *other_vap_chan_info)
{
    uint8_t vap_subband;
    uint8_t other_vap_subband;
    mac_special_dbdc_subband_enum vap_subband_type;
    mac_special_dbdc_subband_enum other_vap_subband_type;
    uint8_t special_dbdc_type = wlan_chip_get_special_dbdc_cap();
    uint8_t vap_work_in_master_map[SPECIAL_DBDC_SUBBAND_BUTT][SPECIAL_DBDC_SUBBAND_BUTT] = {
        { 0, SPECIAL_DBDC_TYPE_5G_LOW_5G_HIGH, 0, 0 },
        { SPECIAL_DBDC_TYPE_5G_HIGH_5G_LOW, 0, SPECIAL_DBDC_TYPE_5G_HIGH_6G_LOW, 0 },
        { 0, SPECIAL_DBDC_TYPE_6G_LOW_5G_HIGH, 0, SPECIAL_DBDC_TYPE_6G_LOW_6G_HIGH },
        { 0, 0, SPECIAL_DBDC_TYPE_6G_HIGH_6G_LOW, 0 }
    };
    uint8_t other_vap_work_in_master_map[SPECIAL_DBDC_SUBBAND_BUTT][SPECIAL_DBDC_SUBBAND_BUTT] = {
        { 0, SPECIAL_DBDC_TYPE_5G_HIGH_5G_LOW, 0, 0 },
        { SPECIAL_DBDC_TYPE_5G_LOW_5G_HIGH, 0, SPECIAL_DBDC_TYPE_6G_LOW_5G_HIGH, 0 },
        { 0, SPECIAL_DBDC_TYPE_5G_HIGH_6G_LOW, 0, SPECIAL_DBDC_TYPE_6G_HIGH_6G_LOW },
        { 0, 0, SPECIAL_DBDC_TYPE_6G_LOW_6G_HIGH, 0 }
    };
    /* 定制化没开特殊dbdc */
    if (special_dbdc_type == SPECIAL_DBDC_TYPE_NOT_SUPPORT) {
        return OAL_FALSE;
    }

    /* 根据天线方案判断是否支持双5G DBDC */
    if (wlan_chip_is_support_dual_5g_dbdc_by_radio_cap() != OAL_TRUE) {
        return OAL_FALSE;
    }

    /* 6G高低band当前还没有定义,暂时只判断5G高低band,待后续补充 */
    wlan_chip_get_rf_band_from_center_freq(wlan_chip_get_center_freq_from_chn(chan_info), &vap_subband);
    wlan_chip_get_rf_band_from_center_freq(wlan_chip_get_center_freq_from_chn(other_vap_chan_info),
        &other_vap_subband);

    vap_subband_type = (vap_subband <= WLAN_5G_SUB_BAND4) ? SPECIAL_DBDC_SUBBAND_5G_LOW : SPECIAL_DBDC_SUBBAND_5G_HIGH;
    other_vap_subband_type =
        (other_vap_subband <= WLAN_5G_SUB_BAND4) ? SPECIAL_DBDC_SUBBAND_5G_LOW : SPECIAL_DBDC_SUBBAND_5G_HIGH;
    oam_warning_log3(0, OAM_SF_DBDC,
        "hmac_jugde_special_dbdc:vap_subband_type = %d, others_vap_subband_type = %d, special_dbdc_type = %d",
        vap_subband_type, other_vap_subband_type, special_dbdc_type);

    if (vap_work_in_master_map[vap_subband_type][other_vap_subband_type] == special_dbdc_type) {
        return OAL_TRUE;
    } else if (other_vap_work_in_master_map[vap_subband_type][other_vap_subband_type] == special_dbdc_type) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

/*
 * 功能描述 : 判断2G、5G通道能力是否对等。OAL_TRUE:对等, OAL_FALSE:不对等。
 */
static oal_bool_enum_uint8 hmac_custom_2g_chain_is_equal_5g_chain(void)
{
    uint8_t chain_2g, chain_5g;
    uint8_t radio_cap = wlan_chip_get_chn_radio_cap();

    chain_2g = radio_cap & WLAN_PHY_CHAIN_QUAD;
    chain_5g = radio_cap >> WLAN_CHAIN_BAND_INDICATOR;

    return ((chain_2g == chain_5g) ? OAL_TRUE : OAL_FALSE);
}

/*
 * 功能描述 : 检查信道组合是否符合单辅路dbdc。定制化开启&&2G5G通道能力不对等
 */
oal_bool_enum_uint8 hmac_check_is_support_single_dbdc(void)
{
    return (oal_bool_enum_uint8)((wlan_chip_is_single_dbdc_ini_en() != OAL_FALSE) &&
                                 hmac_custom_2g_chain_is_equal_5g_chain() != OAL_TRUE);
}

/*
 * 功能描述 : 检查信道组合是否符合特殊dbdc
 */
oal_bool_enum_uint8 hmac_check_is_support_dual_5g_dbdc(mac_vap_stru **up_mac_vap, mac_vap_stru *mac_vap,
    mac_channel_stru *new_channel, uint32_t up_vap_num)
{
    uint8_t idx;
    for (idx = 0; idx < up_vap_num; idx++) {
        /* 新信道跟其他不同信道的vap判断是否符合特殊dbdc */
        if (mac_vap->uc_vap_id != up_mac_vap[idx]->uc_vap_id &&
            new_channel->uc_chan_number != up_mac_vap[idx]->st_channel.uc_chan_number) {
            return hmac_jugde_dual_5g_dbdc(new_channel, &up_mac_vap[idx]->st_channel);
        }
    }

    return OAL_FALSE;
}

/*
 * 功能描述 : 入网、漫游、信道切换时检查是否会导致组成不支持的p2p dbac
 */
static oal_bool_enum_uint8 hmac_check_channel_is_unsupported_p2p_dbac(mac_vap_stru **up_mac_vap, mac_vap_stru *mac_vap,
    mac_channel_stru *new_channel, uint32_t up_vap_num)
{
    uint8_t idx;
    uint8_t p2p_vap_num = 0;
    mac_vap_stru *p2p_vap[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {NULL};

    for (idx = 0; idx < up_vap_num; idx++) {
        if (!is_legacy_vap(up_mac_vap[idx])) {
            p2p_vap[p2p_vap_num++] = up_mac_vap[idx];
        }
    }

    /* 没有2个p2p,不会组成p2p+p2p dbac */
    if (p2p_vap_num != 2) {
        return OAL_FALSE;
    }

    /* 不支持双p2p dbac */
    if (p2p_vap[0]->uc_vap_id == mac_vap->uc_vap_id) {
        if (hmac_chan_is_dbac_cfg(&(p2p_vap[1]->st_channel), new_channel) == OAL_TRUE) {
            return OAL_TRUE; // p2p[0]信道变化，p2p[0]新信道与p2p[1]当前信道 组成dbac
        }
    } else if (p2p_vap[1]->uc_vap_id == mac_vap->uc_vap_id) {
        if (hmac_chan_is_dbac_cfg(&(p2p_vap[0]->st_channel), new_channel) == OAL_TRUE) {
            return OAL_TRUE; // p2p[1]信道变化，p2p[1]新信道与p2p[0]当前信道 组成dbac
        }
    } else {
        if (hmac_chan_is_dbac_cfg(&(p2p_vap[0]->st_channel), &(p2p_vap[1]->st_channel)) == OAL_TRUE) {
            return OAL_TRUE; // 非p2p信道变化，p2p[0]当前信道与p2p[1]当前信道 组成dbac
        }
    }

    return OAL_FALSE;
}

/*
 * 功能描述 : 入网、漫游、信道切换时检查是否会导致组成不支持的dbac共存
 */
static oal_bool_enum_uint8 hmac_check_channel_is_unsupported_dbac(mac_vap_stru **up_mac_vap, mac_vap_stru *mac_vap,
    mac_channel_stru *new_channel, uint32_t up_vap_num)
{
    /* p2p dbac场景限制 */
    if (hmac_check_channel_is_unsupported_p2p_dbac(up_mac_vap, mac_vap, new_channel, up_vap_num) == OAL_TRUE) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

/*
 * 功能描述 : 计算2vap状态
 */
static mac_mvap_state_enum hmac_calc_2vap_new_state(mac_vap_stru **up_mac_vap, mac_vap_stru *mac_vap,
    mac_channel_stru *new_channel, uint32_t up_vap_num, hmac_channel_nums_stat_stru *channel_nums)
{
    uint8_t channel_nums_total = channel_nums->channel_nums_2g + channel_nums->channel_nums_5g;
    if (channel_nums_total == 1) {
        if (channel_nums->channel_nums_5g == 1 && hmac_check_is_support_single_dbdc()) {
            return MAC_MVAP_STATE_DBDC; // [0][1]单辅路dbdc
        }
        return MAC_MVAP_STATE_DCHN; // [1][0]或[0][1]
    }

    if (channel_nums->channel_nums_2g == 1 && channel_nums->channel_nums_5g == 1) {
        return MAC_MVAP_STATE_DBDC; // [1][1]
    }

    if (channel_nums_total == 2) { // 2vap 2信道
        if (channel_nums->channel_nums_5g == 2 &&  // 5G 2信道
            hmac_check_is_support_dual_5g_dbdc(up_mac_vap, mac_vap, new_channel, up_vap_num)) {
            return MAC_MVAP_STATE_DBDC; // [0][2]高低band dbdc
        } else if (hmac_check_channel_is_unsupported_dbac(up_mac_vap, mac_vap, new_channel, up_vap_num) == OAL_TRUE) {
            return MAC_MVAP_STATE_UNSUPPORTED; // 2p2p组dbac不支持
        } else if (channel_nums->channel_nums_5g == 2 && hmac_check_is_support_single_dbdc()) {
            return MAC_MVAP_STATE_DBDC_DBAC; // [0][2]单辅路dbdc+dbac
        }
        return MAC_MVAP_STATE_DBAC; // [2][0]或[0][2]
    }

    return MAC_MVAP_STATE_UNSUPPORTED;
}

/*
 * 功能描述 : 计算3vap状态
 */
static mac_mvap_state_enum hmac_calc_3vap_new_state(mac_vap_stru **up_mac_vap, mac_vap_stru *mac_vap,
    mac_channel_stru *new_channel, uint32_t up_vap_num, hmac_channel_nums_stat_stru *channel_nums)
{
    uint8_t channel_nums_total = channel_nums->channel_nums_2g + channel_nums->channel_nums_5g;
    if (channel_nums_total == 1) {
        if (channel_nums->channel_nums_5g == 1 && hmac_check_is_support_single_dbdc()) {
            return MAC_MVAP_STATE_DBDC; // [0][1]单辅路dbdc
        }
        return MAC_MVAP_STATE_DCHN; // [0][1]或[1][0]
    }

    if (channel_nums->channel_nums_2g == 3 || channel_nums->channel_nums_5g == 3) { // 5G 3信道or2G 3信道
        return MAC_MVAP_STATE_UNSUPPORTED; // [3][0]或[0][3]
    }

    if (channel_nums->channel_nums_2g == 1 && channel_nums->channel_nums_5g == 1) {
        return MAC_MVAP_STATE_DBDC; // [1][1]
    }

    if (channel_nums_total == 2) { // 3vap 2信道
        if (channel_nums->channel_nums_5g == 2 &&  // 5G 2信道
            hmac_check_is_support_dual_5g_dbdc(up_mac_vap, mac_vap, new_channel, up_vap_num)) {
            return MAC_MVAP_STATE_DBDC; // [0][2]高低band dbdc
        }
        return MAC_MVAP_STATE_UNSUPPORTED; // [2][0]或[0][2]
    }

    if (channel_nums_total == 3) { // 3vap 3信道
        /* 判断是否组成了不支持的dbac */
        if (hmac_check_channel_is_unsupported_dbac(up_mac_vap, mac_vap, new_channel, up_vap_num) == OAL_TRUE) {
            return MAC_MVAP_STATE_UNSUPPORTED;
        }
        return MAC_MVAP_STATE_DBDC_DBAC; // [1][2]或[2][1]
    }

    return MAC_MVAP_STATE_UNSUPPORTED;
}

/*
 * 功能描述 : 计算新的多vap状态
 */
static mac_mvap_state_enum hmac_calc_new_state(mac_vap_stru **up_mac_vap, mac_vap_stru *mac_vap,
    mac_channel_stru *new_channel, uint32_t up_vap_num, hmac_channel_nums_stat_stru *channel_nums)
{
    if (up_vap_num == 1) {
        return MAC_MVAP_STATE_SINGLE_VAP; // 单vap切信道都支持
    } else if (up_vap_num == 2) { // 2vap
        return hmac_calc_2vap_new_state(up_mac_vap, mac_vap, new_channel, up_vap_num, channel_nums);
    } else if (up_vap_num == 3) { // 3vap
        return hmac_calc_3vap_new_state(up_mac_vap, mac_vap, new_channel, up_vap_num, channel_nums);
    }
    return MAC_MVAP_STATE_UNSUPPORTED;
}

/*
 * 功能描述 : 获取当前多vap状态
 */
static mac_mvap_state_enum hmac_get_mvap_current_state(mac_device_stru *mac_device, uint32_t up_vap_num)
{
    if (mac_device->en_dbdc_running) {
        if (mac_device->en_dbac_running) {
            return MAC_MVAP_STATE_DBDC_DBAC;
        }
        return MAC_MVAP_STATE_DBDC;
    } else {
        if (mac_device->en_dbac_running) {
            return MAC_MVAP_STATE_DBAC;
        }

        if (up_vap_num > 1) {
            return MAC_MVAP_STATE_DCHN;
        }

        return MAC_MVAP_STATE_SINGLE_VAP;
    }
}

/*
 * 功能描述 : 对多vap状态进行特殊场景的二次过滤
 */
static mac_mvap_state_enum hmac_mvap_state_special_filter(mac_mvap_state_enum mvap_state, mac_vap_stru **up_mac_vap,
    mac_vap_stru *mac_vap, mac_channel_stru *new_channel, uint32_t up_vap_num)
{
#ifdef _PRE_WLAN_CHBA_MGMT
    /* 默认不支持chba 3vap组dbdc+dbac */
    if (mvap_state == MAC_MVAP_STATE_DBDC_DBAC) {
        if (hmac_chba_coex_is_support_dbdc_dbac(up_mac_vap, up_vap_num) == OAL_FALSE) {
            return MAC_MVAP_STATE_UNSUPPORTED;
        }
    }
#endif
    return mvap_state;
}
/*
 * 功能描述 : 获取新的多vap状态
 */
static mac_mvap_state_enum hmac_get_mvap_new_state(mac_device_stru *mac_device, mac_vap_stru **up_mac_vap,
    mac_vap_stru *mac_vap, mac_channel_stru *new_channel, uint32_t up_vap_num)
{
    uint8_t idx;
    mac_mvap_state_enum new_state;
    uint32_t channel_bitmap_2g = 0;
    uint32_t channel_bitmap_5g = 0;
    hmac_channel_nums_stat_stru channel_nums = {};
    mac_channel_stru *vap_channel = NULL;

    for (idx = 0; idx < up_vap_num; idx++) {
        vap_channel = (mac_vap->uc_vap_id == up_mac_vap[idx]->uc_vap_id) ? new_channel : (&up_mac_vap[idx]->st_channel);
        if (vap_channel->en_band == WLAN_BAND_2G) {
            channel_bitmap_2g |= ((uint32_t)1 << vap_channel->uc_chan_idx);
        } else {
            channel_bitmap_5g |= ((uint32_t)1 << vap_channel->uc_chan_idx);
        }
    }

    channel_nums.channel_nums_2g = oal_bit_get_num_four_byte(channel_bitmap_2g);
    channel_nums.channel_nums_5g = oal_bit_get_num_four_byte(channel_bitmap_5g);

    new_state = hmac_calc_new_state(up_mac_vap, mac_vap, new_channel, up_vap_num, &channel_nums);
    /* 特殊场景需要进行二次过滤 */
    new_state = hmac_mvap_state_special_filter(new_state, up_mac_vap, mac_vap, new_channel, up_vap_num);
    return new_state;
}

/* 功能描述:过滤不允许的状态切换 */
static mac_mvap_state_trans_enum hmac_chan_mgmt_filter_invalid_state_trans(
    mac_mvap_chan_switch_type_enum chan_switch_type, mac_mvap_state_trans_enum state_trans)
{
    /* 已被拦截的信道切换 */
    if (state_trans == MAC_MVAP_STATE_TRANS_UNSUPPORTED) {
        return state_trans;
    }

    /* 根据业务类型进行进一步拦截 */
    switch (chan_switch_type) {
        case MAC_MVAP_CHAN_SWITCH_STA_ROAM:
            /* 不允许通过漫游切换dbdc+dbac->dbac */
            if (state_trans == MAC_MVAP_STATE_TRANS_DBDC_AC_TO_DBAC) {
                return MAC_MVAP_STATE_TRANS_UNSUPPORTED;
            }
            break;
        case MAC_MVAP_CHAN_SWITCH_GO_CSA:
            /* go csa不允许 同信道->dbac, dbdc->dbac, dbdc->dbdc+dbac, dbdc+dbac->dbac */
            if ((state_trans == MAC_MVAP_STATE_TRANS_DCHN_TO_DBAC) ||
                (state_trans == MAC_MVAP_STATE_TRANS_DBDC_TO_DBAC) ||
                (state_trans == MAC_MVAP_STATE_TRANS_DBDC_TO_DBDC_AC) ||
                (state_trans == MAC_MVAP_STATE_TRANS_DBDC_AC_TO_DBAC)) {
                return MAC_MVAP_STATE_TRANS_UNSUPPORTED;
            }
            break;
#ifdef _PRE_WLAN_CHBA_MGMT
        case MAC_MVAP_CHAN_SWITCH_CHBA_CSA:
            /* 不允许通过chba csa切换dbdc+dbac->dbac */
            if (state_trans == MAC_MVAP_STATE_TRANS_DBDC_AC_TO_DBAC) {
                return MAC_MVAP_STATE_TRANS_UNSUPPORTED;
            }
            break;
#endif
        default:
            break;
    }
    /* 拦截处增加打印 */
    return state_trans;
}

/*
 * 功能描述 : 信道切换，获取新的多vap状态
 */
mac_mvap_state_trans_enum hmac_chan_switch_get_state_trans(mac_vap_stru **up_mac_vap, mac_vap_stru *mac_vap,
    mac_channel_stru *new_channel, uint32_t up_vap_num, mac_mvap_chan_switch_type_enum chan_switch_type)
{
    mac_mvap_state_enum cur_vap_state;
    mac_mvap_state_enum new_vap_state;
    mac_mvap_state_trans_enum state_trans;
    mac_device_stru *mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_device == NULL) {
        oam_error_log0(mac_vap->uc_device_id, OAM_SF_ANY,
            "{hmac_chan_switch_get_state_trans:fail to get mac_device!}");
        return MAC_MVAP_STATE_TRANS_UNSUPPORTED;
    }

    cur_vap_state = hmac_get_mvap_current_state(mac_device, up_vap_num);
    new_vap_state = hmac_get_mvap_new_state(mac_device, up_mac_vap, mac_vap, new_channel, up_vap_num);
    state_trans = g_chan_switch_state_trans_table[cur_vap_state][new_vap_state];

    /* 根据业务类型进行切信道拦截 */
    state_trans = hmac_chan_mgmt_filter_invalid_state_trans(chan_switch_type, state_trans);

    oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_chan_switch_get_state_trans:current_state[%d], \
        new_state[%d][0:DCHN 1:DBDC 2:DBAC 3:DBDC_DBAC 4:SINGLE_VAP 5:UNSUPPORT]}", cur_vap_state, new_vap_state);

    oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_ANY,
        "{switch[%d][0:roam 1:go/ap csa 2:chba csa], state_trans[%d][0-24:TRANS_TYPE 25:TRANS_UNSUPPORT]",
        chan_switch_type, state_trans);

    return state_trans;
}

/*
 * 功能描述 : 入网，获取新的多vap状态
 */
static mac_mvap_state_enum hmac_connect_get_state_trans(mac_device_stru *mac_device, mac_vap_stru **up_mac_vap,
    mac_vap_stru *mac_vap, mac_channel_stru *new_channel, uint32_t up_vap_num)
{
    int32_t ret;
    mac_mvap_state_enum new_vap_state;
    mac_vap_stru *valid_mac_vap[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {};
    uint32_t valid_vap_num;

    ret = memcpy_s(valid_mac_vap, sizeof(valid_mac_vap), up_mac_vap, sizeof(valid_mac_vap));
    if (ret != EOK) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_connect_get_state_trans:fail to memcpy!}");
        return MAC_MVAP_STATE_UNSUPPORTED;
    }
    /* 将新请求入网的vap也加入up vap列表中统一计算 */
    valid_mac_vap[up_vap_num] = mac_vap;
    valid_vap_num = up_vap_num + 1;
    new_vap_state = hmac_get_mvap_new_state(mac_device, valid_mac_vap, mac_vap, new_channel, valid_vap_num);

    oam_warning_log1(0, 0, "{hmac_connect_get_state_trans:new_state[%d]. \
        [0:DCHN 1:DBDC 2:DBAC 3:DBDC_DBAC 4:SINGLE_VAP 5:UNSUPPORT]}", new_vap_state);

    return new_vap_state;
}

/*
 * 功能描述 : 检查vap入网的信道是否支持。
 */
oal_bool_enum_uint8 hmac_check_coex_channel_is_valid(mac_vap_stru *mac_vap, mac_channel_stru *set_mac_channel)
{
    mac_device_stru *mac_device = NULL;
    mac_vap_stru *up_mac_vap[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {NULL};
    uint32_t up_vap_num;
    mac_mvap_state_enum state_trans;

    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_device == NULL) {
        return OAL_FALSE;
    }

    up_vap_num = mac_device_find_up_vaps(mac_device, up_mac_vap, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    if (up_vap_num == 0) {
        return OAL_TRUE;
    }
    /* 前两个vap组成dbac，不允许第三个vap组网 */
    if (mac_is_dbac_running(mac_device) && up_vap_num == 2) { // 2代表找到2个up vap
        return OAL_FALSE;
    }
    state_trans = hmac_connect_get_state_trans(mac_device, up_mac_vap, mac_vap, set_mac_channel, up_vap_num);
    if (state_trans == MAC_MVAP_STATE_UNSUPPORTED) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

static void hmac_set_ap_channel_ap_after_other_vap(mac_vap_stru *index_mac_vap, mac_vap_stru *check_mac_vap,
    mac_channel_stru *set_mac_channel, uint8_t *follow_channel,
    wlan_channel_bandwidth_enum_uint8 *follow_bandwidth)
{
    /* 替换信道值 */
    *follow_channel = index_mac_vap->st_channel.uc_chan_number;
    /* 默认不跳过cac */
    check_mac_vap->skip_cac = OAL_FALSE;

    if (set_mac_channel->en_band == WLAN_BAND_2G) {
        /* 2G信道和带宽直接跟随之前的VAP */
        *follow_bandwidth = index_mac_vap->st_channel.en_bandwidth;
    } else {
        /* 5G取带宽大的值 */
        wlan_bw_cap_enum_uint8 bw_cap = mac_vap_bw_mode_to_bw(set_mac_channel->en_bandwidth);
        wlan_bw_cap_enum_uint8 index_vap_bw_cap = mac_vap_bw_mode_to_bw(index_mac_vap->st_channel.en_bandwidth);
        if (bw_cap > WLAN_BW_CAP_80M && index_vap_bw_cap <= WLAN_BW_CAP_80M) {
            bw_cap = WLAN_BW_CAP_80M; // ap160M，其他vap小于160M，ap只能起在80M
        } else {
            bw_cap = oal_max(bw_cap, index_vap_bw_cap);
        }
        check_mac_vap->skip_cac = OAL_TRUE; // aput/GO信道跟随，不做cac
        *follow_bandwidth = mac_regdomain_get_bw_by_channel_bw_cap(*follow_channel, bw_cap);
    }

    oam_warning_log4(index_mac_vap->uc_vap_id, OAM_SF_2040,
        "{hmac_set_ap_channel_ap_after_other_vap::change Channel from [%d] To [%d], bw form [%d] To [%d]}.",
        set_mac_channel->uc_chan_number, *follow_channel,
        set_mac_channel->en_bandwidth, *follow_bandwidth);
}

static oal_bool_enum_uint8 hmac_set_ap_channel_check_state_and_mode(mac_vap_stru *index_mac_vap,
    mac_vap_stru *check_mac_vap)
{
    if ((index_mac_vap == NULL) || (check_mac_vap->uc_vap_id == index_mac_vap->uc_vap_id)) {
        return OAL_TRUE;
    }
    if ((index_mac_vap->en_vap_state != MAC_VAP_STATE_UP) &&
        (index_mac_vap->en_vap_state != MAC_VAP_STATE_PAUSE)) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

static void hmac_print_vap_info(mac_vap_stru **mac_vap_array, uint32_t vap_num, uint32_t positon)
{
    uint32_t i;
    oam_warning_log2(0, OAM_SF_ANY,
        "hmac_print_vap_info:: toal_vap_num %d, pos %d",
        vap_num, positon);
    for (i = 0; i < vap_num; i++) {
        mac_vap_stru *mac_vap_tmp = mac_vap_array[i];
        oam_warning_log3(mac_vap_tmp->uc_vap_id, OAM_SF_ANY,
            "hmac_print_vap_info::vap info:vap_mode=%d, p2p_mode=%d, channel=%d",
            mac_vap_tmp->en_vap_mode, mac_vap_tmp->en_p2p_mode,
            mac_vap_tmp->st_channel.uc_chan_number);
    }
}

/*
 * 返回up_vap_array中和新建vap相同band的same_band_vap_array以及数量
 */
static uint32_t hmac_get_same_band_vap_array(mac_vap_stru *mac_vap, const mac_channel_stru *set_mac_channel,
    mac_vap_stru *up_vap_array[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE], uint32_t up_vap_num,
    mac_vap_stru *same_band_vap_array[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE])
{
    uint32_t idx;
    uint32_t same_band_vap_num = 0;
    for (idx = 0; idx < up_vap_num; idx++) {
        if (up_vap_array[idx]->uc_vap_id == mac_vap->uc_vap_id) {
            oam_error_log1(0, OAM_SF_CHAN, "hmac_get_same_band_vap_array:vap %d should not in up_vap_array",
                mac_vap->uc_vap_id);
            hmac_print_vap_info(up_vap_array, up_vap_num, 0);
            continue;
        }
        if (up_vap_array[idx]->st_channel.en_band == set_mac_channel->en_band) {
            same_band_vap_array[same_band_vap_num++] = up_vap_array[idx];
        }
    }

    return same_band_vap_num;
}

/*
 * 函 数 名  : hmac_check_ap_channel_follow_other_vap
 * 功能描述  : 检查ap/go的信道是否需要跟随其他VAP
 * (1) 创建ap 时，如果和同band vap是dbac场景，则跟随同band 前一个vap。
 * (2) 创建go 时，如果和同band AP vap 是dbac场景，则跟随同band 前一个vap。
 * 返回值：OAL_SUCC:信道有跟随；其他值:没有信道跟随
 */
uint32_t hmac_check_ap_channel_follow_other_vap(mac_vap_stru *check_mac_vap,
    mac_cfg_channel_param_stru *channel_param, mac_channel_stru *set_mac_channel)
{
    mac_device_stru *mac_device = mac_res_get_mac_dev();
    mac_vap_stru *up_vap_array[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {NULL};
    uint32_t up_vap_num;
    mac_vap_stru *same_band_vap_array[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {NULL};
    uint32_t same_band_vap_num;
    uint8_t follow_channel = 0;
    wlan_channel_bandwidth_enum_uint8 follow_bandwidth = WLAN_BAND_WIDTH_20M;
    uint8_t vap_idx;

    if (oal_any_null_ptr3(check_mac_vap, channel_param, set_mac_channel)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 查找同band vap */
    up_vap_num = mac_device_find_up_vaps(mac_device, up_vap_array, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    same_band_vap_num = hmac_get_same_band_vap_array(check_mac_vap, set_mac_channel, up_vap_array, up_vap_num,
        same_band_vap_array);
    if (up_vap_num == 0 || same_band_vap_num == 0) {
        /* 无其他同band vap，不用信道跟随 */
        oam_warning_log2(check_mac_vap->uc_vap_id, OAM_SF_2040,
            "{hmac_check_ap_channel_follow_other_vap::no same band vap.up_vap_num %d, same_band_vap_num %d}",
            up_vap_num, same_band_vap_num);
        return OAL_FAIL;
    }

    oam_warning_log3(check_mac_vap->uc_vap_id, OAM_SF_2040,
        "{hmac_check_ap_channel_follow_other_vap:: check_vap_state=%d, check_vap_band=%d check_vap_channel=%d.}",
        check_mac_vap->en_vap_state, set_mac_channel->en_band, set_mac_channel->uc_chan_number);

    for (vap_idx = 0; vap_idx < same_band_vap_num; vap_idx++) {
        mac_vap_stru *index_mac_vap = NULL;
        index_mac_vap = same_band_vap_array[vap_idx];
        if (hmac_set_ap_channel_check_state_and_mode(index_mac_vap, check_mac_vap) == OAL_TRUE) {
            continue;
        }

        oam_warning_log4(index_mac_vap->uc_vap_id, OAM_SF_2040,
            "{index_vap_state=%d,index_vap_mode=%d,idx_vap_band=%d,idx_vap_chan=%d.}",
            index_mac_vap->en_vap_state, index_mac_vap->en_vap_mode, index_mac_vap->st_channel.en_band,
            index_mac_vap->st_channel.uc_chan_number);

        /* AP后启动，信道跟随同band vap，不做cac */
        /* GO后启动，信道跟随同band AP vap，不做cac */
        if (((is_legacy_ap(check_mac_vap) == OAL_TRUE) ||
            (is_p2p_go(check_mac_vap) == OAL_TRUE && is_legacy_ap(index_mac_vap) == OAL_TRUE)) &&
            (hmac_chan_is_dbac_cfg(set_mac_channel, &(index_mac_vap->st_channel)) == OAL_TRUE)) {
            hmac_set_ap_channel_ap_after_other_vap(index_mac_vap, check_mac_vap, set_mac_channel,
                &follow_channel, &follow_bandwidth);
            /* 同步修改信道和带宽 */
            set_mac_channel->uc_chan_number = channel_param->uc_channel = follow_channel;
            set_mac_channel->en_bandwidth = channel_param->en_bandwidth = follow_bandwidth;
            return OAL_SUCC;
        }
    }

    return OAL_FAIL;
}

void hmac_chan_sync_init(mac_vap_stru *pst_mac_vap, dmac_set_chan_stru *pst_set_chan)
{
    int32_t l_ret;

    memset_s(pst_set_chan, sizeof(dmac_set_chan_stru), 0, sizeof(dmac_set_chan_stru));
    l_ret = memcpy_s(&pst_set_chan->st_channel, sizeof(mac_channel_stru),
                     &pst_mac_vap->st_channel, sizeof(mac_channel_stru));
    l_ret += memcpy_s(&pst_set_chan->st_ch_switch_info, sizeof(mac_ch_switch_info_stru),
                      &pst_mac_vap->st_ch_switch_info, sizeof(mac_ch_switch_info_stru));
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_chan_sync_init::memcpy fail!");
        return;
    }
}
static void hmac_dump_chan(mac_vap_stru *pst_mac_vap, uint8_t *puc_param)
{
    dmac_set_chan_stru *pst_chan = NULL;

    if (oal_any_null_ptr2(pst_mac_vap, puc_param)) {
        return;
    }

    pst_chan = (dmac_set_chan_stru *)puc_param;
    oam_info_log0(pst_mac_vap->uc_vap_id, OAM_SF_2040, "channel mgmt info\r\n");
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "uc_chan_number=%d\r\n", pst_chan->st_channel.uc_chan_number);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "en_band=%d\r\n", pst_chan->st_channel.en_band);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "en_bandwidth=%d\r\n", pst_chan->st_channel.en_bandwidth);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "uc_idx=%d\r\n", pst_chan->st_channel.uc_chan_idx);

    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "c_announced_channel=%d\r\n",
                  pst_chan->st_ch_switch_info.uc_announced_channel);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "en_announced_bandwidth=%d\r\n",
                  pst_chan->st_ch_switch_info.en_announced_bandwidth);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "uc_ch_switch_cnt=%d\r\n",
                  pst_chan->st_ch_switch_info.uc_ch_switch_cnt);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "en_ch_switch_status=%d\r\n",
                  pst_chan->st_ch_switch_info.en_ch_switch_status);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "en_bw_switch_status=%d\r\n",
                  pst_chan->st_ch_switch_info.en_bw_switch_status);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "en_csa_present_in_bcn=%d\r\n",
                  pst_chan->st_ch_switch_info.en_csa_present_in_bcn);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "uc_start_chan_idx=%d\r\n",
                  pst_chan->st_ch_switch_info.uc_start_chan_idx);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "uc_end_chan_idx=%d\r\n",
                  pst_chan->st_ch_switch_info.uc_end_chan_idx);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "en_user_pref_bandwidth=%d\r\n",
                  pst_chan->st_ch_switch_info.en_user_pref_bandwidth);

    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "uc_new_channel=%d\r\n",
                  pst_chan->st_ch_switch_info.uc_new_channel);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "en_new_bandwidth=%d\r\n",
                  pst_chan->st_ch_switch_info.en_new_bandwidth);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "en_te_b=%d\r\n", pst_chan->st_ch_switch_info.en_te_b);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "ul_chan_report_for_te_a=%d\r\n",
                  pst_chan->st_ch_switch_info.chan_report_for_te_a);
}
/*
 * 函 数 名  : hmac_chan_do_sync
 * 功能描述  : HMAC模块抛事件到DMAC模块，设置SW/MAC/PHY/RF中的信道和带宽，
 * 1.日    期  : 2014年2月26日
 *   修改内容  : 新生成函数
 */
void hmac_chan_do_sync(mac_vap_stru *pst_mac_vap, dmac_set_chan_stru *pst_set_chan)
{
    frw_event_mem_stru *pst_event_mem = NULL;
    frw_event_stru *pst_event = NULL;
    uint32_t ret;
    uint8_t uc_idx;

    hmac_dump_chan(pst_mac_vap, (uint8_t *)pst_set_chan);
    /* 更新VAP下的主20MHz信道号、带宽模式、信道索引 */
    ret = mac_get_channel_idx_from_num(pst_mac_vap->st_channel.en_band, pst_set_chan->st_channel.uc_chan_number,
                                       pst_set_chan->st_channel.ext6g_band, &uc_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_chan_sync::mac_get_channel_idx_from_num failed[%d].}", ret);

        return;
    }

    pst_mac_vap->st_channel.uc_chan_number = pst_set_chan->st_channel.uc_chan_number;
    pst_mac_vap->st_channel.en_bandwidth = pst_set_chan->st_channel.en_bandwidth;
    pst_mac_vap->st_channel.uc_chan_idx = uc_idx;

    /* 申请事件内存 */
    pst_event_mem = frw_event_alloc_m(sizeof(dmac_set_chan_stru));
    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_chan_sync::pst_event_mem null.}");
        return;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WALN_CTX_EVENT_SUB_TYPR_SELECT_CHAN,
                       sizeof(dmac_set_chan_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    if (EOK != memcpy_s(frw_get_event_payload(pst_event_mem), sizeof(dmac_set_chan_stru),
                        (uint8_t *)pst_set_chan, sizeof(dmac_set_chan_stru))) {
        oam_error_log0(0, OAM_SF_SCAN, "hmac_chan_do_sync::memcpy fail!");
        frw_event_free_m(pst_event_mem);
        return;
    }

    /* 分发事件 */
    ret = frw_event_dispatch_event(pst_event_mem);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_chan_sync::frw_event_dispatch_event failed[%d].}", ret);
        frw_event_free_m(pst_event_mem);
        return;
    }

    /* 释放事件 */
    frw_event_free_m(pst_event_mem);
}

/*
 * 函 数 名  : hmac_chan_sync
 * 功能描述  : HMAC将信道/带宽信息同步到DMAC
 * 1.日    期  : 2014年2月26日
 *   修改内容  : 新生成函数
 */
void hmac_chan_sync(mac_vap_stru *mac_vap,
                    uint8_t uc_channel, wlan_channel_bandwidth_enum_uint8 en_bandwidth,
                    oal_bool_enum_uint8 en_switch_immediately)
{
    dmac_set_chan_stru st_set_chan;

    hmac_chan_sync_init(mac_vap, &st_set_chan);
    st_set_chan.st_channel.uc_chan_number = uc_channel;
    st_set_chan.st_channel.en_bandwidth = en_bandwidth;
    st_set_chan.en_switch_immediately = en_switch_immediately;
    st_set_chan.en_dot11FortyMHzIntolerant = mac_mib_get_FortyMHzIntolerant(mac_vap);
    hmac_cali_send_work_channel_cali_data(mac_vap, &(mac_vap->st_channel), CALI_DATA_NORMAL_JOIN_TYPE);
    hmac_chan_do_sync(mac_vap, &st_set_chan);
}

/*
 * 函 数 名  : hmac_chan_multi_select_channel_mac
 * 功能描述  : 遍历device下所有VAP，设置SW/MAC/PHY/RF中的信道和带宽，使VAP工作在新信道上
 * 1.日    期  : 2014年4月3日
 *   修改内容  : 新生成函数
 */
void hmac_chan_multi_select_channel_mac(mac_vap_stru *pst_mac_vap, uint8_t uc_channel,
                                        wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    uint8_t uc_vap_idx;
    mac_device_stru *pst_device = NULL;
    mac_vap_stru *pst_vap = NULL;

    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                     "{hmac_chan_multi_select_channel_mac::uc_channel=%d,en_bandwidth=%d}",
                     uc_channel, en_bandwidth);

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_device == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                       "{hmac_chan_multi_select_channel_mac::pst_device null,device_id=%d.}",
                       pst_mac_vap->uc_device_id);
        return;
    }

    if (pst_device->uc_vap_num == 0) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_2040, "{hmac_chan_multi_select_channel_mac::none vap.}");
        return;
    }

    if (mac_is_dbac_running(pst_device) == OAL_TRUE) {
        hmac_chan_sync(pst_mac_vap, uc_channel, en_bandwidth, OAL_TRUE);
        return;
    }

    /* 遍历device下所有vap， */
    for (uc_vap_idx = 0; uc_vap_idx < pst_device->uc_vap_num; uc_vap_idx++) {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == NULL) {
            oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                           "{hmac_chan_multi_select_channel_mac::pst_vap null,vap_id=%d.}",
                           pst_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        hmac_chan_sync(pst_vap, uc_channel, en_bandwidth, OAL_TRUE);
    }
}

/*
 * 函 数 名  : hmac_chan_update_40m_intol_user
 * 功能描述  : 更新en_40M_intol_user
 * 1.日    期  : 2015年4月23日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_chan_update_40m_intol_user(mac_vap_stru *pst_mac_vap)
{
    oal_dlist_head_stru *pst_entry = NULL;
    mac_user_stru *pst_mac_user = NULL;

    oal_dlist_search_for_each(pst_entry, &(pst_mac_vap->st_mac_user_list_head))
    {
        pst_mac_user = oal_dlist_get_entry(pst_entry, mac_user_stru, st_user_dlist);
        if (oal_unlikely(pst_mac_user == NULL)) {
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                             "{hmac_chan_update_40m_intol_user::pst_user null pointer.}");
            continue;
        } else {
            if (pst_mac_user->st_ht_hdl.bit_forty_mhz_intolerant) {
                pst_mac_vap->en_40m_intol_user = OAL_TRUE;
                return;
            }
        }
    }

    pst_mac_vap->en_40m_intol_user = OAL_FALSE;
}

/*
 * 函 数 名  : hmac_chan_get_user_pref_primary_ch
 * 功能描述  : 获取用户优先设置的信道
 * 1.日    期  : 2014年5月31日
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE uint8_t hmac_chan_get_user_pref_primary_ch(mac_device_stru *pst_mac_device)
{
    return pst_mac_device->uc_max_channel;
}

/*
 * 函 数 名  : hmac_get_user_pref_bandwidth
 * 功能描述  : 获取用户优先设置的带宽模式
 * 1.日    期  : 2014年5月31日
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE wlan_channel_bandwidth_enum_uint8 hmac_chan_get_user_pref_bandwidth(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->st_ch_switch_info.en_user_pref_bandwidth;
}

/*
 * 函 数 名  : hmac_chan_reval_bandwidth_sta
 * 功能描述  : 评估是否需要进行带宽切换，如需要，则通知硬件进行带宽切换
 * 1.日    期  : 2014年2月27日
 *   修改内容  : 新生成函数
 */
void hmac_chan_reval_bandwidth_sta(mac_vap_stru *pst_mac_vap, uint32_t change)
{
    if (oal_unlikely(pst_mac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_chan_reval_bandwidth_sta::pst_mac_vap null.}");
        return;
    }

    /* 需要进行带宽切换 */
    if (MAC_BW_CHANGE & change) {
        hmac_chan_multi_select_channel_mac(pst_mac_vap, pst_mac_vap->st_channel.uc_chan_number,
                                           pst_mac_vap->st_channel.en_bandwidth);
    }
}

/*
 * 函 数 名  : hmac_start_bss_in_available_channel
 * 功能描述  : 在指定(可用)信道上启动BSS
 * 1.日    期  : 2014年10月20日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_start_bss_in_available_channel(hmac_vap_stru *pst_hmac_vap)
{
    hmac_ap_start_rsp_stru st_ap_start_rsp;
    uint32_t ret;

    /* 解决beacon速率集未更新问题 */
    mac_vap_init_rates(&(pst_hmac_vap->st_vap_base_info));

    /* 设置AP侧状态机为 UP */
    hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_UP);

    /* 调用hmac_config_start_vap_event，启动BSS */
    ret = hmac_config_start_vap_event(&(pst_hmac_vap->st_vap_base_info), OAL_TRUE);
    if (oal_unlikely(ret != OAL_SUCC)) {
        hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_INIT);
        oam_warning_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_start_bss_in_available_channel::hmac_config_send_event failed[%d].}", ret);
        return ret;
    }

    /* 设置bssid */
    mac_vap_set_bssid(&pst_hmac_vap->st_vap_base_info, mac_mib_get_StationID(&pst_hmac_vap->st_vap_base_info),
        WLAN_MAC_ADDR_LEN);

    /* 入网优化，不同频段下的能力不一样 */
    if (WLAN_BAND_2G == pst_hmac_vap->st_vap_base_info.st_channel.en_band) {
        mac_mib_set_SpectrumManagementRequired(&(pst_hmac_vap->st_vap_base_info), OAL_FALSE);
    } else {
        mac_mib_set_SpectrumManagementRequired(&(pst_hmac_vap->st_vap_base_info), OAL_TRUE);
    }

    /* 将结果上报至sme */
    st_ap_start_rsp.en_result_code = HMAC_MGMT_SUCCESS;

    return OAL_SUCC;
}

/*
 * 函 数 名  : mac_get_center_freq1_from_freq_and_bandwidth
 * 功能描述  : 从频点和带宽信息计算得到center_freq1
 * 1.日    期  : 2019年09月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint16_t mac_get_center_freq1_from_freq_and_bandwidth(uint16_t freq,
    wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    uint16_t       us_center_freq1;

    /******************************************************
     * channel number to center channel idx map
    BAND WIDTH                            CENTER CHAN INDEX:

    WLAN_BAND_WIDTH_20M                   chan_num
    WLAN_BAND_WIDTH_40PLUS                chan_num + 2
    WLAN_BAND_WIDTH_40MINUS               chan_num - 2
    WLAN_BAND_WIDTH_80PLUSPLUS            chan_num + 6
    WLAN_BAND_WIDTH_80PLUSMINUS           chan_num - 2
    WLAN_BAND_WIDTH_80MINUSPLUS           chan_num + 2
    WLAN_BAND_WIDTH_80MINUSMINUS          chan_num - 6
    WLAN_BAND_WIDTH_160PLUSPLUSPLUS       chan_num + 14
    WLAN_BAND_WIDTH_160MINUSPLUSPLUS      chan_num + 10
    WLAN_BAND_WIDTH_160PLUSMINUSPLUS      chan_num + 6
    WLAN_BAND_WIDTH_160MINUSMINUSPLUS     chan_num + 2
    WLAN_BAND_WIDTH_160PLUSPLUSMINUS      chan_num - 2
    WLAN_BAND_WIDTH_160MINUSPLUSMINUS     chan_num - 6
    WLAN_BAND_WIDTH_160PLUSMINUSMINUS     chan_num - 10
    WLAN_BAND_WIDTH_160MINUSMINUSMINUS    chan_num - 14
    ********************************************************/
    int8_t ac_center_chan_offset[WLAN_BAND_WIDTH_BUTT - 2] = {  /* 减2可以得到中心信道的最大偏移量 */
#ifdef _PRE_WLAN_FEATURE_160M
        0, 2, -2, 6, -2, 2, -6, 14, -2, 6, -10, 10, -6, 2, -14
#else
        0, 2, -2, 6, -2, 2, -6
#endif
    };
    /* 2412为2.4G频点，5825为5G频点 */
    if (freq < 2412 || freq > 5825) {
        return OAL_FAIL;
    }
    /* 减2可以得到支持的最大带宽 */
    if (en_bandwidth >= WLAN_BAND_WIDTH_BUTT - 2) {
        return OAL_FAIL;
    }
    /* 偏移5M */
    us_center_freq1 = (uint16_t)(((int16_t)freq) + ac_center_chan_offset[en_bandwidth] * 5);

    return us_center_freq1;
}
static uint8_t hmac_check_need_report_channel_switch(hmac_vap_stru *hmac_vap)
{
#ifdef _PRE_WLAN_CHBA_MGMT
    hmac_chba_mgmt_info_stru *chba_mgmt_info = hmac_chba_get_mgmt_info();
    hmac_chba_chan_switch_info_stru *chan_switch_info = &chba_mgmt_info->chan_switch_info;
#endif

    /* 避免漫游状态csa上报导致supplicant和驱动状态不一致 */
    if (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_ROAMING) {
        oam_warning_log0(0, OAM_SF_CHAN, "{hmac_check_need_report_channel_switch::vap in roam state, No csa report!}");
        return OAL_FALSE;
    }
#ifdef _PRE_WLAN_CHBA_MGMT
    if (mac_is_chba_mode(&(hmac_vap->st_vap_base_info)) && chan_switch_info->csa_save_lost_device_flag == TRUE) {
        oam_warning_log0(0, OAM_SF_CHAN, "{hmac_check_need_report_channel_switch::chba save lost device, NO report!}");
        return OAL_FALSE;
    }
#endif
    return OAL_TRUE;
}

static void hmac_fill_report_channel_elements(hmac_vap_stru *hmac_vap, hmac_channel_switch_stru *chan_switch,
    mac_channel_stru *channel)
{
    chan_switch->center_freq = (channel->en_band == WLAN_BAND_2G) ?
            (g_ast_freq_map_2g[channel->uc_chan_idx].us_freq) : (g_ast_freq_map_5g[channel->uc_chan_idx].us_freq);
    chan_switch->width = (oal_nl80211_chan_width)wlan_bandwidth_to_ieee_chan_width(channel->en_bandwidth,
        mac_mib_get_HighThroughputOptionImplemented(&hmac_vap->st_vap_base_info));

    chan_switch->center_freq1 = mac_get_center_freq1_from_freq_and_bandwidth(chan_switch->center_freq,
        channel->en_bandwidth);
    /* 5MHz, 10MHz, 20MHz, 40MHz, 80MHz center_freq2 is zero */
    /* 80+80MHz need set center_freq2 */
    oam_warning_log4(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CHAN,
        "hmac_fill_report_channel_elements: center_freq=%d width=%d, center_freq1=%d center_freq2=%d",
        chan_switch->center_freq, chan_switch->width, chan_switch->center_freq1, chan_switch->center_freq2);
}

/*
 * 函 数 名  : hmac_report_channel_switch
 * 功能描述  : 发送channel switch 通知消息到wpa_supplicant
 * 1.日    期  : 2019年09月26日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_report_channel_switch(hmac_vap_stru *hmac_vap, mac_channel_stru *channel)
{
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *event = NULL;
    hmac_channel_switch_stru chan_switch = {0};

    if (hmac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (hmac_check_need_report_channel_switch(hmac_vap) == OAL_FALSE) {
        return OAL_SUCC;
    }
    if (mac_is_channel_num_valid(channel->en_band, channel->uc_chan_number, channel->ext6g_band) != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_CHAN, "{hmac_report_channel_switch::invalid channel, en_band=%d, ch_num=%d",
            channel->en_band, channel->uc_chan_number);
        return OAL_FAIL;
    }
    hmac_fill_report_channel_elements(hmac_vap, &chan_switch, channel);
    event_mem = frw_event_alloc_m(sizeof(chan_switch));
    if (event_mem == NULL) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CHAN,
            "{hmac_report_channel_switch::frw_event_alloc_m fail! size[%d]}", sizeof(chan_switch));
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写事件 */
    event = frw_get_event_stru(event_mem);
    if (event == NULL) {
        frw_event_free_m(event_mem);
        return OAL_ERR_CODE_PTR_NULL;
    }

    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_HOST_CTX, HMAC_HOST_CTX_EVENT_SUB_TYPE_CH_SWITCH_NOTIFY,
        sizeof(chan_switch), FRW_EVENT_PIPELINE_STAGE_0, hmac_vap->st_vap_base_info.uc_chip_id,
        hmac_vap->st_vap_base_info.uc_device_id, hmac_vap->st_vap_base_info.uc_vap_id);

    if (EOK != memcpy_s((uint8_t *)frw_get_event_payload(event_mem), sizeof(chan_switch),
                        (uint8_t *)&chan_switch, sizeof(chan_switch))) {
        oam_error_log0(0, OAM_SF_CHAN, "hmac_report_channel_switch::memcpy fail!");
        frw_event_free_m(event_mem);
        return OAL_FAIL;
    }

    /* 分发事件 */
    frw_event_dispatch_event(event_mem);
    frw_event_free_m(event_mem);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_chan_restart_network_after_switch
 * 功能描述  : 输入参数  : 无
 * 1.日    期  : 2014年11月7日
 *   修改内容  : 新生成函数
 */
void hmac_chan_restart_network_after_switch(mac_vap_stru *pst_mac_vap)
{
    frw_event_mem_stru *pst_event_mem = NULL;
    frw_event_stru *pst_event = NULL;
    uint32_t ret;

    oam_info_log0(pst_mac_vap->uc_vap_id, OAM_SF_2040, "{hmac_chan_restart_network_after_switch}");

    /* 申请事件内存 */
    pst_event_mem = frw_event_alloc_m(0);
    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                       "{hmac_chan_restart_network_after_switch::pst_event_mem null.}");

        return;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPR_RESTART_NETWORK,
                       0,
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    /* 分发事件 */
    ret = frw_event_dispatch_event(pst_event_mem);
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
            "{hmac_chan_restart_network_after_switch::frw_event_dispatch_event failed[%d].}", ret);
        frw_event_free_m(pst_event_mem);

        return;
    }
    frw_event_free_m(pst_event_mem);
}
static void hmac_ap_chan_switch_end_send_cali_data(mac_vap_stru *mac_vap)
{
    cali_data_req_stru cali_data_req_info;

    cali_data_req_info.mac_vap_id = mac_vap->uc_vap_id;
    cali_data_req_info.channel = mac_vap->st_channel;
    cali_data_req_info.work_cali_data_sub_type =  CALI_DATA_NORMAL_JOIN_TYPE;

    wlan_chip_update_cur_chn_cali_data(&cali_data_req_info);
}

OAL_STATIC void hmac_chan_sync_set_chan_info(mac_device_stru *mac_device, hmac_vap_stru *hmac_vap,
    dmac_set_chan_stru *set_chan, uint8_t idx)
{
    mac_vap_stru *mac_vap = &hmac_vap->st_vap_base_info;

    mac_vap->st_channel.uc_chan_number = set_chan->st_channel.uc_chan_number;
    mac_vap->st_channel.en_band = set_chan->st_channel.en_band;
    mac_vap->st_channel.en_bandwidth = set_chan->st_channel.en_bandwidth;
    mac_vap->st_channel.uc_chan_idx = idx;

    mac_vap->st_ch_switch_info.en_waiting_to_shift_channel = set_chan->st_ch_switch_info.en_waiting_to_shift_channel;

    mac_vap->st_ch_switch_info.en_ch_switch_status = set_chan->st_ch_switch_info.en_ch_switch_status;
    mac_vap->st_ch_switch_info.en_bw_switch_status = set_chan->st_ch_switch_info.en_bw_switch_status;

    /* aput切完信道同步切信道的标志位,防止再有用户关联,把此变量又同步下去 */
    mac_vap->st_ch_switch_info.uc_ch_switch_cnt = set_chan->st_ch_switch_info.uc_ch_switch_cnt;
    mac_vap->st_ch_switch_info.en_csa_present_in_bcn = set_chan->st_ch_switch_info.en_csa_present_in_bcn;

    /* 同步device信息 */
    mac_device->uc_max_channel = mac_vap->st_channel.uc_chan_number;
    mac_device->en_max_band = mac_vap->st_channel.en_band;
    mac_device->en_max_bandwidth = mac_vap->st_channel.en_bandwidth;
}

OAL_STATIC void hmac_dfs_switch_to_new_chann(mac_device_stru *mac_device, hmac_vap_stru *hmac_vap)
{
#ifdef _PRE_WLAN_FEATURE_DFS
    if (OAL_TRUE == hmac_dfs_need_for_cac(mac_device, &hmac_vap->st_vap_base_info)) {
        hmac_dfs_cac_start(mac_device, hmac_vap);
        oam_info_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_2040,
            "{hmac_dfs_switch_to_new_chann::[DFS]CAC START!}");
        return;
    }

    hmac_chan_restart_network_after_switch(&(hmac_vap->st_vap_base_info));
#endif
}

OAL_STATIC void hmac_chan_switch_to_new_chan_check(hmac_vap_stru *hmac_vap, uint8_t *ap_follow_channel)
{
    uint32_t ret;
    mac_vap_stru *mac_vap = &hmac_vap->st_vap_base_info;
#ifdef _PRE_WLAN_CHBA_MGMT
    hmac_chba_mgmt_info_stru *chba_mgmt_info = hmac_chba_get_mgmt_info();
    hmac_chba_chan_switch_info_stru *chan_switch_info = &chba_mgmt_info->chan_switch_info;
#endif
    if (is_sta(mac_vap)) {
        /* dbac 场景信道跟随统一在dbac状态变化通知(hmac_dbac_status_notify)中处理 */
    } else {
#ifdef _PRE_WLAN_CHBA_MGMT
        if (mac_is_chba_mode(&(hmac_vap->st_vap_base_info)) && chan_switch_info->csa_lost_device_num > 0) {
            oam_warning_log0(0, OAM_SF_CHAN,
                "{hmac_chan_switch_to_new_chan_check::chba save lost device, NO update cali data!}");
        } else {
            hmac_ap_chan_switch_end_send_cali_data(mac_vap);
        }
#endif
    }
    ret = hmac_report_channel_switch(hmac_vap, &(mac_vap->st_channel));
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ASSOC,
            "hmac_chan_switch_to_new_chan_check::return fail is[%d]", ret);
    }
}
/*
 * 函 数 名  : hmac_chan_switch_to_new_chan_complete
 * 功能描述  : 处理信道/带宽切换完成事件
 * 1.日    期  : 2014年5月8日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_chan_switch_to_new_chan_complete(frw_event_mem_stru *event_mem)
{
    frw_event_stru *event = NULL;
    mac_device_stru *mac_device = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    mac_vap_stru *mac_vap = NULL;
    dmac_set_chan_stru *set_chan = NULL;
    uint32_t ret;
    uint8_t idx;
    uint8_t ap_follow_channel = 0;
    uint8_t band;

    if (oal_unlikely(event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_2040, "{hmac_chan_switch_to_new_chan_complete::event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    event = frw_get_event_stru(event_mem);
    set_chan = (dmac_set_chan_stru *)event->auc_event_data;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(event->st_event_hdr.uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(event->st_event_hdr.uc_vap_id, OAM_SF_2040,
            "{hmac_chan_switch_to_new_chan_complete::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    mac_vap = &hmac_vap->st_vap_base_info;

    oam_warning_log0(event->st_event_hdr.uc_vap_id, OAM_SF_2040, "hmac_chan_switch_to_new_chan_complete");
    hmac_dump_chan(mac_vap, (uint8_t *)set_chan);

    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (oal_unlikely(mac_device == NULL)) {
        oam_error_log0(event->st_event_hdr.uc_vap_id, OAM_SF_2040,
                       "{hmac_chan_switch_to_new_chan_complete::mac_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }
    band = mac_get_band_by_channel_num(set_chan->st_channel.uc_chan_number);
    ret = mac_get_channel_idx_from_num(band, set_chan->st_channel.uc_chan_number,
        set_chan->st_channel.ext6g_band, &idx);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_2040,
            "{hmac_chan_switch_to_new_chan_complete::mac_get_channel_idx_from_num failed[%d].}", ret);

        return OAL_FAIL;
    }

    hmac_chan_sync_set_chan_info(mac_device, hmac_vap, set_chan, idx);

#ifdef _PRE_WLAN_FEATURE_HID2D
    /* 通知HiD2D ACS切换成功 */
    hmac_hid2d_acs_switch_completed(mac_vap);
#endif

    /* 信道跟随检查 */
    hmac_chan_switch_to_new_chan_check(hmac_vap, &ap_follow_channel);

    if (set_chan->en_check_cac == OAL_FALSE) {
        return OAL_SUCC;
    }

    hmac_dfs_switch_to_new_chann(mac_device, hmac_vap);

    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_40M_intol_sync_event
 * 功能描述  : 初始化带宽或者用户增加导致带宽发生变化
 * 1.日    期  : 2017年1月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_40m_intol_sync_event(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_40M_INTOL_UPDATE, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                         "{hmac_40M_intol_sync_event::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_40m_intol_sync_data
 * 功能描述  : 初始化带宽或者用户增加导致带宽发生变化
 * 1.日    期  : 2017年1月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_40m_intol_sync_data(mac_vap_stru *mac_vap,
                              wlan_channel_bandwidth_enum_uint8 bandwidth_40m,
                              oal_bool_enum_uint8 intol_user_40m)
{
    mac_bandwidth_stru band_prot;

    memset_s(&band_prot, sizeof(mac_bandwidth_stru), 0, sizeof(mac_bandwidth_stru));

    band_prot.en_40m_bandwidth = bandwidth_40m;
    band_prot.en_40m_intol_user = intol_user_40m;
    hmac_40m_intol_sync_event(mac_vap, sizeof(band_prot), (uint8_t *)&band_prot);
}

/*
 * 功能描述  : 根据channel类型返回对应信道的枚举类型
 * 1.日    期  : 2022年3月10日
 *   修改内容  : 新生成函数
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
uint8_t hmac_get_80211_band_type(mac_channel_stru *channel)
{
    if (channel->en_band == WLAN_BAND_2G) {
        return NL80211_BAND_2GHZ;
    }
    if (channel->en_band == WLAN_BAND_5G) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 184))
        if (channel->ext6g_band == OAL_TRUE) {
            return NL80211_BAND_6GHZ;
        }
#endif
        return NL80211_BAND_5GHZ;
    } else {
        oam_error_log0(0, 0, "{hmac_get_80211_band_type::wrong wlan band.}");
        return NUM_NL80211_BANDS;
    }
}
#else
uint8_t hmac_get_80211_band_type(mac_channel_stru *channel)
{
    if (channel->en_band == WLAN_BAND_2G) {
        return IEEE80211_BAND_2GHZ;
    } else if (channel->en_band == WLAN_BAND_5G) {
        return IEEE80211_BAND_5GHZ;
    } else {
        oam_error_log0(0, 0, "{hmac_get_80211_band_type::wrong wlan band.}");
        return IEEE80211_NUM_BANDS;
    }
}
#endif
uint32_t hmac_ch_status_info_syn_event(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    frw_event_hdr_stru *pst_event_hdr = NULL;
    mac_ap_ch_info_stru *past_ap_ch_list = NULL;
    mac_device_stru *pst_mac_device = NULL;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_ch_status_info_syn_event::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件、事件头以及事件payload结构体 */
    pst_event = frw_get_event_stru(pst_event_mem);
    pst_event_hdr = &(pst_event->st_event_hdr);
    past_ap_ch_list = (mac_ap_ch_info_stru *)pst_event->auc_event_data;

    pst_mac_device = mac_res_get_dev(pst_event_hdr->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_ch_status_info_syn_event::mac_res_get_mac_vap fail.vap_id:%u}",
                       pst_event_hdr->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (memcpy_s(pst_mac_device->st_ap_channel_list, sizeof(pst_mac_device->st_ap_channel_list),
        (uint8_t *)past_ap_ch_list, sizeof(mac_ap_ch_info_stru) * MAC_MAX_SUPP_CHANNEL) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_ch_status_info_syn_event::memcpy fail!");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


/*
 * 功能描述 : 判断两vap信道配置是否组成dbac
 */
oal_bool_enum_uint8 hmac_chan_is_dbac_cfg(const mac_channel_stru *channel_1,
    const mac_channel_stru *channel_2)
{
    /* band 不同，不会是DBAC */
    if (channel_1->en_band != channel_2->en_band) {
        return OAL_FALSE;
    }

    /* 2G band场景， 信道不同，则是DBAC */
    if (channel_1->en_band == WLAN_BAND_2G) {
        return (channel_1->uc_chan_number != channel_2->uc_chan_number) ? OAL_TRUE : OAL_FALSE;
    }

    /* 5G band场景，需要判断特殊DBDC */
    if (channel_1->en_band == WLAN_BAND_5G &&
        channel_1->uc_chan_number != channel_2->uc_chan_number &&
        hmac_jugde_dual_5g_dbdc(channel_1, channel_2) == OAL_FALSE) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}
