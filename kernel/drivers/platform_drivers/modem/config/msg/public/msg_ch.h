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

#ifndef __MSG_CH_H__
#define __MSG_CH_H__
#ifndef __MSG_ID_H__
#ifndef __MSG_ID_PUBLIC_H__
#error "please include msg_id.h or msg_id_public.h instead"
#endif
#endif

#include "bmi/msg_ch_bmi.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* 使用通道不要依赖通道的枚举值，枚举只能用于通道标识，不要用于运算 */
enum msg_chnid {
    MSG_CHN_LOOP0,
    MSG_CHN_LOOP1,
    MSG_CHN_FORBIDDEN0 = 0x2, /* IOT解耦，msg_ch_bmi.h占位 */

    /* BEGIN:这部分通道有特殊约定，要么与Modem外组件通信，要么独占资源，要么DTS配置特殊通道 */
    MSG_CHN_PAM_CBT = 0x4,
    MSG_CHN_PMU_CBT = 0x5,
    MSG_CHN_IO_DIE = 0x6,
    MSG_CHN_RESERVED0 = 0x7,
    MSG_CHN_FORBIDDEN1 = 0x8, /* IOT解耦，msg_ch_bmi.h占位 */
    MSG_CHN_APLAY = 0x9,
    MSG_CHN_EDA = 0xA, /* SensorHub和Modem EDA（External Device Adaptation，外部设备适配）通信 */
    MSG_CHN_SKY = 0xB,
    MSG_CHN_PCVOICE = 0xC,
    MSG_CHN_USB_AUDIO = 0xD,
    MSG_CHN_ADSP_AUDIO = 0xE,
    MSG_CHN_SENSHUB= 0xF,
    /* END:这部分通道有特殊约定，要么与Modem外组件通信，要么独占资源 */

    /* BEGIN:Modem使用的通道,若无特殊要求可以直接添加 */
    MSG_CHN_MDM_RST,
    MSG_CHN_MLOADER,
    MSG_CHN_EFUSE,
    MSG_CHN_VSIM,
    MSG_CHN_ADC_CONF,
    MSG_CHN_ADC_DATA,
    MSG_CHN_WARMUP,
    MSG_CHN_THERMAL,
    MSG_CHN_TSENSOR,
    MSG_CHN_DDR_TMON,
    MSG_CHN_HIDSLOG,
    MSG_CHN_DPA_WAKE_LOCK,
    MSG_CHN_VCOM,
    MSG_CHN_RNIC,
    MSG_CHN_ECDC,
    MSG_CHN_MCOV,
    MSG_CHN_AVS,
    MSG_CHN_PMU_OCP,
    MSG_CHN_TRNG,
    MSG_CHN_HMI,
    MSG_CHN_BTEST,
    MSG_CHN_ONOFF,
    MSG_CHN_CSHELL,
    MSG_CHN_SYSVIEW,
    MSG_CHN_SOUND,
    MSG_CHN_PRINT,
    MSG_CHN_LRRC,
    MSG_CHN_MPROBE,
    MSG_CHN_SCI0,
    MSG_CHN_SCI1,
    MSG_CHN_NVA,
    MSG_CHN_NRRC,
    MSG_CHN_DMSS_AUTO,
    MSG_CHN_BASTET,
    MSG_CHN_ADDR_VERIFY,
    MSG_CHN_HIMAC_DATA,
    MSG_CHN_NLL2,
    MSG_CHN_DIAG_DPA,
    /* END:Modem使用的通道,若无特殊要求可以直接添加 */
    MSG_CHN_CNT_MAX
};

#ifdef __cplusplus
}
#endif
#endif
