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

#include <linux/module.h>
#include <linux/string.h>
#include <linux/notifier.h>
#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/topology.h>
#include <linux/version.h>
#include "rnic_srs.h"
#include "rnic_srs_cc.h"
#include "rnic_srs_lb.h"
#include "rnic_srs_boost.h"
#include "rnic_srs_napi.h"
#include "rnic_vnet.h"
#include "rnic_log.h"
#include "rnic_private.h"
#include "rnic_dsm_msg_pif.h"

STATIC struct rnic_srs_ctx_s *rnic_srs_ctx_ptr;
STATIC LIST_HEAD(rnic_srs_rps_rxq_list);

struct rnic_srs_ctx_s *rnic_srs_get_sctx(void)
{
	return rnic_srs_ctx_ptr;
}

STATIC void rnic_srs_set_pps_cfg(const struct rnic_vnet_fea_cfg_s *cfg)
{
	struct rnic_srs_pps_cfg_s *pps_cfg = &rnic_srs_ctx_ptr->pps_cfg;
	u32 i;

	for (i = 0; i < RNIC_PPS_LEVEL; i++) {
		pps_cfg->level_cfg[i].total_pps = cfg->level_cfg[i].total_pps;
		pps_cfg->level_cfg[i].non_tcp_pps = cfg->level_cfg[i].non_tcp_pps;
	}
}

STATIC void rnic_srs_get_pps_level(struct rnic_srs_ctx_s *sctx,
				    u32 *total_level, u32 *combin_level)
{
	struct rnic_srs_flow_stats_s *fs = &sctx->flow_stats;
	struct rnic_srs_flow_stats_s *hst_fs = &sctx->hst_flow_stats;
	struct rnic_pps_level_cfg_s *level_cfg = &sctx->pps_cfg.level_cfg[0];
	u64 total_pps, tcp_pps, non_tcp_pps;
	u32 level;

	total_pps = fs->rx_packets - hst_fs->rx_packets;
	tcp_pps = fs->rx_tcp_pkts - hst_fs->rx_tcp_pkts;
	non_tcp_pps = fs->rx_non_tcp_pkts - hst_fs->rx_non_tcp_pkts;

	for (level = 0; level < RNIC_PPS_LEVEL; level++, level_cfg++) {
		if (total_pps < level_cfg->total_pps)
			break;

		*total_level = level;
		if ((non_tcp_pps >= level_cfg->non_tcp_pps) &&
		    (tcp_pps >= level_cfg->total_pps - level_cfg->non_tcp_pps))
			*combin_level = level;
	}

	sctx->hst_flow_stats = sctx->flow_stats;
}

STATIC void rnic_srs_fs_work(struct work_struct *work)
{
	struct rnic_srs_ctx_s *sctx = container_of(work, struct rnic_srs_ctx_s,
						   fs_work);
	u32 total_level = 0;
	u32 combin_level = 0;

	rnic_srs_get_pps_level(sctx, &total_level, &combin_level);

	if (combin_level != sctx->cur_combin_level) {
		rnic_srs_set_cc_level(sctx, combin_level);
		rnic_srs_set_lb_level(combin_level);
		sctx->cur_combin_level = combin_level;
	}

	if (total_level != sctx->cur_total_level) {
		rnic_srs_set_napi_weight_level(sctx, total_level);
		sctx->cur_total_level = total_level;
	}
}

void rnic_srs_flow_stats_update(struct timer_list *t)
{
	struct rnic_srs_ctx_s *sctx = rnic_srs_ctx_ptr;

	queue_work(sctx->fs_wq, &sctx->fs_work);
	mod_timer(&sctx->fs_timer, RNIC_SRS_FS_TIMEOUT);
}

STATIC void rnic_srs_feature_cfg(const void *cfg)
{
	const struct rnic_vnet_fea_cfg_s *vcfg = (const struct rnic_vnet_fea_cfg_s *)cfg;
	struct rnic_srs_ctx_s *sctx = rnic_srs_ctx_ptr;

	rnic_srs_set_napi_cfg(vcfg);
	rnic_srs_set_pps_cfg(vcfg);
	rnic_srs_set_lb_cfg(vcfg);
	rnic_srs_set_cc_cfg(vcfg);
	mod_timer(&sctx->fs_timer, RNIC_SRS_FS_TIMEOUT);
}

STATIC void rnic_srs_set_report_stats(const struct rnic_srs_napi_priv_s *npriv,
				      struct rnic_srs_stats_rpt_s *info)
{
	struct rnic_srs_ctx_s *sctx = rnic_srs_get_sctx();
	struct rnic_srs_lb_s *lctx = rnic_srs_get_lctx();
	struct rnic_srs_cc_s *cctx = rnic_srs_get_cctx();

	info->stats.cluster_num		 = (u8)sctx->cluster_num;
	info->stats.total_level		 = (u8)sctx->cur_total_level;
	info->stats.combin_level 	 = (u8)sctx->cur_combin_level;
	info->stats.lb_level 		 = (u8)lctx->cur_level;
	info->stats.cc_level 		 = (u8)cctx->cur_level;
	info->stats.rps_core_mask	 = (u8)sctx->rps_core_mask;
	info->stats.nqcc_enable		 = (u8)cctx->nqcc_enable;
	info->stats.cqcc_enable		 = (u8)cctx->cqcc_enable;
	info->stats.cqcc_req_ddr_bd  = cctx->cqcc_last_ddr_bd;
	info->stats.nqcc_req_ddr_bd  = cctx->nqcc_last_ddr_bd;

	info->stats.napi_enable		 = (u8)npriv->napi_enable;
	info->stats.gro_enable		 = (u8)npriv->gro_enable;
	info->stats.poll_acc_enable	 = (u8)npriv->poll_acc_enable;
	info->stats.napi_weight		 = (u8)npriv->napi.weight;
	info->stats.poll_num 		 = npriv->napi_stats.poll_num;
	info->stats.poll_acc_num 	 = npriv->napi_stats.poll_acc_num;
	info->stats.napi_cmpl_num	 = npriv->napi_stats.napi_cmpl_num;
	info->stats.rx_pend_pkts 	 = npriv->napi_stats.rx_pend_pkts;
	info->stats.rx_queue_errs	 = npriv->napi_stats.rx_queue_errs;
}

STATIC void rnic_srs_report_stats_per_netdev(struct rnic_srs_napi_priv_s *npriv)
{
	struct rnic_srs_stats_rpt_s info = {{0}};

	info.hdr.ver = 101; /* version 101 */
	info.hdr.vif_id = npriv->vif_id;
	rnic_srs_set_report_stats(npriv, &info);
	rnic_trans_report(ID_DIAG_RNIC_VNET_SRS_STATS, &info, sizeof(info));
}

STATIC void rnic_srs_report_stats(void)
{
	struct rnic_srs_napi_ctx_s *nctx = rnic_srs_get_nctx();
	struct rnic_srs_napi_priv_s *npriv = NULL;
	unsigned long long pifmask = SRS_PIF_MASK;
	const unsigned long *bitmasks = (const unsigned long *)&pifmask;
	unsigned long bit;

	if (unlikely(nctx->cfg.napi_enable == 0))
		return;

	rnic_srs_boost_report_stats();

	for_each_set_bit(bit, bitmasks, (unsigned long)SRS_MAX_PIF_NUM) {
		npriv = nctx->priv[bit];
		if (!rnic_srs_napi_is_enabled(npriv))
			continue;
		rnic_srs_report_stats_per_netdev(nctx->priv[bit]);
	}
}

STATIC void rnic_srs_boost_cpu(u32 enable)
{
	struct rnic_srs_ctx_s *sctx = rnic_srs_ctx_ptr;

	if (enable != 0)
		rnic_srs_freq_qos_boost(0, sctx->freq_max[0]);
	else
		rnic_srs_freq_qos_boost(0, 0);
}

#if defined(CONFIG_RFS_RPS_MATCH)
STATIC void rnic_srs_add_rps_rxq_list(struct net_device *netdev)
{
	struct rnic_srs_ctx_s *sctx = rnic_srs_ctx_ptr;
	struct rnic_srs_napi_priv_s *npriv = rnic_srs_get_npriv(netdev);
	struct rnic_srs_rx_queue_s *new_node = NULL;

	if (unlikely(npriv == NULL))
		return;

	new_node = kzalloc(sizeof(struct rnic_srs_rx_queue_s), GFP_KERNEL);
	if (unlikely(new_node == NULL))
		return;

	new_node->rx_queue = netdev->_rx;
	new_node->npriv = npriv;

	mutex_lock(&sctx->mutex);
	list_add_tail(&new_node->list, sctx->rxq_list);
	mutex_unlock(&sctx->mutex);
}

STATIC void rnic_srs_del_rps_rxq_list(struct net_device *netdev)
{
	struct rnic_srs_ctx_s *sctx = rnic_srs_ctx_ptr;
	struct rnic_srs_rx_queue_s *rxq_node = NULL;
	struct list_head *pos = NULL;
	struct list_head *tmp = NULL;

	mutex_lock(&sctx->mutex);
	list_for_each_safe(pos, tmp, sctx->rxq_list) {
		rxq_node = list_entry(pos, struct rnic_srs_rx_queue_s, list);
		if (rxq_node->rx_queue->dev == netdev) {
			list_del(&rxq_node->list);
			kfree(rxq_node);
		}
	}
	mutex_unlock(&sctx->mutex);
}

STATIC void rnic_srs_free_rps_rx_queue(void)
{
	struct rnic_srs_ctx_s *sctx = rnic_srs_ctx_ptr;
	struct list_head *pos = sctx->rxq_list;
	struct rnic_srs_rx_queue_s *rxq_node = NULL;

	while (!list_empty(pos)) {
		rxq_node = list_entry(pos->next, struct rnic_srs_rx_queue_s, list);
		list_del(&rxq_node->list);
		kfree(rxq_node);
	}
}

STATIC void rnic_srs_set_rps_cpumask(struct rps_map *map,
				     unsigned long rps_core_mask)
{
	struct rnic_srs_ctx_s *sctx = rnic_srs_ctx_ptr;
	unsigned int last_cpu = 0;
	int cpu;

	cpumask_clear(sctx->rps_cpumask);
	for_each_online_cpu(cpu) {
		int cluster = topology_physical_package_id(cpu);
		if (test_bit(cluster, &rps_core_mask)) {
			cpumask_set_cpu((u32)cpu, sctx->rps_cpumask);
			last_cpu = (u32)cpu;
		}
	}
	cpumask_clear_cpu(0, sctx->rps_cpumask);
	cpumask_copy(&map->cpus_mask, sctx->rps_cpumask);

	/* In TCP DL 1.6Gbps scenario, rps cpu within [1, last_cpu]. */
	if (last_cpu > 0)
		map->len = last_cpu;
}

void rnic_srs_update_rps_cpumask(struct netdev_rx_queue *rxqueue,
				 unsigned long rps_core_mask)
{
	struct rps_map *map = NULL;

	if (rxqueue == NULL || rps_core_mask == 0)
		return;

	preempt_disable();
	rcu_read_lock();
	map = rcu_dereference(rxqueue->rps_map);
	if (unlikely(map == NULL)) {
		rcu_read_unlock();
		preempt_enable();
		return;
	}

	rnic_srs_set_rps_cpumask(map, rps_core_mask);
	rcu_read_unlock();
	preempt_enable();
}

void rnic_srs_update_all_rps_cpumask(unsigned long rps_core_mask)
{
	struct rnic_srs_ctx_s *sctx = rnic_srs_ctx_ptr;
	struct rnic_srs_rx_queue_s *rxq_node = NULL;
	struct list_head *pos = NULL;

	mutex_lock(&sctx->mutex);
	list_for_each(pos, sctx->rxq_list) {
		rxq_node = list_entry(pos, struct rnic_srs_rx_queue_s, list);
		rnic_srs_update_rps_cpumask(rxq_node->rx_queue, rps_core_mask);
	}
	sctx->rps_core_mask = rps_core_mask;
	mutex_unlock(&sctx->mutex);
}
#else
STATIC inline void rnic_srs_add_rps_rxq_list(struct net_device *netdev) {}
STATIC inline void rnic_srs_del_rps_rxq_list(struct net_device *netdev) {}
STATIC inline void rnic_srs_rec_rps_rx_queue(struct net_device *netdev) {}
STATIC inline void rnic_srs_free_rps_rx_queue(void) {}
inline void rnic_srs_update_rps_cpumask(struct netdev_rx_queue *rxqueue,
					unsigned long rps_core_mask) {}
inline void rnic_srs_update_all_rps_cpumask(unsigned long rps_core_mask) {}
#endif

STATIC const struct rnic_srs_ops_s rnic_srs_ops = {
	.set_feature_cfg	= rnic_srs_feature_cfg,
	.rx_skb_list		= rnic_srs_rx_skb_list,
	.rpt_stats		= rnic_srs_report_stats,
	.disable_gro		= rnic_srs_disable_gro,
	.boost_cpu		= rnic_srs_boost_cpu,
};

STATIC void rnic_srs_ops_add(struct net_device *netdev)
{
	struct rnic_vnet_priv_s *vpriv = netdev_priv(netdev);

	if (vpriv->back != NULL && vpriv->back->srs_ops == NULL)
		vpriv->back->srs_ops = &rnic_srs_ops;
}

STATIC void rnic_srs_ops_del(struct net_device *netdev)
{
	struct rnic_vnet_priv_s *vpriv = netdev_priv(netdev);

	if (vpriv->back->srs_ops != NULL)
		vpriv->back->srs_ops = NULL;
}

STATIC void rnic_srs_cluster_init(struct rnic_srs_ctx_s *sctx)
{
	int cpu;

	for_each_online_cpu(cpu) {
		int cluster = topology_physical_package_id(cpu);
		if (rnic_srs_cluster_valid(cluster)) {
			cpumask_set_cpu((u32)cpu, sctx->cluster_cpumask[cluster]);
			sctx->cluster_num = (u32) cluster + 1;
			set_bit(cluster, &sctx->rps_core_mask);
			if (cluster != 0)
				set_bit(cluster, &sctx->mb_core_mask);
			sctx->freq_max[cluster] = cpufreq_quick_get_max(cpu);
		}
	}
}

STATIC void rnic_srs_comm_cpumasks_deinit(struct rnic_srs_ctx_s *sctx)
{
	u32 i;

	for (i = 0; i < SRS_MAX_CLUSTER; i++)
		free_cpumask_var(sctx->cluster_cpumask[i]);
	free_cpumask_var(sctx->rps_cpumask);
}

STATIC void rnic_srs_comm_cpumasks_clear(struct rnic_srs_ctx_s *sctx)
{
	u32 i;

	for (i = 0; i < SRS_MAX_CLUSTER; i++)
		cpumask_clear(sctx->cluster_cpumask[i]);

	cpumask_clear(sctx->rps_cpumask);
}

STATIC int rnic_srs_comm_cpumasks_init(struct rnic_srs_ctx_s *sctx)
{
	u32 i;

	for (i = 0; i < SRS_MAX_CLUSTER; i++) {
		if (!alloc_cpumask_var(&sctx->cluster_cpumask[i], GFP_KERNEL))
			goto err_alloc_cpumask;
	}

	if (!alloc_cpumask_var(&sctx->rps_cpumask, GFP_KERNEL))
		goto err_alloc_cpumask;

	rnic_srs_comm_cpumasks_clear(sctx);
	return 0;

err_alloc_cpumask:
	for (; i > 0; i--)
		free_cpumask_var(sctx->cluster_cpumask[i - 1]);

	return -ENOMEM;
}

STATIC void rnic_srs_fs_workqueue_init(struct rnic_srs_ctx_s *sctx)
{
	INIT_WORK(&sctx->fs_work, rnic_srs_fs_work);
	sctx->fs_wq = alloc_ordered_workqueue("rnic_srs_wq", 0);
	if (sctx->fs_wq == NULL)
		RNIC_LOGE("workqueue alloc fail\n");
}

STATIC void rnic_srs_fs_workqueue_deinit(struct rnic_srs_ctx_s *sctx)
{
	if (sctx->fs_work.func != NULL)
		cancel_work_sync(&sctx->fs_work);

	if (sctx->fs_wq != NULL) {
		flush_workqueue(sctx->fs_wq);
		destroy_workqueue(sctx->fs_wq);
	}
}

STATIC int rnic_srs_comm_init(void)
{
	struct rnic_srs_ctx_s *sctx = NULL;

	sctx = kzalloc(sizeof(*sctx), GFP_KERNEL);
	if (sctx == NULL) {
		RNIC_LOGE("alloc srs ctx failed");
		goto err_alloc;
	}

	if (rnic_srs_lb_init(sctx))
		goto err_lb_init;

	if (rnic_srs_cc_init(sctx))
		goto err_cc_init;

	if (rnic_srs_comm_cpumasks_init(sctx))
		goto err_comm_cpumask_init;

	rnic_srs_cluster_init(sctx);
	rnic_srs_boost_init();
	timer_setup(&sctx->fs_timer, rnic_srs_flow_stats_update, 0);
	rnic_srs_fs_workqueue_init(sctx);
	mutex_init(&sctx->mutex);

	sctx->rxq_list = &rnic_srs_rps_rxq_list;
	rnic_srs_ctx_ptr = sctx;

	return 0;

err_comm_cpumask_init:
	rnic_srs_cc_exit(sctx);
err_cc_init:
	rnic_srs_lb_exit(sctx);
err_lb_init:
	kfree(sctx);
err_alloc:
	return -ENOMEM;
}

STATIC void rnic_srs_exit()
{
	struct rnic_srs_ctx_s *sctx = rnic_srs_ctx_ptr;

	if (sctx == NULL)
		return;

	mutex_destroy(&sctx->mutex);
	rnic_srs_fs_workqueue_deinit(sctx);
	rnic_srs_free_rps_rx_queue();
	rnic_srs_comm_cpumasks_deinit(sctx);
	rnic_srs_lb_exit(sctx);
	rnic_srs_cc_exit(sctx);
	rnic_srs_napi_exit();
	rnic_srs_boost_exit();

	kfree(sctx);
	rnic_srs_ctx_ptr = NULL;
}

STATIC int rnic_srs_init(void)
{
	if (rnic_srs_comm_init())
		goto err_comm_init;

	if (rnic_srs_napi_init(rnic_srs_ctx_ptr))
		goto err_napi_init;

	return 0;

err_napi_init:
	rnic_srs_exit();
err_comm_init:
	return -ENOMEM;
}

STATIC int rnic_srs_netdev_event(struct notifier_block *nb, unsigned long event, void *ptr)
{
	struct net_device *netdev = netdev_notifier_info_to_dev(ptr);

	if (netdev->netdev_ops->ndo_set_features != rnic_vnet_set_features) {
		RNIC_LOGI("Notification from other netdev");
		return NOTIFY_DONE;
	}

	switch (event) {
	case NETDEV_REGISTER:
		rnic_srs_napi_add(netdev);
		rnic_srs_ops_add(netdev);
		break;
	case NETDEV_UP:
		rnic_srs_add_rps_rxq_list(netdev);
		rnic_srs_update_rps_cpumask(netdev->_rx, rnic_srs_ctx_ptr->rps_core_mask);
		rnic_srs_napi_enable(netdev);
		break;
	case NETDEV_DOWN:
		rnic_srs_napi_disable(netdev);
		rnic_srs_del_rps_rxq_list(netdev);
		break;
	case NETDEV_UNREGISTER:
		rnic_srs_ops_del(netdev);
		rnic_srs_napi_del(netdev);
		break;
	default:
		break;
	}

	return NOTIFY_DONE;
}

STATIC struct notifier_block rnic_srs_netdev_notifier = {
	.notifier_call = rnic_srs_netdev_event,
};

int rnic_srs_module_init(void)
{
	int ret;

	ret = rnic_srs_init();
	if (ret) {
		RNIC_LOGE("rnic srs init failed, %d", ret);
		return ret;
	}

	ret = register_netdevice_notifier(&rnic_srs_netdev_notifier);
	if (ret)
		RNIC_LOGE("failed to register netdev notifier, %d", ret);

	return ret;
}

void rnic_srs_module_exit(void)
{
	unregister_netdevice_notifier(&rnic_srs_netdev_notifier);
	rnic_srs_exit();
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(rnic_srs_module_init);
module_exit(rnic_srs_module_exit);
#endif
