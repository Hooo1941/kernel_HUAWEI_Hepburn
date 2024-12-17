/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   :
 * 作者       : wifi
 * 创建日期   : 2018年7月17日
 */


#ifndef HMAC_RATE_CALC_H
#define HMAC_RATE_CALC_H

#include "wlan_spec.h"
#include "wlan_types.h"
#include "hal_common.h"

#ifdef _PRE_WLAN_FEATURE_SNIFFER

uint32_t hmac_get_rate_kbps(hal_statistic_stru *rate_info, uint32_t *rate_kbps);

#endif
#endif /* end of alg_rate_table.h */
