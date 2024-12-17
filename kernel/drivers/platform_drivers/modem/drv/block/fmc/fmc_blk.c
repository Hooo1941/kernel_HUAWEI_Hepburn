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
#include <linux/mtd/mtd.h>
#include <linux/err.h>
#include <linux/version.h>
#include <linux/slab.h>

#include <bsp_print.h>
#include <bsp_partition.h>
#include "fmc_blk.h"
#include "../blk_base.h"

#define THIS_MODU mod_blk

struct blk_op_handler *fmc_blk_open(const char *part_name)
{
    struct blk_op_handler *bh = NULL;
    struct mtd_info *mtd = NULL;
    if (part_name == NULL) {
        bsp_err("<%s> part_name is null.\n", __func__);
        return NULL;
    }
    bh = kzalloc(sizeof(struct blk_op_handler), GFP_KERNEL);
    if (bh == NULL) {
        bsp_err("<%s> kzalloc failed\n", __func__);
        return NULL;
    }

    mtd = get_mtd_device_nm(part_name);
    if (IS_ERR(mtd)) {
        bsp_err("get_mtd_device_nm(%s) error\n", part_name);
        kfree(bh);
        return NULL;
    }
    bh->priv = mtd;
    if (!mtd->erasesize) {
        bsp_err("mtd->erasesize is zero, error\n");
        kfree(bh);
        put_mtd_device(mtd);
        return NULL;
    }
    bh->block_size = mtd->erasesize;
    bh->part_name = part_name;
    bh->part_size = mtd->size;
    return bh;
}

void fmc_blk_close(struct blk_op_handler *bh)
{
    struct mtd_info *mtd = bh->priv;
    put_mtd_device(mtd);
    kfree(bh);
}

int fmc_blk_isbad(struct blk_op_handler *bh, loff_t ofs)
{
    struct mtd_info *mtd = bh->priv;
    return mtd_block_isbad(mtd, ofs);
}

int fmc_blk_read(struct blk_op_handler *bh, loff_t from, void *buf, size_t len)
{
    struct mtd_info *mtd = bh->priv;
    size_t retlen = 0;
    int ret;

    ret = mtd_read(mtd, from, len, &retlen, buf);
    if ((ret && ret != -EUCLEAN) || (retlen != len)) {
        bsp_err("mtd read error, ret = %d. retlen = %ld, readlen = %ld\n", ret, retlen, len);
        return -1;
    }
    return 0;
}
int fmc_blk_erase(struct blk_op_handler *bh, loff_t addr)
{
    struct mtd_info *mtd = bh->priv;

    int ret;
    struct erase_info instr = { 0 };

    instr.addr = addr;
    instr.len = mtd->erasesize;

    ret = mtd_erase(mtd, &instr);
    if (ret) {
        bsp_err("mtd erase error, going to markbad. ret = %d. addr = 0x%llx\n", ret, addr);
        ret = mtd_block_markbad(mtd, instr.addr);
        if (ret) {
            bsp_err("mtd markbad error, ret = %d. addr = 0x%llx\n", ret, addr);
            return -1;
        } else {
            return FMC_BAD_BLK;
        }
    }
    return 0;
}

int fmc_blk_write(struct blk_op_handler *bh, loff_t from, const void *buf, size_t len)
{
    struct mtd_info *mtd = bh->priv;
    size_t retlen = 0;
    int ret;

    if (from % bh->block_size) {
        bsp_err("write start 0x%llx is not blocksize aligned\n", from);
        return -1;
    }
    ret = mtd_write(mtd, from, len, &retlen, buf);
    if ((ret && ret != -EUCLEAN) || (retlen != len)) {
        bsp_err("mtd write error, going to markbad. ret = %d. retlen = %ld, readlen = %ld\n", ret, retlen, len);
        ret = mtd_block_markbad(mtd, from);
        if (ret) {
            bsp_err("mtd markbad error, ret = %d. addr = 0x%llx\n", ret, from);
            return -1;
        } else {
            return FMC_BAD_BLK;
        }
    }
    return 0;
}

int fmc_blk_ops_init(void)
{
    struct block_raw_ops block_raw_ops_fmc = {
        .open = fmc_blk_open,
        .close = fmc_blk_close,
        .isbad = fmc_blk_isbad,
        .read = fmc_blk_read,
        .write = fmc_blk_write,
        .erase = fmc_blk_erase,
    };

    if (blk_register_raw_ops(&block_raw_ops_fmc)) {
        bsp_err("<%s> blk_register_raw_ops failed\n", __func__);
        return -1;
    }
    return 0;
}