/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 配置相关实现hmac接口实现源文件
 * 作    者 :
 * 创建日期 : 2013年1月8日
 */

/* 1 头文件包含 */
#include "oam_ext_if.h"
#include "frw_ext_if.h"

#include "hmac_resource.h"
#include "host_hal_device.h"
#include "host_hal_ext_if.h"
#include "hmac_vap.h"
#include "hmac_config.h"
#include "securec.h"
#include "securectype.h"
#include "wlan_mib.h"
#include "hmac_roam_main.h"
#include "hmac_tx_switch.h"
#include "hmac_roam_alg.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CONFIG_D2H_C

#ifdef _PRE_WLAN_FEATURE_M2S
/*
 * 函 数 名  : hmac_config_d2h_device_m2s_info_syn
 * 功能描述  : dmac_offload架构下同步device的m2s信息到hmac
 * 1.日    期  : 2016年9月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_d2h_device_m2s_info_syn(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_device_m2s_stru *pst_syn_info;
    mac_device_stru *pst_mac_device;

    /* 该vap是配置vap，mib指针为空，此处不需要判断mib */
    if ((pst_mac_vap->uc_init_flag == MAC_VAP_INVAILD) || (puc_param == NULL)) {
        oam_warning_log1(0, OAM_SF_M2S, "{hmac_config_device_m2s_info_syn::pst_mac_vap->uc_init_flag[%d], \
                         pst_mac_vap->pst_mib_info or puc_param null!}",
                         pst_mac_vap->uc_init_flag);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_syn_info = (mac_device_m2s_stru *)puc_param;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log1(0, OAM_SF_M2S, "{hmac_config_device_m2s_info_syn: mac device is null ptr. device id:%d.}",
                       pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 1.mac device的能力需要同步, 防止硬切换重启之后host没有恢复，其他能力待开发 */
    mac_device_set_nss_num(pst_mac_device, pst_syn_info->en_nss_num);

    /* 2.smps能力需要刷新，防止硬切换时候，vap smps mib初始化不正常 */
    mac_device_set_mode_smps(pst_mac_device, pst_syn_info->en_smps_mode);
    /* 关键信息同步提示 */
    oam_warning_log2(0, OAM_SF_M2S,
                     "{hmac_config_device_m2s_info_syn::en_nss_num:[%d] smps mode[%d].}",
                     pst_syn_info->en_nss_num, mac_device_get_mode_smps(pst_mac_device));

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_vap_m2s_info_syn
 * 功能描述  : dmac_offload架构下同步sta vap的状态到dmac
 * 1.日    期  : 2017年11月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_vap_m2s_info_syn(mac_vap_stru *pst_mac_vap)
{
    uint32_t ret;
    mac_vap_m2s_stru st_m2s_vap_info;
    mac_device_stru *pst_mac_dev;

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_dev == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                       "{hmac_config_vap_m2s_info_syn::pst_mac_dev[%d] null.}", pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band) {
        /*  2G VAP下short gi for 160M mib值需置为0 与标杆对齐 */
        mac_mib_set_VHTShortGIOptionIn160and80p80Implemented(pst_mac_vap, OAL_FALSE);

        /* 根据当前带宽刷新mib，5g默认160M支持 */
        mac_mib_set_VHTChannelWidthOptionImplemented(pst_mac_vap, WLAN_MIB_VHT_SUPP_WIDTH_80);
    } else { /* 5G恢复成最大默认能力 */
        if (mac_device_get_cap_bw(pst_mac_dev) >= WLAN_BW_CAP_160M) {
            mac_mib_set_VHTShortGIOptionIn160and80p80Implemented(pst_mac_vap, WLAN_HAL0_VHT_SGI_SUPP_160_80P80);
        } else {
            mac_mib_set_VHTShortGIOptionIn160and80p80Implemented(pst_mac_vap, OAL_FALSE);
        }

        mac_mib_set_VHTChannelWidthOptionImplemented(pst_mac_vap,
            mac_device_trans_bandwith_to_vht_capinfo(mac_device_get_cap_bw(pst_mac_dev)));
    }

    /* host暂时只是opmode的cap flag需要同步 */
    st_m2s_vap_info.en_support_opmode = pst_mac_vap->st_cap_flag.bit_opmode;

    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_VAP_M2S_INFO_SYN,
                                 sizeof(mac_vap_m2s_stru), (uint8_t *)(&st_m2s_vap_info));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                         "{hmac_config_vap_m2s_info_syn::hmac_config_send_event failed[%d],user_id[%d].}", ret);
    }

    return ret;
}

uint32_t hmac_d2h_amsdu_switch_report(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    hmac_vap_stru *hmac_vap = NULL;
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_M2S, "{hmac_d2h_amsdu_switch_report::hmac_vap is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    hmac_vap->d2h_amsdu_switch = *param;
    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_M2S, "{d2h_amsdu_switch::flag[%d]!}", hmac_vap->d2h_amsdu_switch);
    return OAL_SUCC;
}

OAL_INLINE void hmac_config_d2h_vap_m2s_info_syn_set_cap(mac_vap_stru *pst_mac_vap, mac_vap_m2s_stru *pst_syn_info)
{
#ifdef _PRE_WLAN_FEATURE_SMPS
    mac_mib_set_smps(pst_mac_vap, pst_syn_info->en_sm_power_save);
#endif
#ifdef _PRE_WLAN_FEATURE_TXBF_HT
    mac_mib_set_TransmitStaggerSoundingOptionImplemented(pst_mac_vap, pst_syn_info->en_transmit_stagger_sounding);
#endif
    mac_mib_set_TxSTBCOptionImplemented(pst_mac_vap, pst_syn_info->en_tx_stbc);

    mac_mib_set_vht_ctrl_field_cap(pst_mac_vap, pst_syn_info->en_vht_ctrl_field_supported);
#ifdef _PRE_WLAN_FEATURE_TXBF
    mac_mib_set_VHTTxSTBCOptionImplemented(pst_mac_vap, pst_syn_info->en_tx_vht_stbc_optionimplemented);
    mac_mib_set_VHTNumberSoundingDimensions(pst_mac_vap, pst_syn_info->en_vht_number_sounding_dimensions);
    mac_mib_set_VHTSUBeamformerOptionImplemented(pst_mac_vap, pst_syn_info->en_vht_su_beamformer_optionimplemented);

    mac_mib_set_VHTSUBeamformeeOptionImplemented(pst_mac_vap, pst_syn_info->en_su_bfee);
    mac_mib_set_VHTBeamformeeNTxSupport(pst_mac_vap, pst_syn_info->uc_vht_ntx_sts);
#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        mac_mib_set_he_SUBeamformer(pst_mac_vap, pst_syn_info->en_he_su_bfer);
        mac_mib_set_he_BeamformeeSTSBelow80Mhz(pst_mac_vap, pst_syn_info->uc_he_ntx_sts_below_80m);
        mac_mib_set_he_BeamformeeSTSOver80Mhz(pst_mac_vap, pst_syn_info->uc_he_ntx_sts_over_80m);
        mac_mib_set_HENumberSoundingDimensionsBelow80Mhz(pst_mac_vap, pst_syn_info->uc_he_num_dim_below_80m);
        mac_mib_set_HENumberSoundingDimensionsOver80Mhz(pst_mac_vap, pst_syn_info->uc_he_num_dim_over_80m);
    }
#endif
#endif
}

/*
 * 函 数 名  : hmac_config_vap_m2s_info_syn
 * 功能描述  : dmac_offload架构下同步vap的m2s信息到hmac(user涉及的mimo-siso信息差异)
 * 1.日    期  : 2016年9月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_d2h_vap_m2s_info_syn(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_vap_m2s_stru *pst_syn_info;
    mac_cfg_kick_user_param_stru st_kick_user_param;
    uint32_t ret;
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

    /* , 开关机压力测试，对于device同步host事件，vap可能已经删除，
       需要增加此类保护， 后续d2h同步接口都需要增加判空保护 */
    if ((pst_mac_vap->uc_init_flag == MAC_VAP_INVAILD) || oal_any_null_ptr2(pst_mac_vap->pst_mib_info, puc_param)) {
        oam_warning_log1(0, OAM_SF_M2S,
                         "{hmac_config_d2h_vap_m2s_info_syn::pst_mac_vap->uc_init_flag[%d], \
                         pst_mac_vap->pst_mib_info or puc_param null!}",
                         pst_mac_vap->uc_init_flag);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_syn_info = (mac_vap_m2s_stru *)puc_param;

    /* 1.m2s vap信息同步hmac */
    mac_vap_set_rx_nss(pst_mac_vap, pst_syn_info->en_vap_rx_nss);

    if (mac_vap_spec_is_sw_need_m2s_switch(pst_mac_vap)) {
        /*  spec模式需要类似硬切换方式，需要根据spec刷新support速率集 */
        mac_vap_init_rates(pst_mac_vap);
    }

    /* 2. 修改HT能力 */
    hmac_config_d2h_vap_m2s_info_syn_set_cap(pst_mac_vap, pst_syn_info);

    if (WLAN_M2S_TYPE_HW == pst_syn_info->en_m2s_type) {
        /* 硬切换需要踢除所有主路上的用户 */
        st_kick_user_param.us_reason_code = MAC_UNSPEC_REASON;
        st_kick_user_param.send_disassoc_immediately = OAL_TRUE;
        oal_set_mac_addr(st_kick_user_param.auc_mac_addr, auc_mac_addr);

        /* 1. 踢掉该vap的用户 */
        ret = hmac_config_kick_user(pst_mac_vap, sizeof(uint32_t), (uint8_t *)&st_kick_user_param);
        if (ret != OAL_SUCC) {
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                "{hmac_config_d2h_vap_m2s_info_syn::hmac_config_kick_user fail!}");
        }

        /* 2. 更新速率集，vap的空间流，暂时未看到其他需要更新的地方 */
        mac_vap_init_rates(pst_mac_vap);
    }

    /* 关键信息同步提示 */
    oam_warning_log3(0, 0, "{hmac_config_d2h_vap_m2s_info_syn::en_vap_rx_nss:%d, en_sm_power_save:%d, en_m2s_type:%d.}",
                     pst_syn_info->en_vap_rx_nss, pst_syn_info->en_sm_power_save, pst_syn_info->en_m2s_type);

    return OAL_SUCC;
}

/*
 * 功能描述  : m2s切换保护完成状态通知host
 * 1.日    期  : 2018年2月9日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_m2s_switch_protect_comp_event_status(mac_vap_stru *mac_vap, uint8_t uc_len, uint8_t *param)
{
    dmac_m2s_complete_syn_stru *m2s_switch_comp_status = NULL;
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *event = NULL;
    uint32_t ret = OAL_SUCC;

    if ((mac_vap->uc_init_flag == MAC_VAP_INVAILD) || (param == NULL)) {
        oam_warning_log1(0, OAM_SF_M2S, "{hmac_m2s_switch_protect_comp_event_status::mac_vap->uc_init_flag[%d], \
            or param null!}", mac_vap->uc_init_flag);
        return OAL_ERR_CODE_PTR_NULL;
    }

    m2s_switch_comp_status = (dmac_m2s_complete_syn_stru *)param;

    /* WTDB 对于驱动自身触发不用上报，对应于butt状态，触发arp probe即可 */
    if (m2s_switch_comp_status->uc_m2s_state == HAL_M2S_ARP_DETECT) {
        hmac_arp_probe_type_set(mac_vap, m2s_switch_comp_status->pri_data.arp_detect_result.en_arp_detect_on,
            HMAC_VAP_ARP_PROBE_TYPE_M2S);
    } else {
        /* 抛扫描完成事件到WAL */
        event_mem = frw_event_alloc_m(sizeof(dmac_m2s_complete_syn_stru));
        if (event_mem == NULL) {
            oam_error_log0(mac_vap->uc_vap_id, OAM_SF_M2S, "{hmac_m2s_switch_protect_comp_event_status::memory null}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        /* 填写事件 */
        event = frw_get_event_stru(event_mem);

        frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_HOST_CTX, HMAC_HOST_CTX_EVENT_SUB_TYPE_M2S_STATUS,
            sizeof(dmac_m2s_complete_syn_stru), FRW_EVENT_PIPELINE_STAGE_0, mac_vap->uc_chip_id,
            mac_vap->uc_device_id, mac_vap->uc_vap_id);

        if (memcpy_s((uint8_t *)frw_get_event_payload(event_mem), sizeof(dmac_m2s_complete_syn_stru),
            (uint8_t *)m2s_switch_comp_status, uc_len) != EOK) {
            oam_error_log0(0, OAM_SF_M2S, "hmac_m2s_switch_protect_comp_event_status::memcpy fail!");
            frw_event_free_m(event_mem);
            return OAL_FAIL;
        }

        /* 分发事件 */
        ret = frw_event_dispatch_event(event_mem);
        if (ret != OAL_SUCC) {
            oam_error_log0(mac_vap->uc_vap_id, OAM_SF_M2S,
                "{hmac_m2s_switch_protect_comp_event_status::frw_event_dispatch_event fail.}");
        }
        frw_event_free_m(event_mem);
    }
    return ret;
}

/*
 * 函 数 名  : hmac_config_mimo_compatibility
 * 功能描述  : 配置黑名单到hmac 启动重关联
 * 1.日    期  : 2018年12月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_mimo_compatibility(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    dmac_to_hmac_mimo_compatibility_event_stru *pst_dmac_to_hmac_mimo_compatibility;
    hmac_vap_stru *pst_hmac_vap;
    hmac_user_stru *pst_hmac_user;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (oal_unlikely(pst_hmac_vap == NULL)) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "{hmac_config_mimo_compatibility::pst_hmac_vap is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_to_hmac_mimo_compatibility = (dmac_to_hmac_mimo_compatibility_event_stru *)puc_param;

    pst_hmac_user = mac_res_get_hmac_user(pst_dmac_to_hmac_mimo_compatibility->us_user_id);
    if (oal_unlikely(pst_hmac_user == NULL)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                         "{hmac_config_mimo_compatibility::pst_hmac_user is null! user_id is %d.}",
                         pst_dmac_to_hmac_mimo_compatibility->us_user_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_user->en_user_ap_type = pst_dmac_to_hmac_mimo_compatibility->en_ap_type;
    oal_set_mac_addr(pst_hmac_user->auc_mimo_blacklist_mac, pst_dmac_to_hmac_mimo_compatibility->auc_mac_addr);

    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                     "{hmac_config_mimo_compatibility:AP TYPE(mimo compatibility):%d.}",
                     pst_dmac_to_hmac_mimo_compatibility->en_ap_type);
    if (pst_hmac_user->en_user_ap_type & MAC_AP_TYPE_MIMO_BLACKLIST) {
        /* 发起reassoc req */
        hmac_roam_start(pst_hmac_vap, ROAM_SCAN_CHANNEL_ORG_0, OAL_FALSE, NULL, ROAM_TRIGGER_M2S);
    }

    return OAL_SUCC;
}
#endif

/* 功能描述 : rx ddr 接收数据帧 host侧hal vap与mac vap， hal device 之间的映射关系同步 */
uint32_t hmac_config_d2h_usr_lut_syn(mac_vap_stru *mac_vap, uint8_t len, uint8_t *p_param)
{
    d2h_usr_lut_info_syn_event *p_usr_ldx_info = NULL;
    hal_host_device_stru       *p_hal_dev = NULL;

    p_usr_ldx_info = (d2h_usr_lut_info_syn_event *)p_param;
    if (p_usr_ldx_info->lut_idx >= HAL_MAX_LUT) {
        return OAL_FAIL;
    }

    p_hal_dev = hal_get_host_device(p_usr_ldx_info->hal_dev_id);
    if (p_hal_dev == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, 0, "hmac_config_d2h_usr_lut_syn: hal_get_host_device return null.\n");
        return OAL_FAIL;
    }
    if (!p_usr_ldx_info->valid) {
        p_hal_dev->user_sts_info[p_usr_ldx_info->lut_idx].user_valid = 0;
    } else {
        p_hal_dev->user_sts_info[p_usr_ldx_info->lut_idx].user_valid = 1;
        p_hal_dev->user_sts_info[p_usr_ldx_info->lut_idx].user_id = p_usr_ldx_info->usr_idx;
    }

    oam_warning_log4(mac_vap->uc_vap_id, 0, "hmac_config_d2h_usr_lut_syn::haldev[%d],lut_idx[%d],usr_idx[%d],valid[%d]",
        p_usr_ldx_info->hal_dev_id, p_usr_ldx_info->lut_idx, p_usr_ldx_info->usr_idx, p_usr_ldx_info->valid);

    return OAL_SUCC;
}

static void hmac_config_vap_mib_update(mac_vap_stru *mac_vap, mac_d2h_mib_update_info_stru *mib_update_info)
{
    oal_bool_enum_uint8 en_vht_sgi_is_supp_160_80p80;

    /* 设置带宽mib能力 */
    mac_mib_set_dot11VapMaxBandWidth(mac_vap, mib_update_info->en_wlan_bw_max);
    /* 更新VHT相关mib能力 */
    mac_mib_set_VHTChannelWidthOptionImplemented(mac_vap,
        mac_device_trans_bandwith_to_vht_capinfo(mac_mib_get_dot11VapMaxBandWidth(mac_vap)));
    en_vht_sgi_is_supp_160_80p80 = (mac_mib_get_dot11VapMaxBandWidth(mac_vap) >= WLAN_BW_CAP_160M) ?
        WLAN_HAL0_VHT_SGI_SUPP_160_80P80 : OAL_FALSE;
    mac_mib_set_VHTShortGIOptionIn160and80p80Implemented(mac_vap, en_vht_sgi_is_supp_160_80p80);

#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        mac_vap->en_11ax_hal_cap = mib_update_info->en_11ax_cap;
        if (!mac_vap_is_support_11ax(mac_vap)) {
            mac_mib_set_HEOptionImplemented(mac_vap, OAL_FALSE);
        }
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_d2h_vap_mib_update::en_11ax_hal=[%d],ax_custom=[%d],mib_11ax=[%d]}",
            mac_vap->en_11ax_hal_cap, mac_vap->en_11ax_custom_switch, mac_mib_get_HEOptionImplemented(mac_vap));
    }
#endif

#ifdef _PRE_WLAN_FEATURE_TXOPPS
    mac_mib_set_txopps(mac_vap, mib_update_info->en_txopps_is_supp);
#endif

#ifdef _PRE_WLAN_FEATURE_DFS
    mac_vap_set_support_dfs(mac_vap, mib_update_info->en_radar_detector_cap);
#endif

    /* green filed mib设置 */
    /* 暂时未使用，使用时打开 */
#ifdef _PRE_WLAN_FEATURE_TXBF
    mac_mib_set_VHTMUBeamformeeOptionImplemented(mac_vap, mib_update_info->en_mu_beamformee_cap);

    mac_mib_set_VHTNumberSoundingDimensions(mac_vap, mib_update_info->uc_bfer_num_sounding_dim);
    mac_mib_set_VHTSUBeamformeeOptionImplemented(mac_vap, mib_update_info->en_su_bfee);
    /* 设置su_bfee能力:bfee支持的接收空时流数 */
    mac_mib_set_VHTBeamformeeNTxSupport(mac_vap, mib_update_info->uc_su_bfee_num);
#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        mac_mib_set_he_SUBeamformee(mac_vap, mib_update_info->en_su_bfee);
        mac_mib_set_he_BeamformeeSTSBelow80Mhz(mac_vap, mib_update_info->uc_he_ntx_sts_below_80m);
        mac_mib_set_he_BeamformeeSTSOver80Mhz(mac_vap, mib_update_info->uc_he_ntx_sts_over_80m);
        mac_mib_set_HENumberSoundingDimensionsBelow80Mhz(mac_vap, mib_update_info->uc_he_num_dim_below_80m);
        mac_mib_set_HENumberSoundingDimensionsOver80Mhz(mac_vap, mib_update_info->uc_he_num_dim_over_80m);
    }
#endif
#endif

    /*  update dbac GO beacon interval(follow STA) */
    mac_mib_set_BeaconPeriod(mac_vap, mib_update_info->us_beacon_period);

    /*  设置shortgi能力 */
    mac_mib_set_ShortGIOptionInFortyImplemented(mac_vap, mib_update_info->en_40m_shortgi);

    /* 设置40M使能MIB值 */
    mac_mib_set_FortyMHzOperationImplemented(mac_vap, mib_update_info->en_40m_enable);

    /* 设置11n txbf 的能力 */
    mac_vap->st_cap_flag.bit_11ntxbf = mib_update_info->en_11n_txbf;
}

/*
 * 函 数 名  : hmac_config_d2h_vap_mib_update
 * 功能描述  : 同步dmac 的mib能力
 * 1.日    期  : 2017年6月26日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_d2h_vap_mib_update(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    mac_d2h_mib_update_info_stru *mib_update_info = NULL;
    mac_device_stru *mac_dev = NULL;

    if ((mac_vap->uc_init_flag == MAC_VAP_INVAILD) || oal_any_null_ptr2(mac_vap->pst_mib_info, param)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_d2h_vap_mib_update::flag[%d] para null!}", mac_vap->uc_init_flag);
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_dev = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_dev == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
            "{dmac_config_vap_mib_update::mac_dev[%d] null.}", mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    mib_update_info = (mac_d2h_mib_update_info_stru *)param;
    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_d2h_vap_mib_update::11ax_cap=%d,radar_cap=%d \
        lan_bw_max=%d, beacon_period=%d}", mib_update_info->en_11ax_cap,mib_update_info->en_radar_detector_cap,
        mib_update_info->en_wlan_bw_max, mib_update_info->us_beacon_period);

    hmac_config_vap_mib_update(mac_vap, mib_update_info);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_d2h_vap_cap_update
 * 功能描述  : 同步dmac 的cap能力
 * 1.日    期  : 2017年6月26日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_d2h_vap_cap_update(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_cap_flag_stru *pst_mac_cap_flag;

    pst_mac_cap_flag = (mac_cap_flag_stru *)(puc_param);
    pst_mac_vap->st_cap_flag.bit_1024qam = !!pst_mac_cap_flag->bit_1024qam;
    pst_mac_vap->st_cap_flag.bit_nb = pst_mac_cap_flag->bit_nb;

    /* 2g5g定制化能力刷新 */
    pst_mac_vap->st_cap_flag.bit_2g_custom_siso = pst_mac_cap_flag->bit_2g_custom_siso;
    pst_mac_vap->st_cap_flag.bit_5g_custom_siso = pst_mac_cap_flag->bit_5g_custom_siso;
    pst_mac_vap->st_cap_flag.bit_bt20dbm = pst_mac_cap_flag->bit_bt20dbm;

    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_d2h_vap_cap_update::1024_cap=[%d],nb_cap=[%d],2g_custom_siso[%d],bt20dbm[%d].}",
                     pst_mac_vap->st_cap_flag.bit_1024qam, pst_mac_vap->st_cap_flag.bit_nb,
                     pst_mac_vap->st_cap_flag.bit_2g_custom_siso, pst_mac_vap->st_cap_flag.bit_bt20dbm);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_d2h_vap_channel_info
 * 功能描述  : 同步dmac 的信道信息
 * 1.日    期  : 2018年6月08日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_d2h_vap_channel_info(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_cfg_channel_param_stru *pst_channel_param;
    uint32_t ret;

    pst_channel_param = (mac_cfg_channel_param_stru *)puc_param;
    pst_mac_vap->st_channel.en_band = pst_channel_param->en_band;
    pst_mac_vap->st_channel.en_bandwidth = pst_channel_param->en_bandwidth;
    pst_mac_vap->st_channel.uc_chan_number = pst_channel_param->uc_channel;
    pst_mac_vap->st_channel.ext6g_band = pst_channel_param->ext6g_band;
    ret = mac_get_channel_idx_from_num(pst_channel_param->en_band,
                                       pst_channel_param->uc_channel,
                                       pst_channel_param->ext6g_band, &pst_mac_vap->st_channel.uc_chan_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "hmac_config_d2h_vap_channel_info::mac_get_channel_idx_from_num fail!");
    }

    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "hmac_config_d2h_vap_channel_info::chan[%d] band[%d] bandwidth[%d]",
                     pst_mac_vap->st_channel.uc_chan_number,
                     pst_mac_vap->st_channel.en_band, pst_mac_vap->st_channel.en_bandwidth);
    return OAL_SUCC;
}
uint32_t hmac_config_d2h_rx_mode_syn(mac_vap_stru *mac_vap, uint8_t len, uint8_t *p_param)
{
    mac_rx_switch_stru  *rx_switch = (mac_rx_switch_stru *)p_param;
    hal_host_device_stru *hal_device  = hal_get_host_device(rx_switch->hal_device_id);
    if (hal_device == NULL) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_RX, "hmac_config_d2h_rx_mode_syn::hal device[%d]null ",
                       rx_switch->hal_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (hal_device->rx_q_inited == OAL_FALSE && rx_switch->new_rx_mode == HAL_RAM_RX) {
        return OAL_SUCC;  /* 未初始化时默认值为HAL_RAM_RX，无需切换 */
    }

    if (rx_switch->new_rx_mode >= HAL_SWTICH_ING || oal_atomic_read(&hal_device->rx_mode) == rx_switch->new_rx_mode) {
        oam_error_log2(rx_switch->hal_device_id, OAM_SF_RX, "hmac_config_d2h_rx_mode_syn:: \
            new[%d]==old[%d][0:ddr, 1:ram]", rx_switch->new_rx_mode, oal_atomic_read(&hal_device->rx_mode));
    }

    oam_warning_log3(rx_switch->hal_device_id, OAM_SF_RX, "hmac_config_d2h_rx_mode_syn:: \
        switch rx from %d to %d [0:ddr, 1:ram]once reset smac%d",
        oal_atomic_read(&hal_device->rx_mode), rx_switch->new_rx_mode, rx_switch->once_ddr_reset_smac);
    hal_pm_try_wakeup_forbid_sleep();
    if (rx_switch->new_rx_mode == HAL_DDR_RX) {
        /* DEVICE reset过host rx ring */
        if (rx_switch->once_ddr_reset_smac == OAL_TRUE) {
            hal_host_rx_reset_smac_handler(hal_device);
        }
        /* host完成接收的准备工作,抛事件到device使能DDR接收 */
        if (hal_rx_host_start_dscr_queue(rx_switch->hal_device_id) != OAL_SUCC) {
            hal_pm_try_wakeup_allow_sleep();
            return OAL_FAIL;
        }
        hmac_config_send_event(mac_vap, WLAN_CFGID_RX_MODE_SWITCH, sizeof(mac_rx_switch_stru), p_param);
    }

    /* 统一host切换接收模式，防止d2h没内存抛事件失败导致上下状态不一致 */
    oal_atomic_set(&(hal_device->rx_mode), rx_switch->new_rx_mode);
    hal_pm_try_wakeup_allow_sleep();
    /* 触发TX 切换 */
    hmac_tx_switch(rx_switch->new_rx_mode);
    return OAL_SUCC;
}
uint32_t hmac_config_d2h_trigger_reassoc(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "hmac_config_d2h_trigger_reassoc: pst_hmac_vap is null ptr.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 发起reassoc req */
    return hmac_roam_start(pst_hmac_vap, ROAM_SCAN_CHANNEL_ORG_0, OAL_FALSE, NULL, ROAM_TRIGGER_D2H_REASSOC);
}

uint32_t hmac_config_d2h_sta_csa_status_notify(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    hmac_vap_stru *hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);

    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_d2h_sta_csa_status_notify: hmac_vap is null ptr.");
        return OAL_ERR_CODE_PTR_NULL;
    }
    hmac_vap->sta_csa_switching = *param;
    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_d2h_sta_csa_status_notify:csa switching[%d]",
        hmac_vap->sta_csa_switching);

    return OAL_SUCC;
}
