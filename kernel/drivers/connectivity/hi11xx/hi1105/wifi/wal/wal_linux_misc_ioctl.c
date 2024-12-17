/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : linux ioctl配置调试命令
 * 作    者 : wifi
 * 创建日期 : 2016年4月8日
 */

#include "oal_cfg80211.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"

#include "wlan_spec.h"
#include "wlan_types.h"
#include "wlan_chip_i.h"
#include "mac_vap.h"
#include "mac_regdomain.h"
#include "mac_ie.h"

#include "hmac_resource.h"
#include "hmac_chan_mgmt.h"
#include "hmac_tx_data.h"
#include "hmac_config.h"
#include "wal_config_alg_cfg.h"
#include "wal_main.h"
#include "wal_ext_if.h"
#include "wal_config.h"
#include "wal_regdb.h"
#include "wal_linux_scan.h"
#include "wal_linux_ioctl.h"
#include "wal_linux_customize.h"
#include "wal_linux_cfg80211.h"
#include "wal_linux_cfg80211_band.h"
#include "wal_linux_cfgvendor.h"
#include "wal_linux_event.h"

#include "hmac_roam_main.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "oal_net.h"
#ifdef _PRE_WLAN_COUNTRYCODE_SELFSTUDY
#include "hisi_customize_wifi.h"
#endif
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#include "plat_firmware.h"
#endif
#ifdef _PRE_WLAN_FEATURE_DFS
#include "hmac_dfs.h"
#endif
#ifdef _PRE_WLAN_FEATURE_DFR
#include "hmac_dfx.h"
#endif
#include "securec.h"
#include "securectype.h"
#include "hmac_hid2d.h"
#ifdef _PRE_WLAN_CHBA_MGMT
#include "hmac_chba_coex.h"
#include "wal_hipriv_chba.h"
#endif
#ifdef _PRE_WLAN_FEATURE_11D
#include "hisi_customize_config_priv_mp13.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID      OAM_FILE_ID_WAL_LINUX_MISC_IOCTL_C

#define IEEE80211_REGDOMAIN_REG_RULES_SIZE 10
const wal_dfs_domain_entry_stru g_ast_dfs_domain_table[] = {
    { "AE", MAC_DFS_DOMAIN_ETSI },
    { "AL", MAC_DFS_DOMAIN_NULL },
    { "AM", MAC_DFS_DOMAIN_ETSI },
    { "AN", MAC_DFS_DOMAIN_ETSI },
    { "AR", MAC_DFS_DOMAIN_FCC },
    { "AT", MAC_DFS_DOMAIN_ETSI },
    { "AU", MAC_DFS_DOMAIN_FCC },
    { "AZ", MAC_DFS_DOMAIN_ETSI },
    { "BA", MAC_DFS_DOMAIN_ETSI },
    { "BE", MAC_DFS_DOMAIN_ETSI },
    { "BG", MAC_DFS_DOMAIN_ETSI },
    { "BH", MAC_DFS_DOMAIN_ETSI },
    { "BL", MAC_DFS_DOMAIN_NULL },
    { "BN", MAC_DFS_DOMAIN_ETSI },
    { "BO", MAC_DFS_DOMAIN_ETSI },
    { "BR", MAC_DFS_DOMAIN_FCC },
    { "BY", MAC_DFS_DOMAIN_ETSI },
    { "BZ", MAC_DFS_DOMAIN_ETSI },
    { "CA", MAC_DFS_DOMAIN_FCC },
    { "CH", MAC_DFS_DOMAIN_ETSI },
    { "CL", MAC_DFS_DOMAIN_NULL },
    { "CN", MAC_DFS_DOMAIN_CN },
    { "CO", MAC_DFS_DOMAIN_FCC },
    { "CR", MAC_DFS_DOMAIN_FCC },
    { "CS", MAC_DFS_DOMAIN_ETSI },
    { "CY", MAC_DFS_DOMAIN_ETSI },
    { "CZ", MAC_DFS_DOMAIN_ETSI },
    { "DE", MAC_DFS_DOMAIN_ETSI },
    { "DK", MAC_DFS_DOMAIN_ETSI },
    { "DO", MAC_DFS_DOMAIN_FCC },
    { "DZ", MAC_DFS_DOMAIN_NULL },
    { "EC", MAC_DFS_DOMAIN_FCC },
    { "EE", MAC_DFS_DOMAIN_ETSI },
    { "EG", MAC_DFS_DOMAIN_ETSI },
    { "ES", MAC_DFS_DOMAIN_ETSI },
    { "FI", MAC_DFS_DOMAIN_ETSI },
    { "FR", MAC_DFS_DOMAIN_ETSI },
    { "GB", MAC_DFS_DOMAIN_ETSI },
    { "GE", MAC_DFS_DOMAIN_ETSI },
    { "GR", MAC_DFS_DOMAIN_ETSI },
    { "GT", MAC_DFS_DOMAIN_FCC },
    { "HK", MAC_DFS_DOMAIN_FCC },
    { "HN", MAC_DFS_DOMAIN_FCC },
    { "HR", MAC_DFS_DOMAIN_ETSI },
    { "HU", MAC_DFS_DOMAIN_ETSI },
    { "ID", MAC_DFS_DOMAIN_NULL },
    { "IE", MAC_DFS_DOMAIN_ETSI },
    { "IL", MAC_DFS_DOMAIN_ETSI },
    { "IN", MAC_DFS_DOMAIN_NULL },
    { "IQ", MAC_DFS_DOMAIN_NULL },
    { "IR", MAC_DFS_DOMAIN_NULL },
    { "IS", MAC_DFS_DOMAIN_ETSI },
    { "IT", MAC_DFS_DOMAIN_ETSI },
    { "JM", MAC_DFS_DOMAIN_FCC },
    { "JO", MAC_DFS_DOMAIN_ETSI },
    { "JP", MAC_DFS_DOMAIN_MKK },
    { "KP", MAC_DFS_DOMAIN_NULL },
    { "KR", MAC_DFS_DOMAIN_KOREA },
    { "KW", MAC_DFS_DOMAIN_ETSI },
    { "KZ", MAC_DFS_DOMAIN_NULL },
    { "LB", MAC_DFS_DOMAIN_NULL },
    { "LI", MAC_DFS_DOMAIN_ETSI },
    { "LK", MAC_DFS_DOMAIN_FCC },
    { "LT", MAC_DFS_DOMAIN_ETSI },
    { "LU", MAC_DFS_DOMAIN_ETSI },
    { "LV", MAC_DFS_DOMAIN_ETSI },
    { "MA", MAC_DFS_DOMAIN_NULL },
    { "MC", MAC_DFS_DOMAIN_ETSI },
    { "MK", MAC_DFS_DOMAIN_ETSI },
    { "MO", MAC_DFS_DOMAIN_FCC },
    { "MT", MAC_DFS_DOMAIN_ETSI },
    { "MX", MAC_DFS_DOMAIN_FCC },
    { "MY", MAC_DFS_DOMAIN_FCC },
    { "NG", MAC_DFS_DOMAIN_NULL },
    { "NL", MAC_DFS_DOMAIN_ETSI },
    { "NO", MAC_DFS_DOMAIN_ETSI },
    { "NP", MAC_DFS_DOMAIN_NULL },
    { "NZ", MAC_DFS_DOMAIN_FCC },
    { "OM", MAC_DFS_DOMAIN_FCC },
    { "PA", MAC_DFS_DOMAIN_FCC },
    { "PE", MAC_DFS_DOMAIN_FCC },
    { "PG", MAC_DFS_DOMAIN_FCC },
    { "PH", MAC_DFS_DOMAIN_FCC },
    { "PK", MAC_DFS_DOMAIN_NULL },
    { "PL", MAC_DFS_DOMAIN_ETSI },
    { "PR", MAC_DFS_DOMAIN_FCC },
    { "PT", MAC_DFS_DOMAIN_ETSI },
    { "QA", MAC_DFS_DOMAIN_NULL },
    { "RO", MAC_DFS_DOMAIN_ETSI },
    { "RU", MAC_DFS_DOMAIN_FCC },
    { "SA", MAC_DFS_DOMAIN_FCC },
    { "SE", MAC_DFS_DOMAIN_ETSI },
    { "SG", MAC_DFS_DOMAIN_ETSI },
    { "SI", MAC_DFS_DOMAIN_ETSI },
    { "SK", MAC_DFS_DOMAIN_ETSI },
    { "SV", MAC_DFS_DOMAIN_FCC },
    { "SY", MAC_DFS_DOMAIN_NULL },
    { "TH", MAC_DFS_DOMAIN_FCC },
    { "TN", MAC_DFS_DOMAIN_ETSI },
    { "TR", MAC_DFS_DOMAIN_ETSI },
    { "TT", MAC_DFS_DOMAIN_FCC },
    { "TW", MAC_DFS_DOMAIN_NULL },
    { "UA", MAC_DFS_DOMAIN_NULL },
    { "US", MAC_DFS_DOMAIN_FCC },
    { "UY", MAC_DFS_DOMAIN_FCC },
    { "UZ", MAC_DFS_DOMAIN_FCC },
    { "VE", MAC_DFS_DOMAIN_FCC },
    { "VN", MAC_DFS_DOMAIN_ETSI },
    { "YE", MAC_DFS_DOMAIN_NULL },
    { "ZA", MAC_DFS_DOMAIN_FCC },
    { "ZW", MAC_DFS_DOMAIN_NULL },
};
#ifdef _PRE_WLAN_FEATURE_11D
/*
 * 函 数 名  : wal_is_alpha_upper
 * 功能描述  : 判断是否是大写字母
 * 1.日    期  : 2013年10月16日
  *   修改内容  : 新生成函数
 */
OAL_STATIC oal_bool_enum_uint8 wal_is_alpha_upper(int8_t c_letter)
{
    if (c_letter >= 'A' && c_letter <= 'Z') {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*
 * 函 数 名  : wal_regdomain_get_band
 * 功能描述  : 获取一个管制类的起始频带
 * 1.日    期  : 2013年10月16日
  *   修改内容  : 新生成函数
 */
uint8_t wal_regdomain_get_band(uint32_t start_freq, uint32_t end_freq)
{
    if (start_freq > 2400 && end_freq < 2500) { // 2400~2500代表2G频率范围
        return MAC_RC_START_FREQ_2;
    } else if (start_freq > 5000 && end_freq < 5870) { // 5000~5870代表5G频率范围
        return MAC_RC_START_FREQ_5;
    } else if (start_freq > 4900 && end_freq < 4999) { // 4900~4999代表5G频率范围
        return MAC_RC_START_FREQ_5;
    } else {
        return MAC_RC_START_FREQ_BUTT;
    }
}

/*
 * 函 数 名  : wal_regdomain_get_bw
 * 功能描述  : 获取一个管制类的带宽
 * 1.日    期  : 2013年10月17日
  *   修改内容  : 新生成函数
 */
#define WAL_CHAN_BW_80M 80
#define WAL_CHAN_BW_40M 40
#define WAL_CHAN_BW_20M 20
uint8_t wal_regdomain_get_bw(uint8_t uc_bw)
{
    uint8_t uc_bw_map;

    switch (uc_bw) {
        case WAL_CHAN_BW_80M:
            uc_bw_map = MAC_CH_SPACING_80MHZ;
            break;
        case WAL_CHAN_BW_40M:
            uc_bw_map = MAC_CH_SPACING_40MHZ;
            break;
        case WAL_CHAN_BW_20M:
            uc_bw_map = MAC_CH_SPACING_20MHZ;
            break;
        default:
            uc_bw_map = MAC_CH_SPACING_BUTT;
            break;
    };

    return uc_bw_map;
}

/*
 * 函 数 名  : wal_regdomain_get_channel_2g
 * 功能描述  : 获取管制类信道位图，信道在2g频段上
 * 1.日    期  : 2013年10月17日
  *   修改内容  : 新生成函数
 */
uint32_t wal_regdomain_get_channel_2g(uint32_t start_freq, uint32_t end_freq)
{
    uint32_t freq;
    uint32_t i;
    uint32_t ch_bmap = 0;

    for (freq = start_freq + 10; freq <= (end_freq - 10); freq++) { // 10代表找到中心频点需要的增减量
        for (i = 0; i < MAC_CHANNEL_FREQ_2_BUTT; i++) {
            if (freq == g_ast_freq_map_2g[i].us_freq) {
                ch_bmap |= (1 << i);
            }
        }
    }

    return ch_bmap;
}

/*
 * 函 数 名  : wal_regdomain_get_channel_5g
 * 功能描述  : 获取管制类信道位图，信道在5g频段上
 * 1.日    期  : 2013年10月17日
  *   修改内容  : 新生成函数
 */
uint32_t wal_regdomain_get_channel_5g(uint32_t start_freq, uint32_t end_freq)
{
    uint32_t freq;
    uint32_t i;
    uint32_t ch_bmap = 0;
    int value = 0;
    hwifi_get_init_priv_value(WLAN_CFG_PRIV_DISABLE_W58_CHANNEL, &value);
    // 5代表频率增加到下一频率的增量
    for (freq = start_freq + 10; freq <= (end_freq - 10); freq += 5) { // 10代表找到中心频点需要的增减量
        // 去掉W58所有信道（频率在5745~5825之间的5个信道）
        if (value == 1 && (freq >= 5745 && freq <= 5825)) {
            continue;
        }
        for (i = 0; i < MAC_CHANNEL_FREQ_5_BUTT; i++) {
            if (freq == g_ast_freq_map_5g[i].us_freq) {
                ch_bmap |= (1 << i);
            }
        }
    }

    return ch_bmap;
}

/*
 * 函 数 名  : wal_regdomain_get_channel
 * 功能描述  : 获取1个管制类的信道位图
 * 1.日    期  : 2013年10月17日
  *   修改内容  : 新生成函数
 */
uint32_t wal_regdomain_get_channel(uint8_t uc_band, uint32_t start_freq, uint32_t end_freq)
{
    uint32_t ch_bmap = 0;

    switch (uc_band) {
        case MAC_RC_START_FREQ_2:
            ch_bmap = wal_regdomain_get_channel_2g(start_freq, end_freq);
            break;

        case MAC_RC_START_FREQ_5:
            ch_bmap = wal_regdomain_get_channel_5g(start_freq, end_freq);
            break;

        default:
            break;
    }

    return ch_bmap;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 3, 0))
uint32_t wal_linux_update_wiphy_channel_list_num(oal_net_device_stru *net_dev, oal_wiphy_stru *pst_wiphy)
{
    uint16_t len;
    uint32_t ret;
    mac_vendor_cmd_channel_list_stru st_channel_list = { 0 };
    mac_vap_stru *mac_vap;

    if (oal_any_null_ptr2(pst_wiphy, net_dev)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_linux_update_wiphy_channel_list_num::input params null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_linux_update_wiphy_channel_list_num::NET_DEV_PRIV is NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_config_vendor_cmd_get_channel_list(mac_vap, &len, (uint8_t *)(&st_channel_list));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_linux_update_wiphy_channel_list_num::get chan list fail:%d}", ret);
        return ret;
    }

    /* 只更新2G信道个数，5G信道由于存在DFS 区域，且带宽计算并无问题,不需要修改 */
    g_st_supported_band_2ghz_info.n_channels = st_channel_list.uc_channel_num_2g;

    oam_warning_log2(0, OAM_SF_ANY, "{wal_linux_update_wiphy_channel_list_num::2g_chan_num:%d, 5g_chan_num:%d}",
        st_channel_list.uc_channel_num_2g, st_channel_list.uc_channel_num_5g);
    return OAL_SUCC;
}
#endif

/*
 * 函 数 名  : wal_get_dfs_domain
 * 功能描述  : 根据国家码，获取对应的雷达检测标准
 */
OAL_STATIC mac_dfs_domain_enum wal_get_dfs_domain(const int8_t *pc_country)
{
    uint32_t u_idx;
    uint32_t size = oal_array_size(g_ast_dfs_domain_table);

    for (u_idx = 0; u_idx < size; u_idx++) {
        if (oal_strcmp(g_ast_dfs_domain_table[u_idx].pc_country, pc_country) == 0) {
            return g_ast_dfs_domain_table[u_idx].en_dfs_domain;
        }
    }

    return MAC_DFS_DOMAIN_NULL;
}
/*
 * 函 数 名  : wal_regdomain_fill_info
 * 功能描述  : 填充管制下发的管制域信息
 * 1.日    期  : 2013年10月16日
  *   修改内容  : 新生成函数
 */
OAL_STATIC void wal_regdomain_fill_info(const oal_ieee80211_regdomain_stru *pst_regdom,
    mac_regdomain_info_stru *pst_mac_regdom)
{
    uint32_t i, start, end;
    uint8_t uc_band, uc_bw;

    /* 复制国家字符串 */
    pst_mac_regdom->ac_country[BYTE_OFFSET_0] = pst_regdom->alpha2[BYTE_OFFSET_0];
    pst_mac_regdom->ac_country[BYTE_OFFSET_1] = pst_regdom->alpha2[BYTE_OFFSET_1];
    pst_mac_regdom->ac_country[BYTE_OFFSET_2] = 0;

    /* 获取DFS认证标准类型 */
    pst_mac_regdom->en_dfs_domain = wal_get_dfs_domain(pst_regdom->alpha2);

    /* 填充管制类个数 */
    pst_mac_regdom->uc_regclass_num = (uint8_t)pst_regdom->n_reg_rules;

    /* 填充管制类信息 */
    for (i = 0; i < pst_regdom->n_reg_rules; i++) {
        /* 填写管制类的频段(2.4G或5G) */
        start = KHZ_TO_MHZ(pst_regdom->reg_rules[i].freq_range.start_freq_khz);
        end = KHZ_TO_MHZ(pst_regdom->reg_rules[i].freq_range.end_freq_khz);
        uc_band = wal_regdomain_get_band(start, end);
        pst_mac_regdom->ast_regclass[i].en_start_freq = uc_band;

        /* 填写管制类允许的最大带宽 */
        uc_bw = (uint8_t)KHZ_TO_MHZ(pst_regdom->reg_rules[i].freq_range.max_bandwidth_khz);
        pst_mac_regdom->ast_regclass[i].en_ch_spacing = wal_regdomain_get_bw(uc_bw);

        /* 填写管制类信道位图 */
        pst_mac_regdom->ast_regclass[i].channel_bmap = wal_regdomain_get_channel(uc_band, start, end);

        /* 标记管制类行为 */
        pst_mac_regdom->ast_regclass[i].uc_behaviour_bmap = 0;

        if (pst_regdom->reg_rules[i].flags & NL80211_RRF_DFS) {
            pst_mac_regdom->ast_regclass[i].uc_behaviour_bmap |= MAC_RC_DFS;
        }

        if (pst_regdom->reg_rules[i].flags & NL80211_RRF_NO_INDOOR) {
            pst_mac_regdom->ast_regclass[i].uc_behaviour_bmap |= MAC_RC_NO_INDOOR;
        }

        if (pst_regdom->reg_rules[i].flags & NL80211_RRF_NO_OUTDOOR) {
            pst_mac_regdom->ast_regclass[i].uc_behaviour_bmap |= MAC_RC_NO_OUTDOOR;
        }
        /* 填充覆盖类和最大发送功率 */
        pst_mac_regdom->ast_regclass[i].uc_coverage_class = 0;
        pst_mac_regdom->ast_regclass[i].uc_max_reg_tx_pwr =
            (uint8_t)MBM_TO_DBM(pst_regdom->reg_rules[i].power_rule.max_eirp);
        pst_mac_regdom->ast_regclass[i].us_max_tx_pwr =
            (uint16_t)(pst_regdom->reg_rules[i].power_rule.max_eirp / 10); // 10: 实际使用的最大发送功率,扩大了10倍用于计算
    }
}

static void fill_kernel_regdom_disable_w58(const oal_ieee80211_regdomain_stru *regdom,
                                           oal_ieee80211_regdomain_stru *regdom_to_kernel)
{
    int i;
    int n_reg_ruls = 0;
    uint32_t start;
    uint32_t end;

    memcpy_s(regdom_to_kernel->alpha2, sizeof(regdom_to_kernel->alpha2), regdom->alpha2, sizeof(regdom->alpha2));
#ifndef WIN32
    regdom_to_kernel->dfs_region = regdom->dfs_region;
#endif

    for (i = 0; i < regdom->n_reg_rules; i++) {
        start = KHZ_TO_MHZ(regdom->reg_rules[i].freq_range.start_freq_khz);
        end = KHZ_TO_MHZ(regdom->reg_rules[i].freq_range.end_freq_khz);
        // 去掉W58所有信道（频率在5735~5835之间的5个信道）
        if (start == 5735 && end == 5835) {
            continue;
        }
        memcpy_s(&regdom_to_kernel->reg_rules[n_reg_ruls], sizeof(struct ieee80211_reg_rule),
            &regdom->reg_rules[i], sizeof(struct ieee80211_reg_rule));
        n_reg_ruls++;
    }

    regdom_to_kernel->n_reg_rules = (uint32_t)n_reg_ruls;
}
static void wal_regdom_to_kernel(mac_device_stru *pst_device, const oal_ieee80211_regdomain_stru *pst_regdom)
{
    oal_ieee80211_regdomain_stru *regdom_to_kernel = NULL;
    int value = 0;

    hwifi_get_init_priv_value(WLAN_CFG_PRIV_DISABLE_W58_CHANNEL, &value);
    if (value == 1) {
        regdom_to_kernel = oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, sizeof(oal_ieee80211_regdomain_stru) +
            sizeof(struct ieee80211_reg_rule) * IEEE80211_REGDOMAIN_REG_RULES_SIZE, OAL_TRUE);
        if (regdom_to_kernel == NULL) {
            oam_error_log0(0, OAM_SF_ANY, "{wal_regdom_to_kernel::regdom_to_kernel alloc fail.}");
            return;
        }
        fill_kernel_regdom_disable_w58(pst_regdom, regdom_to_kernel);
        oal_wiphy_apply_custom_regulatory(pst_device->pst_wiphy, regdom_to_kernel);
        oal_mem_free_m(regdom_to_kernel, OAL_TRUE);
        regdom_to_kernel = NULL;
    } else {
        oal_wiphy_apply_custom_regulatory(pst_device->pst_wiphy, pst_regdom);
    }
}

int32_t wal_regdomain_update_post_event(oal_net_device_stru *net_dev, const oal_ieee80211_regdomain_stru *pst_regdom,
    int8_t *pc_country, oal_bool_enum_uint8 need_rsp)
{
    wal_msg_write_stru write_msg = {0};
    mac_cfg_country_stru *pst_param = NULL;
    wal_msg_stru *rsp_msg = NULL;
    mac_regdomain_info_stru *pst_mac_regdom = NULL;
    uint16_t us_size = (uint16_t)(sizeof(mac_regclass_info_stru) * pst_regdom->n_reg_rules + MAC_RD_INFO_LEN);
    int32_t ret;
    /* 申请内存存放管制域信息，将内存指针作为事件payload抛下去 */
    /* 此处申请的内存在事件处理函数释放(hmac_config_set_country) */
    pst_mac_regdom = oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, us_size, OAL_TRUE);
    if (pst_mac_regdom == NULL) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_regdomain_update::alloc regdom mem fail(%d),null ptr!}", us_size);
        return -OAL_ENOMEM;
    }

    wal_regdomain_fill_info(pst_regdom, pst_mac_regdom);

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_COUNTRY, sizeof(mac_cfg_country_stru));

    /* 填写WID对应的参数 */
    pst_mac_regdom->en_regdomain = hwifi_get_regdomain_from_country_code(pc_country);
    pst_param = (mac_cfg_country_stru *)(write_msg.auc_value);
    pst_param->p_mac_regdom = pst_mac_regdom;

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_country_stru), (uint8_t *)&write_msg, need_rsp, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_regdomain_update::err %d!}", ret);
        /*  只要下发成功，hmac会释放 */
        if (oal_value_ne_all2(ret, -OAL_ETIMEDOUT, -OAL_EFAUL)) {
            oal_mem_free_m(pst_mac_regdom, OAL_TRUE);
            pst_mac_regdom = NULL;
        }
        if (rsp_msg != NULL) {
            oal_free(rsp_msg);
        }
        return ret;
    }
    oal_free(rsp_msg);
    return OAL_SUCC;
}
int32_t wal_regdomain_update_channel_list(oal_net_device_stru *net_dev, const oal_ieee80211_regdomain_stru *pst_regdom)
{
    mac_device_stru *pst_device = NULL;
    mac_vap_stru *mac_vap = NULL;

    /* 驱动不支持ACS时，更新hostapd管制域信息; 如果驱动支持ACS，则不需要更新，否则hostapd无法配置DFS信道 */
    /*  */
    mac_vap = (mac_vap_stru *)oal_net_dev_priv(net_dev);
    if (oal_unlikely(mac_vap == NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_regdomain_update::OAL_NET_DEV_PRIV(net_dev) null.}");
        return -OAL_FAIL;
    }

    pst_device = mac_res_get_dev(mac_vap->uc_device_id);
    if ((pst_device != NULL) && (pst_device->pst_wiphy != NULL)) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 3, 0))
        /* Linux 内核4.4版本增加10M带宽，将channel 12添加到US信道列表，修改
        wifi 驱动刷新当前管制域支持信道列表    */
        wal_linux_update_wiphy_channel_list_num(net_dev, pst_device->pst_wiphy);
#endif

        oam_warning_log0(0, OAM_SF_ANY, "{wal_regdomain_update::update regdom to kernel.}");
        wal_cfg80211_reset_bands(mac_vap->uc_device_id);

        wal_regdom_to_kernel(pst_device, pst_regdom);

        /* 更新管制域后，将每个信道flag 信息保存到对应orig_flag，
         * 避免去关联时恢复管制域信息错误，导致某些信道不能工作。
         */
        wal_cfg80211_save_bands(mac_vap->uc_device_id);
    }
    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_regdomain_update
 * 功能描述  : 下发配置管制域信息
 * 1.日    期  : 2013年10月22日
  *   修改内容  : 新生成函数
 * 2.日    期  : 2018年9月26日
 *   作    者  : wifi
 *   修改内容  : 事件下发成功在HMAC释放
 */
int32_t wal_regdomain_update(oal_net_device_stru *net_dev, int8_t *pc_country)
{
    const oal_ieee80211_regdomain_stru *pst_regdom = NULL;
    int32_t ret;

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    int8_t old_pc_country[COUNTRY_CODE_LEN] = { '9', '9' };

    if (EOK != memcpy_s(old_pc_country, COUNTRY_CODE_LEN, hwifi_get_country_code(), COUNTRY_CODE_LEN)) {
        oam_error_log0(0, OAM_SF_ANY, "wal_regdomain_update::memcpy fail!");
        return -OAL_EINVAL;
    }
    hwifi_set_country_code(pc_country, COUNTRY_CODE_LEN);
    /* 如果新的国家码和旧国家处于一个regdomain，不刷新RF参数，只更新国家码 */
    if (OAL_TRUE == hwifi_is_regdomain_changed((uint8_t *)old_pc_country, (uint8_t *)pc_country)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_regdomain_update::regdomain changed, refresh rf param!}");

        /* 刷新参数失败，为了保证国家码和功率参数对应 */
        /* 将国家码设回原来的国家码，本次更新失败 */
        if (hwifi_force_refresh_rf_params(net_dev) != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_ANY,
                "{wal_regdomain_update::refresh rf(max_txpwr & dbb scale) params failed. Set country back.!}");
            hwifi_set_country_code(old_pc_country, COUNTRY_CODE_LEN);
        }
    }
#endif

    if (!wal_is_alpha_upper(pc_country[0]) || !wal_is_alpha_upper(pc_country[1])) {
        if ((pc_country[0] == '9') && (pc_country[1] == '9')) {
            oam_info_log0(0, OAM_SF_ANY, "{wal_regdomain_update::set regdomain to 99!}");
        } else {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_regdomain_update::country str invalid!}");
            return -OAL_EINVAL;
        }
    }

    pst_regdom = wal_regdb_find_db(pc_country);
    if (pst_regdom == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_regdomain_update::no regdomain db was found!}");
        return -OAL_EINVAL;
    }

    ret = wal_regdomain_update_post_event(net_dev, pst_regdom, pc_country, OAL_TRUE);
    if (ret != OAL_SUCC) {
        return ret;
    }

    return wal_regdomain_update_channel_list(net_dev, pst_regdom);
}

#ifdef _PRE_WLAN_COUNTRYCODE_SELFSTUDY
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/*
 * 函 数 名  : wal_regdomain_update_rf_param
 * 功能描述  : 管制域变更，更新RF参数
* 修改历史  :
 * 1.日    期  : 2019年12月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void wal_regdomain_update_rf_param(oal_net_device_stru *net_dev, int8_t *pc_country, int8_t *old_pc_country)
{
    if (EOK != memcpy_s(old_pc_country, COUNTRY_CODE_LEN, hwifi_get_country_code(), COUNTRY_CODE_LEN)) {
        oam_error_log0(0, OAM_SF_ANY, "wal_regdomain_update_rf_param::memcpy fail!");
        return;
    }

    hwifi_set_country_code(pc_country, COUNTRY_CODE_LEN);
    /* 如果新的国家码和旧国家处于一个regdomain，不刷新RF参数，只更新国家码 */
    if (OAL_TRUE == hwifi_is_regdomain_changed((uint8_t *)old_pc_country, (uint8_t *)pc_country)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_regdomain_update_rf_param::regdomain changed, refresh rf param!}");

        /* 刷新参数失败，为了保证国家码和功率参数对应 */
        /* 将国家码设回原来的国家码，本次更新失败 */
        if (hwifi_force_refresh_rf_params(net_dev) != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_ANY,
                "{wal_regdomain_update_rf_param::refresh rf params failed. Set country back.!}");
            hwifi_set_country_code(old_pc_country, COUNTRY_CODE_LEN);
        }
    }
}
#endif
/*
 * 函 数 名  : wal_regdomain_confirm
 * 功能描述  : 国家码确认
* 修改历史  :
 * 1.日    期  : 2019年12月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC int32_t wal_regdomain_confirm(int8_t *pc_country, mac_regdomain_info_stru *pst_mac_regdom)
{
    if (!wal_is_alpha_upper(pc_country[0]) || !wal_is_alpha_upper(pc_country[1])) {
        if ((pc_country[0] == '9') && (pc_country[1] == '9')) {
            oam_info_log0(0, OAM_SF_ANY, "{wal_regdomain_confirm::set regdomain to 99!}");
        } else {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_regdomain_confirm::country str invalid!}");
            return -OAL_EINVAL;
        }
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_regdomain_update_hostapd_param
 * 功能描述  : 管制域变更，更新hostapd参数
* 修改历史  :
 * 1.日    期  : 2019年12月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void wal_regdomain_update_hostapd_param(const oal_ieee80211_regdomain_stru *pst_regdom,
    oal_net_device_stru *net_dev)
{
    mac_vap_stru *mac_vap = NULL;
    uint8_t uc_dev_id;
    mac_device_stru *pst_device = NULL;

    mac_vap = (mac_vap_stru *)oal_net_dev_priv(net_dev);
    if (oal_unlikely(mac_vap == NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_regdomain_update_hostapd_param::OAL_NET_DEV_PRIV null.}");
        return;
    }

    uc_dev_id = mac_vap->uc_device_id;
    pst_device = mac_res_get_dev(uc_dev_id);
    if ((pst_device != NULL) && (pst_device->pst_wiphy != NULL)) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 3, 0))
        /* Linux 内核4.4版本增加10M带宽，将channel 12添加到US信道列表，修改
        wifi 驱动刷新当前管制域支持信道列表    */
        wal_linux_update_wiphy_channel_list_num(net_dev, pst_device->pst_wiphy);
#endif

        oam_warning_log0(0, OAM_SF_ANY, "{wal_regdomain_update_hostapd_param::update regdom to kernel.}");

        wal_cfg80211_reset_bands(uc_dev_id);
        oal_wiphy_apply_custom_regulatory(pst_device->pst_wiphy, pst_regdom);
        /* 更新管制域后，将每个信道flag 信息保存到对应orig_flag，
         * 避免去关联时恢复管制域信息错误，导致某些信道不能工作。
         */
        wal_cfg80211_save_bands(uc_dev_id);
    }
}

/*
 * 函 数 名  : wal_selfstdy_regdomain_update
 * 功能描述  : PC管制域变更
* 修改历史  :
 * 1.日    期  : 2019年12月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_selfstdy_regdomain_update(oal_net_device_stru *net_dev, int8_t *pc_country)
{
    const oal_ieee80211_regdomain_stru *pst_regdom;
    mac_regdomain_info_stru *pst_mac_regdom = NULL;
    int32_t ret;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    int8_t old_pc_country[COUNTRY_CODE_LEN] = { '9', '9' };

    wal_regdomain_update_rf_param(net_dev, pc_country, old_pc_country);
#endif

    ret = wal_regdomain_confirm(pc_country, pst_mac_regdom);

    pst_regdom = wal_regdb_find_db(pc_country);
    if (pst_regdom == NULL) {
        return -OAL_EINVAL;
    }
    ret = wal_regdomain_update_post_event(net_dev, pst_regdom, pc_country, OAL_FALSE);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* 驱动不支持ACS时，更新hostapd管制域信息; 如果驱动支持ACS，则不需要更新，否则hostapd无法配置DFS信道 */
    /*  */
    wal_regdomain_update_hostapd_param(pst_regdom, net_dev);
    return OAL_SUCC;
}
#endif
/*
 * 函 数 名  : wal_regdomain_update_for_dfs
 * 功能描述  : 下发配置管制域信息
 * 1.日    期  : 2015年1月19日
  *   修改内容  : 新生成函数
 */
int32_t wal_regdomain_update_for_dfs(oal_net_device_stru *net_dev, int8_t *pc_country)
{
    wal_msg_write_stru write_msg = {0};
    mac_dfs_domain_enum_uint8 *pst_param = NULL;
    int32_t ret;

    if (!wal_is_alpha_upper(pc_country[0]) || !wal_is_alpha_upper(pc_country[1])) {
        if ((pc_country[0] == '9') && (pc_country[1] == '9')) {
            oam_info_log0(0, OAM_SF_ANY, "{wal_regdomain_update_for_dfs::set regdomain to 99!}");
        } else {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_regdomain_update_for_dfs::country str is invalid!}");
            return -OAL_EINVAL;
        }
    }

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_COUNTRY_FOR_DFS, sizeof(mac_dfs_domain_enum_uint8));

    /* 填写WID对应的参数 */
    pst_param = (mac_dfs_domain_enum_uint8 *)(write_msg.auc_value);
    *pst_param = wal_get_dfs_domain(pc_country);

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev,
        WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_dfs_domain_enum_uint8), (uint8_t *)&write_msg,
        OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_regdomain_update_for_dfs::err %d!}", ret);
        return ret;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_regdomain_update_sta
 * 功能描述  : sta关联成功后，更新自己的管制域信息
 * 1.日    期  : 2013年10月22日
  *   修改内容  : 新生成函数
 */
uint32_t wal_regdomain_update_sta(uint8_t uc_vap_id)
{
    int8_t *pc_desired_country = NULL;

    oal_net_device_stru *net_dev = NULL;
    int32_t ret;
    oal_bool_enum_uint8 us_updata_rd_by_ie_switch = OAL_FALSE;

    hmac_vap_get_updata_rd_by_ie_switch(uc_vap_id, &us_updata_rd_by_ie_switch);

    if (us_updata_rd_by_ie_switch == OAL_TRUE) {
        pc_desired_country = hmac_vap_get_desired_country(uc_vap_id);
        if (oal_unlikely(pc_desired_country == NULL)) {
            oam_error_log0(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta::pc_desired_country null!}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        /* 期望的国家码全为0，表示对端AP的国家码不存在，采用sta当前默认的国家码 */
        if ((pc_desired_country[0] == 0) && (pc_desired_country[1] == 0)) {
            oam_info_log0(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta::ap has no country ie,use default}");
            return OAL_SUCC;
        }

        net_dev = hmac_vap_get_net_device(uc_vap_id);
        if (oal_unlikely(net_dev == NULL)) {
            oam_error_log0(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta::net_dev null!}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        ret = wal_regdomain_update_for_dfs(net_dev, pc_desired_country);
        if (ret != OAL_SUCC) {
            oam_warning_log1(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta::regdomain_update err:%d!}", ret);
            return OAL_FAIL;
        }

        ret = wal_regdomain_update(net_dev, pc_desired_country);
        if (ret != OAL_SUCC) {
            oam_warning_log1(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta::regdomain_update err:%d}", ret);
            return OAL_FAIL;
        }
    } else {
        oam_info_log0(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta::us_updata_rd_by_ie_switch is fail!}");
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_regdomain_update_country_code
 * 功能描述  : 下发配置管制域信息
 * 1.日    期  : 2016年5月31日
  *   修改内容  : 新生成函数
 */
int32_t wal_regdomain_update_country_code(oal_net_device_stru *net_dev, int8_t *pc_country)
{
    int32_t ret;

    if (net_dev == NULL || pc_country == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_regdomain_update_country_code::null ptr!}");
        return -OAL_EFAIL;
    }

    /* 设置国家码到wifi 驱动 */
    ret = wal_regdomain_update_for_dfs(net_dev, pc_country);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_regdomain_update_country_code::update_for_dfs err [%d]!}", ret);
        return -OAL_EFAIL;
    }

    ret = wal_regdomain_update(net_dev, pc_country);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_regdomain_update_country_code::update err [%d]!}", ret);
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}
#ifdef _PRE_WLAN_COUNTRYCODE_SELFSTUDY
/*
 * 函 数 名  : wal_regdomain_update_selfstudy_country_code
 * 功能描述  : PC下发配置管制域信息
 * 1.日    期  : 2019年12月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_regdomain_update_selfstudy_country_code(oal_net_device_stru *net_dev, int8_t *pc_country)
{
    int32_t   ret;

    if (net_dev == NULL || pc_country == NULL) {
        oam_error_log0(0, OAM_SF_ANY,
            "{wal_regdomain_update_selfstudy_country_code::null ptr.}");
        return -OAL_EFAIL;
    }

    /* 设置国家码到wifi 驱动 */
    ret = wal_regdomain_update_for_dfs(net_dev, pc_country);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_regdomain_update_selfstudy_country_code::update_for_dfs ret%d", ret);
        return -OAL_EFAIL;
    }

    ret = wal_selfstdy_regdomain_update(net_dev, pc_country);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_regdomain_update_selfstudy_country_code::update return err code [%d]!}\r\n", ret);
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_regdomain_update_by_ap
 * 功能描述  : sta关联成功后，以AP的管制域更新自己的管制域信息
 * 1.日    期  : 2019年12月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void wal_regdomain_update_by_ap(uint8_t uc_vap_id)
{
    int8_t *pc_desired_country = NULL;
    oal_net_device_stru *net_dev = NULL;
    oal_bool_enum_uint8 us_updata_rd_by_ie_switch = OAL_FALSE;

    /* 关联成功后，停止国家码自学习 */
    g_country_code_self_study_flag = OAL_FALSE;

    hmac_vap_get_updata_rd_by_ie_switch(uc_vap_id, &us_updata_rd_by_ie_switch);
    pc_desired_country = hmac_vap_get_desired_country(uc_vap_id);
    /* 若国家码为99，则以关联AP的国家码为准 */
    if (g_country_code_result[0] == '9' && g_country_code_result[1] == '9') {
        if (oal_unlikely(pc_desired_country == NULL)) {
            oam_error_log0(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta::pc_desired_country null!}");
            return;
        }

        /* 期望的国家码全为0，表示对端AP的国家码不存在，采用sta当前默认的国家码 */
        if ((pc_desired_country[0] == 0) && (pc_desired_country[1] == 0)) {
            oam_info_log0(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta::ap has no country ie,use default}");
        }

        net_dev = hmac_vap_get_net_device(uc_vap_id);
        if (oal_unlikely(net_dev == NULL)) {
            oam_error_log0(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta::net_dev null!}");
        }

        wal_regdomain_update_selfstudy_country_code(net_dev, pc_desired_country);
    }
}
/*
 * 函 数 名  : wal_selfstudy_regdomain_update_by_ap
 * 功能描述  : sta关联成功后，以AP的管制域更新自己的管制域信息
 * 1.日    期  : 2019年12月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void wal_selfstudy_regdomain_update_by_ap(frw_event_stru *event)
{
    if (g_cust_country_code_ignore_flag.en_country_code_study_ingore_hipriv_flag == OAL_FALSE) {
        wal_regdomain_update_by_ap(event->st_event_hdr.uc_vap_id);
    }
}
#endif
#endif

/*
 * 函 数 名  : wal_get_cmd_one_arg
 * 功能描述  : 获取字符串第一个参数，以逗号为命令区分标识，并扣除命令前置字符
 * 1.日    期  : 2015年5月22日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_get_parameter_from_cmd(int8_t *pc_cmd, int8_t *pc_arg,
    const int8_t *puc_token, uint32_t *pul_cmd_offset, uint32_t param_max_len)
{
    int8_t *pc_cmd_copy = NULL;
    int8_t ac_cmd_copy[WAL_IOCTL_PRIV_SUBCMD_MAX_LEN];
    uint32_t pos = 0;
    uint32_t arg_len;
    int8_t *pc_cmd_tmp = NULL;

    if (oal_unlikely(oal_any_null_ptr3(pc_cmd, pc_arg, pul_cmd_offset))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_get_parameter_from_cmd::pc_cmd/pc_arg/pul_cmd_offset is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pc_cmd_copy = pc_cmd;

    /* 去掉字符串开始的逗号 */
    while (*pc_cmd_copy == ',') {
        ++pc_cmd_copy;
    }
    /* 取得逗号前的字符串 */
    while ((*pc_cmd_copy != ',') && (*pc_cmd_copy != '\0')) {
        ac_cmd_copy[pos] = *pc_cmd_copy;
        ++pos;
        ++pc_cmd_copy;

        if (oal_unlikely(pos >= param_max_len)) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_get_parameter_from_cmd::pos>=CMD_NAME_MAX_LEN, pos %d!}", pos);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }
    }
    ac_cmd_copy[pos] = '\0';
    /* 字符串到结尾，返回错误码 */
    if (pos == 0) {
        oam_info_log0(0, OAM_SF_ANY, "{wal_get_parameter_from_cmd::return param pc_arg is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    *pul_cmd_offset = (uint32_t)(pc_cmd_copy - pc_cmd);

    /* 检查字符串是否包含期望的前置命令字符 */
    pc_cmd_tmp = &ac_cmd_copy[0];
    if (oal_memcmp(pc_cmd_tmp, puc_token, OAL_STRLEN(puc_token)) != 0) {
        return OAL_FAIL;
    } else {
        /* 扣除前置命令字符，回传参数 */
        arg_len = OAL_STRLEN(ac_cmd_copy) - OAL_STRLEN(puc_token);
        if (EOK != memcpy_s(pc_arg, param_max_len, ac_cmd_copy + OAL_STRLEN(puc_token), arg_len)) {
            oam_warning_log0(0, OAM_SF_ANY, "wal_get_parameter_from_cmd::memcpy fail!");
        }
        pc_arg[arg_len] = '\0';
    }
    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_set_ap_max_user
 * 功能描述  : set ap max user count to hmac
 * 1.日    期  : 2015年8月18日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC int32_t wal_set_ap_max_user(oal_net_device_stru *net_dev, uint32_t ap_max_user)
{
    wal_msg_write_stru write_msg = {0};
    wal_msg_stru *rsp_msg = NULL;
    uint32_t err_code;
    int32_t ret;

    oam_warning_log1(0, OAM_SF_ANY, "{wal_set_ap_max_user::set AP max user:%u.}", ap_max_user);

    if (ap_max_user == 0) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_set_ap_max_user::invalid max user(%u),ignore it}", ap_max_user);
        return OAL_SUCC;
    }

    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_SET_MAX_USER, sizeof(ap_max_user));
    *((uint32_t *)write_msg.auc_value) = ap_max_user;
    ret = wal_send_cfg_event(net_dev,
        WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(ap_max_user),
        (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if ((ret != OAL_SUCC) || (rsp_msg == NULL)) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_set_ap_max_user:: wal_send_cfg_event return err code %d!}", ret);
        return ret;
    }

    /* 读取返回的错误码 */
    err_code = wal_check_and_release_msg_resp(rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_set_ap_max_user::wal_check_and_release_msg_resp fail[%d]!}",
                         err_code);
        return -OAL_EFAIL;
    }
    /* Begin：o版本先设置最大用户数，再启动aput，需要保存该配置，在启动aput的时候用，不能删除 */
    /* 每次设置最大用户数完成后，都清空为非法值0 **/
    /* g_st_ap_config_info.ap_max_user 0 */
    /* End： */
    return ret;
}

OAL_STATIC int32_t wal_config_mac_filter_event_cfg(oal_net_device_stru *net_dev,
    uint32_t mac_cnt, uint32_t i, int8_t *ac_parsed_command, uint32_t arraylen)
{
    int32_t ret;
    wal_msg_write_stru write_msg = {0};
    uint16_t len;
    wal_msg_stru *rsp_msg = NULL;
    mac_blacklist_stru *pst_blklst = NULL;
    uint32_t err_code;

    /* 抛事件到wal层处理 */
    memset_s((uint8_t *)&write_msg, sizeof(write_msg), 0, sizeof(write_msg));
    pst_blklst = (mac_blacklist_stru *)(write_msg.auc_value);
    /* 将字符 ac_name 转换成数组 mac_add[6] */
    oal_strtoaddr(ac_parsed_command, arraylen, pst_blklst->auc_mac_addr, WLAN_MAC_ADDR_LEN);

    len = sizeof(mac_blacklist_stru);
    if (i == (mac_cnt - 1)) {
        /* 等所有的mac地址都添加完成后，才进行关联用户确认，是否需要删除 */
        wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_ADD_BLACK_LIST, len);
    } else {
        wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_ADD_BLACK_LIST_ONLY, len);
    }

    /* 6.1  发送消息 */
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + len, (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if ((ret != OAL_SUCC) || (rsp_msg == NULL)) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_config_mac_filter:: wal_send_cfg_event err %d!}", ret);
        return OAL_FAIL;
    }

    /* 6.2  读取返回的错误码 */
    err_code = wal_check_and_release_msg_resp(rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_config_mac_filter::wal_check_and_release_msg_resp fail[%x]!}",
            err_code);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
static uint32_t wal_get_mac_filter_mode(int8_t *parse_command, int8_t *parsed_cmd, uint32_t *off_set)
{
    uint32_t mac_mode, result;
    result = wal_get_parameter_from_cmd(parse_command, parsed_cmd, "MAC_MODE=",
        off_set, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_get_mac_filter_mode::wal_get_parameter_from_cmd err %u.}", result);
        return OAL_FAIL;
    }
    /* 检查参数是否合法 0,1,2 */
    mac_mode = (uint32_t)oal_atoi(parsed_cmd);
    if (mac_mode > 2) { // 2代表合法参数值上限大小
        oam_warning_log4(0, OAM_SF_ANY, "{wal_get_mac_filter_mode::invalid MAC_MODE[%c%c%c%c]!}",
            (uint8_t)parsed_cmd[0], (uint8_t)parsed_cmd[1],
            (uint8_t)parsed_cmd[2], (uint8_t)parsed_cmd[3]); /* 2 3 cmd para */
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_config_mac_filter
 * 功能描述  : set ap mac filter mode to hmac
 * 1.日    期  : 2015年8月18日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void wal_config_mac_filter(oal_net_device_stru *net_dev, int8_t *pc_command)
{
    int8_t ac_parsed_command[WAL_IOCTL_PRIV_SUBCMD_MAX_LEN] = {0};
    int8_t *pc_parse_command = NULL;
    uint32_t mac_cnt, result, i;
    uint32_t off_set = 0;

    if (pc_command == NULL) {
        return;
    }
    pc_parse_command = pc_command;

    /* 解析MAC_MODE */
    result = wal_get_mac_filter_mode(pc_parse_command, ac_parsed_command, &off_set);
    if (result != OAL_SUCC) {
        return;
    }
    /* 设置过滤模式 */
    result = wal_hipriv_send_cfg_uint32_data(net_dev, ac_parsed_command, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN,
        WLAN_CFGID_BLACKLIST_MODE);
    if (result != OAL_SUCC) {
        return;
    }
    /* 解析MAC_CNT */
    pc_parse_command += off_set;
    result = wal_get_parameter_from_cmd(pc_parse_command, ac_parsed_command, "MAC_CNT=",
        &off_set, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_config_mac_filter::wal_get_parameter_from_cmd err [%u]!}", result);
        return;
    }
    mac_cnt = (uint32_t)oal_atoi(ac_parsed_command);

    for (i = 0; i < mac_cnt; i++) {
        pc_parse_command += off_set;
        result = wal_get_parameter_from_cmd(pc_parse_command, ac_parsed_command, "MAC=",
            &off_set, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
        if (result != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_config_mac_filter::wal_get_parameter_from_cmd err [%u]!}", result);
            return;
        }
        /* 5.1  检查参数是否符合MAC长度 */
        if (WLAN_MAC_ADDR_LEN << BIT_OFFSET_1 != OAL_STRLEN(ac_parsed_command)) {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_config_mac_filter::invalid MAC format}");
            return;
        }
        /* 6. 添加过滤设备 */
        if (wal_config_mac_filter_event_cfg(net_dev, mac_cnt, i, ac_parsed_command, sizeof(ac_parsed_command))
            != OAL_SUCC) {
            return;
        }
    }
}

/*
 * 函 数 名  : wal_kick_sta
 * 功能描述  : deauth auc_mac_addr in ap mode
 */
int32_t wal_kick_sta(oal_net_device_stru *net_dev,
    uint8_t *auc_mac_addr, uint8_t uc_mac_addr_len, uint16_t us_reason_code)
{
    mac_cfg_kick_user_param_stru kick_user_param;

    if ((auc_mac_addr == NULL) || (uc_mac_addr_len != WLAN_MAC_ADDR_LEN)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_kick_sta::check para! .");
        return -OAL_EFAIL;
    }

    oal_set_mac_addr(kick_user_param.auc_mac_addr, auc_mac_addr);
    kick_user_param.us_reason_code = us_reason_code;
    kick_user_param.send_disassoc_immediately = OAL_TRUE;

    return wal_cfg80211_start_disconnect(net_dev, &kick_user_param);
}
static int32_t wal_ioctl_ap_config_fail_proc(int8_t *pc_command)
{
    oal_free(pc_command);
    return -OAL_EFAIL;
}
/*
 * 函 数 名  : wal_ioctl_set_ap_config
 * 功能描述  : 设置netd下发APUT config参数 (最大用户数)
 * 1.日    期  : 2015年5月21日
  *   修改内容  : 新生成函数
 */
int wal_ioctl_set_ap_config(oal_net_device_stru *net_dev,
    oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, char *pc_extra)
{
    int8_t *pc_command = NULL;
    int8_t *pc_parse_cmd = NULL;
    int32_t ret = OAL_SUCC;
    uint32_t result;
    uint32_t off_set = 0;
    int8_t ac_cmd[WAL_IOCTL_PRIV_SUBCMD_MAX_LEN] = {0};

    if (oal_unlikely(oal_any_null_ptr2(net_dev, pst_wrqu))) {
        oam_warning_log0(0, 0, "{wal_ioctl_set_ap_config:input params null.}");
        return -OAL_EFAIL;
    }

    /* 1. 申请内存保存netd 下发的命令和数据 */
    pc_command = oal_memalloc((int32_t)(pst_wrqu->data.length + 1));
    if (pc_command == NULL) {
        return -OAL_ENOMEM;
    }
    /* 2. 拷贝netd 命令到内核态中 */
    memset_s(pc_command, (uint32_t)(pst_wrqu->data.length + 1), 0, (uint32_t)(pst_wrqu->data.length + 1));
    result = oal_copy_from_user(pc_command, pst_wrqu->data.pointer, (uint32_t)(pst_wrqu->data.length));
    if (result != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_ioctl_set_ap_config::oal_copy_from_user: -OAL_EFAIL }");
        return wal_ioctl_ap_config_fail_proc(pc_command);
    }
    pc_command[pst_wrqu->data.length] = '\0';

    oal_io_print("wal_ioctl_set_ap_config,data len:%u \n", (uint32_t)pst_wrqu->data.length);

    pc_parse_cmd = pc_command;
    /* 3.   解析参数 */
    /* 3.1  解析ASCII_CMD */
    result = wal_get_parameter_from_cmd(pc_parse_cmd, ac_cmd, "ASCII_CMD=", &off_set, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if (result != OAL_SUCC) {
        return wal_ioctl_ap_config_fail_proc(pc_command);
    }

    if ((0 != oal_strcmp("AP_CFG", ac_cmd))) {
        return wal_ioctl_ap_config_fail_proc(pc_command);
    }

    /* 3.2  解析CHANNEL，目前不处理netd下发的channel信息 */
    pc_parse_cmd += off_set;
    result = wal_get_parameter_from_cmd(pc_parse_cmd, ac_cmd, "CHANNEL=", &off_set, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if (result != OAL_SUCC) {
        return wal_ioctl_ap_config_fail_proc(pc_command);
    }

    /* 3.3  解析MAX_SCB */
    pc_parse_cmd += off_set;
    result = wal_get_parameter_from_cmd(pc_parse_cmd, ac_cmd, "MAX_SCB=", &off_set, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if (result != OAL_SUCC) {
        return wal_ioctl_ap_config_fail_proc(pc_command);
    }

    g_st_ap_config_info.ap_max_user = (uint32_t)oal_atoi(ac_cmd);
    if (oal_net_dev_priv(net_dev) != NULL) {
        ret = wal_set_ap_max_user(net_dev, (uint32_t)oal_atoi(ac_cmd));
    }

    /* 5. 结束释放内存 */
    oal_free(pc_command);
    return ret;
}

/*
 * 函 数 名  : wal_ioctl_get_assoc_list
 * 功能描述  : netd下发命令取得关联设备列表
 * 1.日    期  : 2015年5月22日
  *   修改内容  : 新生成函数
 */
int wal_ioctl_get_assoc_list(oal_net_device_stru *net_dev,
    oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, char *pc_extra)
{
    int32_t ret;
    int32_t l_mret = EOK;
    wal_msg_query_stru st_query_msg;
    wal_msg_stru *rsp_msg = NULL;
    wal_msg_rsp_stru *pst_query_rsp_msg = NULL;
    int8_t *pc_sta_list = NULL;
    oal_netbuf_stru *pst_rsp_netbuf = NULL;

    if (oal_unlikely(oal_any_null_ptr4(net_dev, pst_info, pst_wrqu, pc_extra))) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_get_assoc_list::para null}");
        return -OAL_EFAIL;
    }

    /* 上层在任何时候都可能下发此命令，需要先判断当前netdev的状态并及时返回 */
    if (oal_unlikely(oal_net_dev_priv(net_dev) == NULL)) {
        return -OAL_EFAIL;
    }

    /* 抛事件到wal层处理 */
    st_query_msg.en_wid = WLAN_CFGID_GET_STA_LIST;

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_QUERY,
        WAL_MSG_WID_LENGTH, (uint8_t *)&st_query_msg, OAL_TRUE, &rsp_msg);
    if ((ret != OAL_SUCC) || (rsp_msg == NULL)) {
        return ret;
    }

    /* 处理返回消息 */
    pst_query_rsp_msg = (wal_msg_rsp_stru *)(rsp_msg->auc_msg_data);
    /* 业务处理 */
    if (pst_query_rsp_msg->us_len >= sizeof(oal_netbuf_stru *)) {
        /* 获取hmac保存的netbuf指针 */
        l_mret = memcpy_s(&pst_rsp_netbuf, sizeof(oal_netbuf_stru *),
            pst_query_rsp_msg->auc_value, sizeof(oal_netbuf_stru *));
        if (pst_rsp_netbuf != NULL) {
            /* 保存ap保存的sta地址信息 */
            pc_sta_list = (int8_t *)oal_netbuf_data(pst_rsp_netbuf);
            pst_wrqu->data.length = (uint16_t)(oal_netbuf_len(pst_rsp_netbuf) + 1);
            l_mret += memcpy_s(pc_extra, pst_wrqu->data.length, pc_sta_list, pst_wrqu->data.length);
            pc_extra[oal_netbuf_len(pst_rsp_netbuf)] = '\0';
            oal_netbuf_free(pst_rsp_netbuf);
        } else {
            ret = -OAL_ENOMEM;
        }
    } else {
        oal_print_hex_dump((uint8_t *)rsp_msg->auc_msg_data, pst_query_rsp_msg->us_len,
            HEX_DUMP_GROUP_SIZE, "query msg: ");
        ret = -OAL_EINVAL;
    }

    if ((ret != OAL_SUCC) || (l_mret != EOK)) {
        oam_error_log2(0, OAM_SF_ANY, "{wal_ioctl_get_assoc_list::process failed,ret:%d, mret:%d}", ret, l_mret);
        ret = -OAL_EINVAL;
    }

    oal_free(rsp_msg);
    return ret;
}

OAL_STATIC int wal_ioctl_set_mac_filters_get_mac_mode(int8_t *param, int8_t *val, uint32_t *mac_mode, uint32_t *offset)
{
    uint32_t result;

    result = wal_get_parameter_from_cmd(param, val, "MAC_MODE=", offset, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_set_mac_filters::get_mac_mode err [%u]!}", result);
        return -OAL_EFAIL;
    }

    /* 检查参数是否合法  */
    *mac_mode = (uint32_t)oal_atoi(val);
    if (*mac_mode > 2) { // 2代表参数数值上限
        oam_warning_log4(0, OAM_SF_ANY, "{wal_ioctl_set_mac_filters::invalid MAC_MODE[%c%c%c%c]!}",
            (uint8_t)val[0], (uint8_t)val[1], (uint8_t)val[2], (uint8_t)val[3]); /* 0 1 2 3 cmd para */
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC int wal_ioctl_set_mac_filters_get_mac_cnt(int8_t *param, int8_t *val, uint32_t *mac_cnt, uint32_t *offset)
{
    uint32_t result;

    result = wal_get_parameter_from_cmd(param, val, "MAC_CNT=", offset, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_set_mac_filters::wal_get_parameter_from_cmd err [%u]!}", result);
        return -OAL_EFAIL;
    }
    *mac_cnt = (uint32_t)oal_atoi(val);

    return OAL_SUCC;
}

OAL_STATIC int wal_ioctl_set_mac_filters_white_mode(oal_net_device_stru *net_dev, uint32_t mac_mode, uint32_t mac_cnt)
{
    int32_t  ret = OAL_SUCC;
    uint8_t  auc_mac_addr[WLAN_MAC_ADDR_LEN];

    if ((mac_cnt == 0) && (mac_mode == 2)) { // 2代表白名单模式
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_set_mac_filters::delete all user!}");

        memset_s(auc_mac_addr, WLAN_MAC_ADDR_LEN, 0xff, WLAN_MAC_ADDR_LEN);
        ret = wal_kick_sta(net_dev, auc_mac_addr, sizeof(auc_mac_addr), MAC_AUTH_NOT_VALID);
    }

    return ret;
}

/*
 * 函 数 名  : wal_ioctl_set_mac_filters
 * 功能描述  : netd下发命令设置黑名单或白名单
 * 1.日    期  : 2015年5月22日
  *   修改内容  : 新生成函数
 */
int wal_ioctl_set_mac_filters(oal_net_device_stru *net_dev,
    oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, char *pc_extra)
{
    int8_t  *pc_command = NULL;
    int32_t  ret;
    uint32_t result;
    int8_t   val[WAL_IOCTL_PRIV_SUBCMD_MAX_LEN] = {0};
    int8_t  *param = NULL;
    uint32_t mac_mode, mac_cnt;
    uint32_t off_set = 0;

    if (oal_unlikely(oal_any_null_ptr4(net_dev, pst_info, pst_wrqu, pc_extra))) {
        oam_warning_log0(0, 0, "{wal_ioctl_set_mac_filters::param null}");
        return -OAL_EFAIL;
    }

    /* 1. 申请内存保存netd 下发的命令和数据 */
    pc_command = oal_memalloc((int32_t)(pst_wrqu->data.length + 1));
    if (pc_command == NULL) {
        return -OAL_ENOMEM;
    }

    /* 2. 拷贝netd 命令到内核态中 */
    memset_s(pc_command, (uint32_t)(pst_wrqu->data.length + 1), 0, (uint32_t)(pst_wrqu->data.length + 1));
    result = oal_copy_from_user(pc_command, pst_wrqu->data.pointer, (uint32_t)(pst_wrqu->data.length));
    if (result != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_ioctl_set_mac_filters::oal_copy_from_user: -OAL_EFAIL }");
        oal_free(pc_command);
        return -OAL_EFAIL;
    }
    pc_command[pst_wrqu->data.length] = '\0';

    oal_io_print("wal_ioctl_set_mac_filters,data len:%d \n", pst_wrqu->data.length);

    param = pc_command;

    memset_s(g_st_ap_config_info.ac_ap_mac_filter_mode, sizeof(g_st_ap_config_info.ac_ap_mac_filter_mode),
        0, sizeof(g_st_ap_config_info.ac_ap_mac_filter_mode));
    strncpy_s(g_st_ap_config_info.ac_ap_mac_filter_mode, sizeof(g_st_ap_config_info.ac_ap_mac_filter_mode),
              pc_command, sizeof(g_st_ap_config_info.ac_ap_mac_filter_mode) - 1);

    if (oal_net_dev_priv(net_dev) == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_set_mac_filters::netdevice vap is null,just save it.}");
        oal_free(pc_command);
        return OAL_SUCC;
    }

    /* 3  解析MAC_MODE */
    if (wal_ioctl_set_mac_filters_get_mac_mode(param, val, &mac_mode, &off_set) != OAL_SUCC) {
        oal_free(pc_command);
        return -OAL_EFAIL;
    }

    /* 4 解析MAC_CNT */
    param += off_set;
    if (wal_ioctl_set_mac_filters_get_mac_cnt(param, val, &mac_cnt, &off_set) != OAL_SUCC) {
        oal_free(pc_command);
        return -OAL_EFAIL;
    }

    wal_config_mac_filter(net_dev, pc_command);

    /* 如果是白名单模式，且下发允许为空，即不允许任何设备关联，需要去关联所有已经关联的STA */
    ret = wal_ioctl_set_mac_filters_white_mode(net_dev, mac_mode, mac_cnt);
    oal_free(pc_command);
    return ret;
}

/*
 * 函 数 名  : wal_ioctl_set_ap_sta_disassoc
 * 功能描述  : netd下发命令去关联STA
 * 1.日    期  : 2015年5月22日
  *   修改内容  : 新生成函数
 */
int wal_ioctl_set_ap_sta_disassoc(oal_net_device_stru *net_dev,
    oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, char *pc_extra)
{
    int8_t  *pc_command = NULL;
    int32_t  ret;
    uint32_t result;
    int8_t   ac_parsed_command[WAL_IOCTL_PRIV_SUBCMD_MAX_LEN] = { 0 };
    uint8_t  auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0 };
    uint32_t off_set = 0;

    if (oal_unlikely(oal_any_null_ptr2(net_dev, pst_wrqu))) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_set_ap_sta_disassoc:: param is null}");
        return -OAL_EFAIL;
    }

    /* 1. 申请内存保存netd 下发的命令和数据 */
    pc_command = oal_memalloc((int32_t)(pst_wrqu->data.length + 1));
    if (pc_command == NULL) {
        return -OAL_ENOMEM;
    }

    /* 2. 拷贝netd 命令到内核态中 */
    memset_s(pc_command, (uint32_t)(pst_wrqu->data.length + 1), 0, (uint32_t)(pst_wrqu->data.length + 1));
    result = oal_copy_from_user(pc_command, pst_wrqu->data.pointer, (uint32_t)(pst_wrqu->data.length));
    if (result != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_ioctl_set_ap_sta_disassoc::oal_copy_from_user: -OAL_EFAIL }");
        oal_free(pc_command);
        return -OAL_EFAIL;
    }
    pc_command[pst_wrqu->data.length] = '\0';

    /* 3. 解析命令获取MAC */
    result = wal_get_parameter_from_cmd(pc_command, ac_parsed_command, "MAC=",
        &off_set, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_set_ap_sta_disassoc::get_parameter_from_cmd MAC err[%u]}", result);
        oal_free(pc_command);
        return -OAL_EINVAL;
    }
    /* 3.1  检查参数是否符合MAC长度 */
    if ((WLAN_MAC_ADDR_LEN << BIT_OFFSET_1) != OAL_STRLEN(ac_parsed_command)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_set_ap_sta_disassoc::invalid MAC format}");
        oal_free(pc_command);
        return -OAL_EINVAL;
    }

    /* 将字符 ac_name 转换成数组 mac_add[6] */
    oal_strtoaddr(ac_parsed_command, sizeof(ac_parsed_command), auc_mac_addr, WLAN_MAC_ADDR_LEN);

    oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_set_ap_sta_disassoc::Geting CMD from APP to DISASSOC!!}");
    ret = wal_kick_sta(net_dev, auc_mac_addr, sizeof(auc_mac_addr), MAC_AUTH_NOT_VALID);

    /* 5. 结束释放内存 */
    oal_free(pc_command);
    return ret;
}
/*
 * 函 数 名  : wal_hipriv_get_mac_addr
 * 功能描述  : 统计指定tid的吞吐量
 * 1.日    期  : 2014年1月14日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_get_mac_addr(int8_t *pc_param,
    uint8_t auc_mac_addr[], uint32_t *pul_total_offset)
{
    uint32_t off_set = 0;
    uint32_t result ;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};

    /* 获取mac地址 */
    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_get_mac_addr::get cmd one arg err[%d]!}", result);
        return result;
    }
    oal_strtoaddr(ac_name, sizeof(ac_name), auc_mac_addr, WLAN_MAC_ADDR_LEN);

    *pul_total_offset = off_set;

    return OAL_SUCC;
}

void wal_init_wlan_ap_mode_set_max_user_count_and_mac_filter(oal_net_device_stru *net_dev,
    oal_wireless_dev_stru *pst_wdev)
{
    if (NL80211_IFTYPE_AP == pst_wdev->iftype) {
        /* AP模式初始化，初始化配置最大用户数和mac地址过滤模式 */
        if (g_st_ap_config_info.ap_max_user > 0) {
            wal_set_ap_max_user(net_dev, g_st_ap_config_info.ap_max_user);
        }

        if (OAL_STRLEN(g_st_ap_config_info.ac_ap_mac_filter_mode) > 0) {
            wal_config_mac_filter(net_dev, (int8_t *)g_st_ap_config_info.ac_ap_mac_filter_mode);
        }
    }
}
#ifdef _PRE_WLAN_CHBA_MGMT
/*
 * 函 数 名  : wal_setup_chba
 * 功能描述  : 创建&启动 chba vap。
 * 1.日    期  : 2021年1月26日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_setup_chba(oal_net_device_stru *net_dev)
{
    int32_t ret;
    mac_vap_stru *mac_vap = NULL;
    if (oal_unlikely(net_dev == NULL || net_dev->ieee80211_ptr == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_setup_chba::net_dev or ieee80211_ptr null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (oal_netdevice_flags(net_dev) & OAL_IFF_RUNNING) {
        /* 切换到AP前如果网络设备处于UP状态，需要先down wlan0网络设备 */
        oal_io_print("wal_setup_chba:stop netdevice:%.16s", net_dev->name);
        oam_warning_log0(0, OAM_SF_CFG, "{wal_setup_chba:: oal_iff_running! now, stop netdevice}");
        wal_netdev_stop(net_dev);
    }

    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap) {
        if (mac_vap->en_vap_state != MAC_VAP_STATE_INIT) {
            /* 切换到AP前如果网络设备处于UP状态，需要先down wlan0网络设备 */
            oal_io_print("wal_setup_chba:stop netdevice:%s", net_dev->name);
            wal_netdev_stop(net_dev);
        }
    }
    net_dev->ieee80211_ptr->iftype = NL80211_IFTYPE_AP;
    /* add vap */
    ret = wal_init_wlan_vap(net_dev);
    if (ret != OAL_SUCC) {
        return ret;
    }
    oam_warning_log0(0, OAM_SF_ANY, "{wal_setup_chba:: start ap}");
    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_set_chba_mac_addr
 * 功能描述  : 初始化chba netdev的mac地址
 * 1.日    期  : 2021年1月25日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
static void wal_set_chba_mac_addr(oal_net_device_stru *net_dev)
{
    uint8_t primary_mac_addr[WLAN_MAC_ADDR_LEN] = { 0 }; /*  */
    oal_random_ether_addr(primary_mac_addr, WLAN_MAC_ADDR_LEN);
    primary_mac_addr[BYTE_OFFSET_0] &= (~0x02); /*  wlan0 MAC[0] bit1 需要设置为0 */
    primary_mac_addr[BYTE_OFFSET_1] = 0x11;
    primary_mac_addr[BYTE_OFFSET_2] = 0x05;
    oal_set_mac_addr((uint8_t *)oal_netdevice_mac_addr(net_dev), primary_mac_addr);
}
/*
 * 函 数 名  : wal_chba_netdev_set
 * 功能描述  : 初始化chba netdev配置
 * 1.日    期  : 2021年1月25日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
static void wal_chba_netdev_set(oal_net_device_stru *net_dev, oal_wireless_dev_stru *wdev, oal_wiphy_stru *wiphy)
{
    memset_s(wdev, sizeof(oal_wireless_dev_stru), 0, sizeof(oal_wireless_dev_stru));
    wal_init_wlan_netdev_set_netdev(net_dev, wdev);
    wdev->netdev = net_dev;
    wdev->iftype = NL80211_IFTYPE_AP;
    wdev->wiphy = wiphy;
    oal_netdevice_flags(net_dev) &= ~OAL_IFF_RUNNING; /* 将net device的flag设为down */
    wal_set_chba_mac_addr(net_dev);
    wal_init_netdev_priv(net_dev);
}

/*
 * 函 数 名  : wal_init_chba_netdev
 * 功能描述  : 初始化chba netdev
 * 1.日    期  : 2021年1月25日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_init_chba_netdev(oal_wiphy_stru *wiphy, const char *dev_name)
{
    mac_device_stru *mac_device = NULL;
    oal_net_device_stru *net_dev = NULL;
    oal_wireless_dev_stru *wdev = NULL;
    mac_wiphy_priv_stru *wiphy_priv = NULL;
    int32_t ret;
    if ((g_optimized_feature_switch_bitmap & BIT(CUSTOM_OPTIMIZE_CHBA)) == 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_chba_netdev::CHBA unsupported, Not create chba netdev!!}");
        return OAL_SUCC;
    }

    wiphy_priv = (mac_wiphy_priv_stru *)(oal_wiphy_priv(wiphy));
    if (wiphy_priv == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_device = wiphy_priv->pst_mac_device;
    if (mac_device == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 创建netdev */
    /* 如果创建的net device已经存在，直接返回 */
    /* 根据dev_name找到dev */
    net_dev = wal_config_get_netdev(dev_name, OAL_STRLEN(dev_name));
    if (net_dev != NULL) {
        /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
        oal_dev_put(net_dev);
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_chba_netdev::the net_device is already exist!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 此函数第一个入参代表私有长度，此处不涉及为0 */
    net_dev = oal_net_alloc_netdev_mqs(sizeof(oal_netdev_priv_stru),
        dev_name, oal_ether_setup, WLAN_NET_QUEUE_BUTT, 1);
    if (oal_unlikely(net_dev == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_init_chba_netdev::oal_net_alloc_netdev return null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    wdev = (oal_wireless_dev_stru *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL,
        sizeof(oal_wireless_dev_stru), OAL_FALSE);
    if (oal_unlikely(wdev == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_init_chba_netdev::alloc mem, wdev null!}");
        oal_net_free_netdev(net_dev);
        return OAL_ERR_CODE_PTR_NULL;
    }
    wal_chba_netdev_set(net_dev, wdev, wiphy);
    /* 注册net_device */
    ret = hmac_net_register_netdev(net_dev);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_init_chba_netdev::hmac_net_register_netdev err:%d!}", ret);
        oal_mem_free_m(wdev, OAL_FALSE);
        oal_net_free_netdev(net_dev);
        return ret;
    }

    oam_warning_log0(0, OAM_SF_ANY, "{wal_init_chba_netdev::succ!}");
    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D

/*
 * 功能描述  : HiD2D场景下更新ACS算法状态
 * 1.日    期  : 2020年04月20日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_ioctl_update_acs_state(oal_net_device_stru *net_dev, int8_t *param)
{
    wal_msg_write_stru              write_msg;
    uint32_t                        off_set;
    int8_t                          name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    uint32_t                        ret;
    int32_t                         rslt;
    uint32_t                        value;

    /* 获取参数 */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    param += off_set;
    value = (oal_bool_enum_uint8)oal_atoi(name);

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_HID2D_ACS_STATE, sizeof(uint32_t));

    /* 设置配置命令参数 */
    *((uint32_t *)(write_msg.auc_value)) = value;

    rslt = wal_send_cfg_event(net_dev,
        WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint32_t),
        (uint8_t *)&write_msg,
        OAL_FALSE,
        NULL);
    if (oal_unlikely(rslt != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "{HiD2D::return err code [%d]!}\r\n", rslt);
        return (uint32_t)rslt;
    }
    return OAL_SUCC;
}

static oal_bool_enum_uint8 wal_ioctl_chan_meas_chan_is_valid(mac_vap_stru *mac_vap,
    mac_hid2d_link_meas_stru *meas_cmd)
{
    hmac_hid2d_chan_stru *candidate_list = NULL;
    uint8_t max_chan_cnt;

    if (meas_cmd->scan_band == WLAN_BAND_5G) {
        candidate_list = g_aus_channel_candidate_list_5g;
        max_chan_cnt = HMAC_HID2D_CHANNEL_NUM_5G;
    } else if (meas_cmd->scan_band == WLAN_BAND_2G) {
        candidate_list = g_aus_channel_candidate_list_2g;
        max_chan_cnt = HMAC_HID2D_CHANNEL_NUM_2G;
    } else {
        oam_error_log1(mac_vap->uc_vap_id, 0,
            "wal_ioctl_chan_meas_chan_is_valid::band[%d] invalidn!", meas_cmd->scan_band);
        return OAL_FALSE;
    }
    if (meas_cmd->scan_chan >= max_chan_cnt) {
        oam_error_log1(mac_vap->uc_vap_id, 0,
            "wal_ioctl_chan_meas_chan_is_valid::scan_chan[%d] invalid!", meas_cmd->scan_chan);
        return OAL_FALSE;
    }
    // 判断是否为合法信道号，以及是否在管制域内
    if (mac_is_channel_num_valid(meas_cmd->scan_band,
        candidate_list[meas_cmd->scan_chan].uc_chan_number, OAL_FALSE) != OAL_SUCC) {
        oam_error_log1(0, 0, "wal_ioctl_chan_meas_chan_is_valid::chan[%d] invalid, or not in regdomain!",
            candidate_list[meas_cmd->scan_chan].uc_chan_number);
        return OAL_FALSE;
    }
#ifdef _PRE_WLAN_CHBA_MGMT
    if (mac_vap->chba_mode == CHBA_MODE &&
        hmac_chba_is_valid_channel(candidate_list[meas_cmd->scan_chan].uc_chan_number) != OAL_SUCC) {
        oam_error_log1(mac_vap->uc_vap_id, 0, "wal_ioctl_chan_meas_chan_is_valid::chan[%d] invalid or is dfs chan!",
            candidate_list[meas_cmd->scan_chan].uc_chan_number);
        return OAL_FALSE;
    }
#endif
    return OAL_TRUE;
}
#define WAL_CHAN_MEAS_START_BY_CHAN 4
#define WAL_CHAN_MEAS_MAX_TIME_MS 10
#define WAL_CHAN_MEAS_SCAN_TIME_OFFSET 5
static uint32_t wal_ioctl_hid2d_link_meas_paras_check(mac_vap_stru *mac_vap,
    mac_hid2d_link_meas_stru *hid2d_meas_cmd)
{
    if (!wal_ioctl_chan_meas_chan_is_valid(mac_vap, hid2d_meas_cmd)) {
        return OAL_FAIL;
    }
    // 只校验信道测量扫描相关参数
    if (hid2d_meas_cmd->link_meas_cmd_type != HMAC_HID2D_LINK_MEAS_START_BY_CHAN) {
        return OAL_SUCC;
    }
    // 如果命令下发扫描时间大于15ms直接取15ms
    if (hid2d_meas_cmd->meas_time > WAL_CHAN_MEAS_MAX_TIME_MS) {
        hid2d_meas_cmd->meas_time = WAL_CHAN_MEAS_MAX_TIME_MS + WAL_CHAN_MEAS_SCAN_TIME_OFFSET;
    } else {
        hid2d_meas_cmd->meas_time = hid2d_meas_cmd->meas_time + WAL_CHAN_MEAS_SCAN_TIME_OFFSET;
    }
#ifdef _PRE_WLAN_CHBA_MGMT
    if ((mac_vap->chba_mode == CHBA_MODE) && (hid2d_meas_cmd->scan_band != WLAN_BAND_5G)) {
        oam_error_log1(mac_vap->uc_vap_id, 0,
            "wal_ioctl_hid2d_link_meas_paras_check::para[%d] invalid, chba only support scan 5G!",
            hid2d_meas_cmd->scan_band);
        return OAL_FAIL;
    }
#endif
    return OAL_SUCC;
}

/* 信道测量命令下发时需要判断当前是否已存在测量扫描，必须等待扫描完成才允许下发下一次扫描 */
OAL_STATIC oal_bool_enum_uint8 wal_ioctl_chan_meas_start_check(mac_vap_stru *mac_vap)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev(mac_vap->uc_device_id);
#ifdef _PRE_WLAN_CHBA_MGMT
    hmac_chba_mgmt_info_stru *chba_mgmt_info = hmac_chba_get_mgmt_info();
#endif
    if (hmac_device == NULL) {
        return OAL_TRUE;
    }
    if (hmac_device->st_scan_mgmt.en_is_scanning) {
        oam_warning_log0(mac_vap->uc_vap_id, 0,
            "wal_ioctl_chan_meas_is_running_check::other scan is running, return.");
        return OAL_TRUE;
    }
    if (hmac_device->st_scan_mgmt.pst_sched_scan_req != NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, 0, "wal_ioctl_chan_meas_start_check::pno scan is running, return.");
        return OAL_TRUE;
    }
#ifdef _PRE_WLAN_CHBA_MGMT
    if ((mac_vap->chba_mode == CHBA_MODE) && (hmac_chba_get_sync_state() <= CHBA_NON_SYNC)) {
        oam_warning_log0(mac_vap->uc_vap_id, 0,
            "wal_ioctl_chan_meas_start_check:chba is non sync or init state, chan meas refused!");
        return OAL_TRUE;
    }
    if ((mac_vap->chba_mode == CHBA_MODE) && (chba_mgmt_info->chan_switch_info.in_island_chan_switch ||
        chba_mgmt_info->chan_switch_info.csa_lost_device_num > 0)) {
        oam_warning_log0(mac_vap->uc_vap_id, 0,
            "wal_ioctl_chan_meas_start_check:chba chan switch processing, chan meas refused!");
        return OAL_TRUE;
    }
#endif
    return OAL_FALSE;
}

OAL_STATIC uint32_t wal_ioctl_hid2d_meas_cfg_send_event(oal_net_device_stru *net_dev,
    wal_msg_write_stru *write_msg, mac_hid2d_link_meas_stru *hid2d_meas_cmd)
{
    int32_t rslt;
    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(write_msg, WLAN_CFGID_HID2D_LINK_MEAS, sizeof(mac_hid2d_link_meas_stru));

    /* 设置配置命令参数 */
    if (memcpy_s(write_msg->auc_value, sizeof(write_msg->auc_value),
        (const void *)hid2d_meas_cmd, sizeof(mac_hid2d_link_meas_stru)) != EOK) {
        return OAL_FAIL;
    }

    rslt = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_hid2d_link_meas_stru), (uint8_t *)write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(rslt != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "{HiD2D::return err code [%d]!}\r\n", rslt);
        return (uint32_t)rslt;
    }
    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_hipriv_hid2d_link_meas
 * 功能描述  : HiD2D场景下自动信道切换总开关
 * 1.日    期  : 2020年04月20日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_ioctl_hid2d_link_meas(oal_net_device_stru *net_dev, int8_t *param)
{
    wal_msg_write_stru              write_msg;
    mac_hid2d_link_meas_stru        hid2d_meas_cmd;
    uint32_t                        off_set, ret;
    int8_t                          name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    mac_vap_stru                    *mac_vap = oal_net_dev_priv(net_dev);

    if (mac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (wal_ioctl_chan_meas_start_check(mac_vap)) {
        return OAL_FAIL;
    }

    /* 获取meas type */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    param += off_set;
    hid2d_meas_cmd.link_meas_cmd_type = (uint8_t)oal_atoi(name);

    /* 获取scan chan */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    param += off_set;
    hid2d_meas_cmd.scan_chan = (uint8_t)oal_atoi(name);

    /* 获取scan interval */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    param += off_set;
    hid2d_meas_cmd.scan_interval = (uint16_t)oal_atoi(name);

    /* 获取scan time */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    param += off_set;
    hid2d_meas_cmd.meas_time = (uint16_t)oal_atoi(name);

    /* 获取scan band */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    param += off_set;
    hid2d_meas_cmd.scan_band = (uint8_t)oal_atoi(name);
    if (wal_ioctl_hid2d_link_meas_paras_check(mac_vap, &hid2d_meas_cmd) != OAL_SUCC) {
        return OAL_FAIL;
    }
    return wal_ioctl_hid2d_meas_cfg_send_event(net_dev, &write_msg, &hid2d_meas_cmd);
}

#endif


#ifdef _PRE_WLAN_FEATURE_SNIFFER
uint32_t wal_ioctl_set_sniffer_config(oal_net_device_stru *net_dev, mac_cfg_sniffer_param_stru *cfg_sniffer_param)
{
    mac_cfg_sniffer_param_stru *pst_cfg_sniffer_param = NULL;
    wal_msg_write_stru write_msg = {0};
    uint32_t mac_address_index, result;
    int32_t ret;
    wal_msg_stru *rsp_msg = NULL;

    oam_warning_log4(0, OAM_SF_ANY, "{wal_ioctl_set_sniffer_config::uc_sniffer_mode[%d],mac[%02X:XX:XX:XX:%02X:%02X].}",
        cfg_sniffer_param->uc_sniffer_mode, cfg_sniffer_param->auc_mac_addr[MAC_ADDR_0],
        cfg_sniffer_param->auc_mac_addr[MAC_ADDR_4], cfg_sniffer_param->auc_mac_addr[MAC_ADDR_5]);

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_SNIFFER, sizeof(mac_cfg_sniffer_param_stru));

    /* 设置配置命令参数 */
    pst_cfg_sniffer_param = (mac_cfg_sniffer_param_stru *)(write_msg.auc_value);
    for (mac_address_index = 0; mac_address_index < WLAN_MAC_ADDR_LEN; mac_address_index++) {
        pst_cfg_sniffer_param->auc_mac_addr[mac_address_index] =
            cfg_sniffer_param->auc_mac_addr[mac_address_index];
    }
    pst_cfg_sniffer_param->uc_sniffer_mode = cfg_sniffer_param->uc_sniffer_mode;

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_sniffer_param_stru),
                             (uint8_t *)&write_msg,
                             OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_sniffer::err [%d]!}", ret);
        return (uint32_t)ret;
    }
    /* 读取返回的错误码 */
    result = wal_check_and_release_msg_resp(rsp_msg);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_sniffer fail, err code[%u]!}", result);
        return result;
    }

    return OAL_SUCC;
}

void wal_ioctl_sniffer_switch_handle(oal_net_device_stru *net_dev, uint8_t sniffer_mode)
{
    if (oal_value_eq_any2(sniffer_mode, WLAN_SNIFFER_TRAVEL_CAP_ON, WLAN_SINFFER_ON)) {
        wal_clear_continuous_disable_enable_wifi_cnt(); // 开启sniffer后将计数清零
    }
}
static void wal_ioctl_set_sniffer_hmac_device_flag(oal_net_device_stru *net_dev, uint8_t sniffer_mode)
{
    oal_wireless_dev_stru *wdev = NULL;
    mac_wiphy_priv_stru *wiphy_priv = NULL;
    mac_device_stru *mac_device = NULL;
    hmac_device_stru *hmac_device = NULL;

    wdev = oal_netdevice_wdev(net_dev);
    if (wdev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_set_sniffer_hmac_device_flag::wdev is null!}");
        return;
    }

    wiphy_priv = (mac_wiphy_priv_stru *)oal_wiphy_priv(wdev->wiphy);
    if (wiphy_priv == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_set_sniffer_hmac_device_flag::wiphy_priv is null!}");
        return;
    }
    mac_device = wiphy_priv->pst_mac_device;
    if (mac_device == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_set_sniffer_hmac_device_flag::mac_device is null!}");
        return;
    }
    hmac_device = hmac_res_get_mac_dev(mac_device->uc_device_id);
    if (hmac_device == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_set_sniffer_hmac_device_flag::hmac_device is null!}");
        return;
    }
    hmac_device->sniffer_mode = sniffer_mode;
}

uint32_t wal_ioctl_set_sniffer(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    uint32_t off_set = 0;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint8_t uc_sniffer_mode;
    int32_t l_idx = 0;
    mac_cfg_sniffer_param_stru st_cfg_sniffer_param; /* 临时保存sniffer配置信息 */
    uint32_t result;

    memset_s((uint8_t *)&st_cfg_sniffer_param, sizeof(st_cfg_sniffer_param), 0, sizeof(st_cfg_sniffer_param));
    /* 获取sniffer开关标志 */
    result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_sniffer::get cmd one arg err[%d]!}", result);
        return result;
    }

    /* 输入命令合法性检测 */
    while (ac_arg[l_idx] != '\0') {
        if (isdigit(ac_arg[l_idx])) {
            l_idx++;
            continue;
        } else {
            l_idx++;
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_sniffer::input illegal!}");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }
    /* 将命令参数值字符串转化为整数 */
    uc_sniffer_mode = (uint8_t)oal_atoi(ac_arg);
    if (uc_sniffer_mode > WLAN_SNIFFER_STATE_BUTT) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_sniffer::sniffer mode invalid.}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    wal_ioctl_set_sniffer_hmac_device_flag(net_dev, uc_sniffer_mode);
    /*
     * 如果sniffer使能并且当前vap不处于单sta模式返回；伴随sniffer关闭下发，则不判断，防止sniffer使能期间vap模式发生变化
     * 而导致之前host置位无法被清理
     */
    if (uc_sniffer_mode && !wal_sniffer_is_single_sta_mode(net_dev)) {
        return OAL_FAIL;
    }

    st_cfg_sniffer_param.uc_sniffer_mode = uc_sniffer_mode;
    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;
    /* 获取mac地址 */
    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_sniffer::get cmd one arg err[%d]!}", result);
        return result;
    }

    oal_strtoaddr(ac_name, sizeof(ac_name), st_cfg_sniffer_param.auc_mac_addr, WLAN_MAC_ADDR_LEN);

    result = wal_ioctl_set_sniffer_config(net_dev, &st_cfg_sniffer_param);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_sniffer::wal_ioctl_set_sniffer_config err[%d]!}", result);
        return result;
    }
    // 开启sniffer将异常计数清零；关闭sniffer进行wlan0 down/up
    wal_ioctl_sniffer_switch_handle(net_dev, uc_sniffer_mode);

    return OAL_SUCC;
}

uint32_t wal_ioctl_set_monitor_mode(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    uint32_t result;
    int32_t ret;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint8_t uc_monitor_mode;
    int32_t l_idx = 0;
    wal_msg_stru *rsp_msg = NULL;

    /* 获取monitor mode开关标志 */
    result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_enable_monitor_mode::get cmd one arg err[%d]!}", result);
        return result;
    }

    /* 输入命令合法性检测 */
    while (ac_arg[l_idx] != '\0') {
        if (isdigit(ac_arg[l_idx])) {
            l_idx++;
            continue;
        } else {
            l_idx++;
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_enable_monitor_mode::input illegal!}");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    /* 将命令参数值字符串转化为整数 */
    uc_monitor_mode = (uint8_t)oal_atoi(ac_arg);
    if (uc_monitor_mode > WLAN_MONITOR_STATE_BUTT) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_enable_monitor_mode::input should be 0 or 1.}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    if (uc_monitor_mode && !wal_sniffer_is_single_sta_mode(net_dev)) {
        return OAL_FAIL;
    }

    *(uint8_t *)(write_msg.auc_value) = uc_monitor_mode;

    oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_enable_monitor_mode::uc_sniffer_mode [%d].}", uc_monitor_mode);

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_MONITOR_MODE, sizeof(uint8_t));

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint8_t),
                             (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_enable_monitor_mode::err [%d]!}", ret);
        return (uint32_t)ret;
    }

    /* 读取返回的错误码 */
    result = wal_check_and_release_msg_resp(rsp_msg);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_enable_monitor_mode fail, err code[%u]!}", result);
        return result;
    }

    return OAL_SUCC;
}

#endif
/*
 * 功能描述  : 设置过滤特定数据包状态使能
 * 1.日    期  : 2019年2月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_set_assigned_filter_enable(int32_t filter_id, int32_t on)
{
    int32_t ret;
    uint32_t cmd_id;
    int32_t params[CMD_PARAMS_MAX_CNT] = { 0 };
    uint32_t offset;
    mac_vap_stru *mac_vap = NULL;
    oal_net_device_stru *net_dev = NULL;

#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_set_assigned_filter_enable:: dfr_process_status[%d]!}",
                         g_st_dfr_info.bit_device_reset_process_flag);
        return -OAL_EFAIL;
    }
#endif  // #ifdef _PRE_WLAN_FEATURE_DFR

    if (on < 0) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_set_assigned_filter_enable::Invalid input, on/off %d!}", on);
        return -OAL_EINVAL;
    }

    net_dev = wal_config_get_netdev("wlan0", OAL_STRLEN("wlan0"));
    if (net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_set_assigned_filter_enable::wlan0 not exist!}");
        return -OAL_EINVAL;
    }
    /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
    oal_dev_put(net_dev);

    /* vap未创建时，不处理下发的命令 */
    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_set_assigned_filter_enable::vap not created yet, ignore the cmd!}");
        return -OAL_EINVAL;
    }

    if (mac_vap->st_cap_flag.bit_icmp_filter != OAL_TRUE) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_set_assigned_filter_enable::Func not enable!}");
        return -OAL_EINVAL;
    }

    ret = (int32_t)wal_get_cmd_id("icmp_filter", &cmd_id, &offset);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "wal_set_assigned_filter_enable:find icmp_filter cmd is fail");
        return -OAL_EINVAL;
    }
    params[0] = filter_id;
    params[1] = (on > 0) ? OAL_TRUE : OAL_FALSE;
    if (wal_process_cmd_params(net_dev, cmd_id, params) != OAL_SUCC) {
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}

int32_t wal_set_ip_filter_enable_cfg(oal_net_device_stru *net_dev, mac_vap_stru *mac_vap, int32_t on)
{
    wal_msg_write_stru write_msg = {0};
    mac_ip_filter_cmd_stru st_ip_filter_cmd;
    mac_ip_filter_cmd_stru *pst_cmd_info = NULL;
    oal_netbuf_stru *pst_netbuf = NULL;
    int32_t ret;
    /* 准备配置命令 */
    uint32_t netbuf_len = sizeof(st_ip_filter_cmd);
    memset_s((uint8_t *)&st_ip_filter_cmd, netbuf_len, 0, netbuf_len);
    st_ip_filter_cmd.en_cmd = MAC_IP_FILTER_ENABLE;
    st_ip_filter_cmd.en_enable = (on > 0) ? OAL_TRUE : OAL_FALSE;

    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_set_ip_filter_enable::IP_filter on/off(%d).}",
                     st_ip_filter_cmd.en_enable);

    /* 申请空间 缓存过滤规则 */
    pst_netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, netbuf_len, OAL_NETBUF_PRIORITY_MID);
    if (pst_netbuf == NULL) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
            "{wal_set_ip_filter_enable::netbuf alloc null,size %d.}", netbuf_len);
        return -OAL_EINVAL;
    }
    memset_s(((uint8_t *)oal_netbuf_data(pst_netbuf)), netbuf_len, 0, netbuf_len);
    pst_cmd_info = (mac_ip_filter_cmd_stru *)oal_netbuf_data(pst_netbuf);

    /* 记录过滤规则 */
    ret = memcpy_s((uint8_t *)pst_cmd_info, netbuf_len, (uint8_t *)(&st_ip_filter_cmd), netbuf_len);
    oal_netbuf_put(pst_netbuf, netbuf_len);

    /* 填写 msg 消息头 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_IP_FILTER, sizeof(oal_netbuf_stru*));

    /* 将申请的netbuf首地址填写到msg消息体内 */
    ret += memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value), (uint8_t *)&pst_netbuf, sizeof(oal_netbuf_stru*));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_set_ip_filter_enable::memcpy fail!");
        oal_netbuf_free(pst_netbuf);
        return -OAL_EINVAL;
    }

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(oal_netbuf_stru*), (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(mac_vap->uc_vap_id, 0, "{wal_set_ip_filter_enable::send_cfg_event fail:%d}", ret);
        oal_netbuf_free(pst_netbuf);
        return ret;
    }
    return OAL_SUCC;
}
/*
 * 功能描述  : 设置ip过滤的使能状态
 * 1.日    期  : 2017年4月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_set_ip_filter_enable(int32_t on)
{
    mac_vap_stru *mac_vap = NULL;
    oal_net_device_stru *net_dev = NULL;

#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_set_ip_filter_enable:: dfr_process_status[%d]!}",
                         g_st_dfr_info.bit_device_reset_process_flag);
        return -OAL_EFAIL;
    }
#endif  // #ifdef _PRE_WLAN_FEATURE_DFR

    if (on < 0) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_set_ip_filter_enable::Invalid input parameter, on/off %d!}", on);
        return -OAL_EINVAL;
    }

    net_dev = wal_config_get_netdev("wlan0", OAL_STRLEN("wlan0"));
    if (net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_set_ip_filter_enable::wlan0 not exist!}");
        return -OAL_EINVAL;
    }
    /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
    oal_dev_put(net_dev);

    /* vap未创建时，不处理下发的命令 */
    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_set_ip_filter_enable::vap not created yet, ignore the cmd!}");
        return -OAL_EINVAL;
    }

    if (mac_vap->st_cap_flag.bit_ip_filter != OAL_TRUE) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_set_ip_filter_enable::Func not enable,ignore!}");
        return -OAL_EINVAL;
    }

    return wal_set_ip_filter_enable_cfg(net_dev, mac_vap, on);
}

void wal_add_ip_filter_items_loop_process(wal_hw_wifi_filter_item *items, mac_ip_filter_cmd_stru *cmd_info,
    uint8_t item_count)
{
    uint32_t items_idx;
    for (items_idx = 0; items_idx < item_count; items_idx++) {
        cmd_info->ast_filter_items[items_idx].uc_protocol = (uint8_t)items[items_idx].protocol;
        cmd_info->ast_filter_items[items_idx].us_port = (uint16_t)items[items_idx].port;
        if (mac_is_support_ipv6_filter() != OAL_TRUE) {
            continue;
        }
        if (items[items_idx].filter_cnt == ETHER_TYPE_IP) {
            cmd_info->ast_filter_items[items_idx].ip_version = MAC_IP_FILTER_IPV4;
        } else if (items[items_idx].filter_cnt == ETHER_TYPE_IPV6) {
            cmd_info->ast_filter_items[items_idx].ip_version = MAC_IP_FILTER_IPV6;
        }
    }
}

int32_t wal_add_ip_filter_items_process(oal_net_device_stru *net_dev, mac_vap_stru *mac_vap,
    wal_hw_wifi_filter_item *pst_items, int32_t l_count)
{
    int32_t ret;
    uint32_t netbuf_len;
    oal_netbuf_stru *pst_netbuf = NULL;
    wal_msg_write_stru write_msg = {0};
    mac_ip_filter_cmd_stru st_ip_filter_cmd;
    mac_ip_filter_cmd_stru *pst_cmd_info = NULL;

    /* 准备配置事件 */
    memset_s((uint8_t *)&st_ip_filter_cmd, sizeof(st_ip_filter_cmd), 0, sizeof(st_ip_filter_cmd));
    st_ip_filter_cmd.en_cmd = MAC_IP_FILTER_UPDATE_BTABLE;

    /* 由于本地名单大小限制，取能收纳的规则条目数最小值 */
    st_ip_filter_cmd.uc_item_count =
        (uint8_t)oal_min((MAC_MAX_IP_FILTER_BTABLE_SIZE / sizeof(mac_ip_filter_item_stru)), (uint32_t)l_count);
    if (st_ip_filter_cmd.uc_item_count < l_count) {
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_ANY,
            "{wal_add_ip_filter_items::Btable(%d) < %d items!}", st_ip_filter_cmd.uc_item_count, l_count);
    }

    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{updating btable, items(%d).}", st_ip_filter_cmd.uc_item_count);

    /* 选择申请事件空间的大小 */
    netbuf_len = (st_ip_filter_cmd.uc_item_count * sizeof(mac_ip_filter_item_stru)) + sizeof(st_ip_filter_cmd);

    /* 申请空间 缓存过滤规则 */
    pst_netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, netbuf_len, OAL_NETBUF_PRIORITY_MID);
    if (pst_netbuf == NULL) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{netbuf alloc null,size %d.}", netbuf_len);
        return -OAL_EINVAL;
    }
    memset_s(((uint8_t *)oal_netbuf_data(pst_netbuf)), netbuf_len, 0, netbuf_len);
    pst_cmd_info = (mac_ip_filter_cmd_stru *)oal_netbuf_data(pst_netbuf);

    /* 记录过滤规则 */
    ret = memcpy_s((uint8_t *)pst_cmd_info, netbuf_len, (uint8_t *)(&st_ip_filter_cmd), sizeof(st_ip_filter_cmd));
    oal_netbuf_put(pst_netbuf, netbuf_len);

    wal_add_ip_filter_items_loop_process(pst_items, pst_cmd_info, st_ip_filter_cmd.uc_item_count);

    /* 填写 msg 消息头 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_IP_FILTER, sizeof(oal_netbuf_stru *));

    /* 将申请的netbuf首地址填写到msg消息体内 */
    ret += memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value),
        (uint8_t *)&pst_netbuf, sizeof(oal_netbuf_stru *));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hwifi_config_init_fcc_ce_txpwr_nvram::memcpy fail!");
        oal_netbuf_free(pst_netbuf);
        return -OAL_EINVAL;
    }

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(oal_netbuf_stru *), (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_add_ip_filter_items::wal_send_cfg_event fail[%d]!}", ret);
        oal_netbuf_free(pst_netbuf);
        return ret;
    }
    return OAL_SUCC;
}
/*
 * 功能描述  : 添加ip过滤的黑名单
 */
int32_t wal_add_ip_filter_items(wal_hw_wifi_filter_item *pst_items, int32_t l_count)
{
    mac_vap_stru *mac_vap = NULL;
    oal_net_device_stru *net_dev = NULL;

#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_add_ip_filter_items:: dfr_process_status[%d]!}",
                         g_st_dfr_info.bit_device_reset_process_flag);
        return -OAL_EFAIL;
    }
#endif  // #ifdef _PRE_WLAN_FEATURE_DFR

    if ((pst_items == NULL) || (l_count <= 0)) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_add_ip_filter_items::Invalid input para, l_count %d!}",
            l_count);
        return -OAL_EINVAL;
    }

    net_dev = wal_config_get_netdev("wlan0", OAL_STRLEN("wlan0"));
    if (net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_add_ip_filter_items::wlan0 not exist!}");
        return -OAL_EINVAL;
    }
    /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
    oal_dev_put(net_dev);

    /* vap未创建时，不处理下发的命令 */
    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_add_ip_filter_items::vap not created yet, ignore the cmd!.}");
        return -OAL_EINVAL;
    }

    if (mac_vap->st_cap_flag.bit_ip_filter != OAL_TRUE) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_add_ip_filter_items::Func disable, ignore cmd}");
        return -OAL_EINVAL;
    }

    return wal_add_ip_filter_items_process(net_dev, mac_vap, pst_items, l_count);
}

/*
 * 功能描述  : 清除ip过滤的黑名单抛事件到wal层处理
 * 1.日    期  : 2020年8月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_clear_ip_filter_config(oal_net_device_stru *net_dev, mac_vap_stru *mac_vap)
{
    mac_ip_filter_cmd_stru ip_filter_cmd;
    mac_ip_filter_cmd_stru *cmd_info = NULL;
    oal_netbuf_stru *netbuf = NULL;
    wal_msg_write_stru write_msg = {0};
    uint32_t netbuf_len;
    int32_t ret;
    uint16_t len;

    /* 清理黑名单 */
    memset_s((uint8_t *)&ip_filter_cmd, sizeof(ip_filter_cmd), 0, sizeof(ip_filter_cmd));
    ip_filter_cmd.en_cmd = MAC_IP_FILTER_CLEAR;

    oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_clear_ip_filter_config::Now start clearing the list.}");

    /* 选择申请事件空间的大小 */
    netbuf_len = sizeof(ip_filter_cmd);

    /* 申请空间 缓存过滤规则 */
    netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, netbuf_len, OAL_NETBUF_PRIORITY_MID);
    if (netbuf == NULL) {
        oam_error_log1(mac_vap->uc_vap_id, 0, "{wal_clear_ip_filter_config::netbuf[%d] alloc fail}", netbuf_len);
        return -OAL_EINVAL;
    }
    memset_s(((uint8_t *)oal_netbuf_data(netbuf)), netbuf_len, 0, netbuf_len);
    cmd_info = (mac_ip_filter_cmd_stru *)oal_netbuf_data(netbuf);

    /* 记录过滤规则 */
    ret = memcpy_s((uint8_t *)cmd_info, netbuf_len, (uint8_t *)(&ip_filter_cmd), netbuf_len);
    oal_netbuf_put(netbuf, netbuf_len);

    /* 抛事件到wal层处理 */
    len = sizeof(oal_netbuf_stru *);

    /* 填写 msg 消息头 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_IP_FILTER, len);

    /* 将申请的netbuf首地址填写到msg消息体内 */
    ret += memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value), (uint8_t *)&netbuf, len);
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_clear_ip_filter_config::memcpy fail!");
        oal_netbuf_free(netbuf);
        return -OAL_EINVAL;
    }

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + len,
                             (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_clear_ip_filter_config::send cfg event fail:%d}", ret);
        oal_netbuf_free(netbuf);
        return ret;
    }

    return OAL_SUCC;
}

/*
 * 功能描述  : 清除ip过滤的黑名单
 * 1.日    期  : 2017年4月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_clear_ip_filter()
{
    int32_t ret;
    mac_vap_stru *mac_vap = NULL;
    oal_net_device_stru *net_dev;

#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_clear_ip_filter:: dfr_process_status[%d]!}",
                         g_st_dfr_info.bit_device_reset_process_flag);
        return -OAL_EFAIL;
    }
#endif

    net_dev = wal_config_get_netdev("wlan0", OAL_STRLEN("wlan0"));
    if (net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_clear_ip_filter::wlan0 not exist!}");
        return -OAL_EINVAL;
    }

    /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
    oal_dev_put(net_dev);

    /* vap未创建时，不处理下发的命令 */
    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_clear_ip_filter::vap not created yet, ignore the cmd!.}");
        return -OAL_EINVAL;
    }

    if (mac_vap->st_cap_flag.bit_ip_filter != OAL_TRUE) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_clear_ip_filter::Func disable,ignore the cmd}");
        return -OAL_EINVAL;
    }

    /* 清除ip过滤的黑名单并抛事件到wal层处理 */
    ret = wal_clear_ip_filter_config(net_dev, mac_vap);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "wal_clear_ip_filter::send cfg event fail!");
        return ret;
    }

    return OAL_SUCC;
}
int32_t wal_register_ip_filter(wal_hw_wlan_filter_ops *pst_ip_filter_ops)
{
#ifdef CONFIG_DOZE_FILTER
    if (pst_ip_filter_ops == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_register_ip_filter::pg_st_ip_filter_ops is null !}");
        return -OAL_EINVAL;
    }
    hw_register_wlan_filter(pst_ip_filter_ops);
#else
    oam_warning_log0(0, OAM_SF_ANY, "{wal_register_ip_filter:: Not support CONFIG_DOZE_FILTER!}");
#endif
    return OAL_SUCC;
}

int32_t wal_unregister_ip_filter()
{
#ifdef CONFIG_DOZE_FILTER
    hw_unregister_wlan_filter();
#else
    oam_warning_log0(0, OAM_SF_ANY, "{wal_unregister_ip_filter:: Not support CONFIG_DOZE_FILTER!}");
#endif
    return OAL_SUCC;
}
