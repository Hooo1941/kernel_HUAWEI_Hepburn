/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2018. All rights reserved.
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
#include <bsp_fipc.h>
#include <bsp_slice.h>
#include <bsp_nvim.h>
#include "nv_comm.h"
#include "nv_msg.h"

struct nvm_msg_info *nv_get_msginfo(void)
{
    struct nvm_info *nvminfo = nv_get_nvminfo();

    return &nvminfo->msg_info;
}

struct nv_msg_element *nv_alloc_msgelement(u32 data_len)
{
    struct nv_msg_element *msg = NULL;

    msg = nv_malloc(sizeof(struct nv_msg_element));
    if (msg == NULL) {
        nv_printf("msg malloc fail\n");
        return NULL;
    }

    msg->msg_data = nv_malloc(sizeof(struct nv_msg_data) + data_len);
    if (msg->msg_data == NULL) {
        nv_free(msg);
        nv_printf("msg data free fail\n");
        return NULL;
    }

    (void)memset_s(msg->msg_data->data, data_len, 0, data_len);

    msg->msg_data->magicnum = 0;
    msg->msg_data->slice = 0;
    msg->msg_data->msg_type = 0;
    msg->msg_data->msg = 0;
    msg->msg_data->sn = 0;
    msg->msg_data->ret = 0;
    msg->msg_data->rcvlen = 0;
    msg->msg_data->data_len = data_len;
    INIT_LIST_HEAD(&msg->list);

    return msg;
}

u32 nv_free_msgelement(struct nv_msg_element *msg)
{
    u32 ret = NV_OK;

    nv_free(msg->msg_data);
    nv_free(msg);
    return ret;
}

struct nv_msg_element *nv_get_msg_element(void)
{
    unsigned long flags;
    struct nv_msg_element *msg_element = NULL;
    struct msg_queue *msgqueue = NULL;
    struct nvm_msg_info *msg_info = nv_get_msginfo();

    msgqueue = &msg_info->msgqueue;

    spin_lock_irqsave(&msgqueue->lock, flags);
    if (!list_empty(&msgqueue->list)) {
        msg_element = list_first_entry(&msgqueue->list, struct nv_msg_element, list);
        list_del(&msg_element->list);
    }
    spin_unlock_irqrestore(&msgqueue->lock, flags);

    if (msg_element != NULL) {
        return msg_element;
    }

    msgqueue = &msg_info->lwmsgqueue;

    spin_lock_irqsave(&msgqueue->lock, flags);
    if (!list_empty(&msgqueue->list)) {
        msg_element = list_first_entry(&msgqueue->list, struct nv_msg_element, list);
        list_del(&msg_element->list);
    }
    spin_unlock_irqrestore(&msgqueue->lock, flags);

    return msg_element;
}

u32 nv_awake_send_msg(struct nv_msg_data *msg_data, struct nv_msg_data *msg)
{
    u32 ret = 0;
    struct nvm_info *nvminfo = nv_get_nvminfo();

    msg->magicnum = msg_data->magicnum;
    msg->slice = bsp_get_slice_value();
    msg->msg_type = msg_data->msg_type;
    msg->msg = msg_data->msg;
    msg->sn = msg_data->sn;
    if (msg_data->data_len != 0) {
        ret = bsp_fipc_chn_recv(nvminfo->msg_info.recvchn.chn, msg->data, msg->data_len, 0);
        if (ret != msg->data_len) {
            nv_printf("nv read fipc msg fail, ret:%d\n", ret);
            return ret;
        }
    }
    return NV_OK;
}
u32 nv_awake_msgtask(struct nv_msg_data *msg_data)
{
    u32 ret = 0;
    unsigned long flags;
    struct nv_msg_data *msg = NULL;
    struct nv_msg_element *msg_element = NULL;
    struct msg_queue *msgqueue = NULL;
    struct nvm_info *nvminfo = nv_get_nvminfo();

    switch (msg_data->msg_type) {
        case NV_ICC_REQ_FLUSH:
        case NV_ICC_REQ_FLUSH_RWNV:
        case NV_ICC_REQ_LOAD_BACKUP:
        case NV_ICC_REQ_UPDATE_DEFAULT:
        case NV_ICC_REQ_UPDATE_BACKUP:
        case NV_ICC_REQ_UPDATE_BACKUP_BK:
        case NV_ICC_REQ_FACTORY_RESET:
        case NV_ICC_REQ_DATA_WRITEBACK:
        case NV_ICC_REQ_READ_BLK:
        case NV_ICC_REQ_SET_PATCH_FLAG:
        case NV_ICC_REQ_RD_PATCH_STATE:
        case NV_ICC_REQ_WR_PATCH_INFO:
        case NV_ICC_REQ_GET_PRODUCTID:
        case NV_ICC_REQ_WRITE_SCBLK:
            msgqueue = &nvminfo->msg_info.msgqueue;
            break;

        case NV_ICC_REQ_FLUSH_RDWR_ASYNC:
            msgqueue = &nvminfo->msg_info.lwmsgqueue;
            break;
        default:
            return BSP_ERR_NV_ICC_CHAN_ERR;
    }

    msg_element = nv_alloc_msgelement(msg_data->data_len);
    if (msg_element == NULL) {
        return BSP_ERR_NV_MALLOC_FAIL;
    }
    msg = msg_element->msg_data;
    ret = nv_awake_send_msg(msg_data, msg);
    if (ret != NV_OK) {
        (void)nv_free_msgelement(msg_element);
        return ret;
    }

    spin_lock_irqsave(&msgqueue->lock, flags);
    list_add_tail(&msg_element->list, &msgqueue->list);
    spin_unlock_irqrestore(&msgqueue->lock, flags);

    osl_sem_up(&nvminfo->task_sem);

    return NV_OK;
}

u32 nv_msg_init(void)
{
    u32 ret;
    struct nvm_msg_info *msg_info = nv_get_msginfo();

    spin_lock_init(&msg_info->msgqueue.lock);
    INIT_LIST_HEAD(&msg_info->msgqueue.list);

    spin_lock_init(&msg_info->lwmsgqueue.lock);
    INIT_LIST_HEAD(&msg_info->lwmsgqueue.list);

    ret = nv_cpmsg_chan_init(msg_info);
    if (ret != NV_OK) {
        nv_printf("nv cp msg chan init fail\n");
        return ret;
    }

    nv_printf("nv msg init success\n");
    return NV_OK;
}
