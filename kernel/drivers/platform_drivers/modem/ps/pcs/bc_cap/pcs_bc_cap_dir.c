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
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/module.h>

#include "securec.h"

#include "product_config_ps_ap.h"
#include "pcs_bc_cap_get_path.h"
#include "pcs_log.h"
#include "pcs_def.h"

char pcs_bc_cap_proc_dir[PCS_BC_CAP_DIR_MAX_LEN + 1];

#define PCS_BC_CAP_PROC_PERMISSION 0660

ssize_t pcs_bc_cap_proc_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
	if (offset == NULL) {
		pcs_log_e("offset is NULL");
		return -EINVAL;
	}

	return simple_read_from_buffer(buf, len, offset,
				       pcs_bc_cap_proc_dir,
				       strlen(pcs_bc_cap_proc_dir) + 1);
}

ssize_t pcs_bc_cap_proc_write(struct file *file, const char __user *buf,
			      size_t len, loff_t *offset)
{
	ssize_t ret;

	if (len >= sizeof(pcs_bc_cap_proc_dir)) {
		pcs_log_e("len too long");
		return -EINVAL;
	}

	if (offset == NULL) {
		pcs_log_e("offset is NULL");
		return -EINVAL;
	}

	ret = simple_write_to_buffer(pcs_bc_cap_proc_dir,
				     sizeof(pcs_bc_cap_proc_dir) - 1,
				     offset, buf, len);

	pcs_bc_cap_proc_dir[len] = 0;
	return ret;
}

STATIC const struct proc_ops pcs_bc_cap_proc_ops = {
	.proc_read  = pcs_bc_cap_proc_read,
	.proc_write = pcs_bc_cap_proc_write,
};

STATIC int pcs_bc_cap_init_proc(void)
{
	struct proc_dir_entry *proc_entry = NULL;

	proc_entry = proc_create("bc_cap_dir", PCS_BC_CAP_PROC_PERMISSION, NULL,
				 &pcs_bc_cap_proc_ops);
	if (proc_entry == NULL) {
		pcs_log_e("proc_create fail");
		return -ENOMEM;
	}

	(void)memset_s(pcs_bc_cap_proc_dir, sizeof(pcs_bc_cap_proc_dir), 0,
		       sizeof(pcs_bc_cap_proc_dir));


	return 0;
}

STATIC int pcs_bc_cap_get_dir_cb(char **data)
{
	size_t dir_len = strlen(pcs_bc_cap_proc_dir) + 1;

	pcs_log_h("entry");

	if (dir_len <= 1) {
		pcs_log_e("len invalid");
		return -1;
	}

	*data = pcs_bc_cap_proc_dir;

	return dir_len;
}

int pcs_bc_cap_init_get_dir(void)
{
	int ret = pcs_bc_cap_init_proc();
	if (ret)
		return ret;

	if (pcs_get_bc_cap_cb() == NULL)
		pcs_reg_get_bc_cap_dir_cb(pcs_bc_cap_get_dir_cb);

	return 0;
}