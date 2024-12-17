/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023.
 * All rights reserved.
 */

#include <linux/kthread.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <platform_include/basicplatform/linux/nve/nve_ap_kernel_interface.h>
#include <securec.h>

#include "hvgr_log_api.h"
#include "hvgr_dm_api.h"
#include "hvgr_defs.h"
#include "hvgr_regmap.h"

#define HVGR_DMD_NV_NUM 512
#define HVGR_DMD_GAF_IDX 21
#define HVGR_DMD_GH_IDX 25
#define HVGR_DMD_SID_OFFSET 2
#define HVGR_DMD_SID_MAX_OFFSET 9

int hvgr_dmd_nv_read(struct hvgr_device *gdev, struct hvgr_dmd_nv_data *data)
{
	int ret;
	errno_t err;
	struct opt_nve_info_user nve = {0};

	err = strncpy_s(nve.nv_name, sizeof(nve.nv_name),
			"GPUSH", strlen("GPUSH"));
	if (err != EOK || sizeof(*data) > NVE_NV_DATA_SIZE) {
		hvgr_err(gdev, HVGR_DM, "higpu_dmd nv read name cpy fail");
		ret = -1;
		return ret;
	}
	nve.nv_number = HVGR_DMD_NV_NUM;
	nve.valid_size = (uint32_t)sizeof(*data);
	nve.nv_operation = NV_READ;

	ret = nve_direct_access_interface(&nve);
	if (ret) {
		hvgr_err(gdev, HVGR_DM, "higpu_dmd nv read fail, %d", ret);
		return ret;
	}
	ret = memcpy_s(data, sizeof(*data), nve.nv_data, sizeof(*data));
	if (ret != 0) {
		hvgr_err(gdev, HVGR_DM, "higpu_dmd nv cpoy fail");
		return ret;
	}
	return 0;
}

int hvgr_dmd_nv_write(struct hvgr_device *gdev, struct hvgr_dmd_nv_data *data)
{
	int ret;
	errno_t err;
	struct opt_nve_info_user nve = {0};

	err = strncpy_s(nve.nv_name, sizeof(nve.nv_name),
		"GPUSH", strlen("GPUSH"));
	if (err != EOK || sizeof(*data) > NVE_NV_DATA_SIZE) {
		hvgr_err(gdev, HVGR_DM, "higpu_dmd nv write name cpy failed");
		ret = -1;
		return ret;
	}
	nve.nv_number = HVGR_DMD_NV_NUM;
	nve.valid_size = (uint32_t)sizeof(*data);
	nve.nv_operation = NV_WRITE;
	ret = memcpy_s(nve.nv_data, sizeof(*data), data, sizeof(*data));
	if (ret != 0) {
		hvgr_err(gdev, HVGR_DM, "higpu_dmd nv data copy fail");
		return ret;
	}

	ret = nve_direct_access_interface(&nve);
	if (ret) {
		hvgr_err(gdev, HVGR_DM, "higpu_dmd nv write fail, %d", ret);
		return ret;
	}

	return 0;
}

void hvgr_dmd_sr_awake(struct hvgr_device *gdev)
{
	if (gdev->dm_dev.dmd_msg->sr_wakeup_lock != NULL)
		__pm_stay_awake(gdev->dm_dev.dmd_msg->sr_wakeup_lock);
}

void hvgr_dmd_sr_relax(struct hvgr_device *gdev)
{
	if (gdev->dm_dev.dmd_msg->sr_wakeup_lock != NULL)
		__pm_relax(gdev->dm_dev.dmd_msg->sr_wakeup_lock);
}

static int hvgr_dmd_write_gaf(void *data)
{
	struct hvgr_ctx *ctx = (struct hvgr_ctx *)data;
	struct hvgr_device *gdev = NULL;
	struct hvgr_dmd_nv_data nv_data = {0};
	uint32_t dmd_gaf;

	if (ctx == NULL)
		return -1;

	gdev = ctx->gdev;
	mutex_lock(&gdev->dm_dev.dmd_msg->dmd_lock);
	hvgr_dmd_sr_awake(gdev);
	if (hvgr_dmd_nv_read(gdev, &nv_data) != 0 || nv_data.info.bit.en == 0)
		goto end;

	nv_data.info.bit.gaf_mark = 1;
	dmd_gaf = (uint32_t)atomic_read(&gdev->dm_dev.dmd_msg->dmd_gaf);
	nv_data.buf[HVGR_DMD_GAF_IDX] = (uint8_t)((dmd_gaf >> 24U) & 0xFFU);
	nv_data.buf[HVGR_DMD_GAF_IDX + 1] = (uint8_t)((dmd_gaf >> 16U) & 0xFFU);
	nv_data.buf[HVGR_DMD_GAF_IDX + 2] = (uint8_t)((dmd_gaf >> 8U) & 0xFFU);
	nv_data.buf[HVGR_DMD_GAF_IDX + 3] = (uint8_t)(dmd_gaf & 0xFFU);
	if (hvgr_dmd_nv_write(gdev, &nv_data) != 0)
		goto end;
	hvgr_dmd_sr_relax(gdev);
	mutex_unlock(&gdev->dm_dev.dmd_msg->dmd_lock);
	hvgr_dmd_msg_report(gdev);
	hvgr_dmd_msg_set(gdev, DMD_SH_NV_MARK, "dmd af nv mark");
	hvgr_dmd_msg_report(gdev);
	return 0;
end:
	hvgr_dmd_sr_relax(gdev);
	mutex_unlock(&gdev->dm_dev.dmd_msg->dmd_lock);
	return -1;
}

void hvgr_dmd_set_sid(struct hvgr_ctx *ctx, uint32_t sid)
{
	struct task_struct *tsk = NULL;
	uint32_t dmd_gaf;
	struct hvgr_device *gdev = ctx->gdev;

	if (ctx->gaf_flag != 1)
		return;

	if ((sid < HVGR_MMU_SID_START_OFFSET) || (sid > HVGR_MMU_SID_END_OFFSET)) {
		hvgr_err(ctx->gdev, HVGR_CQ, "sid = %u is invalid!", sid);
		return;
	}

	dmd_gaf = 1u << (sid - HVGR_MMU_SID_START_OFFSET);
	if ((dmd_gaf & (uint32_t)atomic_read(&gdev->dm_dev.dmd_msg->dmd_gaf)) == 1) {
		hvgr_debug(ctx->gdev, HVGR_CQ, "sid = %u already set in nvram!", sid);
		return;
	}

	dmd_gaf = dmd_gaf | (uint32_t)atomic_read(&gdev->dm_dev.dmd_msg->dmd_gaf);
	atomic_set(&gdev->dm_dev.dmd_msg->dmd_gaf, (int)dmd_gaf);
	hvgr_debug(ctx->gdev, HVGR_CQ, "set dmd_gaf = 0x%x", dmd_gaf);

	tsk = kthread_run(hvgr_dmd_write_gaf, (void *)ctx, "dmd_gaf_process");
	if (IS_ERR(tsk))
		hvgr_err(ctx->gdev, HVGR_DM, "higpu_dmd: gaf tsk fail");
}

void hvgr_dmd_config_ctx_gaf(struct hvgr_ctx *ctx, uint32_t flag)
{
	struct hvgr_device *gdev = ctx->gdev;

	if (gdev == NULL || flag != 1)
		return;

	ctx->gaf_flag = flag;
}

uint32_t hvgr_dmd_read_gaf(struct hvgr_device *gdev)
{
	struct hvgr_dmd_nv_data data = {0};
	uint32_t dmd_gaf = 0;

	if (hvgr_dmd_nv_read(gdev, &data) != 0 ||
		data.info.bit.en == 0 ||
		data.info.bit.gaf_mark == 0)
		return 0;

	dmd_gaf |= (uint32_t)data.buf[HVGR_DMD_GAF_IDX] << 24U;
	dmd_gaf |= (uint32_t)data.buf[HVGR_DMD_GAF_IDX + 1] << 16U;
	dmd_gaf |= (uint32_t)data.buf[HVGR_DMD_GAF_IDX + 2] << 8U;
	dmd_gaf |= (uint32_t)data.buf[HVGR_DMD_GAF_IDX + 3];

	return dmd_gaf;
}

void hvgr_updata_core_mask_info(struct hvgr_device *gdev, uint32_t cores)
{
	struct hvgr_dmd_nv_data nv_data = {0};
	char log_buf[MAX_DMD_LOG_LENTH] = {0};

	mutex_lock(&gdev->dm_dev.dmd_msg->dmd_lock);
	if (hvgr_dmd_nv_read(gdev, &nv_data) != 0 || nv_data.info.bit.en == 0)
		goto end;

	nv_data.buf[HVGR_DMD_GH_IDX] |= (uint8_t)cores;

	if (hvgr_dmd_nv_write(gdev, &nv_data) != 0)
		goto end;

	atomic_set(&(gdev->pm_dev.pm_pwr.core_mask_info), (int)nv_data.buf[HVGR_DMD_GH_IDX]);
	mutex_unlock(&gdev->dm_dev.dmd_msg->dmd_lock);
	hvgr_dmd_msg_report(gdev);
	if (sprintf_s(log_buf, MAX_DMD_LOG_LENTH, "dmd gpu hang nv mark, nve core info is 0x%x",
		nv_data.buf[HVGR_DMD_GH_IDX]) == -1) {
		hvgr_err(gdev, HVGR_DM, "nve core info set dmd msg fail");
		return;
	}
	hvgr_dmd_msg_set(gdev, DMD_SH_HANG_NV_MARK, log_buf);
	hvgr_dmd_msg_report(gdev);
	return;
end:
	mutex_unlock(&gdev->dm_dev.dmd_msg->dmd_lock);
	return;
}

uint32_t hvgr_dmd_get_core_mask_info(struct hvgr_device *gdev)
{
	struct hvgr_dmd_nv_data data = {0};

	mutex_lock(&gdev->dm_dev.dmd_msg->dmd_lock);
	if (hvgr_dmd_nv_read(gdev, &data) != 0 || data.info.bit.en == 0) {
		mutex_unlock(&gdev->dm_dev.dmd_msg->dmd_lock);
		return 0;
	}
	mutex_unlock(&gdev->dm_dev.dmd_msg->dmd_lock);
	return (uint32_t)data.buf[HVGR_DMD_GH_IDX];
}