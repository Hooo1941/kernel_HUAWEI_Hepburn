/*
 * hwcfs_rwsem.c
 *
 * rwsem schedule implementation
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
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
/*lint -save -e578 -e695 -e571*/
#include <chipset_common/hwcfs/hwcfs_rwsem.h>

#include <linux/list.h>
#include <chipset_common/hwcfs/hwcfs_common.h>

#define RWSEM_READER_OWNED ((struct task_struct *)1UL)
#define RWSEM_OWNER_BOOST       (1UL << 6)

static inline bool rwsem_owner_is_writer(struct task_struct *owner)
{
	return owner && owner != RWSEM_READER_OWNED;
}

static void rwsem_list_add_vip(struct list_head *entry, struct list_head *head)
{
	struct list_head *pos = NULL;
	struct list_head *num = NULL;
	struct rwsem_waiter *waiter = NULL;

	list_for_each_safe(pos, num, head) {
		waiter = list_entry(pos, struct rwsem_waiter, list);
		if (!test_task_vip(waiter->task)) {
			list_add(entry, waiter->list.prev);
			return;
		}
	}
	if (pos == head)
		list_add_tail(entry, head);
}

void rwsem_list_add(struct task_struct *task,
	struct list_head *entry, struct list_head *head)
{
	bool is_vip = test_set_dynamic_vip(task);

	if (!entry || !head)
		return;

	if (is_vip)
		rwsem_list_add_vip(entry, head);
	else
		list_add_tail(entry, head);
}

static bool is_rwsem_owner_boost(struct rw_semaphore *sem)
{
	return atomic_long_read(&sem->count) & RWSEM_OWNER_BOOST;
}

static void rwsem_owner_set_boost(struct rw_semaphore *sem)
{
	unsigned long count;

	count = atomic_long_read(&sem->count);
	do {
		if (count & RWSEM_OWNER_BOOST)
			break;
	} while (!atomic_long_try_cmpxchg(&sem->count, &count,
		count | RWSEM_OWNER_BOOST));
}

static void rwsem_owner_clear_boost(struct rw_semaphore *sem)
{
	unsigned long count;

	count = atomic_long_read(&sem->count);
	do {
		if (!(count & RWSEM_OWNER_BOOST))
			break;
	} while (!atomic_long_try_cmpxchg(&sem->count, &count,
		count & ~RWSEM_OWNER_BOOST));
}

void rwsem_dynamic_vip_enqueue(
	struct task_struct *task, struct task_struct *waiter_task,
	struct task_struct *owner, struct rw_semaphore *sem)
{
	if (!waiter_task || !task || !sem || !rwsem_owner_is_writer(owner))
		return;

	if (is_rwsem_owner_boost(sem))
		return;

	if (test_task_vip(owner))
		return;
	if (!test_set_dynamic_vip(task)) {
#ifdef CONFIG_HW_VIP_SEMAPHORE
		if (is_rwsem_vip(sem) && owner->prio >= DEFAULT_PRIO && task->group_leader) {
			if (!task->group_leader->static_vip)
				return;
			task = task->group_leader;
		} else {
			return;
		}
#else
		return;
#endif
	}
	dynamic_vip_enqueue(owner, DYNAMIC_VIP_RWSEM, task->vip_depth);
	rwsem_owner_set_boost(sem);
}

void rwsem_dynamic_vip_dequeue(struct rw_semaphore *sem,
	struct task_struct *tsk)
{
	if (tsk && sem && is_rwsem_owner_boost(sem)) {
		dynamic_vip_dequeue(tsk, DYNAMIC_VIP_RWSEM);
		rwsem_owner_clear_boost(sem);
	}
}

void clear_rwsem_boost_tag(struct rw_semaphore *sem)
{
	if (sem && is_rwsem_owner_boost(sem))
		rwsem_owner_clear_boost(sem);
}

#ifdef CONFIG_RWSEM_PRIO_OPT
static bool test_owner_trans(struct rw_semaphore *sem, int vip_bit)
{
	return test_bit(vip_bit, sem->reader_info->vip_bitmap) ? true : false;
}

void rwsem_read_dynamic_vip_enqueue(
	struct task_struct *tsk, struct task_struct *waiter_task,
	struct task_struct *owner, struct rw_semaphore *sem, int vip_bit)
{
	if (!waiter_task || !tsk || !sem || test_owner_trans(sem, vip_bit))
		return;
	if (test_task_vip(owner))
		return;

	if (!test_set_dynamic_vip(tsk)) {
#ifdef CONFIG_HW_VIP_SEMAPHORE
		if (is_rwsem_vip(sem) && owner->prio >= DEFAULT_PRIO && tsk->group_leader) {
			if (!tsk->group_leader->static_vip)
				return;
			tsk = tsk->group_leader;
		} else {
			return;
		}
#else
		return;
#endif
	}
	dynamic_vip_enqueue(owner, DYNAMIC_VIP_RWSEM, tsk->vip_depth);
	__set_bit(vip_bit, sem->reader_info->vip_bitmap);
}

void rwsem_read_dynamic_vip_dequeue(struct rw_semaphore *sem,
	struct task_struct *tsk, int vip_bit)
{
	if (tsk && sem && test_owner_trans(sem, vip_bit)) {
		dynamic_vip_dequeue(tsk, DYNAMIC_VIP_RWSEM);
		__clear_bit(vip_bit, sem->reader_info->vip_bitmap);
	}
}

#endif

/*lint -restore*/
#endif

