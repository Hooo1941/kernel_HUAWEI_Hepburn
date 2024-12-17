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
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS"
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

#ifndef BSP_MCI_H
#define BSP_MCI_H

#include <linux/proc_fs.h>
#include <linux/version.h>
#include <product_config.h>
#include <osl_types.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /*__cplusplus*/

/* mci atp module id */
#define ATP_IOC_MAGIC 'A'
#define ATP_MOD_ID_EFUSE _IOWR(ATP_IOC_MAGIC, 0, int32_t)
#define ATP_MOD_ID_NVA   _IOWR(ATP_IOC_MAGIC, 1, int32_t)


/**
 * @brief 模块处理上层应用回调
 *
 * @par 描述:
 * atp收到上层应用时，会调用该回调
 *
 * @param[in]  buffer, 模块和上层应用交互的数据
 * @param[in]  len, buffer的长度
 *
 * @retval 透传给上层应用
 */
typedef int (*bsp_mci_atp_mod_callback)(unsigned char *buffer, unsigned int len);

#ifdef CONFIG_MCI_ATP
/**
 * @brief 注册模块到 MCI ATP节点
 *
 * @par 描述:
 * 通过mci atp设备节点，和上层应用进行通信
 *
 * @param[in]  id, 模块ID，每个模块唯一，对应ATP_MOD_ID_XXX，需要和上层应用id一致
 * @param[in]  func, 模块注册的回调，用于处理上层应用下发数据
 * @param[in]  timeout_ms, 模块处理所需最大时间，单位为ms
 * @param[in]  data_max_size, 模块和上层应用交互数据最大长度，func回调中len最大值
 *
 * @retval 0,注册成功
 * @retval 其他, 执行失败
 */
int bsp_mci_atp_mod_register(unsigned int id, bsp_mci_atp_mod_callback func, unsigned int timeout_ms,
    unsigned int data_max_size);
#else
static inline int bsp_mci_atp_mod_register(unsigned int id, bsp_mci_atp_mod_callback func, unsigned int timeout_ms,
    unsigned int data_max_size)
{
    UNUSED(id);
    UNUSED(func);
    UNUSED(timeout_ms);
    UNUSED(data_max_size);
    return 0;
}
#endif

int bsp_mci_proc_create(const char *name, umode_t mode, struct proc_ops *ops);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*BSP_MCI_H*/

