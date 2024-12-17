/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2025. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _DKSM_DMD_H_
#define _DKSM_DMD_H_

#include <linux/types.h>
#include <linux/platform_device.h>
#include <dsm/dsm_pub.h>
#include "dkmd_log.h"

#define DSM_OCCUPY_RETRY_TIMES 10

extern struct dsm_client *dsm_lcd_client;

struct dsm_client* dkmd_get_dmd_client(void);
void dksm_dmd_report_vactive_end_miss(uint32_t conn_id);

#if defined (CONFIG_HUAWEI_DSM) || defined (CONFIG_HUAWEI_OHOS_DSM)
void dksm_dmd_register(uint32_t index, const char *lcd_name);
#else
#define dksm_dmd_register(index, lcd_name)
#endif

#endif
