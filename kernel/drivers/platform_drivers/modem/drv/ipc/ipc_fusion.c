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

#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/spinlock.h>

#include <osl_malloc.h>
#include <bsp_ipc_fusion.h>
#include <bsp_slice.h>
#include <bsp_dump.h>
#include <bsp_reset.h>
#include <bsp_dt.h>
#include <securec.h>
#include "ipc_fusion.h"

#undef THIS_MODU
#define THIS_MODU mod_ipc

struct ipc_device_t *g_ipc_device[IPC_NUM];
struct ipc_channel_t *g_ipc_channel[IPC_NUM * CHN_MAX_NUM] = {0};
static u32 g_channel_fastptr = 0;
u32 g_ipc_init_flag = 0;

static spinlock_t g_ipc_lock;
unsigned int g_dbg_print_sw = 0;
u32 g_modem_reset_flag = 0;

static inline u32 ipc_handle_mku32(struct ipc_handle *pipc_hdl)
{
    u32 tmp;
    if (pipc_hdl == NULL) {
        return 0xABCDEF00;
    }

    tmp = pipc_hdl->dev_id;
    tmp = ((unsigned long)tmp << MKU32_SHIFT) + pipc_hdl->chn_id;
    tmp = ((unsigned long)tmp << MKU32_SHIFT) + pipc_hdl->res_bit;
    return tmp;
}

/* ****************************************************************************
 * 函 数 名: ipc_handle_get
 *
 * 功能描述  :   IPC通道获取
 *
 * 返 回 值: IPC通道
 * *************************************************************************** */
struct ipc_handle *ipc_handle_get(ipc_res_id_e ipc_res_id)
{
    u32 i, j;
    struct ipc_res_t *pres = NULL;

    for (i = 0; i < g_channel_fastptr; i++) {
        for (j = 0; j < RES_MAX_NUM; j++) {
            pres = g_ipc_channel[i]->res[j];
            if (pres == NULL) {
                continue;
            }
            if (ipc_res_id == pres->res_id) {
                return &pres->handle;
            }
        }
    }

    return NULL;
}

int ipc_get_byhandle(struct ipc_handle *pipc_hdl, struct ipc_device_t **ppdev, struct ipc_channel_t **ppchn,
    struct ipc_res_t **ppres)
{
    struct ipc_device_t *pdev = NULL;
    struct ipc_channel_t *pchn = NULL;
    struct ipc_res_t *pres = NULL;

    if (pipc_hdl == NULL) {
        return IPC_ERR;
    }

    if (pipc_hdl->magic != IPC_MAGIC) {
        ipc_print_dbg("ipc_get_byhandle ipc magic: %d filed\n", pipc_hdl->magic);
        return IPC_HANDLE_ERR;
    }

    pdev = g_ipc_device[pipc_hdl->dev_id];
    pchn = &pdev->ipc_chn[pipc_hdl->chn_id];
    pres = pchn->res[pipc_hdl->res_bit];

    if (pipc_hdl != &pres->handle) {
        // should be system error
        ipc_print_dbg("ipc_get_byhandle phandle filed\n");
        return IPC_HANDLE_ERR;
    }

    *ppdev = pdev;
    *ppchn = pchn;
    *ppres = pres;

    return IPC_OK;
}

/* ****************************************************************************
 * 函 数 名: bsp_ipc_send
 *
 * 功能描述  :   IPC通道发送
 *
 * 返 回 值: IPC通道发送
 * *************************************************************************** */
s32 bsp_ipc_send(struct ipc_handle *pipc_hdl)
{
    unsigned long flags = 0;
    u32 bit_value;

    struct ipc_device_t *pdev = NULL;
    struct ipc_channel_t *pchn = NULL;
    struct ipc_res_t *pres = NULL;

    if (ipc_get_byhandle(pipc_hdl, &pdev, &pchn, &pres)) {
        return IPC_ERR;
    }

    bit_value = IPC_BIT_MASK(pchn->chn_id) & MDM_CHN_MASK;
    if (g_modem_reset_flag && bit_value) {
        return IPC_ERR_MODEM_RESETING;
    }

    spin_lock_irqsave(&g_ipc_lock, flags);
    writel((u32)IPC_BIT_MASK(pres->res_bit), (void *)((char *)pdev->base_addr + BSP_IPC_CPU_RAW_INT(pchn->chn_id)));
    pres->send_times++;
    spin_unlock_irqrestore(&g_ipc_lock, flags);

    return IPC_OK;
}

/* ****************************************************************************
 * 函 数 名: bsp_ipc_open
 *
 * 功能描述  :   打开IPC
 *
 * 返 回 值: 打开是否成功
 * *************************************************************************** */
s32 bsp_ipc_open(ipc_res_id_e ipc_res_id, struct ipc_handle **ppipc_hdl)
{
    struct ipc_handle *pipc_hdl = NULL;

    if (g_ipc_init_flag == 0 || ppipc_hdl == NULL) {
        return IPC_ERR;
    }

    pipc_hdl = ipc_handle_get(ipc_res_id);
    if (pipc_hdl == NULL) {
        ipc_print_err("open ipc res[0x%x] failed handle NULL!\n", ipc_res_id);
        return IPC_ERR;
    }
    *ppipc_hdl = pipc_hdl;
    return IPC_OK;
}

/* ****************************************************************************
 * 函 数 名: bsp_ipc_connect
 *
 * 功能描述  :   打开IPC
 *
 * 返 回 值: 打开是否成功
 * *************************************************************************** */
s32 bsp_ipc_connect(struct ipc_handle *pipc_hdl, ipc_cbk routine, void *parameter)
{
    unsigned long flags = 0;
    u32 int_mask;
    struct ipc_device_t *pdev = NULL;
    struct ipc_channel_t *pchn = NULL;
    struct ipc_res_t *pres = NULL;

    if (ipc_get_byhandle(pipc_hdl, &pdev, &pchn, &pres)) {
        ipc_print_err("ipc_get_byhandle err!\n");
        return IPC_ERR;
    }

    spin_lock_irqsave(&g_ipc_lock, flags);
    pres->ipc_int_table.routine = routine;
    pres->ipc_int_table.arg = parameter;

    // enbale interrupt
    int_mask = readl((char *)pdev->base_addr + BSP_IPC_CPU_INT_MASK(pchn->chn_id));
    int_mask |= (u32)(IPC_BIT_MASK(pres->res_bit));
    writel(int_mask, (char *)pdev->base_addr + BSP_IPC_CPU_INT_MASK(pchn->chn_id));
    spin_unlock_irqrestore(&g_ipc_lock, flags);

    return IPC_OK;
}

/* ****************************************************************************
 * 函 数 名: bsp_ipc_disconnect
 *
 * 功能描述  :   打开IPC
 *
 * 返 回 值: 打开是否成功
 * *************************************************************************** */
s32 bsp_ipc_disconnect(struct ipc_handle *pipc_hdl)
{
    unsigned long flags = 0;
    u32 int_mask;
    struct ipc_device_t *pdev = NULL;
    struct ipc_channel_t *pchn = NULL;
    struct ipc_res_t *pres = NULL;

    if (ipc_get_byhandle(pipc_hdl, &pdev, &pchn, &pres)) {
        ipc_print_err("ipc_get_byhandle err!\n");
        return IPC_ERR;
    }

    spin_lock_irqsave(&g_ipc_lock, flags);

    // disenbale interrupt
    int_mask = readl((void *)((char *)pdev->base_addr + BSP_IPC_CPU_INT_MASK(pchn->chn_id)));
    int_mask &= (u32)(~(IPC_BIT_MASK(pres->res_bit)));
    writel(int_mask, (void *)((char *)pdev->base_addr + BSP_IPC_CPU_INT_MASK(pchn->chn_id)));

    pres->ipc_int_table.routine = NULL;
    pres->ipc_int_table.arg = NULL;
    spin_unlock_irqrestore(&g_ipc_lock, flags);

    return IPC_OK;
}

void bsp_ipc_modem_reset(drv_reset_cb_moment_e stage, int userdata)
{
    unsigned long flags = 0;
    spin_lock_irqsave(&g_ipc_lock, flags);
    if (stage == MDRV_RESET_CB_BEFORE) {
        g_modem_reset_flag = 1;
    }
    if (stage == MDRV_RESET_RESETTING) {
        g_modem_reset_flag = 0;
    }
    spin_unlock_irqrestore(&g_ipc_lock, flags);
    return;
}

int ipc_enable_res(int ipc_res_id)
{
    struct ipc_device_t *pdev = NULL;
    struct ipc_channel_t *pchn = NULL;
    struct ipc_res_t *pres = NULL;
    u32 int_mask;

    struct ipc_handle *phandle = ipc_handle_get(ipc_res_id);
    if (phandle == NULL) {
        ipc_print_err("res_id is unavailable!\n");
        return IPC_ERR;
    }

    if (ipc_get_byhandle(phandle, &pdev, &pchn, &pres)) {
        ipc_print_err("ipc_get_byhandle err!\n");
        return IPC_ERR;
    }

    // enbale interrupt
    int_mask = readl((char *)pdev->base_addr + BSP_IPC_CPU_INT_MASK(pchn->chn_id));
    int_mask |= (u32)(IPC_BIT_MASK(pres->res_bit));
    writel(int_mask, (char *)pdev->base_addr + BSP_IPC_CPU_INT_MASK(pchn->chn_id));

    return IPC_OK;
}

void bsp_ccore_ipc_disable(void)
{
    unsigned long flag = 0;
    u32 i, chn_id, mask_status;
    char *base_addr = NULL;
    spin_lock_irqsave(&g_ipc_lock, flag);
    for (i = 0; i < g_channel_fastptr; i++) {
        if (g_ipc_channel[i]->isrecv == 0) {
            continue;
        }
        base_addr = g_ipc_device[g_ipc_channel[i]->dev_id]->base_addr;
        chn_id = g_ipc_channel[i]->chn_id;
        mask_status = readl((void *)base_addr + BSP_IPC_CPU_INT_MASK(chn_id));
        mask_status &= (~g_ipc_device[i]->ipc_chn[chn_id].src_mdm);
        writel(mask_status, (void *)(base_addr + BSP_IPC_CPU_INT_MASK(chn_id)));
    }

    (void)ipc_enable_res(IPC_ACPU_INT_SRC_TSP_RESET_DSS0_IDLE);
    (void)ipc_enable_res(IPC_ACPU_INT_SRC_TSP_RESET_DSS1_IDLE);
    (void)ipc_enable_res(IPC_ACPU_INT_SRC_TSP_RESET_DSS2_IDLE);
    (void)ipc_enable_res(IPC_ACPU_INT_SRC_TSP_RESET_DSS3_IDLE);
    (void)ipc_enable_res(IPC_ACPU_INT_SRC_TSP_RESET_SUCC);
    spin_unlock_irqrestore(&g_ipc_lock, flag);
}

void bsp_ccore_ipc_enable(void)
{
    unsigned long flag = 0;
    u32 i, chn_id, mask_status;
    char *base_addr = NULL;

    spin_lock_irqsave(&g_ipc_lock, flag);
    for (i = 0; i < g_channel_fastptr; i++) {
        if (g_ipc_channel[i]->isrecv == 0) {
            continue;
        }
        base_addr = g_ipc_device[g_ipc_channel[i]->dev_id]->base_addr;
        chn_id = g_ipc_channel[i]->chn_id;
        mask_status = readl((void *)base_addr + BSP_IPC_CPU_INT_MASK(chn_id));
        mask_status |= g_ipc_device[i]->ipc_chn[chn_id].src_mdm;
        writel(mask_status, (void *)(base_addr + BSP_IPC_CPU_INT_MASK(chn_id)));
    }
    spin_unlock_irqrestore(&g_ipc_lock, flag);
}

irqreturn_t ipc_int_handler(int irq_no, void *intput_arg)
{
    u32 int_stat;
    u32 bit_value;

    u32 dev_id, chn_id, i;
    void *arg = intput_arg;
    u32 ipc_hdl = (u32)(uintptr_t)arg;
    struct ipc_res_t *pres = NULL;

    dev_id = GET_DEV_IRQ_ID(ipc_hdl);
    chn_id = GET_CHN_IRQ_ID(ipc_hdl);

    int_stat = readl((void *)((char *)g_ipc_device[dev_id]->base_addr + BSP_IPC_CPU_INT_STAT(chn_id)));
    writel(int_stat, (void *)((char *)g_ipc_device[dev_id]->base_addr + BSP_IPC_CPU_INT_CLR(chn_id)));

    for (i = 0; i < RES_MAX_NUM; i++) {

        bit_value = int_stat & IPC_BIT_MASK(i);
        if (bit_value == 0) {
            continue;
        }

        pres = g_ipc_device[dev_id]->ipc_chn[chn_id].res[i];
        if (pres == NULL) {
            ipc_print_err("IpcIntHandler: unexpected interrupt! devid =%d, chn_id %d,res_bit %d\n", dev_id,
                chn_id, i);
            continue;
        }

        if (pres->ipc_int_table.routine != NULL) {
            pres->begin_stamp = bsp_get_slice_value();
            pres->ipc_int_table.routine(pres->ipc_int_table.arg);
            pres->end_stamp = bsp_get_slice_value();
        } else {
            ipc_print_err("IpcIntHandler:No IntConnect! devid =%d, chn_id %d,res_bit %d\n", dev_id,
                chn_id, i);
        }
        pres->handle_times++;
    }
    return IRQ_HANDLED;
}

irqreturn_t ipc_wake_irq_handler(int irq, void *intput_arg)
{
    UNUSED(irq);
    UNUSED(intput_arg);
    return IRQ_HANDLED;
}

s32 ipc_wake_source_init(void)
{
    u32 wake_irq;
    device_node_s *node = NULL;

    node = bsp_dt_find_node_by_path("/ipc_wake_irq");
    if (node == NULL) {
        ipc_print_err("ipc no need to init wake source.\n");
        return IPC_OK;
    }

    wake_irq = bsp_dt_irq_parse_and_map(node, 0);
    if (request_irq(wake_irq, ipc_wake_irq_handler, 0, "ipc_wake_irq", NULL)) {
        ipc_print_err("ipc_wake_irq register fail\n");
        return IPC_ERR;
    }

    enable_irq_wake(wake_irq);
    return IPC_OK;
}

s32 ipc_init_interrupt(void)
{
    s32 ret;
    u32 i, j;
    struct ipc_channel_t *shadow_chn = NULL;

    for (i = 0; i < IPC_NUM; i++) {
        if (g_ipc_device[i] == NULL) {
            continue;
        }

        for (j = 0; j < CHN_MAX_NUM; j++) {
            shadow_chn = &g_ipc_device[i]->ipc_chn[j];
            if (shadow_chn->used == 0 || shadow_chn->isrecv == 0) {
                continue;
            }

            writel(0, (void *)((char *)g_ipc_device[i]->base_addr + BSP_IPC_CPU_INT_MASK(j)));
            writel(0xffffffff, (void *)((char *)g_ipc_device[i]->base_addr + BSP_IPC_CPU_INT_CLR(j)));
            ret = request_irq(shadow_chn->irq_no, (irq_handler_t)ipc_int_handler, IRQF_NO_SUSPEND,
                "ipc_fusion_irq", (void *)(uintptr_t)(((unsigned long)i << DEV_IRQ_SHIFT) + j));
            if (ret) {
                ipc_print_err("ipc (%d) interrupt: no=[%d] ipc int handler error,init failed\n", i, j);
                return IPC_ERR;
            }
        }
    }

    return IPC_OK;
}

s32 ipc_resources_init(device_node_s *pchn_dev, struct ipc_device_t *pdev, struct ipc_channel_t *pchn)
{
    s32 ret;
    u32 res_bit;
    u32 src_core = 0;
    u32 res_parm[0x2] = {0};

    device_node_s local_res_node;
    device_node_s *res_child_node = &local_res_node;
    struct ipc_res_t *pres = NULL;

    bsp_dt_for_each_child_of_node(pchn_dev, res_child_node)
    {
        ret = bsp_dt_property_read_u32(res_child_node, "res_bit", &res_bit);
        if (ret || res_bit >= RES_MAX_NUM || pchn->res[res_bit]) {
            ipc_print_err("ipc read res_bit failed!\n");
            return IPC_ERR;
        }

        ret = bsp_dt_property_read_u32_array(res_child_node, "res_parm", res_parm, sizeof(res_parm) / sizeof(res_parm[0]));
        if (ret) {
            ipc_print_err("ipc read res_parm failed!\n");
            return IPC_ERR;
        }

        if (res_parm[1]) {
            src_core |= IPC_BIT_MASK(res_bit);
        }

        pres = (struct ipc_res_t *)osl_malloc(sizeof(struct ipc_res_t));
        if (pres == NULL) {
            ipc_print_err("osl_malloc chn_dev->res[%d] failed\n", res_bit);
            return IPC_ERR;
        }
        (void)memset_s((void *)pres, sizeof(struct ipc_res_t), 0, sizeof(struct ipc_res_t));

        pres->res_bit = res_bit;
        pres->res_id = res_parm[0];
        pres->handle.dev_id = pdev->devid;
        pres->handle.chn_id = pchn->chn_id;
        pres->handle.res_bit = res_bit;
        pres->handle.magic = IPC_MAGIC;
        pchn->res[res_bit] = pres;
        ipc_print_dbg("chn_dev->res[%d] res_id:0x%x ok\n", pres->res_bit, pres->res_id);
    }
    pchn->src_mdm = src_core;
    return IPC_OK;
}

s32 ipc_channels_init(device_node_s *dev_node, struct ipc_device_t *pdev)
{
    s32 ret;
    u32 devid, chn_id, irq_no, isrecv;
    device_node_s local_node;
    device_node_s *pchn = &local_node;

    bsp_dt_for_each_child_of_node(dev_node, pchn) {
        devid = pdev->devid;
        ret = bsp_dt_property_read_u32(pchn, "chn_id", &chn_id);
        if (ret || chn_id >= CHN_MAX_NUM || pdev->ipc_chn[chn_id].used) {
            ipc_print_err("ipc (%d) read chn_id: %d failed!\n", devid, chn_id);
            return IPC_ERR;
        }

        ret = bsp_dt_property_read_u32(pchn, "isrecv", &isrecv);
        if (ret) {
            ipc_print_err("ipc (%d) chn_id(%d) read isrecv: %d failed!\n", devid, chn_id, isrecv);
            return IPC_ERR;
        }

        if (isrecv) {
            irq_no = bsp_dt_irq_parse_and_map(pchn, 0);
            if (irq_no == 0) {
                ipc_print_err("ipc (%d):irq[%d] num error\n", devid, irq_no);
                return IPC_ERR;
            }
            pdev->ipc_chn[chn_id].irq_no = irq_no;
        }

        pdev->ipc_chn[chn_id].dev_id = devid;
        pdev->ipc_chn[chn_id].chn_id = chn_id;
        pdev->ipc_chn[chn_id].isrecv = isrecv;
        pdev->ipc_chn[chn_id].used = 1;

        ret = ipc_resources_init(pchn, pdev, &pdev->ipc_chn[chn_id]);
        if (ret) {
            ipc_print_err("ipc (%d) chn_id (%d) ipc_resource_init failed!\n", devid, chn_id);
            return IPC_ERR;
        }
        g_ipc_channel[g_channel_fastptr] = &pdev->ipc_chn[chn_id];
        g_channel_fastptr++;

        ipc_print_dbg("ipc (%d) chn_id (%d) ipc_resource_init ok!\n", devid, chn_id);
    }
    return IPC_OK;
}

s32 ipc_device_init(device_node_s *parent_node)
{
    s32 ret;
    u32 devid, ismdm;
    u32 array_reg[0x2];
    struct ipc_device_t *pdev = NULL;
    device_node_s local_node;
    device_node_s *dev_node = &local_node;

    bsp_dt_for_each_child_of_node(parent_node, dev_node) {
        ret = bsp_dt_property_read_u32(dev_node, "devid", &devid);
        if (ret || devid >= IPC_NUM || g_ipc_device[devid]) {
            ipc_print_err("ipc device[%d]:get devid failed\n", devid);
            return IPC_ERR;
        }

        pdev = (struct ipc_device_t *)osl_malloc(sizeof(struct ipc_device_t));
        if (pdev == NULL) {
            ipc_print_err("osl_malloc pdev[%d] failed\n", devid);
            return IPC_ERR;
        }
        (void)memset_s((void *)pdev, sizeof(struct ipc_device_t), 0, sizeof(struct ipc_device_t));

        ret = bsp_dt_property_read_u32(dev_node, "ismdm", &ismdm);
        if (ret) {
            osl_free(pdev);
            ipc_print_err("ipc device[%d]:get devid failed\n", ismdm);
            return IPC_ERR;
        }

        pdev->devid = devid;
        pdev->ismdm = ismdm;

        ret = bsp_dt_property_read_u32_array(dev_node, "reg", array_reg, sizeof(array_reg) / sizeof(array_reg[0]));
        pdev->base_addr = ioremap(array_reg[0], array_reg[1]);
        ipc_print_dbg("ipc 0x%x!\n", (unsigned int)(uintptr_t)pdev->base_addr);

        if (pdev->base_addr == NULL) {
            osl_free(pdev);
            ipc_print_err("ipc (%d) ioremap failed!\n", devid);
            return IPC_ERR;
        }

        ret = ipc_channels_init(dev_node, pdev);
        if (ret) {
            osl_free(pdev);
            return IPC_ERR;
        }
        g_ipc_device[devid] = pdev;
        ipc_print_dbg("ipc device[%d] init ok\n", devid);
    }
    return IPC_OK;
}

/* ****************************************************************************
 * 函 数 名:  bsp_ipc_init
 *
 * 功能描述  :  IPC初始化函数
 *
 * 返 回 值  :  无
 * *************************************************************************** */
int bsp_ipc_init(void)
{
    s32 ret;
    device_node_s *parent_node = NULL;
    const char *node_path = "/ipc_fusion_device";
    spin_lock_init(&g_ipc_lock);
    parent_node = bsp_dt_find_node_by_path(node_path);
    if (parent_node == NULL) {
        ipc_print_err("of_find_compatible_node didn't find !\n");
        return IPC_ERR;
    }

    ret = ipc_device_init(parent_node);
    if (ret) {
        ipc_print_err("ipc_device_init failed\n");
        return IPC_ERR;
    }
    ret = ipc_wake_source_init();
    if (ret) {
        ipc_print_err("ipc_wake_source_init failed\n");
        return IPC_ERR;
    }
    ret = ipc_init_interrupt();
    if (ret) {
        ipc_print_err("ipc_init_interrupt failed\n");
        return IPC_ERR;
    }
    ipc_print_err("ipc init ok\n");

    g_ipc_init_flag = 1;

    return IPC_OK;
}

int ipc_res_show_dump(int ipc_res_id)
{
    struct ipc_device_t *pdev = NULL;
    struct ipc_channel_t *pchn = NULL;
    struct ipc_res_t *pres = NULL;

    struct ipc_handle *ipc_hdl = ipc_handle_get(ipc_res_id);
    if (ipc_hdl == NULL) {
        ipc_print_err("res_id is unavailable!\n");
        return IPC_ERR;
    }

    if (ipc_get_byhandle(ipc_hdl, &pdev, &pchn, &pres)) {
        ipc_print_err("ipc_get_byhandle err!\n");
        return IPC_ERR;
    }
    ipc_print_err("irq_no: %d\n", pchn->irq_no);
    ipc_print_err("res_id: %d\n", pres->res_id);
    ipc_print_err("res_bit: %d\n", pres->res_bit);
    ipc_print_err("send_times: %d\n", pres->send_times);
    ipc_print_err("handle_times: %d\n", pres->handle_times);
    ipc_print_err("begin_stamp: %d\n", pres->begin_stamp);
    ipc_print_err("end_stamp: %d\n", pres->end_stamp);
    ipc_print_err("ipc_int_table: 0x%x\n", (unsigned int)(uintptr_t) & (pres->ipc_int_table));
    return IPC_OK;
}

s32 bsp_ipc_bind(ipc_handle_t phandle, u32 core_mask)
{
    return 0;
}

EXPORT_SYMBOL(bsp_ipc_open);
EXPORT_SYMBOL(bsp_ipc_connect);
EXPORT_SYMBOL(bsp_ipc_disconnect);
EXPORT_SYMBOL(bsp_ipc_send);
EXPORT_SYMBOL(bsp_ipc_bind);
EXPORT_SYMBOL(ipc_res_show_dump);
EXPORT_SYMBOL(bsp_ipc_modem_reset);
EXPORT_SYMBOL(bsp_ccore_ipc_enable);
EXPORT_SYMBOL(bsp_ccore_ipc_disable);

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
arch_initcall(bsp_ipc_init);
MODULE_LICENSE("GPL");
#endif
