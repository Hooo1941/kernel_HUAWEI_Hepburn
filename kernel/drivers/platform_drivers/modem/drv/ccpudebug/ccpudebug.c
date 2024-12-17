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
 */

/*lint --e{537}*/
/*lint -save -e958*/
#include <product_config.h>

#include <osl_types.h>
#include <osl_cache.h>
#include <bsp_print.h>
#include <bsp_version.h>
#include <bsp_dt.h>
#include <mdrv_memory.h>
#include <linux/module.h>
#include <securec.h>

#undef THIS_MODU
#define THIS_MODU mod_ccpudebug

#define MAX_DEBUG_REGS 4 /* 4 dss max */

struct debug_reg_config {
    u32 addr;
    u32 mask;
    u32 value;
    void *vaddr;
};

struct debug_reg_config g_debug_reg_config[MAX_DEBUG_REGS];
struct debug_reg_config g_ccpujtag_reg_config[MAX_DEBUG_REGS];

u32 g_debug_inited;
u32 g_debug_on;

void debug_cfg_reg(void)
{
    int i;
    u32 reg_value;

    for (i = 0; i < MAX_DEBUG_REGS; i++) {
        if (g_debug_reg_config[i].addr == 0) {
            return;
        }
        if (g_debug_reg_config[i].vaddr == NULL) {
            bsp_err("g_debug_reg_config io_remap failed\n");
            return;
        }
        reg_value = readl(g_debug_reg_config[i].vaddr);
        reg_value = reg_value & (~(g_debug_reg_config[i].mask));
        reg_value = reg_value | g_debug_reg_config[i].value;
        writel(reg_value, g_debug_reg_config[i].vaddr);
    }
}

void tsp_debug_mode_set(void)
{
    if (!g_debug_inited) {
        return;
    }

    debug_cfg_reg();
}

void bsp_debug_cfg_init(void)
{
    if (!g_debug_on || !g_debug_inited) {
        return;
    }

    debug_cfg_reg();
}

int ccpu_debug_probe(void)
{
    device_node_s *node = NULL;
    device_node_s *child_node = NULL;
    int ret;
    int i = 0;

    node = bsp_dt_find_node_by_path("/ccpu_debug_on_reset");
    if (node == NULL) { /* 找不到节点 */
        bsp_err("can't find ccpu_debug_on_reset node\n");
        return BSP_ERROR;
    }
    /* 读取需要初始化的寄存器和配置信息 */
    bsp_dt_for_each_child_of_node(node, child_node)
    {
        if (i >= MAX_DEBUG_REGS) {
            bsp_err("ccpu_debug_on_reset reg_config node num overflow, check MAX_DEBUG_REGS\n");
            return BSP_ERROR;
        }
        ret = (int)bsp_dt_property_read_u32_array(child_node, "reg_config", (u32 *)&g_debug_reg_config[i], 0x3);
        if (ret) {
            bsp_err("ccpu_debug_on_reset reg_config node not found, ret=0x%x\n", ret);
            return BSP_ERROR;
        }
        g_debug_reg_config[i].vaddr = ioremap(g_debug_reg_config[i].addr, 0x4);
        bsp_info("[%d]<0x%x 0x%x 0x%x>\n", i, g_debug_reg_config[i].addr, g_debug_reg_config[i].mask, 
                 g_debug_reg_config[i].value);
        i++;
    }
    return BSP_OK;
}

int debug_on_reset_init(void)
{
    u32 *cfg_addr = NULL;
    u32 shm_size = 0;
    phy_addr_t phy_addr = 0;
    int ret;
    const bsp_version_info_s *version_info = bsp_get_version_info();

    ret = ccpu_debug_probe();
    if (ret == BSP_OK) {
        g_debug_inited = 1;
    }

    if (version_info == NULL) {
        bsp_err("bsp_get_version_info fail.");
        return BSP_ERROR;
    }
    if (version_info->product_type == PRODUCT_PHONE) {
        return BSP_OK;
    }

    cfg_addr = (u32 *)mdrv_mem_share_get("nsroshm_debug_on_reset", &phy_addr, &shm_size, SHM_NSRO);
    if (cfg_addr == NULL || shm_size == 0) {
        bsp_err("bsp_get_debug_status fail: get shared mem err!\n");
        return BSP_ERROR;
    }

    g_debug_on = *cfg_addr;
    if (!g_debug_on) {
        return BSP_OK;
    }

    bsp_debug_cfg_init();
    return BSP_OK;
}

int ccpudebug_init(void)
{
    int ret;
    ret = debug_on_reset_init();
    return ret;
}


#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
late_initcall(ccpudebug_init); /*lint -restore*/
#endif

