/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: function for tee self-recovery, tee reboot
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
#include "reboot.h"
#include <linux/delay.h>
#include <linux/kthread.h>
#include "agent.h"
#include "cmdmonitor.h"
#include "mailbox_mempool.h"
#include "tlogger.h"
#include "tz_spi_notify.h"
#include "secs_power_ctrl.h"
#include "session_manager.h"
#include "smc_smp.h"
#include "ko_adapt.h"
#include "tz_kthread_affinity.h"
#include <linux/sched.h>
#include <linux/sched/rt.h>

static atomic_t secondary_cpu_reboot;
static bool g_is_tee_rebooting = false;
static tee_alarm_func g_tee_alarm_func = NULL;
static struct pid *g_teecd_pid = NULL;
static struct pid *g_tlogcat_pid = NULL;

static void set_tee_is_dead_flag(void)
{
	g_is_tee_rebooting = true;
}

static void clr_tee_is_dead_flag(void)
{
	g_is_tee_rebooting = false;
}


bool is_tee_rebooting(void)
{
	return g_is_tee_rebooting;
}

static void tee_reboot_smc(uint32_t cmd)
{
	int ret;
	unsigned long flags;

	if (power_on_cc() != 0) {
		tloge("power on cc failed\n");
		return;
	}

	local_irq_save(flags);
	ret = send_smc_cmd_rebooting(cmd, 0, 0, NULL);

	local_irq_restore(flags);
	if (power_down_cc() != 0) {
		tloge("power down cc failed\n");
		return;
	}

	return;
}

static struct work_struct reboot_work;
static struct work_struct reboot_done_work;
static struct work_struct secondary_cpu_on_work[64];

static void secondary_cpu_on_func(struct work_struct *dummy)
{
	tee_reboot_smc(TSP_CPU_ON);
	atomic_add(1, &secondary_cpu_reboot);
	return;
}

static void tee_reboot_secondary_cpus(void)
{
	int i;
	tlogd("secondary cpu will reboot\n");
	/* reboot secondary cpus */
	get_online_cpus();
	for_each_online_cpu(i) {
		if (i != 0) {
			INIT_WORK(&secondary_cpu_on_work[i], secondary_cpu_on_func);
			schedule_work_on(i, &secondary_cpu_on_work[i]);
			flush_work(&secondary_cpu_on_work[i]);
		}
	}
	put_online_cpus();
}

static void tee_reboot_work_func(struct work_struct *dummy)
{
	tlogd("primary cpu will reboot\n");
	tee_reboot_smc(TSP_REBOOT);

	tee_reboot_secondary_cpus();
	return;
}

static int tee_reboot_work(void)
{
	uint32_t retry_count = 0;
	atomic_set(&secondary_cpu_reboot, 0);

	INIT_WORK(&reboot_work, tee_reboot_work_func);
	schedule_work_on(0, &reboot_work);
	flush_work(&reboot_work);

	while (retry_count < REBOOT_MAX_COUNT) {
		/* The number of secondary cpu is the total number of cpus minus one. */
		if (atomic_read(&secondary_cpu_reboot) >= (nr_cpu_ids - 1))
			return 0;
		msleep(REBOOT_SLEEP);
		retry_count++;
	}

	return -1;
}

static void tee_reboot_done_work_func(struct work_struct *dummy)
{
	tlogd("clear reboot flag\n");
	tee_reboot_smc(TSP_REBOOT_DONE);
	return;
}

static void tee_reboot_done_work(void)
{
	INIT_WORK(&reboot_done_work, tee_reboot_done_work_func);
	schedule_work_on(0, &reboot_done_work);
	flush_work(&reboot_done_work);
}

static bool get_gic_mpidr_mt(void)
{
	/* MT decides how to send a sgi */
	if (get_mpidr_el1() & MPIDR_MT)
		return true;
	else
		return false;
}

static uint64_t get_gic_cpu_affinity(uint32_t cpu_id)
{
	/* set affinity of each cpu */
	return get_mpidr_el1() & MPIDR_AFF_MASK;
}

static void gic_send_sgi(uint32_t irq, uint32_t target_list)
{
	uint32_t i;
	for_each_online_cpu(i) {
		if ((1 << i) & target_list) {
			uint64_t sgi1r;
			uint64_t aff = get_gic_cpu_affinity(i);
			if (get_gic_mpidr_mt()) {
				sgi1r = ((aff >> MPIDR_AFF3_FIELD) << SGI_AFF3_FIELD) |              /* aff3 */
					(((aff >> MPIDR_AFF2_FIELD) & AFF_MASK) << SGI_AFF2_FIELD) | /* aff2 */
					(((aff >> MPIDR_AFF1_FIELD) & AFF_MASK) << SGI_AFF1_FIELD) | /* aff1 */
					1 |                         /* if mt is true, targetlist is always 1 */
					((uint64_t)irq << SGI_ID_FIELD);                              /* irq */
			} else {
				sgi1r = ((aff >> MPIDR_AFF3_FIELD) << SGI_AFF3_FIELD) |              /* aff3 */
					(((aff >> MPIDR_AFF2_FIELD) & AFF_MASK) << SGI_AFF2_FIELD) | /* aff2 */
					(((aff >> MPIDR_AFF1_FIELD) & AFF_MASK) << SGI_AFF1_FIELD) | /* aff1 */
					(1 << (i % GIC_AFF_LEVEL)) |        /* if mt is false, can shoose PE */
					((uint64_t)irq << SGI_ID_FIELD);                              /* irq */
			}
			set_icc_sgi1r_el1(sgi1r);
			__asm__ volatile ("isb" : : : "memory");
		}
	}
}

static uint32_t get_online_cpu_mask(void)
{
	uint32_t i;
	uint32_t cpu_mask = 0;
	for_each_online_cpu(i)
		cpu_mask |= (1 << i);
	return cpu_mask;
}

static void tee_alarm_report(int alarm_id)
{
	if (g_tee_alarm_func == NULL)
		return;

	int ret = g_tee_alarm_func(alarm_id, ALARM_REPORT);
	if (ret != 0)
		tloge("report crash fail, ret is %d\n", ret);
}

static void tee_alarm_clear(int alarm_id)
{
	if (g_tee_alarm_func == NULL)
		return;

	int ret = g_tee_alarm_func(alarm_id, ALARM_CLEAR);
	if (ret != 0)
		tloge("clean crash fail, ret is %d\n", ret);
}

int teek_register_alarm_func(tee_alarm_func alarm_func)
{
	if (alarm_func == NULL)
		return -EINVAL;

	g_tee_alarm_func = alarm_func;
	return 0;
}
EXPORT_SYMBOL(teek_register_alarm_func);

void get_teecd_pid(void)
{
	get_task_struct(current);
	g_teecd_pid = get_task_pid(current, PIDTYPE_PID);
}

void get_tlogcat_pid(void)
{
	get_task_struct(current);
	g_tlogcat_pid = get_task_pid(current, PIDTYPE_PID);
}

static void kill_teecd_and_tlogcat(void)
{
	if (g_teecd_pid != NULL)
		kill_pid(g_teecd_pid, SIGKILL, 1);
	if (g_tlogcat_pid != NULL)
		kill_pid(g_tlogcat_pid, SIGKILL, 1);
}

static int tee_reboot(void)
{
	int ret = 0;
	set_tee_is_dead_flag();
	tee_alarm_report(TEE_CRASH);
	gic_send_sgi(GIC_CPU_RESCHEDULE, get_online_cpu_mask());
	ret = tee_reboot_work();
	if (ret != 0) {
		tloge("tee reboot work failed, ret 0x%x\n", ret);
		goto err;
	}

	smc_wakeup_broadcast();
#ifndef CONFIG_DISABLE_SVC
	svc_thread_release();
#endif
	occupy_clean_cmd_buf();
	send_smc_reset_cmd_buffer();

	ret = re_register_mailbox();
	if (ret != 0) {
		tloge("re-register mailbox failed, ret 0x%x\n", ret);
		goto err;
	}

	ret = send_notify_cmd(GLOBAL_CMD_ID_REGISTER_NOTIFY_MEMORY);
	if (ret != 0) {
		tloge("send notify cmd failed, ret 0x%x\n", ret);
		goto err;
	}

	free_all_session();
	ret = register_tloger_mem();
	if (ret != 0) {
		tloge("re-register tloger failed, ret 0x%x\n", ret);
		goto err;
	}

	clr_tee_is_dead_flag();
	tee_reboot_done_work();
	clr_system_crash_flag();

#ifndef CONFIG_DISABLE_SVC
	ret = init_smc_svc_thread();
	if (ret != 0) {
		tloge("init svc thread failed\n");
		goto err;
	}
#endif

	tee_alarm_clear(TEE_CRASH);
	kill_teecd_and_tlogcat();
	return 0;

err:
	tee_alarm_report(TEE_REBOOT_FAIL);
	return ret;
}

static struct task_struct *g_reboot_thread;
DEFINE_MUTEX(g_reboot_lock);
static DECLARE_WAIT_QUEUE_HEAD(reboot_th_wait);
static atomic_t g_reboot_th_run;

#define RUN_REBOOT_THREAD 1
#define STOP_REBOOT_THREAD 2
static int tee_reboot_fn(void *arg)
{
	int ret;
	while (1) {
		ret = wait_event_interruptible(reboot_th_wait,
			atomic_read(&g_reboot_th_run));
		if (ret != 0) {
			tloge("wait reboot event interruptible failed!\n");
			return -EINTR;
		}
		if (atomic_read(&g_reboot_th_run) == STOP_REBOOT_THREAD)
			break;

		mutex_lock(&g_reboot_lock);
		if (tee_reboot() != 0)
			tloge("tee reboot failed\n");

		atomic_set(&g_reboot_th_run, 0);
		mutex_unlock(&g_reboot_lock);
	}
	return ret;
}

static int tee_create_reboot_thread(void)
{
	g_reboot_thread = kthread_create(tee_reboot_fn, NULL, "reboot_thread");
	if (unlikely(IS_ERR_OR_NULL(g_reboot_thread))) {
		tloge("couldn't create reboot thread %ld\n",
			PTR_ERR(g_reboot_thread));
		return -1;
	}
	return 0;
}

int tee_init_reboot_thread(void)
{
	if (tee_create_reboot_thread() != 0) {
		tloge("init reboot thread failed\n");
		return -EFAULT;
	}
	wake_up_process(g_reboot_thread);
	return 0;
}

void free_reboot_thread(void)
{
	set_tee_is_dead_flag();
	if (!IS_ERR_OR_NULL(g_reboot_thread)) {
		atomic_set(&g_reboot_th_run, STOP_REBOOT_THREAD);
		wake_up_interruptible(&reboot_th_wait);
		kthread_stop(g_reboot_thread);
		g_reboot_thread = NULL;
	}
}

int tee_wake_up_reboot(void)
{
	tloge("tee will reboot\n");
	atomic_set(&g_reboot_th_run, RUN_REBOOT_THREAD);
	wake_up_interruptible(&reboot_th_wait);
	return 0;
}
