/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#include "bsp_reset.h"

#include "pfa_hal_desc.h"
#include "pfa_hal_reg.h"

#include "pfa.h"
#include "pfa_dbg.h"
#include "pfa_port.h"
#include "pfa_desc.h"

#define PFA_PHONE_CRG_BASE 0x40285000
#define PFA_PHONE_CRG_LEN 0x1000

struct pfa_queue_stash g_pfa_queue;
unsigned int g_pfa_reg[PFA_GENERAL_REG_NUM][STASH_REG_BUTTON] = {
    // pfa globe
    { PFA_BLACK_WHITE, 0 },
    { PFA_PORT_LMTTIME, 0 },
    { PFA_ETH_MINLEN, 0 },
    { PFA_ETH_MAXLEN, 0 },
    { PFA_GLB_DMAC_ADDR_L, 0 },
    { PFA_GLB_DMAC_ADDR_H, 0 },
    { PFA_GLB_SMAC_ADDR_L, 0 },
    { PFA_GLB_SMAC_ADDR_H, 0 },
    // pfa done
    { PFA_RDQ_FULL_INTA_MASK, 0 },
    { PFA_RD_DONE_INTA_MASK, 0 },
    { PFA_TD_DONE_INTA_MASK, 0 },
    { PFA_ADQ_EMPTY_INTA_MASK, 0 },
    { PFA_INTA_INTERVAL, 0 },

    // pfa eng(hash)
    { PFA_HASH_BADDR_L, 0 },
    { PFA_HASH_BADDR_H, 0 },
    { PFA_HASH_ZONE, 0 },
    { PFA_HASH_RAND, 0 },
    { PFA_HASH_L3_PROTO, 0 },
    { PFA_HASH_WIDTH, 0 },
    { PFA_HASH_DEPTH, 0 },
    { PFA_AGING_TIME, 0 },
    { PFA_GMAC_PADDING_IDEN_EN, 0 },

    // pfa pack
    { PFA_PACK_MAX_TIME, 0 },
    { PFA_PACK_ADDR_CTRL, 0 },
    { PFA_PACK_REMAIN_LEN, 0 },
    { PFA_UNPACK_CTRL, 0 },
    { PFA_UNPACK_MAX_LEN, 0 },
    { PFA_PACK_REGION_DIV, 0 },

    // pfa ad
    { PFA_ADQ0_BADDR_L, 0 },
    { PFA_ADQ0_BADDR_H, 0 },
    { PFA_ADQ0_BADDR_L, 0 },
    { PFA_ADQ0_BADDR_H, 0 },
    { PFA_ADQ0_SIZE, 0 },
    { PFA_ADQ0_RPTR_UPDATE_ADDR_L, 0 },
    { PFA_ADQ0_RPTR_UPDATE_ADDR_H, 0 },
    { PFA_ADQ0_RPTR_UPDATE_ADDR_L, 0 },
    { PFA_ADQ0_RPTR_UPDATE_ADDR_H, 0 },
};

static void pfa_queue_registers_save(struct pfa *pfa_ctx)
{
    int portno;
    struct pfa_queue_stash *queue_regs = &g_pfa_queue;
    for (portno = 0; portno < PFA_CCORE_PORT_BEGIN; portno++) {
        // pfa queue
        queue_regs->pfa_tdqx_baddr_l[portno] = pfa_readl(pfa_ctx->regs, PFA_TDQX_BADDR_L(portno));
        queue_regs->pfa_tdqx_baddr_h[portno] = pfa_readl(pfa_ctx->regs, PFA_TDQX_BADDR_H(portno));
        queue_regs->pfa_tdqx_len[portno] = pfa_readl(pfa_ctx->regs, PFA_TDQX_LEN(portno));
        queue_regs->pfa_rdqx_baddr_l[portno] = pfa_readl(pfa_ctx->regs, PFA_RDQX_BADDR_L(portno));
        queue_regs->pfa_rdqx_baddr_h[portno] = pfa_readl(pfa_ctx->regs, PFA_RDQX_BADDR_H(portno));
        queue_regs->pfa_rdqx_len[portno] = pfa_readl(pfa_ctx->regs, PFA_RDQX_LEN(portno));

        // pfa ports
        pfa_ctx->hal->queue_portx_save(pfa_ctx, portno, queue_regs);
    }
}

static void pfa_queue_registers_restore(struct pfa *pfa_ctx)
{
    int portno;
    struct pfa_queue_stash *queue_regs = &g_pfa_queue;

    for (portno = 0; portno < PFA_CCORE_PORT_BEGIN; portno++) {
        // pfa queue
        pfa_writel(pfa_ctx->regs, PFA_TDQX_BADDR_L(portno), queue_regs->pfa_tdqx_baddr_l[portno]);
        pfa_writel(pfa_ctx->regs, PFA_TDQX_BADDR_H(portno), queue_regs->pfa_tdqx_baddr_h[portno]);
        pfa_writel(pfa_ctx->regs, PFA_TDQX_LEN(portno), queue_regs->pfa_tdqx_len[portno]);
        pfa_writel(pfa_ctx->regs, PFA_RDQX_BADDR_L(portno), queue_regs->pfa_rdqx_baddr_l[portno]);
        pfa_writel(pfa_ctx->regs, PFA_RDQX_BADDR_H(portno), queue_regs->pfa_rdqx_baddr_h[portno]);
        pfa_writel(pfa_ctx->regs, PFA_RDQX_LEN(portno), queue_regs->pfa_rdqx_len[portno]);

        // pfa ports
        pfa_ctx->hal->queue_portx_restore(pfa_ctx, portno, queue_regs);
    }
}

/*
 * save pfa registers, this function suppose to be called when
 *    1.All pfa port disabled in ap/cp;
 *    2.AD queue disabled and cleaned in m3;
 *    3.pfa en disabled in m3
 */
static inline void pfa_registers_save(struct pfa *pfa_ctx)
{
    int i;

    for (i = 0; i < PFA_GENERAL_REG_NUM; i++) {
        g_pfa_reg[i][STASH_REG_VALUE] = pfa_readl(pfa_ctx->regs, g_pfa_reg[i][STASH_REG_ADDR]);
    }
    for (i = 0; i < PFA_RESET_BAK_REG_NUM; i++) {
        g_pfa_reset_bak_reg[i][STASH_REG_VALUE] = pfa_readl(pfa_ctx->regs, g_pfa_reset_bak_reg[i][STASH_REG_ADDR]);
    }
}

static inline void pfa_registers_restore(struct pfa *pfa_ctx)
{
    int i;

    for (i = 0; i < PFA_GENERAL_REG_NUM; i++) {
        pfa_writel(pfa_ctx->regs, g_pfa_reg[i][STASH_REG_ADDR], g_pfa_reg[i][STASH_REG_VALUE]);
    }
    for (i = 0; i < PFA_RESET_BAK_REG_NUM; i++) {
        pfa_writel(pfa_ctx->regs, g_pfa_reset_bak_reg[i][STASH_REG_ADDR], g_pfa_reset_bak_reg[i][STASH_REG_VALUE]);
    }

    // rptr can only be write while debug mode on
    pfa_ctx->hal->dbgen_en();
    pfa_ctx->hal->dbgen_dis();
}

static inline void pfa_dis_ad(struct pfa *pfa_ctx)
{
    // disable
    pfa_writel(pfa_ctx->regs, PFA_ADQ0_EN, 0x0);

    /*
     * clean ad cache,
     * *for pfa will pre-read ad to pfa ad cache,
     * *and pfa use ad in cache instead of those AD queue
     * *if we do not AD clean cache before restore registers
     * *pfa will use AD in cache and then pre-read AD queue
     * *and use the same AD again, cause the same AD get used twice
     */
    pfa_ctx->hal->clean_ad_buf(pfa_ctx);
}

static void pfa_en_ad(struct pfa *pfa_ctx)
{
    struct pfa_adq_ctrl *cur_ctrl = NULL;
    enum pfa_adq_num adq_num;
    dma_addr_t dra_rptr_reg_addr;
    dma_addr_t ad_wptr_reg_addr;

    for (adq_num = PFA_ADQ0; adq_num < PFA_ADQ_BOTTOM; adq_num++) {
        cur_ctrl = &pfa_ctx->adqs_ctx.ctrl;
        if (cur_ctrl->ad_base_addr == NULL) {
            PFA_ERR("modem reset adq desc pool NULL.\n");
            BUG_ON(1);
            return;
        }

        if (pfa_ctx->soft_push) {  // evide dra hard error
            dra_rptr_reg_addr = (dma_addr_t)bsp_dra_set_adqbase((unsigned long long)cur_ctrl->ad_dma_addr,
                                                                cur_ctrl->dra_wptr_stub_dma_addr, cur_ctrl->adq_size,
                                                                pfa_ctx->adqs_ctx.ctrl.dra_ipip_type);
        } else {
            ad_wptr_reg_addr = pfa_ctx->res + PFA_ADQ0_WPTR;
            dra_rptr_reg_addr = (dma_addr_t)bsp_dra_set_adqbase((unsigned long long)cur_ctrl->ad_dma_addr,
                                                                lower_32_bits(ad_wptr_reg_addr), cur_ctrl->adq_size,
                                                                pfa_ctx->adqs_ctx.ctrl.dra_ipip_type);
        }
    }
    pfa_writel(pfa_ctx->regs, PFA_ADQ0_EN, 0x1);
}

// this may cause pfa send a ad empty interrupt to ap;
// maybe we just disable ad empty interrupt as default;
static void pfa_drx_restore_reg(struct pfa *pfa_ctx)
{
    pfa_registers_restore(pfa_ctx);
    pfa_queue_registers_restore(pfa_ctx);
}

int pfa_en(struct pfa *pfa_ctx)
{
    pfa_en_t value;

    value.u32 = pfa_readl(pfa_ctx->regs, PFA_EN);
    value.bits.pfa_en = 1;
    pfa_writel(pfa_ctx->regs, PFA_EN, value.u32);

    return 0;
}

int pfa_dis(struct pfa *pfa_ctx)
{
    pfa_en_t value;

    value.u32 = pfa_readl(pfa_ctx->regs, PFA_EN);
    value.bits.pfa_en = 0;
    pfa_writel(pfa_ctx->regs, PFA_EN, value.u32);

    return 0;
}

int pfa_check_event_stoped(struct pfa *pfa_ctx)
{
    int i;
    int ret = 1;

    for (i = PFA_EVENT_RESETTING_BIT + 1; i < PFA_EVENT_END_BIT; i++) {
        ret &= test_bit(i, (void *)&pfa_ctx->event);
    }
    return ret;
}

void pfa_reset_modem(void)
{
    struct pfa *pfa_ctx = &g_pfa;
    struct pfa_port_ctrl *port_ctrl = NULL;
    unsigned int portno;
    // only deal acore port
    for (portno = PFA_CCORE_PORT_BEGIN - 1; portno >= 0 && portno < PFA_CCORE_PORT_BEGIN; portno--) {
        port_ctrl = &pfa_ctx->ports[portno].ctrl;
        port_ctrl->port_flags.enable_after_modem_reset = port_ctrl->port_flags.enable;
        if (port_ctrl->port_flags.enable) {
            pfa_disable_port(pfa_ctx, portno);   // The memory recovery in the desc queue is responsible for dra
        }
    }

    pfa_dis(pfa_ctx);
    pfa_dis_ad(pfa_ctx);
    set_bit(PFA_EVENT_RESETTING_BIT, (void *)&pfa_ctx->event);
    wait_event(pfa_ctx->wqueue, pfa_check_event_stoped(pfa_ctx));
    pfa_queue_registers_save(pfa_ctx);
    pfa_registers_save(pfa_ctx);

    pfa_ctx->modem_reset_count++;
}

void pfa_unreset_modem(void)
{
    struct pfa *pfa_ctx = &g_pfa;
    struct pfa_port_ctrl *port_ctrl = NULL;
    unsigned int portno;

    // reset pfa
    pfa_writel(pfa_ctx->peri.crg_addr, pfa_ctx->peri.crg_rst_en, pfa_ctx->peri.crg_rst_mask);
    mdelay(10); /* 10 ms delay */
    pfa_writel(pfa_ctx->peri.crg_addr, pfa_ctx->peri.crg_rst_dis, pfa_ctx->peri.crg_rst_mask);
    mdelay(10); /* 10 ms delay */

    // restore registers
    pfa_drx_restore_reg(pfa_ctx);
    pfa_en_ad(pfa_ctx);

    for (portno = 0; portno < PFA_CCORE_PORT_BEGIN; portno++) {
        port_ctrl = &pfa_ctx->ports[portno].ctrl;
        port_ctrl->rd_free = 0;
        port_ctrl->rd_busy = 0;
        port_ctrl->td_free = 0;
        port_ctrl->td_busy = 0;
        if (port_ctrl->port_flags.enable_after_modem_reset || portno == PFA_DEFAULT_PFA_TFT_PORT ||
            portno == PFA_DEFAULT_CPU_PORT) {
            pfa_enable_port(portno);
        }
        port_ctrl->port_flags.enable_after_modem_reset = 0;
    }
    pfa_writel(pfa_ctx->regs, PFA_RD_DONE_INTA_MASK, pfa_ctx->rd_done_mask);
    pfa_ctx->event = pfa_ctx->event_default;

    pfa_ctx->modem_unreset_count++;

    return;
}

void bsp_pfa_reinit(void)
{
    struct pfa *pfa_ctx = &g_pfa;

    if (pfa_ctx->modem_unreset_count != (pfa_ctx->modem_reset_count - 1)) {
        PFA_ERR("unblanced call pfa modem unreset!ureset time %u,reset time %u \n", pfa_ctx->modem_unreset_count,
                pfa_ctx->modem_reset_count);
        pfa_bug(pfa_ctx);
        return;
    }
    pfa_unreset_modem();

    return;
}

int pfa_reset_modem_cb(drv_reset_cb_moment_e eparam, int userdata)
{
    struct pfa *pfa_ctx = &g_pfa;
    unsigned long flags = 0;

    PFA_ERR("pfa_reset_modem_cb %d \n", eparam);

    if (eparam == MDRV_RESET_CB_BEFORE) {
        if (pfa_ctx->modem_unreset_count != pfa_ctx->modem_reset_count) {
            PFA_ERR("unblanced call pfa modem reset!ureset time %u,reset time %u \n", pfa_ctx->modem_unreset_count,
                    pfa_ctx->modem_reset_count);
            pfa_bug(pfa_ctx);
            return 0;
        }
        spin_lock_irqsave(&pfa_ctx->reset_lock, flags);
        pfa_ctx->modem_resetting = 1;
        spin_unlock_irqrestore(&pfa_ctx->reset_lock, flags);
        pfa_reset_modem();
    } else if (eparam == MDRV_RESET_CB_AFTER) {
        pfa_en(pfa_ctx);
        spin_lock_irqsave(&pfa_ctx->reset_lock, flags);
        pfa_ctx->modem_resetting = 0;
        spin_unlock_irqrestore(&pfa_ctx->reset_lock, flags);
    } else {
        pfa_ctx->modem_noreset_count++;
        PFA_ERR("drv_reset_cb_moment_e is neither before or after !\n");
        pfa_bug(pfa_ctx);

        return -EINVAL;
    }

    return 0;
}

void pfa_reset_modem_init(void)
{
    int ret;
    struct pfa *pfa_ctx = &g_pfa;
    int i;

    for (i = PFA_EVENT_RESETTING_BIT + 1; i < PFA_EVENT_END_BIT; i++) {
        pfa_ctx->event_default |= BIT(i);
    }

    pfa_ctx->event = pfa_ctx->event_default;
    init_waitqueue_head(&pfa_ctx->wqueue);

    pfa_ctx->modem_reset_count = 0;
    pfa_ctx->modem_unreset_count = 0;
    pfa_ctx->modem_noreset_count = 0;

    ret = bsp_reset_cb_func_register("Eg_pfa", pfa_reset_modem_cb, 0, DRV_RESET_CB_PIOR_PFA);
    if (ret != 0) {
        PFA_ERR("Set modem reset call back func failed\n");
    }

    return;
}
