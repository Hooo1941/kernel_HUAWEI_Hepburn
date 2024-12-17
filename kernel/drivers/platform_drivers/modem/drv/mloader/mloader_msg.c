/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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
#include <securec.h>
#include <msg_id.h>
#include <bsp_msg.h>
#include <bsp_slice.h>
#include <bsp_mloader.h>
#include "mloader_err.h"
#include "mloader_msg.h"
#include "mloader_debug.h"
#include "mloader_load_image.h"
#include "mloader_load_patch.h"
#include "mloader_sec_call.h"

static msg_chn_t g_mloader_msghdl;

void mloader_msg_op_process(mloader_ap_msg_req *mloader_msg, mloader_ap_msg_result *result)
{
    int ret;
    const char *modem_fw_path = bsp_mloader_get_modem_fw_path();

    switch (mloader_msg->op) {
        case MLOADER_OP_GET_INIT_INFO:
            result->op = mloader_msg->op;
            result->vrl_size = bsp_mloader_get_vrl_size();
            ret = sprintf_s(result->modem_fw_path, sizeof(result->modem_fw_path), "%s", modem_fw_path);
            if (ret < 0) {
                mloader_print_err("fail to copy file path, ret = %d\n", ret);
            }
            break;
        case MLOADER_OP_VERIFY_IMAGE:
            result->op = mloader_msg->op;
            result->result = mloader_common_verify();
            break;
        case MLOADER_OP_PATCH_MSG:
            result->op = mloader_msg->op;
            result->result = mloader_patch_msg_process(mloader_msg->name, mloader_msg->patch_cmd, mloader_msg->patch_data);
            break;
        case MLOADER_OP_LOAD_FINISHED:
            mloader_print_err("succeed to load all modem image\n");
            mloader_save_patch_config_info();
            break;
        default:
            result->result = MLOADER_ERR_ARG;
            mloader_print_err("msg op err, op = 0x%x\n", mloader_msg->op);
            break;
    }
}
EXPORT_SYMBOL_GPL(mloader_msg_op_process);

static int mloader_msg_send(mloader_ap_msg_result *result)
{
    int ret;
    struct msg_addr dst;
    dst.core = MSG_CORE_TSP;
    dst.chnid = MSG_CHN_MLOADER;
    ret = bsp_msg_lite_sendto(g_mloader_msghdl, &dst, result, sizeof(mloader_ap_msg_result));
    if (ret) {
        mloader_print_err("fail to send msg to ccore, ret = %d\n", ret);
    }
    return ret;
}

static int mloader_msg_proc(const struct msg_addr *src, void *buf, u32 len)
{
    mloader_ap_msg_req *msg_info = NULL;
    mloader_ap_msg_result result_msg = { 0 };

    if ((src == NULL) || (buf == NULL) || (len != sizeof(mloader_ap_msg_req))) {
        mloader_print_err("mloader req msg err, len = %d\n", len);
        return MLOADER_ERR_ARG;
    }

    msg_info = (mloader_ap_msg_req *)buf;
    mloader_print_err("receive op %d for image %s\n", msg_info->op, msg_info->name);

    mloader_msg_op_process(msg_info, &result_msg);
    if (result_msg.op) {
        result_msg.request_id = msg_info->request_id;
        result_msg.time_stamp = bsp_get_slice_value();
        mloader_msg_send(&result_msg);
        mloader_print_err("succeed to send response\n");
    }

    return 0;
}

int mloader_msg_init(void)
{
    int ret;
    struct msgchn_attr lite_attr = { 0 };

    bsp_msgchn_attr_init(&lite_attr);
    lite_attr.chnid = MSG_CHN_MLOADER;
    lite_attr.coremask = MSG_CORE_MASK(MSG_CORE_TSP);
    lite_attr.lite_notify = mloader_msg_proc;
    ret = bsp_msg_lite_open(&g_mloader_msghdl, &lite_attr);
    if (ret) {
        mloader_print_err("fail to open msg, ret = %d\n", ret);
        return ret;
    }
    return 0;
}
