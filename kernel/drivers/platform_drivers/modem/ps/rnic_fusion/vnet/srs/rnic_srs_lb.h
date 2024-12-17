/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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

#ifndef RNIC_SRS_LB_H
#define RNIC_SRS_LB_H

#include <linux/cache.h>
#include <linux/cpuhotplug.h>
#include "rnic_srs.h"
#include "rnic_dsm_msg_pif.h"

#ifdef __cplusplus
extern "C" {
#endif

struct rnic_srs_lb_stats_s {
	u32 lb_cpu;
	u32 hp_online_num;
	u32 hp_down_num;
};

struct rnic_srs_lb_cfg_s {
	u32 enable;
	u32 cluster_weight[SRS_PPS_LEVEL][SRS_MAX_CLUSTER];
};

struct rnic_srs_lb_s {
	struct rnic_srs_ctx_s *back;
	struct rnic_srs_lb_stats_s lb_stats[NR_CPUS];
	struct rnic_srs_lb_cfg_s cfg;

	enum cpuhp_state online_state;
	u32 cur_level;
	spinlock_t lb_lock ____cacheline_aligned;

	atomic_t tcpu;
	cpumask_var_t cpumask_cfg ____cacheline_aligned;
	cpumask_var_t cpumask_sel;
	u32 weight_cfg[SRS_MAX_CLUSTER];
	u32 weight_sel[SRS_MAX_CLUSTER];
};

static inline bool rnic_srs_cluster_valid(int cluster)
{
	return cluster >= 0 && cluster < SRS_MAX_CLUSTER;
}

void rnic_srs_set_lb_cfg(const struct rnic_vnet_fea_cfg_s *cfg);
void rnic_srs_set_lb_level(u32 level);
int rnic_srs_rx_tcpu(void);
int rnic_srs_lb_init(struct rnic_srs_ctx_s *sctx);
void rnic_srs_lb_exit(struct rnic_srs_ctx_s *sctx);
struct rnic_srs_lb_s *rnic_srs_get_lctx(void);

#ifdef __cplusplus
}
#endif

#endif /* RNIC_SRS_LB_H */
