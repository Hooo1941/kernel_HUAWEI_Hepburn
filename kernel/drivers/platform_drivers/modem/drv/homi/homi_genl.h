/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS"
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

#ifndef HOMI_GENETLINK_H
#define HOMI_GENETLINK_H

#include <bsp_homi.h>

#undef THIS_MODU
#define THIS_MODU mod_homi

#define HOMI_GENL_NAME "hmi_genl"
#define HOMI_GENL_VERSION 0x1

typedef enum {
    HOMI_ERR_PORT_NULL = 0x100,
    HOMI_ERR_MSG_NEW_FAIL,
    HOMI_ERR_MSG_PUT_FAIL,
    HOMI_ERR_BUTT
} homi_genl_errno_e;

typedef struct {
    unsigned int msg_seq;
    unsigned int blk_num;
    unsigned int blk_seq;
    unsigned int reserved;
    unsigned int len;
    unsigned char *data;
} homi_genl_data_desc_s;

typedef struct {
    unsigned short msg_type;
    unsigned short op;
    unsigned int args_length;
    union {
        unsigned int args_start;
        struct {
            unsigned int msg_seq;
            unsigned int blk_num;
            unsigned int blk_seq;
            unsigned int reserved;
            unsigned int length;
            unsigned char data[0];
        } send_data;
    };
} homi_genl_unicast_msg_s;

#define HOMI_GENL_DATA_SIZE (NLMSG_DEFAULT_SIZE - HOMI_GENL_HDR_LEN - sizeof(homi_genl_unicast_msg_s))
#define HOMI_GENL_UNICAST_MSG_ARGS_SIZE(_m) (sizeof(((homi_genl_unicast_msg_s *)0)->_m))

homi_genl_ctrl_s *homi_get_ctrl_info(void);
int homi_genl_init(void);

#endif
