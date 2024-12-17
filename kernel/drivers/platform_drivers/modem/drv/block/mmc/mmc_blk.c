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
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/slab.h>

#include <bsp_print.h>
#include "../blk_base.h"
#include "mmc_blk.h"

#define THIS_MODU mod_blk

#define BLK_PATH_LEN_MAX 128

struct blk_op_handler *mmc_blk_open(const char *part_name)
{
    struct blk_op_handler *bh = NULL;
    char blk_path[BLK_PATH_LEN_MAX] = "";
    struct file* fp = NULL;
    int ret;

    if (part_name == NULL) {
        bsp_err("<%s> part_name is null.\n", __func__);
        return NULL;
    }
    bh = kzalloc(sizeof(struct blk_op_handler), GFP_KERNEL);
    if (bh == NULL) {
        bsp_err("<%s> kzalloc failed\n", __func__);
        return NULL;
    }

    ret = flash_find_ptn_s(part_name, (char *)blk_path, BLK_PATH_LEN_MAX);
    if (ret < 0) {
        bsp_err("%s not found from partition table!\n", part_name);
        kfree(bh);
        return NULL;
    }
    fp = filp_open(blk_path, O_RDWR | O_SYNC, 0);
    if (IS_ERR_OR_NULL(fp)) {
        bsp_err("fail to open file %s!\n", blk_path);
        kfree(bh);
        return NULL;
    }
    bh->priv = fp;
    bh->block_size = 512U;
    bh->part_name = part_name;
    bh->part_size = vfs_llseek(fp, 0, SEEK_END);
    if (bh->part_size <= 0) {
        bsp_err("get %s size is failed, ret %lld!\n", part_name, bh->part_size);
        (void)filp_close(fp, NULL);
        kfree(bh);
        return NULL;
   }

    return bh;
}

void mmc_blk_close(struct blk_op_handler *bh)
{
    struct file* fp = bh->priv;
    (void)filp_close(fp, NULL);
    kfree(bh);
}

int mmc_blk_read(struct blk_op_handler *bh, loff_t from, void *buf, size_t len)
{
    struct file* fp = bh->priv;
    ssize_t ret;
    ret = kernel_read(fp, buf, len, &from);
    if (ret != len) {
        bsp_err("mmc kernel_read error, data_len %ld read_len %ld!\n", len, ret);
        return -1;
    }

    return 0;
}

int mmc_blk_write(struct blk_op_handler *bh, loff_t from, const void *buf, size_t len)
{
    struct file* fp = bh->priv;
    ssize_t ret;
    ret = kernel_write(fp, buf, len, &from);
    if (ret != len) {
        bsp_err("mmc kernel_write error, data_len %ld read_len %ld!\n", len, ret);
        return -1;
    }

    return 0;
}

int mmc_blk_ops_init(void)
{
    struct block_raw_ops block_raw_ops_mmc = {
        .open = mmc_blk_open,
        .close = mmc_blk_close,
        .read = mmc_blk_read,
        .write = mmc_blk_write,
    };
    if (blk_register_raw_ops(&block_raw_ops_mmc)) {
        bsp_err("<%s> blk_register_raw_ops failed\n", __func__);
        return -1;
    }
    return 0;
}