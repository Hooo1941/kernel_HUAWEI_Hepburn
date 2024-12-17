/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#ifndef __BSP_BLK_H__
#define __BSP_BLK_H__

/*
 * Attention
 * Description : Driver for blk
 * Core          : Acore、Mcore、Fastboot
 * Header File : the following head files need to be modified at the same time
 *                 : /acore/kernel/platform_drivers/include/bsp_blk.h
 *                 : /mcore/xloader/include/bsp_blk.h
 *                 : /fastboot/include/bsp_blk.h
 */
/* 
 * fmc自适应约束，注意事项:
 * 1.block0不支持擦除后直接读取，请写入后再读取;
 * 2.block0只支持整块写入，不支持部分写入。
 */
#include<linux/types.h>
#include <linux/io.h>
#include <product_config.h>

#ifndef BSP_CONFIG_PHONE_TYPE
#include <bsp_onoff.h>
#endif

#ifdef __cplusplus /* __cplusplus */
extern "C"
{
#endif /* __cplusplus */

typedef enum {
    BLOCK_NOT_SUPPORT = 0,
    BLOCK_IS_SUPPORT = 1,
    BLOCK_BUTT
} block_support_e;

int bsp_blk_read(const char *part_name, loff_t part_offset, void *data_buf, size_t data_len);
int bsp_blk_write(const char *part_name, loff_t part_offset, const void *data_buf, size_t data_len);
int bsp_blk_erase(const char *part_name);
int bsp_blk_isbad(const char *part_name, loff_t part_offset);

/**
 * @brief 逻辑分区读
 * @attention
 * <ul><li>待读数据长度不能超过目的buffer大小</li></ul>
 *
 * @param[in] part_name, 分区名
 * @param[in] part_offset, 此次读操作在逻辑分区中的起始偏移
 * @param[in] data_buf, 待读数据目的地址
 * @param[in] data_len, 待读数据长度
 *
 * @retval 0，读成功。
 * @retval 非0，读失败，默认返回-1，特殊错误详见logic block层错误码。
 */
int bsp_blk_logic_read(const char *part_name, loff_t part_offset, void *data_buf, size_t data_len);

/**
 * @brief 逻辑分区数据写到flash
 * @attention
 * <ul><li>此接口返回时会将数据写到flash中保存，但在执行此接口过程中异常掉电，此次写入的数据不能保证刷到flash中</li></ul>
 *
 * @param[in] part_name, 分区名
 * @param[in] part_offset, 此次写操作在逻辑分区中的起始偏移
 * @param[in] data_buf, 待写数据源地址
 * @param[in] data_len, 待写数据长度
 *
 * @retval 0，写成功。
 * @retval 非0，写失败，默认返回-1，特殊错误详见logic block层错误码。
 */
int bsp_blk_logic_write(const char *part_name, loff_t part_offset, const void *data_buf, size_t data_len);

block_support_e bsp_blk_is_support(void);

#ifndef BSP_CONFIG_PHONE_TYPE
static inline int bsp_boot_mode_is_fmc(void)
{
    sysboot_boot_mode_e boot_mode = bsp_boot_mode_get();
    if (boot_mode == SYSBOOT_BOOT_MODE_FMC || boot_mode == SYSBOOT_BOOT_MODE_FMC_PAD || boot_mode == SYSBOOT_BOOT_MODE_FMC_DOUBLEBOOT) {
        return 1;
    } else {
        return 0;
    }
}

static inline int bsp_boot_mode_is_emmc(void)
{
    sysboot_boot_mode_e boot_mode = bsp_boot_mode_get();
    if (boot_mode == SYSBOOT_BOOT_MODE_EMMC0) {
        return 1;
    } else {
        return 0;
    }
}

#endif

#ifdef __cplusplus /* __cplusplus */
}
#endif /* __cplusplus */

#endif
