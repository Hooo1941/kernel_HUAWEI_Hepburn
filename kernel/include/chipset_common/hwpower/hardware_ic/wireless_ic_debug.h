/* SPDX-License-Identifier: GPL-2.0 */
/*
 * wireless_ic_debug.h
 *
 * debug interface for wireless hardware ic
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

#ifndef _WIRELESS_IC_DEBUG_H_
#define _WIRELESS_IC_DEBUG_H_

#include <securec.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>

#define WLRX_DEBUG_DIR_NAME          "wlrxic"
#define WLTX_DEBUG_DIR_NAME          "wltxic"
#define WLTRX_DEBUG_DIR_LEN           10
#define WLIC_TX_PARAM_DFLT           (-1)

enum wltx_param_type {
	WLTX_PARAM_BEGIN,
	WLTX_PARAM_PING_FREQ = WLTX_PARAM_BEGIN,
	WLTX_PARAM_PING_DUTY,
	WLTX_PARAM_PT_FOP_MIN,
	WLTX_PARAM_PT_FOP_MAX,
	WLTX_PARAM_PT_DUTY_MAX,
	WLTX_PARAM_PT_DUTY_MIN,
	WLTX_PARAM_PING_OCP,
	WLTX_PARAM_PT_OCP,
	WLTX_PARAM_PT_ILIMIT,
	WLTX_PARAM_MAX,
};

#if defined(CONFIG_WIRELESS_CHARGER) && defined(CONFIG_HUAWEI_POWER_DEBUG)
int wlic_dbg_rx_get_dirname(unsigned int type, char *name, int len);
int wlic_dbg_tx_get_dirname(unsigned int type, char *name, int len);
ssize_t wlic_dbg_tx_param_store(void *dev_data, const char *buf, size_t size);
ssize_t wlic_dbg_tx_param_show(void *dev_data, char *buf, size_t size);
int wlic_dbg_tx_get_param(enum wltx_param_type type);
void wlic_tx_get_dbg_info(char *param, int len, void *dev_data);
#else
static inline int wlic_dbg_rx_get_dirname(unsigned int type, char *name, int len)
{
	return -EINVAL;
}

static inline int wlic_dbg_tx_get_dirname(unsigned int type, char *name, int len)
{
	return -EINVAL;
}

static inline ssize_t wlic_dbg_tx_param_store(void *dev_data, const char *buf, size_t size)
{
	return (ssize_t)size;
}

static inline ssize_t wlic_dbg_tx_param_show(void *dev_data, char *buf, size_t size)
{
	return 0;
}

static inline int wlic_dbg_tx_get_param(enum wltx_param_type type)
{
	return WLIC_TX_PARAM_DFLT;
}

static inline void wlic_tx_get_dbg_info(char *param, int len, void *dev_data)
{
}
#endif /* CONFIG_WIRELESS_CHARGER CONFIG_HUAWEI_POWER_DEBUG */

#endif /* _WIRELESS_IC_DEBUG_H_ */
