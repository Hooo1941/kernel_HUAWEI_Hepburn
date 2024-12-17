/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#include <linux/version.h>
#include <linux/errno.h>
#include <linux/vmalloc.h>

#include "msg_plat.h"
#include "msg_base.h"
#include "msg_mem.h"

struct msg_mem_dbg {
    atomic_t busy_cnt;
};

struct msg_mem_dbg g_msg_mem_dbg = { ATOMIC_INIT(0) };

/*
消息内存格式
|---for align padding(maybe zero)--|---struct msg_cookie---|---struct msg_head---|----payload----|
|---------------for mem management(alloc&free)------|---for user headr----|----payload----|
struct msg_cookie 用于并嵌入链表结构用于msgqueue
*/
void *mdrv_msg_alloc(unsigned mid, unsigned len, unsigned flag)
{
    unsigned alloc_len;
    void *mem = NULL;

    struct msg_cookie *msg_prefix = NULL;
    void *retmsg = NULL;

    if (!len) {
        msg_err("mid[%d] param invaild\n", mid);
        return NULL;
    }
    if (len > MSG_MEM_ALLOC_MAX) {
        msg_err("warning: mid[%d] alloc mem too large len = %d\n", mid, len);
    }

    if (flag & MSG_FULL_LEN) {
        alloc_len = sizeof(struct msg_cookie) + len;
    } else {
        alloc_len = sizeof(struct msg_cookie) + len + MDRV_MSG_HEAD_LEN;
    }
    mem = vmalloc(alloc_len);
    if (IS_ERR_OR_NULL(mem)) {
        return NULL;
    }
    msg_prefix = (struct msg_cookie *)mem;
    msg_prefix->len = len;
    atomic_inc(&g_msg_mem_dbg.busy_cnt);

    retmsg = (void *)((char *)mem + sizeof(struct msg_cookie));
    MSG_HEAD_SET_FLAG(retmsg, 0);

    return retmsg;
}

void mdrv_msg_free(unsigned mid, void *addr)
{
    void *mem = NULL;

    struct msg_head *msg_head = NULL;
    struct msg_flag *msg_flag = NULL;
    if (addr == NULL) {
        msg_err("mid[%d] param invaild\n", mid);
        return;
    }
    msg_head = (struct msg_head *)addr;
    msg_flag = (struct msg_flag *)(&msg_head->flag);
    if (msg_flag->mem_reserve == MSG_MEM_RESERVED) {
        return;
    }
    mem = (void *)((char *)addr - sizeof(struct msg_cookie));

    vfree(mem);
    atomic_dec(&g_msg_mem_dbg.busy_cnt);

    return;
}

int mdrv_msg_reserve(unsigned mid, void *addr)
{
    struct msg_head *msg_head = NULL;
    struct msg_flag *msg_flag = NULL;
    if (addr == NULL) {
        msg_err("mid[%d] param invaild\n", mid);
        return -1;
    }
    msg_head = (struct msg_head *)addr;
    msg_flag = (struct msg_flag *)(&msg_head->flag);
    msg_flag->mem_reserve = MSG_MEM_RESERVED;
    return 0;
}

int mdrv_msg_unreserve(unsigned mid, void *addr)
{
    struct msg_head *msg_head = NULL;
    struct msg_flag *msg_flag = NULL;
    if (addr == NULL) {
        msg_err("mid[%d] param invaild\n", mid);
        return -1;
    }
    msg_head = (struct msg_head *)addr;
    msg_flag = (struct msg_flag *)(&msg_head->flag);
    msg_flag->mem_reserve = MSG_MEM_UNRESERVED;
    return 0;
}

int mdrv_msg_reserve_status(unsigned mid, void *addr, unsigned *status)
{
    struct msg_head *msg_head = NULL;
    struct msg_flag *msg_flag = NULL;
    if (addr == NULL || status == NULL) {
        msg_err("mid[%d] param invaild\n", mid);
        return -1;
    }
    msg_head = (struct msg_head *)addr;
    msg_flag = (struct msg_flag *)(&msg_head->flag);
    *status = msg_flag->mem_reserve;
    return 0;
}

int msg_mem_info(void)
{
    msg_always("busy_cnt: 0x%x\n", (unsigned)atomic_read(&g_msg_mem_dbg.busy_cnt));
    return 0;
}

EXPORT_SYMBOL(mdrv_msg_alloc);
EXPORT_SYMBOL(mdrv_msg_free);
EXPORT_SYMBOL(mdrv_msg_reserve);
EXPORT_SYMBOL(mdrv_msg_unreserve);
EXPORT_SYMBOL(mdrv_msg_reserve_status);
