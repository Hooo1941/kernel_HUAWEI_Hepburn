/*
 * npu_platform_exception.c
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

#include "npu_platform_exception.h"

#include <securec.h>

#include "npu_soc_defines.h"
#include "npu_shm.h"
#include "npu_sink_sqe_fmt.h"
#include "npu_hwts_plat.h"
#include "npu_aicore_plat.h"
#include "npu_pm_framework.h"
#include "npu_pool.h"
#include "bbox/npu_dfx_black_box.h"
#include "npu_proc_ctx.h"
#include "npu_common.h"
#include "npu_atf_subsys.h"

static u64 npu_exception_calc_sqe_addr(
	struct npu_dev_ctx *dev_ctx, u16 hwts_sq_id, u16 channel_id)
{
	u64 hwts_base = 0;
	u16 sq_head = 0;
	u64 sqe_addr = 0;
	int ret = 0;
	struct npu_hwts_sq_info *sq_info = NULL;
	struct npu_entity_info *hwts_sq_sub_info = NULL;

	cond_return_error(dev_ctx == NULL, 0, "dev_ctx is null\n");
	sq_info = npu_calc_hwts_sq_info(dev_ctx->devid, hwts_sq_id);
	cond_return_error(sq_info == NULL, 0,
		"hwts_sq_id= %u, hwts_sq_info is null\n", hwts_sq_id);

	hwts_sq_sub_info = (struct npu_entity_info *)(
		uintptr_t)sq_info->hwts_sq_sub;
	cond_return_error(hwts_sq_sub_info == NULL, 0,
		"hwts_sq_id= %u, hwts_sq_sub is null\n", hwts_sq_id);
	cond_return_error(hwts_sq_sub_info->vir_addr == 0, 0,
		"vir_addr is null\n");

	hwts_base = npu_hwts_get_base_addr();
	cond_return_error(hwts_base == 0, 0, "hwts_base is NULL\n");
	npu_pm_safe_call_with_return(dev_ctx, NPU_NON_TOP_COMMON,
		npu_hwts_query_sq_head(channel_id, &sq_head), ret);
	cond_return_error(ret != 0, 0, "can't get sq_head\n");

	sqe_addr = hwts_sq_sub_info->vir_addr + sq_head *
		sizeof(struct hwts_kernel_sqe);
	npu_drv_debug("sqe_addr = %016llx\n", sqe_addr);
	return sqe_addr;
}

static void npu_exception_pool_conflict_proc(
	struct npu_dev_ctx *dev_ctx, u16 hwts_sq_id, u16 channel_id)
{
	(void)dev_ctx;
	(void)hwts_sq_id;
	(void)channel_id;

	npu_drv_err("forbidden branch: no pool conflict scenario");
}

static void npu_exception_bus_error_proc(
	struct npu_dev_ctx *dev_ctx, u16 hwts_sq_id, u16 channel_id)
{
	(void)hwts_sq_id;
	(void)channel_id;

	npu_pm_safe_call(dev_ctx, NPU_NON_TOP_COMMON,
		(void)npu_hwts_query_bus_error());
}

static void npu_exception_sqe_error_proc(
	struct npu_dev_ctx *dev_ctx, u16 hwts_sq_id, u16 channel_id)
{
	u64 sqe_head_addr = npu_exception_calc_sqe_addr(dev_ctx, hwts_sq_id,
		channel_id);

	(void)npu_hwts_query_sqe_info(sqe_head_addr);
}

static void npu_exception_sw_status_error_proc(
	struct npu_dev_ctx *dev_ctx, u16 hwts_sq_id, u16 channel_id)
{
	u32 status = 0;

	npu_pm_safe_call(dev_ctx, NPU_NON_TOP_COMMON,
		(void)npu_hwts_query_sw_status(channel_id, &status));
	npu_exception_sqe_error_proc(dev_ctx, hwts_sq_id, channel_id);
}

static void npu_exception_bbox_dump_aicore(u8 aic_id,
	struct aicore_exception_info *aic_info,
	struct aicore_exception_dbg_info *aic_dbg_info)
{
	char log_buf[NPU_BUF_LEN_MAX + 1] = {0};
	unused(aic_dbg_info);

	(void)snprintf_s(log_buf, sizeof(log_buf), sizeof(log_buf) - 1,
		"aicore%u\nAIC_ERROR=0x%016llx, 0x%016llx, 0x%016llx\nifu start=0x%016llx\n",
		aic_id, aic_info->aic_error, aic_info->aic_error1, aic_info->aic_error2, (aic_info->ifu_start << 2));
	(void)npu_mntn_copy_reg_to_bbox(log_buf, (unsigned int)strlen(log_buf));
}

static void npu_exception_query_aicore_info(struct npu_dev_ctx *dev_ctx, uint16_t channel_id)
{
	u8 sqe_type = NPU_HWTS_SQE_TYPE_RESV;
	u8 aicore_idx;
	struct aicore_exception_info aicore_info = {0};
	struct aicore_exception_error_info aicore_error_info = {0};
	struct aicore_exception_dbg_info aicore_dbg_info = {0};

	npu_pm_safe_call(dev_ctx, NPU_NON_TOP_COMMON, (void)npu_hwts_query_sqe_type(channel_id, &sqe_type));
	if (sqe_type == NPU_HWTS_SQE_AICORE) {
		npu_drv_warn("npu hwts query aicore exception\n");
		aicore_idx = NPU_AICORE_0;
	} else if (sqe_type == NPU_HWTS_SQE_VECTOR_CORE) {
		npu_drv_warn("npu hwts query aiv exception\n");
		aicore_idx = NPU_AIVECTOR_0;
	} else {
		npu_drv_warn("hwts sqe type not supported");
		return;
	}

	npu_pm_safe_call(dev_ctx, NPU_NON_TOP_COMMON,
		(void)npu_aicore_query_exception_info(aicore_idx, &aicore_info));
	npu_pm_safe_call(dev_ctx, NPU_NON_TOP_COMMON,
		(void)npu_aicore_query_exception_error_info(
		aicore_idx, &aicore_error_info));
	npu_pm_safe_call(dev_ctx, NPU_NON_TOP_COMMON,
		(void)npu_aicore_query_exception_dbg_info(
		aicore_idx, &aicore_dbg_info));
	npu_exception_bbox_dump_aicore(aicore_idx, &aicore_info, &aicore_dbg_info);
}

static void npu_exception_aicore_exception_proc(
	struct npu_dev_ctx *dev_ctx, u16 channel_id)
{
	int ret = 0;
	u8 aic_own_bitmap = 0;
	u8 aic_exception_bitmap = 0;
	u8 i = 0;
	u8 aic_status = 0;
	u8 aiv_status = 0;

	npu_pm_safe_call_with_return(dev_ctx, NPU_NON_TOP_COMMON,
		npu_hwts_query_aic_own_bitmap(channel_id, &aic_own_bitmap),
		ret);
	cond_return_void(ret != 0, "cannot get aic_own_bitmap\n");

	npu_pm_safe_call_with_return(dev_ctx, NPU_NON_TOP_COMMON,
		npu_hwts_query_aic_exception_bitmap(
		channel_id, &aic_exception_bitmap), ret);
	cond_return_void(ret != 0, "cannot get aic_exception_bitmap\n");

	if (aic_exception_bitmap == 0) {
		npu_drv_warn("no aicore get exception\n");
		return;
	}

	npu_pm_safe_call(dev_ctx, NPU_NON_TOP_COMMON,
		(void)npu_hwts_query_aic_task_config());

	for (i = 0; i < NPU_HWTS_MAX_AICORE_POOL_NUM; i++)
		npu_pm_safe_call(dev_ctx, NPU_NON_TOP_COMMON,
			(void)npu_hwts_query_aicore_pool_status(i, &aic_status));

	for (i = 0; i < NPU_HWTS_MAX_AIV_POOL_NUM; i++)
		npu_pm_safe_call(dev_ctx, NPU_NON_TOP_COMMON,
			(void)npu_hwts_query_aiv_pool_status(i, &aiv_status));

	if (aic_exception_bitmap & (0x1 << 0))
		npu_exception_query_aicore_info(dev_ctx, channel_id);
}

static void npu_exception_aicore_trap_proc(
	struct npu_dev_ctx *dev_ctx, u16 channel_id)
{
	int ret = 0;
	u8 aic_own_bitmap = 0;
	u8 aic_trap_bitmap = 0;

	npu_pm_safe_call_with_return(dev_ctx, NPU_NON_TOP_COMMON,
		npu_hwts_query_aic_own_bitmap(channel_id, &aic_own_bitmap),
		ret);
	cond_return_void(ret != 0, "cannot get aic_own_bitmap\n");

	npu_pm_safe_call_with_return(dev_ctx, NPU_NON_TOP_COMMON,
		npu_hwts_query_aic_trap_bitmap(channel_id, &aic_trap_bitmap),
		ret);
	cond_return_void(ret != 0, "cannot get aic_trap_bitmap\n");

	cond_return_void(aic_trap_bitmap == 0, "no aicore get trap\n");
	if (aic_trap_bitmap & (0x1 << 0))
		npu_exception_query_aicore_info(dev_ctx, channel_id);
}

static void npu_exception_task_trap_proc(
	struct npu_dev_ctx *dev_ctx, u16 hwts_sq_id, u16 channel_id)
{
	npu_exception_aicore_trap_proc(dev_ctx, channel_id);
	npu_exception_sqe_error_proc(dev_ctx, hwts_sq_id, channel_id);
}

static void npu_exception_aicore_timeout_proc(
	struct npu_dev_ctx *dev_ctx, u16 channel_id)
{
	int ret = 0;
	u8 aic_own_bitmap = 0;

	npu_pm_safe_call_with_return(dev_ctx, NPU_NON_TOP_COMMON,
		npu_hwts_query_aic_own_bitmap(channel_id, &aic_own_bitmap),
		ret);
	cond_return_void(ret != 0, "cannot get aic_own_bitmap\n");

	if (aic_own_bitmap == 0) {
		npu_drv_warn("no aicore Occupied\n");
		return;
	}

	if (aic_own_bitmap & (0x1 << 0))
		npu_exception_query_aicore_info(dev_ctx, channel_id);
}

static void npu_exception_task_exception_proc(
	struct npu_dev_ctx *dev_ctx, u16 hwts_sq_id, u16 channel_id)
{
	int ret = 0;
	struct hwts_interrupt_info interrupt_info = {0};
	struct sq_exception_info sq_info = {0};
	(void)hwts_sq_id;

	npu_pm_safe_call_with_return(dev_ctx, NPU_NON_TOP_COMMON,
		npu_hwts_query_interrupt_info(&interrupt_info), ret);
	npu_pm_safe_call_with_return(dev_ctx, NPU_NON_TOP_COMMON,
		npu_hwts_query_sq_info(channel_id, &sq_info), ret);
	npu_exception_aicore_exception_proc(dev_ctx, channel_id);
}

static void npu_exception_task_timeout_proc(
	struct npu_dev_ctx *dev_ctx, u16 hwts_sq_id, u16 channel_id)
{
	int ret = 0;
	struct hwts_interrupt_info interrupt_info = {0};
	struct sq_exception_info sq_info = {0};
	(void)hwts_sq_id;

	npu_pm_safe_call_with_return(dev_ctx, NPU_NON_TOP_COMMON,
		npu_hwts_query_interrupt_info(&interrupt_info), ret);
	npu_pm_safe_call_with_return(dev_ctx, NPU_NON_TOP_COMMON,
		npu_hwts_query_sq_info(channel_id, &sq_info), ret);
	npu_exception_aicore_timeout_proc(dev_ctx, channel_id);
}

void npu_hwts_exception_dump(struct npu_dev_ctx *dev_ctx, u8 exception_type,
	u16 hwts_sq_id, u16 channel_id)
{
	// 1. print exception info
	static void (*exception_func[NPU_EXCEPTION_TYPE_MAX])
		(struct npu_dev_ctx *, u16, u16) = {
		npu_exception_task_exception_proc,
		npu_exception_task_timeout_proc,
		npu_exception_task_trap_proc,
		npu_exception_sqe_error_proc,
		npu_exception_sw_status_error_proc,
		npu_exception_bus_error_proc,
		NULL,
		npu_exception_pool_conflict_proc
	};

	(exception_func[exception_type])(dev_ctx, hwts_sq_id, channel_id);
}

void npu_timeout_exception_dump(struct npu_dev_ctx *dev_ctx)
{
	int ret = 0;
	u16 channel_id;
	uint64_t nonsec_bitmap = NPU_HWTS_RTSQ_LITE_UNSEC_BITMAP | NPU_HWTS_RTSQ_ISPNN_BITMAP;
	struct hwts_interrupt_info interrupt_info = {0};
	struct sq_exception_info sq_info = {0};

	npu_pm_safe_call_with_return(dev_ctx, NPU_NON_TOP_COMMON,
		npu_hwts_query_interrupt_info(&interrupt_info), ret);

	/* timeout without hwts exception, check&&dump all channel info */
	for (channel_id = 0; channel_id < NPU_HWTS_CHANNEL_NUM; channel_id++) {
		if (npu_bitmap_get(nonsec_bitmap, channel_id) == 0)
			continue;
		npu_drv_warn("channel_id = %u\n", channel_id);
		npu_pm_safe_call_with_return(dev_ctx, NPU_NON_TOP_COMMON,
			npu_hwts_query_sq_info(channel_id, &sq_info), ret);
		npu_exception_aicore_timeout_proc(dev_ctx, channel_id);
	}
}
void npu_ispnn_exception_dump(struct npu_dev_ctx *dev_ctx, u8 exception_type, u16 channel_id)
{
	int ret = 0;
	struct hwts_interrupt_info interrupt_info = {0};
	struct sq_exception_info sq_info = {0};

	npu_drv_warn("channel_id = %u, exception_type = %u\n", channel_id, exception_type);

	npu_pm_safe_call_with_return(dev_ctx, NPU_NON_TOP_COMMON,
		npu_hwts_query_interrupt_info(&interrupt_info), ret);
	npu_pm_safe_call_with_return(dev_ctx, NPU_NON_TOP_COMMON,
		npu_hwts_query_sq_info(channel_id, &sq_info), ret);
	npu_pm_safe_call_with_return(dev_ctx, NPU_NON_TOP_COMMON,
		npu_hwts_query_ispnn_info(), ret);
	npu_exception_aicore_exception_proc(dev_ctx, channel_id);
}

int npu_exception_aicore_reset(struct npu_dev_ctx *dev_ctx, u16 channel_id)
{
	int ret= -1;
	u8 sqe_type = NPU_HWTS_SQE_TYPE_RESV;
	u8 aicore_id;
	(void)dev_ctx;

	// 1.2.3. handle in TSFW
	// 4. query sqe_type
	npu_drv_warn("reset aicore start, %u", channel_id);
	npu_pm_safe_call_with_return(dev_ctx, NPU_NON_TOP_COMMON, npu_hwts_query_sqe_type(channel_id, &sqe_type), ret);
	cond_return_error(ret != 0, -1, "query sqe_type failed\n");
	cond_return_error(!(sqe_type == NPU_HWTS_SQE_AICORE || sqe_type == NPU_HWTS_SQE_VECTOR_CORE),
		-1, "hwts sqe type not supported\n");

	// 5.1.1
	if (sqe_type == NPU_HWTS_SQE_AICORE) {
		aicore_id = NPU_AICORE_0;
	} else {
		aicore_id = NPU_AIVECTOR_0;
	}

	// 5.1.4 or 5.2.4  reset AIC/AIV
	npu_pm_safe_call_with_return(dev_ctx, NPU_NON_TOP_COMMON,
		atfd_service_npu_smc(NPU_AICORE_RESET, aicore_id, 0, 0), ret);
	if(ret != 0)
		npu_drv_warn("reset aicore failed,  need power off npu\n");

	return ret;
}
