/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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
#include <product_config.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/zlib.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/dma-mapping.h>
#include <bsp_sysctrl.h>
#include <bsp_efuse.h>
#include <bsp_rfile.h>
#include <bsp_version.h>
#include <bsp_dt.h>
#include <securec.h>
#include <bsp_sec_call.h>
#include <bsp_mloader.h>

#define THIS_MODU mod_of

#if (defined(CONFIG_LOAD_SEC_IMAGE)) && (defined(CONFIG_TZDRIVER) || defined(CONFIG_TRUSTZONE_HM))
int bsp_dt_load_and_verify_ccore_dtb_sec(char *image_name, u32 image_type)
{
    int ret = 0;

    ret = bsp_mloader_load_modem_image(image_name, image_type, 0);

    if (bsp_fs_tell_load_mode() != BSP_FS_OK) {
        dt_pr_err("fs not ready, skip to load dts\n");
        return 0;
    }

    if (ret != 0) {
        dt_pr_err("load verify %s failed, ret = %d \n", image_name, ret);
        return ret;
    }

    ret = bsp_dt_parse_image_by_secos(image_name, image_type);
    if (ret != 0) {
        return DTB_ERROR;
    }

    dt_pr_err("load verify %s success\n", image_name);
    return DTB_OK;
}
#else
int bsp_dt_load_dtb_ccore_dtb_unsec(char *image_name, u32 dtb_id)
{
    int ret;
    void *vaddr = NULL;
    unsigned long paddr = 0;
    u32 ddr_size = 0;
    u32 offset = 0;
    char file_path[DTS_FILE_PATH_LEN] = {0};

    if (bsp_fs_tell_load_mode() != BSP_FS_OK) {
        dt_pr_err("fs not ready, skip to load dts\n");
        return 0;
    }

    offset = bsp_mloader_get_vrl_size();

    ret = bsp_dt_get_image_path(image_name, file_path, DTS_FILE_PATH_LEN, MODEM_IMG_CCPU_DTB);
    if (ret < 0) {
        dt_pr_err("bsp dt get image %s path failed, ret = %d\n", image_name, ret);
    }

    ret = bsp_mloader_get_dtb_ddr_region(image_name, &paddr, &ddr_size);
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

int bsp_dt_load_and_verify_dtb(char *image_name, u32 dtb_id)
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
    ret = bsp_dt_load_and_verify_ccore_dtb_sec(image_name, MODEM_IMG_CCPU_DTB);
#else
    ret = bsp_dt_load_dtb_ccore_dtb_unsec(image_name, dtb_id);
#endif

    if (ret) {
        dt_pr_err("mloader load and verify %s fail ,ret = %d\n", image_name, ret);
    };

    return ret;
}

#ifdef CONFIG_ENABLE_DTO
static int bsp_dt_load_and_verify_dto_image(void)
{
    int ret;

    ret = bsp_mloader_load_modem_image(MODEM_DTO_IMG, MODEM_IMG_CCPU_DTO, 0);

    if (bsp_fs_tell_load_mode() != BSP_FS_OK) {
        dt_pr_err("fs not ready, skip to load dto\n");
        return 0;
    }

    if (ret) {
        dt_pr_err("load and verify modem_dto failed : %d\n", ret);
        return ret;
    }

    ret = bsp_dt_parse_image_by_secos(MODEM_DTO_IMG, MODEM_IMG_CCPU_DTO);
    if (ret != 0) {
        return DTB_ERROR;
    }

    return DTB_OK;
}
#endif

int bsp_dt_load_and_verify_ccore_dt_image(void)
{
    int ret;

    ret = bsp_dt_load_and_verify_dtb(MODEM_DT_IMG, BOARDID_CCORE_IDX);
    if (ret) {
        return ret;
    }
#ifdef CONFIG_ENABLE_DTO
    ret = bsp_dt_load_and_verify_dto_image();
#endif
    return ret;
}