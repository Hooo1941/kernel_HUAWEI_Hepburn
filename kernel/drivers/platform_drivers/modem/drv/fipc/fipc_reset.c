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
#include "fipc_platform.h"
#include "fipc_dts.h"
#include "fipc_device.h"
#include "fipc_driver.h"
#include "fipc_core.h"
#include "fipc_pmsr.h"
#include "fipc_dump.h"
#include "fipc_reset.h"
#include "bsp_sec_call.h"


#define FIPC_RESET_DBG_STEPS_CNT 4
#define FIPC_BEFORE_RESET_CMD 0
#define FIPC_AFTER_RESET_CMD 1


struct fipc_reset_dbg {
    u32 before_reset_times;                   /* before reset的成功次数 */
    u32 after_reset_times;                    /* after reset的成功次数 */
    u32 reset_stat[FIPC_RESET_DBG_STEPS_CNT]; /* 一次单独复位的记录 */
    u32 chn_id;                               /* 最新reset失败通道 */
    u32 last_step;                            /* 最新reset失败通道失败位置 */
};

struct fipc_reset_dbg g_fipc_reset_dbg;

int fipc_opipe_rst_halt(struct fipc_device *pdev, struct fipc_channel *pchannel)
{
    u32 opipe_wptr, opipe_id, query_times;
    opipe_id = FIPC_GET_PIPE_ID(pchannel->ldrvchn_id);
    if (pdev == NULL || opipe_id >= pdev->pipepair_cnt) {
        return -1;
    }
    if (!fipc_opipe_is_en(pdev->base_va, opipe_id)) {
        return 0;
    }
    fipc_opipe_abort_set(pdev->base_va, opipe_id);
    for (query_times = 0; query_times < FIPC_HALT_QUERY_WAIT_TIMES; query_times++) {
        if (FIPC_INT_WR_ABORT & fipc_opipe_devint_raw(pdev->base_va, opipe_id)) {
            break;
        }
    }
    if (query_times >= FIPC_HALT_QUERY_WAIT_TIMES) {
        fipc_print_error("fipc_opipe_rst_halt wait abort status timeout");
        fipc_opipe_devint_clr(pdev->base_va, opipe_id, FIPC_INT_WR_ABORT);
        fipc_opipe_local_dis(pdev->base_va, opipe_id);
        opipe_wptr = fipc_opipe_wptr_get(pdev->base_va, opipe_id);
        fipc_opipe_rptr_update(pdev->base_va, opipe_id, opipe_wptr);
        *(volatile u32 *)(pchannel->opipe_cfg->ptr_vaddr) = opipe_wptr;
        for (query_times = 0; query_times < FIPC_HALT_QUERY_WAIT_TIMES; query_times++) {
            if (0x1 & fipc_opipe_local_status(pdev->base_va, opipe_id)) {
                break;
            }
        }
        if (query_times >= FIPC_HALT_QUERY_WAIT_TIMES) {
            return FIPC_ERR_HALT_QUERY_WAIT_TIMEOUT;
        }
        fipc_opipe_abort_clr(pdev->base_va, opipe_id);
        return 0;
    }
    fipc_opipe_devint_clr(pdev->base_va, opipe_id, FIPC_INT_WR_ABORT);
    fipc_opipe_local_dis(pdev->base_va, opipe_id);
    fipc_opipe_abort_clr(pdev->base_va, opipe_id);
    opipe_wptr = fipc_opipe_wptr_get(pdev->base_va, opipe_id);
    fipc_opipe_rptr_update(pdev->base_va, opipe_id, opipe_wptr);
    *(volatile u32 *)(pchannel->opipe_cfg->ptr_vaddr) = opipe_wptr;
    for (query_times = 0; query_times < FIPC_HALT_QUERY_WAIT_TIMES; query_times++) {
        if (0x1 & fipc_opipe_local_status(pdev->base_va, opipe_id)) {
            break;
        }
    }
    if (query_times >= FIPC_HALT_QUERY_WAIT_TIMES) {
        return FIPC_ERR_HALT_QUERY_WAIT_TIMEOUT;
    }
    return 0;
}

void fipc_rst_halt_all_sendtomdm_chn_opipe(void)
{
    int ret = 0;
    u32 idx = 0;
    struct fipc_device *pdev = NULL;
    struct fipc_channel *pchannel = NULL;
    while (fipc_dev_chn_get_byidx(idx, &pdev, &pchannel) == 0) {
        idx++;
        if (pchannel->state != FIPC_CHN_OPEN) {
            continue;
        }

        if ((pchannel->const_flags & FIPC_CHN_SF_MDMRST_CARE) == 0) {
            continue;
        }

        if (isMsgRecvChn(pchannel)) {
            pchannel->state = FIPC_CHN_ORESETTED;
            continue;
        }
        ret = fipc_opipe_rst_halt(pdev, pchannel);
        if (ret) {
            fipc_print_error("dev[%d] pipe[%d] halt fail, ret is 0x%x\n", pdev->id, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id), ret);
        }
        pchannel->state = FIPC_CHN_ORESETTED;
    }
    return;
}

void fipc_rst_disable_all_mdm_chn_ipipe(void)
{
    u32 idx = 0;
    struct fipc_device *pdev = NULL;
    struct fipc_channel *pchannel = NULL;

    while (fipc_dev_chn_get_byidx(idx, &pdev, &pchannel) == 0) {
        idx++;
        if (pchannel->state != FIPC_CHN_ORESETTED) {
            continue;
        }
        if (!isMsgRecvChn(pchannel)) {
            pchannel->state = FIPC_CHN_IRESETTING;
            continue;
        }
        fipc_ipipe_local_dis(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id));
        pchannel->state = FIPC_CHN_IRESETTING;
    };
}


int fipc_rst_check_all_recvmdm_ipipe(void)
{
    int ret;
    u32 idx = 0;
    struct fipc_device *pdev = NULL;
    struct fipc_channel *pchannel = NULL;
    struct fipc_reset_dbg *fipc_reset_dbg = &g_fipc_reset_dbg;

    while (fipc_dev_chn_get_byidx(idx, &pdev, &pchannel) == 0) {
        idx++;
        if (pchannel->state != FIPC_CHN_IRESETTING) {
            continue;
        }
        if (!isMsgRecvChn(pchannel)) {
            pchannel->state = FIPC_CHN_IRESETTED;
            continue;
        }
        ret = ipipe_neg_check(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id));
        if (ret == 0) {
            ret = ipipe_status_check(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id));
        }
        if (ret) {
            if (fipc_reset_dbg->chn_id != pchannel->user_id) {
                fipc_print_error("channel %d pm ipipe ret %d\n", pchannel->user_id, ret);
            }
            fipc_reset_dbg->chn_id = pchannel->user_id;
            fipc_reset_dbg->last_step = (u32)ret;
            return ret;
        }
        pchannel->state = FIPC_CHN_IRESETTED;
    };
    return 0;
}

void fipc_rst_pipe_rptr_config(void)
{
    u32 idx = 0;
    struct fipc_device *pdev = NULL;
    struct fipc_channel *pchannel = NULL;

    while (fipc_dev_chn_get_byidx(idx, &pdev, &pchannel) == 0) {
        idx++;
        if (pchannel->state != FIPC_CHN_IRESETTED) {
            continue;
        }
        if (isMsgSendChn(pchannel)) {
            pchannel->opipe_cfg->rptr = pchannel->opipe_cfg->wptr;
            fipc_opipe_rptr_update(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id), pchannel->opipe_cfg->rptr);
            *(volatile u32 *)(pchannel->opipe_cfg->ptr_vaddr) = pchannel->opipe_cfg->rptr;
        }
        if (isMsgRecvChn(pchannel)) {
            pchannel->ipipe_cfg->wptr = pchannel->ipipe_cfg->rptr;
            pchannel->ipipe_cfg->seek = pchannel->ipipe_cfg->rptr;
            fipc_ipipe_wptr_update(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id), pchannel->ipipe_cfg->wptr);
            *(volatile u32 *)(pchannel->ipipe_cfg->ptr_vaddr) = pchannel->ipipe_cfg->wptr;
        }
        pchannel->state = FIPC_CHN_RESETTED;
    };
    return;
}


int fipc_chn_before_resetting(void)
{
    int ret;
    struct fipc_reset_dbg *fipc_reset_dbg = &g_fipc_reset_dbg;

    fipc_reset_dbg->reset_stat[0x0]++;

    fipc_rst_halt_all_sendtomdm_chn_opipe();

    fipc_reset_dbg->reset_stat[0x1]++;

    fipc_rst_disable_all_mdm_chn_ipipe();

    ret = fipc_rst_check_all_recvmdm_ipipe();
    if (ret) {
        fipc_print_error("reset check recv form modem ipipe status failed, ret %d\n", ret);
        fipc_reset_dbg->reset_stat[0x2]++;
        return ret;
    }

    /* use sec_call to teeos close ipipo */
    ret = bsp_sec_call(FUNC_MDRV_FIPC_CAOPTS, FIPC_BEFORE_RESET_CMD);
    if (ret != 0) {
        fipc_print_error("before reset bsp_sec_call return error,ret is %d.\n", ret);
        fipc_reset_dbg->reset_stat[0x3]++;
        return ret;
    }

    fipc_rst_pipe_rptr_config();

    fipc_reset_dbg->before_reset_times++;
    return 0;
}

int fipc_chn_after_resetting(void)
{
    u32 idx = 0;
    int ret, i;
    struct fipc_device *pdev = NULL;
    struct fipc_channel *pchannel = NULL;
    struct fipc_reset_dbg *fipc_reset_dbg = &g_fipc_reset_dbg;

    /* use sec_call to teeos open ipipo */
    ret = bsp_sec_call(FUNC_MDRV_FIPC_CAOPTS, FIPC_AFTER_RESET_CMD);
    if (ret != 0) {
        fipc_print_error("after reset bsp_sec_call return error,ret is %d.\n", ret);
        return ret;
    }

    while (fipc_dev_chn_get_byidx(idx, &pdev, &pchannel) == 0) {
        idx++;
        if (pchannel->state != FIPC_CHN_RESETTED) {
            continue;
        }
        if (isMsgRecvChn(pchannel)) {
            fipc_ipipe_enable(pdev, pchannel);
        } else if (pchannel->type == FIPC_CHN_TYPE_MSG_OUTSEND) {
            fipc_opipe_enable(pdev, pchannel);
        } else {
            fipc_opipe_enable(pdev, pchannel);
        }
        pchannel->state = FIPC_CHN_OPEN;
    };
    fipc_reset_dbg->after_reset_times++;

    for (i = 0; i < FIPC_RESET_DBG_STEPS_CNT; i++) {
        fipc_reset_dbg->reset_stat[i] = 0;
    }
    return 0;
}
