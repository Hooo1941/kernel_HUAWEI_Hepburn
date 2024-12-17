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
#ifndef _BSP_SYSBOOT_H
#define _BSP_SYSBOOT_H

/*
 增加modem_para_id参数ID，需要同步修改以下文件
 ccore/include/fusion/bsp_sysboot.h
 teeos/include/bsp_modem_secboot.h
*/
enum modem_para_id {
    MODEM_IMAGE_RANDOM_OFFSET,
    MODEM_STACK_GUARD_OFFSET,
    MODEM_HEAP_RANDOM_OFFSET,
    MODEM_MEM_LAYOUT_ADDR_OFFSET,
    MODEM_MEM_LAYOUT_SIZE_OFFSET,
    MODEM_LOGMEM_ADDR_OFFSET,
    MODEM_LOGMEM_SIZE_OFFSET,
    MODEM_MINIDLE_ADDR_OFFSET,
    MODEM_MINIDLE_SIZE_OFFSET,
    MODEM_PARA_MAX,
};

/*
 增加oeminfo_private_rgn_id参数ID，需要同步修改以下文件
 hiboot/private/include/fdrv_sysboot.h
*/
enum oeminfo_err_code {
    OEMINFO_OK = 0,
    OEMINFO_PARA_ERR = -1,
    OEMINFO_CRC_ERR = -2,
    OEMINFO_NOT_INITED = -3,
    OEMINFO_DATA_ERR = -4,
    OEMINFO_MALLOC_MEM_ERR = -5,
};

enum oeminfo_private_rgn_id {
    OEMINFO_PRIVATE_TEST = 0,
    OEMINFO_PRIVATE_DLOAD_SMEM_INFO,
    OEMINFO_PRIVATE_DLOAD_SOFT_VERSION,
    OEMINFO_PRIVATE_DLOAD_PACKAGE_PATH,
    OEMINFO_PRIVATE_DLOAD_PRECESS_INFO,
    OEMINFO_PRIVATE_DLOAD_RECORD_INFO,
    OEMINFO_PRIVATE_RGN_MAX,
};

int bsp_sysboot_set_modem_para(enum modem_para_id id, u32 value);
int bsp_sysboot_set_boot_para(unsigned long addr, u32 size);

int bsp_oeminfo_private_rgn_read(enum oeminfo_private_rgn_id idx, void *buf, u32 buf_len);
int bsp_oeminfo_private_rgn_write_async(enum oeminfo_private_rgn_id idx, void *buf, u32 buf_len);
int bsp_oeminfo_private_rgn_write_sync(enum oeminfo_private_rgn_id idx, void *buf, u32 buf_len);

#endif
