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

#ifndef __NV_MSG_H__
#define __NV_MSG_H__

#include <osl_types.h>
#include <osl_list.h>
#include <osl_sem.h>
#include <osl_spinlock.h>
#include "bsp_fipc.h"
#include <product_config.h>

#define NV_MSG_MAGICNUM (0x766e564e)
#define NV_MSG_TIMEOUT (5000) /* max timeout 50s */

#define NV_MSG_SEND_COUNT 5

#define NV_CHANNAL_FULL FIPC_ERR_EAGAIN

/* icc msg type */
enum nv_msg_type {
    NV_ICC_REQ_READ_BLK = 0x39,
    NV_ICC_REQ_FLUSH,
    NV_ICC_REQ_FLUSH_RWNV,
    NV_ICC_REQ_LOAD_BACKUP,
    NV_ICC_REQ_UPDATE_DEFAULT,
    NV_ICC_REQ_UPDATE_BACKUP,
    NV_ICC_REQ_FACTORY_RESET,
    NV_ICC_REQ_FLUSH_RDWR_ASYNC,
    NV_ICC_REQ_DATA_WRITEBACK,
    NV_ICC_REQ_UPDATE_BACKUP_BK,
    NV_ICC_REQ_SET_PATCH_FLAG,
    NV_ICC_REQ_RD_PATCH_STATE,
    NV_ICC_REQ_WR_PATCH_INFO,
    NV_ICC_REQ_GET_PRODUCTID,
    NV_ICC_CNF,
    NV_ICC_REQ_WRITE_SCBLK,
    NV_ICC_BUTT = 0xFF,
};


struct nv_send_msg {
    u32 msg_type;
    u32 msg;
    u32 data_len;
    u8 *data;
    u32 rcvmsg_len;
};

struct nv_msg_data {
    u32 magicnum;
    u32 slice;
    u32 msg_type;
    u32 msg;
    u32 sn;
    u32 ret;
    u32 rcvlen;
    u32 data_len;
    u8 data[0];
};

struct nv_msg_element {
    struct list_head list;
    struct nv_msg_data *msg_data;
};

struct msg_queue {
    struct list_head list;
    spinlock_t lock;
};

#define NV_FIPC_BUF_ALIGN 32
#define NV_FIPC_A2C_BUF_LEN (24 * 1024)
#ifdef BSP_CONFIG_PHONE_TYPE
#define NV_FIPC_C2A_BUF_LEN (16 * 1024)  /* phone版本传输productid最大4000个，一个id4字节 */
#else
#define NV_FIPC_C2A_BUF_LEN (256)
#endif
#define NV_FIPC_MAX_LEN (NV_FIPC_A2C_BUF_LEN - 128)
#define NV_MSG_LEN NV_FIPC_C2A_BUF_LEN
#define NV_FIPC_C2A_MAX_LEN (NV_FIPC_C2A_BUF_LEN - 128)

struct nv_fipc_chan {
    fipc_chn_t chn;
    fipc_chn_attr_t attr;
};

struct nvm_msg_info {
    u32 msg_sn;
    u32 cb_count;
    u32 cb_reply;
    u32 timeoutcnt;
    struct msg_queue msgqueue;
    struct msg_queue lwmsgqueue; /* low priority msg queue */
    struct nv_fipc_chan recvchn;
    struct nv_fipc_chan sendchn;
};

struct nvm_msg_info *nv_get_msginfo(void);

u32 nv_free_msgelement(struct nv_msg_element *msg);

struct nv_msg_element *nv_get_msg_element(void);

u32 nv_awake_msgtask(struct nv_msg_data *msg_data);

u32 nv_cpmsg_send(u8 *pdata, u32 len);

void nv_cpmsg_cb(struct nv_msg_data *msg);

u32 nv_cpmsg_chan_init(struct nvm_msg_info *msg_info);

u32 nv_msg_init(void);
s32 nv_init_dev(void);

#endif
