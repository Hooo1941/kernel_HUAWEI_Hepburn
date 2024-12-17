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
 * @brief   rpmb模块在bmi仓的对外头文件
 * @file    mdrv_bmi_rpmb.h
 * @version v1.0
 * @date    2023.9.20
 * @note    修改记录(版本号|修订日期|说明)
 * <ul><li>v1.0|2023.9.20|创建文件</li></ul>
 */
#ifndef __MDRV_BMI_RPMB_H__
#define __MDRV_BMI_RPMB_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief RPMB中密钥的状态查询接口
 *
 * @par 描述:
 * RPMB中密钥的状态查询
 *
 * @retval 0, RPMB密钥未烧写
 * @retval 1, RPMB密钥已烧写
 * @retval < 0, 接口返回异常
 * @retval 异常返回值含义 -7:RPMB驱动未初始化; -15:查询RPMB密钥状态失败;
 *
 */
int mdrv_rpmb_get_key_status(void);

/**
 * @brief RPMB中密钥的烧写接口
 *
 * @par 描述:
 * RPMB中密钥烧写
 *
 * @retval 0, RPMB密钥烧写成功
 * @retval < 0, RPMB密钥烧写失败
 * @retval 异常返回值含义 -7:RPMB驱动未初始化; -9:安全os中密钥派生失败; -14:密钥已烧写,不允许重复烧写; -15:查询RPMB密钥状态失败;
 *
 * @attention
 * <ul><li>调用该接口的任务需要在内核上下文发起</li></ul>
 * <ul><li>烧写RPMB密钥需要芯片烧写efuse kdr之后，同时生命周期为非安全状态</li></ul>
 */
int mdrv_rpmb_set_key(void);

#ifdef __cplusplus
}
#endif
#endif
