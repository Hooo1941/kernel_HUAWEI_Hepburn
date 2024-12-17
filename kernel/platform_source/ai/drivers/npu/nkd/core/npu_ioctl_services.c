/*
 * npu_ioctl_service.c
 *
 * about npu ioctl service
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#include "npu_ioctl_services.h"

#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/swap.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <asm/uaccess.h>
#include <linux/version.h>
#include <platform_include/basicplatform/linux/rdr_pub.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#include <linux/mman.h>
#include <securec.h>
#include <linux/iommu/mm_svm.h>

#include "npu_common.h"
#include "npu_calc_channel.h"
#include "npu_calc_cq.h"
#include "npu_stream.h"
#include "npu_shm.h"
#include "npu_log.h"
#include "npu_mailbox.h"
#include "npu_event.h"
#include "npu_notify.h"
#include "npu_hwts_event.h"
#include "npu_model.h"
#include "npu_task.h"
#include "bbox/npu_dfx_black_box.h"
#include "npu_heart_beat.h"
#include "npu_calc_sq.h"
#include "npu_comm_sqe_fmt.h"
#include "npu_pool.h"
#include "npu_iova.h"
#include "npu_svm.h"
#include "npu_doorbell.h"
#include "npu_message.h"
#include "npu_pm_framework.h"
#include "npu_rt_task.h"
#ifdef CONFIG_NPU_FFRT
#include "npu_soc_defines.h"
#endif
#ifdef CONFIG_NPU_SWTS
#include "schedule_interface.h"
#endif
#include "npd_interface.h"
#ifdef CONFIG_NPU_SYSCACHE
#include "npu_syscache.h"
#endif
#include "npu_user_common.h"

static struct npu_toughness_info g_npu_toughness_info = {0};

int npu_ioctl_alloc_stream(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	int ret;
	npu_stream_alloc_ioctl_info_t *para =
		(npu_stream_alloc_ioctl_info_t *)((uintptr_t)arg);
	npu_stream_alloc_ioctl_info_t para_1 = {0};
	struct npu_stream_info *stream_info = NULL;

	npu_drv_debug("enter\n");

	if (copy_from_user_safe(&para_1, (void __user *)(uintptr_t)arg,
		sizeof(npu_stream_alloc_ioctl_info_t))) {
		npu_drv_err("copy from user safe error\n");
		return -EFAULT;
	}

	mutex_lock(&proc_ctx->stream_mutex);
	ret = npu_proc_alloc_stream(proc_ctx, &stream_info, para_1.strategy,
		para_1.priority);
	if (ret != 0) {
		npu_drv_err("npu alloc stream failed\n");
		mutex_unlock(&proc_ctx->stream_mutex);
		return -ENOKEY;
	}

	if (copy_to_user_safe((void __user *)(&(para->stream_id)),
		&stream_info->id, sizeof(int))) {
		npu_drv_err("copy to user safe stream_id = %d error\n",
			stream_info->id);
		ret = npu_proc_free_stream(proc_ctx, stream_info->id);
		if (ret != 0)
			npu_drv_err("npu_ioctl_free_stream_id = %d error\n",
				stream_info->id);

		mutex_unlock(&proc_ctx->stream_mutex);
		return -EFAULT;
	}
	mutex_unlock(&proc_ctx->stream_mutex);

	npu_drv_debug("end\n");
	return 0;
}

int npu_ioctl_alloc_event(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	int ret;
	u16 strategy;
	u32 event_id = 0;
	u32 max_event_id = 0;
	npu_event_alloc_ioctl_info_t para = {0};
	npu_event_alloc_ioctl_info_t *para_1 =
		(npu_event_alloc_ioctl_info_t *)((uintptr_t)arg);
	struct npu_platform_info *plat_info = npu_plat_get_info();

	cond_return_error(plat_info == NULL, -EINVAL, "get plat info fail\n");

	if (copy_from_user_safe(&para, (void __user *)(uintptr_t)arg,
		sizeof(npu_event_alloc_ioctl_info_t))) {
		npu_drv_err("copy from user safe error\n");
		return -1;
	}

	if (para.strategy >= EVENT_STRATEGY_TS) {
		npu_drv_err("proc alloc event failed, invalid input strategy: %u\n",
			para.strategy);
		return -EINVAL;
	} else if (para.strategy == EVENT_STRATEGY_SINK &&
		plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 1) {
		strategy = EVENT_STRATEGY_HWTS;
		max_event_id = NPU_MAX_HWTS_EVENT_ID;
	} else {
		strategy = EVENT_STRATEGY_TS;
		max_event_id = NPU_MAX_EVENT_ID;
	}

	mutex_lock(&proc_ctx->event_mutex);
	ret = npu_proc_alloc_event(proc_ctx, &event_id, strategy);
	if (ret != 0) {
		npu_drv_err("proc alloc event failed, event id: %u\n", event_id);
		mutex_unlock(&proc_ctx->event_mutex);
		return -1;
	}

	if (copy_to_user_safe((void __user *)(&(para_1->event_id)), &event_id,
		sizeof(int))) {
		npu_drv_err("copy to user safe event_id = %u error\n", event_id);
		ret = npu_proc_free_event(proc_ctx, event_id, strategy);
		if (ret != 0)
			npu_drv_err("proc free event id failed, event id: %u\n", event_id);

		mutex_unlock(&proc_ctx->event_mutex);
		return -1;
	}

	mutex_unlock(&proc_ctx->event_mutex);
	return 0;
}

int npu_ioctl_alloc_model(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	int ret;
	u32 model_id = 0;

	mutex_lock(&proc_ctx->model_mutex);
	ret = npu_proc_alloc_model(proc_ctx, &model_id);
	if (ret != 0) {
		npu_drv_err("proc alloc model failed, model id: %d\n", model_id);
		mutex_unlock(&proc_ctx->model_mutex);
		return -EFAULT;
	}

	if (copy_to_user_safe((void __user *)(uintptr_t)arg, &model_id, sizeof(int))) {
		npu_drv_err("copy to user safe model_id = %d error\n", model_id);

		ret = npu_proc_free_model(proc_ctx, model_id);
		if (ret != 0)
			npu_drv_err("proc free model id failed, model id: %d\n", model_id);

		mutex_unlock(&proc_ctx->model_mutex);
		return -EFAULT;
	}

	mutex_unlock(&proc_ctx->model_mutex);
	return 0;
}

int npu_ioctl_alloc_task(struct npu_proc_ctx *proc_ctx, unsigned long arg)
{
	int ret;
	u32 task_id = 0;

	mutex_lock(&proc_ctx->task_mutex);
	ret = npu_proc_alloc_task(proc_ctx, &task_id);
	if (ret != 0) {
		npu_drv_err("proc alloc task failed, task id: %d\n", task_id);
		mutex_unlock(&proc_ctx->task_mutex);
		return -EFAULT;
	}

	if (copy_to_user_safe((void __user *)(uintptr_t)arg, &task_id, sizeof(int))) {
		npu_drv_err("copy to user safe task_id = %d error\n", task_id);
		ret = npu_proc_free_task(proc_ctx, task_id);
		if (ret != 0)
			npu_drv_err("proc free task id failed, task id: %d\n", task_id);

		mutex_unlock(&proc_ctx->task_mutex);
		return -EFAULT;
	}

	mutex_unlock(&proc_ctx->task_mutex);
	return 0;
}

int npu_ioctl_alloc_notify(struct npu_proc_ctx *proc_ctx, unsigned long arg)
{
	int ret;
	u32 notify_id = 0;

	mutex_lock(&proc_ctx->notify_mutex);
	ret = npu_proc_alloc_notify(proc_ctx, &notify_id);
	if (ret != 0) {
		npu_drv_err("proc alloc notify failed, notify id: %u\n", notify_id);
		mutex_unlock(&proc_ctx->notify_mutex);
		return -1;
	}

	if (copy_to_user_safe((void __user *)(uintptr_t)arg, &notify_id, sizeof(u32))) {
		npu_drv_err("copy to user safe notify_id = %u error\n", notify_id);
		ret = npu_proc_free_notify(proc_ctx, notify_id);
		if (ret != 0)
			npu_drv_err("proc free event id failed, notify_id: %u\n", notify_id);

		mutex_unlock(&proc_ctx->notify_mutex);
		return -1;
	}

	mutex_unlock(&proc_ctx->notify_mutex);
	return 0;
}

int npu_ioctl_free_stream(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	int ret;
	struct npu_stream_free_ioctl_info para = {0};

	if (copy_from_user_safe(&para, (void __user *)(uintptr_t)arg, sizeof(para))) {
		npu_drv_err("copy from user safe error\n");
		return -EFAULT;
	}

	mutex_lock(&proc_ctx->stream_mutex);
	ret = npu_proc_check_stream_id(proc_ctx, (u32)para.stream_id);
	if (ret != 0) {
		npu_drv_err("check para fail\n");
		mutex_unlock(&proc_ctx->stream_mutex);
		return -EFAULT;
	}

	ret = npu_proc_free_stream(proc_ctx, (u32)para.stream_id);
	mutex_unlock(&proc_ctx->stream_mutex);
	cond_return_error(ret != 0, -EINVAL,
		"npu ioctl free stream_id = %d error\n", para.stream_id);
	return ret;
}

int npu_ioctl_free_notify(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	int ret;
	u32 notify_id = 0;

	if (copy_from_user_safe(&notify_id, (void __user *)(uintptr_t)arg, sizeof(u32))) {
		npu_drv_err("copy from user safe error\n");
		return -EFAULT;
	}

	mutex_lock(&proc_ctx->notify_mutex);
	ret = npu_proc_check_notify_id(proc_ctx, (u32)notify_id);
	if (ret != 0) {
		npu_drv_err("check para fail\n");
		mutex_unlock(&proc_ctx->notify_mutex);
		return -EFAULT;
	}

	ret = npu_proc_free_notify(proc_ctx, notify_id);
	mutex_unlock(&proc_ctx->notify_mutex);
	cond_return_error(ret != 0, -EINVAL,
		"npu ioctl free notify_id = %u error\n", notify_id);
	return ret;
}

int npu_ioctl_free_event(struct npu_proc_ctx *proc_ctx, unsigned long arg)
{
	int ret;
	u16 strategy;
	npu_event_free_ioctl_info_t para = {0};
	struct npu_platform_info *plat_info = npu_plat_get_info();

	cond_return_error(plat_info == NULL, -EINVAL, "get plat info fail\n");

	if (copy_from_user_safe(&para, (void __user *)(uintptr_t)arg, sizeof(para))) {
		npu_drv_err("copy from user safe error\n");
		return -EFAULT;
	}

	mutex_lock(&proc_ctx->event_mutex);
	if (para.strategy >= EVENT_STRATEGY_TS) {
		npu_drv_err("strategy %u\n", para.strategy);
		mutex_unlock(&proc_ctx->event_mutex);
		return -EINVAL;
	} else if (para.strategy == EVENT_STRATEGY_SINK &&
		plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 1) {
		ret = npu_proc_check_hwts_event_id(proc_ctx, para.event_id);
		if (ret != 0) {
			npu_drv_err("check para fail\n");
			mutex_unlock(&proc_ctx->event_mutex);
			return -EFAULT;
		}
		strategy = EVENT_STRATEGY_HWTS;
	} else {
		ret = npu_proc_check_event_id(proc_ctx, (u32)para.event_id);
		if (ret != 0) {
			npu_drv_err("check para fail\n");
			mutex_unlock(&proc_ctx->event_mutex);
			return -EFAULT;
		}
		strategy = EVENT_STRATEGY_TS;
	}

	ret = npu_proc_free_event(proc_ctx, (u32)para.event_id, strategy);
	mutex_unlock(&proc_ctx->event_mutex);
	cond_return_error(ret != 0, -EINVAL,
		"free event id = %d error\n", para.event_id);

	return ret;
}

int npu_ioctl_free_model(struct npu_proc_ctx *proc_ctx, unsigned long arg)
{
	int ret;
	int model_id = 0;

	if (copy_from_user_safe(&model_id, (void __user *)(uintptr_t)arg, sizeof(int))) {
		npu_drv_err("copy from user safe error\n");
		return -EFAULT;
	}

	mutex_lock(&proc_ctx->model_mutex);
	ret = npu_proc_check_model_id(proc_ctx, (u32)model_id);
	if (ret != 0) {
		npu_drv_err("check para fail\n");
		mutex_unlock(&proc_ctx->model_mutex);
		return -EFAULT;
	}

	ret = npu_proc_free_model(proc_ctx, (u32)model_id);
	mutex_unlock(&proc_ctx->model_mutex);
	cond_return_error(ret != 0, -EINVAL,
		"free model id = %d error\n", model_id);
	return ret;
}

int npu_ioctl_free_task(struct npu_proc_ctx *proc_ctx, unsigned long arg)
{
	int ret;
	int task_id = 0;

	if (copy_from_user_safe(&task_id, (void __user *)(uintptr_t)arg, sizeof(int))) {
		npu_drv_err("copy from user safe error\n");
		return -EFAULT;
	}

	mutex_lock(&proc_ctx->task_mutex);
	ret = npu_proc_check_task_id(proc_ctx, (u32)task_id);
	if (ret != 0) {
		npu_drv_err("check para fail\n");
		mutex_unlock(&proc_ctx->task_mutex);
		return -EFAULT;
	}

	ret = npu_proc_free_task(proc_ctx, (u32)task_id);
	mutex_unlock(&proc_ctx->task_mutex);
	cond_return_error(ret != 0, -EINVAL,
		"free task id = %d error\n", task_id);

	return ret;
}

int npu_ioctl_get_chip_info(struct npu_proc_ctx *proc_ctx, unsigned long arg)
{
	int ret;
	struct npu_chip_info info = {0};
	struct npu_platform_info *plat_info = NULL;
	struct npu_mem_desc *l2_desc = NULL;

	npu_drv_debug("arg = 0x%llx\n", arg);
	unused(proc_ctx);
	plat_info = npu_plat_get_info();
	if (plat_info == NULL) {
		npu_drv_err("get plat_info failed\n");
		return -EFAULT;
	}

	l2_desc = &plat_info->dts_info.reg_desc[NPU_REG_L2BUF_BASE];
	if (l2_desc == NULL) {
		npu_drv_err("npu_plat_get_reg_desc failed\n");
		return -EFAULT;
	}
	if (plat_info->dts_info.feature_switch[NPU_FEATURE_L2_BUFF] == 1) {
		info.l2_size = (uint32)l2_desc->len;
	} else {
		info.l2_size = 0;
	}

	ret = copy_to_user_safe((void __user *)(uintptr_t)arg, &info, sizeof(info));
	if (ret != 0) {
		npu_drv_err("fail to copy chip_info params to user space,ret = %d\n",
			ret);
		return -EINVAL;
	}

	return ret;
}

int npu_ioctl_get_svm_ssid(struct npu_proc_ctx *proc_ctx, unsigned long arg)
{
	int ret;
	uint64_t ttbr;
	uint64_t tcr;
	u16 ssid = 0;
	struct process_info info = {0};

	ret = copy_from_user_safe(&info, (void __user *)(uintptr_t)arg,
		sizeof(info));
	if (ret != 0) {
		npu_drv_err("fail to copy process_info params, ret = %d\n",
			ret);
		return -EINVAL;
	}

	ret = npu_get_ssid_bypid(proc_ctx->devid, current->tgid,
		info.vpid, &ssid, &ttbr, &tcr);
	if (ret != 0) {
		npu_drv_err("fail to get ssid, ret = %d\n", ret);
		return ret;
	}

	info.pasid = ssid;

	npu_drv_debug("pid=%d get ssid 0x%x\n", current->pid, info.pasid);

	ret = copy_to_user_safe((void __user *)(uintptr_t)arg, &info, sizeof(info));
	if (ret != 0) {
		npu_drv_err("fail to copy process info params to user space,"
			"ret = %d\n", ret);
		return -EINVAL;
	}

	return ret;
}

int npu_ioctl_enter_workmode(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	int ret;
	npu_work_mode_info_t wm_info;
	struct npu_dev_ctx *dev_ctx = NULL;

	dev_ctx = get_dev_ctx_by_id(proc_ctx->devid);
	cond_return_error(dev_ctx == NULL, -1, "invalid dev ctx\n");

	ret = copy_from_user_safe(&wm_info,
		(void __user *)(uintptr_t)arg, sizeof(wm_info));
	cond_return_error(ret != 0,
		-EINVAL, "fail to copy work_mode_info params, ret = %d\n", ret);

	cond_return_error(wm_info.work_mode >= NPU_WORKMODE_MAX,
		-EINVAL, "invalid work_mode = %u\n", wm_info.work_mode);

	npu_drv_info("work mode %u flags 0x%x\n", wm_info.work_mode, wm_info.flags);

	ret = npu_pm_proc_ioctl_enter_wm(proc_ctx, dev_ctx, &wm_info);
	cond_return_error(ret != 0,
		-EINVAL, "fail to enter work_wode: %u, ret = %d\n", wm_info.work_mode, ret);

	return 0;
}

int npu_ioctl_exit_workmode(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	int ret;
	npu_work_mode_info_t wm_info;
	struct npu_dev_ctx *dev_ctx = NULL;

	dev_ctx = get_dev_ctx_by_id(proc_ctx->devid);
	cond_return_error(dev_ctx == NULL, -1, "invalid dev ctx\n");

	ret = copy_from_user_safe(&wm_info,
		(void __user *)(uintptr_t)arg, sizeof(wm_info));
	cond_return_error(ret != 0,
		-EINVAL, "fail to copy work_mode_info params, ret = %d\n", ret);

	cond_return_error(wm_info.work_mode >= NPU_WORKMODE_MAX,
		-EINVAL, "invalid work_mode = %u\n", wm_info.work_mode);

	npu_drv_debug("work_mode = %u\n", wm_info.work_mode);

	ret = npu_pm_proc_ioctl_exit_wm(proc_ctx, dev_ctx, wm_info.work_mode);
	cond_return_error(ret != 0,
		-EINVAL, "fail to exit work_wode: %u, ret = %d\n", wm_info.work_mode, ret);

	return 0;
}

int npu_ioctl_set_limit(struct npu_proc_ctx *proc_ctx, unsigned long arg)
{
	int ret = 0;
#ifndef NPU_ARCH_V100
	u32 value;
	struct npu_limit_time_info limit_time_info = {0, 0};
	struct npu_dev_ctx *dev_ctx = NULL;

	dev_ctx = get_dev_ctx_by_id(proc_ctx->devid);
	cond_return_error(dev_ctx == NULL, -1, "invalid dev ctx\n");

	ret = copy_from_user_safe(&limit_time_info,
		(void __user *)(uintptr_t)arg, sizeof(limit_time_info));
	if (ret != 0) {
		npu_drv_err("fail to copy sec_mode_info params, ret = %d\n", ret);
		return -EINVAL;
	}

	value = limit_time_info.time_out;
	if (limit_time_info.type == NPU_LOW_POWER_TIMEOUT) {
		cond_return_error(value < NPU_IDLE_TIME_OUT_MIN_VALUE,
			-EINVAL, "value :%u is too small\n", value);
		dev_ctx->pm.npu_idle_time_out = value;
	} else if (limit_time_info.type == NPU_STREAM_SYNC_TIMEOUT) {
		cond_return_error(value < NPU_TASK_TIME_OUT_MIN_VALUE,
			-EINVAL, "value :%u is too small\n", value);
		cond_return_error(value > NPU_TASK_TIME_OUT_MAX_VALUE,
			-EINVAL, "value :%u is too big\n", value);
		dev_ctx->npu_task_time_out = value;
	} else {
		ret = -1;
		npu_drv_err("limit type wrong!, type = %u\n", limit_time_info.type);
	}
#else
	unused(proc_ctx);
	unused(arg);
#endif

	return ret;
}

int npu_ioctl_enable_feature(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	int ret;
	uint32_t feature_id;

	ret = copy_from_user_safe(&feature_id,
		(void __user *)(uintptr_t)arg, sizeof(uint32_t));
	if (ret != 0) {
		npu_drv_err("copy_from_user_safe error\n");
		return -EINVAL;
	}
	ret = npu_feature_enable(proc_ctx, feature_id, 1);

	return ret;
}

int npu_ioctl_disable_feature(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	int ret;
	uint32_t feature_id;

	ret = copy_from_user_safe(&feature_id,
		(void __user *)(uintptr_t)arg, sizeof(uint32_t));
	if (ret != 0) {
		npu_drv_err("copy_from_user_safe error\n");
		return -EINVAL;
	}

	ret = npu_feature_enable(proc_ctx, feature_id, 0);
	return ret;
}

#ifdef CONFIG_NPU_HWTS
static int npu_verify_model_task_info(npu_model_desc_t *model_desc, u8 idx)
{
	if ((model_desc->stream_id[idx] <
		(NPU_MAX_NON_SINK_STREAM_ID + NPU_MAX_LONG_HWTS_SQ_NUM)) &&
		(model_desc->stream_tasks[idx] > NPU_MAX_LONG_HWTS_SQ_DEPTH ||
		model_desc->stream_tasks[idx] == 0)) {
		npu_drv_err("user sink long stream_tasks[%u]= %u invalid\n",
			idx, model_desc->stream_tasks[idx]);
		return -1;
	}
	if ((model_desc->stream_id[idx] >=
		(NPU_MAX_NON_SINK_STREAM_ID + NPU_MAX_LONG_HWTS_SQ_NUM)) &&
		(model_desc->stream_tasks[idx] > NPU_MAX_HWTS_SQ_DEPTH ||
		model_desc->stream_tasks[idx] == 0)) {
		npu_drv_err("user sink stream_tasks[%u]= %u invalid\n",
			idx, model_desc->stream_tasks[idx]);
		return -1;
	}
	return 0;
}

static int npu_load_hwts_sqe(struct npu_proc_ctx *proc_ctx,
	struct npu_sink_stream_info *sink_stream_info,
	npu_model_desc_info_t *model_desc_info,
	struct npu_entity_info *hwts_sq_sub_info, u32 *core_type)
{
	void *stream_buf_addr = NULL;
	int ret, sqe_num;
	u32 stream_len;
	struct npd_registry *npd = get_npd();

	cond_return_error(npd == NULL, -EINVAL, "npd ptr is NULL\n");
	cond_return_error(sink_stream_info == NULL, -EINVAL, "sink_stream_info is invalid\n");
	cond_return_error(model_desc_info == NULL, -EINVAL,
		"model_desc_info is invalid\n");
	cond_return_error(hwts_sq_sub_info == NULL, -EINVAL,
		"hwts_sq_sub_info is invalid\n");
	// copy tasks of one stream to temp buff
	stream_len = sink_stream_info->task_num * NPU_RT_TASK_SIZE;
	stream_buf_addr = vmalloc(stream_len);
	cond_return_error(stream_buf_addr == NULL, -ENOMEM,
		"vmalloc stream_buf memory size= %u failed\n", stream_len);

	ret = copy_from_user_safe(stream_buf_addr,
		(void __user *)(uintptr_t)sink_stream_info->stream_addr,
		stream_len);
	if (ret != 0) {
		vfree(stream_buf_addr);
		npu_drv_err("fail to copy stream_buf, ret= %d\n", ret);
		return -EINVAL;
	}

	// verify hwts_sqe
	ret = npd->npd_verify_sink_tasks(proc_ctx, stream_buf_addr, stream_len, core_type);
	if (ret != 0) {
		vfree(stream_buf_addr);
		npu_drv_err("npu_verify_hwts_sqe fail, length= %u, ret= %d\n",
			stream_len, ret);
		return -1;
	}

	// save hwts_sqe
	npu_drv_debug("hwts_sq_vir_addr:0x%lx, stream_buf_addr:%pK, stream_len:%u\n",
		hwts_sq_sub_info->vir_addr, stream_buf_addr, stream_len);
	sqe_num = npd->npd_format_hw_sqe(model_desc_info, stream_buf_addr,
		(void *)(uintptr_t)hwts_sq_sub_info->vir_addr, stream_len);

	vfree(stream_buf_addr);
	return sqe_num;
}

#ifdef CONFIG_NPU_FFRT
static int npu_ffrt_load_mdc_buf(npu_model_desc_t *model_desc, uint16_t uid)
{
	int ret = 0;
	uint32_t model_id = model_desc->model_id;
	struct npu_ffrt_model_buff* ffrt_mdc_buf =
		(struct npu_ffrt_model_buff *)get_shm_virt_addr(0, NPU_FFRT_MDC_MEM) + model_id;
	struct npu_hwts_static_swap_buff* ffrt_swap_buf_phy =
		(struct npu_hwts_static_swap_buff *)get_shm_phy_addr(0, NPU_FFRT_SWAP_MEM) + (2 * model_id);

	/* mdl buff config, reserved err_mode pmu_id, only 1 stream */
	ret = memset_s(ffrt_mdc_buf, sizeof(struct npu_ffrt_model_buff), 0, sizeof(struct npu_ffrt_model_buff));
	cond_return_error(ret != 0, -1, "memset_s mdc_buf failed");

	ffrt_mdc_buf->va = 0;
	ffrt_mdc_buf->ssid = uid;
	ffrt_mdc_buf->r0_vld = 1;
	ffrt_mdc_buf->p0_pri = model_desc->priority;
	ffrt_mdc_buf->r0_head = 0;
	ffrt_mdc_buf->r0_tail = model_desc->stream_tasks[0];
	ffrt_mdc_buf->r0_swapbuf_addr_l = (uint32_t)((uint64_t)ffrt_swap_buf_phy & UINT32_MAX);
	ffrt_mdc_buf->r0_swapbuf_addr_h = (uint16_t)(((uint64_t)ffrt_swap_buf_phy >> 32) & UINT16_MAX);

	if (model_desc->stream_cnt == 2) {
		struct npu_hwts_static_swap_buff* ffrt_swap_buf_phy_1 = ffrt_swap_buf_phy + 1;
		ffrt_mdc_buf->r1_vld = 1;
		ffrt_mdc_buf->p1_pri = model_desc->priority;
		ffrt_mdc_buf->r1_head = 0;
		ffrt_mdc_buf->r1_tail = model_desc->stream_tasks[1];
		ffrt_mdc_buf->r1_swapbuf_addr_l = (uint32_t)((uint64_t)ffrt_swap_buf_phy_1 & UINT32_MAX);
		ffrt_mdc_buf->r1_swapbuf_addr_h = (uint16_t)(((uint64_t)ffrt_swap_buf_phy_1 >> 32) & MAX_UINT16_NUM);
	}
	return ret;
}

static int npu_ffrt_load_swap_buf_index(struct npu_proc_ctx *proc_ctx, npu_model_desc_t *model_desc,
	int swap_buf_index, uint16_t uid)
{
	struct npu_stream_info *stream_info = NULL;
	struct npu_hwts_sq_info *hwts_sq_info = NULL;
	struct npu_hwts_cq_info *hwts_cq_info = NULL;
	uint32_t model_id = model_desc->model_id;
	u8 devid = proc_ctx->devid;
	struct npu_hwts_static_swap_buff* ffrt_swap_buf_index =
		(struct npu_hwts_static_swap_buff *)get_shm_virt_addr(0, NPU_FFRT_SWAP_MEM) + (2 * model_id) + swap_buf_index;

	/* get swap buff config */
	stream_info = npu_calc_stream_info(devid, model_desc->stream_id[swap_buf_index]);
	cond_return_error(stream_info == NULL, -1,
		"stream_id= %d stream_info is null\n", model_desc->stream_id[swap_buf_index]);

	hwts_sq_info = npu_calc_hwts_sq_info(devid, stream_info->sq_index);
	cond_return_error(hwts_sq_info == NULL, -1,
		"stream= %d, sq_index= %u, hwts_sq_info is null\n",
		stream_info->id, stream_info->sq_index);

	hwts_cq_info = npu_calc_hwts_cq_info(devid, stream_info->cq_index);
	cond_return_error(hwts_cq_info == NULL, -1,
		"stream= %d, cq_index= %u, hwts_cq_info is null\n",
		stream_info->id, stream_info->cq_index);

	/* sq base addr */
	ffrt_swap_buf_index->sq_base_addr0.reg.rtsq_base_addr_l = hwts_sq_info->iova_addr & UINT32_MAX;
	ffrt_swap_buf_index->sq_base_addr1.reg.rtsq_base_addr_h = (hwts_sq_info->iova_addr >> 32) & MAX_UINT16_NUM;
	ffrt_swap_buf_index->sq_base_addr1.reg.rtsq_base_addr_is_virtual = 1;
	/* sq cfg, reserved for config, ssid, rtsq_rtcqid */
	ffrt_swap_buf_index->sq_cfg0.reg.rtsq_smmu_substream_id = uid;
	ffrt_swap_buf_index->sq_cfg1.reg.rtsq_rtcqid = stream_info->cq_index; // reserved cq id
	ffrt_swap_buf_index->sq_cfg1.reg.rtsq_log_bufid = 0;
	ffrt_swap_buf_index->sq_cfg1.reg.rtsq_profile_bufid = 0;
	ffrt_swap_buf_index->sq_cfg1.reg.rtsq_log_task_en = 0;
	ffrt_swap_buf_index->sq_cfg1.reg.rtsq_log_block_en = 0;
	ffrt_swap_buf_index->sq_cfg1.reg.rtsq_profile_task_en = 0;
	ffrt_swap_buf_index->sq_cfg1.reg.rtsq_profile_block_en = 0;
	ffrt_swap_buf_index->sq_cfg1.reg.rtsq_rsp_mode = 0;
	ffrt_swap_buf_index->sq_cfg1.reg.rtsq_sat_mode = 0;
	ffrt_swap_buf_index->sq_cfg1.reg.rtsq_ban_wr_cqe = 1;
	ffrt_swap_buf_index->sq_cfg1.reg.rtsq_smmu_two_stage_en = 0;
	ffrt_swap_buf_index->sq_cfg2.reg.rtsq_aiv_poolid = 0;
	ffrt_swap_buf_index->sq_cfg2.reg.rtsq_gen_aic_poolid = 0;
	ffrt_swap_buf_index->sq_cfg2.reg.rtsq_eff_aic_poolid = 0;
	ffrt_swap_buf_index->sq_cfg4.reg.rtsq_length = model_desc->stream_tasks[swap_buf_index];
	/* cq cfg  */
	ffrt_swap_buf_index->cq_base_addr0.reg.rtcq_base_addr_l = hwts_cq_info->iova_addr & UINT32_MAX;
	ffrt_swap_buf_index->cq_base_addr1.reg.rtcq_base_addr_h = (hwts_cq_info->iova_addr >> 32) & UINT32_MAX;
	ffrt_swap_buf_index->cq_base_addr1.reg.rtcq_base_addr_is_virtual = 1;
	ffrt_swap_buf_index->cq_cfg0.reg.rtcq_length = 1024;
	ffrt_swap_buf_index->sq_context_switch_ctrl0.reg.rtsq_context_switch_buf_base_addr_l =
		(uint32_t)(model_desc->aiv_csw_buff_addr[swap_buf_index] & UINT32_MAX);
	ffrt_swap_buf_index->sq_context_switch_ctrl1.reg.rtsq_context_switch_buf_base_addr_h =
		(uint16_t)(model_desc->aiv_csw_buff_addr[swap_buf_index] >> 32);
	npu_drv_warn("sq_base_addr 0x%lx 0x%lx, cfg0 0x%lx, cfg1 0x%lx, cfg3 0x%lx "
		"cq_base_addr 0x%lx 0x%lx, cfg0 0x%lx",
		ffrt_swap_buf_index->sq_base_addr0.value, ffrt_swap_buf_index->sq_base_addr1.value,
		ffrt_swap_buf_index->sq_cfg0.value, ffrt_swap_buf_index->sq_cfg1.value, ffrt_swap_buf_index->sq_cfg3.value,
		ffrt_swap_buf_index->cq_base_addr0.value, ffrt_swap_buf_index->cq_base_addr1.value,
		ffrt_swap_buf_index->cq_cfg0.value);

	return 0;
}

static int npu_ffrt_load_swap_buf(struct npu_proc_ctx *proc_ctx, npu_model_desc_t *model_desc, uint16_t uid)
{
	int ret = 0;
	ret = npu_ffrt_load_swap_buf_index(proc_ctx, model_desc, 0, uid);
	cond_return_error(ret != 0, -1, "load swap_buf_0 failed");

	if (model_desc->stream_cnt == 2) {
		ret = npu_ffrt_load_swap_buf_index(proc_ctx, model_desc, 1, uid);
		cond_return_error(ret != 0, -1, "load swap_buf_1 failed");
	}
	return 0;
}

static int npu_ffrt_load_model(struct npu_proc_ctx *proc_ctx,
	npu_model_desc_t *model_desc) {
	uint16_t uid = UINT16_MAX;
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	int ret = 0;
	u8 devid = proc_ctx->devid;

	cur_dev_ctx = get_dev_ctx_by_id(devid);
	cond_return_error(cur_dev_ctx == NULL, -1, "illegal cur_dev_ctx\n");

	ret = npu_svm_bind(cur_dev_ctx, current->tgid, model_desc->execute_pid, &uid);
	cond_return_error(ret != 0, -1, "svm bind error, ret=%d", ret);
	npu_drv_warn("uid %d", uid);

	ret = npu_ffrt_load_mdc_buf(model_desc, uid);
	cond_return_error(ret != 0, -1, "load mdc_buf failed");

	ret = npu_ffrt_load_swap_buf(proc_ctx, model_desc, uid);
	cond_return_error(ret != 0, -1, "load swap_buf failed");

	return ret;
}
#endif

static int load_hwts_sqe(struct npu_proc_ctx *proc_ctx, npu_model_desc_t *model_desc,
		npu_model_desc_info_t *model_desc_info, u32 *pt_core_type)
{
	u16 idx;
	int sqe_num;
	struct npu_stream_info *stream_info = NULL;
	struct npu_entity_info *hwts_sq_sub_info = NULL;
	struct npu_hwts_sq_info *hwts_sq_info = NULL;
	struct npu_sink_stream_info sink_stream_info;

	for (idx = 0; idx < model_desc->stream_cnt; idx++) {
		stream_info = npu_calc_stream_info(proc_ctx->devid, model_desc->stream_id[idx]);
		cond_return_error(stream_info == NULL, -1,
			"stream_id= %d stream_info is null\n", model_desc->stream_id[idx]);

		hwts_sq_info = npu_calc_hwts_sq_info(proc_ctx->devid, stream_info->sq_index);
		cond_return_error(hwts_sq_info == NULL, -1,
			"stream= %d, sq_index= %u, hwts_sq_info is null\n",
			stream_info->id, stream_info->sq_index);

		hwts_sq_sub_info =
			(struct npu_entity_info *)(uintptr_t)hwts_sq_info->hwts_sq_sub;
		cond_return_error(hwts_sq_sub_info == NULL, -1,
			"stream_id= %d sq_sub is null\n", stream_info->id);
		sink_stream_info.task_num = model_desc->stream_tasks[idx];
		sink_stream_info.stream_addr = model_desc->stream_addr[idx];
		sqe_num = npu_load_hwts_sqe(proc_ctx, &sink_stream_info, model_desc_info, hwts_sq_sub_info, pt_core_type);
		if (sqe_num <= 0) {
			npu_drv_err("formate hwts_sq failed, stream:%d\n",
				stream_info->id);
			return -1;
		}

		model_desc_info->stream_id[idx] = model_desc->stream_id[idx];
		model_desc_info->aiv_csw_buff_addr[idx] = model_desc->aiv_csw_buff_addr[idx];
		hwts_sq_info->head = 0;
		hwts_sq_info->tail = (u32)sqe_num;
	}

	return 0;
}


static int npu_load_model(struct npu_proc_ctx *proc_ctx,
	npu_model_desc_t *model_desc)
{
	int ret;
	u32 core_type = 0;
	npu_model_param_info_t model_param_info;
	npu_model_desc_info_t *model_desc_info = NULL;

	cond_return_error(proc_ctx->devid >= NPU_DEV_NUM, -1, "invalid device");

	model_desc_info = npu_calc_model_desc_info(proc_ctx->devid, model_desc->model_id);
	cond_return_error(model_desc_info == NULL, -1,
		"model_desc_info is NULL\n");
	ret = memset_s(model_desc_info, sizeof(npu_model_desc_info_t), 0,
		sizeof(npu_model_desc_info_t));
	cond_return_error(ret != 0, -1,
		"memset_s model_desc_info fail, ret = %d\n", ret);

	model_desc_info->model_id = model_desc->model_id;
	model_desc_info->stream_cnt = model_desc->stream_cnt;
	model_desc_info->compute_type = (uint16_t)(model_desc->compute_type);
	npu_save_model_info(proc_ctx, model_desc);
	npu_drv_debug("model_id:%d stream_cnt:%d compute_type:%d aiv_csw_buff_addr:0x%llx",
		model_desc_info->model_id, model_desc_info->stream_cnt,
		model_desc_info->compute_type, model_desc_info->aiv_csw_buff_addr);

	if (model_desc->stream_cnt > NPU_MODEL_STREAM_NUM ||
		model_desc->stream_cnt == 0) {
		npu_drv_err("user stream_cnt = %u invalid\n", model_desc->stream_cnt);
		return -1;
	}

	ret = load_hwts_sqe(proc_ctx, model_desc, model_desc_info, &core_type);
	cond_return_error(ret != 0, -1, "memset_s load_hwts_sqe fail, ret = %d\n", ret);

	model_param_info.model_id = model_desc->model_id;
	model_param_info.type = MODEL_PARAM_WORKMODE;
	if (core_type == 0x1)
		model_param_info.value = NPU_NONSEC_GE;
	else if (core_type == 0x2)
		model_param_info.value = NPU_NONSEC_EFF;
	else
		model_param_info.value = NPU_NONSEC;
	npu_proc_set_model_param(proc_ctx, &model_param_info);

#ifdef CONFIG_NPU_FFRT
	if (model_desc->scheduling_path == NPU_SCHED_FFTS) {
		ret = npu_ffrt_load_model(proc_ctx, model_desc);
		cond_return_error(ret != 0, -1, "load ffrt model fail, ret = %d\n", ret);
	}
#endif

	return 0;
}

#else
static int npu_verify_model_task_info(npu_model_desc_t *model_desc, u8 idx)
{
	if (model_desc->stream_tasks[idx] > NPU_MAX_SINK_TASK_ID ||
		model_desc->stream_tasks[idx] == 0) {
		npu_drv_err("user sink long stream_tasks[%u]= %u invalid\n",
			idx, model_desc->stream_tasks[idx]);
		return -1;
	}
	return 0;
}

static int npu_load_model(struct npu_proc_ctx *proc_ctx,
	npu_model_desc_t *model_desc)
{
	int ret;
	int stream_idx;
	void *stream_buf_addr = NULL;
	unsigned long stream_len;
	u8 devid = proc_ctx->devid;
	u16 first_task_id = MAX_UINT16_NUM;
	u32 core_type;
	struct npd_registry *npd = get_npd();

	npu_drv_debug("stream_cnt %u\n", model_desc->stream_cnt);
	cond_return_error(npd == NULL, -EINVAL, "npd ptr is NULL\n");
	cond_return_error(devid >= NPU_DEV_NUM, -1, "invalid device");
	npu_save_model_info(proc_ctx, model_desc);
	for (stream_idx = model_desc->stream_cnt - 1; stream_idx >= 0; stream_idx--) {
		stream_len = model_desc->stream_tasks[stream_idx] * NPU_RT_TASK_SIZE;
		stream_buf_addr = vmalloc(stream_len);
		cond_return_error(stream_buf_addr == NULL, -ENOMEM,
			"vmalloc stream_buf memory size= %d failed\n", stream_len);

		ret = copy_from_user_safe(stream_buf_addr,
			(void __user *)(uintptr_t)model_desc->stream_addr[stream_idx],
			stream_len);
		if (ret != 0) {
			vfree(stream_buf_addr);
			npu_drv_err("fail to copy stream_buf, ret= %d\n", ret);
			return -EINVAL;
		}

		// verify tasks
		ret = npd->npd_verify_sink_tasks(proc_ctx, stream_buf_addr, stream_len, &core_type);
		if (ret != 0) {
			vfree(stream_buf_addr);
			npu_drv_err("npu_verify_hwts_sqe fail, length= %d, ret= %d\n",
				stream_len, ret);
			return -1;
		}

		npu_drv_debug("stream_cnt %u, stream_idx = %d\n", model_desc->stream_cnt, stream_idx);
		ret = npd->npd_format_sw_sqe(model_desc, stream_buf_addr, &first_task_id, devid, stream_idx);
		if (ret != 0) {
			vfree(stream_buf_addr);
			npu_drv_err("npu_verify_hwts_sqe fail, length= %d, ret= %d\n",
				stream_len, ret);
			return -1;
		}

		vfree(stream_buf_addr);
		stream_buf_addr = NULL;
	}

	return 0;
}
#endif

static int npu_verify_model_desc(struct npu_proc_ctx *proc_ctx,
	npu_model_desc_t *model_desc)
{
	u8 idx;
	int ret;
	cond_return_error(model_desc == NULL, -1, "model_desc is invalid\n");

	cond_return_error(model_desc->priority > MAX_PRIORITY_LEVEL,
		-EINVAL, "invalid priority(%u)\n", model_desc->priority);

	ret = npu_proc_check_model_id(proc_ctx, model_desc->model_id);
	if (ret != 0) {
		npu_drv_err("user model_id= %u invalid\n", model_desc->model_id);
		return -1;
	}

	if (model_desc->stream_cnt > NPU_MODEL_STREAM_NUM ||
		model_desc->stream_cnt == 0) {
		npu_drv_err("user stream_cnt= %u invalid\n", model_desc->stream_cnt);
		return -1;
	}

	if (model_desc->compute_type >= NPU_COMPUTE_TYPE_MAX) {
		npu_drv_err("user compute_type= %u invalid\n", model_desc->compute_type);
		return -1;
	}

	for (idx = 0; idx < model_desc->stream_cnt; idx++) {
		if (model_desc->stream_id[idx] < NPU_MAX_NON_SINK_STREAM_ID ||
			npu_proc_check_stream_id(proc_ctx, model_desc->stream_id[idx]) != 0) {
			npu_drv_err("user sink stream_id[%u]= %u invalid\n",
				idx, model_desc->stream_id[idx]);
			return -1;
		}

		if (model_desc->stream_addr[idx] == NULL) {
			npu_drv_err("user sink stream_addr[%u] is NULL invalid\n", idx);
			return -1;
		}

		ret = npu_verify_model_task_info(model_desc, idx);
		cond_return_error(ret != 0, -1, "model_desc is invalid\n");
	}
	return 0;
}

int npu_ioctl_load_model(struct npu_proc_ctx *proc_ctx, unsigned long arg)
{
	int ret;
	npu_model_desc_t model_desc = {0};

	npu_drv_debug("enter\n");
	ret = copy_from_user_safe(&model_desc, (void __user *)(uintptr_t)arg,
		sizeof(npu_model_desc_t));
	cond_return_error(ret != 0, -EINVAL, "fail to copy model_desc, ret= %d\n",
		ret);

	npu_drv_debug("model_id= %u, stream_cnt= %u\n",
		model_desc.model_id, model_desc.stream_cnt);

	mutex_lock(&proc_ctx->model_mutex);
	ret = npu_verify_model_desc(proc_ctx, &model_desc);
	if (ret != 0) {
		npu_drv_err("verify model_desc fail, ret= %d\n", ret);
		mutex_unlock(&proc_ctx->model_mutex);
		return -1;
	}

	ret = npu_load_model(proc_ctx, &model_desc);
	if (ret != 0) {
		npu_drv_err("fail to load model, ret= %d\n", ret);
		mutex_unlock(&proc_ctx->model_mutex);
		return ret;
	}
	mutex_unlock(&proc_ctx->model_mutex);

	npu_drv_debug("end\n");
	return 0;
}


int npu_ioctl_set_model_priority(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	npu_model_prio_info_t model_prio_info = {0};
	npu_model_param_info_t model_param_info = {0};
	int ret = 0;

	ret = copy_from_user_safe(&model_prio_info, (void __user *)(uintptr_t)arg,
		sizeof(npu_model_prio_info_t));
	cond_return_error(ret != 0, -EINVAL,
		"fail to copy model_prio_info, ret= %d\n", ret);

	cond_return_error(model_prio_info.model_id >= NPU_MAX_MODEL_ID, -EINVAL,
		"model id is invalid\n");

	cond_return_error(model_prio_info.priority > MAX_PRIORITY_LEVEL, -EINVAL, "priority is invalid\n");

	model_param_info.type = MODEL_PARAM_PRIORITY;
	model_param_info.model_id = model_prio_info.model_id;
	model_param_info.value = model_prio_info.priority;
	ret = npu_proc_set_model_param(proc_ctx, &model_param_info);
	cond_return_error(ret != 0, ret,
		"npu_proc_set_model_priority fail, ret= %d\n", ret);
	return  ret;
}

int npu_ioctl_set_model_profiling(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	npu_model_profiling_info_t model_profiling_info = {0};
	npu_model_param_info_t model_param_info = {0};
	int ret;

	ret = copy_from_user_safe(&model_profiling_info, (void __user *)(uintptr_t)arg,
		sizeof(model_profiling_info));
	cond_return_error(ret != 0, -EINVAL,
		"fail to copy model_profiling_info, ret= %d\n", ret);

	cond_return_error(model_profiling_info.model_id >= NPU_MAX_MODEL_ID, -EINVAL,
		"model id is invalid\n");

	cond_return_error(model_profiling_info.profiling > 1,
		-EINVAL, "profiling is invalid\n");

	model_param_info.type = MODEL_PARAM_PROFILING;
	model_param_info.model_id = model_profiling_info.model_id;
	model_param_info.value = model_profiling_info.profiling;
	ret = npu_proc_set_model_param(proc_ctx, &model_param_info);
		cond_return_error(ret != 0, ret,
		"npu_proc_set_model_priority fail, ret= %d\n", ret);
	return  ret;
}

int npu_ioctl_send_request(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	npu_rt_task_t task = {0};
	int ret;

	ret = copy_from_user_safe(&task, (void __user *)(uintptr_t)arg,
		sizeof(npu_rt_task_t));
	cond_return_error(ret != 0, -EINVAL,
		"fail to copy task, ret= %d\n", ret);
#ifndef CONFIG_NPU_SWTS
	return npu_send_request(proc_ctx, &task);
#else
	return task_sched_put_request(proc_ctx,  &task);
#endif
}

int npu_ioctl_receive_response(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	struct npu_receive_response_info report_info = {0};
	int ret = 0;

	npu_drv_debug("npu ioctl receive report enter\n");
	ret = copy_from_user_safe(&report_info, (void __user *)(uintptr_t)arg,
		sizeof(struct npu_receive_response_info));
	if (ret != 0) {
		npu_drv_err("fail to copy comm_report, ret= %d\n", ret);
		return -EINVAL;
	}
#ifndef CONFIG_NPU_SWTS
	ret = npu_receive_response(proc_ctx, &report_info);
#else
	ret = task_sched_get_response(proc_ctx, &report_info);
#endif
	if (ret != 0)
		npu_drv_err("fail to receive_response ret= %d\n", ret);

	if (copy_to_user_safe((void __user *)(uintptr_t)arg, &report_info,
		sizeof(report_info))) {
		npu_drv_err("ioctl receive response copy_to_user_safe error\n");
		return -EINVAL;
	}

	npu_drv_debug("npu ioctl receive report exit\n");
	return ret;
}

/* new add for TS timeout function */
int npu_ioctl_get_ts_timeout(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	uint64_t exception_code = 0;
	int ret = 0;
	unused(proc_ctx);

	npu_drv_debug("enter\n");
	if (copy_from_user_safe(&exception_code, (void *)(uintptr_t)arg,
		sizeof(uint64_t))) {
		npu_drv_err("copy_from_user_safe error\n");
		return -EFAULT;
	}

	if (exception_code < (uint64_t)MODID_NPU_START || exception_code >
		(uint64_t)MODID_NPU_EXC_END) {
		npu_drv_err("expection code %llu out of npu range\n", exception_code);
		return -1;
	}

	/* receive TS exception */
	npu_rdr_exception_report((uint32_t)exception_code);

	return ret;
}

/* ION memory map */
int npu_ioctl_svm_bind_pid(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	int ret;
	pid_t pid;

	ret = copy_from_user_safe(&pid,
		(void __user *)(uintptr_t)arg, sizeof(pid));
	cond_return_error(ret != 0, -EINVAL, "fail to copy svm params, ret = %d\n",
		ret);

	ret = npu_insert_item_bypid(proc_ctx->devid, current->tgid, pid);
	cond_return_error(ret != 0, -EINVAL,
		"npu insert item bypid fail, ret = %d\n", ret);

	return ret;
}

/* ION memory map */
int npu_ioctl_svm_unbind_pid(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	int ret;
	pid_t pid;

	ret = copy_from_user_safe(&pid,
		(void __user *)(uintptr_t)arg, sizeof(pid));
	cond_return_error(ret != 0, -EINVAL, "fail to copy svm params, ret = %d\n",
		ret);

	ret = npu_release_item_bypid(proc_ctx->devid, current->tgid, pid);
	cond_return_error(ret != 0, -EINVAL, "npu release item bypid fail, ret = %d\n", ret);

	return ret;
}

int npu_ioctl_get_model_execute_info(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	int ret;
	u32 para = 0;

	if (copy_from_user_safe(&para, (void __user *)(uintptr_t)arg, sizeof(u32))) {
		npu_drv_err("copy from user safe error\n");
		return -EFAULT;
	}

	g_npu_toughness_info.model_cnt = para;
	// get model execute times and NPU reboot times.
	ret = npu_proc_get_execute_info(proc_ctx, &g_npu_toughness_info);
	cond_return_error(ret != 0, -EFAULT,
		"get model info failed, ret= %d\n", ret);

	if (copy_to_user_safe((void __user *)(uintptr_t)arg,
		&g_npu_toughness_info, sizeof(g_npu_toughness_info))) {
		npu_drv_err("copy to user safe error size = %u\n", g_npu_toughness_info.size);
		return -EFAULT;
	}

	return 0;
}

int npu_ioctl_set_model_execute_enable(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	int ret;
	struct npu_enable_info para = {0};

	if (copy_from_user_safe(&para, (void __user *)(uintptr_t)arg, sizeof(struct npu_enable_info))) {
		npu_drv_err("copy from user safe error\n");
		return -EFAULT;
	}

	ret = npu_proc_set_execute_enable(proc_ctx, &para);
	cond_return_error(ret != 0, -EFAULT,
		"set execute enable error, ret= %d\n", ret);

	return ret;
}

int npu_ioctl_bbox_record_err_task(struct npu_proc_ctx *proc_ctx, unsigned long arg)
{
	struct npu_task_dump_info para = {0};

	(void)proc_ctx;

	if (copy_from_user_safe(&para, (void __user *)(uintptr_t)arg, sizeof(struct npu_task_dump_info))) {
		npu_drv_err("copy from user safe error\n");
		return -EFAULT;
	}

	/* record error task info to bbox */
	npu_bbox_record_err_task(&para);

	return 0;
}

static int (*npu_ioctl_call[NPU_MAX_CMD])(struct npu_proc_ctx *proc_ctx, unsigned long arg) = {
	[_IOC_NR(NPU_ALLOC_STREAM_ID)] = npu_ioctl_alloc_stream,
	[_IOC_NR(NPU_FREE_STREAM_ID)] = npu_ioctl_free_stream,
	[_IOC_NR(NPU_ALLOC_EVENT_ID)] = npu_ioctl_alloc_event,
	[_IOC_NR(NPU_ALLOC_NOTIFY_ID)] = npu_ioctl_alloc_notify,
	[_IOC_NR(NPU_FREE_EVENT_ID)] = npu_ioctl_free_event,
	[_IOC_NR(NPU_FREE_NOTIFY_ID)] = npu_ioctl_free_notify,
	[_IOC_NR(NPU_GET_SVM_SSID)] = npu_ioctl_get_svm_ssid,
	[_IOC_NR(NPU_GET_CHIP_INFO)] = npu_ioctl_get_chip_info,
	[_IOC_NR(NPU_LOAD_MODEL_BUFF)] = npu_ioctl_load_model,
	[_IOC_NR(NPU_ALLOC_MODEL_ID)] = npu_ioctl_alloc_model,
	[_IOC_NR(NPU_FREE_MODEL_ID)] = npu_ioctl_free_model,
	[_IOC_NR(NPU_REQUEST_SEND)] = npu_ioctl_send_request,
	[_IOC_NR(NPU_RESPONSE_RECEIVE)] = npu_ioctl_receive_response,
	[_IOC_NR(NPU_ALLOC_TASK_ID)] = npu_ioctl_alloc_task,
	[_IOC_NR(NPU_FREE_TASK_ID)] = npu_ioctl_free_task,
	[_IOC_NR(NPU_GET_TS_TIMEOUT_ID)] = npu_ioctl_get_ts_timeout,
	[_IOC_NR(NPU_SVM_BIND_PID)] = npu_ioctl_svm_bind_pid,
	[_IOC_NR(NPU_SVM_UNBIND_PID)] = npu_ioctl_svm_unbind_pid,
	[_IOC_NR(NPU_ENTER_WORKMODE)] = npu_ioctl_enter_workmode,
	[_IOC_NR(NPU_EXIT_WORKMODE)] = npu_ioctl_exit_workmode,
	[_IOC_NR(NPU_SET_LIMIT)] = npu_ioctl_set_limit,
	[_IOC_NR(NPU_ENABLE_FEATURE)] = npu_ioctl_enable_feature,
	[_IOC_NR(NPU_DISABLE_FEATURE)] = npu_ioctl_disable_feature,
	[_IOC_NR(NPU_MODEL_PRIORITY_SET)] = npu_ioctl_set_model_priority,
	[_IOC_NR(NPU_MODEL_PROFILING_SET)] = npu_ioctl_set_model_profiling,
	[_IOC_NR(NPU_MODEL_GET_EXECUTE_INFO)] = npu_ioctl_get_model_execute_info,
	[_IOC_NR(NPU_MODEL_SET_EXECUTE_FLAG)] = npu_ioctl_set_model_execute_enable,
	[_IOC_NR(NPU_BBOX_RECORD_ERROR_TASK)] = npu_ioctl_bbox_record_err_task,
#ifdef CONFIG_NPU_SYSCACHE
	[_IOC_NR(NPU_ATTACH_SYSCACHE)] = npu_ioctl_attach_syscache,
	[_IOC_NR(NPU_DETACH_SYSCACHE)] = npu_ioctl_detach_syscache,
#endif
};

static int npu_proc_npu_ioctl_call(struct npu_proc_ctx *proc_ctx,
	unsigned int cmd, unsigned long arg)
{
	int ret;

	if (cmd < _IO(NPU_ID_MAGIC, 1) || cmd >= _IO(NPU_ID_MAGIC, NPU_MAX_CMD)) {
		npu_drv_err("parameter,arg = 0x%lx, cmd = %d\n", arg, cmd);
		return -EINVAL;
	}

	npu_drv_info("IOC_NR = %d  cmd = %d\n", _IOC_NR(cmd), cmd);

	if (npu_ioctl_call[_IOC_NR(cmd)] == NULL) {
		npu_drv_err("invalid cmd = %d\n", cmd);
		return -EINVAL;
	}

	// porcess ioctl
	ret = npu_ioctl_call[_IOC_NR(cmd)](proc_ctx, arg);
	if (ret != 0) {
		npu_drv_err("process failed,arg = %d\n", cmd);
		return -EINVAL;
	}

	return ret;
}

long npu_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	struct npu_platform_info *plat_info = npu_plat_get_info();
	struct npu_proc_ctx *proc_ctx = NULL;
	int ret;

	if (plat_info == NULL) {
		npu_drv_err("get plat info fail\n");
		return -EINVAL;
	}

	proc_ctx = (struct npu_proc_ctx *)filep->private_data;
	if (proc_ctx == NULL || arg == 0) {
		npu_drv_err("invalid parameter,arg = 0x%lx,cmd = %d\n",
			arg, cmd);
		return -EINVAL;
	}

	if ((plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 1) &&
		proc_ctx->pid != current->tgid) {
		npu_drv_err("can't make ioctl in child process!");
		return -EBUSY;
	}

	ret = npu_proc_npu_ioctl_call(proc_ctx, cmd, arg);
	if (ret != 0) {
		npu_drv_err("process failed,arg = %d\n", cmd);
		return -EINVAL;
	}

	return ret;
}
