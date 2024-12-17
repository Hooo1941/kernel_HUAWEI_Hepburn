/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : WIFI 资源限制实现
 * 作    者 :
 * 创建日期 : 2023年2月1日
 */

#include "hmac_resource_restriction.h"
#include "mac_device.h"
#include "mac_resource.h"
#include "wlan_chip.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_RESOURCE_RESTRICTION_C

/*
 * 每一种vap模式转化为响应的bit标记枚举， 当前支持最大规格为3vap共存, 可能存在多种相同vap共存
 * 共存bitmap含义：bit0:legacy ap; bit1:legacy sta; bit2:go; bit3: gc; bit4: chba; bit5:legacy ap2; bit6: legacy sta2;
 * bit7:go2; bit8:gc2; bit9:chba2 以此类推
 */
typedef enum {
    MAC_VAP_MODE_LEGACY_AP = BIT0,
    MAC_VAP_MODE_LEGACY_STA = BIT1,
    MAC_VAP_MODE_P2P_GO = BIT2,
    MAC_VAP_MODE_P2P_GC = BIT3,
    MAC_VAP_MODE_CHBA = BIT4,

    MAC_VAP_MODE_LEGACY_AP2 = BIT5,
    MAC_VAP_MODE_LEGACY_STA2 = BIT6,
    MAC_VAP_MODE_P2P_GO2 = BIT7,
    MAC_VAP_MODE_P2P_GC2 = BIT8,
    MAC_VAP_MODE_CHBA2 = BIT9,

    MAC_VAP_MODE_LEGACY_AP3 = BIT10,
    MAC_VAP_MODE_LEGACY_STA3 = BIT11,
    MAC_VAP_MODE_P2P_GO3 = BIT12,
    MAC_VAP_MODE_P2P_GC3 = BIT13,
    MAC_VAP_MODE_CHBA3 = BIT14,
    MAC_VAP_MODE_BUTT = BIT15,
} mac_vap_mode_bitmap_enum;

/* 当前支持的所有vap共存模式bitmap枚举 */
typedef enum {
    MAC_VAP_COEX_BITMAP_LEGACY_AP_STA = (MAC_VAP_MODE_LEGACY_AP | MAC_VAP_MODE_LEGACY_STA),
    MAC_VAP_COEX_BITMAP_LEGACY_AP_GO = (MAC_VAP_MODE_LEGACY_AP | MAC_VAP_MODE_P2P_GO),
    MAC_VAP_COEX_BITMAP_LEGACY_AP_GC = (MAC_VAP_MODE_LEGACY_AP | MAC_VAP_MODE_P2P_GC),
    MAC_VAP_COEX_BITMAP_LEGACY_AP_CHBA = (MAC_VAP_MODE_LEGACY_AP | MAC_VAP_MODE_CHBA),
    MAC_VAP_COEX_BITMAP_DUAL_STA = (MAC_VAP_MODE_LEGACY_STA | MAC_VAP_MODE_LEGACY_STA2),
    MAC_VAP_COEX_BITMAP_LEGACY_STA_GO = (MAC_VAP_MODE_LEGACY_STA | MAC_VAP_MODE_P2P_GO),
    MAC_VAP_COEX_BITMAP_LEGACY_STA_GC = (MAC_VAP_MODE_LEGACY_STA | MAC_VAP_MODE_P2P_GC),
    MAC_VAP_COEX_BITMAP_LEGACY_STA_CHBA = (MAC_VAP_MODE_LEGACY_STA | MAC_VAP_MODE_CHBA),
    MAC_VAP_COEX_BITMAP_GO_GO = (MAC_VAP_MODE_P2P_GO | MAC_VAP_MODE_P2P_GO2),
    MAC_VAP_COEX_BITMAP_GO_GC = (MAC_VAP_MODE_P2P_GO | MAC_VAP_MODE_P2P_GC),
    MAC_VAP_COEX_BITMAP_GO_CHBA = (MAC_VAP_MODE_P2P_GO | MAC_VAP_MODE_CHBA),
    MAC_VAP_COEX_BITMAP_GC_GC = (MAC_VAP_MODE_P2P_GC | MAC_VAP_MODE_P2P_GC2),
    MAC_VAP_COEX_BITMAP_GC_CHBA = (MAC_VAP_MODE_P2P_GC | MAC_VAP_MODE_CHBA),
    MAC_VAP_COEX_BITMAP_LEGACY_STA_GO_GO = (MAC_VAP_MODE_LEGACY_STA | MAC_VAP_MODE_P2P_GO | MAC_VAP_MODE_P2P_GO2),
    MAC_VAP_COEX_BITMAP_LEGACY_STA_GO_GC = (MAC_VAP_MODE_LEGACY_STA | MAC_VAP_MODE_P2P_GO | MAC_VAP_MODE_P2P_GC),
    MAC_VAP_COEX_BITMAP_LEGACY_STA_GO_CHBA = (MAC_VAP_MODE_LEGACY_STA | MAC_VAP_MODE_P2P_GO | MAC_VAP_MODE_CHBA),
    MAC_VAP_COEX_BITMAP_LEGACY_STA_GC_GC = (MAC_VAP_MODE_LEGACY_STA | MAC_VAP_MODE_P2P_GC | MAC_VAP_MODE_P2P_GC2),
    MAC_VAP_COEX_BITMAP_LEGACY_STA_GC_CHBA = (MAC_VAP_MODE_LEGACY_STA | MAC_VAP_MODE_P2P_GC | MAC_VAP_MODE_CHBA),
} mac_mvap_coex_mode_bitmap_enum;

/* 多vap共存类型枚举，共存规格表下标值 */
typedef enum {
    HMAC_VAP_COEX_TYPE_LEGACY_AP_STA = 0,
    HMAC_VAP_COEX_TYPE_LEGACY_AP_GO = 1,
    HMAC_VAP_COEX_TYPE_LEGACY_AP_GC = 2,
    HMAC_VAP_COEX_TYPE_LEGACY_AP_CHBA = 3,
    HMAC_VAP_COEX_TYPE_DUAL_STA = 4,
    HMAC_VAP_COEX_TYPE_LEGACY_STA_GO = 5,
    HMAC_VAP_COEX_TYPE_LEGACY_STA_GC = 6,
    HMAC_VAP_COEX_TYPE_LEGACY_STA_CHBA = 7,
    HMAC_VAP_COEX_TYPE_GO_GO = 8,
    HMAC_VAP_COEX_TYPE_GO_GC = 9,
    HMAC_VAP_COEX_TYPE_GO_CHBA = 10,
    HMAC_VAP_COEX_TYPE_GC_GC = 11,
    HMAC_VAP_COEX_TYPE_GC_CHBA = 12,
    HMAC_VAP_COEX_TYPE_LEGACY_STA_GO_GO = 13,
    HMAC_VAP_COEX_TYPE_LEGACY_STA_GO_GC = 14,
    HMAC_VAP_COEX_TYPE_LEGACY_STA_GO_CHBA = 15,
    HMAC_VAP_COEX_TYPE_LEGACY_STA_GC_GC = 16,
    HMAC_VAP_COEX_TYPE_LEGACY_STA_GC_CHBA = 17,
    HMAC_VAP_COEX_TYPE_BUTT = 18,
} hmac_mvap_coex_type_enum;

/* 多vap共存规格芯片支持类型枚举，代表哪些芯片支持 */
typedef enum {
    HMAC_MVAP_COEX_SUPPORT_CHIP_MP13 = BIT(BOARD_VERSION_MP13),
    HMAC_MVAP_COEX_SUPPORT_CHIP_MP15 = BIT(BOARD_VERSION_MP15),
    HMAC_MVAP_COEX_SUPPORT_CHIP_MP16 = BIT(BOARD_VERSION_MP16),
    HMAC_MVAP_COEX_SUPPORT_CHIP_MP16C = BIT(BOARD_VERSION_MP16C),
    HMAC_MVAP_COEX_SUPPORT_CHIP_GF61 = BIT(BOARD_VERSION_GF61),
    HMAC_MVAP_COEX_SUPPORT_CHIP_ALL = HMAC_MVAP_COEX_SUPPORT_CHIP_MP13 | HMAC_MVAP_COEX_SUPPORT_CHIP_MP15 |
                                      HMAC_MVAP_COEX_SUPPORT_CHIP_MP16 | HMAC_MVAP_COEX_SUPPORT_CHIP_MP16C |
                                      HMAC_MVAP_COEX_SUPPORT_CHIP_GF61,
} hmac_mvap_coex_chip_type_enum;

typedef enum {
    HMAC_LEGACY_AP_STA_SUPPORT_CHIP_BITMAP = HMAC_MVAP_COEX_SUPPORT_CHIP_MP16 | HMAC_MVAP_COEX_SUPPORT_CHIP_MP16C |
                                             HMAC_MVAP_COEX_SUPPORT_CHIP_GF61,
    HMAC_LEGACY_AP_GO_SUPPORT_CHIP_BITMAP = HMAC_MVAP_COEX_SUPPORT_CHIP_MP16 | HMAC_MVAP_COEX_SUPPORT_CHIP_MP16C |
                                            HMAC_MVAP_COEX_SUPPORT_CHIP_GF61,
    HMAC_LEGACY_AP_GC_SUPPORT_CHIP_BITMAP = HMAC_MVAP_COEX_SUPPORT_CHIP_MP16 | HMAC_MVAP_COEX_SUPPORT_CHIP_MP16C |
                                            HMAC_MVAP_COEX_SUPPORT_CHIP_GF61,
    HMAC_LEGACY_AP_CHBA_SUPPORT_CHIP_BITMAP = HMAC_MVAP_COEX_SUPPORT_CHIP_MP16 | HMAC_MVAP_COEX_SUPPORT_CHIP_MP16C |
                                              HMAC_MVAP_COEX_SUPPORT_CHIP_GF61,
    HMAC_DUAL_STA_SUPPORT_CHIP_BITMAP = HMAC_MVAP_COEX_SUPPORT_CHIP_ALL,
    HMAC_LEGACY_STA_GO_SUPPORT_CHIP_BITMAP = HMAC_MVAP_COEX_SUPPORT_CHIP_ALL,
    HMAC_LEGACY_STA_GC_SUPPORT_CHIP_BITMAP = HMAC_MVAP_COEX_SUPPORT_CHIP_ALL,
    HMAC_LEGACY_STA_CHBA_SUPPORT_CHIP_BITMAP = HMAC_MVAP_COEX_SUPPORT_CHIP_ALL,
    HMAC_GO_GO_SUPPORT_CHIP_BITMAP = HMAC_MVAP_COEX_SUPPORT_CHIP_MP16 | HMAC_MVAP_COEX_SUPPORT_CHIP_MP16C |
                                     HMAC_MVAP_COEX_SUPPORT_CHIP_GF61,
    HMAC_GO_GC_SUPPORT_CHIP_BITMAP = HMAC_MVAP_COEX_SUPPORT_CHIP_MP16 | HMAC_MVAP_COEX_SUPPORT_CHIP_MP16C |
                                     HMAC_MVAP_COEX_SUPPORT_CHIP_GF61,
    HMAC_GO_CHBA_SUPPORT_CHIP_BITMAP = HMAC_MVAP_COEX_SUPPORT_CHIP_ALL,
    HMAC_GC_GC_SUPPORT_CHIP_BITMAP = HMAC_MVAP_COEX_SUPPORT_CHIP_MP16 | HMAC_MVAP_COEX_SUPPORT_CHIP_MP16C |
                                     HMAC_MVAP_COEX_SUPPORT_CHIP_GF61,
    HMAC_GC_CHBA_SUPPORT_CHIP_BITMAP = HMAC_MVAP_COEX_SUPPORT_CHIP_ALL,
    HMAC_LEGACY_STA_GO_GO_SUPPORT_CHIP_BITMAP = HMAC_MVAP_COEX_SUPPORT_CHIP_MP16 | HMAC_MVAP_COEX_SUPPORT_CHIP_MP16C |
                                                HMAC_MVAP_COEX_SUPPORT_CHIP_GF61,
    HMAC_LEGACY_STA_GO_GC_SUPPORT_CHIP_BITMAP = HMAC_MVAP_COEX_SUPPORT_CHIP_MP16 | HMAC_MVAP_COEX_SUPPORT_CHIP_MP16C |
                                                  HMAC_MVAP_COEX_SUPPORT_CHIP_GF61,
    HMAC_LEGACY_STA_GO_CHBA_SUPPORT_CHIP_BITMAP = HMAC_MVAP_COEX_SUPPORT_CHIP_ALL,
    HMAC_LEGACY_STA_GC_GC_SUPPORT_CHIP_BITMAP = HMAC_MVAP_COEX_SUPPORT_CHIP_MP16 | HMAC_MVAP_COEX_SUPPORT_CHIP_MP16C |
                                                HMAC_MVAP_COEX_SUPPORT_CHIP_GF61,
    HMAC_LEGACY_STA_GC_CHBA_SUPPORT_CHIP_BITMAP = HMAC_MVAP_COEX_SUPPORT_CHIP_ALL,
} hmac_mvap_coex_support_chip_type_enum;

/* 多vap共存规格配置表结构体 */
typedef struct {
    uint32_t coex_mvap_type; /* 当前多vap共存类型 */
    uint32_t coex_mvap_bitmap; /* 当前共存模式bitmap值 */
    uint32_t support_chip_type_bitmap; /* 当前共存模式支持的芯片类型 */
} hmac_mvap_coex_cfg_table_stru;

hmac_mvap_coex_cfg_table_stru g_mvap_coex_cfg_table[HMAC_VAP_COEX_TYPE_BUTT] = {
    [HMAC_VAP_COEX_TYPE_LEGACY_AP_STA] = {
        .coex_mvap_type = HMAC_VAP_COEX_TYPE_LEGACY_AP_STA,
        .coex_mvap_bitmap = MAC_VAP_COEX_BITMAP_LEGACY_AP_STA,
        .support_chip_type_bitmap = HMAC_LEGACY_AP_STA_SUPPORT_CHIP_BITMAP,
    },
    [HMAC_VAP_COEX_TYPE_LEGACY_AP_GO] = {
        .coex_mvap_type = HMAC_VAP_COEX_TYPE_LEGACY_AP_GO,
        .coex_mvap_bitmap = MAC_VAP_COEX_BITMAP_LEGACY_AP_GO,
        .support_chip_type_bitmap = HMAC_LEGACY_AP_GO_SUPPORT_CHIP_BITMAP,
    },
    [HMAC_VAP_COEX_TYPE_LEGACY_AP_GC] = {
        .coex_mvap_type = HMAC_VAP_COEX_TYPE_LEGACY_AP_GC,
        .coex_mvap_bitmap = MAC_VAP_COEX_BITMAP_LEGACY_AP_GC,
        .support_chip_type_bitmap = HMAC_LEGACY_AP_GC_SUPPORT_CHIP_BITMAP,
    },
    [HMAC_VAP_COEX_TYPE_LEGACY_AP_CHBA] = {
        .coex_mvap_type = HMAC_VAP_COEX_TYPE_LEGACY_AP_CHBA,
        .coex_mvap_bitmap = MAC_VAP_COEX_BITMAP_LEGACY_AP_CHBA,
        .support_chip_type_bitmap = HMAC_LEGACY_AP_CHBA_SUPPORT_CHIP_BITMAP,
    },
    [HMAC_VAP_COEX_TYPE_DUAL_STA] = {
        .coex_mvap_type = HMAC_VAP_COEX_TYPE_DUAL_STA,
        .coex_mvap_bitmap = MAC_VAP_COEX_BITMAP_DUAL_STA,
        .support_chip_type_bitmap = HMAC_DUAL_STA_SUPPORT_CHIP_BITMAP,
    },
    [HMAC_VAP_COEX_TYPE_LEGACY_STA_GO] = {
        .coex_mvap_type = HMAC_VAP_COEX_TYPE_LEGACY_STA_GO,
        .coex_mvap_bitmap = MAC_VAP_COEX_BITMAP_LEGACY_STA_GO,
        .support_chip_type_bitmap = HMAC_LEGACY_STA_GO_SUPPORT_CHIP_BITMAP,
    },
    [HMAC_VAP_COEX_TYPE_LEGACY_STA_GC] = {
        .coex_mvap_type = HMAC_VAP_COEX_TYPE_LEGACY_STA_GC,
        .coex_mvap_bitmap = MAC_VAP_COEX_BITMAP_LEGACY_STA_GC,
        .support_chip_type_bitmap = HMAC_LEGACY_STA_GC_SUPPORT_CHIP_BITMAP,
    },
    [HMAC_VAP_COEX_TYPE_LEGACY_STA_CHBA] = {
        .coex_mvap_type = HMAC_VAP_COEX_TYPE_LEGACY_STA_CHBA,
        .coex_mvap_bitmap = MAC_VAP_COEX_BITMAP_LEGACY_STA_CHBA,
        .support_chip_type_bitmap = HMAC_LEGACY_STA_CHBA_SUPPORT_CHIP_BITMAP,
    },
    [HMAC_VAP_COEX_TYPE_GO_GO] = {
        .coex_mvap_type = HMAC_VAP_COEX_TYPE_GO_GO,
        .coex_mvap_bitmap = MAC_VAP_COEX_BITMAP_GO_GO,
        .support_chip_type_bitmap = HMAC_GO_GO_SUPPORT_CHIP_BITMAP,
    },
    [HMAC_VAP_COEX_TYPE_GO_GC] = {
        .coex_mvap_type = HMAC_VAP_COEX_TYPE_GO_GC,
        .coex_mvap_bitmap = MAC_VAP_COEX_BITMAP_GO_GC,
        .support_chip_type_bitmap = HMAC_GO_GC_SUPPORT_CHIP_BITMAP,
    },
    [HMAC_VAP_COEX_TYPE_GO_CHBA] = {
        .coex_mvap_type = HMAC_VAP_COEX_TYPE_GO_CHBA,
        .coex_mvap_bitmap = MAC_VAP_COEX_BITMAP_GO_CHBA,
        .support_chip_type_bitmap = HMAC_GO_CHBA_SUPPORT_CHIP_BITMAP,
    },
    [HMAC_VAP_COEX_TYPE_GC_GC] = {
        .coex_mvap_type = HMAC_VAP_COEX_TYPE_GC_GC,
        .coex_mvap_bitmap = MAC_VAP_COEX_BITMAP_GC_GC,
        .support_chip_type_bitmap = HMAC_GC_GC_SUPPORT_CHIP_BITMAP,
    },
    [HMAC_VAP_COEX_TYPE_GC_CHBA] = {
        .coex_mvap_type = HMAC_VAP_COEX_TYPE_GC_CHBA,
        .coex_mvap_bitmap = MAC_VAP_COEX_BITMAP_GC_CHBA,
        .support_chip_type_bitmap = HMAC_GC_CHBA_SUPPORT_CHIP_BITMAP,
    },
    [HMAC_VAP_COEX_TYPE_LEGACY_STA_GO_GO] = {
        .coex_mvap_type = HMAC_VAP_COEX_TYPE_LEGACY_STA_GO_GO,
        .coex_mvap_bitmap = MAC_VAP_COEX_BITMAP_LEGACY_STA_GO_GO,
        .support_chip_type_bitmap = HMAC_LEGACY_STA_GO_GO_SUPPORT_CHIP_BITMAP,
    },
    [HMAC_VAP_COEX_TYPE_LEGACY_STA_GO_GC] = {
        .coex_mvap_type = HMAC_VAP_COEX_TYPE_LEGACY_STA_GO_GC,
        .coex_mvap_bitmap = MAC_VAP_COEX_BITMAP_LEGACY_STA_GO_GC,
        .support_chip_type_bitmap = HMAC_LEGACY_STA_GO_GC_SUPPORT_CHIP_BITMAP,
    },
    [HMAC_VAP_COEX_TYPE_LEGACY_STA_GO_CHBA] = {
        .coex_mvap_type = HMAC_VAP_COEX_TYPE_LEGACY_STA_GO_CHBA,
        .coex_mvap_bitmap = MAC_VAP_COEX_BITMAP_LEGACY_STA_GO_CHBA,
        .support_chip_type_bitmap = HMAC_LEGACY_STA_GO_CHBA_SUPPORT_CHIP_BITMAP,
    },
    [HMAC_VAP_COEX_TYPE_LEGACY_STA_GC_GC] = {
        .coex_mvap_type = HMAC_VAP_COEX_TYPE_LEGACY_STA_GC_GC,
        .coex_mvap_bitmap = MAC_VAP_COEX_BITMAP_LEGACY_STA_GC_GC,
        .support_chip_type_bitmap = HMAC_LEGACY_STA_GC_GC_SUPPORT_CHIP_BITMAP,
    },
    [HMAC_VAP_COEX_TYPE_LEGACY_STA_GC_CHBA] = {
        .coex_mvap_type = HMAC_VAP_COEX_TYPE_LEGACY_STA_GC_CHBA,
        .coex_mvap_bitmap = MAC_VAP_COEX_BITMAP_LEGACY_STA_GC_CHBA,
        .support_chip_type_bitmap = HMAC_LEGACY_STA_GC_CHBA_SUPPORT_CHIP_BITMAP,
    },
};

/* 功能描述：判断当前是否2vap_ap 场景
 * 返回值：TRUE:是2vap_ap工作场景；FALSE：非2vap_ap 工作场景
 */
oal_bool_enum_uint8 hmac_res_restriction_is_ap_with_other_vap(void)
{
    mac_device_stru *mac_device = mac_res_get_mac_dev();
    mac_vap_stru *up_vap_array[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = { NULL };
    uint32_t up_vap_num;
    uint32_t idx;
    uint32_t have_legacy_ap = OAL_FALSE;
    uint32_t have_other_vap = OAL_FALSE;
    up_vap_num = mac_device_find_up_vaps(mac_device, up_vap_array, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    for (idx = 0; idx < up_vap_num; idx++) {
        mac_vap_stru *tmp_vap = up_vap_array[idx];
        if (is_legacy_ap(tmp_vap) == OAL_TRUE && mac_is_chba_mode(tmp_vap) == OAL_FALSE) {
            have_legacy_ap = OAL_TRUE;
        } else {
            have_other_vap = OAL_TRUE;
        }
    }
    return (have_legacy_ap == OAL_TRUE && have_other_vap == OAL_TRUE) ? OAL_TRUE : OAL_FALSE;
}

uint32_t hmac_vap_mode_trans(wlan_vap_mode_enum_uint8 vap_mode, wlan_p2p_mode_enum_uint8 p2p_mode,
    uint8_t is_add_chba)
{
    if (is_add_chba == OAL_TRUE) {
        return MAC_VAP_MODE_CHBA;
    } else if (p2p_mode == WLAN_P2P_GO_MODE) {
        return MAC_VAP_MODE_P2P_GO;
    } else if (p2p_mode == WLAN_P2P_CL_MODE) {
        return MAC_VAP_MODE_P2P_GC;
    } else if (vap_mode == WLAN_VAP_MODE_BSS_STA) {
        return MAC_VAP_MODE_LEGACY_STA;
    } else if (vap_mode == WLAN_VAP_MODE_BSS_AP) {
        return MAC_VAP_MODE_LEGACY_AP;
    } else {
        return MAC_VAP_MODE_BUTT;
    }
}

/* 判断添加vap是否已存在相同模式的vap，已存在的话做bit偏移处理 */
static uint32_t hmac_add_same_mode_vap_offset_trans(uint32_t current_coex_vap_bitmap, uint32_t current_vap_mode)
{
    uint32_t current_vap_mode_update = current_vap_mode;
    uint8_t vap_mode_offset_cnt = 0;
    /* 查找是否已添加过同类型VAP，从低位到高位查找 */
    for (vap_mode_offset_cnt = 0; vap_mode_offset_cnt < HMAC_VAP_MODE_OFFSET_TRANS_MAX_CNT; vap_mode_offset_cnt++) {
        if ((current_coex_vap_bitmap & current_vap_mode_update) != 0) {
            current_vap_mode_update <<= HMAC_MULTI_VAP_MODE_OFFSET;
        }
    }

    return current_vap_mode_update;
}

/*
 * 功能描述  : 将已存在up的vap及添加的vap合并算出共存bitmap
 * 1.日    期  : 2014年11月11日
 *   修改内容  : 新生成函数
 */
static uint32_t mac_device_calc_coex_vaps_trans_bitmap(mac_device_stru *mac_device,
    mac_vap_stru **up_vap_array, uint8_t up_vap_num)
{
    uint8_t  vap_idx;
    uint32_t current_coex_vap_bitmap = 0; // 计算得出的最终的共存bitmap
    mac_vap_stru *mac_vap = NULL;
    uint32_t already_up_vap_mode = 0;
    uint8_t is_chba = OAL_FALSE;

    for (vap_idx = 0; vap_idx < up_vap_num; vap_idx++) {
        mac_vap = up_vap_array[vap_idx];
        if (mac_vap == NULL) {
            continue;
        }
        is_chba = OAL_FALSE;
#ifdef _PRE_WLAN_CHBA_MGMT
        if (mac_vap->chba_mode == CHBA_MODE) {
            is_chba = OAL_TRUE;
        }
#endif
        already_up_vap_mode = hmac_vap_mode_trans(mac_vap->en_vap_mode, mac_vap->en_p2p_mode, is_chba);
        /* 如果当前存在同一类型vap，则做偏移处理;先查看高位，避免已添加两个相同vap */
        already_up_vap_mode = hmac_add_same_mode_vap_offset_trans(current_coex_vap_bitmap, already_up_vap_mode);
        current_coex_vap_bitmap |= already_up_vap_mode;
    }

    return current_coex_vap_bitmap;
}

/*
 * mp16/mp16c/gf61 2vap场景规格
 * ---------------------------------------------------------------------
 *                |               待添加vap类型                         |
 * --------------------------------------------------------------------|
 *                | legacy APUT | legacy STA  |  GO  |  GC  | CHBA     |
 * ---------------|----------------------------------------------------|
 * 已添加VAP类型   |                                                    |
 * --------------------------------------------------------------------|
 * legacy APUT    |      Y      |      Y      |   Y	 |   Y  |  Y       |
 * --------------------------------------------------------------------|
 * legacy STA     |      Y      |      Y      |   Y  |   Y  |  Y       |
 * --------------------------------------------------------------------|
 * GO             |      Y      |      Y      |   Y  |   Y  |  Y       |
 * --------------------------------------------------------------------|
 * GC             |      Y      |      Y      |   Y  |   Y  |  Y       |
 * --------------------------------------------------------------------|
 * CHBA           |      Y      |      Y      |   Y  |   Y  |  Y       |
 *---------------------------------------------------------------------|
 *
 * mp15 2vap场景规格
 * ---------------------------------------------------------------------
 *                |               待添加vap类型                         |
 * --------------------------------------------------------------------|
 *                | legacy APUT | legacy STA  |  GO  |  GC  | CHBA     |
 * ---------------|----------------------------------------------------|
 * 已添加VAP类型   |                                                    |
 * --------------------------------------------------------------------|
 * legacy APUT    |      N      |      N      |   N  |   N  |  N       |
 * --------------------------------------------------------------------|
 * legacy STA     |      N      |      Y      |   Y  |   Y  |  Y       |
 * --------------------------------------------------------------------|
 * GO             |      N      |      Y      |   N  |   N  |  Y       |
 * --------------------------------------------------------------------|
 * GC             |      N      |      Y      |   N  |   N  |  Y       |
 * --------------------------------------------------------------------|
 * CHBA           |      N      |      Y      |   Y  |   Y  |  N       |
 *---------------------------------------------------------------------|
 *
 * mp16/mp16c/gf61 3vap场景规格
 * ---------------------------------------------------------------------------------
 *                            |               待添加vap类型                         |
 * --------------------------------------------------------------------------------|
 *                            |  legacy APUT | legacy STA  |  GO  |  GC  |  CHBA   |
 * ---------------------------|----------------------------------------------------|
 * 已添加VAP类型               |                                                    |
 * --------------------------------------------------------------------------------|
 * legacy STA  |  legacy STA  |       N      |      N      |   N  |   N  |    N    |
 * --------------------------------------------------------------------------------|
 * legacy STA  |  GO          |       N      |      N      |   N  |   N  |    Y    |
 * --------------------------------------------------------------------------------|
 * legacy STA  |  GC          |       N      |      N      |   N  |   N  |    Y    |
 * --------------------------------------------------------------------------------|
 * legacy STA  |  CHBA        |       N      |      Y      |   Y  |   N  |    N    |
 * --------------------------------------------------------------------------------|
 * GO          |  CHBA        |       N      |      Y      |   N  |   N  |    N    |
 *---------------------------------------------------------------------------------|
 * GC          |  CHBA        |       N      |      Y      |   N  |   N  |    N    |
 *---------------------------------------------------------------------------------|
 *
 * mp15 3vap场景规格
 * ---------------------------------------------------------------------------------
 *                            |               待添加vap类型                         |
 * --------------------------------------------------------------------------------|
 *                            |  legacy APUT | legacy STA  |  GO  |  GC  |  CHBA   |
 * ---------------------------|----------------------------------------------------|
 * 已添加VAP类型               |                                                    |
 * --------------------------------------------------------------------------------|
 * legacy STA  |  legacy STA  |       N      |      N      |   N  |   N  |    N    |
 * --------------------------------------------------------------------------------|
 * legacy STA  |  GO          |       N      |      N      |   N  |   N  |    Y    |
 * --------------------------------------------------------------------------------|
 * legacy STA  |  GC          |       N      |      N      |   N  |   N  |    Y    |
 * --------------------------------------------------------------------------------|
 * legacy STA  |  CHBA        |       N      |      Y      |   Y  |   N  |    N    |
 * --------------------------------------------------------------------------------|
 * GO          |  CHBA        |       N      |      Y      |   N  |   N  |    N    |
 *---------------------------------------------------------------------------------|
 * GC          |  CHBA        |       N      |      Y      |   N  |   N |    N    |
 *---------------------------------------------------------------------------------|
 *
 */
/*
 * 功能描述  : vap up时，检查vap共存规格是否支持。
 * 1.日    期  : 2020年6月3日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
oal_bool_enum_uint8 hmac_vap_up_check_vap_res_supp(mac_vap_stru *mac_vap)
{
    mac_device_stru *mac_device = mac_res_get_mac_dev();
    uint32_t current_coex_vap_bitmap = 0;
    uint8_t loop;
    uint32_t chip_type = (uint32_t)get_mpxx_subchip_type();
    uint8_t up_vap_num;
    mac_vap_stru *up_vap_array[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = { NULL };

    up_vap_num = mac_device_find_up_vaps(mac_device, up_vap_array, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    /* 当前只有自己是up状态则直接返回即可 */
    if (up_vap_num == 1) {
        return HMAC_MULTI_VAP_STATE_SUPPORT;
    }
    current_coex_vap_bitmap = mac_device_calc_coex_vaps_trans_bitmap(mac_device, up_vap_array, up_vap_num);
    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_vap_up_check_vap_res_supp:: current_coex_vap_bitmap[0x%x]}",
        current_coex_vap_bitmap);
    for (loop = 0; loop < oal_array_size(g_mvap_coex_cfg_table); loop++) {
        if ((current_coex_vap_bitmap == g_mvap_coex_cfg_table[loop].coex_mvap_bitmap) &&
            (((g_mvap_coex_cfg_table[loop].support_chip_type_bitmap >> chip_type) & BIT0) != 0)) {
            return HMAC_MULTI_VAP_STATE_SUPPORT;
        }
    }

    return HMAC_MULTI_VAP_STATE_UNSUPPORT;
}
