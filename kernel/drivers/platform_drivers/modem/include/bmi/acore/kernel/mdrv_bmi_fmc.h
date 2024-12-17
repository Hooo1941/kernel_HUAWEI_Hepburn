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

#ifndef __MDRV_FMC_H__
#define __MDRV_FMC_H__

#include <linux/mtd/mtd.h>

/**
 * @brief 获取nand flash块大小
 *
 * @retval 表示blksize。
 */
unsigned int mdrv_fmc_get_blksize(void);
/**
 * @brief 获取nand flash容量
 *
 * @retval 表示chipsize。
 */
unsigned int mdrv_fmc_get_chipsize(void);
/**
 * @brief 获取nand flash页大小
 *
 * @retval 表示pagesize。
 */
unsigned int mdrv_fmc_get_page_size(void);
/**
 * @brief 获取nand flash ecc纠错能力：x/1024
 *
 * @retval 表示ecc_size。
 */
unsigned int mdrv_fmc_get_ecc_size(void);
/**
 * @brief 获取flash 厂商ID：manufacture id
 *
 * @retval 表示maf_id。
 */
unsigned char mdrv_fmc_get_maf_id(void);
/**
 * @brief 获取flash 设备ID：device id
 *
 * @retval 表示dev_id。
 */
unsigned char mdrv_fmc_get_dev_id(void);
/**
 * @brief 获取整个flash的mtd_info
 *
 * @retval mtd_info
 */
struct mtd_info *mdrv_fmc_get_mtd_info(void);

/**
 * @brief 记录fmc对接nand flash最近一次读数据产生的bit翻转信息，ecc_step为纠错单元，err_num为bit翻转个数
 */
typedef void (*fmc_bitflip_info_record_func)(unsigned int ecc_step, unsigned int err_num);

/**
 * @brief 注册fmc_bitflip_info_record_func
 */
void mdrv_fmc_bitflip_info_record_func_register(fmc_bitflip_info_record_func func);
/**
 * @brief 用于fmc软件双boot启动升级场景，根据偏移获取版本号
 *
 * @retval   0：表示 1）非fmc启动 2）fmc启动但未使能xloader双boot 3）启动参数传递失败
 * @retval 非0：表示fmc软件双boot启动时，版本号在xloader镜像中的偏移
 */
unsigned int mdrv_fmc_get_versionid_offset(void);
#endif
