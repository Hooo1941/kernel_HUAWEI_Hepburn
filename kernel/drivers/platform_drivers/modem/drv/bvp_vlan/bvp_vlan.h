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
#ifndef __BVP_VLAN_H__
#define __BVP_VLAN_H__

#include "bsp_vlan.h"
#include "mdrv_udi.h"

#include <bsp_print.h>
#include <bsp_version.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/fs.h>
#include <linux/gfp.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/workqueue.h>
#include <osl_types.h>
#include <securec.h>
#include <uapi/linux/sched/types.h>

#define THIS_MODU mod_vlan

#define vlan_err(fmt, ...) bsp_err(fmt, ##__VA_ARGS__)

#define VLAN_GLB_CFG_REG 0x0
#define VLAN_GLB_STATUS_REG 0x4
#define VLAN_INTR_STATUS_REG 0x10
#define VLAN_INTR_MASK_REG 0x14
#define VLAN_INTR_RAW_REG 0x18
#define CHAN_CTRL_REG(index) 0x100 + 0x100 * (index)
#define CHAN_STATE_REG(index) 0x108 + 0x100 * (index)
#define CHAN_INTR_STATUS_REG(index) 0x110 + 0x100 * (index)
#define CHAN_INTR_MASK_REG(index) 0x114 + 0x100 * (index)
#define CHAN_INTR_RAW_REG(index) 0x118 + 0x100 * (index)
#define MSG_RECV_BUF_ADDR_HIGH_REG(index) 0x120 + 0x100 * (index)
#define MSG_RECV_BUF_ADDR_LOW_REG(index) 0x124 + 0x100 * (index)
#define MSG_RECV_BUF_SIZE_REG(index) 0x128 + 0x100 * (index)
#define MSG_RECV_DONE_ADDR_HIGH_REG(index) 0x130 + 0x100 * (index)
#define MSG_RECV_DONE_ADDR_LOW_REG(index) 0x134 + 0x100 * (index)
#define MSG_RECV_DONE_SIZE_REG(index) 0x138 + 0x100 * (index)
#define MSG_RECV_BUF_CNT_REG(index) 0x140 + 0x100 * (index)
#define MSG_RECV_DONE_CNT_REG(index) 0x144 + 0x100 * (index)
#define MSG_SEND_REQ_ADDR_HIGH_REG(index) 0x150 + 0x100 * (index)
#define MSG_SEND_REQ_ADDR_LOW_REG(index) 0x154 + 0x100 * (index)
#define MSG_SEND_REQ_SIZE_REG(index) 0x158 + 0x100 * (index)
#define MSG_SEND_DONE_ADDR_HIGH_REG(index) 0x160 + 0x100 * (index)
#define MSG_SEND_DONE_ADDR_LOW_REG(index) 0x164 + 0x100 * (index)
#define MSG_SEND_DONE_SIZE_REG(index) 0x168 + 0x100 * (index)
#define MSG_SEND_REQ_CNT_REG(index) 0x170 + 0x100 * (index)
#define MSG_SEND_DONE_CNT_REG(index) 0x174 + 0x100 * (index)

#define CHAN_MAX_NUM 16
#define CHAN_USED_NUM 5
#define READ_BUFF_SIZE 1024
#define MAX_READ_BUFF_NUM 16
#define MAX_WAIT_TIME 1000

#define GET_ADDR_HIGH(addr) ((uint32_t)(((uint64_t)(addr) >> 32) & 0xffffffff))
#define GET_ADDR_LOW(addr) ((uint32_t)((uint64_t)(addr)&0xffffffff))
#define GET_ADDR_FULL(high, low) (((uint64_t)(high)) << 32 | (low))

enum VLAN_INTER_INFO {
    VLAN_GPS_CHAN = 0x1,
    VLAN_LTE_CHAN = 0x2,
    VLAN_AT_CHAN = 0x4,
    VLAN_OM_CHAN = 0x8,
    VLAN_TEST_CHAN = 0x10,
    VLAN_DEV_CONNECT_EVENT = 0x80000000
};

enum CHAN_INTER_INFO {
    VLAN_CLIENT_CONNECT_EVENT = 0x1,
    VLAN_RECV = 0x2,
    VLAN_SEND_COMPLETED = 0x4
};

enum VLAN_CHAN_NAME {
    VLAN_GPS = 0,
    VLAN_LTE = 1,
    VLAN_AT = 2,
    VLAN_OM = 3,
    VLAN_TEST = 4
};

enum BUFF_STATE {
    STORE_IN_MODEL = 0,
    WAITING_TO_READ,
    READING,
    WAITING_FOR_DEL
};

struct read_buf {
    struct acm_wr_async_info read_info;
    int buff_state;
    void *private;
    struct list_head list;
};

struct chan_intr_info {
    int chan_intr_status;
    int chan_state;
    int recv_num;
    int sendcb_num;
};

struct intr_task {
    int vlan_intr_status;
    int vlan_glb_status;
    struct chan_intr_info chan_intr[CHAN_MAX_NUM];
    struct list_head list;
};

struct chan_info {
    int chan_id;
    int client_state;

    struct vlan_dev *dev;

    struct acm_read_buff_info recv_buff_info;

    struct list_head recv_buff_list;
    spinlock_t recv_buff_list_lock;

    struct list_head waiting_read_que;
    spinlock_t waiting_read_que_lock;

    struct list_head writing_que;
    spinlock_t writing_que_lock;

    void (*readcb)(void);
    void (*writecb)(char *vaddr, char *paddr, int len);
    void (*eventcb)(ACM_EVT_E state);
};

typedef struct vlan_dev {
    struct device *dev;

    int irq;
    struct list_head intr_task_list;
    spinlock_t intr_task_list_lock;

    struct kthread_worker irq_worker;
    struct kthread_delayed_work irq_work;
    struct task_struct *irq_worker_task;
    struct wakeup_source *irq_wakelock;

    void *reg_vaddr_base;

    struct wakeup_source *wakelock;
} vlan_dev;

#endif
