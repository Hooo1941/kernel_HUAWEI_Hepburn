/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: for tzdriver debug
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include "tzdebug.h"
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include <stdarg.h>
#include <linux/mm.h>
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#include <securec.h>
#include <asm/io.h>
#include "tc_ns_log.h"
#include "tc_ns_client.h"
#include "tc_client_driver.h"
#include "teek_ns_client.h"
#include "smc_smp.h"
#include "teek_client_constants.h"
#include "mailbox_mempool.h"
#include "tlogger.h"
#include "cmdmonitor.h"
#include "session_manager.h"
#include "internal_functions.h"

#define DEBUG_OPT_LEN 128

#ifdef CONFIG_TA_MEM_INUSE_ONLY
#define TA_MEMSTAT_ALL 0
#else
#define TA_MEMSTAT_ALL 1
#endif

static struct dentry *g_tz_dbg_dentry;

typedef void (*tzdebug_opt_func)(const char *param);

struct opt_ops {
	char *name;
	tzdebug_opt_func func;
};

static DEFINE_MUTEX(g_meminfo_lock);
static struct tee_mem g_tee_meminfo;
static void tzmemdump(const char *param);
static int send_dump_mem(int flag, int history, const struct tee_mem *statmem)
{
	struct tc_ns_smc_cmd smc_cmd = { {0}, 0 };
	struct mb_cmd_pack *mb_pack = NULL;
	int ret = 0;

	if (!statmem) {
		tloge("statmem is NULL\n");
		return -EINVAL;
	}
	mb_pack = mailbox_alloc_cmd_pack();
	if (!mb_pack)
		return -ENOMEM;

	smc_cmd.cmd_id = GLOBAL_CMD_ID_DUMP_MEMINFO;
	smc_cmd.cmd_type = CMD_TYPE_GLOBAL;
	mb_pack->operation.paramtypes = teec_param_types(
		TEE_PARAM_TYPE_MEMREF_INOUT, TEE_PARAM_TYPE_VALUE_INPUT,
		TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);
	mb_pack->operation.params[0].memref.buffer = (unsigned int)mailbox_virt_to_phys((uintptr_t)statmem);
	mb_pack->operation.params[0].memref.size = sizeof(*statmem);
	mb_pack->operation.buffer_h_addr[0] =
		(unsigned int)((uint64_t)mailbox_virt_to_phys((uintptr_t)statmem) >> ADDR_TRANS_NUM);
	mb_pack->operation.params[1].value.a = (unsigned int)flag;
	mb_pack->operation.params[1].value.b = (unsigned int)history;
	smc_cmd.operation_phys =
		(unsigned int)mailbox_virt_to_phys((uintptr_t)&mb_pack->operation);
	smc_cmd.operation_h_phys =
		(unsigned int)((uint64_t)mailbox_virt_to_phys((uintptr_t)&mb_pack->operation) >> ADDR_TRANS_NUM);

	livepatch_down_read_sem();
	if (tc_ns_smc(&smc_cmd) != 0) {
		ret = -EPERM;
		tloge("send dump mem failed\n");
	}
	livepatch_up_read_sem();

	tz_log_write();
	mailbox_free(mb_pack);
	return ret;
}

void tee_dump_mem(void)
{
	tzmemdump(NULL);
	if (tlogger_store_msg(CONFIG_TEE_LOG_ACHIVE_PATH,
		sizeof(CONFIG_TEE_LOG_ACHIVE_PATH)) < 0)
		tloge("[cmd_monitor_tick]tlogger store lastmsg failed\n");
}

/* get meminfo (tee_mem + N * ta_mem < 4Kbyte) from tee */
static int get_tee_meminfo_cmd(void)
{
	int ret;
	struct tee_mem *mem = NULL;

	mem = mailbox_alloc(sizeof(*mem), MB_FLAG_ZERO);
	if (!mem)
		return -ENOMEM;

	ret = send_dump_mem(0, TA_MEMSTAT_ALL, mem);
	if (ret != 0) {
		tloge("send dump failed\n");
		mailbox_free(mem);
		return ret;
	}

	mutex_lock(&g_meminfo_lock);
	ret = memcpy_s(&g_tee_meminfo, sizeof(g_tee_meminfo), mem, sizeof(*mem));
	if (ret != 0)
		tloge("memcpy failed\n");
	mutex_unlock(&g_meminfo_lock);
	mailbox_free(mem);

	return ret;
}

static atomic_t g_cmd_send = ATOMIC_INIT(1);

void set_cmd_send_state(void)
{
	atomic_set(&g_cmd_send, 1);
}

int get_tee_meminfo(struct tee_mem *meminfo)
{
	errno_t s_ret;

	if (!get_tz_init_flag()) return EFAULT;
	if (!meminfo)
		return -EINVAL;

	if (atomic_read(&g_cmd_send) != 0) {
		if (get_tee_meminfo_cmd() != 0)
			return -EFAULT;
	} else {
		atomic_set(&g_cmd_send, 0);
	}

	mutex_lock(&g_meminfo_lock);
	s_ret = memcpy_s(meminfo, sizeof(*meminfo),
		&g_tee_meminfo, sizeof(g_tee_meminfo));
	mutex_unlock(&g_meminfo_lock);
	if (s_ret != 0)
		return -1;

	return 0;
}
EXPORT_SYMBOL(get_tee_meminfo);

static void tzdump(const char *param)
{
	(void)param;
	show_cmd_bitmap();
	wakeup_tc_siq(SIQ_DUMP_SHELL);
}

static void tzmemdump(const char *param)
{
	struct tee_mem *mem = NULL;

	(void)param;
	mem = mailbox_alloc(sizeof(*mem), MB_FLAG_ZERO);
	if (!mem) {
		tloge("mailbox alloc failed\n");
		return;
	}

	if (send_dump_mem(1, 1, mem) != 0)
		tloge("send dump mem failed\n");
	mailbox_free(mem);
}

static struct opt_ops g_opt_arr[] = {
	{"dump", tzdump},
	{"memdump", tzmemdump},
	{"dump_service", dump_services_status},
};

static ssize_t tz_dbg_opt_read(struct file *filp, char __user *ubuf,
	size_t cnt, loff_t *ppos)
{
	char *obuf = NULL;
	char *p = NULL;
	ssize_t ret;
	uint32_t oboff = 0;
	uint32_t i;

	(void)(filp);

	obuf = kzalloc(DEBUG_OPT_LEN, GFP_KERNEL);
	if (!obuf)
		return -ENOMEM;
	p = obuf;

	for (i = 0; i < ARRAY_SIZE(g_opt_arr); i++) {
		int len = snprintf_s(p, DEBUG_OPT_LEN - oboff, DEBUG_OPT_LEN - oboff - 1,
			"%s ", g_opt_arr[i].name);
		if (len < 0) {
			kfree(obuf);
			tloge("snprintf opt name of idx %d failed\n", i);
			return -EINVAL;
		}
		p += len;
		oboff += (uint32_t)len;
	}
	obuf[oboff - 1] = '\n';

	ret = simple_read_from_buffer(ubuf, cnt, ppos, obuf, oboff);
	kfree(obuf);

	return ret;
}

static ssize_t tz_dbg_opt_write(struct file *filp,
	const char __user *ubuf, size_t cnt, loff_t *ppos)
{
	char buf[128] = {0};
	char *value = NULL;
	char *p = NULL;
	uint32_t i = 0;

	if (!ubuf || !filp || !ppos)
		return -EINVAL;

	if (cnt >= sizeof(buf))
		return -EINVAL;

	if (cnt == 0)
		return -EINVAL;

	if (copy_from_user(buf, ubuf, cnt) != 0)
		return -EFAULT;

	buf[cnt] = 0;
	if (cnt > 0 && buf[cnt - 1] == '\n')
		buf[cnt - 1] = 0;
	value = buf;
	p = strsep(&value, ":"); /* when buf has no :, value may be NULL */
	if (!p)
		return -EINVAL;

	for (i = 0; i < ARRAY_SIZE(g_opt_arr); i++) {
		if ((strncmp(p, g_opt_arr[i].name,
			strlen(g_opt_arr[i].name)) == 0) &&
			strlen(p) == strlen(g_opt_arr[i].name)) {
			g_opt_arr[i].func(value);
			return (ssize_t)cnt;
		}
	}
	return -EFAULT;
}

static const struct file_operations g_tz_dbg_opt_fops = {
	.owner = THIS_MODULE,
	.read = tz_dbg_opt_read,
	.write = tz_dbg_opt_write,
};

#ifdef CONFIG_MEMSTAT_DEBUGFS
static int memstat_debug_show(struct seq_file *m, void *v)
{
	struct tee_mem *mem_stat = NULL;
	int ret;
	uint32_t i;
	(void)v;

	mem_stat = kzalloc(sizeof(*mem_stat), GFP_KERNEL);
	if (!mem_stat)
		return -ENOMEM;

	ret = get_tee_meminfo(mem_stat);
	if (ret != 0) {
		tloge("get tee meminfo failed\n");
		kfree(mem_stat);
		mem_stat = NULL;
		return -EINVAL;
	}

	seq_printf(m, "TotalMem:%u Pmem:%u Free_Mem:%u Free_Mem_Min:%u TA_Num:%u\n",
		mem_stat->total_mem, mem_stat->pmem, mem_stat->free_mem, mem_stat->free_mem_min, mem_stat->ta_num);

	for (i = 0; i < mem_stat->ta_num; i++)
		seq_printf(m, "ta_name:%s ta_pmem:%u pmem_max:%u pmem_limit:%u\n",
			mem_stat->ta_mem_info[i].ta_name, mem_stat->ta_mem_info[i].pmem,
			mem_stat->ta_mem_info[i].pmem_max, mem_stat->ta_mem_info[i].pmem_limit);

	kfree(mem_stat);
	mem_stat = NULL;
	return 0;
}

static int tz_memstat_open(struct inode *inode, struct file *file)
{
	(void)inode;
	return single_open(file, memstat_debug_show, NULL);
}

static const struct file_operations g_tz_dbg_memstat_fops = {
	.owner   = THIS_MODULE,
	.open    = tz_memstat_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = single_release,
};
#endif

#ifdef CONFIG_TEE_TRACE
static int tee_trace_event_show(struct seq_file *m, void *v)
{
	struct tee_trace_view_t view = { 0, 0, 0, 0, { 0 }, { 0 } };
	struct trace_log_info log_info;
	(void)v;

	get_tee_trace_start(&view);
	if (view.buffer_is_full == 1)
		seq_printf(m, "Total Trace Events: %u (Notice: Buffer is full)\n", view.total);
	else
		seq_printf(m, "Total Trace Events: %u\n", view.total);

	if (view.total > 0) {
		uint32_t i = 0;

		while (get_tee_trace_next(&view, &log_info, false) != -1) {
			uint32_t task_ca = (uint32_t)(log_info.add_info);
			uint32_t task_idx = (uint32_t)(log_info.add_info >> 32);

			if (log_info.event_id == SCHED_IN || log_info.event_id == SCHED_OUT) {
				seq_printf(m, "[%4u][cpu%3u][ca-%5u] %10llu : %s %u %s\n",
					i++, log_info.cpu, log_info.ca_pid, log_info.time, log_info.event_name,
					task_ca, get_tee_trace_task_name(task_idx));
			} else {
				seq_printf(m, "[%4u][cpu%3u][ca-%5u] %10llu : %s %llu\n",
					i++, log_info.cpu, log_info.ca_pid, log_info.time, log_info.event_name,
					log_info.add_info);
			}
		}
	}

	return 0;
}

static int tee_trace_event_open(struct inode *inode, struct file *file)
{
	return single_open(file, tee_trace_event_show, NULL);
}

struct tee_trace_cmd_t {
	const char *cmd;
	int (*func)(void);
} tee_trace_cmd[] = {
	{"start", tee_trace_event_start},
	{"loop_record", tee_trace_event_start_loop_record},
	{"stop", tee_trace_event_stop}
};

static ssize_t tee_trace_event_write(struct file *filp,
	const char __user *ubuf, size_t cnt, loff_t *ppos)
{
	char buf[32] = {0};
	uint32_t i = 0;

	if (cnt >= sizeof(buf))
		return -EINVAL;

	if (copy_from_user(buf, ubuf, cnt))
		return -EINVAL;

	buf[cnt] = 0;
	if (cnt > 0 && buf[cnt - 1] == '\n')
		buf[cnt - 1] = 0;

	for (i = 0; i < ARRAY_SIZE(tee_trace_cmd); i++) {
		if (!strncmp(buf, tee_trace_cmd[i].cmd,
			strlen(tee_trace_cmd[i].cmd))) {
			tee_trace_cmd[i].func();
			return cnt;
		}
	}
	return -EINVAL;
}

static const struct file_operations tee_trace_event_fops = {
	.owner   = THIS_MODULE,
	.open    = tee_trace_event_open,
	.read    = seq_read,
	.write    = tee_trace_event_write,
	.llseek  = seq_lseek,
	.release = single_release,
};
#endif

int tzdebug_init(void)
{
#if defined(DEF_ENG) || defined(CONFIG_TZDRIVER_MODULE)
	g_tz_dbg_dentry = debugfs_create_dir("tzdebug", NULL);
	if (!g_tz_dbg_dentry)
		return -1;

	debugfs_create_file("opt", 0660, g_tz_dbg_dentry, NULL,
		&g_tz_dbg_opt_fops);

#ifdef CONFIG_MEMSTAT_DEBUGFS
	debugfs_create_file("memstat", 0444, g_tz_dbg_dentry, NULL,
		&g_tz_dbg_memstat_fops);
#endif

#ifdef CONFIG_TEE_TRACE
	debugfs_create_file("tee_trace", 0660, g_tz_dbg_dentry, NULL,
		&tee_trace_event_fops);
	tee_trace_event_enable();
#endif

#else
	(void)g_tz_dbg_dentry;
	(void)g_tz_dbg_opt_fops;
#endif
	return 0;
}

void free_tzdebug(void)
{
#if defined(DEF_ENG) || defined(CONFIG_TZDRIVER_MODULE)
	if (!g_tz_dbg_dentry)
		return;

	debugfs_remove_recursive(g_tz_dbg_dentry);
	g_tz_dbg_dentry = NULL;
#endif
}
