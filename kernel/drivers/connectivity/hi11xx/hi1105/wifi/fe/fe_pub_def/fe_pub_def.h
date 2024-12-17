

#ifndef FE_PUB_DEF_H
#define FE_PUB_DEF_H
#if defined(_PRE_FE_IS_DEVICE)
#include "fe_base_type.h"
#endif
// 前端支持的band情况的宏定义
#define FE_MAX_SUPPORT_2G  1    // 只支持2G
#define FE_MAX_SUPPORT_5G  2    // 支持2/5G
#define FE_MAX_SUPPORT_6G  3    // 支持2/5/6G
#define FE_ONLY_SUPPORT_5G 4    // 只支持5G

typedef enum {
    FE_BANDWIDTH_20M = 0,                       /* 20MHz全带宽 */
    FE_BANDWIDTH_40M = 1,                       /* 40MHz全带宽 */
    FE_BANDWIDTH_2G_BUTT = 2,                   /* 2G只有40M带宽 */
    FE_BANDWIDTH_80M = FE_BANDWIDTH_2G_BUTT,    /* 80MHz全带宽 */
    FE_BANDWIDTH_160M = 3,                      /* 160MHz全带宽 */
    FE_BANDWIDTH_BUTT
} fe_bandwidth_type_enum;
typedef enum {
    FE_CALI_BAND_2G        = 0,
    FE_CALI_BAND_5G        = 1,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    FE_CALI_BAND_6G        = 2,
#endif
    FE_CALI_BAND_BUTT
}fe_cali_band_enum;

typedef enum {
    FE_CHAIN_0 = 0,  /* C0 */
    FE_CHAIN_1 = 1,  /* C1 */
    FE_CHAIN_2 = 2,  /* C2 */
    FE_CHAIN_3 = 3,  /* C3 */
    FE_CHAIN_TYPE_BUTT = 4, /* 代表无效的通道类型，规格宏请采用FE_CHAIN_NUMS */
    FE_CHAIN_BUTT = 4, /* 代表无效的通道类型，规格宏请采用FE_CHAIN_NUMS */
} fe_chain_type_enum;
#if defined(_PRE_FE_IS_DEVICE)
typedef enum {
    FE_ABB_CALI                = 0,
    FE_CALI_ABBRC           = 1,
    FE_CALI_RC              = 2,
    FE_CALI_R               = 3,
    FE_CALI_C               = 4,
    FE_CALI_PPF             = 5,
    FE_CALI_LOGEN_SSB       = 6,
    FE_CALI_LO_BF           = 7,
    FE_CALI_LO_INVOLT_SWING = 8,
    FE_CALI_PA_CURRENT      = 9,
    FE_CALI_LNA_BLK_DCOC    = 10,
    FE_CALI_RX_DC           = 11,
    FE_CALI_MIMO_RX_DC      = 12,
    FE_CALI_DPD_RX_DC       = 13,
    FE_CALI_TX_PWR          = 14,
    FE_CALI_TX_LO           = 15,
    FE_CALI_MIMO_TX_LO      = 16,
    FE_CALI_TX_IQ           = 17,
    FE_CALI_MIMO_TX_IQ      = 18,
    FE_CALI_RX_IQ           = 19,
    FE_CALI_MIMO_RX_IQ      = 20,
    FE_CALI_IIP2            = 21,
    FE_CALI_DPD             = 22,
    FE_CALI_DFS_RX_DC       = 23,
    FE_CALI_LP_RX_DC        = 24,
    FE_CALI_TYPE_BUTT,

    FE_CALI_CHANNEL_START    = 31,
    FE_CALI_CHANNEL_BUTT
} fe_cali_type_enum;
#endif
#endif