/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:amax_multilink.h header file
 * Author: @CompanyNameTag
 */

#ifndef AMAX_MULTILINK_H
#define AMAX_MULTILINK_H

#ifdef _PRE_CONFIG_HILINK_AMAX
#include <huawei_platform/net/amax/amax_multilink_interface.h>

#define external_amax_tx_data amax_hook_tx_data
#define external_amax_tx_complete amax_hook_tx_complete
#define external_amax_rx_data amax_hook_rx_data
#endif

#endif /* end for AMAX_MULTILINK_H */
