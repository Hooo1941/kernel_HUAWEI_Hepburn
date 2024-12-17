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

#ifndef __PCIE_MODEM_H__
#define __PCIE_MODEM_H__

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/skbuff.h>

#define PCDEV_MDM_COUNT 1
#define PCDEV_MDM_CONSOLE_IDX 0
#define PCDEV_MDM_NAME_MAX 64
#define PCDEV_MDM_DFT_RD_BUF_SIZE 2048
#define MAX_MPS_SIZE 1024

struct gs_port_stats {
    unsigned long write_async_call;
    unsigned long write_bytes;
    unsigned long tx_inv_param;
    unsigned long tx_done;
    unsigned long tx_done_bytes;
    unsigned long tx_done_fail;

    unsigned long get_buf_call;
    unsigned long ret_buf_call;
};

struct gs_pcdev_mdm_port {
    char *name;
    spinlock_t port_lock;
    void *fd;
    struct sk_buff_head skb_list;

    unsigned int port_num;
    unsigned open_count;
    bool openclose;
    pcdev_evt_e event_type;

    pcdev_write_done_cb_t write_done_cb;

    unsigned int read_buf_size;
    struct gs_port_stats pstats;
};

struct pcdev_mdm_port_manager {
    unsigned int is_alloc;
    struct mutex open_close_lock; /* protect open/close */
    struct gs_pcdev_mdm_port *port;
    struct device *cdev;
    unsigned int rel_ind_cb_cnt;
};

/* cdev driver */
struct u_modem_driver {
    struct kref kref; /* Reference management */
    struct cdev cdev;
    dev_t dev_no;
    struct module *owner;
    const char *driver_name;
    const char *name;
    int name_base;            /* offset of printed name */
    unsigned int major;       /* major device number */
    unsigned int minor_start; /* start of minor device number */
    int num;                  /* number of devices allocated */

    struct workqueue_struct *pmdm_work_queue;
};

long pcdev_mdm_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

#endif

