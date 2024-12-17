/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/topology.h>
#include <linux/cpufreq.h>
#include <linux/percpu-defs.h>
#include "rnic_private.h"
#include "rnic_srs_boost.h"
#include "rnic_log.h"

DEFINE_PER_CPU_ALIGNED(struct rnic_srs_cpu_idle_time, rnic_cpu_idle_time);

void rnic_srs_perf_core_boost(void)
{
	core_ctl_set_boost(RNIC_PERF_CORE_BOOST_TIMEOUT);
}

STATIC struct rnic_srs_pm_qos_s rnic_pm_qos_pool[RNIC_PM_QOS_MAX];

struct rnic_srs_pm_qos_s *rnic_srs_get_pm_qos_pool(void)
{
	return &rnic_pm_qos_pool[0];
}

void rnic_srs_pm_qos_boost(u32 qos_id, s32 new_value)
{
	struct rnic_srs_pm_qos_s *pool = rnic_srs_get_pm_qos_pool();

	if (unlikely(qos_id >= RNIC_PM_QOS_MAX))
		return;

	platform_qos_update_request(&pool[qos_id].req, new_value);
	pool[qos_id].value = new_value;
}

STATIC void rnic_srs_pm_qos_init(void)
{
	struct rnic_srs_pm_qos_s *pool = rnic_srs_get_pm_qos_pool();
	u32 i;

	for (i = 0; i < RNIC_PM_QOS_MAX; i++)
		platform_qos_add_request_memory_latency(&pool[i].req, 0);
}

STATIC void rnic_srs_pm_qos_exit(void)
{
	struct rnic_srs_pm_qos_s *pool = rnic_srs_get_pm_qos_pool();
	u32 i;

	for (i = 0; i < RNIC_PM_QOS_MAX; i++)
		platform_qos_remove_request(&pool[i].req);
}

STATIC struct rnic_srs_freq_qos_s rnic_freq_qos_pool[RNIC_MAX_FREQ_CPU_CLUSTERS] = {0};
STATIC u32 rnic_freq_qos_num = 0;

u32 rnic_srs_get_last_req_cpufreq(u32 cluster_id)
{
	if (unlikely(cluster_id >= rnic_freq_qos_num))
		return 0;

	return rnic_freq_qos_pool[cluster_id].value;
}

struct rnic_srs_freq_qos_s *rnic_srs_get_freq_qos_pool(void)
{
	return &rnic_freq_qos_pool[0];
}

void rnic_srs_freq_qos_boost(u32 cluster_id, u32 new_value)
{
	struct rnic_srs_freq_qos_s *pool = rnic_srs_get_freq_qos_pool();

	if (unlikely(cluster_id >= rnic_freq_qos_num))
		return;

	if (pool[cluster_id].value != new_value) {
		lpcpu_cpufreq_update_req(&pool[cluster_id].req, new_value);
		pool[cluster_id].value = new_value;
	}
}

STATIC void rnic_srs_freq_qos_init(void)
{
	struct rnic_srs_freq_qos_s *pool = rnic_srs_get_freq_qos_pool();
	struct cpumask cpu_masks[RNIC_MAX_FREQ_CPU_CLUSTERS];
	int cpu, cluster_id;
	u32 i;

	for (i = 0; i < RNIC_MAX_FREQ_CPU_CLUSTERS; i++)
		cpumask_clear(&cpu_masks[i]);

	for_each_online_cpu(cpu) {
		cluster_id = topology_physical_package_id(cpu);
		if (cluster_id >= RNIC_MAX_FREQ_CPU_CLUSTERS)
			break;
		cpumask_set_cpu((u32)cpu, &cpu_masks[cluster_id]);
	}

	for (i = 0; i < RNIC_MAX_FREQ_CPU_CLUSTERS; i++) {
		cpu = (int)cpumask_first(&cpu_masks[i]);
		if ((u32)cpu >= nr_cpu_ids)
			break;
		lpcpu_cpufreq_init_req(&pool[i].req, cpu);
		rnic_freq_qos_num++;
	}
}

STATIC void rnic_srs_freq_qos_exit(void)
{
	struct rnic_srs_freq_qos_s *pool = rnic_srs_get_freq_qos_pool();
	u32 i;

	for (i = 0; i < rnic_freq_qos_num; i++) {
		lpcpu_cpufreq_update_req(&pool[i].req, 0);
		lpcpu_cpufreq_exit_req(&pool[i].req);
		pool[i].value = 0;
	}

	rnic_freq_qos_num = 0;
}

void rnic_srs_boost_init(void)
{
	rnic_srs_pm_qos_init();
	rnic_srs_freq_qos_init();
}

void rnic_srs_boost_exit(void)
{
	rnic_srs_pm_qos_exit();
	rnic_srs_freq_qos_exit();
}

STATIC int rnic_srs_get_cpuload(u32 cpu)
{
	struct rnic_srs_cpu_idle_time *idle_time = NULL;
	u64 now, now_idle, delta_time, delta_idle;
	u32 load;

	now_idle = get_cpu_idle_time(cpu, &now, 0);
	idle_time = &per_cpu(rnic_cpu_idle_time, cpu);
	delta_idle = now_idle - idle_time->time;
	delta_time = now - idle_time->timestamp;

	if (unlikely(delta_time == 0 || delta_time <= delta_idle))
		load = 0;
	else
		load = div64_u64(100 * (delta_time - delta_idle), delta_time); /* percent 100 */

	idle_time->time = now_idle;
	idle_time->timestamp = now;

	return load;
}

STATIC void rnic_srs_get_boost_rpt_stats(struct rnic_srs_cpu_stats_s *stats)
{
	int cpu, cluster_id;
	unsigned long bitmask = 0;

	for (cpu = 0; cpu < NR_CPUS && cpu < SRS_MAX_CPU; cpu++) {
		if (!cpu_online(cpu)) {
			stats->cpuload[cpu] = 0;
			continue;
		}

		stats->cpuload[cpu] = rnic_srs_get_cpuload(cpu);

		cluster_id = topology_physical_package_id(cpu);
		if (test_bit(cluster_id, &bitmask) || cluster_id >= SRS_MAX_CLUSTER)
			continue;

		__set_bit(cluster_id, &bitmask);
		stats->cpufreq[cluster_id].req_cpufreq = rnic_srs_get_last_req_cpufreq(cluster_id);
		stats->cpufreq[cluster_id].cur_freq = cpufreq_quick_get(cpu);
		stats->cpufreq[cluster_id].max_freq = cpufreq_quick_get_max(cpu);
	}
}

void rnic_srs_boost_report_stats(void)
{
	struct rnic_srs_boost_stats_rpt_s info = {{0}};
	info.hdr.ver = 101; /* version 101 */
	rnic_srs_get_boost_rpt_stats(&info.stats);
	rnic_trans_report(ID_DIAG_RNIC_VNET_SRS_BOOST_STATS,
		&info, sizeof(info));
}