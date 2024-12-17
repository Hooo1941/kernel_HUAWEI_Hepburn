/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : vsp发送流程
 * 作    者 : wifi
 * 创建日期 : 2022年6月10日
 */
#include "hmac_vsp_flow_ctrl.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_VSP_FLOW_CTRL_C

#ifdef _PRE_WLAN_FEATURE_VSP
static inline uint8_t hmac_vsp_source_get_layer_max_factor(hmac_vsp_source_stru *source, uint16_t ratio)
{
    uint8_t idx;

    for (idx = 0; idx < HMAC_VSP_LAYER_MAX_FACTOR_NUM - 1; idx++) {
        if (ratio < source->layer_max_mgmt.ratio_th[idx]) {
            break;
        }
    }

    return source->layer_max_mgmt.factor[idx];
}

static inline uint16_t hmac_vsp_source_get_layer_succ_pkts(hmac_vsp_tx_slice_stru *slice, tx_layer_ctrl *layer)
{
    return (slice->tx_result.layer_suc_num[layer->layer_number] & 0x7FFF);
}

static inline uint32_t hmac_vsp_source_get_layer_succ_bytes(hmac_vsp_tx_slice_stru *slice, tx_layer_ctrl *layer)
{
    return hmac_vsp_source_get_layer_succ_pkts(slice, layer) * layer->normal_pack_length;
}

static inline uint32_t hmac_vsp_source_update_layer_max_upwards(uint32_t layer_max, uint32_t bytes)
{
    return layer_max > bytes ? layer_max : (bytes * 105 / 100); // 105 / 100: 每次尝试提升5%的数据量上限
}

#define HMAC_VSP_LAYER_MAX_FAST_RATIO 1
#define HMAC_VSP_LAYER_MAX_RATIO 2

static inline uint32_t hmac_vsp_source_update_layer_max_downwards(
    hmac_vsp_source_stru *source, uint32_t layer_max, uint32_t bytes, uint8_t fast)
{
    uint32_t delta = layer_max - bytes;
    uint8_t factor = hmac_vsp_source_get_layer_max_factor(source, delta * 1000 / (layer_max + 1));

    return layer_max - delta / (factor * (fast ? HMAC_VSP_LAYER_MAX_FAST_RATIO : HMAC_VSP_LAYER_MAX_RATIO));
}

#define HMAC_VSP_SOURCE_LOW_SUCC_RATIO 95
static inline uint8_t hmac_vsp_source_tx_low_ratio(uint16_t succ_pkts, uint16_t total_pkts)
{
    if (total_pkts == 0) {
        return OAL_FALSE;
    }
    return (succ_pkts * 100 / total_pkts) < HMAC_VSP_SOURCE_LOW_SUCC_RATIO; // 100: 百分比
}

uint16_t hmac_vsp_source_get_layerall_max(hmac_vsp_source_stru *source, hmac_vsp_tx_slice_stru *slice)
{
    tx_layer_ctrl *layer = slice->head;
    uint32_t bytes = 0;
    uint16_t succ_pkts = 0;
    uint16_t total_pkts = 0;

    do {
        bytes += hmac_vsp_source_get_layer_succ_bytes(slice, layer);
        succ_pkts += hmac_vsp_source_get_layer_succ_pkts(slice, layer);
        total_pkts += layer->paket_number;
    } while ((layer = (tx_layer_ctrl *)layer->next) != NULL);

    if (bytes < source->layer_max_mgmt.layerall_max && hmac_vsp_source_tx_low_ratio(succ_pkts, total_pkts)) {
        source->layer_max_mgmt.layerall_max =
            hmac_vsp_source_update_layer_max_downwards(source, source->layer_max_mgmt.layerall_max, bytes, 0);
    } else {
        source->layer_max_mgmt.layerall_max =
            hmac_vsp_source_update_layer_max_upwards(source->layer_max_mgmt.layerall_max, bytes);
    }

    return (uint16_t)(source->layer_max_mgmt.layerall_max >> BIT_OFFSET_10);
}

uint16_t hmac_vsp_source_get_layer0_max(hmac_vsp_source_stru *source, hmac_vsp_tx_slice_stru *slice)
{
    uint32_t bytes = hmac_vsp_source_get_layer_succ_bytes(slice, slice->head);
    uint16_t pkts = hmac_vsp_source_get_layer_succ_pkts(slice, slice->head);
    if (bytes < source->layer_max_mgmt.layer0_max && hmac_vsp_source_tx_low_ratio(pkts, slice->head->paket_number)) {
        source->layer_max_mgmt.layer0_max =
            hmac_vsp_source_update_layer_max_downwards(source, source->layer_max_mgmt.layer0_max, bytes, 1);
    } else {
        source->layer_max_mgmt.layer0_max =
            hmac_vsp_source_update_layer_max_upwards(source->layer_max_mgmt.layer0_max, bytes);
    }

    return (uint16_t)(source->layer_max_mgmt.layer0_max >> BIT_OFFSET_10);
}

#endif
