/*
 * hwcfs_fork.c
 *
 * hwcfs fork interface implementation
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifdef CONFIG_HW_VIP_THREAD
#include <chipset_common/hwcfs/hwcfs_common.h>
#include <chipset_common/hwcfs/hwcfs_fork.h>

void init_task_vip_info(struct task_struct *tsk)
{
	if (!tsk)
		return;
	tsk->static_vip = 0;
	atomic64_set(&(tsk->dynamic_vip), 0);
	INIT_LIST_HEAD(&tsk->vip_entry);
	tsk->vip_depth = 0;
	tsk->enqueue_time = 0;
	tsk->dynamic_vip_start = 0;
}

void inherit_vip_property(struct task_struct *task)
{
	if ((!task) || (!(current->static_vip & FORK_INHERIT_VIP_STATE)))
		return;
	task->static_vip = current->static_vip;
}
#endif
