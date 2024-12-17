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
#include <linux/errno.h>
#include <bsp_mloader.h>
#include "nv_comm.h"
#include "nv_partition.h"
#include "nv_debug.h"

u32 nv_data_writeback(u32 backup_type)
{
    u32 ret;

    ret = nv_img_flush_carrier(NV_BACKUP_BAK_MASK);
    if (ret != NV_OK) {
        nv_printf("nv img carrier write back err\n");
        return ret;
    }

    ret = nv_img_flush_data(NV_BACKUP_BAK_MASK);
    if (ret != NV_OK) {
        nv_printf("nv img data write back err\n");
        return ret;
    }

    ret = nv_bak_flush(backup_type);
    if (ret != NV_OK) {
        nv_printf("nv bak data write back err\n");
        return ret;
    }

    return NV_OK;
}

u32 nv_clear_upgrade_flag(u32 backup_type)
{
    u32 ret;

    ret = nv_img_tag_flush(backup_type);
    if (ret != NV_OK) {
        nv_printf("nv clear upgrade flag err\n");
        return ret;
    }

    nv_printf("nv img tag.bin file write success\n");
    return NV_OK;
}

#ifdef BSP_CONFIG_PHONE_TYPE
#define NV_FILE_UPGRADE_FLAG NV_FILE_UPDATA
#define NV_FILE_EXIST (0x766e)
u32 nv_set_coldpatch_upgrade_flag(bool flag)
{
    u32 ret = NV_ERROR;
    u32 flen;
    int vrl_size;
    nv_dload_head phead;
    nv_dload_tail ptail;
    vrl_size = bsp_mloader_get_vrl_size();
    ret = nv_blk_read(NV_BLK_PATCH, (u8 *)&phead, 0 + vrl_size, (u32)sizeof(nv_dload_head));
    if (ret) {
        nv_printf("coldpatch can't access, ret:0x%x\n", ret);
        return ret;
    }

    if (phead.nv_bin.magicnum != NV_FILE_EXIST) {
        nv_printf("coldpatch file not exist\n");
        return BSP_ERR_NV_NO_FILE;
    }

    flen = sizeof(nv_dload_head);
    flen += ((phead.nv_bin.magicnum == NV_FILE_EXIST)? phead.nv_bin.length:0);
    flen += ((phead.xnv_map.magicnum == NV_FILE_EXIST)? phead.xnv_map.length:0);
    flen += ((phead.xnv.magicnum == NV_FILE_EXIST)? phead.xnv.length:0);
    flen += ((phead.cust_map.magicnum == NV_FILE_EXIST)? phead.cust_map.length:0);
    flen += ((phead.cust.magicnum == NV_FILE_EXIST)? phead.cust_map.length:0);

    ret = nv_blk_read(NV_BLK_PATCH, (u8 *)&ptail, flen + vrl_size, (u32)sizeof(nv_dload_tail));
    if (ret) {
        nv_printf("read coldpatch tail failed, ret:0x%x\n", ret);
        return BSP_ERR_NV_READ_DATA_FAIL;
    }

    if (flag) {
        ptail.upgrade_flag = NV_FILE_UPGRADE_FLAG;
    } else {
        ptail.upgrade_flag = NV_FILE_EXIST;
    }

    ret = nv_blk_write(NV_BLK_PATCH, (u8 *)&ptail, flen + vrl_size, (u32)sizeof(nv_dload_tail));
    if (ret) {
        nv_printf("write coldpatch tail failed, ret:0x%x\n", ret);
        return BSP_ERR_NV_WRITE_DATA_FAIL;
    }
    return NV_OK;
}
#endif

u32 nv_set_upgrade_flag(void)
{
    u32 ret;

    ret = nv_img_rm_tag();
    /* -ENOENT表示文件不存在，终端存在多次删除场景，-ENOENT认为是成功 */
    if ((ret != NV_OK) && (ret != -ENOENT)) {
        nv_printf("nv set upgrade flag err\n");
        return ret;
    }

    return NV_OK;
}

u32 nv_set_ftyrst_flag(void)
{
    u32 ret;

    ret = nv_img_ftyrst_flush();
    if (ret != NV_OK) {
        nv_printf("nv set ftyrst flag err\n");
        return ret;
    }

    return NV_OK;
}


u32 nv_upgrade_flush(struct nv_msg_data *msg_data, u32 *sync_flag)
{
    u32 ret;
    UNUSED(msg_data);
    UNUSED(sync_flag);

    nv_printf("nv upgrade flush start\n");

    ret = nv_data_writeback(NV_BACKUP_BASE_MASK | NV_BACKUP_BAK_MASK);
    if (ret) {
        nv_printf("nv upgrade write back failed, ret:0x%x\n", ret);
        return ret;
    }

    ret = nv_clear_upgrade_flag(NV_BACKUP_BAK_MASK);
    if (ret) {
        nv_printf("nv upgrade clear upgrade flag failed, ret:0x%x\n", ret);
        return ret;
    }

    nv_printf("nv upgrade flush end\n");
    return NV_OK;
}
