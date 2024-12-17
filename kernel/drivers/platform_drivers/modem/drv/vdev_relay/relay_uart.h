/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
#ifndef RELAY_UART_H
#define RELAY_UART_H

#include "product_config.h"
#include <bsp_relay.h>
#if defined(CONFIG_DIALUP_UART) || defined(CONFIG_DIALUP_PL011_UART)
#include <bsp_dialup_uart.h>
#endif
#include "relay_def.h"

struct relay_uart_func {
    dialup_uart_read_cb_t read_done_cb;
    dialup_uart_free_cb_t free_cb;
    dialup_uart_msc_read_cb_t msc_read_cb;
    dialup_uart_switch_mode_cb_t switch_mode_cb;
    dialup_uart_water_cb_t water_line_cb;
    dialup_uart_port_ready_cb_t ready_cb;
};

struct vdev_uart_cbs {
    relay_enable_cb enable_cb;
    relay_disable_cb disable_cb;
    mci_uart_read_cb read_cb;
    mci_uart_free_cb free_cb;
};

struct relay_uart_stat {
    unsigned int open;
    unsigned int open_again;
    unsigned int open_fail;
    unsigned int ioctl_fail;
    unsigned int open_succ;

    unsigned int close;
    unsigned int close_again;
    unsigned int close_fail;
    unsigned int close_succ;

    unsigned int enable;
    unsigned int enable_no_cb;

    unsigned int ret_buf; /* success free buf */
    unsigned int ret_buf_fail;

    unsigned int read_done;
    unsigned int read_done_len;
    unsigned int read_done_fail;
    unsigned int read_buff_get_fail;
    unsigned int read_cb_sched;
    unsigned int no_read_cb;

    unsigned int write;
    unsigned int write_len;
    unsigned int write_fail;
    unsigned int write_done;
    unsigned int write_done_len;

    unsigned int droped;
    unsigned int switch_mode;
    unsigned int notify_ready;
    unsigned int msc_read;
};

struct relay_uart_device {
    struct relay_adp_device adp_dev;
    struct relay_uart_stat stat;
    struct vdev_uart_cbs cbs;
    struct relay_uart_func funcs;
};

int relay_uart_dev_open(unsigned int minor_type);
int relay_uart_dev_close(unsigned int minor_type);
int relay_uart_dev_write(unsigned int minor_type, char *addr, unsigned int size);
int relay_uart_dev_ioctl(unsigned int minor_type, unsigned int cmd, void *para, unsigned int para_size);
int relay_uart_reg_enablecb(unsigned int minor_type, relay_enable_cb func, void *private);
int relay_uart_reg_disablecb(unsigned int minor_type, relay_disable_cb func);
void relay_uart_init(void);

#endif /* RELAY_UART_H */
