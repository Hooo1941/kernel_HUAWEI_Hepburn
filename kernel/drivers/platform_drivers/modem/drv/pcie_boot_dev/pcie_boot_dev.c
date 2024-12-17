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
 * Description: PCIE Iot Boot Driver
 * Note: This Driver is specially used for X+B Host PCIE PM.
 */
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/clk.h>
#include <linux/msi.h>
#include <linux/irq.h>
#include <linux/of_irq.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/pci_regs.h>
#include <linux/pci-aspm.h>
#include <bsp_pcie.h>
#include "securec.h"
#include "pcie_boot_dev.h"
#include "adrv_pcie.h"
#include "bsp_slice.h"
#include "bsp_print.h"

#define PCI_VENDOR_ID_IOT 0x19E5
#define PCI_DEVICE_ID_IOT 0x5000

#define MAX_RECORD_BUFFER  0x40
#undef THIS_MODU
#define THIS_MODU mod_pcie

#define PCIE_BOOT_TRACE(fmt, ...) do { \
    bsp_err("%s:" fmt "\n", __FUNCTION__, ##__VA_ARGS__); \
} while (0)

struct pcie_boot_callback_info_list {
    struct pcie_callback_info list[PCIE_BOOT_USER_NUM];
};
enum pcie_boot_msg_act_type {
    MSI_ARRIVE = 1,
    MSG_READ = 2,
    MSG_CLEAR = 3,
    MSG_SEND = 4,
    MSG_ACT_MAX
};

struct pcie_boot_msg_act_dbg {
    u32 user_id;
    u32 msg_act;
    u32 time_stamp;
};

struct pcie_boot_msg_record {
    struct pcie_boot_msg_act_dbg pcie_boot_msg_act[MAX_RECORD_BUFFER];
    u32 trace_index;
};

struct pcie_boot_msg_record g_pcie_boot_msg_record;

struct pcie_boot_callback_info_list g_pcie_boot_user_cb;
struct pcie_rc_dev_info g_pcie_rc_dev_info;
u32 g_pcie_boot_shutdown = 0;
volatile u32 g_pcie_send_msg_done = 1;
u32 g_pcie_boot_uninstall_flag = 0;


void pcie_boot_msg_dbg(enum pcie_msi_cmd_id user_id, enum pcie_boot_msg_act_type msg_act)
{
    u32 index = g_pcie_boot_msg_record.trace_index % MAX_RECORD_BUFFER;
    u32 timestamp = bsp_get_slice_value();

    g_pcie_boot_msg_record.pcie_boot_msg_act[index].msg_act = msg_act;
    g_pcie_boot_msg_record.pcie_boot_msg_act[index].user_id = user_id;
    g_pcie_boot_msg_record.pcie_boot_msg_act[index].time_stamp = timestamp;
    g_pcie_boot_msg_record.trace_index++;

    return;
}

int bsp_pcie_boot_cb_register(enum pcie_boot_user_id usr_id, struct pcie_callback_info *info)
{
    if (info == NULL) {
        PCIE_BOOT_TRACE("param is null");
        return -1;
    }

    if (g_pcie_boot_user_cb.list[usr_id].callback) {
        PCIE_BOOT_TRACE("input usr_id callback has been registered");
        return -1;
    }

    if (usr_id >= PCIE_BOOT_USER_NUM) {
        PCIE_BOOT_TRACE("invalid input usr_id");
        return -1;
    }

    g_pcie_boot_user_cb.list[usr_id].callback = info->callback;
    g_pcie_boot_user_cb.list[usr_id].callback_args = info->callback_args;

    return 0;
}

int bsp_pcie_boot_cb_run(enum pcie_boot_callback_id callback_stage)
{
    u32 ret;
    u32 total_ret = 0;
    int user_id;

    for (user_id = 0; user_id < PCIE_BOOT_USER_NUM; user_id++) {
        if (g_pcie_boot_user_cb.list[user_id].callback) {
            ret = g_pcie_boot_user_cb.list[user_id].callback(user_id, callback_stage,
                                                             g_pcie_boot_user_cb.list[user_id].callback_args);
            total_ret |= ret;
        }
    }
    return (int)total_ret;
}

static const struct pci_device_id g_pci_boot_table[] = {
    { PCI_DEVICE(PCI_VENDOR_ID_IOT, PCI_DEVICE_ID_IOT) },
    {},
};

MODULE_DEVICE_TABLE(pci, g_pci_boot_table);

int pcie_boot_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
    g_pcie_rc_dev_info.rc_dev = dev;
    return 0;
}

void pcie_boot_dev_shutdown(struct pci_dev *dev)
{
    g_pcie_boot_shutdown = 1;
    PCIE_BOOT_TRACE("pcie boot shutdown start");
    if (g_pcie_rc_dev_info.rc_dev->irq) {
        disable_irq(g_pcie_rc_dev_info.rc_dev->irq);  // prevent further irq receive
    } else {
        PCIE_BOOT_TRACE("disable irq failed, invalid irq num %d", g_pcie_rc_dev_info.rc_dev->irq);
    }
    bsp_modem_load_shutdown_handle();
    while (!g_pcie_send_msg_done) { } // if msg ready to send, wait for last msg send out
    PCIE_BOOT_TRACE("pcie boot shutdown complete");
}

struct pci_driver g_pcie_boot_pci_driver = {
    .name = "pcie_boot",
    .id_table = g_pci_boot_table,
    .probe = pcie_boot_probe,
    .shutdown = pcie_boot_dev_shutdown,
    .remove = NULL,
    .err_handler = NULL,
    .driver = {
        .pm = NULL,
    },
};

int bsp_pcie_boot_enumerate(void)
{
    int ret = pcie_kport_enumerate(g_pcie_rc_dev_info.pcie_port_id);
    if (ret) {
        PCIE_BOOT_TRACE("pcie enumerate fail, ret %d", ret);
        return ret;
    }

    if (g_pcie_rc_dev_info.rc_dev == NULL) {
        PCIE_BOOT_TRACE("rc dev null");
        return -1;
    }
    ret = pci_enable_device(g_pcie_rc_dev_info.rc_dev);
    if (ret) {
        PCIE_BOOT_TRACE("pci_enable_device fail, ret %d", ret);
        return ret;
    }

    bsp_pcie_boot_cb_run(PCIE_BOOT_INIT_DONE);

    return 0;
}

unsigned long bsp_pcie_boot_get_bar_addr(enum pcie_boot_bar_id bar_index)
{
    unsigned long ep_bar_pci_addr;
    if (g_pcie_boot_uninstall_flag == 1) {
        PCIE_BOOT_TRACE("pcie boot driver uninstall, get bar addr failed");
        return 0;
    }
    if (bar_index >= PCIE_BAR_BOOT_NUM) {
        PCIE_BOOT_TRACE("bar index invalid");
        return 0;
    }
    ep_bar_pci_addr = pci_resource_start(g_pcie_rc_dev_info.rc_dev, bar_index);
    return ep_bar_pci_addr;
}

unsigned long bsp_pcie_boot_get_bar_size(enum pcie_boot_bar_id bar_index)
{
    unsigned long ep_bar_size;
    if (g_pcie_boot_uninstall_flag == 1) {
        PCIE_BOOT_TRACE("pcie boot driver uninstall, get bar size failed");
        return 0;
    }
    if (bar_index >= PCIE_BAR_BOOT_NUM) {
        PCIE_BOOT_TRACE("bar index invalid");
        return 0;
    }
    ep_bar_size = pci_resource_len(g_pcie_rc_dev_info.rc_dev, bar_index);
    return ep_bar_size;
}

int pcie_rc_get_msg_bar_addr(void)
{
    unsigned long ep_boot_msg_addr;
    unsigned long ep_boot_msg_size;

    /* get ep boot msg addr */
    ep_boot_msg_addr = bsp_pcie_boot_get_bar_addr(PCIE_BAR_BOOT_MSG);
    ep_boot_msg_size = bsp_pcie_boot_get_bar_size(PCIE_BAR_BOOT_MSG);
    if (!ep_boot_msg_addr || !ep_boot_msg_size) {
        PCIE_BOOT_TRACE("ep_boot_msg_resource_start fail, base addr :0x%lx, size:0x%lx",
               ep_boot_msg_addr, ep_boot_msg_size);
        return -1;
    }

    g_pcie_rc_dev_info.pcie_msi_msg_virt_addr = ioremap(ep_boot_msg_addr, ep_boot_msg_size);
    if (g_pcie_rc_dev_info.pcie_msi_msg_virt_addr == NULL) {
        PCIE_BOOT_TRACE("pcie_msi_msg_virt_addr ioremap fail!");
        return -1;
    }

    return 0;
}

void pcie_rc_put_msg_bar(void)
{
    if (g_pcie_rc_dev_info.pcie_msi_msg_virt_addr != NULL) {
        iounmap(g_pcie_rc_dev_info.pcie_msi_msg_virt_addr);
        g_pcie_rc_dev_info.pcie_msi_msg_virt_addr = NULL;
    }
}
#define DEBUG_MSG_LENGTH 204

void bsp_pcie_msg_space_debug(void)
{
    u32 i;
    u32 read_val;
    u32 cnt = 0;
    void *boot_msg_send_addr = NULL;
    void *boot_msg_receive_addr = NULL;
    char print_buf[DEBUG_MSG_LENGTH] = {0};

    if (g_pcie_rc_dev_info.pcie_msi_msg_virt_addr == NULL) {
        return;
    }

    boot_msg_send_addr = g_pcie_rc_dev_info.pcie_msi_msg_virt_addr + PCIE_TRANS_RC_MSG_OFFSET;
    PCIE_BOOT_TRACE("bar space last send msg print: ");
    for (i = 0; i < DEBUG_MSG_LENGTH / sizeof(u32); i++) {
        read_val = readl(boot_msg_send_addr + i * sizeof(u32));
        cnt += snprintf_s((char *)print_buf + cnt,
            (size_t)(DEBUG_MSG_LENGTH - cnt), (size_t)(DEBUG_MSG_LENGTH - cnt - 1), "%x ", read_val);
    }
    PCIE_BOOT_TRACE("%s", print_buf);
    cnt = 0;
    boot_msg_receive_addr = g_pcie_rc_dev_info.pcie_msi_msg_virt_addr;

    PCIE_BOOT_TRACE("bar space last receive msg print: ");
    for (i = 0; i < DEBUG_MSG_LENGTH / sizeof(u32); i++) {
        read_val = readl(boot_msg_receive_addr + i * sizeof(u32));
        cnt += snprintf_s((char *)print_buf + cnt, (size_t)(DEBUG_MSG_LENGTH - cnt),
            (size_t)(DEBUG_MSG_LENGTH - cnt - 1), "%x ", read_val);
    }

    PCIE_BOOT_TRACE("%s", print_buf);

    PCIE_BOOT_TRACE("bar space msg trace record: ");

    for (i = 0; i < MAX_RECORD_BUFFER; i++) {
        PCIE_BOOT_TRACE("user_id %d, msg_action %d, time_slice 0x%x",
               g_pcie_boot_msg_record.pcie_boot_msg_act[i].user_id,
               g_pcie_boot_msg_record.pcie_boot_msg_act[i].msg_act,
               g_pcie_boot_msg_record.pcie_boot_msg_act[i].time_stamp);
    }

    return;
}
#define PCIE_MSG_DATA_OFFSET 0x8
/*
 * bsp_pcie_rc_send_msg - rc send message to ep after
 * send sequence:  cmd_data, cmd_len, msi_id
 */
int bsp_pcie_rc_send_msg(struct pcie_trans_msg_info *msg_info)
{
    u32 i = 0;
    u32 msg_val;
    void *boot_msg_addr = NULL;
    u32 send_feedback;
    u32 user_id = msg_info->msi_id;

    if (g_pcie_boot_shutdown) {
        PCIE_BOOT_TRACE("shutdown start, send msg to ep direct return!");
        return 0;
    }

    g_pcie_send_msg_done = 0;

    pcie_boot_msg_dbg(user_id, MSG_SEND);

    boot_msg_addr = g_pcie_rc_dev_info.pcie_msi_msg_virt_addr + PCIE_TRANS_RC_MSG_OFFSET;

    if ((msg_info->cmd_len) > (PCIE_TRANS_RC_MSG_SIZE - PCIE_MSG_DATA_OFFSET) || msg_info->cmd_len == 0) {
        PCIE_BOOT_TRACE("send msg length 0x%x invalid!", msg_info->cmd_len);
        return -1;
    }
    if (!msg_info->cmd_data_addr) {
        PCIE_BOOT_TRACE("msg_info cmd data addr invalid!");
        return -1;
    }

    for (; i < (msg_info->cmd_len / sizeof(u32)); i++) {
        msg_val = readl(msg_info->cmd_data_addr + i * sizeof(u32));
        writel(msg_val, (boot_msg_addr + PCIE_MSG_DATA_OFFSET + i * sizeof(u32)));
    }
    send_feedback = readl(boot_msg_addr + PCIE_MSG_DATA_OFFSET + msg_info->cmd_len - sizeof(u32));
    if (send_feedback != msg_val) {
        PCIE_BOOT_TRACE("msg val mismatch, msg_val 0x%x, readback 0x%x", msg_val, send_feedback);
    }

    writel(msg_info->cmd_len, boot_msg_addr + sizeof(u32));
    send_feedback = readl(boot_msg_addr + sizeof(u32));
    if (send_feedback != msg_info->cmd_len) {
        PCIE_BOOT_TRACE("cmd_len mismatch, cmd_len 0x%x, readback 0x%x",
            msg_info->cmd_len, send_feedback);
    }

    writel(user_id, boot_msg_addr);
    send_feedback = readl(boot_msg_addr);
    if (send_feedback != user_id) {
        PCIE_BOOT_TRACE("user_id mismatch, user_id 0x%x, readback 0x%x", user_id, send_feedback);
    }

    g_pcie_send_msg_done = 1;

    return 0;
}

/*
 * bsp_pcie_rc_read_msg - rc read message from ep message
 */
int bsp_pcie_rc_read_msg(struct pcie_trans_msg_info *msg_info)
{
    void *boot_msg_addr = NULL;
    u32 i = 0;
    u32 msg_val;

    boot_msg_addr = g_pcie_rc_dev_info.pcie_msi_msg_virt_addr;
    msg_info->msi_id = readl(boot_msg_addr);

    boot_msg_addr += sizeof(u32);
    msg_info->cmd_len = readl(boot_msg_addr);

    if ((msg_info->cmd_len) > (PCIE_TRANS_EP_MSG_SIZE - PCIE_MSG_DATA_OFFSET)) {
        PCIE_BOOT_TRACE("receive msg length invalid!");
        return -1;
    }
    if (!msg_info->cmd_data_addr) {
        PCIE_BOOT_TRACE("msg_info cmd data addr invalid!");
        return -1;
    }

    boot_msg_addr += 0x4;
    for (; i < (msg_info->cmd_len / sizeof(u32)); i++) {
        msg_val = readl(boot_msg_addr + i * sizeof(u32));
        writel(msg_val, (msg_info->cmd_data_addr + i * sizeof(u32)));
    }

    pcie_boot_msg_dbg(msg_info->msi_id, MSG_READ);

    return 0;
}

void bsp_pcie_rc_read_clear(void)
{
    u32 i;
    struct pcie_trans_msg_info *ep_msg_info =
        (struct pcie_trans_msg_info *)(g_pcie_rc_dev_info.pcie_msi_msg_virt_addr + PCIE_TRANS_EP_MSG_OFFSET);
    u32 read_back;

    /* 读完清消息 */
    u32 ep_cmd_len = ep_msg_info->cmd_len;

    pcie_boot_msg_dbg(0, MSG_CLEAR);

    ep_cmd_len = (ep_cmd_len > (PCIE_TRANS_EP_MSG_SIZE - PCIE_MSG_DATA_OFFSET)) ?
                  (PCIE_TRANS_EP_MSG_SIZE - PCIE_MSG_DATA_OFFSET) : ep_cmd_len;

    for (i = 0; i < ((ep_cmd_len + PCIE_MSG_DATA_OFFSET) / sizeof(u32)); i++) {
        writel(0, g_pcie_rc_dev_info.pcie_msi_msg_virt_addr + PCIE_TRANS_EP_MSG_OFFSET + i * sizeof(u32));
        read_back = readl(g_pcie_rc_dev_info.pcie_msi_msg_virt_addr + PCIE_TRANS_EP_MSG_OFFSET + i * sizeof(u32));
        if (read_back != 0) {
            PCIE_BOOT_TRACE("rc read clear read back mismatch, read_back 0x%x", read_back);
        }
    }

    return;
}

static irqreturn_t pcie_boot_msi_int_isr(int irq, void *dev_info)
{
    unsigned long irq_flags = 0;
    struct pcie_rc_dev_info *info = &g_pcie_rc_dev_info;
    u32 msi_id;

    spin_lock_irqsave(&info->spinlock, irq_flags);  // 在解析当前msg结构时，禁止新的中断

    msi_id = readl(info->pcie_msi_msg_virt_addr);
    if (msi_id == PCIE_MSI_ID_IDLE || msi_id >= PCIE_MSI_ID_NUM) {
        PCIE_BOOT_TRACE("receive invalid msi id, msi_id = %d", msi_id);
        goto out;
    }

    pcie_boot_msg_dbg(msi_id, MSI_ARRIVE);

    if (info->irq_chn[msi_id].handler) {
        info->irq_chn[msi_id].handler(msi_id, info->irq_chn[msi_id].irq_data);
    } else {
        bsp_pcie_rc_read_clear();
        PCIE_BOOT_TRACE("msi_id %d handler not valid", msi_id);
        goto out;
    }

out:
    spin_unlock_irqrestore(&info->spinlock, irq_flags);
    return IRQ_HANDLED;
}

int pcie_boot_msi_init(void)
{
    int ret;

    ret = pci_enable_msi(g_pcie_rc_dev_info.rc_dev);
    if (ret != 0) {
        PCIE_BOOT_TRACE("pci_enable_msi fail, ret = %d", ret);
        return -1;
    }

    if (request_irq(g_pcie_rc_dev_info.rc_dev->irq, pcie_boot_msi_int_isr, IRQF_SHARED, "PCIe Boot MSI",
        (void *)g_pcie_rc_dev_info.rc_dev)) {
        PCIE_BOOT_TRACE("request_irq fail, msi irq = %d", g_pcie_rc_dev_info.rc_dev->irq);
        return -1;
    }

    return 0;
}

void pcie_boot_msi_exit(void)
{
    free_irq(g_pcie_rc_dev_info.rc_dev->irq, g_pcie_rc_dev_info.rc_dev);
    pci_disable_msi(g_pcie_rc_dev_info.rc_dev);
}

int bsp_pcie_receive_msi_request(enum pcie_msi_cmd_id id, irq_handler_t handler, const char *name, void *data)
{
    struct pcie_rc_dev_info *info = &g_pcie_rc_dev_info;

    if (id >= PCIE_MSI_ID_NUM || handler == NULL) {
        PCIE_BOOT_TRACE("invalid input, checkinput parameter");
        return -1;
    }

    if (info->irq_chn[id].handler) {
        PCIE_BOOT_TRACE("pcie msi cmd id [%d] has handler", id);
        return -1;
    }

    info->irq_chn[id].handler = handler;
    info->irq_chn[id].irq_data = data;
    info->irq_chn[id].name = name;

    return 0;
}

void bsp_pcie_boot_msi_free(enum pcie_msi_cmd_id id)
{
    g_pcie_rc_dev_info.irq_chn[id].handler = NULL;
    g_pcie_rc_dev_info.irq_chn[id].irq_data = NULL;
    g_pcie_rc_dev_info.irq_chn[id].name = NULL;
    return;
}

void bsp_pcie_boot_notify(void)
{
    pci_set_master(g_pcie_rc_dev_info.rc_dev);
}

/*
 * Attention: activities below must execute in sequence
 *   (1)pci_unregister_driver
 *   (2)pcie_remove
 *   (3)pcie_rescan
 */
int bsp_pcie_boot_driver_uninstall(void)
{
    int ret = 0;
    bsp_pcie_boot_cb_run(PCIE_BOOT_EXIT);
    if (g_pcie_boot_uninstall_flag == 1) {
        PCIE_BOOT_TRACE("pcie boot driver has been uninstalled before, direct return");
        return ret;
    }
    g_pcie_boot_uninstall_flag = 1;

    pci_unregister_driver(&g_pcie_boot_pci_driver);  // uninstall pcie boot driver

    ret = pcie_kport_remove_ep(g_pcie_rc_dev_info.pcie_port_id);
    if (ret) {
        PCIE_BOOT_TRACE("pcie_kport_remove_ep failed, ret = 0x%x", ret);
        return ret;
    }
    ret = pcie_kport_pm_control(0x0, g_pcie_rc_dev_info.pcie_port_id); // power off previous pcie driver
    if (ret) {
        PCIE_BOOT_TRACE("[AP_MODEM_BOOT]fail to power off pcie, ret %d. ", ret);
        return ret;
    }

    ret = pcie_kport_pm_control(0x3, g_pcie_rc_dev_info.pcie_port_id); // provide clk for kernel stage rescan
    if (ret) {
        PCIE_BOOT_TRACE("[AP_MODEM_BOOT]fail to power on ap pcie and provide external clock, ret %d. ", ret);
        return ret;
    }

    return ret;
}

int bsp_pcie_boot_rescan(void)
{
    int ret = pcie_kport_rescan_ep(g_pcie_rc_dev_info.pcie_port_id);
    if (ret) {
        PCIE_BOOT_TRACE("pcie_kport_rescan_ep failed, ret = 0x%x", ret);
        return ret;
    }

    return 0;
}

int get_boot_pcie_dts_cfg(void)
{
    u32 ret;
    struct device_node *dev_node = NULL;
    dev_node = of_find_compatible_node(NULL, NULL, "iot,pcie_iot");
    if (dev_node == NULL) {
        PCIE_BOOT_TRACE("pcie device node not found!");
        return -1;
    }

    ret = (u32)of_property_read_u32(dev_node, "pcie_port_id", &g_pcie_rc_dev_info.pcie_port_id);
    ret |= (u32)of_property_read_u32_index(dev_node, "pcie_id_attr", 0, &g_pcie_rc_dev_info.vendor_id);
    ret |= (u32)of_property_read_u32_index(dev_node, "pcie_id_attr", 1, &g_pcie_rc_dev_info.device_id);
    if (ret) {
        PCIE_BOOT_TRACE("fail to get pcie dts info, ret = %u", ret);
        return (int)ret;
    }

    return 0;
}

int bsp_pcie_boot_init(void)
{
    int ret;
    g_pcie_boot_uninstall_flag = 0;
    g_pcie_rc_dev_info.vendor_id = PCI_VENDOR_ID_IOT;
    g_pcie_rc_dev_info.device_id = PCI_DEVICE_ID_IOT;

    spin_lock_init(&g_pcie_rc_dev_info.spinlock);

    ret = get_boot_pcie_dts_cfg();
    if (ret) {
        PCIE_BOOT_TRACE("[PCIE_BOOT]fail to get pcie dts, ret = %d", ret);
        return ret;
    }
    ret = pci_register_driver(&g_pcie_boot_pci_driver);
    if (ret) {
        PCIE_BOOT_TRACE("[PCIE_BOOT]fail to register pcie boot driver, ret = %d", ret);
        return ret;
    }

    return ret;
}

int bsp_pcie_resource_init(void)
{
    int ret = pcie_rc_get_msg_bar_addr();
    if (ret) {
        PCIE_BOOT_TRACE(" fail to get pcie msg bar addr info");
        return -1;
    }

    ret = pcie_boot_msi_init();
    if (ret) {
        PCIE_BOOT_TRACE(" fail to init pcie boot msi");
        return -1;
    }

    return 0;
}

void bsp_pcie_resource_exit(void)
{
    if (g_pcie_boot_uninstall_flag == 0x1) {
        PCIE_BOOT_TRACE(" pcie driver has been uninstalled, no need to exit resource");
        return;
    }
    pcie_boot_msi_exit();
    pcie_rc_put_msg_bar();
}
