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

#define FIPC_PM_DBG_STEPS_CNT 4

struct fipc_pm_dbg {
    u32 s_times;                        /* 成功的睡眠次数 */
    u32 r_times;                        /* 成功的唤醒次数 */
    u32 sr_stat[FIPC_PM_DBG_STEPS_CNT]; /* 一次睡眠唤醒的记录 */
    u32 chn_id;                         /* 最新睡眠失败通道 */
    u32 last_step;                      /* 最新睡眠失败通道失败位置 */
    u32 last_pm_suspend_time_start;     /* 最新进入低功耗起始时间戳 */
    u32 last_pm_suspend_time_end;       /* 最新进入低功耗结束时间戳 */
    u32 last_pm_suspend_time_failed;    /* 最新进入低功耗失败时间戳 */
    u32 last_pm_resume_time_start;      /* 最新退出低功耗起始时间戳 */
    u32 last_pm_resume_time_end;        /* 最新退出低功耗结束时间戳 */
    u32 last_pm_resume_time_failed;     /* 最新退出低功耗失败时间戳 */
};

struct fipc_pm_dbg g_fipc_pm_dbg;

void fipc_pm_suspend_start(void)
{
    g_fipc_pm_dbg.last_pm_suspend_time_start = fipc_get_slice_value();
}

void fipc_pm_suspend_end(void)
{
    g_fipc_pm_dbg.last_pm_suspend_time_end = fipc_get_slice_value();
}

void fipc_pm_suspend_failed(void)
{
    g_fipc_pm_dbg.last_pm_suspend_time_failed = fipc_get_slice_value();
}

void fipc_pm_resume_start(void)
{
    g_fipc_pm_dbg.last_pm_resume_time_start = fipc_get_slice_value();
}

void fipc_pm_resume_end(void)
{
    g_fipc_pm_dbg.last_pm_resume_time_end = fipc_get_slice_value();
}

void fipc_pm_resume_failed(void)
{
    g_fipc_pm_dbg.last_pm_resume_time_failed = fipc_get_slice_value();
}

void fipc_pm_disable_all_opipe(void)
{
    u32 idx = 0;
    struct fipc_device *pdev = NULL;
    struct fipc_channel *pchannel = NULL;
    while (fipc_dev_chn_get_byidx(idx, &pdev, &pchannel) == 0) {
        idx++;
        if (pdev->ctrl_level == FIPC_DEV_CONTROL_LEVEL_PART) {
            continue;
        }

        if (pchannel->state != FIPC_CHN_OPEN) {
            continue;
        }

        if (isMsgRecvChn(pchannel)) {
            pchannel->state = FIPC_CHN_OSUSPENDING;
            continue;
        }
        fipc_opipe_local_dis(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id));
        pchannel->state = FIPC_CHN_OSUSPENDING;
    };
    return;
}

int fipc_pm_checkpd_all_opipe(void)
{
    int ret;
    u32 idx = 0;
    struct fipc_device *pdev = NULL;
    struct fipc_channel *pchannel = NULL;
    struct fipc_pm_dbg *fipc_pm_dbg = &g_fipc_pm_dbg;

    while (fipc_dev_chn_get_byidx(idx, &pdev, &pchannel) == 0) {
        idx++;
        if (pdev->ctrl_level == FIPC_DEV_CONTROL_LEVEL_PART) {
            continue;
        }
        if (pchannel->state != FIPC_CHN_OSUSPENDING) {
            continue;
        }
        if (isMsgRecvChn(pchannel)) {
            pchannel->state = FIPC_CHN_OSUSPENDED;
            continue;
        }

        ret = opipe_status_check(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id));
        if (ret) {
            if (fipc_pm_dbg->chn_id != pchannel->user_id) {
                fipc_print_error("channel %d pm opipe ret %d\n", pchannel->user_id, ret);
            }
            fipc_pm_dbg->chn_id = pchannel->user_id;
            fipc_pm_dbg->last_step = (u32)ret;
            return ret;
        }
        pchannel->state = FIPC_CHN_OSUSPENDED;
    };
    return 0;
}

void fipc_pm_disable_all_ipipe(void)
{
    u32 idx = 0;
    struct fipc_device *pdev = NULL;
    struct fipc_channel *pchannel = NULL;

    while (fipc_dev_chn_get_byidx(idx, &pdev, &pchannel) == 0) {
        idx++;
        if (pdev->ctrl_level == FIPC_DEV_CONTROL_LEVEL_PART) {
            continue;
        }
        if (pchannel->state != FIPC_CHN_OSUSPENDED) {
            continue;
        }
        if (!isMsgRecvChn(pchannel)) {
            pchannel->state = FIPC_CHN_ISUSPENDING;
            continue;
        }
        fipc_ipipe_local_dis(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id));
        pchannel->state = FIPC_CHN_ISUSPENDING;
    };
}

int fipc_pm_checkpd_all_ipipe(void)
{
    int ret;
    u32 idx = 0;
    struct fipc_device *pdev = NULL;
    struct fipc_channel *pchannel = NULL;
    struct fipc_pm_dbg *fipc_pm_dbg = &g_fipc_pm_dbg;

    while (fipc_dev_chn_get_byidx(idx, &pdev, &pchannel) == 0) {
        idx++;
        if (pdev->ctrl_level == FIPC_DEV_CONTROL_LEVEL_PART) {
            continue;
        }
        if (pchannel->state != FIPC_CHN_ISUSPENDING) {
            continue;
        }
        if (!isMsgRecvChn(pchannel)) {
            pchannel->state = FIPC_CHN_ISUSPENDED;
            continue;
        }
        ret = ipipe_neg_check(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id));
        if (ret == 0) {
            ret = ipipe_status_check(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id));
        }
        if (ret) {
            if (fipc_pm_dbg->chn_id != pchannel->user_id) {
                fipc_print_error("channel %d pm ipipe ret %d\n", pchannel->user_id, ret);
            }
            fipc_pm_dbg->chn_id = pchannel->user_id;
            fipc_pm_dbg->last_step = (u32)ret;
            return ret;
        }
        pchannel->state = FIPC_CHN_ISUSPENDED;
    };
    return 0;
}

int fipc_pm_check_pipe_empty(void)
{
    u32 idx = 0;
    struct fipc_device *pdev = NULL;
    struct fipc_channel *pchannel = NULL;

    union ipipe_stat i_state = { 0 };
    union opipe_stat o_state = { 0 };
    struct fipc_pm_dbg *fipc_pm_dbg = &g_fipc_pm_dbg;

    while (fipc_dev_chn_get_byidx(idx, &pdev, &pchannel) == 0) {
        idx++;
        if (pdev->ctrl_level == FIPC_DEV_CONTROL_LEVEL_PART) {
            continue;
        }
        if (pchannel->state != (u32)FIPC_CHN_ISUSPENDED) {
            continue;
        }
        if (isMsgRecvChn(pchannel)) {
            i_state.val = fipc_ipipe_local_status(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id));
            if (!i_state.union_stru.is_empty) {
                fipc_pm_dbg->chn_id = pchannel->user_id;
                fipc_pm_dbg->last_step = FIPC_ERR_PM_IPIPE_NOT_EMPTY;
                return -1;
            }
        } else {
            o_state.val = fipc_opipe_local_status(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id));
            if (!o_state.union_stru.is_empty) {
                fipc_pm_dbg->chn_id = pchannel->user_id;
                fipc_pm_dbg->last_step = FIPC_ERR_PM_OPIPE_NOT_EMPTY;
                return -1;
            }
        }
        pchannel->state = FIPC_CHN_SUSPENDED;
    };
    return 0;
}

static void fipc_pm_recovery(void)
{
    u32 idx = 0;
    struct fipc_device *pdev = NULL;
    struct fipc_channel *pchannel = NULL;

    while (fipc_dev_chn_get_byidx(idx, &pdev, &pchannel) == 0) {
        idx++;
        if (pdev->ctrl_level == FIPC_DEV_CONTROL_LEVEL_PART) {
            continue;
        }
        if (pchannel->state == FIPC_CHN_INIT || pchannel->state == FIPC_CHN_OPEN ||
            pchannel->state == FIPC_CHN_OPENNOSR) {
            continue;
        }
        if (isMsgRecvChn(pchannel)) {
            fipc_ipipe_local_en(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id));
            pchannel->state = (u32)FIPC_CHN_OPEN;
        } else if (pchannel->type == FIPC_CHN_TYPE_MSG_OUTSEND) {
            fipc_opipe_local_en(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id));
            pchannel->state = (u32)FIPC_CHN_OPEN;
        } else {
            fipc_opipe_local_en(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id));
            pchannel->state = (u32)FIPC_CHN_OPEN;
        }
    };
    return;
}

static void fipc_pm_chn_save_config(void)
{
    u32 idx = 0;
    struct fipc_device *pdev = NULL;
    struct fipc_channel *pchannel = NULL;

    while (fipc_dev_chn_get_byidx(idx, &pdev, &pchannel) == 0) {
        idx++;
        if (pdev->ctrl_level == FIPC_DEV_CONTROL_LEVEL_PART) {
            continue;
        }
        if (pchannel->state != FIPC_CHN_SUSPENDED) {
            continue;
        }
        if (pchannel->opipe_cfg != NULL) {
            pchannel->opipe_cfg->rptr = fipc_opipe_rptr_get(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id));
        }
        if (pchannel->ipipe_cfg != NULL && pchannel->type != FIPC_CHN_TYPE_MSG_OUTSEND) {
            pchannel->ipipe_cfg->wptr = fipc_ipipe_wptr_get(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id));
        }
    };
    return;
}

int fipc_chn_suspend(void)
{
    int ret;
    struct fipc_pm_dbg *fipc_pm_dbg = &g_fipc_pm_dbg;

    fipc_pm_dbg->sr_stat[0x0]++;

    fipc_pm_disable_all_opipe();
    ret = fipc_pm_checkpd_all_opipe();
    if (ret) {
        fipc_pm_recovery();
        fipc_pm_dbg->sr_stat[0x1]++;
        return ret;
    }

    fipc_pm_disable_all_ipipe();
    ret = fipc_pm_checkpd_all_ipipe();
    if (ret) {
        fipc_pm_recovery();
        fipc_pm_dbg->sr_stat[0x2]++;
        return ret;
    }

    ret = fipc_pm_check_pipe_empty();
    if (ret) {
        fipc_pm_recovery();
        fipc_pm_dbg->sr_stat[0x3]++;
        return ret;
    }

    fipc_pm_chn_save_config();
    fipc_pm_dbg->s_times++;
    return 0;
}

void fipc_chn_resume(void)
{
    u32 idx = 0;
    struct fipc_device *pdev = NULL;
    struct fipc_channel *pchannel = NULL;
    struct fipc_pm_dbg *fipc_pm_dbg = &g_fipc_pm_dbg;

    while (fipc_dev_chn_get_byidx(idx, &pdev, &pchannel) == 0) {
        idx++;
        if (pdev->ctrl_level == FIPC_DEV_CONTROL_LEVEL_PART) {
            continue;
        }
        if (pchannel->state != FIPC_CHN_SUSPENDED) {
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
    fipc_pm_dbg->r_times++;

    fipc_pm_dbg->sr_stat[0x0] = 0;
    fipc_pm_dbg->sr_stat[0x1] = 0;
    fipc_pm_dbg->sr_stat[0x2] = 0;
    fipc_pm_dbg->sr_stat[0x3] = 0;
    return;
}

/* dev suspend 失败的时候，由pmsrhook负责进行fipc_chn_resume来恢复通道 */
int fipc_dev_suspend(void)
{
    u32 idx;
    struct fipc_device *pdev = NULL;
    for (idx = 0; idx < FIPC_DEVICE_NUM_MAX; idx++) {
        pdev = fipc_device_get_fast(idx);
        if (pdev == NULL) {
            continue;
        }
        if (pdev->ctrl_level == FIPC_DEV_CONTROL_LEVEL_PART) {
            continue;
        }
        if (pdev->ctrl_level != FIPC_DEV_CONTROL_LEVEL_HOST) {
            continue;
        }
    }

    return 0;
}

void fipc_dev_resume(void)
{
    u32 idx;
    struct fipc_device *pdev = NULL;
    for (idx = 0; idx < FIPC_DEVICE_NUM_MAX; idx++) {
        pdev = fipc_device_get_fast(idx);
        if (pdev == NULL) {
            continue;
        }
        if (pdev->ctrl_level == FIPC_DEV_CONTROL_LEVEL_PART) {
            continue;
        }
        /* restore dev configuration */
        if (pdev->ctrl_level == FIPC_DEV_CONTROL_LEVEL_HOST) {
        }
    }

    return;
}

u32 fipc_pmsr_dump_save(u8 *buf, u32 len)
{
    u32 used_len = sizeof(g_fipc_pm_dbg);
    if (len < used_len) {
        return 0;
    }
    if (memcpy_s(buf, len, &g_fipc_pm_dbg, sizeof(g_fipc_pm_dbg))) {
        return 0;
    }
    return used_len;
}
