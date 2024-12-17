// SPDX-License-Identifier: GPL-2.0
/*
 * power_panel_event.c
 *
 * panel event for power module
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

#include <linux/fb.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_extra_event.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#ifdef CONFIG_DKMD_DKSM
#include <platform_include/display/dkmd/dkmd_notify.h>
#endif

#define HWLOG_TAG power_panel_event
HWLOG_REGIST();

#ifdef CONFIG_DKMD_DKSM
enum {
	SCREEN_INSIDE = 0,
	SCREEN_OUTSIDE,
	SCREEN_ALL
};
static bool g_unblank_states[SCREEN_ALL] = { 0 };

static int power_panel_event_dkmd_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct dkmd_event *blank_event = data;
	int blank;
	int i;

	if (event != DKMD_EVENT_BLANK)
		return NOTIFY_DONE;

	if (!blank_event || (blank_event->panel_id >= SCREEN_ALL) || (blank_event->panel_id < SCREEN_INSIDE)) {
		hwlog_err("blank_event is null\n");
		return NOTIFY_DONE;
	}

	blank = blank_event->value;
	switch (blank) {
	case FB_BLANK_UNBLANK:
		g_unblank_states[blank_event->panel_id] = true;
		power_event_bnc_notify(POWER_BNT_PANEL_EVENT, POWER_NE_PANEL_UNBLANK, NULL);
		hwlog_info("[%d]fb screen on\n", blank_event->panel_id);
		break;
	case FB_BLANK_POWERDOWN:
		g_unblank_states[blank_event->panel_id] = false;

		for (i = 0; i < SCREEN_ALL; i++)
			if (g_unblank_states[i])
				break;

		if (i == SCREEN_ALL) {
			power_event_bnc_notify(POWER_BNT_PANEL_EVENT, POWER_NE_PANEL_BLANK, NULL);
			hwlog_info("fb screen off\n");
		}
		break;
	default:
		hwlog_err("blank_event unknown in %s\n", __func__);
		break;
	}

	return NOTIFY_DONE;
}
#else
static int power_panel_event_fb_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct fb_event *blank_event = data;
	int *blank = NULL;

	if (!blank_event || !blank_event->data) {
		hwlog_err("blank_event is null\n");
		return NOTIFY_DONE;
	}

	blank = blank_event->data;
	if ((event == FB_EVENT_BLANK) && (*blank == FB_BLANK_UNBLANK)) {
		power_event_bnc_notify(POWER_BNT_PANEL_EVENT, POWER_NE_PANEL_UNBLANK, NULL);
		hwlog_info("fb screen on\n");
	} else if ((event == FB_EVENT_BLANK) && (*blank == FB_BLANK_POWERDOWN)) {
		power_event_bnc_notify(POWER_BNT_PANEL_EVENT, POWER_NE_PANEL_BLANK, NULL);
		hwlog_info("fb screen off\n");
	}

	return NOTIFY_DONE;
}
#endif

int power_panel_event_parse_active_panel(void)
{
	return 0;
}

void power_panel_event_register(struct power_extra_event_dev *di)
{
	if (!di)
		return;

#ifdef CONFIG_DKMD_DKSM
	di->nb.notifier_call = power_panel_event_dkmd_notifier_call;
	(void)dkmd_register_client(&di->nb);
#else
	di->nb.notifier_call = power_panel_event_fb_notifier_call;
	(void)fb_register_client(&di->nb);
#endif
}

void power_panel_event_unregister(struct power_extra_event_dev *di)
{
	if (di)
#ifdef CONFIG_DKMD_DKSM
		(void)dkmd_unregister_client(&di->nb);
#else
		(void)fb_unregister_client(&di->nb);
#endif
}
