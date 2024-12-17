/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
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

#ifndef __PCIE_TTY_H__
#define __PCIE_TTY_H__

#include "pcie_cdev_desc.h"

struct pcie_tty_name_type_tbl {
    char *name;
    UNIFY_IF_PROT type;
    unsigned int user_space_check;
};

struct gs_buf {
    unsigned buf_size;
    char *buf_buf;
    char *buf_get;
    char *buf_put;
};

struct gs_port_stats {
    unsigned long open_count;
    unsigned long close_count;
    unsigned long openclose;
    unsigned long fd_null;

    unsigned long tty_write;
    unsigned long tty_write_count_null;
    unsigned long start_tx;
    unsigned long pcdev_write;
    unsigned long pcdev_write_len;
    unsigned long pcdev_write_fail;
    unsigned long pcdev_write_complete;
    unsigned long tty_write_max;

    unsigned long pcdev_read_complete;
    unsigned long rx_push;
    unsigned long last_rw_info_read;
    unsigned long pcdev_get_buf;
    unsigned long pcdev_get_buf_fail;
    unsigned long pcdev_ret_buf;
    unsigned long pcdev_ret_buf_fail;
    unsigned long cannt_push;
    unsigned long n_read;

    unsigned long console_setup;
    unsigned long console_setup_buf_alloc_fail;
    unsigned long console_write_work;
    unsigned long console_write_to_pcie;
    unsigned long pcie_console_write;
    unsigned long pcie_console_write_count_null;
    unsigned long pcie_console_write_fail;
    unsigned long pcie_console_write_len;
    unsigned long console_write;
    unsigned long console_data_max;
    unsigned long console_write_no_room;
};

struct gs_port {
    char *name;
    unsigned int port_num;
    struct tty_port port;
    spinlock_t port_lock;
    wait_queue_head_t buf_free_wait;

    void *fd;
    struct gs_buf port_write_buf;
    unsigned int maxpacket;
    int write_started;
    int read_complete;

    struct gs_port_stats pstats;
};

#endif
