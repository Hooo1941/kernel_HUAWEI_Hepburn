/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : vsp dma-buf管理
 * 作    者 : wifi
 * 创建日期 : 2022年6月9日
 */

#ifndef HMAC_VSP_BUF_H
#define HMAC_VSP_BUF_H

#include "hmac_vsp.h"

#ifdef _PRE_WLAN_FEATURE_VSP
void hmac_vsp_source_dma_buf_func_register(void);
void hmac_vsp_source_dma_buf_func_unregister(void);
#endif
#endif
