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

#ifndef ADRV_PMU_H
#define ADRV_PMU_H

/**
 * @brief PMU过流异常处理回调函数
 */
typedef int (*pmic_ocp_callback)(char *);

/**
 * @brief PMU过流异常处理回调注册
 *
 * @par 描述:
 * PMU过流异常处理回调注册
 *
 * @param[in] power_name OCP产生的电源名称
 * @param[in] handler  异常处理回调
 *
 * @retval 0  处理成功
 * @retval <0 处理失败
 */
int pmic_special_ocp_register(char *power_name, pmic_ocp_callback handler);

/**
 * @brief read pmic register
 *
 * @par 描述:
 * read pmic register
 *
 * @param[in] addr pmic register addr
 *
 * @retval value of addr
 */
unsigned int pmic_read_reg(int addr);

/**
 * @brief pmic reg write
 *
 * @par 描述:
 * pmic reg write
 *
 * @param[in] addr pmic register addr
 * @param[in] val  value to write
 *
 * @retval none
 */
void pmic_write_reg(int addr, int val);

/**
 * @brief read sub pmic register
 *
 * @par 描述:
 * read sub pmic register
 *
 * @param[in] addr pmic register addr
 *
 * @retval value of addr
 */
unsigned int sub_pmic_reg_read(int addr);

/**
 * @brief sub pmic reg write
 *
 * @par 描述:
 * sub pmic reg write
 *
 * @param[in] addr pmic register addr
 * @param[in] val  value to write
 *
 * @retval none
 */
void sub_pmic_reg_write(int addr, int val);

#endif /* ADRV_H */
