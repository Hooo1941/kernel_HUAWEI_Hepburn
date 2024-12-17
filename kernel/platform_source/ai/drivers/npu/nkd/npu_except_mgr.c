/*
 * npu_except_mgr.c
 *
 * Central logic for exception raising, dispathcing and handling.
 *
 * Copyright (c) 2012-2023 Huawei Technologies Co., Ltd.
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
#include "npu_except_mgr.h"

#include <linux/timer.h>
#include "npu_log.h"
#include "bbox/npu_dfx_black_box.h"
#include "npd_interface.h"
#include "npu_common_resource.h"
#include "npu_atf_subsys.h"
#include "npu_pm_framework.h"
#include "npu_proc_ctx.h"

static neh_reply_t npu_save_exception(struct npu_dev_ctx *dev_ctx, struct npu_exception_record *er)
{
	if (er->ex_source == NPU_EXCEPTION_SRC_TS && er->u.ex_hwts_report) {
		switch (er->u.ex_hwts_report->exception_type) {
		case NPU_EXCEPTION_TYPE_HWTS_TASK_EXCEPTION:
		case NPU_EXCEPTION_TYPE_HWTS_TIMEOUT_EXCEPTION:
		case NPU_EXCEPTION_TYPE_HWTS_TRAP_EXCEPTION:
			dev_ctx->except_manager.task_info.is_task_exception = 1;
			dev_ctx->except_manager.task_info.persist_stream_id = er->u.ex_hwts_report->persist_stream_id;
			dev_ctx->except_manager.task_info.persist_task_id = er->u.ex_hwts_report->persist_task_id;
			dev_ctx->except_manager.task_info.stream_id = er->u.ex_hwts_report->stream_id;
			dev_ctx->except_manager.task_info.task_id = er->u.ex_hwts_report->task_id;
			return 0;
		default:
			npu_drv_warn("ts report message type:%u", er->u.ex_hwts_report->exception_type);
			break;
		}
	}

	dev_ctx->except_manager.task_info.is_task_exception = 0;
	dev_ctx->except_manager.task_info.persist_stream_id = UINT16_MAX;
	dev_ctx->except_manager.task_info.persist_task_id = UINT16_MAX;
	dev_ctx->except_manager.task_info.stream_id = UINT16_MAX;
	dev_ctx->except_manager.task_info.task_id = UINT16_MAX;
	return 0;
}

static neh_reply_t npu_dump_exception(struct npd_registry *npd, struct npu_dev_ctx *dev_ctx,
	struct npu_exception_record *er)
{
	int ret = 0;

	switch (er->ex_source) {
	case NPU_EXCEPTION_SRC_TS:
		npd->npd_dump_exception(dev_ctx, er);
		break;
	case NPU_EXCEPTION_SRC_TASK_TIMEOUT:
#ifndef CONFIG_NPU_SWTS
		npu_rdr_exception_report(RDR_EXC_TYPE_TS_RUNNING_TIMEOUT);
#endif
		npd->npd_dump_exception(dev_ctx, er);
		break;
	default:
		npu_drv_debug("unexpected exception source %d\n", er->ex_source);
		break;
	}
	return ret;
}

static neh_reply_t npu_tm_handle_exception(struct npu_dev_ctx *dev_ctx, struct npu_exception_record *er)
{
	struct npu_proc_ctx *proc_ctx = NULL;
	struct npu_proc_ctx *next_proc_ctx = NULL;
	unused(er);

	// TODO mark all the pending tasks, they met exception...

	mutex_lock(&dev_ctx->pm.npu_power_mutex);
	list_for_each_entry_safe(proc_ctx, next_proc_ctx, &dev_ctx->proc_ctx_list,
		dev_ctx_list) {
		mutex_lock(&dev_ctx->pm.task_set_lock);
		if (list_empty_careful(&proc_ctx->task_set) == 0) {
			list_splice_tail_init(&proc_ctx->task_set, &proc_ctx->excpt_task_set);
			wake_up(&proc_ctx->report_wait);
		}
		mutex_unlock(&dev_ctx->pm.task_set_lock);
	}
	mutex_unlock(&dev_ctx->pm.npu_power_mutex);

	return NPU_EXCEPTION_HANDLED;
}
//
// unified function to dispatch exceptions based on npu_handle_exception of v210
// and the npu_exception_common_proc of others
//
static int npu_dispatch_exception(struct npu_dev_ctx *dev_ctx, struct npu_exception_record *er)
{
	int ret = 0;
	struct npd_registry *npd = get_npd();

	cond_return_error(npd == NULL, -1, "npd ptr is NULL\n");
	// step 0: save
	ret = npu_save_exception(dev_ctx, er);
	if (ret == NPU_EXCEPTION_HANDLED) {
		npu_drv_warn("exception from %d has handled", er->ex_source);
		return ret;
	}

	// step 1: dump
	ret = npu_dump_exception(npd, dev_ctx, er);
	if (ret == NPU_EXCEPTION_HANDLED) {
		npu_drv_warn("exception from %d has handled", er->ex_source);
		return ret;
	}

	dev_ctx->except_manager.status = NPU_STATUS_EXCEPTION;
	// step 2: dispatch to npd for platform specific logic
	ret = npd->npd_handle_exception(dev_ctx, er);
	if (ret == NPU_EXCEPTION_HANDLED) {
		goto COMPLETE;
	}

	// step 3: dispatch to power manager
	if (er->ex_source != NPU_EXCEPTION_SRC_GLOBAL_REG)
		npu_set_exception_bits();
	ret = npu_pm_handle_exception(dev_ctx, er);

	// step 4: dispatch to task manager
	(void)npu_tm_handle_exception(dev_ctx, er);

	if (ret != 0)
		return ret;
COMPLETE:
	dev_ctx->except_manager.status = NPU_STATUS_NORMAL;
	return ret;
}

void npu_raise_timeout_exception(struct npu_dev_ctx *dev_ctx)
{
	struct npu_exception_record er = {0};

	npu_drv_err("drv receive response wait timeout\n");
	cond_return_void(dev_ctx == NULL, "dev_ctx is null\n");

	er.ex_source = NPU_EXCEPTION_SRC_TASK_TIMEOUT;
	down_write(&dev_ctx->except_manager.status_lock);
	npu_dispatch_exception(dev_ctx, &er);
	up_write(&dev_ctx->except_manager.status_lock);
}

void npu_raise_hwts_exception(struct npu_dev_ctx *dev_ctx, struct hwts_exception_report_info *report)
{
	struct npu_exception_record er = {0};

	cond_return_void(report == NULL, "report is NULL\n");
	cond_return_void(report->exception_type >= NPU_EXCEPTION_TYPE_MAX,
		"unknown exception type\n");

	npu_drv_err("get hwts exception report info: model_id = %u, persist_stream_id = %u, persist_task_id = %u"
		", channel_id = %u, hwts_sq_id = %u, stream_id = %u, task_id = %u, exception_type = %u\n",
		report->model_id, report->persist_stream_id, report->persist_task_id, report->channel_id,
		report->hwts_sq_id, report->stream_id, report->task_id, report->exception_type);

	er.u.ex_hwts_report = report;
	er.ex_source = NPU_EXCEPTION_SRC_TS;

	down_write(&dev_ctx->except_manager.status_lock);
	npu_dispatch_exception(dev_ctx, &er);
	npu_execute_err_cnt(dev_ctx->devid, report->model_id);
	up_write(&dev_ctx->except_manager.status_lock);
}

void npu_raise_qic_exception(struct npu_dev_ctx *dev_ctx)
{
	struct npu_exception_record er = {0};

	cond_return_void(dev_ctx == NULL, "dev_ctx is null\n");

	er.ex_source = NPU_EXCEPTION_SRC_QIC;
	down_write(&dev_ctx->except_manager.status_lock);
	npu_dispatch_exception(dev_ctx, &er);
	up_write(&dev_ctx->except_manager.status_lock);
}

void npu_raise_global_reg_exception(struct npu_dev_ctx *dev_ctx)
{
	struct npu_exception_record er = {0};
	uint32_t except_bits;

	cond_return_void(dev_ctx == NULL, "dev_ctx is null\n");
	down_write(&dev_ctx->except_manager.status_lock);
	except_bits = (uint32_t)npu_get_exception_bits();
	if (except_bits == UINT32_MAX) {
		npu_drv_err("get exception bits faild");
		up_write(&dev_ctx->except_manager.status_lock);
		return;
	}

	if (dev_ctx->except_manager.status == NPU_STATUS_NORMAL) {
		if ((except_bits & NPU_GLOB_EXCEPT_MASK_LITE) != 0) {
			npu_drv_warn("exception %u found", except_bits);
			er.ex_source = NPU_EXCEPTION_SRC_GLOBAL_REG;
			npu_dispatch_exception(dev_ctx, &er);
		}
	} else {
		// wait tee or tiny power down
		while ((except_bits & NPU_GLOB_EXCEPT_MASK_ALL) != 0) {
			msleep(1);
			except_bits = (uint32_t)npu_get_exception_bits();
		}
		npu_drv_warn("enter exception pu\n");
		(void)npu_pm_handle_power_up(dev_ctx);
		dev_ctx->except_manager.status = NPU_STATUS_NORMAL;
	}

	up_write(&dev_ctx->except_manager.status_lock);
}

void npu_raise_server_released_exception(struct npu_dev_ctx *dev_ctx)
{
	struct npu_exception_record er = {0};

	npu_drv_err("npu server released");
	cond_return_void(dev_ctx == NULL, "dev_ctx is null\n");

	er.ex_source = NPU_EXCEPTION_SRC_SERVER;
	down_write(&dev_ctx->except_manager.status_lock);
	npu_dispatch_exception(dev_ctx, &er);
	up_write(&dev_ctx->except_manager.status_lock);
}

// get exception bits in hw register via ATF
int32_t npu_get_exception_bits(void)
{
	int32_t ret;
	struct timespec64 start;
	struct timespec64 stop;
	u64 cost_time;
	struct npd_registry *npd = get_npd();

	cond_return_error(npd == NULL, -1, "npd ptr is NULL\n");
	if ((npd->feature_mask & NPU_FEATUE_GLOBAL_EXCEPTION_REG) == 0)
		return 0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
	ktime_get_coarse_real_ts64(&start);
#else
	start = current_kernel_time();
#endif
	ret = atfd_service_npu_smc(NPU_EXCEPTION_GET_FLAG, 0, 0, 0);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
	ktime_get_coarse_real_ts64(&stop);
#else
	stop = current_kernel_time();
#endif
	cost_time = (u64)((stop.tv_sec * 1000 + stop.tv_nsec / 1000000) - (start.tv_sec * 1000 + start.tv_nsec / 1000000));
	if (ret != 0) {
		npu_drv_warn("get exception bits:0x%x, cost_time:%ull", ret, cost_time);
		return ret;
	}
	if (cost_time > NPU_ATF_TIMEOUT_IN_US)
		npu_drv_err("get exception timeout:%ull", cost_time);

	return ret;
}

void npu_set_exception_bits(void)
{
	int32_t ret;
	struct timespec64 start;
	struct timespec64 stop;
	u64 cost_time;
	struct npd_registry *npd = get_npd();

	cond_return_void(npd == NULL, "npd ptr is NULL\n");
	npu_drv_debug("set exception bits start");
	if ((npd->feature_mask & NPU_FEATUE_GLOBAL_EXCEPTION_REG) == 0)
		return;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
	ktime_get_coarse_real_ts64(&start);
#else
	start = current_kernel_time();
#endif
	ret = atfd_service_npu_smc(NPU_EXCEPTION_SET_FLAG, 0, 0, 0);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
	ktime_get_coarse_real_ts64(&stop);
#else
	stop = current_kernel_time();
#endif
	cost_time = (u64)((stop.tv_sec * 1000 + stop.tv_nsec / 1000000) - (start.tv_sec * 1000 + start.tv_nsec / 1000000));

	if (cost_time > NPU_ATF_TIMEOUT_IN_US)
		npu_drv_err("set exception timeout:%ull", cost_time);

	npu_drv_debug("set exception bits end");
}

void npu_clear_exception_bits(void)
{
	int32_t ret;
	struct timespec64 start;
	struct timespec64 stop;
	u64 cost_time;
	struct npd_registry *npd = get_npd();

	cond_return_void(npd == NULL, "npd ptr is NULL\n");
	npu_drv_debug("clear exception bits start");
	if ((npd->feature_mask & NPU_FEATUE_GLOBAL_EXCEPTION_REG) == 0)
		return;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
	ktime_get_coarse_real_ts64(&start);
#else
	start = current_kernel_time();
#endif
	ret = atfd_service_npu_smc(NPU_EXCEPTION_CLEAR_FLAG, 0, 0, 0);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
	ktime_get_coarse_real_ts64(&stop);
#else
	stop = current_kernel_time();
#endif
	cost_time = (u64)((stop.tv_sec * 1000 + stop.tv_nsec / 1000000) - (start.tv_sec * 1000 + start.tv_nsec / 1000000));

	if (ret != 0) {
		npu_drv_err("clear exception bits failed, ret:%d, cost_time:%ull", ret, cost_time);
		return;
	}
	if (cost_time > NPU_ATF_TIMEOUT_IN_US)
		npu_drv_err("clear exception timeout:%ull", cost_time);

	npu_drv_debug("clear exception bits end");
}