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

#include <linux/pci.h>
#include <linux/dma-mapping.h>
#include <linux/of_device.h>
#include <linux/atomic.h>
#include <linux/interrupt.h>
#include <linux/syscore_ops.h>
#include <securec.h>
#include "pcie_cdev.h"
#include "pcie_cdev_rc.h"
#include "pcie_cdev_dbg.h"

static u64 g_pcdev_dma_mask = (u64)(-1);
char *g_pcdev_rc_irq_name = "pcdev_irq";
atomic_t g_pcdev_vote_cnt[VOTE_MODE_BOTTOM];
atomic_t g_pcdev_vote_fail_cnt[VOTE_MODE_BOTTOM];
atomic_t g_pcdev_unvote_cnt[VOTE_MODE_BOTTOM];

#define PM_TYPE_SUSPEND 0
#define PM_TYPE_RESUME 1
#define PM_INFO_SIZE_PER_PORT 30
#define PRINT_BUFF_SIZE (PM_INFO_SIZE_PER_PORT * PCIE_CDEV_COUNT)
static char g_pmprint_buf[PRINT_BUFF_SIZE];

void pcdev_vote_dbg(void)
{
    int i;
    for (i = 0; i < VOTE_MODE_BOTTOM; i++) {
        PCDEV_ERR("mode(%d) vote_cnt:%d, fail_cnt:%d, unvote_cnt:%d\n", i, atomic_read(&g_pcdev_vote_cnt[i]),
                  atomic_read(&g_pcdev_vote_fail_cnt[i]), atomic_read(&g_pcdev_unvote_cnt[i]));
    }
}

static int pcie_cdev_vote_lock_rc(int mode)
{
    int ret;
    struct pcdev_ctx *pcdev_ctx = get_pcdev_ctx();
    if (in_interrupt()) {
        PCDEV_ERR("can't send buf in atomic\n");
        WARN_ON_ONCE(1);
        pcdev_ctx->vote_in_interrupt++;
        return -EACCES;
    }

    atomic_inc(&g_pcdev_vote_cnt[mode]);
    ret = bsp_pcie_rc_vote_lock(PCIE_USER_CHAR_DEV, 1);
    if (ret) {
        atomic_inc(&g_pcdev_vote_fail_cnt[mode]);
    }
    return ret;
}

static int pcie_cdev_vote_unlock_rc(int mode)
{
    atomic_inc(&g_pcdev_unvote_cnt[mode]);
    return bsp_pcie_rc_vote_unlock(PCIE_USER_CHAR_DEV);
}

static int pcie_cdev_vote_try_lock_rc(int mode)
{
    int ret;
    atomic_inc(&g_pcdev_vote_cnt[mode]);
    ret = bsp_pcie_rc_vote_lock(PCIE_USER_CHAR_DEV, 0);
    if (ret) {
        atomic_inc(&g_pcdev_vote_fail_cnt[mode]);
    }
    return ret;
}

int pcdev_rc_port_desc_match(unsigned int port_n)
{
    union pcie_cdev_map *desc = NULL;
    union pcie_cdev_map *desc_addr = NULL;
    struct pcdev_ctx *pcdev_ctx = get_pcdev_ctx();
    struct pcdev_ports_desc *ports_desc = NULL;
    struct pcie_cdev_name_type_tbl *type_table = get_pcdev_type_table();
    struct pcie_cdev_port_manager *pcdev_ports = get_pcdev_ports();
    unsigned int i, port_num;

    ports_desc = (struct pcdev_ports_desc *)pcdev_ctx->virt_addr;
    desc_addr = (union pcie_cdev_map *)(pcdev_ctx->virt_addr + sizeof(struct pcdev_ports_desc));
    port_num = ports_desc->port_num;

    for (i = 0; i < port_num; i++) {
        desc = desc_addr + i;
        if (desc->rc.port_id == type_table[port_n].type) {
            pcdev_ports[port_n].desc = desc;
            break;
        }
    }

    if (i >= ports_desc->port_num) {
        return -EACCES;
    }

    return 0;
}

static int pcie_cdev_send_irq_rc(void)
{
    return bsp_pcie_rc_send_msi(PCIE_RC_MSI_CHAR_DEV);
}

static irqreturn_t pcie_cdev_irq_handler_rc(int irq, void *dev_info)
{
    struct pcdev_ctx *pcdev_ctx = get_pcdev_ctx();
    PCDEV_TRACE("in\n");
    return pcdev_ctx->irq_handler();
}

static int pcie_cdev_rc_init(void)
{
    int ret;
    struct pcdev_ctx *pcdev_ctx = get_pcdev_ctx();
    PCDEV_TRACE("in\n");

    pcdev_ctx->phys_addr = (void *)bsp_pcie_rc_get_bar_addr(PCIE_BAR_CHAR_DEV);
    if (pcdev_ctx->phys_addr == NULL) {
        PCDEV_ERR("cannot resource\n");
        ret = -ENODEV;
        return ret;
    }

    pcdev_ctx->buffer_size = bsp_pcie_rc_get_bar_size(PCIE_BAR_CHAR_DEV);
    if (pcdev_ctx->buffer_size < MIN_IO_SIZE) {
        PCDEV_ERR("Invalid PCI region size, aborting\n");
        ret = -ENODEV;
        return ret;
    }

    pcdev_ctx->virt_addr = ioremap((phys_addr_t)pcdev_ctx->phys_addr, pcdev_ctx->buffer_size);
    if (pcdev_ctx->virt_addr == NULL) {
        PCDEV_ERR("cannot map\n");
        ret = -ENODEV;
        return ret;
    }

    ret = bsp_pcie_rc_msi_request(PCIE_EP_MSI_CHAR_DEV, pcie_cdev_irq_handler_rc,
        g_pcdev_rc_irq_name, NULL);
    if (ret) {
        PCDEV_ERR("irq request fail\n");
        ret = -ENODEV;
        return ret;
    }

    ret = bsp_pcie_rc_msi_enable(PCIE_EP_MSI_CHAR_DEV);
    if (ret) {
        PCDEV_ERR("irq enable fail\n");
        ret = -ENODEV;
        return ret;
    }

    pcdev_ctx->send_irq = pcie_cdev_send_irq_rc;

    PCDEV_TRACE("finish \n");

    return ret;
}

static void pcie_cdev_rc_exit(void)
{
    struct pcdev_ctx *pcdev_ctx = get_pcdev_ctx();
    PCDEV_TRACE("in\n");

    bsp_pcie_rc_msi_free(PCIE_EP_MSI_CHAR_DEV);
    if (pcdev_ctx->virt_addr != NULL) {
        iounmap(pcdev_ctx->virt_addr);
    }
    pcdev_ctx->phys_addr = 0;
    pcdev_ctx->buffer_size = 0;
    pcdev_ctx->virt_addr = 0;

    PCDEV_TRACE("finish \n");
    return;
}

static int pcdev_pltform_probe(struct platform_device *pdev)
{
    struct pcdev_ctx *pcdev_ctx = get_pcdev_ctx();
    PCDEV_TRACE("in\n");
    dma_set_mask_and_coherent(&pdev->dev, g_pcdev_dma_mask);
    pcdev_ctx->pdev = pdev;
    return 0;
}

static const struct of_device_id g_pcdev_match[] = {
    { .compatible = "iot,pcdev_app" },
    {},
};

static struct platform_driver g_pcdev_pltfm_driver = {
    .probe = pcdev_pltform_probe,
    .driver = {
        .name = "pcdev",
        .of_match_table = g_pcdev_match,
    },
};

static void pcie_cdev_rc_pm_info(int pm_type)
{
    int ret;
    unsigned int cnt = 0;
    int i;

    ret = memset_s(g_pmprint_buf, PRINT_BUFF_SIZE, 0, PRINT_BUFF_SIZE);
    if (ret) {
        PCDEV_ERR("memset_s failed, line: %d \n", __LINE__);
    }

    ret = snprintf_s((char *)g_pmprint_buf, (size_t)PRINT_BUFF_SIZE, (size_t)(PRINT_BUFF_SIZE - 1),
                      "[C SR][PCDEV]( %d )vote port:", pm_type);
    if (ret < 0) {
        PCDEV_ERR("snprintf_s failed, line: %d \n", __LINE__);
        return;
    }
    cnt += ret;

    for (i = 0; i < PCIE_CDEV_COUNT; i++) {
        ret = snprintf_s((char *)g_pmprint_buf + cnt, (size_t)(PRINT_BUFF_SIZE - cnt),
                          (size_t)(PRINT_BUFF_SIZE - cnt - 1), "[%d]:%lld, ", i,
                          g_pcdev_ctx.vote_dbg[i].vote_port);
        if (ret < 0) {
            PCDEV_ERR("snprintf_s failed, line: %d \n", __LINE__);
            return;
        }
        cnt += ret;
    }

    ret = snprintf_s((char *)g_pmprint_buf + cnt, (size_t)(PRINT_BUFF_SIZE - cnt),
                      (size_t)(PRINT_BUFF_SIZE - cnt - 1), "\n");
    if (ret < 0) {
        PCDEV_ERR("snprintf_s failed, line: %d \n", __LINE__);
        return;
    }
    cnt += ret;
    printk(KERN_INFO "%s", g_pmprint_buf);
}

static int pcie_cdev_rc_cb(u32 usr_id, u32 cb_id, void *callback_args)
{
    int ret = 0;
    int i;

    switch (cb_id) {
        case PCIE_RC_CB_ENUM_DONE:
            for (i = 0; i < VOTE_MODE_BOTTOM; i++) {
                atomic_set(&g_pcdev_vote_cnt[i], 0);
                atomic_set(&g_pcdev_vote_fail_cnt[i], 0);
                atomic_set(&g_pcdev_unvote_cnt[i], 0);
            }
            ret = pcdev_init_cb();
            break;
        case PCIE_RC_CB_SUSPEND:
            pcie_cdev_rc_pm_info(PM_TYPE_SUSPEND);
            break;
        case PCIE_RC_CB_RESUME:
            pcie_cdev_rc_pm_info(PM_TYPE_RESUME);
            break;
        case PCIE_RC_CB_EXIT:
            pcdev_exit();
            break;
        default:
            break;
    }
    return ret;
}

static int pcdev_clr_vote_flag(void)
{
    struct pcdev_ctx *pcdev_ctx = get_pcdev_ctx();
    pcdev_ctx->vote_flag = 0xFFFF;
    return 0;
}

static void pcdev_set_vote_flag(void)
{
    struct pcdev_ctx *pcdev_ctx = get_pcdev_ctx();
    pcdev_ctx->vote_flag = VOTE_FLAG;
}

static struct syscore_ops g_pcdev_ops = {
    .suspend = pcdev_clr_vote_flag,
    .resume = pcdev_set_vote_flag,
};

void pcdev_bsp_slice_getcurtime(u64 *pcurtime)
{
    int ret;
    ret = bsp_slice_getcurtime(pcurtime);
    if (ret) {
        PCDEV_ERR("bsp_slice_getcurtime err\n");
    }
    return;
}

int pcie_cdev_platform_rc_init(void)
{
    int ret;
    struct pcie_callback_info pcie_cdev_callback_info = {0};
    struct pcdev_ctx *pcdev_ctx = get_pcdev_ctx();
    printk(KERN_ERR "[pcdev]pcie_cdev_platform_rc_init in\n");
    pcdev_ports_ctx_clear();
    pcdev_ctx_clear();
    pcdev_ctx->work_mode = PCIE_RC;
    pcdev_ctx->pcie_id = 0;
    pcdev_ctx->msg_level = PCDEV_LEVEL_ERR | PCDEV_LEVEL_INFO;
    pcdev_ctx->print_port = 1;
    pcdev_ctx->pcdev_hw_init = pcie_cdev_rc_init;
    pcdev_ctx->pcdev_hw_exit = pcie_cdev_rc_exit;
    pcdev_ctx->pcdev_vote_lock = pcie_cdev_vote_lock_rc;
    pcdev_ctx->pcdev_vote_unlock = pcie_cdev_vote_unlock_rc;
    pcdev_ctx->pcdev_vote_try_lock = pcie_cdev_vote_try_lock_rc;
    pcdev_ctx->get_curtime = pcdev_bsp_slice_getcurtime;
    pcdev_ctx->pcie_first_user = bsp_is_pcie_first_user;

    PCDEV_ERR("init begin\n");
    pcdev_dump_init();

    ret = pcdev_initwork_init();
    if (ret) {
        PCDEV_ERR("pcdev_initwork_init err\n");
        return ret;
    }

    pcie_cdev_callback_info.callback = pcie_cdev_rc_cb;
    pcie_cdev_callback_info.callback_args = NULL;

    ret = bsp_pcie_rc_cb_register(PCIE_USER_CHAR_DEV, &pcie_cdev_callback_info);
    if (ret) {
        PCDEV_ERR("pcie cb regiest fail\n");
    }

    ret = platform_driver_register(&g_pcdev_pltfm_driver);
    if (ret) {
        PCDEV_ERR("platform_driver_register err\n");
        return ret;
    }

    register_syscore_ops(&g_pcdev_ops);

    PCDEV_ERR("pcie_cdev_platform_rc_init finish\n");
    return ret;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
device_initcall(pcie_cdev_platform_rc_init);
#endif
