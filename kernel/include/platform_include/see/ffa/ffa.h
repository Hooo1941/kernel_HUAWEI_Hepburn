/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: ffa interface
 *
 * Create: 2022-6-28
 */
#pragma once

#define __FFA_INTERL_USE
#include <platform_include/see/ffa/ffa_id.h>
#undef __FFA_INTERL_USE

/* Reserve ID for NS physical FFA Endpoint */
#define FFA_NS_ENDPOINT_ID                  0U
#define FFA_SPMD_ID                         0xFFFFU
#define FFA_SPMC_ID                         0x8000U
