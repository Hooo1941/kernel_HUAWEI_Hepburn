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
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <bsp_dt.h>
#include <bsp_efuse.h>
#include <bsp_rfile.h>
#include <bsp_mloader.h>
#include <bsp_sysctrl.h>
#include <mdrv_memory_layout.h>
#include <securec.h>
#include "mloader_err.h"
#include "mloader_debug.h"
#include "mloader_load_image.h"

#define MODEM_FW_FIXED_PATH "/vendor/modem/modem_fw"
#define MODEM_VENDOR_FIXED_PATH "/vendor/modem/modem_vendor"

struct img_info {
    char file_name[MLOADER_FILE_NAME_LEN];
    char ddr_name[MLOADER_FILE_NAME_LEN];
    unsigned long ddr_phys_addr;
    void *ddr_virt_addr;
    u32 ddr_size;

    char dtb_file_name[MLOADER_FILE_NAME_LEN];
    char dtb_ddr_name[MLOADER_FILE_NAME_LEN];
    unsigned long dtb_ddr_phys_addr;
    void *dtb_ddr_virt_addr;
    u32 dtb_ddr_size;
};

struct loader_info {
    struct semaphore sem;
    char *local_buf;
    u32 img_info_count;
    struct img_info *img_info;
};

struct loader_info g_mloader_loader_info;

int bsp_mloader_get_vrl_size(void)
{
    return CONFIG_VRL_SIZE;
}

const char *bsp_mloader_get_modem_fw_path(void)
{
    return MODEM_FW_FIXED_PATH;
}
EXPORT_SYMBOL_GPL(bsp_mloader_get_modem_fw_path);

const char *bsp_mloader_get_modem_vendor_path(void)
{
    return MODEM_VENDOR_FIXED_PATH;
}
EXPORT_SYMBOL_GPL(bsp_mloader_get_modem_vendor_path);

int mloader_get_file_path(char *file_path, u32 buf_len, const char *file_name)
{
    int ret;
    const char *modem_fw_path = bsp_mloader_get_modem_fw_path();
    ret = sprintf_s(file_path, buf_len, "%s/%s", modem_fw_path, file_name);
    if (ret < 0) {
        mloader_print_err("fail to fmt modem image path: %s, %s\n", modem_fw_path, file_name);
        return ret;
    }
    return 0;
}

char *bsp_mloader_get_local_buffer(u32 *size)
{
    if (size != NULL) {
        *size = MLOADER_SECBOOT_BUFLEN;
    }
    return g_mloader_loader_info.local_buf;
}

#if (defined(CONFIG_LOAD_SEC_IMAGE)) && (defined(CONFIG_TZDRIVER) || defined(CONFIG_TRUSTZONE_HM))
int bsp_mloader_sec_load_image(const char *file_path, mloader_img_id_e image, u32 ddr_offset)
{
    int ret;
    ssize_t read_size;
    struct file *fp = NULL;
    loff_t offset_loff_t = 0;
    u32 buffer_size = 0;
    char *buffer = bsp_mloader_get_local_buffer(&buffer_size);
    u32 vrl_size = bsp_mloader_get_vrl_size();

    if (vrl_size > buffer_size) {
        mloader_print_err("fail to read file(%s) vrl, buffer err\n", file_path);
        return MLOADER_ERR_NO_MEMORY;
    }

#ifndef BSP_CONFIG_PHONE_TYPE
    if (bsp_fs_tell_load_mode() != BSP_FS_OK) {
        mloader_print_err("fs not ready, skip to load image %s\n", file_path); /* phone需要走完整加载校验流程，无法单独校验镜像 */
        return bsp_mloader_sec_verify(image);
    }
#endif

    fp = filp_open(file_path, O_RDONLY, 0);
    if (IS_ERR_OR_NULL(fp)) {
        return (int)PTR_ERR(fp);
    }

    if (image != MODEM_IMG_CCPU_REE) { //CCPU prepare ready before this func
        ret = bsp_mloader_sec_prepare(image);
    }

    read_size = kernel_read(fp, buffer, vrl_size, &offset_loff_t);
    if (read_size != vrl_size) {
        mloader_print_err("fail to read file(%s) vrl, ret = %zd\n", file_path, read_size);
        goto fail_read_file;
    }

    ret = bsp_mloader_sec_copy_vrl(image, buffer, read_size);
    mloader_print_info("copy vrl: image %d, size %zd, ret = %d\n", image, read_size, ret);

    do {
        read_size = kernel_read(fp, buffer, buffer_size, &offset_loff_t);
        if (read_size == 0) {
            break;
        }
        if (read_size < 0) {
            mloader_print_err("fail to read file(%s) data, ret = %zd\n", file_path, read_size);
            goto fail_read_file;
        }
        ret = bsp_mloader_sec_copy_data(image, buffer, ddr_offset, read_size);
        mloader_print_info("copy data: image %d, offset 0x%X, size 0x%zX, ret %d\n", image, ddr_offset, read_size, ret);
        ddr_offset += read_size;
    } while (read_size > 0);

fail_read_file:
    filp_close(fp, NULL);

    ret = bsp_mloader_sec_verify(image);

    return ret;
}

int bsp_mloader_sec_load_modem_image(char *file_name, mloader_img_id_e image, u32 ddr_offset)
{
    int ret;
    char file_path[MLOADER_FILE_PATH_LEN] = { 0 };

    ret = mloader_get_file_path(file_path, sizeof(file_path), file_name);
    if (ret < 0) {
        return ret;
    }

    return bsp_mloader_sec_load_image((const char *)file_path, image, ddr_offset);
}

int bsp_mloader_load_modem_image(char *file_name, mloader_img_id_e image, u32 ddr_offset)
{
    return bsp_mloader_sec_load_modem_image(file_name, image, ddr_offset);
}
#else
char *mloader_map_ddr_region(const char *file_name)
{
    int ret;
    unsigned long phys_addr = 0;
    u32 ddr_size = 0;
    void *addr = NULL;

    ret = bsp_mloader_get_ddr_region(file_name, &phys_addr, &ddr_size);
    addr = ioremap_wc(phys_addr, ddr_size);
    if (addr == NULL) {
        mloader_print_err("fail to map addr\n");
    }
    return (char *)addr;
}

void mloader_unmap_ddr_region(char *addr)
{
    iounmap(addr);
}

int bsp_mloader_unsec_load_modem_image(const char *file_name, mloader_img_id_e image, u32 ddr_offset)
{
    int ret;
    ssize_t read_size;
    char *buffer = NULL;
    struct file *fp = NULL;
    loff_t offset_loff_t = bsp_mloader_get_vrl_size();
    char file_path[MLOADER_FILE_PATH_LEN] = { 0 };

    if (bsp_fs_tell_load_mode() != BSP_FS_OK) {
        mloader_print_err("fs not ready, skip to load image %s\n", file_name);
        return 0;
    }

    ret = mloader_get_file_path(file_path, sizeof(file_path), file_name);
    if (ret < 0) {
        return ret;
    }

    fp = filp_open(file_path, O_RDONLY, 0);
    if (IS_ERR_OR_NULL(fp)) {
        ret = (int)PTR_ERR(fp);
        return ret;
    }
    buffer = mloader_map_ddr_region(file_name);
    if (buffer == NULL) {
        ret = -ENOMEM;
        goto fail_map;
    }
    do {
        read_size = kernel_read(fp, buffer + ddr_offset, MLOADER_SECBOOT_BUFLEN, &offset_loff_t);
        if (read_size == 0) {
            break;
        }
        if (read_size < 0) {
            ret = read_size;
            mloader_print_err("fail to read file(%s) data, ret = %zd\n", file_path, read_size);
            goto fail_read_file;
        }
        ddr_offset += read_size;
    } while (read_size > 0);

fail_read_file:
    mloader_unmap_ddr_region(buffer);
fail_map:
    filp_close(fp, NULL);
    return ret;
}

int bsp_mloader_load_modem_image(char *file_name, mloader_img_id_e image, u32 ddr_offset)
{
    return bsp_mloader_unsec_load_modem_image(file_name, image, ddr_offset);
}
#endif

int mloader_get_modem_image_size(const char *file_name)
{
    int ret;
    char file_path[MLOADER_FILE_PATH_LEN] = { 0 };

    if (bsp_fs_tell_load_mode() != BSP_FS_OK) {
        mloader_print_err("fs not ready, skip to get image size\n");
        return 0;
    }

    ret = mloader_get_file_path(file_path, sizeof(file_path), file_name);
    if (ret < 0) {
        return ret;
    }

    return bsp_mloader_get_file_size((const char *)file_path);
}

int bsp_mloader_get_ddr_region(const char *file_name, unsigned long *phys_addr, u32 *size)
{
    u32 i;
    struct img_info *img_info = g_mloader_loader_info.img_info;

    if ((file_name == NULL) || (phys_addr == NULL) || (size == NULL)) {
        return MLOADER_ERR_ARG;
    }

    for (i = 0; i < g_mloader_loader_info.img_info_count; i++) {
        if (strncmp(img_info[i].file_name, file_name, MLOADER_FILE_NAME_LEN) == 0) {
            *phys_addr = img_info[i].ddr_phys_addr;
            *size = img_info[i].ddr_size;
            return 0;
        }
    }
    return MLOADER_ERR_ARG;
}

int bsp_mloader_get_dtb_ddr_region(const char *file_name, unsigned long *phys_addr, u32 *size)
{
    u32 i;
    struct img_info *img_info = g_mloader_loader_info.img_info;

    if ((file_name == NULL) || (phys_addr == NULL) || (size == NULL)) {
        return MLOADER_ERR_ARG;
    }
    for (i = 0; i < g_mloader_loader_info.img_info_count; i++) {
        if (strncmp(img_info[i].dtb_file_name, file_name, MLOADER_FILE_NAME_LEN) == 0) {
            *phys_addr = img_info[i].dtb_ddr_phys_addr;
            *size = img_info[i].dtb_ddr_size;
            return 0;
        }
    }
    return MLOADER_ERR_ARG;
}
EXPORT_SYMBOL_GPL(bsp_mloader_get_dtb_ddr_region);

void mloader_free_load_buffer(void)
{
    if (g_mloader_loader_info.local_buf != NULL) {
        kfree(g_mloader_loader_info.local_buf);
        g_mloader_loader_info.local_buf = NULL;
    }
}

static int mloader_get_dts_string_property(device_node_s *node, const char *propname,
                                            char *output, u32 output_len)
{
    int ret;
    char *name = NULL;
    ret = bsp_dt_property_read_string_index(node, propname, 0, (const char **)&name);
    if (ret) {
        mloader_print_err("fail to get %s info from dts, ret = %d\n", propname, ret);
        return ret;
    }
    ret = strcpy_s(output, output_len, name);
    if (ret) {
        mloader_print_err("strcpy %s failed, ret = %d\n", propname, ret);
    }
    return ret;
}

static int mloader_get_dtb_info(device_node_s *dev_node, struct img_info *img_info)
{
    u32 ret = 0;
    device_node_s *dts_node = NULL;

    dts_node = bsp_dt_get_child_by_name(dev_node, "modem_dtb");
    if (dts_node == NULL) {
        return 0;
    }

    ret |= mloader_get_dts_string_property(dts_node, "file_name", img_info->dtb_file_name,
                                        sizeof(img_info->dtb_file_name));
    ret |= mloader_get_dts_string_property(dts_node, "dtb_ddr_name", img_info->dtb_ddr_name,
                                        sizeof(img_info->dtb_ddr_name));
    img_info->dtb_ddr_phys_addr = mdrv_mem_region_get(img_info->dtb_ddr_name, &img_info->dtb_ddr_size);

    return (int)ret;
}

static int mloader_get_dts(struct img_info **img_info_out)
{
    u32 ret = 0;

    int img_info_count;
    struct img_info *img_info = NULL;
    device_node_s *dev_node = NULL;
    device_node_s *child_node = NULL;

    dev_node = bsp_dt_find_compatible_node(NULL, NULL, "modem,mloader");
    if (dev_node == NULL) {
        mloader_print_err("fail to get mloader dts\n");
        return MLOADER_ERR_ARG;
    }
    img_info_count = bsp_dt_get_child_count(dev_node);
    if ((img_info_count < 1) || (img_info_count > MLOADER_MAX_IMG_COUNT)) {
        mloader_print_err("mloader node count err, count = %d\n", img_info_count);
        return MLOADER_ERR_ARG;
    }
    img_info = (struct img_info *)kzalloc(sizeof(struct img_info) * img_info_count, GFP_KERNEL);
    if (img_info == NULL) {
        mloader_print_err("fail to malloc, size: 0x%X\n", (u32)sizeof(struct img_info) * img_info_count);
        return MLOADER_ERR_NO_MEMORY;
    }
    *img_info_out = img_info;

    g_mloader_loader_info.img_info_count = img_info_count;

    bsp_dt_for_each_child_of_node(dev_node, child_node) {
        ret |= (u32)mloader_get_dts_string_property(child_node, "image_name", img_info->file_name,
                                                    sizeof(img_info->file_name));
        ret |= (u32)mloader_get_dts_string_property(child_node, "ddr_name", img_info->ddr_name,
                                                    sizeof(img_info->ddr_name));
        img_info->ddr_phys_addr = mdrv_mem_region_get(img_info->ddr_name, &img_info->ddr_size);

        ret |= (u32)mloader_get_dtb_info(child_node, img_info);
        img_info++;
    }

    if (ret) {
        mloader_print_err("fail to get mloader dts, ret = %d\n", (int)ret);
    }

    return (int)ret;
}

int mloader_loader_init(void)
{
    int ret;

    sema_init(&g_mloader_loader_info.sem, 1);

    g_mloader_loader_info.local_buf = (char *)kzalloc(PAGE_ALIGN(MLOADER_SECBOOT_BUFLEN), GFP_KERNEL);
    if (g_mloader_loader_info.local_buf == NULL) {
        mloader_print_err("fail to malloc loader buffer\n");
        return -ENOMEM;
    }

    ret = mloader_get_dts(&g_mloader_loader_info.img_info);
    if (ret) {
        return ret;
    }

    mloader_debug_init();

    mloader_print_err("mloader loader init ok\n");

    return ret;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(mloader_loader_init);
#endif
