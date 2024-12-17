/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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

/**
 *  @brief   飞龙架构2.0 beth模块对外头文件
 *  @file    mdrv_bmi_beth.h
 *  @version v1.0
 *  @date    2023.08.24
*  @note    该文件增删改需通过接口变更评审
 *  <ul><li>v1.0|2023.08.24|创建文件</li></ul>
 *  @since
 */

#ifndef _MDRV_BMI_BETH_H_
#define _MDRV_BMI_BETH_H_

#define MAX_NAPI_CHANNELS 9

struct beth_mac_info {
    int rx_pkts_cnt;
    int tx_pkts_cnt;
    int rx_crc_err_pkts;
    int rx_unicast_pkts;
    int rx_multicast_pkts;
    unsigned int rx_fifo_overflow_pkts[MAX_NAPI_CHANNELS];
    int tx_unicast_pkts;
    int tx_multicast_pkts;
    unsigned int tx_fifo_overflow_pkts[MAX_NAPI_CHANNELS];
    unsigned int rx_bw[MAX_NAPI_CHANNELS];
};

/**
 * @brief 获取网卡信息
 *
 * @par 描述: 获取网卡信息
 *
 * @attention
 * <ul><li> NA </li></ul>
 *
 * @param[in]  gmacid, 网卡ID号
 * @param[in] struct beth_mac_info *, 网卡信息结构体指针；
 * @param[out] !0：参数输入错误；
 *
 * @retval NA。
 * @par 依赖:NA
 *
 * @see NA
 */
int mdrv_beth_get_mac_info(struct beth_mac_info *mac_info, unsigned int gmacid);

#endif