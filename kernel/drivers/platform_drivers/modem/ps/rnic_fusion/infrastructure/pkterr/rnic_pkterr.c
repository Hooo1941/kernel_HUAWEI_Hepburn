/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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

#include <linux/skbuff.h>
#include "mdrv_timer.h"
#include "mdrv_msg.h"
#include "msg_id.h"
#include "rnic_log.h"
#include "rnic_private.h"
#include "rnic_nll2_msg_pif.h"
#include "rnic_pkterr.h"

STATIC struct rnic_pkterr_ctx_s *rnic_pkterr_ctx_ptr;
STATIC struct msg_addr rnic_nll2_chan_dst = {
	.core = MSG_CORE_TSP,
	.chnid = MSG_CHN_NLL2,
};

STATIC void rnic_pkterr_rpt_err_ind(unsigned int modem_id,
				    unsigned int pdusession_id)
{
	struct rnic_nll2_msg_s msg = {0};

	msg.modem_id = modem_id;
	msg.pdusession_id = pdusession_id;
	msg.msg_length = sizeof(msg);
	msg.msg_type = ID_RNIC_NLL2_PKTERR_IND;

	if(rnic_wwan_msg_snd(&rnic_nll2_chan_dst, &msg, sizeof(msg)))
		RNIC_LOGE("pkterr rpt failed");
}

STATIC void rnic_pkterr_rpt(struct rnic_pkterr_info_s *pkterr_info,
			    unsigned int modem_id,
			    unsigned int pdusession_id)
{
	rnic_pkterr_rpt_err_ind(modem_id, pdusession_id);

	pkterr_info->cur_pkterr_num = 0;
	pkterr_info->total_pkterr_num = 0;
	pkterr_info->period_idx = 0;
	pkterr_info->period_start_time = 0;
	pkterr_info->num_start_time = 0;
}

STATIC void rnic_pkterr_record_err(unsigned int modem_id, unsigned int pdusession_id)
{
	struct rnic_pkterr_ctx_s *pctx = rnic_pkterr_ctx_ptr;
	struct rnic_pkterr_info_s *pkterr_info;
	unsigned int cur_time = mdrv_timer_get_normal_timestamp();
	unsigned int period_cycle, num_cycle;

	if (modem_id >= RNIC_MODEM_ID_MAX || pdusession_id >= RNIC_PDUSESSION_ID_MAX)
		return;

	pkterr_info = &rnic_pkterr_ctx_ptr->pkterr_info[modem_id][pdusession_id];
	pkterr_info->cur_pkterr_num++;
	pkterr_info->total_pkterr_num++;

	// The first error packet starts counting statistics.
	if (pkterr_info->total_pkterr_num == 1) {
		pkterr_info->period_idx = 1;
		pkterr_info->num_start_time = cur_time;
		pkterr_info->period_start_time = cur_time;
		return;
	}

	period_cycle = rnic_caculate_gap_time(cur_time, pkterr_info->period_start_time) /
					      pctx->fea_cfg.pkterr_period;

	switch (period_cycle) {
	case 0:
		goto num_gap;
	case 1:
		if (++pkterr_info->period_idx >= pctx->fea_cfg.pkterr_cycle)
			return rnic_pkterr_rpt(pkterr_info, modem_id, pdusession_id);

		// goto the next period
		pkterr_info->cur_pkterr_num = 1;
		pkterr_info->period_start_time += pctx->fea_cfg.pkterr_period;
		goto num_gap;

	default:
		// restart period_idx and period_start_time
		pkterr_info->period_idx = 1;
		pkterr_info->cur_pkterr_num = 1;
		pkterr_info->period_start_time = cur_time;
	}

num_gap:
	num_cycle = rnic_caculate_gap_time(cur_time, pkterr_info->num_start_time) /
					   pctx->fea_cfg.pkterr_period;
	if (num_cycle >= pctx->fea_cfg.pkterr_cycle) {
		pkterr_info->total_pkterr_num = pkterr_info->cur_pkterr_num;
		pkterr_info->num_start_time = pkterr_info->period_start_time;
	}

	if (pkterr_info->total_pkterr_num >= pctx->fea_cfg.pkterr_num_threshhold)
		rnic_pkterr_rpt(pkterr_info, modem_id, pdusession_id);
}

STATIC unsigned int rnic_pkterr_judge_err(struct rx_cb_map_s *map,
					  unsigned int ipf_err_mask)
{
	int result;

	result = map->pfa_tft_result.u32 & ipf_err_mask;
	result |= map->pfa_result.u32 == RNIC_SPE_L3_PORTOCOL_ERR;
	result |= map->pfa_result.u32 == RNIC_SPE_L3_IPHDR_CHECK_ERR;
	result |= map->pfa_result.u32 == RNIC_SPE_IP_IPHDR_LEN_ERR;

	return result;
}

void rnic_pkterr_statistic_err(struct sk_buff *skb, pfa_version_type pfa_ver)
{
	struct rx_cb_map_s *map = NULL;
	struct rnic_pkterr_para_s {
		unsigned int ipf_err_mask;
		unsigned int modem_id;
		unsigned int pdusession_id;
	} temp_para;

	if (!(BIT_ULL(rnic_map_pifid(skb)) & RNIC_PKTERR_PIFMASK) ||
	    rnic_map_pkttype(skb) != RNIC_MAP_PKTTYPE_IP)
		return;

	map = rnic_map_skb_cb(skb);
	switch (pfa_ver) {
	case PFA_VERSION_V0:
		temp_para.ipf_err_mask = RNIC_IPF_ERR_MASK_V0;
		temp_para.modem_id = map->pfa_tft_result.bits.modem_id;
		temp_para.pdusession_id = map->pfa_tft_result.bits.pdu_session_id;
		break;

	case PFA_VERSION_V100:
		temp_para.ipf_err_mask = RNIC_IPF_ERR_MASK_V100;
		temp_para.modem_id = map->pfa_tft_result.bits_v100.modem_id;
		temp_para.pdusession_id = map->pfa_tft_result.bits_v100.pdu_session_id;
		break;

	case PFA_VERSION_V200:
	default:
		return;
	}

	if (rnic_pkterr_judge_err(map, temp_para.ipf_err_mask))
		rnic_pkterr_record_err(temp_para.modem_id, temp_para.pdusession_id);
}

void rnic_pkterr_clear_err_data(unsigned int modem_id,
				unsigned int pdusession_id)
{
	struct rnic_pkterr_ctx_s *pctx = rnic_pkterr_ctx_ptr;
	struct rnic_pkterr_info_s *pkterr_info;

	if (rnic_pkterr_ctx_ptr == NULL) {
		RNIC_LOGE("pkterr init is not ready");
		return;
	}

	if (modem_id >= RNIC_MODEM_ID_MAX || pdusession_id >= RNIC_PDUSESSION_ID_MAX) {
		RNIC_LOGE("clear_err_data: invalid modem_id or pdusession_id");
		return;
	}

	pkterr_info = &pctx->pkterr_info[modem_id][pdusession_id];
	pkterr_info->cur_pkterr_num = 0;
	pkterr_info->total_pkterr_num = 0;
	pkterr_info->period_idx = 0;
	pkterr_info->period_start_time = 0;
	pkterr_info->num_start_time = 0;
}

void rnic_pkterr_fea_cfg(unsigned short period, unsigned short cycle,
			 unsigned int num_threshhold)
{
	struct rnic_pkterr_ctx_s *pctx = rnic_pkterr_ctx_ptr;

	if (rnic_pkterr_ctx_ptr == NULL) {
		RNIC_LOGE("pkterr init is not ready");
		return;
	}

	pctx->fea_cfg.pkterr_cycle = (unsigned int )cycle;
	pctx->fea_cfg.pkterr_num_threshhold = num_threshhold;
	pctx->fea_cfg.pkterr_period = RNIC_MS_TO_TIME_STAMP(period,
		pctx->timestamp_freq);
}

int rnic_pkterr_init(void)
{
	struct rnic_pkterr_ctx_s *pctx= NULL;

	pctx= kzalloc(sizeof(*pctx), GFP_KERNEL);
	if (pctx== NULL) {
		RNIC_LOGE("alloc pkterr resource failed");
		return -ENOMEM;
	}

	rnic_pkterr_ctx_ptr = pctx;
	pctx->timestamp_freq = mdrv_get_normal_timestamp_freq();

	pctx->fea_cfg.pkterr_cycle = RNIC_PKTERR_CYCLE;
	pctx->fea_cfg.pkterr_num_threshhold = RNIC_PKTERR_NUM_THRESHHOLD;
	pctx->fea_cfg.pkterr_period = RNIC_MS_TO_TIME_STAMP(RNIC_PKTERR_PERIOD,
		pctx->timestamp_freq);

	return 0;
}

void rnic_pkterr_deinit(void)
{
	if (rnic_pkterr_ctx_ptr == NULL)
		return;

	kfree(rnic_pkterr_ctx_ptr);
	rnic_pkterr_ctx_ptr = NULL;
}

void rnic_pkterr_set_fea_cfg_stub(unsigned short period, unsigned short cycle,
			 	  unsigned int num_threshhold)
{
	struct rnic_pkterr_ctx_s *pctx = rnic_pkterr_ctx_ptr;

	pctx->fea_cfg.pkterr_cycle = (unsigned int )cycle;
	pctx->fea_cfg.pkterr_num_threshhold = num_threshhold;
	pctx->fea_cfg.pkterr_period = RNIC_MS_TO_TIME_STAMP(period,
		pctx->timestamp_freq);

	RNIC_LOGE("period : %d, cycle : %d, num_threshhold : %d",
		period, cycle, num_threshhold);
}

void rnic_pkterr_show_status(unsigned int modem_id,
			     unsigned int pdusession_id)
{
	struct rnic_pkterr_ctx_s *pctx = rnic_pkterr_ctx_ptr;
	struct rnic_pkterr_info_s *info = NULL;

	if (modem_id >= RNIC_MODEM_ID_MAX || pdusession_id >= RNIC_PDUSESSION_ID_MAX) {
		RNIC_LOGE("pkterr show status: invalid modem_id or pdusession_id");
		return;
	}

	info = &pctx->pkterr_info[modem_id][pdusession_id];
	pr_err("[rnic] pkterr_info period_idx %d \n", info->period_idx);
	pr_err("[rnic] pkterr_info cur_pkterr_num %d \n", info->cur_pkterr_num);
	pr_err("[rnic] pkterr_info total_pkterr_num %u \n", info->total_pkterr_num);
	pr_err("[rnic] pkterr_info num_start_time %u \n", info->num_start_time);
	pr_err("[rnic] pkterr_info period_start_time %u \n", info->period_start_time);
	pr_err("[rnic] pkterr_info num_start_time %u \n", info->num_start_time);

	pr_err("[rnic] pkterr_fea pkterr_cycle %u \n", pctx->fea_cfg.pkterr_cycle);
	pr_err("[rnic] pkterr_fea pkterr_period %u \n",  pctx->fea_cfg.pkterr_period);
	pr_err("[rnic] pkterr_fea pkterr_num_threshhold %u \n",  pctx->fea_cfg.pkterr_num_threshhold);
}
