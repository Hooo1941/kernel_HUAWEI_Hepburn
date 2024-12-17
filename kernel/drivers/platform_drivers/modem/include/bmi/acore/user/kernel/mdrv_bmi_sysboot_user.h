/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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

#ifndef __MDRV_BMI_SYSBOOT_USER_H__
#define __MDRV_BMI_SYSBOOT_USER_H__

enum ap_boot_state {
    AP_STATE_OFF = 0,
    AP_STATE_READY = 1,
};

#define MODEM_STATE_OFF  "MODEM_STATE_OFF\n"   // modem处于下电状态
#define MODEM_STATE_READY  "MODEM_STATE_READY\n"  // modem处于ready状态，可以提供服务
#define MODEM_STATE_INIT_OK  "MODEM_STATE_INIT_OK\n" // modem ccore module init初始化完成
#define MODEM_STATE_INVALID  "MODEM_STATE_INVALID\n" // modem 状态值无效

#define SYSBOOT_BOOT_MODE_ERR  "error"  // modem boot mode错误
#define SYSBOOT_BOOT_MODE_FMC  "fmc"   // fmc启动方式
#define SYSBOOT_BOOT_MODE_EMMC  "emmc"  // emmc启动方式
#define SYSBOOT_BOOT_MODE_FMC_DOUBLEBOOT  "fmc_doubleboot" // fmc软件双boot启动方式
#define SYSBOOT_BOOT_MODE_AP_PCIE  "pcie" // pcie0启动方式
#define SYSBOOT_BOOT_MODE_AP_UART  "uart" // uart启动方式
#define SYSBOOT_BOOT_MODE_AP_PCIE1  "pcie1" // pcie1启动方式
#define SYSBOOT_BOOT_MODE_FMC_PAD  "fmc_pad" // fmc启动方式，从pad获取参数

/**
 * 设置ap子系统启动状态的节点路径；
 *
 * @see enum ap_boot_state
 */
#define SYSBOOT_AP_STATE_PATH "/sys/devices/platform/modem_power/ap_state"

/**
 * 获取modem启动状态的节点路径；
 *
 * @返回值见： MODEM_STATE_OFF、 MODEM_STATE_READY、MODEM_STATE_INIT_OK、MODEM_STATE_INVALID
 */
#define SYSBOOT_MODEM_STATE_PATH "/sys/devices/platform/modem_power/state"

/**
 * 获取系统启动模式的节点路径；
 *
 * @返回值见： SYSBOOT_BOOT_MODEM_ERR、 SYSBOOT_BOOT_MODE_FMC、SYSBOOT_BOOT_MODE_EMMC、SYSBOOT_BOOT_MODE_FMC_DOUBLEBOOT、
 *                         SYSBOOT_BOOT_MODE_AP_PCIE、SYSBOOT_BOOT_MODE_AP_UART、SYSBOOT_BOOT_MODE_AP_PCIE1、SYSBOOT_BOOT_MODE_FMC_PAD
 */
#define SYSBOOT_BOOT_MODE_PATH  "/sys/devices/platform/modem_power/modem_boot_mode"

#endif
