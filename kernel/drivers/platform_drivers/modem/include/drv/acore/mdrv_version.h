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
/**
 * @brief   version模块在acore上的对外头文件
 * @file    mdrv_version.h
 * @version v1.0
 * @date    2019.11.25
 * @note    修改记录(版本号|修订日期|说明)
 * <ul><li>v1.0|2019.11.25|创建文件</li></ul>
 * @since
 */
#ifndef _MDRV_ACORE_VERSION_H_
#define _MDRV_ACORE_VERSION_H_

#include <mdrv_bmi_version.h>

#ifdef _cplusplus
extern "C" {
#endif

/**
 * @brief mdrv_ver_memberctrl 接口操作类型，0表示读取
 * @see mdrv_ver_memberctrl
 */
#define VERIONREADMODE 0

/**
 * @brief mdrv_ver_memberctrl 接口操作类型，1表示写入
 * @see mdrv_ver_memberctrl
 */
#define VERIONWRITEMODE 1

/**
 * @brief 组件写入或读取的版本信息枚举（acore）
 * @todo 未注释枚举成员待清理
 */
typedef enum {
    VER_BOOTLOAD = 0,
    VER_BOOTROM = 1,
    VER_NV = 2,
    VER_VXWORKS = 3, /**< 3表示VXWORKS */
    VER_DSP = 4,
    VER_PRODUCT_ID = 5, /**< 5表示PRODUCT_ID */
    VER_WBBP = 6,
    VER_PS = 7, /**< 7表示PS */
    VER_OAM = 8,
    VER_GBBP = 9,
    VER_SOC = 10,
    VER_HARDWARE = 11, /**< 11表示HARDWARE */
    VER_SOFTWARE = 12, /**< 12表示SOFTWARE */
    VER_MEDIA = 13,
    VER_APP = 14,
    VER_ASIC = 15,
    VER_RF = 16,
    VER_PMU = 17,
    VER_PDM = 18, /**< 18表示PDM */
    VER_PRODUCT_INNER_ID = 19,
    VER_INFO_NUM = 20
} comp_type_e;

#ifdef _cplusplus
}
#endif
#endif
