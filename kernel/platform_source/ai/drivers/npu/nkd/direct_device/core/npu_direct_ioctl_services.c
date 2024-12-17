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
#include "npu_direct_ioctl_services.h"

#include <linux/fs.h>

#include "npu_common.h"
#include "npu_shm.h"
#include "npu_rt_task.h"
#include "npu_ioctl_services.h"
#include "npu_direct_message.h"
#include "npu_message.h"

int npu_ioctl_direct_send_request(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	npu_rt_task_t task = {0};
	execute_info_t execute_info = {0};
	int ret = 0;
	u32 model_id = 0;

	ret = copy_from_user_safe(&execute_info, (void __user *)(uintptr_t)arg,
		sizeof(execute_info_t));
	cond_return_error(ret != 0, -EINVAL,
		"fail to copy arg, ret= %d\n", ret);

	model_id = execute_info.model_id;
	task.task_id = (u16)execute_info.task_id;

	cond_return_error(model_id >= NPU_MAX_MODEL_ID, -EINVAL,
		"model id is invalid\n");
	cond_return_error(execute_info.task_id < NPU_NON_SINK_START_TASK_ID,
		-EINVAL, "task id is invalid\n");
	ret = npu_pm_proc_hts_enter_wm(proc_ctx, model_id);
	cond_return_error(ret != 0, ret, "fail to hts enter wm, ret= %d\n", ret);

	ret = npu_compose_task_for_request(proc_ctx, model_id, &task);
	cond_return_error(ret != 0, ret,
		"fail to compose task, ret= %d\n", ret);

	npu_drv_debug("direct start model id  %d priority id %u stream id %u task id %u\n", model_id,
		task.u.model_execute_task.priority, task.stream_id, task.task_id);
#ifndef CONFIG_NPU_SWTS
	return npu_send_request(proc_ctx, &task);
#else
	return task_sched_put_request(proc_ctx,  &task);
#endif
}

static int npu_ioctl_stop_model_loop_execute(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	npu_rt_task_t task = {0};
	stop_info_t stop_info = {0};
	int ret = 0;
	u32 model_id = 0;

	ret = copy_from_user_safe(&stop_info, (void __user *)(uintptr_t)arg,
		sizeof(stop_info_t));
	cond_return_error(ret != 0, -EINVAL,
		"fail to copy arg, ret= %d\n", ret);

	model_id = stop_info.model_id;
	task.task_id = (u16)stop_info.task_id;

	cond_return_error(model_id >= NPU_MAX_MODEL_ID, -EINVAL,
		"model id is invalid\n");
	cond_return_error(stop_info.task_id < NPU_NON_SINK_START_TASK_ID,
		-EINVAL, "task id is invalid\n");

	ret = npu_compose_stop_task(proc_ctx, model_id, &task);
	cond_return_error(ret != 0, ret,
		"fail or no need to compose task, ret= %d\n", ret);

	ret = npu_pm_proc_hts_exit_wm(proc_ctx, model_id);
	cond_return_error(ret != 0, ret, "fail to hts exit wm, ret= %d\n", ret);
	npu_drv_debug("direct start model id  %d priority id %u stream id %u task id %u\n", model_id,
		task.u.model_execute_task.priority, task.stream_id, task.task_id);
#ifndef CONFIG_NPU_SWTS
	return npu_send_request(proc_ctx, &task);
#else
	return task_sched_put_request(proc_ctx,  &task);
#endif
}

long npu_direct_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	struct npu_platform_info *plat_info = npu_plat_get_info();
	struct npu_proc_ctx *proc_ctx = NULL;
	int ret;

	if (arg == 0) {
		npu_drv_err("invalid parameter,arg = 0x%lx,cmd = %d\n",
			arg, cmd);
		return -EINVAL;
	}

	if (plat_info == NULL) {
		npu_drv_err("get plat info fail\n");
		return -EINVAL;
	}

	proc_ctx = (struct npu_proc_ctx *)filep->private_data;
	if (proc_ctx == NULL) {
		npu_drv_err("get proc_ctx fail\n");
		return -EINVAL;
	}

	if ((plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 1) &&
		proc_ctx->pid != current->tgid) {
		npu_drv_err("can't make ioctl in child process!");
		return -EBUSY;
	}

	npu_drv_debug("IOC_NR = %d  cmd = %d\n", _IOC_NR(cmd), cmd);

	switch (cmd) {
	case NPU_DIRECT_MODEL_EXECUTE:
		ret = npu_ioctl_direct_send_request(proc_ctx, arg);
		break;
	case NPU_DIRECT_RESPONSE_RECEIVE:
		ret = npu_ioctl_receive_response(proc_ctx, arg);
		break;
	case NPU_DIRECT_MODEL_STOP_LOOP_EXECUTE:
		ret = npu_ioctl_stop_model_loop_execute(proc_ctx, arg);
		break;
	default :
		npu_drv_err("no function called cmd:%u", cmd);
		ret = -EINVAL;
	}

	return ret;
}