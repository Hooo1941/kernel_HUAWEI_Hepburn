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
#include <product_config.h>
#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <bsp_dt.h>
#include <bsp_nvim.h>
#include <bsp_reset.h>
#include <bsp_rfile.h>
#include <bsp_slice.h>
#include <bsp_sysboot.h>
#include <bsp_ipc_fusion.h>
#include <bsp_mloader.h>
#include <mdrv_sysboot.h>
#include <mdrv_memory_layout.h>

#include <securec.h>
#include "mloader_err.h"
#include "mloader_util.h"
#include "mloader_load_image.h"
#include "mloader_load_patch.h"
#include "mloader_sec_call.h"
#include "mloader_debug.h"

#define MODEM_IMAGE_NAME "modem_tsp.bin"
#define MODEM_TEE_IMAGE_NAME "modem_mtee.bin"
#define MODEM_ASLR_MAX_OFFSET  (1 * 1024 * 1024)

static int mloader_is_modem_tee_exist(void)
{
    unsigned long addr;
    unsigned int size = 0;

    addr = mdrv_mem_region_get("mdm_tee_ddr", &size);

    if ((!addr) && (!size)) {
        return 0;
    } else {
        return 1;
    }
}

static int mloader_notify_mcu_modem_started(void)
{
    int ret;
    ipc_handle_t mloader_ipc_handle;

    ret = bsp_ipc_open(IPC_MCU_INT_SRC_ACPU_CCPU_START, &mloader_ipc_handle);
    if (ret) {
        mloader_print_err("fail to open ipc, ret = %d\n", ret);
        return ret;
    }

    return bsp_ipc_send(mloader_ipc_handle);
}

int mloader_load_ccpu_image(u32 patch_size)
{
    int ret;
    u32 load_offset;
    u32 ddr_size = 0;
    unsigned long phys_addr = 0;
#ifdef CONFIG_COMPRESS_CCORE_IMAGE
    int image_size;
#endif
#if !((defined(CONFIG_LOAD_SEC_IMAGE)) && (defined(CONFIG_TZDRIVER) || defined(CONFIG_TRUSTZONE_HM)))
    char *virt_addr = NULL;
#endif

    ret = bsp_mloader_get_ddr_region(MODEM_IMAGE_NAME, &phys_addr, &ddr_size);
    if (ret) {
        mloader_print_err("fail to get modem ddr info, ret = %d\n", ret);
        return ret;
    }

#ifdef CONFIG_COMPRESS_CCORE_IMAGE
    image_size = mloader_get_modem_image_size(MODEM_IMAGE_NAME);
    if (image_size < 0) {
        mloader_print_err("fail to get modem image size, ret = %d\n", ret);
        return ret;
    }

    image_size = PAGE_ALIGN(image_size);

    load_offset = ddr_size - image_size - patch_size - MODEM_ASLR_MAX_OFFSET;
#else
    load_offset = 0;
#endif
    ret = bsp_mloader_load_modem_image(MODEM_IMAGE_NAME, MODEM_IMG_CCPU_REE, load_offset);
    if (ret) {
        return ret;
    }
#if !((defined(CONFIG_LOAD_SEC_IMAGE)) && (defined(CONFIG_TZDRIVER) || defined(CONFIG_TRUSTZONE_HM)))
#ifdef CONFIG_COMPRESS_CCORE_IMAGE
    if (bsp_fs_tell_load_mode() != BSP_FS_OK) {
        mloader_print_err("fs not ready, skip to decompress image\n");
        return 0;
    }
    virt_addr = mloader_map_ddr_region(MODEM_IMAGE_NAME);
    ret = bsp_mloader_decompress(virt_addr, ddr_size, virt_addr + load_offset, image_size);
    mloader_unmap_ddr_region(virt_addr);
    if (ret) {
        mloader_print_err("fail to decompress image, ret = %d\n", ret);
    }
#endif
#endif
    return ret;
}

int mloader_load_ccpu_tee_image(u32 patch_size)
{
    int ret;
    u32 ddr_size = 0;
    unsigned long phys_addr = 0;

    ret = bsp_mloader_get_ddr_region(MODEM_TEE_IMAGE_NAME, &phys_addr, &ddr_size);
    if (ret) {
        mloader_print_err("fail to get modem ddr info, ret = %d\n", ret);
        return ret;
    }

    ret = bsp_mloader_load_modem_image(MODEM_TEE_IMAGE_NAME, MODEM_IMG_CCPU_TEE, 0);
    if (ret) {
        return ret;
    }

    return ret;
}

int mloader_load_ccpu_patch(u32 *patch_size)
{
    int ret;
    u32 ddr_size = 0;
    unsigned long phys_addr = 0;

    ret = bsp_mloader_get_ddr_region(MODEM_IMAGE_NAME, &phys_addr, &ddr_size);
    if (ret) {
        mloader_print_err("fail to get modem ddr info, ret = %d\n", ret);
        return ret;
    }

    ret = mloader_load_patch(MODEM_IMAGE_NAME, ddr_size, patch_size);
    return ret;
}

int mloader_load_ccpu_tee_patch(u32 *patch_size)
{
    int ret;
    u32 ddr_size = 0;
    unsigned long phys_addr = 0;

    ret = bsp_mloader_get_ddr_region(MODEM_TEE_IMAGE_NAME, &phys_addr, &ddr_size);
    if (ret) {
        mloader_print_err("fail to get modem ddr info, ret = %d\n", ret);
        return ret;
    }

    ret = mloader_load_patch(MODEM_TEE_IMAGE_NAME, ddr_size, patch_size);
    return ret;
}

static int mloader_set_modem_boot_para(void)
{
#if !((defined(CONFIG_LOAD_SEC_IMAGE)) && (defined(CONFIG_TZDRIVER) || defined(CONFIG_TRUSTZONE_HM)))
    int ret;
    u32 ddr_size = 0;
    unsigned long phys_addr = 0;
    ret = bsp_mloader_get_ddr_region(MODEM_IMAGE_NAME, &phys_addr, &ddr_size);
    if (ret) {
        mloader_print_err("fail to get modem ddr info, ret = %d\n", ret);
        return ret;
    }

    return bsp_sysboot_set_boot_para(phys_addr, ddr_size);
#else
    return 0;
#endif
}

int mloader_load_modem_image(void)
{
    int ret;
    u32 patch_size = 0;

    ret = mloader_verify_prepare(MODEM_IMG_CCPU_REE);
    if (ret) {
        mloader_print_err("fail to assert modem reset, ret = %d\n", ret);
        return ret;
    }

    ret = bsp_dt_load_and_verify_ccore_dt_image();
    if (ret) {
        mloader_print_err("fail to load modem dtb image, ret = %d\n", ret);
        goto fail_to_load_image;
    }

    if (mloader_is_modem_tee_exist()) {
        ret = mloader_load_ccpu_tee_patch(&patch_size);
        if (ret) {
            mloader_print_err("fail to load modem patch image, ret = %d\n", ret);
            goto fail_to_load_image;
        }

        ret = mloader_load_ccpu_tee_image(patch_size);
        if (ret) {
            goto fail_to_load_image;
        }
    }

    ret = mloader_load_ccpu_patch(&patch_size);
    if (ret) {
        mloader_print_err("fail to load modem patch image, ret = %d\n", ret);
        goto fail_to_load_image;
    }

    ret = mloader_load_ccpu_image(patch_size);

fail_to_load_image:
    return ret;
}

void mloader_start_modem(void)
{
    int ret;

    ret = mloader_load_modem_image();
    if (ret) {
        mloader_print_err("fail to load modem image, ret = %d\n", ret);
        return;
    }

    ret = mloader_set_modem_boot_para();
    if (ret) {
        mloader_print_err("fail to set modem boot para, ret = %d\n", ret);
        return;
    }

    if (bsp_reset_is_feature_on() == MDM_RESET_UNSUPPORT) {
#ifndef BSP_CONFIG_PHONE_TYPE
        /* phone can't free, sec_auth need buffer */
        mloader_free_load_buffer();
#endif
    }

    ret = mloader_notify_mcu_modem_started();
    mloader_print_err("notify mcu modem started ret = %d\n", ret);
}
