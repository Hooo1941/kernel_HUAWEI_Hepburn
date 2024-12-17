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
 *  @brief   飞龙架构2.0 timer模块对外头文件
 *  @file    mdrv_bmi_timer.h
 *  @version v1.0
 *  @date    2023.07.25
 *  @note    该文件增删改需通过接口变更评审
 *  <ul><li>v1.0|2023.07.25|创建文件</li></ul>
 *  @since
 */

#ifndef _MDRV_BMI_TIMER_H_
#define _MDRV_BMI_TIMER_H_

/**
 * @brief timer name最大的长度
 */
#define TIMER_NAME_MAX_LEN 16

/**
 * @brief timer回调
 */
typedef int (*timer_handle_cb)(int);

/**
 * @brief timer属性配置
 */
typedef struct {
    char name[TIMER_NAME_MAX_LEN]; /**< 名称 */
    unsigned int time;             /**< 定时时间，默认以微秒为单位，配置了TIMER_UNIT_SLICE属性时以slice为单位 */
    unsigned int flag;             /**< 指示timer属性，属性由宏TIMER_FLAG_DEFAULT/TIMER_WAKE/TIMER_LOW_FREQ/TIMER_PERIOD配置 */
    timer_handle_cb callback;      /**< 超时回调函数 */
    int para;                      /**< 回调函数入参 */
} timer_attr_s;

/**
 * @brief timer接口返回的句柄
 */
typedef int timer_handle;

/**
 * @brief 创建一个低精度定时任务
 *
 * @par 描述:
 * 根据用户配置的属性分配定时资源，并设置超时回调函数、回调参数、定时时间
 *
 * @param[in]  attr, 定时器属性配置，包含定时器的唤醒类型、周期性、回调函数、回调入参和定时时间
 *
 * @retval >=0：表示函数执行成功。
 * @retval <0：表示函数执行失败，返回对应的错误码。
 *
 * @par 依赖:
 * <ul><li>mdrv_timer.h：该接口声明所在的头文件。</li></ul>
 *
 * @see
 */
timer_handle mdrv_timer_add(timer_attr_s *attr);

/**
 * @brief 释放创建的低精度定时任务
 *
 * @par 描述:
 * 单次定时在超时前调用本函数可立即删除定时任务，周期定时在创建定时任务后的任何时刻都可调用本函数
 *
 * @param[in]  handle, add接口返回的句柄
 *
 * @retval 0,表示函数执行成功。
 * @retval !=0,表示函数执行失败。
 *
 * @par 依赖:
 * <ul><li>mdrv_timer.h：该接口声明所在的头文件。</li></ul>
 *
 * @see
 */
int mdrv_timer_delete(timer_handle handle);

#endif

