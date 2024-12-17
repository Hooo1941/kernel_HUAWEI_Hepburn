/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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
 *  @brief   timer模块在ACORE上的对外头文件
 *  @file    mdrv_timer.h
 *  @version v1.0
 *  @date    2019.11.29
 *  @note    修改记录(版本号|修订日期|说明)
 *  <ul><li>v1.0|2012.11.29|创建文件</li></ul>
 *  <ul><li>v2.0|2019.11.29|接口自动化</li></ul>
 *  @since
 */

#ifndef __MDRV_ACORE_TIMER_H__
#define __MDRV_ACORE_TIMER_H__

#include "mdrv_public.h"
#include "mdrv_bmi_timer.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 用户输入的参数存在错误
 */
#define MDRV_TIMER_ENO_INPUT (MDRV_ERROR - 1)

/**
 * @brief timer创建失败
 */
#define MDRV_TIMER_ENO_CREATE (MDRV_ERROR - 2)

/**
 * @brief timer配置失败
 */
#define MDRV_TIMER_ENO_CONFIG (MDRV_ERROR - 3)

/**
 * @brief 序列号校验错误
 */
#define MDRV_TIMER_ENO_SN (MDRV_ERROR - 4)

/**
 * @brief hrtimer默认配置：非唤醒、高频、单次定时、时间单位为微秒；普通timer默认配置：非唤醒、单次定时、时间单位为微秒
 */
#define TIMER_FLAG_DEFAULT 0x0

/**
 * @brief 申请带有唤醒功能的timer
 */
#define TIMER_WAKE         0x1

/**
 * @brief 申请采用低频时钟定时的timer，非唤醒源可以使用低频和高频两种时钟，唤醒源只能使用低频（睡眠时高频时钟会下电）
 */
#define TIMER_LOW_FREQ     0x2

/**
 * @brief 申请周期定时的timer
 */
#define TIMER_PERIOD       0x4

/**
 * @brief 指示定时时间单位为时钟tick
 */
#define TIMER_UNIT_SLICE   0x8

/**
 * @brief 配置单次/周期定时
 */
typedef enum {
    TIMER_MODE_ONESHOT,
    TIMER_MODE_PERIOD
} timer_mode_e;

/**
 * @brief 申请定时器资源
 *
 * @par 描述:
 * 根据传入的属性分配对应类型的资源，并设置对应的超时回调函数和回调参数、配置定时器工作模式。
 *
 * @attention
 * <ul><li>接口内部无锁保护，不允许在多个核上同时操作同一个timer。</li></ul>
 *
 * @param[in]  attr, 定时器属性配置，包含定时器的唤醒类型、周期性、回调函数、回调入参和定时时间
 *
 * @retval >= 0,表示函数执行成功。
 * @retval < 0,表示函数执行失败,返回对应的错误码。
 *
 * @par 依赖:
 * <ul><li>mdrv_timer.h：该接口声明所在的头文件。</li></ul>
 *
 * @see timer_attr_s
 */
timer_handle mdrv_hrtimer_create(timer_attr_s *attr);

/**
 * @brief 配置定时时间，并启动定时
 *
 * @par 描述:
 * 配置定时时间，并启动定时
 *
 * @attention
 * <ul><li>接口内部无锁保护，不允许在多个核上同时操作同一个timer。</li></ul>
 *
 * @param[in]  handle, 申请定时器资源时返回的句柄。
 * @param[in]  time,定时时间，默认情况下单位为us，如果create定时器时指定时间单位为slice则此处的单位为slice
 *
 * @retval = MDRV_OK,表示函数执行成功。
 * @retval !=0,表示函数执行失败。
 *
 * @par 依赖:
 * <ul><li>mdrv_timer.h：该接口声明所在的头文件。</li></ul>
 *
 * @see timer_handle
 */
int mdrv_hrtimer_start(timer_handle handle, unsigned int time);

/**
 * @brief 暂停定时任务
 *
 * @par 描述:
 * 暂停定时任务
 *
 * @attention
 * <ul><li>接口内部无锁保护，不允许在多个核上同时操作同一个timer。</li></ul>
 *
 * @param[in]  handle, 申请定时器资源时返回的句柄。
 *
 * @retval 0,表示函数执行成功。
 * @retval !=0,表示函数执行失败。
 *
 * @par 依赖:
 * <ul><li>mdrv_timer.h：该接口声明所在的头文件。</li></ul>
 *
 * @see timer_handle
 */
int mdrv_hrtimer_stop(timer_handle handle);

/**
 * @brief 释放定时器资源
 *
 * @par 描述:
 * 释放定时器资源，与mdrv_hrtimer_create接口成对使用
 *
 * @attention
 * <ul><li>接口内部无锁保护，不允许在多个核上同时操作同一个timer。</li></ul>
 *
 * @param[in]  handle, 申请定时器资源时返回的句柄。
 *
 * @retval 0,表示函数执行成功。
 * @retval !=0,表示函数执行失败。
 *
 * @par 依赖:
 * <ul><li>mdrv_timer.h：该接口声明所在的头文件。</li></ul>
 *
 * @see timer_handle
 */
int mdrv_hrtimer_delete(timer_handle handle);

/**
 * @brief 封装给上层注册timer唤醒系统时的OM回调
 *
 * @par 描述:
 * 封装给上层注册timer唤醒系统时的OM回调
 *
 * @param[in]  handle, 申请定时器资源时返回的句柄。
 * @param[in]  routinue, 回调函数。
 * @param[in]  arg, 回调参数。
 *
 * @retval 0,表示函数执行成功。
 * @retval !=0,表示函数执行失败。
 *
 * @par 依赖:
 * <ul><li>mdrv_timer.h：该接口声明所在的头文件。</li></ul>
 *
 * @see timer_handle timer_handle_cb
 */
int mdrv_hrtimer_debug_register(timer_handle handle, timer_handle_cb routinue, int arg);

/**
 * @brief 获取定时器的剩余时间
 *
 * @par 描述:
 * 获取定时器的剩余时间，该时间根据传入的单位进行计算。
 *
 * @param[in]  handle, 申请定时器资源时返回的句柄。
 * @param[out] rest_time, 剩余时间，默认情况下单位为us，如果create定时器时指定时间单位为slice则此处的单位为slice
 *
 * @retval 0,表示函数执行成功。
 * @retval !=0,表示函数执行失败。
 *
 * @par 依赖:
 * <ul><li>mdrv_timer.h：该接口声明所在的头文件。</li></ul>
 *
 * @see timer_handle
 */
int mdrv_hrtimer_get_rest_time(timer_handle handle, unsigned int *rest_time);

/**
 * @brief 获取BBP定时器的值。
 *
 * @par 描述:
 * 获取BBP定时器的值，用于OAM 时戳。
 *
 * @param[out]  p_high32bit_value , 高位值。
 * @param[out]  p_low32bit_value, 低位值。
 *
 * @retval 0,表示函数执行成功。
 * @retval ！=0,表示函数执行失败。
 
 * @par 依赖:
 * <ul><li>mdrv_timer.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_timer_get_accuracy_timestamp(unsigned int *p_high32bit_value, unsigned int *p_low32bit_value);

/**
 * @brief 获取32KHz时间戳
 *
 * @par 描述:
 * 获取32KHz时间戳，时间戳返回值为递增值，一个数值代表30.5us。
 *
 * @retval 32KHz时间戳。
 
 * @par 依赖:
 * <ul><li>mdrv_timer.h：该接口声明所在的头文件。</li></ul>
 * 
 * @see mdrv_get_normal_timestamp_freq
 */
unsigned int mdrv_timer_get_normal_timestamp(void);

/**
 * @brief 获取19.2MHz时间戳
 *
 * @par 描述:
 * 获取19.2MHz高精度时间戳，系统深睡会停止计时，一个数值代表52ns。
 *
 * @retval 高精度时间戳。
 
 * @par 依赖:
 * <ul><li>mdrv_timer.h：该接口声明所在的头文件。</li></ul>
 *
 * @see mdrv_get_hrt_timestamp_freq
 */
unsigned int mdrv_timer_get_hrt_timestamp(void);

/**
 * @brief 获取32KHz时间戳频率
 *
 * @par 描述:
 * 获取32KHz时间戳频率，单位HZ
 *
 * @retval 时间戳频率。
 
 * @par 依赖:
 * <ul><li>mdrv_timer.h：该接口声明所在的头文件。</li></ul>
 *
 */
unsigned int mdrv_get_normal_timestamp_freq(void);

/**
 * @brief 获取19.2MHz时间戳频率
 *
 * @par 描述:
 * 获取19.2MHz时间戳频率，单位HZ
 *
 * @retval 时间戳频率。
 
 * @par 依赖:
 * <ul><li>mdrv_timer.h：该接口声明所在的头文件。</li></ul>
 *
 */
unsigned int mdrv_get_hrt_timestamp_freq(void);

#ifdef __cplusplus
}
#endif

#endif
