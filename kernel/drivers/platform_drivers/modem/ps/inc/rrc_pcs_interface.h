/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 * *	notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *	notice, this list of conditions and the following disclaimer in the
 * *	documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *	be used to endorse or promote products derived from this software
 * *	without specific prior written permission.
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
#ifndef RRC_PCS_INTERFACE_H
#define RRC_PCS_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 4)

/*
 * The following is used in cross-core messages, so the bit width occupied
 * by the enumeration members in the structure must be explicitly defined,
 * the enumeration type of the member can be searched according to the name.
 */

#define NRRC_PCS_MAX_PLMN_ID_NUM 16

#define RRC_PCS_MAX_FILE_DIR_LEN 255
#define PCS_RRC_IND_DATA_MAX_LEN 2000

enum rrc_pcs_msg_type {
	NRRC_PCS_BC_CAP_REQ = 0,
	PCS_NRRC_BC_CAP_IND = 1,
	NRRC_PCS_BC_CAP_ACK = 2,
	NRRC_PCS_MSG_BUTT,

	RRC_PCS_READ_FILE_REQ = 10,
	PCS_RRC_READ_FILE_IND,
	RRC_PCS_READ_FILE_ACK,

	PCS_RRC_FILE_UPDATE_NTF = 1000,

	RRC_PCS_MSG_BUTT,
};

enum rrc_pcs_file_type {
	/* NRRC */
	NRRC_PCS_FILE_BASE = 0,
	NRRC_PCS_FILE_SAME_SET = NRRC_PCS_FILE_BASE,
	NRRC_PCS_FILE_BC_DATA,

	RRC_PCS_FILE_BUTT
};

enum pcs_rrc_read_file_ind_rslt {
	PCS_RRC_READ_FILE_IND_SUCC = 0,
	PCS_RRC_READ_FILE_IND_NOT_EXIST,
	PCS_RRC_READ_FILE_IND_OTHER_FAIL,
	PCS_RRC_READ_FILE_IND_BUTT
};

enum rrc_pcs_read_file_ack_rslt {
	RRC_PCS_READ_FILE_ACK_SUCC = 0,
	RRC_PCS_READ_FILE_ACK_FAIL,
	RRC_PCS_READ_FILE_ACK_BUTT
};

enum pcs_rrc_file_update_ntf_type {
	/* NRRC */
	PCS_NRRC_FILE_UPDATE_NTF_BASE = 0,
	PCS_NRRC_SAME_SET_FILE_UPDATE_NTF = PCS_NRRC_FILE_UPDATE_NTF_BASE,

	PCS_NRRC_FILE_UPDATE_NTF_BUTT
};

enum nrrc_pcs_sim_status {
	NRRC_PCS_SINGLE_CARD = 0,
	NRRC_PCS_5_4_DUAL_CARD_MASTER = 1,
	NRRC_PCS_5_5_DUAL_CARD_MASTER = 2,
	NRRC_PCS_5_5_DUAL_CARD_SLAVE = 3,
	NRRC_PCS_SIM_STATUS_BUTT
};

enum nrrc_pcs_bc_type {
	NRRC_PCS_BC_NRCA = 0,
	NRRC_PCS_BC_ENDC = 1,
	NRRC_PCS_BC_TX_SW_NRCA = 2,
	NRRC_PCS_BC_TX_SW_ENDC = 3,
	NRRC_PCS_BC_DAPS = 4,
	NRRC_PCS_BC_TYPE_BUTT
};

enum pcs_nrrc_cap_ind_type {
	PCS_NRRC_CAP_IND_NOT_SAME = 0,
	PCS_NRRC_CAP_IND_SAME = 1, /* reserved */
	PCS_NRRC_CAP_IND_NOT_SUPP = 2,
	PCS_NRRC_CAP_IND_TYPE_BUTT
};

enum nrrc_pcs_ack_rslt {
	NRRC_PCS_ACK_SUCC = 0,
	NRRC_PCS_ACK_OTHER_FAIL = 1,
	NRRC_PCS_ACK_BUTT
};

struct rrc_pcs_read_file_req {
	unsigned int msg_type;
	unsigned int modem_id;
	unsigned int send_pid;
	unsigned int file_type;
	unsigned int op_id;
	unsigned int dir_len;
	char dir[0];
};

struct pcs_rrc_read_file_ind {
	unsigned int msg_type;
	unsigned int modem_id;
	unsigned int rcv_pid;
	unsigned int file_type;
	unsigned int op_id;
	unsigned int ind_rslt;
	unsigned char flow_end;
	unsigned char flow_seq;
	unsigned short data_len;
	unsigned char data[0];
};

struct rrc_pcs_read_file_ack {
	unsigned int msg_type;
	unsigned int modem_id;
	unsigned int send_pid;
	unsigned int file_type;
	unsigned int op_id;
	unsigned int ack_rslt;
	unsigned char flow_seq;
	unsigned char rsv[3];
};

struct pcs_rrc_file_update_ntf {
	unsigned int msg_type;
	unsigned int modem_id;
	unsigned int ntf_type;
};

struct nrrc_pcs_plmn_id {
	unsigned int mcc;
	unsigned int mnc;
};

struct nrrc_pcs_bc_cap_req {
	unsigned int msg_type;
	unsigned int modem_id;
	unsigned int sim_status;
	unsigned int bc_type;
	struct nrrc_pcs_plmn_id req_plmn_id;
	unsigned int loc_plmn_num;
	struct nrrc_pcs_plmn_id loc_plmn_id[NRRC_PCS_MAX_PLMN_ID_NUM];
};

struct pcs_nrrc_bc_cap_ind {
	unsigned int msg_type;
	unsigned int modem_id;
	unsigned int sim_status;
	unsigned int bc_type;
	struct nrrc_pcs_plmn_id req_plmn_id;
	unsigned int cap_ind_type;
	unsigned char flow_end;
	unsigned char flow_seq;
	unsigned short data_len;
	unsigned char data[0];
};

struct nrrc_pcs_bc_cap_ack {
	unsigned int msg_type;
	unsigned int modem_id;
	unsigned int ack_rslt;
	unsigned char flow_seq;
	unsigned char rsv[3];
};

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif
