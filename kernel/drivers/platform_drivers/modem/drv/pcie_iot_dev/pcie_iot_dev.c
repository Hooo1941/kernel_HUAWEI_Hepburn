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
 * Description: PCIE IOT Device Driver
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>

#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/spinlock.h>

#include <linux/fs.h>
#include <linux/clk.h>
#include <linux/msi.h>
#include <linux/irq.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/irqdesc.h>
#include <linux/irqdomain.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>

#include <linux/of_platform.h>
#include <linux/regulator/consumer.h>
#include <linux/dma-mapping.h>
#include <asm/cacheflush.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/pci_regs.h>
#include <linux/pci-aspm.h>
#include <linux/syscore_ops.h>
#include "pcie_iot_dev.h"
#include "pcie_iot_debug.h"
#include "securec.h"
#include "bsp_dump.h"
#include "bsp_print.h"
#include "adrv_pcie.h"

#define PCIE_UPPER_U16_SHIFT 16
#define PCIE_IOT_VENDOR_ID 0x19E5
#define PCIE_IOT_DEVICE_ID 0x5000
#define PCIE_IOT_EP_ID (PCIE_IOT_DEVICE_ID << PCIE_UPPER_U16_SHIFT | PCIE_IOT_VENDOR_ID)
#define PCIE_READ_MAX_RETRY_TIMES 10

#undef THIS_MODU
#define THIS_MODU mod_pcie

#define PCIE_RC_PR_ERR(fmt, ...) do { \
    bsp_err("%s:" fmt "\n", __FUNCTION__, ##__VA_ARGS__); \
} while (0)
struct rc_pcie_flag g_pcie_rc_flag;
struct rc_pcie_info *g_pcie_rc_info = NULL;
struct pcie_callback_info_list g_pcie_rc_user_cb;
struct pcie_kport_register_event g_pcie_kport_event;

struct pcie_rc_debug_info g_pcie_rc_dbg_info[PCIE_RC_EVENT_MAX] = {
    {"PCIE_RC_GET_EP_DEV",                                0},
    {"PCIE_RC_GET_FUNC_NUM",                              0},
    {"PCIE_RC_GET_BAR",                                   0},
    {"PCIE_RC_INT_INIT",                                  0},
    {"PCIE_RC_USER_INIT",                                 0},
};

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

void set_pcie_first_user_flag(void)
{
    PCIE_RC_PR_ERR("pcie syscore resume");
    g_pcie_rc_flag.first_user_flag = 1;
    g_pcie_rc_flag.first_user_id = 0;
}

int clear_pcie_first_user_flag(void)
{
    PCIE_RC_PR_ERR("pcie syscore suspend");
    g_pcie_rc_flag.first_user_flag = 0;
    g_pcie_rc_flag.first_user_id = 0;
    return 0;
}

void pcie_iot_dev_syscore_shutdown(void)
{
    return;
}

/*
 * bsp_is_pcie_first_user - check first pcie msi user id match status.
 * This function is used for check first user id match after syscore resume.
 */
int bsp_is_pcie_first_user(enum pcie_ep_to_rc_msi_id msi_id)
{
    int msi_id_match = g_pcie_rc_flag.first_user_id & (0x1 << msi_id);
    return msi_id_match;
}

static struct syscore_ops g_pcie_iot_dev_dpm_ops = {
    .suspend = clear_pcie_first_user_flag,
    .resume = set_pcie_first_user_flag,
    .shutdown = pcie_iot_dev_syscore_shutdown,
};

void pcie_iot_dev_shutdown(struct pci_dev *dev)
{
    // use bit operation to avoid shutdown and remove happen at same time, only implement one round uninstall
    if (!test_and_set_bit(PCIE_DEV_REMOVE_FLAG, &g_pcie_rc_flag.rm_state)) {
        PCIE_RC_PR_ERR("pcie iot dev shutdown +");

        while (down_interruptible(&g_pcie_rc_info->init_done_sema)) { } // wait for pcie init done
        up(&g_pcie_rc_info->init_done_sema);

        disable_irq(g_pcie_rc_info->iot_ep->irq);
        g_pcie_rc_flag.remove_start = 0x1;  // basic protect to stop new pcie visit

        (void)bsp_pcie_rc_cb_run(PCIE_RC_CB_EXIT);  // first,  stop business
        PCIE_RC_PR_ERR("kernel iot user exit done");

#ifdef CONFIG_IOT_PCIE_DEV_PM
        (void)bsp_pcie_check_vote_clear(PCIE_WAIT_UNLOCK_MS);  // wait for pcie vote clear
        bsp_pcie_rc_pm_remove(RC_PCIE_SHUTDOWN);  // second,  stop pcie pm control to make sure all user task not
        PCIE_RC_PR_ERR("kernel iot pcie pm remove done");
#endif

        PCIE_RC_PR_ERR("pcie iot dev shutdown -");
    }

    return;
}

struct pci_driver g_pcie_iot_dev_driver = {
    .name = "pcie_iot_dev",
    .id_table = g_pcie_iot_dev_table,
    .probe = pcie_iot_dev_probe,
    .shutdown = pcie_iot_dev_shutdown,
    .remove = NULL,
    .err_handler = NULL,
};

int pcie_get_all_ep_dev(void)
{
    int ret;
    ret = pci_register_driver(&g_pcie_iot_dev_driver);
    if (ret) {
        PCIE_RC_PR_ERR("register g_pcie_iot_dev_driver err, ret = %d", ret);
        return ret;
    }

    return ret;
}

int bsp_pcie_rc_cb_register(enum pcie_user_id usr_id, struct pcie_callback_info *info)
{
    if (info == NULL) {
        PCIE_RC_PR_ERR("param is null");
        return -1;
    }

    if (g_pcie_rc_user_cb.list[usr_id].callback) {
        PCIE_RC_PR_ERR("input usr_id callback has been registered");
        return -1;
    }

    if (usr_id >= PCIE_USER_NUM) {
        PCIE_RC_PR_ERR("invalid input usr_id");
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
            pcie_callback_dbg(user_id, callback_stage, ret);
            total_ret |= (u32)ret;
            if (callback_stage == PCIE_RC_CB_ENUM_DONE) {
                g_pcie_rc_dbg_info[PCIE_RC_USER_INIT].init_flag |= (0x1 << user_id);
            }
        }
    }
    return (int)total_ret;
}

unsigned long bsp_pcie_rc_get_bar_addr(enum pcie_bar_id_e bar_index)
{
    unsigned long ep_bar_pci_addr;
    u32 func_num = bar_index / PCIE_BAR_NUM;
    u32 in_func_bar_id = bar_index % PCIE_BAR_NUM;

    if (bar_index >= PCIE_BAR_MAX_NUM) {
        PCIE_RC_PR_ERR("invalid bar id input %d, no more than %d!", bar_index, PCIE_BAR_MAX_NUM);
        return 0;
    }
    if (g_pcie_rc_info->iot_multi_ep[func_num]) {
        ep_bar_pci_addr = pci_resource_start(g_pcie_rc_info->iot_multi_ep[func_num], in_func_bar_id);
        return ep_bar_pci_addr;
    } else {
        PCIE_RC_PR_ERR("no ep device found, bar_index : %d, func_num: %d, in_func_bar_id: %d!", bar_index, func_num,
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
        PCIE_RC_PR_ERR("invalid bar id input %d, no more than %d!", bar_index, PCIE_BAR_MAX_NUM);
        return 0;
    }

    if (g_pcie_rc_info->iot_multi_ep[func_num]) {
        ep_bar_size = pci_resource_len(g_pcie_rc_info->iot_multi_ep[func_num], in_func_bar_id);
        return ep_bar_size;
    } else {
        PCIE_RC_PR_ERR("no ep device found, bar_index : %d, func_num: %d, in_func_bar_id: %d!", bar_index, func_num,
                       in_func_bar_id);
        return 0;
    }
}

void pcie_rc_record_bar_info(void)
{
    u32 func_num;
    u32 in_func_bar_id;
    u32 bar_index = 0;
    unsigned long ep_bar_pci_addr;
    unsigned long ep_bar_size;

    while (bar_index < PCIE_BAR_MAX_NUM) {
        func_num = bar_index / PCIE_BAR_NUM;
        in_func_bar_id = bar_index % PCIE_BAR_NUM;
        if (g_pcie_rc_info->iot_multi_ep[func_num]) {
            ep_bar_pci_addr = pci_resource_start(g_pcie_rc_info->iot_multi_ep[func_num], in_func_bar_id);
            ep_bar_size = pci_resource_len(g_pcie_rc_info->iot_multi_ep[func_num], in_func_bar_id);
            pcie_bar_dbg(bar_index, ep_bar_size, ep_bar_pci_addr);
        }
        bar_index++;
    }
}

void *rc_get_normal_bar_virt_addr(enum pcie_bar_id_e bar_index)
{
    unsigned long bar_pci_addr;
    unsigned long bar_size;
    void *bar_virt_addr = NULL;

    bar_pci_addr = bsp_pcie_rc_get_bar_addr(bar_index);
    bar_size = bsp_pcie_rc_get_bar_size(bar_index);
    if (!bar_pci_addr || !bar_size) {
        PCIE_RC_PR_ERR("bar index %d , get bar resource fail", bar_index);
        return NULL;
    }

    bar_virt_addr = ioremap_wc(bar_pci_addr, bar_size);
    if (bar_virt_addr == NULL) {
        PCIE_RC_PR_ERR("bar index %d, fail to ioremap virt addr", bar_index);
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
        PCIE_RC_PR_ERR("bar index %d , get bar resource fail", bar_index);
        return NULL;
    }

    bar_virt_addr = ioremap(bar_pci_addr, bar_size);
    if (bar_virt_addr == NULL) {
        PCIE_RC_PR_ERR("bar index %d, fail to ioremap virt addr", bar_index);
        return NULL;
    }

    return bar_virt_addr;
}

int pcie_rc_get_base_addr(void)
{
    g_pcie_rc_info->ep_msi_usr_virt_addr = rc_get_normal_bar_virt_addr(PCIE_BAR_MSI_TO_RC);
    if (g_pcie_rc_info->ep_msi_usr_virt_addr == NULL) {
        PCIE_RC_PR_ERR("get ep to rc msi bar resource fail");
        return -1;
    }

    g_pcie_rc_info->ep_in_rc_msi_virt_addr = rc_get_reg_bar_virt_addr(PCIE_BAR_GIC_MSI);
    if (g_pcie_rc_info->ep_in_rc_msi_virt_addr == NULL) {
        PCIE_RC_PR_ERR("get ep gic msi bar resource fail");
        return -1;
    }

    g_pcie_rc_info->ep_pcie_base_virt_addr = rc_get_reg_bar_virt_addr(PCIE_BAR_EP_DMA_CTL);
    if (g_pcie_rc_info->ep_pcie_base_virt_addr == NULL) {
        PCIE_RC_PR_ERR("get ep dma ctl bar resource fail");
        return -1;
    } else {
        g_pcie_rc_info->ep_dma_base_virt_addr = g_pcie_rc_info->ep_pcie_base_virt_addr +
                                                g_pcie_rc_info->ep_dma_base_addr_offset;
    }

    g_pcie_rc_info->ep_intx_virt_addr = rc_get_reg_bar_virt_addr(PCIE_BAR_INTX);
    if (g_pcie_rc_info->ep_intx_virt_addr == NULL) {
        PCIE_RC_PR_ERR("get ep intx bar resource fail");
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
    if (g_pcie_rc_info->ep_pcie_base_virt_addr != NULL) {
        iounmap(g_pcie_rc_info->ep_pcie_base_virt_addr);
    }
    if (g_pcie_rc_info->ep_intx_virt_addr != NULL) {
        iounmap(g_pcie_rc_info->ep_intx_virt_addr);
    }
    return;
}

static int pcie_rc_dma_transfer(struct pcie_dma_transfer_info *transfer_info)
{
    struct rc_pcie_info *info = g_pcie_rc_info;
    struct pcie_dma_data_element *element = &transfer_info->element;

    // Attention:  when set ep dma registers on rc , the direction is opposite
    if (g_pcie_rc_flag.remove_start || g_pcie_rc_flag.linkdown_detect) {
        return -1;
    }

    if (transfer_info->direction == PCIE_DMA_DIRECTION_WRITE) {
        /* DMA Read Engine Enable */
        writel(0x01U, info->ep_dma_base_virt_addr + 0x02CU);
        /* DMA Read Interrupt Mask */
        writel(0x00U, info->ep_dma_base_virt_addr + 0x0A8U);
        /* DMA Read Channel Control 1, Read LIE Enable */
        writel(0x01U << 3U, info->ep_dma_base_virt_addr + transfer_info->channel * 0x200 + 0x300U);
        /* DMA Read Transfer Size */
        writel(element->transfer_size, info->ep_dma_base_virt_addr + transfer_info->channel * 0x200 + 0x308U);
        /* DMA Read SAR Low */
        writel(element->sar_low, info->ep_dma_base_virt_addr + transfer_info->channel * 0x200 + 0x30CU);
        /* DMA Read SAR High */
        writel(element->sar_high, info->ep_dma_base_virt_addr + transfer_info->channel * 0x200 + 0x310U);
        /* DMA Read DAR Low */
        writel(element->dar_low, info->ep_dma_base_virt_addr + transfer_info->channel * 0x200 + 0x314U);
        /* DMA Read DAR High */
        writel(element->dar_high, info->ep_dma_base_virt_addr + transfer_info->channel * 0x200 + 0x318U);
        /* DMA Read Doorbell */
        writel(transfer_info->channel, info->ep_dma_base_virt_addr + 0x030U);
    } else {
        /* DMA Write Engine Enable */
        writel(0x01U, info->ep_dma_base_virt_addr + 0x00CU);
        /* DMA Write Interrupt Mask */
        writel(0x00U, info->ep_dma_base_virt_addr + 0x054U);
        /* DMA Write Channel Control 1, Read LIE Enable */
        writel(0x01U << 3U, info->ep_dma_base_virt_addr + transfer_info->channel * 0x200 + 0x200U);
        /* DMA Write Transfer Size */
        writel(element->transfer_size, info->ep_dma_base_virt_addr + transfer_info->channel * 0x200 + 0x208U);
        /* DMA Write SAR Low */
        writel(element->sar_low, info->ep_dma_base_virt_addr + transfer_info->channel * 0x200 + 0x20CU);
        /* DMA Write SAR High */
        writel(element->sar_high, info->ep_dma_base_virt_addr + transfer_info->channel * 0x200 + 0x210U);
        /* DMA Write DAR Low */
        writel(element->dar_low, info->ep_dma_base_virt_addr + transfer_info->channel * 0x200 + 0x214U);
        /* DMA Write DAR High */
        writel(element->dar_high, info->ep_dma_base_virt_addr + transfer_info->channel * 0x200 + 0x218U);
        /* DMA Write Doorbell */
        writel(transfer_info->channel, info->ep_dma_base_virt_addr + 0x010U);
    }
    return 0;
}

static int pcie_rc_dma_transfer_link_mode(struct pcie_dma_transfer_info *transfer_info)
{
    u64 dma_ll_info;
    struct rc_pcie_info *info = g_pcie_rc_info;

    if (!transfer_info->dma_lli_addr) {
        PCIE_RC_PR_ERR("param error, dma link structure is null!");
        return -1;
    }

    dma_ll_info = (u64)transfer_info->dma_lli_addr;

    // Attention:  when set ep dma registers on rc , the direction is opposite
    if (transfer_info->direction == PCIE_DMA_DIRECTION_WRITE) {
        /* DMA Read Engine Enable */
        writel(0x01U, info->ep_dma_base_virt_addr + 0x02CU);
        /* DMA Read Interrupt Mask */
        writel(0x00U, info->ep_dma_base_virt_addr + 0x0A8U);
        /*
         * DMA Channel Control 1 register
         * ■ Linked List Enable (LLE) =1
         * ■ Consumer Cycle Status (CCS) =PCS =0
         * ■ LIE = 1 bit3
         * ■ AT, RO, NS, TC, Function Number =0
         */
        writel(0x208U, info->ep_dma_base_virt_addr + transfer_info->channel * 0x200 + 0x300U);
        /* DMA Read Linked List Pointer Low Register */
        writel(dma_ll_info & 0xFFFFFFFFUL, info->ep_dma_base_virt_addr + transfer_info->channel * 0x200 + 0x31CU);
        /* DMA Read Linked List Pointer High Register */
        writel(dma_ll_info >> PCIE_DMA_ADDR_SHIFT,
            info->ep_dma_base_virt_addr + transfer_info->channel * 0x200 + 0x320U);
        /* DMA Read Doorbell */
        writel(transfer_info->channel, info->ep_dma_base_virt_addr + 0x030U);
    } else {
        /* DMA Write Engine Enable */
        writel(0x01U, info->ep_dma_base_virt_addr + 0x00CU);
        /* DMA Write Interrupt Mask */
        writel(0x00U, info->ep_dma_base_virt_addr + 0x054U);
        /*
         * DMA Channel Control 1 register
         * ■ Linked List Enable (LLE) =1
         * ■ Consumer Cycle Status (CCS) =PCS =0
         * ■ LIE = 1 bit3
         * ■ AT, RO, NS, TC, Function Number =0
         */
        writel(0x208U, info->ep_dma_base_virt_addr + transfer_info->channel * 0x200 + 0x200U);
        /* DMA Linked List Pointer Low */
        writel(dma_ll_info & 0xFFFFFFFFUL, info->ep_dma_base_virt_addr + transfer_info->channel * 0x200 + 0x21CU);
        /* DMA Linked List Pointer High */
        writel(dma_ll_info >> PCIE_DMA_ADDR_SHIFT,
            info->ep_dma_base_virt_addr + transfer_info->channel * 0x200 + 0x220U);
        /* DMA Write Doorbell */
        writel(transfer_info->channel, info->ep_dma_base_virt_addr + 0x010U);
    }

    return 0;
}

/*
 * bsp_pcie_rc_dma_transfer - rc to ep dma transfer function
 * This function is used to pass transfer info into pcie dma register and start dma transfer
 * process, based on transfer_info->transfer_type, there are two ways for dma transfer,
 * dma normal mode--transfer dma single package with one source and one destination
 * dma link mode--transfer multiple dma package based on transfer info element config.
 */
int bsp_pcie_rc_dma_transfer(struct pcie_dma_transfer_info *transfer_info)
{
    int ret;
    unsigned long irq_flags = 0;
    struct rc_pcie_info *info = g_pcie_rc_info;

    if (transfer_info == NULL) {
        PCIE_RC_PR_ERR("error args, transfer_info is NULL");
        return -1;
    }

    if (transfer_info->channel >= PCIE_DMA_CHN_NUM) {
        PCIE_RC_PR_ERR("error channel index, the max channel index is %u", PCIE_DMA_CHN_NUM);
        return -1;
    }

    if (g_pcie_rc_flag.pm_status == PCIE_SLEEP) {
        PCIE_RC_PR_ERR("error usage detected!dma channel user: %d,  direction: %d use pcie dma during pcie sleep",
                       transfer_info->channel, transfer_info->direction);
        dump_stack();
    }

    if (transfer_info->direction) {
        pcie_dma_dbg(transfer_info->channel, PCIE_DMA_WRITE, PCIE_DMA_NA);
    } else {
        pcie_dma_dbg(transfer_info->channel, PCIE_DMA_READ, PCIE_DMA_NA);
    }

    pcie_dma_addr_dbg(transfer_info);

    spin_lock_irqsave(&info->spinlock, irq_flags);

    /* ****need to check dma callback arg when ep to rc msi enabled** */
    info->dma_channel_state |= (1U << transfer_info->channel);

    if (transfer_info->transfer_type == PCIE_DMA_NORMAL_MODE) {
        ret = pcie_rc_dma_transfer(transfer_info);
    } else {
        ret = pcie_rc_dma_transfer_link_mode(transfer_info);
    }

    spin_unlock_irqrestore(&info->spinlock, irq_flags);

    return ret;
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
        PCIE_RC_PR_ERR("invalid input, checkinput parameter");
        return -1;
    }

    if (g_pcie_rc_info->irq_chn[id].handler) {
        PCIE_RC_PR_ERR("pcie gic msi[%d] has handler", id);
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
        PCIE_RC_PR_ERR("invalid input, checkinput parameter");
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
        PCIE_RC_PR_ERR("invalid id, no more than %d", PCIE_EP_MSI_MAX_ID);
        return -1;
    }
    iot_pcie_rc_msi_enable(id);
    return 0;
}
int bsp_pcie_rc_msi_disable(enum pcie_ep_to_rc_msi_id id)
{
    if (id >= PCIE_EP_MSI_MAX_ID) {
        PCIE_RC_PR_ERR("invalid id, no more than %d", PCIE_EP_MSI_MAX_ID);
        return -1;
    }
    iot_pcie_rc_msi_disable(id);
    return 0;
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

        if (g_pcie_rc_flag.first_user_flag == 1 && i != PCIE_EP_MSI_PM_STATUS &&
            i != PCIE_EP_WAKE_RC_USER_MSI) {
            g_pcie_rc_flag.first_user_id |= (0x1 << i);
        }

        info->irq_chn[i].irq_count++;
        writel(0, info->ep_msi_usr_virt_addr + i * sizeof(u32));
        pcie_msi_dbg(i, MSI_READ, MSI_IN);
        user_handler(i, info->irq_chn[i].irq_data);
        pcie_msi_dbg(i, MSI_READ, MSI_OUT);
        info->irq_chn[i].irq_handler_count++;
    }

    // after recognize first pcie user msi id, clear flag
    if (g_pcie_rc_flag.first_user_id > 0 && g_pcie_rc_flag.first_user_flag > 0) {
        schedule_work(&g_pcie_rc_info->show_wake_user_work);
        g_pcie_rc_flag.first_user_flag = 0;
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
        PCIE_RC_PR_ERR("invalid msi id, no more than %d", PCIE_RC_MSI_MAX_ID);
        return -1;
    }

    pcie_msi_dbg(id, MSI_SEND, MSI_NA);

#ifdef CONFIG_IOT_PCIE_DEV_PM
    if (g_pcie_rc_flag.pm_status == PCIE_SLEEP && id != PCIE_RC_MSI_PM) {
        PCIE_RC_PR_ERR("error usage detected!rc to ep msi user: %d send msi during pcie sleep", id);
        dump_stack();
    }
#endif

    spin_lock_irqsave(&g_pcie_rc_info->spinlock, irq_flags);
    writel(id, g_pcie_rc_info->ep_in_rc_msi_virt_addr);  // protect for the same location write
    spin_unlock_irqrestore(&g_pcie_rc_info->spinlock, irq_flags);

    g_pcie_rc_info->rc_send_msi_count[id]++;

    return 0;
}

int bsp_pcie_rc_notify_ep_user_init(void)
{
    return bsp_pcie_rc_send_msi(PCIE_RC_MSI_USER_INIT);
}

void bsp_pcie_rc_irq_disable(void)
{
    if (g_pcie_rc_info->iot_ep->irq) {
        (void)disable_irq(g_pcie_rc_info->iot_ep->irq);  // disable irq and wait for onging irq finish
    }
}

void bsp_pcie_rc_irq_enable(void)
{
    if (g_pcie_rc_info->iot_ep->irq) {
        (void)enable_irq(g_pcie_rc_info->iot_ep->irq);
    }
}

u32 bsp_pcie_get_port_id(void)
{
    return g_pcie_rc_info->ep_pcie_port_id;
}

int pcie_rc_int_init(void)
{
    int ret;
    ret = request_irq(g_pcie_rc_info->iot_ep->irq, pcie_rc_msi_int_isr,
        IRQF_SHARED | IRQF_NO_SUSPEND, "PCIe KERNEL INTx", (void *)g_pcie_rc_info->iot_ep);
    if (ret) {
        PCIE_RC_PR_ERR("request_irq fail, msi irq = %d, ret = %d", g_pcie_rc_info->iot_ep->irq, ret);
        return -1;
    }
    g_pcie_rc_dbg_info[PCIE_RC_INT_INIT].init_flag = 0x1;
    return 0;
}

int get_iot_pcie_dts_cfg(void)
{
    int ret;
    struct device_node *dev_node = NULL;
    dev_node = of_find_compatible_node(NULL, NULL, "iot,pcie_iot");
    if (dev_node == NULL) {
        PCIE_RC_PR_ERR("pcie device not found!");
        return -1;
    }
    ret = of_property_read_u32(dev_node, "pcie_port_id", &g_pcie_rc_info->ep_pcie_port_id);
    ret |= of_property_read_u32(dev_node, "pcie_dma_bar_offset", &g_pcie_rc_info->ep_dma_base_addr_offset);
    ret |= of_property_read_u32_index(dev_node, "pcie_id_attr", 0, &g_pcie_rc_info->ep_vendor_id);
    ret |= of_property_read_u32_index(dev_node, "pcie_id_attr", 1, &g_pcie_rc_info->ep_device_id);
    if (ret) {
        PCIE_RC_PR_ERR("fail to get iot pcie dts info, ret = %d", ret);
        return ret;
    }
    return 0;
}

int pcie_rc_drv_cb(u32 user_id, u32 user_action_id, void *callback_args)
{
    if (user_action_id == PCIE_RC_CB_POWER_DOWN) {
        g_pcie_rc_flag.pm_status = PCIE_SLEEP;
    } else if (user_action_id == PCIE_RC_CB_RESUME) {
        g_pcie_rc_flag.pm_status = PCIE_WAKE;
    } else if (user_action_id == PCIE_RC_CB_LINKDOWN) {
        g_pcie_rc_flag.linkdown_detect = 0x1;
    }
    return 0;
}

int pcie_rc_drv_cb_register(void)
{
    u32 ret;
    struct pcie_callback_info user_callback_info = {
        .callback = pcie_rc_drv_cb,
        .callback_args = NULL,
    };

    ret = bsp_pcie_rc_cb_register(PCIE_USER_DRV, &user_callback_info);
    return ret;
}

void pcie_show_wake_user(struct work_struct *work)
{
    int msi_user_id;
    for (msi_user_id = 0; msi_user_id < PCIE_EP_MSI_NUM; msi_user_id++) {
        if (bsp_is_pcie_first_user(msi_user_id)) {
            printk(KERN_ERR "[C SR]Pcie First Wake Msi Id: %d\n", msi_user_id);
        }
    }
}

void pcie_bar_space_error_handle(struct work_struct *work)
{
    PCIE_RC_PR_ERR("bar space read error, error bar space content: 0x%x, call system error",
                   g_pcie_rc_flag.error_bar_content);
    system_error(DRV_ERRNO_PCIE_ABNORMAL, RC_PCIE_BAR_SPACE_ERR, 0, NULL, 0);
}

void pcie_rc_linkdown_handler(struct pcie_kport_notify *notify)
{
    if (test_bit(PCIE_DEV_LINKDOWN_FLAG, &g_pcie_rc_flag.rm_state)) {
        PCIE_RC_PR_ERR("iot pcie linkdown detected");
        bsp_pcie_rc_cb_run(PCIE_RC_CB_LINKDOWN);
        system_error(DRV_ERRNO_PCIE_ABNORMAL, PCIE_LINKDOWN_DETECT, 0, NULL, 0);
    } else {
        PCIE_RC_PR_ERR("iot pcie linkdown detected during pcie turn_off, ignore");
    }
    return;
}

void bsp_pcie_rc_linkdown_event_disable(void)
{
    clear_bit(PCIE_DEV_LINKDOWN_FLAG, &g_pcie_rc_flag.rm_state);
    return;
}

void bsp_pcie_rc_linkdown_event_enable(void)
{
    set_bit(PCIE_DEV_LINKDOWN_FLAG, &g_pcie_rc_flag.rm_state);
    return;
}

void pcie_rc_register_linkdown_event(void)
{
    g_pcie_kport_event.events = PCIE_KPORT_EVENT_LINKDOWN;
    g_pcie_kport_event.user = g_pcie_rc_info->iot_ep;
    g_pcie_kport_event.mode = PCIE_KPORT_TRIGGER_CALLBACK;
    g_pcie_kport_event.callback = pcie_rc_linkdown_handler;
    pcie_kport_register_event(&g_pcie_kport_event);
}

void pcie_rc_flag_init(void)
{
    int i;
    for (i = 0; i < PCIE_RC_EVENT_MAX; i++) {
        g_pcie_rc_dbg_info[i].init_flag = 0;
    }
    g_pcie_rc_flag.linkdown_detect = 0x0;
    g_pcie_rc_flag.pm_status = PCIE_WAKE;
    g_pcie_rc_flag.remove_start = 0x0;
    g_pcie_rc_flag.first_user_flag = 0x0;
    g_pcie_rc_flag.first_user_id = 0x0;
    g_pcie_rc_flag.error_bar_content = 0x0;
    clear_bit(PCIE_DEV_REMOVE_FLAG, &g_pcie_rc_flag.rm_state);
}

int bsp_pcie_init_before_boot(void)
{
    g_pcie_rc_info = (struct rc_pcie_info *)kzalloc(sizeof(struct rc_pcie_info), GFP_KERNEL);
    if (g_pcie_rc_info == NULL) {
        PCIE_RC_PR_ERR("fail to kzalloc, size: 0x%lX", (unsigned long)sizeof(struct rc_pcie_info));
        return -1;
    }
    register_syscore_ops(&g_pcie_iot_dev_dpm_ops);
    INIT_WORK(&g_pcie_rc_info->show_wake_user_work, pcie_show_wake_user);
    INIT_WORK(&g_pcie_rc_info->bar_space_error_work, pcie_bar_space_error_handle);
#ifndef CONFIG_IOT_PCIE_DEV_PM
    wakeup_source_init(&g_pcie_rc_info->wake_lock, "pcie_iot_wakelock");
    __pm_stay_awake(&g_pcie_rc_info->wake_lock);
#endif
    spin_lock_init(&g_pcie_rc_info->spinlock);
    sema_init(&g_pcie_rc_info->init_done_sema, 1);
    return 0;
}

int pcie_rc_device_init(void)
{
    int i;
    /* get multifunction ep dev */
    if (pcie_get_all_ep_dev()) {
        PCIE_RC_PR_ERR("fail to get ep device");
        goto err_ret;
    }
    g_pcie_rc_dbg_info[PCIE_RC_GET_EP_DEV].init_flag = 0x1;
    g_pcie_rc_dbg_info[PCIE_RC_GET_FUNC_NUM].init_flag = g_pcie_rc_info->valid_ep_func_num;

    pcie_rc_record_bar_info();

    /* use func0 dev as main function */
    g_pcie_rc_info->iot_ep = g_pcie_rc_info->iot_multi_ep[0];

    /* enable all functions */
    for (i = 0; i < g_pcie_rc_info->valid_ep_func_num; i++) {
        if (pci_enable_device(g_pcie_rc_info->iot_multi_ep[i])) {
            PCIE_RC_PR_ERR("pci_enable_device fail, func_num: %d", i);
            goto err_ret1;
        }
    }

    /* set master all functions */
    for (i = 0; i < g_pcie_rc_info->valid_ep_func_num; i++) {
        pci_set_master(g_pcie_rc_info->iot_multi_ep[i]);
    }

    if (pcie_rc_get_base_addr()) {
        PCIE_RC_PR_ERR("fail to get rc base address");
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
err_ret:
    return -1;
}

void pcie_rc_late_init(void)
{
    // register pcie callback to get pcie pm status
    (void)pcie_rc_drv_cb_register();
    // need to registre NULL notify handler, avoid other user power notify affect
    (void)pcie_kport_power_notifiy_register(g_pcie_rc_info->ep_pcie_port_id, NULL, NULL, NULL);
    // register linkdown handler and enable linkdown detect
    pcie_rc_register_linkdown_event();
    bsp_pcie_rc_linkdown_event_enable();
}

int pcie_rc_drv_init(void)
{
    PCIE_RC_PR_ERR("pcie_rc_iot kernel init start");

    pcie_rc_flag_init();

    if (get_iot_pcie_dts_cfg()) {
        PCIE_RC_PR_ERR("fail to get iot pcie dts info");
        return -1;
    }

    if (pcie_rc_device_init()) {
        PCIE_RC_PR_ERR("fail to get iot pcie device info");
        return -1;
    }

    if (pcie_rc_int_init()) {
        PCIE_RC_PR_ERR("fail to rc msi init");
        goto err_ret;
    }

    pcie_rc_late_init();

#ifdef CONFIG_IOT_PCIE_DEV_TEST
    bsp_pcie_rc_test_init_register();
#endif

    PCIE_RC_PR_ERR("pcie_rc_iot kernel init end");

    return 0;

err_ret:
    pcie_release_rc_base_addr();
    return -1;
}

int bsp_pcie_modem_kernel_init(void)
{
    int ret;

    if (g_pcie_rc_info == NULL) {
        PCIE_RC_PR_ERR("g_pcie_rc_info not init, direct return");
        return -1;
    }

    while (down_interruptible(&g_pcie_rc_info->init_done_sema)) { }

    ret = pcie_rc_drv_init();
    if (ret) {
        PCIE_RC_PR_ERR("g_pcie_rc_info not init, direct return");
        goto err_free;
    }
    bsp_pcie_boot_dbg(PCIE_KERNEL_INIT_DONE);

#ifdef CONFIG_IOT_PCIE_DEV_PM
    ret = bsp_pcie_rc_pm_init();
    if (ret) {
        PCIE_RC_PR_ERR("pcie rc pm init fail");
        goto err_free;
    }

    ret = bsp_pcie_rc_pm_api_init();
    if (ret) {
        PCIE_RC_PR_ERR("pcie rc pm api init fail");
        goto err_free;
    }
    bsp_pcie_boot_dbg(PCIE_PM_INIT_DONE);
#endif

    up(&g_pcie_rc_info->init_done_sema);
    g_pcie_rc_flag.init_done = 0x1;
    return 0;

err_free:
#ifndef CONFIG_IOT_PCIE_DEV_PM
    wakeup_source_trash(&g_pcie_rc_info->wake_lock);
#endif
    kfree(g_pcie_rc_info);
    g_pcie_rc_info = NULL;
    return -1;
}

int rc_kernel_pcie_unlock_debug(void)
{
    __pm_relax(&g_pcie_rc_info->wake_lock);
    return 0;
}

void rc_kernel_pcie_remove(void)
{
    u32 func_num;

    free_irq(g_pcie_rc_info->iot_ep->irq, (void *)g_pcie_rc_info->iot_ep);
    pcie_release_rc_base_addr();

    for (func_num = 0; func_num < g_pcie_rc_info->valid_ep_func_num; func_num++) {
        pci_disable_device(g_pcie_rc_info->iot_multi_ep[func_num]);
        g_pcie_rc_info->iot_multi_ep[func_num] = NULL;
    }
    g_pcie_rc_info->valid_ep_func_num = 0;

    pci_unregister_driver(&g_pcie_iot_dev_driver);  // if remove comes from modem reset, then unregister driver

    (void)pcie_kport_remove_ep(g_pcie_rc_info->ep_pcie_port_id);
    (void)pcie_kport_pm_control(0x2, g_pcie_rc_info->ep_pcie_port_id);  // power off pcie without PME
}

/*
 * bsp_kernel_pcie_remove - remove kernel pcie for modem reset
 * This function is used to remove kernel stage pcie for modem reset
 */
int bsp_kernel_pcie_remove(void)
{
    if (g_pcie_rc_info == NULL) {
        PCIE_RC_PR_ERR("g_pcie_rc_info not init, direct return");
        return -1;
    }

    if (!g_pcie_rc_flag.init_done) {
        PCIE_RC_PR_ERR("remove is called before pcie init done, direct return");
        g_pcie_rc_flag.init_done = 0;
        return 0;
    }

    if (!test_and_set_bit(0, &g_pcie_rc_flag.rm_state)) {
        PCIE_RC_PR_ERR("kernel iot pcie remove +");
        (void)bsp_pcie_rc_cb_run(PCIE_RC_CB_EXIT);  // first,  stop business
        PCIE_RC_PR_ERR("kernel iot user exit done");
#ifdef CONFIG_IOT_PCIE_DEV_PM
        (void)bsp_pcie_check_vote_clear(PCIE_WAIT_UNLOCK_MS);  // wait for pcie vote clear
        bsp_pcie_rc_pm_remove(RC_PCIE_RESET);  // second,  stop pcie pm control to make sure all user vote not suspend
        PCIE_RC_PR_ERR("kernel iot pcie pm remove done");
#endif
        disable_irq(g_pcie_rc_info->iot_ep->irq);
        g_pcie_rc_flag.remove_start = 0x1;  // basic protect to stop new pcie visit
        rc_kernel_pcie_remove();            // last, stop pcie driver
        PCIE_RC_PR_ERR("kernel iot pcie remove -");
    }

    return 0;
}

void show_pcie_config_error(struct pci_dev *pdev, u32 func_num, u32 offset, u32 val, int ret)
{
    printk(KERN_ERR "func_id 0x%x, offset 0x%x, pci read modem config after restore failed, val 0x%x, ret 0x%x\n",
           func_num, offset, val, ret);
}

void bsp_pcie_save_all_ep_cfg(void)
{
    int i;
    int func_num;
    int ret;
    for (func_num = 0; func_num < g_pcie_rc_info->valid_ep_func_num; func_num++) {
        for (i = 0; i < PCIE_CFG_REG_NUM; i++) {
            ret = pci_read_config_dword(g_pcie_rc_info->iot_multi_ep[func_num], i * sizeof(u32),
                                        &g_pcie_rc_info->ep_config_space[func_num][i]);
            if (ret) {
                show_pcie_config_error(g_pcie_rc_info->iot_multi_ep[func_num], func_num, i * sizeof(u32),
                                       g_pcie_rc_info->ep_config_space[func_num][i], ret);
            }
            pcie_config_space_dbg(SAVE_CONFIG, func_num, i, g_pcie_rc_info->ep_config_space[func_num][i]);
        }
    }
    return;
}

void pcie_dump_ep_cfg_after_restore(void)
{
    int i;
    int func_num;
    int ret;
    for (func_num = 0; func_num < g_pcie_rc_info->valid_ep_func_num; func_num++) {
        for (i = 0; i < PCIE_CFG_REG_NUM; i++) {
            ret = pci_read_config_dword(g_pcie_rc_info->iot_multi_ep[func_num], i * sizeof(u32),
                                        &g_pcie_rc_info->ep_config_space[func_num][i]);
            if (ret) {
                show_pcie_config_error(g_pcie_rc_info->iot_multi_ep[func_num], func_num, i * sizeof(u32),
                                       g_pcie_rc_info->ep_config_space[func_num][i], ret);
            }
            pcie_config_space_dbg(RESTORE_CONFIG, func_num, i, g_pcie_rc_info->ep_config_space[func_num][i]);
        }
    }
}

void pci_restore_iot_dword(struct pci_dev *pdev, int offset, u32 saved_val, int retry, u32 func_num)
{
    u32 val = 0;
    int ret = pci_read_config_dword(pdev, offset, &val);
    if (ret) {
        show_pcie_config_error(pdev, func_num, offset, val, ret);
    } else if (val == saved_val) {
        return;
    }

    do {
        pci_write_config_dword(pdev, offset, saved_val);
        ret = pci_read_config_dword(pdev, offset, &val);
        if (ret) {
            show_pcie_config_error(pdev, func_num, offset, val, ret);
        }
        if (val == saved_val) {
            return;
        }
        if (retry-- <= 0) {
            return;
        }
        mdelay(1);
    } while (1);
}

void pci_restore_iot_space_range(struct pci_dev *pdev, int start, int end, int retry, u32 func_num)
{
    int index;
    for (index = end; index >= start; index--) {
        pci_restore_iot_dword(pdev, sizeof(u32) * index, 
                                 g_pcie_rc_info->ep_config_space[func_num][index],
                                 retry, func_num);
    }
}

int pci_read_ep_id_cfg(struct pci_dev *pdev, u32 func_num)
{
    u32 ep_id_info = 0;
    u32 retry_times = 0;
    int ret;
    while (retry_times < PCIE_READ_MAX_RETRY_TIMES) {
        ret = pci_read_config_dword(pdev, 0, &ep_id_info);
        if (ep_id_info == PCIE_IOT_EP_ID) {
            return 0;
        } else {
            printk(KERN_ERR "iot func_num %d device read invalid ep id 0x%x, retry times: %d\n", func_num,
                   ep_id_info, retry_times);
            if (ret) {
                show_pcie_config_error(pdev, func_num, 0, ep_id_info, ret);
            }
        }
        retry_times++;
        mdelay(1);
    }
    printk(KERN_ERR "iot func_num %d device read invalid ep id failed, ret -1\n", func_num);
    return -1;
}

void pci_restore_iot_cfg_space(struct pci_dev *pdev, u32 func_num)
{
    (void)pci_read_ep_id_cfg(pdev, func_num);
    // if cfg read err, use pcie dump func to debug
    pci_restore_iot_space_range(pdev, 0xa, 0xf, 0x0, func_num);
    /* Restore BARs before the command register. */
    pci_restore_iot_space_range(pdev, 0x4, 0x9, 0xa, func_num);
    pci_restore_iot_space_range(pdev, 0x0, 0x3, 0x0, func_num);
    return;
}

int bsp_pcie_restore_all_ep_cfg(void)
{
    int i = 0;
    int func_num;
    int ret;
    for (func_num = 0; func_num < g_pcie_rc_info->valid_ep_func_num; func_num++) {
        if (pci_set_power_state(g_pcie_rc_info->iot_multi_ep[func_num], PCI_D0)) {
            PCIE_RC_PR_ERR("pci set to D0 failed, func_num: %d", i);
            return -1;
        }

        pci_restore_iot_cfg_space(g_pcie_rc_info->iot_multi_ep[func_num], func_num);

        if (pci_enable_device(g_pcie_rc_info->iot_multi_ep[func_num])) {
            PCIE_RC_PR_ERR("pci_enable_device fail, func_num: %d", func_num);
            return -1;
        }

        pci_set_master(g_pcie_rc_info->iot_multi_ep[func_num]);
    }

    pcie_dump_ep_cfg_after_restore();
    ret = pcie_check_restored_config_space_dbg();
    if (ret) {
        PCIE_RC_PR_ERR("pcie restore config space not same error detected!");
    }

    for (i = 0; i < PCIE_EP_MSI_NUM; i++) {
        if (g_pcie_rc_info->irq_chn[i].handler) {
            iot_pcie_rc_msi_enable(i);
        }
    }

    return 0;
}

/* ---------------------------------------------------------- */
/* following functions are reserved for ep->rc msi/multi_msi in the future */
/* multi_msi can support remove dma isr, and ep to rc msi can be allocated to user standalone */
int pcie_modem_dma_common_init(void)
{
    unsigned long irq_flags = 0;
    u32 msi_data_value;
    struct rc_pcie_info *info = g_pcie_rc_info;

    if (info->ep_dma_base_virt_addr == NULL) {
        PCIE_RC_PR_ERR("dma base addr error, init fail");
        return -1;
    }

    spin_lock_irqsave(&info->spinlock, irq_flags);

    /* DMA Read Engine Enable */
    writel(0x01U, info->ep_dma_base_virt_addr + 0x02CU);

    /* DMA Read IMWr Done Addr */
    writel(info->rc_dma_msi_info.msi_addr_l, info->ep_dma_base_virt_addr + 0x0CCU);
    writel(info->rc_dma_msi_info.msi_addr_h, info->ep_dma_base_virt_addr + 0x0D0U);
    /* DMA Read IMWr Abort Addr */
    writel(info->rc_dma_msi_info.msi_addr_l, info->ep_dma_base_virt_addr + 0x0D4U);
    writel(info->rc_dma_msi_info.msi_addr_h, info->ep_dma_base_virt_addr + 0x0D8U);
    /* DMA Read Channel 0 ~ 7 IMWr Data, rc write data -> ep read msi data */
    msi_data_value = (info->rc_dma_msi_info.msi_write_data << PCIE_UPPER_U16_SHIFT) | \
        (info->rc_dma_msi_info.msi_write_data);
    writel(msi_data_value, info->ep_dma_base_virt_addr + 0x0DCU);  // channel 1 (31:16), channel 0 (15:0)
    writel(msi_data_value, info->ep_dma_base_virt_addr + 0x0E0U);  // channel 3 (31:16), channel 2 (15:0)
    writel(msi_data_value, info->ep_dma_base_virt_addr + 0x0E4U);  // channel 5 (31:16), channel 4 (15:0)
    writel(msi_data_value, info->ep_dma_base_virt_addr + 0x0E8U);  // channel 7 (31:16), channel 6 (15:0)

    /* DMA Write Engine Enable */
    writel(0x01U, info->ep_dma_base_virt_addr + 0x00CU);

    /* DMA Write IMWr Done Addr */
    writel(info->rc_dma_msi_info.msi_addr_l, info->ep_dma_base_virt_addr + 0x060U);
    writel(info->rc_dma_msi_info.msi_addr_h, info->ep_dma_base_virt_addr + 0x064U);
    /* DMA Write IMWr Abort Addr */
    writel(info->rc_dma_msi_info.msi_addr_l, info->ep_dma_base_virt_addr + 0x068U);
    writel(info->rc_dma_msi_info.msi_addr_h, info->ep_dma_base_virt_addr + 0x06CU);
    /* DMA Write Channel 0~7 IMWr Data, rc read data -> ep write msi data */
    msi_data_value = (info->rc_dma_msi_info.msi_read_data << PCIE_UPPER_U16_SHIFT) | \
        (info->rc_dma_msi_info.msi_read_data);
    writel(msi_data_value, info->ep_dma_base_virt_addr + 0x070U);  // channel 1 (31:16), channel 0 (15:0)
    writel(msi_data_value, info->ep_dma_base_virt_addr + 0x074U);  // channel 3 (31:16), channel 2 (15:0)
    writel(msi_data_value, info->ep_dma_base_virt_addr + 0x078U);  // channel 5 (31:16), channel 4 (15:0)
    writel(msi_data_value, info->ep_dma_base_virt_addr + 0x07CU);  // channel 7 (31:16), channel 6 (15:0)

    spin_unlock_irqrestore(&info->spinlock, irq_flags);

    return 0;
}

/* functions for ep->rc remote msi int isr */
static irqreturn_t pcie_dma_write_int_isr(int irq, void *dev_info)
{
    unsigned long irq_flags = 0;
    unsigned int write_status;
    unsigned int done_status;
    unsigned int abort_status;
    unsigned int chn_id;
    struct rc_pcie_info *info = g_pcie_rc_info;

    spin_lock_irqsave(&info->spinlock, irq_flags);

    write_status = info->dma_isr_flags[PCIE_DMA_DIRECTION_WRITE];
    info->dma_isr_flags[PCIE_DMA_DIRECTION_WRITE] &= (~write_status);  // clear write flag

    spin_unlock_irqrestore(&info->spinlock, irq_flags);

    if (write_status) {
        done_status = write_status & 0x0000FFFF;
        abort_status = (write_status >> 0x10) & 0x0000FFFF;
        for (chn_id = 0; chn_id < PCIE_DMA_CHN_NUM; chn_id++) {
            if ((write_status & (1U << chn_id)) == 0) {
                continue;
            }

            if (info->dma_int_info[chn_id][PCIE_DMA_DIRECTION_WRITE].dma_int_callback) {
                info->dma_int_info[chn_id][PCIE_DMA_DIRECTION_WRITE].dma_int_callback(
                    PCIE_DMA_DIRECTION_WRITE, abort_status,
                    info->dma_int_info[chn_id][PCIE_DMA_DIRECTION_WRITE].dma_int_callback_args);
            }
            info->dma_channel_state &= (~(1U << chn_id));
        }
    }

    return IRQ_HANDLED;
}

static irqreturn_t pcie_dma_read_int_isr(int irq, void *dev_info)
{
    unsigned long irq_flags = 0;
    unsigned int read_status;
    unsigned int done_status;
    unsigned int abort_status;
    unsigned int chn_id;
    struct rc_pcie_info *info = g_pcie_rc_info;

    spin_lock_irqsave(&info->spinlock, irq_flags);

    read_status = info->dma_isr_flags[PCIE_DMA_DIRECTION_READ];
    info->dma_isr_flags[PCIE_DMA_DIRECTION_READ] &= (~read_status);  // clear read flag

    spin_unlock_irqrestore(&info->spinlock, irq_flags);

    if (read_status) {
        done_status = read_status & 0x0000FFFF;
        abort_status = (read_status >> 0x10) & 0x0000FFFF;

        for (chn_id = 0; chn_id < PCIE_DMA_CHN_NUM; chn_id++) {
            if ((read_status & (1U << chn_id)) == 0) {
                continue;
            }

            if (info->dma_int_info[chn_id][PCIE_DMA_DIRECTION_READ].dma_int_callback) {
                info->dma_int_info[chn_id][PCIE_DMA_DIRECTION_READ].dma_int_callback(
                    PCIE_DMA_DIRECTION_READ, abort_status,
                    info->dma_int_info[chn_id][PCIE_DMA_DIRECTION_READ].dma_int_callback_args);
            }
            info->dma_channel_state &= (~(1U << chn_id));
        }
    }

    return IRQ_HANDLED;
}

/* functions for ep->rc multi_msi request */
int bsp_pcie_rc_multi_msi_request(enum pcie_ep_to_rc_msi_id id, irq_handler_t handler, const char *name, void *data)
{
    u32 msi_irq_num = g_pcie_rc_info->iot_ep->irq + id;

    if (id >= PCIE_EP_MSI_MAX_ID || handler == NULL) {
        PCIE_RC_PR_ERR("invalid input, checkinput parameter");
        return -1;
    }

    if (g_pcie_rc_info->irq_chn[id].handler) {
        PCIE_RC_PR_ERR("pcie ep to rc msi [%d] has handler", id);
        return -1;
    }

    g_pcie_rc_info->irq_chn[id].handler = handler;
    g_pcie_rc_info->irq_chn[id].irq_num = msi_irq_num;
    g_pcie_rc_info->irq_chn[id].name = name;
    g_pcie_rc_info->irq_chn[id].irq_data = data;

    if (request_irq(msi_irq_num, handler, 0, g_pcie_rc_info->irq_chn[id].name, data)) {
        PCIE_RC_PR_ERR("request_irq fail, msi irq = %u", msi_irq_num);
        return -1;
    }
    return 0;
}

/* functions for rc remote dma isr */
int bsp_pcie_rc_dma_isr_register(enum pcie_dma_chn_id chn, u32 direction, pcie_callback call_back, void *arg)
{
    struct rc_pcie_info *info = g_pcie_rc_info;

    if ((chn > PCIE_DMA_CHN_NUM) || (direction > PCIE_DMA_DIRECTION_WRITE) || (call_back == NULL)) {
        return -1;
    }

    info->dma_int_info[chn][direction].dma_int_callback = call_back;
    info->dma_int_info[chn][direction].dma_int_callback_args = arg;
    if (direction == PCIE_DMA_DIRECTION_WRITE) {
        info->dma_isr_flags[PCIE_DMA_DIRECTION_WRITE] |= (1 << chn);
    } else {
        info->dma_isr_flags[PCIE_DMA_DIRECTION_READ] |= (1 << chn);
    }

    return 0;
}

/* functions for ep to rc multi msi get msi addr and data info */
u32 pcie_get_msi_irq_count(u32 irq)
{
    struct irq_desc *desc = irq_to_desc(irq);
    return desc->irq_count;
}

static void pcie_read_msi_msg(struct msi_desc *entry, struct msi_msg *msg)
{
    struct pci_dev *dev = msi_desc_to_pci_dev(entry);
    void __iomem *base;
    int pos;
    u16 data;

    if (dev == NULL) {
        return;
    }

    pos = dev->msi_cap;

    BUG_ON(dev->current_state != PCI_D0);

    if (entry->msi_attrib.is_msix) {
        base = entry->mask_base + entry->msi_attrib.entry_nr * PCI_MSIX_ENTRY_SIZE;
        msg->address_lo = readl(base + PCI_MSIX_ENTRY_LOWER_ADDR);
        msg->address_hi = readl(base + PCI_MSIX_ENTRY_UPPER_ADDR);
        msg->data = readl(base + PCI_MSIX_ENTRY_DATA);
    } else {
        pci_read_config_dword(dev, pos + PCI_MSI_ADDRESS_LO, &msg->address_lo);
        if (entry->msi_attrib.is_64) {
            pci_read_config_dword(dev, pos + PCI_MSI_ADDRESS_HI, &msg->address_hi);
            pci_read_config_word(dev, pos + PCI_MSI_DATA_64, &data);
        } else {
            msg->address_hi = 0;
            pci_read_config_word(dev, pos + PCI_MSI_DATA_32, &data);
        }
        msg->data = data;
    }
}

int pcie_get_dma_msi_info(struct pci_dev *pdev)
{
    struct msi_desc *msidesc = NULL;
    struct msi_msg msg = {0};

    if (pdev == NULL) {
        PCIE_RC_PR_ERR("pci device not exit");
        return -1;
    }

    for_each_pci_msi_entry(msidesc, pdev)
    {
        pcie_read_msi_msg(msidesc, &msg);
    };

    if (msg.address_lo) {
        g_pcie_rc_info->rc_dma_msi_info.msi_addr_h = msg.address_hi;
        g_pcie_rc_info->rc_dma_msi_info.msi_addr_l = msg.address_lo;
        g_pcie_rc_info->rc_dma_msi_info.msi_write_data = msg.data + PCIE_EP_MSI_DMA_WRITE;
        g_pcie_rc_info->rc_dma_msi_info.msi_read_data = msg.data + PCIE_EP_MSI_DMA_READ;
    } else {
        PCIE_RC_PR_ERR("invalid pci dev msi info");
        return -1;
    }
    return 0;
}

/* functions for ep->rc msi/multi_msi init */
int pcie_ep_to_rc_msi_init(void)
{
    int ret;

    // use pci_enable_msi_exact first before kernel 4.14
    ret = bsp_pcie_rc_msi_request(PCIE_EP_MSI_DMA_WRITE, pcie_dma_write_int_isr, "rc_dma_write_msi", NULL);
    if (ret) {
        PCIE_RC_PR_ERR("pcie request remote dma write msi failed, id: %d", PCIE_EP_MSI_DMA_WRITE);
        return -1;
    }

    ret = bsp_pcie_rc_msi_request(PCIE_EP_MSI_DMA_READ, pcie_dma_read_int_isr, "rc_dma_read_msi", NULL);
    if (ret) {
        PCIE_RC_PR_ERR("pcie request remote dma read msi failed, id: %d", PCIE_EP_MSI_DMA_READ);
        return -1;
    }

    ret = pcie_get_dma_msi_info(g_pcie_rc_info->iot_ep);
    if (ret) {
        PCIE_RC_PR_ERR("fail to get dma msi info");
        return -1;
    }

    return 0;
}

u32 pcie_get_base_msi_irq_num(void)
{
    u32 base_msi_irq = g_pcie_rc_info->iot_ep->irq;
    return base_msi_irq;
}

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
    printk(KERN_ERR "Driver path: %s\n", __FILE__);
    printk(KERN_ERR "EP Vendor ID:        0x%04X\n", PCIE_IOT_VENDOR_ID);
    printk(KERN_ERR "EP Device ID:        0x%04X\n", PCIE_IOT_DEVICE_ID);
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

    printk(KERN_ERR "PCIE Resume First User Flag: %d\n", g_pcie_rc_flag.first_user_flag);
    printk(KERN_ERR "PCIE Resume First User Id: 0x%x\n", g_pcie_rc_flag.first_user_id);
    printk(KERN_ERR "**************************** PCIE RC INFO OVER *************************** \n");

    return 0;
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
int bsp_pcie_rc_get_pm_status(void)
{
    return 0;
}

int bsp_pcie_rc_vote_lock_timeout(enum pcie_user_id user_id, long jiffies)
{
    return 0;
}
#endif
