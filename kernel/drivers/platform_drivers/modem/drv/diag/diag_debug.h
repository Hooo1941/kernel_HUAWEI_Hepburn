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
#ifndef __DIAG_DEBUG_H__
#define __DIAG_DEBUG_H__

#include <osl_types.h>
#include <linux/printk.h>
#include <bsp_slice.h>
#include <bsp_print.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_MODU
#define THIS_MODU mod_diag

enum {
    DIAG_STATE_INIT_ERR = 1,

    DIAG_INVALID_LOGPARAM_ERR,
    DIAG_PRINTLOG_NOT_OPEN_ERR,
    DIAG_TRANSLOG_NOT_OPEN_ERR,
    DIAG_TRACELOG_NOT_OPEN_ERR,

    DIAG_PACKMSG_OVERFLOW_ERR,

    DIAG_PARSEMSG_DATA_ERR,
    DIAG_PARSEMSG_GET_MATCHED_MSG_ERR,
    DIAG_PARSEMSG_GET_INVALID_MSG_ERR,
    DIAG_PARSEMSG_MERGE_MSGSLICE_ERR,
    DIAG_PARSEMSG_MERGE_MSGLEN_ERR,

    DIAG_MSG_REGISTER_WAKELOCK_ERR,

    DIAG_ENCODE_INVALID_SRCHTYPE_ERR,
    DIAG_ENCODE_GET_SRCBUF_ERR,
    DIAG_ENCODE_OVERFLOW_SRCBUF_ERR,
    DIAG_ENCODE_GET_ODTPDEV_ERR,
    DIAG_ENCODE_DMA_MALLOC_ERR,
    DIAG_ENCODE_GET_DSTCH_ERR,
    DIAG_ENCODE_GET_DSTBUF_ERR,

    DIAG_SRVPORT_NOTCONNECT_ERR,
    DIAG_SRVPORT_ALLOC_CHAN_ERR,
    DIAG_SRVPORT_GET_POINT_ERR,
    DIAG_SRVPORT_PORT_TYPE_ERR,
    DIAG_SRVPORT_SETLOGMODE_ERR,
    DIAG_SRVPORT_NETLINK_DATA_ERR,
    DIAG_SRVPORT_NETLINK_CMD_ERR,
    DIAG_SRVPORT_NLDATA_MALLOC_ERR,
    DIAG_SRVPORT_MEMCPY_ERR,
    DIAG_SRVPORT_NLWALELOCK_INIT_ERR,

    DIAG_TRANSFERTASK_CREATE_ERR,
    DIAG_TRANSFERTASK_MALLOC_ERR,
};

#define diag_err(fmt, ...) printk(KERN_ERR "[%s]:" fmt, BSP_MOD(THIS_MODU), ##__VA_ARGS__)
#define diag_dbg(fmt, ...)

void logreport_mntn(u32 rptperiod, bool cntstate);

void flowtest_log_report(int testcnt);

/* mntn type of encoder dst send */
enum {
    ENCODER_DSTSEND_START_CNT = 0,
    ENCODER_DSTSEND_CHNULL_CNT,
    ENCODER_DSTSEND_BUFFERR_CNT,
    ENCODER_DSTSEND_TOTAL_LEN,
    ENCODER_DSTSEND_LOST_LEN,
    ENCODER_DSTSEND_FAIL_CNT,
    ENCODER_DSTSEND_SUCC_LEN,
    ENCODER_DSTSEND_SUCC_CNT,

    ENCODER_DSTSEND_MNTN_BUTT
};

void diag_dstsend_record(u32 dstchid, u32 type, u32 count);

void diag_mntn_record_netlink(u32 curport, u32 dstchid, u32 type, u32 count);

void diag_dstsend_leftsizercd(u32 dstchid, u32 leftsize);

void diag_dstmntn_clear(void);

void diag_dstch_mntn(u32 rptperiod, bool cntstate);

enum {
    DIAG_SRVPTR_DECODE_DATA = 0,
    DIAG_SRVPTR_PARSEMSG_IN,
    DIAG_SRVPTR_SEND_DIAGMSG,
    DIAG_SRVPTR_PARSEMSG_END,

    DIAG_SRVPTR_CONNECT_END,
    DIAG_SRVPTR_AUTH_END,
    DIAG_SRVPTR_DISCONNECT_END,

    DIAG_SRVPTR_BUTT
};

#define DIAG_SRVPTR_MAX (64)
struct srvptr {
    u16 type;
    u16 para1;
    u32 para2;
    u32 slice;
};

struct srvptr_mntn {
    u16 curptr;
    struct srvptr srvptr[DIAG_SRVPTR_MAX];
};

extern struct srvptr_mntn g_srvptr_mntn;

/* used for record diag server flow */
static inline void diag_srvptr_record(u16 type, u32 para1, u32 para2)
{
    u32 ptr = g_srvptr_mntn.curptr;

    g_srvptr_mntn.srvptr[ptr].type = type;
    g_srvptr_mntn.srvptr[ptr].para1 = (u16)para1;
    g_srvptr_mntn.srvptr[ptr].para2 = para2;
    g_srvptr_mntn.srvptr[ptr].slice = bsp_get_slice_value();

    g_srvptr_mntn.curptr = (g_srvptr_mntn.curptr + 1) % DIAG_SRVPTR_MAX;
}

/* diag report point record type */
enum {
    /* SRCH_CNF ~ SRCH_LOG used for source chan lost len record, must start with 0 */
    DIAG_RPTPTR_SRCH_CNF = 0,
    DIAG_RPTPTR_SRCH_LOG,
    DIAG_RPTPTR_SRCH_BUTT,

    DIAG_RPTPTR_OVER_MAX_LEN = DIAG_RPTPTR_SRCH_BUTT,
    DIAG_RPTPTR_MEMCPY_SRCBUF,
    DIAG_RPTPTR_SRCRBBUF_NULL,
    DIAG_RPTPTR_ENCODE_MSG,

    DIAG_RPTPTR_BUTT
};

struct srch_mntn {
    u32 reportptr[DIAG_RPTPTR_BUTT];    /* report pointer */
    /* src chan report total data size(contain lost data size) */
    u32 datasize[DIAG_RPTPTR_SRCH_BUTT];
    u32 rptslice[DIAG_RPTPTR_SRCH_BUTT];
    u32 datarate[DIAG_RPTPTR_SRCH_BUTT];
};

extern struct srch_mntn g_srch_mntn;

static inline void diag_reportsize_record(u32 type, u32 para1)
{
    /* type always less than DIAG_RPTPTR_SRCH_BUTT, para1 used for counting data size */
    g_srch_mntn.datasize[type] += para1;
}

static inline void diag_reportptr_record(u32 type, u32 para1)
{
    /* para1 used for counting error */
    if (unlikely(type < DIAG_RPTPTR_BUTT)) {
        g_srch_mntn.reportptr[type] += para1;
    }
}

void diag_reportptr_clear(void);

void diag_show_data(u8 *data, u32 len);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
