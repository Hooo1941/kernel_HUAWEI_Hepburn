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

#include <linux/atomic.h>
#include <linux/cpu.h>
#include <linux/topology.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include "securec.h"
#include "rnic_srs.h"
#include "rnic_private.h"
#include "rnic_log.h"
#include "rnic_dsm_msg_pif.h"
#include "rnic_srs_lb.h"

STATIC struct rnic_srs_lb_s *rnic_srs_lb_ctx_ptr;

struct rnic_srs_lb_s *rnic_srs_get_lctx(void)
{
	return rnic_srs_lb_ctx_ptr;
}

STATIC int rnic_srs_cpuhp_online(unsigned int cpu)
{
	struct rnic_srs_lb_s *lctx = rnic_srs_lb_ctx_ptr;
	unsigned long flags;
	u32 cluster_weight;
	int cluster = topology_physical_package_id((int)cpu);

	spin_lock_irqsave(&lctx->lb_lock, flags);

	cluster_weight = lctx->cfg.cluster_weight[lctx->cur_level][cluster];
	if (cluster_weight != 0) {
		cpumask_set_cpu(cpu, lctx->cpumask_cfg);
		cpumask_set_cpu(cpu, lctx->cpumask_sel);
		lctx->weight_cfg[cluster] += cluster_weight;
	}

	spin_unlock_irqrestore(&lctx->lb_lock, flags);

	lctx->lb_stats[cpu].hp_online_num++;
	return 0;
}

STATIC int rnic_srs_cpuhp_perpare_down(unsigned int cpu)
{
	struct rnic_srs_lb_s *lctx = rnic_srs_lb_ctx_ptr;
	unsigned long flags;
	u32 cluster_weight;
	int cluster = topology_physical_package_id((int)cpu);

	spin_lock_irqsave(&lctx->lb_lock, flags);

	cluster_weight = lctx->cfg.cluster_weight[lctx->cur_level][cluster];
	if (cluster_weight != 0) {
		cpumask_clear_cpu((int)cpu, lctx->cpumask_cfg);
		cpumask_clear_cpu((int)cpu, lctx->cpumask_sel);
		if (lctx->weight_cfg[cluster] >= cluster_weight)
			lctx->weight_cfg[cluster] -= cluster_weight;
	}

	spin_unlock_irqrestore(&lctx->lb_lock, flags);

	lctx->lb_stats[cpu].hp_down_num++;
	return 0;
}

STATIC void rnic_srs_cpuhp_init(struct rnic_srs_lb_s *lctx)
{
	int ret;

	/*
	 * Care cpuhp_online_state, Currently it equel CPUHP_AP_ONLINE_DYN,
	 * so just assigned ret to online_state; When it equal a static STATE,
	 * please assigned the static STATE to online_state.
	 */
	ret = cpuhp_setup_state_nocalls(CPUHP_AP_ONLINE_DYN,
					"rnic:online",
					rnic_srs_cpuhp_online,
					rnic_srs_cpuhp_perpare_down);
	if (ret < 0) {
		RNIC_LOGE("register cpu hotplug failed, ret %d", ret);
		lctx->online_state = CPUHP_INVALID;
	} else {
		lctx->online_state = (enum cpuhp_state)ret;
	}
}

STATIC void rnic_srs_cpuhp_deinit(struct rnic_srs_lb_s *lctx)
{
	if (lctx->online_state == CPUHP_INVALID)
		RNIC_LOGE("invalid online state");
	else
		cpuhp_remove_state(lctx->online_state);
}

STATIC void rnic_srs_set_lb_level_cfg(struct rnic_srs_lb_s *lctx, u32 level)
{
	unsigned long flags;
	u32 cluster_weight, i;
	int cpu, cluster;

	lctx->cur_level = level;

	spin_lock_irqsave(&lctx->lb_lock, flags);

	cpumask_clear(lctx->cpumask_cfg);
	(void)memset_s(lctx->weight_cfg, sizeof(lctx->weight_cfg),
		       0, sizeof(lctx->weight_cfg));

	for_each_online_cpu(cpu) {
		cluster = topology_physical_package_id(cpu);
		cluster_weight = lctx->cfg.cluster_weight[level][cluster];

		if (cluster_weight != 0 && cpu != 0) {
			cpumask_set_cpu((u32)cpu, lctx->cpumask_cfg);
			lctx->weight_cfg[cluster] += cluster_weight;
		}
	}
	cpumask_copy(lctx->cpumask_sel, lctx->cpumask_cfg);

	for (i = 0; i < SRS_MAX_CLUSTER; i++)
		lctx->weight_sel[i] = lctx->weight_cfg[i];

	spin_unlock_irqrestore(&lctx->lb_lock, flags);
}

void rnic_srs_set_lb_cfg(const struct rnic_vnet_fea_cfg_s *cfg)
{
	struct rnic_srs_lb_s *lctx = rnic_srs_lb_ctx_ptr;
	struct rnic_srs_lb_cfg_s *lb_cfg = &lctx->cfg;
	u32 i, j;

	lb_cfg->enable = cfg->lb_enable;

	for (i = 0; i < SRS_PPS_LEVEL; i++) {
		for (j = 0; j < SRS_MAX_CLUSTER; j++)
			lb_cfg->cluster_weight[i][j] = cfg->level_cfg[i].lb_weight[j];
	}

	rnic_srs_set_lb_level_cfg(lctx, 0);
}

void rnic_srs_set_lb_level(u32 level)
{
	struct rnic_srs_lb_s *lctx = rnic_srs_lb_ctx_ptr;

	if (unlikely(lctx->cfg.enable == 0))
		return;

	rnic_srs_set_lb_level_cfg(lctx, level);
}

STATIC u32 rnic_srs_get_lb_tcpu(struct rnic_srs_lb_s *lctx)
{
	u32 tcpu = (u32)atomic_read(&lctx->tcpu);

	if (cpumask_weight(lctx->cpumask_sel) > 0) {
		tcpu = cpumask_next((int)tcpu, lctx->cpumask_sel);
		if (tcpu >= nr_cpu_ids)
			tcpu = cpumask_first(lctx->cpumask_sel);
	} else {
		/* restore lb weight and cpumask. */
		u32 i;
		for (i = 0; i < SRS_MAX_CLUSTER; i++)
			lctx->weight_sel[i] = lctx->weight_cfg[i];
		cpumask_copy(lctx->cpumask_sel, lctx->cpumask_cfg);
		tcpu = cpumask_first(lctx->cpumask_sel);
	}

	if (tcpu >= nr_cpu_ids)
		tcpu = 0;

	return tcpu;
}

STATIC int rnic_srs_select_lb_cpu(struct rnic_srs_lb_s *lctx)
{
	struct rnic_srs_ctx_s *sctx = lctx->back;
	unsigned long flags;
	int cluster;
	u32 tcpu;

	spin_lock_irqsave(&lctx->lb_lock, flags);

	tcpu = rnic_srs_get_lb_tcpu(lctx);
	atomic_set(&lctx->tcpu, (int)tcpu);

	cluster = topology_physical_package_id(tcpu);
	if (lctx->weight_sel[cluster] > 0)
		lctx->weight_sel[cluster]--;
	if (lctx->weight_sel[cluster] == 0)
		cpumask_andnot(lctx->cpumask_sel, lctx->cpumask_sel,
			       sctx->cluster_cpumask[cluster]);

	spin_unlock_irqrestore(&lctx->lb_lock, flags);
	lctx->lb_stats[tcpu].lb_cpu++;

	return (int)tcpu;
}

int rnic_srs_rx_tcpu(void)
{
	struct rnic_srs_lb_s *lctx = rnic_srs_lb_ctx_ptr;

	if (unlikely(lctx->cfg.enable == 0))
		return (int)smp_processor_id();

	return rnic_srs_select_lb_cpu(lctx);
}

STATIC int rnic_srs_lb_cpumasks_init(struct rnic_srs_lb_s *lctx)
{
	if (!alloc_cpumask_var(&lctx->cpumask_cfg, GFP_KERNEL))
		goto err_alloc_cpumask_cfg;

	if (!alloc_cpumask_var(&lctx->cpumask_sel, GFP_KERNEL))
		goto err_alloc_cpumask_sel;

	cpumask_clear(lctx->cpumask_cfg);
	cpumask_clear(lctx->cpumask_sel);

	return 0;

err_alloc_cpumask_sel:
	free_cpumask_var(lctx->cpumask_cfg);
err_alloc_cpumask_cfg:
	return -ENOMEM;
}

STATIC void rnic_srs_lb_cpumasks_deinit(struct rnic_srs_lb_s *lctx)
{
	free_cpumask_var(lctx->cpumask_cfg);
	free_cpumask_var(lctx->cpumask_sel);
}

int rnic_srs_lb_init(struct rnic_srs_ctx_s *sctx)
{
	struct rnic_srs_lb_s *lctx = rnic_srs_lb_ctx_ptr;
	int ret;

	if (lctx != NULL)
		return 0;

	lctx = kzalloc(sizeof(*lctx), GFP_KERNEL);
	if (lctx == NULL) {
		RNIC_LOGE("alloc lb lctx failed");
		goto err_alloc;
	}

	ret = rnic_srs_lb_cpumasks_init(lctx);
	if (ret) {
		RNIC_LOGE("lb cpumask init failed");
		goto err_cpumask_init;
	}

	rnic_srs_cpuhp_init(lctx);
	spin_lock_init(&lctx->lb_lock);
	atomic_set(&lctx->tcpu, 0);

	lctx->back = sctx;
	rnic_srs_lb_ctx_ptr = lctx;
	return 0;

err_cpumask_init:
	kfree(lctx);
err_alloc:
	return -ENOMEM;
}

void rnic_srs_lb_exit(struct rnic_srs_ctx_s *sctx)
{
	struct rnic_srs_lb_s *lctx = rnic_srs_lb_ctx_ptr;

	if (lctx != NULL) {
		rnic_srs_cpuhp_deinit(lctx);
		rnic_srs_lb_cpumasks_deinit(lctx);

		kfree(lctx);
		rnic_srs_lb_ctx_ptr = NULL;
	}

	return;
}
