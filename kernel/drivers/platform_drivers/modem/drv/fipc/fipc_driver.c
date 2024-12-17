/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
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

/* 只能在唤醒中断上下文中使用 */
int fipc_relv_dev_chn_get(struct fipc_device *prdev, u32 ripipe_id, struct fipc_device **ppdev, u32 *plipie_id)
{
    u32 devid;
    u32 lipipeid;
    u32 addrL, addrH;
    fipcsoc_ptr_t lipipe_pa;
    struct fipc_device *pldev = NULL;
    addrL = readl((void *)((char *)prdev->base_va + FIPC_IPIPE_CFG_ADDR_L(ripipe_id)));
    addrH = readl((void *)((char *)prdev->base_va + FIPC_IPIPE_CFG_ADDR_H(ripipe_id)));

    lipipe_pa = FIPC_ADDR_MK(addrL, addrH);
    lipipeid = ((lipipe_pa & 0x7fff) - 0x6000) >> 0x7;

    for (devid = 0; devid < FIPC_DEVICE_NUM_MAX; devid++) {
        pldev = fipc_device_get_fast(devid);
        if (pldev == NULL) {
            continue;
        }
        if ((pldev->base_pa & 0xFFFF8000) != (lipipe_pa & 0xFFFF8000)) {
            /* not belongs to this device */
            continue;
        }

        /*  do check */
        if (lipipeid >= pldev->pipepair_cnt || lipipe_pa != pldev->base_pa + FIPC_IPIPE_BASE_ADDR_L(lipipeid)) {
            /* big error */
            return -1;
        }
        break;
    }
    if (devid >= FIPC_DEVICE_NUM_MAX) {
        return -1;
    }
    *ppdev = pldev;
    *plipie_id = lipipeid;
    return 0;
}

static inline u32 fipc_ffs(u32 num)
{
    u32 i;
    for (i = 0; i < FIPC_PIPE_CNT_PER_REG; i++) {
        if ((num >> i) & 1) {
            return i + 1;
        }
    }
    return 0;
}

int ipipe_neg_check(const void *base_addr, u32 ipipe_id)
{
    int ret = 0;
    u32 loop = 0;
    union ipipe_stat i_state = { 0 };
    while (loop++ < FIPC_IPIPE_STOP_WAIT) {
        i_state.val = fipc_ipipe_local_status(base_addr, ipipe_id);
        if (i_state.union_stru.pt_stat == FIPC_PM_REQ_PD) {
            /* ipipe当localen和remote都为1 然后其中一个变为0. 也就是说上电握手后才有这个过程，没经过握手没这个过程 */
            /* 从这个状态 要进入下一个状态，必须对端为local en为1才行，不然会卡死在这个状态 */
            continue;
        } else if (i_state.union_stru.pt_stat == FIPC_NOT_PM_STAT) {
            /* FIPCv100， 在影子ipipe回推给实体拒绝下电的时，影子ipipe干完活了, 会造成影子ipipe进入这种状态(bug) */
            /* v200没有上面的bug，该状态只在未完成上电握手又发起下电的场景(ipipe置loclen为1后置为0，而remote一直为0) */
            break;
        } else if (i_state.union_stru.pt_stat == FIPC_PM_REFUSE_PD) {
            ret = FIPC_ERR_PM_IPIPE_NEG_REFUSE;
            break;
        } else if (i_state.union_stru.pt_stat == FIPC_PM_ALLOW_PD) {
            break;
        } else {
            ret = FIPC_ERR_PM_IPIPE_NEG_FAIL;
        }
    }
    if (loop >= FIPC_IPIPE_STOP_WAIT) {
        ret = FIPC_ERR_PM_IPIPE_NEG_TIMEOUT;
    }
    return ret;
}

int ipipe_status_check(const void *base_addr, u32 ipipe_id)
{
    int ret = 0;
    u32 loop = 0;
    union ipipe_stat i_state = { 0 };
    while (loop++ < FIPC_IPIPE_IDLE_WAIT) {
        i_state.val = fipc_ipipe_local_status(base_addr, ipipe_id);
        if (i_state.union_stru.is_idle == 0x1) {
            break;
        }
    }
    if (loop >= FIPC_IPIPE_IDLE_WAIT) {
        ret = FIPC_ERR_PM_IPIPE_NOT_IDLE;
    }

    return ret;
}

int opipe_status_check(const void *base_addr, u32 opipe_id)
{
    int loop = 0;
    while (loop++ < FIPC_OPIPE_STOP_WAIT) {
        if (!fipc_opipe_stat_isbusy(base_addr, opipe_id)) {
            break;
        }
    }
    if (loop >= FIPC_OPIPE_STOP_WAIT) {
        return FIPC_ERR_PM_OPIPE_NOT_IDLE;
    }
    return 0;
}

void fipc_opipe_startup(void *base_addr, struct opipe_config *opipe)
{
    writel_relaxed(FIPC_ADDR_LO(opipe->ptr_paddr), (void *)((char *)base_addr + FIPC_OPIPE_PTR_ADDR_L(opipe->id)));
    writel_relaxed(FIPC_ADDR_HI(opipe->ptr_paddr), (void *)((char *)base_addr + FIPC_OPIPE_PTR_ADDR_H(opipe->id)));

    writel_relaxed(FIPC_ADDR_LO(opipe->ringbuf_pa), (void *)((char *)base_addr + FIPC_OPIPE_BASE_ADDR_L(opipe->id)));
    writel_relaxed(FIPC_ADDR_HI(opipe->ringbuf_pa), (void *)((char *)base_addr + FIPC_OPIPE_BASE_ADDR_H(opipe->id)));

    writel_relaxed(opipe->depth, (void *)((char *)base_addr + FIPC_OPIPE_DEPTH(opipe->id)));

    writel_relaxed(opipe->wptr, (void *)((char *)base_addr + FIPC_OPIPE_WPTR(opipe->id)));
    writel_relaxed(opipe->rptr, (void *)((char *)base_addr + FIPC_OPIPE_RPTR(opipe->id)));

    writel_relaxed(opipe->opipe_uctrl.val, (void *)((char *)base_addr + FIPC_OPIPE_CTRL(opipe->id)));
}

void fipc_ipipe_startup(void *base_addr, struct ipipe_config *ipipe)
{
    writel_relaxed(FIPC_ADDR_LO(ipipe->ptr_paddr), (void *)((char *)base_addr + FIPC_IPIPE_PTR_ADDR_L(ipipe->id)));
    writel_relaxed(FIPC_ADDR_HI(ipipe->ptr_paddr), (void *)((char *)base_addr + FIPC_IPIPE_PTR_ADDR_H(ipipe->id)));

    writel_relaxed(FIPC_ADDR_LO(ipipe->relv_pipe_paddr),
        (void *)((char *)base_addr + FIPC_IPIPE_CFG_ADDR_L(ipipe->id)));
    writel_relaxed(FIPC_ADDR_HI(ipipe->relv_pipe_paddr),
        (void *)((char *)base_addr + FIPC_IPIPE_CFG_ADDR_H(ipipe->id)));

    writel_relaxed(FIPC_ADDR_LO(ipipe->ringbuf_pa), (void *)((char *)base_addr + FIPC_IPIPE_BASE_ADDR_L(ipipe->id)));
    writel_relaxed(FIPC_ADDR_HI(ipipe->ringbuf_pa), (void *)((char *)base_addr + FIPC_IPIPE_BASE_ADDR_H(ipipe->id)));

    writel_relaxed(ipipe->depth, (void *)((char *)base_addr + FIPC_IPIPE_DEPTH(ipipe->id)));

    writel_relaxed(ipipe->wptr & 0xFFFF, (void *)((char *)base_addr + FIPC_IPIPE_WPTR(ipipe->id)));
    writel_relaxed(ipipe->rptr, (void *)((char *)base_addr + FIPC_IPIPE_RPTR(ipipe->id)));

    writel_relaxed(ipipe->ipipe_uctrl.val, (void *)((char *)base_addr + FIPC_IPIPE_CTRL(ipipe->id)));
}

static void fipc_opipe_irq_handler(struct fipc_device *pdev, u32 opipe_id)
{
    u32 int_level_status;

    int_level_status = fipc_opipe_devint_stat(pdev->base_va, opipe_id);
    if (FIPC_INT_WR_OVERFLOW & int_level_status) {
        fipc_print_trace("FIPC_INT_WR_OVERFLOW detected for  opipe_id 0x%x \n", opipe_id);
        fipc_opipe_devint_dis(pdev->base_va, opipe_id, FIPC_INT_WR_OVERFLOW);
        fipc_normal_event_notify(FIPC_INT_WR_OVERFLOW, pdev, opipe_id);
    } else if (FIPC_INT_WR_UNDERFLOW & int_level_status) {
        fipc_print_trace("FIPC_INT_WR_UNDERFLOW detected for  opipe_id 0x%x \n", opipe_id);
        fipc_opipe_devint_dis(pdev->base_va, opipe_id, FIPC_INT_WR_UNDERFLOW);
        fipc_normal_event_notify(FIPC_INT_WR_UNDERFLOW, pdev, opipe_id);
    } else if (FIPC_INT_WR_DATA_DONE & int_level_status) {
        fipc_print_trace("FIPC_INT_WR_DATA_DONE detected for  opipe_id 0x%x \n", opipe_id);
        fipc_opipe_devint_clr(pdev->base_va, opipe_id, FIPC_INT_WR_DATA_DONE);
        fipc_normal_event_notify(FIPC_INT_WR_DATA_DONE, pdev, opipe_id);
    } else if (FIPC_INT_WR_PIPE_ERR & int_level_status) {
        fipc_print_crit("FIPC_INT_WR_PIPE_ERR detected for  opipe_id 0x%x \n", opipe_id);
        fipc_opipe_devint_clr(pdev->base_va, opipe_id, FIPC_INT_WR_PIPE_ERR);
    } else {
        fipc_print_crit("fipc_opipe_irq_handler  opipe_id 0x%x no detected \n", opipe_id);
    }

    return;
}

irqreturn_t fipc_localsend_handler(int irq_no, void *arg)
{
    u32 int_pipe_status;
    u32 pipe_id;
    struct irq_bundle *bundle = (struct irq_bundle *)arg;
    struct fipc_device *pdev = bundle->pdev;
    u32 reg_offset = pdev->config_diff->core_pipe_irq_off;

    int_pipe_status = readl((void *)((char *)pdev->base_va + FIPC_CORE_OPIPE_INT_STAT0(bundle->cpu_id, reg_offset)));
    if (int_pipe_status) {
        pipe_id = fipc_ffs(int_pipe_status);
        fipc_opipe_irq_handler(pdev, pipe_id - 1);
        return IRQ_HANDLED;
    }
    int_pipe_status = readl((void *)((char *)pdev->base_va + FIPC_CORE_OPIPE_INT_STAT1(bundle->cpu_id, reg_offset)));
    if (int_pipe_status) {
        pipe_id = fipc_ffs(int_pipe_status);
        fipc_opipe_irq_handler(pdev, pipe_id + FIPC_PIPE_CNT_PER_REG - 1);
        return IRQ_HANDLED;
    }
    return IRQ_NONE;
}

static void fipc_ipipe_irq_handler(struct fipc_device *pdev, u32 ipipe_id)
{
    u32 int_level_status;
    u32 lipipe_id;
    u32 is_inner_irq = 0;
    struct fipc_device *pldev = NULL;
    struct fipc_channel *pchannel = NULL;
    /* 需要判断这是个PIPE是不是自己可以直接访问的PIPE,
    比如FIPC_INT_RD_PIPE_WAKEUP可能是影子报过来的 */
    if (ipipe_id < FIPC_CHN_ID_MAX) {
        pchannel = pdev->channels[ipipe_id];
        if (pchannel != NULL && pchannel->type == FIPC_CHN_TYPE_MSG_INRRECV) {
            is_inner_irq = 1;
        }
    }

    int_level_status = fipc_ipipe_devint_stat(pdev->base_va, ipipe_id);
    if (FIPC_INT_RD_DATA_ARV & int_level_status) {
        fipc_print_trace("FIPC_INT_RD_DATA_ARV detected for  ipipe_id 0x%x \n", ipipe_id);
        fipc_ipipe_devint_clr(pdev->base_va, ipipe_id, FIPC_INT_RD_DATA_ARV);
        fipc_normal_event_notify(FIPC_INT_RD_DATA_ARV, pdev, ipipe_id);
    } else if (FIPC_INT_RD_ARV_TIMEOUT & int_level_status) {
        fipc_print_trace("FICC_INT_RD_DATA_TO detected for  ipipe_id 0x%x \n", ipipe_id);
        fipc_ipipe_devint_clr(pdev->base_va, ipipe_id, FIPC_INT_RD_ARV_TIMEOUT);
        /* 超时处理与数据到达处理完全相同，直接复用数据到达处理*/
        fipc_normal_event_notify(FIPC_INT_RD_DATA_ARV, pdev, ipipe_id);
    } else if (FIPC_INT_RD_PIPE_WAKEUP & int_level_status) {
        fipc_print_trace("FIPC_INT_RD_PIPE_WAKEUP detected for remote ipipe_id 0x%x \n", ipipe_id);
        /* 清唤醒中断 */
        fipc_ipipe_devint_clr(pdev->base_va, ipipe_id, FIPC_INT_RD_PIPE_WAKEUP);
        if (is_inner_irq) {
            return;
        } else {
            /* 根据对端设备和pipe里的信息获取本端的dev和channel */
            if (fipc_relv_dev_chn_get(pdev, ipipe_id, &pldev, &lipipe_id) == 0) {
                /* 设置本端对应ipipe的接收flag或者直接打开remote_en */
                fipc_wakeup_event_notify(pldev, lipipe_id);
            } else {
                fipc_print_crit("relv dev and chn get failed\n");
            }
        }
    } else if (int_level_status) {
        fipc_print_crit("fipc_ipipe_irq_handler ipipe_id 0x%x found int status:0x%x\n", ipipe_id, int_level_status);
    } else {
    }
    return;
}

irqreturn_t fipc_localrecv_handler(int irq_no, void *arg)
{
    u32 int_pipe_status;
    u32 pipe_id;
    struct irq_bundle *bundle = (struct irq_bundle *)arg;
    struct fipc_device *pdev = bundle->pdev;
    u32 reg_offset = pdev->config_diff->core_pipe_irq_off;

    int_pipe_status = readl((void *)((char *)pdev->base_va + FIPC_CORE_IPIPE_INT_STAT0(bundle->cpu_id, reg_offset)));
    if (int_pipe_status) {
        pipe_id = fipc_ffs(int_pipe_status);
        fipc_ipipe_irq_handler(pdev, pipe_id - 1);
        return IRQ_HANDLED;
    }
    int_pipe_status = readl((void *)((char *)pdev->base_va + FIPC_CORE_IPIPE_INT_STAT1(bundle->cpu_id, reg_offset)));
    if (int_pipe_status) {
        pipe_id = fipc_ffs(int_pipe_status);
        fipc_ipipe_irq_handler(pdev, pipe_id + FIPC_PIPE_CNT_PER_REG - 1);
        return IRQ_HANDLED;
    }
    return IRQ_NONE;
}

/* pdev should be a remote fipc device( and must poweon) */
static void fipc_shadow_ipipe_irq_handler(struct fipc_device *pdev, u32 ipipe_id)
{
    u32 int_level_status;
    u32 lipipe_id;
    struct fipc_device *pldev = NULL;

    /* 此函数直接访问非本子系统的外设，必须由流程保证远端外设在此访问过程不会下电 */
    int_level_status = fipc_ipipe_devint_stat(pdev->base_va, ipipe_id);
    if (FIPC_INT_RD_PIPE_WAKEUP & int_level_status) {
        fipc_print_trace("FIPC_INT_RD_PIPE_WAKEUP detected for remote ipipe_id 0x%x \n", ipipe_id);
        /* 清对端的唤醒中断 */
        fipc_ipipe_devint_clr(pdev->base_va, ipipe_id, FIPC_INT_RD_PIPE_WAKEUP);
        /* 根据对端设备和pipe里的信息获取本端的dev和channel */
        if (fipc_relv_dev_chn_get(pdev, ipipe_id, &pldev, &lipipe_id) == 0) {
            /* 设置本端对应ipipe的接收flag或者直接打开remote_en */
            fipc_wakeup_event_notify(pldev, lipipe_id);
        } else {
            fipc_print_crit("relv dev and chn get failed\n");
        }
    } else {
        fipc_print_crit("fipc_shadow_ipipe_irq_handler ipipe_id 0x%x no detected \n", ipipe_id);
    }
    return;
}

/* pdev should be a remote fipc device( and must poweon) */
irqreturn_t fipc_remote_handler(int irq_no, void *arg)
{
    u32 int_pipe_status;
    u32 pipe_id;
    struct irq_bundle *bundle = (struct irq_bundle *)arg;
    /* 此函数直接访问非本子系统的外设，必须由流程保证远端外设在此访问过程不会下电 */
    struct fipc_device *pdev = bundle->pdev;
    u32 reg_offset = pdev->config_diff->core_pipe_irq_off;

    int_pipe_status = readl((void *)((char *)pdev->base_va + FIPC_CORE_IPIPE_INT_STAT0(bundle->cpu_id, reg_offset)));
    if (int_pipe_status) {
        pipe_id = fipc_ffs(int_pipe_status);
        fipc_shadow_ipipe_irq_handler(pdev, pipe_id - 1);
        return IRQ_HANDLED;
    }

    int_pipe_status = readl((void *)((char *)pdev->base_va + FIPC_CORE_IPIPE_INT_STAT1(bundle->cpu_id, reg_offset)));
    if (int_pipe_status) {
        pipe_id = fipc_ffs(int_pipe_status);
        fipc_shadow_ipipe_irq_handler(pdev, pipe_id + FIPC_PIPE_CNT_PER_REG - 1);
        return IRQ_HANDLED;
    }

    fipc_print_crit("fipc_remote_handler no handled \n");
    return IRQ_NONE;
}

int fipc_irqs_local_init(struct fipc_device *pdev)
{
    u32 idx;
    u32 uret;
    int ret = 0;
    struct irq_bundle *bundle = NULL;
    for (idx = 0; idx < sizeof(pdev->maps) / sizeof(pdev->maps[0]); idx++) {
        bundle = pdev->maps[idx];
        if (bundle == NULL) {
            continue;
        }
        bundle->pdev = pdev;
        uret = 0;
        uret |= (u32)request_irq(bundle->irq[0x0], fipc_localsend_handler, 0, "fipc_irq_lsend", (void *)bundle);
        uret |= (u32)request_irq(bundle->irq[0x1], fipc_localrecv_handler, 0, "fipc_irq_lrecv", (void *)bundle);
        if (uret) {
            ret = -1;
            fipc_print_error("fipc_irq[%d]or[%d] failed!\n", bundle->irq[0x0], bundle->irq[0x1]);
        }
    }
    return ret;
}
int fipc_irqs_remote_init(struct fipc_device *pdev)
{
    u32 idx;
    u32 uret;
    int ret = 0;
    struct irq_bundle *bundle = NULL;
    for (idx = 0; idx < sizeof(pdev->maps) / sizeof(pdev->maps[0]); idx++) {
        bundle = pdev->maps[idx];
        if (bundle == NULL) {
            continue;
        }
        bundle->pdev = pdev;
        uret = 0;
        uret |= (u32)request_irq(bundle->irq[0x0], fipc_remote_handler, 0, "fipc_irq_remote", (void *)bundle);
        if (uret) {
            ret = -1;
            fipc_print_error("fipc_irq[%d] failed!\n", bundle->irq[0x0]);
        }
        enable_irq_wake(bundle->irq[0x0]);
    }
    return ret;
}

int _fipc_irqs_init(struct fipc_device *pdev)
{
    int ret = 0;
    if (pdev->ctrl_level == FIPC_DEV_CONTROL_LEVEL_HOST || pdev->ctrl_level == FIPC_DEV_CONTROL_LEVEL_GUEST ||
        pdev->ctrl_level == FIPC_DEV_CONTROL_LEVEL_PART) {
        ret = fipc_irqs_local_init(pdev);
    } else if (pdev->ctrl_level == FIPC_DEV_CONTROL_LEVEL_IRQCLR) {
        ret = fipc_irqs_remote_init(pdev);
    } else {
        ; /* nothing need to do */
    }
    if (ret) {
        return -1;
    }
    return 0;
}
int fipc_irqs_init(void)
{
    int ret;
    u32 idx = 0;
    struct fipc_device *pdev = NULL;
    struct fipc_device *prdev = NULL;
    struct fipc_channel *pchannel = NULL;
    u32 cpu_id;

    /* get every channel, do check init pipe mask */
    while (fipc_dev_chn_get_byidx(idx, &pdev, &pchannel) == 0) {
        idx++;
        cpu_id = FIPC_GET_CORE_ID(pchannel->ldrvchn_id);
        if (pdev->maps[cpu_id] == NULL) {
            fipc_print_error("fipc_irqs_init failed!\n");
            return -1;
        }
        if (pchannel->type == FIPC_CHN_TYPE_MSG_OUTRECV) {
            prdev = fipc_device_get_fast(FIPC_GET_DEV_ID(pchannel->rdrvchn_id));
            if (prdev == NULL) {
                return -1;
            }
            if (prdev->maps[FIPC_GET_RCORE_ID(pchannel->rdrvchn_id)] == NULL) {
                fipc_print_error("fipc_irqs_init failed!\n");
                return -1;
            }
        }
    }
    for (idx = 0; idx < FIPC_DEVICE_NUM_MAX; idx++) {
        pdev = fipc_device_get_fast(idx);
        if (pdev == NULL) {
            continue;
        }
        ret = _fipc_irqs_init(pdev);
        if (ret) {
            return ret;
        }
    }

    return 0;
}
