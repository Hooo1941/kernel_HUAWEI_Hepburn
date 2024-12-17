/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
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
 * Description: PCIE IOT Host Normalized Driver
 * Note: User can implant this driver and compile for Host PCIE Driver
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/msi.h>
#include <linux/irq.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/of_platform.h>
#include <asm/cacheflush.h>
#include <linux/reboot.h>
#include <linux/pci.h>
#include <linux/pci_regs.h>

#include "pcie_iot_dev.h"

#define PCIE_IOT_VENDOR_ID 0x19E5
#define PCIE_IOT_DEVICE_ID 0x5010

/* Use IOT ID for Example, user must replace with HOST RC ID */
#define PCIE_RC_DEV_VENDOR_ID PCIE_IOT_VENDOR_ID
#define PCIE_RC_DEV_DEVICE_ID PCIE_IOT_DEVICE_ID

#define PCIE_DEV_TRACE(fmt, args...) do { \
    printk(KERN_ERR "[PCIE_DEV]%s:" fmt "\n", __FUNCTION__, ##args); \
} while (0)

struct rc_pcie_flag g_pcie_rc_flag;
struct rc_pcie_info *g_pcie_rc_info = NULL;
struct pcie_callback_info_list g_pcie_rc_user_cb;

struct pcie_rc_debug_info g_pcie_rc_dbg_info[PCIE_RC_EVENT_MAX] = {
    {"PCIE_RC_GET_EP_DEV",                                0},
    {"PCIE_RC_GET_FUNC_NUM",                              0},
    {"PCIE_RC_GET_BAR",                                   0},
    {"PCIE_RC_INT_INIT",                                  0},
    {"PCIE_RC_USER_INIT",                                 0},
};

int bsp_pcie_rc_cb_register(enum pcie_user_id usr_id, struct pcie_callback_info *info)
{
    if (info == NULL) {
        PCIE_DEV_TRACE("param is null");
        return -1;
    }

    if (g_pcie_rc_user_cb.list[usr_id].callback) {
        PCIE_DEV_TRACE("input usr_id callback has been registered");
        return -1;
    }

    if (usr_id >= PCIE_USER_NUM) {
        PCIE_DEV_TRACE("invalid input usr_id");
        return -1;
    }

    g_pcie_rc_user_cb.list[usr_id].callback = info->callback;
    g_pcie_rc_user_cb.list[usr_id].callback_args = info->callback_args;

    return 0;
}

int bsp_pcie_rc_cb_run(enum pcie_rc_callback_id callback_stage)
{
    int ret;
    u32 total_ret = 0;
    int user_id;

    for (user_id = 0; user_id < PCIE_USER_NUM; user_id++) {
        if (g_pcie_rc_user_cb.list[user_id].callback) {
            ret = g_pcie_rc_user_cb.list[user_id].callback(user_id, callback_stage,
                g_pcie_rc_user_cb.list[user_id].callback_args);
            if (ret) {
                PCIE_DEV_TRACE("rc pcie user %d cb run err, ret %d", user_id, ret);
            }
            total_ret |= (u32)ret;
            if (callback_stage == PCIE_RC_CB_ENUM_DONE) {
                g_pcie_rc_dbg_info[PCIE_RC_USER_INIT].init_flag |= (0x1 << user_id);
            }
        }
    }
    return (int)total_ret;
}

int pcie_dev_enable_ltr(struct pci_dev *dev)
{
    int ret;
    u32 value = 0;

    ret = pcie_capability_read_dword(dev, PCI_EXP_DEVCAP2, &value);
    if (ret) {
        PCIE_DEV_TRACE("pcie_capability_read_dword fail, ret = %d", ret);
        return ret;
    }
    if (value & PCI_EXP_DEVCAP2_LTR) {
        ret = pcie_capability_read_dword(dev, PCI_EXP_DEVCTL2, &value);
        if (ret) {
            PCIE_DEV_TRACE("pcie_capability_read_dword fail, ret = %d", ret);
            return ret;
        }
        value |= PCI_EXP_DEVCTL2_LTR_EN;
        ret = pcie_capability_write_dword(dev, PCI_EXP_DEVCTL2, value);
        if (ret) {
            PCIE_DEV_TRACE("pcie_capability_write_dword fail, ret = %d", ret);
            return ret;
        }
    } else {
        PCIE_DEV_TRACE("LTR Mechanism not support");
        return -1;
    }

    return 0;
}

struct pci_dev *pcie_get_rc_dev(u32 vendor_id, u32 device_id)
{
    u32 pdev_class;
    struct pci_dev *pdev = NULL;

    do {
        pdev = pci_get_device(vendor_id, device_id, pdev);
        if (pdev == NULL) {
            break;
        }
        pdev_class = (pdev->class) >> 0x8;
        if (pdev_class == PCI_CLASS_BRIDGE_PCI) {
            break;
        }
    } while (pdev != NULL);

    return pdev;
}

unsigned long bsp_pcie_rc_get_bar_addr(enum pcie_bar_id_e bar_index)
{
    unsigned long ep_bar_pci_addr;
    u32 func_num = bar_index / PCIE_BAR_NUM;
    u32 in_func_bar_id = bar_index % PCIE_BAR_NUM;

    if (bar_index >= PCIE_BAR_MAX_NUM) {
        PCIE_DEV_TRACE("invalid bar id input %d, no more than %d!", bar_index, PCIE_BAR_MAX_NUM);
        return 0;
    }
    if (g_pcie_rc_info->iot_multi_ep[func_num]) {
        ep_bar_pci_addr = pci_resource_start(g_pcie_rc_info->iot_multi_ep[func_num], in_func_bar_id);
        return ep_bar_pci_addr;
    } else {
        PCIE_DEV_TRACE("no ep device found, bar_index : %d, func_num: %d, in_func_bar_id: %d!", bar_index, func_num,
                       in_func_bar_id);
        return 0;
    }
}

unsigned long bsp_pcie_rc_get_bar_size(enum pcie_bar_id_e bar_index)
{
    unsigned long ep_bar_size;
    u32 func_num = bar_index / PCIE_BAR_NUM;
    u32 in_func_bar_id = bar_index % PCIE_BAR_NUM;

    if (bar_index >= PCIE_BAR_MAX_NUM) {
        PCIE_DEV_TRACE("invalid bar id input %d, no more than %d!", bar_index, PCIE_BAR_MAX_NUM);
        return 0;
    }

    if (g_pcie_rc_info->iot_multi_ep[func_num]) {
        ep_bar_size = pci_resource_len(g_pcie_rc_info->iot_multi_ep[func_num], in_func_bar_id);
        return ep_bar_size;
    } else {
        PCIE_DEV_TRACE("no ep device found, bar_index : %d, func_num: %d, in_func_bar_id: %d!", bar_index, func_num,
                       in_func_bar_id);
        return 0;
    }
}

void *rc_get_normal_bar_virt_addr(enum pcie_bar_id_e bar_index)
{
    unsigned long bar_pci_addr;
    unsigned long bar_size;
    void *bar_virt_addr = NULL;

    bar_pci_addr = bsp_pcie_rc_get_bar_addr(bar_index);
    bar_size = bsp_pcie_rc_get_bar_size(bar_index);
    if (bar_pci_addr == 0 || bar_size == 0) {
        PCIE_DEV_TRACE("bar index %d , get bar resource fail", bar_index);
        return NULL;
    }

    bar_virt_addr = ioremap_wc(bar_pci_addr, bar_size);
    if (bar_virt_addr == NULL) {
        PCIE_DEV_TRACE("bar index %d, fail to ioremap virt addr", bar_index);
        return NULL;
    }

    return bar_virt_addr;
}

void *rc_get_reg_bar_virt_addr(enum pcie_bar_id_e bar_index)
{
    unsigned long bar_pci_addr;
    unsigned long bar_size;
    void *bar_virt_addr = NULL;

    bar_pci_addr = bsp_pcie_rc_get_bar_addr(bar_index);
    bar_size = bsp_pcie_rc_get_bar_size(bar_index);
    if (!bar_pci_addr || !bar_size) {
        PCIE_DEV_TRACE("bar index %d , get bar resource fail", bar_index);
        return NULL;
    }

    bar_virt_addr = ioremap(bar_pci_addr, bar_size);
    if (bar_virt_addr == NULL) {
        PCIE_DEV_TRACE("bar index %d, fail to ioremap virt addr", bar_index);
        return NULL;
    }

    return bar_virt_addr;
}

int pcie_rc_get_base_addr(void)
{
    g_pcie_rc_info->ep_msi_usr_virt_addr = rc_get_normal_bar_virt_addr(PCIE_BAR_MSI_TO_RC);
    if (g_pcie_rc_info->ep_msi_usr_virt_addr == NULL) {
        PCIE_DEV_TRACE("get ep to rc msi bar resource fail");
        return -1;
    }

    g_pcie_rc_info->ep_in_rc_msi_virt_addr = rc_get_reg_bar_virt_addr(PCIE_BAR_GIC_MSI);
    if (g_pcie_rc_info->ep_in_rc_msi_virt_addr == NULL) {
        PCIE_DEV_TRACE("get ep gic msi bar resource fail");
        return -1;
    }

    g_pcie_rc_info->ep_intx_virt_addr = rc_get_reg_bar_virt_addr(PCIE_BAR_INTX);
    if (g_pcie_rc_info->ep_intx_virt_addr == NULL) {
        PCIE_DEV_TRACE("get ep intx bar resource fail");
        return -1;
    }

    return 0;
}

void pcie_release_rc_base_addr(void)
{
    if (g_pcie_rc_info->ep_msi_usr_virt_addr != NULL) {
        iounmap(g_pcie_rc_info->ep_msi_usr_virt_addr);
    }
    if (g_pcie_rc_info->ep_in_rc_msi_virt_addr != NULL) {
        iounmap(g_pcie_rc_info->ep_in_rc_msi_virt_addr);
    }
    if (g_pcie_rc_info->ep_intx_virt_addr != NULL) {
        iounmap(g_pcie_rc_info->ep_intx_virt_addr);
    }
    PCIE_DEV_TRACE("pcie_release_rc_base_addr done");
    return;
}

static void iot_pcie_rc_msi_enable(enum pcie_ep_to_rc_msi_id index)
{
    unsigned int reg_offset = index * sizeof(u32) + EP_TO_RC_MSI_INT_ENABLE_OFFSET;

    /* irq enable */
    writel(1U << index, g_pcie_rc_info->ep_msi_usr_virt_addr + reg_offset);
}

static void iot_pcie_rc_msi_disable(unsigned int index)
{
    unsigned int reg_offset = index * sizeof(u32) + EP_TO_RC_MSI_INT_ENABLE_OFFSET;

    /* irq disable */
    writel(0, g_pcie_rc_info->ep_msi_usr_virt_addr + reg_offset);
}

/*
 * bsp_pcie_rc_msi_request - request msi and link with user handler
 * This function is used to link user defined handler with ep_to_rc_msi_id,
 * when ep_to_rc msi "id" detected, run user defined handler with input data.
 */
int bsp_pcie_rc_msi_request(enum pcie_ep_to_rc_msi_id id, irq_handler_t handler, const char *name, void *data)
{
    if (id >= PCIE_EP_MSI_MAX_ID || handler == NULL) {
        PCIE_DEV_TRACE("invalid input, checkinput parameter");
        return -1;
    }

    if (g_pcie_rc_info->irq_chn[id].handler) {
        PCIE_DEV_TRACE("pcie gic msi[%d] has handler", id);
        return -1;
    }

    g_pcie_rc_info->irq_chn[id].handler = handler;
    g_pcie_rc_info->irq_chn[id].irq_data = data;
    g_pcie_rc_info->irq_chn[id].name = name;

    /* irq enable */
    iot_pcie_rc_msi_enable(id);
    return 0;
}

int bsp_pcie_rc_msi_free(enum pcie_ep_to_rc_msi_id id)
{
    if (id >= PCIE_EP_MSI_MAX_ID) {
        PCIE_DEV_TRACE("invalid input, checkinput parameter");
        return -1;
    }

    g_pcie_rc_info->irq_chn[id].handler = NULL;
    g_pcie_rc_info->irq_chn[id].irq_data = NULL;
    g_pcie_rc_info->irq_chn[id].name = NULL;

    return 0;
}

int bsp_pcie_rc_msi_enable(enum pcie_ep_to_rc_msi_id id)
{
    if (id >= PCIE_EP_MSI_MAX_ID) {
        PCIE_DEV_TRACE("invalid id, no more than %d", PCIE_EP_MSI_MAX_ID);
        return -1;
    }
    iot_pcie_rc_msi_enable(id);
    return 0;
}

int bsp_pcie_rc_msi_disable(enum pcie_ep_to_rc_msi_id id)
{
    if (id >= PCIE_EP_MSI_MAX_ID) {
        PCIE_DEV_TRACE("invalid id, no more than %d", PCIE_EP_MSI_MAX_ID);
        return -1;
    }
    iot_pcie_rc_msi_disable(id);
    return 0;
}

void pcie_bar_space_error_handle(struct work_struct *work)
{
    PCIE_DEV_TRACE("bar space read error, error bar space content: 0x%x, call system error",
                   g_pcie_rc_flag.error_bar_content);
}

static irqreturn_t pcie_rc_msi_int_isr(int irq, void *dev_info)
{
    u32 i;
    u32 reg_stat_value;
    struct rc_pcie_info *info = g_pcie_rc_info;
    irq_handler_t user_handler;

    if (g_pcie_rc_flag.remove_start || g_pcie_rc_flag.linkdown_detect) {
        return IRQ_HANDLED;
    }

    reg_stat_value = readl_relaxed(info->ep_intx_virt_addr + PCIE_INTX_REG_OFFSET);
    reg_stat_value &= ~(1U << PCIE_INTX_REG_BIT);
    writel_relaxed(reg_stat_value, info->ep_intx_virt_addr + PCIE_INTX_REG_OFFSET);
    wmb();

    for (i = 0; i < PCIE_EP_MSI_NUM; i++) {
        user_handler = info->irq_chn[i].handler;
        if (user_handler == NULL) {
            continue;
        }
        reg_stat_value = readl(info->ep_msi_usr_virt_addr + i * sizeof(u32));
        if (!reg_stat_value) {
            continue;
        }
        if (reg_stat_value != (0x1 << i)) {
            g_pcie_rc_flag.error_bar_content = reg_stat_value;
            schedule_work(&g_pcie_rc_info->bar_space_error_work);
            return IRQ_HANDLED;
        }
        info->irq_chn[i].irq_count++;
        writel(0, info->ep_msi_usr_virt_addr + i * sizeof(u32));
        user_handler(i, info->irq_chn[i].irq_data);
        info->irq_chn[i].irq_handler_count++;
    }
    return IRQ_HANDLED;
}

/*
 * bsp_rc_pcie_send_msi - rc msi user send irq to ep
 * This function is used to send irq to ep by gic_msi, the msi irq number is
 * managed by enum pcie_rc_to_ep_msi_id
 */
int bsp_pcie_rc_send_msi(enum pcie_rc_to_ep_msi_id id)
{
    unsigned long irq_flags = 0;

    if (g_pcie_rc_flag.remove_start || g_pcie_rc_flag.linkdown_detect) {
        return -1;
    }

    if (id >= PCIE_RC_MSI_MAX_ID) {
        PCIE_DEV_TRACE("invalid msi id, no more than %d", PCIE_RC_MSI_MAX_ID);
        return -1;
    }

    spin_lock_irqsave(&g_pcie_rc_info->spinlock, irq_flags);
    writel(id, g_pcie_rc_info->ep_in_rc_msi_virt_addr);  // protect for the same location write
    spin_unlock_irqrestore(&g_pcie_rc_info->spinlock, irq_flags);
    g_pcie_rc_info->rc_send_msi_count[id]++;
    return 0;
}

static const struct pci_device_id g_pcie_iot_dev_table[] = {
    { PCI_DEVICE(PCIE_IOT_VENDOR_ID, PCIE_IOT_DEVICE_ID) },
    {0},
};

MODULE_DEVICE_TABLE(pci, g_pcie_iot_dev_table);

int pcie_iot_dev_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
    g_pcie_rc_info->iot_multi_ep[dev->devfn] = dev;
    g_pcie_rc_info->valid_ep_func_num++;
    return 0;
}

struct pci_driver g_pcie_iot_dev_driver = {
    .name = "pcie_iot_dev",
    .id_table = g_pcie_iot_dev_table,
    .probe = pcie_iot_dev_probe,
    .shutdown = NULL,
    .remove = NULL,
    .err_handler = NULL,
};

int pcie_rc_device_init(void)
{
    int i;
    /* get multifunction ep dev */
    int ret = pci_register_driver(&g_pcie_iot_dev_driver);
    if (ret) {
        PCIE_DEV_TRACE("register g_pcie_iot_dev_driver err, ret = %d", ret);
        goto err_ret;
    }
    g_pcie_rc_dbg_info[PCIE_RC_GET_EP_DEV].init_flag = 0x1;
    g_pcie_rc_dbg_info[PCIE_RC_GET_FUNC_NUM].init_flag = g_pcie_rc_info->valid_ep_func_num;

    /* scan pci bridge for pcie rc dev */
    g_pcie_rc_info->iot_rc = pcie_get_rc_dev(PCIE_RC_DEV_VENDOR_ID, PCIE_RC_DEV_DEVICE_ID);

    /* use func0 dev as main function */
    g_pcie_rc_info->iot_ep = g_pcie_rc_info->iot_multi_ep[0];

    if (g_pcie_rc_info->iot_rc == NULL || g_pcie_rc_info->iot_ep == NULL) {
        PCIE_DEV_TRACE("can't find iot pcie device");
        goto err_ret;
    }

    /* enable all functions */
    for (i = 0; i < g_pcie_rc_info->valid_ep_func_num; i++) {
        if (pci_enable_device(g_pcie_rc_info->iot_multi_ep[i])) {
            PCIE_DEV_TRACE("pci_enable_device fail, func_num: %d", i);
            goto err_ret1;
        }
        pci_set_master(g_pcie_rc_info->iot_multi_ep[i]);
    }

    /* LTR enable */
    ret = pcie_dev_enable_ltr(g_pcie_rc_info->iot_rc);
    if (ret) {
        PCIE_DEV_TRACE("enable pcie rc ltr failed, PCIE PM L1 Substate not available");
    }
    ret = pcie_dev_enable_ltr(g_pcie_rc_info->iot_ep);
    if (ret) {
        PCIE_DEV_TRACE("enable pcie ep ltr failed, PCIE PM L1 Substate not available");
    }

    if (pcie_rc_get_base_addr()) {
        PCIE_DEV_TRACE("fail to get rc base address");
        goto err_ret2;
    }
    g_pcie_rc_dbg_info[PCIE_RC_GET_BAR].init_flag = 0x1;
    return 0;

err_ret2:
    for (i = 0; i < g_pcie_rc_info->valid_ep_func_num; i++) {
        pci_disable_device(g_pcie_rc_info->iot_multi_ep[i]);
    }
err_ret1:
    for (i = 0; i < g_pcie_rc_info->valid_ep_func_num; i++) {
        g_pcie_rc_info->iot_multi_ep[i] = NULL;
    }
    pci_unregister_driver(&g_pcie_iot_dev_driver);
err_ret:
    return -1;
}

int pcie_rc_int_init(void)
{
    int ret;
    ret = request_irq(g_pcie_rc_info->iot_ep->irq, pcie_rc_msi_int_isr,
        IRQF_SHARED | IRQF_NO_SUSPEND, "PCIe KERNEL INTx", (void *)g_pcie_rc_info->iot_ep);
    if (ret) {
        PCIE_DEV_TRACE("request_irq fail, msi irq = %d, ret = %d", g_pcie_rc_info->iot_ep->irq, ret);
        return -1;
    }
    g_pcie_rc_dbg_info[PCIE_RC_INT_INIT].init_flag = 0x1;
    return 0;
}

void pcie_rc_flag_init(void)
{
    int i;
    for (i = 0; i < PCIE_RC_EVENT_MAX; i++) {
        g_pcie_rc_dbg_info[i].init_flag = 0;
    }
    g_pcie_rc_flag.linkdown_detect = 0x0;
    g_pcie_rc_flag.remove_start = 0x0;
    g_pcie_rc_flag.error_bar_content = 0x0;
}

int pcie_rc_software_init(void)
{
    g_pcie_rc_info = (struct rc_pcie_info *)kzalloc(sizeof(struct rc_pcie_info), GFP_KERNEL);
    if (g_pcie_rc_info == NULL) {
        PCIE_DEV_TRACE("fail to kzalloc, size: 0x%lX", (unsigned long)sizeof(struct rc_pcie_info));
        return -1;
    }

    INIT_WORK(&g_pcie_rc_info->bar_space_error_work, pcie_bar_space_error_handle);
    g_pcie_rc_info->wake_lock = wakeup_source_register(NULL, "pcie_iot_wakelock");
    if (g_pcie_rc_info->wake_lock == NULL) {
         PCIE_DEV_TRACE("g_pcie_rc_info->wake_lock register not succ");
    }
    __pm_stay_awake(g_pcie_rc_info->wake_lock);
    spin_lock_init(&g_pcie_rc_info->spinlock);

    pcie_rc_flag_init();
    return 0;
}

void pcie_rc_linkdown_handler(void)
{
    PCIE_DEV_TRACE("iot pcie linkdown detected");
    g_pcie_rc_flag.linkdown_detect = 0x1;
    (void)bsp_pcie_rc_cb_run(PCIE_RC_CB_LINKDOWN);
    return;
}

void pcie_rc_register_linkdown_event(void)
{
    // use soc linkdown register pcie_rc_linkdown_handler function
    return;
}

void pcie_iot_dev_shutdown(void)
{
    PCIE_DEV_TRACE("pcie iot dev shutdown +");
    disable_irq(g_pcie_rc_info->iot_ep->irq);
    g_pcie_rc_flag.remove_start = 0x1;  // basic protect to stop new pcie visit
    (void)bsp_pcie_rc_cb_run(PCIE_RC_CB_EXIT);  // first,  stop business
    PCIE_DEV_TRACE("pcie iot dev shutdown -");
    return;
}

int pcie_rc_halt(struct notifier_block *nb, unsigned long event, void *buf)
{
    PCIE_DEV_TRACE("notifier shutdown in, event = 0x%lx", event);
    switch (event) {
        case SYS_RESTART:
        case SYS_POWER_OFF:
        case SYS_HALT:
            pcie_iot_dev_shutdown();
            break;
        default:
            return 0;
    }
    PCIE_DEV_TRACE("notifier shutdown out");
    return 0;
}

static struct notifier_block g_pcie_shutdown_notifier = {
    .notifier_call = pcie_rc_halt,
};

int bsp_pcie_rc_notify_ep_user_init(void)
{
    return bsp_pcie_rc_send_msi(PCIE_RC_MSI_USER_INIT);
}

int bsp_pcie_rc_normalize_init(void)
{
    int ret;

    ret = pcie_rc_software_init();
    if (ret) {
        PCIE_DEV_TRACE("pcie_rc_software_init init failed, ret %d", ret);
        return -1;
    }

    PCIE_DEV_TRACE("pcie_rc_software_init done");

    if (pcie_rc_device_init()) {
        PCIE_DEV_TRACE("fail to get iot pcie device info");
        goto err_free;
    }

    PCIE_DEV_TRACE("pcie_rc_device_init done");

    if (pcie_rc_int_init()) {
        PCIE_DEV_TRACE("fail to rc msi init");
        pcie_release_rc_base_addr();
        goto err_free;
    }

    ret = bsp_pcie_rc_cb_run(PCIE_RC_CB_ENUM_DONE);
    if (ret) {
        PCIE_DEV_TRACE("pcie rc fail to callback, ret = %d", ret);
        goto err_free;
    }

    PCIE_DEV_TRACE("pcie rc user init done");

    ret = bsp_pcie_rc_notify_ep_user_init();
    if (ret) {
        PCIE_DEV_TRACE("pcie rc notify ep fail, ret = %d", ret);
        goto err_free;
    }

    PCIE_DEV_TRACE("pcie rc notify ep done");

    pcie_rc_register_linkdown_event();

    register_reboot_notifier(&g_pcie_shutdown_notifier);

    PCIE_DEV_TRACE("pcie_rc_init done");

    return 0;

err_free:
    wakeup_source_remove(g_pcie_rc_info->wake_lock);
    __pm_relax(g_pcie_rc_info->wake_lock);

    kfree(g_pcie_rc_info);
    g_pcie_rc_info = NULL;
    return -1;
}

late_initcall(bsp_pcie_rc_normalize_init);

/* debug function */
int pcie_rc_info(void)
{
    u32 i;

    unsigned long bar_addr[PCIE_BAR_MAX_NUM] = {0};
    unsigned long bar_size[PCIE_BAR_MAX_NUM] = {0};

    printk(KERN_ERR "*************************** PCIE RC INIT EVENT FLAG ************************** \n");
    for (i = 0; i < PCIE_RC_EVENT_MAX; i++) {
        printk(KERN_ERR "<%-32s>: %d \r\n", g_pcie_rc_dbg_info[i].pcie_rc_event_name, g_pcie_rc_dbg_info[i].init_flag);
    }

    printk(KERN_ERR "*************************** PCIE RC INFO BEGIN ************************** \n");
    printk(KERN_ERR "EP Vendor ID:        0x%04X\n", PCIE_IOT_VENDOR_ID);
    printk(KERN_ERR "EP Device ID:        0x%04X\n", PCIE_IOT_DEVICE_ID);

    if (g_pcie_rc_info == NULL) {
        PCIE_DEV_TRACE("pcie init failed, check init log");
        return -1;
    }

    printk(KERN_ERR "PCIE Func Num:     %d\n", g_pcie_rc_info->valid_ep_func_num);

    for (i = 0; i < PCIE_EP_MSI_NUM; i++) {
        if (g_pcie_rc_info->irq_chn[i].handler && g_pcie_rc_info->irq_chn[i].irq_count) {
            printk(KERN_ERR "<RC MSI User Id: %d , Get Irq Num>: %lu \n", i, g_pcie_rc_info->irq_chn[i].irq_count);
            printk(KERN_ERR "<RC MSI User Id: %d , Handle Irq Num>: %lu \n", i,
                   g_pcie_rc_info->irq_chn[i].irq_handler_count);
        }
    }

    for (i = 0; i < PCIE_RC_MSI_NUM; i++) {
        if (g_pcie_rc_info->rc_send_msi_count[i]) {
            printk(KERN_ERR "<RC To EP MSI User Id: %d , Send Irq Num>: %lld \n", i,
                   g_pcie_rc_info->rc_send_msi_count[i]);
        }
    }

    for (i = 0; i < PCIE_BAR_MAX_NUM; i++) {
        bar_addr[i] = bsp_pcie_rc_get_bar_addr(i);
        bar_size[i] = bsp_pcie_rc_get_bar_size(i);
        if (bar_addr[i] && bar_size[i]) {
            printk(KERN_ERR "Bar_User Id: %d, Bar_Size: 0x%08lX \n", i, bar_size[i]);
        }
    }

    printk(KERN_ERR "**************************** PCIE RC INFO OVER *************************** \n");

    return 0;
}

int pcie_rc_wake_unlock_debug(void)
{
    __pm_relax(g_pcie_rc_info->wake_lock);
    return 0;
}

/* msi test for intx */
static irqreturn_t pcie_rc_intx_debug_handler(int irq_num, void *data)
{
    PCIE_RC_TEST_PR_ERR("msi_user: %d,  irq arrived", (u32)(uintptr_t)data);
    return IRQ_HANDLED;
}

void pcie_rc_msi_receive_debug(void)
{
    int id = PCIE_EP_MSI_TEST_BEGIN;
    while (id <= PCIE_EP_MSI_TEST_END) {
        (void)bsp_pcie_rc_msi_request(id, pcie_rc_intx_debug_handler, NULL, (void *)(uintptr_t)id);
        id++;
    }
}

void m_pcie_rc_pm_state(u32 power_state)
{
    pci_set_power_state(g_pcie_rc_info->iot_rc, power_state);
}

u32 d_pcie_rc_pm_state(void)
{
    return g_pcie_rc_info->iot_rc->current_state;
}

void m_pcie_ep_pm_state(u32 power_state)
{
    u32 func_id;
    for (func_id = 0; func_id < g_pcie_rc_info->valid_ep_func_num; func_id++) {
        pci_set_power_state(g_pcie_rc_info->iot_multi_ep[func_id], power_state);
    }
}

u32 d_pcie_ep_pm_state(u32 id)
{
    return g_pcie_rc_info->iot_ep->current_state;
}

#ifndef CONFIG_IOT_PCIE_DEV_PM
int bsp_pcie_rc_vote_unlock(enum pcie_user_id user_id)
{
    return 0;
}
int bsp_pcie_rc_vote_lock(enum pcie_user_id user_id, int force_wake)
{
    return 0;
}
int bsp_pcie_rc_vote_lock_timeout(enum pcie_user_id user_id, long jiffies)
{
    return 0;
}
int bsp_is_pcie_first_user(enum pcie_ep_to_rc_msi_id msi_id)
{
    return 0;
}
#endif

