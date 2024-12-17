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

#ifndef __NVA_MODEM_H__
#define __NVA_MODEM_H__

#include <product_config.h>
#include <bsp_print.h>
#include <osl_sem.h>
#include <bsp_partition.h>
#include "nva_partition.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define nva_printf(fmt, ...) (bsp_err(fmt, ##__VA_ARGS__))

#define NVA_OK 0
#define NVA_ERROR 0xffffffff

/* 定义NVA错误码 */
#define BSP_ERR_NVA_BASE 0x10ff0000
#define NVA_FILE_READ_ERR (BSP_ERR_NVA_BASE + 0x1)
#define NVA_FILE_NOT_EXIST (BSP_ERR_NVA_BASE + 0x2)

#define NVA_MIN_ID 7000
#define NVA_MAX_ID 7999

#define NVA_UPGRADE 0x1234abcd
#define NVA_RELOAD 0x4321dcba

#define NVA_FILE_EXIST 0x61766e // 判断文件是否存在

#define NVA_NEED_UPGRADE (0x76566e4e)
#define NVA_NEED_RELOAD (0x4e6e5676)

#define NVA_SEC_VRL_SIZE 0x1000
#define NVA_PART_NAME PTABLE_MNVDLOAD
#define NVA_FILE "nva.bin"
#define NVA_MAX_LEN 128

enum {
    NVA_INIT_FAIL = 0x0,
    NVA_INIT_OK = 0x1,
};

typedef struct {
    u32 nva_upgrade_flag;    /* nva upgrade flag: NVA_UPGRADE or NVA_RELOAD */
    u32 fastboot_init_state; /* fastboot init state */
    u32 kernel_init_state;
    u32 mcore_init_state;
} nva_global_info_s;

typedef struct {
    u16 item_id;
    u16 data_len;
    u32 base_addr;
    u16 flag;
    u16 reserve;
} nva_item_info_s;

typedef struct {
    u32 nva_init_state; /* init state */
    osl_sem_id rw_sem;
    osl_sem_id task_sem;
    void *load_addr;
    struct nva_fs nvafs;
} nva_ctrl_info_s;

nva_ctrl_info_s *nva_get_nvainfo(void);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // __NVA_MODEM_H__
