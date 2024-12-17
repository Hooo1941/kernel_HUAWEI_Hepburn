/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
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

/*lint --e{528,537,715} */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/syscalls.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/rtc.h>
#include <linux/freezer.h>
#include <linux/version.h>
#include <asm/system_misc.h>
#include <product_config.h>
#include <mdrv_sysboot.h>
#include <mdrv_chg.h>
#include <mdrv_errno.h>
#include <bsp_dump.h>
#include <bsp_onoff.h>
#include <securec.h>
#include <bsp_print.h>
#include <bsp_sysctrl.h>
#include "power_exchange.h"
#include "mdrv_chg.h"
#include "onoff_msg.h"
#include "power_off_mbb.h"
#include "power_hal.h"
#include "bsp_modem_boot.h"
#include "power_para.h"

#define THIS_MODU mod_onoff
drv_start_mode_e mdrv_sysboot_get_bootmode(void)
{
    return 0;
}

power_on_start_reason_e mdrv_sysboot_get_power_on_reason(void)
{
    return 0;
}

int mdrv_sysboot_get_power_on_time(struct rtc_time *time)
{
    return 0;
}

EXPORT_SYMBOL_GPL(mdrv_sysboot_get_power_on_reason);
EXPORT_SYMBOL_GPL(mdrv_sysboot_get_bootmode);
EXPORT_SYMBOL_GPL(mdrv_sysboot_get_power_on_time);

/*
 * 功能描述: 用于获取开机模式
 */
int bsp_start_mode_get(void)
{
    return DRV_START_MODE_NORMAL;
}

#ifndef BSP_CONFIG_PHONE_TYPE
sysboot_boot_mode_e bsp_boot_mode_get(void)
{
    u32 i;
    int boot_mode;
    u32 size = 0;
    static sysboot_boot_mode_e logic_mode = SYSBOOT_BOOT_MODE_ERROR;
    struct boot_mode_map *map = boot_mode_map_get(&size);
    struct power_cfg *cfg = power_get_config();
    if (cfg == NULL) {
        bsp_err("power cfg info is null\n");
        return SYSBOOT_BOOT_MODE_ERROR;
    }

    if (logic_mode != SYSBOOT_BOOT_MODE_ERROR) {
        return logic_mode;
    }

    boot_mode = (int)(readl(bsp_sysctrl_addr_get((void*)(uintptr_t)cfg->boot_mode_addr_base)) & cfg->boot_mode_mask);
    for (i = 0; i < size; i++) {
        if (boot_mode == map->phy_mode) {
            logic_mode = map->logic_mode;
            return logic_mode;
        }
        map++;
    }

    bsp_err("phy boot mode is %d\n", boot_mode);
    return SYSBOOT_BOOT_MODE_ERROR;
}
#else
sysboot_boot_mode_e bsp_boot_mode_get(void)
{
    return SYSBOOT_BOOT_MODE_STUB;
}
#endif
EXPORT_SYMBOL_GPL(bsp_boot_mode_get);

sysboot_boot_mode_e mdrv_sysboot_get_boot_mode(void)
{
    return bsp_boot_mode_get();
}
EXPORT_SYMBOL_GPL(mdrv_sysboot_get_boot_mode);

int modem_power_on_init(void)
{
    int ret;
    bsp_debug("modem power on init start\n");
    ret = onoff_msg_init();
    if (ret == 0) {
        bsp_debug("modem power on init end\n");
    }

    return ret;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
late_initcall(modem_power_on_init);
#endif
