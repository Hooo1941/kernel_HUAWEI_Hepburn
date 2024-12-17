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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/atomic.h>
#include <linux/interrupt.h>
#include <linux/of_device.h>
#include <securec.h>
#include <bsp_pcie.h>
#include "pcie_cdev.h"
#include "pcie_cdev_desc.h"
#include "pcie_cdev_ep.h"
#include "pcie_cdev_bar.h"
#include "pcie_cdev_dbg.h"

static u64 g_pcdev_dma_mask = (u64)(-1);
char *g_pcdev_ep_irq_name = "pcdev_irq";

atomic_t g_pcdev_vote_cnt[VOTE_MODE_BOTTOM];
atomic_t g_pcdev_vote_fail_cnt[VOTE_MODE_BOTTOM];
atomic_t g_pcdev_unvote_cnt[VOTE_MODE_BOTTOM];

void pcdev_vote_dbg(void)
{
    int i;
    for (i = 0; i < VOTE_MODE_BOTTOM; i++) {
        PCDEV_ERR("mode(%d) vote_cnt:%d, fail_cnt:%d, unvote_cnt:%d\n", i, atomic_read(&g_pcdev_vote_cnt[i]),
                  atomic_read(&g_pcdev_vote_fail_cnt[i]), atomic_read(&g_pcdev_unvote_cnt[i]));
    }
}

static int pcie_cdev_vote_lock_ep(int mode)
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
    ret = bsp_pcie_ep_vote_lock(PCIE_USER_CHAR_DEV, 1);
    if (ret) {
        atomic_inc(&g_pcdev_vote_fail_cnt[mode]);
    }
    return ret;
}

static int pcie_cdev_vote_unlock_ep(int mode)
{
    atomic_inc(&g_pcdev_unvote_cnt[mode]);
    return bsp_pcie_ep_vote_unlock(PCIE_USER_CHAR_DEV);
}

static int pcie_cdev_vote_try_lock_ep(int mode)
{
    int ret;
    atomic_inc(&g_pcdev_vote_cnt[mode]);
    ret = bsp_pcie_ep_vote_lock(PCIE_USER_CHAR_DEV, 0);
    if (ret) {
        atomic_inc(&g_pcdev_vote_fail_cnt[mode]);
    }
    return ret;
}

static irqreturn_t pcie_cdev_irq_handle_ep(int i, void *data)
{
    struct pcdev_ctx *pcdev_ctx = get_pcdev_ctx();
    PCDEV_TRACE("in\n");
    pcdev_ctx->irq_handler();
    return IRQ_HANDLED;
}

static int pcie_cdev_send_irq_ep(void)
{
    return bsp_pcie_ep_send_msi(PCIE_EP_MSI_CHAR_DEV);
}

static int pcie_cdev_ep_init(void)
{
    int ret;
    struct pcdev_ctx *pcdev_ctx = get_pcdev_ctx();
    PCDEV_TRACE("in\n");

    ret = bsp_pcie_ep_msi_request(PCIE_RC_MSI_CHAR_DEV, pcie_cdev_irq_handle_ep,
        g_pcdev_ep_irq_name, NULL);
    if (ret) {
        PCDEV_ERR("bsp_pcie_ep_msi_request  fail\n");
        return ret;
    }

    pcdev_ctx->send_irq = pcie_cdev_send_irq_ep;
    pcdev_ctx->dma_transfer = bsp_pcie_ep_dma_transfer;

    return 0;
}

static int pcie_cdev_ep_cb(u32 usr_id, u32 cb_id, void *callback_args)
{
    int ret = 0;
    int i;
    struct pcdev_ctx *pcdev_ctx = get_pcdev_ctx();
    switch (cb_id) {
        case PCIE_EP_CB_BAR_CONFIG:
            ret = pcie_cdev_bar_init();
            break;
        case PCIE_EP_CB_PCIE_INIT_DONE:
            for (i = 0; i < VOTE_MODE_BOTTOM; i++) {
                atomic_set(&g_pcdev_vote_cnt[i], 0);
                atomic_set(&g_pcdev_vote_fail_cnt[i], 0);
                atomic_set(&g_pcdev_unvote_cnt[i], 0);
            }
            ret = pcdev_init_cb();
            break;
        case PCIE_EP_CB_EXIT:
            pcdev_exit();
            break;

        case PCIE_EP_CB_PRE_LINKUP:
            if (pcdev_ctx->diag_mode == 1) {
                bsp_pcie_data_volume_set_atomic(PCIE_USER_CHAR_DEV, PCIE_VOLUME_SPEED_SET);
            }
            break;

        default:
            break;
    }
    return ret;
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
    { .compatible = "iot,pcdev_ep" },
    {},
};

static struct platform_driver g_pcdev_pltfm_driver = {
    .probe = pcdev_pltform_probe,
    .driver = {
        .name = "pcdev",
        .of_match_table = g_pcdev_match,
    },
};

int pcie_cdev_platform_ep_init(void)
{
    int ret;
    struct pcie_callback_info pcie_cdev_callback_info;
    struct pcdev_ctx *pcdev_ctx = get_pcdev_ctx();

    pcdev_ports_ctx_clear();

    pcdev_ctx->work_mode = PCIE_EP;
    pcdev_ctx->pcie_id = 0;
    pcdev_ctx->msg_level = PCDEV_LEVEL_ERR | PCDEV_LEVEL_INFO;
    pcdev_ctx->print_port = 1;
    pcdev_ctx->pcdev_hw_init = pcie_cdev_ep_init;
    pcdev_ctx->pcdev_vote_lock = pcie_cdev_vote_lock_ep;
    pcdev_ctx->pcdev_vote_unlock = pcie_cdev_vote_unlock_ep;
    pcdev_ctx->pcdev_vote_try_lock = pcie_cdev_vote_try_lock_ep;
    pcdev_ctx->get_curtime = bsp_ab_sync_slice_getcurtime;

    PCDEV_ERR("init begin\n");
    pcdev_dump_init();

    ret = pcdev_initwork_init();
    if (ret) {
        PCDEV_ERR("pcdev_initwork_init err\n");
        return ret;
    }

    pcie_cdev_callback_info.callback = pcie_cdev_ep_cb;
    pcie_cdev_callback_info.callback_args = NULL;

    ret = bsp_pcie_ep_cb_register(PCIE_USER_CHAR_DEV, &pcie_cdev_callback_info);
    if (ret) {
        PCDEV_ERR("pcie cb regiest fail\n");
    }

    ret = platform_driver_register(&g_pcdev_pltfm_driver);
    if (ret) {
        PCDEV_ERR("platform_driver_register err\n");
        return ret;
    }

    return ret;
}

postcore_initcall_sync(pcie_cdev_platform_ep_init);

int pcdev_set_diag_mode_ep(unsigned long arg)
{
    struct pcdev_ctx *pcdev_ctx = NULL;
    pcdev_diag_mode_e diag_mode;

    if (arg == 0) {
        PCDEV_ERR("pcdev_set_diag_mode_ep invalid param\n");
        return -EFAULT;
    }

    pcdev_ctx = get_pcdev_ctx();

    diag_mode = *((pcdev_diag_mode_e *)((uintptr_t)arg));
    if (diag_mode == PCDEV_DIAG_DISCONNECT) {
        bsp_pcie_data_volume_set(PCIE_USER_CHAR_DEV, 0);
        bsp_pcie_app_clk_release(PCIE_USER_CHAR_DEV);
        pcdev_ctx->diag_mode = 0;
    } else if (diag_mode == PCDEV_DIAG_CONNECT) {
        bsp_pcie_data_volume_set(PCIE_USER_CHAR_DEV, PCIE_VOLUME_SPEED_SET);
        bsp_pcie_app_clk_req(PCIE_USER_CHAR_DEV);
        pcdev_ctx->diag_mode = 1;
    }

    return 0;
}
