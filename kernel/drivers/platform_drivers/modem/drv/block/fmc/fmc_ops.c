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
#include <linux/vmalloc.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <securec.h>
#include <bsp_print.h>
#include <bsp_blk.h>
#include "fmc_blk.h"
#include "../blk_base.h"

#define THIS_MODU mod_blk

struct rw_ops_para {
    u32 rwsize;
    u32 start_offset;
    u32 data_len;
    u32 remainder;
    u8 *data_buf;
    u8 *tmp_buf;
};

enum {
    FMC_READ = 0,
    FMC_WRITE,
};

void fmc_rw_ops_para_set(struct rw_ops_para *fmc_rw_ops_para, u32 rwsize, u32 start_offset, u32 remainder,
                         u8 *data_buf)
{
    fmc_rw_ops_para->rwsize = rwsize;
    fmc_rw_ops_para->start_offset = start_offset;
    fmc_rw_ops_para->data_buf = data_buf;
    fmc_rw_ops_para->remainder = remainder;
}

int fmc_write_ops(struct blk_op_handler *bh, struct rw_ops_para *p)
{
    int ret;
    long long addrblockalign;
    unsigned char *buffer = NULL;

    addrblockalign = (long long)(div_u64(p->start_offset, bh->block_size) * bh->block_size);
    if ((addrblockalign != p->start_offset) || (p->data_len < bh->block_size)) {
        ret = fmc_blk_read(bh, addrblockalign, p->tmp_buf, bh->block_size);
        if (ret) {
            return ret;
        }
        ret = memcpy_s(p->tmp_buf + p->remainder, (unsigned long)(p->rwsize), p->data_buf, (unsigned long)(p->rwsize));
        if (ret != EOK) {
            bsp_err("<%s> memcpy_s err :%d\n", __func__, ret);
            return ret;
        }
        buffer = p->tmp_buf;
    } else {
        buffer = p->data_buf;
    }
    ret = fmc_blk_erase(bh, addrblockalign);
    if (ret) {
        return ret;
    }
    ret = fmc_blk_write(bh, addrblockalign, (unsigned char *)buffer, bh->block_size);
    if (ret) {
        return ret;
    }
    return 0;
}

int fmc_read_ops(struct blk_op_handler *bh, struct rw_ops_para *p)
{
    return fmc_blk_read(bh, (long long)p->start_offset, p->data_buf, p->rwsize);
}

int fmc_rw_param_check(struct blk_op_handler *bh, struct rw_ops_para *p_para)
{
    if (p_para->data_buf == NULL) {
        bsp_err("<%s> para error\n", __func__);
        return -EINVAL;
    }
    if (p_para->start_offset >= bh->part_size || p_para->data_len > bh->part_size ||
        (p_para->start_offset + p_para->data_len) > bh->part_size) {
        bsp_err("<%s> para error, offset 0x%x, len 0x%x, part_size, 0x%llx\n", __func__, p_para->start_offset,
            p_para->data_len, bh->part_size);
        return -EINVAL;
    }
    return 0;
}

int fmc_read_write_ops(struct blk_op_handler *bh, struct rw_ops_para *p_para, u32 ops_type)
{
    int ret;
    u32 rwsize, start_blknum;
    u32 remainder = 0;
    u32 start_offset = 0;
    unsigned char *data_buf = NULL;

    start_blknum = div_u64_rem(p_para->start_offset, bh->block_size, &start_offset);
    data_buf = p_para->data_buf;
    while (p_para->data_len > 0) {
        if (start_offset >= bh->part_size) {
            bsp_err("<%s> partition offset 0x%x beyond partition\n", __func__, start_offset);
            return -EINVAL;
        }

        div_u64_rem(start_offset, bh->block_size, &remainder);
        ret = fmc_blk_isbad(bh, (long long)(start_offset - remainder));
        if (ret) {
            bsp_err("<%s> bad block detected, partition offset 0x%x\n", __func__, start_offset - remainder);
            start_offset += bh->block_size;
            continue;
        }

        if (start_blknum != 0) {
            start_offset += bh->block_size;
            start_blknum--;
            continue;
        }

        rwsize = (remainder + p_para->data_len < bh->block_size) ? p_para->data_len : (bh->block_size - remainder);
        if (!rwsize) {
            bsp_err("<%s> rwsize is zero!\n", __func__);
            return -1;
        }

        fmc_rw_ops_para_set(p_para, rwsize, start_offset, remainder, data_buf);

        if (ops_type == FMC_WRITE) {
            ret = fmc_write_ops(bh, p_para);
        } else {
            ret = fmc_read_ops(bh, p_para);
        }
        if (ret == FMC_BAD_BLK) {
            start_offset += bh->block_size;
            continue;
        } else if (ret) {
            bsp_err("<%s> fmc read write failed, ops 0x%x \n", __func__, ops_type);
            return ret;
        }

        start_offset += rwsize;
        p_para->data_len = (p_para->data_len >= rwsize) ? p_para->data_len - rwsize : 0;
        data_buf += rwsize;
    }

    return 0;
}

int fmc_read(const char *part_name, loff_t part_offset, void *data_addr, size_t data_len)
{
    int ret;
    struct blk_op_handler *bh = NULL;
    struct rw_ops_para ops_para = {0};

    bh = fmc_blk_open(part_name);
    if (bh == NULL) {
        bsp_err("<%s> open err\n", __func__);
        return -1;
    }

    ops_para.start_offset = part_offset;
    ops_para.data_len = data_len;
    ops_para.data_buf = (void *)data_addr;
    ops_para.tmp_buf = NULL;
    if (fmc_rw_param_check(bh, &ops_para)) {
        fmc_blk_close(bh);
        return -EINVAL;
    }
    ret = fmc_read_write_ops(bh, &ops_para, FMC_READ);
    fmc_blk_close(bh);
    return ret;
}

int fmc_write(const char *part_name, loff_t part_offset, const void *data_addr, size_t data_len)
{
    int ret;
    struct blk_op_handler *bh = NULL;
    struct rw_ops_para ops_para = {0};

    bh = fmc_blk_open(part_name);
    if (bh == NULL) {
        bsp_err("<%s> open err\n", __func__);
        return -1;
    }

    ops_para.start_offset = part_offset;
    ops_para.data_len = data_len;
    ops_para.data_buf = (void *)data_addr;
    if (fmc_rw_param_check(bh, &ops_para)) {
        fmc_blk_close(bh);
        return -EINVAL;
    }
    ops_para.tmp_buf = (unsigned char *)vmalloc(bh->block_size);
    if (ops_para.tmp_buf == NULL) {
        bsp_err("<%s> vmalloc failed\n", __func__);
        fmc_blk_close(bh);
        return -1;
    }

    ret = fmc_read_write_ops(bh, &ops_para, FMC_WRITE);
    vfree(ops_para.tmp_buf);
    fmc_blk_close(bh);
    return ret;
}

int fmc_erase(const char *part_name)
{
    int ret;
    u64 count = 0;
    struct blk_op_handler *bh = NULL;

    bh = fmc_blk_open(part_name);
    if (bh == NULL) {
        bsp_err("<%s> open err.\n", __func__);
        return -1;
    }
    while (count <= (bh->part_size - bh->block_size)) {
        ret = fmc_blk_isbad(bh, count);
        if (ret) {
            bsp_err("<%s> bad block detected, partition offset 0x%x\n", __func__, (u32)count);
            count += bh->block_size;
            ret = 0;
            continue;
        }

        ret = fmc_blk_erase(bh, count);
        if (ret == FMC_BAD_BLK) {
            count += bh->block_size;
            ret = 0;
            continue;
        } else if (ret) {
            bsp_err("<%s> fmc_erase_ops err, partition = %s, offset = 0x%x, ret %d\n", __func__, part_name, (u32)count, ret);
            break;
        }
        count += bh->block_size;
    }

    fmc_blk_close(bh);
    return ret;
}

int fmc_isbad(const char *part_name, loff_t part_offset)
{
    int ret;
    struct blk_op_handler *bh = NULL;

    bh = fmc_blk_open(part_name);
    if (bh == NULL) {
        bsp_err("<%s> open err.\n", __func__);
        return -1;
    }

    ret = fmc_blk_isbad(bh, part_offset);
    if (ret < 0) {
        bsp_err("<%s> fmc_blk_isbad err %d\n", __func__, ret);
        goto exit;
    }
    if (ret) {
        bsp_err("bad block detected, partition %s offset 0x%x\n", part_name, (u32)part_offset);
    }

exit:
    fmc_blk_close(bh);
    return ret;
}

int __init fmc_init(void)
{
    struct block_ops block_ops_fmc = {
        .read = fmc_read,
        .write = fmc_write,
        .erase = fmc_erase,
        .isbad = fmc_isbad,
    };
    if (bsp_blk_is_support() != BLOCK_IS_SUPPORT) {
        bsp_err("<%s> flash is not needed\n", __func__);
        return 0;
    }
    if (!bsp_boot_mode_is_fmc()) {
        return 0;
    }
    if (blk_register_ops(&block_ops_fmc)) {
        bsp_err("<%s> blk_register_ops failed\n", __func__);
        return -1;
    }
    if (fmc_blk_ops_init()) {
        return -1;
    }
    return 0;
}

fs_initcall(fmc_init);
