/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * foss@huawei.com
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
 *
 */

#ifndef RNIC_DSM_MSG_PIF_H
#define RNIC_DSM_MSG_PIF_H

#ifdef __cplusplus
extern "C" {
#endif

#define RNIC_MSG_TYPE_BASE  0x0000
#define RNIC_MAX_CLUSTER    3
#define RNIC_PPS_LEVEL      16

/* 0x0000 - 0x1000 */
enum rnic_dsm_msg_type_e {
	/* _MSGPARSE_Interception ELF_RNIC_DATA_LINK_CFG_IND_STRU */
	ID_RNIC_DATA_LINK_CFG_IND   = RNIC_MSG_TYPE_BASE + 0x01,
	/* _MSGPARSE_Interception ELF_RNIC_DATA_LINK_REL_IND_STRU */
	ID_RNIC_DATA_LINK_REL_IND   = RNIC_MSG_TYPE_BASE + 0x02,
	/* _MSGPARSE_Interception ELF_RNIC_VNET_FEA_CFG_IND_STRU */
	ID_RNIC_VNET_FEA_CFG_IND    = RNIC_MSG_TYPE_BASE + 0x03,
	ID_RNIC_MSG_TYPE_BUTT
};

enum rnic_pdu_sess_type_s {
	RNIC_PDU_SESS_TYPE_STUB   = 0,
	RNIC_PDU_SESS_TYPE_IPV4   = 1,
	RNIC_PDU_SESS_TYPE_IPV6   = 2,
	RNIC_PDU_SESS_TYPE_IPV4V6 = 3,
	RNIC_PDU_SESS_TYPE_ETHER  = 4,
	RNIC_PDU_SESS_TYPE_MBS    = 8,
	RNIC_PDU_SESS_TYPE_BUTT
};

struct rnic_data_link_cfg_s {
	unsigned char ps_iface_id;
	unsigned char pdu_sess_type;
	unsigned char pdu_sess_id;
	unsigned char modem_id;
	unsigned char fc_head;
	unsigned char copy_en;
	unsigned char reserved[2];
};

struct rnic_vnet_level_cfg_s {
	unsigned int total_pps;
	unsigned int non_tcp_pps;
	unsigned char napi_weight;
	unsigned char lb_weight[RNIC_MAX_CLUSTER];
	unsigned char cqcc_enable;
	unsigned char rps_core_mask;
	unsigned char isolation_disable;
	unsigned char reserved0;
	unsigned int backlog_len_limit;
	unsigned int ddr_bd;
	unsigned int cpufreq[RNIC_MAX_CLUSTER];
	unsigned int reserved1;
	unsigned int reserved2;
	unsigned int reserved3;
};

struct rnic_vnet_fea_cfg_s {
	unsigned char napi_enable;
	unsigned char gro_enable;
	unsigned char napi_weight_adj;
	unsigned char napi_weight;
	unsigned char napi_que_len_max;
	unsigned char napi_poll_acc_enable;
	unsigned char napi_poll_max;
	unsigned char lb_enable;
	unsigned char cc_enable;
	unsigned char nqcc_enable;
	unsigned char nqcc_ddr_req_enable;
	unsigned char nqcc_rps_mask_enable;
	unsigned int nqcc_timeout;
	unsigned int ddr_mid_bd;
	unsigned int ddr_high_bd;
	unsigned short pkterr_period;
	unsigned short pkterr_cycle;
	unsigned int pkterr_num_threshhold;
	unsigned int reserved;
	struct rnic_vnet_level_cfg_s level_cfg[RNIC_PPS_LEVEL];
};

struct rnic_dsm_msg_s {
	unsigned int msg_type;
	unsigned int args_length;
	union {
		unsigned int args_start;
		struct rnic_data_link_cfg_s data_cfg;
		struct rnic_vnet_fea_cfg_s vnet_fea_cfg;
	};
};

#define RNIC_DSM_MSG_OFFSETOF(_m)   offsetof(struct rnic_dsm_msg_s, _m)
#define RNIC_DSM_MSG_ARGS_SIZE(_m)  sizeof(((struct rnic_dsm_msg_s *)0)->_m)

#define RNIC_DSM_MSG_FIX_LEN        sizeof(struct rnic_dsm_msg_s)
#define RNIC_DSM_MSG_HDR_LEN        RNIC_DSM_MSG_OFFSETOF(args_start)
#define RNIC_DSM_MSG_MAX_PAYLOAD    (sizeof(struct rnic_dsm_msg_s) - RNIC_DSM_MSG_HDR_LEN)

#ifdef __cplusplus
}
#endif

#endif /* RNIC_DSM_MSG_PIF_H */
