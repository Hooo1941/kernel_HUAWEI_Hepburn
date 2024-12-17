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

#include "dpu_comp_fusa_handler.h"
#include <linux/kobject.h>
#include <securec.h>

#include "dkmd_log.h"
#include "dpu_comp_mgr.h"
#include "dkmd_notify.h"
#include "dkmd_isr.h"

static struct dpu_composer *g_dpu_composer = NULL;

int32_t dpu_fusa_get_ffd_info(struct composer *comp, struct dkmd_ffd_cfg *info)
{
	struct dpu_composer *dpu_comp = NULL;
	struct dpu_fusa_ctrl *fusa_ctrl = NULL;
	errno_t err_ret;

	if ((!comp) || (!info)) {
		dpu_pr_err("param invalid");
		return -EINVAL;
	}

	dpu_comp = to_dpu_composer(comp);
	fusa_ctrl = &dpu_comp->fusa_ctrl;

	if (!fusa_ctrl->ffd_en) {
		info->ffd_en = false;
		return 0;
	}

	err_ret = memcpy_s(info, sizeof(struct dkmd_ffd_cfg), &fusa_ctrl->ffd_cfg, sizeof(struct dkmd_ffd_cfg));
	if (err_ret != EOK) {
		dpu_pr_err("memcpy_s failed");
		return -EINVAL;
	}

	dpu_pr_info("[FUSA]ffd_en = %u\n", info->ffd_en);

	return 0;
}

static int32_t ffd_check_cfg(struct dkmd_ffd_cfg *ffd_cfg)
{
	uint32_t roi_index = 0;
	uint32_t x = 0;
	uint32_t y = 0;
	uint32_t w = 0;
	uint32_t h = 0;

	if (ffd_cfg->ffd_mode >= FFD_MODE_CNT) {
		dpu_pr_err("ffd_mode %u is invalid\n", ffd_cfg->ffd_mode);
		return -1;
	}

	if (ffd_cfg->ffd_mode == FFD_ROI0) {
		if ((ffd_cfg->mode0_roi_cfg_bytes == 0) ||
			(ffd_cfg->mode0_roi_cfg_bytes % MODE0_ROI_CFG_BYTES_CONSTRAINT != 0)) {
			dpu_pr_err("mode0_roi_cfg_bytes = %u is invalid\n", ffd_cfg->mode0_roi_cfg_bytes);
			return -1;
		}
	}

	if (ffd_cfg->ffd_mode == FFD_ROI1) {
		for (roi_index = 0; roi_index < FFD_ROI_CNT_MAX; ++roi_index) {
			if (!ffd_cfg->roi_cfg[roi_index].roi_en)
				continue;

			x = ffd_cfg->roi_cfg[roi_index].roi_area.x;
			y = ffd_cfg->roi_cfg[roi_index].roi_area.y;
			w = ffd_cfg->roi_cfg[roi_index].roi_area.width;
			h = ffd_cfg->roi_cfg[roi_index].roi_area.height;
			if (w >= FFD_ROI_AREA_MAX || h >= FFD_ROI_AREA_MAX ||
				w == 0 || h == 0) {
				dpu_pr_err("w = %u, h = %u is invalid\n", w, h);
				return -1;
			}
		}
	}

	return 0;
}

static int32_t dpu_ffd_save_ffd_cfg(struct dkmd_ffd_cfg *ffd_cfg, struct dpu_fusa_ctrl *fusa_ctrl)
{
	int32_t ret;
	errno_t err_ret;

	ret = ffd_check_cfg(ffd_cfg);
	if (ret != 0)
		return -1;

	err_ret = memcpy_s(&fusa_ctrl->ffd_cfg, sizeof(struct dkmd_ffd_cfg), ffd_cfg, sizeof(struct dkmd_ffd_cfg));
	if (err_ret != EOK) {
		dpu_pr_err("memcpy_s failed");
		return -1;
	}

	return 0;
}

static void dpu_ffd_change_event(struct composer *comp, char *trigger, bool ffd_en)
{
	char *envp[2] = { ffd_en ? "FFD_EVENT=1" : "FFD_EVENT=0", NULL};

	if (!comp || !trigger) {
		dpu_pr_err("comp or trigger is NULL Pointer\n");
		return;
	}

	kobject_uevent_env(&(comp->base.peri_device->dev.kobj), KOBJ_CHANGE, envp);
	dpu_pr_info("%s occur! ffd enable state %d\n", trigger, ffd_en);
}

static void dpu_ffd_event_handle(struct work_struct *work)
{
	struct dpu_composer *dpu_comp = NULL;
	struct dpu_fusa_ctrl* fusa_ctrl = NULL;

	fusa_ctrl = container_of(work, struct dpu_fusa_ctrl, ffd_event_work);
	dpu_check_and_no_retval(!fusa_ctrl, err, "fusa_ctrl is null!");
	dpu_comp = fusa_ctrl->dpu_comp;

	dpu_ffd_change_event(&dpu_comp->comp, "ffd_en_status_change", fusa_ctrl->ffd_en);
}

static int32_t fusa_notifier_handle(struct notifier_block *nb, unsigned long action, void *data)
{
	struct dpu_composer *dpu_comp = NULL;
	struct dpu_fusa_ctrl *fusa_ctrl = NULL;
	struct dkmd_ffd_cfg *ffd_cfg = NULL;
	struct dkmd_event *event = NULL;
	bool ffd_en;

	if (!data)
		return NOTIFY_OK;

	if ((action != DKMD_EVENT_FFD_ENABLE) && (action != DKMD_EVENT_FFD_DISABLE)) {
		dpu_pr_info("action is error\n");
		return NOTIFY_OK;
	}

	event = (struct dkmd_event *)data;
	ffd_cfg = (struct dkmd_ffd_cfg *)event->data;
	if (!ffd_cfg) {
		dpu_pr_err("ffd_cfg is nullptr\n");
		return NOTIFY_OK;
	}

	ffd_en = ffd_cfg->ffd_en;
	dpu_comp = g_dpu_composer;
	if (!dpu_comp)
		return NOTIFY_OK;

	fusa_ctrl = &dpu_comp->fusa_ctrl;

	if (fusa_ctrl->ffd_en == ffd_en) {
		dpu_pr_info("ffd en status do not change %d\n", ffd_en);
		return NOTIFY_OK;
	}

	if (ffd_en && (dpu_ffd_save_ffd_cfg(ffd_cfg, fusa_ctrl) != 0))
		return NOTIFY_OK;

	fusa_ctrl->ffd_en = ffd_en;
	queue_work(fusa_ctrl->ffd_event_wq, &fusa_ctrl->ffd_event_work);

	return NOTIFY_OK;
}

// get ffd info notifier
static struct notifier_block fusa_notify = {
	.notifier_call = fusa_notifier_handle,
};

static void dpu_ffd_result_handle(struct work_struct *work)
{
	struct dpu_fusa_ctrl* fusa_ctrl = NULL;
	struct dkmd_event event = {0};
	unsigned long flags = 0;

	fusa_ctrl = container_of(work, struct dpu_fusa_ctrl, ffd_result_work);
	dpu_check_and_no_retval(!fusa_ctrl, err, "fusa_ctrl is null!");

	event.data = NULL;
	spin_lock_irqsave(&fusa_ctrl->ffd_result_lock, flags);
	event.ffd_result = fusa_ctrl->ffd_result;
	spin_unlock_irqrestore(&fusa_ctrl->ffd_result_lock, flags);

	dpu_pr_warn("ffd_result %u\n", event.ffd_result);
	dkmd_notifier_call_chain(DKMD_EVENT_FFD_RESULT, (void *)&event);
}

static int32_t ffd_isr_handle(struct notifier_block *self, unsigned long action, void *data)
{
	unsigned long flags = 0;
	struct dkmd_listener_data *listener_data = NULL;
	struct dpu_composer *dpu_comp = NULL;
	uint32_t isr_ffd_state = 0;
	char __iomem *dpu_base = NULL;
	struct dpu_fusa_ctrl* fusa_ctrl = NULL;

	listener_data = (struct dkmd_listener_data *)data;
	dpu_check_and_return(!listener_data, 0, err, "listener_data is NULL\n");
	dpu_comp = (struct dpu_composer *)(listener_data->data);
	dpu_check_and_return(!dpu_comp, 0, err, "dpu_comp is NULL\n");
	dpu_check_and_return(!dpu_comp->comp_mgr, 0, err, "comp_mgr is NULL\n");
	dpu_base = dpu_comp->comp_mgr->dpu_base;

	fusa_ctrl = &dpu_comp->fusa_ctrl;

	isr_ffd_state = inp32(DPU_FFD_INTR_CRC_ADDR(dpu_base + DPU_SDMA_FFD0_OFFSET));
	outp32(DPU_FFD_INTR_CRC_ADDR(dpu_base + DPU_SDMA_FFD0_OFFSET), isr_ffd_state);

	spin_lock_irqsave(&fusa_ctrl->ffd_result_lock, flags);
	fusa_ctrl->ffd_result = isr_ffd_state;
	spin_unlock_irqrestore(&fusa_ctrl->ffd_result_lock, flags);
	dpu_pr_warn("isr_ffd_state %u\n", isr_ffd_state);

	queue_work(fusa_ctrl->ffd_result_wq, &fusa_ctrl->ffd_result_work);

	return 0;
}

// process ffd check result
static struct notifier_block ffd_isr_notifier = {
	.notifier_call = ffd_isr_handle,
};

void dpu_comp_fusa_register(struct dpu_composer *dpu_comp)
{
	struct dpu_fusa_ctrl* fusa_ctrl = NULL;
	struct dkmd_isr *isr_ctrl = NULL;

	dpu_check_and_no_retval(!dpu_comp, err, "dpu_comp is NULL\n");
	if (!is_primary_panel(&dpu_comp->conn_info->base))
		return;

	fusa_ctrl = &dpu_comp->fusa_ctrl;
	isr_ctrl = &dpu_comp->comp_mgr->sdp_isr_ctrl;

	(void)memset_s(fusa_ctrl, sizeof(struct dpu_fusa_ctrl), 0, sizeof(struct dpu_fusa_ctrl));

	fusa_ctrl->ffd_en = false;
	fusa_ctrl->ffd_cfg.ffd_en = false;
	g_dpu_composer = dpu_comp;
	fusa_ctrl->dpu_comp = dpu_comp;

	dkmd_register_client(&fusa_notify);
	dkmd_isr_register_listener(isr_ctrl, &ffd_isr_notifier, DPU_FFD_LE_NS_FFD_INT, dpu_comp);
	spin_lock_init(&fusa_ctrl->ffd_result_lock);

	/* creat ffd result workqueue */
	fusa_ctrl->ffd_result_wq =
		alloc_workqueue("ffd_result_wq", WQ_HIGHPRI | WQ_UNBOUND, 0);
	if (!fusa_ctrl->ffd_result_wq) {
		dpu_pr_err("creat ffd result queue failed!\n");
		return;
	}
	INIT_WORK(&fusa_ctrl->ffd_result_work, dpu_ffd_result_handle);

	/* creat ffd event workqueue */
	fusa_ctrl->ffd_event_wq =
		alloc_workqueue("ffd_event_wq", WQ_HIGHPRI | WQ_UNBOUND, 0);
	if (!fusa_ctrl->ffd_event_wq) {
		dpu_pr_err("creat ffd event queue failed!\n");
		return;
	}
	INIT_WORK(&fusa_ctrl->ffd_event_work, dpu_ffd_event_handle);

	fusa_ctrl->fusa_inited = true;
}

void dpu_comp_fusa_unregister(struct dpu_composer *dpu_comp)
{
	struct dpu_fusa_ctrl* fusa_ctrl = NULL;
	struct dkmd_isr *isr_ctrl = NULL;

	dpu_check_and_no_retval(!dpu_comp, err, "dpu_comp is NULL\n");
	fusa_ctrl = &dpu_comp->fusa_ctrl;
	if (!fusa_ctrl->fusa_inited)
		return;

	isr_ctrl = &dpu_comp->comp_mgr->sdp_isr_ctrl;

	fusa_ctrl->ffd_en = false;
	fusa_ctrl->ffd_cfg.ffd_en = false;
	fusa_ctrl->ffd_result = 0;
	g_dpu_composer = NULL;

	dkmd_unregister_client(&fusa_notify);
	dkmd_isr_unregister_listener(isr_ctrl, &ffd_isr_notifier, DPU_FFD_LE_NS_FFD_INT);

	if (fusa_ctrl->ffd_event_wq)
		destroy_workqueue(fusa_ctrl->ffd_event_wq);

	if (fusa_ctrl->ffd_result_wq)
		destroy_workqueue(fusa_ctrl->ffd_result_wq);
}
