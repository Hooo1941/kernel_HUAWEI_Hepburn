/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
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


#ifndef _ESL_SOCKET_CHANNEL_CTR_H
#define _ESL_SOCKET_CHANNEL_CTR_H

#include "esl_socket_os_interface.h"

#define SIMSOCK_MAX 16
#define SIMSOCK_CHAN_MAX 16

#define SIMSOCK_MAX_READFDS 32

#define SIMSOCK_FDS_SIZE (sizeof(int) * SIMSOCK_MAX)

#define SIMSOCK_VERSION 0x0
#define SIMSOCK_HWPARAMS 0x4
#define SIMSOCK_CHINTSTATS 0x8

#define SIMSOCK_OP(ch) (0x10 + (ch)*60 + 0x0)
#define SIMSOCK_RESULT(ch) (0x10 + (ch)*60 + 0x4)
#define SIMSOCK_PARAM0(ch) (0x10 + (ch)*60 + 0x8)
#define SIMSOCK_PARAM1(ch) (0x10 + (ch)*60 + 0xc)
#define SIMSOCK_PARAM2(ch) (0x10 + (ch)*60 + 0x10)
#define SIMSOCK_PARAM3(ch) (0x10 + (ch)*60 + 0x14)
#define SIMSOCK_PARAM4(ch) (0x10 + (ch)*60 + 0x18)
#define SIMSOCK_PARAM5(ch) (0x10 + (ch)*60 + 0x1c)
#define SIMSOCK_PARAM6(ch) (0x10 + (ch)*60 + 0x20)
#define SIMSOCK_PARAM7(ch) (0x10 + (ch)*60 + 0x24)
#define SIMSOCK_PARAM8(ch) (0x10 + (ch)*60 + 0x28)
#define SIMSOCK_PARAM9(ch) (0x10 + (ch)*60 + 0x2C)
#define SIMSOCK_INTRAW(ch) (0x10 + (ch)*60 + 0x30)
#define SIMSOCK_INTMASK(ch) (0x10 + (ch)*60 + 0x34)
#define SIMSOCK_INTMSTATS(ch) (0x10 + (ch)*60 + 0x38)
#define SIMSOCK_SLPSTAT (0x10 + SIMSOCK_CHAN_MAX*60 + 0x3c)

#define INTMASKENABLE 0xffffffff

struct simsock_chan {
    sim_atomic_t cnt;
    int sockfd;
    int opt;
    osl_sem_id sock_wait;
    void *buf;
    sim_dma_addr_t buf_phy;
    unsigned long size;
    unsigned long len;
};

int simsock_chan_alloc(int sockfd, int opt);
int simsock_chan_free(int ch);

int simsock_chan_ctrl_init(void);
void simsock_chan_ctrl_exit(void);

#endif
