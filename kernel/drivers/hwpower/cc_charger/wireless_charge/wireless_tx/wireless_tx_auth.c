// SPDX-License-Identifier: GPL-2.0
/*
 * wireless_tx_auth.c
 *
 * authenticate for wireless tx charge
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/completion.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_genl.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/protocol/wireless_protocol.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_auth.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_ic_intf.h>

#define HWLOG_TAG wireless_tx_auth
HWLOG_REGIST();

static struct completion g_wltx_auth_completion;
static bool g_wltx_auth_srv_state;
static int g_wltx_auth_result;
static u8 g_wltx_auth_hash[WLTX_AUTH_RANDOM_LEN];

bool wltx_auth_get_srv_state(void)
{
	return g_wltx_auth_srv_state;
}

void wltx_auth_clean_hash_data(void)
{
	memset(g_wltx_auth_hash, 0x00, WLTX_AUTH_RANDOM_LEN);
}

u8 *wltx_auth_get_hash_data_header(void)
{
	return g_wltx_auth_hash;
}

unsigned int wltx_auth_get_hash_data_size(void)
{
	return WLTX_AUTH_RANDOM_LEN;
}

static int wltx_auth_wait_service_ready(void)
{
	int i;
	int delay = WLTX_AUTH_RETRY_INTERVAL_BASE;

	for (i = 1; i <= WLTX_AUTH_RETRY_COUNT; i++) {
		power_msleep(delay, 0, NULL);
		if (g_wltx_auth_srv_state == true)
			return 0;

		if (i % WLTX_AUTH_RETRY_INTERVAL_CYCLE == 0) {
			delay = WLTX_AUTH_RETRY_INTERVAL_BASE +
				i / WLTX_AUTH_RETRY_INTERVAL_CYCLE * WLTX_AUTH_RETRY_INTERVAL_INC;
			power_event_bnc_notify(POWER_BNT_BMS_AUTH, POWER_NE_BMS_AUTH_START_FORCE, "wireless_tx");
			power_msleep(delay, 0, NULL);
		}
	}

	return -EPERM;
}

int wltx_auth_wait_completion(void)
{
	int ret = -EPERM;

	g_wltx_auth_result = 0;
	reinit_completion(&g_wltx_auth_completion);

	power_event_bnc_notify(POWER_BNT_BMS_AUTH, POWER_NE_BMS_AUTH_START, "wireless_tx");

	/*
	 * if bms_auth service not ready, we assume the serivce is dead,
	 * return hash calculate ok anyway
	 */
	if (wltx_auth_wait_service_ready()) {
		hwlog_err("service not ready\n");
		goto end;
	}

	if (power_genl_easy_send(POWER_GENL_TP_AF,
		POWER_GENL_CMD_WLTX_AUTH_HASH, 0,
		g_wltx_auth_hash, WLTX_AUTH_RANDOM_LEN)) {
		hwlog_err("power genl send wltx auth failed\n");
		goto end;
	}

	/*
	 * if timeout happend, we assume the serivce is dead,
	 * return hash calculate ok anyway
	 */
	if (!wait_for_completion_timeout(&g_wltx_auth_completion,
		msecs_to_jiffies(WLTX_AUTH_WAIT_TIMEOUT))) {
		hwlog_err("service wait timeout\n");
		goto end;
	}

	/*
	 * if not timeout,
	 * return the antifake result base on the hash calc result
	 */
	if (g_wltx_auth_result == 0) {
		hwlog_err("hash calculate fail\n");
		goto end;
	}

	hwlog_info("hash calculate ok\n");
	ret = 0;
end:
	power_event_bnc_notify(POWER_BNT_BMS_AUTH, POWER_NE_BMS_AUTH_STOP, "wireless_tx");
	return ret;
}

static int wltx_auth_srv_state_cb(bool state)
{
	g_wltx_auth_srv_state = state;
	hwlog_info("srv_%s_cb ok\n", state ? "on" : "off");
	return 0;
}

static int wltx_auth_cb(unsigned char version, void *data, int len)
{
	u8 hash[WLTX_AUTH_HASH_LEN] = { 0 };

	if (!data || (len != WLTX_AUTH_HASH_LEN)) {
		hwlog_err("data is null or len invalid\n");
		return -EPERM;
	}

	g_wltx_auth_result = true;
	complete(&g_wltx_auth_completion);

	memcpy(hash, data, WLTX_AUTH_HASH_LEN);
	wireless_set_tx_cipherkey(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI,
		hash, WLTX_AUTH_HASH_LEN);

	hwlog_info("version=%u auth_result=%d\n", version, g_wltx_auth_result);
	return 0;
}

static const struct power_genl_easy_ops wltx_auth_easy_ops[] = {
	{
		.cmd = POWER_GENL_CMD_WLTX_AUTH_HASH,
		.doit = wltx_auth_cb,
	}
};

static struct power_genl_node wltx_auth_genl_node = {
	.target = POWER_GENL_TP_AF,
	.name = "WLTX_AUTH",
	.easy_ops = wltx_auth_easy_ops,
	.n_easy_ops = WLTX_AUTH_GENL_OPS_NUM,
	.srv_state_cb = wltx_auth_srv_state_cb,
};

static int __init wltx_auth_init(void)
{
	init_completion(&g_wltx_auth_completion);
	power_genl_easy_node_register(&wltx_auth_genl_node);
	return 0;
}

static void __exit wltx_auth_exit(void)
{
}

subsys_initcall(wltx_auth_init);
module_exit(wltx_auth_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("auth for wireless tx charge module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
