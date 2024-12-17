/*
 * memcheck_mod.c
 *
 * memory leak detect
 *
 * Copyright (c) 2021-2022 Huawei Technologies Co., Ltd.
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

#include <linux/module.h>
#include <linux/version.h>
#if (KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE)
#include <platform/trace/hooks/memcheck.h>
#else
#include <platform/trace/events/memcheck.h>
#endif
#include "memcheck_account.h"
#include "memcheck_log_cma.h"
#ifdef CONFIG_HISI_GPU
#include "include/hvgr_mem_profile.h"
#elif (defined(CONFIG_DFX_OHOS) && defined(CONFIG_MALI_MIDGARD))
#include <platform_include/maligpu/linux/mali_kbase_mem_profile.h>
#endif

#if defined(CONFIG_HISI_GPU) || (defined(CONFIG_DFX_OHOS) && defined(CONFIG_MALI_MIDGARD))
static void hook_gpu_get_total_used(void *ignore, unsigned long *total)
{
#ifdef CONFIG_HISI_GPU
	*total = hvgr_mem_statics_used_pages();
#else
	*total = kbase_mem_statics_used_pages();
#endif
}
#endif

static void hook_mm_mem_stats_show(void *ignore, int unused)
{
	memcheck_stats_show();
}

static void hook_cma_report(void *ignore, char *name, unsigned long total,
			    unsigned long req)
{
	memcheck_cma_report(name, total, req);
}

static void hook_slub_obj_report(void *ignore, struct kmem_cache *s)
{
	memcheck_slub_obj_report(s);
}

static void hook_lowmem_report(void *ignore, struct task_struct *p,
				 unsigned long points)
{
	memcheck_lowmem_report(p, points);
}

static int __init dfx_memcheck_init(void)
{
	memcheck_createfs();
#ifdef CONFIG_HISI_GPU
	REGISTER_HCK_VNH(gpu_get_total_used, hook_gpu_get_total_used, NULL);
#elif defined(CONFIG_DFX_OHOS) && defined(CONFIG_MALI_MIDGARD)
	REGISTER_HCK_VRH(gpu_get_total_used, hook_gpu_get_total_used, NULL);
#endif
	REGISTER_HCK_VRH(mm_mem_stats_show, hook_mm_mem_stats_show, NULL);
	REGISTER_HCK_VRH(cma_report, hook_cma_report, NULL);
	REGISTER_HCK_VRH(slub_obj_report, hook_slub_obj_report, NULL);
	REGISTER_HCK_VRH(lowmem_report, hook_lowmem_report, NULL);

	return 0;
}
module_init(dfx_memcheck_init);

static void __exit dfx_memcheck_exit(void)
{
}
module_exit(dfx_memcheck_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_DESCRIPTION("DFX Memcheck Module");
