/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
#ifndef EFUSE_DRIVER_H
#define EFUSE_DRIVER_H

#include <osl_types.h>

/* ccore & teeos & lpmcu need modify together */
#define EFUSE_OPT_READ (0)
#define EFUSE_OPT_WRITE (1)
#define EFUSE_OPT_SEC_READ (2)
#define EFUSE_OPT_SEC_WRITE (3)

/* tee need modify together */
enum efuse_op_cmd {
    COMMON_READ = 0,
    COMMON_WRITE,
    GET_SOC_ID,
    GET_SECURE_STATE,
    SET_SECKCE,
    SET_SECURE_STATE,
};

struct efuse_auto_dfs_s {
    void *dfs_base_addr;
    u32 dfs_en_offset;
    u32 dfs_en_bit;
    u32 dfs_en_bit_mask;
};

typedef struct {
    u32 bit_start;
    u32 bits_len;
    u32 *src;
    u32 src_group_len;
    u32 *dst;
    u32 dst_group_len;
} efuse_trans_s;

#define MCI_ATP_MOD_EFUSE_TIMEOUT_MS (10000)
#define MCI_ATP_MOD_EFUSE_MAX_LEN 48
struct mci_atp_mod_efuse
{
    int cmd;
    int arg;
    int word_cnt;
    unsigned char buffer[0];
};

unsigned int efuse_calc_group_len(u32 bit_start, u32 bits_len);
#endif