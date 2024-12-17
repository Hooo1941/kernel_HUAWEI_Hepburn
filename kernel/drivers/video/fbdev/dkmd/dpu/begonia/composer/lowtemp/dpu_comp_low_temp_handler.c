/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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

#include "dpu_comp_low_temp_handler.h"
#include <linux/kobject.h>
#include <dpu/soc_dpu_define.h>

#include "dkmd_log.h"
#include "dpu_comp_mgr.h"
#include "dpu_config_utils.h"


#define LOW_TEMP_STATE true
#define NORMAL_TEMP_STATE false
#define PMCTRL_PERI_CTRL4_TEMPERATURE_MASK GENMASK(27, 26)
#define PMCTRL_PERI_CTRL4_TEMPERATURE_SHIFT 26
#define MIDIA_PERI_CTRL4 0x350

uint32_t check_low_temperature(void)
{
	uint32_t perictrl4;
	char __iomem *pmctrl_base = dpu_config_get_ip_base(DISP_IP_BASE_PMCTRL);
	dpu_check_and_return(!pmctrl_base, false, err, "pmctrl_base is NULL!\n");
	perictrl4 = inp32(pmctrl_base + MIDIA_PERI_CTRL4);
	perictrl4 &= PMCTRL_PERI_CTRL4_TEMPERATURE_MASK;
	perictrl4 = (perictrl4 >> PMCTRL_PERI_CTRL4_TEMPERATURE_SHIFT);
	dpu_pr_debug("Get current temperature: %d\n", perictrl4);
	return perictrl4;
}

static void dpu_temp_change_event(struct composer *comp, char *trigger, uint32_t low_temp_flag)
{
	char *envp[2] = { low_temp_flag == LOW_TEMP_STATE ? "LowTemp=1" : "LowTemp=0", NULL};

	if (!comp || !trigger) {
		dpu_pr_err("comp or trigger is NULL Pointer\n");
		return;
	}

	kobject_uevent_env(&(comp->base.peri_device->dev.kobj), KOBJ_CHANGE, envp);
	dpu_pr_info("%s occur! low temperature state %d\n", trigger, low_temp_flag);
}

void reset_low_temperature_state(struct dpu_composer *dpu_comp)
{
	uint32_t low_temp_now = false;

	dpu_check_and_no_retval(!dpu_comp, err, "dpu_comp is NULL\n");
	if ((dpu_comp->comp.index != DEVICE_COMP_PRIMARY_ID) && (dpu_comp->comp.index != DEVICE_COMP_BUILTIN_ID))
		return;
	low_temp_now = check_low_temperature();
	dpu_temp_change_event(&dpu_comp->comp, "low_temp_reset", low_temp_now);
	dpu_comp->low_temp_ctrl.low_temp_state_pre = low_temp_now;
}

static void dpu_low_temp_handle(struct work_struct *work)
{
	struct dpu_composer *dpu_comp = NULL;
	struct dpu_low_temp_ctrl* low_temp_ctrl = NULL;
	uint32_t low_temp_now = false;

	low_temp_ctrl = container_of(work, struct dpu_low_temp_ctrl, low_temp_work);
	dpu_comp = low_temp_ctrl->dpu_comp;
	low_temp_now = check_low_temperature();
	dpu_temp_change_event(&dpu_comp->comp, "low_temp_change", low_temp_now);
	low_temp_ctrl->low_temp_state_pre = low_temp_now;
}

static int32_t dpu_low_temp_handle_isr_handle(struct notifier_block *self, unsigned long action, void *data)
{
	struct dkmd_listener_data *listener_data = NULL;
	struct dpu_composer *dpu_comp = NULL;
	struct dpu_low_temp_ctrl* low_temp_ctrl = NULL;
	uint32_t low_temp_now = false;

	listener_data = (struct dkmd_listener_data *)data;
	dpu_check_and_return(!listener_data, 0, err, "listener_data is NULL\n");
	dpu_comp = (struct dpu_composer *)(listener_data->data);
	dpu_check_and_return(!dpu_comp, 0, err, "dpu_comp is NULL\n");
	low_temp_ctrl = &dpu_comp->low_temp_ctrl;

	low_temp_now = check_low_temperature();
	if (low_temp_ctrl->low_temp_wq && (low_temp_now != low_temp_ctrl->low_temp_state_pre))
		queue_work(low_temp_ctrl->low_temp_wq, &low_temp_ctrl->low_temp_work);
	low_temp_ctrl->low_temp_state_pre = low_temp_now;
	return 0;
}

static struct notifier_block dpu_low_temp_handle_isr_notifier = {
	.notifier_call = dpu_low_temp_handle_isr_handle,
};

static void dpu_low_temp_irq_init(struct dkmd_isr *isr_ctrl, struct dpu_composer *dpu_comp, uint32_t listening_bit)
{
	dkmd_isr_register_listener(isr_ctrl, &dpu_low_temp_handle_isr_notifier, listening_bit, dpu_comp);
}

static void dpu_low_temp_irq_deinit(struct dkmd_isr *isr_ctrl, uint32_t listening_bit)
{
	dkmd_isr_unregister_listener(isr_ctrl, &dpu_low_temp_handle_isr_notifier, listening_bit);
}


void dpu_low_temp_register(struct dpu_composer *dpu_comp, struct comp_online_present *present)
{
	uint32_t te_bit;
	uint32_t vsync_bit;
	struct dkmd_isr *isr_ctrl = NULL;
	struct dkmd_connector_info *pinfo = NULL;
	struct dpu_comp_dfr_ctrl* dfr_ctrl = NULL;
	struct dpu_low_temp_ctrl* low_temp_ctrl = NULL;

	dpu_check_and_no_retval(!dpu_comp, err, "dpu_comp is NULL\n");
	pinfo = dpu_comp->conn_info;
	low_temp_ctrl = &dpu_comp->low_temp_ctrl;
	dpu_check_and_no_retval(!present, err, "present is NULL\n");
	dfr_ctrl = &present->dfr_ctrl;

	switch (dfr_ctrl->mode) {
	case DFR_MODE_CONSTANT:
	case DFR_MODE_LONG_V:
	case DFR_MODE_LONG_H:
	case DFR_MODE_LONG_VH:
		te_bit = pinfo->base.lcd_te_idx == 0 ? DSI_INT_LCD_TE0 : DSI_INT_LCD_TE1;
		vsync_bit = is_mipi_cmd_panel(&pinfo->base) ? te_bit : DSI_INT_VSYNC;
		isr_ctrl = &dpu_comp->isr_ctrl;
		break;
	case DFR_MODE_TE_SKIP_BY_MCU:
	case DFR_MODE_LONGH_TE_SKIP_BY_MCU:
		vsync_bit = NOTIFY_BOTH_VSYNC_TIMELINE;
		isr_ctrl = &dpu_comp->comp_mgr->mdp_isr_ctrl;
		break;
	case DFR_MODE_TE_SKIP_BY_ACPU:
		vsync_bit = DSI_INT_VSYNC_COUNT_BY_TE;
		isr_ctrl = &dpu_comp->isr_ctrl;
		break;
	default:
		dpu_pr_err("invalid dfr mode %d", dfr_ctrl->mode);
		return;
	}
	low_temp_ctrl->dpu_comp = dpu_comp;
	/* creat low temp workqueue */
	low_temp_ctrl->low_temp_wq =
		alloc_workqueue("low_temp_wq", WQ_HIGHPRI | WQ_UNBOUND, 0);
	if (!low_temp_ctrl->low_temp_wq) {
		dpu_pr_err("creat low temp queue failed!\n");
		return;
	}
	INIT_WORK(&low_temp_ctrl->low_temp_work, dpu_low_temp_handle);
	dpu_low_temp_irq_init(isr_ctrl, dpu_comp, vsync_bit);
}


void dpu_low_temp_unregister(struct dpu_composer *dpu_comp, struct comp_online_present *present)
{
	struct dkmd_isr *isr_ctrl = NULL;
	struct dkmd_connector_info *pinfo = NULL;
	struct dpu_comp_dfr_ctrl* dfr_ctrl = NULL;
	struct dpu_low_temp_ctrl* low_temp_ctrl = NULL;
	uint32_t te_bit;
	uint32_t vsync_bit;

	dpu_check_and_no_retval(!dpu_comp, err, "dpu_comp is NULL\n");
	pinfo = dpu_comp->conn_info;
	low_temp_ctrl = &dpu_comp->low_temp_ctrl;
	dpu_check_and_no_retval(!present, err, "present is NULL\n");
	dfr_ctrl = &present->dfr_ctrl;

	switch (dfr_ctrl->mode) {
	case DFR_MODE_CONSTANT:
	case DFR_MODE_LONG_V:
	case DFR_MODE_LONG_H:
	case DFR_MODE_LONG_VH:
		te_bit = pinfo->base.lcd_te_idx == 0 ? DSI_INT_LCD_TE0 : DSI_INT_LCD_TE1;
		vsync_bit = is_mipi_cmd_panel(&pinfo->base) ? te_bit : DSI_INT_VSYNC;
		isr_ctrl = &dpu_comp->isr_ctrl;
		break;
	case DFR_MODE_TE_SKIP_BY_MCU:
	case DFR_MODE_LONGH_TE_SKIP_BY_MCU:
		vsync_bit = NOTIFY_BOTH_VSYNC_TIMELINE;
		isr_ctrl = &dpu_comp->comp_mgr->mdp_isr_ctrl;
		break;
	case DFR_MODE_TE_SKIP_BY_ACPU:
		vsync_bit = DSI_INT_VSYNC_COUNT_BY_TE;
		isr_ctrl = &dpu_comp->isr_ctrl;
		break;
	default:
		dpu_pr_err("invalid dfr mode %d", dfr_ctrl->mode);
		return;
	}
	dpu_low_temp_irq_deinit(isr_ctrl, vsync_bit);
	if (low_temp_ctrl->low_temp_wq)
		destroy_workqueue(low_temp_ctrl->low_temp_wq);
	dpu_pr_info("End");
}
