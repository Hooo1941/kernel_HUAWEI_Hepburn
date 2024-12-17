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

#include <securec.h>
#include <osl_types.h>
#include <asm/io.h>
#include <bsp_dump.h>
#include <bsp_dt.h>
#include <bsp_sysboot.h>
#include <mdrv_memory_layout.h>
#include <bsp_print.h>
#include "syslog_logmem.h"
#include "syslog_print.h"


#undef THIS_MODU
#define THIS_MODU mod_print

syslog_logmem_ctrl_info_s g_syslog_logmem_ctrl;

static inline bool syslog_logmem_check_head_info(syslog_logmem_head_info_s *headinfo)
{
    if ((headinfo->magic1 != LOGMEM_MAGIC1) ||
        (headinfo->magic2 != LOGMEM_MAGIC2) ||
        (headinfo->magic3 != LOGMEM_MAGIC3) ||
        (headinfo->magic4 != LOGMEM_MAGIC4)) {
            return false;
    }
    return true;
}


int bsp_syslog_get_logmem_info(syslog_logmem_module_e type, syslog_logmem_area_info_s *info)
{
    syslog_logmem_head_info_s *headinfo = NULL;

    headinfo = g_syslog_logmem_ctrl.virt_addr;

    if (info == NULL || type >= LOGMEM_BUTT || headinfo == NULL) {
        return -1;
    }
    if (syslog_logmem_check_head_info(headinfo) != true) {
        return -1;
    }
    info->phy_addr = g_syslog_logmem_ctrl.phy_addr + headinfo->area[type].offset;
    info->length = headinfo->area[type].length;
    info->virt_addr = (void*)((uintptr_t)g_syslog_logmem_ctrl.virt_addr + headinfo->area[type].offset);
    return 0;
}

int syslog_logmem_reinit(void)
{
    device_node_s *dev_node = NULL;
    device_node_s *child_node = NULL;
    u32 index, offset, length;

    dev_node = bsp_dt_find_node_by_path("/logmem_layout");
    if (dev_node == NULL) {
        return -1;
    }
    g_syslog_logmem_ctrl.virt_addr->magic1 = LOGMEM_MAGIC1;
    g_syslog_logmem_ctrl.virt_addr->magic2 = LOGMEM_MAGIC2;
    bsp_dt_for_each_child_of_node(dev_node, child_node) {
        if (bsp_dt_property_read_u32(child_node, (const char*)"id", &index)) {
            bsp_err("id get failed!\n");
            continue;
        }
        if (index >= LOGMEM_BUTT) {
            continue;
        }

        if (bsp_dt_property_read_u32(child_node, (const char*)"offset", &offset)) {
            bsp_err("offset get failed!\n");
            continue;
        }
        if (offset >= g_syslog_logmem_ctrl.size) {
            continue;
        }
        if (bsp_dt_property_read_u32(child_node, (const char*)"length", &length)) {
            bsp_err("length get failed!\n");
            continue;
        }
        if (length >= g_syslog_logmem_ctrl.size || (offset + length) > g_syslog_logmem_ctrl.size) {
            continue;
        }
        g_syslog_logmem_ctrl.virt_addr->area[index].offset = offset;
        g_syslog_logmem_ctrl.virt_addr->area[index].length = length;
    }
    g_syslog_logmem_ctrl.virt_addr->magic3 = LOGMEM_MAGIC3;
    g_syslog_logmem_ctrl.virt_addr->magic4 = LOGMEM_MAGIC4;
    return 0;
}

int bsp_syslog_save_stage_log(syslog_logmem_module_e stage)
{
    int ret;
    syslog_logmem_area_info_s info;
    // 如果需要扩展保存的log阶段，按照入参的枚举的顺序在数组中添加文件名
    char *stage_name[LOGMEM_BUTT] = {NULL, NULL, NULL, "xloader.txt", "hiboot.txt", NULL, NULL, NULL, NULL};

    ret = bsp_syslog_get_logmem_info(stage, &info);
    if (ret < 0 || info.phy_addr == 0 || info.length == 0 || stage_name[stage] == NULL) {
        return -1;
    }
    ret = bsp_dump_log_save("/data/dfx_logs/modem_log/print/", stage_name[stage], (void *)(uintptr_t)info.virt_addr,
        (void *)(uintptr_t)info.phy_addr, info.length);
    if (ret != 0) {
        return -1;
    }
    return 0;
}

int syslog_set_modem_para(void)
{
    int ret;
    syslog_logmem_area_info_s info;
    ret = bsp_syslog_get_logmem_info(LOGMEM_CCPUBOOT, &info);
    if (ret < 0 || info.phy_addr == 0 || info.length == 0) {
        return -1;
    }

    if (memset_s(info.virt_addr, info.length, 0, info.length)) {
        bsp_err("memset ccpuboot area fail\n");
        return -1;
    }

    if (bsp_sysboot_set_modem_para(MODEM_LOGMEM_ADDR_OFFSET, info.phy_addr) != EOK) {
        bsp_err("set modem para LOGMEM_ADDR failed!\n");
        return -1;
    }
    if (bsp_sysboot_set_modem_para(MODEM_LOGMEM_SIZE_OFFSET, info.length) != EOK) {
        bsp_err("set modem para LOGMEM_SIZE failed!\n");
        return -1;
    }

    return 0;
}

void syslog_logmem_save_file(const char *dir_name)
{
    bsp_dump_log_save(dir_name, "logmem.bin", (void *)(uintptr_t)g_syslog_logmem_ctrl.virt_addr,
        (void *)(uintptr_t)g_syslog_logmem_ctrl.phy_addr, g_syslog_logmem_ctrl.size);
    bsp_err("save logmem.bin finished.\n");
}

int syslog_preset_ccore_logmem(void)
{
    int ret;
    struct log_mem *mem = NULL;
    syslog_logmem_area_info_s info;
    ret = bsp_syslog_get_logmem_info(LOGMEM_CCPU, &info);
    if (ret < 0 || info.virt_addr == NULL || info.length == 0) {
        return -1;
    }
    if (memset_s((void*)(uintptr_t)info.virt_addr, info.length, 0, info.length)) {
        return -1;
    }
    mem = (struct log_mem *)info.virt_addr;
    mem->size = info.length - sizeof(struct log_mem_stru);
    /* preset mtee */
    ret = bsp_syslog_get_logmem_info(LOGMEM_CCPU_TEE, &info);
    if (ret < 0 || info.virt_addr == NULL || info.length == 0) {
        bsp_err("no mtee logmem.\n");
        return 0;
    }
    if (memset_s((void*)(uintptr_t)info.virt_addr, info.length, 0, info.length)) {
        return -1;
    }
    mem = (struct log_mem *)info.virt_addr;
    mem->size = info.length - sizeof(struct log_mem_stru);
    return 0;
}

int syslog_logmem_init(void)
{
    unsigned long phy_addr;
    unsigned int size = 0;
    int ret;

    phy_addr = mdrv_mem_region_get("early_log_ddr", &size);
    if (phy_addr == 0 || size == 0) {
        bsp_err("get early log region failed!\n");
        return -1;
    }
    g_syslog_logmem_ctrl.phy_addr = phy_addr;
    g_syslog_logmem_ctrl.size = size;
    g_syslog_logmem_ctrl.virt_addr = (syslog_logmem_head_info_s *)((uintptr_t)ioremap_wc(phy_addr, size));
    if (g_syslog_logmem_ctrl.virt_addr == NULL) {
        return -1;
    }

#ifdef BSP_CONFIG_PHONE_TYPE
    if (g_syslog_logmem_ctrl.init_flag != true) {
#else
    if (syslog_logmem_check_head_info(g_syslog_logmem_ctrl.virt_addr) != true) {
#endif
        (void)memset_s(g_syslog_logmem_ctrl.virt_addr, size, 0, size);
        ret = syslog_logmem_reinit();
        if (ret < 0) {
            return -1;
        }
    }

    if (syslog_set_modem_para() != EOK) {
        return -1;
    }

    if (syslog_preset_ccore_logmem() != EOK) {
        bsp_err("syslog preset ccore logmem failed!\n");
        return -1;
    }
    print_register_last_kmsg();
    bsp_dump_register_log_notifier(DUMP_SAVE_LOGMEM, syslog_logmem_save_file);
    g_syslog_logmem_ctrl.init_flag = true;
    bsp_err("log mem init ok!\n");
    return 0;
}
#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
core_initcall_sync(syslog_logmem_init);
#endif
EXPORT_SYMBOL(syslog_logmem_save_file);
