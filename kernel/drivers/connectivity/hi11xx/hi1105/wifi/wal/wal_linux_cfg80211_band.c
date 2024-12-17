/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : wal_linux_cfg80211_band.c 的头文件
 * 作    者 :
 * 创建日期 : 2022年10月13日
 */

#include "oal_cfg80211.h"
#include "wal_linux_cfg80211_band.h"

#define HI1151_A_RATES      (g_hi1151_rates + 4)
#define HI1151_A_RATES_SIZE 8
#define HI1151_G_RATES      (g_hi1151_rates + 0)
#define HI1151_G_RATES_SIZE 12

/* 设备支持的速率 */
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

/* 2.4G 频段 */
OAL_STATIC oal_ieee80211_channel g_ast_supported_channels_2ghz_info[] = {
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

/* 5G 频段 */
OAL_STATIC oal_ieee80211_channel g_ast_supported_channels_5ghz_info[] = {
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

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 43))
/* 6G 频段 */
OAL_STATIC oal_ieee80211_channel g_ast_supported_channels_6ghz_info[] = {
    chan6g(1, 0),
    chan6g(5, 0),
    chan6g(9, 0),
    chan6g(13, 0),
    chan6g(17, 0),
    chan6g(21, 0),
    chan6g(25, 0),
    chan6g(29, 0),
    chan6g(33, 0),
    chan6g(37, 0),
    chan6g(41, 0),
    chan6g(45, 0),
    chan6g(49, 0),
    chan6g(53, 0),
    chan6g(57, 0),
    chan6g(61, 0),
    chan6g(65, 0),
    chan6g(69, 0),
    chan6g(73, 0),
    chan6g(77, 0),
    chan6g(81, 0),
    chan6g(85, 0),
    chan6g(89, 0),
    chan6g(93, 0),
    chan6g(97, 0),
    chan6g(101, 0),
    chan6g(105, 0),
    chan6g(109, 0),
    chan6g(113, 0),
    chan6g(117, 0),
    chan6g(121, 0),
    chan6g(125, 0),
    chan6g(129, 0),
    chan6g(133, 0),
    chan6g(137, 0),
    chan6g(141, 0),
    chan6g(145, 0),
    chan6g(149, 0),
    chan6g(153, 0),
    chan6g(157, 0),
    chan6g(161, 0),
    chan6g(165, 0),
    chan6g(169, 0),
    chan6g(173, 0),
    chan6g(177, 0),
    chan6g(181, 0),
    chan6g(185, 0),
    chan6g(189, 0),
    chan6g(193, 0),
    chan6g(197, 0),
    chan6g(201, 0),
    chan6g(205, 0),
    chan6g(209, 0),
    chan6g(213, 0),
    chan6g(217, 0),
    chan6g(221, 0),
    chan6g(225, 0),
    chan6g(229, 0),
    chan6g(233, 0),
};
#endif

/* 2.4G 频段信息 */
oal_ieee80211_supported_band g_st_supported_band_2ghz_info = {
    .channels = g_ast_supported_channels_2ghz_info,
    .n_channels = sizeof(g_ast_supported_channels_2ghz_info) / sizeof(oal_ieee80211_channel),
    .bitrates = HI1151_G_RATES,
    .n_bitrates = HI1151_G_RATES_SIZE,
    .ht_cap = {
        .ht_supported = OAL_TRUE,
        .cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 | IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40,
    },
};

/* 5G 频段信息 */
oal_ieee80211_supported_band g_st_supported_band_5ghz_info = {
    .channels = g_ast_supported_channels_5ghz_info,
    .n_channels = sizeof(g_ast_supported_channels_5ghz_info) / sizeof(oal_ieee80211_channel),
    .bitrates = HI1151_A_RATES,
    .n_bitrates = HI1151_A_RATES_SIZE,
    .ht_cap = {
        .ht_supported = OAL_TRUE,
        .cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 | IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40,
    },
    .vht_cap = {
        .vht_supported = OAL_TRUE,
        .cap = IEEE80211_VHT_CAP_SHORT_GI_80 | IEEE80211_VHT_CAP_HTC_VHT |
               IEEE80211_VHT_CAP_SHORT_GI_160 | IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ,
    },
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 43))
/* 6G 频段 iftype_data初始化 */
struct ieee80211_sband_iftype_data g_sband_iftype_data_info[] = {
    {
        .types_mask = BIT(NL80211_IFTYPE_STATION),
        .he_cap = {
            .has_he = OAL_TRUE,
        },
        .he_6ghz_capa = {
            .capa = OAL_TRUE,
        },
    },
    {
        .types_mask = BIT(NL80211_IFTYPE_AP),
        .he_cap = {
            .has_he = OAL_TRUE,
        },
        .he_6ghz_capa = {
            .capa = IEEE80211_HE_6GHZ_CAP_MIN_MPDU_START | IEEE80211_HE_6GHZ_CAP_MAX_AMPDU_LEN_EXP |
                    IEEE80211_HE_6GHZ_CAP_MAX_MPDU_LEN | IEEE80211_HE_6GHZ_CAP_SM_PS |
                    IEEE80211_HE_6GHZ_CAP_RD_RESPONDER | IEEE80211_HE_6GHZ_CAP_RX_ANTPAT_CONS |
                    IEEE80211_HE_6GHZ_CAP_TX_ANTPAT_CONS,
        },
    },
};

/* 6G 频段信息 */
oal_ieee80211_supported_band g_st_supported_band_6ghz_info = {
    .channels = g_ast_supported_channels_6ghz_info,
    .n_channels = sizeof(g_ast_supported_channels_6ghz_info) / sizeof(oal_ieee80211_channel),
    .bitrates = HI1151_A_RATES,
    .n_bitrates = HI1151_A_RATES_SIZE,
    /* 当前暂适配STA和AP两种模式 */
    .n_iftype_data = sizeof(g_sband_iftype_data_info) / sizeof (struct ieee80211_sband_iftype_data),
    .iftype_data = g_sband_iftype_data_info,
};
#endif
