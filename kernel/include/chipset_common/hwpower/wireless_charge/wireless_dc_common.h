/* SPDX-License-Identifier: GPL-2.0 */
/*
 * wireless_dc_common.h
 *
 * common interface, variables, definition etc of wireless_dc_common.c
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _WIRELESS_DC_COMMON_H_
#define _WIRELESS_DC_COMMON_H_

#define WLRX_RVS_MODE_POSTFIX        "RVS"
#define WLRX_SC_MODE_PREFIX          "SC"
#define WLRX_SCX_MODE_PREFIX         "SCX"
#define WLRX_LVC_MODE_PREFIX         "LVC"
#define WLRX_12V_MODE_POSTFIX        "12V"
#define WLRX_15V_MODE_POSTFIX        "15V"

enum wldc_dc_mode {
	WLDC_MODE_BEGIN = 0,
	WLDC_MODE_LVC = WLDC_MODE_BEGIN,
	WLDC_MODE_SC,
	WLDC_MODE_SCX,
	WLDC_MODE_END,
};

#ifdef CONFIG_WIRELESS_CHARGER
unsigned int wldc_get_mode_by_name(const char *mode_name);
#else
static inline unsigned int wldc_get_mode_by_name(const char *mode_name)
{
	return WLDC_MODE_END;
}

#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_DC_COMMON_H_ */