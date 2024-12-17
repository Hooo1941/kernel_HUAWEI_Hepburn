/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: gear ctrl implement
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <securec.h>
#include <platform_include/basicplatform/linux/hck/ufs/hck_scsi_ufs_gear_ctrl.h>
#include "ufshcd.h"
#include "hufs_plat.h"
#include "ufs_gear_ctrl.h"
#include "scsi_gear_ctrl.h"
#include "dsm_ufs.h"
#include "ufshcd_extend.h"

#define break_if_fail_else_add_ret(ret, count) \
	do {				       \
		if ((ret) < 0)		       \
			return 0;	       \
		else			       \
			(count) += (ret);      \
	} while (0)

#define UFS_CTRL_GEAR_VALUE_GEAR3 3
#define UFS_CTRL_GEAR_VALUE_GEAR4 4
#define UFS_CTRL_GEAR_VALUE_LANE1 1
#define UFS_CTRL_GEAR_VALUE_LANE2 2

static ufs_ctrl_gear_type_t g_gear_type = UFS_CTRL_GEAR_TYPE_GEAR;

struct ufs_gear_ctrl_info {
	ufs_ctrl_gear_type_t type;
	ufs_ctrl_gear_level_t level;
	uint8_t gear;
	uint8_t lane;
};

static struct ufs_gear_ctrl_info g_gear_info[] = {
	{ .type = UFS_CTRL_GEAR_TYPE_LANE,
	  .level = UFS_CTRL_GEAR_LEVEL_0,
	  .gear = UFS_CTRL_GEAR_VALUE_GEAR4,
	  .lane = UFS_CTRL_GEAR_VALUE_LANE1 },
	{ .type = UFS_CTRL_GEAR_TYPE_LANE,
	  .level = UFS_CTRL_GEAR_LEVEL_1,
	  .gear = UFS_CTRL_GEAR_VALUE_GEAR4,
	  .lane = UFS_CTRL_GEAR_VALUE_LANE2 },
	{ .type = UFS_CTRL_GEAR_TYPE_GEAR,
	  .level = UFS_CTRL_GEAR_LEVEL_0,
	  .gear = UFS_CTRL_GEAR_VALUE_GEAR3,
	  .lane = UFS_CTRL_GEAR_VALUE_LANE2 },
	{ .type = UFS_CTRL_GEAR_TYPE_GEAR,
	  .level = UFS_CTRL_GEAR_LEVEL_1,
	  .gear = UFS_CTRL_GEAR_VALUE_GEAR4,
	  .lane = UFS_CTRL_GEAR_VALUE_LANE2 },
};

static spinlock_t switch_info_lock;
static bool is_gear_switching;
static unsigned long gear_switch_count;
static int last_set_gear_level;
static int last_gear_change_reason;
static ktime_t last_gear_proc_ts[UFS_DSS_LATENCY_SCENARIO_MAX];
static bool last_gear_proc_request;

static int ufshcd_gear_ctrl_fill_value_by_info(struct ufs_gear_ctrl_info *info)
{
	uint32_t i;

	if (!info || info->level > UFS_CTRL_GEAR_LEVEL_MAX || info->type > UFS_CTRL_GEAR_TYPE_MAX)
		return -EINVAL;

	for (i = 0; i < ARRAY_SIZE(g_gear_info); i++) {
		if (g_gear_info[i].type == info->type && g_gear_info[i].level == info->level) {
			info->gear = g_gear_info[i].gear;
			info->lane = g_gear_info[i].lane;
			return 0;
		}
	}
	return -EINVAL;
}

static int ufshcd_gear_ctrl_find_level_by_type_and_value(struct ufs_gear_ctrl_info *info)
{
	uint32_t i;

	if (!info || info->type > UFS_CTRL_GEAR_TYPE_MAX)
		return -EINVAL;

	for (i = 0; i < ARRAY_SIZE(g_gear_info); i++) {
		if (g_gear_info[i].type == info->type && g_gear_info[i].gear == info->gear &&
		    g_gear_info[i].lane == info->lane) {
			info->level = g_gear_info[i].level;
			return 0;
		}
	}
	return -EINVAL;
}

static int ufshcd_swith_gear_param_get(ufs_ctrl_gear_level_t gear_level, struct ufs_pa_layer_attr *pwr_mode)
{
	struct ufs_gear_ctrl_info info;
	int ret;

	info.type = g_gear_type;
	info.level = gear_level;
	ret = ufshcd_gear_ctrl_fill_value_by_info(&info);
	if (ret) {
		pr_err("%s, unsupport gear levle. type %d level %d\n", g_gear_type, gear_level);
		return ret;
	}

	pwr_mode->gear_rx = info.gear;
	pwr_mode->gear_tx = info.gear;
	pwr_mode->lane_rx = info.lane;
	pwr_mode->lane_tx = info.lane;

	return 0;
}

static int ufshcd_dev_read_lrb_use_rate(struct scsi_device *sdev, unsigned long *lrb_use_rate)
{
	struct ufs_hba *hba = NULL;
	unsigned long num = 0;
	unsigned long total;

	if (!sdev || !sdev->host)
		return -EINVAL;
	hba = shost_priv(sdev->host);
	if (unlikely(!hba))
		return -EINVAL;

	num = hweight64(hba->lrb_in_use);
	total = (unsigned long)hba->nutrs;
	if (num >= total)
		*lrb_use_rate = 100;
	else
		*lrb_use_rate = (num * 100) / total;

	return 0;
}

#ifdef CONFIG_HUAWEI_UFS_DSM
static void ufshcd_dss_dmd_update(struct ufs_hba *hba, u32 set_gear, int sub_err, int *sched_ret)
{
	unsigned long flags;

	spin_lock_irqsave(hba->host->host_lock, flags);
	*sched_ret = dsm_ufs_update_ufs_dynamic_switch_info(hba, g_gear_type, set_gear, sub_err,
							    DSM_UFS_DYNAMIC_SWITCH_SPEED_ERR);
	spin_unlock_irqrestore(hba->host->host_lock, flags);
	return;
}
static void ufshcd_dss_dmd_schedule(struct ufs_hba *hba, int sched_ret)
{
	if (sched_ret == DSM_UFS_DSS_SCHEDUAL)
		schedule_ufs_dsm_work(hba);
}
#else
#define DSM_UFS_DSS_ERR (-1)
static inline void ufshcd_dss_dmd_update(struct ufs_hba *hba, u32 set_gear, int sub_err, int *sched_ret)
{
}
static inline void ufshcd_dss_dmd_schedule(struct ufs_hba *hba, int sched_ret)
{
}
#endif

static bool ufshcd_change_gear_result_process(struct ufs_hba *hba, u32 set_gear, int result)
{
	struct ufs_gear_ctrl_info info;
	static uint8_t continues_reset_count = 0;
	int ret;
	bool need_hreset = false;
	int code;
	int sched_ret = DSM_UFS_DSS_ERR;
	bool device_abandon = mas_blk_check_gear_ctrl_abandon_on_device();

	if (!result) {
		code = (set_gear == UFS_CTRL_GEAR_LEVEL_MAX) ? UFS_DSS_INFO_SET_HIGH : UFS_DSS_INFO_SET_LOW;
		/* dmd1: update switch count */
		ufshcd_dss_dmd_update(hba, set_gear, code, &sched_ret);
		if (set_gear == UFS_CTRL_GEAR_LEVEL_MAX)
			continues_reset_count = 0;
		goto out;
	}

	info.type = g_gear_type;
	info.gear = hba->pwr_info.gear_rx;
	info.lane = hba->pwr_info.lane_rx;
	/* dmd2: report switch failure */
	ufshcd_dss_dmd_update(hba, set_gear, UFS_DSS_SWITCH_ERR, &sched_ret);
	dev_err(hba->dev, "%s: Failed to switch gear %d, err = %d\n", __func__, set_gear, result);
	dev_err(hba->dev, "gear:\t\t%d\t%d\nlane:\t\t%d\t%d\npwr mode:\t%d\t%d\n", hba->pwr_info.gear_rx,
		hba->pwr_info.gear_tx, hba->pwr_info.lane_rx, hba->pwr_info.lane_tx, hba->pwr_info.pwr_rx,
		hba->pwr_info.pwr_tx);
	ret = ufshcd_gear_ctrl_find_level_by_type_and_value(&info);
	if (ret || info.level != UFS_CTRL_GEAR_LEVEL_MAX || device_abandon) {
		/* Case1: unrecognize gear ctrl level. it means unexpected error occurred in power mode change.
		 * Case2: current gear ctrl level not equal level max. it means "switch to high" failed.
		 * Case3: device abandon this feature in run-time, now only because of long ufs pwc change expired time.
		 * In upper cases, there need a hardware reset. */
		need_hreset = true;
		/* dmd3: report hardware reset event. */
		ufshcd_dss_dmd_update(hba, set_gear, UFS_DSS_RESET, &sched_ret);
		dev_err(hba->dev, "%s: reset occur. get failed code %d, cur level %d\n", __func__, ret, info.level);
		continues_reset_count++;
		/* continues rest threshold now is 3. */
		if (continues_reset_count >= 3 || device_abandon) {
			/* dmd4: report ufs dynamic switch speed feature to be closed. */
			ufshcd_dss_dmd_update(hba, set_gear, UFS_DSS_FEATURE_CLOSE, &sched_ret);
			dev_err(hba->dev, "%s: feature closed occur.\n", __func__);
			mas_blk_disable_gear_ctrl(true);
		}
	}
out:
	ufshcd_dss_dmd_schedule(hba, sched_ret);
	return need_hreset;
}

static int ufshcd_change_gear(struct ufs_hba *hba, u32 set_gear)
{
	int ret;
	errno_t errno;
	struct ufs_pa_layer_attr pwr_info;
	unsigned long flags;
	bool need_hreset = false;

	errno = memcpy_s(&pwr_info, sizeof(struct ufs_pa_layer_attr), &hba->pwr_info, sizeof(struct ufs_pa_layer_attr));
	if (errno != EOK) {
		dev_err(hba->dev, "%s: Failed to get ori pwr para\n", __func__);
		ret = -ENOMEM;
		goto out;
	}
	ret = ufshcd_swith_gear_param_get(set_gear, &pwr_info);
	if (ret) {
		if (ret == -ECANCELED) {
			ret = 0;
			dev_err(hba->dev, "%s: No need to redo gear switch\n", __func__);
		} else {
			dev_err(hba->dev, "%s: Failed to get pwr para\n", __func__);
		}
		goto out;
	}

	spin_lock_irqsave(hba->host->host_lock, flags);
	if (hba->ufshcd_state != UFSHCD_STATE_OPERATIONAL || hba->force_reset)
		goto unlock_hostlock;
	if (ufshcd_eh_in_progress(hba))
		goto unlock_hostlock;
	spin_unlock_irqrestore(hba->host->host_lock, flags);
	mutex_lock(&hba->eh_mutex);
	if (!hba->ufs_in_err_handle) {
		down(&hba->host_sem);
		down_write(&hba->clk_scaling_lock);
		ufshcd_switch_latency_check(hba, UFS_DSS_LATENCY_SCENARIO_PROCESS, 0, 0);
		ret = ufshcd_config_pwr_mode(hba, &pwr_info);
		up_write(&hba->clk_scaling_lock);
		need_hreset = ufshcd_change_gear_result_process(hba, set_gear, ret);
		up(&hba->host_sem);
	}
	mutex_unlock(&hba->eh_mutex);
	goto out;
unlock_hostlock:
	spin_unlock_irqrestore(hba->host->host_lock, flags);
out:
	if (need_hreset)
		ufshcd_host_force_reset_sync(hba);
	return ret;
}

#define UFS_GEAR_SWITCH_LEVEL_NOT_SUPPORT_PRINT_INTERVAL_MASK 0xFFF
static int ufshcd_gear_level_get_by_hba(struct ufs_hba *hba, u32 *get_gear)
{
	int ret;
	struct ufs_gear_ctrl_info info;
	static uint32_t unsupport_print_cnt = 0;

	info.type = g_gear_type;
	if (hba->pwr_info.gear_rx != hba->pwr_info.gear_tx || hba->pwr_info.lane_rx != hba->pwr_info.lane_tx)
		goto unrecognize;
	info.gear = hba->pwr_info.gear_rx;
	info.lane = hba->pwr_info.lane_rx;
	ret = ufshcd_gear_ctrl_find_level_by_type_and_value(&info);
	if (ret)
		goto unrecognize;
	unsupport_print_cnt = 0;
	*get_gear = info.level;

	return 0;
unrecognize:
	if (!(unsupport_print_cnt & UFS_GEAR_SWITCH_LEVEL_NOT_SUPPORT_PRINT_INTERVAL_MASK))
		pr_err("%s, unrecognize gear type and value. type %d, gear %d, lane %d, %d\n", __func__, info.type,
		       info.gear, info.lane, g_gear_type);
	unsupport_print_cnt++;
	return -EPERM;
}

static int ufshcd_gear_level_get(struct scsi_device *sdev, u32 *get_gear)
{
	struct ufs_hba *hba = NULL;

	if (unlikely(!sdev || !get_gear))
		return -EINVAL;

	hba = shost_priv(sdev->host);
	if (unlikely(!hba))
		return -EINVAL;

	return ufshcd_gear_level_get_by_hba(hba, get_gear);
}

static inline int ufshcd_gear_ctrl_inner(struct ufs_hba *hba, u32 set_gear)
{
	int ret;

	ret = ufshcd_change_gear(hba, set_gear);

	return ret;
}

static int ufshcd_gear_ctrl(struct scsi_device *sdev, u32 set_gear)
{
	int ret;
	struct ufs_hba *hba = NULL;
	u32 cur_gear;

	if (unlikely(!sdev || set_gear > UFS_CTRL_GEAR_LEVEL_MAX))
		return -EINVAL;

	hba = shost_priv(sdev->host);
	if (unlikely(!hba))
		return -EINVAL;

	ret = ufshcd_gear_level_get(sdev, &cur_gear);
	if (unlikely(ret))
		return -EINVAL;

	if (set_gear == cur_gear)
		return 0;

	ret = ufshcd_gear_ctrl_inner(hba, set_gear);
	return ret;
}

static int ufshcd_gear_level_cap_get(struct scsi_device *sdev, u32 *min_gear, u32 *max_gear)
{
	if (min_gear)
		*min_gear = UFS_CTRL_GEAR_LEVEL_MIN;
	if (max_gear)
		*max_gear = UFS_CTRL_GEAR_LEVEL_MAX;
	return 0;
}

#define UFS_DSS_WHITE_LIST_MAX 5
struct ufs_dss_dtsi_cfg_info {
	u32 chip_id;
	char *chipname;
	u32 enable;
	u32 gear_type;
	u32 flow_cnt_per_ms;
	u32 flow_size_per_100us;
	char *model_name;
	int white_list_cnt;
	char *white_lists[UFS_DSS_WHITE_LIST_MAX];
};

static int ufshcd_ufs_dss_white_list(struct ufs_hba *hba, struct ufs_dss_dtsi_cfg_info *info)
{
	int i;
	for (i = 0; i < info->white_list_cnt; i++) {
		if (strstarts(info->model_name, info->white_lists[i])) {
			dev_err(hba->dev, "%s, %s === %s", __func__, info->model_name, info->white_lists[i]);
			return 0;
		}
		dev_err(hba->dev, "%s, %s != %s", __func__, info->model_name, info->white_lists[i]);
	}

	return -EPERM;
}

static int ufshcd_dtsi_read_cfg(struct ufs_hba *hba, struct device_node *np, struct ufs_dss_dtsi_cfg_info *info)
{
	int ret;
	u32 chip_id = 0;

	if (info->chip_id) {
		ret = of_property_read_u32(np, "manufacturer_id", &chip_id);
		if (ret < 0) {
			dev_err(hba->dev,
				"%s, manufacturer_id read failed! %d\n",
				__func__, ret);
			return ret;
		}
		if (info->chip_id != chip_id) {
			dev_info(hba->dev,
				 "%s, manufacturer_id not equal, %x, %x!\n",
				 __func__, info->chip_id, chip_id);
			return -EAGAIN;
		}
		dev_info(hba->dev, "%s, chip id bingo %x\n", __func__,
			 info->chip_id);
	} else {
		dev_err(hba->dev, "%s, not support no-fix chip_id %x\n",
			__func__, info->chip_id);
		return -EPERM;
	}

	ret = of_property_read_string(np, "chipname",
				      (const char **)&info->chipname);
	if (ret < 0) {
		dev_err(hba->dev, "%s, chipname read failed! %d\n", __func__,
			ret);
		return ret;
	}
	ret = of_property_read_u32(np, "mode_enable", &info->enable);
	if (ret < 0) {
		dev_err(hba->dev, "%s, mode_enable read failed! %d\n", __func__,
			ret);
		return ret;
	}
	ret = of_property_read_u32(np, "flow_cnt_per_ms",
				   &info->flow_cnt_per_ms);
	if (ret < 0) {
		dev_err(hba->dev, "%s, flow_cnt_per_ms read failed! %d\n",
			__func__, ret);
		return ret;
	}
	ret = of_property_read_u32(np, "flow_size_per_100us",
				   &info->flow_size_per_100us);
	if (ret < 0) {
		dev_err(hba->dev, "%s, flow_size_per_100us read failed! %d\n",
			__func__, ret);
		return ret;
	}
	ret = of_property_read_u32(np, "gear_type", &info->gear_type);
	if (ret < 0) {
		dev_err(hba->dev, "%s, gear_type read failed! %d\n", __func__,
			ret);
		return ret;
	}
	dev_err(hba->dev, "%s, %s enable %d cnt %d size %d, gear_type %d\n",
		__func__, info->chipname, info->enable, info->flow_cnt_per_ms,
		info->flow_size_per_100us, info->gear_type);

	if (info->enable && info->model_name) {
		ret = of_property_read_string_array( np, "model_white_lists",
			(const char **)&info->white_lists, UFS_DSS_WHITE_LIST_MAX);
		if (ret > 0) {
			dev_info(hba->dev, "%s, white lists exit!\n", __func__);
			info->white_list_cnt = ret;
			return ufshcd_ufs_dss_white_list(hba, info);
		}
	}
	return 0;
}

static int ufshcd_cap_get_by_chip(struct ufs_hba *hba, struct device_node *np, struct ufs_dss_dtsi_cfg_info *info)
{
	struct device_node *chipcfg, *chips;
	int ret = -EINVAL;

	chips = of_get_child_by_name(np, "Chips");
	if (!chips) {
		dev_err(hba->dev, "%s, chips not exit!\n", __func__);
		return -EINVAL;
	}
	info->chip_id = hba->manufacturer_id;
	info->model_name = hba->model;
	for_each_available_child_of_node (chips, chipcfg) {
		ret = ufshcd_dtsi_read_cfg(hba, chipcfg, info);
		if (ret == -EAGAIN) {
			continue;
		} else if (ret < 0) {
			dev_err(hba->dev, "%s, default confit read failed! %d\n", __func__, ret);
			return ret;
		} else {
			return 0;
		}
	}
	return ret;
}

static int ufshcd_cap_get(struct ufs_hba *hba)
{
	struct device_node *np = of_find_compatible_node(NULL, NULL, "ufs,ufs_dss");
	struct ufs_dss_dtsi_cfg_info tmp;
	int ret;

	if (!np) {
		dev_err(hba->dev, "%s, ufs,ufs_dss path not exit!\n", __func__);
		return -EINVAL;
	}

	ret = ufshcd_cap_get_by_chip(hba, np, &tmp);
	if (ret < 0) {
		dev_err(hba->dev, "%s, can't find chip config in dtsi\n", __func__);
		return ret;
	}

	if (!tmp.enable)
		return -EPERM;

	mas_gear_ctrl_flow_config(tmp.flow_size_per_100us, tmp.flow_cnt_per_ms);
	g_gear_type = tmp.gear_type;

	return 0;
}

static int ufshcd_is_gear_ctrl_support(struct ufs_hba *hba)
{
	struct hufs_host *host = NULL;

	if (!hba || !hba->max_pwr_info.is_valid) {
		pr_err("%s, haven't init yet!\n", __func__);
		return -EINVAL;
	}

	if (hba->max_pwr_info.info.gear_rx != UFS_CTRL_GEAR_VALUE_GEAR4 ||
	    hba->max_pwr_info.info.gear_tx != UFS_CTRL_GEAR_VALUE_GEAR4 ||
	    hba->max_pwr_info.info.lane_rx != UFS_CTRL_GEAR_VALUE_LANE2 ||
	    hba->max_pwr_info.info.lane_tx != UFS_CTRL_GEAR_VALUE_LANE2) {
		dev_err(hba->dev, "%s, mode unrecognize %d,%d %d,%d\n", __func__,
				hba->max_pwr_info.info.gear_rx, hba->max_pwr_info.info.gear_tx,
				hba->max_pwr_info.info.lane_rx, hba->max_pwr_info.info.lane_tx);
		return -EINVAL;
	}

	host = (struct hufs_host *)hba->priv;
	if (host->caps & USE_HUFS_MPHY_TC) {
		dev_err(hba->dev, "%s, caps not support!\n", __func__);
		return -EPERM;
	}
	if (ufshcd_cap_get(hba)) {
		dev_err(hba->dev, "%s, dtsi config not support!\n", __func__);
		return -EPERM;
	}

	return 0;
}

static void hck_gear_ctrl_adjust_capbility(struct ufs_hba *hba)
{
	if (ufshcd_is_gear_ctrl_support(hba))
		mas_blk_disable_gear_ctrl(true);
}

const char *ufs_dss_scnario_str[UFS_DSS_LATENCY_SCENARIO_MAX] = { "UFS_DSS_LATENCY_SCENARIO_REQUEST",
							  "UFS_DSS_LATENCY_SCENARIO_ISSUE",
							  "UFS_DSS_LATENCY_SCENARIO_PROCESS",
							  "UFS_DSS_LATENCY_SCENARIO_REG_DONE",
							  "UFS_DSS_LATENCY_SCENARIO_INT_BACK",
							  "UFS_DSS_LATENCY_SCENARIO_FINISH" };
void ufshcd_dss_dump_ts(struct ufs_hba *hba)
{
	int i;
	char log[UFSDSS_LOG_SIZE];
	int offset = 0;
	int ret;

	if (last_gear_proc_request) {
		pr_err("[ufs-dss]: set %d, reason %d, total %d\n", last_set_gear_level, last_gear_change_reason,
		       gear_switch_count);
		for (i = 0; i < UFS_DSS_LATENCY_SCENARIO_MAX; i++) {
			ret = snprintf_s(log + offset, UFSDSS_LOG_SIZE - offset, UFSDSS_LOG_SIZE - offset - 1,
					 "%s=%lld ", ufs_dss_scnario_str[i], last_gear_proc_ts[i]);
			if (ret < 0)
				break;
			offset += ret;
		}
		log[offset] = '\0';
		pr_err("[ufs-dss]: %s\n", log);
		pr_err("[ufs-dss]: from queue last %dus, device proc %dus\n",
		       ktime_us_delta(last_gear_proc_ts[UFS_DSS_LATENCY_SCENARIO_INT_BACK],
				      last_gear_proc_ts[UFS_DSS_LATENCY_SCENARIO_REQUEST]),
		       ktime_us_delta(last_gear_proc_ts[UFS_DSS_LATENCY_SCENARIO_INT_BACK],
				      last_gear_proc_ts[UFS_DSS_LATENCY_SCENARIO_REG_DONE]));
	} else if (is_gear_switching) {
		pr_err("[ufs-dss]: direct pwm change %lld ~ %lld %dus\n",
				last_gear_proc_ts[UFS_DSS_LATENCY_SCENARIO_REG_DONE],
				last_gear_proc_ts[UFS_DSS_LATENCY_SCENARIO_INT_BACK],
				ktime_us_delta(last_gear_proc_ts[UFS_DSS_LATENCY_SCENARIO_INT_BACK],
					last_gear_proc_ts[UFS_DSS_LATENCY_SCENARIO_REG_DONE]));
	}
}

static bool ufshcd_dss_should_dump(void)
{
	int delta_us;
	bool ret = false;
	if (last_gear_proc_request) {
		delta_us = ktime_us_delta(last_gear_proc_ts[UFS_DSS_LATENCY_SCENARIO_INT_BACK],
					  last_gear_proc_ts[UFS_DSS_LATENCY_SCENARIO_REQUEST]);
		ret = delta_us > 4000;
	}
	if (!ret && is_gear_switching) {
		delta_us = ktime_us_delta(last_gear_proc_ts[UFS_DSS_LATENCY_SCENARIO_INT_BACK],
					  last_gear_proc_ts[UFS_DSS_LATENCY_SCENARIO_REG_DONE]);
		ret = delta_us > 1000;
	}
	return ret;
}

int ufshcd_switch_latency_check(struct ufs_hba *hba, enum ufs_dss_latency_enum scenario, int gear, int reason)
{
	int ret = 0;
	bool should_dump = false;
	ktime_t now = ktime_get();

	if (scenario >= UFS_DSS_LATENCY_SCENARIO_MAX)
		return -EINVAL;

	trace_printk("%s %lld\n", ufs_dss_scnario_str[scenario], now);
	spin_lock(&switch_info_lock);
	last_gear_proc_ts[scenario] = now;
	switch (scenario) {
	case UFS_DSS_LATENCY_SCENARIO_REQUEST:
		last_set_gear_level = gear;
		last_gear_change_reason = reason;
		last_gear_proc_request = true;
		break;
	case UFS_DSS_LATENCY_SCENARIO_REG_DONE:
		is_gear_switching = true;
		break;
	case UFS_DSS_LATENCY_SCENARIO_INT_BACK:
		should_dump = ufshcd_dss_should_dump();
		gear_switch_count++;
		break;
	default:
		break;
	}
	spin_unlock(&switch_info_lock);
	if (should_dump)
		ufshcd_dss_dump_ts(hba);
	if (scenario == UFS_DSS_LATENCY_SCENARIO_INT_BACK) {
		spin_lock(&switch_info_lock);
		is_gear_switching = false;
		last_gear_proc_request = false;
		spin_unlock(&switch_info_lock);
	}
	return 0;
}

static int ufshcd_switch_latency_check_in(struct scsi_device *sdev, enum ufs_dss_latency_enum scenario, int gear,
					  int reason)
{
	struct ufs_hba *hba = NULL;

	hba = shost_priv(sdev->host);
	if (unlikely(!hba))
		return -EINVAL;

	return ufshcd_switch_latency_check(hba, scenario, gear, reason);
}

struct scsi_host_template_gear_ctrl_ops ufs_gear_ctrl_ops = {
	.read_lrb_use_rate = ufshcd_dev_read_lrb_use_rate,
	.gear_ctrl = ufshcd_gear_ctrl,
	.gear_level_get = ufshcd_gear_level_get,
	.gear_level_cap_get = ufshcd_gear_level_cap_get,
	.switch_latency_check = ufshcd_switch_latency_check_in,
};

void ufshcd_gear_ctrl_op_register(struct ufs_hba *hba)
{
	struct scsi_host_template *hostt = NULL;

	if (!hba || !hba->host)
		return;

	hostt = hba->host->hostt;
	if (!hostt)
		return;

	hostt->gear_ctrl_ops = &ufs_gear_ctrl_ops;
	spin_lock_init(&switch_info_lock);
	REGISTER_HCK_VH(scsi_dev_gear_ctrl_register, hck_scsi_dev_gear_ctrl_register);
	REGISTER_HCK_VH(ufs_gear_ctrl_adjust_capbility, hck_gear_ctrl_adjust_capbility);
	return;
}

void ufshcd_gear_ctrl_set_type(struct ufs_hba *hba, ufs_ctrl_gear_type_t type)
{
	int ret;

	if (type <= UFS_CTRL_GEAR_TYPE_MAX && g_gear_type != type) {
		ret = ufshcd_gear_ctrl_inner(hba, UFS_CTRL_GEAR_LEVEL_MAX);
		if (!ret)
			g_gear_type = type;
	}
}

ufs_ctrl_gear_type_t ufshcd_gear_ctrl_get_type(void)
{
	return g_gear_type;
}

static ssize_t hufs_gear_type_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long value;
	struct ufs_hba *hba = dev_get_drvdata(dev);

	if (kstrtoul(buf, 0, &value))
		return -EINVAL;

	ufshcd_gear_ctrl_set_type(hba, value);
	return count;
}

static ssize_t hufs_gear_type_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	const char *strinfo[] = { "Gear", "Lane" };
	ssize_t offset = 0;
	int ret;
	int i;

	ret = snprintf_s(buf, PAGE_SIZE - offset, PAGE_SIZE - offset - 1, "Support gear type below:\n");
	break_if_fail_else_add_ret(ret, offset);

	for (i = UFS_CTRL_GEAR_TYPE_GEAR; i <= UFS_CTRL_GEAR_TYPE_MAX; i++) {
		ret = snprintf_s(buf + offset, PAGE_SIZE - offset, PAGE_SIZE - offset - 1,
				"%d: %s\n", i, strinfo[i]);
		break_if_fail_else_add_ret(ret, offset);
	}

	ret = snprintf_s(buf + offset, PAGE_SIZE - offset, PAGE_SIZE - offset - 1, "==>> %d\n",
			     ufshcd_gear_ctrl_get_type());
	break_if_fail_else_add_ret(ret, offset);

	return offset;
}

void hufs_gear_type_sysfs_init(struct ufs_hba *hba)
{
	hba->gear_type.gear_type.show = hufs_gear_type_show;
	hba->gear_type.gear_type.store = hufs_gear_type_store;
	sysfs_attr_init(&hba->gear_type.gear_type.attr);
	hba->gear_type.gear_type.attr.name = "gear_type";
	hba->gear_type.gear_type.attr.mode = S_IRUSR | S_IRGRP | S_IWUSR;
	if (device_create_file(hba->dev, &hba->gear_type.gear_type))
		dev_err(hba->dev, "Failed to create sysfs for gear_type\n");
}

