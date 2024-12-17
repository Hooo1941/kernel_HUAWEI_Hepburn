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

#include <product_config.h>
#include <linux/init.h>
#include <securec.h>
#include <bsp_print.h>
#include <bsp_blk.h>
#include <bsp_version.h>
#include "blk_base.h"

#define THIS_MODU mod_blk


struct block_ops g_block_ops;
struct block_raw_ops g_block_raw_ops;

block_support_e g_block_is_support = BLOCK_NOT_SUPPORT;

/* *****************************block加载模式判断************************************************** */
block_support_e bsp_blk_is_support(void)
{
#ifndef BSP_CONFIG_PHONE_TYPE
    const bsp_version_info_s *version_info = bsp_get_version_info();

    if (version_info != NULL && version_info->plat_type != PLAT_EMU) {
        return BLOCK_IS_SUPPORT;
    }

    return g_block_is_support;
#endif
    return BLOCK_IS_SUPPORT;
}

__init int bsp_blk_parse_boot_para(char *cmdline)
{
    if (cmdline == NULL) {
        bsp_err("cmdline is null.\n");
        return -EINVAL;
    }
    bsp_err("bsp_blk_load_mode :%s\n", cmdline);

    if (strcmp(cmdline, "flash") == 0) {
        g_block_is_support = BLOCK_IS_SUPPORT;
    }

    return 0;
}

#ifndef BSP_CONFIG_PHONE_TYPE
early_param("bsp_blk_load_mode", bsp_blk_parse_boot_para);
#endif

/* *****************************block按块操作接口************************************************** */
struct blk_op_handler *blk_open(const char *part_name)
{
    if (g_block_raw_ops.open != NULL) {
        return g_block_raw_ops.open(part_name);
    } else {
        bsp_info("%s not support\n", __func__);
        return NULL;
    }
}

void blk_close(struct blk_op_handler *bh)
{
    if (g_block_raw_ops.close != NULL) {
        return g_block_raw_ops.close(bh);
    } else {
        bsp_info("%s not support\n", __func__);
        return;
    }
}

int blk_isbad(struct blk_op_handler *bh, loff_t ofs)
{
    if (g_block_raw_ops.isbad != NULL) {
        return g_block_raw_ops.isbad(bh, ofs);
    } else {
        return 0;
    }
}

int blk_read(struct blk_op_handler *bh, loff_t from, void *buf, size_t len)
{
    if (g_block_raw_ops.read != NULL) {
        return g_block_raw_ops.read(bh, from, buf, len);
    } else {
        bsp_info("%s not support\n", __func__);
        return 0;
    }
}

int blk_write(struct blk_op_handler *bh, loff_t from, const void *buf, size_t len)
{
    if (g_block_raw_ops.write != NULL) {
        return g_block_raw_ops.write(bh, from, buf, len);
    } else {
        bsp_info("%s not support\n", __func__);
        return 0;
    }
}

int blk_erase(struct blk_op_handler *bh, loff_t addr)
{
    if (g_block_raw_ops.erase != NULL) {
        return g_block_raw_ops.erase(bh, addr);
    } else {
        return 0;
    }
}

int blk_register_raw_ops(struct block_raw_ops* ops)
{
    int ret = memcpy_s(&g_block_raw_ops, sizeof(g_block_raw_ops), ops, sizeof(struct block_raw_ops));
    if (ret) {
        bsp_err("%s memcpy_s failed, ret:%d\n", __func__, ret);
        return ret;
    }
    return 0;
}

int blk_register_ops(struct block_ops* ops)
{
    int ret = memcpy_s(&g_block_ops, sizeof(g_block_ops), ops, sizeof(struct block_ops));
    if (ret) {
        bsp_err("%s memcpy_s failed, ret:%d\n", __func__, ret);
        return ret;
    }
    return 0;
}

/*
 * 功能: 读一个分区指定偏移的数据
 * 输入: part_name 分区名字
 * 输入: part_offset 分区偏移
 * 输入: data_len  要读的大小
 * 输出: data_buf  存放读出的数据
 * 返回: 小于0失败，大于等于0成功
 */
int bsp_blk_read(const char *part_name, loff_t part_offset, void *data_addr, size_t data_len)
{
    if (g_block_ops.read != NULL) {
        return g_block_ops.read(part_name, part_offset, data_addr, data_len);
    } else {
        bsp_info("%s not support\n", __func__);
        return 0;
    }
}

int mdrv_blk_read(const char *part_name, unsigned long long part_offset, void *data_buf, unsigned long long data_len)
{
    return bsp_blk_read(part_name, part_offset, data_buf, data_len);
}
/*
 * 功能: 对一个分区指定偏移写数据
 * 输入: part_name 分区名字
 * 输入: part_offset 分区偏移
 * 输入: data_len  要写的大小
 * 输出: data_buf  存放要写的数据
 * 返回: 小于0失败，大于等于0成功
 */
int bsp_blk_write(const char *part_name, loff_t part_offset, const void *data_addr, size_t data_len)
{
    if (g_block_ops.write != NULL) {
        return g_block_ops.write(part_name, part_offset, data_addr, data_len);
    } else {
        bsp_info("%s not support\n", __func__);
        return 0;
    }
}

int mdrv_blk_write(const char *part_name, unsigned long long part_offset, void *data_buf, unsigned long long data_len)
{
    return bsp_blk_write(part_name, part_offset, data_buf, data_len);
}

/*
 * 函数  : bsp_blk_erase
 * 功能  : 擦除指定分区的数据
 * 输入  : 分区名
 * 输出  : 无
 * 返回  : 0成功，非0失败
 */
int bsp_blk_erase(const char *part_name)
{
    if (g_block_ops.erase != NULL) {
        return g_block_ops.erase(part_name);
    } else {
        bsp_info("%s not support\n", __func__);
        return 0;
    }
}

int mdrv_blk_erase(const char *part_name)
{
    return bsp_blk_erase(part_name);
}

int bsp_blk_isbad(const char *part_name, loff_t part_offset)
{
    if (g_block_ops.isbad != NULL) {
        return g_block_ops.isbad(part_name, part_offset);
    } else {
        bsp_info("%s not support\n", __func__);
        return 0;
    }
}

int mdrv_blk_isbad(const char *part_name, loff_t part_offset)
{
    return bsp_blk_isbad(part_name, part_offset);
}

EXPORT_SYMBOL_GPL(bsp_blk_read);
EXPORT_SYMBOL_GPL(bsp_blk_write);
EXPORT_SYMBOL_GPL(bsp_blk_erase);
EXPORT_SYMBOL_GPL(bsp_blk_isbad);

EXPORT_SYMBOL_GPL(mdrv_blk_read);
EXPORT_SYMBOL_GPL(mdrv_blk_write);
EXPORT_SYMBOL_GPL(mdrv_blk_erase);
EXPORT_SYMBOL_GPL(mdrv_blk_isbad);
