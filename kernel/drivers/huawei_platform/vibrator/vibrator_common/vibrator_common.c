/*
 * vibrator_common.c
 *
 * vibrator_common driver
 *
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
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
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/module.h>

static bool enter_recovery = false;
static bool enter_erecovery = false;

bool vibrator_get_enter_recovery_mode(void)
{
	if (enter_recovery || enter_erecovery)
		return true;

	return false;
}

static int __init early_parse_recovery_cmdline(char *p)
{
	if (p) {
		enter_recovery = (strncmp(p, "1", strlen("1")) == 0) ? true : false;
		pr_info("vibrator %s enter_mode = %s\n", __func__, enter_recovery ? "recovery" : "normal");
	}

	return 0;
}
early_param("enter_recovery", early_parse_recovery_cmdline);

static int __init early_parse_erecovery_cmdline(char *p)
{
	if (p) {
		enter_erecovery = (strncmp(p, "1", strlen("1")) == 0) ? true : false;
		pr_info("vibrator %s enter_mode = %s\n", __func__, enter_erecovery ? "erecovery" : "normal");
	}

	return 0;
}
early_param("enter_erecovery", early_parse_erecovery_cmdline);

MODULE_DESCRIPTION("Haptic Driver");
MODULE_LICENSE("GPL v2");
