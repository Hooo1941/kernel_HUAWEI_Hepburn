/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <linux/cpumask.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/sched/stat.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/irqnr.h>
#include <linux/sched/cputime.h>
#include <linux/tick.h>
#include "sysview_comm.h"
#undef THIS_MODU
#define THIS_MODU mod_coresight
sysview_usage_s g_sysview_usage = { 0, 0 };
void sysview_stop(struct sysview_info *load_info)
{
    unsigned long flags;
    if (load_info->jiffies == SYSVIEW_WAIT_FOREVER) {
        bsp_debug("no need stop again\n");
        return;
    }
    spin_lock_irqsave(&load_info->sysview_lock, flags);
    load_info->mode = 0;
    load_info->jiffies = SYSVIEW_WAIT_FOREVER;
    spin_unlock_irqrestore(&load_info->sysview_lock, flags);
}
int sysview_start(const diag_drv_cpuinfo_s *cpuinfo, struct sysview_info *load_info)
{
    unsigned long flags;
    if (cpuinfo->enable == 0) {
        sysview_stop(load_info);
        return BSP_OK;
    }
    if (load_info->jiffies != SYSVIEW_WAIT_FOREVER) {
        bsp_err("cannot start again\n");
        return BSP_ERROR;
    }
    spin_lock_irqsave(&load_info->sysview_lock, flags);
    if (cpuinfo->period > 0 && cpuinfo->period <= SYSVIEW_MAX_SECOND) {
        load_info->jiffies = msecs_to_jiffies(cpuinfo->period * SYSVIEW_MSCD2TICK); // 500ms/period
    } else {
        load_info->jiffies = SYSVIEW_WAIT_FOREVER;
    }
    load_info->mode = cpuinfo->enable;
    load_info->period = cpuinfo->period;
    load_info->core_mask = cpuinfo->core_mask;
    spin_unlock_irqrestore(&load_info->sysview_lock, flags);
    osl_sem_up(&load_info->load_task_sem);
    return BSP_OK;
}
int sysview_task_init(struct sysview_info *load_info, char *name, OSL_TASK_FUNC entry)
{
    s32 ret;
    struct task_struct *pid = NULL;
    (void)memset_s(load_info, sizeof(struct sysview_info), 0, sizeof(struct sysview_info));
    spin_lock_init(&load_info->sysview_lock);
    load_info->jiffies = SYSVIEW_WAIT_FOREVER;
    osl_sem_init(0, &load_info->load_task_sem);
    ret = osl_task_init(name, SYSVIEW_TASK_PRIO, 0x1000, entry, NULL, &pid);
    if (ret) {
        bsp_err("sysview mprboe task err %d\n", ret);
        return BSP_ERROR;
    }
    load_info->init_flag = SYSVIEW_INIT_OK;
    return BSP_OK;
}
u64 sysview_get_idle_time(int cpu)
{
    u64 idle;
    u64 idle_usecs = -1ULL;
    if (cpu_online(cpu)) {
        idle_usecs = get_cpu_idle_time_us(cpu, NULL);
    }
    if (idle_usecs == -1ULL) {
        /* !NO_HZ or cpu offline so we can rely on cpustat.idle */
        idle = kcpustat_cpu(cpu).cpustat[CPUTIME_IDLE];
    } else {
        idle = idle_usecs * NSEC_PER_USEC;
    }
    return idle;
}
u64 sysview_get_iowait_time(int cpu)
{
    u64 iowait, iowait_usecs = -1ULL;

    if (cpu_online(cpu))
        iowait_usecs = get_cpu_iowait_time_us(cpu, NULL);

    if (iowait_usecs == -1ULL)
        /* !NO_HZ or cpu offline so we can rely on cpustat.iowait */
        iowait = kcpustat_cpu(cpu).cpustat[CPUTIME_IOWAIT];
    else
        iowait = iowait_usecs * NSEC_PER_USEC;

    return iowait;
}
u32 sysview_cpu_usage_get(u32 cpu)
{
    int j;
    u32 load = 0;
    u64 cpuinfo[NR_STATS] = {0, };
    u64 all_time;
    u64 busy_time = 0;
    u64 idle_time =
        div_u64(sysview_get_idle_time(cpu), NSEC_PER_MSEC) + div_u64(sysview_get_iowait_time(cpu), NSEC_PER_MSEC);
    sysview_usage_s old_info;
    for (j = 0; j < NR_STATS; j++) {
        cpuinfo[j] = kcpustat_cpu(cpu).cpustat[j];
        busy_time += cpuinfo[j];
    }
    busy_time -= (cpuinfo[CPUTIME_IDLE] + cpuinfo[CPUTIME_IOWAIT]);
    busy_time = div_u64(busy_time, NSEC_PER_MSEC);
    all_time = idle_time + busy_time;
    old_info.all_time = g_sysview_usage.all_time;
    old_info.busy_time = g_sysview_usage.busy_time;
    g_sysview_usage.all_time = all_time;
    g_sysview_usage.busy_time = busy_time;
    if (all_time > old_info.all_time && busy_time > old_info.busy_time) {
        all_time = all_time - old_info.all_time;
        busy_time = busy_time - old_info.busy_time;
    }
    if (all_time) {
        load = div_u64(busy_time * SYSVIEW_PRECISION, all_time);
    }
    return load;
}
void sysview_memory_get(sysview_meminfo_s *mem_info)
{
    struct sysinfo sys_info;
    if (mem_info == NULL) {
        return;
    }
    si_meminfo(&sys_info);
    mem_info->total = SYSVIEW_PAGETOK((unsigned int)(sys_info.totalram));
    mem_info->free = SYSVIEW_PAGETOK((unsigned int)(sys_info.freeram));
    return;
}