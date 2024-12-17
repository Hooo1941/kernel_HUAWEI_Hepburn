/* Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "dpu_comp_mgr.h"
#include "tui.h"
#include "dpu_comp_tui.h"
#include "dpu_isr.h"
#include "gfxdev_mgr.h"
#include "dpu_comp_vsync.h"
#include "dkmd_tui_interface.h"

#define UEVENT_BUF_LEN 120
#define TUI_POLL_CFG_FAIL 1
#define TUI_POLL_CFG_OK   0
#define TUI_MSG_VAL 0

struct dpu_composer *g_active_dpu_comp = NULL;
uint32_t g_dirty_data; /* send to tui tzdriver avoid check null */

static void dpu_tui_send_msg(int type)
{
	send_tui_msg_config(type, TUI_MSG_VAL, "DSS");
}

static void dpu_tui_notify_status(struct dpu_composer *dpu_comp, struct dpu_secure *secure_ctrl)
{
	if ((secure_ctrl->secure_status == TUI_SEC_RUNNING) &&
		(secure_ctrl->secure_event == TUI_SEC_ENABLE) &&
		(secure_ctrl->tui_need_switch)) {
		if (dpu_comp->secure_ctrl.tui_quit_flag) {
			dpu_tui_send_msg(TUI_POLL_CFG_FAIL);
			secure_ctrl->secure_event = TUI_SEC_DISABLE;
			dpu_pr_info("TUI blank switch to TUI_SEC_RUNNING failed !\n");
		} else {
			dpu_tui_send_msg(TUI_POLL_CFG_OK);
			dpu_pr_info("TUI switch to TUI_SEC_RUNNING succ !\n");
		}
		secure_ctrl->tui_need_switch = false;
	}
}

static int dpu_tui_request_update(struct dpu_secure *secure_ctrl,
	int tui_request, int secure)
{
	if ((int32_t)(secure_ctrl->secure_status) == tui_request) {
		if (secure) {
			dpu_tui_send_msg(TUI_POLL_CFG_FAIL);
			secure_ctrl->secure_event = TUI_SEC_DISABLE;
		}
		dpu_pr_info("secure_status is not changed, secure_status = %d,---!\n", secure_ctrl->secure_status);
		return -1;
	}
	secure_ctrl->secure_event = (uint32_t)tui_request;
	secure_ctrl->tui_need_switch = true;

	return 0;
}

static void dpu_tui_switch_event(struct composer *comp, char *trigger, int state)
{
	char *envp[2];
	char state_buf[UEVENT_BUF_LEN];
	int ret;

	ret = snprintf(state_buf, sizeof(state_buf), "TUI_EVENT=%d\n", state);

	envp[0] = state_buf;
	envp[1] = NULL;

	if (!comp || !trigger) {
		dpu_pr_err("comp or trigger is NULL Pointer\n");
		return;
	}

	kobject_uevent_env(&(comp->base.peri_device->dev.kobj), KOBJ_CHANGE, envp);
	dpu_pr_info("%s need switch!", trigger);
}

static int dpu_tui_check_panel_power_status(struct dpu_composer *dpu_comp,
	struct dpu_secure *secure_ctrl, int secure)
{
	if (!dpu_comp->comp.power_on) {
		if (secure) {
			dpu_tui_send_msg(TUI_POLL_CFG_FAIL);
			secure_ctrl->secure_event = TUI_SEC_DISABLE;
		}
		return -1;
	}
	return 0;
}

static int dpu_tui_check_secure_created(struct dpu_secure *secure_ctrl, int secure)
{
	if (!secure_ctrl->secure_created) {
		if (secure) {
			dpu_tui_send_msg(TUI_POLL_CFG_FAIL);
			secure_ctrl->secure_event = TUI_SEC_DISABLE;
		}
		return -1;
	}
	return 0;
}

/* receive switch tui request
 * 1: secure enable
 * 0: secure disable
 */
static int notify_dpu_tui_request(void *data, int secure)
{
	int ret;
	int tui_request;
	struct dpu_secure *secure_ctrl = NULL;
	struct dpu_composer *dpu_comp = NULL;
	struct composer *comp = NULL;

	dpu_comp = g_active_dpu_comp;
	dpu_check_and_return(!dpu_comp, -EINVAL, err, "dpu_comp is NULL\n");
	secure_ctrl = &dpu_comp->secure_ctrl;
	comp = &dpu_comp->comp;

	/*
	1. tui only support one panel
	2. product_type=2, builtin device not support tui
	*/
	if (is_builtin_panel(&dpu_comp->comp.base) && (dpu_comp->conn_info->base.fold_type == PANEL_FLIP)) {
		dpu_pr_warn("%d not support tui\n", dpu_comp->conn_info->sw_post_chn_idx[PRIMARY_CONNECT_CHN_IDX]);
		return -1;
	}

	if ((!is_builtin_panel(&dpu_comp->comp.base)) && (!is_primary_panel(&dpu_comp->comp.base))) {
		dpu_pr_err("comp->index=%d not support TUI!\n", comp->index);
		return -1;
	}

	if (dpu_tui_check_secure_created(secure_ctrl, secure) < 0) {
		dpu_pr_err("secure is not created yet!\n");
		return -1;
	}

	if (!secure_ctrl->tui_quit_flag)
		down(&comp->blank_sem);

	if (dpu_tui_check_panel_power_status(dpu_comp, secure_ctrl, secure) < 0) {
		dpu_pr_err("panel is power off!\n");
		ret = -1;
		goto err_out;
	}

	tui_request = (secure != 0) ? TUI_SEC_ENABLE : TUI_SEC_DISABLE;
	dpu_pr_info(": secure_status = %d, secure_event = %d, tui_request = %d +++\n",
		secure_ctrl->secure_status, secure_ctrl->secure_event, tui_request);

	ret = dpu_tui_request_update(secure_ctrl, tui_request, secure);
	if (ret)
		goto err_out;

	if (secure_ctrl->secure_event == TUI_SEC_ENABLE) {
		dpu_comp_active_vsync(dpu_comp);
		dpu_tui_switch_event(&dpu_comp->comp, "tui", TUI_SEC_ENABLE);
	} else {
		dpu_comp_deactive_vsync(dpu_comp);
		secure_ctrl->secure_status = TUI_SEC_IDLE;
		dpu_tui_switch_event(&dpu_comp->comp, "tui", TUI_SEC_IDLE);
		dpu_pr_info("panel_power_on = %d\n", dpu_comp->comp.power_on);
	}

	dpu_pr_info("secure_status = %d, secure_event = %d, tui_request = %d ---\n",
		secure_ctrl->secure_status, secure_ctrl->secure_event, tui_request);

err_out:
	if (!secure_ctrl->tui_quit_flag)
		up(&comp->blank_sem);

	secure_ctrl->tui_quit_flag = false;

	return ret;
}

static void dpu_tui_irq_handler_work(struct kthread_work *work)
{
	struct dpu_secure *secure_ctrl = NULL;
	struct dpu_composer *dpu_comp = NULL;

	secure_ctrl = container_of(work, struct dpu_secure, secure_ctrl_work);
	dpu_check_and_no_retval(!secure_ctrl, err, "secure_ctrl is NULL!\n");

	dpu_comp = secure_ctrl->dpu_comp;
	dpu_check_and_no_retval(!dpu_comp, err, "dpu_comp is NULL!\n");
	dpu_pr_debug("secure_status = %d, secure_event = %d+++\n",
		secure_ctrl->secure_status, secure_ctrl->secure_event);

	if ((secure_ctrl->secure_status == TUI_SEC_RUNNING) &&
		(secure_ctrl->secure_event == TUI_SEC_DISABLE)) {
			dpu_comp_deactive_vsync(dpu_comp);
			secure_ctrl->secure_status = TUI_SEC_IDLE;
			return;
	}

	if (secure_ctrl->tui_ready_status == TUI_SEC_ENABLE) {
		if (secure_ctrl->secure_event == TUI_SEC_ENABLE &&
			(dpu_tui_check_panel_power_status(dpu_comp, secure_ctrl, 1) < 0)) {
			dpu_pr_info("panel is power off!\n");
			return;
		}

		if ((secure_ctrl->secure_status == TUI_SEC_IDLE) &&
			(secure_ctrl->secure_event == TUI_SEC_ENABLE))
				secure_ctrl->secure_status = TUI_SEC_RUNNING;

		dpu_tui_notify_status(dpu_comp, secure_ctrl);
	}

	dpu_pr_debug("secure_status = %d, secure_event = %d---\n",
		secure_ctrl->secure_status, secure_ctrl->secure_event);
}

static int32_t dpu_tui_irq_handle(struct notifier_block *self, unsigned long action, void *data)
{
	struct dkmd_listener_data *listener_data = (struct dkmd_listener_data *)data;
	struct dpu_composer *dpu_comp = (struct dpu_composer *)(listener_data->data);
	struct dpu_secure *secure_ctrl = NULL;

	secure_ctrl = &dpu_comp->secure_ctrl;

	if (secure_ctrl->secure_status != secure_ctrl->secure_event)
		kthread_queue_work(&dpu_comp->handle_worker, &secure_ctrl->secure_ctrl_work);
	return 0;
}

static struct notifier_block secure_handle_isr_notifier = {
	.notifier_call = dpu_tui_irq_handle,
};

static void dpu_tui_irq_init(struct dkmd_isr *isr_ctrl, struct dpu_composer *dpu_comp, uint32_t listening_bit)
{
	struct dpu_secure *secure_ctrl =  &dpu_comp->secure_ctrl;

	kthread_init_work(&secure_ctrl->secure_ctrl_work, dpu_tui_irq_handler_work);

	dkmd_isr_register_listener(isr_ctrl, &secure_handle_isr_notifier, listening_bit, dpu_comp);
}

static void dpu_tui_irq_deinit(struct dkmd_isr *isr_ctrl, uint32_t listening_bit)
{
	dkmd_isr_unregister_listener(isr_ctrl, &secure_handle_isr_notifier, listening_bit);
}

int dpu_tui_wait_quit(struct dpu_composer *dpu_comp)
{
	struct composer *comp = NULL;
	uint32_t timeout_count = 5000;  /* wait 5s */
	bool is_timeout = false;
	uint32_t delay_count = 0;
	dpu_check_and_return(!dpu_comp, -EINVAL, err, "dpu_comp is NULL\n");
	comp = &dpu_comp->comp;

	/*
	1. tui only support one panel
	2. product_type=2, builtin device not support tui
	*/
	if (is_builtin_panel(&dpu_comp->comp.base) && (dpu_comp->conn_info->base.fold_type == PANEL_FLIP)) {
		dpu_pr_warn("%d not support tui\n", dpu_comp->conn_info->sw_post_chn_idx[PRIMARY_CONNECT_CHN_IDX]);
		return 0;
	}

	if ((!is_builtin_panel(&dpu_comp->comp.base)) && (!is_primary_panel(&dpu_comp->comp.base)))
		return 0;

	/* 1. if tui is running, dss should not powerdown,
	 *    because register will be writen in tui mode.
	 * 2. if tui is enable, but not running, then tui should not be ok,
	 *    send the msg that tui config fail.
	 */
	dpu_pr_info("secure_status=%d, secure_event=%d\n",
		dpu_comp->secure_ctrl.secure_status, dpu_comp->secure_ctrl.secure_event);

	if (dpu_comp->secure_ctrl.secure_status == TUI_SEC_RUNNING) {
		dpu_comp->secure_ctrl.tui_quit_flag = true;
		tui_poweroff_work_start();
		dpu_pr_info("wait for tui quit\n");
		while (1) {
			if ((dpu_comp->secure_ctrl.secure_status != TUI_SEC_RUNNING) || (delay_count > timeout_count)) {
				is_timeout = (delay_count > timeout_count) ? true : false;
				break;
			}
			mdelay(1);
			++delay_count;
		}
		if (!is_timeout) {
			dpu_pr_info("quite tui success,delay_count = %d,secure_status=%d, secure_event=%d!\n",
				delay_count, dpu_comp->secure_ctrl.secure_status, dpu_comp->secure_ctrl.secure_event);
		} else {
			dpu_pr_err("quit tui failed,delay_count = %d,secure_status=%d, secure_event=%d!\n",
				delay_count, dpu_comp->secure_ctrl.secure_status, dpu_comp->secure_ctrl.secure_event);
			return -1;
		}
	} else if (dpu_comp->secure_ctrl.secure_event == TUI_SEC_ENABLE) {
		dpu_comp->secure_ctrl.secure_event = TUI_SEC_DISABLE;
		dpu_tui_send_msg(TUI_POLL_CFG_FAIL);
		dpu_pr_info("In power down, secure event will not be handled\n");
	}

	return 0;
}

void dpu_tui_update_ready_status(struct dpu_composer *dpu_comp, uint32_t tui_ready_status)
{
	dpu_comp->secure_ctrl.tui_ready_status = tui_ready_status;
}

void dpu_tui_register(struct dpu_composer *dpu_comp)
{
	struct dpu_secure *secure_ctrl = NULL;
	struct dkmd_isr *isr_ctrl = NULL;

	dpu_check_and_no_retval(!dpu_comp, err, "dpu_comp is NULL\n");

	/*
	1. tui only support one panel
	2. product_type=2, builtin device not support tui
	*/
	if (is_builtin_panel(&dpu_comp->comp.base) && (dpu_comp->conn_info->base.fold_type == PANEL_FLIP)) {
		dpu_pr_warn("%d not support tui\n", dpu_comp->conn_info->sw_post_chn_idx[PRIMARY_CONNECT_CHN_IDX]);
		return;
	}

	if ((dpu_comp->comp.index != DEVICE_COMP_PRIMARY_ID) && (dpu_comp->comp.index != DEVICE_COMP_BUILTIN_ID))
		return;

	dpu_pr_info("+ dpu_comp->comp.index=%d\n", dpu_comp->comp.index);
	secure_ctrl = &dpu_comp->secure_ctrl;
	isr_ctrl = &dpu_comp->isr_ctrl;

	down(&secure_ctrl->tui_sem);
	if (secure_ctrl->secure_created) {
		dpu_pr_info("comp.index=%d tui is already registered\n", dpu_comp->comp.index);
		up(&secure_ctrl->tui_sem);
		return;
	}
	secure_ctrl->secure_created = true;
	up(&secure_ctrl->tui_sem);

	secure_ctrl->secure_status = TUI_SEC_IDLE;
	secure_ctrl->secure_event  = TUI_SEC_DISABLE;
	secure_ctrl->tui_need_switch = false;
	secure_ctrl->tui_quit_flag = false;

	register_tui_driver(notify_dpu_tui_request, "DSS", &g_dirty_data);
	dpu_tui_irq_init(isr_ctrl, dpu_comp, DSI_INT_VACT0_END);

	secure_ctrl->dpu_comp = dpu_comp;
	g_active_dpu_comp = dpu_comp;
	dpu_pr_info("-\n");
}

void dpu_tui_unregister(struct dpu_composer *dpu_comp)
{
	struct dpu_secure *secure_ctrl = NULL;
	struct dkmd_isr *isr_ctrl = NULL;

	dpu_check_and_no_retval(!dpu_comp, err, "dpu_comp is NULL\n");

	/*
	1. tui only support one panel
	2. product_type=2, builtin device not support tui
	*/
	if (is_builtin_panel(&dpu_comp->comp.base) && (dpu_comp->conn_info->base.fold_type == PANEL_FLIP)) {
		dpu_pr_warn("%d not support tui\n", dpu_comp->conn_info->sw_post_chn_idx[PRIMARY_CONNECT_CHN_IDX]);
		return;
	}

	if ((dpu_comp->comp.index != DEVICE_COMP_PRIMARY_ID) && (dpu_comp->comp.index != DEVICE_COMP_BUILTIN_ID))
		return;

	dpu_pr_info("+ dpu_comp->comp.index=%d\n", dpu_comp->comp.index);
	secure_ctrl = &dpu_comp->secure_ctrl;
	isr_ctrl = &dpu_comp->isr_ctrl;

	down(&secure_ctrl->tui_sem);
	if (!secure_ctrl->secure_created) {
		dpu_pr_info("comp.index=%d tui is already unregistered\n", dpu_comp->comp.index);
		up(&secure_ctrl->tui_sem);
		return;
	}
	secure_ctrl->secure_created = false;
	up(&secure_ctrl->tui_sem);

	dpu_tui_irq_deinit(isr_ctrl, DSI_INT_VACT0_END);
	if (secure_ctrl->secure_status == TUI_SEC_RUNNING) {
		dpu_comp_deactive_vsync(dpu_comp);
		secure_ctrl->secure_status = TUI_SEC_IDLE;
	}

	if (secure_ctrl->secure_event == TUI_SEC_ENABLE) {
		secure_ctrl->secure_event = TUI_SEC_DISABLE;
		dpu_tui_send_msg(TUI_POLL_CFG_FAIL);
		dpu_pr_warn("In power down, secure event will not be handled\n");
	}

	unregister_tui_driver("DSS");
	g_active_dpu_comp = NULL;
	dpu_pr_info("-\n");
}

bool dpu_tui_is_power_on(void)
{
	dpu_check_and_return(!g_active_dpu_comp, false, warn, "Tui is unregistered");

	return g_active_dpu_comp->comp.power_on;
}

int32_t dpu_tui_get_active_display_rect(struct dkmd_rect *active_rect)
{
	struct dkmd_connector_info *pinfo = NULL;

	dpu_check_and_return(!active_rect, -1, err, "active_rect is null");
	dpu_check_and_return(!g_active_dpu_comp, -1, warn, "Tui is unregistered");

	pinfo = g_active_dpu_comp->conn_info;
	dpu_check_and_return(!pinfo, -1, err, "pinfo is null");
	dpu_check_and_return(!pinfo->get_active_display_rect, -1, err, "get_active_display_rect is null");

	return pinfo->get_active_display_rect(pinfo, active_rect);
}

int32_t dpu_tui_get_rotation(void)
{
	struct dkmd_connector_info *pinfo = NULL;

	dpu_check_and_return(!g_active_dpu_comp, -1, err, "g_active_dpu_comp is null");

	pinfo = g_active_dpu_comp->conn_info;
	dpu_check_and_return(!pinfo, -1, err, "pinfo is null");

	if (is_ppc_support(&pinfo->base))
		return TUI_DISPLAY_ROT_180;

	return TUI_DISPLAY_ROT_0;
}

void dpu_tui_frame_start_isr_handler(struct dpu_composer *dpu_comp)
{
	struct dpu_secure *secure_ctrl = NULL;
	
	secure_ctrl = &dpu_comp->secure_ctrl;
	if ((secure_ctrl->tui_ready_status == TUI_SEC_ENABLE) && (secure_ctrl->secure_status == TUI_SEC_IDLE) &&
		(secure_ctrl->secure_event == TUI_SEC_DISABLE)) {
		if (is_mipi_cmd_panel(&dpu_comp->conn_info->base) && dpu_comp->conn_info->disable_ldi) {
			dpu_comp->conn_info->disable_ldi(dpu_comp->conn_info);
			dpu_pr_info("quit tui disable ldi!");
		}
	}	
}