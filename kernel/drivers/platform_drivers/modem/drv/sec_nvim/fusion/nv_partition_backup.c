/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2018. All rights reserved.
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

#include <securec.h>
#include <bsp_nvim.h>
#include "nv_comm.h"
#include "nv_partition.h"

u32 nv_bak_flush(u32 backup_type)
{
    u32 ret, size;
    struct nvm_info *nvminfo = nv_get_nvminfo();
    u64 dtaddr = nvminfo->dtaddr;
    u64 dtsize = nvminfo->dtsize;
    struct nv_backup_cust_cb *cb = NULL;

    if (dtaddr == 0) {
        nv_printf("nv memory init err\n");
        return BSP_ERR_NV_MEM_INIT_FAIL;
    }

    size = nv_get_data_size(dtaddr);
    if (size > dtsize) {
        nv_printf("flush backup nv data overflow\n");
        return BSP_ERR_NV_OVER_MEM_ERR;
    }

    if (backup_type & NV_BACKUP_BASE_MASK) {
        ret = nv_blk_write(NV_BLK_BACKUP, (u8 *)(uintptr_t)dtaddr, 0, size);
        if (ret) {
            nv_printf("nv write backup err, ret:0x%x\n", ret);
            return ret;
        }
    }

    cb = nv_get_backup_cust_cb();
    if ((backup_type & NV_BACKUP_BAK_MASK) && (cb != NULL) && (cb->blk_write != NULL)) {
        ret = cb->blk_write((u8 *)(uintptr_t)dtaddr, size);
        if (ret) {
            nv_printf("nv_get_backup_cust_cb nv write backup err, ret:0x%x\n", ret);
            return ret;
        }
    }

    nv_debug_printf(NV_INFO, "flush backup success\n");
    return NV_OK;
}
