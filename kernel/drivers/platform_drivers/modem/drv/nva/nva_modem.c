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
 */

#include <product_config.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/of_device.h>
#include <linux/dma-mapping.h>
#include <securec.h>
#include <osl_malloc.h>
#include <osl_thread.h>
#include <osl_sem.h>
#include <mdrv_memory.h>
#include <bsp_nvim.h>
#include <bsp_nva.h>
#include <bsp_blk.h>
#include <bsp_mloader.h>
#include <bsp_msg.h>
#include <msg_id.h>
#include "nva_modem.h"


#undef THIS_MODU
#define THIS_MODU mod_nva

#ifdef BSP_CONFIG_PHONE_TYPE
unsigned long g_nva_shm_addr = 0;
u32 g_nva_shm_size = 0;
u32 g_acore_init_state = 0;
nva_ctrl_info_s g_nva_info = { 0 };

nva_ctrl_info_s *nva_get_nvainfo(void)
{
    return &g_nva_info;
}

/*
 * nva id查找接口
 */
s32 nva_search_item_info(u32 itemid, nva_item_info_s **item_info)
{
    u32 low;
    u32 high;
    u32 mid;
    nva_ctrl_info_s *nvainfo = nva_get_nvainfo();
    nva_item_info_s *item_table = NULL;
    nva_file_info_s *file_info = NULL;

    item_table = (nva_item_info_s *)(uintptr_t)((uintptr_t)nvainfo->load_addr + sizeof(nva_file_info_s));
    file_info = (nva_file_info_s *)(uintptr_t)(nvainfo->load_addr);

    low = 1;
    high = file_info->id_sum;

    while (low <= high) {
        mid = (low + high) / 0x2;

        *item_info = &item_table[mid - 1];

        if (itemid < (*item_info)->item_id) {
            high = mid - 1;
        } else if (itemid > (*item_info)->item_id) {
            low = mid + 1;
        } else {
            return NVA_OK;
        }
    }

    *item_info = NULL;
    return NVA_ERROR;
}

static struct platform_device g_modem_nva_device = {
    .name = "modem_nva",
    .id = 0,
    .dev = {
        .init_name = "modem_nva",
    },
};
void nva_dma_init(void)
{
    struct device *pdev = &g_modem_nva_device.dev;
    of_dma_configure(pdev, NULL, true);
    dma_set_mask_and_coherent(pdev, ~(0x1FULL)); // 32 bytes align

    return;
}
/* 仅用于初始化nv设备 */
s32 nva_init_dev(void)
{
    u32 ret;
    ret = (u32)platform_device_register(&g_modem_nva_device);
    if (ret) {
        nva_printf("platform_device_register modem_nv_device fail !\n");
        return -1;
    }
    nva_dma_init();
    nva_printf("init modem nv dev ok\n");
    return NV_OK;
}

void *nva_dma_alloc(u32 size, dma_addr_t *dma_handle)
{
    void *mem_addr = NULL;
    struct device *pdev = &g_modem_nva_device.dev;

    mem_addr = dma_alloc_coherent(pdev, size, dma_handle, GFP_KERNEL);
    if (IS_ERR_OR_NULL(mem_addr)) {
        nva_printf("nv alloc failed\n");
        return NULL;
    }
    return mem_addr;
}

void nva_dma_free(size_t size, void *cpu_addr, dma_addr_t dma_handle)
{
    struct device *pdev = &g_modem_nva_device.dev;
    dma_free_coherent(pdev, size, cpu_addr, dma_handle);
    return;
}

s32 nva_read_right(void)
{
    nva_ctrl_info_s *nvainfo = nva_get_nvainfo();

    if (nvainfo->nva_init_state == NVA_INIT_FAIL) {
        return NVA_ERROR;
    }

    return NVA_OK;
}

s32 nva_ddr_init(void)
{
    nva_ctrl_info_s *nvainfo = nva_get_nvainfo();
    dma_addr_t dma_handle;
    u32 update_size = nvainfo->nvafs.update.pinfo.fileinfo.size;
    u32 image_size = nvainfo->nvafs.image.pinfo.fileinfo.size;
    u32 size = update_size > image_size ? update_size : image_size;
    nvainfo->load_addr = nva_dma_alloc(size, &dma_handle);
    if (nvainfo->load_addr == NULL) {
        nva_printf("image not exit, need upgrade\n");
        return NVA_ERROR;
    }
    return NVA_OK;
}
s32 nva_check_upgrade_flag(void)
{
    u32 ret;
    nva_ctrl_info_s *nvainfo = nva_get_nvainfo();
    struct nva_update_partition *update = &nvainfo->nvafs.update;
    struct nva_img_partition *image = &nvainfo->nvafs.image;

    if (image->pinfo.fileinfo.magic_num != NVA_FILE_EXIST) {
        nva_printf("image not exit, need upgrade\n");
        return NVA_NEED_UPGRADE;
    }

    ret = (u32)memcmp((void *)update->hashinfo, (void *)image->finfo[NVA_HASH].hashinfo, HASH_RESULT_LEN);
    if (ret) {
        nva_printf("update need upgrade\n");
        return NVA_NEED_UPGRADE;
    }

    return NVA_NEED_RELOAD;
}

s32 nva_init(void)
{
    s32 ret;
    nva_ctrl_info_s *nvainfo = nva_get_nvainfo();
    nvainfo->nva_init_state = NVA_INIT_FAIL;
    nva_printf("nva init begin");
    osl_sem_init(1, &nvainfo->rw_sem);
    ret = nva_init_dev();
    if (ret) {
        nva_printf("nva dev init err\n");
        return NVA_ERROR;
    }
    ret = nvafs_init(&nvainfo->nvafs);
    if (ret) {
        nva_printf("nvafs init err\n");
        return NVA_ERROR;
    }
    ret = nva_ddr_init();
    if (ret) {
        nva_printf("ddr init err\n");
        return NVA_ERROR;
    }
    ret = nva_check_upgrade_flag();
    if (ret == NVA_NEED_UPGRADE) {
        nva_printf("upgrade start.\n");
        ret = (s32)nva_load_update();
        if (ret != NV_OK) {
            nva_printf("upgrade fail\n");
            return NVA_ERROR;
        }
        nvainfo->nva_init_state = NVA_INIT_OK;
        nva_printf("upgrade success\n");
    } else {
        nva_printf("reload start.\n");
        ret = (s32)nva_load_img();
        if (ret != NV_OK) {
            nva_printf("reload fail\n");
            ret = (s32)nva_load_update();
            if (ret != NV_OK) {
                nva_printf("emergency recover fail\n");
                return NVA_ERROR;
            }
        }
        nvainfo->nva_init_state = NVA_INIT_OK;
        nva_printf("nva reload success\n");
    }
    nva_printf("nva init ok");
    return NVA_OK;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(nva_init);
#endif

/*
 * nva 读取接口
 */
s32 nva_read(u32 itemid, u8 *data_buf, u32 buf_len)
{
    s32 ret;
    nva_ctrl_info_s *nvainfo = nva_get_nvainfo();

    nva_item_info_s *item_info = NULL;
    void *item_addr = NULL;

    if ((data_buf == NULL) || (buf_len == 0)) {
        return NVA_ERROR;
    }

    ret = nva_search_item_info(itemid, &item_info);
    if (ret) {
        nva_printf("can not find 0x%x !\n", itemid);
        return NVA_ERROR;
    }

    if (buf_len > item_info->data_len) {
        nva_printf("len overflow, nvid=0x%x, buf_len=0x%x, item_data_len=0x%x\n", itemid, buf_len, item_info->data_len);
        return NVA_ERROR;
    }

    item_addr = (void *)(uintptr_t)((uintptr_t)nvainfo->load_addr + item_info->base_addr);

    ret = memcpy_s(data_buf, buf_len, item_addr, buf_len);
    if (ret) {
        nva_printf("read nva from memory failed nvid=0x%x 0x%x!\n", itemid, ret);
        return NVA_ERROR;
    }

    return NVA_OK;
}

/*
 * 函数说明：nva读取接口（底软）
 * 参数说明：nva_id为待读取nva的编号；data_buf为存储nva数据的地址；buf_len为buf长度，通常为nva结构体大小
 */
s32 bsp_nva_read(u32 nva_id, u8 *data_buf, u32 buf_len)
{
    s32 ret;
    ret = nva_read_right();
    if (ret) {
        nva_printf("nva_read_right err\n");
        return NVA_ERROR;
    }
    ret = nva_read(nva_id, data_buf, buf_len);
    if (ret) {
        nva_printf("nva_read err\n");
        return NVA_ERROR;
    }
    return NVA_OK;
}

/*
 * 函数说明：nva或nv读取接口（底软）
 * 参数说明：item_id为待读取nva或nv的编号；data_buf为存储nva或nv数据的地址；buf_len为buf长度，通常为nva或nv结构体大小
 */
s32 bsp_nv_read(u32 item_id, u8 *data_buf, u32 buf_len)
{
    return bsp_nva_read(item_id, data_buf, buf_len);
}
EXPORT_SYMBOL_GPL(bsp_nv_read);

s32 nva_get_item_len(u32 nva_id)
{
    s32 ret;

    nva_item_info_s *item_info = NULL;

    ret = nva_search_item_info(nva_id, &item_info);
    if (ret) {
        nva_printf("can not find 0x%x !\n", nva_id);
        return 0;
    }
    return item_info->data_len;
}

s32 bsp_nva_item_debug(u32 nva_id)
{
    u8 *data_buf = NULL;
    s32 ret, i, len;

    len = nva_get_item_len(nva_id);
    if (len > NVA_MAX_LEN) {
        nva_printf("length = %d err\n", len);
        return NVA_ERROR;
    }

    data_buf = osl_malloc(len);
    if (data_buf == NULL) {
        nva_printf("malloc buff err\n");
        return NVA_ERROR;
    }
    ret = bsp_nva_read(nva_id, data_buf, len);
    if (ret) {
        nva_printf("nva read err\n");
        osl_free(data_buf);
        return NVA_ERROR;
    }
    nva_printf("nva item data:\n");
    for (i = 0; i < len; i++) {
        nva_printf("byte%d: 0x%x\n", i, data_buf[i]);
    }
    osl_free(data_buf);
    return NVA_OK;
}

EXPORT_SYMBOL_GPL(bsp_nva_item_debug);

s32 bsp_nva_write(u32 nva_id, void *pdata, u32 buf_len)
{
    s32 ret;
    nva_ctrl_info_s *nvainfo = nva_get_nvainfo();
    nva_item_info_s *item_info = NULL;
    void *item_addr = NULL;

    if ((pdata == NULL) || (buf_len == 0)) {
        return NVA_ERROR;
    }

    ret = nva_read_right();
    if (ret) {
        nva_printf("nva_read_right err\n");
        return NVA_ERROR;
    }

    ret = nva_search_item_info(nva_id, &item_info);
    if (ret) {
        nva_printf("can not find 0x%x !\n", nva_id);
        return NVA_ERROR;
    }

    if (buf_len > item_info->data_len) {
        nva_printf("len overflow, nvid=0x%x, buf_len=0x%x, item_data_len=0x%x\n", nva_id, buf_len, item_info->data_len);
        return NVA_ERROR;
    }

    item_addr = (void *)(uintptr_t)((uintptr_t)nvainfo->load_addr + item_info->base_addr);

    osl_sem_down(&nvainfo->rw_sem);
    ret = memcpy_s(item_addr, buf_len, pdata, buf_len);
    if (ret) {
        nva_printf("write nva to memory failed nvid=0x%x 0x%x!\n", nva_id, ret);
        osl_sem_up(&nvainfo->rw_sem);
        return NVA_ERROR;
    }
    if (nva_img_flush()) {
        nva_printf("flush img err\n");
        osl_sem_up(&nvainfo->rw_sem);
        return NVA_ERROR;
    }
    osl_sem_up(&nvainfo->rw_sem);
    return NVA_OK;
}

nva_item_info_s *nva_get_item(u32 nva_id)
{
    nva_item_info_s *item_info = NULL;
    if (nva_read_right()) {
        nva_printf("no write right\n");
        return NULL;
    }
    if (nva_search_item_info(nva_id, &item_info)) {
        nva_printf("can not find 0x%x !\n", nva_id);
        return NULL;
    }
    return item_info;
}

/*
 * nva调测写内存，kernel更新数据到存储分区
 */
s32 bsp_nva_write_debug(u32 nva_id, u32 type, u32 offset, u32 data)
{
    s32 ret;
    u8 data_u8 = (u8)data;
    u16 data_u16 = (u16)data;
    u32 data_u32 = (u32)data;
    u64 data_addr;
    nva_ctrl_info_s *nvainfo = nva_get_nvainfo();
    nva_item_info_s *item_info = NULL;

    item_info = nva_get_item(nva_id);
    if (item_info == NULL) {
        return NVA_ERROR;
    }
    if (offset > item_info->data_len) {
        nva_printf("input offset err\n");
        return NVA_ERROR;
    }

    data_addr = (uintptr_t)nvainfo->load_addr + item_info->base_addr + offset;

    osl_sem_down(&nvainfo->rw_sem);
    switch (type) {
        case TYPE_U8:
            ret = memcpy_s((void *)(uintptr_t)data_addr, (item_info->data_len - offset), &data_u8, sizeof(data_u8));
            break;
        case TYPE_U16:
            ret = memcpy_s((void *)(uintptr_t)data_addr, (item_info->data_len - offset), &data_u16, sizeof(data_u16));
            break;
        case TYPE_U32:
            ret = memcpy_s((void *)(uintptr_t)data_addr, (item_info->data_len - offset), &data_u32, sizeof(data_u32));
            break;
        default:
            nva_printf("input type err\n");
            osl_sem_up(&nvainfo->rw_sem);
            return NVA_ERROR;
    }
    if (ret != EOK) {
        nva_printf("memcpy err\n");
        osl_sem_up(&nvainfo->rw_sem);
        return NVA_ERROR;
    }
    if (nva_img_flush()) {
        osl_sem_up(&nvainfo->rw_sem);
        return NVA_ERROR;
    }
    osl_sem_up(&nvainfo->rw_sem);
    ret = bsp_nva_item_debug(nva_id);
    return ret;
}
EXPORT_SYMBOL_GPL(bsp_nva_write_debug);
#endif
