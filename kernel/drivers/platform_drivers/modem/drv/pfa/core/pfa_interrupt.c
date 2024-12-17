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

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <bsp_net_om.h>

#include "pfa.h"
#include "pfa_dbg.h"
#include "pfa_desc.h"
#include "pfa_port.h"
#include "pfa_interrupt.h"
#include <bsp_slice.h>
#include <bsp_diag.h>
#include "wan.h"

#define PFA_SCHED_GAP 160 // task delay report, 5ms

unsigned int pfa_process_desc_poll(unsigned int evt_rd_done, unsigned int limit)
{
    int count = 0;
    struct pfa *pfa = &g_pfa;
    unsigned int port_num;
    struct pfa_port_ctrl *ctrl = NULL;

    pfa->stat.wq_task_begin_time = bsp_get_slice_value();
    if (pfa->stat.wq_task_begin_time - pfa->stat.rd_int_time > pfa->stat.task_delay_max) {
        bsp_diag_drv_log_report(BSP_P_INFO, "pfa sche time:0x%x, irq:0x%x, work:0x%x\n", pfa->stat.wq_task_begin_time - pfa->stat.rd_int_time,
            pfa->stat.rd_int_time, pfa->stat.wq_task_begin_time);
        pfa_bug(pfa);
    }
    PFA_TRACE("pfa sche time:%u, irq:%u, work:%u\n", pfa->stat.wq_task_begin_time - pfa->stat.rd_int_time,
            pfa->stat.rd_int_time, pfa->stat.wq_task_begin_time);
    if (test_bit(PFA_EVENT_RESETTING_BIT, (void *)&pfa->event)){
        set_bit(PFA_EVENT_THREAD_STOPED_BIT, (void *)&pfa->event);
        wake_up(&pfa->wqueue);
        return limit;
    }

    for (port_num = 0; port_num < PFA_CCORE_PORT_BEGIN; port_num++) {
        ctrl = &pfa->ports[port_num].ctrl;

        if (0x1 & (evt_rd_done >> port_num)) {
            pfa->stat.evt_rd_complt[port_num]++;
        }
        if (port_num != pfa->pfa_tftport.portno && ctrl->port_flags.alloced && !ctrl->dpa_en) {
            count += pfa_process_rd_desc(port_num);
        }
    }
    pfa->stat.wq_task_end_time = bsp_get_slice_value();
    PFA_TRACE("pfa wq_task_end_time:0x%x\n", pfa->stat.wq_task_end_time);
    set_bit(PFA_EVENT_THREAD_STOPED_BIT, (void *)&pfa->event);
    wake_up(&pfa->wqueue);

    return count;
}
static inline void pfa_update_td_soft_rptr(struct pfa *pfa, unsigned int portno, struct pfa_port_ctrl *ctrl)
{
    unsigned int td_rptr, td_num;

    td_rptr = pfa_readl(pfa->regs, PFA_TDQX_RPTR(portno));
    if (unlikely(td_rptr >= ctrl->td_depth)) { // security check
        PFA_ERR("update_td_soft_rptr td_rptr reg is error\n");
        return;
    }
    if (ctrl->td_busy <= td_rptr) {
        td_num = td_rptr - ctrl->td_busy;
    } else {
        td_num = ctrl->td_depth + td_rptr - ctrl->td_busy;
    }
    ctrl->td_busy = td_rptr;
    pfa->ports[portno].stat.td_pkt_complete += td_num;
}
static inline void pfa_process_td(struct pfa *pfa, unsigned int evt_td_done, unsigned int port_num, struct pfa_port_ctrl *ctrl)
{
    int need_td_process = 1;

    if (pfa->td_fail_generate_rd && pfa->ports[port_num].ctrl.td_copy_en == 0 && (pfa->usbport.portno != port_num)) {
        need_td_process = 0;
    }
    if ((0x1 & (evt_td_done >> port_num)) && (port_num != pfa->pfa_tftport.portno)) {
        pfa->stat.evt_td_complt[port_num]++;
        if (need_td_process) {
            (void)pfa_process_td_desc(port_num);
        } else {
            pfa_update_td_soft_rptr(pfa, port_num, ctrl);
        }
    }
}

void pfa_process_desc_napi(void *pfa_ctx, unsigned int evt_rd_done, unsigned int evt_td_done)
{
    struct pfa *pfa = (struct pfa *)pfa_ctx;
    struct wan_ctx_s *wan = &g_wan_ctx;
    unsigned int port_num;
    struct pfa_port_ctrl *ctrl = NULL;

    wan->priv->evt_rd_done |= evt_rd_done;

    for (port_num = 0; port_num < PFA_CCORE_PORT_BEGIN; port_num++) {
        ctrl = &pfa->ports[port_num].ctrl;

        pfa_process_td(pfa, evt_td_done, port_num, ctrl);
    }

    if (evt_rd_done) {
        pfa->stat.rd_int_time = bsp_get_slice_value();
        PFA_TRACE("rd_int_time:0x%x\n", pfa->stat.rd_int_time);
        if (napi_schedule_prep(&wan->priv->napi)) {
            clear_bit(PFA_EVENT_THREAD_STOPED_BIT, (void *)&pfa->event);
            pfa_writel_relaxed(pfa->regs, PFA_RD_DONE_INTA_MASK, PFA_EVENT_ENABLE);
            __napi_schedule(&wan->priv->napi);
        }
    }

    return;
}

static inline void pfa_reenable_intr(struct pfa *pfa)
{
    pfa_writel(pfa->regs, PFA_RD_DONE_INTA_MASK, pfa->rd_done_mask);
}

static void pfa_intr_bh_workqueue(struct work_struct *work)
{
    struct pfa *pfa = &g_pfa;
    struct pfa_port_ctrl *ctrl = NULL;
    unsigned int port_num;

    if (test_bit(PFA_EVENT_RESETTING_BIT, (void *)&pfa->event)){
        set_bit(PFA_EVENT_THREAD_STOPED_BIT, (void *)&pfa->event);
        wake_up(&pfa->wqueue);
        return;
    }

    pfa->stat.wq_task_begin_time = bsp_get_slice_value();
    if (pfa->stat.wq_task_begin_time - pfa->stat.rd_int_time > pfa->stat.task_delay_max) {
        bsp_diag_drv_log_report(BSP_P_INFO, "pfa sche time:%u, irq:%u, work:%u\n", pfa->stat.wq_task_begin_time - pfa->stat.rd_int_time,
            pfa->stat.rd_int_time, pfa->stat.wq_task_begin_time);
        pfa_bug(pfa);
    }
    PFA_INFO("pfa sche time:%u, irq:%u, work:%u\n", pfa->stat.wq_task_begin_time - pfa->stat.rd_int_time,
            pfa->stat.rd_int_time, pfa->stat.wq_task_begin_time);
    for (port_num = 0; port_num < PFA_CCORE_PORT_BEGIN; port_num++) {
        ctrl = &pfa->ports[port_num].ctrl;

        if (port_num != pfa->pfa_tftport.portno && ctrl->port_flags.alloced && !ctrl->dpa_en) {
            (void)pfa_process_rd_desc(port_num);
        }
    }
    pfa_reenable_intr(pfa);
    pfa->stat.wq_task_end_time = bsp_get_slice_value();
    PFA_INFO("pfa wq_task_end_time:0x%x\n", pfa->stat.wq_task_end_time);
    set_bit(PFA_EVENT_THREAD_STOPED_BIT, (void *)&pfa->event);
    wake_up(&pfa->wqueue);
    return;
}

void pfa_process_desc_workqueue(void *pfa_ctx, unsigned int evt_rd_done, unsigned int evt_td_done)
{
    struct pfa *pfa = (struct pfa *)pfa_ctx;
    unsigned int port_num;
    struct pfa_port_ctrl *ctrl = NULL;

    for (port_num = 0; port_num < PFA_CCORE_PORT_BEGIN; port_num++) {
        ctrl = &pfa->ports[port_num].ctrl;

        if (ctrl->port_flags.enable) {
            pfa_process_td(pfa, evt_td_done, port_num, ctrl);

            if (0x1 & (evt_rd_done >> port_num)) {
                pfa->stat.evt_rd_complt[port_num]++;
            }
        }
    }
    if (evt_rd_done) {
        pfa->stat.rd_int_time = bsp_get_slice_value();
        PFA_TRACE("rd_int_time:0x%x\n", pfa->stat.rd_int_time);
        clear_bit(PFA_EVENT_THREAD_STOPED_BIT, (void *)&pfa->event);
        pfa_writel_relaxed(pfa->regs, PFA_RD_DONE_INTA_MASK, PFA_EVENT_ENABLE);
        queue_work(pfa->smp.workqueue, &pfa->smp.work);
    }
}

void pfa_init_workqueue(struct pfa *pfa)
{
    INIT_WORK(&pfa->smp.work, pfa_intr_bh_workqueue);
    pfa->smp.workqueue = alloc_ordered_workqueue("pfa_wq", 0);
    if (pfa->smp.workqueue == NULL) {
        PFA_ERR("pfa port workqueue alloc fail\n");
    }

    PFA_INFO("pfa port workqueue allocd\n");
    return;
}

void pfa_process_desc_intr(void *pfa_ctx, unsigned int evt_rd_done, unsigned int evt_td_done)
{
    struct pfa *pfa = (struct pfa *)pfa_ctx;
    unsigned int port_num;
    struct pfa_port_ctrl *ctrl = NULL;

    for (port_num = 0; port_num < PFA_CCORE_PORT_BEGIN; port_num++) {
        ctrl = &pfa->ports[port_num].ctrl;

        if (0x1 & (evt_rd_done >> port_num)) {
            pfa->stat.evt_rd_complt[port_num]++;
            if (port_num != pfa->pfa_tftport.portno && !ctrl->dpa_en) {
                (void)pfa_process_rd_desc(port_num);
            }
        }

        pfa_process_td(pfa, evt_td_done, port_num, ctrl);
    }
}

static irqreturn_t pfa_interrupt(int irq, void *dev_id)
{
    struct pfa *pfa = (struct pfa *)dev_id;
    unsigned int evt_rd_full;
    unsigned int evt_rd_done;
    unsigned int evt_td_done;
    unsigned int evt_ad_empty;
    unsigned int port_num;

    if (unlikely(pfa->dbg_level & PFA_DBG_HIDS_UPLOAD)) {
        bsp_net_report_set_time(); /* set trans report timestamp */
    }

    evt_rd_full = pfa_readl_relaxed(pfa->regs, PFA_RDQ_FULL_INTA);
    evt_rd_done = pfa_readl_relaxed(pfa->regs, PFA_RD_DONE_INTA);
    evt_td_done = pfa_readl_relaxed(pfa->regs, PFA_TD_DONE_INTA);
    evt_ad_empty = pfa_readl_relaxed(pfa->regs, PFA_ADQ_EMPTY_INTA);

    pfa_writel_relaxed(pfa->regs, PFA_RDQ_FULL_INTA_STATUS, evt_rd_full);
    pfa_writel_relaxed(pfa->regs, PFA_RD_DONE_INTA_STATUS, evt_rd_done);
    pfa_writel_relaxed(pfa->regs, PFA_TD_DONE_INTA_STATUS, evt_td_done);
    pfa_writel_relaxed(pfa->regs, PFA_ADQ_EMPTY_INTA_STATUS, evt_ad_empty);

    /*
     * pfav200,if td fail, do not produce rd desc.need free mem at td desc.
     *         for pfa_tft port, this operation is completed before pushing the pointer.
     * pfav300, if pfa_tft port td fail, produce rd desc.need free mem at cpu rd desc.
     *          if other port,td fail, do not produce rd desc.need free mem at other port desc.
     */
    if (evt_rd_done || evt_td_done) {
        pfa->stat.int_time = bsp_get_slice_value();
        pfa->smp.process_desc((void *)pfa, evt_rd_done, evt_td_done);
    }
    evt_rd_full |= pfa_readl_relaxed(pfa->regs, PFA_RDQ_FULL_INTA_STATUS);
    if (unlikely(evt_rd_full)) {
        for (port_num = 0; port_num < PFA_CCORE_PORT_BEGIN; port_num++) {
            if (unlikely(0x1 & (evt_rd_full >> port_num))) {
                pfa->stat.evt_rd_full[port_num]++;
            }
        }
    }

    if (unlikely(evt_ad_empty)) {
        if (0x1 & evt_ad_empty) {
            pfa->stat.evt_ad_empty[PFA_ADQ0]++;
        }
    }

#ifndef CONFIG_PFA_PHONE_SOC
    if (pfa->soft_push) { /* evide dra hard error */
        pfa_update_adq_wptr(pfa);
    }
#endif

    return IRQ_HANDLED;
}

irqreturn_t pfa_wake_irq_handler(int irq, void *dev_id)
{
    UNUSED(irq);
    UNUSED(dev_id);
    return IRQ_HANDLED;
}

static int pfa_wake_irq_init(struct pfa *pfa)
{
    device_node_s *node = NULL;

    node = bsp_dt_find_node_by_path("/pfa_wake_irq");
    if (node == NULL) {
        PFA_ERR("pfa no pfa_wake_irq\n");
        return 0;
    }

    pfa->wake_irq = bsp_dt_irq_parse_and_map(node, 0);
    if (request_irq(pfa->wake_irq, pfa_wake_irq_handler, 0, "pfa_wake_irq", NULL)) {
        PFA_ERR("pfa_wake_irq register fail\n");
        return -1;
    }

    enable_irq_wake(pfa->wake_irq);
    return 0;
}

int pfa_interrupt_dts_init(struct pfa *pfa)
{
    int ret;

    ret = pfa_wake_irq_init(pfa);
    if (ret) {
        return ret;
    }

    pfa->irq_interval = PFA_INTR_DEF_INTEVAL;
    if (bsp_dt_property_read_u32_array(pfa->np, "interrupt_interval", &pfa->irq_interval, 1)) {
        PFA_ERR("irq interval use default %d\n",PFA_INTR_DEF_INTEVAL );
    }
    pfa->td_fail_generate_rd = 1;
    if (bsp_dt_property_read_u32_array(pfa->np, "td_fail_generate_rd", &pfa->td_fail_generate_rd, 1)) {
        PFA_ERR("td generate rd %d\n",pfa->td_fail_generate_rd);
    }
    return 0;
}

int pfa_interrput_init(struct pfa *pfa)
{
    int ret;

    pfa_writel_relaxed(pfa->regs, PFA_RDQ_FULL_INTA_MASK, PFA_EVENT_ENABLE);
    pfa_writel_relaxed(pfa->regs, PFA_RD_DONE_INTA_MASK, PFA_EVENT_ENABLE);
    pfa_writel_relaxed(pfa->regs, PFA_TD_DONE_INTA_MASK, PFA_EVENT_ENABLE);
    pfa_writel_relaxed(pfa->regs, PFA_ADQ_EMPTY_INTA_MASK, PFA_EVENT_ENABLE);
    pfa_writel_relaxed(pfa->regs, PFA_INTA_INTERVAL, pfa->irq_interval);
    pfa->rd_done_mask = PFA_EVENT_ENABLE;

    // set to 1k as default
    pfa_writel_relaxed(pfa->regs, PFA_RDQ_FULL_LEVEL, PFA_EVT_RD_FULL);
    pfa_writel(pfa->regs, PFA_ADQ_EMPTY_LEVEL, PFA_EVT_AD_EMPTY);

     if (pfa->pfa_feature.smp_sch) {
        pfa->smp.process_desc = pfa_process_desc_napi;
        PFA_ERR("pfa tasklet sch\n");
    } else if (pfa->pfa_feature.smp_hp) {
        pfa->smp.process_desc = pfa_process_desc_workqueue;
        pfa_init_workqueue(pfa);
        PFA_ERR("pfa workqueue sch\n");
    } else {
        pfa->smp.process_desc = pfa_process_desc_intr;
        PFA_ERR("pfa int sch\n");
    }

    ret = request_irq(pfa->irq, pfa_interrupt, IRQF_SHARED, "pfa", pfa);
    if (pfa->wake_irq == 0) {
        enable_irq_wake(pfa->irq);
    }
    pfa->stat.task_delay_max = PFA_SCHED_GAP;

    return ret;
}
