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
 * @brief   飞龙架构2.0 msg模块对外头文件
 * @file    mdrv_bmi_msg.h
 * @version v1.0
 * @date    2023.08.10
 * @note    该文件增删改需通过接口变更评审
 * <ul><li>v1.0|2023.08.10|创建文件</li></ul>
 * @since
 */

#ifndef __MDRV_MSG_BMI_H__
#define __MDRV_MSG_BMI_H__
#ifdef __cplusplus
extern "C" {
#endif

/** @brief 跨核发送通道特殊状态返回值：通道满重发 */
#define MSG_ERR_EAGAIN (-11) /**< Resource temporarily unavailable */
/** @brief 跨核发送通道特殊状态返回值：目的端处于复位态度 */
#define MSG_ERR_EHOSTRST (-132) /**< Host is resetting */

/**
 * @brief msg通道打开后返回句柄类型
 * @attention
 * <ul><li>用户不需要使用该结构类型, 只需要使用该结构类型的指针</li></ul>
 */
struct msg_chn_hdl;

/**
 * @brief 描述通道位于哪个核
 * @attention
 * <ul><li>次枚举只能用于相等判断，用户不应该依赖此枚举的排列顺序，枚举号大小</li></ul>
 */
enum {
    MSG_CORE_STUB = 0, /* 编号为0的空着不用 */
    MSG_CORE_APP = 1,
    MSG_CORE_TSP = 2,
    MSG_CORE_LPM = 4,
    MSG_CORE_HIFI = 5,
    MSG_CORE_TEE = 6,
    MSG_CORE_SENSOR = 7,
    MSG_CORE_DPA = 9,
    MSG_CORE_CNTMAX,
};

/** @brief 用于msgchn_attr的coremask生成宏 */
#define MSG_CORE_MASK(x) (1U << (x))

/** @brief 描述msg通道的地址结构 */
struct msg_addr {
    unsigned core;
    unsigned chnid;
};

/**
 * @brief 用户打开通道时的输入参数
 * @attention
 * <ul><li>该结构体在用于open前,需要使用mdrv_msgchn_attr_init初始化</li></ul>
 * <ul><li>该结构仅用于打开通道，用户不应该存储，也不应该依赖本结构体的成员排序，字段大小，结构大小等</li></ul>
 */
struct msgchn_attr {
    unsigned magic;     /**< 由mdrv_msgchn_attr_init填写，用户不要访问 */
    unsigned chnid;     /**< 通道的id定义, 在mdrv_msgchn_attr_init后用户填写 */
    unsigned coremask;  /**< 允许从哪几个核收消息 */
    int (*lite_notify)(const struct msg_addr *src, void *msg, unsigned len);
};

/**
 * @brief 初始化属性的接口
 *
 * @par 描述:
 * 用于描述打开通道的各种参数，该函数用于实现将pattr设置为默认值
 *
 * @param[in]  pattr, 通道属性描述
 */
void mdrv_msgchn_attr_init(struct msgchn_attr *pattr);

/**
 * @brief 通道打开接口
 *
 * @par 描述:
 * 打开通道的接口，存在约束，需mdrv_msgchn_attr_init初始化pattr
 *
 * @param[in]  pattr, 通道属性描述
 *
 * @retval 非NULL, 执行成功
 * @retval NULL, 执行失败
 */
struct msg_chn_hdl *mdrv_msgchn_lite_open(struct msgchn_attr *pattr);

/**
 * @brief 数据发送接口
 *
 * @par 描述:
 * 将消息数据发送到目的端
 *
 * @param[in]  chn_hdl, 通道成功打开后返回的句柄
 * @param[in]  dst, 该消息发往的目的地描述
 * @param[in]  msg, 需要发送的数据buff指针
 * @param[in]  len, 需要发送的数据buff的长度
 *
 * @retval 0, 执行成功
 * @retval MSG_ERR_EAGAIN，MSG_ERR_EHOSTRST，特殊场景按需进行处理
 * @retval 其他, 执行失败
 */
int mdrv_msgchn_lite_sendto(struct msg_chn_hdl *chn_hdl, const struct msg_addr *dst, void *msg, unsigned len);

#ifdef __cplusplus
}
#endif
#endif
