/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
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

#include <bsp_msg.h>
#include <msg_id.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <securec.h>
#include <bsp_print.h>
#include "vdev.h"

#define THIS_MODU mod_vcom
#define MAX_LIST_LEN 128
static msg_chn_t g_vcom_msghdl;

struct vdev_msg {
    MDRV_MSG_HEAD;
    char s[0];
};

static int vcom_idle_list(struct vcom_map_s *map)
{
    int ret;
    unsigned long flags;

    VCOM_PRINT("[map:%s]mci idle call\n", map->hd->name);
    spin_lock_irqsave(&map->link_lock, flags);
    ret = list_empty(&map->high_list);
    spin_unlock_irqrestore(&map->link_lock, flags);
    return ret;
}

unsigned char *vcom_dequeue(struct vcom_map_s *map)
{
    struct vcom_msg_seg *sp = NULL;
    unsigned long flags;
    unsigned char *buf = NULL;

    VCOM_PRINT("map %s, dequeue\n", map->hd->name);

    map->stax.dequeue++;
    spin_lock_irqsave(&map->link_lock, flags);
    if (list_empty(&map->high_list)) {
        spin_unlock_irqrestore(&map->link_lock, flags);
        VCOM_PRINT_ERROR("map %s, dequeue fail\n", map->hd->name);
        return NULL;
    }

    sp = list_first_entry(&map->high_list, struct vcom_msg_seg, list);
    list_del(&sp->list);
    map->high_list_len--;
    spin_unlock_irqrestore(&map->link_lock, flags);

    buf = sp->buf;
    kfree(sp);
    sp = NULL;
    return buf;
}

int vcom_enqueue(struct vcom_map_s *map, unsigned char *buf)
{
    struct vcom_msg_seg *sp = NULL;
    unsigned long flags;
    unsigned int len;
    VCOM_PRINT("map %s, enqueue\n", map->hd->name);

    spin_lock_irqsave(&map->link_lock, flags);
    len = map->high_list_len;
    spin_unlock_irqrestore(&map->link_lock, flags);

    if (len > MAX_LIST_LEN) {
        map->stax.read_drop++;
        goto en_fail;
    }

    sp = kmalloc(sizeof(struct vcom_msg_seg), GFP_KERNEL);
    if (sp == NULL) {
        VCOM_PRINT_ERROR("map %s, seg mem alloc failed\n", map->hd->name);
        goto en_fail;
    }

    sp->buf = buf;

    spin_lock_irqsave(&map->link_lock, flags);
    list_add_tail(&sp->list, &map->high_list);
    map->stax.enqueue++;
    map->high_list_len++;
    spin_unlock_irqrestore(&map->link_lock, flags);
    return 0;
en_fail:
    map->stax.fail_free++;
    VCOM_PRINT("map %s, enqueue fail\n", map->hd->name);
    kfree(buf);
    return 0;
}

int vcom_queue_head(struct vcom_map_s *map, unsigned char *buf)
{
    struct vcom_msg_seg *sp = NULL;
    unsigned long flags;

    VCOM_PRINT("map %s, mci requeue\n", map->hd->name);

    sp = kmalloc(sizeof(struct vcom_msg_seg), GFP_KERNEL);
    if (sp == NULL) {
        VCOM_PRINT_ERROR("map %s, seg mem alloc failed\n", map->hd->name);
        return -ENOMEM;
    }

    sp->buf = buf;
    spin_lock_irqsave(&map->link_lock, flags);
    list_add(&sp->list, &map->high_list);
    map->stax.requeue++;
    map->high_list_len++;
    spin_unlock_irqrestore(&map->link_lock, flags);
    return 0;
}

static int vcom_write_msg(struct vcom_map_s *map, unsigned char *userbuf, unsigned int size)
{
    int ret;
    struct msg_addr dst;
    struct vcom_debug_stax *debug = vcom_get_debug_stax();

    VCOM_PRINT("map %s, mci write\n", map->hd->name);
    debug->write++;
    dst.core = MSG_CORE_TSP;
    dst.chnid = MSG_CHN_VCOM;
    ret = bsp_msg_lite_sendto(g_vcom_msghdl, &dst, userbuf, size);
    if (ret) {
        VCOM_PRINT_ERROR("map %s, msg send fail:%d\n", map->hd->name, ret);
    }

    debug->write_suc++;

    return 0;
}

static int vdev_msg_read_cb(const struct msg_addr *src_info, void *buf, u32 len)
{
    unsigned char *vcom_buf = NULL;
    struct vcom_debug_stax *debug = vcom_get_debug_stax();

    if ((src_info == NULL) || (src_info->core != MSG_CORE_TSP) ||
        (buf == NULL) || (len > VCOM_MAX_DATA_SIZE + sizeof(struct vcom_buf)) || (len == 0)) {
        VCOM_PRINT_ERROR("param err\n");
        return -1;
    }

    debug->msg_rcv++;

    vcom_buf = (unsigned char *)kmalloc(len, GFP_KERNEL);
    if (vcom_buf == NULL) {
        VCOM_PRINT_ERROR("mem alloc failed\n");
        return -1;
    }
    debug->down_kmalloc++;

    if (EOK != memcpy_s(vcom_buf, len, buf, len)) {
        kfree(vcom_buf);
        VCOM_PRINT_ERROR("memcpy_s failed\n");
        debug->fail_free++;
        return -1;
    }

    if (vcom_msg_parse(vcom_buf, len)) {
        VCOM_PRINT_ERROR("vcom_msg_parse error\n");
        kfree(vcom_buf);
        debug->fail_free++;
    }

    return 0;
}

static int vdev_msg_wakeup_notify_cb(void *arg)
{
    vcom_wakeup_notify();
    return 0;
}

int vdev_msg_init(void)
{
    int ret;
    struct msgchn_attr lite_attr = {0};

    VCOM_PRINT_ERROR("[init] msg start\n");

    bsp_msgchn_attr_init(&lite_attr);
    lite_attr.chnid = MSG_CHN_VCOM;
    lite_attr.coremask = MSG_CORE_MASK(MSG_CORE_TSP);
    lite_attr.lite_notify = vdev_msg_read_cb;
    ret = bsp_msg_lite_open(&g_vcom_msghdl, &lite_attr);
    if (ret) {
        VCOM_PRINT_ERROR("open msg fail:%d\n", ret);
        return ret;
    }

    ret = bsp_msg_wakeup_register(g_vcom_msghdl, vdev_msg_wakeup_notify_cb, NULL);
    if (ret) {
        VCOM_PRINT_ERROR("wakeup cb registe fail:%d\n", ret);
        return ret;
    }

    VCOM_PRINT_ERROR("[init] msg ok\n");
    return ret;
}

int vcom_link_init(struct vcom_map_s *map)
{
    return 0;
}

struct vcom_link_ops g_vcom_link_list_ops = {
    .init = vcom_link_init,
    .idle = vcom_idle_list,
    .write = vcom_write_msg,
    .enqueue = vcom_enqueue,
    .dequeue = vcom_dequeue,
    .queue_head = vcom_queue_head,
};

