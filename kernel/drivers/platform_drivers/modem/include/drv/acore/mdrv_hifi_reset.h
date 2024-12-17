/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
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
 *  @brief   提供给HIFI DSP单独复位的对外头文件
 *  @file    mdrv_hifi_reset.h
 *  @version v1.0
 *  @date    2019.11.25
 *  @note    修改记录(版本号|修订日期|说明)
 *  <ul><li>v1.0|2019.11.25|创建文件</li></ul>
 *  @since   DRV1.0
 */

#ifndef __MDRV_HIFI_RESET_H__
#define __MDRV_HIFI_RESET_H__

#ifdef __cplusplus
extern "C"
{
#endif

/**
* @brief 单独复位回调函数格式
*/
typedef unsigned int (*hifi_cb_func)(unsigned int proc_result);

/**
 * @brief HIFI单独复位接口
 *
 * @par 描述:
 * HIFI DSP通过调用此接口触发单独复位
 *
 * @attention
 * <ul><li>调用前请保证已注册单独复位回调函数</li>
 * <li>不支持单独复位时，函数打桩返回-1。</li></ul>
 *
 * @retval 0,表示单独复位触发成功
 * @retval -1,表示未注册回调函数
 *
 * @par 依赖:
 * <ul><li>mdrv_hifi_reset.h：该接口声明所在的头文件。</li></ul>
 *
 */
int mdrv_hifi_reset(void);

/**
 * @brief HIFI单独复位回调函数注册接口
 *
 * @par 描述:
 * HIFI DSP通过此接口注册单独复位回调函数
 *
 * @attention
 * <ul><li>不支持单独复位时，函数打桩返回-1。</li></ul>
 *
 * @retval 0,表示回调函数注册成功
 * @retval -1,表示传入的回调函数为空
 *
 * @par 依赖:
 * <ul><li>mdrv_hifi_reset.h：该接口声明所在的头文件。</li></ul>
 *
 * @see hifi_cb_func
 *
 */
int mdrv_hifi_reset_cb_register(hifi_cb_func pcbfun);


#ifdef __cplusplus
}
#endif
#endif

