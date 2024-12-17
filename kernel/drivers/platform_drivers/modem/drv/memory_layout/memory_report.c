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

#include <linux/mm.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/fcntl.h>
#include <linux/spinlock.h>
#include <linux/syscalls.h>
#include <../fs/internal.h>
#include <mdrv_memory_layout.h>
#include <bsp_dra.h>
#include <bsp_msg.h>
#include <bsp_print.h>
#include <bsp_onoff.h>
#include <msg_id.h>
#include <osl_types.h>
#include "memory_layout.h"
#include <securec.h>
#include <product_config.h>

#define COOKIE_NUM_MAX 4000
#define DYNMEM_DUMP_SIZE 0xC000
#define DYNMEM_REPORT_DEBUG_INFO_PTNO_MAX 2
#define DYNMEM_REPORT_BUFFER_LEN 100
/* save acore total size */
#define DYNMEM_REPORT_ACORE_FILE "/data/vendor/log/dfx_logs/modem_log/drv/mem/acore.txt"
typedef struct {
    unsigned int type; /* 分区类型：0-cache,1-uncache */
    unsigned int size; /* 内存申请大小 */
    unsigned int cookie; /* 内存申请cookie信息 */
} dynmem_report_info_s;

typedef struct {
    unsigned int ap_used;
    unsigned int ap_available;
    unsigned int ap_maa_alloc;
    unsigned int aslr_offset;
    unsigned int total_size[DYNMEM_REPORT_DEBUG_INFO_PTNO_MAX];
    unsigned int free_size[DYNMEM_REPORT_DEBUG_INFO_PTNO_MAX];
    unsigned int mem_count;
    dynmem_report_info_s mem_info[COOKIE_NUM_MAX];
} dynmem_detail_dump_s;
dynmem_detail_dump_s *g_dynmem_dump_field = NULL;
static msg_chn_t g_acore_mem_msg_chn;

unsigned int get_acore_available_size(void)
{
    struct sysinfo si;
    unsigned long long available;
    si_meminfo(&si);
    available = (unsigned long long)(si_mem_available() * si.mem_unit);
    return (unsigned int)available;
}

#ifndef BSP_CONFIG_PHONE_TYPE
unsigned int get_acore_total_size(void)
{
    unsigned int total_size = 0;
    unsigned long pa;
    unsigned int size;
    int i = 0;
    char *name[] = {"acore_ddr", "acore_dts_ddr", "secos_ddr", "bl31_ddr", NULL};
    while(name[i] != NULL) {
        pa = mdrv_mem_region_get(name[i], &size);
        if (pa != 0 && size != 0) {
            total_size += size;
        }
        i++;
    }
    return total_size;
}
#endif

static void mem_drop_pagecache(void)
{
#ifndef BSP_CONFIG_PHONE_TYPE
    char *value = "3";
    struct file *filp = NULL;
    unsigned long fs;
    loff_t pos = 0;

    fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);
    filp = filp_open("/proc/sys/vm/drop_caches", O_WRONLY, 0);
    if (IS_ERR_OR_NULL(filp)) {
        set_fs(fs);
        bsp_err("fail to open file %s!\n", "/proc/sys/vm/drop_caches");
        return;
    }
    (void)kernel_write(filp, value, strlen(value), &pos);
    (void)vfs_fsync(filp, 0);
    (void)filp_close(filp, NULL);
    set_fs(fs);
#endif
}

static void acore_drop_cache(void)
{
#ifndef BSP_CONFIG_PHONE_TYPE
    ksys_sync();
    mem_drop_pagecache();
    drop_slab();
#endif
}

#ifndef BSP_CONFIG_PHONE_TYPE
static void save_acore_meminfo(void)
{
    int ret;
    char buffer[DYNMEM_REPORT_BUFFER_LEN + 1] = {0};
    struct file *filp = NULL;
    unsigned long fs;
    loff_t pos = 0;

    ret = snprintf_s(buffer, (DYNMEM_REPORT_BUFFER_LEN + 1), DYNMEM_REPORT_BUFFER_LEN,
        "ACORE DRV %u\nACORE OPENCPU %u\nACORE USER_PLANE %u\n", g_dynmem_dump_field->ap_used,
        g_dynmem_dump_field->ap_available, g_dynmem_dump_field->ap_maa_alloc);
    if (ret < 0) {
        bsp_err("snprintf_s fail!\n");
        return;
    }
    fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);
    filp = filp_open(DYNMEM_REPORT_ACORE_FILE, O_CREAT | O_RDWR, 0);
    set_fs(fs);
    if (IS_ERR_OR_NULL(filp)) {
        bsp_err("fail to open file %s!\n", DYNMEM_REPORT_ACORE_FILE);
        return;
    }
    set_fs((mm_segment_t)KERNEL_DS);
    (void)kernel_write(filp, buffer, strlen(buffer), &pos);
    (void)vfs_fsync(filp, 0);
    (void)filp_close(filp, NULL);
    set_fs(fs);
}
#else
static void save_acore_meminfo(void)
{
    int ret;
    char buffer[DYNMEM_REPORT_BUFFER_LEN + 1] = {0};
    struct msg_addr dst;
    ret = snprintf_s(buffer, (DYNMEM_REPORT_BUFFER_LEN + 1), DYNMEM_REPORT_BUFFER_LEN,
        "ACORE DRV %u\nACORE OPENCPU %u\nACORE USER_PLANE %u\n", g_dynmem_dump_field->ap_used,
        g_dynmem_dump_field->ap_available, g_dynmem_dump_field->ap_maa_alloc);
    if (ret < 0) {
        bsp_err("snprintf_s fail!\n");
        return;
    }
    dst.core = MSG_CORE_TSP;
    dst.chnid = MSG_CHN_DMSS_AUTO;
    ret = bsp_msg_lite_sendto(g_acore_mem_msg_chn, &dst, buffer, strlen(buffer));
     if (ret) {
        bsp_err("send acore info to tsp fail!\n");
    }
}
#endif

int memory_acore_msg_handler(const struct msg_addr *src, void *buf, unsigned int len)
{
    if (g_dynmem_dump_field == NULL) {
        return -1;
    }
    acore_drop_cache();
    g_dynmem_dump_field->ap_available = get_acore_available_size();
    g_dynmem_dump_field->ap_maa_alloc = bsp_dra_get_cur_mem();
#ifdef BSP_CONFIG_PHONE_TYPE
    g_dynmem_dump_field->ap_used = 0;
#else
    g_dynmem_dump_field->ap_used = get_acore_total_size() - g_dynmem_dump_field->ap_available -
        g_dynmem_dump_field->ap_maa_alloc;
#endif
    save_acore_meminfo();
    return 0;
}

void bsp_dynmem_field_init(void)
{
    unsigned long pa;
    unsigned int size;
    unsigned int phy_addr;

    pa = mdrv_mem_region_get("mntn_ddr", &size);
    if (pa == 0 || size == 0) {
        bsp_err("get mntn_ddr addr fail!\n");
        return;
    }
    phy_addr = pa + size - DYNMEM_DUMP_SIZE;
    g_dynmem_dump_field = (dynmem_detail_dump_s *)ioremap_wc(phy_addr, DYNMEM_DUMP_SIZE);
    if (g_dynmem_dump_field == NULL) {
        bsp_err("dynmem_dump ioremap_wc error\n");
    }
}

int acore_mem_msg_init(void)
{
    int ret;
    struct msgchn_attr lite_attr = { 0 };

    bsp_msgchn_attr_init(&lite_attr);
    /* 复用DMSS自动化测试用例通道，自动化测试不涉及ccore发消息给acore */
    lite_attr.chnid = MSG_CHN_DMSS_AUTO;
    lite_attr.coremask = MSG_CORE_MASK(MSG_CORE_TSP);
    lite_attr.lite_notify = memory_acore_msg_handler;
    ret = bsp_msg_lite_open(&g_acore_mem_msg_chn, &lite_attr);
    return ret;
}

int bsp_memory_report_init(void)
{
    acore_mem_msg_init();
    bsp_dynmem_field_init();
    return OK;
}
#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(bsp_memory_report_init);
#endif
