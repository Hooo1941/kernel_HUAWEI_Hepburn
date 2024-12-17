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

#ifndef RNIC_SRS_BOOST_H
#define RNIC_SRS_BOOST_H

#include <linux/cpu.h>
#include <linux/pm_qos.h>
#include "adrv_corectrl.h"
#include "adrv_platform_qos.h"
#include "rnic_srs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RNIC_MAX_FREQ_CPU_CLUSTERS	3
#define RNIC_PERF_CORE_BOOST_TIMEOUT	1200

enum rnic_pm_qos_id_e {
	RNIC_PM_QOS_MEM_TPUT = 0,
	RNIC_PM_QOS_MEM_CONG,
	RNIC_PM_QOS_MAX,
};

void rnic_srs_perf_core_boost(void);

struct rnic_srs_pm_qos_s {
	struct platform_qos_request req;
	s32 value;
};

struct rnic_srs_pm_qos_s *rnic_srs_get_pm_qos_pool(void);
void rnic_srs_pm_qos_boost(u32 qos_id, s32 new_value);

struct rnic_srs_freq_qos_s {
	struct cpufreq_req req;
	u32 value;
};

struct rnic_srs_cpu_idle_time {
	u64 time;
	u64 timestamp;
};

struct rnic_srs_cpufreq {
	u32 req_cpufreq;
	u32 cur_freq;
	u32 max_freq;
	u32 reserved;
};

struct rnic_srs_cpu_stats_s {
	struct rnic_srs_cpufreq cpufreq[SRS_MAX_CLUSTER];
	u32 cpuload[SRS_MAX_CPU];
};

struct rnic_srs_boost_stats_rpt_s {
	struct {
		u8 ver;
		u8 reserved[7];
	} hdr;
	struct rnic_srs_cpu_stats_s stats;
};

struct rnic_srs_freq_qos_s *rnic_srs_get_freq_qos_pool(void);
void rnic_srs_freq_qos_boost(u32 cluster_id, u32 new_value);
u32 rnic_srs_get_last_req_cpufreq(u32 cluster_id);
void rnic_srs_boost_init(void);
void rnic_srs_boost_exit(void);
void rnic_srs_show_boost_info(void);
void rnic_srs_boost_report_stats(void);

#ifdef __cplusplus
}
#endif

#endif /* RNIC_SRS_BOOST_H */
