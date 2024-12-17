/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
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

#ifndef __MSG_ID_DRV_H__
#define __MSG_ID_DRV_H__

#include "msg_id_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MDRV_MSG_DRV_MIDS \
        DRV_MID_START = MSG_DRV_MID_START,            /* 4096 + 0 = 4096 */  \
        DEFINE_MSG_MID(DRV_MID_MSG)                   /* 4096 + 1 = 4097 */  \
        DEFINE_MSG_MID(DRV_MID_TEST1)      \
        DEFINE_MSG_MID(DRV_MID_TEST2)      \
        DEFINE_MSG_MID(DRV_MID_TEST3)      \
        DEFINE_MSG_MID(DRV_MID_TEST4)      \
        DEFINE_MSG_MID(DRV_MID_TEST5)      \
        DEFINE_MSG_MID(DRV_MID_TEST6)      \
        DEFINE_MSG_MID(DRV_MID_TEST7)      \
        DEFINE_MSG_MID(DRV_MID_TEST8)      \
        DEFINE_MSG_MID(DRV_MID_DIAG_APSS)  \
        DEFINE_MSG_MID(MSP_PID_DIAG_AGENT = DRV_MID_DIAG_APSS) /* compatible ps msg check */\
        DEFINE_MSG_MID(DRV_MID_DIAG_TSP)   \
        DEFINE_MSG_MID(MSP_PID_DIAG_APP_AGENT = DRV_MID_DIAG_TSP) /* compatible ps msg check */\
        DEFINE_MSG_MID(DRV_MID_BBPDS)      \
        DEFINE_MSG_MID(DRV_MID_NVSRV_PROC)  \
        DEFINE_MSG_MID(DRV_MID_MEMDEBUG_SWITCH) /* compatible hids tool */ \
        DEFINE_MSG_MID(DRV_MID_MEMINFO_TSP) /* compatible hids tool */ \
        DEFINE_MSG_MID(DRV_MID_MEMINFO_TOTAL_TSP) /* compatible hids tool */ \
        DEFINE_MSG_MID(DRV_MID_DRV_PM) /* compatible diag tool, diag used only */ \
        DEFINE_MSG_MID(DRV_MID_CHR_APSS)     \
        DEFINE_MSG_MID(MSP_PID_CHR_MDM = DRV_MID_CHR_APSS) /* compatible ps msg check */\
        DEFINE_MSG_MID(DRV_MID_CHR_TSP)      \
        DEFINE_MSG_MID(MSP_PID_CHR = DRV_MID_CHR_TSP) /* compatible ps msg check */\
        DEFINE_MSG_MID(DRV_MID_CPUDFS)      \
        DEFINE_MSG_MID(DRV_MID_BBPDS_APSS) \
        DEFINE_MSG_MID(DRV_MID_THERMAL_TSP) \
        DEFINE_MSG_MID(DRV_MID_GNSS_APSS)   \
        DEFINE_MSG_MID(DRV_MID_GNSS_TSP)   \
        DEFINE_MSG_MID(DRV_MID_AVS_TSP) \
        DEFINE_MSG_MID(DRV_MID_SYSVIEW_TSP) \
        DEFINE_MSG_MID(DRV_MID_PM_CCORE) \
        DEFINE_MSG_MID(DRV_MID_MCI_CCORE) \
        DEFINE_MSG_MID(DRV_MID_NETOM_RPT_SWITCH) \
        DRV_MID_END,                                 /* drv max mid num is 4096+4096-1 = 8191 */

#ifdef __cplusplus
}
#endif
#endif
