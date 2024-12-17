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

#include <linux/module.h>
#include <linux/export.h>
#include <linux/crc16.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <mdrv_bmi_oeminfo.h>
#include <mdrv_bmi_blk.h>
#include <oeminfo_core.h>
#include <bsp_blk.h>
#include <securec.h>
#include <osl_sem.h>
#include "oeminfo_cfg/oeminfo_cfg.h"
#include <bsp_sysboot.h>

#define OEMINFO_PART_NAME "oeminfo"

struct oeminfo_data_info {
    u32 is_dirty;
    u32 inited;
    osl_sem_id oeminfo_sem;
    struct oeminfo_data_node data;
};

static struct oeminfo_cfg_info *g_oeminfo_cfg = NULL;
static struct oeminfo_data_info g_oeminfo_data_info = {0};

static int oeminfo_para_check(enum oeminfo_mgr id, u32 offset,const void *buf, u32 buf_len)
{
    if (g_oeminfo_data_info.inited == 0) {
        oeminfo_log("oeminfo is not inited\n");
        return OEMINFO_NOT_INITED;
    }

    if (id < OEMINFO_COMM_MGR || id >= OEMINFO_MGR_MAX) {
        oeminfo_log("oeminfo manager id(%d) err\n", id);
        return OEMINFO_PARA_ERR;
    }

    if (offset >= g_oeminfo_cfg[id].size || buf_len >= g_oeminfo_cfg[id].size || (offset + buf_len) >= g_oeminfo_cfg[id].size) {
        oeminfo_log("oeminfo offset(%d) or buf_len(%d) err, max size is %d\n", offset, buf_len, g_oeminfo_cfg[id].size);
        return OEMINFO_PARA_ERR;
    }

    if (buf == NULL) {
        oeminfo_log("buf is NULL\n");
        return OEMINFO_PARA_ERR;
    }

    return OEMINFO_OK;
}

static int oeminfo_crc_check(enum oeminfo_mgr id, u32 offset)
{
    u32 base_addr = g_oeminfo_cfg[id].base_addr;
    struct oeminfo_data_head *head = (struct oeminfo_data_head *)&(g_oeminfo_data_info.data.buf[base_addr + offset]);
    unsigned char *data_offset = &(g_oeminfo_data_info.data.buf[base_addr + offset]) + sizeof(struct oeminfo_data_head);
    u16 crc_value;

    if (head->total_bytes >= g_oeminfo_cfg[id].size || head->total_bytes == 0) {
        oeminfo_log("data err\n");
        return OEMINFO_DATA_ERR;
    }
    crc_value = crc16(0, data_offset, head->total_bytes);
    if (crc_value != (u16)head->crc) {
        oeminfo_log("crc check failed\n");
        return OEMINFO_CRC_ERR;
    }

    return OEMINFO_OK;
}

static inline int oeminfo_get_data(enum oeminfo_mgr id, u32 offset, void *buf, u32 buf_len)
{
    int ret;
    u32 base_addr = g_oeminfo_cfg[id].base_addr;
    struct oeminfo_data_head *head = (struct oeminfo_data_head *)&(g_oeminfo_data_info.data.buf[base_addr + offset]);
    unsigned char *data_offset = &(g_oeminfo_data_info.data.buf[base_addr + offset]) + sizeof(struct oeminfo_data_head);
    u32 len = (head->total_bytes > buf_len) ? buf_len : head->total_bytes;

    ret = memcpy_s(buf, buf_len, data_offset, len);
    if (ret) {
        oeminfo_log("get data err, ret = 0x%x\n", ret);
        return ret;
    }
    return OEMINFO_OK;
}

static int oeminfo_update_data(enum oeminfo_mgr id, u32 offset, const void *buf, u32 buf_len, u16 crc)
{
    int ret;
    u32 base_addr;
    struct oeminfo_data_head *head = NULL;
    unsigned char *data_offset = NULL;

    base_addr = g_oeminfo_cfg[id].base_addr;
    head = (struct oeminfo_data_head *)&(g_oeminfo_data_info.data.buf[base_addr + offset]);
    data_offset = &(g_oeminfo_data_info.data.buf[base_addr + offset]) + sizeof(struct oeminfo_data_head);
    ret = memcpy_s(data_offset, buf_len, buf, buf_len);
    if (ret) {
        oeminfo_log("update data err, ret = 0x%x\n", ret);
        return ret;
    }
    head->crc = crc;
    head->total_bytes = buf_len;
    g_oeminfo_data_info.is_dirty = 1;

    return ret;
}

static int oeminfo_flush_data(enum oeminfo_mgr id, u32 offset)
{
    int ret;
    u32 base_addr = g_oeminfo_cfg[id].base_addr;
    struct oeminfo_data_head *head = (struct oeminfo_data_head *)&(g_oeminfo_data_info.data.buf[base_addr + offset]);

    ret = bsp_blk_logic_write(OEMINFO_PART_NAME, (u64)(base_addr + offset), &g_oeminfo_data_info.data.buf[base_addr + offset], 
                            (u64)(head->total_bytes + sizeof(struct oeminfo_data_head)));
    if (ret) {
        oeminfo_log("flush data err, ret = 0x%x\n", ret);
        return ret;
    }
    return OEMINFO_OK;
}

int oeminfo_read(enum oeminfo_mgr id, u32 offset, void *buf, u32 buf_len)
{
    int ret = oeminfo_para_check(id, offset, buf, buf_len);
    if (ret) {
        return ret;
    }

    osl_sem_down(&g_oeminfo_data_info.oeminfo_sem);
    ret = oeminfo_crc_check(id, offset);
    if (ret) {
        goto out;
    }

    ret = oeminfo_get_data(id, offset, buf, buf_len);

out:
    osl_sem_up(&g_oeminfo_data_info.oeminfo_sem);
    return ret;
}
EXPORT_SYMBOL_GPL(oeminfo_read);

static int oeminfo_write_to_ddr(enum oeminfo_mgr id, u32 offset, const void *buf, u32 buf_len)
{
    u32 crc = crc16(0, buf, buf_len);
    return oeminfo_update_data(id, offset, buf, buf_len, crc);
}

int oeminfo_write_async(enum oeminfo_mgr id, u32 offset, const void *buf, u32 buf_len)
{
    int ret = oeminfo_para_check(id, offset, buf, buf_len);
    if (ret) {
        return ret;
    }

    osl_sem_down(&g_oeminfo_data_info.oeminfo_sem);
    ret = oeminfo_write_to_ddr(id, offset, buf, buf_len);
    osl_sem_up(&g_oeminfo_data_info.oeminfo_sem);
    return ret;
}
EXPORT_SYMBOL_GPL(oeminfo_write_async);

int oeminfo_write_sync(enum oeminfo_mgr id, u32 offset, const void *buf, u32 buf_len)
{
    int ret = oeminfo_para_check(id, offset, buf, buf_len);
    if (ret) {
        return ret;
    }

    osl_sem_down(&g_oeminfo_data_info.oeminfo_sem);
    ret = oeminfo_write_to_ddr(id, offset, buf, buf_len);
    if (ret) {
        goto out;
    }

    ret = oeminfo_flush_data(id, offset);
out:
    osl_sem_up(&g_oeminfo_data_info.oeminfo_sem);
    return ret;
}
EXPORT_SYMBOL_GPL(oeminfo_write_sync);

int mdrv_oeminfo_sync(void)
{
    int ret = OEMINFO_OK;

    if (g_oeminfo_data_info.inited == 0) {
        oeminfo_log("oeminfo is not inited\n");
        return OEMINFO_NOT_INITED;
    }

    osl_sem_down(&g_oeminfo_data_info.oeminfo_sem);
    if (g_oeminfo_data_info.is_dirty == 0) {
        goto out;
    }
    ret = bsp_blk_logic_write(OEMINFO_PART_NAME, 0, g_oeminfo_data_info.data.buf, (u64)g_oeminfo_data_info.data.size);
    if (ret) {
        oeminfo_log("sync oeminfo data err, ret = 0x%x\n", ret);
        goto out;
    }
    g_oeminfo_data_info.is_dirty = 0;

out:
    osl_sem_up(&g_oeminfo_data_info.oeminfo_sem);
    return OEMINFO_OK;
}
EXPORT_SYMBOL_GPL(mdrv_oeminfo_sync);

int oeminfo_get_unit_size(enum oeminfo_mgr id)
{
    if (id >= OEMINFO_MGR_MAX) {
        oeminfo_log("manager id(%d) err\n", id);
        return OEMINFO_PARA_ERR;
    }
    return (int)g_oeminfo_cfg[id].data_unit_size;
}
EXPORT_SYMBOL_GPL(oeminfo_get_unit_size);

int oeminfo_get_data_node(struct oeminfo_data_node *data)
{
    u32 i;
    u32 total_size = 0;
    for (i = 0; i < OEMINFO_MGR_MAX; i++) {
        total_size += g_oeminfo_cfg[i].size;
    }

    data->buf = kzalloc(total_size, GFP_KERNEL);
    if (data->buf == NULL) {
        oeminfo_log("malloc oeminfo data failed\n");
        return OEMINFO_MALLOC_MEM_ERR;
    }
    data->size = total_size;
    return 0;
}

int oeminfo_init(void)
{
    int ret;

    if (g_oeminfo_data_info.inited == 1) {
        oeminfo_log("oeminfo is inited\n");
        return 0;
    }

    g_oeminfo_cfg = oeminfo_get_cfg_info();
    ret = oeminfo_get_data_node(&g_oeminfo_data_info.data);
    if (ret) {
        return ret;
    }
    ret = bsp_blk_logic_read(OEMINFO_PART_NAME, 0, g_oeminfo_data_info.data.buf, (u64)g_oeminfo_data_info.data.size);
    if (ret) {
        if (ret == BLK_LOGIC_NO_VALID_DATA) {
            ret = 0;
        } else {
            oeminfo_log("read oeminfo err, ret = 0x%x\n", ret);
            return ret;
        }
    }

    osl_sem_init(1, &g_oeminfo_data_info.oeminfo_sem);
    g_oeminfo_data_info.inited = 1;

    return ret;
}

static ssize_t oeminfo_init_status_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    (void)oeminfo_init();
    return (ssize_t)count;
}

static DEVICE_ATTR(init, S_IWUSR, NULL, oeminfo_init_status_write);

static struct platform_device g_oeminfo_device = {
    .name = OEMINFO_PART_NAME,
    .id = -1,
};

int oeminfo_module_init(void)
{
    int ret;
    ret = platform_device_register(&g_oeminfo_device);
    if (ret) {
        oeminfo_log("register oeminfo device failed.\n");
        return ret;
    }

    ret = device_create_file(&g_oeminfo_device.dev, &dev_attr_init);
    if (ret) {
        oeminfo_log("fail to create init node, ret = %d\n", ret);
        return ret;
    }
    return 0;
}
module_init(oeminfo_module_init);

