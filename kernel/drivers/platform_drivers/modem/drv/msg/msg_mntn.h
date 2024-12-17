/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2018. All rights reserved.
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


#ifndef _MSG_MNTN_H_
#define _MSG_MNTN_H_

#include "msg_plat.h"

#define DUMP_SEND_FLAG 0
#define DUMP_RECV_FLAG 1
#define MSG_DUMP_MID 0X3666
#define MSG_DUMP_LITE_MAGIC 0x4554494C
#define MSG_DUMP_CROSS_MAGIC 0x534F5243
#define MSG_DUMP_INTER_MAGIC 0x45544E49
#define MSG_DUMP_TX_FAIL_MAGIC 0x4E415254
#define MSG_DUMP_CROSS_TX_NUM 0x8
#define MSG_DUMP_LITE_TX_NUM 0x10

/*Process code used for tracing*/
#define MSG_TRACE_LITE_BEFORE (100)
#define MSG_TRACE_LITE_AFTER (101)
#define MSG_TRACE_CROSS_BEFORE (102)
#define MSG_TRACE_CROSS_AFTER (103)
#define MSG_TRACE_INTER_SEND_COMPLETE (104)
#define MSG_TRACE_INTER_RECV_COMPLETE (105)

typedef struct {
    s16 trace_code;
    u16 flag;
    u16 src_mid;
    u16 dst_mid;
    u32 time_stamp;
    u32 len;
} msg_dump_s;

typedef struct {
    u16 opposite_coreid;
    s16 trace_code;
    u16 src_chnid;
    u16 dst_chnid;
    u32 start_time;
    u32 end_time;
    u32 len;
} lite_dump_s;

typedef struct {
    msg_dump_s cross_pm_dump;
    lite_dump_s lite_pm_dump;
} msg_pm_dump_info_s;

typedef struct {
    u32 lite_info_cnt;
    u32 lite_dump_size;
    u32 lite_dump_pos;
    lite_dump_s *lite_dumps;
} lite_dump_info_s;

typedef struct {
    u32 cross_info_cnt;
    u32 cross_dump_size;
    u32 cross_dump_pos[2];
    msg_dump_s *cross_dumps;
} cross_dump_info_s;

typedef struct {
    u32 inter_info_cnt;
    u32 inter_dump_size;
    u32 inter_dump_pos;
    msg_dump_s *inter_dumps;
} inter_dump_info_s;

typedef struct {
    struct spinlock lite_lock;
    struct spinlock cross_lock;
} msg_dump_spinlock_s;

typedef struct {
    u16 cross_tx_size;
    u16 cross_tx_pos;
    u16 lite_tx_size;
    u16 lite_tx_pos;
    msg_dump_s *cross_tx;
    lite_dump_s *lite_tx;
} tx_fail_info_s;

typedef struct {
    u8 msg_pm_flag;
    u8 msg_irq_wakeup;
    u8 *dump_base;
    u32 dump_size;
    lite_dump_info_s lite_dump_info;
    cross_dump_info_s cross_dump_info;
    inter_dump_info_s inter_dump_info;
    msg_pm_dump_info_s msg_pm_dump_info;
    tx_fail_info_s tx_fail_info;
} msg_dump_info_s;

typedef struct {
    u32 dump_magic;
    u32 dump_size;
} msg_dump_head_s;

void cross_dump_record(msg_dump_s *pinfo, u8 direction, s16 trace_code);
void inter_dump_record(msg_dump_s *pinfo);
void lite_dump_record(const struct msg_addr *src, const struct msg_addr *dst, u32 len, u32 record_pos, s16 trace_code);
void lite_assign_dump_pos(struct msg_chn_hdl *msg_fd_info);
void get_msg_dump_info(msg_dump_s *info, void *msg);
void lite_tx_record(u32 record_pos, s16 trace_code);
void cross_tx_record(msg_dump_s *pinfo, s16 trace_code);
void msg_dump_resume(void);
int msg_irq_wakeup_set(void *cbk_arg);
void msg_diag_trans_report(msg_dump_s *info);
void msg_diag_pm_report(msg_pm_dump_info_s *info);
int msg_mntn_init(void);
#endif
