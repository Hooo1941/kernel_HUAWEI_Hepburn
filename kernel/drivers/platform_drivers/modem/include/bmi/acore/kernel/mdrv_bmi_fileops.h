/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
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
 *  @brief   飞龙架构2.0 file_ops对外头文件
 *  @file    mdrv_bmi_fileops.h
 *  @version v1.0
 *  @date    2023.08.07
 *  @note    该文件增删改需通过接口变更评审
 *  <ul><li>v1.0|2023.08.07|创建文件</li></ul>
 *  @since
 */
#ifndef __MDRV_BMI_FILEOPS_H__
#define __MDRV_BMI_FILEOPS_H__
#include <linux/types.h>
/**
 * @brief 创建软链接
 *
 * @par 描述:
 * 创建软链接
 *
 * @param[in]  oldname 源文件路径
 * @param[in]  newname 软链接文件路径
 *
 * @retval =0，表示创建成功。
 * @retval !=0，表示创建失败。
 */
int mdrv_symlink(const char *old_name, const char *new_name);

/**
 * @brief 创建目录
 *
 * @par 描述:
 * 创建目录，并指定权限
 *
 * @param[in]  pathname 目录名称
 * @param[in]  mode 目录的权限
 *
 * @retval =0，表示创建成功。
 * @retval !=0，表示创建失败。
 */
int mdrv_mkdir(const char *path_name, umode_t mode);

/**
 * @brief 删除一个文件
 *
 * @par 描述:
 * 删除文件
 *
 * @param[in]  pathname 文件名称
 *
 * @retval =0，表示删除成功。
 * @retval !=0，表示删除失败。
 */
int mdrv_unlink(const char *path_name);

#endif

