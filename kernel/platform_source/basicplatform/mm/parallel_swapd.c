/*
 * parallel_swapd.c
 *
 * Copyright (C) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/cpumask.h>
#include <linux/module.h>
#include <linux/mm/parallel_swapd.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/version.h>
#include <linux/workqueue.h>
#ifndef CONFIG_MM_LTS_KERNEL
#include <linux/hp/memcg_policy.h>
#endif
#include <asm/arch_timer.h>
#include <asm/uaccess.h>

#if defined(CONFIG_MM_LTS_KERNEL) && defined(CONFIG_PARA_SWAPD)
#include <linux/memcg_policy.h>
#endif

static int parak_enable = 1;

int get_parak_enable(void)
{
	return parak_enable;
}

static int parad_enable = 1;

int get_parad_enable(void)
{
	return parad_enable;
}

#ifdef CONFIG_DFX_DEBUG_FS
u64 parad_get_counter(void)
{
	return arch_timer_read_counter();
}

static u64 zswapd_cnt;
static u64 zswapd_work[NR_CPUS];
static u64 zswapd_nr_scanned;
static u64 zswapd_nr_reclaimed;
static atomic_t zswapd_wakeup = ATOMIC_INIT(0);
static atomic_t zswapd_realwake = ATOMIC_INIT(0);
static atomic_t zswapd_suitable = ATOMIC_INIT(0);
static atomic_t zswapd_interval = ATOMIC_INIT(0);
static atomic_t zswapd_refault = ATOMIC_INIT(0);
static atomic_t zswapd_shrink_anon = ATOMIC_INIT(0);
static u64 kswapd_cnt;
static u64 kswapd_nr_scanned;
static u64 kswapd_nr_reclaimed;
static u64 kswapd_work[NR_CPUS];

void parad_stat_add(enum parad_state state, u64 counter)
{
	switch (state) {
	case PARAD_ZSWAPD:
		zswapd_cnt += counter;
		break;
	case PARAD_ZSWAPD_WORK:
		zswapd_work[smp_processor_id()] += counter;
		break;
	case PARAD_ZSWAPD_NR_SCANNED:
		zswapd_nr_scanned += counter;
		break;
	case PARAD_ZSWAPD_NR_RECLAIMED:
		zswapd_nr_reclaimed += counter;
		break;
	case PARAD_ZSWAPD_WAKEUP:
		atomic_add(counter, &zswapd_wakeup);
		break;
	case PARAD_ZSWAPD_REALWAKE:
		atomic_add(counter, &zswapd_realwake);
		break;
	case PARAD_ZSWAPD_SUITABLE:
		atomic_add(counter, &zswapd_suitable);
		break;
	case PARAD_ZSWAPD_INTERVAL:
		atomic_add(counter, &zswapd_interval);
		break;
	case PARAD_ZSWAPD_REFAULT:
		atomic_add(counter, &zswapd_refault);
		break;
	case PARAD_ZSWAPD_SHRINK_ANON:
		atomic_add(counter, &zswapd_shrink_anon);
		break;
	case PARAD_KSWAPD:
		kswapd_cnt += counter;
		break;
	case PARAD_KSWAPD_NR_SCANNED:
		kswapd_nr_scanned += counter;
		break;
	case PARAD_KSWAPD_NR_RECLAIMED:
		kswapd_nr_reclaimed += counter;
		break;
	case PARAD_KSWAPD_WORK:
		kswapd_work[smp_processor_id()] += counter;
		break;
	}
}

static int stat_show(struct seq_file *seq, void *v)
{
	int cpu;

	seq_printf(seq, "zswapd:\t\t %16ld\n", zswapd_cnt);
	seq_printf(seq, "nr_scanned:\t %16ld\n", zswapd_nr_scanned);
	seq_printf(seq, "nr_reclaimed:\t %16ld\n", zswapd_nr_reclaimed);
	for_each_online_cpu(cpu) {
		seq_printf(seq, "cpu[%d]:\t\t %16ld\n", cpu,
			   zswapd_work[cpu]);
	}
	seq_printf(seq, "zswapd_wakeup:\t %16ld\n",
		   atomic_read(&zswapd_wakeup));
	seq_printf(seq, "zswapd_realwake:\t %16ld\n",
		   atomic_read(&zswapd_realwake));
	seq_printf(seq, "zswapd_suitable:\t %16ld\n",
		   atomic_read(&zswapd_suitable));
	seq_printf(seq, "zswapd_interval:\t %16ld\n",
		   atomic_read(&zswapd_interval));
	seq_printf(seq, "zswapd_refault:\t %16ld\n",
		   atomic_read(&zswapd_refault));
	seq_printf(seq, "zswapd_shrink_anon:\t %16ld\n",
		   atomic_read(&zswapd_shrink_anon));
	seq_printf(seq, "kswapd:\t\t %16ld\n", kswapd_cnt);
	seq_printf(seq, "nr_scanned:\t %16ld\n", kswapd_nr_scanned);
	seq_printf(seq, "nr_reclaimed:\t %16ld\n", kswapd_nr_reclaimed);
	for_each_online_cpu(cpu)
		seq_printf(seq, "cpu[%d]:\t\t %16ld\n", cpu, kswapd_work[cpu]);

	return 0;
}

static ssize_t stat_write(struct file *file, const char __user *buf,
			  size_t count, loff_t *ppos)
{
	int cpu;

	zswapd_cnt = 0;
	zswapd_nr_scanned = 0;
	zswapd_nr_reclaimed = 0;

	for_each_online_cpu(cpu)
		zswapd_work[cpu] = 0;
	atomic_set(&zswapd_wakeup, 0);
	atomic_set(&zswapd_realwake, 0);
	atomic_set(&zswapd_suitable, 0);
	atomic_set(&zswapd_interval, 0);
	atomic_set(&zswapd_refault, 0);
	atomic_set(&zswapd_shrink_anon, 0);

	kswapd_cnt = 0;
	kswapd_nr_scanned = 0;
	kswapd_nr_reclaimed = 0;
	for_each_online_cpu(cpu)
		kswapd_work[cpu] = 0;

	return count;
}

static int stat_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, stat_show, NULL);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
static const struct file_operations stat_ops = {
	.open = stat_open,
	.read = seq_read,
	.write = stat_write,
};
#else
static const struct proc_ops stat_ops = {
	.proc_open = stat_open,
	.proc_read = seq_read,
	.proc_write = stat_write,
};
#endif

static int enable_show(struct seq_file *seq, void *v)
{
	seq_printf(seq, "parad enabled: %d\n", parad_enable);
	return 0;
}

static ssize_t enable_write(struct file *file, const char __user *buffer,
			    size_t count, loff_t *pos)
{
	char enable;

	if (count > 0) {
		if (get_user(enable, buffer))
			return -EFAULT;

		if (enable == '0')
			parad_enable = 0;
		else if (enable == '1')
			parad_enable = 1;
		else
			return -EINVAL;
	}

	return count;
}

static int enable_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, enable_show, NULL);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
static const struct file_operations enable_ops = {
	.open = enable_open,
	.read = seq_read,
	.write = enable_write,
};
#else
static const struct proc_ops enable_ops = {
	.proc_open = enable_open,
	.proc_read = seq_read,
	.proc_write = enable_write,
};
#endif

static int parak_enable_show(struct seq_file *seq, void *v)
{
	seq_printf(seq, "parak_enable %d\n", parak_enable);
	return 0;
}

static ssize_t parak_enable_write(struct file *file, const char __user *buffer,
			    size_t count, loff_t *pos)
{
	char enable;

	if (count > 0) {
		if (get_user(enable, buffer))
			return -EFAULT;

		if (enable == '0')
			parak_enable = 0;
		else if (enable == '1')
			parak_enable = 1;
		else
			return -EINVAL;
	}

	return count;
}

static int parak_enable_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, parak_enable_show, NULL);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
static const struct file_operations parak_enable_ops = {
	.open = parak_enable_open,
	.read = seq_read,
	.write = parak_enable_write,
};
#else
static const struct proc_ops parak_enable_ops = {
	.proc_open = parak_enable_open,
	.proc_read = seq_read,
	.proc_write = parak_enable_write,
};
#endif

static int parad_proc_create(void)
{
	static struct proc_dir_entry *root;

	root = proc_mkdir("parad", NULL);

	proc_create("parak_enable", 0660, root, &parak_enable_ops);
	proc_create("enable", 0660, root, &enable_ops);
	proc_create("stat", 0440, root, &stat_ops);

	return 0;
}
#endif

static int mm_parad_init(void)
{
	parak_task_create();

	parad_task_create();

#ifdef CONFIG_DFX_DEBUG_FS
	parad_proc_create();
#endif

	return 0;
}
module_init(mm_parad_init);
