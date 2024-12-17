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
#include "power_hal.h"

struct power_cfg g_power_cfg = {
    .reboot_addr_offset = 0x404,
    .reboot_bits_offset = 0x4,
    .reboot_cmd_value = 0x1,
    .pwroff_addr_offset = 0x404,
    .pwroff_bits_offset = 0x5,
    .pwroff_cmd_value = 0x0,
    .boot_mode_addr_base = 0xEDF00498, // SC_PAD_LOCK_STAT
    .boot_mode_mask = (0x1 << 0x3) - 0x1,
    .boot_protect_addr_base = 0xEDF00524, // SC_POR_REMAIN_MSK16_REG5;
    .boot_part_switched_offset = 0,
    .boot_part_switched_mask = 0x1 << 0,
    .boot_part_switched_enable = 0x1 << 0x10,
    .boot_escape_offset = 1,
    .boot_escape_mask = 0x1 << 1,
    .boot_escape_enable = 0x1 << 0x11,
    .boot_times_offset = 0x4,
    .boot_times_mask = 0xf << 0x4,
    .boot_times_enable = 0xf << 0x14,
    .boot_download_mode_offset = 0x8,
    .boot_download_mode_mask = 0x1 << 0x8,
    .boot_download_mode_enable = 0x1 << 0x18,
};

struct boot_mode_map g_boot_mode_map[] = {
    {SYSBOOT_BOOT_MODE_FMC, 0x0},
    {SYSBOOT_BOOT_MODE_EMMC0, 0x1},
    {SYSBOOT_BOOT_MODE_FMC_DOUBLEBOOT, -1},
    {SYSBOOT_BOOT_MODE_AP_PCIE, 0x2},
    {SYSBOOT_BOOT_MODE_AP_UART, 0x3},
    {SYSBOOT_BOOT_MODE_AP_PCIE1, -1},
    {SYSBOOT_BOOT_MODE_FMC_PAD, -1},
};

struct boot_mode_map *boot_mode_map_get(u32 *size)
{
    *size = sizeof(g_boot_mode_map) / sizeof(g_boot_mode_map[0]);
    return &g_boot_mode_map[0];
}
struct power_cfg *power_get_config(void)
{
    return &g_power_cfg;
}

