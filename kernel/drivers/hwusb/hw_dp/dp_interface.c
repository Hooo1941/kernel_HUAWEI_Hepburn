/*
 * dp_interface.c
 *
 * interface for huawei dp module
 *
 * Copyright (c) 2021-2022 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwusb/hw_dp/dp_interface.h>
#include <linux/module.h>
#include <huawei_platform/log/hw_log.h>
#include "dp_dsm_internal.h"
#include "dp_dsm.h"
#include "dp_factory.h"
#include "dp_source_switch.h"

#define HWLOG_TAG dp_interface
HWLOG_REGIST();

/* Interface provided by hw, include data monitor, event report, and etc. */
void huawei_dp_imonitor_set_param(enum dp_imonitor_param param,
	void *data)
{
	dp_imonitor_set_param(param, data);
}
EXPORT_SYMBOL_GPL(huawei_dp_imonitor_set_param);

void huawei_dp_imonitor_set_param_aux_rw(bool rw, bool i2c,
	uint32_t addr, uint32_t len, int retval)
{
	dp_imonitor_set_param_aux_rw(rw, i2c, addr, len, retval);
}
EXPORT_SYMBOL_GPL(huawei_dp_imonitor_set_param_aux_rw);

void huawei_dp_imonitor_set_param_timing(uint16_t h_active,
	uint16_t v_active, uint32_t pixel_clock)
{
	dp_imonitor_set_param_timing(h_active, v_active, pixel_clock, 0);
}
EXPORT_SYMBOL_GPL(huawei_dp_imonitor_set_param_timing);

void huawei_dp_imonitor_set_param_vs_pe(int index, uint8_t *vs,
	uint8_t *pe)
{
	dp_imonitor_set_param_vs_pe(index, vs, pe);
}
EXPORT_SYMBOL_GPL(huawei_dp_imonitor_set_param_vs_pe);

bool huawei_dp_factory_mode_is_enable(void)
{
	return dp_factory_mode_is_enable();
}
EXPORT_SYMBOL_GPL(huawei_dp_factory_mode_is_enable);

void huawei_dp_factory_link_cr_or_ch_eq_fail(bool is_cr)
{
	dp_factory_link_cr_or_ch_eq_fail(is_cr);
}
EXPORT_SYMBOL_GPL(huawei_dp_factory_link_cr_or_ch_eq_fail);

bool huawei_dp_factory_is_4k_60fps(uint8_t rate, uint8_t lanes,
	uint16_t h_active, uint16_t v_active, uint8_t fps)
{
	return dp_factory_is_4k_60fps(rate, lanes, h_active, v_active, fps);
}
EXPORT_SYMBOL_GPL(huawei_dp_factory_is_4k_60fps);

bool huawei_dp_is_vrefresh_valid(uint32_t vrefresh)
{
#ifndef DP_FACTORY_MODE_ENABLE
	/* Limit refresh rate 50fps to 70fps */
	if ((vrefresh < 50) || (vrefresh > 70))
		return false;
#endif
	return true;
}
EXPORT_SYMBOL_GPL(huawei_dp_is_vrefresh_valid);

void huawei_dp_register_switch_handler(switch_source_handler handler)
{
	register_switch_handler(handler);
}
EXPORT_SYMBOL_GPL(huawei_dp_register_switch_handler);

#ifdef CONFIG_VR_DISPLAY
void huawei_dp_set_dptx_vr_status(bool dptx_vr)
{
	dp_set_dptx_vr_status(dptx_vr);
}
EXPORT_SYMBOL_GPL(huawei_dp_set_dptx_vr_status);
#endif /* CONFIG_VR_DISPLAY */

int huawei_dp_get_current_dp_source_mode(void)
{
	return get_current_dp_source_mode();
}
EXPORT_SYMBOL_GPL(huawei_dp_get_current_dp_source_mode);

void huawei_dp_imonitor_set_pd_event(uint8_t irq_type,
	uint8_t cur_mode, uint8_t mode_type, uint8_t dev_type, uint8_t typec_orien)
{
	dp_imonitor_set_pd_event(irq_type, cur_mode, mode_type,
		dev_type, typec_orien);
}
EXPORT_SYMBOL_GPL(huawei_dp_imonitor_set_pd_event);

void huawei_dp_save_edid(const uint8_t *edid_buf, uint32_t buf_len)
{
	save_dp_edid(edid_buf, buf_len);
}
EXPORT_SYMBOL_GPL(huawei_dp_save_edid);
