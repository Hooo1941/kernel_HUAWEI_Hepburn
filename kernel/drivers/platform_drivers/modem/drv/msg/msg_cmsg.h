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


#ifndef __MSG_CMSG_H__
#define __MSG_CMSG_H__

#include <linux/version.h>
#include <linux/device.h>
#include <linux/semaphore.h>
#include <linux/dma-mapping.h>
#include <linux/of_device.h>
#include <linux/kthread.h>
#include "msg_plat.h"
#include "bsp_fipc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CMSG_SEND_TRY_WAIT_TIMES 200

#define MSG_CHN_STATUS_UNINIT 0
#define MSG_CHN_STATUS_INITING 1
#define MSG_CHN_STATUS_INITED 2

#define MSG_SHARE_TSK_NAMESZ 16
#define MSG_SHARE_TSK_PRIO 4
#define MSG_SHARE_TSK_STKSZ 4096

#define MSG_FIPC_CHN 0x1
#define MSG_IPCMSG_CHN 0x2

/* 如果使用的通道数量超出最大值，该宏要扩大 */
#define CUSTOM_CHANNEL_MAX 0x10

#define MSG_PHYCHN_SEND(mode) ((mode) & 0x1)
#define MSG_PHYCHN_RECV(mode) ((mode) & 0x2)

#define FIPC_DEFAULT_CHN      0x100
#define FIPC_TRANSPARENT_CHN  0x101
#define FIPC_MEGA_CHN         0x102

#define IPCMSG_DEFAULT_CHN      0x200
#define IPCMSG_TRANSPARENT_CHN  0x201

typedef struct {
    unsigned cnt;
    struct {
        void *buf;
        unsigned len;
    } datablk[0x3];
} msg_blkx3_desc_t;

struct cmsg_task_t {
    u32 status;
    struct task_struct *worker;
    struct semaphore worker_sem;
    struct wakeup_source *worker_wakelock;
};

struct channel_ctrl_t;
struct cmsg_ctrl_t;
struct channel_ops {
    int (*phy_init)(struct channel_ctrl_t *chn_ctrl, device_node_s *pnode, struct cmsg_ctrl_t* cmsg_ctrl);
    int (*s_open)(struct channel_ctrl_t* chn_ctrl);
    int (*r_open)(struct channel_ctrl_t* chn_ctrl);
    int (*send)(struct channel_ctrl_t *chn_ctrl, msg_blkx3_desc_t *send_desc);
    int (*recv)(struct channel_ctrl_t *chn_ctrl, void *buf, u32 len);
    int (*task_init)(struct channel_ctrl_t *chn_ctrl);
    int (*nextpkt_len)(struct channel_ctrl_t *chn_ctrl);
};

struct channel_ctrl_t {
    u32 cid;
    u32 mode;
    u32 phychnid;
    u32 smsg_sz;
    struct channel_ops *ops;
    void *cmsg_chn;
    void *msg_hdl;
    struct spinlock s_lock;
    struct cmsg_task_t cmsg_task;
};

struct cmsg_ctrl_t {
    u32 custom_ctrl_idx;
    struct channel_ctrl_t *default_ctrl[MSG_CORE_MAX];
    struct channel_ctrl_t *custom_ctrl[CUSTOM_CHANNEL_MAX];
};

struct fipc_cmsg_chn_t {
    u32 buf_sz;
    fipc_chn_t handle;
};

struct channel_device_match {
    struct channel_ops *ops;
    u32 mode;
    const char *device_name;
};

struct channel_map {
    u32 logic_id;
    u32 phy_id;
};

int msg_crosscore_init(void);
int msg_crosscore_send(unsigned cid, void *msg);
int msg_crosscore_send_lite(const struct msg_addr *src_addr, const struct msg_addr *dst_addr, void *buf, u32 len);
int msg_recv_process(struct channel_ctrl_t *chn_ctrl);
int get_phy_chnid(u32 chnid);
struct cmsg_ctrl_t* get_cmsg_ctrl(void);
int cmsg_default_task_init(struct channel_ctrl_t *chn_ctrl);
void *msg_dma_buf_alloc(size_t size, dma_addr_t *dma_handle);
void msg_dma_buf_free(size_t size, void *cpu_addr, dma_addr_t dma_handle);

#ifdef __cplusplus
}
#endif
#endif
