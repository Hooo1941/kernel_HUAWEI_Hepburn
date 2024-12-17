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

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/string.h>
#include <linux/init.h>
#include "bsp_onoff.h"
#include <securec.h>
#include <bsp_print.h>
#include "power_para.h"

#define THIS_MODU mod_onoff
#define HEX_STRING_MAX 10
#define TRANSFER_BASE 16

struct sysboot_dpart_info g_sysboot_dpart_info = {
    .boot_part = BOOT_FROM_A_PART_FLAG,
};

static drv_charging_mode_e g_pd_charge_flag = NORMAL_CHARGING_MODE;
static drv_reset_type_e g_reset_mode_flag = NORMAL_RESET_FLAG;
static drv_soft_ver_type_e g_soft_ver_type = NORMAL_VERTYPE_FLAG;
static drv_run_mode_e g_soft_run_mode = RUN_MODE_NORMAL;
static sysboot_monitor_state_e g_sysboot_monitor_flag = SYSBOOT_MONITOR_DISABLED;

struct sysboot_dpart_info *sysboot_get_dpart_info(void)
{
    return &g_sysboot_dpart_info;
}

/*
 * 函 数 名  : early_parse_pdcharge_cmdline
 * 功能描述  : parse powerdown charge cmdline which is configured in hiboot
 * 输入参数  : char *cmd
 * 输出参数  : 无
 * 返 回 值  : ok/error
 * 调用函数  : pd_charge=0 or 1    1:pw charge 0:normal charge
 * 被调函数  :
 */
static int __init early_parse_pdcharge_cmdline(char *p)
{
    int ret;
    char pd_charge[HEX_STRING_MAX + 1] = {0};
    char *endptr = NULL;

    if (p == NULL) {
        bsp_debug("arg is null.\n");
        return -EINVAL;
    }

    ret = strcpy_s(pd_charge, HEX_STRING_MAX, p);
    if (ret != EOK) {
        bsp_debug("parse pdcharge memcpy fail.\n");
        return ret;
    }

    g_pd_charge_flag = simple_strtoull(pd_charge, &endptr, TRANSFER_BASE);
    bsp_debug("power down charge p:%s, g_pd_charge_flag :%d\n", p, g_pd_charge_flag);

    return 0;
}
early_param("pd_charge", early_parse_pdcharge_cmdline);

/*
 * 功能描述: 是否为正常开机版本
 */
drv_charging_mode_e get_pd_charge_flag(void)
{
    if (g_pd_charge_flag >= INVALID_CHARGING_MODE) {
        bsp_debug("invalid charging mode \n");
        return NORMAL_CHARGING_MODE;
    }

    return g_pd_charge_flag;
}
EXPORT_SYMBOL(get_pd_charge_flag);

/*
 * 函 数 名  : early_parse_softver_cmdline
 * 功能描述  : parse runmode cmdline which is configured in hiboot.
 * 输入参数  : char *p
 * 输出参数  : 无
 * 返 回 值  : ok/error
 * 调用函数  : swtype=0 or 1  1:factory 0:normal version
 * 被调函数  :
 */
static int __init early_parse_softver_cmdline(char *p)
{
    if (p == NULL) {
        bsp_debug("arg is null.\n");
        return -EINVAL;
    }

    if (!strcmp(p, "factory")) {
        g_soft_ver_type = FACTORY_VERTYPE_FLAG;
    }
    bsp_debug("softver is %s, softver_factory = %d\n", p, g_soft_ver_type);

    return 0;
}
early_param("androidboot.swtype", early_parse_softver_cmdline);

/*
 * 功能描述: 判断是否为工厂版本
 */
drv_soft_ver_type_e get_version_type(void)
{
    if (g_soft_ver_type >= INVALID_VERTYPE_FLAG) {
        bsp_err("running version is invalid\n");
        return INVALID_VERTYPE_FLAG;
    }

    return g_soft_ver_type;
}
EXPORT_SYMBOL(get_version_type);

/*
 * 函 数 名  : early_parse_reset_cmdline
 * 功能描述  : parse runmode cmdline which is configured in hiboot.
 * 输入参数  : char *p
 * 输出参数  : 无
 * 返 回 值  : ok/error
 * 调用函数  : reset_type=0 or 1  1:abnormal reset 0:normal reset
 * 被调函数  :
 */
static int __init early_parse_reset_cmdline(char *p)
{
    if (p == NULL) {
        bsp_debug("arg is null.\n");
        return -EINVAL;
    }

    if (!strcmp(p, "normal")) {
        g_reset_mode_flag = NORMAL_RESET_FLAG;
    } else if (!strcmp(p, "abnormal")) {
        g_reset_mode_flag = ABNORMAL_RESET_FLAG;
    }

    bsp_debug("reset type is %s, g_reset_mode_flag = %d\n", p, g_reset_mode_flag);

    return 0;
}
early_param("reset_type", early_parse_reset_cmdline);

static int __init early_parse_monitor_cmdline(char *p)
{
    if (p == NULL) {
        bsp_debug("arg is null.\n");
        return -EINVAL;
    }

    if (!strcmp(p, "enable")) {
        g_sysboot_monitor_flag = SYSBOOT_MONITOR_ENABLED;
    }

    bsp_info("sysboot monitor cmd is %s, flag = %d\n", p, g_sysboot_monitor_flag);

    return 0;
}
early_param("sysboot_monitor_cmd", early_parse_monitor_cmdline);

int sysboot_get_monitor_flag(void)
{
    return g_sysboot_monitor_flag;
}

/*
 * 功能描述: 判断是否为异常复位
 */
drv_reset_type_e get_reset_reason(void)
{
    if (g_reset_mode_flag >= INVALID_RESET_FLAG) {
        bsp_err("invalid reset flag\n");
        return NORMAL_RESET_FLAG;
    }

    return g_reset_mode_flag;
}
EXPORT_SYMBOL(get_reset_reason);

/*
 * 函 数 名  : early_parse_pwmode_cmdline
 * 功能描述  : parse runmode cmdline which is configured in hiboot.
 * 输入参数  : char *p
 * 输出参数  : 无
 * 返 回 值  : ok/error
 * 调用函数  : power_mode=0 or 1  0:normal 1:recovery
 * 被调函数  :
 */
static int __init early_parse_pwmode_cmdline(char *p)
{
    if (p == NULL) {
        bsp_debug("arg is null.\n");
        return -EINVAL;
    }

    if (!strcmp(p, "recovery")) {
        g_soft_run_mode = RUN_MODE_RECOVERY;
    } else if (!strcmp(p, "normal")) {
        g_soft_run_mode = RUN_MODE_NORMAL;
    } else {
        g_soft_run_mode = RUN_MODE_INVALID;
    }

    bsp_debug("run mode is %s, g_soft_run_mode = %d\n", p, g_soft_run_mode);
    return 0;
}
early_param("power_mode", early_parse_pwmode_cmdline);

/*
 * 功能描述: 判断是否为异常复位
 */
drv_run_mode_e get_run_mode(void)
{
    if (g_soft_run_mode >= RUN_MODE_INVALID) {
        return RUN_MODE_NORMAL;
    }

    return g_soft_run_mode;
}
EXPORT_SYMBOL(get_run_mode);

static int __init boot_part_parse_args(char *val)
{
    return kstrtou32(val, TRANSFER_BASE, &g_sysboot_dpart_info.boot_part);
}
early_param("boot_part", boot_part_parse_args);

static int __init is_switched_parse_args(char *val)
{
    return kstrtou32(val, TRANSFER_BASE, &g_sysboot_dpart_info.is_switched);
}
early_param("is_switched", is_switched_parse_args);

static int __init boot_part_switched_parse_args(char *val)
{
    return kstrtou32(val, TRANSFER_BASE, &g_sysboot_dpart_info.boot_part_switched);
}
early_param("boot_part_switched", boot_part_switched_parse_args);

static int __init boot_times_parse_args(char *val)
{
    return kstrtou32(val, TRANSFER_BASE, &g_sysboot_dpart_info.curr_boot_times);
}
early_param("boot_times", boot_times_parse_args);

static int __init escape_flag_parse_args(char *val)
{
    return kstrtou32(val, TRANSFER_BASE, &g_sysboot_dpart_info.escape_flag);
}
early_param("escape_flag", escape_flag_parse_args);

static int __init switch_type_parse_args(char *val)
{
    return kstrtou32(val, TRANSFER_BASE, &g_sysboot_dpart_info.switch_type);
}
early_param("switch_type", switch_type_parse_args);

static int __init dpart_enable_parse_args(char *val)
{
    return kstrtou32(val, TRANSFER_BASE, &g_sysboot_dpart_info.dpart_enable);
}
early_param("dpart_enable", dpart_enable_parse_args);

unsigned int mdrv_sysboot_get_switch_type(void)
{
    return g_sysboot_dpart_info.switch_type;
}
EXPORT_SYMBOL(mdrv_sysboot_get_switch_type);

int bsp_sysboot_part_is_switched(void)
{
    return g_sysboot_dpart_info.is_switched;
}
EXPORT_SYMBOL(bsp_sysboot_part_is_switched);

int bsp_sysboot_get_switch_type(void)
{
    return g_sysboot_dpart_info.switch_type;
}
EXPORT_SYMBOL(bsp_sysboot_get_switch_type);

int bsp_sysboot_dpart_is_enabled(void)
{
    return g_sysboot_dpart_info.dpart_enable;
}
EXPORT_SYMBOL(bsp_sysboot_dpart_is_enabled);

unsigned int mdrv_sysboot_get_boot_part(void)
{
    return g_sysboot_dpart_info.boot_part;
}
EXPORT_SYMBOL(mdrv_sysboot_get_boot_part);

int bsp_sysboot_boot_is_back(void)
{
    bsp_err("boot_part is %s\n", (g_sysboot_dpart_info.boot_part == BOOT_FROM_B_PART_FLAG) ? "back" : "master");
    return (g_sysboot_dpart_info.boot_part == BOOT_FROM_B_PART_FLAG);
}
