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
#ifndef RELAY_ACM_H
#define RELAY_ACM_H

#include <mdrv_usb.h>
#include <bsp_relay.h>
#include <bsp_usb.h>
#include "relay_def.h"

#define PCUI_BUFSIZE 1024
#define GPS_BUFSIZE 8192
#define G4DIAG_BUFSIZE 8192
#define G3DIAG_BUFSIZE 16384
#define ACM_CTRL_BUFSIZE 1024
#define SKYTONE_BUFSIZE 1024
#define PCDEV_AT_BUFSIZE 1024
#define AGENTNV_BUFSIZE 2048
#define AGENTOM_BUFSIZE 65536
#define AGENTMSG_BUFSIZE 2048
#define ASHELL_BUFSIZE 2048
#define MODEM_BUFSIZE 1536
#define PMOM_BUFSIZE 2048
#define RESERVE_BUFSIZE 2048

struct relay_acm_func {
    acm_read_done_cb read_done_cb;
    acm_write_done_cb write_done_cb;
    acm_event_cb evt_cb;
    usb_modem_msc_read_cb msc_read_cb;
    acm_modem_switch_mode_cb switch_mode_cb;
};

struct vdev_acm_cbs {
    relay_enable_cb enable_cb;
    relay_disable_cb disable_cb;
    relay_acm_read_cb read_cb;
    relay_acm_write_donecb write_cb;
    relay_acm_event_cb evt_cb;
};

struct relay_acm_stat {
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
    unsigned int disable;
    unsigned int disable_no_cb;

    unsigned int ret_buf; /* success free buf */
    unsigned int ret_buf_fail;

    unsigned int read_done;
    unsigned int read_done_len;
    unsigned int read_done_fail;
    unsigned int read_buff_get_fail;
    unsigned int read_cb_sched;
    unsigned int no_read_cb;
    unsigned int ack_msc;
    unsigned int switch_mode;
    unsigned int suspend;

    unsigned int write_queue;
    unsigned int write_suspend;
    unsigned int write_len;
    unsigned int write_fail;
    unsigned int write_full;
    unsigned int write_succ;
    unsigned int write_done;
    unsigned int write_done_len;

    unsigned int droped;
};

struct relay_acm_device {
    unsigned int buff_size;
    unsigned int buff_num;
    enum relay_evt port_status;
    struct relay_adp_device adp_dev;
    struct relay_acm_stat stat;
    struct relay_acm_func funcs;
    struct vdev_acm_cbs cbs;

    bool flow_ctrl;
    struct list_head tx_queue;
    unsigned int capacity_num;
    unsigned int send_num;
    unsigned int pending_num;
    spinlock_t tx_lock;
    struct delayed_work write_work;
    struct workqueue_struct *work_queue;
};

struct relay_acm_packet {
    struct list_head list;
    struct acm_wr_async_info info;
};

int relay_acm_dev_open(unsigned int minor_type);
int relay_acm_dev_close(unsigned int minor_type);
int relay_acm_dev_write(unsigned int minor_type, char *addr, unsigned int size);
int relay_acm_dev_ioctl(unsigned int minor_type, unsigned int cmd, void *para, unsigned int para_size);
void relay_acm_enable(unsigned int minor_type);
void relay_acm_disable(unsigned int minor_type);
int relay_acm_init(void);
int relay_acm_reg_enablecb(unsigned int minor_type, relay_enable_cb func, void *private);
int relay_acm_reg_disablecb(unsigned int minor_type, relay_disable_cb func);

#endif /* RELAY_ACM_H */
