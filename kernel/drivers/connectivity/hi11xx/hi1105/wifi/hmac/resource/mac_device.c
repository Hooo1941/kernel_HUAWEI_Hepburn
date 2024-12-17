/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : board、chip、device对应操作函数实现的源文件
 * 作    者 :
 * 创建日期 : 2012年10月19日
 */

#include "mac_device.h"
#include "pcie_host.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "mac_resource.h"
#include "mac_regdomain.h"
#ifdef _PRE_WLAN_FEATURE_HIEX
#include "mac_hiex.h"
#endif
#ifdef _PRE_WLAN_FEATURE_DFS
#include "hmac_dfs.h"
#endif
#include "mac_mib.h"
#include "wlan_chip.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_DEVICE_C

/* mac模块板子的全局控制变量 */
mac_board_stru g_st_mac_board;

/* 动态切换pcie定制化结果 */
uint8_t g_dyn_pcie_switch = 0;
#ifdef _PRE_WLAN_FEATURE_WMMAC
oal_bool_enum_uint8 g_en_wmmac_switch = OAL_TRUE;
#endif

/* 这里指的是每个chip上mac device的频道能力 */
uint8_t g_auc_mac_device_radio_cap[WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP] = {
    MAC_DEVICE_CAP_5G,
};

uint32_t g_ddr_freq = 0;
mac_device_custom_cfg_stru g_st_mac_device_custom_cfg;
/* bit0~bit1:icmpv6 ra 报文过滤|NAN */
uint8_t g_optimized_feature_switch_bitmap = 0x3;
uint8_t g_rx_filter_frag = OAL_FALSE;
#ifdef _PRE_WLAN_CHBA_MGMT
uint8_t g_chba_init_social_channel = 36;
uint8_t g_chba_social_chan_cap = 0;
#endif
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
/* 组播聚合开关 */
mac_mcast_ampdu_cfg_stru g_mcast_ampdu_cfg = {
    .mcast_ampdu_enable = OAL_FALSE, /* 默认不进行组播聚合 */
    .mcast_ampdu_bitmap = 0x1, /* 暂只支持AP模式下组播聚合 */
    .mcast_ampdu_retry = 1, /* 默认重传1次 */
};
#endif
oal_bool_enum_uint8 g_pt_mcast_enable = OAL_FALSE;
uint32_t g_expand_feature_switch_bitmap = 0; /* 扩展特殊开关 */

/* 每个chip下的mac device部分能力定制化 */
/* 02和51每个chip下只有1个device，51双芯片时每个chip的能力是相等的 */
/* mp13 DBDC开启时mac device个数取2(考虑静态DBDC), 否则取1 */
/* 默认初始化值为动态DBDC，取HAL Device0的能力赋值 */
OAL_STATIC mac_device_capability_stru g_st_mac_device_capability[WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP] = {
    {
        /* nss num */
        WLAN_HAL0_NSS_NUM,      /* 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
        /* mib_bw_supp_width */
        WLAN_HAL0_BW_MAX_WIDTH,
        /* NB */
        WLAN_HAL0_NB_IS_EN,
        /* 1024QAM */
        WLAN_HAL0_1024QAM_IS_EN,

        /* 80211 MC */
        WLAN_HAL0_11MC_IS_EN,
        /* ldpc coding */
        WLAN_HAL0_LDPC_IS_EN,
        /* tx stbc */
        WLAN_HAL0_TX_STBC_IS_EN, /* 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
        /* rx stbc */
        WLAN_HAL0_RX_STBC_IS_EN, /* 该值需与hal device保持一致，定制化打开后须统一刷成一致 */

        /* su bfer */
        WLAN_HAL0_SU_BFER_IS_EN, /* 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
        /* su bfee */
        WLAN_HAL0_SU_BFEE_IS_EN, /* 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
        /* mu bfer */
        WLAN_HAL0_MU_BFER_IS_EN, /* 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
        /* mu bfee */
        WLAN_HAL0_MU_BFEE_IS_EN, /* 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
        /* 11ax */
        WLAN_HAL0_11AX_IS_EN,
    },
};
mac_device_capability_stru *g_pst_mac_device_capability = &g_st_mac_device_capability[0];

uint32_t mac_board_init(void)
{
    memset_s(&g_st_mac_board, sizeof(mac_board_stru), 0, sizeof(mac_board_stru));
    return OAL_SUCC;
}

uint8_t mac_is_support_ipv6_filter(void)
{
    return ((g_expand_feature_switch_bitmap & CUSTOM_PM_FILTER_IPV6) != 0);
}

uint8_t mac_get_dyn_pcie_switch(void)
{
    return g_dyn_pcie_switch & 0x3; /* 对应定制化，BIT0代表STA，BIT1 代表GC */
}
uint8_t mac_dyn_pcie_switch_thpt(void)
{
    return (g_dyn_pcie_switch & BIT2) | (g_dyn_pcie_switch & BIT3); /* 随流量切定制化，BIT2使用流量门限，BIT3跟随ring */
}
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
/*
 * 函 数 名  : mac_get_mcast_ampdu_switch
 * 功能描述  : 判断是否支持组播聚合
 * 1.日    期  : 2020年1月14日
 *   作    者  : wifi7
 *   修改内容  : 新生成函数
 */
oal_bool_enum mac_get_mcast_ampdu_switch(void)
{
    return ((g_mcast_ampdu_cfg.mcast_ampdu_enable) &&
        (g_mcast_ampdu_cfg.mcast_ampdu_bitmap & BIT(MCAST_AMPDU_APUT)));
}
#endif

/*
 * 函 数 名  : mac_random_mac_oui_init
 * 功能描述  : 初始化随机mac oui为0
 * 1.日    期  : 2020年1月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_random_mac_oui_init(mac_device_stru *pst_mac_device)
{
    /* 初始化随机mac oui为0(3个字节都是0),
     * 确保只有系统下发有效mac oui才进行随机mac地址扫描(在随机mac扫描开关打开的情况下)
     */
    pst_mac_device->en_is_random_mac_addr_scan = OAL_FALSE;
    pst_mac_device->auc_mac_oui[BYTE_OFFSET_0] = 0x0;
    pst_mac_device->auc_mac_oui[BYTE_OFFSET_1] = 0x0;
    pst_mac_device->auc_mac_oui[BYTE_OFFSET_2] = 0x0;
}

#ifdef _PRE_WLAN_FEATURE_HIEX
/*
 * 函 数 名  : mac_hiex_cap_init
 * 功能描述  : 初始化mac device hiex cap
 * 1.日    期  : 2020年1月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_hiex_cap_init(mac_device_stru *pst_mac_device)
{
    if (memcpy_s(&pst_mac_device->st_hiex_cap, sizeof(mac_hiex_cap_stru), &g_st_default_hiex_cap,
        sizeof(mac_hiex_cap_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "mac_hiex_cap_init::hiex cap memcpy fail!");
    }
}
#endif

OAL_STATIC uint32_t mac_device_init_protocol_cand_cap(mac_device_stru *pst_mac_device,
    uint32_t chip_ver, uint8_t uc_device_id)
{
#ifdef _PRE_WLAN_FEATURE_11AX
    pst_mac_device->en_protocol_cap = g_wlan_spec_cfg->feature_11ax_is_open ?
        WLAN_PROTOCOL_CAP_HE : WLAN_PROTOCOL_CAP_VHT;
#endif

    pst_mac_device->en_band_cap =
        mac_device_band_is_support(uc_device_id, MAC_DEVICE_CAP_5G) ? WLAN_BAND_CAP_2G_5G : WLAN_BAND_CAP_2G;

    return OAL_SUCC;
}
/*
 * 函 数 名  : mac_device_init_params
 * 功能描述  : 初始化device级别参数（降低函数行数）
 * 1.日    期  : 2020年04月18日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
static void mac_device_init_params(mac_device_stru *pst_mac_device)
{
    pst_mac_device->en_max_bandwidth = WLAN_BAND_WIDTH_BUTT;
    pst_mac_device->en_max_band = WLAN_BAND_BUTT;
    pst_mac_device->uc_max_channel = 0;
    pst_mac_device->beacon_interval = WLAN_BEACON_INTVAL_DEFAULT;

    pst_mac_device->st_bss_id_list.us_num_networks = 0;

    pst_mac_device->uc_wapi = OAL_FALSE;
    pst_mac_device->debug_hw_wapi = OAL_TRUE; /* MP16C配置命令使用，默认使能硬件WAPI加解密 */
    mutex_init(&pst_mac_device->pcie_switch_lock);
}

OAL_STATIC void mac_device_p2p_info_init(mac_p2p_info_stru *p_p2p_info)
{
    p_p2p_info->uc_p2p_device_num = 0;
    p_p2p_info->uc_p2p_goclient_num = 0;
    p_p2p_info->pst_primary_net_device = NULL; /* 初始化主net_device 为空指针 */
    p_p2p_info->p2p_scenes = MAC_P2P_SCENES_LOW_LATENCY;
}

void mac_device_init_dyn_pcie(mac_device_stru *mac_device)
{
    mac_device->pcie_switch_vote = HMAC_PCIE_SWITCH_IDLE;
    mac_device->cur_pcie_state = g_wlan_spec_cfg->pcie_speed;
    mac_device->pm_work_disable = OAL_FALSE;
    mac_device->sta_pcie_probe = WLAN_PCIE_PROBE_DEFAULT; // 去关联一定要刷回默认值
    mac_device->gc_pcie_probe = WLAN_PCIE_PROBE_DEFAULT;
    mac_device->pre_host_mac_irq_mask = 0xffffffff;
    if (mac_device->pcie_up_recover_timer.en_is_registerd == OAL_TRUE) {
        frw_timer_immediate_destroy_timer_m(&mac_device->pcie_up_recover_timer);
    }
}

/*
 * 函 数 名  : mac_device_init
 * 功能描述  : 初始化device级别参数
 * 1.日    期  : 2013年8月8日
 *   修改内容  : 新生成函数
 */
uint32_t mac_device_init(mac_device_stru *pst_mac_device, uint32_t chip_ver, uint8_t uc_chip_id, uint8_t uc_device_id)
{
    uint8_t uc_device_id_per_chip;

    memset_s(pst_mac_device, sizeof(mac_device_stru), 0, sizeof(mac_device_stru));

    /* 初始化device的索引 */
    pst_mac_device->uc_chip_id = uc_chip_id;
    pst_mac_device->uc_device_id = uc_device_id;

    /* 初始化device级别的一些参数 */
    mac_device_init_params(pst_mac_device);
    mac_device_init_dyn_pcie(pst_mac_device);

    /* mp13两个业务device,00 01,取不同定制化,51双芯片00 11,取同一个定制化 */
    uc_device_id_per_chip = uc_device_id - uc_chip_id;
    if (uc_device_id_per_chip >= WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP) {
        oam_error_log3(0, OAM_SF_ANY, "{mac_device_init::pst_mac_device device id[%d] chip id[%d] >support[%d].}",
                       uc_device_id, uc_chip_id, WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP);
        uc_device_id_per_chip = 0;
    }

    /* 将定制化的信息保存到mac device结构体下 */
    /* 初始化mac device的能力 */
    if (memcpy_s(&pst_mac_device->st_device_cap, sizeof(mac_device_capability_stru),
        &g_pst_mac_device_capability[uc_device_id_per_chip], sizeof(mac_device_capability_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "mac_device_init::memcpy fail!");
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_SMPS
    /* 对于mp13来说，另一个device只支持SISO,配置为MIMO SAVE状态是否没有影响，再确认; 这些能力在m2s切换中会动态变化 */
    mac_device_set_mode_smps(pst_mac_device, WLAN_MIB_MIMO_POWER_SAVE_MIMO);
#endif

    pst_mac_device->en_device_state = OAL_TRUE;
    pst_mac_device->en_reset_switch = OAL_FALSE;

    /* 默认关闭wmm,wmm超时计数器设为0 */
    pst_mac_device->en_wmm = OAL_TRUE;

    /* 根据芯片版本初始化device能力信息 */
    if (mac_device_init_protocol_cand_cap(pst_mac_device, chip_ver, uc_device_id) != OAL_SUCC) {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    /* 初始化vap num统计信息 */
    pst_mac_device->uc_vap_num = 0;
    pst_mac_device->uc_sta_num = 0;
    mac_device_p2p_info_init(&pst_mac_device->st_p2p_info);

    /* 初始化默认管制域 */
    mac_init_regdomain();

    /* 初始化信道列表 */
    mac_init_channel_list();

    /* 初始化复位状态 */
    mac_device_set_dfr_reset(pst_mac_device, OAL_FALSE);
    pst_mac_device->us_device_reset_num = 0;

    /* 默认关闭DBAC特性 */
    pst_mac_device->en_dbac_enabled = OAL_TRUE;

    pst_mac_device->en_dbdc_running = OAL_FALSE;
    mac_set_2040bss_switch(pst_mac_device, OAL_FALSE);

    pst_mac_device->uc_arpoffload_switch = OAL_FALSE;

    mac_random_mac_oui_init(pst_mac_device);

#ifdef _PRE_WLAN_FEATURE_DFS
    mac_dfs_init(pst_mac_device);
#endif /* #ifdef _PRE_WLAN_FEATURE_DFS */

#ifdef _PRE_WLAN_FEATURE_HIEX
    mac_hiex_cap_init(pst_mac_device);
#endif

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : mac_device_exit
 功能描述  : 卸载device的操作函数(调用之前，已从chip上删除)
  1.日    期   : 2012年10月19日
    修改内容   : 新生成函数

*****************************************************************************/
uint32_t mac_device_exit(mac_device_stru *pst_device)
{
    if (oal_unlikely(pst_device == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{dmac_device_exit::pst_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_device->uc_vap_num = 0;
    pst_device->uc_sta_num = 0;
    pst_device->st_p2p_info.uc_p2p_device_num = 0;
    pst_device->st_p2p_info.uc_p2p_goclient_num = 0;

    mac_res_free_dev(pst_device->uc_device_id);

    mac_device_set_state(pst_device, OAL_FALSE);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : mac_chip_exit
  1.日    期   : 2012年10月19日
    修改内容   : 新生成函数

*****************************************************************************/
uint32_t mac_chip_exit(mac_board_stru *pst_board, mac_chip_stru *pst_chip)
{
    if (oal_unlikely(pst_chip == NULL || pst_board == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_chip_init::param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_chip->uc_device_nums = 0;

    /* destroy流程最后将状态置为FALSE */
    pst_chip->en_chip_state = OAL_FALSE;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : mac_board_exit
 功能描述  : 释放全局变量包含的结构体
 输入参数  : board对象指针
 输出参数  : 无
 返 回 值  : 成功或失败
 调用函数  : mac_chip_destroy、oal_bit_find_first_bit_one_byte、oal_bit_clear_bit_one_byte
  1.日    期   : 2012年10月19日
    修改内容   : 新生成函数

*****************************************************************************/
uint32_t mac_board_exit(mac_board_stru *pst_board)
{
    if (oal_unlikely(pst_board == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{mac_board_exit::pst_board null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    return OAL_SUCC;
}
void mac_device_set_dfr_reset(mac_device_stru *pst_mac_device, uint8_t uc_device_reset_in_progress)
{
    pst_mac_device->uc_device_reset_in_progress = uc_device_reset_in_progress;
}

void mac_device_set_state(mac_device_stru *pst_mac_device, uint8_t en_device_state)
{
    pst_mac_device->en_device_state = en_device_state;
}

/*
 * 函 数 名  : mac_device_trans_bandwith_to_vht_capinfo
 * 功能描述  : 最大支持带宽换到vht capability info中的带宽
 * 1.日    期  : 2016年11月30日
 *   修改内容  : 新生成函数
 */
wlan_mib_vht_supp_width_enum mac_device_trans_bandwith_to_vht_capinfo(wlan_bw_cap_enum_uint8 en_max_op_bd)
{
    switch (en_max_op_bd) {
        case WLAN_BW_CAP_20M:
        case WLAN_BW_CAP_40M:
        case WLAN_BW_CAP_80M:
            return WLAN_MIB_VHT_SUPP_WIDTH_80;
        case WLAN_BW_CAP_160M:
            return WLAN_MIB_VHT_SUPP_WIDTH_160;
        case WLAN_BW_CAP_80PLUS80:
            return WLAN_MIB_VHT_SUPP_WIDTH_80PLUS80;
        default:
            oam_error_log1(0, OAM_SF_ANY,
                           "{mac_device_trans_bandwith_to_vht_capinfo::bandwith[%d] is invalid.}",
                           en_max_op_bd);
            return WLAN_MIB_VHT_SUPP_WIDTH_BUTT;
    }
}

/*
 * 函 数 名  : mac_device_check_5g_enable
 * 功能描述  : 检查mac device中是否使能5g
 * 1.日    期  : 2016年11月30日
 *   修改内容  : 新生成函数
 */
oal_bool_enum_uint8 mac_device_band_is_support(uint8_t mac_device_id, mac_device_radio_cap_enum_uint8 radio_cap)
{
    uint8_t        device_id_per_chip;
    mac_device_stru *mac_device = mac_res_get_dev(mac_device_id);

    if (mac_device == NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "mac_device_check_5g_enable::get dev fail uc_device_id[%d]", mac_device_id);
        return OAL_FALSE;
    }
    /* mp13两个业务device,00 01,取不同定制化,51双芯片00 11,取同一个定制化 */
    device_id_per_chip = mac_device_id - mac_device->uc_chip_id;
    return ((g_auc_mac_device_radio_cap[device_id_per_chip] & radio_cap) != 0) ? OAL_TRUE : OAL_FALSE;
}

uint32_t mac_chip_init(mac_chip_stru *pst_chip, uint8_t uc_device_max)
{
    pst_chip->uc_assoc_user_cnt = 0;
    pst_chip->uc_active_user_cnt = 0;

    /* 保存device数量 */
    pst_chip->uc_device_nums = uc_device_max;

    /* 初始化最后再将state置为TRUE */
    pst_chip->en_chip_state = OAL_TRUE;

    return OAL_SUCC;
}

void mac_device_set_beacon_interval(mac_device_stru *pst_mac_device, uint32_t beacon_interval)
{
    pst_mac_device->beacon_interval = beacon_interval;
}

void mac_chip_inc_assoc_user(mac_chip_stru *pst_mac_chip)
{
    pst_mac_chip->uc_assoc_user_cnt++;
    /* 添加维测 */
    oam_warning_log1(0, OAM_SF_UM, "{mac_chip_inc_assoc_user::uc_asoc_user_cnt[%d].}", pst_mac_chip->uc_assoc_user_cnt);
    if (pst_mac_chip->uc_assoc_user_cnt == 0xFF) {
        oam_error_log0(0, OAM_SF_UM, "{mac_chip_inc_assoc_user::uc_asoc_user_cnt=0xFF now!}");
        oam_report_backtrace();
    }
}

void mac_chip_dec_assoc_user(mac_chip_stru *pst_mac_chip)
{
    oam_warning_log1(0, OAM_SF_UM, "{mac_chip_dec_assoc_user::uc_asoc_user_cnt[%d].}", pst_mac_chip->uc_assoc_user_cnt);
    if (pst_mac_chip->uc_assoc_user_cnt == 0) {
        oam_error_log0(0, OAM_SF_UM, "{mac_chip_dec_assoc_user::uc_assoc_user_cnt is already zero.}");
        oam_report_backtrace();
    } else {
        pst_mac_chip->uc_assoc_user_cnt--;
    }
}

/*
 * 函 数 名  : mac_device_get_all_rates
 * 功能描述  : 获取所有速率
 * 1.日    期  : 2013年7月5日
 *   修改内容  : 新生成函数
 */
void *mac_device_get_all_rates(mac_device_stru *pst_dev)
{
    return (void *)pst_dev->st_mac_rates_11g;
}

void mac_set_dual_sta_mode(uint8_t mode)
{
    mac_device_stru *mac_device = mac_res_get_dev(0);
    if (mac_device == NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{mac_set_dual_sta_mode::mac_device null.}");
        return;
    }
    mac_device->is_dual_sta_mode = mode;
}

uint8_t mac_is_dual_sta_mode(void)
{
    mac_device_stru *mac_device = mac_res_get_dev(0);
    return mac_device->is_dual_sta_mode;
}

OAL_STATIC void mac_device_add_vap_id(mac_device_stru *mac_device, mac_vap_stru *mac_vap, uint8_t vap_idx,
    wlan_vap_mode_enum_uint8 vap_mode)
{
    uint8_t uc_vap_tmp_idx;
    mac_vap_stru *pst_tmp_vap = NULL;

    mac_device->auc_vap_id[mac_device->uc_vap_num++] = vap_idx;
    if (vap_mode == WLAN_VAP_MODE_BSS_STA) {
        mac_device->uc_sta_num++;
        mac_vap->us_assoc_vap_id = g_wlan_spec_cfg->invalid_user_id;
    }
    mac_inc_p2p_num(mac_vap);
    if (is_p2p_go(mac_vap)) {
        for (uc_vap_tmp_idx = 0; uc_vap_tmp_idx < mac_device->uc_vap_num; uc_vap_tmp_idx++) {
            pst_tmp_vap = (mac_vap_stru *)mac_res_get_mac_vap(mac_device->auc_vap_id[uc_vap_tmp_idx]);
            if (pst_tmp_vap == NULL) {
                oam_error_log1(0, OAM_SF_SCAN, "{mac_device_add_vap_id::pst_mac_vap null,vap_idx=%d.}",
                    mac_device->auc_vap_id[uc_vap_tmp_idx]);
                continue;
            }
            if ((pst_tmp_vap->en_vap_state == MAC_VAP_STATE_UP) && (pst_tmp_vap != mac_vap)) {
                mac_vap->st_channel.en_band        = pst_tmp_vap->st_channel.en_band;
                mac_vap->st_channel.en_bandwidth   = pst_tmp_vap->st_channel.en_bandwidth;
                mac_vap->st_channel.uc_chan_number = pst_tmp_vap->st_channel.uc_chan_number;
                mac_vap->st_channel.uc_chan_idx = pst_tmp_vap->st_channel.uc_chan_idx;
                break;
            }
        }
    }
}

/*
 * 功能描述  : 基于device增量或者删除一个vap，并且维护vap记数
 * 1.日    期  : 2015年5月7日
 *   修改内容  : 新生成函数
 */
void mac_device_set_vap_id(mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap,
    uint8_t uc_vap_idx, wlan_vap_mode_enum_uint8 en_vap_mode,
    wlan_p2p_mode_enum_uint8 en_p2p_mode, uint8_t is_add_vap)
{
    if (is_add_vap) {
        mac_device_add_vap_id(pst_mac_device, pst_mac_vap, uc_vap_idx, en_vap_mode);
    } else {
        /* ?offload???,????HMAC????? */
        pst_mac_device->auc_vap_id[pst_mac_device->uc_vap_num--] = 0;

        /* device?sta???1 */
        if (en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
            pst_mac_device->uc_sta_num--;

            /* ???us_assoc_vap_id??????ap??? */
            pst_mac_vap->us_assoc_vap_id = g_wlan_spec_cfg->invalid_user_id;
        }

        pst_mac_vap->en_p2p_mode = en_p2p_mode;
        mac_dec_p2p_num(pst_mac_vap);
    }
}

/*
 * 功能描述  : 寻找另一个UP状态的VAP
 * 1.日    期  : 2016年7月15日
 *   修改内容  : 新生成函数
 */
mac_vap_stru *mac_device_find_another_up_vap(mac_device_stru *mac_device, uint8_t vap_id_self)
{
    uint8_t vap_idx;
    mac_vap_stru *mac_vap = NULL;

    for (vap_idx = 0; vap_idx < mac_device->uc_vap_num; vap_idx++) {
        mac_vap = mac_res_get_mac_vap(mac_device->auc_vap_id[vap_idx]);
        if (mac_vap == NULL || vap_id_self == mac_vap->uc_vap_id) {
            continue;
        }

        if (mac_vap->en_vap_state == MAC_VAP_STATE_UP || mac_vap->en_vap_state == MAC_VAP_STATE_PAUSE ||
            (mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN && mac_vap->us_user_nums > 0) ||
            (mac_vap->en_vap_state == MAC_VAP_STATE_ROAMING)) {
            return mac_vap;
        }
    }

    return NULL;
}

/*
 * 功能描述  : 计算up的vap个数
 * 1.日    期  : 2014年11月17日
 *   修改内容  : 新生成函数
 */
uint32_t mac_device_calc_up_vap_num(mac_device_stru *pst_mac_device)
{
    mac_vap_stru                  *pst_vap = NULL;
    uint8_t                      uc_vap_idx;
    uint8_t                      up_vap_num = 0;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == NULL) {
            oam_error_log1(0, OAM_SF_ANY, "vap is null, vap id is %d",
                           pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if ((pst_vap->en_vap_state == MAC_VAP_STATE_UP) ||
            (pst_vap->en_vap_state == MAC_VAP_STATE_PAUSE) ||
            (pst_vap->en_vap_state == MAC_VAP_STATE_ROAMING) ||
            (pst_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN && pst_vap->us_user_nums > 0)) {
            up_vap_num++;
        }
    }

    return up_vap_num;
}

/*
 * 功能描述  : 寻找处在UP状态的 P2P_GO
 * 1.日    期  : 2014年11月11日
 *   修改内容  : 新生成函数
 */
uint32_t mac_device_find_up_p2p_go(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap)
{
    uint8_t       uc_vap_idx;
    mac_vap_stru   *pst_mac_vap = NULL;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (oal_unlikely(pst_mac_vap == NULL)) {
            oam_warning_log1(0, OAM_SF_SCAN,
                             "vap is null! vap id is %d",
                             pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if ((pst_mac_vap->en_vap_state == MAC_VAP_STATE_UP || pst_mac_vap->en_vap_state == MAC_VAP_STATE_PAUSE) &&
            (pst_mac_vap->en_p2p_mode == WLAN_P2P_GO_MODE)) {
            *ppst_mac_vap = pst_mac_vap;

            return OAL_SUCC;
        }
    }

    *ppst_mac_vap = NULL;

    return OAL_FAIL;
}

/*
 * 功能描述  : 寻找处在UP状态的两个VAP
 * 1.日    期  : 2014年11月11日
 *   修改内容  : 新生成函数
 */
uint32_t mac_device_find_2up_vap(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap1,
    mac_vap_stru **ppst_mac_vap2)
{
    mac_vap_stru                  *pst_vap = NULL;
    uint8_t                      uc_vap_idx;
    uint8_t                      up_vap_num = 0;
    mac_vap_stru                  *past_vap[NUM_2_BYTES] = {0};

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == NULL) {
            oam_error_log1(0, OAM_SF_ANY, "mac_device_find_2up_vap vap is null, vap id is %d",
                           pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if ((pst_vap->en_vap_state == MAC_VAP_STATE_UP) ||
            (pst_vap->en_vap_state == MAC_VAP_STATE_PAUSE)
            || (pst_vap->en_vap_state == MAC_VAP_STATE_ROAMING)
            || (pst_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN && pst_vap->us_user_nums > 0)) {
            past_vap[up_vap_num] = pst_vap;
            up_vap_num++;

            if (up_vap_num >= 2) { /* 2 vap 个数 */
                break;
            }
        }
    }

    if (up_vap_num < 2) { /* 2 vap 个数 */
        return OAL_FAIL;
    }

    *ppst_mac_vap1 = past_vap[0];
    *ppst_mac_vap2 = past_vap[1];

    return OAL_SUCC;
}

/*
 * 功能描述  : 寻找处在UP状态的STA
 * 1.日    期  : 2015年6月23日
 *   修改内容  : 新生成函数
 */
uint32_t mac_device_find_up_sta(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap)
{
    uint8_t       uc_vap_idx;
    mac_vap_stru   *pst_mac_vap = NULL;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (oal_unlikely(pst_mac_vap == NULL)) {
            oam_warning_log1(0, OAM_SF_SCAN, "vap is null! vap id is %d", pst_mac_device->auc_vap_id[uc_vap_idx]);

            *ppst_mac_vap = NULL;

            return OAL_ERR_CODE_PTR_NULL;
        }

        if ((pst_mac_vap->en_vap_state == MAC_VAP_STATE_UP || pst_mac_vap->en_vap_state == MAC_VAP_STATE_PAUSE) &&
            (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA)) {
            *ppst_mac_vap = pst_mac_vap;

            return OAL_SUCC;
        }
    }

    *ppst_mac_vap = NULL;

    return OAL_FAIL;
}

/*
 * 功能描述  : 寻找处在UP状态的VAP
 * 1.日    期  : 2014年11月11日
 *   修改内容  : 新生成函数
 */
uint32_t mac_device_find_up_vap(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap)
{
    uint8_t       uc_vap_idx;
    mac_vap_stru   *pst_mac_vap = NULL;

    if (pst_mac_device == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (oal_unlikely(pst_mac_vap == NULL)) {
            oam_warning_log1(0, OAM_SF_SCAN, "mac_device_find_up_vap::vap is null! vap id is %d",
                pst_mac_device->auc_vap_id[uc_vap_idx]);

            *ppst_mac_vap = NULL;
            return OAL_ERR_CODE_PTR_NULL;
        }

        if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_UP ||
            pst_mac_vap->en_vap_state == MAC_VAP_STATE_PAUSE ||
            (pst_mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN && pst_mac_vap->us_user_nums > 0) ||
            (pst_mac_vap->en_vap_state == MAC_VAP_STATE_ROAMING)) {
            *ppst_mac_vap = pst_mac_vap;

            return OAL_SUCC;
        }
    }

    *ppst_mac_vap = NULL;

    return OAL_FAIL;
}

/*
 * 功能描述  : 寻找处在入网状态的sta,用于ap启动时，如果有STA入网会造成device DBDC/DBAC状态异常，AP只有up/wait_start态无需查找
 */
uint32_t mac_device_find_connecting_sta(mac_device_stru *mac_device, mac_vap_stru **mac_vap_array,
    uint32_t vap_array_num)
{
    uint8_t  vap_idx;
    uint32_t up_vap_num = 0;
    mac_vap_stru *mac_vap = NULL;

    if (mac_device == NULL) {
        return up_vap_num;
    }

    for (vap_idx = 0; vap_idx < mac_device->uc_vap_num; vap_idx++) {
        mac_vap = mac_res_get_mac_vap(mac_device->auc_vap_id[vap_idx]);
        if (oal_unlikely(mac_vap == NULL)) {
            oam_warning_log1(0, OAM_SF_SCAN, "mac_device_find_join_vaps::vap is null! vap id is %d",
                mac_device->auc_vap_id[vap_idx]);
            continue;
        }
        if (mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA) {
            continue;
        }
        if (hmac_sta_is_connecting(mac_vap->en_vap_state) || (mac_vap->en_vap_state == MAC_VAP_STATE_ROAMING)) {
            if (up_vap_num < vap_array_num) {
                mac_vap_array[up_vap_num] = mac_vap;
                up_vap_num++;
            } else {
                oam_error_log2(0, OAM_SF_ANY,
                    "mac_device_find_join_vaps::up vap number[%d] bigger than input array num [%d]",
                    mac_device_calc_up_vap_num(mac_device), vap_array_num);
            }
        }
    }

    return up_vap_num;
}

/*
 * 功能描述  : 寻找处在UP状态的所有VAP
 * 1.日    期  : 2014年11月11日
 *   修改内容  : 新生成函数
 */
uint32_t mac_device_find_up_vaps(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap, uint32_t vap_array_num)
{
    uint8_t  uc_vap_idx;
    uint32_t up_vap_num = 0;
    mac_vap_stru *mac_vap = NULL;

    if (pst_mac_device == NULL) {
        return up_vap_num;
    }

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (oal_unlikely(mac_vap == NULL)) {
            oam_warning_log1(0, OAM_SF_SCAN, "mac_device_find_up_vaps::vap is null! vap id is %d",
                pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if (mac_vap->en_vap_state == MAC_VAP_STATE_UP ||
            mac_vap->en_vap_state == MAC_VAP_STATE_PAUSE ||
            mac_vap->en_vap_state == MAC_VAP_STATE_ROAMING ||
            (mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN && mac_vap->us_user_nums > 0)) {
            if (up_vap_num < vap_array_num) {
                ppst_mac_vap[up_vap_num] = mac_vap;
                up_vap_num++;
            } else {
                oam_error_log2(0, OAM_SF_ANY,
                    "mac_device_find_up_vaps::up vap number[%d] bigger than input array num [%d]",
                    mac_device_calc_up_vap_num(pst_mac_device), vap_array_num);
            }
        }
    }

    return up_vap_num;
}

/*
 * 功能描述  : 寻找处在UP状态的AP VAP
 * 1.日    期  : 2015年4月15日
 *   修改内容  : 新生成函数
 */
uint32_t mac_device_find_up_ap(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap)
{
    uint8_t       uc_vap_idx;
    mac_vap_stru   *pst_mac_vap = NULL;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (oal_unlikely(pst_mac_vap == NULL)) {
            oam_warning_log1(0, OAM_SF_SCAN,
                             "vap is null! vap id is %d",
                             pst_mac_device->auc_vap_id[uc_vap_idx]);
            return OAL_ERR_CODE_PTR_NULL;
        }

        if ((pst_mac_vap->en_vap_state == MAC_VAP_STATE_UP || pst_mac_vap->en_vap_state == MAC_VAP_STATE_PAUSE) &&
            (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP)) {
            *ppst_mac_vap = pst_mac_vap;

            return OAL_SUCC;
        }
    }

    *ppst_mac_vap = NULL;

    return OAL_FAIL;
}

/*
 * 功能描述  : 计算work的vap个数
 * 1.日    期  : 2015年12月2日
 *   修改内容  : 新生成函数
 */
uint32_t mac_device_calc_work_vap_num(mac_device_stru *pst_mac_device)
{
    mac_vap_stru    *pst_vap = NULL;
    uint8_t        uc_vap_idx;
    uint8_t        work_vap_num = 0;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == NULL) {
            oam_error_log1(0, OAM_SF_ANY, "mac_device_calc_work_vap_numv::vap[%d] is null",
                           pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if ((pst_vap->en_vap_state != MAC_VAP_STATE_INIT) && (pst_vap->en_vap_state != MAC_VAP_STATE_BUTT)) {
            work_vap_num++;
        }
    }

    return work_vap_num;
}


/*
 * 功能描述  : 计算up的vap个数
 * 1.日    期  : 2016年6月17日
 *   修改内容  : 新生成函数
 */
uint32_t mac_device_get_up_vap_num(mac_device_stru *pst_mac_device)
{
    mac_vap_stru                  *pst_vap = NULL;
    uint8_t                      uc_vap_idx;
    uint8_t                      up_vap_num = 0;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == NULL) {
            oam_error_log1(0, OAM_SF_ANY, "vap is null, vap id is %d",
                           pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if (pst_vap->en_vap_state == MAC_VAP_STATE_UP) {
            up_vap_num++;
        }
    }

    return up_vap_num;
}

/*
 * 功能描述  : 判断p2p设备是否关连
 * 1.日    期  : 2015年4月1日
 *   修改内容  : 新生成函数
 */
uint32_t mac_device_is_p2p_connected(mac_device_stru *pst_mac_device)
{
    uint8_t       uc_vap_idx;
    mac_vap_stru   *pst_mac_vap = NULL;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (oal_unlikely(pst_mac_vap == NULL)) {
            oam_warning_log1(0, OAM_SF_P2P, "vap is null! vap id is %d",
                             pst_mac_device->auc_vap_id[uc_vap_idx]);
            return OAL_ERR_CODE_PTR_NULL;
        }
        if ((is_p2p_go(pst_mac_vap) || is_p2p_cl(pst_mac_vap)) &&
            (pst_mac_vap->us_user_nums > 0)) {
            return OAL_SUCC;
        }
    }
    return OAL_FAIL;
}

#ifdef _PRE_WLAN_FEATURE_SMPS
/*
 * 功能描述  : DEVICE SMPS能力是否可以配置
 * 1.日    期  : 2016年9月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t mac_device_find_smps_mode_en(mac_device_stru *pst_mac_device,
    wlan_mib_mimo_power_save_enum en_smps_mode)
{
    mac_vap_stru *pst_mac_vap = NULL;
    uint8_t     uc_vap_idx;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_mac_vap == NULL) {
            oam_error_log0(0, OAM_SF_SMPS, "{mac_device_find_smps_mode_en::pst_mac_vap null.}");
            continue;
        }

        /* 存在不支持HT则MIMO，模式不可配置 */
        if (OAL_FALSE == mac_mib_get_HighThroughputOptionImplemented(pst_mac_vap)) {
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_SMPS,
                             "{mac_device_find_smps_mode_en::exist none ht vap.}");
            return OAL_FALSE;
        }

        /* 存在一个vap MIB不支持SMPS则MIMO */
        if (mac_vap_get_smps_mode(pst_mac_vap) == WLAN_MIB_MIMO_POWER_SAVE_BUTT) {
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_SMPS,
                             "{mac_device_find_smps_mode_en::exist no support SMPS vap.}");
            return OAL_FALSE;
        }

        /* 获取当前SMPS模式，若未改变则直接返回，模式不可配置来处理(vap和device smps mode始终保持一致) */
        if (en_smps_mode == mac_vap_get_smps_mode(pst_mac_vap)) {
            oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_SMPS,
                             "{mac_device_find_smps_mode_en::vap smps mode[%d]unchanged smps mode[%d].}",
                             pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MIMOPowerSave, en_smps_mode);
            return OAL_FALSE;
        }
    }

    return OAL_TRUE;
}
#endif

void mac_device_set_channel(mac_device_stru *pst_mac_device, mac_cfg_channel_param_stru *pst_channel_param)
{
    pst_mac_device->uc_max_channel = pst_channel_param->uc_channel;
    pst_mac_device->en_max_band = pst_channel_param->en_band;
    pst_mac_device->en_max_bandwidth = pst_channel_param->en_bandwidth;
}

void mac_device_get_channel(mac_device_stru *pst_mac_device, mac_cfg_channel_param_stru *pst_channel_param)
{
    pst_channel_param->uc_channel = pst_mac_device->uc_max_channel;
    pst_channel_param->en_band = pst_mac_device->en_max_band;
    pst_channel_param->en_bandwidth = pst_mac_device->en_max_bandwidth;
}

/*
 * 函 数 名  : hmac_m2s_sync_event
 * 功能描述  : dmac同步hmac做m2s切换
 * 1.日    期  : 2017年1月23日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_m2s_sync_event(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event;
    dmac_to_hmac_m2s_event_stru *pst_m2s_event;
    mac_device_stru *pst_mac_device;

    pst_event = frw_get_event_stru(pst_event_mem);

    pst_m2s_event = (dmac_to_hmac_m2s_event_stru *)pst_event->auc_event_data;

    pst_mac_device = mac_res_get_dev(pst_m2s_event->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log1(0, OAM_SF_M2S, "{hmac_m2s_sync_event: mac device is null ptr. device id:%d}",
                       pst_m2s_event->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (WLAN_M2S_TYPE_HW == pst_m2s_event->en_m2s_type) {
        /* 硬切换更换mac device的nss能力 */
        mac_device_set_nss_num(pst_mac_device, pst_m2s_event->en_m2s_nss);
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_init_event_process
 * 功能描述  : dmac 抛给hmac的初始化回复事件
 * 1.日    期  : 2013年1月15日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_init_event_process(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL; /* 事件结构体 */
    mac_data_rate_stru *pst_data_rate = NULL;
    dmac_tx_event_stru *pst_ctx_event = NULL;
    mac_device_stru *pst_mac_device = NULL;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_init_event_process::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    pst_ctx_event = (dmac_tx_event_stru *)pst_event->auc_event_data;
    pst_data_rate = (mac_data_rate_stru *)(oal_netbuf_data(pst_ctx_event->pst_netbuf));

    /* 同步mac支持的速率集信息 */
    pst_mac_device = mac_res_get_dev(pst_event->st_event_hdr.uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_init_event_process::pst_mac_device null.}");
        oal_netbuf_free(pst_ctx_event->pst_netbuf);
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    if (memcpy_s((uint8_t *)(pst_mac_device->st_mac_rates_11g), sizeof(pst_mac_device->st_mac_rates_11g),
        (uint8_t *)pst_data_rate, sizeof(mac_data_rate_stru) * MAC_DATARATES_PHY_80211G_NUM) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_init_event_process::memcpy fail!");
        /* 释放掉02同步消息所用的netbuf信息 */
        oal_netbuf_free(pst_ctx_event->pst_netbuf);
        return OAL_FAIL;
    }

    /* 释放掉02同步消息所用的netbuf信息 */
    oal_netbuf_free(pst_ctx_event->pst_netbuf);
    return OAL_SUCC;
}

void mac_device_reset_state_flag(void)
{
    mac_device_stru *mac_device = mac_res_get_dev(0);
    if (mac_device == NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{mac_device_reset_state_flag::mac_device null.}");
        return;
    }
    mac_device->en_dbac_running = OAL_FALSE;
    mac_device->en_dbdc_running = OAL_FALSE;
}
