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

#ifndef __MDRV_BMI_UPDATE_H__
#define __MDRV_BMI_UPDATE_H__

/*****************************************************************************
     *                           Attention                           *
 * Core        : Xloader Hiboot Acore
 * Header File : the following head files need to be modified at the same time
 *             : /hiboot/mdrv_bmi_update.h
 *             : /mcore/xloader/drivers/sysboot/oeminfo/oeminfo_comm_mgr.h
 ******************************************************************************/


/**
  * @brief oeminfo comm区域UPDATE_COMPLETE_FLAG结构magic, 在struct oeminfo_update_complete_flag结构中使用
  */
#define OEMINFO_UPDATE_COMPLETE_FLAG_MAGIC 0x50434D4E

/**
  * @brief oeminfo comm区域BOOT_PART_FLAG结构magic, 在struct oeminfo_boot_part_flag结构中使用
  */
#define OEMINFO_BOOT_PART_FLAG_MAGIC 0x44524543

/**
  * @brief 从主分区启动标记, 在struct oeminfo_boot_part_flag结构中使用
  */
#define OEMINFO_BOOT_PART_A_FLAG 0x5245434A

/**
  * @brief 从备份分区启动标记, 在struct oeminfo_boot_part_flag结构中使用
  */
#define OEMINFO_BOOT_PART_B_FLAG 0x5245434B

/**
 * @brief oeminfo comm区域UPDATE_COMPLETE_FLAG枚举对应结构
 *
 * @par 依赖:
 * <ul><li>mdrv_oeminfo.h：UPDATE_COMPLETE_FLAG枚举声明所在的头文件。</li></ul>
 *
 */
struct oeminfo_update_complete_flag {
    unsigned int magic; /**< 用于校验oeminfo_update_complete_flag结构，取值参考 OEMINFO_UPDATE_COMPLETE_FLAG_MAGIC */
    unsigned int complete_flag; /**< 0: 升级模式, 1: 正常模式, 非零即为正常模式 */
    unsigned int update_times; /**< 记录升级次数 */
    unsigned int auto_update_flag; /**< 自动升级标记 */
};

/**
 * @brief oeminfo comm区域BOOT_PART_FLAG枚举对应结构
 *
* @par 依赖:
 * <ul><li>mdrv_oeminfo.h：BOOT_PART_FLAG枚举声明所在的头文件。</li></ul>
 *
 */
struct oeminfo_boot_part_flag {
    unsigned int magic; /**< 用于校验oeminfo_boot_part_flag结构，取值参考 OEMINFO_BOOT_PART_FLAG_MAGIC */
    unsigned int boot_part; /**< 启动分区flag，A(主)分区(取值参考 OEMINFO_BOOT_PART_A_FLAG)，B(备份)分区(取值参考 OEMINFO_BOOT_PART_B_FLAG)，判断方式非B即A */
};

/*****************************************************************************
     *                        Attention   END                        *
 *****************************************************************************/

#endif
