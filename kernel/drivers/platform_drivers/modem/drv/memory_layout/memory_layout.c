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

#include "memory_layout.h"
#include <linux/module.h>
#include <linux/version.h>
#include <mdrv_memory.h>
#include <product_config.h>
#include <bsp_sysboot.h>
#include <bsp_dt.h>
#include <securec.h>

mem_layout g_mem_layout_info;

#if (defined BSP_CONFIG_PHONE_TYPE) && (!defined CONFIG_TZDRIVER)
static int mem_layout_node_parse(device_node_s *dev_node, unsigned int *num, unsigned long base_addr)
{
    const char* mem_name = NULL;
    unsigned int mem_size;
    mem_node_info *mem_info = g_mem_layout_info.memory_info + *num;
    unsigned int max_rec = g_mem_layout_info.size / sizeof(mem_node_info);
    unsigned int ddr_offset = 0;
    device_node_s *child_node = NULL;

    bsp_dt_for_each_child_of_node(dev_node, child_node) {
        if (bsp_dt_property_read_string(child_node, (const char*)"mem-name", &mem_name)) {
            MEM_ERR("[memory_layout]fail to read mem-name!\n");
            return ERROR;
        }
        if (bsp_dt_property_read_u32(child_node, (const char*)"size", &mem_size)) {
            MEM_ERR("[memory_layout]mem_size get failed!\n");
            return ERROR;
        }
        mem_info->addr = ddr_offset + base_addr;
        mem_info->size = mem_size;
        ddr_offset += mem_size;
        if (strcpy_s(mem_info->name, sizeof(mem_info->name), (void*)(mem_name))) {
            MEM_ERR("[memory_layout]mem_name strcpy failed!\n");
            return ERROR;
        }
        (*num)++;
        if (*num >= max_rec) {
            MEM_ERR("[memory_layout]node num overflowed!\n");
            return ERROR;
        }

        mem_info = g_mem_layout_info.memory_info + *num;
    }

    return OK;
}

static int meminfo_of_parse(void)
{
    unsigned int num = 0;
    int ret;
    unsigned int mem_addr = 0;
    device_node_s *dev_node = NULL;
    device_node_s *child_node = NULL;

    dev_node = bsp_dt_find_compatible_node(NULL, NULL, "mem_layout_modem");
    if (dev_node == NULL) {
        MEM_ERR("[memory_layout]fail to get mem_layout_modem dts!\n");
        return ERROR;
    }
    bsp_dt_for_each_child_of_node(dev_node, child_node) {
        if (bsp_dt_property_read_u32_array(child_node, "addr", &mem_addr, 1)) {
            MEM_ERR("[memory_layout]mem_addr get failed!\n");
            return ERROR;
        }
        ret = mem_layout_node_parse(child_node, &num, (unsigned long)mem_addr);
        if (ret != OK) {
            MEM_ERR("[memory_layout]mem_layout_node_parse fail!\n");
            return ret;
        }
    }
    return OK;
}
static void set_dump_base(void)
{
    unsigned long addr = 0;
    void *virt_addr = NULL;
    unsigned int size = 0;
    virt_addr = mdrv_mem_share_get("nsroshm_dump_base", &addr, &size, SHM_NSRO);
    if (virt_addr == NULL) {
        MEM_ERR("get nsroshm_dump_base error\n");
        return;
    }
    *((unsigned long *)virt_addr) = DDR_MEM_ADDR;
    return;
}
#endif

unsigned long mdrv_mem_region_get(const char *name, unsigned int *size)
{
    mem_node_info *mem_info = NULL;
    unsigned int max_rec;
    unsigned long phy_addr;
    unsigned int i;

    if (name == NULL || size == NULL) {
        MEM_ERR("[memory_layout]input error!\n");
        return 0;
    }

    mem_info = g_mem_layout_info.memory_info;
    max_rec = g_mem_layout_info.size / sizeof(mem_node_info);
    for (i = 0; i < max_rec; i++) {
        if (strcmp((mem_info + i)->name, name) == 0) {
            phy_addr = (unsigned long)((mem_info + i)->addr);
            *size = (mem_info + i)->size;
            return phy_addr;
        }
    }
    *size = 0;
    MEM_ERR("[memory_layout]get memory %s error!\n", name);
    return 0;
}
EXPORT_SYMBOL_GPL(mdrv_mem_region_get);

int bsp_memory_layout_init(void)
{
    int ret;
    unsigned long addr;
    void *virt_addr = NULL;

    MEM_ERR("[memory_layout]init start\n");
#if (defined BSP_CONFIG_PHONE_TYPE)
    virt_addr = mdrv_mem_share_get("nsroshm_memory_layout", &addr, &g_mem_layout_info.size, SHM_NSRO);
#else
    addr = DDR_MEM_LAYOUT_ADDR;
    g_mem_layout_info.size = DDR_MEM_LAYOUT_SIZE;
    virt_addr = ioremap_wc(DDR_MEM_LAYOUT_ADDR, DDR_MEM_LAYOUT_SIZE);
#endif
    if (virt_addr == NULL) {
        MEM_ERR("[memory_layout]get virt_addr error\n");
        return ERROR;
    }
    ret = bsp_sysboot_set_modem_para(MODEM_MEM_LAYOUT_ADDR_OFFSET, (u32)addr);
    if (ret) {
        MEM_ERR("[memory_layout]set memory layout addr failed\n");
        return ERROR;
    }
    ret = bsp_sysboot_set_modem_para(MODEM_MEM_LAYOUT_SIZE_OFFSET, g_mem_layout_info.size);
    if (ret) {
        MEM_ERR("[memory_layout]set memory layout size failed\n");
        return ERROR;
    }
    g_mem_layout_info.memory_info = (mem_node_info *)(uintptr_t)virt_addr;
#if (defined BSP_CONFIG_PHONE_TYPE) && (!defined CONFIG_TZDRIVER)
    ret = meminfo_of_parse();
    if (ret) {
        MEM_ERR("[memory_layout]meminfo_of_parse failed.\n");
        return ERROR;
    }
    set_dump_base();
#endif
    MEM_ERR("[memory_layout]init ok\n");
    return OK;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
pure_initcall(bsp_memory_layout_init); /*lint !e528*/
#endif

