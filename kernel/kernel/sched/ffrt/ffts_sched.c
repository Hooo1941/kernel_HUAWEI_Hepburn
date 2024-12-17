// SPDX-License-Identifier: GPL-2.0
/*
 * ffrt worker monitor
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#include "platform_include/cee/linux/sched/ffrt/ffts_sched.h"
#include "linux/wait.h"
#include "linux/sched.h"
#include <linux/spinlock.h>
#include <linux/seq_file.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <securec.h>
#include <linux/slab.h>
#include <linux/compat.h>
#include <linux/miscdevice.h>
#include <linux/spinlock.h>
#include <linux/cpumask.h>
#include <linux/of.h>
#include <linux/pid.h>

#define ffts_sched_err(fmt, ...) pr_err("FFTS_SCHED "fmt"\n", ##__VA_ARGS__)
#define ffts_sched_info(fmt, ...) pr_info("FFTS_SCHED "fmt"\n", ##__VA_ARGS__)
#define ffts_sched_debug(fmt, ...) pr_debug("FFTS_SCHED "fmt"\n", ##__VA_ARGS__)

struct sleeped_worker {
	struct task_struct 	*worker;
	bool			wakeup_flag;
	spinlock_t		lock;
};

struct ffrt_proc {
	u16			ffts_uid;
	u16			ffts_tid; /* if use one sq */
	u8 			qos_num; /* sworkers num */
	u8 			rsd;
	struct sleeped_worker	*sworkers;
};

struct ffrt_thd {
	u16	ffts_tid;
	u8 	qos;
	u8 	rsd;
};

struct ffts_sched_callback *g_ffts_sched_cb;
static unsigned int *g_cpu_phy_map __read_mostly;

int ffts_sched_register_callback(struct ffts_sched_callback *cb)
{
	if (cb == NULL)
		return -EINVAL;

	g_ffts_sched_cb = cb;
	return 0;
}
EXPORT_SYMBOL(ffts_sched_register_callback);

int ffts_sched_proc_init(u16 ffts_uid, u16 ffts_tid, u8 qos_num)
{
	struct task_struct *proc = current->group_leader;
	int i;
	struct ffrt_proc *fp = NULL;

	if (proc->ffrt_proc) {
		ffts_sched_err("[%d], ffrt proc already inited\b", proc->pid);
		return -EEXIST;
	}

	fp = kzalloc(sizeof(struct ffrt_proc), GFP_KERNEL);
	if (!fp) {
		ffts_sched_err("ffrt_proc kzalloc failed\n");
		return -ENOMEM;
	}

	fp->ffts_uid = ffts_uid;
	fp->ffts_tid = ffts_tid;
	fp->qos_num = qos_num;
	fp->sworkers = kzalloc(qos_num * sizeof(struct sleeped_worker), GFP_KERNEL);
	if(!fp->sworkers) {
		ffts_sched_err("ffrt_proc sworkers kzalloc failed\n");
		kfree(fp);
		return -ENOMEM;
	}

	for (i = 0; i < qos_num; i++) {
		fp->sworkers[i].worker = NULL;
		fp->sworkers[i].wakeup_flag = false;
		spin_lock_init(&fp->sworkers[i].lock);
	}
	proc->ffrt_proc = fp;
	atomic_set(&proc->ffts_flag, 1);

	return 0;
}
EXPORT_SYMBOL(ffts_sched_proc_init);

void ffts_sched_proc_exit(pid_t pid)
{
	struct task_struct *proc = NULL;

	rcu_read_lock();
	proc = find_task_by_vpid(pid);
	if (!proc) {
		rcu_read_unlock();
		return;
	}

	get_task_struct(proc);
	rcu_read_unlock();

	atomic_set(&proc->ffts_flag, 0);

	if (proc->ffrt_proc == NULL) {
		put_task_struct(proc);
		return;
	}

	kfree(proc->ffrt_proc->sworkers);
	proc->ffrt_proc->sworkers = NULL;
	kfree(proc->ffrt_proc);
	proc->ffrt_proc = NULL;

	put_task_struct(proc);
}
EXPORT_SYMBOL(ffts_sched_proc_exit);

static int ffts_sched_thread_init(pid_t pid, int qos)
{
	struct task_struct *p;
	struct task_struct *proc;
	struct ffrt_proc *fp ;
	struct ffrt_thd *ft = NULL;
	unsigned int lpid;

	rcu_read_lock();
	p = find_task_by_vpid(pid);
	if (!p) {
		rcu_read_unlock();
		return -EINVAL;
	}

	get_task_struct(p);
	rcu_read_unlock();

	proc = p->group_leader;
	fp = proc->ffrt_proc;

	if (p->ffrt_thd) {
		put_task_struct(p);
		ffts_sched_err("[%d], ffrtr worker already inited\b", p->pid);
		return -EEXIST;
	}
	if (!fp) {
		put_task_struct(p);
		ffts_sched_err("proc[%d] is not ffrt proc\n", proc->pid);
		return -EINVAL;
	}

	if (qos < 0 || qos >= fp->qos_num) {
		put_task_struct(p);
		ffts_sched_err("invalid qos level[%d]\n", qos);
		return -EINVAL;
	}

	ft = kzalloc(sizeof(struct ffrt_thd), GFP_KERNEL);
	if (!ft) {
		put_task_struct(p);
		ffts_sched_err("ffrt_thd kzalloc failed\n");
		return -ENOMEM;
	}

	ft->qos = qos;
	p->ffrt_thd = ft;

	preempt_disable();
	lpid = g_cpu_phy_map[smp_processor_id()];
	g_ffts_sched_cb->thread_switch(lpid, fp->ffts_uid, fp->ffts_tid);
	atomic_set(&p->ffts_flag, 1);
	preempt_enable();

	put_task_struct(p);
	return 0;
}

void ffts_sched_thread_exit(struct task_struct *p)
{
	atomic_set(&p->ffts_flag, 0);

	if (!p->ffrt_thd)
		return;

	kfree(p->ffrt_thd);
	p->ffrt_thd = NULL;
}

void ffts_sched_context_switch(int cpu, struct task_struct *prev, struct task_struct *next)
{
	if (!g_ffts_sched_cb)
		return;

	if (atomic_read(&prev->ffts_flag) && prev->ffrt_thd)
		g_ffts_sched_cb->thread_switch(g_cpu_phy_map[cpu], 0, 0);

	if (atomic_read(&next->ffts_flag) && next->ffrt_thd)
		g_ffts_sched_cb->thread_switch(g_cpu_phy_map[cpu], next->group_leader->ffrt_proc->ffts_uid,
			next->group_leader->ffrt_proc->ffts_tid);
}

void ffts_sched_init(struct task_struct *p)
{
	p->ffrt_thd = NULL;
	p->ffrt_proc = NULL;
	atomic_set(&p->ffts_flag, 0);
}

void ffts_sched_release(struct task_struct *p)
{
	if(!atomic_read(&p->ffts_flag) || !g_ffts_sched_cb)
		return;

	ffts_sched_thread_exit(p);
}

void ffrt_worker_wait_interrupt(struct task_struct *p)
{
	int qos;
	struct ffrt_proc *fp = p->group_leader->ffrt_proc;
	struct ffrt_thd *ft = p->ffrt_thd;
	unsigned long flags;

	if (!ft || !fp) {
		ffts_sched_err("task[%d] is not ffrt proc or ffrt worker\n", p->pid);
		return;
	}

	qos = ft->qos;

	spin_lock_irqsave(&fp->sworkers[qos].lock, flags);
	set_current_state(TASK_INTERRUPTIBLE);
	for (;;) {
		if (fp->sworkers[qos].wakeup_flag || signal_pending_state(TASK_INTERRUPTIBLE, p)) {
			break;
		}
		fp->sworkers[qos].worker = current;
		spin_unlock_irqrestore(&fp->sworkers[qos].lock, flags);
		schedule();
		spin_lock_irqsave(&fp->sworkers[qos].lock, flags);
	}
	set_current_state(TASK_RUNNING);
	fp->sworkers[qos].worker = NULL;
	fp->sworkers[qos].wakeup_flag = false;
	spin_unlock_irqrestore(&fp->sworkers[qos].lock, flags);
}

void ffts_sched_worker_wakeup(int tid, int qos)
{
	struct task_struct *sleeped_worker = NULL;
	struct task_struct *proc = NULL;
	struct ffrt_proc *fp = NULL;
	unsigned long flags;

	rcu_read_lock();
	proc = find_task_by_pid_ns(tid, &init_pid_ns);
	if (!proc) {
		ffts_sched_err("proc[%d] not find \n", tid);
		rcu_read_unlock();
		return;
	}

	get_task_struct(proc);
	rcu_read_unlock();

	fp = proc->ffrt_proc;
	if (!fp) {
		ffts_sched_err("proc[%d] is not ffrt proc\n", tid);
		put_task_struct(proc);
		return;
	}

	if (qos < 0 || qos >= fp->qos_num) {
		ffts_sched_err("invalid qos level[%d]\n", qos);
		put_task_struct(proc);
		return;
	}

	spin_lock_irqsave(&fp->sworkers[qos].lock, flags);
	fp->sworkers[qos].wakeup_flag = true;
	sleeped_worker = fp->sworkers[qos].worker;
	if (!sleeped_worker) {
		spin_unlock_irqrestore(&fp->sworkers[qos].lock, flags);
		put_task_struct(proc);
		return;
	}

	wake_up_process(sleeped_worker);
	spin_unlock_irqrestore(&fp->sworkers[qos].lock, flags);
	put_task_struct(proc);
}
EXPORT_SYMBOL(ffts_sched_worker_wakeup);

static void ffrt_worker_wakeup_ioctl(int qos)
{
	struct task_struct *proc = current->group_leader;
	struct task_struct *sleeped_worker = NULL;
	struct ffrt_proc *fp = proc->ffrt_proc;
	unsigned long flags;

	if(!fp) {
		ffts_sched_err("proc[%d] is not ffrt proc\n", proc->pid);
		return;
	}

	if (qos < 0 || qos >= fp->qos_num)
		return;

	/* invoke when ffrt proc exit, so supposed not sleep when invoke this lock */
	spin_lock_irqsave(&fp->sworkers[qos].lock, flags);
	sleeped_worker = fp->sworkers[qos].worker;
	if (!sleeped_worker) {
		spin_unlock_irqrestore(&fp->sworkers[qos].lock, flags);
		return;
	}

	fp->sworkers[qos].wakeup_flag = true;
	wake_up_process(sleeped_worker);
	spin_unlock_irqrestore(&fp->sworkers[qos].lock, flags);
}

#define FFTS_WORKER_WAIT 	1
#define FFTS_WORKER_WAKEUP 	2
#define FFTS_PROC_INIT		3
#define	FFTS_WORKER_INIT	4
#define FFTS_SCHED_MAGIC 	0XBF

struct sched_worker_s {
	pid_t pid;
	int qos;
};

static long ffts_sched_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	unsigned int value;
	struct sched_worker_s worker;

	if (_IOC_TYPE(cmd) != FFTS_SCHED_MAGIC) {
		ffts_sched_err("Invalid Magic %d", _IOC_TYPE(cmd));
		return -EINVAL;
	}

	switch (_IOC_NR(cmd)) {
	case FFTS_WORKER_WAIT:
		ffrt_worker_wait_interrupt(current);
		break;
	case FFTS_WORKER_WAKEUP:
		copy_from_user(&value, arg, sizeof(value));
		ffrt_worker_wakeup_ioctl(value);
		break;
	case FFTS_WORKER_INIT:
		copy_from_user(&worker, arg, sizeof(struct sched_worker_s));
		ffts_sched_thread_init(worker.pid, worker.qos);
		break;
	default:
		break;
	}

	return 0;
}

#ifdef CONFIG_COMPAT
static long compat_ffts_sched_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return ffts_sched_ioctl(file, cmd, (unsigned long)(compat_ptr((compat_uptr_t)arg)));
}
#endif

static const struct file_operations ffts_sched_ops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = ffts_sched_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = compat_ffts_sched_ioctl,
#endif
};

static struct miscdevice ffts_dev = {
	.minor =	MISC_DYNAMIC_MINOR,
	.name =		"ffts_sched",
	.fops =		&ffts_sched_ops,
};

static int possiable_cpu_phy_map_init(void)
{
	struct device_node *cpu_np = NULL;
	int ret = 0;
	unsigned int lpid;
	int i = 0;
	int cpu;

	g_cpu_phy_map = kzalloc(num_possible_cpus() * sizeof(unsigned int), GFP_KERNEL);
	if (!g_cpu_phy_map) {
		ffts_sched_err("g_cpu_phy_map kzalloc failed\n");
		return -ENOMEM;
	}

	for_each_possible_cpu(cpu) {
		cpu_np = of_get_cpu_node(cpu, NULL);
		if (!cpu_np) {
			ffts_sched_err("%s: failed to get cpu%d node\n",
			       __func__, cpu);
			return -ENOENT;
		}
		ret = of_property_read_u32(cpu_np, "hw_cpu_no", &lpid);
		if (ret) {
			ffts_sched_err("%s: failed to get hw_cpu_no\n", __func__);
			of_node_put(cpu_np);
			break;
		}
		g_cpu_phy_map[i] = lpid;
		of_node_put(cpu_np);
		i++;
	}

	return ret;
}

static int __init ffts_sched_dev_init(void)
{
	int ret = 0;

	ret = possiable_cpu_phy_map_init();
	if (ret)
		return ret;

	return misc_register(&ffts_dev);
}
fs_initcall(ffts_sched_dev_init);

static void __exit ffts_sched_exit(void)
{
	misc_deregister(&ffts_dev);
	kfree(g_cpu_phy_map);
	g_cpu_phy_map = NULL;
}
module_exit(ffts_sched_exit);

MODULE_DESCRIPTION("ffrt");
MODULE_LICENSE("GPL v2");
