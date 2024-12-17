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
#ifndef __NLSRVPORT_H__
#define __NLSRVPORT_H__

#include <linux/netlink.h>
#include <bsp_homi.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define SRVCHAN_NLMSG_CMD 1

#define SRVCHAN_NLMSG_DATA 2

enum {
    SRVCHAN_NLMSG_BASE = 0,
    SRVCHAN_NLBIND_PORT,
    SRVCHAN_NLUNBIND_PORT,

    SRVCHAN_NLSWITCH_PORT = SRVCHAN_NLMSG_BASE + 0x65,
    SRVCHAN_NLGET_PORT_TYPE,
    SRVCHAN_NLSET_LOG_MODE,
    SRVCHAN_NLGET_LOG_MODE,

    SRVCHAN_NLHANDLE_CMD = SRVCHAN_NLMSG_BASE + 0x12D,

    SRVCHAN_NLHANDLE_DATA = SRVCHAN_NLMSG_BASE + 0x1F5,
};

/* srvchan netlink msg */
struct srvchan_nlmsg {
    u16 msg_type;
    u16 op;
    union {
        u32 args_length;
        u32 result;
    };
    union {
        u32 args_start;
        struct {
            u32 pid;
        } port;
        struct {
            u32 port;
        } log_port_config;
        struct {
            u32 mode;
        } log_config;
        struct {
            u32 enable;
        } log_nve_config;
        struct {
            u32 length;
            u8 data[0];
        } om_command;
    };
};
/* srvchan netlink log msg */
struct srvchan_nllog {
    u16 msg_type;
    u16 op;
    u32 args_length;
    union {
        u32 args_start;
        struct {
            u32 msg_seq;
            u32 blk_num;
            u32 blk_seq;
            u8 mode;
            u8 reserved[0x3];
            u32 length;
            u8 data[0];
        } om_data;
    };
};

#define SRVCHAN_NLLOG_SIZE (NLMSG_DEFAULT_SIZE - sizeof(struct srvchan_nllog))

void nlsrvpoint_data_recv(struct homi_chan *chan, void *data, int len);

u32 nlsrvport_data_send(u32 srvpoint, u8 *data, int len);

u32 nlsrvport_init(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif   /* __NLSRVPORT_H__ */
