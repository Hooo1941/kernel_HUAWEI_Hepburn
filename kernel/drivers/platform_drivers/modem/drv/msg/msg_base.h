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

#ifndef _MSG_BASE_H_
#define _MSG_BASE_H_

#include "msg_plat.h"

#define MDMMSG_TSKQ_MAX 0x10

struct msg_cookie {
    void *alloc_addr;
    uint32_t len;
    struct list_head m_list;
};

struct msg_tskque {
    unsigned queueid;
    unsigned pending;
    struct msg_tskque_grp *grp;
    struct list_head msg_list;
    struct msg_cookie *volatile r_msg;
    unsigned msg_cnt;
};

struct msg_tskque_grp {
    struct rb_node node;
    struct task_struct *tsk;
    unsigned pending;
    struct spinlock lock;
    uint32_t event;
    uint32_t event_req;
    uint32_t event_opt;
    struct msg_cookie *volatile r_msg;
    unsigned q_num;
    unsigned msg_cnt;
    struct msg_tskque queue[0];
};

/* sender只清除对应唤醒原因的pending位，recver需要根据这个弄清自己是被什么唤醒的
recver 在退出时清除所有pending位 */
#define PENDDING_ON_M (1 << 0)
#define PENDDING_ON_V (1 << 1)
#define PENDDING_ON_E (1 << 2)

static inline void PENDING_ZERO(unsigned *pending)
{
    *pending = 0;
}

static inline void PENDING_SET(unsigned *pending, unsigned field)
{
    *pending |= field;
}

static inline void PENDING_CLR(unsigned *pending, unsigned field)
{
    *pending &= (~field);
}

static inline int PENDING_ISZERO(const unsigned *pending)
{
    return *pending == 0;
}

static inline int PENDING_ISSET(const unsigned *pending, unsigned field)
{
    return (*pending & field) == field;
}

int msg_base_init(void);
int msg_tqg_create(struct task_struct *tsk, unsigned int qid_num);
struct msg_tskque_grp *msg_tqg_search(struct task_struct *tsk);
int msg_tskq_get_num(struct task_struct *tsk, unsigned int qid);
int msg_tskq_send(struct task_struct *tsk, unsigned int qid, void *msg, int send_flags);
int msg_tskq_recv(unsigned int qid, void **outmsg, unsigned int time_out);
int msg_evt_send(struct task_struct *tsk, unsigned int event);
int msg_evt_recv(unsigned int req_event, unsigned int opt, unsigned int time_out, unsigned int *rsp_event);
int msg_evt_vrecv(unsigned int req_event, unsigned int opt, unsigned int time_out, unsigned int *rsp_event);

#endif
