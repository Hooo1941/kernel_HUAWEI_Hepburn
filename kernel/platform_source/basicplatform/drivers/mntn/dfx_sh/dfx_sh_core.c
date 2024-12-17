/*
 * dfx_sh_core.c
 *
 * Copyright (c) 2023 Huawei Technologies Co., Ltd.
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
#include "dfx_sh.h"
#include "dfx_sh_check.h"
#include "rdr_inner.h"
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/printk.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/time64.h>
#include <linux/delay.h>
#include <platform_include/basicplatform/linux/nve/nve_ap_kernel_interface.h>
#include <platform_include/basicplatform/linux/dfx_bbox_diaginfo.h>
#include <platform_include/basicplatform/linux/rdr_platform.h>
#include <securec.h>

static struct task_struct *g_sh_thread;
struct sh_cfg_info g_sh_cfg_info;
bool g_sh_need_update_nve = false;

struct opt_nve_info_user g_nve_sh_cfg_info = {
	.nv_number = SH_CFG_INFO,
	.nv_name = "SHCFG",
	.valid_size = sizeof(g_sh_cfg_info),
};

static bool is_sh_threshold_match(enum sh_type type)
{
	if (g_sh_cfg_info.sw_cnt[type].threshold > 0 &&
			g_sh_cfg_info.sw_cnt[type].count >= g_sh_cfg_info.sw_cnt[type].threshold)
		return true;
	return false;
}

void sh_update_rtc(enum sh_type type, u32 rtc_sec)
{
	pr_info("sh update rtc for type %u, rtc %u\n", type, rtc_sec);
	switch (type) {
	case sh_type_cpu_lit:
		g_sh_cfg_info.panic_time_cpu_lit = rtc_sec;
		break;
	case sh_type_cpu_mid:
		g_sh_cfg_info.panic_time_cpu_mid = rtc_sec;
		break;
	case sh_type_cpu_big:
		g_sh_cfg_info.panic_time_cpu_big = rtc_sec;
		break;
	case sh_type_peri:
		g_sh_cfg_info.panic_time_peri = rtc_sec;
		break;
	default:
		break;
	}
}

static bool sh_check_need_clear(bool enable, unsigned int cur_time, unsigned int last_time, unsigned int interval)
{
	if (!enable || interval == 0)
		return false;
	if (last_time == 0)
		return false;
	if (cur_time > last_time && cur_time - last_time >= interval)
		return true;
	return false;
}

static bool sh_need_clear(enum sh_type type, u32 rtc_sec)
{
	bool need_clear = false;
	switch (type) {
	case sh_type_cpu_lit:
		need_clear = sh_check_need_clear(g_sh_cfg_info.clear_interval_enable.cpu_lit,
				rtc_sec, g_sh_cfg_info.panic_time_cpu_lit, g_sh_cfg_info.clear_interval_cpu_lit);
		break;
	case sh_type_cpu_mid:
		need_clear = sh_check_need_clear(g_sh_cfg_info.clear_interval_enable.cpu_mid,
				rtc_sec, g_sh_cfg_info.panic_time_cpu_mid, g_sh_cfg_info.clear_interval_cpu_mid);
		break;
	case sh_type_cpu_big:
		need_clear = sh_check_need_clear(g_sh_cfg_info.clear_interval_enable.cpu_big,
				rtc_sec, g_sh_cfg_info.panic_time_cpu_big, g_sh_cfg_info.clear_interval_cpu_big);
		break;
	case sh_type_peri:
		need_clear = sh_check_need_clear(g_sh_cfg_info.clear_interval_enable.peri,
				rtc_sec, g_sh_cfg_info.panic_time_peri, g_sh_cfg_info.clear_interval_peri);
		break;
	default:
		break;
	}
	if (need_clear)
		pr_info("sh type %d need clear data\n", type);

	return need_clear;
}

void sh_diaginfo_record_cnt(enum sh_type type)
{
	pr_info("sh record cnt for type %u, cnt %u\n", type, g_sh_cfg_info.sw_cnt[type].count);
	switch (type) {
	case sh_type_cpu_lit:
		bbox_diaginfo_record(ACPU_LIT_SH_CNT, NULL,
				"acpu cluster lit sh cnt, ver %u, cnt %d, threshold %d, reason %s:%s",
				g_sh_cfg_info.version,
				g_sh_cfg_info.sw_cnt[type].count,
				g_sh_cfg_info.sw_cnt[type].threshold,
				rdr_get_reboot_reason(),
				rdr_get_exec_subtype());
		break;
	case sh_type_cpu_mid:
		bbox_diaginfo_record(ACPU_MID_SH_CNT, NULL,
				"acpu cluster mid sh cnt, ver %u, cnt %d, threshold %d, reason %s:%s",
				g_sh_cfg_info.version,
				g_sh_cfg_info.sw_cnt[type].count,
				g_sh_cfg_info.sw_cnt[type].threshold,
				rdr_get_reboot_reason(),
				rdr_get_exec_subtype());
		break;
	case sh_type_cpu_big:
		bbox_diaginfo_record(ACPU_BIG_SH_CNT, NULL,
				"acpu cluster big sh cnt, ver %u, cnt %d, threshold %d, reason %s:%s",
				g_sh_cfg_info.version,
				g_sh_cfg_info.sw_cnt[type].count,
				g_sh_cfg_info.sw_cnt[type].threshold,
				rdr_get_reboot_reason(),
				rdr_get_exec_subtype());
		break;

	case sh_type_peri:
		bbox_diaginfo_record(PERI_SH_CNT, NULL,
				"soc peri sh cnt, ver %u, cnt %d, threshold %d, reason %s:%s",
				g_sh_cfg_info.version,
				g_sh_cfg_info.sw_cnt[type].count,
				g_sh_cfg_info.sw_cnt[type].threshold,
				rdr_get_reboot_reason(),
				rdr_get_exec_subtype());
		break;
	default:
		break;
	}
}

void sh_diaginfo_record_pv(enum sh_type type)
{
	pr_info("sh record pv for type %u, cnt %u\n", type, g_sh_cfg_info.sw_cnt[type].count);
	switch (type) {
	case sh_type_cpu_lit:
		bbox_diaginfo_record(ACPU_LIT_SH_PV, NULL,
				"acpu cluster lit sh pv, ver %u, cnt %d, threshold %d",
				g_sh_cfg_info.version,
				g_sh_cfg_info.sw_cnt[type].count,
				g_sh_cfg_info.sw_cnt[type].threshold);
		break;
	case sh_type_cpu_mid:
		bbox_diaginfo_record(ACPU_MID_SH_PV, NULL,
				"acpu cluster mid sh pv, ver %u, cnt %d, threshold %d",
				g_sh_cfg_info.version,
				g_sh_cfg_info.sw_cnt[type].count,
				g_sh_cfg_info.sw_cnt[type].threshold);
		break;
	case sh_type_cpu_big:
		bbox_diaginfo_record(ACPU_BIG_SH_PV, NULL,
				"acpu cluster big sh pv, ver %u, cnt %d, threshold %d",
				g_sh_cfg_info.version,
				g_sh_cfg_info.sw_cnt[type].count,
				g_sh_cfg_info.sw_cnt[type].threshold);
		break;

	case sh_type_peri:
		bbox_diaginfo_record(PERI_SH_PV, NULL,
				"soc peri sh pv, ver %u, cnt %d, threshold %d",
				g_sh_cfg_info.version,
				g_sh_cfg_info.sw_cnt[type].count,
				g_sh_cfg_info.sw_cnt[type].threshold);
		break;
	default:
		break;
	}
}

#define VALID_SH_RTC_TIME	1700000000 /* 2023-11-15 06:13:20 */
static void do_sh(enum sh_type type, u32 rtc_sec)
{
	bool need_clear = false;

	pr_info("sh for type %d\n", type);

	if (!g_sh_cfg_info.sw_cnt[type].enable) {
		pr_err("sh type %d enable %u\n", type, g_sh_cfg_info.sw_cnt[type].enable);
		return;
	}

	if (is_sh_threshold_match(type)) {
		sh_diaginfo_record_pv(type);
		return;
	}

	if (rtc_sec <= VALID_SH_RTC_TIME) {
		pr_err("invalid rtc time %u\n", rtc_sec);
		return;
	}
	if (!sh_check_panic(type))
		return;

	g_sh_cfg_info.sw_cnt[type].count++;
	g_sh_need_update_nve = true;
	pr_info("sh type %d panic happened, cnt %u\n", type, g_sh_cfg_info.sw_cnt[type].count);
	if (g_sh_cfg_info.sw_cnt[type].threshold == 1) {
		sh_update_rtc(type, rtc_sec);
	} else if (g_sh_cfg_info.sw_cnt[type].threshold > 1) {
		need_clear = sh_need_clear(type, rtc_sec);
		sh_update_rtc(type, rtc_sec);
		if (need_clear) {
			pr_info("sh type %d need clear cnt to 1\n", type);
			g_sh_cfg_info.sw_cnt[type].count = 1;
		}
	}
	if (type == sh_type_peri && sh_peri_need_fast_pv()) {
			pr_info("peri ahb timeout happened, set sh peri cnt to threshold\n");
			g_sh_cfg_info.sw_cnt[type].count = g_sh_cfg_info.sw_cnt[type].threshold;
	}
	sh_diaginfo_record_cnt(type);
}

static int sh_get_nve_info(void)
{
	struct opt_nve_info_user *nve_sh_cfg_info = &g_nve_sh_cfg_info;
	int ret;

	nve_sh_cfg_info->nv_operation = NV_READ;
	ret = nve_direct_access_interface(nve_sh_cfg_info);
	if (ret < 0) {
		pr_err("[%s]read nve_sh_cfg_info error\n", __func__);
		return ret;
	}

	ret = memcpy_s(&g_sh_cfg_info, sizeof(g_sh_cfg_info), nve_sh_cfg_info->nv_data, sizeof(g_sh_cfg_info));
	if (ret) {
		pr_err("[%s]copy nve_sh_cfg_info data error, ret %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static void sh_update_nve_info(void)
{
	struct opt_nve_info_user *nve_sh_cfg_info = &g_nve_sh_cfg_info;
	int ret;

	ret = memcpy_s(nve_sh_cfg_info->nv_data, NVE_NV_DATA_SIZE, &g_sh_cfg_info, sizeof(g_sh_cfg_info));
	if (ret) {
		pr_err("[%s]copy nve_sh_cfg_info data error, ret %d\n", __func__, ret);
		return;
	}

	nve_sh_cfg_info->nv_operation = NV_WRITE;
	ret = nve_direct_access_interface(nve_sh_cfg_info);
	if (ret < 0) {
		pr_err("[%s]update nve_sh_cfg_info error\n", __func__);
		return;
	}

	return;
}

static int sh_thread_body(void *arg)
{
	enum sh_type type;
	struct timespec64 tv = {0};

	ssleep(30); /* wait for rtc and nve ready */

	ktime_get_real_ts64(&tv);
	if (sh_get_nve_info())
		return -EINVAL;
	pr_info("dfx sh process, rtc %lld\n", tv.tv_sec);
	for (type = sh_type_cpu_lit; type < sh_type_max; type++)
		do_sh(type, tv.tv_sec);

	if (g_sh_need_update_nve)
		sh_update_nve_info();

	return 0;
}

static int __init sh_init(void)
{
	g_sh_thread = kthread_run(sh_thread_body, NULL, "sh_thread");
	if (g_sh_thread == NULL) {
		pr_err("create sh thread failed\n");
		return -EINVAL;
	}

	return 0;
}

static void __exit sh_exit(void)
{
	if (g_sh_thread != NULL)
		kthread_stop(g_sh_thread);
}

late_initcall(sh_init);
module_exit(sh_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("DFX_SH MODULE");
