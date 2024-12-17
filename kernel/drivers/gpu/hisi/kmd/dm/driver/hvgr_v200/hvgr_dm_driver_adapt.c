/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include <linux/bsearch.h>
#include <linux/rcupdate.h>
#include <securec.h>

#include "hvgr_dm_driver_adapt_defs.h"

#include "hvgr_defs.h"
#include "hvgr_regmap.h"
#include "hvgr_dm_api.h"
#include "hvgr_pm_api.h"
#include "hvgr_sch_api.h"
#include "hvgr_mem_api.h"
#include "hvgr_log_api.h"
#include "hvgr_dm_driver_base.h"
#include "hvgr_datan_api.h"

/*
 * The GPU and JOB fault names are defined as follows:
 */
static struct hvgr_dm_fault_msg gpu_fault_name_tbl[] = {
	{0x00, "NOT_STARTED/IDLE/OK"},
	{0x01, "DONE"},
	{0x02, "INTERRUPTED"},
	{0x03, "STOPPED"},
	{0x04, "TERMINATED"},
	{0x05, "KABOOM"},
	{0x06, "IEUREKA"},
	{0x08, "ACTIVE"},
	{0x39, "FAULT_EXTRA"},
	{0x40, "FAULT_PWR"},
	{0x48, "FAULT_OOB"},
	{0x49, "FAULT_SKIP_CSHARP"},
	{0x4a, "FAULT_INDEX_CSHARP"},
	{0x4b, "FAULT_PKC_SIZE_ZERO"},
	{0x4c, "FAULT_TCM_ADDR_ERR"},
	{0x4d, "FAULT_TCM_ADDR_REQ_ERR"},
	{0x4e, "FAULT_DESC_ERR"},
	{0x4f, "FAULT_OUT_SIZE_ZERO"},
	{0x50, "FAULT_SW_CTX"},
	{0x51, "FAULT_HW_CTX"},
	{0x52, "FAULT_ENTRY"},
	{0x54, "FAULT_INLINE"},
	{0x55, "FAULT_INST"},
	{0x56, "FAULT_MCU_CALL"},
	{0x58, "JOB_CONFIG_FAULT"},
	{0x59, "JOB_AFFINITY_FAULT"},
	{0x5a, "DATA_INVALID_FAULT"},
	{0x60, "OUT_OF_MEMORY"},
	{0x63, "FBD"},
	{0x69, "PGL_HEADER"},
	{0x6a, "PGL_CMD"},
	{0x6b, "TIMEOUT"},
	{0x72, "PIPE_SYNC"},
	{0x74, "TS_SDC_CFG_WHEN_FULL"},
	{0x76, "TS_PRIMTYPE_CONSISENT"},
	{0x77, "TS_FS_SCISSOR_CFG"},
	{0x7b, "FAULT_BARRIER"},
	{0x7c, "LSC_FAULT"},
	{0x7d, "HEBC_FAULT"},
	{0x80, "DELAYED_BUS_FAULT"},
	{0x90, "UMMAP"},
	{0x98, "MMU_UNSUP"},
	{0xa0, "HTU"},
	{0xa1, "TU_CNT"},
	{0xb0, "GPU_UNSUPPORTED"},
	{0xb1, "SYS_UNSUPPORTED"},
	{0xc0, "TRANSLATION_FAULT_0"},
	{0xc1, "TRANSLATION_FAULT_1"},
	{0xc2, "TRANSLATION_FAULT_2"},
	{0xc3, "TRANSLATION_FAULT_3"},
	{0xc8, "PERMISSION_FAULT_0"},
	{0xd0, "TRANSTAB_BUS_FAULT_0"},
	{0xd1, "TRANSTAB_BUS_FAULT_1"},
	{0xd2, "TRANSTAB_BUS_FAULT_2"},
	{0xd3, "TRANSTAB_BUS_FAULT_3"},
	{0xe4, "ADDR_SZ_FAULT_4"},
};

static const uint32_t fault_no_need_reset_gpu[] = {
	0x0, /* NOT_STARTED/IDLE/OK */
	0x1, /* DONE */
	0x2, /* INTERRUPTED */
	0x3, /* STOPPED */
	0x4, /* TERMINATED */
	0x5, /* KABOOM */
	0x6, /* IEUREKA */
	0x8, /* ACTIVE */
	0x39, /* FAULT_EXTRA */
	0x7c, /* LSC_FAULT */
	0x7d, /* HEBC_FAULT */
};

static const uint32_t fault_reset_gpu_normal[] = {
	0x60, /* OUT_OF_MEMORY */
};

bool hvgr_gpu_reset_needed_by_fault(uint32_t fault_code)
{
	uint32_t i;
	size_t arry_size = ARRAY_SIZE(fault_no_need_reset_gpu);

	for (i = 0; i < arry_size; i++) {
		if (fault_code == fault_no_need_reset_gpu[i])
			return false;
	}
	return true;
}

bool hvgr_gpu_reset_type_normal(uint32_t fault_code)
{
	uint32_t i;
	size_t arry_size = ARRAY_SIZE(fault_reset_gpu_normal);

	for (i = 0; i < arry_size; i++) {
		if (fault_code == fault_reset_gpu_normal[i])
			return true;
	}
	return false;
}

int hvgr_msg_fault_cmp(const void *key, const void *elt)
{
	uint32_t target = *(uint32_t *)key;
	struct hvgr_dm_fault_msg *msg_elt = (struct hvgr_dm_fault_msg *)elt;

	if (target == msg_elt->fault_code)
		return 0;
	if (target > msg_elt->fault_code)
		return 1;

	return -1;
}

const char *hvgr_exception_name(uint32_t exception_code)
{
	struct hvgr_dm_fault_msg *msg = NULL;

	msg = bsearch(&exception_code, gpu_fault_name_tbl, ARRAY_SIZE(gpu_fault_name_tbl),
		sizeof(struct hvgr_dm_fault_msg),
		hvgr_msg_fault_cmp);
	if (msg == NULL)
		return "UNKNOWN";

	return msg->fault_name;
}

void hvgr_gpu_fault_proc(struct hvgr_device *gdev, uint32_t val)
{
	uint32_t status;
	uint64_t address;
	uint64_t value;
	uint32_t type;
	bool is_reset_gpu = false;
	uint32_t m = val & GPU_FAULT;
	static const char * const gpu_fault_type_tbl[] = { "JOB", "CQ", "FCP",
		"FCP_CTRL", "MISC_CTRL" };
	uint32_t chn;
	int ret;
	uint32_t offset;
	char process_msg[MAX_DMD_LOG_LENTH] = {0};
	char dmd_msg[MAX_DMD_LOG_LENTH] = {0};

	while (m) {
		type = (uint32_t)ffs((int)m) - 1;
		offset = type * GPU_FAULT_REG_STRIDE;

		status = hvgr_read_reg(gdev,
			gpu_control_reg(gdev, GPU_FAULTSTATUS + offset));
		address = (uint64_t)hvgr_read_reg(gdev,
			gpu_control_reg(gdev, GPU_FAULTDATA1 + offset));
		address <<= 32;
		address |= hvgr_read_reg(gdev,
			gpu_control_reg(gdev, GPU_FAULTDATA0 + offset));

		value = (uint64_t)hvgr_read_reg(gdev,
			gpu_control_reg(gdev, GPU_FAULTDATA3 + offset));
		value <<= 32;
		value |= hvgr_read_reg(gdev,
			gpu_control_reg(gdev, GPU_FAULTDATA2 + offset));

		chn = (status & 0xff00);
		status &= 0xFF;

		if (type == 1) {
			hvgr_err(gdev, HVGR_DM,
				"%s Fault 0x%08x (%s) at 0x%016llx, value 0x%016llx, chn %d",
				gpu_fault_type_tbl[type], status,
				hvgr_exception_name(status),
				address, value, chn);
		} else {
			hvgr_err(gdev, HVGR_DM,
				"%s Fault 0x%08x (%s) at 0x%016llx, value 0x%016llx",
				gpu_fault_type_tbl[type], status,
				hvgr_exception_name(status),
				address, value);
		}

		if (hvgr_gpu_reset_needed_by_fault(status))
			is_reset_gpu = true;

		m &= ~(1U << ((uint32_t)ffs(m) - 1));
	}

	hvgr_gpu_fault_show_process(gdev, process_msg, MAX_DMD_LOG_LENTH);
	if (is_reset_gpu) {
		ret = snprintf_s(dmd_msg, MAX_DMD_LOG_LENTH, MAX_DMD_LOG_LENTH - 1,
			"higpu gpu fault,fault:%u 0x%x,data:%u,task:%s",
			type, status, chn, process_msg);
		if (ret > 0)
			hvgr_datan_gpu_fault_type_set(gdev, DMD_JOB_FAIL, dmd_msg);
		else
			hvgr_datan_gpu_fault_type_set(gdev, DMD_JOB_FAIL, "higpu gpu fault");

		if (!hvgr_gpu_reset_type_normal(status))
			hvgr_pm_gpu_reset(gdev, GPU_RESET_TYPE_IMMEDIATELY);
		else
			hvgr_pm_gpu_reset(gdev, GPU_RESET_TYPE_NORMAL);
	}
}
