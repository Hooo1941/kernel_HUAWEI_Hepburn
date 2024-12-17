/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : hmac_vsp_flow_ctrl.h
 * 作者       : wifi
 * 创建日期   : 2022年6月10日
 */

#ifndef HMAC_VSP_FLOW_CTRL_H
#define HMAC_VSP_FLOW_CTRL_H

#include "hmac_vsp_source.h"

#ifdef _PRE_WLAN_FEATURE_VSP

uint16_t hmac_vsp_source_get_layer0_max(hmac_vsp_source_stru *source, hmac_vsp_tx_slice_stru *slice);
uint16_t hmac_vsp_source_get_layerall_max(hmac_vsp_source_stru *source, hmac_vsp_tx_slice_stru *slice);

#endif
#endif
