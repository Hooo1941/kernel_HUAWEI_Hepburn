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

#include "device_tree.h"
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/syscalls.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/version.h>
#include <bsp_file_ops.h>
#include <securec.h>
#include <bsp_rfile.h>
#include <bsp_mloader.h>
#include <bsp_version.h>
#include <bsp_sec_call.h>

int bsp_dt_get_sec_call_info(u32 image_type, dt_sec_call_s *dt_info)
{
    if (image_type != MODEM_IMG_FW_DTB && image_type != MODEM_IMG_CCPU_DTB && image_type != MODEM_IMG_CCPU_DTO) {
        dt_pr_err("bsp dt sec load image failed, invalid image type %d  \n", image_type);
        return DTB_ERROR;
    }

    dt_info->image_type = image_type;
    if (image_type == MODEM_IMG_CCPU_DTO) {
        const bsp_version_info_s *version_info = NULL;
        version_info = bsp_get_version_info();
        if (version_info == NULL) {
            dt_pr_err("fail to get verison info\n");
            return DTB_ERROR;
        }

        dt_info->dto_ver[0x0] = version_info->dto_ver[0x0];
        dt_info->dto_ver[0x1] = version_info->dto_ver[0x1];
        dt_info->dto_ver[0x2] = version_info->dto_ver[0x2];
    }

    return DTB_OK;
}

int bsp_dt_get_image_path(char *image_name, char *file_path, u32 buf_len, u32 image_type) {
    int ret;
    const char *path = NULL;
    switch (image_type) {
        case MODEM_IMG_FW_DTB:
            path = bsp_mloader_get_modem_vendor_path();
            ret = sprintf_s(file_path, buf_len, "%s/%s/%s", path, "image", image_name);
            break;
        case MODEM_IMG_CCPU_DTB:
            path = bsp_mloader_get_modem_fw_path();
            ret = sprintf_s(file_path, buf_len, "%s/%s", path, image_name);
            break;
        case MODEM_IMG_CCPU_DTO:
            path = bsp_mloader_get_modem_fw_path();
            ret = sprintf_s(file_path, buf_len, "%s/%s", path, image_name);
            break;
        default:
            ret = DTB_ERROR;
            dt_pr_err("bsp dt sec load image failed, invalid image type %d  \n", image_type);
            break;
    }

    return ret;
}

int bsp_dt_parse_image_by_secos(char *image_name, u32 image_type) {
    int ret = 0;
    dt_sec_call_s dt_info;

    ret = bsp_dt_get_sec_call_info(image_type, &dt_info);
    if (ret) {
        dt_pr_err(" %s make sec_call info failed: %d\n",image_name, ret);
        return DTB_ERROR;
    }

    dt_pr_err("request secos to parse  %s\n", image_name);
    ret = bsp_sec_call_ext(FUNC_DTS_LOAD_DT, 0, (void *)&dt_info, sizeof(dt_info));
    if (ret) {
        dt_pr_err("secos parse %s failed : %d\n",image_name, ret);
        return DTB_ERROR;
    }
    dt_pr_err("request secos to parse  %s success\n", image_name);

    return DTB_OK;
}

int bsp_dt_parse_image_by_kernel(char *image_path, u32 dtb_id, int offset, char *vaddr, u32 vaddr_size)
{
    int readed_bytes;
    struct dtb_entry dt_entry_ptr;

    if (memset_s((void *)&dt_entry_ptr, sizeof(dt_entry_ptr), 0, sizeof(struct dtb_entry))) {
        dt_pr_err("<%s> memset_s error", __FUNCTION__);
        return DTB_ERROR;
    }

    if (bsp_dt_get_dtb_entry(image_path, dtb_id, &dt_entry_ptr, offset)) {
        dt_pr_err("bsp_dt_load_and_find_dtb_entry failed. \n");
        return DTB_ERROR;
    }

    if (dt_entry_ptr.dtb_size > vaddr_size) {
        dt_pr_err("modem dtb dtb_size too large %d than ddr_size %d\n", dt_entry_ptr.dtb_size, vaddr_size);
        return DTB_ERROR;
    }

    readed_bytes = bsp_mloader_read_file(image_path, offset + dt_entry_ptr.dtb_offset,
        dt_entry_ptr.dtb_size, (char *)vaddr);

    if (readed_bytes != dt_entry_ptr.dtb_size) {
        dt_pr_err("read_file %s err: readed_bytes %d\n", image_path, readed_bytes);
        return DTB_ERROR;
    }

    dt_pr_err("load image %s success\n", image_path);
    return DTB_OK;
}


void *bsp_dt_alloc_buffer(unsigned int bufsz)
{
    void *vaddr = NULL;

    if (bufsz > MALLOC_LEN_LIMIT) {
        dt_pr_err("malloc too large size %u\n", bufsz);
        return NULL;
    }

    vaddr = kzalloc((size_t)bufsz, GFP_KERNEL);
    if (vaddr == NULL) {
        dt_pr_err("malloc buffer failed, size %u\n", bufsz);
        return NULL;
    }

    return vaddr;
}

void bsp_dt_free_buffer(void *buf)
{
    if (buf != NULL) {
        kfree(buf);
    }
}

int bsp_dt_get_dtb_entry(const char *file_name, u32 dtb_id, struct dtb_entry *entry_out, u32 offset)
{
    int ret = BSP_ERROR;
    int rd_bytes;
    struct dtb_table dtb_table = { 0, 0, 0 };
    struct dtb_entry *entries = NULL;
    u32 len;

    /* get the dtb header and entries */
    rd_bytes = bsp_mloader_read_file(file_name, offset, (u32)sizeof(struct dtb_table), (char *)(&dtb_table));
    if (rd_bytes != (int)sizeof(struct dtb_table)) {
        dt_pr_err("read dtb header failed, readed %d, size %lu\n", rd_bytes, sizeof(struct dtb_table));
        return BSP_ERROR;
    }
    if (dtb_table.magic != HSDT_MAGIC) {
        dt_pr_err("bad adrv dtb magic 0x%08x\n", dtb_table.magic);
        return BSP_ERROR;
    }

    len = sizeof(struct dtb_entry) * dtb_table.num_entries;
    entries = (struct dtb_entry *)bsp_dt_alloc_buffer(len);
    if (entries == NULL) {
        dt_pr_err("malloc dtb entries buffer failed, size %u, entries num %u\n", len, dtb_table.num_entries);
        return BSP_ERROR;
    }

    rd_bytes = bsp_mloader_read_file(file_name, offset + sizeof(struct dtb_table), len, (char *)entries);
    if (rd_bytes != (int)len) {
        dt_pr_err("read dtb entries failed, readed %d, size 0x%u\n", rd_bytes, len);
        goto out;
    }

    if (bsp_dt_find_dtb_entry(dtb_id, entries, dtb_table.num_entries, entry_out)) {
        dt_pr_err("fail to get dtb entry\n");
        goto out;
    }
    ret = BSP_OK;

out:
    bsp_dt_free_buffer(entries);
    return ret;
}


int bsp_dt_find_dtb_entry(u32 dtb_id, const struct dtb_entry *base_ptr, u32 num, struct dtb_entry *dtb_entry)
{
    int ret;
    u32 i;

    for (i = 0; i < num; i++) {
        if ((base_ptr->boardid[0x3] == dtb_id)) {
            dt_pr_err("found entry %d for modem_id(0x%x, 0x%x, 0x%x, 0x%x)\n", i, base_ptr->boardid[0x0],
                base_ptr->boardid[0x1], base_ptr->boardid[0x2], base_ptr->boardid[0x3]);

            ret = memcpy_s((void *)dtb_entry, sizeof(*dtb_entry), (void *)base_ptr, sizeof(*base_ptr));
            if (ret) {
                dt_pr_err("%s memcpy_s error, ret = %d\n", __FUNCTION__, ret);
                return BSP_ERROR;
            }
            return BSP_OK;
        }
        base_ptr++;
    }

    if (i == num) {
        dt_pr_err("not found entry for modemid(0x%x, 0x%x, 0x%x, 0x%x)\n", base_ptr->boardid[0x0],
            base_ptr->boardid[0x1], base_ptr->boardid[0x2], base_ptr->boardid[0x3]);
        return BSP_ERROR;
    }

    return BSP_OK;
}
