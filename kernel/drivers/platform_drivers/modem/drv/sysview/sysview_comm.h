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
#ifndef __SYSVIEW_COMM_H__
#define __SYSVIEW_COMM_H__
#include <linux/mm.h>
#include <mdrv_msg.h>
#include <mdrv_timer.h>
#include <linux/kernel_stat.h>
#include "osl_types.h"
#include "sysview.h"
#ifdef __cplusplus
extern "C" {
#endif
#define SYSVIEW_MSCD2TICK 500
#define SYSVIEW_MAX_SECOND 8500000
#define SYSVIEW_MEM_MAX_NUM 2
#define SYSVIEW_TASK_PRIO 31
#define SYSVIEW_PRECISION 100000
#define SYSVIEW_WAIT_FOREVER 0xffffffff
#define SYSVIEW_PAGETOK(x) ((x) << (PAGE_SHIFT - 10))
#define SYSVIEW_INIT_OK 1
#define SYSVIEW_MAX_CORE 32
typedef struct {
    unsigned int enable;
    unsigned int period;
    unsigned int core_mask;
} diag_drv_cpuinfo_s;
typedef struct {
    u64 stat[NR_STATS];
} sysview_cpuinfo_s;
typedef struct {
    u64 all_time;
    u64 busy_time;
} sysview_usage_s;
typedef struct {
    unsigned int total;
    unsigned int free;
} sysview_meminfo_s;
struct sysview_info {
    u32 init_flag; // 0:error, 1: ok
    u32 mode;      // 1:only cpu 2: cpu + task
    u32 period;    // second
    u32 core_mask; // bit => core
    long jiffies;
    osl_sem_id load_task_sem;
    spinlock_t sysview_lock;
};
typedef struct {
    unsigned int core_num;
    unsigned int usage[SYSVIEW_MAX_CORE];
} drv_cpuinfo_data_s;
int sysview_task_init(struct sysview_info *load_info, char *name, OSL_TASK_FUNC entry);
void sysview_stop(struct sysview_info *load_info);
int sysview_start(const diag_drv_cpuinfo_s *cpuinfo, struct sysview_info *load_info);
u32 sysview_cpu_usage_get(u32 cpu);
void sysview_memory_get(sysview_meminfo_s *mem_info);
#ifdef __cplusplus
}
#endif

#endif
