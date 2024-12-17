/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
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
#include <osl_types.h>
#include <linux/kthread.h>
#include <msg_id.h>
#include <mdrv_msg.h>
#include <bsp_odt.h>
#include "srvchan.h"
#include "encoder.h"
#include "connectsrv.h"
#include "configsrv.h"
#include "logreport.h"
#include "diag_debug.h"
#include "transfertask.h"


static u32 transfertask_msg_send(struct ttmsg_info *ttmsg_info)
{
    u32 ret, tmplen;
    u32 offset = 0;
    struct transfertask_msg *txttmsg = NULL;
    u32 length = sizeof(struct transfertask_msg) + ttmsg_info->datasz + ttmsg_info->datahdsz;

    txttmsg = (struct transfertask_msg *)mdrv_msg_alloc(ttmsg_info->src_id, length, MSG_FULL_LEN);
    if (txttmsg == NULL) {
        diag_err("transfertask msg malloc fail\n");
        return DIAG_TRANSFERTASK_MALLOC_ERR;
    }

    MDRV_MSG_HEAD_FILL(txttmsg, ttmsg_info->src_id, ttmsg_info->dst_id, length - sizeof(struct msg_head));

    if (ttmsg_info->datahd != NULL) {
        tmplen = length - sizeof(struct msg_head);
        if (memcpy_s(txttmsg->data, tmplen, ttmsg_info->datahd, ttmsg_info->datahdsz)) {
            diag_err("transfer task msg head send memcpy fail\n");
        }
    }

    if (ttmsg_info->data != NULL) {
        offset = ttmsg_info->datahdsz;
        tmplen = length - sizeof(struct msg_head) - ttmsg_info->datahdsz;
        if (memcpy_s(txttmsg->data + ttmsg_info->datahdsz, tmplen, \
            ttmsg_info->data, ttmsg_info->datasz)) {
            diag_err("transfer task msg data send memcpy fail\n");
        }
    }

    ret = mdrv_msg_send(txttmsg, 0);
    if (ret) {
        diag_err("msg send fail:%d\n", ret);
        mdrv_msg_free(ttmsg_info->src_id, (void *)txttmsg);
    }

    return ret;
}

u32 send_ttmsg_to_tsp(struct ipc_ttmsg *tspttmsg, u8 *data, u32 datasz)
{
    u32 ret;
    struct ttmsg_info ttmsg_info = {0};

    ttmsg_info.src_id = DRV_MID_DIAG_APSS;
    ttmsg_info.dst_id = DRV_MID_DIAG_TSP;
    ttmsg_info.datahdsz = sizeof(struct ipc_ttmsg);
    ttmsg_info.datahd = (u8 *)tspttmsg;
    ttmsg_info.datasz = datasz;
    ttmsg_info.data = data;
    ret = transfertask_msg_send(&ttmsg_info);
    if (ret) {
        diag_err("trans msg to tsp fail:%d\n", ret);
    }

    return ret;
}

static void handle_tsp_ttmsg(struct ipc_ttmsg *tspttmsg, u32 msglen)
{
    if (msglen != (tspttmsg->datasz + sizeof(struct ipc_ttmsg))) {
        diag_err("recv invalid msg id:0x%x, datasz:0x%x, msglen:0x%x\n", \
            tspttmsg->msgid, tspttmsg->datasz, msglen);
        return;
    }

    switch (tspttmsg->msgid) {
        case TTMSG_CNTSRV_CONNECT:
        case TTMSG_CNTSRV_AUTHSTAT:
        case TTMSG_CNTSRV_RXREADY:
        case TTMSG_CNTSRV_DISCONNECT:
            connectsrv_msg_handle(tspttmsg->msgid, tspttmsg->data, tspttmsg->datasz);
            break;
        case TTMSG_PRINTLOG_CNF:
        case TTMSG_DRVPRINT_CNF:
        case TTMSG_TRACELOG_CNF:
        case TTMSG_TRANSLOG_CNF:
        case TTMSG_MNTN_CNF:
        case TTMSG_FLOWTEST_CNF:
            configsrv_msg_handle(tspttmsg->msgid, tspttmsg->data, tspttmsg->datasz);
            break;
        case TTMSG_LOG_CONFIG:
            logreport_log_config(tspttmsg->data, tspttmsg->datasz);
            break;
        case TTMSG_SRVPORT_SET:
            if (tspttmsg->datasz != sizeof(u32)) {
                return;
            }
            switch_srvport_type((u32)(*tspttmsg->data));
            break;
        case TTMSG_DSTCHCFG_QUERY:
            encoder_dstch_cfg_query();
            break;
        case TTMSG_DSTMODE_SET:
            if (tspttmsg->datasz != sizeof(u32)) {
                return;
            }
            encoder_atchange_cntworkmode((u32)(*tspttmsg->data));
            break;
        default:
            diag_err("get invalid tsp tranfer task msg:%d\n", tspttmsg->msgid);
            break;
    }
}

#define MSG_QID_DIAG_APSS 0
#define MSG_QNUM_DIAG_APSS 4
static int transfertask(void *data)
{
    u32 ret;
    struct transfertask_msg *ttmsg = NULL;

    for (;;) {
        ret = mdrv_msg_recv(MSG_QID_DIAG_APSS, (void **)&ttmsg, 0xFFFFFFFF);
        if (ret) {
            diag_err("transfer task recv msg fail:%d\n", ret);
            continue;
        }

        if (ttmsg == NULL) {
            diag_err("transfer task recv null msg\n");
            continue;
        }

        switch (ttmsg->src_id) {
            case DRV_MID_DIAG_TSP:
                handle_tsp_ttmsg((struct ipc_ttmsg *)ttmsg->data, ttmsg->data_len);
                break;

            default:
                diag_err("transfer task get invalid msg, src mid:0x%x\n", ttmsg->src_id);
                break;
        }

        mdrv_msg_free(DRV_MID_DIAG_APSS, (void *)ttmsg);
    }

    return 0;
}

u32 transfertask_init(void)
{
    u32 ret;
    struct task_struct *tsk = NULL;

    tsk = kthread_create(transfertask, NULL, "transfertask");
    if (IS_ERR(tsk)) {
        diag_err("transfer task create fail\n");
        return DIAG_TRANSFERTASK_CREATE_ERR;
    }

    ret = mdrv_msg_qcreate(tsk->pid, MSG_QNUM_DIAG_APSS);
    if (ret) {
        diag_err("transfer task create queue fail:%d\n", ret);
        return ret;
    }

    ret = mdrv_msg_register_mid(tsk->pid, DRV_MID_DIAG_APSS, MSG_QID_DIAG_APSS);
    if (ret) {
        diag_err("transfer task register msg mid fail:%d\n", ret);
        return ret;
    }

    wake_up_process(tsk);

    return 0;
}

