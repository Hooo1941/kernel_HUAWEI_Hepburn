/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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
#ifndef __MSOCKET_H__
#define __MSOCKET_H__

#define THIS_MODU mod_socket

#define THIS_FILE_ID PS_FILE_ID_ACPU_HISOCKET_C

#define HI_POLLIN_SET (POLLRDNORM | POLLRDBAND | POLLIN | POLLHUP | POLLERR)
#define HI_POLLOUT_SET (POLLWRBAND | POLLWRNORM | POLLOUT | POLLERR)
#define HI_POLLEX_SET (POLLPRI)
#define HI_MAX_SLACK (100 * NSEC_PER_MSEC)
#define HISOCK_DUMP_SIZE 0x400

#define HI_SOCKET_MAX 32

struct hi_files_struct {
    struct socket *sock;
    atomic_t cnt; /* reference count */
};

struct hi_fs_table {
    struct hi_files_struct hi_socks[HI_SOCKET_MAX];
    int used;
    struct mutex hi_mutex;
};

struct socket_dump_t {
    char *dump_reg;
    unsigned int dump_size;
};

struct socket_dbg_cnt_t {
    // interface func count
    atomic_t sock;
    atomic_t bind;
    atomic_t listen;
    atomic_t accept_in;
    atomic_t accept_out; // 5
    atomic_t connect_in;
    atomic_t connect_out;
    atomic_t select_in;
    atomic_t select_out;
    atomic_t send; // 10
    atomic_t recv_in;
    atomic_t recv_out;
    atomic_t sendto;
    atomic_t recvfrom_in;
    atomic_t recvfrom_out; // 15
    atomic_t getsockname;
    atomic_t getpeername;
    atomic_t getsockopt;
    atomic_t setsockopt;
    atomic_t ioctl; // 20
    atomic_t shutdown;
    atomic_t inet_addr;
    atomic_t close;
    // core func count
    atomic_t sockregfd;
    atomic_t sockunregfd; // 25
    atomic_t sockgetfd;
    atomic_t sockputfd;
};

struct socket_dbg_table {
    struct socket_dbg_cnt_t socket_dbg_cnt;
    struct socket_dump_t socket_dump;
};

struct hi_poll_table_entry {
    unsigned int key;
    wait_queue_entry_t wait;
    wait_queue_head_t *wait_address;
};

struct hi_poll_wqueues {
    poll_table pt;
    struct task_struct *polling_task;
    int triggered;
    int error;
    int inline_index;
    struct hi_poll_table_entry inline_entries[HI_SOCKET_MAX];
};

struct hi_fd_set_bits {
    unsigned int in;
    unsigned int out;
    unsigned int ex;
    unsigned int res_in;
    unsigned int res_out;
    unsigned int res_ex;
};

#endif   /* __MSOCKET_H__ */
