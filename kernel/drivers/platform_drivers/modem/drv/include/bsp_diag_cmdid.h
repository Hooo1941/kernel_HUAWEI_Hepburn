/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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


#ifndef __DIAG_CMD_ID_DEF_H__
#define __DIAG_CMD_ID_DEF_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#define DIAG_CMD_HOST_CONNECT 0x10065000
#define DIAG_CMD_HOST_CONNECT_AGENT 0x10066000
#define DIAG_CMD_HOST_DISCONNECT 0x10065001
#define DIAG_CMD_MODEM_WILL_RESET 0x10014000
#define DIAG_CMD_GET_TIMESTAMP_VALUE 0x10014001
#define DIAG_CMD_GET_MODEM_NUM 0x10014002
#define DIAG_CMD_PID_TABLE_MSG 0x10014003

#define DIAG_CMD_TIMER_OUT_IND 0x10015100

#define DIAG_CMD_LOG_CAT_PRINT 0x10015310
#define DIAG_CMD_LOG_CAT_TRACE 0x10015311
#define DIAG_CMD_LOG_CAT_AIR 0x10015312
#define DIAG_CMD_LOG_CAT_L1 0x10015314
#define DIAG_CMD_LOG_CAT_EVENT 0x10015315
#define DIAG_CMD_LOG_CAT_CMD 0x10015316
#define DIAG_CMD_LOG_CAT_MSG 0x10015317
#define DIAG_CMD_DEBUG_MSG 0x10015318 /* debug 收到后会上报所有类型IND 消息 */
#define DIAG_CMD_LOG_USERPLANE 0x10015319
#define DIAG_CMD_LOG_TRANS 0x1001531A
#define DIAG_CMD_LOG_TIMESTAMP 0x1001531B
#define DIAG_CMD_LOG_MNTN 0x1001531C
#define DIAG_CMD_FLOW_TEST 0x1001531D
#define DIAG_CMD_DT_HB 0x1001531E
#define DIAG_CMD_DRV_PRINT_CFG 0x10015321


#define DIAG_CMD_GTR_SET 0x10015454
#define DIAG_CMD_GU_GTR_SET 0x10015455    /* GU的RTT测试，只转发，不回复 */
#define DIAG_CMD_5G_GTR_SET 0x10015456    /* 5G GTR测试命令 */
#define DIAG_CMD_5G_PS_GTR_SET 0x10015457 /* 5G PS GTR测试命令 */

#define DIAG_CMD_DS_GEN_REQ 0x4f015212       /* BBP数采start/stop命令 */

#define DIAG_CMD_APPLOG_CONFIG 0xEF000001

#define DIAG_DEBUG_DST_MNTN_CMDID 0x9f382000
#define DIAG_DEBUG_AP_SRC_MNTN_CMDID 0x9f382001
#define DIAG_DEBUG_HAC_SRC_MNTN_CMDID 0x96382080

void bsp_diag_dpa_set_translog_onoff(unsigned int transonoff);
void bsp_diag_dpa_set_onoff(unsigned int onoff);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
