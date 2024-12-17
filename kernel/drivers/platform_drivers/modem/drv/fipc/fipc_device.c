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

#include "fipc_platform.h"
#include "fipc_dts.h"

#include "fipc_device.h"
#include "fipc_driver.h"
#include "fipc_core.h"
#define FIPC_DTS_CUST_STR_LEN 64
/* 用于加速访问device的数组，每个成员是一个指针，指向对应idx的device结构 */
struct fipc_device *g_fipc_devices_fastptr[FIPC_DEVICE_NUM_MAX] = {0};
struct fipc_channel *g_fipc_channel_fastptr[FIPC_USRCHN_NUM_MAX] = {0};
static u32 g_channel_fastptr_usedidx = 0;

struct fipc_device *fipc_device_get_fast(u32 devid)
{
    if (devid >= FIPC_DEVICE_NUM_MAX) {
        return NULL;
    }
    return g_fipc_devices_fastptr[devid];
}

struct fipc_channel *fipc_channel_get_fast(u32 usr_chnid)
{
    u32 idx;
    for (idx = 0; idx < g_channel_fastptr_usedidx; idx++) {
        if (usr_chnid == g_fipc_channel_fastptr[idx]->user_id) {
            return g_fipc_channel_fastptr[idx];
        }
    }
    return NULL;
}

struct fipc_channel *fipc_channel_get_byidx(u32 idx)
{
    if (idx < FIPC_USRCHN_NUM_MAX) {
        return g_fipc_channel_fastptr[idx];
    }
    return NULL;
}

int fipc_dev_chn_get(u32 phy_chn_id, struct fipc_device **ppdev, struct fipc_channel **ppchannel)
{
    struct fipc_device *pdev = NULL;
    struct fipc_channel *pchannel = NULL;

    pdev = fipc_device_get_fast(GET_DEV_ID(phy_chn_id));
    if (pdev == NULL) {
        return -1;
    }
    if (GET_PIPE_ID(phy_chn_id) >= FIPC_CHN_ID_MAX) {
        return -1;
    }
    pchannel = pdev->channels[GET_PIPE_ID(phy_chn_id)];
    if (pchannel == NULL) {
        return -1;
    }
    *ppdev = pdev;
    *ppchannel = pchannel;
    return 0;
}

int fipc_dev_chn_get_byid(u32 user_id, struct fipc_device **ppdev, struct fipc_channel **ppchannel)
{
    struct fipc_channel *pchannel_fake = NULL;
    if (ppdev == NULL || ppchannel == NULL) {
        return -1;
    }
    pchannel_fake = fipc_channel_get_fast(user_id);
    if (pchannel_fake == NULL) {
        return -1;
    }
    if (fipc_dev_chn_get(FIPC_GET_PHYCHN_ID(pchannel_fake->ldrvchn_id), ppdev, ppchannel)) {
        return -1;
    }
    return 0;
}

int fipc_dev_chn_get_byidx(u32 chn_idx, struct fipc_device **ppdev, struct fipc_channel **ppchannel)
{
    struct fipc_channel *pchannel_fake = NULL;
    if (ppdev == NULL || ppchannel == NULL) {
        return -1;
    }
    pchannel_fake = fipc_channel_get_byidx(chn_idx);
    if (pchannel_fake == NULL) {
        return -1;
    }
    if (fipc_dev_chn_get(FIPC_GET_PHYCHN_ID(pchannel_fake->ldrvchn_id), ppdev, ppchannel)) {
        return -1;
    }
    return 0;
}

static struct fipc_channel *fipc_chn_make(struct fipc_chn_dts *pchn_dts)
{
    struct fipc_channel *channel = NULL;
    channel = (struct fipc_channel *)fipc_malloc(sizeof(struct fipc_channel));
    if (channel == NULL) {
        fipc_print_error("fipc_chn_make failed!\n");
        return NULL;
    }
    (void)memset_s(channel, sizeof(struct fipc_channel), 0, sizeof(struct fipc_channel));
    spin_lock_init(&channel->lock);
    channel->user_id = pchn_dts->id;
    channel->type = pchn_dts->type;
    channel->ldrvchn_id = pchn_dts->ldrvchn_id;
    channel->rdrvchn_id = pchn_dts->rdrvchn_id;
    channel->dym_flags = 0;
    channel->const_flags = pchn_dts->const_flags;
    channel->remote_awake = 0;
    channel->state = (u32)FIPC_CHN_INIT;
    return channel;
}

static int fipc_dev_of_map(struct fipc_device *pdev, struct fipc_device_dts *pdev_dts)
{
    pdev->base_pa = pdev_dts->reg_addr;
    if (pdev->ctrl_level != FIPC_DEV_CONTROL_LEVEL_IGNORE) {
        pdev->base_va = fipc_ioremap(pdev_dts->reg_addr, pdev_dts->reg_size);
        if (pdev->base_va == NULL) {
            fipc_print_error("remap failed when fipc device init!\n");
            return -1;
        }
    }

    if (pdev->ctrl_level == FIPC_DEV_CONTROL_LEVEL_HOST || pdev->ctrl_level == FIPC_DEV_CONTROL_LEVEL_GUEST ||
        pdev->ctrl_level == FIPC_DEV_CONTROL_LEVEL_PART) {
        pdev->push_ptr_va = (u32 *)fipc_pushmem_alloc(sizeof(u32) * FIPC_CHN_ID_MAX, &pdev->push_ptr_pa, sizeof(u32));
        if (pdev->push_ptr_va == NULL) {
            fipc_print_error("push mem alloc failed when fipc device init!\n");
            return -1;
        }
        (void)memset_s(pdev->push_ptr_va, sizeof(u32) * FIPC_CHN_ID_MAX, 0, sizeof(u32) * FIPC_CHN_ID_MAX);
    }

    return 0;
}
static int fipc_chn_dts_parse(device_node_s *p_chn_node, struct fipc_chn_dts *pchn_dts)
{
    u32 uret = 0;
    u32 chn_cfg[0x2] = {0};
    uret |= (u32)bsp_dt_property_read_u32_array(p_chn_node, "chn_id", &pchn_dts->id, 1);
    uret |= (u32)bsp_dt_property_read_u32_array(p_chn_node, "chn_type", &pchn_dts->type, 1);
    uret |= (u32)bsp_dt_property_read_u32_array(p_chn_node, "chn_cfg", chn_cfg, sizeof(chn_cfg) / sizeof(chn_cfg[0]));
    if (uret) {
        fipc_print_error("fipc_chn_probe failed!\n");
        return -1;
    }
    pchn_dts->ldrvchn_id = chn_cfg[0x0];
    pchn_dts->rdrvchn_id = chn_cfg[0x1];
    if (bsp_dt_property_read_u32_array(p_chn_node, "const_flags", &pchn_dts->const_flags, 1)) {
        pchn_dts->const_flags = 0;
    }
    return 0;
}

int fipc_chns_probe(struct fipc_device *pdev)
{
    u32 ldrvchn_id;
    struct fipc_channel *channel = NULL;

    struct fipc_chn_dts chn_dts;
    device_node_s *pnode = NULL;
    struct fipc_chn_dts *pchn_dts = &chn_dts;

    device_node_s temp_node;
    device_node_s *p_chn_node = &temp_node; /* for reduced dts version */

    (void)memset_s(p_chn_node, sizeof(device_node_s), 0, sizeof(device_node_s));
    if (pdev == NULL || pdev->of_node == NULL) {
        return -1;
    }

    pnode = (device_node_s *)pdev->of_node;
    bsp_dt_for_each_available_child_of_node(pnode, p_chn_node)
    {
        if (bsp_dt_property_read_u32_array(p_chn_node, "chn_id", &pchn_dts->id, 1)) {
            /* not a channel,may be other type; */
            continue;
        }
        if (fipc_chn_dts_parse(p_chn_node, pchn_dts)) {
            return -1;
        }
        ldrvchn_id = pchn_dts->ldrvchn_id;
        if (FIPC_GET_CORE_ID(ldrvchn_id) >= FIPC_CPU_ID_MAX || pdev->maps[FIPC_GET_CORE_ID(ldrvchn_id)] == NULL) {
            fipc_print_error("chn_id 0x%x detected not design to this!\n", pchn_dts->id);
            continue;
        }
        if (FIPC_GET_DEV_ID(ldrvchn_id) != pdev->id || FIPC_GET_PIPE_ID(ldrvchn_id) >= pdev->pipepair_cnt ||
            pdev->channels[FIPC_GET_PIPE_ID(ldrvchn_id)] != NULL) {
            fipc_print_error("chn_id 0x%x detected err!\n", pchn_dts->id);
            return -1;
        }
        if (g_channel_fastptr_usedidx >= FIPC_USRCHN_NUM_MAX) {
            fipc_print_error("fipc_chn_probe too much!\n");
            return -1;
        }

        channel = fipc_chn_make(pchn_dts);
        if (channel == NULL) {
            return -1;
        }
        pdev->channels[FIPC_GET_PIPE_ID(channel->ldrvchn_id)] = channel;
        fipc_print_trace("chan init ok,chan_id=0x%x,drvcnn_id=0x%x\n", (unsigned)channel->user_id,
            (unsigned)channel->ldrvchn_id);
        g_fipc_channel_fastptr[g_channel_fastptr_usedidx] = channel;
        g_channel_fastptr_usedidx++;
    }
    return 0;
}

static int fipc_dev_irq_parse(device_node_s *p_irq_node, struct fipc_device_dts *pdev_dts)
{
    u32 cpu_id;
    struct irq_bundle *maps = NULL;
    if (bsp_dt_property_read_u32_array(p_irq_node, "cpu_id", &cpu_id, 1)) {
        /* not a irq bundle ,may be other type; */
        return 0;
    }

    if (cpu_id >= FIPC_CPU_ID_MAX) {
        fipc_print_error("dts irq_bundle[%d] outrange error!\n", cpu_id);
        return -1;
    }
    if (pdev_dts->maps[cpu_id] != NULL) {
        fipc_print_error("dts irq_bundle[%d] dup error!\n", cpu_id);
        return -1;
    }
    maps = (struct irq_bundle *)fipc_malloc(sizeof(struct irq_bundle));
    if (maps == NULL) {
        fipc_print_error("fipc_devdts_parse malloc error!\n");
        return -1;
    }
    (void)memset_s(maps, sizeof(struct irq_bundle), 0, sizeof(struct irq_bundle));
    maps->cpu_id = cpu_id;

    if (pdev_dts->ctrl_level == FIPC_DEV_CONTROL_LEVEL_HOST || pdev_dts->ctrl_level == FIPC_DEV_CONTROL_LEVEL_GUEST ||
        pdev_dts->ctrl_level == FIPC_DEV_CONTROL_LEVEL_PART) {
        maps->irq[0] = bsp_dt_irq_parse_and_map(p_irq_node, 0);
        maps->irq[1] = bsp_dt_irq_parse_and_map(p_irq_node, 1);
    } else if (pdev_dts->ctrl_level == FIPC_DEV_CONTROL_LEVEL_IRQCLR) {
        maps->irq[0] = bsp_dt_irq_parse_and_map(p_irq_node, 0);
        maps->irq[1] = 0xFFFFFFFF;
    } else {
        maps->irq[0] = 0xFFFFFFFF;
        maps->irq[1] = 0xFFFFFFFF;
    }
    pdev_dts->maps[cpu_id] = maps;
    return 0;
}
static int fipc_dev_dts_parse(device_node_s *p_dev_node, struct fipc_device_dts *pdev_dts)
{
    u32 uret = 0;
    u32 array[0x2] = {0};

    device_node_s temp_node;
    device_node_s *p_irq_node = &temp_node; /* for reduced dts version */

    uret |= (u32)bsp_dt_property_read_u32_array(p_dev_node, "dev_id", &pdev_dts->dev_id, 1);
    uret |= (u32)bsp_dt_property_read_u32_array(p_dev_node, "reg", array, sizeof(array) / sizeof(array[0]));
    uret |= (u32)bsp_dt_property_read_u32_array(p_dev_node, "pipepair_cnt", &pdev_dts->pipepair_cnt, 1);
    uret |= (u32)bsp_dt_property_read_u32_array(p_dev_node, "ctrl_level", &pdev_dts->ctrl_level, 1);
    uret |= (u32)bsp_dt_property_read_u32_array(p_dev_node, "irq_type", &pdev_dts->irq_type, 1);
    if (uret) {
        fipc_print_error("dts node error!\n");
        return -1;
    }
    pdev_dts->reg_addr = array[0x0];
    pdev_dts->reg_size = array[0x1];

    /* IRQ MAPS PARSE */
    (void)memset_s(p_irq_node, sizeof(device_node_s), 0, sizeof(device_node_s));
    bsp_dt_for_each_child_of_node(p_dev_node, p_irq_node)
    {
        if (fipc_dev_irq_parse(p_irq_node, pdev_dts)) {
            return -1;
        }
    }

    return 0;
}

/* pdev->version and pdev->of_node be set by fipc_devices_init */
int fipc_dev_probe(struct fipc_device *pdev)
{
    u32 devid;
    struct fipc_device_dts dev_dts;
    struct fipc_device_dts *pdev_dts = &dev_dts;
    device_node_s *p_dev_node = NULL;
    if (pdev == NULL || pdev->of_node == NULL) {
        return -1;
    }
    p_dev_node = (device_node_s *)pdev->of_node;
    (void)memset_s(&dev_dts, sizeof(struct fipc_device_dts), 0, sizeof(struct fipc_device_dts));
    if (fipc_dev_dts_parse(p_dev_node, pdev_dts)) {
        return -1;
    }
    devid = pdev_dts->dev_id;
    if (devid >= FIPC_DEVICE_NUM_MAX || g_fipc_devices_fastptr[devid]) {
        fipc_print_error("dts node devid error or dup!\n");
        return -1;
    }
    pdev->id = devid;
    pdev->ctrl_level = pdev_dts->ctrl_level;
    pdev->pipepair_cnt = pdev_dts->pipepair_cnt;

    pdev->irq_type = pdev_dts->irq_type;
    if (memcpy_s(pdev->maps, sizeof(pdev->maps), pdev_dts->maps, sizeof(pdev_dts->maps))) {
        fipc_print_error("dts node maps size not match!\n");
        return -1;
    }

    spin_lock_init(&pdev->lock);

    if (fipc_dev_of_map(pdev, pdev_dts)) {
        return -1;
    }

    g_fipc_devices_fastptr[devid] = pdev;
    return 0;
}

irqreturn_t fipc_wake_irq_handler(int irq, void *intput_arg)
{
    UNUSED(irq);
    UNUSED(intput_arg);
    return IRQ_HANDLED;
}

int fipc_irq_wake_probe(device_node_s *parent_node)
{
    s32 ret;
    u32 irq_wake_num, wake_irq, irq_source_idx;
    device_node_s *wake_node = NULL;
    if (parent_node == NULL) {
        return -1;
    }

    ret = bsp_dt_property_read_u32(parent_node, "irq_wake_num", &irq_wake_num);
    if (ret) {
        fipc_print_error("fipc no need to init wake source.\n");
        return 0;
    }
    wake_node = bsp_dt_find_node_by_path("/fipc_wake_irq");
    if (wake_node == NULL) {
        return -1;
    }
    for (irq_source_idx = 0; irq_source_idx < irq_wake_num; irq_source_idx++) {
        ret = bsp_dt_property_read_u32_index(wake_node, "interrupts", irq_source_idx, &wake_irq);
        if (ret) {
            fipc_print_error("get %d interrupts fail\n", wake_irq);
            return -1;
        }
        wake_irq = bsp_dt_irq_parse_and_map(wake_node, irq_source_idx);
        if (request_irq(wake_irq, fipc_wake_irq_handler, 0, "fipc_wake_irq", NULL)) {
            fipc_print_error("fipc_wake_irq register fail\n");
            return -1;
        }
        enable_irq_wake(wake_irq);
    }
    return 0;
}

int fipc_probe(struct fipc_device *pdev)
{
    if (fipc_dev_probe(pdev)) {
        return -1;
    };

    if (fipc_chns_probe(pdev)) {
        return -1;
    };

    if (pdev->ctrl_level == FIPC_DEV_CONTROL_LEVEL_PART) {
        pdev->state = FIPC_DEV_STATE_WAIT;
    } else {
        pdev->state = FIPC_DEV_STATE_WORKING;
    }  
    return 0;
}

int fipc_devices_init(void)
{
    int ret;
    u32 version;
    struct fipc_device *device = NULL;

    device_node_s temp_node;
    device_node_s *parent_node = NULL;
    device_node_s *child_node = &temp_node; /* for reduced dts version */
    struct fipc_config_diff* config_diff = get_fipc_config_diff();
    const char *node_path = "/fipc_device";
    parent_node = bsp_dt_find_node_by_path(node_path);
    if (parent_node == NULL) {
        fipc_print_error("fipc dts not found!\n");
        return 0;
    }
    ret = fipc_irq_wake_probe(parent_node);
    if (ret) {
        return -1;
    }

    (void)memset_s(child_node, sizeof(device_node_s), 0, sizeof(device_node_s));
    bsp_dt_for_each_child_of_node(parent_node, child_node)
    {
        ret = bsp_dt_property_read_u32_array(child_node, "version", &version, 1);
        if (ret) {
            fipc_print_error("dts node error!\n");
            return -1;
        }
        if (version >= FIPC_VERSION_MAXNUM) {
            fipc_print_error("version exceed!\n");
            return -1;
        }

        device = (struct fipc_device *)fipc_malloc(sizeof(struct fipc_device));
        if (device == NULL) {
            fipc_print_error("malloc failed when fipc device init!\n");
            return -1;
        }
        (void)memset_s(device, sizeof(struct fipc_device), 0, sizeof(struct fipc_device));
        device->version = version;
        device->of_node = child_node;
        device->config_diff = &config_diff[device->version];
        ret = fipc_probe(device);
        if (ret) {
            fipc_print_error("fipc_probe failed\n");
            return ret;
        }
    }
    fipc_print_always("fipc_devices_init completed ok!\n");
    return 0;
}

EXPORT_SYMBOL(g_fipc_devices_fastptr);

