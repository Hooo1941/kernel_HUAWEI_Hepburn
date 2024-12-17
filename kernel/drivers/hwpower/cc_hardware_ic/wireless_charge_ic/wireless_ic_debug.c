// SPDX-License-Identifier: GPL-2.0
/*
 * wireless_ic_debug.c
 *
 * ic debug for wireless charging
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

#include <chipset_common/hwpower/hardware_ic/wireless_ic_debug.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_ic_intf.h>
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG wireless_ic_debug
HWLOG_REGIST();

#define WLIC_TX_PARAM_LEN           2
#define WLIC_TX_PARAM_KEY_LEN       20
#define WLIC_TX_PARAM_OUT           128

struct wltxic_param_input {
	int id;
	char key[WLIC_TX_PARAM_KEY_LEN];
	int value;
};

static struct wltxic_param_input g_param_input[] = {
	{ WLTX_PARAM_PING_FREQ,     "ping_freq",   WLIC_TX_PARAM_DFLT },
	{ WLTX_PARAM_PING_DUTY,     "ping_duty",   WLIC_TX_PARAM_DFLT },
	{ WLTX_PARAM_PT_FOP_MIN,    "pt_fop_min",  WLIC_TX_PARAM_DFLT },
	{ WLTX_PARAM_PT_FOP_MAX,    "pt_fop_max",  WLIC_TX_PARAM_DFLT },
	{ WLTX_PARAM_PT_DUTY_MAX,   "pt_duty_max", WLIC_TX_PARAM_DFLT },
	{ WLTX_PARAM_PT_DUTY_MIN,   "pt_duty_min", WLIC_TX_PARAM_DFLT },
	{ WLTX_PARAM_PING_OCP,      "ping_ocp",    WLIC_TX_PARAM_DFLT },
	{ WLTX_PARAM_PT_OCP,        "pt_ocp",      WLIC_TX_PARAM_DFLT },
	{ WLTX_PARAM_PT_ILIMIT,     "pt_ilimit",   WLIC_TX_PARAM_DFLT },
};

int wlic_dbg_rx_get_dirname(unsigned int type, char *name, int len)
{
	int ret;

	if (type > WLTRX_DRV_MAIN)
		ret = snprintf_s(name, len, len - 1, "%s %d", WLRX_DEBUG_DIR_NAME, type);
	else
		ret = snprintf_s(name, len, len - 1, "%s", WLRX_DEBUG_DIR_NAME);
	return ret;
}

int wlic_dbg_tx_get_dirname(unsigned int type, char *name, int len)
{
	if (type > WLTRX_DRV_MAIN)
		return snprintf_s(name, len, len - 1, "%s %d", WLTX_DEBUG_DIR_NAME, type);

	return snprintf_s(name, len, len - 1, "%s", WLTX_DEBUG_DIR_NAME);
}

int wlic_dbg_tx_get_param(enum wltx_param_type type)
{
	int i;

	if (type >= WLTX_PARAM_MAX || type < WLTX_PARAM_BEGIN)
		return 0;

	for (i = 0; i < ARRAY_SIZE(g_param_input); i++) {
		if (type == g_param_input[i].id)
			break;
	}

	if (i >= ARRAY_SIZE(g_param_input))
		return 0;

	return g_param_input[i].value;
}

static void wlic_tx_param_save(const char *key, int value)
{
	int len = ARRAY_SIZE(g_param_input);
	int i;

	if (!strncmp(key, "reset", sizeof("reset"))) {
		for (i = 0; i < len; i++)
			g_param_input[i].value = WLIC_TX_PARAM_DFLT;
	}

	for (i = 0; i < len; i++) {
		if (!strncmp(g_param_input[i].key, key, strlen(g_param_input[i].key)))
			break;
	}

	if (i >= len)
		return;

	g_param_input[i].value = value;
}

ssize_t wlic_dbg_tx_param_store(void *dev_data, const char *buf, size_t size)
{
	int value = 0;
	char key[WLIC_TX_PARAM_KEY_LEN] = {0};

	if (sscanf_s(buf, "%s %d", key, WLIC_TX_PARAM_KEY_LEN, &value) != WLIC_TX_PARAM_LEN) {
		hwlog_err("tx_param_store: unable to parse input %s\n", buf);
		return -EINVAL;
	}

	hwlog_info("[tx_param_store] input=%s %d\n", key, value);
	wlic_tx_param_save(key, value);
	return (ssize_t)size;
}

ssize_t wlic_dbg_tx_param_show(void *dev_data, char *buf, size_t size)
{
	int i;
	int out_len = 0;
	int len = ARRAY_SIZE(g_param_input);

	for (i = 0; i < len; i++)
		out_len += snprintf_s(buf + out_len, size - out_len, size - out_len - 1,
			"%s %d\n", g_param_input[i].key, g_param_input[i].value);

	return (ssize_t)out_len;
}

void wlic_tx_get_dbg_info(char *info, int len, void *dev_data)
{
	int i;
	int ret;
	int out_len = 0;
	int arr_size = ARRAY_SIZE(g_param_input);

	out_len += ret;
	for (i = 0; i < arr_size; i++) {
		if (g_param_input[i].value >= 0)
			ret = snprintf_s(info + out_len, len - out_len, len - out_len - 1,
				"%s:%d ", g_param_input[i].key, g_param_input[i].value);
		if (ret < 0)
			return;
		out_len += ret;
	}
}
