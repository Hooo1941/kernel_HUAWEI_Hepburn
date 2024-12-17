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
#ifndef _BSP_CHR_H_
#define _BSP_CHR_H_

#include "mdrv_chr.h"
#include <osl_types.h>
#include "bsp_homi.h"

#define OM_NO_DIRECT_REPORT 0     /* 非直通alamtype类型 */ 
#define OM_DIRECT_REPORT_TYPE 6   /* 直通alamtype类型 */
#define CHR_BLOCKLIST_NOT_ALLOW 0x100 /* block list not allow */ 
#define CHR_FRAGMENT_LEN_MAX (3 * 1024 + 512)

typedef enum {
    OM_ERR_LOG_MSG_SET_ERR_REPORT = 0x01, /* 上层触发故障上报 */
    OM_ERR_LOG_RESET_INFO_REPORT = 0x03,  /* 控制上类型开关 */
    OM_ERR_LOG_MSG_ERR_REPORT = 0x4, /* 故障上报 */
    OM_ERR_LOG_MSG_FAULT_REPORT = 0x09, /* 平台检测故障主动上报 */
    OM_ERR_LOG_MSG_ALARM_REPORT = 0x0A, /* 平台检测告警主动上报 */
    OM_ERR_LOG_MSG_BLOCKLIST_CFG = 0x14, /* 黑名单配置*/
    OM_ERR_LOG_MSG_PRIORITY_CFG = 0x15, /* 优先级配置*/
    OM_ERR_LOG_MSG_PERIOD_CFG = 0x16, /* 周期配置*/
    OM_ERR_LOG_MSG_CFG_CNF = 0x17, /* 配置确认*/
    OM_ERR_LOG_MSG_CONNECT_CFG = 0x18, /* 工具链接命令 */
    OM_ERR_LOG_MSG_REQ_FAIL_CNF = 0x19, /* 查询信息失败返回消息 */
    OM_ERR_LOG_MSG_TYPE_BUTT /* 消息类型最大值*/
} chr_msg_type_e;

typedef enum {
    OM_ERR_LOG_MOUDLE_ID_GUNAS = 0x020001,   /* GU NAS */
    OM_ERR_LOG_MOUDLE_ID_GAS = 0x020002,     /* GAS */
    OM_ERR_LOG_MOUDLE_ID_WAS = 0x020003,     /* WAS */
    OM_ERR_LOG_MOUDLE_ID_GUL2 = 0x020004,    /* GUL2 */
    OM_ERR_LOG_MOUDLE_ID_GUPHY = 0x020005,   /* GUPHY */
    OM_ERR_LOG_MOUDLE_ID_USIMM = 0x020006,   /* USIMM */
    OM_ERR_LOG_MOUDLE_ID_DRV_SCI = 0x020007, /* SCI DRV */
    OM_ERR_LOG_MOUDLE_ID_HIFI = 0x020008,    /* HIFI */
    OM_ERR_LOG_MOUDLE_ID_RRM = 0x020009,     /* RRM */
    OM_ERR_LOG_MOUDLE_ID_VISP = 0x02000A,    /* VISP CHR */
    OM_ERR_LOG_MOUDLE_ID_GNAS = 0x020011, /* G NAS */
    OM_ERR_LOG_MOUDLE_ID_GL2 = 0x020014,  /* GL2 */
    OM_ERR_LOG_MOUDLE_ID_GPHY = 0x020015, /* GPHY */
    OM_ERR_LOG_MOUDLE_ID_UNAS = 0x020021, /* U NAS */
    OM_ERR_LOG_MOUDLE_ID_UL2 = 0x020024,  /* UL2 */
    OM_ERR_LOG_MOUDLE_ID_UPHY = 0x020025, /* UPHY */
    OM_ERR_LOG_MOUDLE_ID_DRV_MCI = 0x020016, /* MCI DRV */
    OM_ERR_LOG_MOUDLE_ID_LMM = 0x020030,   /* LTE EMM */
    OM_ERR_LOG_MOUDLE_ID_ESM = 0x020031,   /* LTE ESM */
    OM_ERR_LOG_MOUDLE_ID_LSMS = 0x020032,  /* LTE SMS(复用GU SMS，不会使用，预留) */
    OM_ERR_LOG_MOUDLE_ID_LPP = 0x020033,   /* LTE LPP */
    OM_ERR_LOG_MOUDLE_ID_LRRC = 0x020034,  /* LTE RRC */
    OM_ERR_LOG_MOUDLE_ID_LTEL2 = 0x020035, /* LTEL2 */
    OM_ERR_LOG_MOUDLE_ID_LPHY = 0x020036,  /* LTE PHY */
    OM_ERR_LOG_MOUDLE_ID_TDS_MM = 0x020040,  /* TDS MM */
    OM_ERR_LOG_MOUDLE_ID_TDS_GMM = 0x020041, /* TDS GMM */
    OM_ERR_LOG_MOUDLE_ID_TDS_CC = 0x020042,  /* TDS CC */
    OM_ERR_LOG_MOUDLE_ID_TDS_SM = 0x020043,  /* TDS SM */
    OM_ERR_LOG_MOUDLE_ID_TDS_SMS = 0x020044, /* TDS SMS(复用GU SMS，不会使用，预留) */
    OM_ERR_LOG_MOUDLE_ID_TDS_LPP = 0x020045, /* TDS LPP */
    OM_ERR_LOG_MOUDLE_ID_TRRC = 0x020046,    /* TDS RRC */
    OM_ERR_LOG_MOUDLE_ID_TDSL2 = 0x020047,   /* TDSL2 */
    OM_ERR_LOG_MOUDLE_ID_TDS_PHY = 0x020048, /* TDS PHY */
    OM_ERR_LOG_MOUDLE_ID_LPSOM = 0x020049,   /* LPS OM */
    /* 以下模块不具备收发消息功能，需借助COM模块中转 */
    OM_ERR_LOG_MOUDLE_ID_IMS = 0x020050,     /* IMS */
    OM_ERR_LOG_MOUDLE_ID_IMSA = 0x020051,    /* IMSA */
    OM_ERR_LOG_MOUDLE_ID_BBA_CHR = 0x020052, /* BBA CHR */
    OM_ERR_LOG_MOUDLE_ID_CNAS = 0x020060,    /* CNAS */
    OM_ERR_LOG_MOUDLE_ID_CAS = 0x020061,     /* CAS */
    OM_ERR_LOG_MOUDLE_ID_CL2 = 0x020062,     /* CL2 */
    OM_ERR_LOG_MOUDLE_ID_CPROC = 0x020063,   /* CPROC */
    OM_ERR_LOG_MOUDLE_ID_CSDR = 0x020064,    /* CSDR */
    OM_ERR_LOG_MOUDLE_ID_CHIFI = 0x020065,   /* CHIFI */
    OM_ERR_LOG_MOUDLE_ID_PPPC = 0x020066,    /* PPPC */
    OM_ERR_LOG_MOUDLE_ID_DRV = 0x020067,     /* DRV 用于底软低功耗上报 */
    OM_ERR_LOG_MOUDLE_ID_CHR_STA = 0x020068, /* OM维测 */
    OM_ERR_LOG_MOUDLE_ID_NRRC = 0x020069,    /* NRRC */
    OM_ERR_LOG_MOUDLE_ID_NRL2    = 0x02006A,   /* NRL2 */
    OM_ERR_LOG_MOUDLE_ID_NRMM = 0x020070, /* NR MM */
    OM_ERR_LOG_MOUDLE_ID_NRSM = 0x020071, /* NR SM */
    OM_ERR_LOG_MOUDLE_ID_NR_AGENT = 0x020080, /* NPHY AGENT */
    OM_ERR_LOG_MOUDLE_ID_BUTT
} chr_module_id_e;

typedef struct {
    unsigned int sid_4b : 4;    /* 服务ID，包括DIAG、CHR */
    unsigned int ver_4b : 4;    /* 版本号 */
    unsigned int mdmid_3b : 3;  /* modem id，下发时可填0 */
    unsigned int rsv_5b : 5;
    unsigned int ssid_8b : 8;   /* sub sys id，下发时可填0 */
    unsigned int mt_2b : 2;     /* msg type，REQ=1，CNF=2, IND=3 */
    unsigned int index_4b : 4;  /* 数据包标号(分包下发或上报有效) */
    unsigned int eof_1b : 1;    /* 是否为最后一包(分包下发或上报有效) */
    unsigned int ff_1b : 1;     /* 数据是否分包，0=不分包；1=分包 */

    unsigned int rsv;
    unsigned int timestamp;     /* 时间戳信息 */
} chr_oam_head_s;

typedef struct {
    chr_oam_head_s oam_head;
    unsigned int msg_type;  /* CHR下发的消息类型，参数定义chr_msg_type_e */
    unsigned int msg_sn;    /* CHR消息SN号，查询上报需要带回给工具 */
    unsigned int msg_len;   /* CHR消息长度 */
    unsigned char data[0];  /* 上报的实际数据内容 */
} chr_app_ind_s;

typedef struct {
    unsigned int msg_name; /* msg name:REQ/CNF/IND */
    unsigned int module_id; /* module id */
    unsigned short alarm_id; /* alarm id */
    unsigned short alarm_type; /* alarm type */
} chr_report_head_s;

typedef struct {
    chr_report_head_s report_head;
    unsigned int msg_type;  /* CHR下发的消息类型，参数定义chr_msg_type_e */
    unsigned int msg_sn;    /* CHR消息SN号，查询上报需要带回给工具 */
    unsigned int msg_len;   /* CHR消息长度 */
    unsigned char data[0];  /* 上报的实际数据内容 */
} chr_ind_info_s;

typedef struct {
    unsigned int moduleid; /* module id */
    unsigned short modemid; /* modem id */
    unsigned short alarmid; /* alarm id */
    unsigned short alarm_level; /* alarm level */
    unsigned short alarm_type; /* alarm type */
    unsigned int low_slice; /* low slice */
    unsigned int high_slice; /* high slice */
    unsigned int alarm_len; /* alarm len */
    unsigned char data[0]; /* 数据占位符 */
} chr_data_header_s;

typedef struct {
    unsigned int msg_name;      /* 下发的消息名(类型) */
    unsigned int module_id;     /* module id */
    unsigned short modem_id;    /* 应用层下发待查询的modem id */
    unsigned short alarm_id;    /* 应用层下发待查询的alarm id */
    unsigned int msg_sn;        /* 工具下发的msg_sn，查询上报需要组件带回 */
    unsigned char data[0];
} chr_req_trans_s;

int bsp_chr_genl_rcv_handler(struct sk_buff *skb, struct genl_info *info);
unsigned int bsp_chr_report(chr_ind_info_s *chr_data);


#endif
