/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als gamma config header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ALS_GAMMA_CONFIG_H__
#define __ALS_GAMMA_CONFIG_H__

#if ((defined DSS_ALSC_NOISE) || (defined DKMD_ALSC_NOISE))
#include <linux/types.h>

bool als_gamma_param_config(uint32_t tag, uint32_t mipi);
#ifdef DSS_ALSC_NOISE
void als_gamma_set_alpha(uint32_t alpha);
#elif defined DKMD_ALSC_NOISE
void als_gamma_set_alpha(uint32_t alpha, uint32_t panel_id);
#endif
int als_gamma_update_with_alpha(void);
#endif

#endif
