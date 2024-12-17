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

#ifndef ADRV_PCIE_H
#define ADRV_PCIE_H

/* *******pcie start*********** */
enum pcie_kport_trigger {
    PCIE_KPORT_TRIGGER_CALLBACK,
    PCIE_KPORT_TRIGGER_COMPLETION,
};

enum pcie_kport_event {
    PCIE_KPORT_EVENT_MIN_INVALID = 0x0,  /* min invalid value */
    PCIE_KPORT_EVENT_LINKUP = 0x1,       /* linkup event  */
    PCIE_KPORT_EVENT_LINKDOWN = 0x2,     /* linkdown event */
    PCIE_KPORT_EVENT_WAKE = 0x4,         /* wake event */
    PCIE_KPORT_EVENT_L1SS = 0x8,         /* l1ss event */
    PCIE_KPORT_EVENT_CPL_TIMEOUT = 0x10, /* completion timeout event */
    PCIE_KPORT_EVENT_MAX_INVALID = 0x1F, /* max invalid value */
};

struct pcie_kport_notify {
    enum pcie_kport_event event;
    void *user;
    void *data;
    u32 options;
};

struct pcie_kport_register_event {
    u32 events;
    void *user;
    enum pcie_kport_trigger mode;
    void (*callback)(struct pcie_kport_notify *notify);
    struct pcie_kport_notify notify;
    struct completion *completion;
    u32 options;
};

/* ****************************************************************************
 * 函 数 名  : pcie_kport_register_event  
 *
 * 功能描述  : EP register hook fun for link event notification
 *
 * 输入参数  : reg register_event
 *
 * 输出参数  : 无
 *
 * 返 回 值  : 0  处理成功
 * 非0 处理失败
 *
 * 其它说明  : kernel
 *
 * *************************************************************************** */
int pcie_kport_register_event(struct pcie_kport_register_event *reg);

/* ****************************************************************************
 * 函 数 名  : pcie_kport_deregister_event  
 *
 * 功能描述  : EP deregister hook fun for link event notification
 *
 * 输入参数  : reg register_event
 *
 * 输出参数  : 无
 *
 * 返 回 值  : 0  处理成功
 * 非0 处理失败
 *
 * 其它说明  : kernel
 *
 * *************************************************************************** */
int pcie_kport_deregister_event(struct pcie_kport_register_event *reg);

/* ****************************************************************************
 * 函 数 名  : pcie_kport_pm_control
 *
 * 功能描述  : ep控制rc上电接口
 *
 * 输入参数  : power_ops 0---PowerOff normally
 * 1---Poweron normally
 * 2---PowerOFF without PME
 * 3---Poweron without LINK
 * rc_idx ep对接的rc端口号
 *
 * 输出参数  : 无
 *
 * 返 回 值  : 0  处理成功
 * 非0 处理失败
 *
 * 其它说明  : kernel
 *
 * *************************************************************************** */
int pcie_kport_pm_control(int power_ops, u32 rc_idx);

/* ****************************************************************************
 * 函 数 名  : pcie_kport_ep_off
 *
 * 功能描述  : 获取ep设备下电状态接口
 *
 * 输入参数  : rc_idx ep对接的rc端口号
 *
 * 输出参数  : 无
 *
 * 返 回 值  : 0  未下电
 * 1  已下电
 *
 * 其它说明  : kernel
 *
 * *************************************************************************** */
int pcie_kport_ep_off(u32 rc_idx);

/* ****************************************************************************
 * 函 数 名  : pcie_kport_lp_ctrl
 *
 * 功能描述  : 打开关闭低功耗接口
 *
 * 输入参数  : rc_idx ep对接的rc端口号
 * enable 0---关闭
 * 1---打开
 *
 * 输出参数  : 无
 *
 * 返 回 值  : 0  处理成功
 * 非0 处理失败
 *
 * 其它说明  : kernel
 *
 * *************************************************************************** */
int pcie_kport_lp_ctrl(u32 rc_idx, u32 enable);

/* ****************************************************************************
 * 函 数 名  : pcie_kport_enumerate
 *
 * 功能描述  : pcie枚举接口
 *
 * 输入参数  : rc_idx ep对接的rc端口号
 *
 * 输出参数  : 无
 *
 * 返 回 值  : 0  处理成功
 * 非0 处理失败
 *
 * 其它说明  : kernel
 *
 * *************************************************************************** */
int pcie_kport_enumerate(u32 rc_idx);

/* ****************************************************************************
 * 函 数 名  : pcie_kport_remove_ep
 *
 * 功能描述  : pcie删除ep设备接口
 *
 * 输入参数  : rc_idx ep对接的rc端口号
 *
 * 输出参数  : 无
 *
 * 返 回 值  : 0  处理成功
 * 非0 处理失败
 *
 * 其它说明  : kernel
 *
 * *************************************************************************** */
int pcie_kport_remove_ep(u32 rc_idx);

/* ****************************************************************************
 * 函 数 名  : pcie_kport_rescan_ep
 *
 * 功能描述  : pcie扫描ep设备接口
 *
 * 输入参数  : rc_idx ep对接的rc端口号
 *
 * 输出参数  : 无
 *
 * 返 回 值  : 0  处理成功
 * 非0 处理失败
 *
 * 其它说明  : kernel
 *
 * *************************************************************************** */
int pcie_kport_rescan_ep(u32 rc_idx);

/* ****************************************************************************
 * 函 数 名  : pcie_ep_link_ltssm_notify
 *
 * 功能描述  : 设置ep设备link状态接口
 *
 * 输入参数  : rc_id ep对接的rc端口号
 * link_status ep设备link状态
 *
 * 输出参数  : 无
 *
 * 返 回 值  : 0  处理成功
 * 非0 处理失败
 *
 * 其它说明  : kernel
 *
 * *************************************************************************** */
int pcie_ep_link_ltssm_notify(u32 rc_id, u32 link_status);

/* ****************************************************************************
 * 函 数 名  : pcie_kport_power_notifiy_register
 *
 * 功能描述  : 注册pcie上电提供时钟后ep回调
 *
 * 输入参数  : rc_id ep对接的rc端口号
 * poweron ep上电回调指针
 * poweroff ep下电回调指针
 * data ep回调函数入参
 *
 * 输出参数  : 无
 *
 * 返 回 值  : 0  处理成功
 * 非0 处理失败
 *
 * 其它说明  : kernel
 *
 * *************************************************************************** */
int pcie_kport_power_notifiy_register(u32 rc_id, int (*poweron)(void *data), int (*poweroff)(void *data), void *data);

#endif
