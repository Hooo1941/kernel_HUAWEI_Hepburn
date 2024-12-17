/* Copyright (c) 2022-2022, Hisilicon Tech. Co., Ltd. All rights reserved.
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
#include <soc_lock_interface.h>
#include"dpu/dpu_base_addr.h"
#include <dpu/soc_dpu_define.h>
#include "dkmd_log.h"
#include "dpu_config_utils.h"
#include "dpu_comp_mgr.h"
#include "gfxdev_mgr.h"
#include "dpu_sh_aod.h"
#include "dpu_dacc.h"
#include "dvfs.h"
#include "dpu_comp_smmu.h"
#include "dkmd_lcd_interface.h"

DEFINE_SEMAPHORE(hw_lock_semaphore);
DEFINE_SEMAPHORE(dpu_sensorhub_aod_blank_sem);

#define MAX_BIT 31
#define MAX_BIT_VALUE  1

static int sh_aod_blank_refcount;

extern int dpu_aod_set_blank_mode(int blank_mode);
extern bool dpu_aod_get_aod_lock_status(void);
extern bool dpu_aod_need_fast_unblank(uint32_t panel_id);
extern uint32_t dpu_aod_get_panel_id(void);
extern void dpu_aod_wait_stop_nolock(void);
extern int get_aod_support(void);

struct dpu_composer *g_dpu_composer[PANEL_ID_MAX_NUM] = {NULL, NULL};

static uint8_t is_support_aod(struct dpu_composer *dpu_comp)
{
	dpu_check_and_return(!dpu_comp->comp_mgr, 0, err, "comp_mgr is NULL");
	if ((get_aod_support() == 1) && (dpu_comp->comp_mgr->aod_enable == true))
		return 1;

	return 0;
}

bool dpu_sensorhub_aod_hw_lock(struct dpu_composer *dpu_comp)
{
	bool hw_lock_succ = false;
	bool is_timeout = false;
	uint32_t lock_status;
	uint32_t delay_count = 0;
	uint32_t timeout_count = 5000;  /* wait 5000 times */
	char __iomem *sourcelock_base;

	dpu_check_and_return(!dpu_comp, false, err, "dpu_comp is NULL");

	if (!is_support_aod(dpu_comp))
		return true;

	sourcelock_base = dpu_config_get_ip_base(DISP_IP_BASE_SOURCELOCK);
	if (!sourcelock_base) {
		dpu_pr_err("pctrl_base is null ptr!");
		return hw_lock_succ;
	}

	down(&hw_lock_semaphore);

	while (1) {
		set_reg(SOC_LOCK_RESOURCE3_LOCK_ADDR(sourcelock_base), 0x30000000, 32, 0);
		lock_status = inp32(SOC_LOCK_RESOURCE3_LOCK_ST_ADDR(sourcelock_base));
		if (((lock_status & 0x70000000) == 0x30000000) || (delay_count > timeout_count)) {
			is_timeout = (delay_count > timeout_count) ? true : false;
			break;
		}

		mdelay(1);
		++delay_count;
	}

	if (!is_timeout)
		hw_lock_succ = true;
	else
		dpu_pr_err("AP hw lock fail, lock_status = 0x%x, delay_count = %d!",
			lock_status, delay_count);

	up(&hw_lock_semaphore);

	return hw_lock_succ;
}

static bool dpu_sh_need_start_aod(struct dpu_composer *dpu_comp)
{
	uint32_t comp_index = DEVICE_COMP_PRIMARY_ID;
	struct composer *comp = NULL;
	uint32_t connect_status = DP_DISCONNECTED;

	if (dpu_comp->comp_mgr->is_power_restarting)
		return false;

	comp = get_comp_from_device_name(DEVICE_COMP_DP_ID);
	if (comp)
		connect_status = comp->base.connect_status;

	if (connect_status == DP_CONNECTED || connect_status == DP_SHORTPLUG)
		return false;

	for (comp_index = DEVICE_COMP_PRIMARY_ID; comp_index <= DEVICE_COMP_BUILTIN_ID; comp_index++) {
		if (dpu_comp->comp_mgr->power_status.refcount.value[comp_index] == 1) {
			dpu_pr_info("comp_index %d power_status is on", comp_index);
			return false;
		}
	}
	return true;
}

bool dpu_sensorhub_aod_hw_unlock(struct dpu_composer *dpu_comp)
{
	bool hw_unlock_succ = false;
	uint32_t unlock_status;
	char __iomem *sourcelock_base;

	dpu_check_and_return(!dpu_comp, false, err, "dpu_comp is NULL");

	if (!is_support_aod(dpu_comp))
		return true;

	sourcelock_base = dpu_config_get_ip_base(DISP_IP_BASE_SOURCELOCK);
	if (!sourcelock_base) {
		dpu_pr_err("pctrl_base is null point!");
		return hw_unlock_succ;
	}

	down(&hw_lock_semaphore);
	set_reg(SOC_LOCK_RESOURCE3_UNLOCK_ADDR(sourcelock_base), 0x30000000, 32, 0);
	unlock_status = inp32(SOC_LOCK_RESOURCE3_LOCK_ST_ADDR(sourcelock_base));
	hw_unlock_succ = true;
	up(&hw_lock_semaphore);

	if (!hw_unlock_succ)
		dpu_pr_err("AP hw unlock fail, unlock_status = 0x%x!", unlock_status);

	return hw_unlock_succ;
}

int dpu_sh_aod_blank(struct dpu_composer *dpu_comp, int blank_mode)
{
	int ret = 0;

	dpu_check_and_return(!dpu_comp, ret, err, "dpu_comp is NULL");
	if ((!is_builtin_panel(&dpu_comp->comp.base)) && (!is_primary_panel(&dpu_comp->comp.base)))
		return ret;

	if (!is_support_aod(dpu_comp))
		return ret;

	if (blank_mode == DISP_BLANK_UNBLANK) {
		dpu_pr_info("+, aod blank_mode %d", blank_mode);
		ret = dpu_aod_set_blank_mode(blank_mode);
		if (ret == FAST_UNLOCK_RET) {
			if (dpu_comp->conn_info != NULL && dpu_check_panel_product_type(dpu_comp->conn_info->base.fold_type)) {
				dpu_pr_info("enable_fast_unblank");
				dpu_comp->sh_aod_info.enable_fast_unblank = true;
			}
		}
	} else if (blank_mode == DISP_BLANK_POWERDOWN) {
		if (!dpu_sh_need_start_aod(dpu_comp))
			return ret;

		dpu_pr_info("+, aod blank_mode %d", blank_mode);
		ret = dpu_aod_set_blank_mode(blank_mode);
	}

	return ret;
}

void dpu_restore_fast_unblank_status(struct dpu_composer *dpu_comp)
{
	dpu_check_and_no_retval(!dpu_comp, err, "dpu_comp is NULL");

	if (!is_support_aod(dpu_comp))
		return;

	down(&dpu_comp->sh_aod_info.fast_unblank_sem);
	dpu_comp->sh_aod_info.enable_fast_unblank = false;
	up(&dpu_comp->sh_aod_info.fast_unblank_sem);
}

static void sh_aod_unblank_wq_handle(struct work_struct *work)
{
	struct dpu_composer *dpu_comp = NULL;
	struct sh_aod *dpu_sh_aod = NULL;

	dpu_sh_aod = container_of(work, struct sh_aod, aod_ud_fast_unblank_work);
	dpu_check_and_no_retval(!dpu_sh_aod, err, "dpu_sh_aod is NULL");

	dpu_comp = container_of(dpu_sh_aod, struct dpu_composer, sh_aod_info);
	dpu_check_and_no_retval(!dpu_comp, err, "dpu_comp is NULL");

	dpu_pr_info("comp index = %u", dpu_comp->comp.index);

	down(&dpu_comp->sh_aod_info.fast_unblank_sem);

	if (dpu_aod_get_aod_lock_status() == false) {
		dpu_pr_info("ap had released aod lock, stop unblank wq handle!");
		goto fast_unblank_fail;
	}

	if (dpu_comp->comp.on(&dpu_comp->comp) != 0) {
		dpu_pr_err("gfxdev power on failed!");
		goto fast_unblank_fail;
	}

	dpu_comp->sh_aod_info.is_fast_unblanking = false;

	device_mgr_primary_frame_refresh(&dpu_comp->comp, "aod");
	dpu_pr_info("-");

fast_unblank_fail:
	dpu_comp->sh_aod_info.is_fast_unblanking = false;
	up(&dpu_comp->sh_aod_info.fast_unblank_sem);
}

uint32_t dpu_get_panel_product_type(void)
{
	struct dpu_composer *dpu_comp = NULL;
	struct dkmd_connector_info *pinfo = NULL;

	uint32_t cur_panel_id = dpu_aod_get_panel_id();
	dpu_pr_info("cur_panel_id = [%u]!", cur_panel_id);
	if (cur_panel_id >= PANEL_ID_MAX_NUM)
		cur_panel_id = PANEL_ID_PRIMARY;

	dpu_comp = g_dpu_composer[cur_panel_id];
	if (!dpu_comp) {
		dpu_pr_err("dpu_comp is null ptr!");
		return PANEL_NORMAL;
	}

	pinfo = dpu_comp->conn_info;
	if (!pinfo) {
		dpu_pr_err("pinfo is null ptr!");
		return PANEL_NORMAL;
	}

	dpu_pr_info("fold_type = %u!", pinfo->base.fold_type);
	return pinfo->base.fold_type;
}

bool dpu_check_panel_product_type(uint32_t product_type)
{
	if (product_type != PANEL_NORMAL)
		return true;

	return false;
}

static struct dpu_composer* sh_aod_get_fast_unblank_comp(void)
{
	struct dpu_composer *dpu_comp = NULL;
	uint32_t i = 0;

	for (i = PANEL_ID_PRIMARY; i < PANEL_ID_MAX_NUM; i++) {
		dpu_comp = g_dpu_composer[i];
		if (!dpu_comp || !(dpu_comp->conn_info)) {
			dpu_pr_err("dpu_comp or conn_infois NULL");
			continue;
		}

		// normal panel
		down(&dpu_comp->sh_aod_info.fast_unblank_sem);
		if (!dpu_check_panel_product_type(dpu_comp->conn_info->base.fold_type)) {
			dpu_comp->sh_aod_info.is_fast_unblanking = true;
			up(&dpu_comp->sh_aod_info.fast_unblank_sem);
			return dpu_comp;
		}

		// fast unblank fold panel
		if (dpu_comp->sh_aod_info.enable_fast_unblank) {
			dpu_comp->sh_aod_info.is_fast_unblanking = true;
			up(&dpu_comp->sh_aod_info.fast_unblank_sem);
			return dpu_comp;
		}
		up(&dpu_comp->sh_aod_info.fast_unblank_sem);
	}

	dpu_pr_info("[fold] power up with fb blank!");
	return NULL;
}

void dpu_aod_schedule_wq(void)
{
	struct dpu_composer *dpu_comp = NULL;

	dpu_comp = sh_aod_get_fast_unblank_comp();
	dpu_check_and_no_retval(!dpu_comp, info, "dpu_comp is NULL");

	dpu_pr_info("comp index = %u!", dpu_comp->comp.index);
	if (dpu_comp->sh_aod_info.aod_ud_fast_unblank_workqueue)
		queue_work(dpu_comp->sh_aod_info.aod_ud_fast_unblank_workqueue,
			&dpu_comp->sh_aod_info.aod_ud_fast_unblank_work);
}

bool dpu_sh_need_fast_unblank(struct dpu_composer *dpu_comp)
{
	dpu_check_and_return(!dpu_comp, false, err, "dpu_comp is null ptr");

	if (!is_support_aod(dpu_comp))
		return false;

	dpu_pr_info("+");
	if (dpu_aod_need_fast_unblank(dpu_comp->comp.base.id)) {
		dpu_comp->sh_aod_info.enable_fast_unblank = true;
		return true;
	}

	dpu_pr_info("-");
	return false;
}

void dpu_wait_for_aod_stop(struct dpu_composer *dpu_comp)
{
	dpu_check_and_no_retval(!dpu_comp, err, "dpu_comp is NULL");
	if (!is_support_aod(dpu_comp))
		return;

	dpu_check_and_no_retval(!(dpu_comp->conn_info), err, "conn_info is NULL");
	dpu_pr_info("fold_type = %u!", dpu_comp->conn_info->base.fold_type);
	if (!dpu_check_panel_product_type(dpu_comp->conn_info->base.fold_type))
		return;

	dpu_pr_info("+");
	dpu_aod_wait_stop_nolock();
	dpu_pr_info("-");
}

void dpu_aod_wait_for_blank(struct dpu_composer *dpu_comp, int blank_mode)
{
	uint32_t try_times;

	dpu_check_and_no_retval(!dpu_comp, err, "dpu_comp is NULL");
	if (!is_support_aod(dpu_comp))
		return;

	if (blank_mode == DISP_BLANK_UNBLANK) {
		while (dpu_comp->sh_aod_info.is_fast_unblanking)
			msleep(1);  /* delay 1ms */
	}

	if ((blank_mode == DISP_BLANK_POWERDOWN) && (dpu_comp->sh_aod_info.is_fast_unblanking)) {
		dpu_pr_info("need to wait for aod fast unblank wq end!");
		try_times = 0;
		while (dpu_comp->sh_aod_info.is_fast_unblanking) {
			msleep(1);
			if (++try_times > 1000) {  /* wait times */
				dpu_pr_err("wait for aod fast unblank wq end timeout!");
				break;
			}
		}
	}
}

static void sh_create_aod_wq(struct dpu_composer *dpu_comp)
{
	/* creat aod workqueue */
	dpu_comp->sh_aod_info.aod_ud_fast_unblank_workqueue =
		alloc_ordered_workqueue("aod_ud_fast_unblank", WQ_HIGHPRI | WQ_MEM_RECLAIM);
	if (!dpu_comp->sh_aod_info.aod_ud_fast_unblank_workqueue) {
		dpu_pr_err("creat aod work queue failed!");
		return;
	}

	INIT_WORK(&dpu_comp->sh_aod_info.aod_ud_fast_unblank_work, sh_aod_unblank_wq_handle);

	if (dpu_comp->comp.base.id >= PANEL_ID_MAX_NUM) {
		dpu_pr_info("comp index = %u!", dpu_comp->comp.index);
		return;
	}
	g_dpu_composer[dpu_comp->comp.base.id] = dpu_comp;
}

void dpu_sh_aod_init(struct dpu_composer *dpu_comp)
{
	dpu_check_and_no_retval(!dpu_comp, err, "dpu_comp is NULL");
	dpu_check_and_no_retval(!dpu_comp->conn_info, err, "conn_info is NULL");
	if (dpu_comp->conn_info->aod_enable != 1)
		return;

	if (dpu_comp->comp_mgr)
		dpu_comp->comp_mgr->aod_enable = true;

	dpu_comp->sh_aod_info.is_fast_unblanking = false;
	dpu_comp->sh_aod_info.enable_fast_unblank = false;

	sema_init(&dpu_comp->sh_aod_info.fast_unblank_sem, 1);
	sh_create_aod_wq(dpu_comp);
}

void dpu_sh_aod_deinit(struct dpu_composer *dpu_comp)
{
	dpu_check_and_no_retval(!dpu_comp, err, "dpu_comp is NULL");
	dpu_check_and_no_retval(!dpu_comp->conn_info, err, "conn_info is NULL");
	if (dpu_comp->conn_info->aod_enable != 1)
		return;

	destroy_workqueue(dpu_comp->sh_aod_info.aod_ud_fast_unblank_workqueue);

	if (dpu_comp->comp.base.id >= PANEL_ID_MAX_NUM) {
		dpu_pr_info("comp index = %u!", dpu_comp->comp.index);
		return;
	}
	g_dpu_composer[dpu_comp->comp.base.id] = dpu_comp;
}

uint32_t dkmd_get_lcd_status(void)
{
	uint32_t value = 0;
	char __iomem *sctrl_base;

	sctrl_base = dpu_config_get_ip_base(DISP_IP_BASE_SCTRL);
	dpu_check_and_return(!sctrl_base, value, err, "sctrl_base is NULL");

	value = inp32(SOC_SCTRL_SCBAKDATA11_ADDR(sctrl_base));
	dpu_pr_info("value = %u!", value);
	return value;
}

void dkmd_set_lcd_status(uint32_t bit, uint32_t bit_value)
{
	char __iomem *sctrl_base = NULL;

	if (bit > MAX_BIT || bit_value > MAX_BIT_VALUE) {
		dpu_pr_err("bit = %u, bit_value = %u!", bit, bit_value);
		return;
	}
	dpu_pr_info("bit = %u, bit_value = %u!", bit, bit_value);
	sctrl_base = dpu_config_get_ip_base(DISP_IP_BASE_SCTRL);
	dpu_check_and_no_retval(!sctrl_base, err, "sctrl_base is NULL");

	set_reg(SOC_SCTRL_SCBAKDATA11_ADDR(sctrl_base), bit_value, 1, (uint8_t)bit);
}

static int sh_check_aod_blank_refcount(struct dpu_composer *dpu_comp, uint32_t msg_no)
{
	char __iomem *sctrl_base = dpu_config_get_ip_base(DISP_IP_BASE_SCTRL);
	dpu_check_and_return(!sctrl_base, -1, err, "sctrl_base is NULL!");

	sh_aod_blank_refcount++;
	dpu_pr_info("fb%d +, sh_aod_blank_refcount=%d!", dpu_comp->comp.base.id, sh_aod_blank_refcount);
	dpu_pr_info("Power State Reg is 0x%x", inp32(SOC_SCTRL_SCBAKDATA0_ADDR(sctrl_base)));

	/* high 16bit indicate msg no */
	set_reg(SOC_SCTRL_SCBAKDATA0_ADDR(sctrl_base), msg_no & 0xFFFF, 16, 16);

	if (sh_aod_blank_refcount != 1) {
		dpu_pr_err("fb%d, sh_aod_blank_refcount=%d is error", dpu_comp->comp.base.id, sh_aod_blank_refcount);
		return -1;
	}

	return 0;
}

static void sh_aod_dvfs_direct_vote(struct dpu_composer *dpu_comp, uint32_t perf_level)
{
	uint32_t value;
	uint32_t core_val;
	char __iomem *sctrl_base;

	if (dpu_comp->conn_info == NULL || dpu_comp->conn_info->update_core_clk_support == 0)
		return;

	dpu_pr_info("update_core_clk_support[%d]!", dpu_comp->conn_info->update_core_clk_support);
	sctrl_base = dpu_config_get_ip_base(DISP_IP_BASE_SCTRL);
	dpu_check_and_no_retval(!sctrl_base, err, "sctrl_base is NULL");

	/* sctrl 0x438
	* bit[14]:Sensorhub dpu core clk level
	* bit[15]:Sensorhub dpu core clk level
	*/
	value = inp32(SOC_SCTRL_SCBAKDATA11_ADDR(sctrl_base));
	core_val = ((value >> 14) & 0x3);
	if (core_val == 0) {
		dpu_pr_info("value is 0x%x", value);
		return;
	}

	dpu_dvfs_direct_vote(dpu_comp->comp.index, min(perf_level, (core_val + 1)), true);
}

int dpu_sensorhub_aod_unblank(uint32_t msg_no)
{
	int ret = 0;
	uint32_t cur_panel_id = 0;
	struct dpu_composer *dpu_comp = NULL;
	struct composer_manager *comp_mgr = NULL;
	char __iomem *sctrl_base = dpu_config_get_ip_base(DISP_IP_BASE_SCTRL);
	dpu_check_and_return(!sctrl_base, -EINVAL, err, "sctrl_base is NULL!");

	cur_panel_id = dpu_aod_get_panel_id();
	if (cur_panel_id >= PANEL_ID_MAX_NUM) {
		dpu_pr_err("[fold] not supported panel id :%d", cur_panel_id);
		return -EINVAL;
	}

	dpu_comp = g_dpu_composer[cur_panel_id];
	dpu_check_and_return(!dpu_comp, -EINVAL, err, "dpu_comp is NULL!");

	comp_mgr = dpu_comp->comp_mgr;
	dpu_check_and_return(!comp_mgr, -EINVAL, err, "comp_mgr is NULL!");

	dpu_pr_info("fb%d, msg_no is %d +", dpu_comp->comp.base.id, msg_no);

	down(&dpu_sensorhub_aod_blank_sem);
	composer_manager_power_down(dpu_comp);

	if (!dpu_sensorhub_aod_hw_lock(dpu_comp))
		goto up_blank_sem;

	if (sh_check_aod_blank_refcount(dpu_comp, msg_no))
		goto hw_unlock;

	if (comp_mgr->power_status.status == 0 && comp_mgr->power_on_stage == DPU_POWER_ON_NONE) {
		/* public function need use public data as interface parameter, such as 'comp_mgr' */
		ret = media_regulator_enable(comp_mgr);
		if (ret) {
			dpu_pr_err("media_regulator_enable fail!");
			goto hw_unlock;
		}
		composer_dpu_power_on(comp_mgr, &dpu_comp->comp);
		dpu_comp_smmuv3_on(comp_mgr, dpu_comp);
		dpu_dacc_load();
		sh_aod_dvfs_direct_vote(dpu_comp, DPU_PERF_LEVEL_MAX);
		comp_mgr->power_on_stage = DPU_POWER_ON_STAGE1;
		set_reg(SOC_SCTRL_SCBAKDATA0_ADDR(sctrl_base), 0x1, 2, 6);
		goto up_blank_sem;
	} else {
		dpu_pr_warn("fb%d already powerup!", dpu_comp->comp.base.id);
		dpu_comp_smmuv3_on(comp_mgr, dpu_comp);
		sh_aod_dvfs_direct_vote(dpu_comp, DPU_PERF_LEVEL_MAX);
		comp_mgr->power_on_stage = DPU_POWER_ON_STAGE1;
		set_reg(SOC_SCTRL_SCBAKDATA0_ADDR(sctrl_base), 0x1, 2, 6);
		goto up_blank_sem;
	}

hw_unlock:
	comp_mgr->power_on_stage = DPU_POWER_ON_NONE;
	set_reg(SOC_SCTRL_SCBAKDATA0_ADDR(sctrl_base), 0x3, 2, 6);
	if (!dpu_sensorhub_aod_hw_unlock(dpu_comp))
		dpu_pr_err("aod_hw_unlock fail!");

up_blank_sem:
	composer_manager_power_up(dpu_comp);
	up(&dpu_sensorhub_aod_blank_sem);
	dpu_pr_info("fb%d, -", dpu_comp->comp.base.id);
	return ret;
}

int dpu_sensorhub_aod_blank(uint32_t msg_no)
{
	int ret = 0;
	uint32_t cur_panel_id = 0;
	struct dpu_composer *dpu_comp = NULL;
	struct composer_manager *comp_mgr = NULL;
	char __iomem *sctrl_base = dpu_config_get_ip_base(DISP_IP_BASE_SCTRL);
	dpu_check_and_return(!sctrl_base, -EINVAL, err, "sctrl_base is NULL!");

	cur_panel_id = dpu_aod_get_panel_id();
	if (cur_panel_id >= PANEL_ID_MAX_NUM) {
		dpu_pr_err("[fold] not supported panel id :%d", cur_panel_id);
		return -EINVAL;
	}

	dpu_comp = g_dpu_composer[cur_panel_id];
	dpu_check_and_return(!dpu_comp, -EINVAL, err, "dpu_comp is NULL!");

	comp_mgr = dpu_comp->comp_mgr;
	dpu_check_and_return(!comp_mgr, -EINVAL, err, "comp_mgr is NULL!");

	dpu_pr_info("fb%d, msg_no is %d +", dpu_comp->comp.base.id, msg_no);

	down(&dpu_sensorhub_aod_blank_sem);
	composer_manager_power_down(dpu_comp);

	sh_aod_blank_refcount--;
	dpu_pr_info("sh_aod_blank_refcount=%d!", sh_aod_blank_refcount);
	dpu_pr_info("Power State Reg is 0x%x", inp32(SOC_SCTRL_SCBAKDATA0_ADDR(sctrl_base)));

	/* high 16bit indicate msg no */
	set_reg(SOC_SCTRL_SCBAKDATA0_ADDR(sctrl_base), msg_no & 0xFFFF, 16, 16);

	if (sh_aod_blank_refcount != 0) {
		set_reg(SOC_SCTRL_SCBAKDATA0_ADDR(sctrl_base), 0x3, 2, 6);
		dpu_pr_err("fb%d, sh_aod_blank_refcount=%d is error", dpu_comp->comp.base.id, sh_aod_blank_refcount);
		goto up_blank_sem;
	}

	if (comp_mgr->power_on_stage == DPU_POWER_ON_STAGE1) {
		comp_mgr->power_on_stage = DPU_POWER_ON_NONE;
		dpu_comp_smmuv3_off(comp_mgr, dpu_comp);
		if (comp_mgr->power_status.status == 0) {
			/* public function need use public data as interface parameter, such as 'comp_mgr' */
			composer_dpu_power_off(comp_mgr, &dpu_comp->comp);
			media_regulator_disable(comp_mgr);
		} else {
			/* if only aod off, cancel max perf config and recovery dvfs vote */
			sh_aod_dvfs_direct_vote(dpu_comp, DPU_CORE_LEVEL_ON);
		}
	}

	if (!dpu_sensorhub_aod_hw_unlock(dpu_comp))
		dpu_pr_err("fb%d, aod_hw_unlock fail!", dpu_comp->comp.base.id);

	dpu_pr_info("Power State Reg is 0x%x", inp32(SOC_SCTRL_SCBAKDATA0_ADDR(sctrl_base)));
	set_reg(SOC_SCTRL_SCBAKDATA0_ADDR(sctrl_base), 0x2, 2, 6);

up_blank_sem:
	composer_manager_power_up(dpu_comp);
	up(&dpu_sensorhub_aod_blank_sem);

	dpu_pr_info("fb%d, -", dpu_comp->comp.base.id);
	return ret;
}
