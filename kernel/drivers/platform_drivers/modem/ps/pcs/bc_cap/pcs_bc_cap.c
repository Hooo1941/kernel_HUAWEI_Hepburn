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
#include "pcs_bc_cap.h"

#include <linux/list.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/in.h>

#include "securec.h"

#include "product_config_ps_ap.h"
#include "rrc_pcs_interface.h"
#include "pcs_bc_cap_get_path.h"
#include "pcs_bc_cap_dir.h"
#include "pcs_log.h"

struct pcs_bc_cap_list {
	struct list_head list;
	u32 modem_id;
	enum nrrc_pcs_sim_status sim_status;
	enum nrrc_pcs_bc_type bc_type;
	enum pcs_nrrc_cap_ind_type cap_ind_type;
	struct nrrc_pcs_plmn_id req_plmn_id;
	u8 flow_end;
	u8 flow_seq;
	u8 data_bc_type;
	u16 data_remain_cnt;
	u8 rsv[3];
	u32 data_len;
	u32 data_offset;
	u8 *data;
};

typedef void (*pcs_bc_cap_msg_proc_func)(void *data, u32 len);

struct pcs_bc_cap_msg_proc_tbl {
	u32 msg_type;
	pcs_bc_cap_msg_proc_func func;
};

struct pcs_bc_cap_req_diag_info {
	enum pcs_diag_cause cause;
	unsigned int msg_type;
	unsigned int modem_id;
	unsigned int sim_status;
	unsigned int bc_type;
	struct nrrc_pcs_plmn_id req_plmn_id;
};

struct pcs_bc_cap_ack_diag_info {
	enum pcs_diag_cause cause;
	unsigned int msg_type;
	unsigned int modem_id;
};

#define PCS_BC_CAP_DATA_HEAD_LEN 3

/* data len usually tens of KB  */
#define PCS_BC_CAP_DATA_MAX_LEN (120 * 1024)

LIST_HEAD(pcs_bc_cap_list);

STATIC void pcs_show_bc_cap_req(struct nrrc_pcs_bc_cap_req *req)
{
	pcs_log_i("msg_type: %d, modem_id: %d, sim_status: %d, bc_type: %d",
		  req->msg_type, req->modem_id, req->sim_status, req->bc_type);
}

STATIC void pcs_show_bc_cap_ind(struct pcs_nrrc_bc_cap_ind *ind)
{
	pcs_log_i("msg_type: %d, modem_id: %d, sim_status: %d, bc_type: %d, "
		  "flow_end: %d, flow_seq: %d, data_len: %d",
		  ind->msg_type, ind->modem_id, ind->sim_status, ind->bc_type,
		  ind->flow_end, ind->flow_seq, ind->data_len);
}

STATIC void pcs_show_bc_cap_ack(struct nrrc_pcs_bc_cap_ack *ack)
{
	pcs_log_i("msg_type: %d, modem_id: %d, ack_rslt: %d, flow_seq: %d",
		  ack->msg_type, ack->modem_id, ack->ack_rslt, ack->flow_seq);
}

STATIC void pcs_show_bc_cap_list(void)
{
	struct pcs_bc_cap_list *node = NULL;
	struct list_head *pos = NULL;

	pcs_log_i("entry");
	list_for_each(pos, &pcs_bc_cap_list) {
		node = list_entry(pos, struct pcs_bc_cap_list, list);
		pcs_log_i("modem_id: %d, sim_status: %d, bc_type: %d, "
			  "cap_ind_type: %d, flow_end: %d, flow_seq: %d, "
			  "data_bc_type: %d, data_remain_cnt: %d, "
			  "data_len: %d, data_offset: %d",
			  node->modem_id, node->sim_status, node->bc_type,
			  node->cap_ind_type, node->flow_end, node->flow_seq,
			  node->data_bc_type, node->data_remain_cnt,
			  node->data_len, node->data_offset);
	}
}

STATIC void pcs_bc_cap_req_trans(struct nrrc_pcs_bc_cap_req *req, enum pcs_diag_cause cause)
{
	struct pcs_bc_cap_req_diag_info info = {0};
	info.cause = cause;

	if (req != NULL) {
		info.msg_type = req->msg_type;
		info.modem_id = req->modem_id;
		info.sim_status = req->sim_status;
		info.bc_type = req->bc_type;
		info.req_plmn_id.mcc = req->req_plmn_id.mcc;
		info.req_plmn_id.mnc = req->req_plmn_id.mnc;
	}

	pcs_trans_report(ID_DIAG_PCS_BC_CAP_REQ_INFO, &info, sizeof(info));
}

STATIC void pcs_bc_cap_ack_trans(struct nrrc_pcs_bc_cap_ack *ack, enum pcs_diag_cause cause)
{
	struct pcs_bc_cap_ack_diag_info info = {0};
	info.cause = cause;

	if (ack != NULL) {
		info.msg_type = ack->msg_type;
		info.modem_id = ack->modem_id;
	}

	pcs_trans_report(ID_DIAG_PCS_BC_CAP_ACK_INFO, &info, sizeof(info));
}

STATIC void pcs_bc_cap_clear_ctx_node(struct pcs_bc_cap_list *node)
{
	if (node == NULL)
		return;

	list_del(&node->list);
	if (node->data != NULL)
		kfree(node->data);

	kfree(node);
	node = NULL;
}

STATIC struct pcs_bc_cap_list *pcs_bc_cap_get_ctx_node_by_modem_id(u32 modem_id)
{
	struct list_head *pos = NULL;
	struct pcs_bc_cap_list *node = NULL;

	list_for_each(pos, &pcs_bc_cap_list) {
		node = list_entry(pos, struct pcs_bc_cap_list, list);
		if (node->modem_id == modem_id)
			break;
	}

	if (pos == &pcs_bc_cap_list)
		return NULL;

	return node;
}

STATIC int pcs_bc_cap_parse_ind_data_info(struct pcs_bc_cap_list *node,
					  u8 *end, u32 *len, u16 *cnt)
{
	u32 offset = node->data_offset;
	u16 i;
	u8 tmp_len;

	*cnt = 0;
	*end = 0;
	*len = PCS_BC_CAP_DATA_HEAD_LEN;

	if (node->cap_ind_type != PCS_NRRC_CAP_IND_NOT_SAME) {
		*end = 1;
		*len = 0;
		return 0;
	}

	for (i = 0; i < node->data_remain_cnt; i++) {
		if (offset > node->data_len) {
			pcs_log_e("offset overflow");
			return -EOVERFLOW;
		}

		tmp_len = node->data[offset] + 1;
		if (*len + tmp_len > PCS_RRC_IND_DATA_MAX_LEN)
			break;

		*len += tmp_len;
		offset += tmp_len;
		(*cnt)++;
	}

	if (i == node->data_remain_cnt)
		*end = 1;

	return 0;
}

STATIC int pcs_bc_cap_fill_ind_data(u8 *data, u32 len,
				    struct pcs_bc_cap_list *node, u16 data_cnt)
{
	u32 i;
	for (i = 0; i < sizeof(data_cnt) / sizeof(*data); i++)
		data[i + 1] = ((u8 *)(&data_cnt))[i];

	data[0] = node->data_bc_type;
	if (memcpy_s(data + PCS_BC_CAP_DATA_HEAD_LEN,
		     len - PCS_BC_CAP_DATA_HEAD_LEN,
		     node->data + node->data_offset,
		     len - PCS_BC_CAP_DATA_HEAD_LEN)) {
		pcs_log_e("memcpy_s fail");
		return -EINVAL;
	}

	return 0;
}

STATIC int pcs_bc_cap_get_ind(struct pcs_nrrc_bc_cap_ind **ind, u32 *ind_len,
		       struct pcs_bc_cap_list *node)
{
	u32 data_len = 0;
	u16 data_cnt = 0;
	u8 flow_end = 0;
	int ret;

	*ind_len = 0;
	ret = pcs_bc_cap_parse_ind_data_info(node, &flow_end, &data_len, &data_cnt);
	if (ret)
		return ret;

	node->flow_end = flow_end;

	*ind_len = sizeof(**ind) + data_len;
	*ind = kzalloc(*ind_len, GFP_KERNEL);
	if (*ind == NULL) {
		pcs_log_e("kzalloc fail");
		return -ENOMEM;
	}

	(*ind)->msg_type = PCS_NRRC_BC_CAP_IND;
	(*ind)->modem_id = node->modem_id;
	(*ind)->sim_status = node->sim_status;
	(*ind)->bc_type = node->bc_type;
	(*ind)->req_plmn_id = node->req_plmn_id;
	(*ind)->flow_end = flow_end;
	(*ind)->flow_seq = node->flow_seq++;
	(*ind)->cap_ind_type = node->cap_ind_type;
	(*ind)->data_len = data_len;
	if ((*ind)->cap_ind_type != PCS_NRRC_CAP_IND_NOT_SAME)
		return 0;

	/* The cnt in the data is in big-endian mode. */
	ret = pcs_bc_cap_fill_ind_data((*ind)->data, data_len, node,
				       cpu_to_be16(data_cnt));
	if (ret) {
		kfree(*ind);
		*ind = NULL;
		return ret;
	}

	node->data_offset += data_len - PCS_BC_CAP_DATA_HEAD_LEN;
	node->data_remain_cnt -= data_cnt;
	return 0;
}

STATIC void pcs_bc_cap_send_msg(struct pcs_bc_cap_list *node)
{
	struct pcs_nrrc_bc_cap_ind *ind = NULL;
	u32 ind_len;

	if (pcs_bc_cap_get_ind(&ind, &ind_len, node))
		return;

	pcs_show_bc_cap_ind(ind);
	pcs_show_bc_cap_list();

	if (pcs_send_msg((struct pcs_msg_head *)ind, ind_len))
		pcs_log_e("pcs_send_msg fail");

	kfree(ind);
	return;
}

STATIC int pcs_bc_cap_read_data_to_node(struct file *fp, struct pcs_bc_cap_list *node,
					struct nrrc_pcs_bc_cap_req *req)
{
	u8 **data = &node->data;
	loff_t size;
	loff_t pos = 0;
	u32 read_size;

	size = i_size_read(file_inode(fp));
	if (size == 0 || size > PCS_BC_CAP_DATA_MAX_LEN) {
		pcs_log_e("size invalid");
		pcs_bc_cap_req_trans(req, PCS_DIAG_CAUSE_READ_FILE_FAILED);
		return -EFBIG;
	}

	node->data_len = size;
	*data = kzalloc(size, GFP_KERNEL);
	if (*data == NULL) {
		pcs_log_e("kzalloc fail");
		pcs_bc_cap_req_trans(req, PCS_DIAG_CAUSE_ALLOC_BUFF_FAILED);
		return -ENOMEM;
	}

	read_size = kernel_read(fp, *data, size, &pos);
	if (read_size != size) {
		pcs_log_e("kernel_read fail");
		pcs_bc_cap_req_trans(req, PCS_DIAG_CAUSE_READ_FILE_FAILED);
		kfree(*data);
		*data = NULL;
		return -ETXTBSY;
	}

	return 0;
}

STATIC int pcs_bc_cap_get_data(struct pcs_bc_cap_list *node, struct nrrc_pcs_bc_cap_req *req)
{
	struct file *fp = NULL;
	char path[PCS_BC_CAP_PATH_MAX_LEN] = {0};
	int ret;
	int close_ret;

	ret = pcs_bc_cap_get_path(path, sizeof(path), req);
	if (ret) {
		pcs_log_e("cap ind not supp, ret is %d", ret);
		return ret;
	}

	fp = filp_open(path, O_RDONLY, 0);
	if (IS_ERR_OR_NULL(fp)) {
		pcs_log_e("fail to open file %s, ret is %ld", path, PTR_ERR(fp));
		pcs_dir_trans(path, strlen(path), PCS_DIAG_CAUSE_OPEN_FILE_FAILED);
		goto file_not_supp;
	}

	ret = pcs_bc_cap_read_data_to_node(fp, node, req);
	close_ret = filp_close(fp, NULL);
	if (close_ret)
		pcs_log_e("filp_close fail, ret: %d", close_ret);

	return ret;

file_not_supp:
	node->cap_ind_type = PCS_NRRC_CAP_IND_NOT_SUPP;
	node->data_offset = 0;
	return 0;
}

STATIC int pcs_bc_cap_init_node(struct nrrc_pcs_bc_cap_req *req,
				struct pcs_bc_cap_list *node)
{
	int ret;
	u32 i;
	u16 temp_cnt;

	if (node->data != NULL) {
		kfree(node->data);
		node->data = NULL;
	}

	node->modem_id = req->modem_id;
	node->sim_status = req->sim_status;
	node->bc_type = req->bc_type;
	node->req_plmn_id = req->req_plmn_id;
	node->flow_end = 0;
	node->flow_seq = 0;
	node->data_bc_type = 0;
	node->data_remain_cnt = 0;
	node->data_len = 0;
	node->data_offset = PCS_BC_CAP_DATA_HEAD_LEN;
	node->cap_ind_type = PCS_NRRC_CAP_IND_NOT_SAME;

	ret = pcs_bc_cap_get_data(node, req);
	if (ret)
		return ret;

	if (node->cap_ind_type == PCS_NRRC_CAP_IND_NOT_SAME) {
		node->data_bc_type = *(u8 *)(node->data);

		for (i = 0; i < sizeof(temp_cnt); i++)
			((u8 *)(&temp_cnt))[i] = (node->data)[i + 1];

		node->data_remain_cnt =  be16_to_cpu(temp_cnt);
	}

	return 0;
}

STATIC void pcs_bc_cap_req_proc(void *data, u32 len)
{
	struct nrrc_pcs_bc_cap_req *req = (struct nrrc_pcs_bc_cap_req *)data;
	struct pcs_bc_cap_list *node = NULL;

	if (len != sizeof(*req)) {
		pcs_log_e("invalid data");
		pcs_bc_cap_req_trans(NULL, PCS_DIAG_CAUSE_RCV_LEN_INVALID);
		return;
	}

	pcs_show_bc_cap_req(req);
	pcs_bc_cap_req_trans(req, PCS_DIAG_CAUSE_RCV_ENTRY);

	node = pcs_bc_cap_get_ctx_node_by_modem_id(req->modem_id);
	/*
	 * before the current data transmission is complete,
	 * If a new request with the same modem_id is received,
	 * the req information is assigned to the node with the same ID
	 * instead of kzalloc a new node.
	 */
	if (node == NULL) {
		node = kzalloc(sizeof(*node), GFP_KERNEL);
		if (node == NULL) {
			pcs_log_e("kzalloc fail");
			pcs_bc_cap_req_trans(req, PCS_DIAG_CAUSE_ALLOC_NODE_FAILED);
			return;
		}

		INIT_LIST_HEAD(&node->list);
		list_add_tail(&node->list, &pcs_bc_cap_list);
	}

	if (pcs_bc_cap_init_node(req, node))
		return;

	pcs_bc_cap_send_msg(node);
	pcs_bc_cap_req_trans(req, PCS_DIAG_CAUSE_SEND_ENTRY);
}

STATIC void pcs_bc_cap_ack_proc(void *data, u32 len)
{
	struct nrrc_pcs_bc_cap_ack *ack = (struct nrrc_pcs_bc_cap_ack *)data;
	struct pcs_bc_cap_list *node = NULL;
	u32 ack_status = 0;

	if (len != sizeof(*ack)) {
		pcs_log_e("invalid data");
		pcs_bc_cap_ack_trans(NULL, PCS_DIAG_CAUSE_RCV_LEN_INVALID);
		return;
	}

	pcs_show_bc_cap_ack(ack);
	pcs_bc_cap_ack_trans(ack, PCS_DIAG_CAUSE_RCV_ENTRY);

	node = pcs_bc_cap_get_ctx_node_by_modem_id(ack->modem_id);
	if (node == NULL) {
		pcs_log_h("ack invalid");
		pcs_bc_cap_ack_trans(ack, PCS_DIAG_CAUSE_NODE_INVALID);
		return;
	}

	if (node->flow_end == 1) {
		pcs_bc_cap_ack_trans(ack, PCS_DIAG_CAUSE_ACK_FINISH);
		goto not_send;
	}

	ack_status |= (ack->ack_rslt ^ NRRC_PCS_ACK_SUCC);
	ack_status |= (ack->flow_seq ^ (node->flow_seq - 1));
	if (ack_status) {
		pcs_log_e("ack error");
		pcs_bc_cap_ack_trans(ack, PCS_DIAG_CAUSE_ACK_PARAM_ABNORMAL);
		goto not_send;
	}

	pcs_bc_cap_send_msg(node);
	pcs_bc_cap_ack_trans(ack, PCS_DIAG_CAUSE_SEND_ENTRY);
	return;

not_send:
	pcs_bc_cap_clear_ctx_node(node);
}

struct pcs_bc_cap_msg_proc_tbl pcs_bc_cap_proc_tbl[] = {
	{ NRRC_PCS_BC_CAP_REQ, pcs_bc_cap_req_proc },
	{ NRRC_PCS_BC_CAP_ACK, pcs_bc_cap_ack_proc },
};

int pcs_bc_cap_msg_proc(struct pcs_msg_head *head, u32 len)
{
	u32 i;
	for (i = 0; i < ARRAY_SIZE(pcs_bc_cap_proc_tbl); i++) {
		if (head->msg_type == pcs_bc_cap_proc_tbl[i].msg_type) {
			pcs_bc_cap_proc_tbl[i].func(head, len);
			return 0;
		}
	}

	pcs_log_e("msg type[%d] invalid", head->msg_type);
	return -EINVAL;
}

int pcs_bc_cap_init(void)
{
	INIT_LIST_HEAD(&pcs_bc_cap_list);
	return 0;
}

void pcs_bc_cap_deinit(void)
{
	struct pcs_bc_cap_list *node = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	list_for_each_safe(pos, n, &pcs_bc_cap_list) {
		node = list_entry(pos, struct pcs_bc_cap_list, list);
		list_del(&node->list);
		if (node->data != NULL)
			kfree(node->data);

		kfree(node);
		node = NULL;
	}
}
