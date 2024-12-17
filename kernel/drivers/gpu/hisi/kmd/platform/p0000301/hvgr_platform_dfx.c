/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include <linux/security.h>
#include <linux/version.h>
#include <linux/kthread.h>
#include <linux/device.h>

#include "hvgr_log_api.h"
#include "hvgr_dm_api.h"
#include "hvgr_platform_ffa.h"
#include "hvgr_platform_api.h"

static bool hvgr_dmd_need_check_nv(struct hvgr_device *gdev)
{
	u64 *check_ts = &(gdev->dm_dev.dmd_msg->dmd_check_ts[0]);
	u32 last_idx = (gdev->dm_dev.dmd_msg->dmd_check_cnt - 1) % MAX_DMD_CHECK_CNT;
	u32 first_idx = (1 + last_idx) % MAX_DMD_CHECK_CNT;

	if (check_ts[last_idx] == 0 || check_ts[first_idx] == 0 ||
		check_ts[last_idx] < check_ts[first_idx])
		return false;
	if ((check_ts[last_idx] - check_ts[first_idx]) <= MAX_DMD_CHECK_INTERVAL)
		return true;

	return false;
}

static int hvgr_dmd_sh_process(void *data)
{
	enum dmd_nv_mark nv_mark;
	struct hvgr_dmd_nv_data nv_data = {0};
	struct hvgr_device *gdev = (struct hvgr_device *)data;

	if (unlikely(gdev == NULL))
		return -1;

	mutex_lock(&gdev->dm_dev.dmd_msg->dmd_lock);
	hvgr_dmd_sr_awake(gdev);
	if (hvgr_dmd_nv_read(gdev, &nv_data) != 0 || nv_data.info.bit.en == 0)
		goto err_end;

	if (nv_data.info.bit.sh_mark == 1) {
		/* unmark, write sh_mark to 0 */
		nv_data.info.bit.sh_mark = 0;
		if (hvgr_dmd_nv_write(gdev, &nv_data) != 0)
			goto err_end;

		hvgr_dmd_sr_relax(gdev);
		nv_mark = DMD_NV_UNMARK;
		goto end;
	}
	/* mark, write dmd nv to mark value */
	nv_data.info.bit.sh_mark = 1;
	if (hvgr_dmd_nv_write(gdev, &nv_data) != 0)
		goto err_end;

	hvgr_dmd_sr_relax(gdev);
	nv_mark = DMD_NV_MARK;
	(void)hvgr_platform_sh_runtime(gdev, (unsigned char *)&nv_data, sizeof(nv_data));
end:
	gdev->dm_dev.dmd_msg->dmd_sh_flag = 1;
	mutex_unlock(&gdev->dm_dev.dmd_msg->dmd_lock);
	if (nv_mark == DMD_NV_MARK)
		hvgr_dmd_msg_set(gdev, DMD_SH_NV_MARK, "dmd sh nv mark");
	else
		hvgr_dmd_msg_set(gdev, DMD_SH_NV_MARK, "dmd sh nv unmark");
	hvgr_dmd_msg_report(gdev);
	return 0;
err_end:
	gdev->dm_dev.dmd_msg->dmd_sh_flag = 0;
	hvgr_dmd_sr_relax(gdev);
	mutex_unlock(&gdev->dm_dev.dmd_msg->dmd_lock);
	return -1;
}

void hvgr_dmd_msg_check(struct hvgr_device *gdev)
{
	struct task_struct *tsk = NULL;

	if (unlikely(gdev == NULL))
		return;

	mutex_lock(&gdev->dm_dev.dmd_msg->dmd_lock);
	if (gdev->dm_dev.dmd_msg->dmd_sh_flag != 0 || gdev->dm_dev.dmd_msg->dmd_check_cnt == 0 ||
		!hvgr_dmd_need_check_nv(gdev)) {
		mutex_unlock(&gdev->dm_dev.dmd_msg->dmd_lock);
		return;
	}
	gdev->dm_dev.dmd_msg->dmd_sh_flag = 1;
	mutex_unlock(&gdev->dm_dev.dmd_msg->dmd_lock);

	tsk = kthread_run(hvgr_dmd_sh_process, (void *)gdev, "dmd_sh_process");
	if (IS_ERR(tsk)) {
		gdev->dm_dev.dmd_msg->dmd_sh_flag = 0;
		hvgr_err(gdev, HVGR_DM, "higpu_dmd: sh tsk fail");
	}
}
