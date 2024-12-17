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
 *
 */
#include <osl_types.h>
#include <mdrv_timer.h>
#include <bsp_diag_cmdid.h>
#include "msgsrv.h"
#include "logreport.h"
#include "transfertask.h"
#include "connectsrv.h"
#include "diag_debug.h"


static void config_printlog(u8 *data, u32 datasz)
{
    u32 ret;
    struct srvhead *srvhd = (struct srvhead *)data;
    struct ptlvl_cfg *ptlvl = (struct ptlvl_cfg *)srvhd->data;

    set_printlog_onoff(ptlvl, datasz - sizeof(struct srvhead));

    ret = srvmsg_response(DIAG_CMD_LOG_CAT_PRINT, srvhd, 0);
    if (ret) {
        diag_err("config print log onoff response fail:%d\n", ret);
    }
}

static void config_drvprint(u8 *data, u32 datasz)
{
    u32 ret;
    struct srvhead *srvhd = (struct srvhead *)data;
    u32 *drvlevel = (u32 *)srvhd->data;

    set_drvprint_onoff(*drvlevel);

    ret = srvmsg_response(DIAG_CMD_DRV_PRINT_CFG, srvhd, 0);
    if (ret) {
        diag_err("config drv print log onoff response fail:%d\n", ret);
    }
}

static void config_tracelog(u8 *data, u32 datasz)
{
    u32 ret;
    struct srvhead *srvhd = (struct srvhead *)data;
    struct trace_cfg *tcfg = (struct trace_cfg *)srvhd->data;

    set_tracelog_onoff(tcfg, datasz - sizeof(struct srvhead));

    ret = srvmsg_response(DIAG_CMD_LOG_CAT_TRACE, srvhd, 0);
    if (ret) {
        diag_err("config trace log onoff response fail:%d\n", ret);
    }
}

static void config_translog(u8 *data, u32 datasz)
{
    u32 ret;
    struct srvhead *srvhd = (struct srvhead *)data;
    u32 *transonoff = (u32 *)srvhd->data;

    set_translog_onoff(*transonoff);

    ret = srvmsg_response(DIAG_CMD_LOG_TRANS, srvhd, 0);
    if (ret) {
        diag_err("config trans log onoff response fail:%d\n", ret);
    }
    bsp_diag_dpa_set_translog_onoff(*transonoff);
}

struct cnftimer {
    timer_attr_s tmattr;
    timer_handle tmhandle;
};
enum {
    CNFTIMER_FLOWTEST = 0,
    CNFTIMER_MNTN,
    CNFTIMER_BUTT
};
#define INVALID_CNFTIMER_HANDLE (-1)
#define FLOWTEST_TIMEOUT (1000000)   /* 1s */
#define MNTN_TIMEOUT (3 * 1000000)   /* 3s */

extern struct cnftimer g_cnftimer[CNFTIMER_BUTT];

static int mntnsrv(int para)
{
    bool cntstate = is_tool_connected();
    struct cnftimer *mntntimer = &g_cnftimer[CNFTIMER_MNTN];

    logreport_mntn(mntntimer->tmattr.time, cntstate);

    diag_dstch_mntn(mntntimer->tmattr.time, cntstate);

    if (cntstate != TOOLCNT_STATE_CONNECT) {
        mntntimer->tmhandle = INVALID_CNFTIMER_HANDLE;
        return 0;
    }

    mntntimer->tmhandle = mdrv_timer_add(&mntntimer->tmattr);

    return 0;
}

static int flowtestsrv(int para)
{
    struct cnftimer *fttimer = &g_cnftimer[CNFTIMER_FLOWTEST];

    if (!is_tool_connected() && (fttimer->tmhandle >= 0)) {
        fttimer->tmhandle = INVALID_CNFTIMER_HANDLE;
        return 0;
    }

    flowtest_log_report(para);

    fttimer->tmhandle = mdrv_timer_add(&fttimer->tmattr);

    return 0;
}

struct cnftimer g_cnftimer[CNFTIMER_BUTT] = {
    {{"cnfflowtest", FLOWTEST_TIMEOUT, TIMER_WAKE, flowtestsrv, 0}, INVALID_CNFTIMER_HANDLE},
    {{"cnfmntn", MNTN_TIMEOUT, 0, mntnsrv, 0}, INVALID_CNFTIMER_HANDLE},
};

/* used for mntnsrv and ratesrv timer */
struct timercfg {
    u16 onoff;
    u16 period;
};

static void start_mntnsrv(u8 *data, u32 datasz)
{
    u32 ret;
    struct srvhead *srvhd = (struct srvhead *)data;
    struct timercfg *timercfg = (struct timercfg *)srvhd->data;
    struct cnftimer *mntntimer = &g_cnftimer[CNFTIMER_MNTN];

    if (mntntimer->tmhandle >= 0) {
        (void)mdrv_timer_delete(mntntimer->tmhandle);
        mntntimer->tmhandle = INVALID_CNFTIMER_HANDLE;
    }

    if (timercfg->onoff) {
        /* add single time period ms */
        mntntimer->tmattr.time = timercfg->period * 1000;
        mntntimer->tmhandle = mdrv_timer_add(&mntntimer->tmattr);
    }

    ret = srvmsg_response(DIAG_CMD_LOG_MNTN, srvhd, 0);
    if (ret) {
        diag_err("mntn srv response fail:%d\n", ret);
    }
}

struct flowtest_cfg {
    u32 apperiod;
    u32 tspperiod;
    u32 aponoff;
    u32 tsponoff;
    u32 testcnt;
};

static void start_flowtest(u8 *data, u32 datasz)
{
    u32 ret = 0;
    struct srvhead *srvhd = (struct srvhead *)data;
    struct flowtest_cfg *ftcfg = (struct flowtest_cfg *)srvhd->data;
    struct cnftimer *fttimer = &g_cnftimer[CNFTIMER_FLOWTEST];

    if (fttimer->tmhandle >= 0) {
        (void)mdrv_timer_delete(fttimer->tmhandle);
        fttimer->tmhandle = INVALID_CNFTIMER_HANDLE;
    }

    if (ftcfg->aponoff) {
        /* add single apperiod seconds */
        fttimer->tmattr.time = ftcfg->apperiod * 1000000;
        fttimer->tmattr.para = ftcfg->testcnt;
        fttimer->tmhandle = mdrv_timer_add(&fttimer->tmattr);
    }

    ret = srvmsg_response(DIAG_CMD_FLOW_TEST, srvhd, 0);
    if (ret) {
        diag_err("mntn srv response fail:%d\n", ret);
    }
}

void configsrv_msg_handle(u32 msgid, u8 *data, u32 datasz)
{
    switch (msgid) {
        case TTMSG_PRINTLOG_CNF:
            config_printlog(data, datasz);
            break;

        case TTMSG_DRVPRINT_CNF:
            config_drvprint(data, datasz);
            break;

        case TTMSG_TRACELOG_CNF:
            config_tracelog(data, datasz);
            break;

        case TTMSG_TRANSLOG_CNF:
            config_translog(data, datasz);
            break;

        case TTMSG_MNTN_CNF:
            start_mntnsrv(data, datasz);
            break;

        case TTMSG_FLOWTEST_CNF:
            start_flowtest(data, datasz);
            break;

        default:
            diag_err("recv invalid config msg id:%d\n", msgid);
    }
}

