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

#ifndef _BSP_ACM_H_
#define _BSP_ACM_H_

/**
 * @brief 打开acm设备
 *
 * @par 描述:
 * 打开acm设备
 *
 * @param[in]  dev_id 要打开的设备id
 *
 * @retval 非NULL 返回的handle
 * @retval NULL 错误
 */
void *bsp_acm_open(unsigned int dev_id);

/**
 * @brief 关闭acm设备
 *
 * @par 描述:
 * 关闭acm设备
 *
 * @param[in]  handle  要关闭的设备handle
 *
 * @retval 0  处理成功
 * @retval <0 处理失败
 */
int bsp_acm_close(void *handle);

/**
 * @brief 数据写
 *
 * @par 描述:
 * 关闭acm设备
 *
 * @param[in]  handle 设备handle
 * @param[in]  buf  待写入的数据
 * @param[in]  size 代写入的数据大小
 *
 * @retval >0  处理成功的数据大小
 * @retval <=0 处理失败
 */
int bsp_acm_write(void *handle, void *buf, unsigned int size);

/**
 * @brief 数据读
 *
 * @par 描述:
 * 数据读
 *
 * @param[in]  handle 设备handle
 * @param[out]  buf  待返回的数据
 * @param[in]  size 待读出的数据大小
 *
 * @retval >0  处理成功的数据大小
 * @retval <=0 处理失败
 */
int bsp_acm_read(void *handle, void *buf, unsigned int size);

/**
 * @brief 根据命令码进行设置
 *
 * @par 描述:
 * 根据命令码进行设置
 *
 * @param[in]  handle 设备handle
 * @param[in]  cmd 命令码
 * @param[in]  para 命令参数
 *
 * @retval 0  处理成功
 * @retval <0 处理失败
 */
int bsp_acm_ioctl(void *handle, unsigned int cmd, void *para);

#endif