/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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

#include "ipcmsg_platform.h"
#include "ipcmsg_core.h"
#include "ipcmsg_device.h"
#include "ipcmsg_driver.h"

ipcmsg_device_t *g_ipcmsg_device[IPCMSG_DEVICE_MAX] = {0};
u32 g_ipcmsg_dev_num = 0;
ipcmsg_channel_t *g_ipcmsg_channel[IPCMSG_FUNC_MAX] = {0};
ipcmsg_regs_backup_t g_ipcmsg_regs_backup = {0};

void *get_ipcmsg_ns_base_vaddr(void)
{
    return g_ipcmsg_device[0]->base_addr;
}

ipcmsg_channel_t *ipcmsg_channel_get(u32 chn_id)
{
    u32 idx;
    for (idx = 0; idx < IPCMSG_FUNC_MAX; idx++) {
        if (g_ipcmsg_channel[idx] != NULL) {
            if (chn_id == g_ipcmsg_channel[idx]->chn_id) {
                return g_ipcmsg_channel[idx];
            }
        }
    }
    return NULL;
}

ipcmsg_mailbox_t *ipcmsg_mailbox_get(u32 dev_id, u32 mbx_id)
{
    u32 idx, mailbox_id;
    ipcmsg_device_t *device = ipcmsg_get_device(dev_id);
    if (device == NULL) {
        return NULL;
    }
    for (idx = 0; idx < device->mbx_num; idx++) {
        mailbox_id = device->ipcmsg_mailbox[idx]->mbx_id;
        if (mailbox_id == mbx_id) {
            return device->ipcmsg_mailbox[idx];
        }
    }
    return NULL;
}

void ipcmsg_resetting_handler(void)
{
    u32 i, j;
    ipcmsg_mailbox_t *mailbox = NULL;
    for (i = 0; i < g_ipcmsg_dev_num; i++) {
        for (j = 0; j < g_ipcmsg_device[i]->mbx_num; j++) {
            mailbox = g_ipcmsg_device[i]->ipcmsg_mailbox[j];
            if (mailbox->cp_reset == RESET_FLAG_ACORE2MODEM) {
                set_mailbox_into_reset_state(mailbox);
            }
        }
    }
}

static s32 ipcmsg_backup_regs_init(void)
{
    u32 i;
    g_ipcmsg_regs_backup.reg_idx = 0;
    for (i = 0; i < IPCMSG_SEND_MBX_NUM; i++) {
        g_ipcmsg_regs_backup.reg[i] = (ipcmsg_reg_backup_t *)ipcmsg_malloc(sizeof(ipcmsg_reg_backup_t));
        if (g_ipcmsg_regs_backup.reg[i] == NULL) {
            return IPCMSG_ERROR;
        }
    }
    return IPCMSG_OK;
}

s32 ipcmsg_mailbox_init(device_node_s *dev_node, ipcmsg_device_t *pdev)
{
    u32 ret;
    u32 src_cpu;
    u32 irq_no = 0;
    ipcmsg_mailbox_t *mailbox = NULL;
    ret = (u32)bsp_dt_property_read_u32(dev_node, "src_bit", &src_cpu);
    if (ret) {
        return IPCMSG_OK;    /* not a mailbox DTS */
    }
    if (pdev->mbx_ptr >= pdev->mbx_num) {
        return IPCMSG_ERROR;
    }
    mailbox = (ipcmsg_mailbox_t *)ipcmsg_malloc(sizeof(ipcmsg_mailbox_t));
    if (mailbox == NULL) {
        return IPCMSG_ERROR;
    }
    (void)memset_s(mailbox, sizeof(ipcmsg_mailbox_t), 0, sizeof(ipcmsg_mailbox_t));
    mailbox->data = (u32 *)ipcmsg_malloc(pdev->data_reg_size * pdev->data_reg_num);
    if (mailbox->data == NULL) {
        return IPCMSG_ERROR;
    }
    ret |= (u32)bsp_dt_property_read_u32(dev_node, "type", &mailbox->type);
    ret |= (u32)bsp_dt_property_read_u32(dev_node, "mode", &mailbox->mode);
    ret |= (u32)bsp_dt_property_read_u32(dev_node, "des_bit", &mailbox->dst_cpu);
    ret |= (u32)bsp_dt_property_read_u32(dev_node, "mailbox", &mailbox->mbx_id);
    ret |= (u32)bsp_dt_property_read_u32(dev_node, "cp_reset", &mailbox->cp_reset);
    if (ret) {
        return IPCMSG_ERROR;
    }
    if (MBX_MODE_RECV(mailbox->mode) != 0) {
        irq_no = bsp_dt_irq_parse_and_map(dev_node, 0);
        if (irq_no == 0) {
            return IPCMSG_ERROR;
        }
    }
    mailbox->src_cpu = src_cpu;
    mailbox->dev_id = pdev->dev_id;
    mailbox->irq_no = irq_no;
    mailbox->dev = (void *)pdev;
    spin_lock_init(&mailbox->mbx_lock);
    if (MBX_MODE_SEND(mailbox->mode) != 0) {
        ipcmsg_mailbox_prepare(mailbox, &g_ipcmsg_regs_backup);
    }
    pdev->ipcmsg_mailbox[pdev->mbx_ptr] = mailbox;
    pdev->mbx_ptr++;
    return IPCMSG_OK;
}

s32 ipcmsg_mailboxes_init(device_node_s *dev_node, ipcmsg_device_t *pdev)
{
    u32 ret;
    u32 i, ack_num, mailbox_num;
    u32 *ack_cpu = NULL;
    device_node_s local_node;
    device_node_s *pmbx_node = &local_node;
    ret = (u32)bsp_dt_property_read_u32(dev_node, "mailbox_num", &mailbox_num);
    if (ret) {
        ipcmsg_print_err("ipcmsg device[%d]:get data_reg_num or mailbox_num failed\n", pdev->dev_id);
        return IPCMSG_ERROR;
    }
    pdev->mbx_num = mailbox_num;
    pdev->ipcmsg_mailbox = (ipcmsg_mailbox_t **)ipcmsg_malloc(sizeof(ipcmsg_mailbox_t *) * mailbox_num);
    if (pdev->ipcmsg_mailbox == NULL) {
        ipcmsg_print_err("ipcmsg device[%d]:ipcmsg irq init pdev->ipcmsg_mailbox malloc failed!\n", pdev->dev_id);
        return IPCMSG_ERROR;
    }
    bsp_dt_for_each_child_of_node(dev_node, pmbx_node)
    {
        ret = (u32)ipcmsg_mailbox_init(pmbx_node, pdev);
        if (ret) {
            ipcmsg_print_err("ipcmsg device[%d]:ipcmsg_mailboxes_init failed\n", pdev->dev_id);
            return IPCMSG_ERROR;
        }
    }
    ret |= (u32)bsp_dt_property_read_u32(dev_node, "ack_num", &ack_num);
    ack_cpu = (u32 *)ipcmsg_malloc(sizeof(u32) * ack_num);
    pdev->irq_bdl = (irq_bundle_t **)ipcmsg_malloc(sizeof(irq_bundle_t *) * ack_num);
    if (ack_cpu == NULL || pdev->irq_bdl == NULL) {
        ipcmsg_print_err("ipcmsg device[%d]:ipcmsg irq init pdev->irq_bdl malloc failed!\n", pdev->dev_id);
        return IPCMSG_ERROR;
    }
    ret |= bsp_dt_property_read_u32_array(dev_node, "ack_cpu", ack_cpu, ack_num);
    if (ret) {
        ipcmsg_print_err("ipcmsg device[%d]:get ack_cpu failed\n", pdev->dev_id);
        return IPCMSG_ERROR;
    }
    for (i = 0; i < ack_num; i++) {
        pdev->irq_bdl[i] = (irq_bundle_t *)ipcmsg_malloc(sizeof(irq_bundle_t));
        if (pdev->irq_bdl[i] == NULL) {
            ipcmsg_print_err("ipcmsg device[%d]:pdev->irq_bdl %d malloc failed!\n", pdev->dev_id, i);
            return IPCMSG_ERROR;
        }
        pdev->irq_bdl[i]->dev = pdev;
        pdev->irq_bdl[i]->ack_cpu = ack_cpu[i];
    }
    osl_free(ack_cpu);
    ack_cpu = NULL;
    return IPCMSG_OK;
}

s32 ipcmsg_device_init(ipcmsg_device_t *pdev)
{
    u32 ret, dev_id, reg_num, reg_type;
    u32 addr_buf[0x2];
    device_node_s *p_dev_node = NULL;
    if (pdev == NULL || pdev->of_node == NULL) {
        return IPCMSG_ERROR;
    }
    p_dev_node = (device_node_s *)pdev->of_node;
    ret = bsp_dt_property_read_u32_array(p_dev_node, "reg", addr_buf, sizeof(addr_buf) / sizeof(addr_buf[0]));
    pdev->base_addr = ioremap(addr_buf[0], addr_buf[0x1]);
    if (pdev->base_addr == NULL) {
        ipcmsg_print_err("ipcmsg ioremap fail\n");
        return IPCMSG_ERROR;
    }
    ret = (u32)bsp_dt_property_read_u32(p_dev_node, "dev_id", &dev_id);
    ret |= (u32)bsp_dt_property_read_u32(p_dev_node, "data_reg_num", &reg_num);
    ret |= (u32)bsp_dt_property_read_u32(p_dev_node, "reg_type", &reg_type);
    if (ret) {
        ipcmsg_print_err("ipcmsg device[%d]:get data_reg_num or mailbox_num failed\n", dev_id);
        return IPCMSG_ERROR;
    }
    if (dev_id >= IPCMSG_DEVICE_MAX || reg_num > IPCMSG_REG_MAX || reg_type > IPCMSG_DEVICE_TYPE_NUM) {
        ipcmsg_print_err("ipcmsg device[%d] or data_reg_num[%d] or reg_type[%d] error\n", dev_id, reg_num, reg_type);
        return IPCMSG_ERROR;
    }
    pdev->dev_id = dev_id;
    pdev->data_reg_num = reg_num;
    pdev->data_reg_size = IPCMSG_DATA_REG_SIZE;
    pdev->reg_type = reg_type;
    if (ipcmsg_device_unlock(pdev->base_addr, reg_type)) {
        ipcmsg_print_err("ipc reg is lock\n");
        return IPCMSG_ERROR;
    }
    ret = (u32)ipcmsg_mailboxes_init(p_dev_node, pdev);
    if (ret) {
        ipcmsg_print_err("ipcmsg device[%d]:init failed!\n", dev_id);
        ipcmsg_free(pdev);
        return IPCMSG_ERROR;
    }
    g_ipcmsg_device[dev_id] = pdev;
    g_ipcmsg_dev_num++;
    ipcmsg_print_dbg("ipcmsg device[%d] init ok\n", dev_id);
    return IPCMSG_OK;
}

s32 ipcmsg_irq_init(ipcmsg_device_t *pdev)
{
    s32 ret;
    u32 i, fast_irq_no, fast_irq_num;
    device_node_s *p_dev_node = NULL;
    if (pdev == NULL || pdev->of_node == NULL) {
        return IPCMSG_ERROR;
    }
    p_dev_node = (device_node_s *)pdev->of_node;
    ret = (u32)bsp_dt_property_read_u32(p_dev_node, "fast_irq_num", &fast_irq_num);
    if (ret) {
        ipcmsg_print_err("ipcmsg device[%d]:ipcmsg_channel_init failed\n", pdev->dev_id);
        return IPCMSG_ERROR;
    }
    for (i = 0; i < pdev->mbx_num; i++) {
        if (MBX_MODE_RECV(pdev->ipcmsg_mailbox[i]->mode) != 0) {
            ret = request_irq(pdev->ipcmsg_mailbox[i]->irq_no, ipcmsg_mailbox_handler, IRQF_NO_SUSPEND,
                "ipcmsg_mailbox_irq", (void *)pdev->ipcmsg_mailbox[i]);
            if (ret) {
                ipcmsg_print_err("ipcmsg device[%d]:ipcmsg mailbox request irq failed!\n", pdev->dev_id);
                return IPCMSG_ERROR;
            }
        }
    }
    if (fast_irq_num != 0) {
        for (i = 0; i < fast_irq_num; i++) {
            fast_irq_no = bsp_dt_irq_parse_and_map(p_dev_node, i);
            if (fast_irq_no == 0) {
                return IPCMSG_ERROR;
            }
            ret = request_irq(fast_irq_no, ipcmsg_common_handler, IRQF_NO_SUSPEND, "ipcmsg_common_irq",
                (void *)pdev->irq_bdl[i]);
            if (ret) {
                ipcmsg_print_err("ipcmsg device[%d]:ipcmsg fast request irq failed!\n", pdev->dev_id);
                return ret;
            }
        }
    }
    return ret;
}

s32 channel_mailbox_correlate(ipcmsg_device_t *pdev, ipcmsg_channel_t *channel)
{
    ipcmsg_mailbox_t *mailbox = NULL;
    mailbox = ipcmsg_mailbox_get(pdev->dev_id, channel->chn_cfg.chn_info.mbx_id);
    if (mailbox == NULL) {
        ipcmsg_print_err("ipcmsg channel get mailbox failed when ipcmsg channels init!\n");
        return IPCMSG_ERROR;
    }
    channel->mbx = mailbox;
    if (mailbox->type != DATA_WITH_HEADER) {
        mailbox->channel = channel;
    }
    return IPCMSG_OK;
}

s32 ipcmsg_channels_init(device_node_s *dev_node, ipcmsg_device_t *pdev)
{
    u32 chn_cfg, channel_size, ret, func_id;
    ipcmsg_channel_t *channel = NULL;
    ret = (u32)bsp_dt_property_read_u32(dev_node, "chn_cfg", &chn_cfg);
    if (ret) {
        return IPCMSG_OTHER_TYPE;
    }
    func_id = GET_FUNC_ID(chn_cfg);
    channel = (ipcmsg_channel_t *)ipcmsg_malloc(sizeof(ipcmsg_channel_t));
    if (channel == NULL) {
        ipcmsg_print_err("ipcmsg channel malloc failed when ipcmsg channels init!\n");
        return IPCMSG_ERROR;
    }
    (void)memset_s(channel, sizeof(ipcmsg_channel_t), 0, sizeof(ipcmsg_channel_t));
    if (bsp_dt_property_read_u32(dev_node, "chn_id", &channel->chn_id)) {
        ipcmsg_print_err("ipcmsg channel get chn_id failed when ipcmsg channels init!\n");
        goto err_free_channel;
    }
    channel->magic = IPC_MSG_CHN_MAGIC;
    channel->chn_cfg.value = chn_cfg;
    channel_size = pdev->data_reg_num * pdev->data_reg_size * IPCMSG_PKT_MAX;
    channel->chn_buf = (u8 *)ipcmsg_malloc(channel_size);
    if (channel->chn_buf == NULL) {
        ipcmsg_print_err("ipcmsg channel->chn_buf malloc failed when ipcmsg channels init!\n");
        goto err_free_channel;
    }
    (void)memset_s(channel->chn_buf, channel_size, 0, channel_size);
    if (channel->chn_cfg.chn_info.mbx_id >= IPCMSG_MBX_ID_MAX ||
        channel->chn_cfg.chn_info.dev_id >= IPCMSG_DEVICE_MAX) {
        ipcmsg_print_err("ipcmsg channel [0x%x]:ipcmsg_channels init failed\n", func_id);
        goto err_free_chn_buf;
    }
    ret = (u32)channel_mailbox_correlate(pdev, channel);
    if (ret) {
        ipcmsg_print_err("ipcmsg channel correlate mailbox failed!\n");
        goto err_free_chn_buf;
    }
    channel->chn_state = CHANNEL_CLOSE;
    g_ipcmsg_channel[func_id] = channel;
    return IPCMSG_OK;
err_free_chn_buf:
    ipcmsg_free(channel->chn_buf);
err_free_channel:
    ipcmsg_free(channel);
    return IPCMSG_ERROR;
}

s32 ipcmsg_chn_init(ipcmsg_device_t *pdev)
{
    s32 ret;
    device_node_s *p_dev_node = NULL;
    device_node_s *child_node = NULL;
    device_node_s local_node;
    if (pdev == NULL || pdev->of_node == NULL) {
        return IPCMSG_ERROR;
    }
    p_dev_node = (device_node_s *)pdev->of_node;
    child_node = &local_node;
    (void)memset_s(child_node, sizeof(device_node_s), 0, sizeof(device_node_s));
    bsp_dt_for_each_child_of_node(p_dev_node, child_node)
    {
        ret = ipcmsg_channels_init(child_node, pdev);
        if (ret == IPCMSG_OTHER_TYPE) {
            continue;
        } else if (ret == IPCMSG_ERROR) {
            ipcmsg_print_err("ipcmsg device[%d]:ipcmsg_channels_init failed\n", pdev->dev_id);
            continue;
        } else {
            ipcmsg_print_dbg("chan init ok\n");
        }
    }
    return IPCMSG_OK;
}

/*
 * 功能描述:  ipc低功耗接口
 */
static int ipcmsg_suspend(struct device *dev)
{
    g_ipcmsg_regs_backup.reg_idx = 0;
    return IPCMSG_OK;
}

static int ipcmsg_resume(struct device *dev)
{
    u32 i, j;
    ipcmsg_mailbox_t *mailbox = NULL;
    /* 恢复邮箱寄存器 */
    for (i = 0; i < g_ipcmsg_dev_num; i++) {
        for (j = 0; j < g_ipcmsg_device[i]->mbx_num; j++) {
            mailbox = g_ipcmsg_device[i]->ipcmsg_mailbox[j];
            if (MBX_MODE_SEND(mailbox->mode) != 0) {
                ipcmsg_mailbox_resume(mailbox, &g_ipcmsg_regs_backup);
                g_ipcmsg_regs_backup.reg_idx++;
            }
        }
    }
    return IPCMSG_OK;
}

ipcmsg_device_t *ipcmsg_get_device(u32 dev_id)
{
    if (dev_id >= IPCMSG_DEVICE_MAX) {
        ipcmsg_print_err("dev_id is larger than ipcmsg_device num!\n");
        return NULL;
    }
    return g_ipcmsg_device[dev_id];
}

ipcmsg_channel_t *ipcmsg_get_channel(u32 func_id)
{
    if (func_id >= IPCMSG_FUNC_MAX) {
        ipcmsg_print_err("func_id is larger than ipcmsg_func_cb num!\n");
        return NULL;
    }
    return g_ipcmsg_channel[func_id];
}

irqreturn_t ipcmsg_wake_irq_handler(int irq, void *intput_arg)
{
    UNUSED(irq);
    UNUSED(intput_arg);
    return IRQ_HANDLED;
}

s32 ipcmsg_wake_source_init(device_node_s *dev_node)
{
    s32 ret;
    u32 irq_wake_num, wake_irq, irq_source_idx;
    device_node_s *wake_node = NULL;
    ret = bsp_dt_property_read_u32(dev_node, "irq_wake_num", &irq_wake_num);
    if (ret) {
        ipcmsg_print_err("ipcmsg no need to init wake source.\n");
        return IPCMSG_OK;
    }
    wake_node = bsp_dt_find_node_by_path("/ipcmsg_wake_irq");
    if (wake_node == NULL) {
        return IPCMSG_ERROR;
    }
    for (irq_source_idx = 0; irq_source_idx < irq_wake_num; irq_source_idx++) {
        ret = bsp_dt_property_read_u32_index(wake_node, "interrupts", irq_source_idx, &wake_irq);
        if (ret) {
            ipcmsg_print_err("get %d interrupts fail\n", wake_irq);
            return IPCMSG_ERROR;
        }
        wake_irq = bsp_dt_irq_parse_and_map(wake_node, irq_source_idx);
        if (request_irq(wake_irq, ipcmsg_wake_irq_handler, 0, "ipcmsg_wake_irq", NULL)) {
            ipcmsg_print_err("ipcmsg_wake_irq register fail\n");
            return IPCMSG_ERROR;
        }
        enable_irq_wake(wake_irq);
    }
    return IPCMSG_OK;
}

static s32 ipcmsg_probe(struct platform_device *pdev)
{
    s32 ret;
    ipcmsg_device_t *device = NULL;
    device_node_s *ptr_device_node = NULL;
    device_node_s child_node;
    device_node_s *local_node = &child_node;
    const char *node_path = "/ipcmsg_device";
    ptr_device_node = bsp_dt_find_node_by_path(node_path);
    if (ptr_device_node == NULL) {
        return IPCMSG_ERROR;
    }
    ret = ipcmsg_backup_regs_init();
    if (ret) {
        return IPCMSG_ERROR;
    }
    ret = ipcmsg_wake_source_init(ptr_device_node);
    if (ret) {
        return IPCMSG_ERROR;
    }
    bsp_dt_for_each_child_of_node(ptr_device_node, local_node)
    {
        device = (ipcmsg_device_t *)ipcmsg_malloc(sizeof(ipcmsg_device_t));
        if (device == NULL) {
            ipcmsg_print_err("ipcmsg dts not found\n");
            return IPCMSG_ERROR;
        }
        (void)memset_s(device, sizeof(ipcmsg_device_t), 0, sizeof(ipcmsg_device_t));
        device->of_node = local_node;
        ret = ipcmsg_device_init(device);
        if (ret) {
            ipcmsg_print_err("ipcmsg_device_init failed\n");
            return IPCMSG_ERROR;
        }
        ret = ipcmsg_chn_init(device);
        if (ret) {
            ipcmsg_print_err("ipcmsg_channel_init failed\n");
            return IPCMSG_ERROR;
        }
        ret = ipcmsg_irq_init(device);
        if (ret) {
            ipcmsg_print_err("ipcmsg_irq_init failed\n");
            return IPCMSG_ERROR;
        }
    }
    return IPCMSG_OK;
}

static struct platform_device g_ipcmsg_platform_device = {
    .name = "Modem IPCMSG",
};

static const struct dev_pm_ops g_ipcmsg_pm_ops = {
    .suspend_noirq = ipcmsg_suspend,
    .resume_noirq = ipcmsg_resume,
    .prepare = NULL,
    .complete = NULL,
};
static struct platform_driver g_ipcmsg_platform_driver = {
    .probe = ipcmsg_probe,
    .driver =
        {
            .name = "Modem IPCMSG",
            .owner = THIS_MODULE,
            .pm = &g_ipcmsg_pm_ops,
        },
};

s32 bsp_ipcmsg_init(void)
{
    s32 ret;
    static int inited = 0;
    if (inited) {
        ipcmsg_print_err("ipcmsg initialization process has been executed!\n");
        return IPCMSG_OK;
    }
    ret = platform_device_register(&g_ipcmsg_platform_device);
    if (ret) {
        return ret;
    }
    ret = platform_driver_register(&g_ipcmsg_platform_driver);
    if (ret) {
        platform_device_unregister(&g_ipcmsg_platform_device);
        return ret;
    }
    inited = 1;
    ipcmsg_print_err("ipcmsg init ok\n");
    return ret;
}


EXPORT_SYMBOL(get_ipcmsg_ns_base_vaddr);
#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
arch_initcall(bsp_ipcmsg_init);
#endif
MODULE_DESCRIPTION("IPCMSG Driver");
MODULE_LICENSE("GPL");
