/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
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

#ifndef __BVP_VLAN_ADAPTER_H__
#define __BVP_VLAN_ADAPTER_H__

#include "bsp_vlan.h"
#include <bsp_print.h>
#include <linux/in.h>
#include <linux/net.h>
#include <linux/socket.h>
#include <mdrv_socket.h>
#include <mdrv_udi.h>
#include <osl_thread.h>
#include <osl_types.h>

#define THIS_MODU mod_vlan

#define vlan_err(fmt, ...) bsp_err(fmt, ##__VA_ARGS__)

#define USB_MAX_NUM 30
#define USB_USED_NUM 6
#define READ_BUFF_SIZE 1024

enum VLAN_CHAN_NAME {
    HIDS_CMD = 0,
    HIDS_DATA = 1,
    VLAN_TEST = 2
};


struct socketcfg {
    u64 fd;
    u32 acpfd;
    int family;
    int type;
    int protocol;
    int port;
    OSL_TASK_ID taskid;
    struct sockaddr skaddr;
    void *recv_buf;
    int recv_len;
};

struct chan_info {
    int chan_id;
    int socket_state;

    void (*readcb)(void);
    void (*writecb)(char *vaddr, char *paddr, int len);
    void (*eventcb)(ACM_EVT_E state);

    struct socketcfg *skcfg;
};


#endif
