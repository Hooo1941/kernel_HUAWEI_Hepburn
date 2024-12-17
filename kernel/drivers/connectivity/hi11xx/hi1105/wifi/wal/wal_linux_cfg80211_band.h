/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : wal_linux_cfg80211_band.c 的头文件
 * 作    者 :
 * 创建日期 : 2022年10月13日
 */

#ifndef WAL_LINUX_CFG80211_BAND_H
#define WAL_LINUX_CFG80211_BAND_H

#include "oal_ext_if.h"
#include "oal_types.h"
#include "oal_net.h"

/* 802.11n HT 能力掩码 */
#define IEEE80211_HT_CAP_LDPC_CODING      0x0001
#define IEEE80211_HT_CAP_SUP_WIDTH_20_40  0x0002
#define IEEE80211_HT_CAP_SM_PS            0x000C
#define IEEE80211_HT_CAP_SM_PS_SHIFT      2
#define IEEE80211_HT_CAP_GRN_FLD          0x0010
#define IEEE80211_HT_CAP_SGI_20           0x0020
#define IEEE80211_HT_CAP_SGI_40           0x0040
#define IEEE80211_HT_CAP_TX_STBC          0x0080
#define IEEE80211_HT_CAP_RX_STBC          0x0300
#define IEEE80211_HT_CAP_DELAY_BA         0x0400
#define IEEE80211_HT_CAP_MAX_AMSDU        0x0800
#define IEEE80211_HT_CAP_DSSSCCK40        0x1000
#define IEEE80211_HT_CAP_RESERVED         0x2000
#define IEEE80211_HT_CAP_40MHZ_INTOLERANT 0x4000
#define IEEE80211_HT_CAP_LSIG_TXOP_PROT   0x8000

/* 802.11ac VHT Capabilities */
#define IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_3895             0x00000000
#define IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_7991             0x00000001
#define IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_11454            0x00000002
#define IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ           0x00000004
#define IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160_80PLUS80MHZ  0x00000008
#define IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_MASK             0x0000000C
#define IEEE80211_VHT_CAP_RXLDPC                           0x00000010
#define IEEE80211_VHT_CAP_SHORT_GI_80                      0x00000020
#define IEEE80211_VHT_CAP_SHORT_GI_160                     0x00000040
#define IEEE80211_VHT_CAP_TXSTBC                           0x00000080
#define IEEE80211_VHT_CAP_RXSTBC_1                         0x00000100
#define IEEE80211_VHT_CAP_RXSTBC_2                         0x00000200
#define IEEE80211_VHT_CAP_RXSTBC_3                         0x00000300
#define IEEE80211_VHT_CAP_RXSTBC_4                         0x00000400
#define IEEE80211_VHT_CAP_RXSTBC_MASK                      0x00000700
#define IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE            0x00000800
#define IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE            0x00001000
#define IEEE80211_VHT_CAP_BEAMFORMER_ANTENNAS_MAX          0x00006000
#define IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_MAX          0x00030000
#define IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE            0x00080000
#define IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE            0x00100000
#define IEEE80211_VHT_CAP_VHT_TXOP_PS                      0x00200000
#define IEEE80211_VHT_CAP_HTC_VHT                          0x00400000
#define IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_SHIFT 23
#define IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK  \
    (7 << IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_SHIFT)
#define IEEE80211_VHT_CAP_VHT_LINK_ADAPTATION_VHT_UNSOL_MFB 0x08000000
#define IEEE80211_VHT_CAP_VHT_LINK_ADAPTATION_VHT_MRQ_MFB   0x0c000000
#define IEEE80211_VHT_CAP_RX_ANTENNA_PATTERN                0x10000000
#define IEEE80211_VHT_CAP_TX_ANTENNA_PATTERN                0x20000000

/* HE 6 GHz band capabilities */
/* uses enum ieee80211_min_mpdu_spacing values */
#define IEEE80211_HE_6GHZ_CAP_MIN_MPDU_START	0x0007
/* uses enum ieee80211_vht_max_ampdu_length_exp values */
#define IEEE80211_HE_6GHZ_CAP_MAX_AMPDU_LEN_EXP	0x0038
/* uses IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_* values */
#define IEEE80211_HE_6GHZ_CAP_MAX_MPDU_LEN	0x00c0
/* WLAN_HT_CAP_SM_PS_* values */
#define IEEE80211_HE_6GHZ_CAP_SM_PS		0x0600
#define IEEE80211_HE_6GHZ_CAP_RD_RESPONDER	0x0800
#define IEEE80211_HE_6GHZ_CAP_RX_ANTPAT_CONS	0x1000
#define IEEE80211_HE_6GHZ_CAP_TX_ANTPAT_CONS	0x2000

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) || (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#define ratetab_ent(_rate, _rateid, _flags) \
    {                                       \
        .bitrate = (_rate),                 \
        .hw_value = (_rateid),              \
        .flags = (_flags),                  \
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
#define chan2g(_channel, _freq, _flags) \
    {                                   \
        .band = NL80211_BAND_2GHZ,      \
        .center_freq = (_freq),         \
        .hw_value = (_channel),         \
        .flags = (_flags),              \
        .max_antenna_gain = 0,          \
        .max_power = 30,                \
    }

#define chan5g(_channel, _flags)                \
    {                                           \
        .band = NL80211_BAND_5GHZ,              \
        .center_freq = 5000 + (5 * (_channel)), \
        .hw_value = (_channel),                 \
        .flags = (_flags),                      \
        .max_antenna_gain = 0,                  \
        .max_power = 30,                        \
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 43))
#define chan6g(_channel, _flags)         \
    {                                           \
        .band = NL80211_BAND_6GHZ,              \
        .center_freq = 5950 + (5 * (_channel)), \
        .hw_value = (_channel),                 \
        .flags = (_flags),                      \
        .max_antenna_gain = 0,                  \
        .max_power = 30,                        \
    }
#endif

#define chan4_9g(_channel, _flags)              \
    {                                           \
        .band = NL80211_BAND_5GHZ,              \
        .center_freq = 4000 + (5 * (_channel)), \
        .hw_value = (_channel),                 \
        .flags = (_flags),                      \
        .max_antenna_gain = 0,                  \
        .max_power = 30,                        \
    }
#else
#define chan2g(_channel, _freq, _flags) \
    {                                   \
        .band = IEEE80211_BAND_2GHZ,    \
        .center_freq = (_freq),         \
        .hw_value = (_channel),         \
        .flags = (_flags),              \
        .max_antenna_gain = 0,          \
        .max_power = 30,                \
    }

#define chan5g(_channel, _flags)                \
    {                                           \
        .band = IEEE80211_BAND_5GHZ,            \
        .center_freq = 5000 + (5 * (_channel)), \
        .hw_value = (_channel),                 \
        .flags = (_flags),                      \
        .max_antenna_gain = 0,                  \
        .max_power = 30,                        \
    }

#define chan4_9g(_channel, _flags)              \
    {                                           \
        .band = IEEE80211_BAND_5GHZ,            \
        .center_freq = 4000 + (5 * (_channel)), \
        .hw_value = (_channel),                 \
        .flags = (_flags),                      \
        .max_antenna_gain = 0,                  \
        .max_power = 30,                        \
    }
#endif

#elif (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)

#define ratetab_ent(_rate, _rateid, _flags) \
    {                                       \
        (_flags),                           \
            (_rate),                        \
            (_rateid),                      \
    }

#define chan2g(_channel, _freq, _flags) \
    {                                   \
        IEEE80211_BAND_2GHZ,            \
            (_freq),                    \
            (_channel),                 \
            (_flags),                   \
            0,                          \
            30,                         \
    }

#define chan5g(_channel, _flags)     \
    {                                \
        IEEE80211_BAND_5GHZ,         \
            5000 + (5 * (_channel)), \
            (_channel),              \
            (_flags),                \
            0,                       \
            30,                      \
    }

#define chan4_9g(_channel, _flags)   \
    {                                \
        IEEE80211_BAND_5GHZ,         \
            4000 + (5 * (_channel)), \
            (_channel),              \
            (_flags),                \
            0,                       \
            30,                      \
    }

#else
error "WRONG OS VERSION"
#endif

extern oal_ieee80211_supported_band g_st_supported_band_2ghz_info;
extern oal_ieee80211_supported_band g_st_supported_band_5ghz_info;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 43))
extern oal_ieee80211_supported_band g_st_supported_band_6ghz_info;
#endif

#endif /* end of wal_linux_cfg80211_band.h */
