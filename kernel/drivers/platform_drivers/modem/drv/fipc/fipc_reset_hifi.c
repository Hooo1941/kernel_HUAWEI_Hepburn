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
#include <linux/kthread.h>
#include <adrv_rdr_audio_soc.h>
#include <osl_sem.h>
#include <bsp_dump.h>
#include <bsp_ipc_fusion.h>
#include "fipc_platform.h"
#include "fipc_device.h"
#include "fipc_driver.h"
#include "fipc_core.h"
#include "fipc_pmsr.h"
#include "fipc_dump.h"
#include "fipc_reset.h"

#define FIPC_HIFI_RESET_PROC_NAME "MDRV"
#define FIPC_HIFI_RESET_CBFUNC_PRIO 0x10
#define FIPC_HIFI_RESET_WAIT_TIME_MS 600

enum fipc_hifi_reset_dbg_steps_type {
    FIPC_RESET_DBG_START = 0x0,
    FIPC_RESET_DBG_NOTIFY,
    FIPC_RESET_DBG_NOTIFY_TIMEOUT,
    FIPC_RESET_DBG_NOTIFY_RECV,
    FIPC_RESET_DBG_NOTIFY_DONE,
    FIPC_RESET_DBG_DONE,
};

enum fipc_hifi_reset_dbg_state_type {
    FIPC_RESET_DBG_STATE_COUNT = 0x0,
    FIPC_RESET_DBG_STATE_TIMESTAMP,
    FIPC_RESET_DBG_STATE_MAX,
};

#define FIPC_RESET_DBG_STEPS_CNT FIPC_RESET_DBG_NOTIFY_DONE

struct fipc_reset_dbg {
    u32 before_reset_times;                   /* before reset的成功次数 */
    u32 after_reset_times;                    /* after reset的成功次数 */
    u32 reset_stat[FIPC_RESET_DBG_STEPS_CNT][FIPC_RESET_DBG_STATE_MAX]; /* 一次单独复位的记录,
                                                                           第一个是累计次数，第二个是最近一次时间戳 */
    u32 chn_id;                                                         /* 最新reset失败通道 */
    u32 last_step;                            /* 最新reset失败通道失败位置 */
};

struct fipc_hifi_reset_control {
    ipc_handle_t nofity_c_handle;
    ipc_handle_t recv_c_handle;
    osl_sem_id wait_c_reply;
    u32 notify_c_ts;
    u32 recv_c_ts;
};

struct fipc_reset_dbg g_fipc_hifi_reset_dbg;
struct fipc_hifi_reset_control g_fipc_hifi_reset_ctrl;

int fipc_proc_hifi_reset(enum DRV_RESET_CALLCBFUN_MOMENT reset_status, int userdata)
{
    int ret;
    struct fipc_reset_dbg *hifi_reset_dbg = &g_fipc_hifi_reset_dbg;
    struct fipc_hifi_reset_control *hifi_rst_ctrl = &g_fipc_hifi_reset_ctrl;

    hifi_reset_dbg->reset_stat[FIPC_RESET_DBG_START][FIPC_RESET_DBG_STATE_COUNT]++;
    hifi_reset_dbg->reset_stat[FIPC_RESET_DBG_START][FIPC_RESET_DBG_STATE_TIMESTAMP] = fipc_get_slice_value();
    hifi_reset_dbg->last_step = FIPC_RESET_DBG_START;
    if (reset_status == DRV_RESET_CALLCBFUN_RESET_BEFORE) {
        hifi_reset_dbg->before_reset_times++;

        hifi_rst_ctrl->notify_c_ts = fipc_get_slice_value();
        fipc_print_error("fipc sents a notification to process hifi channel, ts: 0x%x\n", hifi_rst_ctrl->notify_c_ts);
        ret = bsp_ipc_send(hifi_rst_ctrl->nofity_c_handle);
        if (ret) {
            hifi_reset_dbg->reset_stat[FIPC_RESET_DBG_NOTIFY][FIPC_RESET_DBG_STATE_COUNT]++;
            hifi_reset_dbg->reset_stat[FIPC_RESET_DBG_NOTIFY][FIPC_RESET_DBG_STATE_TIMESTAMP] = fipc_get_slice_value();
            hifi_reset_dbg->last_step = FIPC_RESET_DBG_NOTIFY;
            fipc_print_error("fipc reset notify c ok \n");
            return -1;
        }

        ret = osl_sem_downtimeout(&hifi_rst_ctrl->wait_c_reply, msecs_to_jiffies(FIPC_HIFI_RESET_WAIT_TIME_MS));
        if (ret) {
            hifi_reset_dbg->reset_stat[FIPC_RESET_DBG_NOTIFY_TIMEOUT][FIPC_RESET_DBG_STATE_COUNT]++;
            hifi_reset_dbg->reset_stat[FIPC_RESET_DBG_NOTIFY_TIMEOUT][FIPC_RESET_DBG_STATE_TIMESTAMP] = fipc_get_slice_value();
            hifi_reset_dbg->last_step = FIPC_RESET_DBG_NOTIFY_TIMEOUT;
            fipc_print_error("status:%d waiting is timeout,ret is %d.\n", reset_status, ret);
            return -1;
        }
        hifi_rst_ctrl->recv_c_ts = fipc_get_slice_value();
        hifi_reset_dbg->last_step = FIPC_RESET_DBG_NOTIFY_DONE;
        fipc_print_error("fipc receives the notification of reseting completion, ts: 0x%x\n", hifi_rst_ctrl->recv_c_ts);
    } else if (reset_status == DRV_RESET_CALLCBFUN_RESET_AFTER) {
        hifi_reset_dbg->after_reset_times++;
        hifi_reset_dbg->last_step = FIPC_RESET_DBG_DONE;
        fipc_print_error("fipc after resetting finished\n");
    }

    return 0;
}

void fipc_hifi_reset_recv_c_notify(void *arg)
{
    osl_sem_up(&g_fipc_hifi_reset_ctrl.wait_c_reply);
    g_fipc_hifi_reset_dbg.reset_stat[FIPC_RESET_DBG_NOTIFY_RECV][FIPC_RESET_DBG_STATE_COUNT]++;
    g_fipc_hifi_reset_dbg.reset_stat[FIPC_RESET_DBG_NOTIFY_RECV][FIPC_RESET_DBG_STATE_TIMESTAMP] = fipc_get_slice_value();
    g_fipc_hifi_reset_dbg.last_step = FIPC_RESET_DBG_NOTIFY_RECV;
}

int fipc_reset_hifi_init(void)
{
    int ret;

    ret = bsp_ipc_open(IPC_TSP_INT_SRC_ACPU_HIFIRESET, &g_fipc_hifi_reset_ctrl.nofity_c_handle);
    if (ret) {
        fipc_print_error("fipc reset open s ipc failed\n");
        return -1;
    }

    ret = bsp_ipc_open(IPC_ACPU_INT_SRC_TSP_HIFIRESET, &g_fipc_hifi_reset_ctrl.recv_c_handle);
    if (ret) {
        fipc_print_error("fipc reset open r ipc failed\n");
        return -1;
    }

    ret = bsp_ipc_connect(g_fipc_hifi_reset_ctrl.recv_c_handle, fipc_hifi_reset_recv_c_notify, NULL);
    if (ret) {
        fipc_print_error("fipc reset connect ipc failed\n");
        return -1;
    }

    /* (const char *pname, hifi_reset_cbfunc pcbfun, int userdata, int priolevel); */
    ret = hifireset_regcbfunc(FIPC_HIFI_RESET_PROC_NAME, fipc_proc_hifi_reset, 0, FIPC_HIFI_RESET_CBFUNC_PRIO);
    if (ret) {
        fipc_print_error("hifi reset cb register failed\n");
        return -1;
    }

    osl_sem_init(0, &g_fipc_hifi_reset_ctrl.wait_c_reply);

    fipc_print_error("fipc reset init ok\n");
    return 0;
}

u32 fipc_hifi_dbg_dump_save(u8 *buf, u32 len)
{
    u32 used_len = sizeof(g_fipc_hifi_reset_dbg);
    if (len < used_len) {
        return 0;
    }
    if (memcpy_s(buf, len, &g_fipc_hifi_reset_dbg, sizeof(g_fipc_hifi_reset_dbg))) {
        return 0;
    }
    return used_len;
}
