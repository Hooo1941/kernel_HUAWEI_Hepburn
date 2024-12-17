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
#include <bsp_mloader.h>
#include <securec.h>
#include <asm/memory.h>

#if (defined(CONFIG_LOAD_SEC_IMAGE)) && (defined(CONFIG_TZDRIVER) || defined(CONFIG_TRUSTZONE_HM))
int bsp_dt_sec_load_image(const char *file_path, mloader_img_id_e image)
{
    int ret;
    int rd_bytes;
    void *fdt = NULL;
    u32 total_size = bsp_mloader_get_file_size(file_path);
    u32 vrl_size = bsp_mloader_get_vrl_size();
    u32 image_size = total_size - vrl_size;

    fdt = bsp_dt_alloc_buffer(total_size);
    if (fdt == NULL) {
        dt_pr_err("malloc dtb buffer failed, dtb size is %u\n", total_size);
        return DTB_ERROR;
    }

    ret = bsp_mloader_sec_prepare(image);
    if (ret) {
        dt_pr_err("sec prepare error, ret %d!\n", ret);
        goto err_out;
    }

    rd_bytes = bsp_mloader_read_file(file_path, 0, vrl_size, (char *)fdt);
    if (rd_bytes != vrl_size) {
        dt_pr_err("read dtb %s failed: readed %d, dtb size %u\n", file_path, rd_bytes, vrl_size);
        ret = DTB_ERROR;
        goto err_out;
    }

    ret = bsp_mloader_sec_copy_vrl(image, fdt, vrl_size);
    if (ret) {
        dt_pr_err("trans_vrl_to_os error, ret %d!\n", ret);
        goto err_out;
    }

    rd_bytes = bsp_mloader_read_file(file_path, vrl_size, image_size, (char *)fdt);
    if (rd_bytes != image_size) {
        dt_pr_err("read dtb %s failed: readed %d, dtb size %u\n", file_path, rd_bytes, image_size);
        ret = DTB_ERROR;
        goto err_out;
    }

    ret = bsp_mloader_sec_copy_data(image, fdt, 0, image_size);
    if (ret) {
        dt_pr_err("trans_data_to_os error, ret %d!\n", ret);
        goto err_out;
    }

    ret = bsp_mloader_sec_verify(image);
    if (ret) {
        dt_pr_err("bsp_mloader_sec_verify error, ret %d!\n", ret);
        goto err_out;
    }
err_out:
    bsp_dt_free_buffer(fdt);
    return ret;
}

int bsp_dt_load_and_verify_fw_dtb_sec(char *image_name, u32 image_type)
{
    int ret = 0;
    char file_path[DTS_FILE_PATH_LEN] = {0};

    ret = bsp_dt_get_image_path(image_name, file_path, DTS_FILE_PATH_LEN, image_type);
    if (ret < 0) {
        dt_pr_err("get image  %s path failed, ret %d!\n", image_name, ret);
        return DTB_ERROR;
    }

    ret = bsp_dt_sec_load_image(file_path, image_type);
    if (ret < 0) {
        dt_pr_err("load verify %s failed, ret %d!\n", file_path, ret);
        return DTB_ERROR;
    }

    ret = bsp_dt_parse_image_by_secos(image_name, image_type);
    if (ret != 0) {
        return DTB_ERROR;
    }

    dt_pr_err("load verify %s success\n", image_name);
    return DTB_OK;
}
#else
int bsp_dt_load_fw_dtb_unsec(char *image_name, u32 dtb_id)
{
    int ret;
    void *vaddr = NULL;
    unsigned long paddr = DDR_FW_DTB_ADDR;
    u32 ddr_size = DDR_FW_DTB_SIZE;
    u32 offset = 0;
    char file_path[DTS_FILE_PATH_LEN] = {0};

    offset = bsp_mloader_get_vrl_size();

    ret = bsp_dt_get_image_path(image_name, file_path, DTS_FILE_PATH_LEN, MODEM_IMG_FW_DTB);
    if (ret < 0) {
        dt_pr_err("bsp dt get image %s path failed, ret = %d\n", image_name, ret);
    }

    vaddr = ioremap_wc(paddr, (unsigned long)ddr_size);
    if (vaddr == NULL) {
        dt_pr_err("ioremap_wc error\n");
        return DTB_ERROR;
    }

    ret = bsp_dt_parse_image_by_kernel(file_path, dtb_id, offset, vaddr, ddr_size);
    if(ret != 0) {
        dt_pr_err("parse %s by kernel failed, ret: %d\n", file_path, ret);
        iounmap(vaddr);
        return DTB_ERROR;
    }
    dt_pr_err("load image %s success\n", file_path);
    return DTB_OK;
}
#endif

int bsp_dt_load_verify_fw_dtb(char *image_name, u32 dtb_id)
{
    int ret;
    
    if (image_name == NULL) {
        dt_pr_err("image_name is NULL\n");
        return DTB_ERROR;
    }

/*
 * CONFIG_TZDRIVER & CONFIG_LOAD_SEC_IMAGE is for tzdrier(old secos, for 765)
 * CONFIG_TRUSTZONE_HM & CONFIG_LOAD_SEC_IMAGE is for tzdrier_hm(new hm secos)
 */
#if (defined(CONFIG_LOAD_SEC_IMAGE)) && (defined(CONFIG_TZDRIVER) || defined(CONFIG_TRUSTZONE_HM))
    /* 安全版本且使能了签名 */
    ret = bsp_dt_load_and_verify_fw_dtb_sec(image_name, MODEM_IMG_FW_DTB);
#else
    ret = bsp_dt_load_fw_dtb_unsec(image_name, dtb_id);
#endif

    if (ret) {
        dt_pr_err("mloader load and verify %s fail ,ret = %d\n", image_name, ret);
    };

    return ret;

}

int bsp_fw_dt_init(void)
{
    int ret = 0;
    dt_pr_err("use %d as dtb_id to load base dtb\n", BOARDID_FW_IDX);

    ret = bsp_dt_load_verify_fw_dtb(MODEM_FW_DT_IMG, BOARDID_FW_IDX);
    if (ret) {
        dt_pr_err("load dtb fail from file %s, ret = %d\n", MODEM_FW_DT_IMG, ret);
        return -1;
    }

    return BSP_OK;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(bsp_fw_dt_init);
#endif
