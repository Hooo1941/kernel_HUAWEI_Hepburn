

/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "wlan_types.h"

#include "oal_net.h"
#include "oal_cfg80211.h"
#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "plat_pm_wlan.h"

#include "dmac_ext_if.h"

#include "mac_device.h"
#include "mac_vap.h"
#include "mac_ie.h"
#include "mac_resource.h"

#include "hmac_device.h"
#include "hmac_resource.h"
#include "hmac_ext_if.h"
#include "hmac_vap.h"
#include "hmac_p2p.h"
#include "hmac_scan.h"

#include "wal_linux_cfg80211.h"
#include "wal_linux_scan.h"
#include "wal_linux_event.h"
#include "wal_main.h"
#include "wal_ext_if.h"
#include "wal_config.h"
#include "wal_regdb.h"
#include "wal_linux_ioctl.h"
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#include "wal_linux_cfgvendor.h"
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "hisi_customize_wifi.h"
#endif

#include "oal_main.h"
#include "hmac_statistic_data_flow.h"
#ifdef _PRE_WLAN_CHBA_MGMT
#include "hmac_chba_function.h"
#endif
#include "oal_kernel_file.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_CFG80211_C

/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/
#define HI1151_A_RATES      (g_hi1151_rates + 4)
#define HI1151_A_RATES_SIZE 8
#define HI1151_G_RATES      (g_hi1151_rates + 0)
#define HI1151_G_RATES_SIZE 12

/* �豸֧�ֵ����� */
OAL_STATIC oal_ieee80211_rate g_hi1151_rates[] = {
    ratetab_ent(10, 0x1, 0),
    ratetab_ent(20, 0x2, 0),
    ratetab_ent(55, 0x4, 0),
    ratetab_ent(110, 0x8, 0),
    ratetab_ent(60, 0x10, 0),
    ratetab_ent(90, 0x20, 0),
    ratetab_ent(120, 0x40, 0),
    ratetab_ent(180, 0x80, 0),
    ratetab_ent(240, 0x100, 0),
    ratetab_ent(360, 0x200, 0),
    ratetab_ent(480, 0x400, 0),
    ratetab_ent(540, 0x800, 0),
};

/* 2.4G Ƶ�� */
OAL_STATIC oal_ieee80211_channel g_hi1151_2ghz_channels[] = {
    chan2g(1, 2412, 0),
    chan2g(2, 2417, 0),
    chan2g(3, 2422, 0),
    chan2g(4, 2427, 0),
    chan2g(5, 2432, 0),
    chan2g(6, 2437, 0),
    chan2g(7, 2442, 0),
    chan2g(8, 2447, 0),
    chan2g(9, 2452, 0),
    chan2g(10, 2457, 0),
    chan2g(11, 2462, 0),
    chan2g(12, 2467, 0),
    chan2g(13, 2472, 0),
    chan2g(14, 2484, 0),
};

/* 5G Ƶ�� */
OAL_STATIC oal_ieee80211_channel g_hi1151_5ghz_channels[] = {
    chan5g(36, 0),
    chan5g(40, 0),
    chan5g(44, 0),
    chan5g(48, 0),
    chan5g(52, 0),
    chan5g(56, 0),
    chan5g(60, 0),
    chan5g(64, 0),
    chan5g(100, 0),
    chan5g(104, 0),
    chan5g(108, 0),
    chan5g(112, 0),
    chan5g(116, 0),
    chan5g(120, 0),
    chan5g(124, 0),
    chan5g(128, 0),
    chan5g(132, 0),
    chan5g(136, 0),
    chan5g(140, 0),
    chan5g(144, 0),
    chan5g(149, 0),
    chan5g(153, 0),
    chan5g(157, 0),
    chan5g(161, 0),
    chan5g(165, 0),
    /* 4.9G */
    chan4_9g(184, 0),
    chan4_9g(188, 0),
    chan4_9g(192, 0),
    chan4_9g(196, 0),
};

/* �豸֧�ֵļ����׼� */
OAL_STATIC const oal_uint32 g_ast_wlan_supported_cipher_suites[] = {
    WLAN_CIPHER_SUITE_WEP40,
    WLAN_CIPHER_SUITE_WEP104,
    WLAN_CIPHER_SUITE_TKIP,
    WLAN_CIPHER_SUITE_CCMP,
    WLAN_CIPHER_SUITE_GCMP,
    WLAN_CIPHER_SUITE_GCMP_256,
    WLAN_CIPHER_SUITE_CCMP_256,
    WLAN_CIPHER_SUITE_AES_CMAC,
    WLAN_CIPHER_SUITE_BIP_CMAC_256,
    WLAN_CIPHER_SUITE_SMS4,
    WLAN_CIPHER_SUITE_BIP_GMAC_256,
};
oal_workqueue_stru *g_del_virtual_inf_workqueue = OAL_PTR_NULL;

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

/* 2.4G Ƶ����Ϣ */
oal_ieee80211_supported_band g_hi1151_band_2ghz = {
    .channels = g_hi1151_2ghz_channels,
    .n_channels = sizeof(g_hi1151_2ghz_channels) / sizeof(oal_ieee80211_channel),
    .bitrates = HI1151_G_RATES,
    .n_bitrates = HI1151_G_RATES_SIZE,
    .ht_cap = {
        .ht_supported = OAL_TRUE,
        .cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 | IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40,
    },
};

/* 5G Ƶ����Ϣ */
OAL_STATIC oal_ieee80211_supported_band g_hi1151_band_5ghz = {
    .channels = g_hi1151_5ghz_channels,
    .n_channels = sizeof(g_hi1151_5ghz_channels) / sizeof(oal_ieee80211_channel),
    .bitrates = HI1151_A_RATES,
    .n_bitrates = HI1151_A_RATES_SIZE,
    .ht_cap = {
        .ht_supported = OAL_TRUE,
        .cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 | IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40,
    },
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
    .vht_cap = {
        .vht_supported = OAL_TRUE,
        .cap = IEEE80211_VHT_CAP_SHORT_GI_80 | IEEE80211_VHT_CAP_HTC_VHT,
    },
#endif
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
#ifdef _PRE_WLAN_FEATURE_P2P
OAL_STATIC oal_ieee80211_iface_limit g_sta_p2p_limits[] = {
    {
        .max = 2, /* �ӿ����͵��������Ϊ2 */
        .types = BIT(NL80211_IFTYPE_STATION),
    },
    {
        .max = 2, /* �ӿ����͵��������Ϊ2 */
        .types = BIT(NL80211_IFTYPE_P2P_GO) | BIT(NL80211_IFTYPE_P2P_CLIENT),
    },
    {
        .max = 1,
        .types = BIT(NL80211_IFTYPE_P2P_DEVICE),
    },
};

OAL_STATIC oal_ieee80211_iface_combination
g_sta_p2p_iface_combinations[] = {
    {
        .num_different_channels = 2, /* ������ʹ�ò�ͬ�ŵ���2 */
        .max_interfaces = 3, /* group�����������ӿ�����Ϊ3 */
        .limits = g_sta_p2p_limits,
        .n_limits = oal_array_size(g_sta_p2p_limits),
    },
};

#else /* ��p2p���� */
/* E5ƽ̨�����Ҫ֧����vap */
OAL_STATIC oal_ieee80211_iface_limit g_ap_dbac_limits[] = {
    {
        .max = 2, /* �ӿ����͵��������Ϊ2 */
        .types = BIT(NL80211_IFTYPE_AP),
    },
    {
        .max = 1,
        .types = BIT(NL80211_IFTYPE_STATION),
    },
};

OAL_STATIC oal_ieee80211_iface_combination
g_ap_dbac_iface_combinations[] = {
    {
        .num_different_channels = 2, /* ������ʹ�ò�ͬ�ŵ���2 */
        .max_interfaces = 2, /* group�����������ӿ�����Ϊ2 */
        .limits = g_ap_dbac_limits,
        .n_limits = oal_array_size(g_ap_dbac_limits),
    },
};
#endif

/* There isn't a lot of sense in it, but you can transmit anything you like */
static const struct ieee80211_txrx_stypes g_wal_cfg80211_default_mgmt_stypes[NUM_NL80211_IFTYPES] = {
    [NL80211_IFTYPE_ADHOC] = {
        .tx = 0xffff,
        .rx = BIT(IEEE80211_STYPE_ACTION >> 4) /* �����ADHOC��IEEE80211_STYPE_ACTION����4λ������֡���ͣ�ֻʹ�ø�4λ�� */
    },
    [NL80211_IFTYPE_STATION] = {
        .tx = 0xffff, .rx = BIT(IEEE80211_STYPE_ACTION >> 4) /* �����STATION��IEEE80211_STYPE_ACTION����4λ */
        | BIT(IEEE80211_STYPE_PROBE_REQ >> 4) /* �����STATION��IEEE80211_STYPE_PROBE_REQ����4λ������֡���ͣ� */
#ifdef _PRE_WLAN_FEATURE_SAE
        | BIT(IEEE80211_STYPE_AUTH >> 4) /* �����STATION��IEEE80211_STYPE_AUTH����4λ������֡���ͣ� */
#endif
    },
    [NL80211_IFTYPE_AP] = {
        .tx = 0xffff,
        .rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) | /* �����AP��IEEE80211_STYPE_ASSOC_REQ����4λ������֡���ͣ� */
              BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) | /* �����AP��IEEE80211_STYPE_REASSOC_REQ����4λ������֡���ͣ� */
              BIT(IEEE80211_STYPE_PROBE_REQ >> 4) | /* �����AP��IEEE80211_STYPE_PROBE_REQ����4λ������֡���ͣ� */
              BIT(IEEE80211_STYPE_DISASSOC >> 4) | /* �����AP��IEEE80211_STYPE_DISASSOC����4λ������֡���ͣ� */
              BIT(IEEE80211_STYPE_AUTH >> 4) | /* �����AP��IEEE80211_STYPE_AUTH����4λ������֡���ͣ� */
              BIT(IEEE80211_STYPE_DEAUTH >> 4) | /* �����AP��IEEE80211_STYPE_DEAUTH����4λ������֡���ͣ� */
              BIT(IEEE80211_STYPE_ACTION >> 4) }, /* �����AP��IEEE80211_STYPE_ACTION����4λ������֡���ͣ� */
    [NL80211_IFTYPE_AP_VLAN] = { /* copy AP */
        .tx = 0xffff,
        .rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) | /* �����AP VLAN��IEEE80211_STYPE_ASSOC_REQ����4λ������֡���ͣ� */
              BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) | /* �����AP VLAN��IEEE80211_STYPE_REASSOC_REQ����4λ������֡���ͣ� */
              BIT(IEEE80211_STYPE_PROBE_REQ >> 4) | /* �����AP VLAN��IEEE80211_STYPE_PROBE_REQ����4λ������֡���ͣ� */
              BIT(IEEE80211_STYPE_DISASSOC >> 4) | /* �����AP VLAN��IEEE80211_STYPE_DISASSOC����4λ������֡���ͣ� */
              BIT(IEEE80211_STYPE_AUTH >> 4) | /* �����AP VLAN��IEEE80211_STYPE_AUTH����4λ������֡���ͣ� */
              BIT(IEEE80211_STYPE_DEAUTH >> 4) | /* �����AP VLAN��IEEE80211_STYPE_DEAUTH����4λ������֡���ͣ� */
              BIT(IEEE80211_STYPE_ACTION >> 4) /* �����AP VLAN��IEEE80211_STYPE_ACTION����4λ������֡���ͣ� */
    },
    [NL80211_IFTYPE_P2P_CLIENT] = {
        .tx = 0xffff,
        /* �����P2P CLIENT��IEEE80211_STYPE_ACTION����4λ��IEEE80211_STYPE_PROBE_REQ����4λ������֡���ͣ� */
        .rx = BIT(IEEE80211_STYPE_ACTION >> 4) | BIT(IEEE80211_STYPE_PROBE_REQ >> 4) },
    [NL80211_IFTYPE_P2P_GO] = {
        .tx = 0xffff,
        .rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) | /* �����P2P GO��IEEE80211_STYPE_ASSOC_REQ����4λ������֡���ͣ� */
              BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) | /* �����P2P GO��IEEE80211_STYPE_REASSOC_REQ����4λ������֡���ͣ� */
              BIT(IEEE80211_STYPE_PROBE_REQ >> 4) | /* �����P2P GO��IEEE80211_STYPE_PROBE_REQ����4λ������֡���ͣ� */
              BIT(IEEE80211_STYPE_DISASSOC >> 4) | /* �����P2P GO��IEEE80211_STYPE_DISASSOC����4λ������֡���ͣ� */
              BIT(IEEE80211_STYPE_AUTH >> 4) | /* �����P2P GO��IEEE80211_STYPE_AUTH����4λ������֡���ͣ� */
              BIT(IEEE80211_STYPE_DEAUTH >> 4) | /* �����P2P GO��IEEE80211_STYPE_DEAUTH����4λ������֡���ͣ� */
              BIT(IEEE80211_STYPE_ACTION >> 4) }, /* �����P2P GO��IEEE80211_STYPE_ACTION����4λ������֡���ͣ� */
#if defined(_PRE_WLAN_FEATURE_P2P)
    [NL80211_IFTYPE_P2P_DEVICE] = {
        .tx = 0xffff,
        /* �����P2P DEVICE��IEEE80211_STYPE_ACTION����4λ��IEEE80211_STYPE_PROBE_REQ����4λ������֡���ͣ� */
        .rx = BIT(IEEE80211_STYPE_ACTION >> 4) | BIT(IEEE80211_STYPE_PROBE_REQ >> 4) },
#endif
};

#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44) */

#elif (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)

/* 2.4G Ƶ����Ϣ */
OAL_STATIC struct ieee80211_supported_band g_hi1151_band_2ghz = {
    g_hi1151_2ghz_channels,
    HI1151_G_RATES,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
    NL80211_BAND_2GHZ,
#else
    IEEE80211_BAND_2GHZ,
#endif
    sizeof(g_hi1151_2ghz_channels) / sizeof(oal_ieee80211_channel),
    HI1151_G_RATES_SIZE,
    {
        IEEE80211_HT_CAP_SUP_WIDTH_20_40 | IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40,
        OAL_TRUE,
    },
};

/* 5G Ƶ����Ϣ */
OAL_STATIC oal_ieee80211_supported_band g_hi1151_band_5ghz = {
    g_hi1151_5ghz_channels,
    HI1151_A_RATES,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
    NL80211_BAND_5GHZ,
#else
    IEEE80211_BAND_5GHZ,
#endif
    sizeof(g_hi1151_5ghz_channels) / sizeof(oal_ieee80211_channel),
    HI1151_A_RATES_SIZE,
    {
        IEEE80211_HT_CAP_SUP_WIDTH_20_40 | IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40,
        OAL_TRUE,
    },
};
#endif

oal_uint8 g_uc_cookie_array_bitmap = 0; /* ÿ��bit ��ʾcookie array ���Ƿ�ʹ�ã�1 - ��ʹ�ã�0 - δʹ�� */
cookie_arry_stru g_cookie_array[WAL_COOKIE_ARRAY_SIZE];

/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_UAPSD

OAL_STATIC oal_uint32 wal_find_wmm_uapsd(oal_uint8 *puc_wmm_ie)
{
    /* �ж� WMM UAPSD �Ƿ�ʹ�� */
    if (puc_wmm_ie[1] < MAC_WMM_QOS_INFO_POS) {
        return OAL_FALSE;
    }

    if (puc_wmm_ie[MAC_WMM_QOS_INFO_POS] & BIT7) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}
#endif


oal_uint32 wal_cfg80211_open_wmm(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    if (oal_unlikely((pst_mac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL))) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_open_wmm::pst_mac_vap/puc_param is null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* �������vap������ */
    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_CONFIG) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{wal_cfg80211_open_wmm::this is config vap! can't get info.}");
        return OAL_FAIL;
    }

    return hmac_config_sync_cmd_common(pst_mac_vap, WLAN_CFGID_WMM_SWITCH, us_len, puc_param);
}


oal_uint32 wal_parse_rsn_ie(const oal_uint8 *puc_ie, mac_beacon_param_stru *pst_beacon_param)
{
    oal_uint8 uc_ie_idx = 0;
    oal_uint8 uc_ie_len;
    oal_uint16 us_suite_idx = 0;
    oal_uint16 us_ver;
    oal_uint16 us_suites_num;
    oal_uint8 *puc_grp_policy = OAL_PTR_NULL;
    oal_uint8 *puc_grp_mgmt_policy = OAL_PTR_NULL;
    oal_uint8 *puc_pcip_policy = OAL_PTR_NULL;
    oal_uint8 *puc_auth_policy = OAL_PTR_NULL;
    OAL_CONST oal_uint8 *puc_oui = g_auc_rsn_oui;

    /*************************************************************************/
    /* RSN Element Format */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Version | Group Cipher Suite | Pairwise Cipher */
    /* --------------------------------------------------------------------- */
    /* |     1     |    1   |    2    |         4          |       2 */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* Suite Count| Pairwise Cipher Suite List | AKM Suite Count | AKM Suite List */
    /* --------------------------------------------------------------------- */
    /* |       2       |         4*m                |     2           |   4*n */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* |RSN Capabilities|PMKID Count|PMKID List|Group Management Cipher Suite */
    /* --------------------------------------------------------------------- */
    /* |        2       |    2      |   16 *s  |               4           | */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    puc_grp_policy = &(pst_beacon_param->uc_group_crypto);
    puc_grp_mgmt_policy = &(pst_beacon_param->uc_group_mgmt_cipher);
    puc_pcip_policy = pst_beacon_param->auc_pairwise_crypto_wpa2;
    puc_auth_policy = pst_beacon_param->auc_auth_type;

    /* ������С��MAC_MIN_RSN_LEN��ȡ�����ֵ�����쳣 */
    if (puc_ie[1] < MAC_MIN_RSN_LEN) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_parse_rsn_ie::invalid RSN IE len[%d]!}\r\n", puc_ie[1]);
        return OAL_FAIL;
    }

    /* ��ȡie���� */
    uc_ie_len = puc_ie[1];

    /* ����RSN IE �� IE ����2�ֽ� */
    uc_ie_idx += 2;

    /* ��ȡRSN �汾�� */
    us_ver = oal_make_word16(puc_ie[uc_ie_idx], puc_ie[uc_ie_idx + 1]);
    if (us_ver != MAC_RSN_IE_VERSION) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_parse_rsn_ie::RSN version illegal!}\r\n");
        return OAL_FAIL;
    }

    /* ���ü���ģʽ */
    pst_beacon_param->uc_crypto_mode |= WLAN_WPA2_BIT;

    /* ���� RSN �汾�ų���2�ֽ� */
    uc_ie_idx += 2;

    /* ��ȡ�鲥��Կ�׼� */
    if (oal_memcmp(puc_oui, puc_ie + uc_ie_idx, MAC_OUI_LEN) != 0) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_parse_rsn_ie::RSN group OUI illegal!}\r\n");
        return OAL_FAIL;
    }
    *puc_grp_policy = puc_ie[uc_ie_idx + MAC_OUI_LEN];

    /* ���� �鲥��Կ�׼� ����4�ֽ� */
    uc_ie_idx += 4;

    /* ��ȡ�ɶ���Կ�׼� */
    us_suites_num = oal_make_word16(puc_ie[uc_ie_idx], puc_ie[uc_ie_idx + 1]);
    us_suites_num = oal_min(us_suites_num, MAC_PAIRWISE_CIPHER_SUITES_NUM);
    uc_ie_idx += 2; /* ���Գɶ���Կ�׼�����2�ֽ� */

    /* �������׼���ʼ��Ϊ0x00 */
    memset_s(puc_pcip_policy, MAC_PAIRWISE_CIPHER_SUITES_NUM, 0x00, MAC_PAIRWISE_CIPHER_SUITES_NUM);

    /* ��ȡ������Կ���� */
    for (us_suite_idx = 0; us_suite_idx < us_suites_num; us_suite_idx++) {
        /* 2Ϊie��ID��len���ȣ�4ΪPairwise Cipher Suite List��С���� */
        if (MAC_IE_REAMIN_LEN_IS_ENOUGH(2, uc_ie_idx, uc_ie_len, 4) == OAL_FALSE) {
            return OAL_FAIL;
        }

        if (oal_memcmp(puc_oui, puc_ie + uc_ie_idx, MAC_OUI_LEN) != 0) {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_parse_rsn_ie::RSN paerwise OUI illegal!}\r\n");
            return OAL_FAIL;
        }
        puc_pcip_policy[us_suite_idx] = puc_ie[uc_ie_idx + MAC_OUI_LEN];

        uc_ie_idx += 4; /* Pairwise Cipher Suite List 1��Ϊ4�ֽ� */
    }

    /* ��ȡ��֤�׼����� */
    us_suites_num = oal_make_word16(puc_ie[uc_ie_idx], puc_ie[uc_ie_idx + 1]);
    us_suites_num = oal_min(us_suites_num, WLAN_AUTHENTICATION_SUITES);
    uc_ie_idx += 2; /* AKM Suite Count 2�ֽ� */

    /* ����֤�׼���ʼ��Ϊ0x00 */
    memset_s(puc_auth_policy, WLAN_AUTHENTICATION_SUITES, 0x00, WLAN_AUTHENTICATION_SUITES);

    /* ��ȡ��֤���� */
    for (us_suite_idx = 0; us_suite_idx < us_suites_num; us_suite_idx++) {
        /* 2Ϊie��ID��len���ȣ�4ΪAKM Suite List��С���� */
        if (MAC_IE_REAMIN_LEN_IS_ENOUGH(2, uc_ie_idx, uc_ie_len, 4) == OAL_FALSE) {
            return OAL_FAIL;
        }

        if (oal_memcmp(puc_oui, puc_ie + uc_ie_idx, MAC_OUI_LEN) != 0) {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_parse_rsn_ie::RSN auth OUI illegal!}\r\n");
            return OAL_FAIL;
        }
        puc_auth_policy[us_suite_idx] = puc_ie[uc_ie_idx + MAC_OUI_LEN];
        uc_ie_idx += 4; /* AKM Suite List 1��Ϊ4�ֽ� */
    }
    /* ��һ��2Ϊie��ID��len���ȣ�����2ΪRSN Capabilities���� */
    if (MAC_IE_REAMIN_LEN_IS_ENOUGH(2, uc_ie_idx, uc_ie_len, 2) == OAL_FALSE) {
        /* ������RSN capability�ֶΣ����سɹ� */
        return OAL_SUCC;
    }
    /* ��ȡRSN ������Ϣ */
    pst_beacon_param->us_rsn_capability = oal_make_word16(puc_ie[uc_ie_idx], puc_ie[uc_ie_idx + 1]);
    uc_ie_idx += 2; /* RSN Capabilities 2�ֽ� */

    /* ��һ��2Ϊie��ID��len���ȣ���������PMKID Count (Len = 2), ֱ�ӷ��� */
    if (MAC_IE_REAMIN_LEN_IS_ENOUGH(2, uc_ie_idx, uc_ie_len, 2) == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_parse_rsn_ie:: didn't parse grp mgmt cipher}");
        return OAL_SUCC;
    }

    /* ��ȡPMKID Count */
    us_suites_num = oal_make_word16(puc_ie[uc_ie_idx], puc_ie[uc_ie_idx + 1]);
    uc_ie_idx += 2; /* PMKID Count����2�ֽ� */

    /* PMKID List�ֶ� (16 * PMKID Count) */
    for (us_suite_idx = 0; us_suite_idx < us_suites_num; us_suite_idx++) {
        /* 2Ϊie��ID��len���ȣ�IEʣ�೤�Ȳ���16 */
        if (MAC_IE_REAMIN_LEN_IS_ENOUGH(2, uc_ie_idx, uc_ie_len, 16) == OAL_FALSE) {
            /* PMKID List������PMKID Countָ�����Ȳ�һ��, �ο�03, Ŀǰ����עPMK, ���سɹ� */
            return OAL_SUCC;
        }

        uc_ie_idx += 16; /* һ��PMKID����Ϊ16 */
    }

    /* 2Ϊie��ID��len���ȣ���������Group Management Cipher Suite (Len = 4), ֱ�ӷ��� */
    if (MAC_IE_REAMIN_LEN_IS_ENOUGH(2, uc_ie_idx, uc_ie_len, 4) == OAL_FALSE) {
        return OAL_SUCC;
    }

    /* ��ȡGroup Management Cipher Suite��Ϣ */
    *puc_grp_mgmt_policy = puc_ie[uc_ie_idx + MAC_OUI_LEN];

    return OAL_SUCC;
}


oal_uint32 wal_parse_wpa_ie(oal_uint8 *puc_ie, mac_beacon_param_stru *pst_beacon_param)
{
    oal_uint8 uc_index;
    oal_uint8 uc_ie_len;
    oal_uint16 us_temp = 0;
    oal_uint16 us_ver;
    oal_uint16 us_pcip_num;
    oal_uint16 us_auth_num;
    oal_uint8 *puc_pcip_policy = OAL_PTR_NULL;
    oal_uint8 *puc_grp_policy = OAL_PTR_NULL;
    oal_uint8 *puc_auth_policy = OAL_PTR_NULL;
    OAL_CONST oal_uint8 *puc_oui = g_auc_wpa_oui;

    /*************************************************************************/
    /* WPA Element Format */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length |    WPA OUI    |  Version |  Group Cipher Suite */
    /* --------------------------------------------------------------------- */
    /* |     1     |   1    |        4      |     2    |         4 */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* Pairwise Cipher |  Pairwise Cipher   |                 | */
    /* Suite Count     |    Suite List      | AKM Suite Count |AKM Suite List */
    /* --------------------------------------------------------------------- */
    /* 2        |          4*m       |         2       |     4*n */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    puc_grp_policy = &(pst_beacon_param->uc_group_crypto);
    puc_pcip_policy = pst_beacon_param->auc_pairwise_crypto_wpa;
    puc_auth_policy = pst_beacon_param->auc_auth_type;

    uc_ie_len = puc_ie[1];
    if (uc_ie_len < MAC_MIN_WPA_LEN) {
        return OAL_FAIL;
    }

    /* ���� WPA IE��IE ����(2 �ֽ�) ��WPA OUI(4 �ֽ�) */
    uc_index = 2 + 4;

    us_ver = oal_make_word16(puc_ie[uc_index], puc_ie[uc_index + 1]);
    /* �Ա�WPA �汾��Ϣ */
    if (us_ver != MAC_WPA_IE_VERSION) {
        OAM_ERROR_LOG0(0, OAM_SF_WPA, "{wal_parse_wpa_ie::WPA version illegal!}\r\n");
        return OAL_FAIL;
    }

    /* ���� �汾�� ����2�ֽ� */
    uc_index += 2;

    /* ��ȡ�鲥��Կ�׼� */
    if (oal_memcmp(puc_oui, puc_ie + uc_index, MAC_OUI_LEN) != 0) {
        OAM_ERROR_LOG0(0, OAM_SF_WPA, "{wal_parse_wpa_ie::WPA group OUI illegal!}\r\n");
        return OAL_FAIL;
    }
    *puc_grp_policy = puc_ie[uc_index + MAC_OUI_LEN];

    /* �����鲥��Կ�׼�����4�ֽ� */
    uc_index += 4;

    /* ��ȡ�ɶ���Կ�׼� */
    us_pcip_num = oal_make_word16(puc_ie[uc_index], puc_ie[uc_index + 1]);
    if (us_pcip_num > MAC_PAIRWISE_CIPHER_SUITES_NUM) {
        OAM_ERROR_LOG1(0, OAM_SF_WPA, "{wal_parse_wpa_ie::pairwise chiper num illegal %d!}\r\n", us_pcip_num);
        return OAL_FAIL;
    }

    /* �������׼���ʼ��Ϊ0x00 */
    memset_s(puc_pcip_policy, MAC_PAIRWISE_CIPHER_SUITES_NUM, 0x00, MAC_PAIRWISE_CIPHER_SUITES_NUM);

    uc_index += 2; /* Suite Count����Ϊ2�ֽ� */
    for (us_temp = 0; us_temp < us_pcip_num; us_temp++) {
        /* 2Ϊie��ID��len���ȣ�Suite List 1��Ϊ4�ֽ� */
        if (MAC_IE_REAMIN_LEN_IS_ENOUGH(2, uc_index, uc_ie_len, 4) == OAL_FALSE) {
            return OAL_FAIL;
        }

        if (oal_memcmp(puc_oui, puc_ie + uc_index, MAC_OUI_LEN) != 0) {
            OAM_ERROR_LOG0(0, OAM_SF_WPA, "{wal_parse_wpa_ie::WPA pairwise OUI illegal!}\r\n");
            return OAL_FAIL;
        }
        puc_pcip_policy[us_temp] = puc_ie[uc_index + MAC_OUI_LEN];
        uc_index += 4; /* Suite List 1��Ϊ4�ֽ� */
    }

    /* ��ȡ��֤�׼����� */
    us_auth_num = oal_make_word16(puc_ie[uc_index], puc_ie[uc_index + 1]);
    us_auth_num = oal_min(us_auth_num, WLAN_AUTHENTICATION_SUITES);
    uc_index += 2; /* AKM Suite Count����Ϊ2�ֽ� */

    /* ����֤�׼���ʼ��Ϊ0x00 */
    memset_s(puc_auth_policy, us_auth_num, 0x00, us_auth_num);

    /* ��ȡ��֤���� */
    for (us_temp = 0; us_temp < us_auth_num; us_temp++) {
        /* 2Ϊie��ID��len���ȣ�AKM Suite List 1��Ϊ4�ֽ� */
        if (MAC_IE_REAMIN_LEN_IS_ENOUGH(2, uc_index, uc_ie_len, 4) == OAL_FALSE) {
            return OAL_FAIL;
        }

        if (oal_memcmp(puc_oui, puc_ie + uc_index, MAC_OUI_LEN) != 0) {
            OAM_ERROR_LOG0(0, OAM_SF_WPA, "{wal_parse_wpa_ie::WPA auth OUI illegal!}\r\n");
            return OAL_FAIL;
        }
        puc_auth_policy[us_temp] = puc_ie[uc_index + MAC_OUI_LEN];
        uc_index += 4; /* AKM Suite List 1��Ϊ4�ֽ� */
    }

    /* ���ü���ģʽ */
    pst_beacon_param->uc_crypto_mode |= WLAN_WPA_BIT;

    return OAL_SUCC;
}


oal_uint32 wal_parse_wpa_wpa2_ie(oal_beacon_parameters *pst_beacon_info,
                                 mac_beacon_param_stru *pst_beacon_param)
{
    const oal_uint8 *puc_rsn_ie = OAL_PTR_NULL;
    oal_uint8 *puc_wpa_ie = OAL_PTR_NULL;
    oal_uint32 ul_ret = OAL_SUCC;
    oal_ieee80211_mgmt *pst_mgmt = OAL_PTR_NULL;
    oal_uint16 us_capability_info;

    /* �ж��Ƿ���� */
    pst_mgmt = (oal_ieee80211_mgmt *)pst_beacon_info->head;

    us_capability_info = pst_mgmt->u.beacon.capab_info;
    pst_beacon_param->en_privacy = OAL_FALSE;

    if (WLAN_WITP_CAPABILITY_PRIVACY & us_capability_info) {
        pst_beacon_param->en_privacy = OAL_TRUE;

        /* ���� RSN ��ϢԪ�� */
        puc_rsn_ie = mac_find_ie(MAC_EID_RSN, pst_beacon_info->tail, pst_beacon_info->tail_len);
        if (puc_rsn_ie != OAL_PTR_NULL) {
            /* ����RSN ��ϢԪ�ؽ�������֤���� */
            ul_ret = wal_parse_rsn_ie(puc_rsn_ie, pst_beacon_param);
            if (ul_ret != OAL_SUCC) {
                oam_warning_log0(0, OAM_SF_WPA, "{wal_parse_wpa_wpa2_ie::Failed to parse RSN ie!}\r\n");
                return OAL_FAIL;
            }
        }

        /* ���� WPA ��ϢԪ�أ�����������֤���� */
        puc_wpa_ie = mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_OUITYPE_WPA, pst_beacon_info->tail,
                                        pst_beacon_info->tail_len);
        if (puc_wpa_ie != OAL_PTR_NULL) {
            ul_ret = wal_parse_wpa_ie(puc_wpa_ie, pst_beacon_param);
            if (ul_ret != OAL_SUCC) {
                oam_warning_log0(0, OAM_SF_WPA, "{wal_parse_wpa_wpa2_ie::Failed to parse WPA ie!}\r\n");
                return OAL_FAIL;
            }
        }
    }

    return OAL_SUCC;
}


oal_uint32 wal_parse_wmm_ie(oal_net_device_stru *pst_dev,
                            mac_vap_stru *pst_mac_vap, oal_beacon_parameters *pst_beacon_info)
{
    oal_uint8 *puc_wmm_ie;
    oal_uint16 us_len = OAL_SIZEOF(oal_uint8);
    oal_uint8 uc_wmm = OAL_TRUE;
    oal_uint32 ul_ret = OAL_SUCC;

#ifdef _PRE_WLAN_FEATURE_UAPSD
    oal_uint8 uc_uapsd = OAL_TRUE;
    wal_msg_write_stru st_write_msg;
#endif

    /* ����wmm_ie */
    puc_wmm_ie = mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WMM,
                                    pst_beacon_info->tail, pst_beacon_info->tail_len);
    if (puc_wmm_ie == OAL_PTR_NULL) {
        /* wmm ieδ�ҵ�����˵��wmm �� */
        uc_wmm = OAL_FALSE;
    }
#ifdef _PRE_WLAN_FEATURE_UAPSD
    /* �ҵ�wmm ie��˳���ж���uapsd�Ƿ�ʹ�� */
    else {
        /* :�ж�WMM��ϢԪ�غ�ƫ��8�ֽ� ��bit7λ�Ƿ�Ϊ1,1��ʾuapsdʹ�� */
        if (wal_find_wmm_uapsd(puc_wmm_ie) == OAL_FALSE) {
            /* ��ӦUAPSD �� */
            uc_uapsd = OAL_FALSE;
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_parse_wmm_ie::uapsd is disabled!!}");
        }

        /* ��д msg ��Ϣͷ */
        wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_UAPSD_EN, OAL_SIZEOF(uc_uapsd));

        /* ��д msg ��Ϣ�� */
#ifdef _PRE_WLAN_FEATURE_P2P
        /*  ���Go��֧��UAPSD������ */
        /* �ù���Ŀǰ��������������ʵ�֣�������Ҫ����ϲ����ã�TBD */
        if (IS_P2P_GO(pst_mac_vap)) {
            uc_uapsd = WLAN_FEATURE_UAPSD_IS_OPEN;
            oam_info_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                          "{wal_parse_wmm_ie_etc:: (1103)It is a Go, set uapsd = WLAN_FEATURE_UAPSD_IS_OPEN)}");
        }
#endif

        if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(uc_uapsd),
            &uc_uapsd, OAL_SIZEOF(uc_uapsd)) != EOK) {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "wal_parse_wmm_ie::memcpy fail!");
            return OAL_FAIL;
        }

        /* ������Ϣ */
        ul_ret = (oal_uint32)wal_send_cfg_event(pst_dev, WAL_MSG_TYPE_WRITE,
                                                WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(uc_uapsd),
                                                (oal_uint8 *)&st_write_msg,
                                                OAL_FALSE,
                                                OAL_PTR_NULL);
        if (oal_unlikely(ul_ret != OAL_SUCC)) {
            ul_ret = OAL_FAIL;
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{wal_parse_wmm_ie::uapsd switch set failed[%d].}", ul_ret);
        }
    }
#endif
    /* wmm ����/�ر� ��� */
    ul_ret = wal_cfg80211_open_wmm(pst_mac_vap, us_len, &uc_wmm);
    if (ul_ret != OAL_SUCC) {
        ul_ret = OAL_FAIL;
        oam_warning_log0(0, OAM_SF_TX, "{wal_parse_wmm_ie::can not open wmm!}\r\n");
    }

    return ul_ret;
}

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)


oal_uint32 wal_cfg80211_add_vap(mac_cfg_add_vap_param_stru *pst_add_vap_param)
{
    /* : P2P change interfaceʱ����Ҫ���³�ʼ����ע��netdev.
     * nl80211 netlink pre diot �л��ȡrntl_lock�������� ע��net_device ���ȡrntl_lock����������������� */
    oal_int32 l_ret;
    oal_net_device_stru *pst_net_dev;
    oal_net_device_stru *pst_cfg_net_dev = OAL_PTR_NULL;
    wal_msg_write_stru st_write_msg = {0};
    wal_msg_stru *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32 ul_err_code;

    oal_wireless_dev_stru *pst_wdev;
    mac_wiphy_priv_stru *pst_wiphy_priv;

    mac_vap_stru *pst_cfg_mac_vap;
    hmac_vap_stru *pst_cfg_hmac_vap = OAL_PTR_NULL;
    mac_device_stru *pst_mac_device;
    wlan_vap_mode_enum_uint8 en_vap_mode;
#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8 en_p2p_mode;
    en_p2p_mode = pst_add_vap_param->en_p2p_mode;
#endif

    en_vap_mode = pst_add_vap_param->en_vap_mode;

    /* ��ȡmac device */
    pst_net_dev = pst_add_vap_param->pst_net_dev;
    pst_wdev = pst_net_dev->ieee80211_ptr;
    pst_wiphy_priv = (mac_wiphy_priv_stru *)oal_wiphy_priv(pst_wdev->wiphy);
    pst_mac_device = pst_wiphy_priv->pst_mac_device;
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_add_vap::pst_mac_device is null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_cfg_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->uc_cfg_vap_id);
    if (oal_unlikely(pst_cfg_mac_vap == OAL_PTR_NULL)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_add_vap::pst_cfg_mac_vap is null mac_vap:%d!}\r\n",
                         pst_mac_device->uc_cfg_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_cfg_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_device->uc_cfg_vap_id);
    if (oal_unlikely(pst_cfg_hmac_vap == OAL_PTR_NULL)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_add_vap::pst_cfg_hmac_vap is null vap_id:%d!}\r\n",
                         pst_mac_device->uc_cfg_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_cfg_net_dev = pst_cfg_hmac_vap->pst_net_device;

    if (en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        pst_wdev->iftype = NL80211_IFTYPE_AP;
    } else if (en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        pst_wdev->iftype = NL80211_IFTYPE_STATION;
    }
#ifdef _PRE_WLAN_FEATURE_P2P
    if (en_p2p_mode == WLAN_P2P_DEV_MODE) {
        pst_wdev->iftype = NL80211_IFTYPE_P2P_DEVICE;
    } else if (en_p2p_mode == WLAN_P2P_CL_MODE) {
        pst_wdev->iftype = NL80211_IFTYPE_P2P_CLIENT;
    } else if (en_p2p_mode == WLAN_P2P_GO_MODE) {
        pst_wdev->iftype = NL80211_IFTYPE_P2P_GO;
    }
#endif                                                    /* _PRE_WLAN_FEATURE_P2P */
    oal_netdevice_flags(pst_net_dev) &= ~OAL_IFF_RUNNING; /* ��net device��flag��Ϊdown */

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_ADD_VAP, OAL_SIZEOF(mac_cfg_add_vap_param_stru));
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_net_dev;
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->en_vap_mode = en_vap_mode;
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->uc_cfg_vap_indx = pst_cfg_mac_vap->uc_vap_id;
#ifdef _PRE_WLAN_FEATURE_P2P
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode = en_p2p_mode;
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->bit_11ac2g_enable =
        (oal_uint8) !!hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_11AC2G_ENABLE);
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->bit_disable_capab_2ght40 =
        g_wlan_customize.uc_disable_capab_2ght40;
#endif
    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_cfg_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_add_vap_param_stru),
                               (oal_uint8 *)&st_write_msg, OAL_TRUE, &pst_rsp_msg);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_cfg_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_cfg80211_add_vap::return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (ul_err_code != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_cfg_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_cfg80211_add_vap::hmac add vap fail, ul_err_code[%u]!}\r\n", ul_err_code);
        return ul_err_code;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    wal_set_random_mac_to_mib(pst_net_dev); /* set random mac to mib ; for hi1102-cb */
#endif
    return OAL_SUCC;
}


oal_uint32 wal_cfg80211_del_vap(mac_cfg_del_vap_param_stru *pst_del_vap_param)
{
    wal_msg_write_stru st_write_msg = {0};
    wal_msg_stru *pst_rsp_msg = OAL_PTR_NULL;
    oal_net_device_stru *pst_net_dev = OAL_PTR_NULL;

    if (oal_unlikely(pst_del_vap_param == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_del_vap::pst_del_vap_param null ptr !}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_net_dev = pst_del_vap_param->pst_net_dev;
    /* �豸��up״̬������ɾ����������down */
    if (oal_unlikely((OAL_IFF_RUNNING & oal_netdevice_flags(pst_net_dev)) != 0)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_del_vap::device is busy, please down it first %d!}\r\n",
                       oal_netdevice_flags(pst_net_dev));
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    /* ����Ҫ�ͷ�net_device �ṹ�µ�wireless_dev ��Ա */
    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    /* ��ʼ��ɾ��vap ���� */
    ((mac_cfg_del_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_net_dev;
#ifdef _PRE_WLAN_FEATURE_P2P
    ((mac_cfg_del_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode = pst_del_vap_param->en_p2p_mode;
#endif
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_DEL_VAP, OAL_SIZEOF(mac_cfg_del_vap_param_stru));

    if (OAL_SUCC != wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_del_vap_param_stru),
                                       (oal_uint8 *)&st_write_msg, OAL_TRUE, &pst_rsp_msg)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_vap::wal_send_cfg_event fail!}");
        return -OAL_EFAIL;
    }

    if (wal_check_and_release_msg_resp(pst_rsp_msg) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_vap::wal_check_and_release_msg_resp fail!}");
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
#else

oal_uint32 wal_cfg80211_add_vap(mac_cfg_add_vap_param_stru *pst_add_vap_param)
{
    return OAL_SUCC;
}

oal_uint32 wal_cfg80211_del_vap(mac_cfg_del_vap_param_stru *pst_del_vap_param)
{
    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_VOWIFI

oal_uint32 wal_cfg80211_vowifi_report(frw_event_mem_stru *pst_event_mem)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
    frw_event_stru *pst_event;
    hmac_vap_stru *pst_hmac_vap;

    if (oal_unlikely(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_vowifi_report::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = (frw_event_stru *)pst_event_mem->puc_data;
    pst_hmac_vap = mac_res_get_hmac_vap(pst_event->st_event_hdr.uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_TX, "{wal_cfg80211_vowifi_report::pst_hmac_vap null.vap_id[%d]}",
                       pst_event->st_event_hdr.uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* �ϱ�vowifi�л����� */
    oal_cfg80211_vowifi_report(pst_hmac_vap->pst_net_device, GFP_KERNEL);
#endif /* (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)) */

    return OAL_SUCC;
}
#endif /* _PRE_WLAN_FEATURE_VOWIFI */


oal_bool_enum wal_check_support_basic_rate_6m(oal_uint8 *puc_supported_rates_ie, oal_uint8 uc_supported_rates_num,
    oal_uint8 *puc_extended_supported_rates_ie, oal_uint8 uc_extended_supported_rates_num)
{
    oal_uint8 uc_loop;
    oal_bool_enum en_support = OAL_FALSE;
    for (uc_loop = 0; uc_loop < uc_supported_rates_num; uc_loop++) {
        if (puc_supported_rates_ie == OAL_PTR_NULL) {
            break;
        }
        if (puc_supported_rates_ie[2 + uc_loop] == 0x8c) { /* �ж�puc_supported_rates_ie[2 + uc_loop]�Ƿ����0x8c */
            en_support = OAL_TRUE;
        }
    }

    for (uc_loop = 0; uc_loop < uc_extended_supported_rates_num; uc_loop++) {
        if (puc_extended_supported_rates_ie == OAL_PTR_NULL) {
            break;
        }
        /* �ж�puc_extended_supported_rates_ie[2 + uc_loop]�Ƿ����0x8c */
        if (puc_extended_supported_rates_ie[2 + uc_loop] == 0x8c) {
            en_support = OAL_TRUE;
        }
    }

    return en_support;
}


oal_uint32 wal_parse_protocol_mode(wlan_channel_band_enum_uint8 en_band,
                                   oal_beacon_parameters *pst_beacon_info,
                                   oal_uint8 *puc_ht_ie,
                                   oal_uint8 *puc_vht_ie,
                                   wlan_protocol_enum_uint8 *pen_protocol)
{
    oal_uint8 *puc_supported_rates_ie = OAL_PTR_NULL;
    oal_uint8 *puc_extended_supported_rates_ie = OAL_PTR_NULL;
    oal_uint8 uc_supported_rates_num = 0;
    oal_uint8 uc_extended_supported_rates_num = 0;
    oal_uint16 us_offset;
    oal_uint8 uc_rate_nums;

    if (puc_vht_ie != OAL_PTR_NULL) {
        /* ����AP Ϊ11ac ģʽ */
        *pen_protocol = WLAN_VHT_MODE;
        return OAL_SUCC;
    }
    if (puc_ht_ie != OAL_PTR_NULL) {
        /* ����AP Ϊ11n ģʽ */
        *pen_protocol = WLAN_HT_MODE;
        return OAL_SUCC;
    }

    /* hostapd ������Ƶ�Σ�������add beacon �����add beacon�����У�����ʹ��Ƶ����Ϣ��������Ϣ����AP ģʽ(a/b/g) */
    if (en_band == WLAN_BAND_5G) {
        *pen_protocol = WLAN_LEGACY_11A_MODE;
        return OAL_SUCC;
    } else if (en_band == WLAN_BAND_2G) {
        us_offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
        /* ����supported_rates_ie��head + 24 + offset(12)�� */
        puc_supported_rates_ie = mac_find_ie(MAC_EID_RATES, pst_beacon_info->head + 24 + us_offset,
                                             pst_beacon_info->head_len - us_offset);
        if (puc_supported_rates_ie != OAL_PTR_NULL) {
            uc_supported_rates_num = puc_supported_rates_ie[1];
        }

        puc_extended_supported_rates_ie = mac_find_ie(MAC_EID_XRATES, pst_beacon_info->tail, pst_beacon_info->tail_len);
        if (puc_extended_supported_rates_ie != OAL_PTR_NULL) {
            uc_extended_supported_rates_num = puc_extended_supported_rates_ie[1];
        }

        uc_rate_nums = uc_supported_rates_num + uc_extended_supported_rates_num;
        /* ��������� */
        if (uc_rate_nums < uc_supported_rates_num) {
            oam_error_log2(0, 0, "{wal_parse_protocol_mode::rate_num[%d], ext_rate_num[%d]}",
                           uc_supported_rates_num, uc_extended_supported_rates_num);
            *pen_protocol = WLAN_PROTOCOL_BUTT;
            return OAL_FAIL;
        }

        if (uc_rate_nums == 4) { /* rate_numsֵΪ4��11b, 2.4G */
            *pen_protocol = WLAN_LEGACY_11B_MODE;
            return OAL_SUCC;
        } else if (uc_rate_nums == 8) { /* rate_numsֵΪ8���ɵ�11g only�ѷ���, 2.4G, OFDM�� */
            *pen_protocol = WLAN_LEGACY_11G_MODE;
            return OAL_SUCC;
        } else if (uc_rate_nums == 12) { /* rate_numsֵΪ12��11bg, 2.4G */
            /* ���ݻ�����������Ϊ 11gmix1 ���� 11gmix2 */
            /* ����������ʼ�֧�� 6M , ���ж�Ϊ 11gmix2 */
            *pen_protocol = WLAN_MIXED_ONE_11G_MODE;
            if (wal_check_support_basic_rate_6m(puc_supported_rates_ie,
                                                uc_supported_rates_num,
                                                puc_extended_supported_rates_ie,
                                                uc_extended_supported_rates_num) == OAL_TRUE) {
                *pen_protocol = WLAN_MIXED_TWO_11G_MODE;
            }
            return OAL_SUCC;
        }
    }

    /* �����������Ϊ���ò����� */
    *pen_protocol = WLAN_PROTOCOL_BUTT;

    return OAL_FAIL;
}


oal_uint32 wal_parse_ht_vht_ie(mac_vap_stru *pst_mac_vap,
                               oal_beacon_parameters *pst_beacon_info,
                               mac_beacon_param_stru *pst_beacon_param)
{
    oal_uint8 *puc_ht_ie;
    oal_uint8 *puc_vht_ie;
    oal_uint32 ul_ret;
    mac_frame_ht_cap_stru *pst_ht_cap = OAL_PTR_NULL;
    mac_vht_cap_info_stru *pst_vht_cap = OAL_PTR_NULL;

    puc_ht_ie = mac_find_ie(MAC_EID_HT_CAP, pst_beacon_info->tail, pst_beacon_info->tail_len);
    puc_vht_ie = mac_find_ie(MAC_EID_VHT_CAP, pst_beacon_info->tail, pst_beacon_info->tail_len);

    /* ����Э��ģʽ */
    ul_ret = wal_parse_protocol_mode(pst_mac_vap->st_channel.en_band, pst_beacon_info, puc_ht_ie, puc_vht_ie,
                                     &pst_beacon_param->en_protocol);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_parse_ht_vht_ie::return err code!}\r\n", ul_ret);

        return ul_ret;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    /* ���ƻ�ʵ��P2P GO 2.4G��Ĭ��֧��11ac Э��ģʽ */
    if (IS_P2P_GO(pst_mac_vap) && (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G)) {
        pst_beacon_param->en_protocol =
            ((pst_mac_vap->st_cap_flag.bit_11ac2g == OAL_TRUE) ? WLAN_VHT_MODE : WLAN_HT_MODE);
    }

#endif /* _PRE_WLAN_FEATURE_P2P */

    /* ����short gi���� */
    if (puc_ht_ie == OAL_PTR_NULL) {
        return OAL_SUCC;
    }

    /* ʹ��ht cap ie���������2���ֽ� */
    if (puc_ht_ie[1] < OAL_SIZEOF(mac_frame_ht_cap_stru)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_parse_ht_vht_ie::invalid ht cap ie len[%d]!}\r\n", puc_ht_ie[1]);
        return OAL_SUCC;
    }

    pst_ht_cap = (mac_frame_ht_cap_stru *)(puc_ht_ie + MAC_IE_HDR_LEN);

    pst_beacon_param->en_shortgi_20 = (oal_uint8)pst_ht_cap->bit_short_gi_20mhz;
    pst_beacon_param->en_shortgi_40 = 0;

    if ((pst_mac_vap->st_channel.en_bandwidth > WLAN_BAND_WIDTH_20M)
        && (pst_mac_vap->st_channel.en_bandwidth != WLAN_BAND_WIDTH_BUTT)) {
        pst_beacon_param->en_shortgi_40 = (oal_uint8)pst_ht_cap->bit_short_gi_40mhz;
    }

    pst_beacon_param->uc_smps_mode = (oal_uint8)pst_ht_cap->bit_sm_power_save;

    if (puc_vht_ie == OAL_PTR_NULL) {
        return OAL_SUCC;
    }

    /* ʹ��vht cap ie���������4���ֽ� */
    if (puc_vht_ie[1] < OAL_SIZEOF(mac_vht_cap_info_stru)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_parse_ht_vht_ie::invalid ht cap ie len[%d]!}\r\n", puc_vht_ie[1]);
        return OAL_SUCC;
    }

    pst_vht_cap = (mac_vht_cap_info_stru *)(puc_vht_ie + MAC_IE_HDR_LEN);

    pst_beacon_param->en_shortgi_80 = 0;

    if ((pst_mac_vap->st_channel.en_bandwidth > WLAN_BAND_WIDTH_40MINUS)
        && (pst_mac_vap->st_channel.en_bandwidth != WLAN_BAND_WIDTH_BUTT)) {
        pst_beacon_param->en_shortgi_80 = pst_vht_cap->bit_short_gi_80mhz;
    }

    return OAL_SUCC;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
oal_int32 wal_cfg80211_sched_scan_stop(oal_wiphy_stru *pst_wiphy,
                                       oal_net_device_stru *pst_netdev,
                                       oal_uint64 ul_reqid);

#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 44))
oal_int32 wal_cfg80211_sched_scan_stop(oal_wiphy_stru *pst_wiphy,
                                       oal_net_device_stru *pst_netdev);
#else
/* do nothing */
#endif


OAL_STATIC oal_int32 wal_p2p_stop_roc(mac_vap_stru *pst_mac_vap, oal_net_device_stru *pst_netdev)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    hmac_vap_stru *pst_hmac_vap;
    hmac_device_stru *pst_hmac_device;
    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
                       "{wal_p2p_stop_roc:: pst_hmac_device[%d] null!}\r\n", pst_mac_vap->uc_device_id);
        return -OAL_EFAIL;
    }

    /* tx mgmt roc ���ȼ���,���Ա��Լ���80211 roc�Լ�80211 scan��� */
    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN) {
        if (pst_hmac_device->st_scan_mgmt.en_is_scanning != OAL_TRUE) {
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
                             "{wal_p2p_stop_roc::not in scan state but vap is listen state!}");
            return OAL_SUCC;
        }

        pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
        if (oal_unlikely(pst_hmac_vap == OAL_PTR_NULL)) {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{wal_p2p_stop_roc:: pst_hmac_vap null!}\r\n");
            return -OAL_EFAIL;
        }
        pst_hmac_vap->en_wait_roc_end = OAL_TRUE;
        oal_init_completion(&(pst_hmac_vap->st_roc_end_ready));
        wal_force_scan_complete(pst_netdev, OAL_TRUE);
        /* ��ʱʱ��200ms */
        if (oal_wait_for_completion_timeout(&(pst_hmac_vap->st_roc_end_ready), (oal_uint32)oal_msecs_to_jiffies(200))
            == 0) {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{wal_p2p_stop_roc::cancel old roc timout!}");
            return -OAL_EFAIL;
        }
    }
#endif
    return OAL_SUCC;
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
OAL_STATIC void wal_cfg80211_set_scan_random_mac(hmac_device_stru *hmac_device,
    uint32_t flags, uint8_t *mac_addr, uint8_t *mac_addr_mask)
{
    uint8_t random_mac[WLAN_MAC_ADDR_LEN];
    uint8_t *random_mac_addr = random_mac;
    uint8_t idx;
    /*
     *  flags��bit3Ϊ1��ʾ�ϲ��·����macɨ�衣
     * mac_addr_mask��bitΪ1λ��mac_addr�е�ֵ��
     * mac_addr_mask��bitΪ0λ�����ֵ��
     */
    if (flags & NL80211_SCAN_FLAG_RANDOM_ADDR) {
        hmac_device->st_scan_mgmt.random_mac_from_kernel = OAL_TRUE;
        oal_random_ether_addr(random_mac_addr);
        for (idx = 0; idx < WLAN_MAC_ADDR_LEN; idx++) {
            random_mac[idx] &= ~(mac_addr_mask[idx]);
            random_mac[idx] |= (mac_addr[idx] & mac_addr_mask[idx]);
        }
        random_mac[MAC_ADDR_0] &= 0xfe;
        memcpy_s(hmac_device->st_scan_mgmt.auc_random_mac, WLAN_MAC_ADDR_LEN, random_mac, WLAN_MAC_ADDR_LEN);
        oam_warning_log3(0, OAM_SF_SCAN,
                         "{wal_cfg80211_set_scan_random_mac::rand_mac_addr[%02X:XX:XX:XX:%02X:%02X].}",
                         hmac_device->st_scan_mgmt.auc_random_mac[MAC_ADDR_0],
                         hmac_device->st_scan_mgmt.auc_random_mac[MAC_ADDR_4],
                         hmac_device->st_scan_mgmt.auc_random_mac[MAC_ADDR_5]);
    } else {
        hmac_device->st_scan_mgmt.random_mac_from_kernel = OAL_FALSE;
    }
}
#endif

/*lint -e801*/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))  // ȷ����ȷ�� Linux �汾��
oal_int32 wal_cfg80211_scan(oal_wiphy_stru *pst_wiphy,
                            oal_cfg80211_scan_request_stru *pst_request)
#else
oal_int32 wal_cfg80211_scan(oal_wiphy_stru *pst_wiphy,
                            oal_net_device_stru *pst_netdev,
                            oal_cfg80211_scan_request_stru *pst_request)
#endif
{
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    hmac_scan_stru *pst_scan_mgmt = OAL_PTR_NULL;
    oal_int32 l_ret = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))  // ȷ����ȷ�� Linux �汾��
    oal_net_device_stru *pst_netdev;
#endif

    if (oal_any_null_ptr2(pst_wiphy, pst_request)) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_scan_param_check::scan failed, null ptr!}");
        goto fail;
    }

    /* �ж�ɨ�贫���ڴ泤�Ȳ��ܴ��ں�������ռ��С�����⿽���ڴ�Խ�� */
    if (pst_request->ie_len > WLAN_WPS_IE_MAX_SIZE) {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_scan:: scan ie is too large to save. [%d]!}", pst_request->ie_len);
        return -OAL_EFAIL;
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))  // ȷ����ȷ�� Linux �汾��
    pst_netdev = pst_request->wdev->netdev;
#endif
    if (pst_netdev == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_scan::scan failed, null ptr, pst_netdev = null!}");
        goto fail;
    }
#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_scan:: dfr_process_status[%d]!}",
                         g_st_dfr_info.bit_device_reset_process_flag);
        return -OAL_EFAIL;
    }
#endif  // #ifdef _PRE_WLAN_FEATURE_DFR
    /* ͨ��net_device �ҵ���Ӧ��mac_device_stru �ṹ */
    pst_mac_vap = oal_net_dev_priv(pst_netdev);
    if (pst_mac_vap == NULL) {
        /* ��STA�л���APģʽʱ��VAP�Ѿ�ɾ������ʱ�Ҳ���vap������ */
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_scan::can't get mac vap from netdevice priv data!}");
        goto fail;
    }

    /* �����׶β��·�����ܹ����ɹ�˲��WLAN����հ׵Ĳ������� */
    if (HMAC_VAP_IN_ASSOCING_STAT(pst_mac_vap)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_scan::vap_state [%d]--stop normal scan \
            while connecting.}", pst_mac_vap->en_vap_state);
        /*lint -e801*/
        goto fail;
        /*lint +e801*/
    }

#ifdef _PRE_WLAN_FEATURE_WAPI
    if (is_p2p_scan_req(pst_request) && (hmac_user_is_wapi_connected(pst_mac_vap->uc_device_id) == OAL_TRUE)) {
        oam_warning_log0(0, OAM_SF_CFG, "{stop p2p scan under wapi!}");
        /*lint -e801*/
        goto fail;
        /*lint +e801*/
    }
#endif /* #ifdef _PRE_WLAN_FEATURE_WAPI */

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "wal_cfg80211_scan:scan failed, null ptr, pst_hmac_device is null device_id:%d",
                       pst_mac_vap->uc_device_id);
        /*lint -e801*/
        goto fail;
        /*lint +e801*/
    }
    pst_scan_mgmt = &(pst_hmac_device->st_scan_mgmt);

    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_scan::start a new normal scan. \
        n_channels[%d], ie_len[%d], n_ssid[%d]}", pst_request->n_channels, pst_request->ie_len, pst_request->n_ssids);

    /* �������ص�ɨ��, ���ϴ�δ��������Ҫ����busy */
    /* �ȴ��ϴε�ɨ��������� */
    /*lint -e730*/ /* info, boolean argument to function */
    l_ret = oal_wait_event_interruptible(pst_scan_mgmt->st_wait_queue, (pst_scan_mgmt->pst_request == OAL_PTR_NULL));
    /*lint +e730*/
    if (l_ret < 0) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{wal_cfg80211_scan::start a new scan failed, wait return error.}");
        /*lint -e801*/
        goto fail;
        /*lint +e801*/
    }

    /* p2p normal scan�Լ����Դ���Լ���һ�ε�roc */
    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_scan::stop roc scan, before normal scan.}");
        l_ret = wal_p2p_stop_roc(pst_mac_vap, pst_netdev);
        if (l_ret < 0) {
            /*lint -e801*/
            goto fail;
            /*lint +e801*/
        }
    }

    /* ���浱ǰ�ں��·���ɨ�����󵽱��� */
    pst_scan_mgmt->pst_request = pst_request;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 44))
    /* �����ǰ����ɨ�������У�����ͣ����ɨ�� */
    if (pst_scan_mgmt->pst_sched_scan_req != OAL_PTR_NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_scan::stop sched scan before normal scan}");
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
        wal_cfg80211_sched_scan_stop(pst_wiphy, pst_netdev, 0);
#else
        wal_cfg80211_sched_scan_stop(pst_wiphy, pst_netdev);
#endif
    }
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    OAM_WARNING_LOG1(0, 0, "wal_cfg80211_scan::flags[%d]", pst_request->flags);
    wal_cfg80211_set_scan_random_mac(pst_hmac_device, pst_request->flags,
        (uint8_t *)(pst_request->mac_addr), (uint8_t *)(pst_request->mac_addr_mask));
#endif
    /* ����ɨ�� */
    if (wal_scan_work_func(pst_scan_mgmt, pst_netdev, pst_request) != OAL_SUCC) {
        pst_scan_mgmt->pst_request = OAL_PTR_NULL;
        return -OAL_EFAIL;
    }

    return OAL_SUCC;

fail:
    return -OAL_EFAIL;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))


oal_uint32 wal_set_wep_key(mac_cfg80211_connect_param_stru *pst_connect_param,
                           oal_cfg80211_conn_stru *pst_sme)
{
    oal_uint8 *puc_wep_key = OAL_PTR_NULL;

    pst_connect_param->uc_wep_key_len = pst_sme->key_len;
    pst_connect_param->uc_wep_key_index = pst_sme->key_idx;
    pst_connect_param->st_crypto.cipher_group = (oal_uint8)pst_sme->crypto.cipher_group;

    puc_wep_key = (oal_uint8 *)oal_memalloc(pst_sme->key_len);
    if (puc_wep_key == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_set_wep_key::alloc wep key len = %d return null ptr!}", pst_sme->key_len);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    if (memcpy_s (puc_wep_key, pst_sme->key_len, (oal_uint8 *)pst_sme->key, (oal_uint32)(pst_sme->key_len)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_set_wep_key::memcpy fail!}");
    }

    pst_connect_param->puc_wep_key = puc_wep_key;

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_set_rsn_key(oal_uint8 *puc_ie, mac_cfg80211_connect_param_stru *pst_connect_param)
{
    oal_uint8 uc_ie_len;
    oal_uint32 ul_offset;
    oal_uint8 uc_loop;
    oal_uint8 uc_akm_suite_num;
    oal_uint8 uc_pairwise_cipher_num;

    uc_ie_len = puc_ie[1];
    if (uc_ie_len < MAC_MIN_RSN_LEN) {
        return OAL_FAIL;
    }

    /* �汾��Ϣ */
    ul_offset = MAC_IE_HDR_LEN;

    /* ��ȡgroup cipher type */
    ul_offset += MAC_RSN_VERSION_LEN + MAC_OUI_LEN;
    pst_connect_param->st_crypto.cipher_group = puc_ie[ul_offset];

    /* ��ȡpairwise cipher cout */
    ul_offset += MAC_OUITYPE_WPA;
    pst_connect_param->st_crypto.n_ciphers_pairwise = puc_ie[ul_offset];
    /* puc_ieƫ��ul_offset + 1�ֽ�������8λ */
    pst_connect_param->st_crypto.n_ciphers_pairwise += (oal_uint8)(puc_ie[ul_offset + 1] << 8);

    /* ��ȡpairwise cipher type */
    ul_offset += MAC_RSN_CIPHER_COUNT_LEN;
    if (pst_connect_param->st_crypto.n_ciphers_pairwise) {
        uc_pairwise_cipher_num = oal_min(pst_connect_param->st_crypto.n_ciphers_pairwise,
                                         OAL_NL80211_MAX_NR_CIPHER_SUITES);
        for (uc_loop = 0; uc_loop < uc_pairwise_cipher_num; uc_loop++) {
            /* �ж�rsn ie�е�����Կ�Ƿ��ȡ ��2Ϊie��ID��len���ȣ�Pairwise Cipher Suite List 1��Ϊ4�ֽ� */
            if (MAC_IE_REAMIN_LEN_IS_ENOUGH(2, ul_offset, uc_ie_len, 4) == OAL_FALSE) {
                return OAL_FAIL;
            }
            ul_offset += MAC_OUI_LEN;
            pst_connect_param->st_crypto.ciphers_pairwise[uc_loop] = (oal_uint8)puc_ie[ul_offset];
            ul_offset += MAC_OUITYPE_WPA;
        }
    }

    /* ��ȡAKM cout */
    pst_connect_param->st_crypto.n_akm_suites = puc_ie[ul_offset];
    /* puc_ieƫ��ul_offset + 1�ֽ�������8λ */
    pst_connect_param->st_crypto.n_akm_suites += (oal_uint8)(puc_ie[ul_offset + 1] << 8);

    /* ��ȡAKM type */
    ul_offset += MAC_RSN_CIPHER_COUNT_LEN;
    if (pst_connect_param->st_crypto.n_akm_suites) {
        uc_akm_suite_num = oal_min(pst_connect_param->st_crypto.n_akm_suites, OAL_NL80211_MAX_NR_AKM_SUITES);
        for (uc_loop = 0; uc_loop < uc_akm_suite_num; uc_loop++) {
            /* �ж�rsn ie����֤�׼��Ƿ��ȡ��2Ϊie��ID��len���ȣ�AKM Suite List 1��Ϊ4�ֽ� */
            if (MAC_IE_REAMIN_LEN_IS_ENOUGH(2, ul_offset, uc_ie_len, 4) == OAL_FALSE) {
                return OAL_FAIL;
            }
            ul_offset += MAC_OUI_LEN;
            pst_connect_param->st_crypto.akm_suites[uc_loop] = (oal_uint8)puc_ie[ul_offset];
            ul_offset += MAC_OUITYPE_WPA;
        }
    }
    return OAL_SUCC;
}


oal_uint32 wal_set_crypto_info(mac_cfg80211_connect_param_stru *pst_connect_param,
                               oal_cfg80211_conn_stru *pst_sme)
{
    oal_uint8 uc_loop = 0;
    oal_uint8 uc_akm_suite_num = 0;
    oal_uint8 uc_pairwise_cipher_num = 0;
    oal_uint32 ul_ret = OAL_SUCC;
    oal_uint8 *puc_ie = OAL_PTR_NULL;

    puc_ie = mac_find_ie(MAC_EID_RSN, (oal_uint8 *)pst_sme->ie, (oal_int32)(pst_sme->ie_len));

    if ((pst_sme->key_len != 0) && (pst_sme->crypto.n_akm_suites == 0)) {
        /* ����wep������Ϣ */
        ul_ret = wal_set_wep_key(pst_connect_param, pst_sme);
    } else if (pst_sme->crypto.n_akm_suites != 0) {
        /* ����WPA/WPA2 ������Ϣ */
        pst_connect_param->st_crypto.wpa_versions = (oal_uint8)pst_sme->crypto.wpa_versions;
        pst_connect_param->st_crypto.cipher_group = (oal_uint8)pst_sme->crypto.cipher_group;
        pst_connect_param->st_crypto.n_ciphers_pairwise = (oal_uint8)pst_sme->crypto.n_ciphers_pairwise;
        pst_connect_param->st_crypto.n_akm_suites = (oal_uint8)pst_sme->crypto.n_akm_suites;
        pst_connect_param->st_crypto.control_port = (oal_uint8)pst_sme->crypto.control_port;

        uc_pairwise_cipher_num = oal_min(pst_connect_param->st_crypto.n_ciphers_pairwise,
                                         OAL_NL80211_MAX_NR_CIPHER_SUITES);
        for (uc_loop = 0; uc_loop < uc_pairwise_cipher_num; uc_loop++) {
            pst_connect_param->st_crypto.ciphers_pairwise[uc_loop] =
                (oal_uint8)pst_sme->crypto.ciphers_pairwise[uc_loop];
        }

        uc_akm_suite_num = oal_min(pst_connect_param->st_crypto.n_akm_suites, OAL_NL80211_MAX_NR_AKM_SUITES);
        for (uc_loop = 0; uc_loop < uc_akm_suite_num; uc_loop++) {
            pst_connect_param->st_crypto.akm_suites[uc_loop] = (oal_uint8)pst_sme->crypto.akm_suites[uc_loop];
        }
    } else if (puc_ie != OAL_PTR_NULL) {
        /* ����ʹ��PMF STAUT�·�n_akm_suites==0��RSN������� */
        /* ����WPA/WPA2 ������Ϣ */
        pst_connect_param->st_crypto.control_port = (oal_uint8)pst_sme->crypto.control_port;
        pst_connect_param->st_crypto.wpa_versions = (oal_uint8)pst_sme->crypto.wpa_versions;

        ul_ret = wal_set_rsn_key(puc_ie, pst_connect_param);
    } else if (mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WPS,
                                  (oal_uint8 *)pst_sme->ie, (oal_int32)(pst_sme->ie_len))) {
        /*  ���ʹ����WPS���򷵻سɹ� */
        oam_warning_log0(0, OAM_SF_CFG, "{wal_set_crypto_info:connect use wps method!}");
        ul_ret = OAL_SUCC;
    } else {
        ul_ret = OAL_FAIL;
    }

    return ul_ret;
}

#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)) */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34)


oal_bool_enum_uint8 wal_is_p2p_device(oal_net_device_stru *pst_net_device)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    mac_vap_stru *pst_mac_vap;
    hmac_vap_stru *pst_hmac_vap;

    pst_mac_vap = oal_net_dev_priv(pst_net_device);
    if (pst_mac_vap == OAL_PTR_NULL) {
        return OAL_FALSE;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if ((pst_hmac_vap != OAL_PTR_NULL) && (pst_hmac_vap->pst_p2p0_net_device != OAL_PTR_NULL) &&
        (pst_net_device == pst_hmac_vap->pst_p2p0_net_device)) {
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
#else
    return OAL_FALSE;
#endif /* _PRE_WLAN_FEATURE_P2P */
}


OAL_STATIC oal_void wal_free_connect_param_resource(mac_cfg80211_connect_param_stru *pst_conn_param)
{
    if (pst_conn_param->puc_wep_key != OAL_PTR_NULL) {
        oal_free(pst_conn_param->puc_wep_key);
        pst_conn_param->puc_wep_key = OAL_PTR_NULL;
    }
    if (pst_conn_param->puc_ie != OAL_PTR_NULL) {
        oal_free(pst_conn_param->puc_ie);
        pst_conn_param->puc_ie = OAL_PTR_NULL;
    }
}


oal_void wal_set_wapi_flag(
    mac_cfg80211_connect_param_stru *pst_mac_cfg80211_connect_param, oal_cfg80211_conn_stru *pst_sme)
{
#ifdef _PRE_WLAN_FEATURE_WAPI
    if (pst_sme->crypto.wpa_versions == WITP_WAPI_VERSION) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_set_wapi_flag::crypt ver is wapi!");
        pst_mac_cfg80211_connect_param->uc_wapi = OAL_TRUE;
    } else {
        pst_mac_cfg80211_connect_param->uc_wapi = OAL_FALSE;
    }
#endif
}

oal_int32 wal_cfg80211_connect(oal_wiphy_stru *pst_wiphy,
                               oal_net_device_stru *pst_net_device,
                               oal_cfg80211_conn_stru *pst_sme)
{
    mac_cfg80211_connect_param_stru st_mac_cfg80211_connect_param;
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint8 *puc_ie = OAL_PTR_NULL;

    if ((pst_wiphy == OAL_PTR_NULL) || (pst_net_device == OAL_PTR_NULL) || (pst_sme == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_connect::connect failed, pst_wiphy, pst_netdev,\
            pst_sme is NULL!}");

        return -OAL_EINVAL;
    }
#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_connect::dfr_process_status[%d]!}",
                         g_st_dfr_info.bit_device_reset_process_flag);
        return -OAL_EFAIL;
    }

#endif  // #ifdef _PRE_WLAN_FEATURE_DFR
    /* begin: ��ֹ����p2p device�豸�������� */
    if (wal_is_p2p_device(pst_net_device)) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_cfg80211_connect:connect stop, p2p device should not connect.");
        return -OAL_EINVAL;
    }
    /* end: ��ֹ����p2p device�豸�������� */
    /* ��ʼ���������Ӳ��� */
    memset_s(&st_mac_cfg80211_connect_param, OAL_SIZEOF(mac_cfg80211_connect_param_stru),
             0, OAL_SIZEOF(mac_cfg80211_connect_param_stru));

    /* �����ں��·��� ssid */
    st_mac_cfg80211_connect_param.uc_ssid_len = oal_min((oal_uint8)pst_sme->ssid_len, WLAN_SSID_MAX_LEN);
    l_ret = memcpy_s(st_mac_cfg80211_connect_param.auc_ssid, OAL_SIZEOF(st_mac_cfg80211_connect_param.auc_ssid),
        pst_sme->ssid, st_mac_cfg80211_connect_param.uc_ssid_len);
    /* BEGIN  roaming failure between different encryption methods ap. */
    if (pst_sme->bssid) {
        oal_set_mac_addr(st_mac_cfg80211_connect_param.auc_bssid, (oal_uint8 *)pst_sme->bssid);
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
    else if (pst_sme->bssid_hint) {
        oal_set_mac_addr(st_mac_cfg80211_connect_param.auc_bssid, (oal_uint8 *)pst_sme->bssid_hint);
    }
#endif
    else {
        oam_warning_log0(0, OAM_SF_ASSOC, "{wal_cfg80211_connect::bssid and bssid_hint is NULL.}");
        return -OAL_EFAIL;
    }
    /* END  roaming failure between different encryption methods ap. */
    /* �����ں��·��İ�ȫ��ز��� */
    /* ������֤���� */
    st_mac_cfg80211_connect_param.en_auth_type = pst_sme->auth_type;

    /* ���ü������� */
    st_mac_cfg80211_connect_param.en_privacy = pst_sme->privacy;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    /* ��ȡ�ں��·���pmf��ʹ�ܵĽ�� */
    st_mac_cfg80211_connect_param.en_mfp = pst_sme->mfp;
    oam_warning_log4(0, OAM_SF_ANY, "{wal_cfg80211_connect::start a new connect, ssid_len[%d], auth_type[%d], \
            privacy[%d], mfp[%d]}", pst_sme->ssid_len, pst_sme->auth_type, pst_sme->privacy, pst_sme->mfp);
#endif

    /* ���ü��ܲ��� */
    wal_set_wapi_flag(&st_mac_cfg80211_connect_param, pst_sme);
    if (pst_sme->privacy) {
        ul_ret = wal_set_crypto_info(&st_mac_cfg80211_connect_param, pst_sme);
        if (ul_ret != OAL_SUCC) {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_connect::connect failed, wal_set_wep_key fail:%d!}", ul_ret);
            return -OAL_EFAIL;
        }
    }
    /* ���ù���P2P/WPS ie */
    st_mac_cfg80211_connect_param.ul_ie_len = (oal_uint32)(pst_sme->ie_len);
    if ((st_mac_cfg80211_connect_param.ul_ie_len > 0) && (pst_sme->ie != OAL_PTR_NULL)) {
        puc_ie = (oal_uint8 *)oal_memalloc(pst_sme->ie_len);
        if (puc_ie == OAL_PTR_NULL) {
            OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_connect::alloc ie [%d] return null ptr!}",
                           pst_sme->ie_len);
            wal_free_connect_param_resource(&st_mac_cfg80211_connect_param);
            return -OAL_ENOMEM;
        }
        l_ret += memcpy_s(puc_ie, pst_sme->ie_len, (oal_uint8 *)pst_sme->ie, st_mac_cfg80211_connect_param.ul_ie_len);
        st_mac_cfg80211_connect_param.puc_ie = puc_ie;
    }
    if (l_ret != EOK) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "wal_cfg80211_connect:memcpy fail, l_ret=%d", l_ret);
        wal_free_connect_param_resource(&st_mac_cfg80211_connect_param);
        return -OAL_EINVAL;
    }
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    wlan_pm_set_timeout(WLAN_SLEEP_LONG_CHECK_CNT);
#endif
    /* BEGIN:, ����ǰ��Ҫȡ�����ڽ��е�ɨ�裬
     * ������������·�ʱvap ����ɨ��״̬��������ִ�й���������¹���ʧ�� */
    wal_force_scan_complete(pst_net_device, OAL_TRUE);
    /* END:, ����ǰ��Ҫȡ�����ڽ��е�ɨ�裬
     * ������������·�ʱvap ����ɨ��״̬��������ִ�й���������¹���ʧ�� */
    /* ���¼����������������� */
    l_ret = wal_cfg80211_start_connect(pst_net_device, &st_mac_cfg80211_connect_param);
    if (l_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_connect::wal_cfg80211_start_connect fail %d!}\r\n", l_ret);

        /* :ͬ��1103�޸ģ����������·��ɹ�����HMAC�ͷ��ڴ棻������WAL�ͷ��ڴ� */
        if ((l_ret != -OAL_EFAIL) && (l_ret != -OAL_ETIMEDOUT)) {
            wal_free_connect_param_resource(&st_mac_cfg80211_connect_param);
        }
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


oal_int32 wal_cfg80211_disconnect(oal_wiphy_stru *pst_wiphy,
                                  oal_net_device_stru *pst_net_device,
                                  oal_uint16 us_reason_code)
{
    mac_cfg_kick_user_param_stru st_mac_cfg_kick_user_param;
    oal_int32 l_ret;

    mac_user_stru *pst_mac_user;
    mac_vap_stru *pst_mac_vap;

    if ((pst_wiphy == OAL_PTR_NULL) || (pst_net_device == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_ANY,
                       "{wal_cfg80211_disconnect::input param pointer is null,pst_wiphy, pst_netdev %x, %x!}\r\n",
                       (uintptr_t)pst_wiphy, (uintptr_t)pst_net_device);

        return -OAL_EINVAL;
    }
#ifdef _PRE_WLAN_CHBA_MGMT
    /* ����CHBA VAP���������� */
    if ((hwifi_get_chba_en() == OAL_TRUE) &&
        (oal_strcmp("chba0", pst_net_device->name) == 0)) {
        oam_warning_log0(0, 0, "CHBA: Not suppport such disconnect cmd, please use NL80211_CMD_DEL_STATION.");
        return OAL_SUCC;
    }
#endif
#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_disconnect::dfr_process_status[%d]!}",
                         g_st_dfr_info.bit_device_reset_process_flag);
        return -OAL_EFAIL;
    }

#endif  // #ifdef _PRE_WLAN_FEATURE_DFR
    /* �����ں��·���connect���� */
    memset_s(&st_mac_cfg_kick_user_param, OAL_SIZEOF(mac_cfg_kick_user_param_stru),
             0, OAL_SIZEOF(mac_cfg_kick_user_param_stru));

    /* �����ں��·���ȥ����ԭ�� */
    st_mac_cfg_kick_user_param.us_reason_code = us_reason_code;

    /* ��д��sta������ap mac ��ַ */
    pst_mac_vap = oal_net_dev_priv(pst_net_device);
    /* Begin�� ��ֹɾ��p2p��ʱ�����netdev�µ�vap����ʱdisconnect������netdev���ڣ�vap����գ��Ӷ�ʹ�ÿ�ָ������ */
    if (pst_mac_vap == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_disconnect::pst_mac_vap is null!}\r\n");
        return -OAL_EFAIL;
    }
    /* End�� */
    pst_mac_user = mac_res_get_mac_user(pst_mac_vap->uc_assoc_vap_id);
    if (pst_mac_user == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY,
                         "{wal_cfg80211_disconnect::mac_res_get_mac_user pst_mac_user is null, user idx[%d]!}\r\n",
                         pst_mac_vap->uc_assoc_vap_id);
        return OAL_SUCC;
    }

    memcpy_s(st_mac_cfg_kick_user_param.auc_mac_addr, WLAN_MAC_ADDR_LEN,
             pst_mac_user->auc_user_mac_addr, WLAN_MAC_ADDR_LEN);

    l_ret = wal_cfg80211_start_disconnect(pst_net_device, &st_mac_cfg_kick_user_param);
    if (l_ret != OAL_SUCC) {
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
#endif


#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_int32 wal_cfg80211_add_key(oal_wiphy_stru *pst_wiphy,
                               oal_net_device_stru *pst_netdev,
                               oal_uint8 uc_key_index,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 44)
                               bool en_pairwise,
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34)
                               OAL_CONST oal_uint8 *puc_mac_addr,
#else
                               oal_uint8 *puc_mac_addr,
#endif
                               oal_key_params_stru *pst_params)
#elif (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)
oal_int32 wal_cfg80211_add_key(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
                               oal_uint8 uc_key_index,
                               oal_bool_enum en_pairwise,
                               OAL_CONST oal_uint8 *puc_mac_addr,
                               oal_key_params_stru *pst_params)

#endif
{
    wal_msg_write_stru st_write_msg;
    mac_addkey_param_stru st_payload_params;
    wal_msg_stru *pst_rsp_msg = NULL;
    oal_int32 l_ret = EOK;

    /* 1.1 ��μ�� */
    if ((pst_wiphy == OAL_PTR_NULL) || (pst_netdev == OAL_PTR_NULL) || (pst_params == OAL_PTR_NULL)) {
        oam_error_log3(0, OAM_SF_ANY,
                       "{wal_cfg80211_add_key::Param Check ERROR,pst_wiphy, pst_netdev, pst_params %x, %x, %x!}\r\n",
                       (uintptr_t)pst_wiphy, (uintptr_t)pst_netdev, (uintptr_t)pst_params);
        return -OAL_EINVAL;
    }

    /* 1.2 key���ȼ�飬��ֹ����Խ�� */
    if ((pst_params->key_len > OAL_WPA_KEY_LEN) || (pst_params->key_len < 0) ||
        (pst_params->seq_len > OAL_WPA_SEQ_LEN) || (pst_params->seq_len < 0)) {
        oam_error_log2(0, OAM_SF_ANY, "{wal_cfg80211_add_key::Param Check ERROR! key_len[%x]  seq_len[%x]!}\r\n",
                       (oal_int32)pst_params->key_len, (oal_int32)pst_params->seq_len);
        return -OAL_EINVAL;
    }

    /* 2.1 ��Ϣ����׼�� */
    memset_s(&st_payload_params, OAL_SIZEOF(st_payload_params), 0, OAL_SIZEOF(st_payload_params));
    st_payload_params.uc_key_index = uc_key_index;

    memset_s(st_payload_params.auc_mac_addr, WLAN_MAC_ADDR_LEN, 0, WLAN_MAC_ADDR_LEN);
    if (puc_mac_addr != OAL_PTR_NULL) {
        /* ����ʹ���ں��·���macָ�룬���ܱ��ͷţ���Ҫ������������ʹ�� */
        l_ret += memcpy_s(st_payload_params.auc_mac_addr, WLAN_MAC_ADDR_LEN, puc_mac_addr, WLAN_MAC_ADDR_LEN);
    }

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 44)
    st_payload_params.en_pairwise = en_pairwise;
#else
    st_payload_params.en_pairwise = (puc_mac_addr != OAL_PTR_NULL) ? OAL_TRUE : OAL_FALSE;
#endif
#else
    st_payload_params.en_pairwise = en_pairwise;
#endif

    /* 2.2 ��ȡ�����Կֵ */
    st_payload_params.st_key.key_len = pst_params->key_len;
    st_payload_params.st_key.seq_len = pst_params->seq_len;
    st_payload_params.st_key.cipher = pst_params->cipher;
    l_ret += memcpy_s(st_payload_params.st_key.auc_key, OAL_WPA_KEY_LEN,
                      pst_params->key, (oal_uint32)pst_params->key_len);
    l_ret += memcpy_s(st_payload_params.st_key.auc_seq, OAL_WPA_SEQ_LEN,
                      pst_params->seq, (oal_uint32)pst_params->seq_len);
    oam_info_log3(0, OAM_SF_ANY, "{wal_cfg80211_add_key::key_len:%d, seq_len:%d, cipher:0x%08x!}\r\n",
                  pst_params->key_len, pst_params->seq_len, pst_params->cipher);

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* 3.1 ��д msg ��Ϣͷ */
    st_write_msg.en_wid = WLAN_CFGID_ADD_KEY;
    st_write_msg.us_len = OAL_SIZEOF(mac_addkey_param_stru);

    /* 3.2 ��д msg ��Ϣ�� */
    l_ret += memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value),
                      &st_payload_params, OAL_SIZEOF(mac_addkey_param_stru));
    if (l_ret != EOK) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_cfg80211_add_key::memcpy fail! l_ret[%d]", l_ret);
    }

    /* ������Ϣ��ʹ���˾ֲ�����ָ�룬�����Ҫ�����͸ú�������Ϊͬ��������hmac����ʱ��ʹ��Ұָ�� */
    if (OAL_SUCC != wal_send_cfg_event(pst_netdev,
                                       WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_addkey_param_stru),
                                       (oal_uint8 *)&st_write_msg,
                                       OAL_TRUE, &pst_rsp_msg)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_add_key::wal_send_cfg_event fail!}");
        return -OAL_EFAIL;
    }

    if (wal_check_and_release_msg_resp(pst_rsp_msg) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_add_key::wal_check_and_release_msg_resp fail!}");
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_int32 wal_cfg80211_get_key(oal_wiphy_stru *pst_wiphy,
                               oal_net_device_stru *pst_netdev,
                               oal_uint8 uc_key_index,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 44)
                               bool en_pairwise,
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34)
                               OAL_CONST oal_uint8 *puc_mac_addr,
#else
                               oal_uint8 *puc_mac_addr,
#endif
                               void *cookie,
                               void (*callback)(void *cookie, oal_key_params_stru *))
#else
oal_int32 wal_cfg80211_get_key(oal_wiphy_stru *pst_wiphy,
                               oal_net_device_stru *pst_netdev,
                               oal_uint8 uc_key_index,
                               oal_bool_enum en_pairwise,
                               OAL_CONST oal_uint8 *puc_mac_addr,
                               void *cookie,
                               void (*callback)(void *cookie, oal_key_params_stru *))

#endif

{
    wal_msg_write_stru st_write_msg;
    mac_getkey_param_stru st_payload_params = { 0 };
    oal_uint8 auc_mac_addr[WLAN_MAC_ADDR_LEN];
    wal_msg_stru *pst_rsp_msg = NULL;
    oal_int32 l_ret = EOK;

    /* 1.1 ��μ�� */
    if ((oal_unlikely(pst_wiphy == OAL_PTR_NULL)) || (oal_unlikely(pst_netdev == OAL_PTR_NULL))
        || (oal_unlikely(cookie == OAL_PTR_NULL)) || (oal_unlikely(callback == OAL_PTR_NULL))) {
        oam_error_log4(0, OAM_SF_ANY,
                       "{wal_cfg80211_get_key::Param Check ERROR,pst_wiphy, pst_netdev, cookie, callback\
            %x, %x, %x, %x!}\r\n",
                       (uintptr_t)pst_wiphy, (uintptr_t)pst_netdev, (uintptr_t)cookie, (uintptr_t)callback);
        return -OAL_EINVAL;
    }

    /* 2.1 ��Ϣ����׼�� */
    st_payload_params.pst_netdev = pst_netdev;
    st_payload_params.uc_key_index = uc_key_index;

    if (puc_mac_addr != OAL_PTR_NULL) {
        /* ����ʹ���ں��·���macָ�룬���ܱ��ͷţ���Ҫ������������ʹ�� */
        l_ret += memcpy_s(auc_mac_addr, WLAN_MAC_ADDR_LEN, puc_mac_addr, WLAN_MAC_ADDR_LEN);
        st_payload_params.puc_mac_addr = auc_mac_addr;
    } else {
        st_payload_params.puc_mac_addr = OAL_PTR_NULL;
    }

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 44)
    st_payload_params.en_pairwise = en_pairwise;
#else
    if (puc_mac_addr != OAL_PTR_NULL) {
        st_payload_params.en_pairwise = OAL_TRUE;
    } else {
        st_payload_params.en_pairwise = OAL_FALSE;
    }
#endif
#else
    st_payload_params.en_pairwise = en_pairwise;
#endif
    st_payload_params.cookie = cookie;
    st_payload_params.callback = callback;

    oam_info_log2(0, OAM_SF_ANY, "{wal_cfg80211_get_key::key_idx:%d, en_pairwise:%d!}\r\n",
                  uc_key_index, st_payload_params.en_pairwise);
    if (puc_mac_addr != OAL_PTR_NULL) {
        oam_info_log4(0, OAM_SF_ANY, "{wal_cfg80211_get_key::MAC ADDR: %02X:XX:XX:%02X:%02X:%02X!}\r\n",
                      puc_mac_addr[0], puc_mac_addr[3], puc_mac_addr[4], puc_mac_addr[5]);
    } else {
        oam_info_log0(0, OAM_SF_ANY, "{wal_cfg80211_get_key::MAC ADDR IS null!}\r\n");
    }
    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* 3.1 ��д msg ��Ϣͷ */
    st_write_msg.en_wid = WLAN_CFGID_GET_KEY;
    st_write_msg.us_len = OAL_SIZEOF(mac_getkey_param_stru);

    /* 3.2 ��д msg ��Ϣ�� */
    l_ret += memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value),
                      &st_payload_params, OAL_SIZEOF(mac_getkey_param_stru));
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_cfg80211_get_key::memcpy fail!");
        return -OAL_EINVAL;
    }

    /* ������Ϣ��ʹ���˾ֲ�����ָ�룬�����Ҫ�����͸ú�������Ϊͬ��������hmac����ʱ��ʹ��Ұָ�� */
    if (OAL_SUCC != wal_send_cfg_event(pst_netdev,
                                       WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_getkey_param_stru),
                                       (oal_uint8 *)&st_write_msg,
                                       OAL_TRUE,
                                       &pst_rsp_msg)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_get_key::wal_send_cfg_event fail.}");
        return -OAL_EINVAL;
    }

    if (wal_check_and_release_msg_resp(pst_rsp_msg) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_get_key::wal_check_and_release_msg_resp fail.}");
        return -OAL_EINVAL;
    }

    return OAL_SUCC;
}


#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_int32 wal_cfg80211_remove_key(oal_wiphy_stru *pst_wiphy,
    oal_net_device_stru *pst_netdev,
    oal_uint8 uc_key_index,
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 37)
    bool en_pairwise,
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34)
    OAL_CONST oal_uint8 *puc_mac_addr)
#else
    oal_uint8 *puc_mac_addr)
#endif
#else
oal_int32 wal_cfg80211_remove_key(oal_wiphy_stru *pst_wiphy,
                                  oal_net_device_stru *pst_netdev,
                                  oal_uint8 uc_key_index,
                                  oal_bool_enum en_pairwise,
                                  OAL_CONST oal_uint8 *puc_mac_addr)

#endif

{
    mac_removekey_param_stru st_payload_params = { 0 };
    wal_msg_write_stru st_write_msg = { 0 };
    wal_msg_stru *pst_rsp_msg = NULL;
    oal_int32 l_ret = EOK;

    /* 1.1 ��μ�� */
    if ((oal_unlikely(pst_wiphy == OAL_PTR_NULL)) || (oal_unlikely(pst_netdev == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_ANY, "{wal_cfg80211_remove_key::Param Check ERROR,pst_wiphy, pst_netdev %x, %x!}\r\n",
                       (uintptr_t)pst_wiphy, (uintptr_t)pst_netdev);
        return -OAL_EINVAL;
    }

#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_remove_key::dfr_process_status[%d]!}",
                         g_st_dfr_info.bit_device_reset_process_flag);
        return OAL_SUCC;
    }
#endif  // #ifdef _PRE_WLAN_FEATURE_DFR

    /* 2.1 ��Ϣ����׼�� */
    st_payload_params.uc_key_index = uc_key_index;
    memset_s(st_payload_params.auc_mac_addr, OAL_MAC_ADDR_LEN, 0, OAL_MAC_ADDR_LEN);
    if (puc_mac_addr != OAL_PTR_NULL) {
        /* ����ʹ���ں��·���macָ�룬���ܱ��ͷţ���Ҫ������������ʹ�� */
        l_ret += memcpy_s(st_payload_params.auc_mac_addr, OAL_MAC_ADDR_LEN, puc_mac_addr, WLAN_MAC_ADDR_LEN);
    }

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 37)
    st_payload_params.en_pairwise = en_pairwise;
#else
    if (puc_mac_addr != OAL_PTR_NULL) {
        st_payload_params.en_pairwise = OAL_TRUE;
        oam_info_log4(0, OAM_SF_ANY, "{wal_cfg80211_remove_key::MAC ADDR: %02X:XX:XX:%02X:%02X:%02X!}\r\n",
                      puc_mac_addr[0], puc_mac_addr[3], puc_mac_addr[4], puc_mac_addr[5]);
    } else {
        st_payload_params.en_pairwise = OAL_FALSE;
        oam_info_log0(0, OAM_SF_ANY, "{wal_cfg80211_remove_key::MAC ADDR IS null!}\r\n");
    }
#endif
#else
    st_payload_params.en_pairwise = en_pairwise;
#endif

    oam_info_log2(0, OAM_SF_ANY, "{wal_cfg80211_remove_key::uc_key_index:%d, en_pairwise:%d!}\r\n",
                  uc_key_index, st_payload_params.en_pairwise);

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* 3.1 ��д msg ��Ϣͷ */
    st_write_msg.en_wid = WLAN_CFGID_REMOVE_KEY;
    st_write_msg.us_len = OAL_SIZEOF(mac_removekey_param_stru);

    /* 3.2 ��д msg ��Ϣ�� */
    l_ret += memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value),
                      &st_payload_params, OAL_SIZEOF(mac_removekey_param_stru));
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_cfg80211_remove_key::memcpy fail!");
        return -OAL_EFAIL;
    }

    if (OAL_SUCC != wal_send_cfg_event(pst_netdev,
                                       WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_removekey_param_stru),
                                       (oal_uint8 *)&st_write_msg,
                                       OAL_TRUE, &pst_rsp_msg)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_remove_key::wal_send_cfg_event fail.}");
        return -OAL_EFAIL;
    }

    if (wal_check_and_release_msg_resp(pst_rsp_msg) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_remove_key::wal_check_and_release_msg_resp fail.}");
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_int32 wal_cfg80211_set_default_key(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
    oal_uint8 uc_key_index
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 38)
,   bool en_unicast, bool en_multicast
#endif
)
#else
oal_int32 wal_cfg80211_set_default_key(
    oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev, oal_uint8 uc_key_index,
    oal_bool_enum en_unicast, oal_bool_enum en_multicast)

#endif
{
    mac_setdefaultkey_param_stru st_payload_params = { 0 };
    oal_int32 l_ret;
    wal_msg_write_stru st_write_msg = { 0 };

    /* 1.1 ��μ�� */
    if ((oal_unlikely(pst_wiphy == OAL_PTR_NULL)) || (oal_unlikely(pst_netdev == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_ANY,
                       "{wal_cfg80211_set_default_key::pst_wiphy or pst_netdev ptr is null, error %x, %x!}",
                       (uintptr_t)pst_wiphy, (uintptr_t)pst_netdev);
        return -OAL_EINVAL;
    }

    /* 2.1 ��Ϣ����׼�� */
    st_payload_params.uc_key_index = uc_key_index;

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 44)
    st_payload_params.en_unicast = en_unicast;
    st_payload_params.en_multicast = en_multicast;
#else
    st_payload_params.en_unicast = OAL_TRUE;
    st_payload_params.en_multicast = OAL_TRUE;
#endif
#else
    st_payload_params.en_unicast = en_unicast;
    st_payload_params.en_multicast = en_multicast;
#endif

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* 3.1 ��д msg ��Ϣͷ */
    st_write_msg.en_wid = WLAN_CFGID_DEFAULT_KEY;
    st_write_msg.us_len = OAL_SIZEOF(mac_setdefaultkey_param_stru);

    /* 3.2 ��д msg ��Ϣ�� */
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(mac_setdefaultkey_param_stru),
                 &st_payload_params, OAL_SIZEOF(mac_setdefaultkey_param_stru)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_cfg80211_set_default_key::memcpy fail!");
        return -OAL_EFAIL;
    }

    l_ret = wal_send_cfg_event(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_setdefaultkey_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY,
                         "{wal_cfg80211_set_default_key::wal_send_cfg_event return err code %d!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


oal_int32 wal_cfg80211_set_default_mgmt_key(oal_wiphy_stru *pst_wiphy,
                                            oal_net_device_stru *pst_netdev,
                                            oal_uint8 uc_key_index)
{
    mac_setdefaultkey_param_stru st_payload_params = { 0 };
    oal_int32 l_ret;
    wal_msg_write_stru st_write_msg = { 0 };

    /* 1.1 ��μ�� */
    if ((oal_unlikely(pst_wiphy == OAL_PTR_NULL)) || (oal_unlikely(pst_netdev == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_ANY,
                       "{wal_cfg80211_set_default_mgmt_key::pst_wiphy or pst_netdev ptr is null, \
            error %x, %x!}\r\n",
                       (uintptr_t)pst_wiphy, (uintptr_t)pst_netdev);
        return -OAL_EINVAL;
    }

    /* 2.1 ��Ϣ����׼�� */
    st_payload_params.uc_key_index = uc_key_index;
    st_payload_params.en_unicast = OAL_FALSE;
    st_payload_params.en_multicast = OAL_TRUE;

    oam_info_log3(0, OAM_SF_ANY, "{wal_cfg80211_set_default_mgmt_key::key_index:%d, unicast:%d, multicast:%d!}\r\n",
                  uc_key_index, st_payload_params.en_unicast, st_payload_params.en_multicast);

    /***************************************************************************
    ���¼���wal�㴦��
    ***************************************************************************/
    /* 3.1 ��д msg ��Ϣͷ */
    st_write_msg.en_wid = WLAN_CFGID_DEFAULT_KEY;
    st_write_msg.us_len = OAL_SIZEOF(mac_setdefaultkey_param_stru);

    /* 3.2 ��д msg ��Ϣ�� */
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(mac_setdefaultkey_param_stru),
                 &st_payload_params, OAL_SIZEOF(mac_setdefaultkey_param_stru)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_cfg80211_set_default_mgmt_key::memcpy fail!");
        return -OAL_EFAIL;
    }

    l_ret = wal_send_cfg_event(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_setdefaultkey_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY,
                         "{wal_cfg80211_set_default_mgmt_key::wal_send_cfg_event return err code %d!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


/*lint -e40*/
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 34)) || ((_PRE_OS_VERSION_WIN32_RAW == _PRE_OS_VERSION) ||  \
                                                            (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION))
OAL_STATIC oal_int32 wal_cfg80211_set_channel(oal_wiphy_stru *pst_wiphy,
                                              oal_ieee80211_channel *pst_chan,
                                              oal_nl80211_channel_type en_channel_type)
{
    /* ͨ��HOSTAPD �����ŵ����ýӿ�wal_ioctl_set_channel */
    oam_warning_log0(0, OAM_SF_ANY,
                     "{wal_cfg80211_set_channel::should not call this function. call wal_ioctl_set_channel!}\r\n");
    return -OAL_EFAIL;
}
#endif
/*lint +e40*/

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
OAL_STATIC oal_int32 wal_cfg80211_set_wiphy_params(oal_wiphy_stru *pst_wiphy, oal_uint32 ul_changed)
{
    /* ͨ��HOSTAPD ����RTS ���ޣ���Ƭ���� ���ýӿ�wal_ioctl_set_frag�� wal_ioctl_set_rts */
    oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_set_wiphy_params::should not call this function. \
        call wal_ioctl_set_frag/wal_ioctl_set_rts!}\r\n");
    return OAL_SUCC;
}
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 0, 44))


oal_int32 wal_cfg80211_set_beacon(oal_wiphy_stru *pst_wiphy,
                                  oal_net_device_stru *pst_dev,
                                  oal_beacon_parameters *pst_beacon_info)
{
    mac_beacon_param_stru st_beacon_param; /* beacon info struct */
    wal_msg_write_stru st_write_msg;
    mac_vap_stru *pst_mac_vap;
    oal_int32 l_loop = 0;
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint8 uc_vap_id;
    oal_uint8 *puc_ie;
    mac_cfg_ssid_param_stru *pst_ssid_param;
    oal_uint8 uc_ssid_len;
    oal_int32 l_memcpy_ret = EOK;

    if ((pst_wiphy == OAL_PTR_NULL) || (pst_dev == OAL_PTR_NULL) || (pst_beacon_info == OAL_PTR_NULL)) {
        oam_error_log3(0, OAM_SF_ANY,
                       "{wal_cfg80211_set_beacon::pst_wiphy = %x, pst_dev = %x, \
            pst_beacon_info = %x!}\r\n", pst_wiphy,
                       pst_dev, pst_beacon_info);
        return -OAL_EINVAL;
    }

    if (pst_beacon_info->head == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY,
                       "{wal_cfg80211_set_beacon::pst_beacon_info->head %x!}\r\n", pst_beacon_info->head);
        return -OAL_EINVAL;
    }

    /* ��ȡvap id */
    pst_mac_vap = oal_net_dev_priv(pst_dev);
    uc_vap_id = pst_mac_vap->uc_vap_id;

    /* ��ʼ��beacon interval ��DTIM_PERIOD ���� */
    memset_s(&st_beacon_param, OAL_SIZEOF(mac_beacon_param_stru), 0, OAL_SIZEOF(mac_beacon_param_stru));
    st_beacon_param.l_interval = pst_beacon_info->interval;
    st_beacon_param.l_dtim_period = pst_beacon_info->dtim_period;

    puc_ie = mac_get_ssid(pst_beacon_info->head + MAC_80211_FRAME_LEN,
                          (pst_beacon_info->head_len - MAC_80211_FRAME_LEN), &uc_ssid_len);
    /* ����SSID�������:(1)����Ϊ0��(2)����Ϊʵ��ssid���ȣ�������Ϊ�� */
    st_beacon_param.uc_hidden_ssid = 0;
    if (mac_is_hide_ssid(puc_ie, uc_ssid_len) == OAL_TRUE) {
        st_beacon_param.uc_hidden_ssid = 1;
    }

    /*****************************************************************************
        1. �·���ȫ������Ϣ
    *****************************************************************************/
    /* ��ȡ WPA/WPA2 ��ϢԪ�� */
    if ((pst_beacon_info->tail == OAL_PTR_NULL) || (pst_beacon_info->head == OAL_PTR_NULL)) {
        oam_error_log2(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_set_beacon::beacon frame error tail = %d, head = %d!}\r\n",
                       pst_beacon_info->tail, pst_beacon_info->head);
        return -OAL_EINVAL;
    }

    ul_ret = wal_parse_wpa_wpa2_ie(pst_beacon_info, &st_beacon_param);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_set_beacon::Failed to parse WPA/WPA2 ie!}\r\n");
        return -OAL_EINVAL;
    }

    ul_ret = wal_parse_ht_vht_ie(pst_mac_vap, pst_beacon_info, &st_beacon_param);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_set_beacon::Failed to parse HT/VHT ie!}\r\n");
        return -OAL_EINVAL;
    }

#ifdef _PRE_WLAN_FEATURE_11D
    /* ���ձ�14�ŵ��������жϣ�ֻ��11bģʽ�²�������14����11bģʽ ��Ϊ11b */
    if ((pst_mac_vap->st_channel.uc_chan_number == 14) && (st_beacon_param.en_protocol != WLAN_LEGACY_11B_MODE)) {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_set_beacon::Now change protocol to 11b!}\r\n");
        st_beacon_param.en_protocol = WLAN_LEGACY_11B_MODE;
    }
#endif
    for (l_loop = 0; l_loop < MAC_PAIRWISE_CIPHER_SUITES_NUM; l_loop++) {
        oam_info_log2(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_set_beacon::Wpa2 pariwise[%d] = %d!}\r\n",
                      l_loop, st_beacon_param.auc_pairwise_crypto_wpa2[l_loop]);
    }

    /* ��д msg ��Ϣͷ */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CFG80211_CONFIG_BEACON, OAL_SIZEOF(mac_beacon_param_stru));

    /* ��д msg ��Ϣ�� */
    l_memcpy_ret += memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value),
        &st_beacon_param, OAL_SIZEOF(mac_beacon_param_stru));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_beacon_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (l_ret != OAL_SUCC) {
        oam_warning_log0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_beacon::Failed to start addset beacon!}\r\n");
        return -OAL_EFAIL;
    }

    /*****************************************************************************
        2. �·�SSID������Ϣ
    *****************************************************************************/
    if (uc_ssid_len != 0) {
        wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SSID, OAL_SIZEOF(mac_cfg_ssid_param_stru));

        pst_ssid_param = (mac_cfg_ssid_param_stru *)(st_write_msg.auc_value);
        pst_ssid_param->uc_ssid_len = uc_ssid_len;
        l_memcpy_ret += memcpy_s(pst_ssid_param->ac_ssid, WLAN_SSID_MAX_LEN, puc_ie, uc_ssid_len);

        l_ret = wal_send_cfg_event(pst_dev,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_ssid_param_stru),
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_FALSE,
                                   OAL_PTR_NULL);
        if (l_ret != OAL_SUCC) {
            oam_warning_log0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_beacon::fail to send ssid cfg msg!}\r\n");
            return -OAL_EFAIL;
        }
    }
    if (l_memcpy_ret != EOK) {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_beacon::memcpy fail!}");
    }
    return OAL_SUCC;
}


oal_int32 wal_cfg80211_add_beacon(oal_wiphy_stru *pst_wiphy,
                                  oal_net_device_stru *pst_dev,
                                  oal_beacon_parameters *pst_beacon_info)
{
    mac_beacon_param_stru st_beacon_param; /* beacon info struct */
    wal_msg_write_stru st_write_msg;
    mac_vap_stru *pst_mac_vap;
    oal_int32 l_loop = 0;
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint8 uc_vap_id;
    oal_uint8 *puc_ie;
    mac_cfg_ssid_param_stru *pst_ssid_param;
    oal_uint8 uc_ssid_len;
    oal_int32 l_memcpy_ret = EOK;

    if ((pst_wiphy == OAL_PTR_NULL) || (pst_dev == OAL_PTR_NULL) || (pst_beacon_info == OAL_PTR_NULL)) {
        oam_error_log3(0, OAM_SF_ANY,
                       "{wal_cfg80211_add_beacon::pst_wiphy = %x, pst_dev = %x, \
            pst_beacon_info = %x!}\r\n", pst_wiphy,
                       pst_dev, pst_beacon_info);
        return -OAL_EINVAL;
    }

    if (pst_beacon_info->head == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_add_beacon::pst_beacon_info->head %x!}\r\n",
                       pst_beacon_info->head);
        return -OAL_EINVAL;
    }

    /* ��ȡvap id */
    pst_mac_vap = oal_net_dev_priv(pst_dev);
    uc_vap_id = pst_mac_vap->uc_vap_id;

    /* ��ʼ��beacon interval ��DTIM_PERIOD ���� */
    memset_s(&st_beacon_param, OAL_SIZEOF(mac_beacon_param_stru), 0, OAL_SIZEOF(mac_beacon_param_stru));
    st_beacon_param.l_interval = pst_beacon_info->interval;
    st_beacon_param.l_dtim_period = pst_beacon_info->dtim_period;

    puc_ie = mac_get_ssid(pst_beacon_info->head + MAC_80211_FRAME_LEN,
                          (pst_beacon_info->head_len - MAC_80211_FRAME_LEN), &uc_ssid_len);
    /* ����SSID�������:(1)����Ϊ0��(2)����Ϊʵ��ssid���ȣ�������Ϊ�� */
    st_beacon_param.uc_hidden_ssid = 0;
    if (mac_is_hide_ssid(puc_ie, uc_ssid_len) == OAL_TRUE) {
        st_beacon_param.uc_hidden_ssid = 1;
    }

    /*****************************************************************************
        1. �·���ȫ������Ϣ
    *****************************************************************************/
    /* ��ȡ WPA/WPA2 ��ϢԪ�� */
    if ((pst_beacon_info->tail == OAL_PTR_NULL) || (pst_beacon_info->head == OAL_PTR_NULL)) {
        oam_error_log2(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_beacon::beacon frame error tail = %d, head = %d!}\r\n",
                       pst_beacon_info->tail, pst_beacon_info->head);
        return -OAL_EINVAL;
    }

    ul_ret = wal_parse_wpa_wpa2_ie(pst_beacon_info, &st_beacon_param);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_beacon::Failed to parse WPA/WPA2 ie!}\r\n");
        return -OAL_EINVAL;
    }

    ul_ret = wal_parse_ht_vht_ie(pst_mac_vap, pst_beacon_info, &st_beacon_param);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_beacon::Failed to parse HT/VHT ie!}\r\n");
        return -OAL_EINVAL;
    }

#ifdef _PRE_WLAN_FEATURE_11D
    /* ���ձ�14�ŵ��������жϣ�ֻ��11bģʽ�²�������14����11bģʽ ��Ϊ11b */
    if ((pst_mac_vap->st_channel.uc_chan_number == 14) && (st_beacon_param.en_protocol != WLAN_LEGACY_11B_MODE)) {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_beacon::Now change protocol to 11b!}\r\n");
        st_beacon_param.en_protocol = WLAN_LEGACY_11B_MODE;
    }
#endif
    for (l_loop = 0; l_loop < MAC_PAIRWISE_CIPHER_SUITES_NUM; l_loop++) {
        oam_info_log2(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_beacon::Wpa2 pariwise[%d] = %d!}\r\n",
                      l_loop, st_beacon_param.auc_pairwise_crypto_wpa2[l_loop]);
    }

    /* ��д msg ��Ϣͷ */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CFG80211_CONFIG_BEACON, OAL_SIZEOF(mac_beacon_param_stru));

    /* ��д msg ��Ϣ�� */
    l_memcpy_ret += memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value),
        &st_beacon_param, OAL_SIZEOF(mac_beacon_param_stru));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_beacon_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (l_ret != OAL_SUCC) {
        oam_warning_log0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_beacon::Failed to start addset beacon!}\r\n");
        return -OAL_EFAIL;
    }

    /*****************************************************************************
        2. �·�SSID������Ϣ
    *****************************************************************************/
    if (uc_ssid_len != 0) {
        wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SSID, OAL_SIZEOF(mac_cfg_ssid_param_stru));

        pst_ssid_param = (mac_cfg_ssid_param_stru *)(st_write_msg.auc_value);
        pst_ssid_param->uc_ssid_len = uc_ssid_len;
        l_memcpy_ret += memcpy_s(pst_ssid_param->ac_ssid, WLAN_SSID_MAX_LEN, puc_ie, uc_ssid_len);

        l_ret = wal_send_cfg_event(pst_dev,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_ssid_param_stru),
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_FALSE,
                                   OAL_PTR_NULL);
        if (l_ret != OAL_SUCC) {
            oam_warning_log0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_beacon::fail to send ssid cfg msg!}\r\n");
            return -OAL_EFAIL;
        }
    }
    if (l_memcpy_ret != EOK) {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_beacon::memcpy fail!}");
    }
    /* 4.����Wmm��ϢԪ�� */
    ul_ret = wal_parse_wmm_ie(pst_dev, pst_mac_vap, pst_beacon_info);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_beacon::Failed to parse wmm ie!}\r\n");
        return -OAL_EINVAL;
    }

    return OAL_SUCC;
}
#endif

#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 44)) || (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION))


oal_int32 wal_cfg80211_set_ssid(oal_net_device_stru *pst_netdev,
    const uint8_t *ssid_ie, uint32_t ssid_len)
{
    wal_msg_write_stru st_write_msg;
    mac_cfg_ssid_param_stru *pst_ssid_param;
    oal_int32 l_ret;

    if ((ssid_len > OAL_IEEE80211_MAX_SSID_LEN) || (ssid_len <= 0)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_set_ssid::ssid len error, len[%d].}", ssid_len);
        return -OAL_EFAIL;
    }

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SSID, OAL_SIZEOF(mac_cfg_ssid_param_stru));

    pst_ssid_param = (mac_cfg_ssid_param_stru *)(st_write_msg.auc_value);
    pst_ssid_param->uc_ssid_len = ssid_len;
    if (memcpy_s(pst_ssid_param->ac_ssid, sizeof(pst_ssid_param->ac_ssid),
        (oal_int8 *)ssid_ie, ssid_len) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_cfg80211_set_ssid::memcpy fail!");
        return -OAL_EFAIL;
    }

    l_ret = wal_send_cfg_event(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_ssid_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (l_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_set_ssid::fail to send ssid cfg msg, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

/*
 * �� �� ��  : wal_cfg80211_set_auth_mode
 * ��������  : ����vap��auth����
 */
OAL_STATIC oal_int32 wal_cfg80211_set_auth_mode(oal_net_device_stru *pst_netdev, oal_uint8 en_auth_algs)
{
#ifdef _PRE_WLAN_FEATURE_SAE
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;

    if ((en_auth_algs != WLAN_WITP_AUTH_OPEN_SYSTEM) &&
        (en_auth_algs != WLAN_WITP_AUTH_SHARED_KEY) &&
        (en_auth_algs != WLAN_WITP_AUTH_SAE) &&
        (en_auth_algs != WLAN_WITP_AUTH_AUTOMATIC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_set_auth_mode::en_auth_algs error[%d].}", en_auth_algs);
        return -OAL_EFAIL;
    }

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_AUTH_MODE, OAL_SIZEOF(en_auth_algs));
    *((oal_uint8 *)(st_write_msg.auc_value)) = en_auth_algs; /* ��������������� */

    l_ret = wal_send_cfg_event(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(en_auth_algs),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (l_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_set_auth_mode::fail to send auth_tpye cfg msg, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }
#endif
    return OAL_SUCC;
}


oal_int32 wal_cfg80211_fill_beacon_param(mac_vap_stru *pst_mac_vap,
                                         oal_beacon_data_stru *pst_beacon_info,
                                         mac_beacon_param_stru *pst_beacon_param)
{
    oal_beacon_parameters st_beacon_info_tmp;
    oal_uint32 ul_loop;
    oal_uint8 *puc_bcn_info_tmp;
    oal_uint32 ul_bcn_head_len;
    oal_uint32 ul_beacon_tail_len;
    oal_uint8 uc_vap_id;
    oal_int32 l_ret = EOK;
    oal_uint32 ul_offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    if (oal_any_null_ptr3(pst_mac_vap, pst_beacon_info, pst_beacon_param)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::beacon_info or beacon_param is NULL!");
        return -OAL_EINVAL;
    }

    uc_vap_id = pst_mac_vap->uc_vap_id;
    /*****************************************************************************
        1.��ȫ����ie��Ϣ��
    *****************************************************************************/
    if ((pst_beacon_info->tail == OAL_PTR_NULL) || (pst_beacon_info->head == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::beacon tail or head is null!}");
        return -OAL_EINVAL;
    }

    /* oal_ieee80211_mgmt ǰ�湫������sizeΪ MAC_80211_FRAME_LEN(24) */
    if (pst_beacon_info->head_len < (ul_offset + MAC_80211_FRAME_LEN)) {
        OAM_ERROR_LOG1(uc_vap_id, OAM_SF_CFG, "{beacon_info head_len[%d] error.}", pst_beacon_info->head_len);
        return -OAL_EINVAL;
    }

    ul_bcn_head_len = (oal_uint32)pst_beacon_info->head_len;
    ul_beacon_tail_len = (oal_uint32)pst_beacon_info->tail_len;

    /* �����������*/
    if ((ul_bcn_head_len + ul_beacon_tail_len) < ul_bcn_head_len) {
        oam_error_log2(0, OAM_SF_CFG, "{beacon len abnormal:head %d,tail%d.}", ul_bcn_head_len, ul_beacon_tail_len);
        return -OAL_EINVAL;
    }

    puc_bcn_info_tmp = (oal_uint8 *)(oal_memalloc(ul_bcn_head_len + ul_beacon_tail_len));
    if (puc_bcn_info_tmp == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::puc_bcn_info_tmp memalloc failed.}");
        return -OAL_EINVAL;
    }
    l_ret += memcpy_s(puc_bcn_info_tmp, ul_bcn_head_len + ul_beacon_tail_len, pst_beacon_info->head, ul_bcn_head_len);
    l_ret += memcpy_s(puc_bcn_info_tmp + ul_bcn_head_len, ul_beacon_tail_len,
        pst_beacon_info->tail, ul_beacon_tail_len);
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "wal_cfg80211_fill_beacon_param::memcpy fail!");
        oal_free(puc_bcn_info_tmp);
        return -OAL_EINVAL;
    }

    /* Ϊ�˸���51�Ľ����ӿڣ������ں˽ṹ�е����ݸ�ֵ��51�ӿ�ʶ��Ľṹ�壬������ȡ��ϢԪ�� */
    memset_s(&st_beacon_info_tmp, OAL_SIZEOF(st_beacon_info_tmp), 0, OAL_SIZEOF(st_beacon_info_tmp));
    st_beacon_info_tmp.head = puc_bcn_info_tmp;
    st_beacon_info_tmp.head_len = (oal_int32)ul_bcn_head_len;
    st_beacon_info_tmp.tail = puc_bcn_info_tmp + ul_bcn_head_len;
    st_beacon_info_tmp.tail_len = (oal_int32)ul_beacon_tail_len;

    /* ��ȡ WPA/WPA2 ��ϢԪ�� */
    if (wal_parse_wpa_wpa2_ie(&st_beacon_info_tmp, pst_beacon_param) != OAL_SUCC) {
        oam_warning_log0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::failed to parse WPA/WPA2 ie!}");
        oal_free(puc_bcn_info_tmp);
        return -OAL_EINVAL;
    }

    /* �˽ӿ���Ҫ�޸ģ�linux��û���⣬����win32�д� TBD */
    if (wal_parse_ht_vht_ie(pst_mac_vap, &st_beacon_info_tmp, pst_beacon_param) != OAL_SUCC) {
        oam_warning_log0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::failed to parse HT/VHT ie!}");
        oal_free(puc_bcn_info_tmp);
        return -OAL_EINVAL;
    }

    /* �ͷ���ʱ������ڴ� */
    oal_free(puc_bcn_info_tmp);

    oam_warning_log3(uc_vap_id, OAM_SF_ANY, "{crypto_mode=%d, group_crypt=%d, en_protocol=%d!}",
        pst_beacon_param->uc_crypto_mode, pst_beacon_param->uc_group_crypto, pst_beacon_param->en_protocol);

    oam_warning_log2(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::auth_type[0]=%d, auth_type[1]=%d}",
                     pst_beacon_param->auc_auth_type[0], pst_beacon_param->auc_auth_type[1]);

#ifdef _PRE_WLAN_FEATURE_11D
    /* ���ձ�14�ŵ��������жϣ�ֻ��11bģʽ�²�������14����11bģʽ ��Ϊ11b */
    if ((pst_mac_vap->st_channel.uc_chan_number == 14) && (pst_beacon_param->en_protocol != WLAN_LEGACY_11B_MODE)) {
        OAM_ERROR_LOG1(uc_vap_id, OAM_SF_ANY, "{ch 14 shouldn't in %d,change to 11b!}", pst_beacon_param->en_protocol);
        pst_beacon_param->en_protocol = WLAN_LEGACY_11B_MODE;
    }
#endif

    for (ul_loop = 0; ul_loop < MAC_PAIRWISE_CIPHER_SUITES_NUM; ul_loop++) {
        oam_warning_log2(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::wpa pariwise[%d] = %d!}",
                         ul_loop, pst_beacon_param->auc_pairwise_crypto_wpa[ul_loop]);
    }

    for (ul_loop = 0; ul_loop < MAC_PAIRWISE_CIPHER_SUITES_NUM; ul_loop++) {
        oam_warning_log2(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::wpa2 pariwise[%d] = %d!}",
                         ul_loop, pst_beacon_param->auc_pairwise_crypto_wpa2[ul_loop]);
    }

    return OAL_SUCC;
}


oal_int32 wal_cfg80211_change_beacon(oal_wiphy_stru *pst_wiphy,
                                     oal_net_device_stru *pst_netdev,
                                     oal_beacon_data_stru *pst_beacon_info)
{
    mac_beacon_param_stru st_beacon_param; /* beacon info struct */
    wal_msg_write_stru st_write_msg;
    mac_vap_stru *pst_mac_vap;
    oal_int32 l_ret;

    oam_info_log0(0, OAM_SF_ANY, "{wal_cfg80211_change_beacon::enter here.}");

    /* �����Ϸ��Լ�� */
    if ((pst_wiphy == OAL_PTR_NULL) || (pst_netdev == OAL_PTR_NULL) || (pst_beacon_info == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_change_beacon::pst_wiphy or pst_netdev or pst_beacon_info null!}");
        return -OAL_EINVAL;
    }
#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_change_beacon::dfr_process_status[%d]!}",
                         g_st_dfr_info.bit_device_reset_process_flag);
        return OAL_SUCC;
    }
#endif  // #ifdef _PRE_WLAN_FEATURE_DFR

    /* ��ȡvap id */
    pst_mac_vap = oal_net_dev_priv(pst_netdev);
    if (pst_mac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_change_beacon::pst_mac_vap = %x}", (uintptr_t)pst_mac_vap);
        return -OAL_EINVAL;
    }

    /* ��ʼ��beacon interval ��DTIM_PERIOD ���� */
    memset_s(&st_beacon_param, OAL_SIZEOF(mac_beacon_param_stru), 0, OAL_SIZEOF(mac_beacon_param_stru));

    l_ret = wal_cfg80211_fill_beacon_param(pst_mac_vap, pst_beacon_info, &st_beacon_param);
    if (l_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{wal_cfg80211_change_beacon::failed to fill beacon param, error[%d]}", l_ret);
        return -OAL_EINVAL;
    }

    /* ��д msg ��Ϣͷ */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CFG80211_CONFIG_BEACON, OAL_SIZEOF(mac_beacon_param_stru));

    /* ��д msg ��Ϣ�� */
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(mac_beacon_param_stru),
                 &st_beacon_param, OAL_SIZEOF(mac_beacon_param_stru)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_cfg80211_change_beacon::memcpy fail!");
        return -OAL_EFAIL;
    }

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_beacon_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE, OAL_PTR_NULL);
    if (l_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY,
                         "{wal_cfg80211_change_beacon::Failed to start addset beacon, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


oal_int32 wal_cfg80211_convert_width_to_value(oal_int32 l_channel_width)
{
    oal_int32 l_channel_width_value = 0;

    switch (l_channel_width) {
        case 0: /* �¼�0Ϊ�� */
        case 1: /* �¼�1����Ϊ20MHz */
            l_channel_width_value = 20;
            break;
        case 2: /* �¼�2��ʾ����Ϊ40MHz */
            l_channel_width_value = 40;
            break;
        case 3: /* �¼�3Ϊ�� */
        case 4: /* �¼�4��ʾ����Ϊ80MHz */
            l_channel_width_value = 80;
            break;
        case 5: /* �¼�5��ʾ����Ϊ160MHz */
            l_channel_width_value = 160;
            break;
        default:
            break;
    }

    return l_channel_width_value;
}
#ifdef _PRE_WLAN_CHBA_MGMT
int32_t wal_chba_set_channel(oal_net_device_stru *net_dev)
{
    mac_cfg_channel_param_stru *pst_channel_param = NULL;
    wal_msg_write_stru st_write_msg = {0};
    mac_vap_stru *pst_mac_vap = NULL;
    wal_msg_stru *pst_rsp_msg = NULL;
    uint8_t uc_vap_id;
    int32_t l_ret;
    uint32_t err_code;
    /* ��ȡvap id */
    pst_mac_vap = oal_net_dev_priv(net_dev);
    if (pst_mac_vap == NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_chba_set_channel::pst_mac_vap = NULL}");
        return -OAL_EFAIL;
    }
    uc_vap_id = pst_mac_vap->uc_vap_id;

    pst_channel_param = (mac_cfg_channel_param_stru *)(st_write_msg.auc_value);
    pst_channel_param->uc_channel = 36;
    pst_channel_param->en_band = WLAN_BAND_5G;
    pst_channel_param->en_bandwidth = WLAN_BAND_WIDTH_80PLUSPLUS;

    oam_warning_log3(uc_vap_id, OAM_SF_ANY, "{wal_chba_set_channel::uc_channel=%d, en_band=%d, en_bandwidth=%d.}",
        pst_channel_param->uc_channel, pst_channel_param->en_band, pst_channel_param->en_bandwidth);

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CFG80211_SET_CHANNEL, sizeof(mac_cfg_channel_param_stru));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_channel_param_stru),
        (uint8_t *)&st_write_msg, OAL_TRUE, &pst_rsp_msg);
    if (l_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_P2P, "{wal_chba_set_channel::wal_send_cfg_event return err:%d}", l_ret);
        return -OAL_EFAIL;
    }

    /* ��ȡ���صĴ����� */
    err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (err_code != OAL_SUCC) {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY,
            "{wal_chba_set_channel::wal_check_and_release_msg_resp fail return err code:%u}", err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
#endif


oal_int32 wal_cfg80211_set_channel_info(oal_wiphy_stru *pst_wiphy,
                                        oal_net_device_stru *pst_netdev)
{
    mac_cfg_channel_param_stru *pst_channel_param;
    oal_ieee80211_channel *pst_channel;
    wlan_channel_bandwidth_enum_uint8 en_bandwidth;
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_err_code;
    oal_int32 l_channel;
    oal_int32 l_center_freq1;
    oal_int32 l_bandwidth;
    oal_int32 l_bandwidth_value;
    wal_msg_stru *pst_rsp_msg = OAL_PTR_NULL;
    oal_int32 l_ret;
    mac_vap_stru *pst_mac_vap;
    oal_uint8 uc_vap_id;
    oal_int32 l_channel_center_freq;

    /* ��ȡvap id */
    pst_mac_vap = oal_net_dev_priv(pst_netdev);
    uc_vap_id = pst_mac_vap->uc_vap_id;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    l_bandwidth = pst_netdev->ieee80211_ptr->preset_chandef.width;
    l_center_freq1 = (oal_int32)pst_netdev->ieee80211_ptr->preset_chandef.center_freq1;
    pst_channel = pst_netdev->ieee80211_ptr->preset_chandef.chan;
#else
    l_bandwidth = 1;
    l_center_freq1 = (oal_int32)pst_netdev->ieee80211_ptr->channel->center_freq;
    pst_channel = pst_netdev->ieee80211_ptr->channel;
#endif
#else
    l_bandwidth = pst_netdev->ieee80211_ptr->preset_chandef.width;
    l_center_freq1 = (oal_int32)pst_netdev->ieee80211_ptr->preset_chandef.center_freq1;
    pst_channel = pst_netdev->ieee80211_ptr->preset_chandef.chan;
#endif
    l_channel = pst_channel->hw_value;

    oam_warning_log3(uc_vap_id, OAM_SF_ANY,
                     "{wal_cfg80211_set_channel::l_bandwidth = %d, l_center_freq1 = %d, l_channel = %d.}",
                     l_bandwidth, l_center_freq1, l_channel);
    /* �ж��ŵ��ڲ��ڹ������� */
    l_ret = (oal_int32)mac_is_channel_num_valid(pst_channel->band, (oal_uint8)l_channel);
    if (l_ret != OAL_SUCC) {
        oam_warning_log2(uc_vap_id, OAM_SF_ANY,
                         "{wal_cfg80211_set_channel::channel num is invalid. band, ch num [%d] [%d]!}\r\n",
                         pst_channel->band, l_channel);
        return -OAL_EINVAL;
    }
    /* �����ں˴���ֵ��WITP ����ֵת�� */
    l_channel_center_freq = oal_ieee80211_frequency_to_channel(l_center_freq1);
    l_bandwidth_value = wal_cfg80211_convert_width_to_value(l_bandwidth);
    if (l_bandwidth_value == 0) {
        OAM_ERROR_LOG1(uc_vap_id, OAM_SF_ANY,
                       "{wal_cfg80211_set_channel::channel width is invalid, l_bandwidth = %d.\r\n", l_bandwidth);
        return -OAL_EINVAL;
    }

    if (l_bandwidth_value == 80) { /* �жϴ���ֵ�Ƿ����80MHz */
        en_bandwidth = mac_get_bandwith_from_center_freq_seg0((oal_uint8)l_channel, (oal_uint8)l_channel_center_freq);
#if (_PRE_WLAN_CHIP_ASIC != _PRE_WLAN_CHIP_VERSION)
        if ((en_bandwidth == WLAN_BAND_WIDTH_80PLUSPLUS) || (en_bandwidth == WLAN_BAND_WIDTH_80PLUSMINUS)) {
            en_bandwidth = WLAN_BAND_WIDTH_40PLUS;
        } else {
            en_bandwidth = WLAN_BAND_WIDTH_40MINUS;
        }
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY,
                         "{wal_cfg80211_set_channel::FGPA is not support 80M,Set en_bandwidth = %d.\r\n", en_bandwidth);
#endif
    } else if (l_bandwidth_value == 40) { /* �жϴ���ֵ�Ƿ����40MHz */
        switch (l_channel_center_freq - l_channel) {
            case -2: /* �¼�-2��ʾ��20M���ŵ�����Ƶ��ƫ��-2�ŵ� */
                en_bandwidth = WLAN_BAND_WIDTH_40MINUS;
                break;
            case 2: /* �¼�2��ʾ��20M���ŵ�����Ƶ��ƫ��+2�ŵ� */
                en_bandwidth = WLAN_BAND_WIDTH_40PLUS;
                break;
            default:
                en_bandwidth = WLAN_BAND_WIDTH_20M;
                break;
        }
    } else {
        en_bandwidth = WLAN_BAND_WIDTH_20M;
    }

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    pst_channel_param = (mac_cfg_channel_param_stru *)(st_write_msg.auc_value);
    pst_channel_param->uc_channel = (oal_uint8)pst_channel->hw_value;
    pst_channel_param->en_band = pst_channel->band;
    pst_channel_param->en_bandwidth = en_bandwidth;

    oam_warning_log3(uc_vap_id, OAM_SF_ANY,
                     "{wal_cfg80211_set_channel::uc_channel = %d, en_band = %d, en_bandwidth = %d.}",
                     pst_channel_param->uc_channel, pst_channel_param->en_band, pst_channel_param->en_bandwidth);

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CFG80211_SET_CHANNEL, OAL_SIZEOF(mac_cfg_channel_param_stru));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_channel_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);
    if (l_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_P2P,
                         "{wal_cfg80211_set_channel_info::wal_send_cfg_event return err code: [%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (ul_err_code != OAL_SUCC) {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_set_channel::wal_check_and_release_msg_resp fail \
            return err code: [%u].}", ul_err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

uint8_t wal_cfg80211_convert_value_to_vht_width(int32_t l_channel_value)
{
    uint8_t uc_channel_vht_width = WLAN_MIB_VHT_OP_WIDTH_20_40;

    switch (l_channel_value) {
        case 20:  /* 20M���� */
        case 40:  /* 40M���� */
            uc_channel_vht_width = WLAN_MIB_VHT_OP_WIDTH_20_40;
            break;
        case 80:  /* 80M���� */
            uc_channel_vht_width = WLAN_MIB_VHT_OP_WIDTH_80;
            break;
        case 160: /* 160M���� */
            uc_channel_vht_width = WLAN_MIB_VHT_OP_WIDTH_160;
            break;
        default:
            break;
    }

    return uc_channel_vht_width;
}
#ifdef _PRE_WLAN_CHBA_MGMT
 
OAL_STATIC int32_t wal_cfg80211_cfg_chba(oal_wiphy_stru *wiphy, oal_net_device_stru *netdev,
    mac_vap_stru *mac_vap)
{
    mac_chba_param_stru chba_param;
    wal_msg_write_stru write_msg = {0};
    int32_t err;
    if ((hwifi_get_chba_en() == OAL_TRUE) &&
        (oal_strcmp("chba0", netdev->name) == 0)) {
        memset_s(&chba_param, sizeof(mac_chba_param_stru), 0, sizeof(mac_chba_param_stru));
        /* ����mac��ַ */
        oal_set_mac_addr(chba_param.mac_addr, (uint8_t *)oal_netdevice_mac_addr(netdev));
        ((mac_vap_rom_stru *)(mac_vap->_rom))->chba_mode = CHBA_MODE;
        chba_param.init_channel = mac_vap->st_channel;
        oam_warning_log3(0, 0, "CHBA: start ap, init channel is chan no[%d], bandwidth[%d], band[%d].",
            chba_param.init_channel.uc_chan_number, chba_param.init_channel.en_bandwidth,
            chba_param.init_channel.en_band);
        /* ���¼���wal��������� */
        wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_START_CHBA, sizeof(mac_chba_param_stru));
        if (memcpy_s(write_msg.auc_value, sizeof(mac_chba_param_stru),
            &chba_param, sizeof(mac_chba_param_stru)) != EOK) {
            return -OAL_EFAIL;
        }
        err = wal_send_cfg_event(netdev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_chba_param_stru),
            (uint8_t *)&write_msg, OAL_FALSE, NULL);
        if (err != OAL_SUCC) {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_cfg_chba::Fail to config chba vap, error[%d]}", err);
            return -OAL_EFAIL;
        }
    }
    return OAL_SUCC;
}
#endif

oal_int32 wal_cfg80211_start_ap(oal_wiphy_stru *wiphy,
                                oal_net_device_stru *netdev,
                                oal_ap_settings_stru *ap_settings)
{
    mac_beacon_param_stru st_beacon_param; /* beacon info struct */
    wal_msg_write_stru st_write_msg;
    mac_vap_stru *mac_vap;
    oal_beacon_data_stru *pst_beacon_info;
    oal_int32 l_ret;
    oal_uint8 uc_vap_id;
    oal_netdev_priv_stru *pst_netdev_priv = OAL_PTR_NULL;
    oal_int32 l_memcpy_ret;
    oam_info_log0(0, OAM_SF_ANY, "{wal_cfg80211_start_ap::enter here.}");

    /* �����Ϸ��Լ�� */
    if (oal_any_null_ptr3(wiphy, netdev, ap_settings)) {
        oam_error_log3(0, OAM_SF_ANY, "{wal_cfg80211_start_ap::wiphy = %x, netdev = %x, ap_settings = %x!}",
                       (uintptr_t)wiphy, (uintptr_t)netdev, (uintptr_t)ap_settings);
        return -OAL_EINVAL;
    }
    /* ��ȡvap id */
    mac_vap = oal_net_dev_priv(netdev);
    if (mac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_start_ap::mac_vap = %x}", (uintptr_t)mac_vap);
        return -OAL_EINVAL;
    }

    uc_vap_id = mac_vap->uc_vap_id;

    /*****************************************************************************
        1.�����ŵ�
    *****************************************************************************/
    l_ret = wal_cfg80211_set_channel_info(wiphy, netdev);
    if (l_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY,
                         "{wal_cfg80211_start_ap::failed to set channel, return err code[%x]}", l_ret);
        return -OAL_EFAIL;
    }

    /*****************************************************************************
        2.����ssid����Ϣ
    *****************************************************************************/
    if (ap_settings->ssid_len != 0) {
        if (wal_cfg80211_set_ssid(netdev, ap_settings->ssid, ap_settings->ssid_len) != OAL_SUCC) {
            OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY,
                             "{wal_cfg80211_start_ap::fail to send ssid cfg msg, error[%d]}", l_ret);
            return -OAL_EFAIL;
        }
    }

    /*****************************************************************************
        2.2 ����auth mode��Ϣ
    *****************************************************************************/
    l_ret = wal_cfg80211_set_auth_mode(netdev, ap_settings->auth_type);
    if (l_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::fail to send authtpye msg, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    /* ��Ҫ���������Ʒ�ͷ������Ʒ, ��ʽ�汾kernel 5.11.0�Ժ����ӳ�Ա sae_pwe */
    /* �����ƷҪʹ�øó�Ա��kernel�߰汾��ֲ���Ͱ汾 */
    /* �������Ժ�����_PRE_WLAN_WPA3_R3 */
    /* ���ɰ汾��5.10.43,��Ҫ�ó�Ա����Ҫ���� */
#if (defined(_PRE_WLAN_WPA3_R3) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 43)) || \
    (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 11, 0)))
    wal_ioctl_set_sae_pwe(netdev, ap_settings->crypto.sae_pwe);
#endif

    /*****************************************************************************
        3.����beaconʱ������tim period�Լ���ȫ������Ϣ��
    *****************************************************************************/
    /* ��ʼ��beacon interval ��DTIM_PERIOD ���� */
    memset_s(&st_beacon_param, OAL_SIZEOF(mac_beacon_param_stru), 0, OAL_SIZEOF(mac_beacon_param_stru));
    st_beacon_param.l_interval = ap_settings->beacon_interval;
    st_beacon_param.l_dtim_period = ap_settings->dtim_period;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 44))  // 1102 ��Ҫ����ʱ���ں˰汾������
    st_beacon_param.uc_hidden_ssid = (ap_settings->hidden_ssid == 1);

    oam_warning_log3(0, OAM_SF_ANY, "{wal_cfg80211_start_ap::beacon_interval=%d, dtim_period=%d, hidden_ssid=%d!}",
                     ap_settings->beacon_interval, ap_settings->dtim_period, ap_settings->hidden_ssid);
#endif
#ifdef _PRE_WLAN_CHBA_MGMT
    /* ����CHBA VAP���������� */
    if ((hwifi_get_chba_en() == OAL_TRUE) &&
        (oal_strcmp("chba0", netdev->name) == 0)) {
        st_beacon_param.l_interval = MAC_CHBA_DEFALUT_PERIODS_MS;
        st_beacon_param.l_dtim_period = 2;
        st_beacon_param.uc_hidden_ssid = 1;
    }
#endif
    pst_beacon_info = &(ap_settings->beacon);
    l_ret = wal_cfg80211_fill_beacon_param(mac_vap, pst_beacon_info, &st_beacon_param);
    if (l_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::failed to fill beacon param, error[%d]}", l_ret);
        return -OAL_EINVAL;
    }

    /* ��д msg ��Ϣͷ */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CFG80211_CONFIG_BEACON, OAL_SIZEOF(mac_beacon_param_stru));

    /* ��д msg ��Ϣ�� */
    l_memcpy_ret = memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(mac_beacon_param_stru),
        &st_beacon_param, OAL_SIZEOF(mac_beacon_param_stru));
    if (l_memcpy_ret != EOK) {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "wal_cfg80211_start_ap::memcpy fail!");
        return -OAL_EFAIL;
    }

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(netdev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_beacon_param_stru),
                               (oal_uint8 *)&st_write_msg, OAL_FALSE, OAL_PTR_NULL);
    if (l_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::failed to addset beacon, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }
#ifdef _PRE_WLAN_CHBA_MGMT
    /* ����CHBA VAP���������� */
    if (wal_cfg80211_cfg_chba(wiphy, netdev, mac_vap) != OAL_SUCC) {
        return -OAL_EFAIL;
    }
#endif
    /*****************************************************************************
        4.����ap
    *****************************************************************************/
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    l_ret = wal_start_vap(netdev);
#else
    l_ret = wal_netdev_open(netdev);
#endif
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::failed to start ap, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    pst_netdev_priv = (oal_netdev_priv_stru *)oal_net_dev_wireless_priv(netdev);
    if (pst_netdev_priv->uc_napi_enable && (!pst_netdev_priv->uc_state)) {
        oal_napi_enable(&pst_netdev_priv->st_napi);
        pst_netdev_priv->uc_state = 1;
    }

    return OAL_SUCC;
}


oal_int32 wal_cfg80211_stop_ap(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev)
{
    wal_msg_write_stru st_write_msg = {0};
    mac_vap_stru *pst_mac_vap;
    oal_int32 l_ret;
    oal_uint8 uc_vap_id;
    oal_netdev_priv_stru *pst_netdev_priv;
#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8 en_p2p_mode;
    oal_wireless_dev_stru *pst_wdev;
#endif

    /* �����Ϸ��Լ�� */
    if ((pst_wiphy == OAL_PTR_NULL) || (pst_netdev == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap::pst_wiphy = %x, pst_netdev = %x!}",
                       (uintptr_t)pst_wiphy, (uintptr_t)pst_netdev);
        return -OAL_EINVAL;
    }

#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap::dfr_process_status[%d]!}",
                         g_st_dfr_info.bit_device_reset_process_flag);
        return OAL_SUCC;
    }

#endif
#ifdef _PRE_WLAN_CHBA_MGMT
    /* ����CHBA VAP���������� */
    if ((hwifi_get_chba_en() == OAL_TRUE) &&
        (oal_strcmp("chba0", pst_netdev->name) == 0)) {
        oam_warning_log0(0, 0, "CHBA: enter stop ap.");
        return wal_netdev_stop_ap(pst_netdev);
    }
#endif
    /* ��ȡvap id */
    pst_mac_vap = oal_net_dev_priv(pst_netdev);
    if (pst_mac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap::can't get mac vap from netdevice priv data!}");
        return -OAL_EINVAL;
    }

    uc_vap_id = pst_mac_vap->uc_vap_id;

    /* �ж��Ƿ�Ϊ��apģʽ */
    if (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_stop_ap::vap is not in ap mode!}");
        return -OAL_EINVAL;
    }

    /* ���netdev����running״̬������Ҫdown */
    if ((oal_netdevice_flags(pst_netdev) & OAL_IFF_RUNNING) == 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap::vap is already down!}\r\n");
        return OAL_SUCC;
    }

    /*****************************************************************************
        ������Ϣ��ͣ��ap
    *****************************************************************************/
    /* ��д��Ϣ */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_DOWN_VAP, OAL_SIZEOF(mac_cfg_start_vap_param_stru));

#ifdef _PRE_WLAN_FEATURE_P2P
    pst_wdev = pst_netdev->ieee80211_ptr;
    en_p2p_mode = wal_wireless_iftype_to_mac_p2p_mode(pst_wdev->iftype);
    if (en_p2p_mode == WLAN_P2P_BUTT) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap::wal_wireless_iftype_to_mac_p2p_mode return BUFF}\r\n");
        return -OAL_EINVAL;
    }
    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap::en_p2p_mode=%u}\r\n", en_p2p_mode);

    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode = en_p2p_mode;
#endif
    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_netdev;
    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_netdev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_start_vap_param_stru),
        (oal_uint8 *)&st_write_msg, OAL_FALSE, OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_stop_ap::failed to stop ap, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    pst_netdev_priv = (oal_netdev_priv_stru *)oal_net_dev_wireless_priv(pst_netdev);
    if (pst_netdev_priv->uc_napi_enable) {
        oal_netbuf_queue_purge(&pst_netdev_priv->st_rx_netbuf_queue);
        oal_napi_disable(&pst_netdev_priv->st_napi);
        pst_netdev_priv->uc_state = 0;
    }
    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_cfg80211_change_bss(oal_wiphy_stru *pst_wiphy,
                                             oal_net_device_stru *pst_netdev,
                                             oal_bss_parameters *pst_bss_params)
{
    return OAL_SUCC;
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44)) || !defined(_PRE_WLAN_EXPORT)

oal_void wal_cfg80211_print_sched_scan_req_info(oal_cfg80211_sched_scan_request_stru *pst_request)
{
    oal_int8 ac_tmp_buff[200];
    oal_int32 l_loop = 0;

    /* ��ӡ�������� */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
    oam_warning_log3(0, OAM_SF_SCAN, "wal_cfg80211_print_sched_scan_req_info::channels[%d],flags[%d],rssi_thold[%d]",
                     pst_request->n_channels, pst_request->flags, pst_request->min_rssi_thold);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    oam_warning_log4(0, OAM_SF_SCAN,
                     "wal_cfg80211_print_sched_scan_req_info::channels[%d],interval[%d]ms,flags[%d],rssi_thold[%d]",
                     pst_request->n_channels, pst_request->interval,
                     pst_request->flags,
                     pst_request->min_rssi_thold);
#else
    oam_warning_log4(0, OAM_SF_SCAN,
                     "wal_cfg80211_print_sched_scan_req_info::channels[%d],interval[%d]ms,flags[%d],rssi_thold[%d]",
                     pst_request->n_channels, pst_request->interval,
                     pst_request->flags,
                     pst_request->rssi_thold);
#endif

    /* ��ӡssid���ϵ���Ϣ */
    for (l_loop = 0; l_loop < pst_request->n_match_sets; l_loop++) {
        memset_s(ac_tmp_buff, OAL_SIZEOF(ac_tmp_buff), 0, OAL_SIZEOF(ac_tmp_buff));
        if (snprintf_s(ac_tmp_buff, OAL_SIZEOF(ac_tmp_buff), OAL_SIZEOF(ac_tmp_buff) - 1,
            "mactch_sets[%d] info, ssid_len[%d], ssid: %.32s.\n",
            l_loop, pst_request->match_sets[l_loop].ssid.ssid_len, pst_request->match_sets[l_loop].ssid.ssid) != EOK) {
                break;
            }
        oam_print(ac_tmp_buff);
    }

    for (l_loop = 0; l_loop < pst_request->n_ssids; l_loop++) {
        memset_s(ac_tmp_buff, OAL_SIZEOF(ac_tmp_buff), 0, OAL_SIZEOF(ac_tmp_buff));
        if (snprintf_s(ac_tmp_buff, OAL_SIZEOF(ac_tmp_buff), OAL_SIZEOF(ac_tmp_buff) - 1,
            "ssids[%d] info, ssid_len[%d], ssid: %.32s.\n",
            l_loop, pst_request->ssids[l_loop].ssid_len, pst_request->ssids[l_loop].ssid) != EOK) {
                break;
            }
        oam_print(ac_tmp_buff);
    }

    return;
}
#endif

OAL_STATIC oal_bool_enum_uint8 wal_pno_scan_with_assigned_ssid(oal_cfg80211_ssid_stru *pst_ssid,
                                                               oal_cfg80211_ssid_stru *pst_ssid_list,
                                                               oal_int32 l_count)
{
    oal_int32 l_loop;

    if ((pst_ssid == OAL_PTR_NULL) || (pst_ssid_list == OAL_PTR_NULL)) {
        return OAL_FALSE;
    }

    for (l_loop = 0; l_loop < l_count; l_loop++) {
        if ((pst_ssid->ssid_len == pst_ssid_list[l_loop].ssid_len) &&
            (oal_memcmp(pst_ssid->ssid, pst_ssid_list[l_loop].ssid, pst_ssid->ssid_len) == 0)) {
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}


oal_int32 wal_cfg80211_sched_scan_start(oal_wiphy_stru *pst_wiphy,
                                        oal_net_device_stru *pst_netdev,
                                        oal_cfg80211_sched_scan_request_stru *pst_request)
{
    hmac_device_stru *pst_hmac_device;
    hmac_scan_stru *pst_scan_mgmt;
    mac_vap_stru *pst_mac_vap;
    oal_cfg80211_ssid_stru *pst_ssid_tmp;
    oal_cfg80211_ssid_stru *pst_scan_ssid_list;
    mac_pno_scan_stru st_pno_scan_info;
    oal_int32 l_loop = 0;
    oal_int32 l_ret;

    if (g_hitalk_status) {
        return -OAL_EINVAL;
    }

    /* �����Ϸ��Լ�� */
    if ((pst_wiphy == OAL_PTR_NULL) || (pst_netdev == OAL_PTR_NULL) || (pst_request == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, 0, "{wal_cfg80211_sched_scan_start::input param null!}");
        return -OAL_EINVAL;
    }

#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_sched_scan_start:: dfr_process_status[%d]!}",
                         g_st_dfr_info.bit_device_reset_process_flag);
        return -OAL_EFAIL;
    }
#endif  // #ifdef _PRE_WLAN_FEATURE_DFR

    /* ͨ��net_device �ҵ���Ӧ��mac_device_stru �ṹ */
    pst_mac_vap = oal_net_dev_priv(pst_netdev);
    if (pst_mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_sched_scan_start:: pst_mac_vap is null!}");
        return -OAL_EINVAL;
    }

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_sched_scan_start:: pst_mac_device is null!}");
        return -OAL_EINVAL;
    }

    pst_scan_mgmt = &(pst_hmac_device->st_scan_mgmt);

    /* �����ǰ�豸����ɨ��״̬��abort��ǰɨ�� */
    if (pst_scan_mgmt->pst_request != OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_sched_scan_start:: device is busy, stop current scan!}");

        wal_force_scan_complete(pst_netdev, OAL_TRUE);
    }

    /* ����ں��·�����Ҫƥ���ssid���ϵĸ����Ƿ�Ϸ� */
    if (pst_request->n_match_sets <= 0) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{wal_cfg80211_sched_scan_start::match_sets = %d!}",
                         pst_request->n_match_sets);
        return -OAL_EINVAL;
    }

    /* ��ʼ��pnoɨ��Ľṹ����Ϣ */
    memset_s(&st_pno_scan_info, OAL_SIZEOF(st_pno_scan_info), 0, OAL_SIZEOF(st_pno_scan_info));

    /* ���ں��·���ƥ���ssid���ϸ��Ƶ����� */
    pst_scan_ssid_list = OAL_PTR_NULL;
    if (pst_request->n_ssids > 0) {
        pst_scan_ssid_list = pst_request->ssids;
    }
    for (l_loop = 0; l_loop < oal_min(pst_request->n_match_sets, MAX_PNO_SSID_COUNT); l_loop++) {
        pst_ssid_tmp = &(pst_request->match_sets[l_loop].ssid);
        if (pst_ssid_tmp->ssid_len >= WLAN_SSID_MAX_LEN) {
            OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{wal_cfg80211_sched_scan_start:: wrong ssid_len[%d]!}",
                             pst_ssid_tmp->ssid_len);
            continue;
        }
        l_ret = memcpy_s(st_pno_scan_info.ast_match_ssid_set[l_loop].auc_ssid, WLAN_SSID_MAX_LEN,
                         pst_ssid_tmp->ssid, pst_ssid_tmp->ssid_len);
        if (l_ret != EOK) {
            OAM_ERROR_LOG0(0, OAM_SF_SCAN, "wal_cfg80211_sched_scan_start::memcpy fail!");
            continue;
        }
        st_pno_scan_info.ast_match_ssid_set[l_loop].auc_ssid[pst_ssid_tmp->ssid_len] = '\0';
        st_pno_scan_info.ast_match_ssid_set[l_loop].en_scan_ssid =
            wal_pno_scan_with_assigned_ssid(pst_ssid_tmp, pst_scan_ssid_list, pst_request->n_ssids);
        st_pno_scan_info.l_ssid_count++;
    }

    /* ����������ֵ */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    st_pno_scan_info.l_rssi_thold = pst_request->min_rssi_thold;
#else
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)) */
    st_pno_scan_info.ul_pno_scan_interval = PNO_SCHED_SCAN_INTERVAL; /* �����Լ�����Ϊ60s */
    st_pno_scan_info.uc_pno_scan_repeat = MAX_PNO_REPEAT_TIMES;

    /* ���浱ǰ��PNO����ɨ������ָ�� */
    pst_scan_mgmt->pst_sched_scan_req = pst_request;
    pst_scan_mgmt->en_sched_scan_complete = OAL_FALSE;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    /* ά���ӡ�ϲ��·��ĵ���ɨ�����������Ϣ */
    wal_cfg80211_print_sched_scan_req_info(pst_request);
    OAM_WARNING_LOG1(0, 0, "wal_cfg80211_sched_scan_start::flags[%d]", pst_request->flags);
    wal_cfg80211_set_scan_random_mac(pst_hmac_device, pst_request->flags,
        (uint8_t *)(pst_request->mac_addr), (uint8_t *)(pst_request->mac_addr_mask));
#endif
    /* �·�pnoɨ������hmac */
    l_ret = (oal_int32)wal_cfg80211_start_sched_scan(pst_netdev, &st_pno_scan_info);
    if (l_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{wal_cfg80211_sched_scan_start::start_sched_scan fail[%d]!}", l_ret);
        return -OAL_EBUSY;
    }

    return OAL_SUCC;
}


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
oal_int32 wal_cfg80211_sched_scan_stop(oal_wiphy_stru *pst_wiphy,
                                       oal_net_device_stru *pst_netdev,
                                       oal_uint64 ul_reqid)

#else
oal_int32 wal_cfg80211_sched_scan_stop(oal_wiphy_stru *pst_wiphy,
                                       oal_net_device_stru *pst_netdev)
#endif
{
    if (g_hitalk_status) {
        return -OAL_EINVAL;
    }

    /* �����Ϸ��Լ�� */
    if ((pst_wiphy == OAL_PTR_NULL) || (pst_netdev == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_sched_scan_stop::input param pointer is null!}");
        return -OAL_EINVAL;
    }

#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_sched_scan_stop:: dfr_process_status[%d]!}",
                         g_st_dfr_info.bit_device_reset_process_flag);
        return -OAL_EFAIL;
    }
#endif  // #ifdef _PRE_WLAN_FEATURE_DFR

    return wal_stop_sched_scan(pst_netdev);
}

#endif
#ifdef _PRE_WLAN_FEATURE_P2P
static oal_int32 wal_process_virtual_intf_type(oal_net_device_stru *net_dev,
                                               enum nl80211_iftype type,
                                               wlan_p2p_mode_enum_uint8 *p2p_mode,
                                               wlan_vap_mode_enum_uint8 *vap_mode,
                                               oal_bool_enum_uint8 *ret_out)
{
    oal_int32 ret;

    switch (type) {
        case NL80211_IFTYPE_MONITOR:
        case NL80211_IFTYPE_WDS:
        case NL80211_IFTYPE_MESH_POINT:
        case NL80211_IFTYPE_ADHOC:
            OAM_ERROR_LOG1(0, OAM_SF_CFG,
                           "{wal_cfg80211_change_virtual_intf::currently we do not support this type[%d]}", type);
            *ret_out = OAL_TRUE;
            return -OAL_EINVAL;

        case NL80211_IFTYPE_STATION:
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
            if (net_dev->ieee80211_ptr->iftype == NL80211_IFTYPE_AP) {
                /* �µ�APUT���л�netdev״̬Ϊstation */
                ret = wal_netdev_stop_ap(net_dev);
                *ret_out = OAL_TRUE;
                return ret;
            }
#endif
            net_dev->ieee80211_ptr->iftype = type;  // P2P BUG P2P_DEVICE ��ǰ����������Ҫͨ��wpa_supplicant ����
            oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf::change to station}\r\n");
            *ret_out = OAL_TRUE;
            return OAL_SUCC;
        case NL80211_IFTYPE_P2P_CLIENT:
            *vap_mode = WLAN_VAP_MODE_BSS_STA;
            *p2p_mode = WLAN_P2P_CL_MODE;
            break;
        case NL80211_IFTYPE_AP:
        case NL80211_IFTYPE_AP_VLAN:
            *vap_mode = WLAN_VAP_MODE_BSS_AP;
            *p2p_mode = WLAN_LEGACY_VAP_MODE;
            break;
        case NL80211_IFTYPE_P2P_GO:
            *vap_mode = WLAN_VAP_MODE_BSS_AP;
            *p2p_mode = WLAN_P2P_GO_MODE;
            break;
        default:
            OAM_ERROR_LOG1(0, OAM_SF_CFG,
                           "{wal_cfg80211_change_virtual_intf::currently we do not support this type[%d]}", type);
            *ret_out = OAL_TRUE;
            return -OAL_EINVAL;
    }
    *ret_out = OAL_FALSE;
    return OAL_SUCC;
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))
oal_int32 wal_cfg80211_change_virtual_intf(oal_wiphy_stru *pst_wiphy,
                                           oal_net_device_stru *pst_net_dev,
                                           enum nl80211_iftype en_type,
                                           oal_vif_params_stru *pst_params)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
oal_int32 wal_cfg80211_change_virtual_intf(oal_wiphy_stru *pst_wiphy,
                                           oal_net_device_stru *pst_net_dev,
                                           enum nl80211_iftype en_type,
                                           oal_uint32 *pul_flags,
                                           oal_vif_params_stru *pst_params)
#else
oal_int32 wal_cfg80211_change_virtual_intf(oal_wiphy_stru *pst_wiphy,
                                           oal_int32 l_ifindex,
                                           enum nl80211_iftype en_type,
                                           oal_uint32 *pul_flags,
                                           oal_vif_params_stru *pst_params)
#endif
{
#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8 en_p2p_mode;
    wlan_vap_mode_enum_uint8 en_vap_mode;
    mac_cfg_del_vap_param_stru st_del_vap_param;
    mac_cfg_add_vap_param_stru st_add_vap_param;
    mac_vap_stru *pst_mac_vap;
    oal_int32 l_ret;
    oal_uint32 ul_ret;
    oal_bool_enum_uint8 ret_out = OAL_FALSE;

    /* 1.1 ��μ�� */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
    if (pst_net_dev == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf::pst_dev is null!}\r\n");
        return -OAL_EINVAL;
    }
#else
    oal_net_device_stru *pst_net_dev;
    pst_net_dev = oal_dev_get_by_index(l_ifindex);
    if (pst_net_dev == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf::pst_dev is null!}\r\n");
        return -OAL_EINVAL;
    }
    oal_dev_put(pst_net_dev); /* ����oal_dev_get_by_index�󣬱������oal_dev_putʹnet_dev�����ü�����һ */
#endif
#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_change_virtual_intf::dfr_process_status[%d]!}",
                         g_st_dfr_info.bit_device_reset_process_flag);
        return OAL_SUCC;
    }

#endif  // #ifdef _PRE_WLAN_FEATURE_DFR

    if ((pst_wiphy == OAL_PTR_NULL) || (pst_params == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf::pst_wiphy or pul_flag or pst_params is null, \
            error %x, %x, %d!}\r\n", (uintptr_t)pst_wiphy, (uintptr_t)pst_params);
        return -OAL_EINVAL;
    }

    /* ���VAP ��ǰģʽ��Ŀ��ģʽ�Ƿ���ͬ�������ͬ��ֱ�ӷ��� */
    if (pst_net_dev->ieee80211_ptr->iftype == en_type) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf::same iftype[%d],do not need change !}\r\n",
                         en_type);
        return OAL_SUCC;
    }

    OAL_IO_PRINT("wal_cfg80211_change_virtual_intf,dev_name is:%.16s\n", pst_net_dev->name);
    oam_warning_log2(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf::from [%d] to [%d]}\r\n",
                     (pst_net_dev->ieee80211_ptr->iftype), en_type);

    l_ret = wal_process_virtual_intf_type(pst_net_dev, en_type, &en_p2p_mode, &en_vap_mode, &ret_out);
    if (ret_out == OAL_TRUE) {
        return l_ret;
    }

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    if (en_type == NL80211_IFTYPE_AP) {
        l_ret = wal_setup_ap(pst_net_dev);
        return l_ret;
    }
#endif
#ifdef _PRE_WLAN_CHBA_MGMT
    /* Supplicant��create chba0֮ǰ����set modeΪGO */
    if ((hwifi_get_chba_en() == OAL_TRUE) &&
        (oal_strcmp("chba0", pst_net_dev->name) == 0) && en_type == NL80211_IFTYPE_P2P_GO) {
        oam_warning_log0(0, 0, "CHBA: set mode to GO.");
        if (wal_setup_chba(pst_net_dev) != OAL_SUCC) {
            return -OAL_EINVAL;
        }
        pst_net_dev->ieee80211_ptr->iftype = en_type;
        return OAL_SUCC;
    }
#endif
    /* �豸ΪP2P �豸����Ҫ����change virtual interface */
    pst_mac_vap = oal_net_dev_priv(pst_net_dev);
    if (pst_mac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG,
                       "{wal_cfg80211_change_virtual_intf::can't get mac vap from netdevice priv data.}\r\n");
        return -OAL_EINVAL;
    }

    if (IS_LEGACY_VAP(pst_mac_vap)) {
        pst_net_dev->ieee80211_ptr->iftype = en_type;
        return OAL_SUCC;
    }

    if ((oal_strcmp(wal_get_p2p_name(), pst_net_dev->name)) == 0) {
        /* ����쳣�����,wpa_supplicant�·�p2p0�豸�л���p2p go/cliģʽ����fastboot������ */
        oam_warning_log0(0, OAM_SF_CFG, "wal_cfg80211_change_virtual_intf:p2p0 netdev can not change to P2P CLI/GO");
        return -OAL_EINVAL;
    }

    /* �����ǰģʽ��Ŀ��ģʽ��ͬ������Ҫ:
       1. ֹͣ VAP
       2. ɾ�� VAP
       3. ���´�����ӦģʽVAP
       4. ����VAP
 */
    /* ֹͣVAP */
    wal_netdev_stop(pst_net_dev);

    memset_s(&st_del_vap_param, OAL_SIZEOF(st_del_vap_param), 0, OAL_SIZEOF(st_del_vap_param));
    /* ɾ��VAP */
    st_del_vap_param.pst_net_dev = pst_net_dev;
    /* �豸p2p ģʽ��Ҫ��net_device �л�ȡ */
    st_del_vap_param.en_p2p_mode = wal_wireless_iftype_to_mac_p2p_mode(pst_net_dev->ieee80211_ptr->iftype);
    if (wal_cfg80211_del_vap(&st_del_vap_param)) {
        return -OAL_EFAIL;
    }

    memset_s(&st_add_vap_param, OAL_SIZEOF(st_add_vap_param), 0, OAL_SIZEOF(st_add_vap_param));
    /* ���´�����ӦģʽVAP */
    st_add_vap_param.pst_net_dev = pst_net_dev;
    st_add_vap_param.en_vap_mode = en_vap_mode;
    st_add_vap_param.en_p2p_mode = en_p2p_mode;
    ul_ret = wal_cfg80211_add_vap(&st_add_vap_param);

    /* ����VAP */
    wal_netdev_open(pst_net_dev);
#endif /* _PRE_WLAN_FEATURE_P2P */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
    pst_net_dev->ieee80211_ptr->iftype = en_type;
#endif
    return OAL_SUCC;
}


oal_int32 wal_cfg80211_add_station(oal_wiphy_stru *pst_wiphy,
                                   oal_net_device_stru *pst_dev,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
                                   const oal_uint8 *puc_mac,
#else
                                   oal_uint8 *puc_mac,
#endif
                                   oal_station_parameters_stru *pst_sta_parms)
{
    return OAL_SUCC;
}


oal_int32 wal_cfg80211_del_station(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_dev,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    struct station_del_parameters *params
#else
    oal_uint8 *puc_mac
#endif
)
{
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    mac_cfg_kick_user_param_stru st_kick_user_param;
    oal_int32 int_user_count_ok = 0;
    oal_int32 int_user_count_fail = 0;
    oal_int32 uint_ret = OAL_FAIL;
    oal_uint8 auc_mac_boardcast[OAL_MAC_ADDR_LEN];
    oal_uint16 us_reason_code = MAC_INACTIVITY;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    oal_uint8 *puc_mac;
    if (params == OAL_PTR_NULL) {
        return -OAL_EFAUL;
    }
    puc_mac = (oal_uint8 *)params->mac;
    us_reason_code = params->reason_code;
#endif

    if ((pst_wiphy == OAL_PTR_NULL) || (pst_dev == OAL_PTR_NULL)) {
        return -OAL_EFAUL;
    }
#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_del_station::dfr_process_status[%d]!}",
                         g_st_dfr_info.bit_device_reset_process_flag);
        return OAL_SUCC;
    }

#endif  // #ifdef _PRE_WLAN_FEATURE_DFR

    pst_mac_vap = oal_net_dev_priv(pst_dev);
    if (pst_mac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_del_station::can't get mac vap from netdevice priv data!}\r\n");
        return -OAL_EFAUL;
    }

    /* �ж��Ƿ���APģʽ */
    if (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{wal_cfg80211_del_station::WLAN_VAP_MODE_BSS_AP != vap_mode[%d]!}", pst_mac_vap->en_vap_mode);
        return -OAL_EINVAL;
    }

    if (puc_mac == OAL_PTR_NULL) {
        memset_s(auc_mac_boardcast, OAL_MAC_ADDR_LEN, 0xff, OAL_MAC_ADDR_LEN);
        puc_mac = auc_mac_boardcast;
    }

    st_kick_user_param.us_reason_code = us_reason_code;
    if (memcpy_s(st_kick_user_param.auc_mac_addr, WLAN_MAC_ADDR_LEN, puc_mac, OAL_MAC_ADDR_LEN) != EOK) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_cfg80211_del_station::memcpy fail!");
        return -OAL_EINVAL;
    }
    uint_ret = wal_cfg80211_start_disconnect(pst_dev, &st_kick_user_param);
    if (uint_ret != OAL_SUCC) {
        /* ����ɾ����ʱ������û��Ѿ�ɾ������ʱ�ٽ����û����ң��᷵�ش������ERROR��ӡ���޸�Ϊwarning */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_cfg80211_del_station::hmac_config_kick_user fail[%d]!}\r\n", uint_ret);
        int_user_count_fail++;
    } else {
        int_user_count_ok++;
    }

    if (int_user_count_fail > 0) {
        oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                      "{wal_cfg80211_del_station::%d user is failed to be deleted!}\r\n", int_user_count_fail);
        return -OAL_EINVAL;
    }

    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_del_station::%d user is deleted!}\r\n",
                  int_user_count_ok);
    return OAL_SUCC;
}


oal_int32 wal_cfg80211_change_station(oal_wiphy_stru *pst_wiphy,
                                      oal_net_device_stru *pst_dev,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
                                      const oal_uint8 *puc_mac,
#else
                                      oal_uint8 *puc_mac,
#endif
                                      oal_station_parameters_stru *pst_sta_parms)
{
    return OAL_SUCC;
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))

OAL_STATIC oal_void wal_fill_station_rate_info(oal_station_info_stru *pst_sta_info,
    oal_station_info_stru *pst_stats)
{
    pst_sta_info->filled |= BIT(NL80211_STA_INFO_TX_BITRATE);
    pst_sta_info->txrate.legacy = (oal_uint16)(pst_stats->txrate.legacy * 10); /* �ں��е�λΪ100kbps */
    pst_sta_info->txrate.flags = pst_stats->txrate.flags;
    pst_sta_info->txrate.mcs = pst_stats->txrate.mcs;
    pst_sta_info->txrate.nss = pst_stats->txrate.nss;
    pst_sta_info->txrate.bw = pst_stats->txrate.bw;
    /* �ϱ��������� */
    pst_sta_info->filled |= BIT(NL80211_STA_INFO_RX_BITRATE);
    pst_sta_info->rxrate.legacy = pst_stats->rxrate.legacy;
}
#endif

#define QUERY_STATION_INFO_TIME (5 * OAL_TIME_HZ)

OAL_STATIC oal_void wal_cfg80211_fill_station_info(oal_station_info_stru *pst_sta_info,
                                                   oal_station_info_stru *pst_stats)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    /* ����linux 4.1.�޸�ΪBIT(NL80211_STA_INFO_SIGNAL)��. */
    pst_sta_info->filled |= BIT(NL80211_STA_INFO_SIGNAL);

    pst_sta_info->signal = pst_stats->signal;

    pst_sta_info->filled |= BIT(NL80211_STA_INFO_RX_PACKETS);
    pst_sta_info->filled |= BIT(NL80211_STA_INFO_TX_PACKETS);

    pst_sta_info->rx_packets = pst_stats->rx_packets;
    pst_sta_info->tx_packets = pst_stats->tx_packets;

    pst_sta_info->filled |= BIT(NL80211_STA_INFO_RX_BYTES);
    pst_sta_info->filled |= BIT(NL80211_STA_INFO_TX_BYTES);
    pst_sta_info->rx_bytes = pst_stats->rx_bytes;
    pst_sta_info->tx_bytes = pst_stats->tx_bytes;

    pst_sta_info->filled |= BIT(NL80211_STA_INFO_TX_RETRIES);
    pst_sta_info->filled |= BIT(NL80211_STA_INFO_TX_FAILED);
    pst_sta_info->filled |= BIT(NL80211_STA_INFO_RX_DROP_MISC);

    pst_sta_info->tx_retries = pst_stats->tx_retries;
    pst_sta_info->tx_failed = pst_stats->tx_failed;
    pst_sta_info->rx_dropped_misc = pst_stats->rx_dropped_misc;

    wal_fill_station_rate_info(pst_sta_info, pst_stats);
#else
    pst_sta_info->filled |= STATION_INFO_SIGNAL;

    pst_sta_info->signal = pst_stats->signal;

    pst_sta_info->filled |= STATION_INFO_RX_PACKETS;
    pst_sta_info->filled |= STATION_INFO_TX_PACKETS;

    pst_sta_info->rx_packets = pst_stats->rx_packets;
    pst_sta_info->tx_packets = pst_stats->tx_packets;

    pst_sta_info->filled |= STATION_INFO_RX_BYTES;
    pst_sta_info->filled |= STATION_INFO_TX_BYTES;
    pst_sta_info->rx_bytes = pst_stats->rx_bytes;
    pst_sta_info->tx_bytes = pst_stats->tx_bytes;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
    pst_sta_info->filled |= STATION_INFO_TX_RETRIES;
    pst_sta_info->filled |= STATION_INFO_TX_FAILED;
    pst_sta_info->filled |= STATION_INFO_RX_DROP_MISC;

    pst_sta_info->tx_retries = pst_stats->tx_retries;
    pst_sta_info->tx_failed = pst_stats->tx_failed;
    pst_sta_info->rx_dropped_misc = pst_stats->rx_dropped_misc;
#endif

    pst_sta_info->filled |= STATION_INFO_TX_BITRATE;
    pst_sta_info->txrate.legacy = (oal_uint16)(pst_stats->txrate.legacy * 10); /* �ں��е�λΪ100kbps */
    pst_sta_info->txrate.flags = pst_stats->txrate.flags;
    pst_sta_info->txrate.mcs = pst_stats->txrate.mcs;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
    pst_sta_info->txrate.nss = pst_stats->txrate.nss;
#endif
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)) */
}


oal_uint8 wal_cfg80211_get_station_filter(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_mac)
{
    hmac_user_stru *pst_hmac_user;
    oal_uint32 ul_current_time = (oal_uint32)oal_time_get_stamp_ms();
    oal_uint32 ul_runtime;
    oal_uint32 ul_get_station_threshold;
#ifndef WIN32
#ifdef _PRE_WLAN_FEATURE_VOWIFI
    mac_device_stru *pst_mac_dev;
#endif
#endif

    pst_hmac_user = mac_vap_get_hmac_user_by_addr(pst_mac_vap, puc_mac, WLAN_MAC_ADDR_LEN);
    if (pst_hmac_user == OAL_PTR_NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_get_station_filter::user is null.}");
        return OAL_FALSE;
    }
#ifndef WIN32
#ifdef _PRE_WLAN_FEATURE_VOWIFI
    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_dev == OAL_PTR_NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_get_station_filter::dev is null.}");
        return OAL_FALSE;
    }
    if (IS_LEGACY_STA(pst_mac_vap) &&
        ((pst_mac_vap->pst_vowifi_cfg_param != OAL_PTR_NULL) &&
         (pst_mac_vap->pst_vowifi_cfg_param->en_vowifi_mode == VOWIFI_DISABLE_REPORT))) { /* vowifi����ʹ��ʱ */
        ul_get_station_threshold = WAL_VOWIFI_GET_STATION_THRESHOLD;
    } else
#endif
#endif
    {
        ul_get_station_threshold = WAL_GET_STATION_THRESHOLD;
    }

#ifdef _PRE_WLAN_FEATURE_ROAM
    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_ROAMING) {
        return OAL_TRUE;
    }
#endif

    ul_runtime = (oal_uint32)oal_time_get_runtime(pst_hmac_user->ul_rssi_last_timestamp, ul_current_time);
    if (ul_get_station_threshold > ul_runtime) {
        return OAL_FALSE;
    }

    pst_hmac_user->ul_rssi_last_timestamp = ul_current_time;
    return OAL_TRUE;
}
OAL_STATIC oal_void wal_cfg80211_print_rssi_info(hmac_vap_stru *pst_hmac_vap, oal_uint8 uc_vap_id)
{
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    if (board_get_wifi_support_tas()) {
        oam_warning_log2(uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_get_station::rssi %d,TAS ant[%d]}",
                         pst_hmac_vap->station_info.signal, board_get_wifi_tas_gpio_state());
    } else
#endif
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "wal_cfg80211_get_station:rssi %d",
                         pst_hmac_vap->station_info.signal);
    }
}

oal_int32 wal_cfg80211_get_station(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_dev,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
                                   const oal_uint8 *puc_mac,
#else
                                   oal_uint8 *puc_mac,
#endif
                                   oal_station_info_stru *pst_sta_info)
{
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    dmac_query_request_event st_dmac_query_request_event;
    dmac_query_station_info_request_event *pst_query_station_info = OAL_PTR_NULL;
    wal_msg_write_stru st_write_msg = {0};
    oal_int i_leftime;
    oal_int32 l_ret;
    oal_uint8 uc_vap_id;

    if ((pst_wiphy == OAL_PTR_NULL) || (pst_dev == OAL_PTR_NULL) ||
        (puc_mac == OAL_PTR_NULL) || (pst_sta_info == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_get_station::pst_wiphy,pst_dev,puc_mac,pst_sta_info is null!}");
        return -OAL_EINVAL;
    }

    pst_mac_vap = oal_net_dev_priv(pst_dev);
    if (pst_mac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_get_station::oal_net_dev_priv, return null!}");
        return -OAL_EINVAL;
    }

    uc_vap_id = pst_mac_vap->uc_vap_id;

    pst_query_station_info = (dmac_query_station_info_request_event *)&st_dmac_query_request_event;
    pst_query_station_info->query_event = OAL_QUERY_STATION_INFO_EVENT;
    oal_set_mac_addr(pst_query_station_info->auc_query_sta_addr, (oal_uint8 *)puc_mac);

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_get_station::mac_res_get_hmac_vap fail.vap_id[%u]}", uc_vap_id);
        return -OAL_EINVAL;
    }

    /* �̶�ʱ����������һ��RSSI */
    if (wal_cfg80211_get_station_filter(&pst_hmac_vap->st_vap_base_info, (oal_uint8 *)puc_mac) == OAL_FALSE) {
        wal_cfg80211_fill_station_info(pst_sta_info, &pst_hmac_vap->station_info);
#ifdef CONFIG_HW_GET_EXT_SIG
        pst_sta_info->filled |= BIT(NL80211_STA_INFO_CNAHLOAD);
        pst_sta_info->chload = (oal_int32)pst_hmac_vap->st_station_info_extend.us_chload;
#endif
        return OAL_SUCC;
    }

    pst_hmac_vap->station_info_query_completed_flag = OAL_FALSE;

    /********************************************************************************
        ���¼���wal�㴦�� �����ڵ͹�����Ҫ�����⴦�����������²����¼���ֱ����ʱ��
        �͹��Ļ��ڽ���beacon֡��ʱ�������ϱ���Ϣ��
    ********************************************************************************/
    /* 3.1 ��д msg ��Ϣͷ */
    st_write_msg.en_wid = WLAN_CFGID_QUERY_STATION_STATS;
    st_write_msg.us_len = OAL_SIZEOF(st_dmac_query_request_event);

    /* 3.2 ��д msg ��Ϣ�� */
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(dmac_query_station_info_request_event),
                 pst_query_station_info, OAL_SIZEOF(dmac_query_station_info_request_event)) != EOK) {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "wal_cfg80211_get_station::memcpy fail!");
        return -OAL_EFAIL;
    }

    l_ret = wal_send_cfg_event(pst_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(dmac_query_station_info_request_event),
                               (oal_uint8 *)&st_write_msg, OAL_FALSE, OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_get_station::wal_send_cfg_event fail, l_ret=%d}", l_ret);
        return -OAL_EFAIL;
    }
    /*lint -e730*/ /* info, boolean argument to function */
    i_leftime = oal_wait_event_interruptible_timeout(pst_hmac_vap->query_wait_q,
                                                     (pst_hmac_vap->station_info_query_completed_flag == OAL_TRUE),
                                                     QUERY_STATION_INFO_TIME);
    /*lint +e730*/
    if (i_leftime == 0) {
        /* ��ʱ��û���ϱ�ɨ����� */
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_get_station::query info wait for %ld ms timeout!}",
                         ((QUERY_STATION_INFO_TIME * 1000) / OAL_TIME_HZ)); /* 1s����1000ms */
        return -OAL_EINVAL;
    } else if (i_leftime < 0) {
        /* ��ʱ���ڲ����� */
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_get_station::query info wait for %ld ms error!}",
                         ((QUERY_STATION_INFO_TIME * 1000) / OAL_TIME_HZ)); /* 1s����1000ms */
        return -OAL_EINVAL;
    } else {
        /* �������� */
        wal_cfg80211_fill_station_info(pst_sta_info, &pst_hmac_vap->station_info);
#ifdef CONFIG_HW_GET_EXT_SIG
        pst_sta_info->filled |= BIT(NL80211_STA_INFO_CNAHLOAD);
        pst_sta_info->chload = (oal_int32)pst_hmac_vap->st_station_info_extend.us_chload;
#endif
        wal_cfg80211_print_rssi_info(pst_hmac_vap, uc_vap_id);
        return OAL_SUCC;
    }
}


OAL_STATIC oal_int32 wal_cfg80211_dump_station(oal_wiphy_stru *pst_wiphy,
                                               oal_net_device_stru *pst_dev,
                                               oal_int32 int_index,
                                               oal_uint8 *puc_mac,
                                               oal_station_info_stru *pst_sta_info)
{
    return OAL_SUCC;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 44))  // 1102 ��Ҫ����ʱ���ں˰汾������
#if (_PRE_CONFIG_TARGET_PRODUCT != _PRE_TARGET_PRODUCT_TYPE_E5)
#ifdef _PRE_WLAN_FEATURE_P2P

OAL_STATIC oal_uint32 wal_is_p2p_group_exist(mac_device_stru *pst_mac_device)
{
    if ((hmac_check_p2p_vap_num(pst_mac_device, WLAN_P2P_GO_MODE) != OAL_SUCC) ||
        (hmac_check_p2p_vap_num(pst_mac_device, WLAN_P2P_CL_MODE) != OAL_SUCC)) {
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
}
#endif

oal_uint32 wal_del_p2p_group(mac_device_stru *pst_mac_device)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    oal_uint8 uc_vap_idx;
    mac_vap_stru *pst_mac_vap;
    hmac_vap_stru *pst_hmac_vap;
    oal_net_device_stru *pst_net_dev;
    mac_cfg_del_vap_param_stru st_del_vap_param;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (oal_unlikely(pst_mac_vap == OAL_PTR_NULL)) {
            OAM_WARNING_LOG1(0, OAM_SF_P2P, "{wal_del_p2p_group::get mac vap resource fail! vap id is %d}",
                             pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (oal_unlikely(pst_hmac_vap == OAL_PTR_NULL)) {
            OAM_WARNING_LOG1(0, OAM_SF_P2P, "{wal_del_p2p_group::get hmac vap resource fail! vap id is %d}",
                             pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        pst_net_dev = pst_hmac_vap->pst_net_device;
        if (oal_unlikely(pst_net_dev == OAL_PTR_NULL)) {
            OAM_WARNING_LOG1(0, OAM_SF_P2P, "{wal_del_p2p_group::get net device fail! vap id is %d}",
                             pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if (IS_P2P_GO(pst_mac_vap) || IS_P2P_CL(pst_mac_vap)) {
            memset_s(&st_del_vap_param, OAL_SIZEOF(st_del_vap_param), 0, OAL_SIZEOF(st_del_vap_param));
            OAL_IO_PRINT("wal_del_p2p_group:: ifname %.16s\r\n", pst_net_dev->name);
            st_del_vap_param.pst_net_dev = pst_net_dev;
            st_del_vap_param.en_vap_mode = pst_mac_vap->en_vap_mode;
            st_del_vap_param.en_p2p_mode = mac_get_p2p_mode(pst_mac_vap);
            oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{wal_del_p2p_group:: vap mode[%d], p2p mode[%d]}\r\n",
                             st_del_vap_param.en_vap_mode, st_del_vap_param.en_p2p_mode);
            /* ɾ���Ѿ����ڵ�P2P group */
            wal_force_scan_complete(pst_net_dev, OAL_TRUE);
            wal_stop_vap(pst_net_dev);
            if (wal_cfg80211_del_vap(&st_del_vap_param) == OAL_SUCC) {
                /* ����linux work queue ɾ��net_device����Ҫֱ�ӵ���unregister_netdev�Է�wal_netdev_stop��ѭ�� */
                pst_hmac_vap->pst_del_net_device = pst_net_dev;
                oal_workqueue_schedule(&(pst_hmac_vap->st_del_virtual_inf_worker));
            }
        }
    }
#endif
    return OAL_SUCC;
}


oal_uint32 wal_cfg80211_register_netdev(oal_net_device_stru *pst_net_dev)
{
    oal_uint8 uc_rollback_lock = OAL_FALSE;
    oal_uint32 ul_ret;
    oal_netdev_priv_stru *pst_netdev_priv;
    /*  nl80211 netlink pre diot �л��ȡrntl_lock�������� ע��net_device ���ȡrntl_lock����������������� */
    if (rtnl_is_locked()) {
        rtnl_unlock();
        uc_rollback_lock = OAL_TRUE;
    }
    /* NAPI pri netdevע�������net_deviceǰ�� */
    pst_netdev_priv = (oal_netdev_priv_stru *)oal_net_dev_wireless_priv(pst_net_dev);
#ifdef _PRE_WLAN_EXPORT
    pst_netdev_priv->uc_napi_enable = OAL_FALSE;
    pst_netdev_priv->uc_gro_enable = OAL_FALSE;
#else
    pst_netdev_priv->uc_napi_enable = OAL_TRUE;
    pst_netdev_priv->uc_gro_enable = OAL_TRUE;
#endif
    pst_netdev_priv->uc_napi_weight = NAPI_POLL_WEIGHT_MAX;
    pst_netdev_priv->uc_napi_dyn_weight = OAL_TRUE;
    pst_netdev_priv->uc_state = 0;
    pst_netdev_priv->ul_queue_len_max = NAPI_NETDEV_PRIV_QUEUE_LEN_MAX;
    pst_netdev_priv->ul_period_pkts = 0;
    pst_netdev_priv->ul_period_start = 0;
    oal_netbuf_list_head_init(&pst_netdev_priv->st_rx_netbuf_queue);
    /* poll�������� */
    oal_netif_napi_add(pst_net_dev, &pst_netdev_priv->st_napi, hmac_rxdata_polling, NAPI_POLL_WEIGHT_MAX);
    /* �ں�ע��net_device, ֻ����0 */
    ul_ret = (oal_uint32)oal_net_register_netdev(pst_net_dev);

    if (uc_rollback_lock) {
        rtnl_lock();
    }
    /*  END */
    return ul_ret;
}


oal_void wal_cfg80211_unregister_netdev(oal_net_device_stru *pst_net_dev)
{
    oal_uint8 uc_rollback_lock = OAL_FALSE;

    if (rtnl_is_locked()) {
        rtnl_unlock();
        uc_rollback_lock = OAL_TRUE;
    }

    /* ȥע��netdev */
    oal_net_unregister_netdev(pst_net_dev);

    if (uc_rollback_lock) {
        rtnl_lock();
    }
}

OAL_STATIC oal_void wal_cfg80211_add_p2p_interface_init(oal_net_device_stru *pst_net_dev,
                                                        mac_device_stru *pst_mac_device)
{
    oal_uint8 auc_primary_mac_addr[WLAN_MAC_ADDR_LEN] = { 0 }; /* MAC��ַ */

    if (oal_strncmp("p2p-p2p0", pst_net_dev->name, OAL_STRLEN("p2p-p2p0")) != 0) {
        return;
    }

    if (oal_unlikely(pst_mac_device->st_p2p_info.pst_primary_net_device == OAL_PTR_NULL)) {
        oal_random_ether_addr(auc_primary_mac_addr);
        auc_primary_mac_addr[0] &= (~0x02);
        auc_primary_mac_addr[1] = 0x11;
        auc_primary_mac_addr[2] = 0x02;
    } else {
        if (oal_unlikely(oal_netdevice_mac_addr(pst_mac_device->st_p2p_info.pst_primary_net_device) == OAL_PTR_NULL)) {
            OAM_ERROR_LOG0(0, OAM_SF_ANY,
                           "{wal_cfg80211_add_p2p_interface_init() pst_primary_net_device; addr is null}\r\n");
            return;
        }
        if (memcpy_s(auc_primary_mac_addr, WLAN_MAC_ADDR_LEN,
            oal_netdevice_mac_addr(pst_mac_device->st_p2p_info.pst_primary_net_device),
            WLAN_MAC_ADDR_LEN) != EOK) {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_add_p2p_interface_init::memcpy fail}");
        }
    }

    oal_set_mac_addr((oal_uint8 *)oal_netdevice_mac_addr(pst_net_dev), auc_primary_mac_addr);
    pst_net_dev->dev_addr[0] |= 0x02;
    pst_net_dev->dev_addr[4] ^= 0x80; /* dev_addr 4 byte��0x80������� */
}
oal_wireless_dev_stru* wal_get_virtual_intf_mode(mac_device_stru *pst_mac_device, enum nl80211_iftype en_type,
    wlan_p2p_mode_enum_uint8 *pen_p2p_mode, wlan_vap_mode_enum_uint8 *pen_vap_mode, oal_bool_enum_uint8 *pen_ret)
{
    oal_net_device_stru *pst_net_dev = OAL_PTR_NULL;
    oal_wireless_dev_stru *pst_wdev = OAL_PTR_NULL;
    switch (en_type) {
        case NL80211_IFTYPE_ADHOC:
        case NL80211_IFTYPE_AP_VLAN:
        case NL80211_IFTYPE_WDS:
        case NL80211_IFTYPE_MESH_POINT:
        case NL80211_IFTYPE_MONITOR:
            OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_add_virtual_intf::Unsupported interface type[%d]!}\r\n",
                en_type);
            *pen_ret = OAL_TRUE;
            return ERR_PTR(-EINVAL);
        case NL80211_IFTYPE_P2P_DEVICE:
            pst_net_dev = pst_mac_device->st_p2p_info.pst_p2p_net_device;
            pst_wdev = pst_net_dev->ieee80211_ptr;
            *pen_ret = OAL_TRUE;
            return pst_wdev;
        case NL80211_IFTYPE_P2P_CLIENT:
            *pen_vap_mode = WLAN_VAP_MODE_BSS_STA;
            *pen_p2p_mode = WLAN_P2P_CL_MODE;
            break;
        case NL80211_IFTYPE_STATION:
            *pen_vap_mode = WLAN_VAP_MODE_BSS_STA;
            *pen_p2p_mode = WLAN_LEGACY_VAP_MODE;
            break;
        case NL80211_IFTYPE_P2P_GO:
            *pen_vap_mode = WLAN_VAP_MODE_BSS_AP;
            *pen_p2p_mode = WLAN_P2P_GO_MODE;
            break;
        case NL80211_IFTYPE_AP:
            *pen_vap_mode = WLAN_VAP_MODE_BSS_AP;
            *pen_p2p_mode = WLAN_LEGACY_VAP_MODE;
            break;
        default:
            OAM_ERROR_LOG1(0, OAM_SF_CFG,
                "{wal_cfg80211_add_virtual_intf::Unsupported interface type[%d]!}\r\n", en_type);
            *pen_ret = OAL_TRUE;
            return ERR_PTR(-EINVAL);
    }
    *pen_ret = OAL_FALSE;
    return pst_wdev;
}
void wal_virtual_intf_init_netdev(oal_wireless_dev_stru *pst_wdev, oal_net_device_stru *pst_net_dev,
    mac_device_stru *pst_mac_device, enum nl80211_iftype en_type)
{
    memset_s(pst_wdev, OAL_SIZEOF(oal_wireless_dev_stru), 0, OAL_SIZEOF(oal_wireless_dev_stru));

    /* ��netdevice���и�ֵ */
    /* ���´�����net_device ��ʼ����Ӧ���� */
#ifdef CONFIG_WIRELESS_EXT
    pst_net_dev->wireless_handlers = &g_st_iw_handler_def;
#endif
    pst_net_dev->netdev_ops = &g_st_wal_net_dev_ops;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    pst_net_dev->ethtool_ops = &g_st_wal_ethtool_ops;
#endif

    oal_netdevice_destructor(pst_net_dev) = oal_net_free_netdev;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 0, 44))
    oal_netdevice_master(pst_net_dev) = OAL_PTR_NULL;
#endif

    oal_netdevice_ifalias(pst_net_dev) = OAL_PTR_NULL;
    oal_netdevice_watchdog_timeo(pst_net_dev) = 5;
    oal_netdevice_wdev(pst_net_dev) = pst_wdev;
    oal_netdevice_qdisc(pst_net_dev, OAL_PTR_NULL);

    pst_wdev->iftype = en_type;
    pst_wdev->wiphy = pst_mac_device->pst_wiphy;
    pst_wdev->netdev = pst_net_dev; /* ��wdev �е�net_device ��ֵ */
}
void wal_virtual_intf_init_vap_param(mac_cfg_add_vap_param_stru *pst_add_vap_param, oal_net_device_stru *pst_net_dev,
    wlan_p2p_mode_enum_uint8 en_p2p_mode, wlan_vap_mode_enum_uint8 en_vap_mode, oal_uint8 uc_cfg_vap_id)
{
    pst_add_vap_param->pst_net_dev = pst_net_dev;
    pst_add_vap_param->en_vap_mode = en_vap_mode;
    pst_add_vap_param->uc_cfg_vap_indx = uc_cfg_vap_id;
#ifdef _PRE_WLAN_FEATURE_P2P
    pst_add_vap_param->en_p2p_mode = en_p2p_mode;
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    pst_add_vap_param->bit_11ac2g_enable = (oal_uint8) !!hwifi_get_init_value(CUS_TAG_INI,
                                                                              WLAN_CFG_INIT_11AC2G_ENABLE);
    pst_add_vap_param->bit_disable_capab_2ght40 = g_wlan_customize.uc_disable_capab_2ght40;
#endif
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))
OAL_STATIC oal_wireless_dev_stru *wal_cfg80211_add_virtual_intf(oal_wiphy_stru *pst_wiphy,
                                                                const char *puc_name,
                                                                unsigned char name_assign_type,
                                                                enum nl80211_iftype en_type,
                                                                oal_vif_params_stru *pst_params)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
OAL_STATIC oal_wireless_dev_stru *wal_cfg80211_add_virtual_intf(oal_wiphy_stru *pst_wiphy,
                                                                const char *puc_name,
                                                                unsigned char name_assign_type,
                                                                enum nl80211_iftype en_type,
                                                                oal_uint32 *pul_flags,
                                                                oal_vif_params_stru *pst_params)
#else
OAL_STATIC oal_net_device_stru *wal_cfg80211_add_virtual_intf(oal_wiphy_stru *pst_wiphy,
                                                                char *puc_name,
                                                                enum nl80211_iftype en_type,
                                                                oal_uint32 *pul_flags,
                                                                oal_vif_params_stru *pst_params)
#endif
{
    oal_wireless_dev_stru *pst_wdev = OAL_PTR_NULL;
    wlan_p2p_mode_enum_uint8 en_p2p_mode;
    wlan_vap_mode_enum_uint8 en_vap_mode;
    oal_net_device_stru *pst_net_dev;
    mac_wiphy_priv_stru *pst_wiphy_priv;
    mac_device_stru *pst_mac_device;
    hmac_device_stru *pst_hmac_device;
#if (!defined(_PRE_PRODUCT_ID_HI110X_HOST))
    hmac_vap_stru *pst_p2p0_hmac_vap;
    oal_uint8 uc_p2p0_vap_idx;
#endif
    wal_msg_write_stru st_write_msg = {0};
    wal_msg_stru *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint8 uc_cfg_vap_id;
    mac_vap_stru *pst_cfg_mac_vap;
    hmac_vap_stru *pst_cfg_hmac_vap;
    mac_vap_stru *pst_mac_vap;
    oal_net_device_stru *pst_cfg_net_dev;
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    mac_cfg_add_vap_param_stru *pst_add_vap_param;
    oal_uint8 auc_name[OAL_IF_NAME_SIZE] = { 0 };
#ifdef _PRE_WLAN_FEATURE_P2P
    oal_uint8 uc_rollback_lock = 0;
    oal_int32 l_timeout;
#endif
    oal_bool_enum_uint8 en_ret;

    /* 1.1 ��μ�� */
    if (oal_any_null_ptr3(pst_wiphy, puc_name, pst_params)) {
        oam_error_log3(0, OAM_SF_CFG,
                       "{wal_cfg80211_add_virtual_intf:: ptr is null,error pst_wiphy %x, puc_name %x, pst_params %x!}",
                       (uintptr_t)pst_wiphy, (uintptr_t)puc_name, (uintptr_t)pst_params);
        return ERR_PTR(-EINVAL);
    }

    /* ��μ�����쳣��ֵ��������OALͳһ�ӿ� */
    pst_wiphy_priv = oal_wiphy_priv(pst_wiphy);
    if (pst_wiphy_priv == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_add_virtual_intf::pst_wiphy_priv is null!}\r\n");
        return ERR_PTR(-EINVAL);
    }

    pst_mac_device = pst_wiphy_priv->pst_mac_device;
    if (pst_mac_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_add_virtual_intf::pst_mac_device is null!}\r\n");
        return ERR_PTR(-EINVAL);
    }

    OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_add_virtual_intf::en_type[%d]!}\r\n", en_type);
    /* ��ʾ:SDT�޷���ӡ%s�ַ�����ʽ */
    OAL_IO_PRINT("wal_cfg80211_add_virtual_intf,dev_name is:%.16s\n", puc_name);

    pst_wdev = wal_get_virtual_intf_mode(pst_mac_device, en_type, &en_p2p_mode, &en_vap_mode, &en_ret);
    if (en_ret == OAL_TRUE) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
        return pst_wdev;
#else
        return pst_wdev->netdev;
#endif
    }
    /* ���������net device�Ѿ����ڣ�ֱ�ӷ��� */
    /* ����dev_name�ҵ�dev */
    pst_net_dev = oal_dev_get_by_name(puc_name);
    if (pst_net_dev != OAL_PTR_NULL) {
        /* ����oal_dev_get_by_name�󣬱������oal_dev_putʹnet_dev�����ü�����һ */
        oal_dev_put(pst_net_dev);

        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_add_virtual_intf::the net_device is already exist!}\r\n");
        pst_wdev = pst_net_dev->ieee80211_ptr;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
                return pst_wdev;
#else
                return pst_net_dev;
#endif
    }

    /* ����net_device ǰ���жϵ�ǰ�Ƿ�����ɾ��net_device ״̬��
        �������ɾ��net_device����ȴ�ɾ����ɣ������� */
    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_device->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_add_virtual_intf::hmac_device is null!}\r\n",
                         pst_mac_device->uc_device_id);
        return ERR_PTR(-ENODEV);
    }
#ifdef _PRE_WLAN_FEATURE_P2P
    if (hmac_get_p2p_status(pst_hmac_device->ul_p2p_intf_status, P2P_STATUS_IF_DELETING) == OAL_TRUE) {
        /* �ȴ�ɾ����� */
        if (rtnl_is_locked()) {
            rtnl_unlock();
            uc_rollback_lock = OAL_TRUE;
        }
        oam_warning_log0(0, OAM_SF_ANY,
                         "{wal_cfg80211_add_virtual_intf:Released the lock and wait till IF_DEL is complete!}\r\n");
        l_timeout = oal_wait_event_interruptible_timeout(pst_hmac_device->st_netif_change_event,
                                                         (hmac_get_p2p_status(pst_hmac_device->ul_p2p_intf_status,
                                                                              P2P_STATUS_IF_DELETING) == OAL_FALSE),
                                                         (oal_uint32)oal_msecs_to_jiffies(WAL_MAX_WAIT_TIME));

        /* put back the rtnl_lock again */
        if (uc_rollback_lock) {
            rtnl_lock();
        }

        if (l_timeout <= 0) {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_add_virtual_intf::timeount < 0, return -EAGAIN!}\r\n");
            return ERR_PTR(-EAGAIN);
        }
    }

    /* ���wifi �����У�P2P group �Ƿ��Ѿ����������P2P group �Ѿ�������
        �򽫸�P2P group ɾ�����������´���P2P group */
    if (wal_is_p2p_group_exist(pst_mac_device) == OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_add_virtual_intf::found exist p2p group, delet it first!}\r\n");
        if (wal_del_p2p_group(pst_mac_device) != OAL_SUCC) {
            return ERR_PTR(-EAGAIN);
        }
    }
#endif
    /* ��ȡ����VAP �ṹ */
    uc_cfg_vap_id = pst_mac_device->uc_cfg_vap_id;
    pst_cfg_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(uc_cfg_vap_id);
    if (pst_cfg_mac_vap == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_add_virtual_intf::mac_vap is null vap_id:%d!}", uc_cfg_vap_id);
        return ERR_PTR(-ENODEV);
    }
    pst_cfg_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(uc_cfg_vap_id);
    if (pst_cfg_hmac_vap == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_add_virtual_intf::vap is null vap_id:%d!}\r\n", uc_cfg_vap_id);
        return ERR_PTR(-ENODEV);
    }
    pst_cfg_net_dev = pst_cfg_hmac_vap->pst_net_device;
    if (memcpy_s(auc_name, OAL_IF_NAME_SIZE, puc_name, oal_min(OAL_IF_NAME_SIZE, OAL_STRLEN(puc_name))) != EOK) {
        OAM_ERROR_LOG0(pst_cfg_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_virtual_intf::memcpy fail!}");
    }

    pst_net_dev = oal_net_alloc_netdev_mqs(OAL_SIZEOF(oal_netdev_priv_stru), auc_name, oal_ether_setup,
                                           WLAN_NET_QUEUE_BUTT, 1); /* �˺�����һ����δ���˽�г��ȣ�����NAPI���޸� */
    if (oal_unlikely(pst_net_dev == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_cfg_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{wal_cfg80211_add_virtual_intf::pst_net_dev null ptr error!}\r\n");
        return ERR_PTR(-ENOMEM);
    }

    pst_wdev = (oal_wireless_dev_stru *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(oal_wireless_dev_stru),
                                                        OAL_FALSE);
    if (oal_unlikely(pst_wdev == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_cfg_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{wal_cfg80211_add_virtual_intf::alloc mem, pst_wdev is null ptr!}\r\n");
        /* �쳣�������ͷ��ڴ� */
        oal_net_free_netdev(pst_net_dev);
        return ERR_PTR(-ENOMEM);
    }

    wal_virtual_intf_init_netdev(pst_wdev, pst_net_dev, pst_mac_device, en_type);

#ifdef _PRE_WLAN_FEATURE_P2P
    if ((en_p2p_mode == WLAN_LEGACY_VAP_MODE) && (en_vap_mode == WLAN_VAP_MODE_BSS_STA)) {
        /* �������wlan0�� �򱣴�wlan0 Ϊ��net_device,p2p0 ��p2p-p2p0 MAC ��ַ����netdevice ��ȡ */
        if (pst_mac_device->st_p2p_info.pst_primary_net_device == OAL_PTR_NULL) {
            /* ����wlan0 �ڼ���wifi ����ʱ������Ҫͨ���ýӿ� */
            OAM_ERROR_LOG0(pst_cfg_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_virtual_intf::canot here!}\r\n");
            oal_mem_free_m(pst_wdev, OAL_TRUE);
            oal_net_free_netdev(pst_net_dev);
            return ERR_PTR(-ENODEV);
        }
    }
#endif
    /* ��ֹ�ϲ㲻�·�mac addr,����������p2p-p2p0-x mac addrδ��ʼ�� */
    wal_cfg80211_add_p2p_interface_init(pst_net_dev, pst_mac_device);

    oal_netdevice_flags(pst_net_dev) &= ~OAL_IFF_RUNNING; /* ��net device��flag��Ϊdown */

    ul_ret = wal_cfg80211_register_netdev(pst_net_dev);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        /* ע�᲻�ɹ����ͷ���Դ */
        oal_mem_free_m(pst_wdev, OAL_TRUE);
        oal_net_free_netdev(pst_net_dev);
        return ERR_PTR(-EBUSY);
    }

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_ADD_VAP, OAL_SIZEOF(mac_cfg_add_vap_param_stru));

    pst_add_vap_param = (mac_cfg_add_vap_param_stru *)(st_write_msg.auc_value);
    wal_virtual_intf_init_vap_param(pst_add_vap_param, pst_net_dev, en_p2p_mode, en_vap_mode, uc_cfg_vap_id);
    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_add_vap_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);
    if (l_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_add_virtual_intf::wal_send_cfg_event return err code: [%d]!}",
                         l_ret);
        /*lint -e801*/
        goto ERR_STEP;
        /*lint +e801*/
    }

    /* ��ȡ���صĴ����� */
    if (wal_check_and_release_msg_resp(pst_rsp_msg) != OAL_SUCC) {
        oam_warning_log0(pst_cfg_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_cfg80211_add_virtual_intf::wal_check_and_release_msg_resp fail:ul_err_code!}");
        goto ERR_STEP; /*lint !e801*/
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    wal_set_random_mac_to_mib(pst_net_dev); /* set random mac to mib ; for hi1102-cb */
#endif

    /* ����netdevice��MAC��ַ��MAC��ַ��HMAC�㱻��ʼ����MIB�� */
    pst_mac_vap = oal_net_dev_priv(pst_net_dev);
    if (oal_unlikely(pst_mac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_add_virtual_intf::oal_net_dev_priv(pst_net_dev) is null ptr.}");
        goto ERR_STEP; /*lint !e801*/
    }
    oal_set_mac_addr((oal_uint8 *)oal_netdevice_mac_addr(pst_net_dev),
                     pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID);

    /* ����VAP UP */
    wal_netdev_open(pst_net_dev);

    oam_warning_log2(0, OAM_SF_CFG, "{wal_cfg80211_add_virtual_intf::succ. en_type[%d],vap_id[%d]!}\r\n",
                     en_type, pst_mac_vap->uc_vap_id);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
            return pst_wdev;
#else
            return pst_net_dev;
#endif


    /* �쳣���� */
ERR_STEP:
    wal_cfg80211_unregister_netdev(pst_net_dev);
    /* ��ȥע�ᣬ���ͷ� */
    oal_mem_free_m(pst_wdev, OAL_FALSE);
    return ERR_PTR(-EAGAIN);
}
OAL_STATIC oal_bool_enum_uint8 wal_cfg80211_check_is_primary_netdev(oal_wiphy_stru *wiphy,
    oal_net_device_stru *net_dev)
{
    mac_device_stru *mac_device;
    mac_wiphy_priv_stru *wiphy_priv;
    wiphy_priv = oal_wiphy_priv(wiphy);
    if (wiphy_priv == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_check_is_primary_netdev::pst_wiphy_priv is null!}");
        return OAL_FALSE;
    }
    mac_device = wiphy_priv->pst_mac_device;
    if (mac_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_check_is_primary_netdev::pst_mac_device is null!}");
        return OAL_FALSE;
    }
    return mac_device->st_p2p_info.pst_primary_net_device == net_dev;
}


 #if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
OAL_STATIC oal_int32 wal_cfg80211_del_virtual_intf(oal_wiphy_stru *pst_wiphy,
                                                   oal_wireless_dev_stru *pst_wdev)
#else
OAL_STATIC oal_int32 wal_cfg80211_del_virtual_intf(oal_wiphy_stru *pst_wiphy,
                                                   oal_net_device_stru *pst_net_dev)
#endif
{
    /* �첽ȥע��net_device */
    wal_msg_write_stru st_write_msg = {0};
    wal_msg_stru *pst_rsp_msg = OAL_PTR_NULL;
    oal_int32 l_ret;
    mac_vap_stru *pst_mac_vap;
    hmac_vap_stru *pst_hmac_vap;
    hmac_device_stru *pst_hmac_device;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    oal_net_device_stru *pst_net_dev;
    if (oal_unlikely((pst_wiphy == OAL_PTR_NULL) || (pst_wdev == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_ANY,
                       "{wal_cfg80211_del_virtual_intf::pst_wiphy or pst_wdev null ptr \
            error %x, %x!}\r\n",
                       (uintptr_t)pst_wiphy, (uintptr_t)pst_wdev);
        return -OAL_EINVAL;
    }

#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::dfr_process_status[%d]!}",
                         g_st_dfr_info.bit_device_reset_process_flag);
        return OAL_SUCC;
    }

#endif  // #ifdef _PRE_WLAN_FEATURE_DFR

    pst_net_dev = pst_wdev->netdev;
#else
    oal_wireless_dev_stru *pst_wdev;
#endif
    if (pst_net_dev == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::pst_net_dev is null by netdev!}\r\n");
        return -OAL_EINVAL;
    }

    pst_mac_vap = oal_net_dev_priv(pst_net_dev);
    if (pst_mac_vap == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::mac_vap is null by netdev, mode[%d]!}\r\n",
                         pst_net_dev->ieee80211_ptr->iftype);
        return -OAL_EINVAL;
    }
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::mac_res_get_hmac_vap fail.vap_id[%u]}",
                       pst_mac_vap->uc_vap_id);
        return -OAL_EINVAL;
    }

    if (wal_cfg80211_check_is_primary_netdev(pst_wiphy, pst_net_dev)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::cannot del primary netdev}");
        return -OAL_EINVAL;
    }
    oal_net_tx_stop_all_queues(pst_net_dev);
    wal_netdev_stop(pst_net_dev);
    /* ���ͷ���net_device ���ͷ�wireless device */
    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    /* ��ʼ��ɾ��vap ���� */
    ((mac_cfg_del_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_net_dev;
#ifdef _PRE_WLAN_FEATURE_P2P
    pst_wdev = pst_net_dev->ieee80211_ptr;
    if (wal_wireless_iftype_to_mac_p2p_mode(pst_wdev->iftype) == WLAN_P2P_BUTT) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY,
                       "{wal_cfg80211_del_virtual_intf::wal_wireless_iftype_to_mac_p2p_mode return BUTT}\r\n");
        return -OAL_EINVAL;
    }

    ((mac_cfg_del_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode = mac_get_p2p_mode(pst_mac_vap);
#endif

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_DEL_VAP, OAL_SIZEOF(mac_cfg_del_vap_param_stru));

    /* ����ɾ��net_device ��ʶ */
    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::hmac_device is null!}\r\n",
                         pst_mac_vap->uc_device_id);
        return -OAL_EINVAL;
    }
#ifdef _PRE_WLAN_FEATURE_P2P
    hmac_set_p2p_status(&pst_hmac_device->ul_p2p_intf_status, P2P_STATUS_IF_DELETING);

    /* ����linux work ɾ��net_device */
    pst_hmac_vap->pst_del_net_device = pst_net_dev;
    oal_queue_work(g_del_virtual_inf_workqueue, &(pst_hmac_vap->st_del_virtual_inf_worker));
#endif
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_del_vap_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (wal_check_and_release_msg_resp(pst_rsp_msg) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::wal_check_and_release_msg_resp fail}");
    }

    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::return err code %d}\r\n", l_ret);
        l_ret = -OAL_EFAIL;
    }

    oam_warning_log2(0, OAM_SF_ANY,
                     "{wal_cfg80211_del_virtual_intf::pst_hmac_device->ul_p2p_intf_status %d, del result: %d}\r\n",
                     pst_hmac_device->ul_p2p_intf_status, l_ret);

    return l_ret;
}
#endif /* _PRE_CONFIG_TARGET_PRODUCT != _PRE_TARGET_PRODUCT_TYPE_E5 */

#else /* LINUX_VERSION_CODE < KERNEL_VERSION(3, 0, 44) */

oal_uint32 wal_del_p2p_group(mac_device_stru *pst_mac_device)
{
    return OAL_SUCC;
}
#endif

#if (_PRE_CONFIG_TARGET_PRODUCT != _PRE_TARGET_PRODUCT_TYPE_E5)


 #if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
OAL_STATIC oal_int32 wal_cfg80211_mgmt_tx_cancel_wait(oal_wiphy_stru *pst_wiphy,
                                                      oal_wireless_dev_stru *pst_wdev,
                                                      oal_uint64 ull_cookie)
#else
OAL_STATIC oal_int32 wal_cfg80211_mgmt_tx_cancel_wait(oal_wiphy_stru *pst_wiphy,
                                                      oal_net_device_stru *pst_netdev,
                                                      oal_uint64 ull_cookie)
#endif
{
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    oal_net_device_stru *pst_netdev = OAL_PTR_NULL;
    if (pst_wdev == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx_cancel_wait::pst_wdev is Null!}");
        return -OAL_EFAIL;
    }

    pst_netdev = pst_wdev->netdev;
#endif
    if (pst_netdev == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx_cancel_wait::pst_netdev is Null!}");
        return -OAL_EFAIL;
    }

    pst_mac_vap = oal_net_dev_priv(pst_netdev);
    if (pst_mac_vap == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx_cancel_wait::mac vap from netdevice is null!}");
        return -OAL_EFAIL;
    }

    return wal_p2p_stop_roc(pst_mac_vap, pst_netdev);
}
#endif /* _PRE_CONFIG_TARGET_PRODUCT != _PRE_TARGET_PRODUCT_TYPE_E5 */


OAL_STATIC oal_uint8 wal_tx_mgmt_send_event(oal_net_device_stru *pst_netdev,
                                            mac_mgmt_frame_stru *pst_mgmt_tx_param)
{
    oal_uint16 us_len;
    oal_int32 l_ret;
    wal_msg_write_stru st_write_msg;
    oal_netbuf_stru *pst_netbuf_mgmt_tx;
    mac_tx_ctl_stru *pst_tx_ctl = OAL_PTR_NULL;

    /* ����netbuf �ռ� */
    pst_netbuf_mgmt_tx = (oal_netbuf_stru *)oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, pst_mgmt_tx_param->us_len,
                                                                 OAL_NETBUF_PRIORITY_MID);
    if (pst_netbuf_mgmt_tx == OAL_PTR_NULL) {
        /* Reserved Memory pool tried for high priority deauth frames */
        pst_netbuf_mgmt_tx = (oal_netbuf_stru *)oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2,
                                                                     OAL_NETBUF_PRIORITY_MID);
        if (pst_netbuf_mgmt_tx == OAL_PTR_NULL) {
            OAM_ERROR_LOG0(0, OAM_SF_P2P, "{wal_tx_mgmt_send_event::pst_mgmt_tx null.}");
            return OAL_TX_MGMT_FAIL;
        }
    }

    oal_mem_netbuf_trace(pst_netbuf_mgmt_tx, OAL_TRUE);

    memset_s(oal_netbuf_cb(pst_netbuf_mgmt_tx), OAL_SIZEOF(mac_tx_ctl_stru), 0, OAL_SIZEOF(mac_tx_ctl_stru));

    /* ���netbuf */
    if (memcpy_s((oal_uint8 *)oal_netbuf_header(pst_netbuf_mgmt_tx), pst_mgmt_tx_param->us_len,
        pst_mgmt_tx_param->puc_frame, pst_mgmt_tx_param->us_len) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_P2P, "{wal_tx_mgmt_send_event::memcpy fail.}");
    }
    oal_netbuf_put(pst_netbuf_mgmt_tx, pst_mgmt_tx_param->us_len);

    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf_mgmt_tx); /* ��ȡcb�ṹ�� */
    pst_tx_ctl->us_mpdu_len = pst_mgmt_tx_param->us_len;               /* dmac������Ҫ��mpdu���� */
    pst_tx_ctl->us_tx_user_idx = 0xF;                                  /* ���������Ҫ��ȡuser�ṹ�� */
    pst_tx_ctl->bit_need_rsp = OAL_TRUE;
    pst_tx_ctl->bit_is_vipframe = OAL_TRUE;
    pst_tx_ctl->bit_is_needretry = OAL_TRUE;
    pst_tx_ctl->bit_mgmt_frame_id = pst_mgmt_tx_param->mgmt_frame_id;

    oam_warning_log2(0, OAM_SF_P2P, "{wal_tx_mgmt_send_event::mgmt frame id=[%d] len = [%d]}",
                     pst_mgmt_tx_param->mgmt_frame_id, pst_mgmt_tx_param->us_len);

    /***************************************************************************
       ���¼���wal�㴦��
   ***************************************************************************/
    us_len = OAL_SIZEOF(oal_netbuf_stru *);
    /* 3.1 ��д msg ��Ϣͷ */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CFG80211_MGMT_TX, us_len);

    /* 3.2 ��д msg ��Ϣ�� */
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value),
        (oal_uint8 *)&pst_netbuf_mgmt_tx, us_len) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_P2P, "{wal_tx_mgmt_send_event::memcpy fail}");
    }

    /* 3.3 ������Ϣ */
    l_ret = wal_send_cfg_event(pst_netdev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
        (oal_uint8 *)&st_write_msg, OAL_FALSE, OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oal_netbuf_free(pst_netbuf_mgmt_tx);
        return OAL_TX_MGMT_FAIL;
    }
    return OAL_TX_MGMT_SUCC;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 44))  // ȷ����ȷ�� Linux �汾��
#if (_PRE_CONFIG_TARGET_PRODUCT != _PRE_TARGET_PRODUCT_TYPE_E5)


OAL_STATIC oal_int32 wal_cfg80211_set_pmksa(oal_wiphy_stru *pst_wiphy,
                                            oal_net_device_stru *pst_net_device,
                                            oal_cfg80211_pmksa_stru *pmksa)
{
    wal_msg_write_stru st_write_msg = { 0 };
    mac_cfg_pmksa_param_stru *pst_cfg_pmksa;
    oal_int32 l_ret;
    oal_int32 l_memcpy_ret;

    if ((pst_wiphy == OAL_PTR_NULL) || (pst_net_device == OAL_PTR_NULL) || (pmksa == OAL_PTR_NULL)) {
        oam_error_log3(0, OAM_SF_CFG,
                       "{wal_cfg80211_set_pmksa::param null! pst_wiphy[%x], pst_net_device[%x], pmksa[%x]!!}\r\n",
                       (uintptr_t)pst_wiphy, (uintptr_t)pst_net_device, (uintptr_t)pmksa);
        return -OAL_EINVAL;
    }

    if ((pmksa->bssid == OAL_PTR_NULL) || (pmksa->pmkid == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_CFG, "{wal_cfg80211_set_pmksa::param null! bssid[%x] pmkid[%x]}\r\n",
                       (uintptr_t)(pmksa->bssid), (uintptr_t)(pmksa->pmkid));
        return -OAL_EINVAL;
    }

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CFG80211_SET_PMKSA, OAL_SIZEOF(mac_cfg_pmksa_param_stru));
    pst_cfg_pmksa = (mac_cfg_pmksa_param_stru *)st_write_msg.auc_value;
    l_memcpy_ret = memcpy_s(pst_cfg_pmksa->auc_bssid, WLAN_MAC_ADDR_LEN, pmksa->bssid, WLAN_MAC_ADDR_LEN);
    l_memcpy_ret += memcpy_s(pst_cfg_pmksa->auc_pmkid, WLAN_PMKID_LEN, pmksa->pmkid, WLAN_PMKID_LEN);
    if (l_memcpy_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "wal_cfg80211_set_pmksa::memcpy fail!");
        return -OAL_EFAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_device,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_pmksa_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (l_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_set_pmksa::wal_send_cfg_event fail[%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_cfg80211_del_pmksa(oal_wiphy_stru *pst_wiphy,
                                            oal_net_device_stru *pst_net_device,
                                            oal_cfg80211_pmksa_stru *pmksa)
{
    wal_msg_write_stru st_write_msg = { 0 };
    mac_cfg_pmksa_param_stru *pst_cfg_pmksa;
    oal_int32 l_ret;
    oal_int32 l_memcpy_ret;

    if ((pst_wiphy == OAL_PTR_NULL) || (pst_net_device == OAL_PTR_NULL) || (pmksa == OAL_PTR_NULL)) {
        oam_error_log3(0, OAM_SF_CFG,
                       "{wal_cfg80211_del_pmksa::param null! pst_wiphy[%x], pst_net_device[%x], pmksa[%x]!!}\r\n",
                       (uintptr_t)pst_wiphy, (uintptr_t)pst_net_device, (uintptr_t)pmksa);
        return -OAL_EINVAL;
    }

    if ((pmksa->bssid == OAL_PTR_NULL) || (pmksa->pmkid == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_CFG, "{wal_cfg80211_del_pmksa::param null! bssid[%x] pmkid[%x]}\r\n",
                       (uintptr_t)(pmksa->bssid), (uintptr_t)(pmksa->pmkid));
        return -OAL_EINVAL;
    }

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CFG80211_DEL_PMKSA, OAL_SIZEOF(mac_cfg_pmksa_param_stru));
    pst_cfg_pmksa = (mac_cfg_pmksa_param_stru *)st_write_msg.auc_value;
    l_memcpy_ret = memcpy_s(pst_cfg_pmksa->auc_bssid, WLAN_MAC_ADDR_LEN, pmksa->bssid, WLAN_MAC_ADDR_LEN);
    l_memcpy_ret += memcpy_s(pst_cfg_pmksa->auc_pmkid, WLAN_PMKID_LEN, pmksa->pmkid, WLAN_PMKID_LEN);
    if (l_memcpy_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "wal_cfg80211_del_pmksa::memcpy fail!");
        return -OAL_EFAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_device,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_pmksa_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (l_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_del_pmksa::wal_send_cfg_event fail[%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_cfg80211_flush_pmksa(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_net_device)
{
    wal_msg_write_stru st_write_msg = { 0 };
    oal_int32 l_ret;
    mac_vap_stru *pst_mac_vap;

    if ((pst_wiphy == OAL_PTR_NULL) || (pst_net_device == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_CFG,
                       "{wal_cfg80211_flush_pmksa::param null! pst_wiphy[%x], pst_net_device[%x]!!}\r\n",
                       (uintptr_t)pst_wiphy, (uintptr_t)pst_net_device);
        return -OAL_EINVAL;
    }

    /* ͨ��net_device �ҵ���Ӧ��mac_vap_stru �ṹ���Է�wal_alloc_cfg_event����ERROR */
    pst_mac_vap = oal_net_dev_priv(pst_net_device);
    if (pst_mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_flush_pmksa::can't get mac vap from netdevice priv data!}");
        return -OAL_EFAIL;
    }

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CFG80211_FLUSH_PMKSA, 0);

    l_ret = wal_send_cfg_event(pst_net_device,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (l_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_flush_pmksa::wal_send_cfg_event fail[%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC oal_bool_enum_uint8 wal_check_all_vap_state_connecting(mac_device_stru *pst_mac_device)
{
    mac_vap_stru *pst_other_vap;
    oal_uint8 uc_vap_idx;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_other_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_other_vap == OAL_PTR_NULL) {
            OAM_WARNING_LOG1(0, OAM_SF_P2P, "{wal_check_all_vap_state_connecting::vap is null! vap id is %d}",
                             pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if (((pst_other_vap->en_vap_state >= MAC_VAP_STATE_STA_JOIN_COMP) &&
             (pst_other_vap->en_vap_state <= MAC_VAP_STATE_STA_WAIT_ASOC))
#ifdef _PRE_WLAN_FEATURE_ROAM
            || (pst_other_vap->en_vap_state == MAC_VAP_STATE_ROAMING)
#endif
) {
            OAM_WARNING_LOG1(pst_other_vap->uc_vap_id, OAM_SF_P2P, "{wal_check_all_vap_state_connecting::roaming or \
                connecting state[%d]!}", pst_other_vap->en_vap_state);
            return OAL_FALSE;
        }
    }
    return OAL_TRUE;
}


OAL_STATIC oal_uint32 wal_drv_init_roc_channel(mac_remain_on_channel_param_stru *pst_remain_on_channel,
                                               struct ieee80211_channel *pst_chan,
                                               oal_uint64 *pull_cookie,
                                               mac_device_stru *pst_mac_device,
                                               oal_uint32 ul_duration,
                                               wlan_ieee80211_roc_type_uint8 en_roc_type)
{
    oal_uint16 us_center_freq;
    oal_int32 l_channel;

    if (oal_any_null_ptr3(pst_chan, pull_cookie, pst_mac_device)) {
        oam_error_log3(0, OAM_SF_P2P,
                       "{wal_drv_init_roc_channel::pst_chan or pull_cookie or pst_mac_device ptr is null,\
            error %d, %d, %d!}\r\n",
                       (uintptr_t)pst_chan, (uintptr_t)pull_cookie, (uintptr_t)pst_mac_device);
        return OAL_ERR_CODE_PTR_NULL;
    }

    us_center_freq = pst_chan->center_freq;
    l_channel = (oal_int32)oal_ieee80211_frequency_to_channel((oal_int32)us_center_freq);

    pst_remain_on_channel->uc_listen_channel = (oal_uint8)l_channel;
    pst_remain_on_channel->ul_listen_duration = ul_duration;
    pst_remain_on_channel->st_listen_channel = *pst_chan;
    pst_remain_on_channel->en_listen_channel_type = WLAN_BAND_WIDTH_20M;
    pst_remain_on_channel->en_roc_type = en_roc_type;

    if (pst_chan->band == HISI_IEEE80211_BAND_2GHZ) {
        pst_remain_on_channel->en_band = WLAN_BAND_2G;
    } else if (pst_chan->band == HISI_IEEE80211_BAND_5GHZ) {
        pst_remain_on_channel->en_band = WLAN_BAND_5G;
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_P2P, "{wal_drv_init_roc_channel::wrong band type[%d]!}\r\n", pst_chan->band);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    if (en_roc_type == IEEE80211_ROC_TYPE_NORMAL) {
        /* ��cookie+1ֵ��ǰ����֤������ɨ���cookieֵһ�±�������ɨ��cookieֵŪ������� */
        /* ����cookie ֵ */
        *pull_cookie = ++pst_mac_device->st_p2p_info.ull_last_roc_id; /* cookieֵ�ϲ������Ҫ�ж��Ƿ�����εķ��͵��µ�callback */
        if (*pull_cookie == 0) {
            *pull_cookie = ++pst_mac_device->st_p2p_info.ull_last_roc_id;
        }

        /* ����cookie ֵ���·���HMAC ��DMAC */
        pst_remain_on_channel->ull_cookie = pst_mac_device->st_p2p_info.ull_last_roc_id;
    }

    oam_warning_log3(0, OAM_SF_P2P,
                     "{wal_drv_init_roc_channel::SUCC! l_channel=%d, ul_duration=%d, roc type=%d!}\r\n",
                     l_channel, ul_duration, en_roc_type);
    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_drv_remain_on_channel(oal_wiphy_stru *pst_wiphy,
                                               oal_wireless_dev_stru *pst_wdev,
                                               struct ieee80211_channel *pst_chan,
                                               oal_uint32 ul_duration,
                                               oal_uint64 *pull_cookie,
                                               wlan_ieee80211_roc_type_uint8 en_roc_type)
{
    wal_msg_write_stru st_write_msg = { 0 };
    wal_msg_stru *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32 ul_err_code;
    oal_net_device_stru *pst_netdev;
    mac_remain_on_channel_param_stru st_remain_on_channel = { 0 };
    oal_int32 l_ret;
    mac_device_stru *pst_mac_device;
    mac_vap_stru *pst_mac_vap;

    /* 1.1 ��μ�� */
    if (oal_any_null_ptr4(pst_wiphy, pst_wdev, pst_chan, pull_cookie)) {
        OAM_ERROR_LOG0(0, OAM_SF_P2P, "{wal_drv_remain_on_channel::wiphy or wdev or chan or pull_cookie is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_netdev = pst_wdev->netdev;
    if (pst_netdev == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_P2P, "{wal_drv_remain_on_channel::pst_netdev ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_drv_remain_on_channel:: dfr_process_status[%d]!}",
                         g_st_dfr_info.bit_device_reset_process_flag);
        return -OAL_EFAIL;
    }
#endif  // #ifdef _PRE_WLAN_FEATURE_DFR

    /* ͨ��net_device �ҵ���Ӧ��mac_device_stru �ṹ */
    pst_mac_vap = oal_net_dev_priv(pst_netdev);
    if (pst_mac_vap == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_P2P, "{wal_drv_remain_on_channel::can't get mac vap from netdevice priv data!}\r\n");
        return -OAL_EINVAL;
    }

    pst_mac_device = (mac_device_stru *)mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_P2P, "{wal_drv_remain_on_channel::pst_mac_device ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

#ifdef _PRE_WLAN_FEATURE_WAPI
    if (hmac_user_is_wapi_connected(pst_mac_vap->uc_device_id) == OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_CFG, "{stop p2p remaining under wapi!}");
        return -OAL_EINVAL;
    }
#endif /* #ifdef _PRE_WLAN_FEATURE_WAPI */

    // ������ڵ���ǰ�п�
    if (wal_check_all_vap_state_connecting(pst_mac_device) != OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_check_all_vap_state_connecting failed!}");
        return -OAL_EFAIL;
    }

    /* tx mgmt roc ���ȼ���,���Ա��Լ���80211 roc�Լ�80211 scan��� */
    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
                         "{wal_drv_remain_on_channel::new roc type[%d],cancel old roc!}", en_roc_type);
        l_ret = wal_p2p_stop_roc(pst_mac_vap, pst_netdev);
        if (l_ret < 0) {
            return -OAL_EFAIL;
        }
    }
    /* 2.1 ��Ϣ����׼�� */
    if (wal_drv_init_roc_channel(&st_remain_on_channel, pst_chan, pull_cookie, pst_mac_device, ul_duration,
                                 en_roc_type) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_P2P, "{wal_drv_remain_on_channel::init roc channel init failed!}\r\n");
        return -OAL_EINVAL;
    }

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* 3.1 ��д msg ��Ϣͷ */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CFG80211_REMAIN_ON_CHANNEL,
                           OAL_SIZEOF(mac_remain_on_channel_param_stru));

    /* 3.2 ��д msg ��Ϣ�� */
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(mac_remain_on_channel_param_stru),
                 &st_remain_on_channel, OAL_SIZEOF(mac_remain_on_channel_param_stru)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_P2P, "wal_drv_remain_on_channel::memcpy fail!");
        return -OAL_EFAIL;
    }

    /* 3.3 ������Ϣ */
    l_ret = wal_send_cfg_event(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_remain_on_channel_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE, &pst_rsp_msg);
    if (l_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_P2P,
                       "{wal_drv_remain_on_channel::wal_send_cfg_event return err code:[%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    /* 4.1 ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (ul_err_code != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG,
                         "{wal_drv_remain_on_channel::wal_check_and_release_msg_resp fail return err code:[%u]!}\r\n",
                         ul_err_code);
        return -OAL_EFAIL;
    }
    if (en_roc_type == IEEE80211_ROC_TYPE_NORMAL) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
        /* �ϱ���ͣ��ָ���ŵ��ɹ� */
        oal_cfg80211_ready_on_channel(pst_wdev, *pull_cookie, pst_chan, ul_duration, GFP_KERNEL);
#endif
    }
    oam_warning_log3(0, OAM_SF_P2P,
                     "{wal_drv_remain_on_channel::SUCC! ul_duration=%d, cookie 0x%x, band= %d!}\r\n",
                     ul_duration, *pull_cookie, st_remain_on_channel.en_band);
    return OAL_SUCC;
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))

OAL_STATIC oal_int32 wal_cfg80211_remain_on_channel(oal_wiphy_stru *pst_wiphy,
                                                    oal_wireless_dev_stru *pst_wdev,
                                                    struct ieee80211_channel *pst_chan,
                                                    oal_uint32 ul_duration,
                                                    oal_uint64 *pull_cookie)
{
    return wal_drv_remain_on_channel(pst_wiphy, pst_wdev, pst_chan, ul_duration, pull_cookie,
                                     IEEE80211_ROC_TYPE_NORMAL);
}
#else
OAL_STATIC oal_int32 wal_cfg80211_remain_on_channel(oal_wiphy_stru *pst_wiphy,
                                                    oal_net_device_stru *pst_netdev,
                                                    struct ieee80211_channel *pst_chan,
                                                    oal_nl80211_channel_type ul_channel_type,
                                                    oal_uint32 ul_duration,
                                                    oal_uint64 *pull_cookie)
{
    oal_wireless_dev_stru *pst_wdev;
    pst_wdev = pst_netdev->ieee80211_ptr;
    return wal_drv_remain_on_channel(pst_wiphy, pst_wdev, pst_chan, ul_duration, pull_cookie,
                                     IEEE80211_ROC_TYPE_NORMAL);
}

#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
OAL_STATIC oal_int32 wal_cfg80211_cancel_remain_on_channel(oal_wiphy_stru *pst_wiphy,
                                                           oal_wireless_dev_stru *pst_wdev,
                                                           oal_uint64 ull_cookie)
#else
OAL_STATIC oal_int32 wal_cfg80211_cancel_remain_on_channel(oal_wiphy_stru *pst_wiphy,
                                                           oal_net_device_stru *pst_netdev,
                                                           oal_uint64 ull_cookie)
#endif
{
    wal_msg_write_stru st_write_msg = { 0 };
    mac_remain_on_channel_param_stru st_cancel_remain_on_channel = { 0 };
    wal_msg_stru *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32 ul_err_code;
    oal_int32 l_ret;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    oal_net_device_stru *pst_netdev;
    if ((pst_wiphy == OAL_PTR_NULL) || (pst_wdev == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_P2P,
                       "{wal_cfg80211_cancel_remain_on_channel::pst_wiphy or pst_wdev is null,error %x, %x!}\r\n",
                       (uintptr_t)pst_wiphy, (uintptr_t)pst_wdev);
        return -OAL_EINVAL;
    }

    pst_netdev = pst_wdev->netdev;
#endif
    if (pst_netdev == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_P2P, "{wal_cfg80211_cancel_remain_on_channel::pst_netdev ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    OAM_WARNING_LOG1(0, OAM_SF_P2P, "wal_cfg80211_cancel_remain_on_channel[0x%x].", ull_cookie);

    /* 3.1 ��д msg ��Ϣͷ */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CFG80211_CANCEL_REMAIN_ON_CHANNEL,
                           OAL_SIZEOF(mac_remain_on_channel_param_stru));

    /* 3.2 ��д msg ��Ϣ�� */
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(mac_remain_on_channel_param_stru),
                 &st_cancel_remain_on_channel, OAL_SIZEOF(mac_remain_on_channel_param_stru)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_P2P, "wal_cfg80211_cancel_remain_on_channel::memcpy fail!");
        return -OAL_EFAIL;
    }

    /* 3.3 ������Ϣ */
    l_ret = wal_send_cfg_event(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_remain_on_channel_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);
    if (l_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_P2P,
                         "{wal_cfg80211_cancel_remain_on_channel::wal_send_cfg_event return err code: [%d]!}", l_ret);
        return -OAL_EFAIL;
    }

    /* 4.1 ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (ul_err_code != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_P2P, "wal_cfg80211_cancel_remain_on_channel:wal_check_and_release_msg_resp fail, \
            err:[%u]", ul_err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


oal_uint8 wal_find_oldest_cookie(cookie_arry_stru *pst_cookie_array)
{
    oal_uint8 uc_loops = 0;
    oal_uint8 uc_target_index = 0;

    /* �ⲿ��ֻ֤����cookie����������²Ż���øú�������˳�ʼindexѡ0���� */
    for (uc_loops = 1; uc_loops < WAL_COOKIE_ARRAY_SIZE; uc_loops++) {
        if (oal_time_after32(pst_cookie_array[uc_target_index].ul_record_time,
                             pst_cookie_array[uc_loops].ul_record_time)) {
            uc_target_index = uc_loops;
        }
    }

    return uc_target_index;
}


OAL_STATIC oal_void wal_check_cookie_timeout(cookie_arry_stru *pst_cookie_array,
                                             oal_uint8 *puc_cookie_bitmap,
                                             oal_uint32 ul_current_time)
{
    oal_uint8 uc_loops = 0;
    cookie_arry_stru *pst_tmp_cookie;

    oam_warning_log0(0, OAM_SF_CFG, "{wal_check_cookie_timeout::time_out!}\r\n");
    for (uc_loops = 0; uc_loops < WAL_COOKIE_ARRAY_SIZE; uc_loops++) {
        pst_tmp_cookie = &pst_cookie_array[uc_loops];
        if (oal_time_after32(OAL_TIME_JIFFY,
                             pst_tmp_cookie->ul_record_time + oal_msecs_to_jiffies(WAL_MGMT_TX_TIMEOUT_MSEC))) {
            /* cookie array �б����cookie ֵ��ʱ */
            /* ���cookie array �г�ʱ��cookie */
            pst_tmp_cookie->ul_record_time = 0;
            pst_tmp_cookie->ull_cookie = 0;
            /* ���ռ�õ�cookie bitmapλ */
            oal_bit_clear_bit_one_byte(puc_cookie_bitmap, uc_loops);
        }
    }
}


OAL_STATIC oal_uint32 wal_del_cookie_from_array(cookie_arry_stru *pst_cookie_array,
                                                oal_uint8 *puc_cookie_bitmap,
                                                oal_uint8 uc_cookie_idx)
{
    cookie_arry_stru *pst_tmp_cookie;

    /* �����Ӧcookie bitmap λ */
    oal_bit_clear_bit_one_byte(puc_cookie_bitmap, uc_cookie_idx);

    /* ���cookie array �г�ʱ��cookie */
    pst_tmp_cookie = &pst_cookie_array[uc_cookie_idx];
    pst_tmp_cookie->ull_cookie = 0;
    pst_tmp_cookie->ul_record_time = 0;
    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_add_cookie_to_array(cookie_arry_stru *pst_cookie_array,
                                              oal_uint8 *puc_cookie_bitmap,
                                              oal_uint64 *pull_cookie,
                                              oal_uint8 *puc_cookie_idx)
{
    oal_uint8 uc_idx;
    cookie_arry_stru *pst_tmp_cookie;

    if (*puc_cookie_bitmap == WAL_COOKIE_FULL_MASK) {
        /* cookie array �������ش��� */
        oam_warning_log0(0, OAM_SF_CFG, "{wal_add_cookie_to_array::array full!}\r\n");
        return OAL_FAIL;
    }

    /* ��cookie ���ӵ�array �� */
    uc_idx = oal_bit_find_first_zero_one_byte(*puc_cookie_bitmap);
    oal_bit_set_bit_one_byte(puc_cookie_bitmap, uc_idx);

    pst_tmp_cookie = &pst_cookie_array[uc_idx];
    pst_tmp_cookie->ull_cookie = *pull_cookie;
    pst_tmp_cookie->ul_record_time = OAL_TIME_JIFFY;

    *puc_cookie_idx = uc_idx;
    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_check_cookie_from_array(oal_uint8 *puc_cookie_bitmap, oal_uint8 uc_cookie_idx)
{
    /* ��cookie bitmap�в�����Ӧ��cookie index�����λͼΪ0����ʾ�Ѿ���del */
    if (*puc_cookie_bitmap & (BIT(uc_cookie_idx))) {
        return OAL_SUCC;
    }
    /* �Ҳ����򷵻�FAIL */
    return OAL_FAIL;
}


OAL_STATIC oal_uint8 wal_mgmt_do_tx(oal_net_device_stru *pst_netdev,
                                    mac_mgmt_frame_stru *pst_mgmt_tx_param,
                                    bool en_offchan,
                                    oal_uint32 ul_wait)
{
    mac_vap_stru *pst_mac_vap;
    hmac_vap_stru *pst_hmac_vap;
    oal_mgmt_tx_stru *pst_mgmt_tx;
    oal_int i_leftime;
    oal_uint8 uc_ret;

    pst_mac_vap = oal_net_dev_priv(pst_netdev);
    if (pst_mac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_mgmt_do_tx::can't get mac vap from netdevice priv data.}\r\n");
        return OAL_TX_MGMT_FAIL;
    }

    //  �����Go, ��Ȼ��������(����Go Device Discoverability Request)
    if (!IS_P2P_GO(pst_mac_vap)) {
        // �������Go�����������ж�
        /* ��Ҫoffchanel�������Ѿ�����roc״̬��,��action֡����Ҫ���� */
        if (en_offchan == OAL_TRUE) {
            if (pst_mac_vap->en_vap_state != MAC_VAP_STATE_STA_LISTEN) {
                OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                                 "{wal_mgmt_do_tx::pst_mac_vap state[%d]not in listen!}", pst_mac_vap->en_vap_state);
                return OAL_TX_MGMT_ABORT;  // ����OAL_TX_MGMT_ABORT,���������ش�,�����ϲ�˴�tx mgmtʧ�ܽ���
            }
        }
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_mgmt_do_tx::pst_hmac_vap ptr is null!}\r\n");
        return OAL_TX_MGMT_FAIL;
    }

    pst_mgmt_tx = &(pst_hmac_vap->st_mgmt_tx);
    pst_mgmt_tx->mgmt_tx_complete = OAL_FALSE;
    pst_mgmt_tx->mgmt_tx_status = OAL_FAIL;

    uc_ret = wal_tx_mgmt_send_event(pst_netdev, pst_mgmt_tx_param);
    if (uc_ret != OAL_TX_MGMT_SUCC) {
        return OAL_TX_MGMT_FAIL;
    }

    /*lint -e730*/
    i_leftime = oal_wait_event_interruptible_timeout(pst_mgmt_tx->st_wait_queue,
                                                     pst_mgmt_tx->mgmt_tx_complete == OAL_TRUE,
                                                     (oal_uint32)oal_msecs_to_jiffies(ul_wait));
    if (i_leftime == 0) {
        /* ��ʱ����ʱ */
        oam_warning_log0(0, OAM_SF_ANY, "{wal_mgmt_do_tx::mgmt tx timeout!}\r\n");
        return OAL_TX_MGMT_FAIL;
    } else if (i_leftime < 0) {
        /* ��ʱ���ڲ����� */
        oam_warning_log0(0, OAM_SF_ANY, "{wal_mgmt_do_tx::mgmt tx timer error!}\r\n");
        return OAL_TX_MGMT_FAIL;
    } else {
        /* �������� */
        oam_info_log0(0, OAM_SF_ANY, "{wal_mgmt_do_tx::mgmt tx commpleted!}\r\n");
        /* . �������ͽ��������ط������״̬ */
        return (pst_mgmt_tx->mgmt_tx_status != OAL_SUCC) ? OAL_TX_MGMT_FAIL : OAL_TX_MGMT_SUCC;
    }
}
#ifdef _PRE_WLAN_FEATURE_SAE
static void wal_ap_set_mgmt_tx_chan(mac_vap_stru *mac_vap, oal_ieee80211_channel *st_chan)
{
    st_chan->band = mac_vap->st_channel.en_band;
    st_chan->center_freq = (oal_uint16)oal_ieee80211_channel_to_frequency(mac_vap->st_channel.uc_chan_number,
        mac_vap->st_channel.en_band);
    st_chan->hw_value = mac_vap->st_channel.uc_chan_number;

    oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_SAE,
        "{wal_cfg80211_mgmt_tx:: AP pst_chan ptr is null! vap_mode[%d], p2p_mode [%d]. use vap_channel [%d]}",
        mac_vap->en_vap_mode, mac_vap->en_p2p_mode, mac_vap->st_channel.uc_chan_number);
}
#endif

 #if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
OAL_STATIC oal_int32 wal_cfg80211_mgmt_tx(oal_wiphy_stru *pst_wiphy,
                                          oal_wireless_dev_stru *pst_wdev,
                                          struct cfg80211_mgmt_tx_params *pst_params,
                                          oal_uint64 *pull_cookie)
#else
OAL_STATIC oal_int32 wal_cfg80211_mgmt_tx(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
                              oal_ieee80211_channel *chan, bool offchan,
                              oal_nl80211_channel_type channel_type,
                              bool channel_type_valid, oal_uint32 wait,
                              const oal_uint8 *buf, oal_size_t len, bool no_cck,
                              bool dont_wait_for_ack, oal_uint64 *pull_cookie)
#endif
{
    mac_device_stru *pst_mac_device;
    mac_vap_stru *pst_mac_vap;
    OAL_CONST oal_ieee80211_mgmt *pst_mgmt;
    oal_int32 ul_ret;
    mac_mgmt_frame_stru st_mgmt_tx;
    oal_uint8 uc_cookie_idx;
    oal_uint8 uc_retry;
    mac_p2p_info_stru *pst_p2p_info;
    hmac_vap_stru *pst_hmac_vap;
    oal_mgmt_tx_stru *pst_mgmt_tx;
    unsigned long ul_start_time_stamp;
    bool en_need_offchan = OAL_FALSE;
    oal_ieee80211_channel st_chan = { 0 };
    oal_ieee80211_channel *pst_chan;
    OAL_CONST oal_uint8 *puc_buf;
    oal_size_t ul_len;
    oal_uint32 ul_wait;
    bool en_offchan = OAL_FALSE;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    oal_net_device_stru *pst_netdev;
    oal_wireless_dev_stru *pst_roc_wireless_dev = pst_wdev;
    if (pst_params == OAL_PTR_NULL) {
        return -OAL_EINVAL;
    }
    pst_chan = pst_params->chan;
    puc_buf = pst_params->buf;
    ul_len = pst_params->len;
    en_offchan = pst_params->offchan;
    ul_wait = pst_params->wait;
    /* 1.1 ��μ�� */
    if (oal_any_null_ptr4(pst_wiphy, pst_wdev, pull_cookie, puc_buf)) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::wiphy or wdev or pull_cookie or puc_buf ptr is null!}");
        return -OAL_EINVAL;
    }

    /* ͨ��net_device �ҵ���Ӧ��mac_device_stru �ṹ */
    pst_netdev = pst_wdev->netdev;
#else
    oal_wireless_dev_stru *pst_roc_wireless_dev = pst_netdev->ieee80211_ptr;
    pst_chan = chan;
    puc_buf = buf;
    ul_len = len;
    en_offchan = offchan;
    ul_wait = wait;
#endif
    if (pst_netdev == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::pst_netdev ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_mac_vap = oal_net_dev_priv(pst_netdev);
    if (pst_mac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::can't get mac vap from netdevice priv data!}\r\n");
        return -OAL_EINVAL;
    }

    pst_mac_device = (mac_device_stru *)mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::pst_mac_device ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::pst_hmac_vap ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_p2p_info = &pst_mac_device->st_p2p_info;
    *pull_cookie = pst_p2p_info->ull_send_action_id++; /* cookieֵ�ϲ������Ҫ�ж��Ƿ�����εķ��͵��µ�callback */
    if (*pull_cookie == 0) {
        *pull_cookie = pst_p2p_info->ull_send_action_id++;
    }
    pst_mgmt = (const struct ieee80211_mgmt *)puc_buf;
    if (oal_ieee80211_is_probe_resp(pst_mgmt->frame_control)) {
        *pull_cookie = 0; /* set cookie default value */
        /* host should not send PROE RESPONSE,
           device will send immediately when receive probe request packet */
        oal_cfg80211_mgmt_tx_status(pst_roc_wireless_dev, *pull_cookie, puc_buf, ul_len, OAL_TRUE, GFP_KERNEL);
        return OAL_SUCC;
    }

#ifdef _PRE_WLAN_FEATURE_SAE
    if (pst_chan == OAL_PTR_NULL) {
        if (IS_AP(pst_mac_vap)) {
            /* APֱ��ȡVAP�ŵ� */
            wal_ap_set_mgmt_tx_chan(pst_mac_vap, &st_chan);
        } else {
            /* STAȡɨ�����ŵ� */
            mac_bss_dscr_stru *pst_bss_dscr;
            /* SAE����auth commit/comfirm ֡���·���pst_channel�����NULL,
             * ��Ҫ����Ŀ��MAC��ɨ�����в����ŵ���Ϣ */
            oam_warning_log0(0, OAM_SF_SAE, "{wal_cfg80211_mgmt_tx::STA tx frame, find channel from scan result}");
            /* �жϴ����֡�����Ƿ񹻳� */
            if (ul_len < MAC_80211_FRAME_LEN) {
                OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SAE,
                    "{wal_cfg80211_mgmt_tx::frame length is too short to send. %d}", ul_len);
                return -OAL_EINVAL;
            }

            pst_bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(pst_mac_vap, pst_mgmt->da);
            if (pst_bss_dscr == OAL_PTR_NULL) {
                oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_SAE,
                    "{wal_cfg80211_mgmt_tx::can not find [%02X:XX:XX:XX:%02X:%02X] from scan result.}",
                    pst_mgmt->da[0], pst_mgmt->da[4], pst_mgmt->da[5]);
                return -OAL_EINVAL;
            }
            st_chan.band = pst_bss_dscr->st_channel.en_band;
            st_chan.center_freq =
                (oal_uint16)oal_ieee80211_channel_to_frequency(pst_bss_dscr->st_channel.uc_chan_number,
                                                               pst_bss_dscr->st_channel.en_band);
            st_chan.hw_value = pst_bss_dscr->st_channel.uc_chan_number;
        }
    } else {
#ifdef _PRE_WLAN_CHBA_MGMT
        /* �����chba����ֱ�ӻ�ȡvap���ŵ�Ϊ��ǰmgmt �����ŵ� */
        if (hmac_chba_vap_start_check(pst_hmac_vap) == OAL_TRUE) {
            wal_ap_set_mgmt_tx_chan(pst_mac_vap, &st_chan);
        } else {
#endif
            st_chan = *pst_chan;
#ifdef _PRE_WLAN_CHBA_MGMT
        }
#endif
    }
#else
    if (pst_chan == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx:: pst_chan ptr is null!}");
        return -OAL_EINVAL;
    } else {
        st_chan = *pst_chan;
    }
#endif /* _PRE_WLAN_FEATURE_SAE */

    /* 2.1 ��Ϣ����׼�� */
    memset_s(&st_mgmt_tx, OAL_SIZEOF(st_mgmt_tx), 0, OAL_SIZEOF(st_mgmt_tx));
    st_mgmt_tx.channel = oal_ieee80211_frequency_to_channel(st_chan.center_freq);

    /* :����ⷢ��cookie�ռ����꣬�������ɵ�һ��cookie */
    if (g_uc_cookie_array_bitmap == WAL_COOKIE_FULL_MASK) {
        uc_cookie_idx = wal_find_oldest_cookie(g_cookie_array);
        wal_del_cookie_from_array(g_cookie_array, &g_uc_cookie_array_bitmap, uc_cookie_idx);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx::cookies is [0x%x] after clear}",
                         g_uc_cookie_array_bitmap);
    }

    ul_ret = (oal_int32)wal_add_cookie_to_array(g_cookie_array, &g_uc_cookie_array_bitmap,
        pull_cookie, &uc_cookie_idx);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_cfg80211_mgmt_tx::Failed to add cookies, ul_ret[%d]!}\r\n", ul_ret);
        return -OAL_EINVAL;
    } else {
        st_mgmt_tx.mgmt_frame_id = uc_cookie_idx;
    }

    st_mgmt_tx.us_len = ul_len;
    st_mgmt_tx.puc_frame = puc_buf;

    pst_mgmt_tx = &(pst_hmac_vap->st_mgmt_tx);
    pst_mgmt_tx->mgmt_tx_complete = OAL_FALSE;
    pst_mgmt_tx->mgmt_tx_status = OAL_FAIL;

    /* APUTģʽ����roc */
    switch (pst_hmac_vap->st_vap_base_info.en_vap_mode) {
        case WLAN_VAP_MODE_BSS_AP:
            // �˴�Ӧ����channel number��������channel index
            oam_warning_log3(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx: \
                p2p mode[%d](0=Legacy, 1=Go, 2=Dev, 3=Gc), tx mgmt vap channel[%d], mgmt tx channel[%d]",
                pst_hmac_vap->st_vap_base_info.en_p2p_mode,
                pst_hmac_vap->st_vap_base_info.st_channel.uc_chan_number, st_mgmt_tx.channel);
            if (pst_hmac_vap->st_vap_base_info.st_channel.uc_chan_number != st_mgmt_tx.channel) {
                if (IS_P2P_GO(&pst_hmac_vap->st_vap_base_info)) {
                    if (pst_mac_device->st_p2p_info.pst_p2p_net_device == OAL_PTR_NULL) {
                        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                            "{wal_cfg80211_mgmt_tx::go mode but p2p dev is null");
                        return -OAL_EINVAL;
                    }
                    pst_roc_wireless_dev = oal_netdevice_wdev(pst_mac_device->st_p2p_info.pst_p2p_net_device);
                }
                en_need_offchan = OAL_TRUE;
            }
            break;

        /* P2P CL DEV */
        case WLAN_VAP_MODE_BSS_STA:
            if ((en_offchan == OAL_TRUE) && (pst_wiphy->flags & WIPHY_FLAG_OFFCHAN_TX)) {
                en_need_offchan = OAL_TRUE;
            }
            if ((pst_hmac_vap->st_vap_base_info.en_p2p_mode == WLAN_LEGACY_VAP_MODE) &&
                (pst_hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_UP) &&
                (st_mgmt_tx.channel == pst_hmac_vap->st_vap_base_info.st_channel.uc_chan_number)) {
                en_need_offchan = OAL_FALSE;
            }
            break;

        default:
            break;
    }

    if ((en_need_offchan == OAL_TRUE) && (st_chan.center_freq == 0)) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{wal_cfg80211_mgmt_tx::need offchannle but channel is null}\r\n", en_offchan);
        return -OAL_EINVAL;
    }

    //  �����Ƿ���Ҫ�����ŵ������淢�͹���֡ʱ����Ҫ�ȴ�һ��ʱ��
    if (ul_wait == 0) {
        ul_wait = WAL_MGMT_TX_TIMEOUT_MSEC;
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx::wait time is 0, set it to %d ms}",
                         ul_wait);
    }

    oam_warning_log4(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                     "{wal_cfg80211_mgmt_tx::offchannel[%d].channel[%d]vap state[%d],wait[%d]}\r\n",
                     en_need_offchan, st_mgmt_tx.channel, pst_hmac_vap->st_vap_base_info.en_vap_state, ul_wait);

    /* ��Ҫoffchannel,��������е���Ӧ���ŵ�XXms */
    if (en_need_offchan == OAL_TRUE) {
        ul_ret = wal_drv_remain_on_channel(pst_wiphy, pst_roc_wireless_dev, &st_chan, ul_wait, pull_cookie,
                                           IEEE80211_ROC_TYPE_MGMT_TX);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log4(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "wal_cfg80211_mgmt_tx: \
                wal_drv_remain_on_channel[%d]!!!offchannel[%d].channel[%d], vap state[%d]",
                             ul_ret, en_need_offchan, st_mgmt_tx.channel, pst_hmac_vap->st_vap_base_info.en_vap_state);
            return -OAL_EBUSY;
        }
    }

    ul_start_time_stamp = OAL_TIME_JIFFY;

    uc_retry = 0;
    /* �����ŵ�,��������һֱ�ش�,���������ش��������� */
    /* ����ʧ�ܣ������ش� */
    do {
        ul_ret = wal_mgmt_do_tx(pst_netdev, &st_mgmt_tx, en_need_offchan, ul_wait);
        uc_retry++;
    } while ((ul_ret != OAL_TX_MGMT_SUCC) && (ul_ret != OAL_TX_MGMT_ABORT) && (uc_retry <= WAL_MGMT_TX_RETRY_CNT)
             && time_before(OAL_TIME_JIFFY, ul_start_time_stamp + oal_msecs_to_jiffies(ul_wait))); /*lint !e666*/

    //  �����⣬�����Go��(ͨ��p2p0)��һ�λỰ�еڶ���(��һ�η����ѳɹ�)������������Device Discoverability Response��֡ʱ�����ܻ����ڶԷ�������������ʱʧ�ܡ�
    if (uc_retry > WAL_MGMT_TX_RETRY_CNT) {
        oam_warning_log3(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                         "{wal_cfg80211_mgmt_tx::retry count[%d]>max[%d],tx status[%d],stop tx mgmt}\r\n",
                         uc_retry, WAL_MGMT_TX_RETRY_CNT, ul_ret);
    }
    if ((ul_ret != OAL_TX_MGMT_SUCC)) {
        /* ����ʧ�ܣ�������ʱ֡��bitmap */
        wal_check_cookie_timeout(g_cookie_array, &g_uc_cookie_array_bitmap, OAL_TIME_JIFFY);

        oam_warning_log2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                         "wal_cfg80211_mgmt_tx:vap status[%d], tx mgmt timeout=%d", pst_mac_vap->en_vap_state, ul_wait);

#ifdef _PRE_WLAN_FEATURE_P2P
        /* ,ʧ�ܳ�ʱ��Ҫ�ָ�ԭvap״̬����ȡ��device��remain on channel */
        if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN) {
            mac_vap_state_change(&pst_hmac_vap->st_vap_base_info, pst_mac_device->st_p2p_info.en_last_vap_state);

            /* ���¼���DMAC �����ؼ����ŵ� */
            hmac_p2p_send_listen_expired_to_device(pst_hmac_vap);
        }
#endif

        oal_cfg80211_mgmt_tx_status(pst_roc_wireless_dev, *pull_cookie, puc_buf, ul_len, OAL_FALSE, GFP_KERNEL);
    } else {
        if (pst_mgmt_tx->mgmt_frame_id >= WAL_COOKIE_ARRAY_SIZE) {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx::invalid frame id [%d]}", pst_mgmt_tx->mgmt_frame_id);
            return OAL_FAIL;
        }
        /* �������� */
        *pull_cookie = g_cookie_array[pst_mgmt_tx->mgmt_frame_id].ull_cookie;
        wal_del_cookie_from_array(g_cookie_array, &g_uc_cookie_array_bitmap, pst_mgmt_tx->mgmt_frame_id);
        oal_cfg80211_mgmt_tx_status(pst_roc_wireless_dev, *pull_cookie, puc_buf, ul_len, OAL_TRUE, GFP_KERNEL);
    }

    oam_warning_log3(0, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx::tx status [%d], retry cnt[%d]}, delta_time[%d]\r\n",
                     ul_ret, uc_retry, oal_jiffies_to_msecs(OAL_TIME_JIFFY - ul_start_time_stamp));

    return OAL_SUCC;
}


oal_uint32 wal_cfg80211_mgmt_tx_status(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event;
    dmac_crx_mgmt_tx_status_stru *pst_mgmt_tx_status_param;
    hmac_vap_stru *pst_hmac_vap;
    oal_mgmt_tx_stru *pst_mgmt_tx;

    if (oal_unlikely(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx_status::pst_event_mem is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = (frw_event_stru *)pst_event_mem->puc_data;
    pst_hmac_vap = mac_res_get_hmac_vap(pst_event->st_event_hdr.uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_TX, "{wal_cfg80211_mgmt_tx_status::pst_hmac_vap null.vap_id[%d]}",
                       pst_event->st_event_hdr.uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mgmt_tx_status_param = (dmac_crx_mgmt_tx_status_stru *)(pst_event->auc_event_data);
    pst_mgmt_tx = &(pst_hmac_vap->st_mgmt_tx);
    pst_mgmt_tx->mgmt_tx_complete = OAL_TRUE;
    pst_mgmt_tx->mgmt_tx_status = pst_mgmt_tx_status_param->uc_dscr_status;
    pst_mgmt_tx->mgmt_frame_id = pst_mgmt_tx_status_param->mgmt_frame_id;

    /* �Ҳ�����Ӧ��cookieֵ��˵���Ѿ���ʱ������������Ҫ�ٻ��� */
    if (wal_check_cookie_from_array(&g_uc_cookie_array_bitmap, pst_mgmt_tx->mgmt_frame_id) == OAL_SUCC) {
        /* �ñ������Ż�ʱ��֤OAL_WAIT_QUEUE_WAKE_UP�����ִ�� */
        oal_smp_mb();
        oal_wait_queue_wake_up_interrupt(&pst_mgmt_tx->st_wait_queue);
    }

    return OAL_SUCC;
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
/* P2P ����ȱʧ��CFG80211�ӿ� */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
void wal_cfg80211_mgmt_frame_register(struct wiphy *wiphy,
    struct wireless_dev *wdev,
    struct mgmt_frame_regs *upd)
#else
void wal_cfg80211_mgmt_frame_register(struct wiphy *wiphy, struct wireless_dev *wdev,
    uint16_t frame_type, bool reg)
#endif
{
    if ((wiphy == OAL_PTR_NULL) || (wdev == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_frame_register::wiphy[%x], wdev[%x]}",
                       (uintptr_t)wiphy, (uintptr_t)wdev);
        return;
    }
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
    oam_info_log3(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_frame_register::enter.frame_type[0x%02x], reg[%d], if_type[%d]}",
                  frame_type, reg, wdev->iftype);
#endif
    return;
}
#endif
oal_int32 wal_cfg80211_set_bitrate_mask(struct wiphy *wiphy,
                                        struct net_device *dev,
                                        const u8 *peer,
                                        const struct cfg80211_bitrate_mask *mask)
{
    oam_info_log0(0, OAM_SF_CFG, "{wal_cfg80211_set_bitrate_mask::enter 000.}");

    return OAL_SUCC;
}

#endif /* _PRE_CONFIG_TARGET_PRODUCT != _PRE_TARGET_PRODUCT_TYPE_E5 */

#else

oal_void wal_check_cookie_timeout(cookie_arry_stru *pst_cookie_array,
                                  oal_uint8 *puc_cookie_bitmap,
                                  oal_uint32 ul_current_time)
{
    oal_uint8 uc_loops = 0;
    cookie_arry_stru *pst_tmp_cookie = OAL_PTR_NULL;

    oam_warning_log0(0, OAM_SF_CFG, "{wal_check_cookie_timeout::time_out!}\r\n");
    for (uc_loops = 0; uc_loops < WAL_COOKIE_ARRAY_SIZE; uc_loops++) {
        pst_tmp_cookie = &pst_cookie_array[uc_loops];
        if (oal_time_is_before(pst_tmp_cookie->ul_record_time + oal_msecs_to_jiffies(WAL_MGMT_TX_TIMEOUT_MSEC))) {
            /* cookie array �б����cookie ֵ��ʱ */
            /* ���cookie array �г�ʱ��cookie */
            pst_tmp_cookie->ul_record_time = 0;
            pst_tmp_cookie->ull_cookie = 0;
            /* ���ռ�õ�cookie bitmapλ */
            oal_bit_clear_bit_one_byte(puc_cookie_bitmap, uc_loops);
        }
    }
}


oal_uint32 wal_del_cookie_from_array(cookie_arry_stru *pst_cookie_array,
                                     oal_uint8 *puc_cookie_bitmap,
                                     oal_uint8 uc_cookie_idx)
{
    cookie_arry_stru *pst_tmp_cookie = OAL_PTR_NULL;

    /* �����Ӧcookie bitmap λ */
    oal_bit_clear_bit_one_byte(puc_cookie_bitmap, uc_cookie_idx);

    /* ���cookie array �г�ʱ��cookie */
    pst_tmp_cookie = &pst_cookie_array[uc_cookie_idx];
    pst_tmp_cookie->ull_cookie = 0;
    pst_tmp_cookie->ul_record_time = 0;
    return OAL_SUCC;
}


oal_uint32 wal_add_cookie_to_array(cookie_arry_stru *pst_cookie_array,
                                   oal_uint8 *puc_cookie_bitmap,
                                   oal_uint64 *pull_cookie,
                                   oal_uint8 *puc_cookie_idx)
{
    oal_uint8 uc_idx;
    cookie_arry_stru *pst_tmp_cookie = OAL_PTR_NULL;

    if (*puc_cookie_bitmap == 0xFF) {
        /* cookie array �������ش��� */
        oam_warning_log0(0, OAM_SF_CFG, "{wal_add_cookie_to_array::array full!}\r\n");
        return OAL_FAIL;
    }

    /* ��cookie ���ӵ�array �� */
    uc_idx = oal_bit_get_num_one_byte(*puc_cookie_bitmap);
    oal_bit_set_bit_one_byte(puc_cookie_bitmap, uc_idx);

    pst_tmp_cookie = &pst_cookie_array[uc_idx];
    pst_tmp_cookie->ull_cookie = *pull_cookie;
    pst_tmp_cookie->ul_record_time = OAL_TIME_JIFFY;

    *puc_cookie_idx = uc_idx;
    return OAL_SUCC;
}

oal_int32 wal_cfg80211_mgmt_tx(
    oal_wiphy_stru *pst_wiphy, oal_wireless_dev_stru *pst_wdev, oal_ieee80211_channel *pst_chan,
    oal_bool_enum_uint8 en_offchan, oal_uint32 ul_wait, OAL_CONST oal_uint8 *puc_buf,
    size_t ul_len, oal_bool_enum_uint8 en_no_cck,  oal_bool_enum_uint8 en_dont_wait_for_ack, oal_uint64 *pull_cookie)
{
    wal_msg_write_stru st_write_msg;
    oal_net_device_stru *pst_netdev = OAL_PTR_NULL;
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    OAL_CONST oal_ieee80211_mgmt *pst_mgmt;
    oal_int32 l_ret = 0;
    oal_uint32 ul_ret = 0;
    oal_uint8 uc_ret = 0;
    mac_mgmt_frame_stru st_mgmt_tx;
    oal_int i_leftime;
    oal_uint8 uc_cookie_idx;
    mac_p2p_info_stru *pst_p2p_info = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    oal_mgmt_tx_stru *pst_mgmt_tx = OAL_PTR_NULL;

    /* 1.1 ��μ�� */
    if ((pst_wiphy == OAL_PTR_NULL) || (pst_wdev == OAL_PTR_NULL) || (pst_chan == OAL_PTR_NULL) ||
        (pull_cookie == OAL_PTR_NULL) || (puc_buf == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG,
                       "{wal_cfg80211_mgmt_tx::pst_wiphy or pst_wdev or pst_chan or pull_cookie or puc_buf is null!}");
        return -OAL_EINVAL;
    }

    /* ͨ��net_device �ҵ���Ӧ��mac_device_stru �ṹ */
    pst_netdev = pst_wdev->netdev;
    if (pst_netdev == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::pst_netdev ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_mac_vap = oal_net_dev_priv(pst_netdev);
    if (pst_mac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::pst_mac_vap ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_mac_device = (mac_device_stru *)mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::pst_mac_device ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::pst_hmac_vap ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_p2p_info = &pst_mac_device->st_p2p_info;
    *pull_cookie = pst_p2p_info->ull_send_action_id++; /* cookieֵ�ϲ������Ҫ�ж��Ƿ�����εķ��͵��µ�callback */
    if (*pull_cookie == 0) {
        *pull_cookie = pst_p2p_info->ull_send_action_id++;
    }
    pst_mgmt = (const struct ieee80211_mgmt *)puc_buf;
    if (oal_ieee80211_is_probe_resp(pst_mgmt->frame_control)) {
        *pull_cookie = 0; /* set cookie default value */
        /* host should not send PROE RESPONSE,
           device will send immediately when receive probe request packet */
        oal_cfg80211_mgmt_tx_status(pst_wdev, *pull_cookie, puc_buf, ul_len, OAL_TRUE, GFP_KERNEL);
        return OAL_SUCC;
    }

    /* 2.1 ��Ϣ����׼�� */
    memset_s(&st_mgmt_tx, OAL_SIZEOF(st_mgmt_tx), 0, OAL_SIZEOF(st_mgmt_tx));
    st_mgmt_tx.channel = oal_ieee80211_frequency_to_channel((oal_int32)pst_chan->center_freq);
    ul_ret = wal_add_cookie_to_array(g_cookie_array, &g_uc_cookie_array_bitmap, pull_cookie, &uc_cookie_idx);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx::Failed to add cookies!}\r\n");
        return -OAL_EINVAL;
    } else {
        st_mgmt_tx.mgmt_frame_id = uc_cookie_idx;
    }
    st_mgmt_tx.us_len = (oal_uint16)ul_len;
    st_mgmt_tx.puc_frame = puc_buf;

    uc_ret = wal_tx_mgmt_send_event(pst_netdev, &st_mgmt_tx);
    if (uc_ret != OAL_TX_MGMT_SUCC) {
        return -OAL_EINVAL;
    }

    pst_mgmt_tx = &(pst_hmac_vap->st_mgmt_tx);
    pst_mgmt_tx->mgmt_tx_complete = OAL_FALSE;
    pst_mgmt_tx->mgmt_tx_status = OAL_FAIL;
    /*lint -e730*/
    i_leftime = oal_wait_event_interruptible_timeout(pst_mgmt_tx->st_wait_queue,
                                                     pst_mgmt_tx->mgmt_tx_complete == OAL_TRUE,
                                                     (oal_uint32)oal_msecs_to_jiffies(WAL_MGMT_TX_TIMEOUT_MSEC));
    /*lint +e730*/
    if (i_leftime == 0) {
        /* ��ʱ��û���ϱ�ɨ����� */
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx::mgmt tx wait for %ld ms timeout!}\r\n",
                         ((oal_uint32)WAL_MGMT_TX_TIMEOUT_MSEC));
        wal_check_cookie_timeout(g_cookie_array, &g_uc_cookie_array_bitmap, OAL_TIME_JIFFY);

    } else if (i_leftime < 0) {
        /* ��ʱ���ڲ����� */
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx::mgmt tx wait for %ld ms error!}\r\n",
                         ((oal_uint32)WAL_MGMT_TX_TIMEOUT_MSEC));
    } else {
        /* �������� */
        oam_info_log1(0, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx::mgmt tx wait for %ld ms complete!}\r\n",
                      ((oal_uint32)WAL_MGMT_TX_TIMEOUT_MSEC));
        *pull_cookie = g_cookie_array[pst_mgmt_tx->mgmt_frame_id].ull_cookie;
        wal_del_cookie_from_array(g_cookie_array, &g_uc_cookie_array_bitmap, pst_mgmt_tx->mgmt_frame_id);
    }

    if (pst_mgmt_tx->mgmt_tx_status != OAL_SUCC) {
        oal_cfg80211_mgmt_tx_status(pst_wdev, *pull_cookie, puc_buf, ul_len, OAL_FALSE, GFP_KERNEL);
        return -OAL_EINVAL;
    } else {
        oal_cfg80211_mgmt_tx_status(pst_wdev, *pull_cookie, puc_buf, ul_len, OAL_TRUE, GFP_KERNEL);
        return OAL_SUCC;
    }
}
/*lint +e774*/

oal_uint32 wal_cfg80211_mgmt_tx_status(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = OAL_PTR_NULL;
    dmac_crx_mgmt_tx_status_stru *pst_mgmt_tx_status_param = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    oal_mgmt_tx_stru *pst_mgmt_tx = OAL_PTR_NULL;

    if (oal_unlikely(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx_status::pst_event_mem is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_event = (frw_event_stru *)pst_event_mem->puc_data;
    pst_hmac_vap = mac_res_get_hmac_vap(pst_event->st_event_hdr.uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{wal_cfg80211_mgmt_tx_status::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mgmt_tx_status_param = (dmac_crx_mgmt_tx_status_stru *)(pst_event->auc_event_data);
    pst_mgmt_tx = &(pst_hmac_vap->st_mgmt_tx);
    pst_mgmt_tx->mgmt_tx_complete = OAL_TRUE;
    pst_mgmt_tx->mgmt_tx_status = pst_mgmt_tx_status_param->uc_dscr_status;
    pst_mgmt_tx->mgmt_frame_id = pst_mgmt_tx_status_param->mgmt_frame_id;

    /* �ñ������Ż�ʱ��֤OAL_WAIT_QUEUE_WAKE_UP�����ִ�� */
    oal_smp_mb();
    oal_wait_queue_wake_up_interrupt(&pst_mgmt_tx->st_wait_queue);

    return OAL_SUCC;
}

#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 44))
#if (_PRE_CONFIG_TARGET_PRODUCT != _PRE_TARGET_PRODUCT_TYPE_E5)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))

OAL_STATIC oal_int32 wal_cfg80211_start_p2p_device(oal_wiphy_stru *pst_wiphy, oal_wireless_dev_stru *pst_wdev)
{
    return -OAL_EFAIL;
}


OAL_STATIC void wal_cfg80211_stop_p2p_device(oal_wiphy_stru *pst_wiphy, oal_wireless_dev_stru *pst_wdev)
{
}
#endif

static oal_int32 wal_cfg80211_set_power_mgmt(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
    bool enabled, oal_int32 timeout)
{
#ifdef _PRE_WLAN_FEATURE_STA_PM
    wal_msg_write_stru st_write_msg = {0};
    mac_cfg_ps_open_stru *pst_sta_pm_open;
    oal_int32 l_ret;
    mac_vap_stru *pst_mac_vap;

    if (oal_unlikely((pst_wiphy == OAL_PTR_NULL) || (pst_netdev == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_ANY, "{wal_cfg80211_set_power_mgmt::pst_wiphy or pst_wdev null ptr error %x, %x!}\r\n",
                       (uintptr_t)pst_wiphy, (uintptr_t)pst_netdev);
        return -OAL_EINVAL;
    }

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_STA_PM_ON, OAL_SIZEOF(mac_cfg_ps_open_stru));

    pst_mac_vap = oal_net_dev_priv(pst_netdev);
    if (oal_unlikely(pst_mac_vap == NULL)) {
        oam_warning_log0(0, OAM_SF_PWR, "{wal_cfg80211_set_power_mgmt::get mac vap failed.}");
        return OAL_SUCC;
    }

    /* P2P dev���·� */
    if (IS_P2P_DEV(pst_mac_vap)) {
        oam_warning_log0(0, OAM_SF_PWR, "wal_cfg80211_set_power_mgmt:vap is p2p dev return");
        return OAL_SUCC;
    }

    oam_warning_log3(0, OAM_SF_PWR, "{wal_cfg80211_set_power_mgmt::vap mode[%d]p2p mode[%d]set pm:[%d]}",
                     pst_mac_vap->en_vap_mode, pst_mac_vap->en_p2p_mode, enabled);
    pst_sta_pm_open = (mac_cfg_ps_open_stru *)(st_write_msg.auc_value);
    /* MAC_STA_PM_SWITCH_ON / MAC_STA_PM_SWITCH_OFF */
    pst_sta_pm_open->uc_pm_enable = enabled;
    pst_sta_pm_open->uc_pm_ctrl_type = MAC_STA_PM_CTRL_TYPE_HOST;

    l_ret = wal_send_cfg_event(pst_netdev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_ps_open_stru), (oal_uint8 *)&st_write_msg, OAL_FALSE,
        OAL_PTR_NULL);
    if (l_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_set_power_mgmt::fail to send pm cfg msg, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }
#endif
    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_11R


OAL_STATIC oal_int32 wal_cfg80211_update_ft_ies(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
    oal_cfg80211_update_ft_ies_stru *pst_fties)
{
    wal_msg_write_stru st_write_msg = {0};
    mac_cfg80211_ft_ies_stru *pst_mac_ft_ies;
    wal_msg_stru *pst_rsp_msg;
    oal_uint32 ul_err_code;
    oal_int32 l_ret;

    if ((pst_wiphy == OAL_PTR_NULL) || (pst_netdev == OAL_PTR_NULL) || (pst_fties == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_update_ft_ies::param is null.}\r\n");

        return -OAL_EINVAL;
    }

    if ((pst_fties->ie == OAL_PTR_NULL) || (pst_fties->ie_len == 0) || (pst_fties->ie_len >= MAC_MAX_FTE_LEN)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_update_ft_ies::unexpect ie or len[%d].}\r\n", pst_fties->ie_len);

        return -OAL_EINVAL;
    }

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    st_write_msg.en_wid = WLAN_CFGID_SET_FT_IES;
    st_write_msg.us_len = OAL_SIZEOF(mac_cfg80211_ft_ies_stru);

    pst_mac_ft_ies = (mac_cfg80211_ft_ies_stru *)st_write_msg.auc_value;
    pst_mac_ft_ies->us_mdid = pst_fties->md;
    pst_mac_ft_ies->us_len = pst_fties->ie_len;
    if (memcpy_s(pst_mac_ft_ies->auc_ie, MAC_MAX_FTE_LEN, pst_fties->ie, pst_fties->ie_len) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_cfg80211_update_ft_ies::memcpy fail!");
        return -OAL_EFAIL;
    }

    l_ret = wal_send_cfg_event(pst_netdev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg80211_ft_ies_stru), (oal_uint8 *)&st_write_msg,
        OAL_TRUE, &pst_rsp_msg);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY,
                       "{wal_cfg80211_update_ft_ies::wal_send_cfg_event: return err code %d!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (ul_err_code != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_cfg80211_update_ft_ies:wal_check_and_release_msg_resp fail \
            return err code:[%u]", ul_err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
#endif  // _PRE_WLAN_FEATURE_11R
#endif  /* _PRE_CONFIG_TARGET_PRODUCT != _PRE_TARGET_PRODUCT_TYPE_E5 */
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))

oal_int32 wal_cfg80211_dump_survey(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
                                   oal_int32 l_idx, oal_survey_info_stru *pst_info)
{
#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_dump_survey::dfr_process_status[%d]!}",
                         g_st_dfr_info.bit_device_reset_process_flag);
        return -OAL_EFAIL;
    }
#endif  // #ifdef _PRE_WLAN_FEATURE_DFR
    return hmac_cfg80211_dump_survey(pst_wiphy, pst_netdev, l_idx, pst_info);
}
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
OAL_STATIC oal_void wal_cfg80211_abort_scan(oal_wiphy_stru *pst_wiphy, oal_wireless_dev_stru *pst_wdev)
{
    oal_net_device_stru *pst_netdev;

    /* 1.1 ��μ�� */
    if ((pst_wiphy == OAL_PTR_NULL) || (pst_wdev == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_abort_scan::wiphy or wdev is null!}\r\n");
        return;
    }

    pst_netdev = pst_wdev->netdev;
    if (pst_netdev == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_abort_scan::netdev is null!}\r\n");
        return;
    }
    oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_abort_scan::enter!}\r\n");
    wal_force_scan_complete(pst_netdev, OAL_TRUE);
    return;
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#ifdef _PRE_WLAN_FEATURE_SAE
/*
 * �� �� ��  : wal_cfg80211_external_auth
 * ��������  : ִ���ں��·� ext_auth ����
 */
OAL_STATIC oal_int32 wal_cfg80211_external_auth(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
    oal_cfg80211_external_auth_stru *pst_external_auth_params)
{
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    hmac_external_auth_req_stru st_ext_auth;

    memset_s(&st_ext_auth, OAL_SIZEOF(st_ext_auth), 0, OAL_SIZEOF(st_ext_auth));
    st_ext_auth.us_status = pst_external_auth_params->status;
    l_ret = memcpy_s(st_ext_auth.auc_bssid, WLAN_MAC_ADDR_LEN, pst_external_auth_params->bssid, WLAN_MAC_ADDR_LEN);
    st_ext_auth.st_ssid.uc_ssid_len = oal_min(pst_external_auth_params->ssid.ssid_len, OAL_IEEE80211_MAX_SSID_LEN);
    l_ret += memcpy_s(st_ext_auth.st_ssid.auc_ssid, OAL_IEEE80211_MAX_SSID_LEN,
                      pst_external_auth_params->ssid.ssid, st_ext_auth.st_ssid.uc_ssid_len);
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_SAE, "wal_cfg80211_external_auth::memcpy fail!");
        return -OAL_EFAIL;
    }

    oam_warning_log4(0, OAM_SF_SAE, "{wal_cfg80211_external_auth::status %d, bssid[%02X:XX:XX:XX:%02X:%02X]}",
                     st_ext_auth.us_status,
                     st_ext_auth.auc_bssid[0], /* auc_bssid��0byteΪ���������ӡ */
                     st_ext_auth.auc_bssid[4], /* auc_bssid��4byteΪ���������ӡ */
                     st_ext_auth.auc_bssid[5]); /* auc_bssid��5byteΪ���������ӡ */

    ul_ret = wal_cfg80211_do_external_auth(pst_netdev, &st_ext_auth);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_SAE, "{wal_cfg80211_external_auth::do external auth fail. ret %d}", ul_ret);
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}
#endif /* _PRE_WLAN_FEATURE_SAE */
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0) */

/* ��ͬ����ϵͳ����ָ��ṹ�巽ʽ��ͬ */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
OAL_STATIC oal_cfg80211_ops_stru g_wal_cfg80211_ops = {
    .scan = wal_cfg80211_scan,

    .connect = wal_cfg80211_connect,
    .disconnect = wal_cfg80211_disconnect,

    .add_key = wal_cfg80211_add_key,
    .get_key = wal_cfg80211_get_key,
    .del_key = wal_cfg80211_remove_key,
    .set_default_key = wal_cfg80211_set_default_key,
    .set_default_mgmt_key = wal_cfg80211_set_default_mgmt_key,

    .set_wiphy_params = wal_cfg80211_set_wiphy_params,

    .change_beacon = wal_cfg80211_change_beacon,
    .start_ap = wal_cfg80211_start_ap,
    .stop_ap = wal_cfg80211_stop_ap,
    .change_bss = wal_cfg80211_change_bss,
    .sched_scan_start = wal_cfg80211_sched_scan_start,
    .sched_scan_stop = wal_cfg80211_sched_scan_stop,

    .change_virtual_intf = wal_cfg80211_change_virtual_intf,
    .add_station = wal_cfg80211_add_station,
    .del_station = wal_cfg80211_del_station,
    .change_station = wal_cfg80211_change_station,
    .get_station = wal_cfg80211_get_station,
    .dump_station = wal_cfg80211_dump_station,
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
    .dump_survey = wal_cfg80211_dump_survey,
#endif
    .set_pmksa = wal_cfg80211_set_pmksa,
    .del_pmksa = wal_cfg80211_del_pmksa,
    .flush_pmksa = wal_cfg80211_flush_pmksa,
    .remain_on_channel = wal_cfg80211_remain_on_channel,
    .cancel_remain_on_channel = wal_cfg80211_cancel_remain_on_channel,
    .mgmt_tx = wal_cfg80211_mgmt_tx,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
    .update_mgmt_frame_registrations = wal_cfg80211_mgmt_frame_register,
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
    .mgmt_frame_register = wal_cfg80211_mgmt_frame_register,
#endif
#endif
    .set_bitrate_mask = wal_cfg80211_set_bitrate_mask,

    .add_virtual_intf = wal_cfg80211_add_virtual_intf,
    .del_virtual_intf = wal_cfg80211_del_virtual_intf,
    .mgmt_tx_cancel_wait = wal_cfg80211_mgmt_tx_cancel_wait,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
    .start_p2p_device = wal_cfg80211_start_p2p_device,
    .stop_p2p_device = wal_cfg80211_stop_p2p_device,
#endif
    .set_power_mgmt = wal_cfg80211_set_power_mgmt,
#ifdef _PRE_WLAN_FEATURE_11R
    .update_ft_ies = wal_cfg80211_update_ft_ies,
#endif  // _PRE_WLAN_FEATURE_11R
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    .abort_scan = wal_cfg80211_abort_scan,
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#ifdef _PRE_WLAN_FEATURE_SAE
    .external_auth = wal_cfg80211_external_auth,
#endif /* _PRE_WLAN_FEATURE_SAE */
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0) */
};
#elif (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)
OAL_STATIC oal_cfg80211_ops_stru g_wal_cfg80211_ops = {
    wal_cfg80211_add_key,
    wal_cfg80211_get_key,
    wal_cfg80211_remove_key,
    wal_cfg80211_set_default_key,
    wal_cfg80211_set_default_key,
    wal_cfg80211_scan,
    wal_cfg80211_connect,
    wal_cfg80211_disconnect,
    wal_cfg80211_set_channel,
    wal_cfg80211_set_wiphy_params,
    wal_cfg80211_add_beacon,
    wal_cfg80211_change_virtual_intf,
    wal_cfg80211_add_station,
    wal_cfg80211_del_station,
    wal_cfg80211_change_station,
    wal_cfg80211_get_station,
    wal_cfg80211_dump_station,
    wal_cfg80211_change_beacon,
    wal_cfg80211_start_ap,
    wal_cfg80211_stop_ap,
    wal_cfg80211_change_bss,
};

#endif


oal_void wal_cfg80211_reset_bands(oal_void)
{
    int i;

    /* ÿ�θ��¹�����,flags���ᱻ�޸�,���ϴ��޸ĵ�ֵ���ᱻ���,�൱��ÿ���޸ĵĹ����붼����Ч,��˸��¹�����Ҫ���flag��־ */
    for (i = 0; i < g_hi1151_band_2ghz.n_channels; i++) {
        g_hi1151_band_2ghz.channels[i].flags = 0;
    }

    if (mac_get_band_5g_enabled()) {
        for (i = 0; i < g_hi1151_band_5ghz.n_channels; i++) {
            g_hi1151_band_5ghz.channels[i].flags = 0;
        }
    }
}


oal_void wal_cfg80211_save_bands(oal_void)
{
    int i;

    /* ÿ�θ��¹�����,flags���ᱻ�޸�,���ϴ��޸ĵ�ֵ���ᱻ���,
       �൱��ÿ���޸ĵĹ����붼����Ч,��˸��¹�����Ҫ���flag��־��
       ÿ�����ù�����flag �󣬱��浱ǰ���õ�orig_flags��
 */
    for (i = 0; i < g_hi1151_band_2ghz.n_channels; i++) {
        g_hi1151_band_2ghz.channels[i].orig_flags = g_hi1151_band_2ghz.channels[i].flags;
    }

    if (mac_get_band_5g_enabled()) {
        for (i = 0; i < g_hi1151_band_5ghz.n_channels; i++) {
            g_hi1151_band_5ghz.channels[i].orig_flags = g_hi1151_band_5ghz.channels[i].flags;
        }
    }
}
OAL_STATIC oal_void wal_wiphy_p2p_init(oal_wiphy_stru *pst_wiphy)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
#ifdef _PRE_WLAN_FEATURE_P2P
    pst_wiphy->iface_combinations = g_sta_p2p_iface_combinations;
    pst_wiphy->n_iface_combinations = (oal_int32)oal_array_size(g_sta_p2p_iface_combinations);
    pst_wiphy->mgmt_stypes = g_wal_cfg80211_default_mgmt_stypes;
    pst_wiphy->max_remain_on_channel_duration = 5000; /* RTW_MAX_REMAIN_ON_CHANNEL_DURATION ֵ�� 5000ms */
    /* ʹ���������� */
    pst_wiphy->flags |= WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL | WIPHY_FLAG_OFFCHAN_TX;
    /*  ���GO Beacon registerʧ������ */
    pst_wiphy->flags |= WIPHY_FLAG_HAVE_AP_SME; /* ����GO ����λ */
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    /* 1102ע��֧��pno����ɨ�����������Ϣ */
    pst_wiphy->max_sched_scan_ssids = MAX_PNO_SSID_COUNT;
    pst_wiphy->max_match_sets = MAX_PNO_SSID_COUNT;
    pst_wiphy->max_sched_scan_ie_len = WAL_MAX_SCAN_IE_LEN;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))
    pst_wiphy->max_sched_scan_reqs = 1;
#else
    pst_wiphy->flags |= WIPHY_FLAG_SUPPORTS_SCHED_SCAN;
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44) */
#endif /* _PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST */

#else  /* ��p2p���� */
    pst_wiphy->iface_combinations = g_ap_dbac_iface_combinations;
    pst_wiphy->n_iface_combinations = oal_array_size(g_ap_dbac_iface_combinations);
#endif /* _PRE_WLAN_FEATURE_P2P */
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44) */
#endif /* _PRE_OS_VERSION_LINUX == _PRE_OS_VERSION */
}
static void wal_cfg80211_init_wiphy_feature(oal_wiphy_stru *wiphy)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
#ifdef _PRE_WLAN_FEATURE_SAE
    wiphy->features |= NL80211_FEATURE_SAE; /* ����֧��SAE ���� */
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0))
    oal_wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_LOW_POWER_SCAN);
#endif
    wiphy->features |= NL80211_FEATURE_SCAN_RANDOM_MAC_ADDR;
    wiphy->features |= NL80211_FEATURE_SCHED_SCAN_RANDOM_MAC_ADDR;
#endif
}
OAL_STATIC oal_void wal_wiphy_init(oal_wiphy_stru *pst_wiphy)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    pst_wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION) | BIT(NL80211_IFTYPE_AP)
                                 | BIT(NL80211_IFTYPE_P2P_CLIENT)
                                 | BIT(NL80211_IFTYPE_P2P_GO)
                                 | BIT(NL80211_IFTYPE_P2P_DEVICE);
#else
    pst_wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION) | BIT(NL80211_IFTYPE_AP);
#endif

    wal_wiphy_p2p_init(pst_wiphy);
    pst_wiphy->max_scan_ssids = WLAN_SCAN_REQ_MAX_SSID;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
    pst_wiphy->max_scan_ie_len = WAL_MAX_SCAN_IE_LEN;
    pst_wiphy->cipher_suites = g_ast_wlan_supported_cipher_suites;
    pst_wiphy->n_cipher_suites = (oal_int32)(sizeof(g_ast_wlan_supported_cipher_suites) / sizeof(oal_uint32));

    /* ��ʹ�ܽ��� */
    pst_wiphy->flags &= ~WIPHY_FLAG_PS_ON_BY_DEFAULT;

#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)) */

    /* BEGIN  roaming failure between different encryption methods ap. */
#if defined(_PRE_WLAN_FEATURE_ROAM) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
    /* wifi �����ϱ�֧��FW_ROAM,����ʱ(cfg80211_connect)��ʹ��bssid_hint ���bssid�� */
    pst_wiphy->flags |= WIPHY_FLAG_SUPPORTS_FW_ROAM;
#endif
    /* END  roaming failure between different encryption methods ap. */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
    /* linux 3.14 �汾�����������������޸� */
    pst_wiphy->regulatory_flags |= REGULATORY_CUSTOM_REG;
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
    /* ���������� */
    pst_wiphy->flags |= WIPHY_FLAG_CUSTOM_REGULATORY;
#else
    /* linux-2.6.30  ���������� */
    pst_wiphy->custom_regulatory |= WIPHY_FLAG_CUSTOM_REGULATORY;
#endif
    wal_cfg80211_init_wiphy_feature(pst_wiphy);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
    pst_wiphy->bands[NL80211_BAND_2GHZ] = &g_hi1151_band_2ghz; /* ֧�ֵ�Ƶ����Ϣ 2.4G */
    if (mac_get_band_5g_enabled()) {
        pst_wiphy->bands[NL80211_BAND_5GHZ] = &g_hi1151_band_5ghz; /* ֧�ֵ�Ƶ����Ϣ 5G */
    }
#else
    pst_wiphy->bands[IEEE80211_BAND_2GHZ] = &g_hi1151_band_2ghz; /* ֧�ֵ�Ƶ����Ϣ 2.4G */
    if (mac_get_band_5g_enabled()) {
        pst_wiphy->bands[IEEE80211_BAND_5GHZ] = &g_hi1151_band_5ghz; /* ֧�ֵ�Ƶ����Ϣ 5G */
    }
#endif
    pst_wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;

    oal_wiphy_apply_custom_regulatory(pst_wiphy, &g_st_default_regdom);
}

oal_uint32 wal_cfg80211_init(oal_void)
{
    oal_uint32 ul_chip;
    oal_uint8 uc_device;
    oal_int32 l_return;
    oal_uint8 uc_dev_id;
    mac_device_stru *pst_device = OAL_PTR_NULL;
    oal_uint32 ul_chip_max_num;
    mac_board_stru *pst_hmac_board = OAL_PTR_NULL;
    oal_wiphy_stru *pst_wiphy = OAL_PTR_NULL;
    mac_wiphy_priv_stru *pst_wiphy_priv = OAL_PTR_NULL;

    hmac_board_get_instance(&pst_hmac_board);

    ul_chip_max_num = oal_bus_get_chip_num();

    for (ul_chip = 0; ul_chip < ul_chip_max_num; ul_chip++) {
        for (uc_device = 0; uc_device < pst_hmac_board->ast_chip[ul_chip].uc_device_nums; uc_device++) {
            /* ��ȡdevice_id */
            uc_dev_id = pst_hmac_board->ast_chip[ul_chip].auc_device_id[uc_device];

            pst_device = mac_res_get_dev(uc_dev_id);
            if (oal_unlikely(pst_device == OAL_PTR_NULL)) {
                OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_init::mac_res_get_dev,pst_dev null!}\r\n");
                return OAL_FAIL;
            }

            pst_device->pst_wiphy = oal_wiphy_new(&g_wal_cfg80211_ops, OAL_SIZEOF(mac_wiphy_priv_stru));

            if (pst_device->pst_wiphy == OAL_PTR_NULL) {
                OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_init::oal_wiphy_new failed!}\r\n");
                return OAL_FAIL;
            }

            /* ��ʼ��wiphy �ṹ������ */
            pst_wiphy = pst_device->pst_wiphy;
            wal_wiphy_init(pst_wiphy);
#if (defined(_PRE_PRODUCT_ID_HI110X_HOST) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))) && \
    (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
            wal_cfgvendor_init(pst_wiphy);
#endif

            OAL_IO_PRINT("wiphy_register start.\n");
            l_return = oal_wiphy_register(pst_wiphy);
            if (l_return != 0) {
                oal_wiphy_free(pst_device->pst_wiphy);
                OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_init::oal_wiphy_register failed!}\r\n");
                return (oal_uint32)l_return;
            }

            /* P2P add_virtual_intf ����wiphy ��������wiphy priv ָ�뱣��wifi ����mac_devie_stru �ṹָ�� */
            pst_wiphy_priv = (mac_wiphy_priv_stru *)(oal_wiphy_priv(pst_wiphy));
            pst_wiphy_priv->pst_mac_device = pst_device;

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
            OAL_IO_PRINT("wal_init_wlan_netdev %s.\n", wal_get_wlan_name());

#if defined(CONFIG_ARCH_QCOM)
            l_return = wal_init_wlan_netdev(pst_wiphy, wal_get_wlan_name());
#else
            l_return = wal_init_wlan_netdev(pst_wiphy, "wlan0");
#endif
            if (l_return != OAL_SUCC) {
                OAL_IO_PRINT("wal_init_wlan_netdev wlanx failed.l_return:%d\n", l_return);
                return (oal_uint32)l_return;
            }
            // limit 1102a P2P function
#if !defined(CONFIG_ARCH_QCOM)
            OAL_IO_PRINT("wal_init_wlan_netdev %s.\n", wal_get_p2p_name());
            l_return = wal_init_wlan_netdev(pst_wiphy, wal_get_p2p_name());
            if (l_return != OAL_SUCC) {
                OAL_IO_PRINT("wal_init_wlan_netdev p2p0 failed.l_return:%d\n", l_return);

                /* �ͷ�wlan0�����豸��Դ */
                oal_mem_free_m(oal_netdevice_wdev(pst_device->st_p2p_info.pst_primary_net_device), OAL_TRUE);
                oal_net_unregister_netdev(pst_device->st_p2p_info.pst_primary_net_device);
                return (oal_uint32)l_return;
            }
#endif
#endif
#ifdef _PRE_WLAN_CHBA_MGMT
            /* ע��chba device */
            l_return = wal_init_chba_netdev(pst_wiphy, "chba0");
            if (l_return != OAL_SUCC) {
                OAL_IO_PRINT("wal_init_chba_netdev chba failed.l_return:%d\n", l_return);
                oal_mem_free_m(oal_netdevice_wdev(pst_device->st_p2p_info.pst_p2p_net_device), OAL_TRUE);
                oal_net_unregister_netdev(pst_device->st_p2p_info.pst_p2p_net_device);
                oal_mem_free_m(oal_netdevice_wdev(pst_device->st_p2p_info.pst_primary_net_device), OAL_TRUE);
                oal_net_unregister_netdev(pst_device->st_p2p_info.pst_primary_net_device);
                return (uint32_t)l_return;
            }
#endif
        }
    }

    return OAL_SUCC;
}


oal_void wal_cfg80211_exit(oal_void)
{
    oal_uint32 ul_chip;
    oal_uint8 uc_device;
    oal_uint8 uc_dev_id;
    mac_device_stru *pst_device = OAL_PTR_NULL;
    oal_uint32 ul_chip_max_num;
    mac_board_stru *pst_hmac_board = OAL_PTR_NULL;

    hmac_board_get_instance(&pst_hmac_board);

    ul_chip_max_num = oal_bus_get_chip_num(); /* ����ط���ȷ�� */

    for (ul_chip = 0; ul_chip < ul_chip_max_num; ul_chip++) {
        for (uc_device = 0; uc_device < pst_hmac_board->ast_chip[ul_chip].uc_device_nums; uc_device++) {
            /* ��ȡdevice_id */
            uc_dev_id = pst_hmac_board->ast_chip[ul_chip].auc_device_id[uc_device];

            pst_device = mac_res_get_dev(uc_dev_id);
            if (pst_device == OAL_PTR_NULL) {
                OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_init::mac_res_get_dev pst_device is null!}\r\n");
                return;
            }
#if (defined(_PRE_PRODUCT_ID_HI110X_HOST) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))) && \
    (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
            wal_cfgvendor_deinit(pst_device->pst_wiphy);
#endif

            /* ע��ע�� wiphy device */
            oal_wiphy_unregister(pst_device->pst_wiphy);

            /* ж��wiphy device */
            oal_wiphy_free(pst_device->pst_wiphy);
        }
    }

    return;
}


oal_uint32 wal_cfg80211_init_evt_handle(frw_event_mem_stru *pst_event_mem)
{
    oal_uint32 ul_ret;

    ul_ret = wal_cfg80211_init();
    if (ul_ret != OAL_SUCC) {
        return OAL_SUCC;
    }
    return OAL_SUCC;
}
