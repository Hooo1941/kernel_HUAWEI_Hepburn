/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
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


#ifndef ADRV_VERSION_H
#define ADRV_VERSION_H

/**
 * @brief get soc chip level
 *
 * @par 描述:
 * get soc chip level
 *
 *
 * @retval 0 LEVEL_I
 * @retval 1 LEVEL_II
 * @retval -1 Unsupported this features;
 */
int get_soc_chip_level(void);


/**
 * @brief get_checked_product_id
 *
 * @par 描述:
 * 终端phone产品, 获取经过vc校验（射频id信息）的productid, 可用于nv加载
 * udp产品, 获取带扣板信息的的udp productid, 可用于udp的dts/nv加载, 如0x2df5fc15
 *
 * @retval 0xFFFFFFFF, 无效的productid
 * @retval 其他, 正确的productid
 */
unsigned int get_checked_product_id(const unsigned int *prdt_list, unsigned int length);

/**
 * @brief get_product_id
 *
 * @par 描述:
 * 终端phone产品, 获取未经过vc校验（射频id信息）的productid, 可用于dts加载
 * udp产品, 获取带扣板信息的的udp productid, 可用于udp的dts/nv加载, 如0x2df5fc15
 *
 * @retval 0xFFFFFFFF, 无效的productid
 * @retval 其他, 正确的productid
 */
unsigned int get_product_id(void);


#endif /* ADRV_H */
