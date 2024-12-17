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
#ifndef __DIAG_MSG_H__
#define __DIAG_MSG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

struct msghead;

#define DIAGMSG_SLICE_MAX_LEN (4 * 1024)
#define PAYLOAD_MAX_LEN (DIAGMSG_SLICE_MAX_LEN - sizeof(struct msghead))
#define PAYLOAD_MAX_CNT (16)
/* only msg slice of segidx=0 has p_id and p_len */
#define PACK_MAX_PAYLOAD_LEN (PAYLOAD_MAX_CNT * DIAGMSG_SLICE_MAX_LEN - sizeof(struct msghead))

struct packmsg {
    u32 mdmid;   /* modem id */
    u32 srcsn;
    u32 ssid;   /* not used for acpu */
    u32 msgtype;
    u32 encsrch;    /* use special encoder source chan to delevery data, default 0xFF use normal chan */
    u32 payloadid;
    u32 ver : 8;
    u32 rsv : 24;
    u8 *datahd;      /* data head to be added for pack */
    u32 datahdsz;    /* data head size */
    u8 *data;        /* data for pack */
    u32 datasz;      /* data size */
};

enum payloadtype {
    PAYLOAD_TYPE_CMD = 0,
    PAYLOAD_TYPE_AIR = 0x1,
    PAYLOAD_TYPE_TRACE = 0x2,
    PAYLOAD_TYPE_PRINT = 0x3,
    PAYLOAD_TYPE_EVENT = 0x4,
    PAYLOAD_TYPE_USERPLAN = 0x5,
    PAYLOAD_TYPE_VOLTE = 0x6,
    PAYLOAD_TYPE_TRANS = 0x7,
    PAYLOAD_TYPE_DS = 0xD,   /* data sample */
    PAYLOAD_TYPE_DT = 0xF,   /* driver test */
};

enum payloadmode {
    PAYLOAD_MODE_LTE = 0,
    PAYLOAD_MODE_TDS = 0x1,
    PAYLOAD_MODE_GSM = 0x2,
    PAYLOAD_MODE_UMTS = 0x3,
    PAYLOAD_MODE_1X = 0x4,
    PAYLOAD_MODE_EVDO = 0x5,
    PAYLOAD_MODE_NR = 0x6,
    PAYLOAD_MODE_NONE = 0xF,
};

enum payloadgid {
    PAYLOAD_GID_RSV = 0,
    PAYLOAD_GID_MSP = 0x1,
    PAYLOAD_GID_PS = 0x2,
    PAYLOAD_GID_PHY = 0x3,
    PAYLOAD_GID_DS = 0x4,   /* data sample use fdbg */
    PAYLOAD_GID_HSO = 0x5,
    PAYLOAD_GID_BSP = 0x9,
    PAYLOAD_GID_EASYRF = 0xa,
    PAYLOAD_GID_APBSP = 0xb,
    PAYLOAD_GID_AUDIO = 0xc,
    PAYLOAD_GID_APP = 0xe,
    PAYLOAD_GID_BUTT,
};

#define GEN_PAYLOADID(gid, mode, type, cmdid)  \
    (((gid) << 28) | ((mode) << 24) | ((type) << 19) | ((cmdid) & 0x7FFFF))

struct payload {
    u32 p_id;
    u16 p_len;
    u8 rsv;
    u8 ver;   /* payload version */
    u8 p_data[0];
};

/* OM service ID */
#define DIAG_MSG_SID 0x2

enum {
    DIAG_MSG_VER_4G = 0,
    DIAG_MSG_VER_5G = 1
};

/* id of subsystem generated diag msg */
enum {
    DIAG_MSG_SSID_UND = 0x0,   /*undefine */
    DIAG_MSG_SSID_ACPU = 0x1,
    DIAG_MSG_SSID_CCPU = 0x2,
    DIAG_MSG_SSID_TLDSP_BBE_NX = 0x3,
    DIAG_MSG_SSID_BBP_DEBUG = 0x4,
    DIAG_MSG_SSID_GUC_BBE_NX = 0x5,
    DIAG_MSG_SSID_HIFI = 0x6,
    DIAG_MSG_SSID_LTE_V_DSP = 0x7,
    DIAG_MSG_SSID_RSV0 = 0x8,   /* reserved */
    DIAG_MSG_SSID_MCU = 0x9,
    DIAG_MSG_SSID_TEE = 0xA,
    DIAG_MSG_SSID_RSV1 = 0xB,   /* reserved */
    DIAG_MSG_SSID_IOM3 = 0xC,
    DIAG_MSG_SSID_EASYRF0 = 0xD,
    DIAG_MSG_SSID_X_DSP = 0xE,
    DIAG_MSG_SSID_GUC_L1C = 0xE,
    DIAG_MSG_SSID_RSV2 = 0xF,   /* reserved */
    DIAG_MSG_SSID_5G_CCPU = 0x10,
    DIAG_MSG_SSID_L2HAC = 0x11,
    DIAG_MSG_SSID_HL1C = 0x12,
    DIAG_MSG_SSID_LL1C_CORE0 = 0x13,
    DIAG_MSG_SSID_LL1C_CORE1 = 0x14,
    DIAG_MSG_SSID_LL1C_CORE2 = 0x15,
    DIAG_MSG_SSID_LL1C_CORE3 = 0x16,
    DIAG_MSG_SSID_LL1C_CORE4 = 0x17,
    DIAG_MSG_SSID_LL1C_CORE5 = 0x18,
    DIAG_MSG_SSID_SDR_CORE0 = 0x19,
    DIAG_MSG_SSID_SDR_CORE1 = 0x1A,
    DIAG_MSG_SSID_SDR_CORE2 = 0x1B,
    DIAG_MSG_SSID_SDR_CORE3 = 0x1C,
    DIAG_MSG_SSID_5G_BBP_DEBUG = 0x1D,
    DIAG_MSG_SSID_EASYRF1 = 0x1E,
    DIAG_MSG_SSID_BBP_ACCESS_DEBUG = 0x1F,
    DIAG_MSG_SSID_BUTT,
};

enum {
    DIAG_MSG_TYPE_UND = 0,   /* undefine */
    DIAG_MSG_TYPE_CMD = 0x1,
    DIAG_MSG_TYPE_CNF = 0x2,   /* confirm */
    DIAG_MSG_TYPE_LOG = 0x3,
};

#define DIAGMSG_NEW_VER 1

struct msghead {
    u32 sid : 4;    /* server id */
    u32 ver : 4;    /* service version */
    u32 mdmid : 3;  /* modem id */
    u32 rsv : 5;
    u32 ssid : 8;    /* subsystem id */
    u32 msgtype : 2;
    u32 segidx : 4;   /* segment index */
    u32 segend : 1;   /* end segment flag */
    u32 segment : 1;    /* segment flag */
    u16 srctransid;
    u16 odttransid;
    u32 timestamp;
    struct payload pyld;
};

/* tool connect msg use old msg head when ver equal 0 */
struct oldmsghead {
    u32 sid : 4;
    u32 ver : 4;
    u32 mdmid : 3;
    u32 rsv : 1;
    u32 ssid : 4;
    u32 srvid : 8;
    u32 msgtype : 2;
    u32 segidx : 4;
    u32 segend : 1;
    u32 segment : 1;
    u32 transid;
    u32 timelow;
    u32 timehigh;
    struct payload pyld;
};

u32 pack_diagmsg(struct packmsg *packmsg);

u32 parse_diagmsg(u8 *data, u32 len);

u32 diagmsg_init(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif   /* __DIAG_MSG_H__ */
