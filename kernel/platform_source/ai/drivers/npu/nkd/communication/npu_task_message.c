/*
 * npu_task_message.c
 *
 * about npu task message
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
 *
 */
#include "npu_task_message.h"

#include <linux/vmalloc.h>

#include "npu_pm_framework.h"
#include "bbox/npu_dfx_black_box.h"
#include "npu_common_resource.h"
#include "npd_interface.h"
#include "npu_counter.h"

void npu_set_report_timeout(struct npu_dev_ctx *dev_ctx, int *timeout_out)
{
	cond_return_void(dev_ctx == NULL, "dev_ctx is null\n");
	cond_return_void(timeout_out == NULL, "timeout_out is null\n");
#ifdef NPU_ARCH_V100
	unused(dev_ctx);
	unused(timeout_out);
#else
	*timeout_out = (int)dev_ctx->npu_task_time_out;
#endif
	return;
}

void npu_exception_timeout(struct npu_dev_ctx *dev_ctx)
{
#ifdef NPU_ARCH_V100
	unused(dev_ctx);
#else
	npu_raise_timeout_exception(dev_ctx);
#endif
}

int npu_task_set_init(u8 dev_ctx_id)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct npu_task_info *task_buffer = NULL;
	int i = 0;
	int count = 0;

	cond_return_error(dev_ctx_id >= NPU_DEV_NUM, -EINVAL,
		"invalid dev_ctx_id\n");
	cur_dev_ctx = get_dev_ctx_by_id(dev_ctx_id);
	cond_return_error(cur_dev_ctx == NULL, -ENODATA,
		"cur_dev_ctx %u is null\n", dev_ctx_id);

	count = NPU_MAX_SQ_NUM * NPU_MAX_SQ_DEPTH + TS_TASK_BUFF_SIZE;
	task_buffer = vmalloc(sizeof(struct npu_task_info) * (unsigned int)count);
	if (task_buffer == NULL) {
		npu_drv_err("alloc memory fail\n");
		return -1;
	}

	INIT_LIST_HEAD(&cur_dev_ctx->pm.task_available_list);
	for (i = 0; i < count; i++)
		list_add(&task_buffer[i].node, &cur_dev_ctx->pm.task_available_list);
	cur_dev_ctx->pm.task_buffer = task_buffer;
	atomic_set(&cur_dev_ctx->pm.task_ref_cnt, 0);

	return 0;
}

int npu_task_set_destroy(u8 dev_ctx_id)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;

	npu_drv_warn("task_set_lock after destroy");
	cond_return_error(dev_ctx_id >= NPU_DEV_NUM, -EINVAL,
		"invalid dev_ctx_id\n");
	cur_dev_ctx = get_dev_ctx_by_id(dev_ctx_id);
	cond_return_error(cur_dev_ctx == NULL, -ENODATA,
		"cur_dev_ctx %u is null\n", dev_ctx_id);

	INIT_LIST_HEAD(&cur_dev_ctx->pm.task_available_list);
	vfree(cur_dev_ctx->pm.task_buffer);
	cur_dev_ctx->pm.task_buffer = NULL;

	return 0;
}

// the func is not in lock
int npu_task_set_insert(struct npu_dev_ctx *dev_ctx,
	struct npu_proc_ctx *proc_ctx, npu_rt_task_t *comm_task, u32 work_mode)
{
	struct npu_task_info *task = NULL;

	cond_return_error(comm_task->stream_id >= NPU_MAX_NON_SINK_STREAM_ID, -EINVAL,
		"invalid stream id, stream_id = %u\n", comm_task->stream_id);

	mutex_lock(&dev_ctx->pm.task_set_lock);
	if (list_empty(&dev_ctx->pm.task_available_list)) {
		npu_drv_err("task_available_list is empty\n");
		mutex_unlock(&dev_ctx->pm.task_set_lock);
		return -1;
	}
	task = list_first_entry(&dev_ctx->pm.task_available_list,
		struct npu_task_info, node);
	list_move_tail(&task->node, &proc_ctx->task_set);
	task->task_id = (u16)comm_task->task_id;
	task->stream_id = (u16)comm_task->stream_id;
	if (comm_task->type == NPU_TASK_MODEL_EXECUTE)
		task->model_id = (u16)comm_task->u.model_execute_task.model_id;
	else
		task->model_id = UINT16_MAX;
	task->work_mode = work_mode;

	mutex_unlock(&dev_ctx->pm.task_set_lock);
	return 0;
}

// the func is not in lock
int npu_task_set_remove(struct npu_dev_ctx *dev_ctx, struct npu_proc_ctx *proc_ctx,
	u32 stream_id, u32 task_id, u32 *work_mode)
{
	struct npu_task_info *task = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	cond_return_error(stream_id >= NPU_MAX_NON_SINK_STREAM_ID, -EINVAL,
		"invalid stream id, stream_id = %u\n", stream_id);

	mutex_lock(&dev_ctx->pm.task_set_lock);
	list_for_each_safe(pos, n, &proc_ctx->task_set) {
		task = list_entry(pos, struct npu_task_info, node);
		if (task->task_id == task_id) {
			if (task->model_id != UINT16_MAX)
				npu_counter_inc(NPU_COUNTER_RECV_MODEL_RPT_FROM_TS);
			*work_mode = task->work_mode;
			list_move_tail(&task->node, &dev_ctx->pm.task_available_list);
			mutex_unlock(&dev_ctx->pm.task_set_lock);
			return 0;
		}
	}
	mutex_unlock(&dev_ctx->pm.task_set_lock);
	return -1;
}

// the func can not be in npu_power_mutex
int npu_task_set_clear(struct npu_dev_ctx *dev_ctx,
	struct npu_proc_ctx *proc_ctx)
{
	mutex_lock(&dev_ctx->pm.task_set_lock);
	list_splice_tail_init(&proc_ctx->excpt_task_set, &dev_ctx->pm.task_available_list);
	list_splice_tail_init(&proc_ctx->task_set, &dev_ctx->pm.task_available_list);
	atomic_set(&dev_ctx->pm.task_ref_cnt, 0);
	mutex_unlock(&dev_ctx->pm.task_set_lock);
	return 0;
}

int get_task_set_model_id(struct npu_dev_ctx *dev_ctx,
	struct npu_proc_ctx *proc_ctx, u32 task_id, u32 *model_id)
{
	struct npu_task_info *task = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	mutex_lock(&dev_ctx->pm.task_set_lock);
	list_for_each_safe(pos, n, &proc_ctx->task_set) {
		task = list_entry(pos, struct npu_task_info, node);
		if (task->task_id == task_id) {
			*model_id = task->model_id;
			mutex_unlock(&dev_ctx->pm.task_set_lock);
			return 0;
		}
	}
	mutex_unlock(&dev_ctx->pm.task_set_lock);
	return -1;
}

int npu_task_exist(struct npu_proc_ctx *proc_ctx)
{
	int ret = 1;
#ifdef NPU_ARCH_V100
	unused(proc_ctx);
#else
	u32 i;
	mutex_lock(&proc_ctx->wm_lock);
	for (i = 0; i < NPU_NOSEC_WORKMODE_NUM; i++) {
		if (proc_ctx->task_cnt[nonsec_workmode[i]] != 0) {
			break;
		}
	}
	if (i == NPU_NOSEC_WORKMODE_NUM)
		ret = 0;
	mutex_unlock(&proc_ctx->wm_lock);
#endif
	return ret;
}