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


#ifndef ADRV_EFUSE_H
#define ADRV_EFUSE_H

/**
 * @brief initlize struct cpufreq
 *
 * @par 描述:
 * initlize struct cpufreq, register cpufreq call chain notifier block
 *
 * @param[out]  buf store the dieid read value
 * @param[in]  size defined in bytes, and requires a minimum of 20 byte
 * @param[in]  timeout overrun time, usually 1000 ms, 0 means waiting unless the func finish your request successfully.
 *
 * @retval 0 SUCCESS
 * @retval <0 FAIL
 */
int get_efuse_dieid_value(unsigned char *buf, unsigned int size, unsigned int timeout);

/**
 * @brief initlize struct cpufreq
 *
 * @par 描述:
 * initlize struct cpufreq, register cpufreq call chain notifier block
 *
 * @param[out] buf store the chipid read value
 * @param[in]  size  defined in bytes, and requires a minimum of 8 byte
 * @param[in]  timeout overrun time, usually 1000 ms, 0 means waiting unless the func finish your request successfully.
 *
 * @retval 0 SUCCESS
 * @retval <0 FAIL
 */
int get_efuse_chipid_value(unsigned char *buf, unsigned int size, unsigned int timeout);

/**
 * @brief get efuse kce value
 *
 * @par 描述:
 * get efuse kce value
 *
 * @param[out] buf store the kce efuse read value
 * @param[in]  size  defined in bytes, and requires a minimum of 16 byte
 * @param[in]  timeout overrun time, usually 1000 ms, 0 means waiting unless the func finish your request successfully.
 *
 * @retval 0 SUCCESS
 * @retval <0 FAIL
 */
int get_efuse_kce_value(unsigned char *buf, unsigned int size, unsigned int timeout);

/**
 * @brief set efuse kce value
 *
 * @par 描述:
 * set efuse kce value
 *
 * @param[out] buf store the kce efuse you want to write
 * @param[in]  size  defined in bytes, and requires a minimum of 16 byte
 * @param[in]  timeout overrun time, usually 1000 ms, 0 means waiting unless the func finish your request successfully.
 *
 * @retval 0 SUCCESS
 * @retval <0 FAIL
 */
int set_efuse_kce_value(unsigned char *buf, unsigned int size, unsigned int timeout);

/**
 * @brief get efuse avs value
 *
 * @par 描述:
 * get efuse avs value
 *
 * @param[out] buf store the avs efuse read value
 * @param[in]  size  defined in bytes, and requires a minimum of 1 byte
 * @param[in]  timeout overrun time, usually 1000 ms, 0 means waiting unless the func finish your request successfully.
 *
 * @retval 0 SUCCESS
 * @retval <0 FAIL
 */
int get_efuse_avs_value(unsigned char *buf, unsigned int size, unsigned int timeout);

#endif /* ADRV_H */
