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

#ifndef __CHR_CFG_H__
#define __CHR_CFG_H__

#include <linux/spinlock.h>
#include <mdrv_msg.h>
#include <bsp_chr.h>

#define CHR_MAX_CFG_LEN                     4096

/* 检查配置分包下发结果 */
#define CHR_PACKET_CHECK_COMPLETE           0
#define CHR_PACKET_CHECK_CONTINUE           1
#define CHR_PACKET_CHECK_RESTART            2
#define CHR_PACKET_CHECK_FAIL               3

// 检查黑名单、优先级的结果返回值
#define CHR_TRUE                            1
#define CHR_FALSE                           0

/* 周期单位默认值 */
#define CHR_PERIOD_DEFAULT                  1

#define CHR_COMM_HEAD_LEN                   (sizeof(chr_comm_head_s))
#define CHR_APP_PRIORITY_INFO_LEN           (sizeof(chr_app_priority_cfg_s) - CHR_COMM_HEAD_LEN)
#define CHR_APP_PERIOD_INFO_LEN             (sizeof(chr_app_period_cfg_s) - CHR_COMM_HEAD_LEN)
#define CHR_APP_REQ_MODULE_INFO_LEN         (sizeof(chr_app_req_s) - CHR_COMM_HEAD_LEN)

typedef enum {
    CHR_STATE_DISCONNECT,
    CHR_STATE_CONNECT,
    CHR_STATE_BUTT
} chr_state_e;

typedef struct {
    unsigned int msg_type;     /* CHR下发的消息类型，参数定义chr_msg_type_e */
    unsigned int msg_sn;       /* CHR消息SN号，查询上报需要带回给工具 */
    unsigned int msg_len;       /* CHR消息长度 */
}chr_info_head_s;

typedef struct {
    chr_oam_head_s oam_head;        /* 工具下发的OAM消息头 */
    chr_info_head_s info_head;      /* 工具下发的CHR消息头 */
    unsigned int module_id;         /* 应用层下发待查询的module id */
    unsigned short modem_id;        /* 应用层下发待查询的modem id */
    unsigned short alarm_id;        /* 应用层下发待查询的alarm id */
    unsigned char data[0]; /*lint !e43 */
} chr_app_req_s;

typedef struct {
    chr_oam_head_s oam_head;        /* 工具下发的OAM消息头 */
    chr_info_head_s info_head;      /* 工具下发的CHR消息头 */
    unsigned char data[0]; /*lint !e43 */
}chr_comm_head_s;

typedef struct {
    unsigned int conn_state;
} chr_app_conn_info_s;

typedef struct {
    unsigned int module_id;
    unsigned short alarm_id;
    unsigned short alarm_type;
} chr_blocklist_info_s;

typedef struct {
    chr_oam_head_s oam_head;
    chr_info_head_s info_head;
    chr_blocklist_info_s blocklist_cfg[0]; /*lint !e43 */
} chr_app_blocklist_cfg_s;

typedef struct {
    spinlock_t blocklist_lock;
    unsigned int blocklist_num;
    unsigned char *blocklist_info;
} chr_blocklist_cfg_s;

typedef struct {
    unsigned int module_id;
    unsigned short alarm_id;   /* alarm id */
    unsigned short priority;   /* 优先级 */
    unsigned short alarm_type; /* alarm type */
    unsigned short resv;
} chr_priority_info_s;

typedef struct {
    chr_oam_head_s oam_head;
    chr_info_head_s info_head;
    chr_priority_info_s priority_cfg[0]; /*lint !e43 */
} chr_app_priority_cfg_s;

typedef struct {
    unsigned int prio_num;
    unsigned char *prio_info;
} chr_prio_cfg_s;

typedef struct {
    unsigned int module_id; /* module id */
    unsigned int count;     /* 周期内1优先级允许上报次数 */
} chr_period_info_s;

typedef struct {
    chr_oam_head_s oam_head;
    chr_info_head_s info_head;
    chr_period_info_s period_cfg[0]; /*lint !e43 */
} chr_app_period_cfg_s;

typedef struct {
    unsigned int period_num;
    unsigned char *period_info;
} chr_period_cfg_s;

typedef struct {
    unsigned int conn_state;
    chr_blocklist_cfg_s blocklist_cfg;
    chr_prio_cfg_s prio_cfg;
    chr_period_cfg_s period_cfg;
} chr_cfg_info_s;

typedef struct {
    chr_oam_head_s oam_head;
    chr_info_head_s info_head;
    unsigned int module_id;     /* 应用层下发的module_id */
    unsigned short modem_id;    /* 应用层下发的modem_id */
    unsigned short alarm_id;    /* 应用层下发的alarm_id */
    unsigned int msg_sn;        /* 应用层下发的msg_sn */
    unsigned int result;        /* 回复给工具的结果 */
} chr_app_cnf_s;

unsigned int chr_get_connect_state(void);
int chr_connect_cfg_proc(unsigned char *data, unsigned int data_len);
int chr_app_query_proc(unsigned char *data, unsigned int data_len);
int chr_blocklist_cfg_proc(unsigned char *data, unsigned int data_len);
int chr_priority_cfg_proc(unsigned char *data, unsigned int data_len);
int chr_period_cfg_proc(unsigned char *data, unsigned int data_len);
void chr_cfg_send_to_modem(void);
unsigned int chr_check_blocklist(unsigned int module_id, unsigned short alarm_id, unsigned short alarm_type);
void chr_cfg_init(void);

#endif

