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

#include "rnic_srs.h"
#include "rnic_srs_boost.h"
#include "rnic_srs_cc.h"
#include "rnic_srs_lb.h"
#include "rnic_srs_napi.h"

void rnic_srs_dbg_set_cpufreq(u32 freq0, u32 freq1)
{
	struct rnic_srs_cc_s *cctx = rnic_srs_get_cctx();

	if (cctx == NULL) {
		pr_err("[nas_rnic] cctx is NULL\n");
		return;
	}

	rnic_srs_freq_qos_boost(0, freq0);
	rnic_srs_freq_qos_boost(1, freq1);
}

void rnic_srs_dbg_set_lb_cfg(u32 level, u32 weight0, u32 weight1)
{
	struct rnic_srs_lb_s *lctx = rnic_srs_get_lctx();

	if (level >= SRS_PPS_LEVEL)
		return;

	lctx->cfg.cluster_weight[level][0] = weight0;
	lctx->cfg.cluster_weight[level][1] = weight1;

	pr_err("[nas_rnic] lb cluster_weight[%d][0] = %d\n", level, lctx->cfg.cluster_weight[level][0]);
	pr_err("[nas_rnic] lb cluster_weight[%d][1] = %d\n", level, lctx->cfg.cluster_weight[level][1]);
}

void rnic_srs_dbg_set_napi_cfg(u32 pif_id, u32 weight_adj, u32 weight)
{
	struct rnic_srs_napi_ctx_s *nctx = rnic_srs_get_nctx();
	struct rnic_srs_napi_priv_s *npriv = NULL;

	if (nctx == NULL || pif_id >= SRS_MAX_PIF_NUM) {
		pr_err("[nas_rnic] nctx is NULL or invalid pif_id %d\n", pif_id);
		return;
	}

	npriv = nctx->priv[pif_id];
	if (npriv == NULL) {
		pr_err("[nas_rnic] napi ctx of pif_id %d is null\n", pif_id);
		return;
	}
	nctx->cfg.weight_adj = weight_adj;
	npriv->napi_weight = weight;
	npriv->napi.weight = (int)weight;
	pr_err("[nas_rnic] napi weight_adj      %d\n", nctx->cfg.weight_adj);
	pr_err("[nas_rnic] npriv->napi.weight   %d\n", npriv->napi.weight);
}

void rnic_srs_show_cc_cfg(void)
{
	struct rnic_srs_cc_s *cctx = rnic_srs_get_cctx();
	struct rnic_nqcc_cfg_s *nqcc_cfg = NULL;
	struct rnic_cqcc_cfg_s *cqcc_cfg = NULL;
	u32 i, j;

	if (cctx == NULL) {
		pr_err("[nas_rnic] cctx is NULL\n");
		return;
	}

	nqcc_cfg = &cctx->cfg.nqcc_cfg;
	pr_err("[nas_rnic] nqcc ddr_req_enable    = %d\n", nqcc_cfg->ddr_req_enable);
	pr_err("[nas_rnic] nqcc rps_mask_enable   = %d\n", nqcc_cfg->rps_mask_enable);
	pr_err("[nas_rnic] nqcc ddr_mid_bd        = %d\n", nqcc_cfg->ddr_mid_bd);
	pr_err("[nas_rnic] nqcc ddr_high_bd       = %d\n", nqcc_cfg->ddr_high_bd);
	pr_err("[nas_rnic] nqcc timeout           = %d\n", nqcc_cfg->timeout);

	for (i = 0; i < SRS_PPS_LEVEL; i++) {
		cqcc_cfg = &cctx->cfg.cqcc_cfg[i];
		pr_err("[nas_rnic] cqcc[%d] enable        = %d\n", i, cqcc_cfg->enable);
		pr_err("[nas_rnic] cqcc[%d] rps_core_mask = %d\n", i, cqcc_cfg->rps_core_mask);
		pr_err("[nas_rnic] cqcc[%d] isolation_dis = %d\n", i, cqcc_cfg->isolation_dis);
		pr_err("[nas_rnic] cqcc[%d] backlog_limit = %d\n", i, cqcc_cfg->backlog_len_limit);
		pr_err("[nas_rnic] cqcc[%d] ddr_bd        = %d\n", i, cqcc_cfg->ddr_bd);
		for (j = 0; j < SRS_MAX_CLUSTER; j++)
			pr_err("[nas_rnic] cqcc[%d] cpufreq[%d]   = %d\n", i, j, cqcc_cfg->cpufreq[j]);
	}
}

void rnic_srs_show_cc_info(void)
{
	struct rnic_srs_cc_s *cctx = rnic_srs_get_cctx();
	u32 i;

	if (cctx == NULL) {
		pr_err("[nas_rnic] cctx is NULL\n");
		return;
	}

	pr_err("[nas_rnic] cc cqcc_enable     = %d\n", cctx->cqcc_enable);
	pr_err("[nas_rnic] cc cur_level       = %d\n", cctx->cur_level);
	pr_err("[nas_rnic] cc queue_load      = %d\n", cctx->queue_load);
	for (i = 0; i < SRS_MAX_CLUSTER; i++)
		pr_err("[nas_rnic] cc req_cpufreq[%d]  = %d\n", i, rnic_srs_get_last_req_cpufreq(i));
	pr_err("[nas_rnic] cc cqcc_req_ddr_bd = %d\n", cctx->cqcc_last_ddr_bd);
	pr_err("[nas_rnic] cc nqcc_req_ddr_bd = %d\n", cctx->nqcc_last_ddr_bd);
	pr_err("[nas_rnic] cc wt_exp_num      = %d\n", cctx->cc_stats.cc_wt_exp_num);
	pr_err("[nas_rnic] cc cc_wt_rst_num   = %d\n", cctx->cc_stats.cc_wt_rst_num);
}

void rnic_srs_show_napi_cfg(u32 vnet_id)
{
	struct rnic_srs_napi_ctx_s *nctx = rnic_srs_get_nctx();
	struct rnic_srs_napi_priv_s *npriv = NULL;
	struct rnic_srs_napi_cfg_s *ncfg = NULL;
	u32 i;

	if (nctx == NULL || vnet_id >= SRS_MAX_PIF_NUM) {
		pr_err("[nas_rnic] nctx is NULL or invalid vnet_id %d\n", vnet_id);
		return;
	}

	npriv = nctx->priv[vnet_id];
	if (npriv == NULL) {
		pr_err("[nas_rnic] napi ctx of vnet %d is null\n", vnet_id);
		return;
	}

	ncfg = &nctx->cfg;
	pr_err("[nas_rnic] napi weight_adj      %d\n", ncfg->weight_adj);
	for (i = 0; i < SRS_PPS_LEVEL; i++)
		pr_err("[nas_rnic] napi weight_level[%d] = %d\n", i, ncfg->weight_level[i]);
}

void rnic_srs_show_napi_info(u32 pif_id)
{
	struct rnic_srs_napi_ctx_s *nctx = rnic_srs_get_nctx();
	struct rnic_srs_napi_priv_s *npriv = NULL;
#ifdef CONFIG_RPS
	struct netdev_rx_queue *rx_queue = NULL;
	struct rps_map *map = NULL;
#endif

	if (pif_id >= SRS_MAX_PIF_NUM) {
		pr_err("[nas_rnic] invalid pif_id %d\n", pif_id);
		return;
	}

	npriv = nctx->priv[pif_id];
	if (npriv == NULL) {
		pr_err("[nas_rnic] napi ctx of pif_id %d is NULL\n", pif_id);
		return;
	}

	pr_err("[nas_rnic] napi_enable        = %d\n", npriv->napi_enable);
	pr_err("[nas_rnic] gro_enable         = %d\n", npriv->gro_enable);
	pr_err("[nas_rnic] napi weight        = %d\n", npriv->napi.weight);
	pr_err("[nas_rnic] que_len_max        = %d\n", npriv->que_len_max);
	pr_err("[nas_rnic] poll_acc_enable    = %d\n", npriv->poll_acc_enable);
	pr_err("[nas_rnic] poll_max_num       = %d\n", npriv->poll_max_num);
	pr_err("[nas_rnic] napi_cpu           = %d\n", atomic_read(&npriv->napi_cpu));
	pr_err("[nas_rnic] input_queue len    = %d\n", skb_queue_len(&npriv->input_queue));
	pr_err("[nas_rnic] napi_queue len     = %d\n", skb_queue_len(&npriv->napi_queue));
	pr_err("[nas_rnic] rps_pend_queue len = %d\n", skb_queue_len(&npriv->rps_pend_queue));
	pr_err("[nas_rnic] rx_pend_pkts       = %d\n", npriv->napi_stats.rx_pend_pkts);
	pr_err("[nas_rnic] rx_queue_errs      = %d\n", npriv->napi_stats.rx_queue_errs);
	pr_err("[nas_rnic] poll_num           = %d\n", npriv->napi_stats.poll_num);
	pr_err("[nas_rnic] poll_acc_num       = %d\n", npriv->napi_stats.poll_acc_num);
	pr_err("[nas_rnic] napi_cmpl_num      = %d\n", npriv->napi_stats.napi_cmpl_num);
#ifdef CONFIG_RPS
	rx_queue = npriv->netdev->_rx;
	map = rcu_dereference(rx_queue->rps_map);
	if (map == NULL)
		return;
#ifdef CONFIG_RFS_RPS_MATCH
	pr_err("[nas_rnic] map->cpus_mask     = 0x%x\n", *cpumask_bits(&map->cpus_mask));
#endif
	pr_err("[nas_rnic] map->len           = %d\n", map->len);
#endif
}

void rnic_srs_show_lb_cfg(void)
{
	struct rnic_srs_lb_s *lctx = rnic_srs_get_lctx();
	u32 i, j;

	if (lctx == NULL) {
		pr_err("[nas_rnic] lctx is NULL\n");
		return;
	}

	pr_err("[nas_rnic] lb enable    = %d\n", lctx->cfg.enable);
	for (i = 0; i < SRS_PPS_LEVEL; i++) {
		for (j = 0; j < SRS_MAX_CLUSTER; j++)
			pr_err("[nas_rnic] lb cluster_weight[%d][%d] = %d\n",
			       i, j, lctx->cfg.cluster_weight[i][j]);
	}
}

void rnic_srs_show_lb_info(void)
{
	struct rnic_srs_lb_s *lctx = rnic_srs_get_lctx();
	u32 i;

	if (lctx == NULL) {
		pr_err("[nas_rnic] lctx is NULL\n");
		return;
	}

	pr_err("[nas_rnic] lb cur_level     = %d\n", lctx->cur_level);
	pr_err("[nas_rnic] lb tcpu          = %d\n", atomic_read(&lctx->tcpu));
	for (i = 0; i < SRS_MAX_CLUSTER; i++)
		pr_err("[nas_rnic] lb weight_cfg[%d] = %d\n", i, lctx->weight_cfg[i]);
	for (i = 0; i < SRS_MAX_CLUSTER; i++)
		pr_err("[nas_rnic] lb weight_sel[%d] = %d\n", i, lctx->weight_sel[i]);
	for (i = 0; i < NR_CPUS; i++)
		pr_err("[nas_rnic] lb cpu:%08x, online:%08x, down:%08x\n", lctx->lb_stats[i].lb_cpu,
			lctx->lb_stats[i].hp_online_num, lctx->lb_stats[i].hp_down_num);
}

void rnic_srs_show_comm_info(void)
{
	struct rnic_srs_ctx_s *sctx = rnic_srs_get_sctx();

	if (sctx == NULL) {
		pr_err("[nas_rnic] sctx is NULL\n");
		return;
	}

	pr_err("[nas_rnic] cluster_num       = %d\n", sctx->cluster_num);
	pr_err("[nas_rnic] rps_core_mask     = 0x%x\n", sctx->rps_core_mask);
	pr_err("[nas_rnic] mb_core_mask      = 0x%x\n", sctx->mb_core_mask);
	pr_err("[nas_rnic] cur_total_level   = %d\n", sctx->cur_total_level);
	pr_err("[nas_rnic] cur_combin_level  = %d\n", sctx->cur_combin_level);
}

void rnic_srs_show_pm_qos_info(void)
{
	struct rnic_srs_pm_qos_s *pool = rnic_srs_get_pm_qos_pool();
	u32 i;

	if (pool == NULL)
		return;

	for (i = 0; i < RNIC_PM_QOS_MAX; i++)
		pr_err("[nas_rnic] pm qos val[%d]: %d\n", i, pool[i].value);
}

void rnic_srs_show_freq_qos_info(void)
{
	struct rnic_srs_freq_qos_s *pool = rnic_srs_get_freq_qos_pool();
	u32 i;

	if (pool == NULL)
		return;

	for (i = 0; i < RNIC_MAX_FREQ_CPU_CLUSTERS; i++)
		pr_err("[nas_rnic] freq qos val[%d]: %d\n", i, pool[i].value);
}

void rnic_srs_show_boost_info(void)
{
	rnic_srs_show_pm_qos_info();
	rnic_srs_show_freq_qos_info();
}
