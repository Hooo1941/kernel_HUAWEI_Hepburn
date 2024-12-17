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

#ifndef __BLK_BASE_H__
#define __BLK_BASE_H__

#include<linux/types.h>


struct block_ops {
    int (*read)(const char *part_name, loff_t part_offset, void *data_addr, size_t data_len);
    int (*write)(const char *part_name, loff_t part_offset, const void *data_addr, size_t data_len);
    int (*erase)(const char *part_name);
    int (*isbad)(const char *part_name, loff_t part_offset);
};

struct blk_op_handler {
    const char* part_name;
    loff_t part_size;
    u32 block_size;
    void* priv;
};

struct block_raw_ops {
    struct blk_op_handler *(*open)(const char *part_name);
    void (*close)(struct blk_op_handler *bh);
    int (*isbad)(struct blk_op_handler *bh, loff_t ofs);
    int (*read)(struct blk_op_handler *bh, loff_t from, void *buf, size_t len);
    int (*write)(struct blk_op_handler *bh, loff_t from, const void *buf, size_t len);
    int (*erase)(struct blk_op_handler *bh, loff_t addr);
};

struct blk_op_handler *blk_open(const char *part_name);
void blk_close(struct blk_op_handler *bh);
int blk_isbad(struct blk_op_handler *bh, loff_t ofs);
/* 读的起始地址和长度不必物理块对齐的 */
int blk_read(struct blk_op_handler *bh, loff_t from, void *buf, size_t len);
/* 注意：
1. 对于nand，写的起始地址必须是物理块对齐的，长度无对齐要求
2. 写接口里包含擦除，先擦再写，如需不对齐访问需提前把此块数据读出来 */
int blk_write(struct blk_op_handler *bh, loff_t from, const void *buf, size_t len);
int blk_erase(struct blk_op_handler *bh, loff_t addr);
int blk_register_raw_ops(struct block_raw_ops* ops);
int blk_register_ops(struct block_ops* ops);
#endif
