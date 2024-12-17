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
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <securec.h>
#include <product_config.h>
#include <bsp_print.h>
#include <bsp_blk.h>
#ifdef BSP_CONFIG_PHONE_TYPE
#include <adrv_partition.h>
#else
#include <bsp_partition.h>
#include "mmc_blk.h"
#endif
#include "../blk_base.h"


#define THIS_MODU mod_blk
#define BLK_PATH_LEN_MAX 128

#ifndef BSP_CONFIG_PHONE_TYPE
int flash_find_ptn_s(const char *part_name, char *blk_path, unsigned int blk_path_len)
{
    if ((part_name == NULL) || (blk_path == NULL) || (!strlen(part_name))) {
        bsp_err("%s para err.\n", __func__);
        return -1;
    }

    return sprintf_s(blk_path, blk_path_len, "/dev/mmcblock/%s", part_name);
}
#endif

int mmc_read(const char *part_name, loff_t part_offset, void *data_buf, size_t data_len)
{ /*lint --e{838,501}*/
    long ret;
    ssize_t len;
    struct file* fp = NULL;
    char blk_path[BLK_PATH_LEN_MAX] = "";

    /* check param */
    if (part_name == NULL || data_buf == NULL) {
        bsp_err("%s:invalid param.\n", __func__);
        return -1;
    }

    ret = flash_find_ptn_s(part_name, (char *)blk_path, BLK_PATH_LEN_MAX);
    if (ret < 0) {
        bsp_err("%s not found from partition table!\n", part_name);
        return -1;
    }

    fp = filp_open(blk_path, O_RDONLY, 0);
    if (IS_ERR_OR_NULL(fp)) {
        bsp_err("fail to open file %s!\n", blk_path);
        return -1;
    }

    len = kernel_read(fp, data_buf, data_len, &part_offset);
    if (len != data_len) {
        bsp_err("%s read error, data_len %zu read_len %ld!\n", blk_path, data_len, len);
        (void)filp_close(fp, NULL);
        return -1;
    }

    (void)filp_close(fp, NULL);

    return 0;
}

int mmc_write(const char *part_name, loff_t part_offset, const void *data_buf, size_t data_len)
{ /*lint --e{838, 501}*/
    long ret;
    ssize_t len;
    struct file* fp = NULL;
    char blk_path[BLK_PATH_LEN_MAX] = "";

    /* check param */
    if (part_name == NULL || data_buf == NULL) {
        bsp_err("%s:invalid param.\n", __func__);
        return -1;
    }

    ret = flash_find_ptn_s(part_name, (char *)blk_path, BLK_PATH_LEN_MAX);
    if (ret < 0) {
        bsp_err("%s not found from partition table!\n", part_name);
        return -1;
    }
    fp = filp_open(blk_path, O_WRONLY | O_SYNC, 0);
    if (IS_ERR_OR_NULL(fp)) {
        bsp_err("fail to open file %s!\n", blk_path);
        return -1;
    }

    len = kernel_write(fp, data_buf, data_len, &part_offset);
    if (len != data_len) {
        bsp_err("%s read error, data_len %zu read_len %ld!\n", blk_path, data_len, len);
        (void)filp_close(fp, NULL);
        return -1;
    }

    (void)filp_close(fp, NULL);

    return 0;
}

int mmc_isbad(const char *part_name, loff_t part_offset)
{
    return 0;
}

int mmc_init(void)
{
    struct block_ops block_ops_mmc = {
        .read = mmc_read,
        .write = mmc_write,
        .isbad = mmc_isbad,
    };
#ifndef BSP_CONFIG_PHONE_TYPE
    if (bsp_blk_is_support() != BLOCK_IS_SUPPORT) {
        bsp_err("<%s> flash is not needed\n", __func__);
        return 0;
    }
    if (!bsp_boot_mode_is_emmc()) {
        return 0;
    }
    if (mmc_blk_ops_init()) {
        return -1;
    }
#endif
    if (blk_register_ops(&block_ops_mmc)) {
        bsp_err("<%s> blk_register_ops failed\n", __func__);
        return -1;
    }

    return 0;
}


#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
fs_initcall(mmc_init);
#endif


