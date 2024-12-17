/*
 * npu_arch_exception.c
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#include "npu_arch_exception.h"

#include <linux/kthread.h>
#include <linux/delay.h>

#include "npu_except_mgr.h"
#include "npu_log.h"
#include "npu_pm_internal.h"
#include "npu_ipc.h"
#include "npu_ipc_msg.h"
#include "npu_platform_exception.h"


#define NPU_EXCEPTION_TIMEOUT_IN_MS 100

static int32_t handle_exception_svc(void *arg)
{
	struct npu_dev_ctx *dev_ctx = (struct npu_dev_ctx *)arg;

	npu_drv_warn("in exception thread");

	do {
		msleep_deferrable(NPU_EXCEPTION_TIMEOUT_IN_MS);
		npu_raise_global_reg_exception(dev_ctx);
	} while (!kthread_should_stop());
	return 0;
}

int32_t npu_exception_svc_init(struct npu_dev_ctx *dev_ctx)
{
	npu_drv_debug("enter");
	cond_return_error(dev_ctx == NULL, -1, "dev_ctx is null\n");

	dev_ctx->except_manager.status = NPU_STATUS_NORMAL;

	npu_drv_warn("start npu exception thread");
	dev_ctx->except_manager.thread = kthread_run(handle_exception_svc, dev_ctx, "npu_excp");
	if (IS_ERR_OR_NULL(dev_ctx->except_manager.thread)) {
		npu_drv_err("start exception thead fail");
		return -1;
	}
	npu_drv_warn("start npu exception thread succ");

	return 0;
}

void npu_exception_svc_deinit(struct npu_dev_ctx *dev_ctx)
{
	cond_return_void(dev_ctx == NULL, "dev_ctx is null\n");

	npu_drv_warn("end npu exception thread");
	if (!IS_ERR_OR_NULL(dev_ctx->except_manager.thread)) {
		kthread_stop(dev_ctx->except_manager.thread);
		dev_ctx->except_manager.thread = NULL;
	}
	npu_drv_warn("end npu exception thread succ");

	return;
}

static int npu_handle_task_exception(struct npu_dev_ctx *dev_ctx, u16 channel_id)
{
	int ret;

	ret = npu_exception_aicore_reset(dev_ctx, channel_id);
	if (ret != 0) {
		npu_drv_warn("aicore reset failed, need reset npu to handle exception");
		return NPU_EXCEPTION_CONTINUE;
	}

	// reset succ; inform ts to disable sq, send cq, free channel
	ret = send_ipc_msg_to_ts(IPCDRV_TS_HWTS_TASK_EXCEPTION, IPCDRV_MSG_ASYNC,
		(u8 *)&channel_id, sizeof(channel_id), IPCDRV_SEND_WITH_ACK);
	if (ret != 0) {
		npu_drv_warn("send ipc to ts failed, need reset npu to handle exception");
		return NPU_EXCEPTION_CONTINUE;
	}

	return NPU_EXCEPTION_HANDLED;
}

int npd_handle_exception(struct npu_dev_ctx *dev_ctx, struct npu_exception_record* er)
{
	struct hwts_exception_report_info *report = er->u.ex_hwts_report;

	if ((er->ex_source != NPU_EXCEPTION_SRC_TS) || (report == NULL))
		return NPU_EXCEPTION_CONTINUE;

	/* For ISPNN task exception, do npu reset in v210 */
	if (report->service_type == NPU_SERVICE_TYPE_ISPNN)
		return NPU_EXCEPTION_CONTINUE;

	npu_drv_info("ts report message type:%u", report->exception_type);
	switch (report->exception_type) {
	case NPU_EXCEPTION_TYPE_HWTS_TASK_EXCEPTION:
	case NPU_EXCEPTION_TYPE_HWTS_TIMEOUT_EXCEPTION:
	case NPU_EXCEPTION_TYPE_HWTS_TRAP_EXCEPTION:
		return npu_handle_task_exception(dev_ctx, report->channel_id);
	case NPU_EXCEPTION_TYPE_HWTS_BUS_ERROR:
	case NPU_EXCEPTION_TYPE_HWTS_POOL_CONFLICT_ERROR:
		break;
	default:
		npu_drv_err("invalid ts report message type:%u", report->exception_type);
	}

	return NPU_EXCEPTION_CONTINUE;
}