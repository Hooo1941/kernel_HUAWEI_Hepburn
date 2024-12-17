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
 * @brief   simhotplug对外公共头文件
 * @file    mdrv_bmi_simhotplug.h
 * @version v1.0
 * @date    2023.07.15
 * @note    修改记录(版本号|修订日期|说明)
 * <ul><li>v1.0|2023.07.15|增加注册接口</li></ul>
 * @since   DRV1.0
 */
#ifndef __MDRV_BMI_SIMHOTPLUG_H__
#define __MDRV_BMI_SIMHOTPLUG_H__

/**
 * @brief 卡插拔状态枚举值
 *
 * @par 描述:
 *   卡插拔状态枚举值
 *
 * @attention
 *  <ul><li>   </li></ul>
 * @param[in] NA
 *
 * @retval NA
 *
 * @par 依赖:
 * <ul><li> NA </li></ul>
 *
 * @see attention
 */

typedef enum {
    SIM_CARD_STATUS_OUT         = 0, /**< 卡拔出最终状态 */
    SIM_CARD_STATUS_IN_POSITION = 1, /**< 卡插入最终状态 */
    SIM_CARD_STATUS_IN          = 2, /**< 卡插入流程处理中间状态 */
    SIM_CARD_STATUS_LEAVE       = 3, /**< 卡拔出流程处理中间状态 */
    SIM_CARD_STATUS_BUTT
} sci_detect_status_e;

/**
* @brief SIM热插拔结果通知函数注册函数原型
*
* @par 描述:
* 当产生热插拔中断，在识别完卡是插入还是拔出，回调注册的函数
*
* @attention
* <ul><li>入参值为当前卡状态, 接口中请勿增加阻塞功能 </li></ul>
*
* @param[in] sci_detect_status_e 参考枚举类型定义，
*
* @retval NA

* @par 依赖:
* <ul><li>无。</li></ul>
*
* @see sci_detect_status_e
*/
typedef void (*sci_card_status_notify)(sci_detect_status_e det_status);

/**
* @brief SIM热插拔结果通知函数注册接口
*
* @par 描述:
* 注册后，才能执行通知函数
*
* @attention
* <ul><li>null</li></ul>
*
* @param[in] sim id
* @param[in] 需要执行的通知回调函数
*
* @retval 0，表示本函数执行成功。
* @retval != 0，表示本函数执行失败。
*
* @par 依赖:
* <ul><li>无。</li></ul>
*
* @see null
*/
int mdrv_sci_card_status_callback(unsigned int sim_id, sci_card_status_notify func);

#define SCI_MSG_NOT_TO_CP  1    /**< 插拔卡消息默认不通知cp的flag */

/**
* @brief 插拔状态是否通知modem
*
* @par 描述:
* 当产生热插拔中断，在识别完卡是插入还是拔出，控制是否通知modem走上下电流程
*
* @attention
* <ul><li> 无</li></ul>
*
* @param[in] sim id
* @param[in] notify_flag 当为0，则通知modem；当 != 0，不通知modem
*
* @retval 0，表示函数执行成功。
* @retval != 0，表示函数执行失败。
*
* @par 依赖:
* <ul><li>无。</li></ul>
*
* @see
*/
int mdrv_sci_hotplug_notify_modem(unsigned int sim_id, unsigned int notify_flag);

#endif

