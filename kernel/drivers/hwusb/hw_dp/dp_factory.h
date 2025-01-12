/*
 * dp_factory.h
 *
 * factory test for DP module
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef __DP_FACTORY_H__
#define __DP_FACTORY_H__

#include <linux/types.h>

int dp_factory_init(void);
void dp_factory_exit(void);
bool dp_factory_mode_is_enable(void);
void dp_factory_link_cr_or_ch_eq_fail(bool is_cr);
bool dp_factory_is_4k_60fps(uint8_t rate, uint8_t lanes,
	uint16_t h_active, uint16_t v_active, uint8_t fps);
int dp_factory_get_test_result(char *buffer, int size);
bool dp_factory_need_report_event(void);
void dp_factory_set_link_event_no(uint32_t event_no,
	bool cablein, char *event, int hotplug);
void dp_factory_set_link_rate_lanes(uint8_t rate,
	uint8_t lanes, uint8_t sink_rate, uint8_t sink_lanes);
void dp_factory_set_mmie_test_flag(bool test_enable);

#endif /* __DP_FACTORY_H__ */
