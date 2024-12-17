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
#include "pcs_bc_cap_get_path.h"

#include <linux/path.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/dcache.h>
#include <linux/limits.h>
#include <linux/slab.h>
#include <linux/errno.h>

#include "securec.h"

#include "product_config_ps_ap.h"
#include "pcs_log.h"
#include "pcs_def.h"

struct pcs_bc_cap_dir_tbl {
	u32 index;
	char *str;
};

pcs_get_bc_cap_dir_cb pcs_bc_cap_cb = NULL;
STATIC DEFINE_MUTEX(pcs_bc_cap_cb_lock);

void pcs_show_bc_cap_dir(void)
{
	int ret;
	char *dir = NULL;

	mutex_lock(&pcs_bc_cap_cb_lock);
	if (pcs_bc_cap_cb == NULL) {
		mutex_unlock(&pcs_bc_cap_cb_lock);
		pcs_log_e("cb is NULL");
		return;
	}

	ret = pcs_bc_cap_cb(&dir);
	mutex_unlock(&pcs_bc_cap_cb_lock);

	if (ret <= 0 || ret >= PCS_BC_CAP_DIR_MAX_LEN) {
		pcs_log_e("ret: %d, error", ret);
		return;
	}

	pcs_log_h("dir: %s, ret: %d", dir, ret);
}

struct pcs_dir_diag_info {
	enum pcs_diag_cause cause;
	char dir[PCS_BC_CAP_PATH_MAX_LEN];
};

void pcs_dir_trans(char *dir, size_t dir_len, enum pcs_diag_cause cause)
{
	struct pcs_dir_diag_info info = {0};
	info.cause = cause;
	if (dir != NULL) {
		if (memcpy_s(info.dir, sizeof(info.dir), dir, dir_len))
			pcs_log_e("memcpy_s fail");
	}

	pcs_trans_report(ID_DIAG_PCS_DIR_INFO, &info, sizeof(info));
}

void pcs_reg_get_bc_cap_dir_cb(pcs_get_bc_cap_dir_cb cb)
{
	mutex_lock(&pcs_bc_cap_cb_lock);
	pcs_bc_cap_cb = cb;
	mutex_unlock(&pcs_bc_cap_cb_lock);

	pcs_show_bc_cap_dir();
}

pcs_get_bc_cap_dir_cb pcs_get_bc_cap_cb(void)
{
	return pcs_bc_cap_cb;
}

#define ASCII_SLASH 0x2F

const char *pcs_bc_cap_dir_allowlist[] = {
	"/odm/etc/dataservice/ca",
	"/chip_prod/etc/dataservice/ca",

	"/system/app/config",

};

STATIC char *pcs_realpath(const char *path, char *buf, u32 buf_len)
{
	int ret;
	char *dpath = NULL;
	struct path base_path = {
		.dentry = NULL
	};

	ret = kern_path(path, LOOKUP_FOLLOW, &base_path);
	if (ret != 0) {
		pcs_log_e("kern_path err: %d", ret);
		return NULL;
	}

	dpath = d_path(&base_path, buf, buf_len);
	path_put(&base_path);
	return dpath;
}

bool pcs_bc_cap_verify_dir(char *dir)
{
	char *real_dir = NULL;
	char buf[PCS_BC_CAP_PATH_MAX_LEN] = {0};
	u32 i;

	real_dir = pcs_realpath(dir, buf, sizeof(buf));
	if (IS_ERR_OR_NULL(real_dir)) {
		pcs_log_e("pcs_realpath fail, dir: %s ", dir);
		return false;
	}

	for (i = 0; i < ARRAY_SIZE(pcs_bc_cap_dir_allowlist); i++) {
		if (strlen(real_dir) < strlen(pcs_bc_cap_dir_allowlist[i]))
			continue;

		if (strncmp(real_dir, pcs_bc_cap_dir_allowlist[i],
			    strlen(pcs_bc_cap_dir_allowlist[i])) == 0)
			return true;
	}

	pcs_log_e("real_dir: %s is not in allowlist", real_dir);
	return false;
}

int pcs_get_bc_cap_dir(char *data, u32 data_len)
{
	char *dir = NULL;
	int len;

	mutex_lock(&pcs_bc_cap_cb_lock);
	if (pcs_bc_cap_cb == NULL) {
		mutex_unlock(&pcs_bc_cap_cb_lock);
		pcs_log_e("cb is NULL");
		pcs_dir_trans(NULL, 0, PCS_DIAG_CAUSE_AP_DIR_ABNORMAL);
		return -EINVAL;
	}

	len = pcs_bc_cap_cb(&dir);
	mutex_unlock(&pcs_bc_cap_cb_lock);

	if (len <= 0) {
		pcs_log_e("pcs_bc_cap_cb error");
		pcs_dir_trans(NULL, 0, PCS_DIAG_CAUSE_AP_DIR_ABNORMAL);
		return -ENOENT;
	}

	if (len > PCS_BC_CAP_DIR_MAX_LEN || len > data_len - 1) {
		pcs_log_e("len too long");
		pcs_dir_trans(NULL, 0, PCS_DIAG_CAUSE_AP_DIR_ABNORMAL);
		return -ENAMETOOLONG;
	}

	if (dir[len - 1] != 0) {
		pcs_log_e("dir invalid");
		pcs_dir_trans(NULL, 0, PCS_DIAG_CAUSE_AP_DIR_ABNORMAL);
		return -EINVAL;
	}

	if (!pcs_bc_cap_verify_dir(dir)) {
		pcs_dir_trans(dir, strlen(dir), PCS_DIAG_CAUSE_AP_DIR_INVALID);
		return -EINVAL;
	}

	if (memcpy_s(data, data_len, dir, strlen(dir) + 1)) {
		pcs_log_e("memcpy_s dir fail");
		pcs_dir_trans(dir, strlen(dir), PCS_DIAG_CAUSE_AP_DIR_ABNORMAL);
		return -EINVAL;
	}

	len--;

	if (data[len - 1] != ASCII_SLASH) {
		data[len] = ASCII_SLASH;
		data[len + 1] = 0;
		len++;
	}

	return len;
}

STATIC bool pcs_bc_cap_is_plmn_unspecified(struct nrrc_pcs_plmn_id plmn_id)
{
	static const struct nrrc_pcs_plmn_id unspecified_plmn = {
		.mcc = 0xFFFFFFFF,
		.mnc = 0xFFFFFFFF,
	};

	return (plmn_id.mcc == unspecified_plmn.mcc &&
		plmn_id.mnc == unspecified_plmn.mnc);
}

STATIC int pcs_bc_cap_cat_plmn_set_dir(char *dir, u32 dir_len,
				       struct nrrc_pcs_plmn_id plmn_id)
{
	if (snprintf_s(dir, dir_len, dir_len - 1, "%03d/%03d%02d",
		       plmn_id.mcc, plmn_id.mcc, plmn_id.mnc) < 0) {
		pcs_log_e("snprintf_s fail");
		pcs_dir_trans(NULL, 0, PCS_DIAG_CAUSE_CP_SET_DIR_ABNORMAL);
		return -EINVAL;
	}

	return 0;
}

STATIC int pcs_bc_cap_cat_set_dir(char *path, u32 path_len, u32 dir_offset,
				  char *set_dir, u32 set_dir_len)
{
	(void)memset_s(path + dir_offset, path_len - dir_offset, 0,
		       path_len - dir_offset);
	if (strncat_s(path, path_len, set_dir, set_dir_len)) {
		pcs_log_e("strncat_s fail");
		pcs_dir_trans(set_dir, set_dir_len, PCS_DIAG_CAUSE_CP_SET_DIR_ABNORMAL);
		return -EINVAL;
	}

	return 0;
}

STATIC bool pcs_bc_cap_is_dir_exist(const char *dir, u32 dir_len)
{
	struct file *fp = NULL;
	int close_ret;

	(void)dir_len;
	fp = filp_open(dir, O_RDONLY, 0);
	if (IS_ERR_OR_NULL(fp)) {
		pcs_log_e("open file fail, ret is %ld", PTR_ERR(fp));
		return false;
	}

	close_ret = filp_close(fp, NULL);
	if (close_ret != 0)
		pcs_log_e("filp_close fail, ret: %d", close_ret);

	return true;
}

STATIC int pcs_bc_cap_save_set_dir(char *set_dir, u32 set_dir_len,
				   char *dst_dir, u32 dst_dir_len)
{
	(void)memset_s(set_dir, set_dir_len, 0, set_dir_len);
	if (memcpy_s(set_dir, set_dir_len, dst_dir, dst_dir_len)) {
		pcs_log_e("memcpy_s fail");
		pcs_dir_trans(dst_dir, dst_dir_len, PCS_DIAG_CAUSE_CP_SET_DIR_ABNORMAL);
		return -EINVAL;
	}

	return 0;
}

/* The order cannot be changed, plmn -> base -> initail */
enum PCS_BC_CAP_SET_DIR_ID {
	PCS_BC_CAP_PLMN_SET_DIR_ID = 0,
	PCS_BC_CAP_BASE_SET_DIR_ID,
	PCS_BC_CAP_INITIAL_SET_DIR_ID,
	PCS_BC_CAP_SET_DIR_ID_BUTT,
};

STATIC int pcs_bc_cap_get_set_dir(struct nrrc_pcs_bc_cap_req *req,
				  const char *dir, u32 dir_len,
				  char *set_dir, u32 set_dir_len)
{
	char *temp_dir = NULL;
	char plmn_set_dir[PCS_BC_CAP_EXT_DIR_MAX_LEN] = {0};
	char *set_dir_tbl[PCS_BC_CAP_SET_DIR_ID_BUTT] = {
		[PCS_BC_CAP_PLMN_SET_DIR_ID] =  plmn_set_dir,
		[PCS_BC_CAP_BASE_SET_DIR_ID] =  "base",
		[PCS_BC_CAP_INITIAL_SET_DIR_ID] =  "initialSet",
	};
	char path[PCS_BC_CAP_PATH_MAX_LEN] = {0};
	enum PCS_BC_CAP_SET_DIR_ID index = PCS_BC_CAP_PLMN_SET_DIR_ID;
	int ret;

	if (memcpy_s(path, sizeof(path), dir, dir_len)) {
		pcs_log_e("memcpy_s fail");
		pcs_dir_trans(NULL, 0, PCS_DIAG_CAUSE_CP_SET_DIR_ABNORMAL);
		return -EINVAL;
	}

	if (pcs_bc_cap_is_plmn_unspecified(req->req_plmn_id)) {
		/* start searching from the initial set */
		index = PCS_BC_CAP_INITIAL_SET_DIR_ID;
	} else {
		ret = pcs_bc_cap_cat_plmn_set_dir(plmn_set_dir,
						  sizeof(plmn_set_dir),
						  req->req_plmn_id);
		if (ret)
			return ret;
	}

	for (; index < PCS_BC_CAP_SET_DIR_ID_BUTT; index++) {
		temp_dir = set_dir_tbl[index];
		ret = pcs_bc_cap_cat_set_dir(path, sizeof(path),
					     dir_len, temp_dir, strlen(temp_dir));
		if (ret)
			return ret;

		if (pcs_bc_cap_is_dir_exist(path, sizeof(path)))
			goto path_exist;
	}

	pcs_log_e("set_dir not found in %s", dir);
	pcs_dir_trans(path, strlen(path), PCS_DIAG_CAUSE_CP_SET_DIR_ABNORMAL);
	return -ENOENT;

path_exist:
	return pcs_bc_cap_save_set_dir(set_dir, set_dir_len, temp_dir, strlen(temp_dir));
}

STATIC int pcs_bc_cap_cat_file_dir_from_tbl(char *dir, u32 len,
					    struct pcs_bc_cap_dir_tbl *tbl,
					    u32 size, u32 index)
{
	char *str = NULL;
	int ret = 0;
	u32 i;
	for (i = 0; i < size; i++) {
		if (tbl[i].index == index) {
			str = tbl[i].str;
			ret = strncat_s(dir, len, str, strlen(str));
			break;
		}
	}

	if (ret) {
		pcs_log_e("strncat_s fail");
		return ret;
	}

	if (i == size) {
		pcs_log_e("index invalid");
		return -EINVAL;
	}

	return 0;
}

STATIC int pcs_bc_cap_get_file_dir(struct nrrc_pcs_bc_cap_req *req, char *file_dir, u32 len)
{
	static struct pcs_bc_cap_dir_tbl sim_status_tbl[] = {
		{ NRRC_PCS_SINGLE_CARD, "/single/" },
		{ NRRC_PCS_5_4_DUAL_CARD_MASTER, "/dual_master/5+4_" },
		{ NRRC_PCS_5_5_DUAL_CARD_MASTER, "/dual_master/5+5_" },
		{ NRRC_PCS_5_5_DUAL_CARD_SLAVE, "/dual_slave/" },
	};

	static struct pcs_bc_cap_dir_tbl bc_type_tbl[] = {
		{ NRRC_PCS_BC_NRCA, "nr/nrca.bin" },
		{ NRRC_PCS_BC_ENDC, "endc/endc.bin" },
		{ NRRC_PCS_BC_TX_SW_NRCA, "nr/txswnrca.bin" },
		{ NRRC_PCS_BC_TX_SW_ENDC, "endc/txswendc.bin" },
		{ NRRC_PCS_BC_DAPS, "nr/daps.bin" },
	};

	int ret;
	ret = pcs_bc_cap_cat_file_dir_from_tbl(file_dir, len, sim_status_tbl,
					       ARRAY_SIZE(sim_status_tbl),
					       req->sim_status);
	if (ret) {
		pcs_log_e("cat sim_status[%d] fail", req->sim_status);
		pcs_dir_trans(NULL, 0, PCS_DIAG_CAUSE_CP_SET_DIR_ABNORMAL);
		return ret;
	}

	ret = pcs_bc_cap_cat_file_dir_from_tbl(file_dir, len, bc_type_tbl,
					       ARRAY_SIZE(bc_type_tbl),
					       req->bc_type);
	if (ret) {
		pcs_log_e("cat bc_type[%d] fail", req->bc_type);
		pcs_dir_trans(NULL, 0, PCS_DIAG_CAUSE_CP_SET_DIR_ABNORMAL);
		return ret;
	}

	return 0;
}

int pcs_bc_cap_get_path(char *path, u32 path_len, struct nrrc_pcs_bc_cap_req *req)
{
	char dir[PCS_BC_CAP_DIR_MAX_LEN + 1] = {0};
	char set_dir[PCS_BC_CAP_EXT_DIR_MAX_LEN] = {0};
	char file_dir[PCS_BC_CAP_EXT_DIR_MAX_LEN] = {0};
	int dir_len = pcs_get_bc_cap_dir(dir, sizeof(dir));
	int ret;

	if (dir_len <= 0) {
		pcs_log_e("pcs_get_bc_cap_dir error");
		return -ENOENT;
	}

	ret = pcs_bc_cap_get_set_dir(req, (const char *)dir, dir_len,
				     set_dir, sizeof(set_dir));
	if (ret)
		return ret;

	ret = pcs_bc_cap_get_file_dir(req, file_dir, sizeof(file_dir));
	if (ret)
		return ret;

	(void)memset_s(path, path_len, 0, path_len);

	ret |= memcpy_s(path, path_len, dir, dir_len);
	ret |= strncat_s(path, path_len, set_dir, strlen(set_dir));
	ret |= strncat_s(path, path_len, file_dir, strlen(file_dir));
	if (ret) {
		pcs_log_e("path cat fail");
		pcs_dir_trans(NULL, 0, PCS_DIAG_CAUSE_PATH_INVALID);
		return -EINVAL;
	}

	return 0;
}
