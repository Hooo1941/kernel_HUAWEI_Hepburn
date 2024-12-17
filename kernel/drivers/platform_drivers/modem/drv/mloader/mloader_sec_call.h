/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
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

#ifndef _MLOADER_SEC_CALL_H_
#define _MLOADER_SEC_CALL_H_

#define MAX_IMG_NAME_LEN (32)

// 以下宏需要和镜像名匹配，镜像名修改时需要同步修改此处。同时需要和tee宏保持一致。
#define MODEM_PARTITION_NM "modem_fw"
#define MODEM_CCPU_IMG_NM "modem_tsp"
#define MODEM_COLD_PATCH_IMG_NM "modem_tsp_p"
#define MODEM_LPMCU_IMG_NM "lpmcu_modem"
#define MODEM_CCPU_TEE_IMG_NM "modem_mtee"
#define MODEM_CCPU_TEE_PATCH_IMG_NM "modem_mtee_p"
#define MODEM_FW_DTB_IMG_NM "modem_fw_dtb"
#define MODEM_CCPU_DTB_IMG_NM "modem_dt"
#define MODEM_CCPU_DTO_IMG_NM "modem_dtcust"
#define MODEM_COMM_IMG_NM "modem_common_img"

struct img_id_name_tbl {
    mloader_img_id_e img_type;
    char img_name[MAX_IMG_NAME_LEN];
};

int mloader_verify_prepare(mloader_img_id_e image);
int mloader_common_verify(void);
void mloader_load_notifier_process(mloader_stage_e stage, int result, mloader_img_id_e img_id);

#endif
