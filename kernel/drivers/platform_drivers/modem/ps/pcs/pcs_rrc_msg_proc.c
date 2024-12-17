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
#include "pcs_rrc_msg_proc.h"

#include <linux/list.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/in.h>

#include "securec.h"

#include "rrc_pcs_interface.h"
#include "pcs_bc_cap_get_path.h"
#include "pcs_bc_cap_dir.h"
#include "pcs_log.h"

struct pcs_rrc_list_s {
	struct list_head list;
	u32 modem_id;
	u32 rcv_pid;
	u32 file_type;
	u32 op_id;
	u32 ind_rslt;
	u8 flow_end;
	u8 flow_seq;
	u8 data_type;
	u8 rsv0[1];
	u16 data_remain_cnt;
	u8 rsv1[2];
	u32 data_len;
	u32 data_offset;
	u8 *data;
};

typedef void (*pcs_rrc_msg_proc_func)(void *data, u32 len);

struct pcs_rrc_msg_proc_tbl {
	u32 msg_type;
	pcs_rrc_msg_proc_func func;
};

struct pcs_rrc_read_diag_info {
	enum pcs_diag_cause cause;
	unsigned int msg_type;
	unsigned int modem_id;
	unsigned int pid;
	unsigned int file_type;
	unsigned int op_id;
};

#define PCS_TLV_DATA_HEAD_LEN 3
#define PCS_TLV_DATA_MAX_LEN (120 * 1024)

#define PCS_DIR_MAX_LEN 256
#define PCS_RRC_PATH_MAX_LEN (PCS_DIR_MAX_LEN + RRC_PCS_MAX_FILE_DIR_LEN)

#define PCS_RRC_NODE_MAX_NUM 1000

LIST_HEAD(pcs_rrc_list);

STATIC void pcs_show_rrc_req(struct rrc_pcs_read_file_req *req)
{
	pcs_log_i("msg_type: %d, modem_id: %d, send_pid: %d, file_type: %d, "
		  "op_id: %d, dir_len: %d",
		  req->msg_type, req->modem_id, req->send_pid, req->file_type,
		  req->op_id, req->dir_len);
}

STATIC void pcs_show_rrc_ind(struct pcs_rrc_read_file_ind *ind)
{
	pcs_log_i("msg_type: %d, modem_id: %d, rcv_pid: %d, file_type: %d, "
		  "op_id: %d, ind_rslt: %d, flow_end: %d, flow_seq: %d, "
		  "data_len: %d",
		  ind->msg_type, ind->modem_id, ind->rcv_pid, ind->file_type,
		  ind->op_id, ind->ind_rslt, ind->flow_end, ind->flow_seq,
		  ind->data_len);
}

STATIC void pcs_show_rrc_ack(struct rrc_pcs_read_file_ack *ack)
{
	pcs_log_i("msg_type: %d, modem_id: %d, send_pid: %d, file_type: %d, "
		  "op_id: %d, ack_rslt: %d, flow_seq: %d",
		  ack->msg_type, ack->modem_id, ack->send_pid, ack->file_type,
		  ack->op_id, ack->ack_rslt, ack->flow_seq);
}

STATIC void pcs_show_rrc_list(void)
{
	struct pcs_rrc_list_s *node = NULL;
	struct list_head *pos = NULL;

	pcs_log_i("entry");
	list_for_each(pos, &pcs_rrc_list) {
		node = list_entry(pos, struct pcs_rrc_list_s, list);
		pcs_log_i("modem_id: %d, rcv_pid: %d, file_type: %d, "
			  "op_id: %d, ind_rslt: %d, flow_end: %d, "
			  "flow_seq: %d, data_type: %d, data_remain_cnt: %d, "
			  "data_len: %d, data_offset: %d",
			  node->modem_id, node->rcv_pid, node->file_type,
			  node->op_id, node->ind_rslt, node->flow_end,
			  node->flow_seq, node->data_type, node->data_remain_cnt,
			  node->data_len, node->data_offset);
	}
}

STATIC void pcs_rrc_read_req_trans(struct rrc_pcs_read_file_req *req, enum pcs_diag_cause cause)
{
	struct pcs_rrc_read_diag_info info = {0};
	info.cause = cause;

	if (req != NULL) {
		info.msg_type = req->msg_type;
		info.modem_id = req->modem_id;
		info.pid = req->send_pid;
		info.file_type = req->file_type;
		info.op_id = req->op_id;
	}

	pcs_trans_report(ID_DIAG_PCS_RRC_READ_INFO, &info, sizeof(info));
}

STATIC void pcs_rrc_read_ack_trans(struct rrc_pcs_read_file_ack *ack, enum pcs_diag_cause cause)
{
	struct pcs_rrc_read_diag_info info = {0};
	info.cause = cause;

	if (ack != NULL) {
		info.msg_type = ack->msg_type;
		info.modem_id = ack->modem_id;
		info.pid = ack->send_pid;
		info.file_type = ack->file_type;
		info.op_id = ack->op_id;
	}

	pcs_trans_report(ID_DIAG_PCS_RRC_READ_INFO, &info, sizeof(info));
}

STATIC void pcs_rrc_clear_ctx_node(struct pcs_rrc_list_s *node)
{
	if (node == NULL)
		return;

	list_del(&node->list);
	if (node->data != NULL)
		kfree(node->data);

	kfree(node);
	node = NULL;
}

STATIC struct pcs_rrc_list_s *pcs_rrc_get_ctx_node(u32 modem_id, u32 send_pid,
						   u32 file_type, u32 op_id)
{
	struct list_head *pos = NULL;
	struct pcs_rrc_list_s *node = NULL;

	list_for_each(pos, &pcs_rrc_list) {
		node = list_entry(pos, struct pcs_rrc_list_s, list);
		if (node->modem_id == modem_id && node->rcv_pid == send_pid &&
		    node->file_type == file_type && node->op_id == op_id)
			break;
	}

	if (pos == &pcs_rrc_list)
		return NULL;

	return node;
}

STATIC int pcs_rrc_parse_ind_data_info(struct pcs_rrc_list_s *node,
				       u8 *end, u32 *len, u16 *cnt)
{
	u32 offset = node->data_offset;
	u16 i;
	u8 tmp_len;

	*cnt = 0;
	*end = 0;
	*len = PCS_TLV_DATA_HEAD_LEN;

	if (node->ind_rslt != PCS_RRC_READ_FILE_IND_SUCC) {
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

STATIC int pcs_rrc_fill_ind_data(u8 *data, u32 len,
				 struct pcs_rrc_list_s *node, u16 data_cnt)
{
	u32 i;
	for (i = 0; i < sizeof(data_cnt) / sizeof(*data); i++)
		data[i + 1] = ((u8 *)(&data_cnt))[i];

	data[0] = node->data_type;
	if (memcpy_s(data + PCS_TLV_DATA_HEAD_LEN,
		     len - PCS_TLV_DATA_HEAD_LEN,
		     node->data + node->data_offset,
		     len - PCS_TLV_DATA_HEAD_LEN)) {
		pcs_log_e("memcpy_s fail");
		return -EINVAL;
	}

	return 0;
}

STATIC int pcs_rrc_get_ind(struct pcs_rrc_read_file_ind **ind, u32 *ind_len,
		       struct pcs_rrc_list_s *node)
{
	u32 data_len = 0;
	u16 data_cnt = 0;
	u8 flow_end = 0;
	int ret;

	*ind_len = 0;
	ret = pcs_rrc_parse_ind_data_info(node, &flow_end, &data_len, &data_cnt);
	if (ret)
		return ret;

	node->flow_end = flow_end;

	*ind_len = sizeof(**ind) + data_len;
	*ind = kzalloc(*ind_len, GFP_KERNEL);
	if (*ind == NULL) {
		pcs_log_e("kzalloc fail");
		return -ENOMEM;
	}

	(*ind)->msg_type = PCS_RRC_READ_FILE_IND;
	(*ind)->modem_id = node->modem_id;
	(*ind)->rcv_pid = node->rcv_pid;
	(*ind)->file_type = node->file_type;
	(*ind)->op_id = node->op_id;
	(*ind)->flow_end = flow_end;
	(*ind)->flow_seq = node->flow_seq++;
	(*ind)->ind_rslt = node->ind_rslt;
	(*ind)->data_len = data_len;
	if ((*ind)->ind_rslt != PCS_RRC_READ_FILE_IND_SUCC)
		return 0;

	/* The cnt in the data is in big-endian mode. */
	ret = pcs_rrc_fill_ind_data((*ind)->data, data_len, node,
				    cpu_to_be16(data_cnt));
	if (ret) {
		kfree(*ind);
		*ind = NULL;
		return ret;
	}

	node->data_offset += data_len - PCS_TLV_DATA_HEAD_LEN;
	node->data_remain_cnt -= data_cnt;
	return 0;
}

STATIC void pcs_rrc_send_msg(struct pcs_rrc_list_s *node)
{
	struct pcs_rrc_read_file_ind *ind = NULL;
	u32 ind_len;

	if (pcs_rrc_get_ind(&ind, &ind_len, node))
		return;

	pcs_show_rrc_ind(ind);
	pcs_show_rrc_list();

	if (pcs_send_msg((struct pcs_msg_head *)ind, ind_len))
		pcs_log_e("pcs_send_msg fail");

	kfree(ind);
}

STATIC int pcs_rrc_get_file_dir(struct rrc_pcs_read_file_req *req,
				char *dir, u32 dir_len)
{
	if (memcpy_s(dir, dir_len, req->dir, req->dir_len)) {
		pcs_log_e("memcpy_s fail");
		pcs_dir_trans(dir, strlen(dir), PCS_DIAG_CAUSE_CP_DIR_ABNORMAL);
		return -EINVAL;
	}

	return 0;
}

STATIC int pcs_rrc_get_path(char *path, u32 path_len,
			    struct rrc_pcs_read_file_req *req)
{
	char dir[PCS_DIR_MAX_LEN] = {0};
	char file_dir[RRC_PCS_MAX_FILE_DIR_LEN] = {0};
	int dir_len = 0;
	int ret;

	switch (req->file_type) {
	case NRRC_PCS_FILE_SAME_SET:
		dir_len = pcs_get_bc_cap_dir(dir, sizeof(dir));
		ret = pcs_rrc_get_file_dir(req, file_dir, sizeof(file_dir));
		break;
	default:
		pcs_log_e("file_type: %d invalid", req->file_type);
		return -EINVAL;
	}

	if (dir_len <= 0 || ret != 0)
		return -EINVAL;

	(void)memset_s(path, path_len, 0, path_len);

	ret |= memcpy_s(path, path_len, dir, dir_len);
	ret |= strncat_s(path, path_len, file_dir, strlen(file_dir));
	if (ret) {
		pcs_log_e("path cat fail");
		pcs_dir_trans(NULL, 0, PCS_DIAG_CAUSE_PATH_INVALID);
		return -EINVAL;
	}

	return 0;
}

STATIC bool pcs_rrc_verify_path(char *dir, u32 dir_size, u32 type)
{
	switch (type) {
	case NRRC_PCS_FILE_SAME_SET:
		return pcs_bc_cap_verify_dir(dir);
	default:
		pcs_log_e("type: %d invalid", type);
		return false;
	}

	return false;
}

STATIC int pcs_rrc_read_data_to_node(struct file *fp, struct pcs_rrc_list_s *node,
				     struct rrc_pcs_read_file_req *req)
{
	u8 **data = &node->data;
	loff_t size;
	loff_t pos = 0;
	u32 read_size;

	size = i_size_read(file_inode(fp));
	if (size == 0 || size > PCS_TLV_DATA_MAX_LEN) {
		pcs_log_e("size invalid");
		pcs_rrc_read_req_trans(req, PCS_DIAG_CAUSE_READ_FILE_FAILED);
		return -EFBIG;
	}

	node->data_len = size;
	*data = kzalloc(size, GFP_KERNEL);
	if (*data == NULL) {
		pcs_log_e("kzalloc fail");
		pcs_rrc_read_req_trans(req, PCS_DIAG_CAUSE_ALLOC_BUFF_FAILED);
		return -ENOMEM;
	}

	read_size = kernel_read(fp, *data, size, &pos);
	if (read_size != size) {
		pcs_log_e("kernel_read fail");
		pcs_rrc_read_req_trans(req, PCS_DIAG_CAUSE_READ_FILE_FAILED);
		kfree(*data);
		*data = NULL;
		return -ETXTBSY;
	}

	return 0;
}

STATIC int pcs_rrc_get_data(struct pcs_rrc_list_s *node, struct rrc_pcs_read_file_req *req)
{
	struct file *fp = NULL;
	char path[PCS_RRC_PATH_MAX_LEN] = {0};
	int ret;
	int close_ret;

	ret = pcs_rrc_get_path(path, sizeof(path), req);
	if (ret)
		goto file_not_exist;

	if (!pcs_rrc_verify_path(path, sizeof(path), req->file_type)) {
		pcs_dir_trans(path, strlen(path), PCS_DIAG_CAUSE_PATH_INVALID);
		goto file_not_exist;
	}

	fp = filp_open(path, O_RDONLY, 0);
	if (IS_ERR_OR_NULL(fp)) {
		pcs_dir_trans(path, strlen(path), PCS_DIAG_CAUSE_OPEN_FILE_FAILED);
		goto file_not_exist;
	}

	ret = pcs_rrc_read_data_to_node(fp, node, req);
	close_ret = filp_close(fp, NULL);
	if (close_ret)
		pcs_log_e("filp_close fail, ret: %d", close_ret);

	return ret;

file_not_exist:
	node->ind_rslt = PCS_RRC_READ_FILE_IND_NOT_EXIST;
	node->data_offset = 0;
	return 0;
}

STATIC int pcs_rrc_init_node(struct rrc_pcs_read_file_req *req,
				struct pcs_rrc_list_s *node)
{
	int ret;
	u32 i;
	u16 temp_cnt;

	if (node->data != NULL) {
		kfree(node->data);
		node->data = NULL;
	}

	node->modem_id = req->modem_id;
	node->rcv_pid = req->send_pid;
	node->file_type = req->file_type;
	node->op_id = req->op_id;
	node->ind_rslt = PCS_RRC_READ_FILE_IND_SUCC;
	node->flow_end = 0;
	node->flow_seq = 0;
	node->data_type = 0;
	node->data_remain_cnt = 0;
	node->data_len = 0;
	node->data_offset = PCS_TLV_DATA_HEAD_LEN;

	ret = pcs_rrc_get_data(node, req);
	if (ret)
		return ret;

	/*
	 * data structure:
	 * [type:u8][cnt:u16]
	 * [len_0:u8][data_0:(len_0)*u8]
	 * [len_1:u8][data_1:(len_1)*u8]
	 * …
	 * [len_cnt:u8][data_cnt:(len_cnt)*u8]
	 *
	 * the u16 cnt is big endian.
	 */
	if (node->ind_rslt == PCS_RRC_READ_FILE_IND_SUCC) {
		node->data_type = *(u8 *)(node->data);

		for (i = 0; i < sizeof(temp_cnt); i++)
			((u8 *)(&temp_cnt))[i] = (node->data)[i + 1];

		node->data_remain_cnt =  be16_to_cpu(temp_cnt);
	}

	return 0;
}

STATIC void pcs_rrc_send_err_msg(u32 modem_id, u32 send_pid, u32 file_type,
				 u32 op_id, u32 ind_rslt)
{
	struct pcs_rrc_read_file_ind ind = {0};

	ind.msg_type = PCS_RRC_READ_FILE_IND;
	ind.modem_id = modem_id;
	ind.rcv_pid = send_pid;
	ind.file_type = file_type;
	ind.op_id = op_id;
	ind.ind_rslt = ind_rslt;
	ind.flow_end = 1;
	ind.flow_seq = 0;
	ind.data_len = 0;

	pcs_show_rrc_ind(&ind);

	if (pcs_send_msg((struct pcs_msg_head *)(&ind), sizeof(ind)))
		pcs_log_e("pcs_send_msg fail");
}

STATIC void pcs_rrc_req_proc(void *data, u32 len)
{
	struct rrc_pcs_read_file_req *req = (struct rrc_pcs_read_file_req *)data;
	struct pcs_rrc_list_s *node = NULL;
	struct list_head *pos = NULL;
	u32 cnt = 0;

	if (len < sizeof(*req)) {
		pcs_log_e("invalid data");
		pcs_rrc_read_req_trans(NULL, PCS_DIAG_CAUSE_RCV_LEN_INVALID);
		return;
	}

	if (req->dir_len > PCS_DIR_MAX_LEN ||
	    len != sizeof(*req) + req->dir_len ||
	    req->dir_len != (strnlen(req->dir, len - sizeof(*req)) + 1)) {
		pcs_log_e("invalid dir");
		pcs_rrc_read_req_trans(req, PCS_DIAG_CAUSE_RCV_DIR_LEN_INVALID);
		return;
	}

	pcs_show_rrc_req(req);
	pcs_rrc_read_req_trans(req, PCS_DIAG_CAUSE_RCV_ENTRY);

	node = pcs_rrc_get_ctx_node(req->modem_id, req->send_pid,
				    req->file_type, req->op_id);
	/*
	 * before the current data transmission is complete,
	 * If a new request is received with the same
	 * modem_id & send_pid & file_type & op_id,
	 * the req information is assigned to the node with the same ID
	 * instead of kzalloc a new node.
	 */
	if (node == NULL) {
		list_for_each(pos, &pcs_rrc_list)
			cnt++;

		if (cnt >= PCS_RRC_NODE_MAX_NUM) {
			pcs_log_e("too many nodes");
			pcs_show_rrc_list();
			pcs_rrc_read_req_trans(req, PCS_DIAG_CAUSE_TOO_MANY_NODE);
			goto err;
		}

		node = kzalloc(sizeof(*node), GFP_KERNEL);
		if (node == NULL) {
			pcs_log_e("kzalloc fail");
			pcs_rrc_read_req_trans(req, PCS_DIAG_CAUSE_ALLOC_NODE_FAILED);
			goto err;
		}

		INIT_LIST_HEAD(&node->list);
		list_add_tail(&node->list, &pcs_rrc_list);
	}

	if (pcs_rrc_init_node(req, node))
		goto init_node_err;

	pcs_rrc_send_msg(node);
	pcs_rrc_read_req_trans(req, PCS_DIAG_CAUSE_SEND_ENTRY);
	return;

init_node_err:
	pcs_rrc_clear_ctx_node(node);

err:
	pcs_rrc_send_err_msg(req->modem_id, req->send_pid, req->file_type,
			     req->op_id, PCS_RRC_READ_FILE_IND_OTHER_FAIL);
}

STATIC void pcs_rrc_ack_proc(void *data, u32 len)
{
	struct rrc_pcs_read_file_ack *ack = (struct rrc_pcs_read_file_ack *)data;
	struct pcs_rrc_list_s *node = NULL;
	u32 ack_status = 0;

	if (len != sizeof(*ack)) {
		pcs_log_e("invalid data");
		pcs_rrc_read_ack_trans(NULL, PCS_DIAG_CAUSE_RCV_LEN_INVALID);
		return;
	}

	pcs_show_rrc_ack(ack);
	pcs_rrc_read_ack_trans(ack, PCS_DIAG_CAUSE_RCV_ENTRY);

	node = pcs_rrc_get_ctx_node(ack->modem_id, ack->send_pid,
				    ack->file_type, ack->op_id);
	if (node == NULL) {
		pcs_log_h("ack invalid");
		pcs_rrc_read_ack_trans(ack, PCS_DIAG_CAUSE_RCV_LEN_INVALID);
		return;
	}

	if (node->flow_end == 1) {
		pcs_rrc_read_ack_trans(ack, PCS_DIAG_CAUSE_ACK_FINISH);
		goto clear_node;
	}

	ack_status |= (ack->ack_rslt ^ RRC_PCS_READ_FILE_ACK_SUCC);
	ack_status |= (ack->flow_seq ^ (node->flow_seq - 1));
	if (ack_status) {
		pcs_log_e("ack error");
		pcs_rrc_read_ack_trans(ack, PCS_DIAG_CAUSE_ACK_PARAM_ABNORMAL);
		goto clear_node;
	}

	pcs_rrc_send_msg(node);
	pcs_rrc_read_ack_trans(ack, PCS_DIAG_CAUSE_SEND_ENTRY);
	return;

clear_node:
	pcs_rrc_clear_ctx_node(node);
}

struct pcs_rrc_msg_proc_tbl pcs_rrc_proc_tbl[] = {
	{ RRC_PCS_READ_FILE_REQ, pcs_rrc_req_proc },
	{ RRC_PCS_READ_FILE_ACK, pcs_rrc_ack_proc },
};

int pcs_rrc_msg_proc(struct pcs_msg_head *head, u32 len)
{
	u32 i;
	for (i = 0; i < ARRAY_SIZE(pcs_rrc_proc_tbl); i++) {
		if (head->msg_type == pcs_rrc_proc_tbl[i].msg_type) {
			pcs_rrc_proc_tbl[i].func(head, len);
			return 0;
		}
	}

	pcs_log_e("msg type[%d] invalid", head->msg_type);
	return -EINVAL;
}

int pcs_rrc_init(void)
{
	int ret = pcs_bc_cap_init_get_dir();
	if (ret)
		return ret;

	INIT_LIST_HEAD(&pcs_rrc_list);
	return 0;
}

void pcs_rrc_deinit(void)
{
	struct pcs_rrc_list_s *node = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	list_for_each_safe(pos, n, &pcs_rrc_list) {
		node = list_entry(pos, struct pcs_rrc_list_s, list);
		list_del(&node->list);
		if (node->data != NULL)
			kfree(node->data);

		kfree(node);
		node = NULL;
	}
}
