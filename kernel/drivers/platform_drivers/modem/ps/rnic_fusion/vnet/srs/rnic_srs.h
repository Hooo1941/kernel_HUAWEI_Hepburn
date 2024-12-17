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

#ifndef RNIC_SRS_H
#define RNIC_SRS_H

#include <linux/cpumask.h>
#include <linux/netdevice.h>
#include <linux/timer.h>
#include <linux/mutex.h>
#include "rnic_data.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SRS_PPS_LEVEL		16
#define SRS_MAX_CLUSTER		3
#define SRS_MAX_CPU		12
#define RNIC_SRS_FS_TIMEOUT	(jiffies + msecs_to_jiffies(1000))

struct rnic_srs_stats_s {
	u8 cluster_num;
	u8 total_level;
	u8 combin_level;
	u8 lb_level;
	u8 cc_level;
	u8 rps_core_mask;
	u8 nqcc_enable;
	u8 cqcc_enable;
	u8 napi_enable;
	u8 gro_enable;
	u8 napi_weight;
	u8 poll_acc_enable;
	u32 poll_num;
	u32 poll_acc_num;
	u32 napi_cmpl_num;
	u64 rx_pend_pkts;
	u64 rx_queue_errs;
	u32 cqcc_req_ddr_bd;
	u32 nqcc_req_ddr_bd;
};

struct rnic_srs_stats_rpt_s {
	struct {
		u8 ver;
		u8 vif_id;
		u8 reserved[6];
	} hdr;
	struct rnic_srs_stats_s stats;
};

struct rnic_srs_rx_queue_s {
	struct list_head list;
	struct netdev_rx_queue *rx_queue;
	struct rnic_srs_napi_priv_s *npriv;
};

struct rnic_pps_level_cfg_s {
	u32 total_pps;
	u32 non_tcp_pps;
};

struct rnic_srs_pps_cfg_s {
	struct rnic_pps_level_cfg_s level_cfg[SRS_PPS_LEVEL];
};

struct rnic_srs_flow_stats_s {
	u64 rx_packets;
	u64 tx_packets;
	u64 rx_tcp_pkts;
	u64 rx_non_tcp_pkts;
};

struct rnic_srs_ctx_s {
	struct list_head *rxq_list;
	struct rnic_srs_pps_cfg_s pps_cfg;
	struct timer_list fs_timer;
	struct workqueue_struct *fs_wq;
	struct work_struct fs_work;
	struct rnic_srs_flow_stats_s flow_stats;
	struct rnic_srs_flow_stats_s hst_flow_stats;
	u32 cur_total_level;
	u32 cur_combin_level;

	u32 cluster_num;
	u32 freq_max[SRS_MAX_CLUSTER];
	unsigned long rps_core_mask;
	unsigned long mb_core_mask;
	cpumask_var_t cluster_cpumask[SRS_MAX_CLUSTER] ____cacheline_aligned;
	cpumask_var_t rps_cpumask;
	struct mutex mutex;
};

void rnic_srs_update_rps_cpumask(struct netdev_rx_queue *rxqueue,
				 unsigned long rps_core_mask);
void rnic_srs_update_all_rps_cpumask(unsigned long rps_core_mask);
struct rnic_srs_ctx_s *rnic_srs_get_sctx(void);

#ifdef __cplusplus
}
#endif

#endif /* RNIC_SRS_H */
