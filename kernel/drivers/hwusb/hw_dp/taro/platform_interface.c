/*
 * platform_interface.c
 *
 * interface for taro dp module
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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
#include "../dp_dsm_internal.h"

#define HWLOG_TAG dp_interface
HWLOG_REGIST();

#define DP_CONFIGURE_MASK     0x3f
#define DP_HPD_STATE_MASK     0x40
#define DP_HPD_IRQ_MASK       0x80
#define DP_HPD_STATE_OFFSET   6
#define DP_HPD_IRQ_OFFSET     7

void qcom_dp_set_pd_event(uint8_t cur_hpd_high, uint8_t connected,
	uint8_t dp_data, uint8_t orientation)
{
	uint8_t cur_mode = connected ? TCPC_DP : TCPC_NC;
	uint8_t pin, hpd_state, hpd_irq;
	uint8_t mode_type, irq_type;
	uint8_t dev_type = 0;

	pin = dp_data & DP_CONFIGURE_MASK;
	hpd_state = (dp_data & DP_HPD_STATE_MASK) >> DP_HPD_STATE_OFFSET;
	hpd_irq = (dp_data & DP_HPD_IRQ_MASK) >> DP_HPD_IRQ_OFFSET;

	mode_type = pin ? TCPC_DP : TCPC_NC;
	irq_type = !!hpd_state;

	if (!cur_hpd_high && hpd_state)
		dev_type = TCA_DP_IN;
	else if (cur_hpd_high && !hpd_state)
		dev_type = TCA_DP_OUT;

	if (hpd_irq)
		irq_type = TCA_IRQ_SHORT;

	huawei_dp_imonitor_set_param(DP_PARAM_IRQ_VECTOR, &hpd_irq);
	huawei_dp_imonitor_set_pd_event(irq_type, cur_mode, mode_type,
		dev_type, orientation);
}
EXPORT_SYMBOL_GPL(qcom_dp_set_pd_event);

