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


#ifndef _ESL_SOCKET_API_H
#define _ESL_SOCKET_API_H

#include <mdrv_socket.h>
#include <bsp_esl_socket.h>

#include "esl_socket_os_interface.h"
#include "esl_socket_channel_ctr.h"
#include "mdrv_memory_layout.h"

#define DISABLE_SOCKET_DEBUG 0
#define ENABLE_SOCKET_DEBUG 1

#define SIMSOCK_OK 0
#define SIMSOCK_FAIL (-1)

#define TOSLEEP 1
#define TOWAKEUP 0

typedef int socklen_t;

struct simsock_fd {
    int sockfd;
    sim_atomic_t cnt; /* reference count */
};

struct simsock_ctx {
    struct simsock_fd socks[SIMSOCK_MAX];
    struct simsock_chan chan[SIMSOCK_CHAN_MAX];
    unsigned long chan_map;
    unsigned long chan_num;
    unsigned int chan_used;
    int used;
    sim_mutex simch_mutex;
    sim_mutex simfd_mutex;
    struct device *dev;
    unsigned int copy_mode;
    unsigned int irq;
    void *simsock_regs;
    void *simsock_iqrstatus;
};

void set_simsock_debug(int level);
struct simsock_ctx* get_simsock_config(void);
int simsock_set_copymode(int copy_mode);

#endif
