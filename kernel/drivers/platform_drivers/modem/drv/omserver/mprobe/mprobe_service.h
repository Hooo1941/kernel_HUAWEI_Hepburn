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

#ifndef __MPROBE_SERVICE_H__
#define __MPROBE_SERVICE_H__

#include <linux/device.h>
#include <linux/version.h>
#include <linux/pm_wakeup.h>
#include <product_config.h>
#include <osl_types.h>
#include <bsp_odt.h>
#include <bsp_omp.h>
#include <bsp_mprobe.h>


typedef enum {
    MPROBE_MSGID_HOST_CONNECT = 0,
    MPROBE_MSGID_HOST_DISCONNECT = 1,
    MPROBE_MSGID_BUTT,
} mprobe_msgid_e;

#define MPROBE_TMST_LEN 4
#define MPROBE_PDT_INFO 64
#define MPROBE_SN_LEN 36

typedef struct {
    u32 auid;      /* 原AUID */
    int result;    /* 结果码 */
    u16 authflag;
    u16 authver;
    char sn[MPROBE_SN_LEN];
    char product_info[MPROBE_PDT_INFO];
    char time_stamp[0x8];
    unsigned long aslr;
} mprobe_conn_info_s;

/* 获取单板信息 */
typedef struct {
    u32 auid;
    u32 sn;
    u32 cmd; /* 命令:区分查询版本号、鉴权和连接 */
} mprobe_get_board_info_s;

#define MPROBE_PAYLOAD_MSG_ID(data) ((u32)(((mprobe_frame_head_s *)(data))->msg_id.msgid_16b))

typedef struct {
    struct wakeup_source *wakeup_lock;
} mprobe_srv_ctrl_s;

typedef struct {
    u32 compid;
    mprobe_service_func srv_func;
    mprobe_notify_func notify_fn;
} mprobe_service_s;

int mprobe_notify_port_disconn(void);

void mprobe_srv_init(void);

#endif
