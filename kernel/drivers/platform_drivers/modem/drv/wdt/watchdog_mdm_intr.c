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

#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <mdrv_errno.h>
#include "bsp_dt.h"
#include "bsp_sysctrl.h"
#include "bsp_wdt.h"
#include "watchdog_mdm_intr.h"

#undef THIS_MODU
#define THIS_MODU mod_wdt

#define MDM_WDT_NUM (16)

enum {
    WDOG_MOD_ID_NONE = 0x0,
    WDOG_MOD_ID_EXIST = 0x1,
};

struct mdm_wdt_int_reg {
    unsigned int start_bit;
    unsigned int end_bit;
    unsigned int mask_bit;
};

struct mdm_wdt_info {
    unsigned int irq[MDM_WDT_NUM];
    unsigned int total;
    unsigned long long *int_state_addr;
    struct mdm_wdt_int_reg int_state_mask;
    unsigned int mod_id_flag;
    unsigned int thread_num;
    unsigned int mod_id[MDM_WDT_NUM];
};

unsigned int g_wdt_int_no = 0;
static struct mdm_wdt_info g_mdm_wdt;
wdt_timeout_cb g_wdt_reboot_func = NULL;

static unsigned int watchdog_get_mod_id(void)
{
    unsigned int i, val;
    unsigned int exc_core = 0;

    if (g_mdm_wdt.mod_id_flag != WDOG_MOD_ID_EXIST) {
        return DRV_ERRNO_DUMP_CP_WDT;
    }

    val = g_mdm_wdt.int_state_mask.mask_bit & readl(g_mdm_wdt.int_state_addr);
    val = val >> g_mdm_wdt.int_state_mask.start_bit;
    for (i = 0; i < g_mdm_wdt.thread_num; i++) {
        if (val & ((1UL) << i)) {
            exc_core = i;
            break;
        }
    }
    wdt_err("mdm int state:0x%x, exc_core: 0x%x\n", val, exc_core);

    return g_mdm_wdt.mod_id[exc_core];
}

static irqreturn_t watchdog_mdm_int(int irq, void *dev_id)
{
    unsigned int mode_id;

    disable_irq_nosync(g_wdt_int_no);

    mode_id = watchdog_get_mod_id();

    wdt_err("wdt irq:0x%x, mode_id:0x%x\n", irq, mode_id);
    if (g_wdt_reboot_func != NULL) {
        g_wdt_reboot_func(mode_id);
    }
    return IRQ_HANDLED;
}

void bsp_wdt_irq_disable(WDT_CORE_ID core_id)
{
    if (g_wdt_int_no && core_id == WDT_CCORE_ID) {
        disable_irq_nosync(g_wdt_int_no);
        wdt_err("stop mdm wdt\n");
    } else {
        wdt_err("stop err wdt\n");
    }
}

void bsp_wdt_irq_enable(WDT_CORE_ID core_id)
{
}

static void watchdog_mdm_dts_init(device_node_s *node)
{
    unsigned int int_state_addr;

    if (bsp_dt_property_read_u32(node, "mod_id_flag", &g_mdm_wdt.mod_id_flag)) {
        wdt_err("mod_id none, ignore\n");
        goto wdt_fail;
    }
    if (bsp_dt_property_read_u32(node, "thread_num", &g_mdm_wdt.thread_num)) {
        goto wdt_fail;
    }
    if (bsp_dt_property_read_u32_array(node, "mod_id", g_mdm_wdt.mod_id, g_mdm_wdt.thread_num)) {
        goto wdt_fail;
    }
    if (bsp_dt_property_read_u32(node, "int_state_addr", &int_state_addr)) {
        goto wdt_fail;
    }
    if (bsp_dt_property_read_u32_array(node, "int_state_mask", (u32 *)&g_mdm_wdt.int_state_mask, \
        sizeof(g_mdm_wdt.int_state_mask) / sizeof(unsigned int))) {
        goto wdt_fail;
    }

    g_mdm_wdt.int_state_addr = bsp_sysctrl_addr_get((void*)(uintptr_t)int_state_addr);
    return;

wdt_fail:
    g_mdm_wdt.mod_id_flag = WDOG_MOD_ID_NONE;
}

int bsp_mdm_wdt_init(wdt_timeout_cb func)
{
    unsigned int flag = 0;
    device_node_s *node = NULL;

    node = bsp_dt_find_compatible_node(NULL, NULL, "modem,watchdog_app");
    if (node == NULL) {
        return BSP_ERROR;
    }

    if (bsp_dt_property_read_u32(node, "flag", &flag)) {
        return BSP_ERROR;
    }
    if (flag) {
        g_wdt_int_no = (unsigned int)bsp_dt_irq_parse_and_map(node, 0);
        if (request_irq(g_wdt_int_no, watchdog_mdm_int, 0, "watchdog_mdm", NULL) != BSP_OK) {
            wdt_err("mdm watchdog int err\n");
            return BSP_ERROR;
        }
        enable_irq_wake(g_wdt_int_no);
    }

    watchdog_mdm_dts_init(node);

    g_wdt_reboot_func = func;
    return BSP_OK;
}

int bsp_wdt_register_hook(WDT_CORE_ID core_id, wdt_timeout_cb func)
{
    if (core_id == WDT_CCORE_ID) {
        return bsp_mdm_wdt_init(func);
    }
    wdt_err("wdt core id err %d\n", (unsigned int)core_id);
    return BSP_ERROR;
}

void bsp_wdt_reinit(void)
{
    if (g_wdt_reboot_func != NULL && g_wdt_int_no) {
        free_irq(g_wdt_int_no, NULL);
        if (request_irq(g_wdt_int_no, watchdog_mdm_int, 0, "watchdog_mdm", NULL) != BSP_OK) {
            wdt_err("mdm watchdog reint err\n");
        }
    }
}

