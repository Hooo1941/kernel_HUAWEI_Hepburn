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

#ifndef RNIC_SRS_CC_H
#define RNIC_SRS_CC_H

#include <linux/skbuff.h>
#include "rnic_srs.h"
#include "rnic_srs_napi.h"
#include "rnic_dsm_msg_pif.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SRS_NAPI_MAX_WT		64

#define rnic_srs_lp_qlen(len)	((len) >> 2)
#define rnic_srs_hp_qlen(len)	((len) >> 1)

enum rnic_srs_que_load_state_e {
	SRS_LOW = 0,
	SRS_MID = 1,
	SRS_HIGH
};

struct rnic_cc_stats_s {
	u32 cc_wt_exp_num;
	u32 cc_wt_rst_num;
};

struct rnic_cqcc_cfg_s {
	u32 enable;
	u32 rps_core_mask;
	u32 isolation_dis;
	u32 backlog_len_limit;
	u32 ddr_bd;
	u32 cpufreq[SRS_MAX_CLUSTER];
};

struct rnic_nqcc_cfg_s {
	u32 ddr_req_enable;
	u32 rps_mask_enable;
	u32 ddr_mid_bd;
	u32 ddr_high_bd;
	u32 timeout;
};

struct rnic_cc_cfg_s {
	struct rnic_nqcc_cfg_s nqcc_cfg;
	struct rnic_cqcc_cfg_s cqcc_cfg[SRS_PPS_LEVEL];
};

struct rnic_srs_cc_s {
	struct rnic_srs_ctx_s *back;
	struct rnic_cc_stats_s cc_stats;
	struct rnic_cc_cfg_s cfg;

	u32 cc_enable;
	u32 nqcc_enable;
	u32 cqcc_enable;
	enum rnic_srs_que_load_state_e queue_load;
	u32 cur_level;
	u32 cqcc_last_ddr_bd;
	u32 nqcc_last_ddr_bd;
	u32 backlog_len_limit;
	unsigned long rps_pend_cpu_bitmask;
	struct delayed_work nqcc_rst_work;
};

void rnic_srs_set_cc_cfg(const struct rnic_vnet_fea_cfg_s *cfg);
void rnic_srs_set_cc_level(struct rnic_srs_ctx_s *sctx, u32 level);
bool rnic_srs_rx_cqcc(struct net_device *netdev, struct sk_buff *skb);
void rnic_srs_cqcc_clear_pend(void);
void rnic_srs_rx_nqcc(struct rnic_srs_napi_priv_s *npriv, u32 len, u32 max_len);
int rnic_srs_cc_init(struct rnic_srs_ctx_s *sctx);
int rnic_srs_cc_exit(struct rnic_srs_ctx_s *sctx);
struct rnic_srs_cc_s *rnic_srs_get_cctx(void);

#ifdef __cplusplus
}
#endif

#endif /* RNIC_SRS_CC_H */
