/*
 * hungtask_mod.c
 *
 * Detect Hung Task
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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
#if (KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE)
#include <platform/trace/hooks/hungtask.h>
#else
#include <platform/trace/events/hungtask.h>
#endif

#include "hungtask_base.h"
#include "hungtask_mmap_sem.h"

void hungtask_insmod(void);
void hungtask_rmmod(void);

static void hook_set_did_panic(void *ignore, int did_panic)
{
	htbase_set_panic(did_panic);
}

static void hook_set_timeout_secs(void *ignore, int ts)
{
	htbase_set_timeout_secs(ts);
}

static void hook_check_tasks(void *ignore, unsigned long ts)
{
	htbase_check_tasks(ts);
}

#ifdef CONFIG_DFX_HUNGTASK_MMAP_SEM_DBG
static void hook_mmap_sem_debug(void *ignore, const struct rw_semaphore *sem)
{
	htmmap_sem_debug(sem);
}
#endif

static int __init hungtask_init(void)
{
	if (htbase_create_sysfs())
		return -EFAULT;

	REGISTER_HCK_VNH(set_did_panic, hook_set_did_panic, NULL);
	REGISTER_HCK_VNH(set_timeout_secs, hook_set_timeout_secs, NULL);
	REGISTER_HCK_VRH(check_tasks, hook_check_tasks, NULL);

#ifdef CONFIG_DFX_HUNGTASK_MMAP_SEM_DBG
	REGISTER_HCK_VRH(mmap_sem_debug, hook_mmap_sem_debug, NULL);
#endif
	hungtask_insmod();

	return 0;
}
module_init(hungtask_init);

static void __exit hungtask_exit(void)
{
	hungtask_rmmod();
}
module_exit(hungtask_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Huang Yu");
MODULE_DESCRIPTION("DFX Hung Task Module");
