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
/**
 *  @brief   header files of print on acore
 *  @file    mdrv_print.h
 *  @version v1.0
 *  @date    2019.11.29
 *  @note    修改记录(版本号|修订日期|说明)
 *  v1.0|100.00.00|创建文件
 *  @since
 */
#ifndef __MDRV_PRINT_H__
#define __MDRV_PRINT_H__

#include <mdrv_bmi_print.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif

/**
* @brief print module id
*/
typedef enum {
    MDRV_MOD_BASE_START = 0,
    MDRV_MOD_BASE_END = 256,

    MDRV_MOD_OTHER_START = 256,
    MDRV_MOD_COMMOM = MDRV_MOD_OTHER_START,
    MDRV_MOD_OTHER_END = MDRV_BMI_MODID_START,

    MDRV_MOD_EXTEND_START = MDRV_BMI_MODID_START,
    MDRV_MOD_EXTEND_END = MDRV_BMI_MODID_END,
} mdrv_modid_e;

/**
  @brief different levels of print interfaces
 */
#define mdrv_crit(fmt, ...)      (mdrv_print(MDRV_MOD_COMMOM, MDRV_P_CRIT, "[%s]:" fmt, THIS_MODU, ##__VA_ARGS__))
#define mdrv_err(fmt, ...)       (mdrv_print(MDRV_MOD_COMMOM, MDRV_P_ERR, "[%s]:" fmt, THIS_MODU, ##__VA_ARGS__))
#define mdrv_wrn(fmt, ...)       (mdrv_print(MDRV_MOD_COMMOM, MDRV_P_WRN, "[%s]:" fmt, THIS_MODU, ##__VA_ARGS__))
#define mdrv_info(fmt, ...)      (mdrv_print(MDRV_MOD_COMMOM, MDRV_P_INFO, "[%s]:" fmt, THIS_MODU, ##__VA_ARGS__))
#define mdrv_debug(fmt, ...)     (mdrv_print(MDRV_MOD_COMMOM, MDRV_P_DEBUG, "[%s]:" fmt, THIS_MODU, ##__VA_ARGS__))

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
