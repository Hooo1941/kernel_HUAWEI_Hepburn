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

#include "rnic_srs_cc.h"
#include "rnic_srs_boost.h"
#include "rnic_srs_napi.h"
#include "rnic_private.h"
#include "rnic_log.h"
#include "rnic_dsm_msg_pif.h"

STATIC struct rnic_srs_cc_s *rnic_srs_cc_ctx_ptr;

struct rnic_srs_cc_s *rnic_srs_get_cctx(void)
{
	return rnic_srs_cc_ctx_ptr;
}

STATIC void rnic_srs_cqcc_set_cpufreq(struct rnic_cqcc_cfg_s *cfg, u32 cluster_num)
{
	u32 cluster;

	for (cluster = 0; cluster < cluster_num; cluster++)
		rnic_srs_freq_qos_boost(cluster, cfg->cpufreq[cluster]);
}

STATIC void rnic_srs_cqcc_set_ddr_bd(struct rnic_cqcc_cfg_s *cfg)
{
	struct rnic_srs_cc_s *cctx = rnic_srs_cc_ctx_ptr;

	if (cfg->ddr_bd != cctx->cqcc_last_ddr_bd) {
		rnic_srs_pm_qos_boost(RNIC_PM_QOS_MEM_TPUT, (s32)cfg->ddr_bd);
		cctx->cqcc_last_ddr_bd = cfg->ddr_bd;
	}
}

void rnic_srs_set_cc_level(struct rnic_srs_ctx_s *sctx, u32 level)
{
	struct rnic_srs_cc_s *cctx = rnic_srs_cc_ctx_ptr;
	struct rnic_cqcc_cfg_s *cfg = &cctx->cfg.cqcc_cfg[level];
	unsigned long rps_core_mask = cfg->rps_core_mask;

	if (unlikely(cctx->cc_enable == 0))
		return;

	cctx->cqcc_enable = cfg->enable;
	cctx->cur_level = level;
	cctx->backlog_len_limit = cfg->backlog_len_limit;

	rnic_srs_cqcc_set_cpufreq(cfg, sctx->cluster_num);
	rnic_srs_cqcc_set_ddr_bd(cfg);
	if (cfg->isolation_dis)
		rnic_srs_perf_core_boost();

	if (rps_core_mask != 0 && rps_core_mask != sctx->rps_core_mask)
		rnic_srs_update_all_rps_cpumask(rps_core_mask);
}

void rnic_srs_set_cc_cfg(const struct rnic_vnet_fea_cfg_s *cfg)
{
	struct rnic_cc_cfg_s *cc_cfg = &rnic_srs_cc_ctx_ptr->cfg;
	struct rnic_srs_ctx_s *sctx = rnic_srs_cc_ctx_ptr->back;
	u32 i, j;

	rnic_srs_cc_ctx_ptr->cc_enable = cfg->cc_enable;
	rnic_srs_cc_ctx_ptr->nqcc_enable = cfg->nqcc_enable;
	cc_cfg->nqcc_cfg.ddr_req_enable = cfg->nqcc_ddr_req_enable;
	cc_cfg->nqcc_cfg.rps_mask_enable = cfg->nqcc_rps_mask_enable;
	cc_cfg->nqcc_cfg.timeout = cfg->nqcc_timeout;
	cc_cfg->nqcc_cfg.ddr_mid_bd = cfg->ddr_mid_bd;
	cc_cfg->nqcc_cfg.ddr_high_bd = cfg->ddr_high_bd;

	for (i = 0; i < SRS_PPS_LEVEL; i++) {
		cc_cfg->cqcc_cfg[i].enable = cfg->level_cfg[i].cqcc_enable;
		cc_cfg->cqcc_cfg[i].rps_core_mask = cfg->level_cfg[i].rps_core_mask;
		cc_cfg->cqcc_cfg[i].isolation_dis = cfg->level_cfg[i].isolation_disable;
		cc_cfg->cqcc_cfg[i].backlog_len_limit = cfg->level_cfg[i].backlog_len_limit;
		cc_cfg->cqcc_cfg[i].ddr_bd = cfg->level_cfg[i].ddr_bd;

		for (j = 0; j < SRS_MAX_CLUSTER; j++)
			cc_cfg->cqcc_cfg[i].cpufreq[j] = cfg->level_cfg[i].cpufreq[j];
	}

	rnic_srs_set_cc_level(sctx, 0);
}

#if defined(CONFIG_RFS_RPS_MATCH)
STATIC bool rnic_srs_rx_cqcc_pend(struct rps_dev_flow *rflow, u32 next_cpu)
{
	struct rnic_srs_cc_s *cctx = rnic_srs_cc_ctx_ptr;
	u32 tcpu = rflow->cpu;

	/*
	 * If the desired CPU (where last recvmsg was done) is
	 * different from current CPU (one in the rx-queue flow
	 * table entry), current CPU is valid (< nr_cpu_ids)
	 * and online, and the current CPU's queue hasn't advanced
	 * beyond the last packet that was enqueued using this table entry.
	 */
	if (tcpu >= nr_cpu_ids || !cpu_online((int)tcpu))
		return false;

	if (test_bit((int)tcpu, &cctx->rps_pend_cpu_bitmask))
		return true;

	if (tcpu != next_cpu &&
	    ((int)(rflow->last_qtail -
	     per_cpu(softnet_data, tcpu).input_queue_head)) > 0 &&
	    (skb_queue_len(&per_cpu(softnet_data, tcpu).input_pkt_queue) +
	     skb_queue_len(&per_cpu(softnet_data, tcpu).process_queue)) >
	     cctx->backlog_len_limit) {
		__test_and_set_bit((int)tcpu, &cctx->rps_pend_cpu_bitmask);
		return true;
	}

	return false;
}

bool rnic_srs_rx_cqcc(struct net_device *netdev, struct sk_buff *skb)
{
	struct rnic_srs_cc_s *cctx = rnic_srs_cc_ctx_ptr;
	struct netdev_rx_queue *rx_queue = netdev->_rx;
	const struct rps_sock_flow_table *sock_flow_table = NULL;
	struct rps_dev_flow_table *flow_table = NULL;
	struct rps_map *map = NULL;
	struct rps_dev_flow *rflow = NULL;
	u32 hash, next_cpu, ident;
	bool ret = false;

	if (cctx->cqcc_enable == 0)
		return false;

	rcu_read_lock();
	flow_table = rcu_dereference(rx_queue->rps_flow_table);
	map = rcu_dereference(rx_queue->rps_map);
	sock_flow_table = rcu_dereference(rps_sock_flow_table);

	skb_reset_network_header(skb);
	hash = skb_get_hash(skb);
	if (flow_table == NULL || map == NULL || sock_flow_table == NULL || !hash)
		goto done;

	/* First check into global flow table if there is a match */
	ident = sock_flow_table->ents[hash & sock_flow_table->mask];
	if ((ident ^ hash) & ~rps_cpu_mask)
		goto done;

	next_cpu = ident & rps_cpu_mask;

	/*
	 * OK, now we know there is a match,
	 * we can look at the local (per receive queue) flow table
	 */
	rflow = &flow_table->flows[hash & flow_table->mask];
	ret = rnic_srs_rx_cqcc_pend(rflow, next_cpu);
done:
	rcu_read_unlock();
	return ret;
}

STATIC void rnic_srs_nqcc_rest_rps_cpumask(struct rnic_nqcc_cfg_s *cfg,
					   struct net_device *netdev)
{
	struct rnic_srs_cc_s *cctx = rnic_srs_cc_ctx_ptr;
	struct rnic_srs_ctx_s *sctx = cctx->back;
	struct netdev_rx_queue *rx_queue = netdev->_rx;
	unsigned long level_rps_core_mask;

	level_rps_core_mask = cctx->cfg.cqcc_cfg[cctx->cur_level].rps_core_mask;

	if (cfg->rps_mask_enable && sctx->rps_core_mask != level_rps_core_mask) {
		rnic_srs_update_rps_cpumask(rx_queue, level_rps_core_mask);
		sctx->rps_core_mask = level_rps_core_mask;
	}
}

STATIC u32 rnic_srs_nqcc_upgrade_rps_cpumask(struct rnic_nqcc_cfg_s *cfg,
					     struct net_device *netdev)
{
	struct rnic_srs_ctx_s *sctx = rnic_srs_cc_ctx_ptr->back;
	struct netdev_rx_queue *rx_queue = netdev->_rx;
	u32 ret = 0;

	mutex_lock(&sctx->mutex);
	if (cfg->rps_mask_enable && sctx->rps_core_mask != sctx->mb_core_mask) {
		rnic_srs_update_rps_cpumask(rx_queue, sctx->mb_core_mask);
		sctx->rps_core_mask = sctx->mb_core_mask;
		ret = 1;
	}
	mutex_unlock(&sctx->mutex);

	return ret;
}
#else
bool rnic_srs_rx_cqcc(struct net_device *netdev, struct sk_buff *skb)
{
	return false;
}
STATIC void rnic_srs_nqcc_rest_rps_cpumask(struct rnic_nqcc_cfg_s *cfg,
					   struct net_device *netdev) {}
STATIC u32 rnic_srs_nqcc_upgrade_rps_cpumask(struct rnic_nqcc_cfg_s *cfg,
					      struct net_device *netdev)
{
	return 1;
}
#endif

void rnic_srs_cqcc_clear_pend(void)
{
	rnic_srs_cc_ctx_ptr->rps_pend_cpu_bitmask = 0;
}

/*
 * expand napi weight to accelerate napi poll.
 * Return 1 if success, otherwise return 0.
 */
STATIC u32 rnic_srs_nqcc_exp_weight(struct rnic_srs_napi_priv_s *npriv)
{
	struct rnic_srs_cc_s *cctx = rnic_srs_cc_ctx_ptr;
	u32 ret = 0;
	int weight;

	weight = rnic_srs_get_napi_weight(npriv);
	if (weight != SRS_NAPI_MAX_WT) {
		rnic_srs_set_napi_weight(npriv, SRS_NAPI_MAX_WT);
		cctx->cc_stats.cc_wt_exp_num++;
		ret = 1;
	}

	return ret;
}

STATIC void rnic_srs_nqcc_rest_weight(struct rnic_srs_napi_priv_s *npriv)
{
	struct rnic_srs_cc_s *cctx = rnic_srs_cc_ctx_ptr;

	if (rnic_srs_reset_napi_weight(npriv))
		cctx->cc_stats.cc_wt_rst_num++;
}

STATIC u32 rnic_srs_nqcc_set_ddr_bd(struct rnic_nqcc_cfg_s *cfg,
				     u32 qos_id, u32 ddr_bd)
{
	struct rnic_srs_cc_s *cctx = rnic_srs_cc_ctx_ptr;
	u32 ret = 0;

	if (cfg->ddr_req_enable && ddr_bd != cctx->nqcc_last_ddr_bd) {
		rnic_srs_pm_qos_boost(qos_id, (s32)ddr_bd);
		cctx->nqcc_last_ddr_bd = ddr_bd;
		ret = 1;
	}

	return ret;
}

STATIC void rnic_srs_nqcc_lp_with_lock(struct rnic_srs_napi_priv_s *npriv)
{
	struct rnic_srs_cc_s *cctx = rnic_srs_cc_ctx_ptr;
	struct rnic_nqcc_cfg_s *cfg = &cctx->cfg.nqcc_cfg;

	cctx->queue_load = SRS_LOW;

	if (delayed_work_pending(&cctx->nqcc_rst_work))
		return;

	rnic_srs_nqcc_rest_weight(npriv);
	rnic_srs_nqcc_set_ddr_bd(cfg, RNIC_PM_QOS_MEM_CONG, 0);
	rnic_srs_nqcc_rest_rps_cpumask(cfg, npriv->netdev);
}

STATIC void rnic_srs_nqcc_lp(struct rnic_srs_napi_priv_s *npriv)
{
	struct rnic_srs_ctx_s *sctx = rnic_srs_cc_ctx_ptr->back;

	mutex_lock(&sctx->mutex);
	rnic_srs_nqcc_lp_with_lock(npriv);
	mutex_unlock(&sctx->mutex);
}

STATIC void rnic_srs_nqcc_hp(struct rnic_srs_napi_priv_s *npriv, u32 queue_load)
{
	struct rnic_srs_cc_s *cctx = rnic_srs_cc_ctx_ptr;
	struct rnic_nqcc_cfg_s *cfg = &cctx->cfg.nqcc_cfg;
	u32 delay = cfg->timeout;
	u32 ddr_bd_req;
	u32 reset = 0;

	if (queue_load == SRS_MID) {
		cctx->queue_load = SRS_MID;
		ddr_bd_req = cfg->ddr_mid_bd;
	} else {
		cctx->queue_load = SRS_HIGH;
		ddr_bd_req = cfg->ddr_high_bd;
	}

	reset |= rnic_srs_nqcc_exp_weight(npriv);
	reset |= rnic_srs_nqcc_set_ddr_bd(cfg, RNIC_PM_QOS_MEM_CONG, ddr_bd_req);
	reset |= rnic_srs_nqcc_upgrade_rps_cpumask(cfg, npriv->netdev);
	if (reset && (!delayed_work_pending(&cctx->nqcc_rst_work)))
		schedule_delayed_work(&cctx->nqcc_rst_work, msecs_to_jiffies(delay));
}

STATIC void rnic_srs_nqcc_update(struct rnic_srs_napi_priv_s *npriv, u32 queue_load)
{
	switch (queue_load) {
	case SRS_LOW:
		rnic_srs_nqcc_lp(npriv);
		break;
	case SRS_MID:
	case SRS_HIGH:
		rnic_srs_nqcc_hp(npriv, queue_load);
		break;
	default:
		break;
	}
}

STATIC u32 rnic_srs_nqcc_calc_load(u32 len, u32 max_len)
{
	if (len < rnic_srs_lp_qlen(max_len))
		return SRS_LOW;
	if (len < rnic_srs_hp_qlen(max_len))
		return SRS_MID;
	return SRS_HIGH;
}

void rnic_srs_rx_nqcc(struct rnic_srs_napi_priv_s *npriv, u32 len, u32 max_len)
{
	struct rnic_srs_cc_s *cctx = rnic_srs_cc_ctx_ptr;
	u32 load;

	if (unlikely(!cctx->nqcc_enable))
		return;

	load = rnic_srs_nqcc_calc_load(len, max_len);
	if (load != cctx->queue_load)
		rnic_srs_nqcc_update(npriv, load);
}

void rnic_srs_nqcc_rst(struct work_struct *work)
{
	struct rnic_srs_ctx_s *sctx = rnic_srs_cc_ctx_ptr->back;
	struct rnic_srs_rx_queue_s *rxq_node = NULL;
	struct list_head *tmp = NULL;

	mutex_lock(&sctx->mutex);
	list_for_each(tmp, sctx->rxq_list) {
		rxq_node = list_entry(tmp, struct rnic_srs_rx_queue_s, list);
		rnic_srs_nqcc_lp_with_lock(rxq_node->npriv);
	}
	mutex_unlock(&sctx->mutex);
}

int rnic_srs_cc_init(struct rnic_srs_ctx_s *sctx)
{
	struct rnic_srs_cc_s *cctx = NULL;

	if (rnic_srs_cc_ctx_ptr != NULL)
		return 0;

	cctx = kzalloc(sizeof(*cctx), GFP_KERNEL);
	if (cctx == NULL) {
		RNIC_LOGE("alloc cctx failed");
		return -ENOMEM;
	}

	INIT_DELAYED_WORK(&cctx->nqcc_rst_work, rnic_srs_nqcc_rst);
	rnic_srs_cc_ctx_ptr = cctx;
	rnic_srs_cc_ctx_ptr->back = sctx;

	return 0;
}

int rnic_srs_cc_exit(struct rnic_srs_ctx_s *sctx)
{
	struct rnic_srs_cc_s *cctx = rnic_srs_cc_ctx_ptr;

	if (cctx == NULL)
		return 0;

	cancel_delayed_work_sync(&cctx->nqcc_rst_work);
	kfree(cctx);
	rnic_srs_cc_ctx_ptr = NULL;

	return 0;
}
