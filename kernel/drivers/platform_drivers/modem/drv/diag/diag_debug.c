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
#include <securec.h>
#include <osl_types.h>
#include <msg_id.h>
#include <mdrv_diag.h>
#include <bsp_slice.h>
#include <bsp_diag.h>
#include <bsp_diag_cmdid.h>
#include "encoder.h"
#include "logreport.h"
#include "connectsrv.h"
#include "diagmsg.h"
#include "srvchan.h"
#include "diag_debug.h"


void logreport_mntn(u32 rptperiod, bool cntstate)
{
    u32 i, ret, curslice, deltslice;
    static u32 lastslice;
    u32 currentbytes[DIAG_RPTPTR_SRCH_BUTT];
    static u32 lastbytes[DIAG_RPTPTR_SRCH_BUTT];

    if (cntstate != TOOLCNT_STATE_CONNECT) {
        (void)memset_s((u8 *)lastbytes, sizeof(lastbytes), 0, sizeof(lastbytes));
        return;
    }

    curslice = bsp_get_slice_value();
    deltslice = (curslice - lastslice) * 0x3E8 / 0x8000;
    deltslice = (deltslice == 0) ? (rptperiod * 0x3E8) : deltslice;

    for (i = 0; i < DIAG_RPTPTR_SRCH_BUTT; i++) {
        currentbytes[i] = g_srch_mntn.datasize[i];
        g_srch_mntn.datarate[i] = ((currentbytes[i] - lastbytes[i]) / 0x400 * 0x3E8) / deltslice;
    }

    ret = bsp_diag_trans_report(DIAG_DEBUG_AP_SRC_MNTN_CMDID, DRV_MID_DIAG_APSS, (u8 *)&g_srch_mntn, \
        sizeof(g_srch_mntn));
    if (ret) {
        return;
    }

    lastslice = curslice;

    for (i = 0; i < DIAG_RPTPTR_SRCH_BUTT; i++) {
        lastbytes[i] = currentbytes[i];
    }
}

void logreport_interface_test(void)
{
    u32 ret;
    static int testcnt = 0;
    mdrv_diag_trans_ind_s translog = {0};
    mdrv_diag_trace_s tracelog = {0};

    testcnt++;

    ret = bsp_diag_drv_log_report(PRINTLOG_LEVEL_INFO, "drv print log report test:%d\n", testcnt);
    if (ret) {
        diag_err("diag drv log report err:%d\n", ret);
    }

    ret = mdrv_diag_log_report(CREATE_MODULE(0, 0, PRINTLOG_LEVEL_INFO, 0), 1, __FILE__, __LINE__, \
        "mdrv print log report test:%d\n", testcnt);
    if (ret) {
        diag_err("diag log report err:%d\n", ret);
    }

    translog.ulPid = 0x1;    /* timer's pid is 1 */
    translog.ulLength = sizeof(g_srch_mntn);
    translog.pData = (void *)&g_srch_mntn;
    ret = mdrv_diag_trans_report(&translog);
    if (ret) {
        diag_err("diag trans report err:%d\n", ret);
    }

    tracelog.sendpid = 1;
    tracelog.recvpid = 6;
    tracelog.length = 0;
    ret = mdrv_diag_trace_report(&tracelog);
    if (ret) {
        diag_err("diag trace report err:%d\n", ret);
    }
}

#define DIAG_TESTDATA_SIZE (128)
static u32 g_diag_testdata[DIAG_TESTDATA_SIZE];

void logreport_interface_llt01(void)
{
    u32 ret;
    mdrv_diag_trans_ind_s translog = {0};

    translog.ulPid = 0x1;    /* timer's pid is 1 */
    translog.ulLength = sizeof(g_diag_testdata);
    translog.pData = (void *)g_diag_testdata;
    ret = mdrv_diag_trans_report(&translog);
    if (ret) {
        diag_err("diag trans report err:%d\n", ret);
    }
}

void flowtest_log_report(int testcnt)
{
    int i;

    diag_err("flowtest log cnt:%d\n", testcnt);
    for (i = 0; i < testcnt; i++) {
        logreport_interface_llt01();
    }
}

struct dstsend_mntn {
    u32 recordptr[ENCODER_DSTCH_BUTT][ENCODER_DSTSEND_MNTN_BUTT];
    u32 recvslice[ENCODER_DSTCH_BUTT];
    u32 sendslice[ENCODER_DSTCH_BUTT];
    u32 recvrate[ENCODER_DSTCH_BUTT];
    u32 sendrate[ENCODER_DSTCH_BUTT];
    u32 ovfcnt[ENCODER_DSTCH_BUTT];
    u32 trfcnt[ENCODER_DSTCH_BUTT];
    u32 leftsize[ENCODER_DSTCH_BUTT];
};
struct dstsend_mntn  g_dstsend_mntn = {0};

u32 g_diag_mntn_netlink[ENCODER_DSTCH_BUTT][ENCODER_DSTSEND_MNTN_BUTT];

void diag_mntn_record_netlink(u32 curport, u32 dstchid, u32 type, u32 count)
{
    if (curport != SRVPORT_NETLINK) {
        return;
    }
    g_diag_mntn_netlink[dstchid][type] += count;
}

void diag_dstsend_record(u32 dstchid, u32 type, u32 count)
{
    g_dstsend_mntn.recordptr[dstchid][type] += count;
}

void diag_dstsend_leftsizercd(u32 dstchid, u32 leftsize)
{
    g_dstsend_mntn.leftsize[dstchid] = leftsize;
}

void diag_dstmntn_clear(void)
{
    (void)memset_s(&g_dstsend_mntn, sizeof(g_dstsend_mntn), 0, sizeof(g_dstsend_mntn));
}

void diag_dstch_mntn(u32 rptperiod, bool cntstate)
{
    u32 i, ret, curslice, deltslice;
    static u32 lastslice;
    u32 recvbytes[ENCODER_DSTCH_BUTT], sendbytes[ENCODER_DSTCH_BUTT];
    static u32 lastrecvbytes[ENCODER_DSTCH_BUTT], lastsendbytes[ENCODER_DSTCH_BUTT];

    if (cntstate != TOOLCNT_STATE_CONNECT) {
        (void)memset_s((u8 *)lastrecvbytes, sizeof(lastrecvbytes), 0, sizeof(lastrecvbytes));
        (void)memset_s((u8 *)lastsendbytes, sizeof(lastsendbytes), 0, sizeof(lastsendbytes));
        return;
    }
    curslice = bsp_get_slice_value();
    deltslice = curslice - lastslice;
    deltslice = (deltslice == 0) ? (rptperiod * 0x3E8) : (deltslice * 0x3E8 / 0x8000);

    for (i = 0; i < ENCODER_DSTCH_BUTT; i++) {
        recvbytes[i] = g_dstsend_mntn.recordptr[i][ENCODER_DSTSEND_TOTAL_LEN];
        sendbytes[i] = g_dstsend_mntn.recordptr[i][ENCODER_DSTSEND_SUCC_LEN];
        g_dstsend_mntn.recvrate[i] = ((recvbytes[i] - lastrecvbytes[i]) / 0x400 * 0x3E8) / deltslice;
        g_dstsend_mntn.sendrate[i] = ((sendbytes[i] - lastsendbytes[i]) / 0x400 * 0x3E8) / deltslice;
    }

    ret = bsp_diag_trans_report(DIAG_DEBUG_DST_MNTN_CMDID, DRV_MID_DIAG_APSS, (u8 *)&g_dstsend_mntn, \
        sizeof(g_dstsend_mntn));
    if (ret) {
        return;
    }

    lastslice = curslice;

    for (i = 0; i < ENCODER_DSTCH_BUTT; i++) {
        lastrecvbytes[i] = recvbytes[i];
        lastsendbytes[i] = sendbytes[i];
    }
}

void diag_show_dstch_mntn(void)
{
    u32 i;

    for (i = 0; i < ENCODER_DSTCH_BUTT; i++) {
        diag_err("dst chan:0x%x\n", i);
        diag_err("dst send start count:0x%x\n", g_dstsend_mntn.recordptr[i][ENCODER_DSTSEND_START_CNT]);
        diag_err("dst send chan null count:0x%x\n", g_dstsend_mntn.recordptr[i][ENCODER_DSTSEND_CHNULL_CNT]);
        diag_err("dst send buffer err count:0x%x\n", g_dstsend_mntn.recordptr[i][ENCODER_DSTSEND_BUFFERR_CNT]);
        diag_err("dst send total len:0x%x\n", g_dstsend_mntn.recordptr[i][ENCODER_DSTSEND_TOTAL_LEN]);
        diag_err("dst send lost len:0x%x\n", g_dstsend_mntn.recordptr[i][ENCODER_DSTSEND_LOST_LEN]);
        diag_err("dst send fail count:0x%x\n", g_dstsend_mntn.recordptr[i][ENCODER_DSTSEND_FAIL_CNT]);
        diag_err("dst send success len:0x%x\n", g_dstsend_mntn.recordptr[i][ENCODER_DSTSEND_SUCC_LEN]);
        diag_err("dst send success count:0x%x\n", g_dstsend_mntn.recordptr[i][ENCODER_DSTSEND_SUCC_CNT]);
        diag_err("dst chan recv rate:0x%x\n", g_dstsend_mntn.recvrate[i]);
        diag_err("dst chan send rate:0x%x\n", g_dstsend_mntn.sendrate[i]);
        diag_err("\n");
    }
}

void diag_show_netlink_mntn(void)
{
    u32 i;

    for (i = 0; i < ENCODER_DSTCH_BUTT; i++) {
        diag_err("dst chan:0x%x\n", i);
        diag_err("dst netlink send start count:0x%x\n", g_diag_mntn_netlink[i][ENCODER_DSTSEND_START_CNT]);
        diag_err("dst netlink send chan null count:0x%x\n", g_diag_mntn_netlink[i][ENCODER_DSTSEND_CHNULL_CNT]);
        diag_err("dst netlink send buffer err count:0x%x\n", g_diag_mntn_netlink[i][ENCODER_DSTSEND_BUFFERR_CNT]);
        diag_err("dst netlink send total len:0x%x\n", g_diag_mntn_netlink[i][ENCODER_DSTSEND_TOTAL_LEN]);
        diag_err("dst netlink send lost len:0x%x\n", g_diag_mntn_netlink[i][ENCODER_DSTSEND_LOST_LEN]);
        diag_err("dst netlink send fail count:0x%x\n", g_diag_mntn_netlink[i][ENCODER_DSTSEND_FAIL_CNT]);
        diag_err("dst netlink send success len:0x%x\n", g_diag_mntn_netlink[i][ENCODER_DSTSEND_SUCC_LEN]);
        diag_err("dst netlink send success count:0x%x\n", g_diag_mntn_netlink[i][ENCODER_DSTSEND_SUCC_CNT]);
        diag_err("\n");
    }
}

struct srvptr_mntn g_srvptr_mntn = {0};

void diag_show_srvptr(void)
{
    u32 i;

    diag_err("cur srv ptr:0x%x\n", g_srvptr_mntn.curptr);
    for (i = 0; i < DIAG_SRVPTR_MAX; i++) {
        diag_err("server point:0x%02x, type:0x%02x, para1:0x%04x, para2:0x%08x, slice:0x%08x\n", i, g_srvptr_mntn.srvptr[i].type, \
            g_srvptr_mntn.srvptr[i].para1, g_srvptr_mntn.srvptr[i].para2, g_srvptr_mntn.srvptr[i].slice);
    }
}

struct srch_mntn g_srch_mntn = {{0}, {0}, {0}};

void diag_reportptr_clear(void)
{
    (void)memset_s(&g_srch_mntn, sizeof(g_srch_mntn), 0, sizeof(g_srch_mntn));
}

void diag_show_reportptr(void)
{
    u32 i;

    for (i = 0; i < DIAG_RPTPTR_BUTT; i++) {
        diag_err("report point:0x%x, value:0x%x\n", i, g_srch_mntn.reportptr[i]);
    }

    for (i = 0; i < DIAG_RPTPTR_SRCH_BUTT; i++) {
        diag_err("src chan:0x%x report bytes value:0x%x, rptslice:0x%x, datarate:0x%x\n", i, g_srch_mntn.datasize[i], g_srch_mntn.rptslice[i], g_srch_mntn.datarate[i]);
    }
}

