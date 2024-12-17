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
 *  @brief   飞龙架构2.0 mloader模块对外头文件
 *  @file    mdrv_bmi_mloader.h
 *  @version v1.0
 *  @date    2023.07.07
 *  @note    该文件增删改需通过接口变更评审
 *  <ul><li>v1.0|2023.07.07|创建文件</li></ul>
 *  @since
 */

#ifndef _MDRV_BMI_MLOADER_H_
#define _MDRV_BMI_MLOADER_H_

typedef enum {
    MLOADER_STAGE_IMG_VERIFY_IN,        /* 镜像校验开始 */
    MLOADER_STAGE_IMG_VERIFY_OUT,       /* 镜像校验完成 */
} mloader_stage_e;

/**
 * @brief mloader回调函数定义
 *
 * @par 描述:
 * mloader回调函数定义
 *
 * @attention
 * <ul><li>1、所有阶段mloader均会回调，注册的回调函数中仅处理需要关注的阶段即可，对于不需要处理的阶段不能作为异常处理</li></ul>
 * <ul><li>2、回调函数中不能有耗时较长操作，避免影响系统启动时间</li></ul>
 *
 * @param[in] stage，当前回调的阶段
 * @param[in] result，当前回调阶段的执行结果，非0表示失败
 *
 */
typedef void (*mloader_notifier_func)(mloader_stage_e stage, int result);


/**
 * @brief mloader回调注册
 *
 * @par 描述:
 * mloader回调注册
 *
 * @attention
 * <ul><li>1、所有阶段mloader均会回调，注册的回调函数中仅处理需要关注的阶段即可，对于不需要处理的阶段不能作为异常处理</li></ul>
 * <ul><li>2、回调函数中不能有耗时较长操作，避免影响系统启动时间</li></ul>
 *
 * @param[in] notifier_func，注册的回调函数
 *
 * @retval !=0，表示函数执行失败。
 * @retval =0，表示函数执行成功。
 *
 * @par 依赖:
 * <ul><li>需要在 mloader 初始化完成后才能注册</li></ul>
 *
 */
int mdrv_mloader_notifier_register(mloader_notifier_func notifier_func);

#endif
