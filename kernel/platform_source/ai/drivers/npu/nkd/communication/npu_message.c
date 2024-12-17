/*
 * npu_message.c
 *
 * about npu sq/cq msg
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
#include "npu_message.h"

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
#include <linux/mm.h>
#include <linux/mman.h>
#include <securec.h>
#include <linux/timer.h>

#include "npu_task_message.h"
#include "npu_common.h"
#include "npu_proc_ctx.h"
#include "npu_calc_channel.h"
#include "npu_calc_cq.h"
#include "npu_stream.h"
#include "npu_shm.h"
#include "npu_log.h"
#include "npu_mailbox.h"
#include "npu_event.h"
#include "npu_model.h"
#include "npu_task.h"
#include "npu_pm_framework.h"
#include "npu_heart_beat.h"
#include "npu_comm_sqe_fmt.h"
#include "npu_comm_task_verify.h"
#include "npu_calc_sq.h"
#include "npu_pool.h"
#include "npu_iova.h"
#include "npu_doorbell.h"
#include "npu_platform.h"
#include "npu_svm.h"
#include "bbox/npu_dfx_black_box.h"
#include "npd_interface.h"
#include "npu_syscache.h"
#include "npu_counter.h"

static const u32 dummy = 0xFFFFFFFF;
static int npu_proc_get_calc_sq_info(struct npu_proc_ctx *proc_ctx,
	u32 sq_index, npu_ts_sq_info_t **sq_info, struct npu_sq_sub_info **sq_sub_info);

static inline struct npu_report *__npu_get_report(phys_addr_t base_addr,
	u32 slot_size, u32 slot_id)
{
	return (struct npu_report *)(uintptr_t)(base_addr + slot_size * slot_id);
}

static inline u32 npu_report_get_phase(struct npu_report *report)
{
	cond_return_error(report == NULL, dummy, "failed\n");
	return report->phase;
}

static inline u32 npu_report_get_sq_index(struct npu_report *report)
{
	cond_return_error(report == NULL, dummy, "failed\n");
	return report->sq_id;
}

static inline u32 npu_report_get_sq_head(struct npu_report *report)
{
	cond_return_error(report == NULL, dummy, "failed\n");
	return report->sq_head;
}

/* according to DevdrvUpdateCqTailAndSqHead() */
void npu_update_cq_tail_sq_head(
	struct npu_proc_ctx *proc_ctx, struct npu_cq_sub_info *cq_sub_info,
	struct npu_ts_cq_info *cq_info)
{
	npu_ts_sq_info_t *sq_info = NULL;
	struct npu_report *report = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	u8 dev_id = 0;
	u32 next_tail;
	u32 sq_head;

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	cond_return_void(cur_dev_ctx == NULL, "cur dev ctx is null\n");
	cond_return_void(cq_info->tail >= NPU_MAX_CQ_DEPTH,
		"cq_tail invalid, exceed max limit");
	report = __npu_get_report(cq_sub_info->virt_addr, cq_info->slot_size,
		cq_info->tail);

	npu_drv_debug("cq_tail=%u, cq_base_addr=%pK, valid_phase=%u\n",
		cq_info->tail, cq_sub_info->virt_addr,
		cq_info->phase);
	/* update cq_info->tail */
	while (npu_report_get_phase(report) == cq_info->phase) {
		npu_drv_debug("cq_tail=%u\n", cq_info->tail);
		next_tail = (cq_info->tail + 1) % NPU_MAX_CQ_DEPTH;

		/* cq_info->tail must not cover register->cq_head
		 * use one slot to keep tail not cover the head, which may have report
		 */
		if (next_tail == cq_info->head)
			break;

		sq_info = npu_calc_sq_info(proc_ctx->devid,
			npu_report_get_sq_index(report));
		cond_return_void(sq_info == NULL,
			"calc sq info error, sq_index = %d\n",
			npu_report_get_sq_index(report));

		/* handle to which one (sq slot) */
		sq_head = npu_report_get_sq_head(report);
		if (sq_head >= NPU_MAX_SQ_DEPTH) {
			npu_drv_err("wrong sq head from cq, sqHead = %u\n", sq_head);
			break;
		}

		sq_info->head = sq_head;
		npu_drv_debug("update sqinfo[%d]: head:%d, tail:%d, credit:%d\n",
			sq_info->index, sq_info->head, sq_info->tail, sq_info->credit);

		next_tail = cq_info->tail + 1;
		if (next_tail > (NPU_MAX_CQ_DEPTH - 1)) {
			cq_info->phase = (cq_info->phase == 0) ? 1 : 0;
			cq_info->tail = 0;
		} else {
			cq_info->tail++;
		}
		report = __npu_get_report(cq_sub_info->virt_addr,
			cq_info->slot_size, cq_info->tail);
	}
	npu_drv_debug("head=%u cq_tail=%u phase=%u\n",
		cq_info->head, cq_info->tail, cq_info->phase);
}

static u32 proc_exception_task(struct npu_dev_ctx *dev_ctx,struct npu_proc_ctx *proc_ctx,
		struct npu_receive_response_info *report_info, u32 *work_mode_cnt, u32 work_mode_size, u32 err_code)
{
	int ret = 0;
	u32 count = 0;
	struct npu_report *tmp = NULL;
	struct npu_report_payload *payload = NULL;
	struct npu_task_info *curr_task = NULL;
	struct npu_task_info *next_task = NULL;
	struct npu_report report = {0};
	u32 nonsec_work_mode = 0;

	tmp = (struct npu_report *)(uintptr_t)report_info->response_addr;
	list_for_each_entry_safe(curr_task, next_task, &proc_ctx->excpt_task_set, node) {
		if (unlikely(count == report_info->response_count)) {
			npu_drv_warn("response buffer is full, response count = %u\n",
				report_info->response_count);
			break;
		}

		payload = (struct npu_report_payload *)(&report.pay_load);
		report.stream_id = curr_task->stream_id;
		report.task_id = curr_task->task_id;

		payload->err_code = err_code;
		if ((payload->err_code == TS_EXCEPTION_FAIL) &&
			(dev_ctx->except_manager.task_info.is_task_exception != 0) &&
			(dev_ctx->except_manager.task_info.task_id == report.task_id)) {
			payload->persist_stream_id =
				dev_ctx->except_manager.task_info.persist_stream_id;
			payload->persist_task_id =
				dev_ctx->except_manager.task_info.persist_task_id;
		} else {
			payload->persist_stream_id = UINT16_MAX;
			payload->persist_task_id = UINT16_MAX;
		}

		npu_drv_debug("tg_id = %d, stream_id = %d, task_id = %d, persist_stream_id = %d, persist_task_id = %d\n",
			current->tgid, report.stream_id, report.task_id, payload->persist_stream_id, payload->persist_task_id);

		ret = copy_to_user_safe((void *)tmp, (const void *)(&report), sizeof(struct npu_report));
		if (ret != 0) {
			npu_drv_err("copy report to user fail ret %d\n", ret);
			break;
		}
		list_move_tail(&curr_task->node, &dev_ctx->pm.task_available_list);
		tmp++;
		count++;
		if (is_workmode_nsec(curr_task->work_mode)) {
			nonsec_work_mode = nonsec_workmode_map[curr_task->work_mode];
			if (nonsec_work_mode < work_mode_size)
				work_mode_cnt[nonsec_work_mode]++;
		} else {
			npu_drv_err("invalid workmode %u", curr_task->work_mode);
		}
	}
	return count;
}

static int npu_proc_process_exception_response(struct npu_dev_ctx *dev_ctx,
	struct npu_proc_ctx *proc_ctx, struct npu_receive_response_info *report_info)
{
	int ret = 0;
	int i;
	u32 count = 0;
	u32 work_mode_cnt[NPU_NOSEC_WORKMODE_NUM] = {0};
	u32 err_code = TS_EXCEPTION_FAIL;
	struct npd_registry *npd = NULL;

	/* 1. fake exception cqe by taskset */
	mutex_lock(&dev_ctx->pm.task_set_lock);
	if (list_empty(&proc_ctx->excpt_task_set)) {
		mutex_unlock(&dev_ctx->pm.task_set_lock);
		return -1;
	}

	if (dev_ctx->except_manager.task_info.is_task_exception != 0) {
		if (npu_proc_check_stream_id(proc_ctx, dev_ctx->except_manager.task_info.stream_id) != 0)
			err_code = TS_EXCEPTION_RETRY;
	}

	count = proc_exception_task(dev_ctx, proc_ctx, report_info, work_mode_cnt, NPU_NOSEC_WORKMODE_NUM, err_code);
	mutex_unlock(&dev_ctx->pm.task_set_lock);

	if (unlikely(count <= 0)) {
		npu_drv_err("no report, ret = %d, count = %d",
			ret, count);
		return 0;
	}
	for (i = 0; i < NPU_NOSEC_WORKMODE_NUM; i++) {
		if (work_mode_cnt[i] > 0)
			(void)npu_pm_proc_release_task_exit_wm(proc_ctx, dev_ctx,
				work_mode_cnt[i], nonsec_workmode[i]);
	}

	/* reset npu_exception_status */
	npd = get_npd();
	cond_return_error(npd == NULL, -1, "npd ptr is NULL\n");

	npd->npd_notify(dev_ctx, NPD_NOTICE_RESET_EXCEPTION_STATUS, (long)(uintptr_t)proc_ctx, (long)ret);
	report_info->response_count = count;
	report_info->wait_result = 1;
	return 0;
}

static int npu_get_report(struct npu_proc_ctx *proc_ctx,
	struct npu_ts_cq_info *cq_info, struct npu_cq_sub_info *cq_sub_info,
	struct npu_receive_response_info *report_info, struct npu_dev_ctx *dev_ctx)
{
	int ret;
	u32 count;
	struct npu_report *report = NULL;
	struct npu_report_payload *payload = NULL;

	u32 tail = cq_info->tail;
	if (tail > cq_info->head)
		count = tail - cq_info->head;
	else
		count = NPU_MAX_CQ_DEPTH - cq_info->head;

	if (unlikely(count > (u32)atomic_read(&cq_info->count_report)))
		npu_drv_err("devid: %d, too much report, cq id: %d, "
			"cq head: %d, cq tail: %d, count: %u, count_report: %d\n",
			proc_ctx->devid, cq_info->index, cq_info->head,
			tail, count, atomic_read(&cq_info->count_report));

	count = (report_info->response_count < count ?
		report_info->response_count : count);
	report = __npu_get_report(cq_sub_info->virt_addr,
		cq_info->slot_size, cq_info->head);
	if (report->task_id == dev_ctx->except_manager.task_info.task_id &&
		report->stream_id == dev_ctx->except_manager.task_info.stream_id) {
		payload = (struct npu_report_payload *)&report->pay_load;
		payload->persist_stream_id = dev_ctx->except_manager.task_info.persist_stream_id;
		payload->persist_task_id = dev_ctx->except_manager.task_info.persist_task_id;
	}

	ret = copy_to_user_safe(
		(void *)(uintptr_t)report_info->response_addr, report,
		count * sizeof(struct npu_report));
	cond_return_error(ret != 0, ret, "copy report to user fail ret %d\n", ret);

	atomic_sub((int)count, &cq_info->count_report);
	report_info->response_count = count;
	return 0;
}

static void npu_release_report(
	struct npu_dev_ctx *dev_ctx, struct npu_proc_ctx *proc_ctx,
	struct npu_ts_cq_info *cq_info, struct npu_cq_sub_info *cq_sub_info, u32 count)
{
	struct npu_report *p_report = NULL;
	unsigned long flags;
	u32 head;
	u32 i;
	u32 task_release_cnt[NPU_NOSEC_WORKMODE_NUM] = {0};
	u32 work_mode = 0;
#ifdef NPU_ARCH_V100
	u32 model_id = UINT16_MAX;
#endif

	/* remove task */
	for (i = 0; i < count; i++) {
		p_report = __npu_get_report(cq_sub_info->virt_addr,
			cq_info->slot_size,
			(cq_info->head + i) % NPU_MAX_CQ_DEPTH);
#ifdef NPU_ARCH_V100
		if (get_task_set_model_id(dev_ctx, proc_ctx, p_report->task_id, &model_id) == 0) {
			if (model_id != UINT16_MAX && p_report->pay_load == NPU_MODEL_STREAM_EXE_FAILED)
				npu_execute_err_cnt(proc_ctx->devid, (uint16_t)model_id);
		}
#endif
		npu_task_set_remove(dev_ctx, proc_ctx,
			p_report->stream_id, p_report->task_id, &work_mode);
		if (nonsec_workmode_map[work_mode] < NPU_NOSEC_WORKMODE_NUM)
			task_release_cnt[nonsec_workmode_map[work_mode]]++;
		npu_drv_info("release report, stream_id = %u task_id = %u\n",
			p_report->stream_id, p_report->task_id);
	}

	/* update head */
	spin_lock_irqsave(&dev_ctx->spinlock, flags);
	head = (cq_info->head + count) % NPU_MAX_CQ_DEPTH;
	cq_info->head = head;
	spin_unlock_irqrestore(&dev_ctx->spinlock, flags);

	mutex_lock(&dev_ctx->pm.npu_power_mutex);
	if (dev_ctx->pm.power_stage == NPU_PM_UP && (dev_ctx->pm.work_mode & NPU_NOSEC_BITMAP_MASK) != 0)
		(void)npu_write_doorbell_val(
			DOORBELL_RES_CAL_CQ, cq_info->index, head);
	else
		npu_drv_err("npu is powered off or in sec, power_stage[%u], work mode = %u\n",
			dev_ctx->pm.power_stage, dev_ctx->pm.work_mode);

	npu_drv_debug("receive report, cq_id= %u, cq_tail= %u, cq_head= %u",
		cq_info->index, cq_info->tail, cq_info->head);
	mutex_unlock(&dev_ctx->pm.npu_power_mutex);

	for (i = 0; i < NPU_NOSEC_WORKMODE_NUM; i++) {
		if (task_release_cnt[i] > 0)
			(void)npu_pm_proc_release_task_exit_wm(proc_ctx, dev_ctx, task_release_cnt[i], nonsec_workmode[i]);
	}
}

static int npu_proc_process_valid_response(struct npu_dev_ctx *dev_ctx,
	struct npu_proc_ctx *proc_ctx, struct npu_ts_cq_info *cq_info,
	struct npu_receive_response_info *report_info)
{
	int ret = 0;
	struct npu_cq_sub_info *cq_sub_info = NULL;

	/* 1. do not have valid cqe */
	if (cq_info->head == cq_info->tail)
		return -1;

	/* 2. have valid cqe */
	cq_sub_info = npu_get_cq_sub_addr(dev_ctx, cq_info->index);
	cond_return_error(cq_sub_info == NULL, -1,
		"invalid para cq info index %u\n", cq_info->index);

	ret = npu_get_report(proc_ctx, cq_info, cq_sub_info, report_info, dev_ctx);
	cond_return_error(ret != 0, ret, "fail to get report ret= %d\n", ret);

	npu_release_report(dev_ctx, proc_ctx,
		cq_info, cq_sub_info, report_info->response_count);

	npu_counter_add((int)report_info->response_count, NPU_COUNTER_RECV_RPT_FROM_TS);
	report_info->wait_result = 1;
	return 0;
}

static int npu_proc_process_receive_response(
	struct npu_dev_ctx *dev_ctx, struct npu_proc_ctx *proc_ctx,
	struct npu_ts_cq_info *cq_info, struct npu_receive_response_info *report_info)
{
	int ret = 0;
	down_read(&dev_ctx->except_manager.status_lock);

	ret = npu_proc_process_exception_response(dev_ctx, proc_ctx, report_info);
	if (ret != 0)
		ret = npu_proc_process_valid_response(dev_ctx, proc_ctx, cq_info, report_info);

	up_read(&dev_ctx->except_manager.status_lock);
	return ret;
}
static void npu_proc_cq_info_print(struct npu_dev_ctx *dev_ctx, struct npu_ts_cq_info *cq_info)
{
	struct npu_report *report = NULL;
	struct npu_cq_sub_info *cq_sub_info = NULL;
	npu_drv_warn("cq id:%u head:%u tail:%u phase:%u", cq_info->index, cq_info->head, cq_info->tail, cq_info->phase);
	cq_sub_info = npu_get_cq_sub_addr(dev_ctx, cq_info->index);
	if (cq_sub_info == NULL) {
		npu_drv_err("get cq %u sub info failed");
		return;
	}
	report = __npu_get_report(cq_sub_info->virt_addr,
		cq_info->slot_size, cq_info->tail);
	npu_drv_warn("task_id:%u stream_id:%u phase:%u", report->task_id, report->stream_id, report->phase);
}

static int npu_proc_receive_response_wait(struct npu_dev_ctx *dev_ctx,
	struct npu_proc_ctx *proc_ctx, struct npu_receive_response_info *report_info)
{
	struct npu_ts_cq_info *cq_info = NULL;

	ktime_t tmp;
	int remaining;
	uint64_t chk_time;

	int wait_time = report_info->timeout;

	u32 cq_id = 0xFF;
	int ret = npu_proc_get_cq_id(proc_ctx, &cq_id);
	cond_return_error((ret != 0), -1, "get cq fail\n");

	cq_info = npu_calc_cq_info(proc_ctx->devid, cq_id);
	cond_return_error(cq_info == NULL, -1, "get cq info failed\n");

	/* 1. wait event */
	if (unlikely(report_info->timeout == -1)) {
		wait_event(proc_ctx->report_wait,
			cq_info->head != cq_info->tail || list_empty(&proc_ctx->excpt_task_set) == 0);
	} else {
		tmp = ns_to_ktime(RCV_CHK_TIME * NSEC_PER_USEC);
		for (chk_time = RCV_CHK_TIME; chk_time < (((uint64_t)wait_time) * USEC_PER_MSEC);
			chk_time += RCV_CHK_TIME) {
			remaining = wait_event_hrtimeout(proc_ctx->report_wait,
				cq_info->head != cq_info->tail || list_empty(&proc_ctx->excpt_task_set) == 0, tmp);
			/* remaining == 0 : receive success */
			if (remaining == 0) {
				npu_counter_inc(NPU_COUNTER_WAKE_UP_RESPONSE);
				break;
			}
		}
		if (unlikely(chk_time >= (((uint64_t)wait_time) * USEC_PER_MSEC))) {
			/* check if other client in work, v100 won't return false */
			struct timespec64 receive_out_time;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
			ktime_get_coarse_real_ts64(&receive_out_time);
#else
			receive_out_time = current_kernel_time();
#endif
			if (receive_out_time.tv_sec >= dev_ctx->npu_live_stamp.tv_sec + (wait_time / MSEC_PER_SEC)) {
				npu_proc_cq_info_print(dev_ctx, cq_info);
				npu_exception_timeout(dev_ctx);
			} else {
				npu_drv_err("npu is in busy !\n");
				return -1;
			}
		}
	}

	/* 2. process report:1)exception; 2)valid cqe */
	npu_proc_process_receive_response(dev_ctx, proc_ctx, cq_info, report_info);

	return 0;
}

int npu_receive_response(struct npu_proc_ctx *proc_ctx,
	struct npu_receive_response_info *report_info)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	int ret = 0;

	cond_return_error(proc_ctx == NULL, -1, "invalid proc ctx\n");

	cur_dev_ctx = get_dev_ctx_by_id(proc_ctx->devid);
	cond_return_error(cur_dev_ctx == NULL, -1, "invalid dev ctx\n");

	if (npu_task_exist(proc_ctx) == 0) {
		npu_drv_err("no task exist !!!\n");
		report_info->response_count = 0;
		return 0;
	}

	npu_set_report_timeout(cur_dev_ctx, &(report_info->timeout));
	ret = npu_proc_receive_response_wait(cur_dev_ctx, proc_ctx, report_info);
	if (report_info->wait_result <= 0) {
		npu_drv_err("drv receive response wait timeout, need powerdown!!!\n");
		return ret;
	}
	cond_return_error((ret != 0), ret,
		"receive response wait ret %d result %d\n", ret,
		report_info->wait_result);

	return 0;
}

static int npu_complete_comm_sqe(struct npu_proc_ctx *proc_ctx, npu_rt_task_t *comm_task)
{
	uint64_t ttbr;
	uint16_t ssid;
	uint64_t tcr;
	int ret;

	cond_return_error(comm_task == NULL, -1, "comm_task is null\n");
	if (comm_task->type == NPU_TASK_MODEL_EXECUTE ||
		comm_task->type == NPU_TASK_MODEL_LOOP_EXECUTE) {
		// v200:new procedure
		ret = npu_get_ssid_bypid(proc_ctx->devid, current->tgid,
			comm_task->u.model_execute_task.execute_pid,
			&ssid, &ttbr, &tcr);
		cond_return_error(ret != 0, -1,
			"npu_get_ssid_bypid fail, ret= %d\n", ret);
#ifdef NPU_ARCH_V100
		ret = npu_get_ttbr(&ttbr, &tcr, comm_task->u.model_execute_task.execute_pid);
		cond_return_error(ret != 0, -1,
			"fail to get process ttbr info, ret= %d\n", ret);
#endif
		comm_task->u.model_execute_task.asid = (u16)(ttbr >> 48);
		comm_task->u.model_execute_task.asid_baddr = ttbr &
			(0x0000FFFFFFFFFFFFu);
		comm_task->u.model_execute_task.smmu_svm_ssid = ssid;
		comm_task->u.model_execute_task.tcr = tcr;
	}

	npu_drv_debug("tg_id = %d stream_id = %d, task_id = %d, task_type = %d\n",
		current->tgid, comm_task->stream_id, comm_task->task_id, comm_task->type);

	return 0;
}

static int npu_proc_get_calc_sq_info(struct npu_proc_ctx *proc_ctx,
	u32 sq_index, npu_ts_sq_info_t **sq_info, struct npu_sq_sub_info **sq_sub_info)
{
	struct npu_dev_ctx *cur_dev_ctx = get_dev_ctx_by_id(proc_ctx->devid);

	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx %u is null\n", proc_ctx->devid);
		return -1;
	}

	/* outer function should make sure paras are not null */
	*sq_info = npu_calc_sq_info(proc_ctx->devid, sq_index);
	if (*sq_info == NULL) {
		npu_drv_err("sq_index= %u, sq_info is null\n", sq_index);
		return -1;
	}

	*sq_sub_info = npu_get_sq_sub_addr(cur_dev_ctx, (*sq_info)->index);
	if (*sq_sub_info == NULL) {
		npu_drv_err("sq_index= %u sq_sub_info is null\n", sq_index);
		return -1;
	}

	return 0;
}

static int npu_proc_send_request_occupy(npu_ts_sq_info_t *sq_info, u32 cmd_count)
{
	u32 available_count;
	u32 sq_head = sq_info->head;
	u32 sq_tail = sq_info->tail;
	/* if credit is less than cmdCount, update credit */
	if (sq_info->credit < cmd_count)
		sq_info->credit = (sq_tail >= sq_head) ?
			(NPU_MAX_SQ_DEPTH - (sq_tail - sq_head + 1)) :
			(sq_head - sq_tail - 1);

	if (sq_info->credit < cmd_count)
		return -1;

	if ((sq_tail >= sq_head) && (NPU_MAX_SQ_DEPTH - sq_tail <= cmd_count)) {
		if (sq_head == 0)
			available_count = NPU_MAX_SQ_DEPTH - sq_tail - 1;
		else
			available_count = NPU_MAX_SQ_DEPTH - sq_tail;

		if (available_count < cmd_count)
			return -2;
	}
	return 0;
}

static int npu_proc_send_request_send(struct npu_dev_ctx *cur_dev_ctx,
	npu_ts_sq_info_t *sq_info, struct npu_sq_sub_info *sq_sub_info,
	u32 sq_index, u32 cq_index, npu_rt_task_t *comm_task)
{
	struct npu_ts_cq_info *cq_info = NULL;
	u32 new_sq_tail;
	unsigned long flags;
	int ret = 0;

	cq_info = npu_calc_cq_info(cur_dev_ctx->devid, cq_index);
	cond_return_error(cq_info == NULL, -1, "get cq info failed\n");

	spin_lock_irqsave(&cur_dev_ctx->spinlock, flags);
	/* check sqe buffer */
	ret = npu_proc_send_request_occupy(sq_info, 1);
	if (ret != 0) {
		spin_unlock_irqrestore(&cur_dev_ctx->spinlock, flags);
		npu_drv_err("proc send request occypy failed, "
			"ret= %d, sq_id= %u, sq_head= %u sq_tail= %u\n",
			ret, sq_info->index, sq_info->head, sq_info->tail);
		return ret;
	}
	/* save sqe */
	ret = npu_format_ts_sqe((void *)(uintptr_t)sq_sub_info->virt_addr,
		comm_task, sq_info->tail);
	if (ret != 0) {
		spin_unlock_irqrestore(&cur_dev_ctx->spinlock, flags);
		npu_drv_err("format sqe failed, ret= %d, tail= %u\n",
			ret, sq_info->tail);
		return ret;
	}
	/* update sq tail... */
	new_sq_tail = (sq_info->tail + 1) % NPU_MAX_SQ_DEPTH;
	sq_info->tail = new_sq_tail;
	sq_info->credit -= 1;
	sq_info->send_count += 1;
	/* use atomic add */
	atomic_add(1, &cq_info->count_report);
	spin_unlock_irqrestore(&cur_dev_ctx->spinlock, flags);

	npu_drv_debug("ts_sqe_addr= %pK, comm_task= %pK, type=%d\n",
		(void *)(uintptr_t)sq_sub_info->virt_addr, comm_task,
		comm_task->type);
	npu_drv_debug("update sqinfo, id= %u, head= %u, tail= %u, credit= %u\n",
		sq_info->index, sq_info->head,
		sq_info->tail, sq_info->credit);

	/* update doorbell */
	mutex_lock(&cur_dev_ctx->pm.npu_power_mutex);
	if (cur_dev_ctx->pm.power_stage == NPU_PM_UP)
		(void)npu_write_doorbell_val(
			DOORBELL_RES_CAL_SQ, sq_index, sq_info->tail);
	else
		npu_drv_err("npu is powered off, power_stage[%u]\n",
			cur_dev_ctx->pm.power_stage);
	mutex_unlock(&cur_dev_ctx->pm.npu_power_mutex);
	return 0;
}

static int npu_verify_execute_enable(struct npu_proc_ctx *proc_ctx, npu_rt_task_t *comm_task)
{
	uint16_t model_id = comm_task->u.model_execute_task.model_id;
	struct npu_id_allocator *id_allocator = NULL;
	struct npu_id_entity *id_entity = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;

	if (model_id >= NPU_MAX_MODEL_ID) {
		npu_drv_err("model_id %u error", model_id);
		return -EINVAL;
	}

	cur_dev_ctx = get_dev_ctx_by_id(proc_ctx->devid);
	cond_return_error(cur_dev_ctx == NULL, -EINVAL, "cur_dev_ctx %u is null\n",
		proc_ctx->devid);

	id_allocator = npu_get_model_entity(proc_ctx->devid);
	cond_return_error(id_allocator == NULL, -EINVAL,
		"id allocator handel is null\n");

	mutex_lock(&id_allocator->lock);
	id_entity = (struct npu_id_entity*)(
		(uintptr_t)id_allocator->id_entity_base_addr +
		id_allocator->entity_size * (model_id - id_allocator->start_id));

	if (id_entity->data[5] != NPU_MODEL_EXECUTE_ENABLE) {
		npu_drv_err("model_id = %u forbidden execute\n", model_id);
		mutex_unlock(&id_allocator->lock);
		return -EPERM;
	}

	++(id_entity->data[3]); // 3 is model execute times

	mutex_unlock(&id_allocator->lock);
	return 0;
}

static int npu_send_request_(struct npu_dev_ctx *cur_dev_ctx,
		struct npu_proc_ctx *proc_ctx, npu_rt_task_t *comm_task)
{
	npu_stream_info_t *stream_info = NULL;
	npu_ts_sq_info_t *sq_info = NULL;
	struct npu_sq_sub_info *sq_sub_info = NULL;
	struct npd_registry *npd = NULL;
	u32 work_mode;
	int ret = 0;

	npd = get_npd();
	cond_return_error(npd == NULL, -1, "npd ptr is NULL\n");
	work_mode = (u32)npd->npd_nosec_get_workmode(proc_ctx, comm_task);
	npu_drv_info("work mode = %u", work_mode);

	down_read(&cur_dev_ctx->except_manager.status_lock);

	cond_goto_error(
		cur_dev_ctx->except_manager.status != NPU_STATUS_NORMAL,
		NPU_EXCEPTION, ret, -1, "npu is in an abnormal state\n");

	ret = npu_pm_proc_send_task_enter_wm(proc_ctx, cur_dev_ctx, work_mode);
	cond_goto_error(ret != 0, NPU_EXCEPTION, ret, ret,
			"first task : power up fail, ret= %d\n", ret);

#ifdef CONFIG_NPU_SYSCACHE
	if (comm_task->type == NPU_TASK_MODEL_EXECUTE || comm_task->type == NPU_TASK_MODEL_LOOP_EXECUTE)
#ifdef CONFIG_MM_LB_V550
		npu_sc_enable(cur_dev_ctx);
#else
		npu_sc_enable_and_high(cur_dev_ctx);
#endif
#endif
	/* complete ts_sqe */
	ret = npu_complete_comm_sqe(proc_ctx, comm_task);
	cond_goto_error(ret != 0, PRE_SEND_FAIL, ret, ret,
		"complete comm sqe fail, ret= %d\n", ret);

	npu_drv_info("send task, stream_id = %u, task_id:%u, type:%u\n",
		comm_task->stream_id, comm_task->task_id, comm_task->type);

	stream_info = npu_calc_stream_info(proc_ctx->devid,
		comm_task->stream_id);
	cond_goto_error(stream_info == NULL, PRE_SEND_FAIL, ret, -1,
		"calc stream info fail, ret= %d\n", -1);
	ret = npu_proc_get_calc_sq_info(proc_ctx, stream_info->sq_index,
		&sq_info, &sq_sub_info);
	cond_goto_error(((ret != 0 || sq_info == NULL || sq_sub_info == NULL)),
		PRE_SEND_FAIL, ret, ret,
		"proc send request occypy failed, "
		"sq_info or sq_sub_info is NULL\n");

	npu_drv_debug("sq_sub_info, index= %u, "
		"ref_by_streams= %u, phy_addr= 0x%llx, virt_addr= 0x%llx\n",
		sq_sub_info->index, sq_sub_info->ref_by_streams,
		sq_sub_info->phy_addr, sq_sub_info->virt_addr);

	npu_task_set_insert(cur_dev_ctx, proc_ctx, comm_task, work_mode);
	cond_goto_error(ret != 0, PRE_SEND_FAIL, ret, ret,
		"insert task set failed, ret= %d, stream id = %u, task id = %u\n",
		ret, stream_info->id, comm_task->task_id);

	/* save ts_sqe and send */
	ret = npu_proc_send_request_send(cur_dev_ctx, sq_info, sq_sub_info,
		stream_info->sq_index, stream_info->cq_index, comm_task);
	cond_goto_error(ret != 0, SEND_FAIL, ret, ret,
		"proc send request occypy failed, sq_id= %u, sq_head= %u sq_tail= %u\n",
		sq_info->index, sq_info->head, sq_info->tail);

	npu_drv_debug("communication stream= %d, sq_indx= %u, sq_tail= %u, send_count= %lld\n",
		stream_info->id,
		stream_info->sq_index, sq_info->tail, sq_info->send_count);
	npu_counter_inc(NPU_COUNTER_SEND_TASK_TO_TS);

	up_read(&cur_dev_ctx->except_manager.status_lock);
	return 0;

SEND_FAIL:
	npu_task_set_remove(cur_dev_ctx, proc_ctx, stream_info->id,
		comm_task->task_id, &work_mode);
PRE_SEND_FAIL:
	(void)npu_pm_proc_release_task_exit_wm(proc_ctx, cur_dev_ctx, 1, work_mode);
NPU_EXCEPTION:
	up_read(&cur_dev_ctx->except_manager.status_lock);
	return ret;
}

int npu_send_request(struct npu_proc_ctx *proc_ctx,
	npu_rt_task_t *comm_task)
{
	struct npu_dev_ctx *cur_dev_ctx = get_dev_ctx_by_id(proc_ctx->devid);
	int ret = 0;

	cond_return_error(cur_dev_ctx == NULL, -1, "cur_dev_ctx %d is null\n",
		proc_ctx->devid);

	if (comm_task->type == NPU_TASK_MODEL_EXECUTE) {
		ret = npu_verify_execute_enable(proc_ctx, comm_task);
		cond_return_error(ret != 0, ret, "model execute disable, ret= %d\n", ret);
		npu_counter_inc(NPU_COUNTER_SEND_MODEL_EXE_TO_TS);
	}

	/* verify ts_sqe */
	ret = npu_verify_ts_sqe(proc_ctx, comm_task);
	cond_return_error(ret != 0, ret,
		"verify ts sqe fail, ret= %d\n", ret);

	ret = npu_send_request_(cur_dev_ctx, proc_ctx, comm_task);
#ifndef NPU_ARCH_V100
	// it needs to wait when busy
	while (ret == -EBUSY) {
		wait_event(cur_dev_ctx->pm.nosec_wait, atomic_read(&cur_dev_ctx->pm.security_state) == 0 &&
			npu_bitmap_get(cur_dev_ctx->pm.work_mode, NPU_SEC) == 0);
		npu_drv_warn("wakeuped! now it can send task");
		ret = npu_send_request_(cur_dev_ctx, proc_ctx, comm_task);
	}
#endif
	return ret;
}
