/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
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

#include "ips_traffic_statistic.h"
#include "ips_util.h"
#include "ips_om_interface.h"
#include "mdrv_timer.h"

#define THIS_MODU mod_nfext

#define IPS_FILTER_ALPHA_TATOL 10
#define IPS_FILTER_ALPHA 1
#define IPS_FILTER_DELTA_ALPHA (IPS_FILTER_ALPHA_TATOL - IPS_FILTER_ALPHA)
#define IPS_TRACE_OVERDUE_TIME 1000
#define IPS_TRACE_SPEED_PERIOD 100
#define TIME_STAMP_CNT_TO_MS(a, b) (((a) * 1000) / (b))
#define IPS_SPEED_BYTE_TO_KB(a) ((a) / 1024)
#define IPS_FILTER_MAX_SPEED_DEFAULT 5000 /* USB2.0 要求IPS LOG速率大于5MBps(5%误差)时关闭 */

typedef struct {
    uint32_t pktLen;
    uint32_t lastTimeStamp;
    uint32_t avgSpeed; /* 速度计算以KBps为单位 */
} IPS_MntnPktState;

STATIC bool g_trafficCtrlOn = false;
STATIC uint32_t g_timeStampFreq = 0;
STATIC uint32_t g_ipsTrafficCtrlMaxSpeed = IPS_FILTER_MAX_SPEED_DEFAULT;
STATIC spinlock_t g_lockIpsMntnPktStateFresh;
STATIC IPS_MntnPktState g_ipsTrafficCtrlEntity = { 0 };

void IPSMNTN_TrafficCtrlInit(void)
{
    g_timeStampFreq = mdrv_get_normal_timestamp_freq();
    spin_lock_init(&g_lockIpsMntnPktStateFresh);
}

/* 接收到OM配置可维可测信息捕获配置请求 */
void IPSMNTN_TrafficCtrlReq(const NfMsgBlock *msg)
{
    const OM_IpsMntnTrafficCtrlReq *rcvMsg = (OM_IpsMntnTrafficCtrlReq *)msg;
    g_trafficCtrlOn = !!rcvMsg->trafficCtrlOn;
    g_ipsTrafficCtrlMaxSpeed = (rcvMsg->speedKBps != 0) ? rcvMsg->speedKBps : IPS_FILTER_MAX_SPEED_DEFAULT;

    IPS_PRINT_WARNING("IPS Traffic Ctrl State: %d, At Max Speed %d, Now avg Speed: %d\n", g_trafficCtrlOn,
        g_ipsTrafficCtrlMaxSpeed, g_ipsTrafficCtrlEntity.avgSpeed);
}

STATIC uint64_t IPSMNTN_GetPktSpeed(uint32_t deltaTime)
{
    uint32_t alphaSpeed;
    uint32_t nowSpeed;

    if (deltaTime == 0) {
        return 0;
    }

    alphaSpeed = (IPS_SPEED_BYTE_TO_KB(g_ipsTrafficCtrlEntity.pktLen) * g_timeStampFreq) / deltaTime;
    nowSpeed = (alphaSpeed * IPS_FILTER_ALPHA) + (g_ipsTrafficCtrlEntity.avgSpeed * IPS_FILTER_DELTA_ALPHA);
    nowSpeed /= IPS_FILTER_ALPHA_TATOL;
    return nowSpeed;
}

STATIC void IPSMNTN_UpdateTrafficCtrlInfo(uint32_t infoLen)
{
    uint32_t deltaTime;
    uint32_t deltaTimeMs;
    uint32_t nowTimeStamp = mdrv_timer_get_normal_timestamp();
    unsigned long flag = 0UL;

    spin_lock_irqsave(&g_lockIpsMntnPktStateFresh, flag);
    deltaTime = nowTimeStamp - g_ipsTrafficCtrlEntity.lastTimeStamp;

    deltaTimeMs = TIME_STAMP_CNT_TO_MS(deltaTime, g_timeStampFreq);
    if (deltaTimeMs < IPS_TRACE_SPEED_PERIOD) {
        g_ipsTrafficCtrlEntity.pktLen += infoLen;
        spin_unlock_irqrestore(&g_lockIpsMntnPktStateFresh, flag);
        return;
    }

    g_ipsTrafficCtrlEntity.avgSpeed =
        (deltaTimeMs >= IPS_TRACE_OVERDUE_TIME) ? 0 : (uint32_t)IPSMNTN_GetPktSpeed(deltaTime);
    g_ipsTrafficCtrlEntity.lastTimeStamp = nowTimeStamp;
    g_ipsTrafficCtrlEntity.pktLen = infoLen;
    spin_unlock_irqrestore(&g_lockIpsMntnPktStateFresh, flag);
}

bool IPSMNTN_PktIsOverFlow(uint32_t infoLen)
{
    if ((g_trafficCtrlOn == false) || (g_timeStampFreq == 0)) {
        return false;
    }

    IPSMNTN_UpdateTrafficCtrlInfo(infoLen);
    return ((g_ipsTrafficCtrlEntity.avgSpeed >= g_ipsTrafficCtrlMaxSpeed) ? true : false);
}
