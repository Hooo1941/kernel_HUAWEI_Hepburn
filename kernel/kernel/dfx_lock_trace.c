/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Provide log to find lock's owner.
 * Author: Zhang Kuangqi <zhangkuangqi1@huawei.com>
 * Create: 2022-04-19
 */
#include <linux/cpu.h>
#include <linux/cpumask.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/sched/debug.h>
#include <linux/sched/task.h>
#include <linux/spinlock_types.h>
#include <linux/sysrq.h>
#include <log/log_usertype.h>

#ifdef CONFIG_DFX_FIQ
#include <platform_include/basicplatform/linux/dfx_fiq.h>
#endif

#include "sched/sched.h"

#define TIMER_PERIOD 5
#define DECIMAL 10
#define LOCK_BUF_MAX 30
#define INVALID_LOCK_INDEX ARRAY_SIZE(g_locks)

enum LOCK_TYPE {
	MUTEX,
#ifdef CONFIG_DEBUG_SPINLOCK
	SPIN_LOCK,
#endif
	TYPE_MAX
};

#ifdef CONFIG_QCOM_DFX_LOCK_TRACE
static inline raw_spinlock_t *get_rq_lock(struct rq *rq)
{
	return &(rq->lock);
}
#else
static inline raw_spinlock_t *get_rq_lock(struct rq *rq)
{
	return &(rq->__lock);
}
#endif

extern void *get_rtnl_mutex(void);

static unsigned int g_lock_index;

struct lock_operations {
	void (*lock)(void *lock);
	void (*unlock)(void *lock);
	int (*trylock)(void *lock);
	struct task_struct *(*get_owner)(void *lock);
};

struct lock_info {
	const char *name;
	void *(*get_lock)(void);
	enum LOCK_TYPE type;
};

static void _mutex_lock(void *lock)
{
	mutex_lock(lock);
}

static void _mutex_unlock(void *lock)
{
	mutex_unlock(lock);
}

static int _mutex_trylock(void *lock)
{
	return mutex_trylock(lock);
}

static struct task_struct *_mutex_owner(void *lock)
{
	return mutex_owner(lock);
}

struct lock_operations mutex_ops = {
	.lock = _mutex_lock,
	.unlock = _mutex_unlock,
	.trylock = _mutex_trylock,
	.get_owner = _mutex_owner,
};

#ifdef CONFIG_DEBUG_SPINLOCK
void dump_rq_lock_owner(void)
{
	unsigned int i;
	unsigned int owner_cpu;
	struct rq *rq = NULL;
	struct task_struct *curr = NULL;
	struct task_struct *owner = NULL;

	rcu_read_lock();
	for_each_online_cpu(i) {
		rq = cpu_rq(i);
		curr = rq->curr;
		if (curr) {
			pr_err("cpu%d curr info: comm %s pid %d prio %d\n",
				i, curr->comm, curr->pid, curr->prio);
			show_stack(curr, NULL, KERN_ERR);
		}

		owner = (struct task_struct *)(get_rq_lock(rq)->owner);
		owner_cpu = get_rq_lock(rq)->owner_cpu;
		if (owner && owner != SPINLOCK_OWNER_INIT) {
			pr_err("cpu%d rq_lock is holding by name=%s,PID=%d owner_cpu: %d\n", i,
				owner->comm, owner->pid, owner_cpu);
			show_stack(owner, NULL, KERN_ERR);
		}
	}
	rcu_read_unlock();
}

struct task_struct *spinlock_owner(struct spinlock *lock)
{
	struct task_struct *owner = NULL;

	owner = READ_ONCE(lock->rlock.owner);
	return (owner != SPINLOCK_OWNER_INIT ? owner : NULL);
}

static void _spin_lock(void *lock)
{
	spin_lock(lock);
}

static void _spin_unlock(void *lock)
{
	spin_unlock(lock);
}

static int _spin_trylock(void *lock)
{
	return spin_trylock(lock);
}

static struct task_struct *_spinlock_owner(void *lock)
{
	return spinlock_owner(lock);
}

struct lock_operations spinlock_ops = {
	.lock = _spin_lock,
	.unlock = _spin_unlock,
	.trylock = _spin_trylock,
	.get_owner = _spinlock_owner,
};
#endif

static struct lock_operations *g_ops[TYPE_MAX] = {
	&mutex_ops,
#ifdef CONFIG_DEBUG_SPINLOCK
	&spinlock_ops
#endif
};

static struct lock_info g_locks[] = {
	{"rtnl_mutex", get_rtnl_mutex, MUTEX},
};

static inline void *get_lock(unsigned int index)
{
	return g_locks[index].get_lock();
}

static inline void lock(unsigned int index)
{
	g_ops[g_locks[index].type]->lock(get_lock(index));
}

static inline void unlock(unsigned int index)
{
	g_ops[g_locks[index].type]->unlock(get_lock(index));
}

static inline int trylock(unsigned int index)
{
	return g_ops[g_locks[index].type]->trylock(get_lock(index));
}

static inline struct task_struct *get_owner(unsigned int index)
{
	if (index >= ARRAY_SIZE(g_locks) || g_locks[index].type >= ARRAY_SIZE(g_ops))
		return NULL;

	return g_ops[g_locks[index].type]->get_owner(get_lock(index));
}

static inline const char *get_name(unsigned int index)
{
	return g_locks[index].name;
}

static void do_lock_trace_dump(struct timer_list *timer)
{
	struct task_struct *owner = NULL;

	if (g_lock_index >= ARRAY_SIZE(g_locks))
		return;

	rcu_read_lock();
	owner = get_owner(g_lock_index);
	if (!owner) {
		rcu_read_unlock();
		return;
	}

	pr_err("lock is holding by comm=%s, PID=%d\n", owner->comm, owner->pid);
	sched_show_task(owner);
	rcu_read_unlock();
	__handle_sysrq('l', false);
}

void dump_lock_trace(struct timer_list *timer)
{
	do_lock_trace_dump(timer);
	mod_timer(timer, (jiffies + TIMER_PERIOD * HZ));
}

struct timer_list lock_timer;
DEFINE_TIMER(lock_timer, dump_lock_trace);

static void check_lock_state(unsigned int index)
{
	if (trylock(index)) {
		unlock(index);
		return;
	}

	if (!get_owner(index))
		return;

	g_lock_index = index;
	lock(index);
	unlock(index);
	g_lock_index = INVALID_LOCK_INDEX;
}

static bool is_beta_user(void)
{
	int i;
	unsigned int beta_flag = BETA_USER;

#ifdef CONFIG_LOG_EXCEPTION
	/* Try 60 times, wait 120s at most */
	for (i = 0; i < 60; i++) {
		beta_flag = get_logusertype_flag();
		if (beta_flag != 0)
			break;
		ssleep(2);
	}
#endif
	return (beta_flag == BETA_USER);
}

static int start_lock_trace(void *info)
{
	unsigned int i;
	unsigned int size;

	if (!is_beta_user()) {
		pr_err("non-beta user\n");
		return 0;
	}

	mod_timer(&lock_timer, (jiffies + TIMER_PERIOD * HZ));
	size = ARRAY_SIZE(g_locks);
	while (1) {
		for (i = 0; i < size; i++)
			check_lock_state(i);

		ssleep(30);
	}
	return 0;
}

#if defined CONFIG_DFX_LOCK_TRACE_DEBUG || defined CONFIG_QCOM_DFX_LOCK_TRACE_DEBUG
static ssize_t lock_trace_write(struct file *file,
	const char __user *buf, size_t size, loff_t *ppos)
{
	int i;
	int ret;
	unsigned int index;
	char val[LOCK_BUF_MAX] = {0};

	ret = simple_write_to_buffer(val, sizeof(val) - 1, ppos, buf, size);
	if (ret < 0)
		return size;

	ret = kstrtouint(val, DECIMAL, &index);
	if ((ret < 0) || (index >= ARRAY_SIZE(g_locks))) {
		pr_err("%s: Invalid input (%d)\n", __func__, ret);
		return size;
	}

	lock(index);
	i = 0;
	while (1) {
		mdelay(1000);
		i++;
		pr_err("%s : %s locked for %d seconds\n", __func__, get_name(index), i);
	}
	return size;
}

static int lock_trace_show(struct seq_file *s, void *unused)
{
	int i;

	seq_printf(s, "%s\n", "set index to start lock_debug");
	seq_printf(s, "%-5s %-20s\n", "index", "name");
	for (i = 0; i < ARRAY_SIZE(g_locks); i++)
		seq_printf(s, "%-5d %-20s\n", i, get_name(i));

	return 0;
}

static int lock_trace_open(struct inode *inode, struct file *file)
{
	return single_open(file, lock_trace_show, NULL);
}

static const struct file_operations lock_trace_proc_fops = {
	.open		= lock_trace_open,
	.read		= seq_read,
	.write		= lock_trace_write,
	.llseek		= seq_lseek,
	.release	= single_release,
};
#endif

static int __init lock_trace_init(void)
{
	struct dentry *debug_file = NULL;
	struct task_struct *thread = NULL;

#if defined CONFIG_DFX_LOCK_TRACE_DEBUG || defined CONFIG_QCOM_DFX_LOCK_TRACE_DEBUG
	debug_file = debugfs_create_file("lock_trace_debug", 0600, NULL, NULL,
		&lock_trace_proc_fops);
	if (IS_ERR_OR_NULL(debug_file)) {
		pr_err("%s: failed to create %s\n", __func__, "lock_trace_debug");
		return -EIO;
	}
#endif

	thread = kthread_run(start_lock_trace, NULL, "klocktraced");
	if (IS_ERR_OR_NULL(thread))
		pr_err("%s: create thread fail!\n", __func__);

	return 0;
}

module_init(lock_trace_init);
