
/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include <platform_include/basicplatform/linux/rdr_pub.h>
#include <platform_include/basicplatform/linux/rdr_platform.h>

#include "mntn_subtype_exception.h"
#include "hvgr_mntn.h"
#include "debugbus/hvgr_mntn_dump.h"
#include "hvgr_log_api.h"

struct rdr_exception_info_s gpu_excetption_info[] = {
	{
	.e_modid            = (u32)MODID_GPU_HANG_EXCEPTION,
	.e_modid_end        = (u32)MODID_GPU_HANG_EXCEPTION,
	.e_process_priority = RDR_ERR,
	.e_reboot_priority  = RDR_REBOOT_NO,
	.e_notify_core_mask = RDR_GPU,
	.e_reset_core_mask  = RDR_GPU,
	.e_from_core        = RDR_GPU,
	.e_reentrant        = (u32)RDR_REENTRANT_DISALLOW,
	.e_exce_type        = GPU_S_EXCEPTION,
	.e_exce_subtype     = GPU_HVGR_HANG,
	.e_upload_flag      = (u32)RDR_UPLOAD_YES,
	.e_from_module      = HVGR_RDR_MODULE_NAME,
	.e_desc             = "GPU_HVGR_HANG",
	},
	{
	.e_modid            = (u32)MODID_GPU_FAULT_EXCEPTION,
	.e_modid_end        = (u32)MODID_GPU_FAULT_EXCEPTION,
	.e_process_priority = RDR_ERR,
	.e_reboot_priority  = RDR_REBOOT_NO,
	.e_notify_core_mask = RDR_GPU,
	.e_reset_core_mask  = RDR_GPU,
	.e_from_core        = RDR_GPU,
	.e_reentrant        = (u32)RDR_REENTRANT_DISALLOW,
	.e_exce_type        = GPU_S_EXCEPTION,
	.e_exce_subtype     = GPU_HVGR_FAULT,
	.e_upload_flag      = (u32)RDR_UPLOAD_YES,
	.e_from_module      = HVGR_RDR_MODULE_NAME,
	.e_desc             = "GPU_HVGR_FAULT",
	},
	{
	.e_modid            = (u32)MODID_GPU_PAGE_FAULT_EXCEPTION,
	.e_modid_end        = (u32)MODID_GPU_PAGE_FAULT_EXCEPTION,
	.e_process_priority = RDR_ERR,
	.e_reboot_priority  = RDR_REBOOT_NO,
	.e_notify_core_mask = RDR_GPU,
	.e_reset_core_mask  = RDR_GPU,
	.e_from_core        = RDR_GPU,
	.e_reentrant        = (u32)RDR_REENTRANT_DISALLOW,
	.e_exce_type        = GPU_S_EXCEPTION,
	.e_exce_subtype     = GPU_HVGR_PAGE_FAULT,
	.e_upload_flag      = (u32)RDR_UPLOAD_YES,
	.e_from_module      = HVGR_RDR_MODULE_NAME,
	.e_desc             = "GPU_HVGR_PAGE_FAULT",
	},
	{
	.e_modid            = (u32)MODID_GPU_CRASH_EXCEPTION,
	.e_modid_end        = (u32)MODID_GPU_CRASH_EXCEPTION,
	.e_process_priority = RDR_ERR,
	.e_reboot_priority  = RDR_REBOOT_NO,
	.e_notify_core_mask = RDR_GPU,
	.e_reset_core_mask  = RDR_GPU,
	.e_from_core        = RDR_GPU,
	.e_reentrant        = (u32)RDR_REENTRANT_DISALLOW,
	.e_exce_type        = GPU_S_EXCEPTION,
	.e_exce_subtype     = GPU_HVGR_CRASH,
	.e_upload_flag      = (u32)RDR_UPLOAD_YES,
	.e_from_module      = HVGR_RDR_MODULE_NAME,
	.e_desc             = "GPU_HVGR_CRASH",
	},
};

static u32 hvgr_get_mntn_exce_subtype(enum hvgr_dmd_id dmd_id)
{
	switch (dmd_id) {
	case DMD_JOB_HANG:
		return MODID_GPU_HANG_EXCEPTION;
	case DMD_JOB_FAIL:
	case DMD_PW_ONOFF_FAIL:
		return MODID_GPU_FAULT_EXCEPTION;
	case DMD_BUS_FAULT:
	case DMD_PAGE_FAULT:
		return MODID_GPU_PAGE_FAULT_EXCEPTION;
	default :
		return INVALID_MODID_TYPE;
	}
}

static void hvgr_mntn_dump(u32 modid, u32 etype, u64 coreid,
	char *pathname, pfn_cb_dump_done pfn_cb)
{
	struct hvgr_device *gdev = NULL;

	gdev = hvgr_get_device();

	hvgr_info(gdev, HVGR_DATAN, "%s IN, dump dir is %s\n", __func__, pathname);

#ifdef CONFIG_HVGR_DFX_DEBUG_BUS_DUMP
	hvgr_mntn_debugbus_write_file(gdev, pathname);
#endif

	if (pfn_cb != NULL)
		pfn_cb(modid, coreid);
}

static int hvgr_mntn_register_core(struct hvgr_device * const gdev)
{
	struct rdr_register_module_result retinfo;
	struct rdr_module_ops_pub module_ops;
	int ret;

	module_ops.ops_dump = hvgr_mntn_dump;
	module_ops.ops_reset = NULL;

	ret = rdr_register_module_ops(RDR_GPU, &module_ops, &retinfo);
	if (ret < 0) {
		hvgr_err(gdev, HVGR_DATAN,"register failed! ret = %d\n", ret);
		return ret;
	}

	return 0;
}

static int hvgr_mntn_register_exception(struct hvgr_device * const gdev)
{
	u32 ret;
	size_t size;
	unsigned long index;

	size = sizeof(gpu_excetption_info) / sizeof(struct rdr_exception_info_s);

	for (index = 0; index < (unsigned long)size; index++) {
		/* error return 0, ok return modid */
		ret = rdr_register_exception(&gpu_excetption_info[index]);
		if (ret == 0U) {
			hvgr_err(gdev, HVGR_DATAN,"register fail! index = %ld, ret = %d\n",
				index, ret);
			return -EINVAL;
		}
	}

	return 0;
}

int hvgr_mntn_init(struct hvgr_device * const gdev)
{
	int ret;

	if (gdev == NULL)
		return 0;

	hvgr_info(gdev, HVGR_DATAN, "%s IN", __func__);

	/* register module */
	ret = hvgr_mntn_register_core(gdev);
	if (ret != 0) {
		hvgr_err(gdev, HVGR_DATAN,"hvgr mntn register core failed! ret = %d\n", ret);
		return ret;
	}

	/* regitster exception */
	ret = hvgr_mntn_register_exception(gdev);
	if (ret != 0) {
		rdr_unregister_module_ops(RDR_GPU);
		hvgr_err(gdev, HVGR_DATAN,"hvgr mntn register exception failed! ret = %d\n", ret);
		return ret;
	}

	hvgr_info(gdev, HVGR_DATAN, "%s OUT", __func__);

	return 0;
}

void hvgr_mntn_term(struct hvgr_device * const gdev)
{
	int ret;

	if (gdev == NULL)
		return;

	ret = rdr_unregister_module_ops(RDR_GPU);
	if (ret != 0)
		hvgr_err(gdev, HVGR_DATAN,"hvgr mntn unreg module failed! ret = %d\n", ret);

	ret = rdr_unregister_exception(RDR_GPU);
	if (ret != 0)
		hvgr_err(gdev, HVGR_DATAN,"hvgr mntn unregister failed! ret = %d\n", ret);

	hvgr_info(gdev, HVGR_DATAN,"hvgr mntn deinit success!\n");

	return;
}

void hvgr_mntn_error_report(struct hvgr_device *gdev)
{
	struct hvgr_debugbus_dev *bus_dev = NULL;
	u32 mntn_exce_type = 0U;

	if (gdev == NULL)
		return;

	hvgr_info(gdev, HVGR_DATAN, "%s IN", __func__);

	bus_dev = &gdev->datan_dev.datan_bus_dev;
	mntn_exce_type = hvgr_get_mntn_exce_subtype(bus_dev->gpu_fault_type);
	if (mntn_exce_type == INVALID_MODID_TYPE){
		hvgr_err(gdev, HVGR_DATAN, "%s exce_subtype is INVALID.", __func__);
		rdr_system_error(MODID_GPU_CRASH_EXCEPTION, 0, 0);
		return;
	}

	rdr_system_error(mntn_exce_type, 0, 0);

	hvgr_info(gdev, HVGR_DATAN, "%s OUT", __func__);

	return;
}
