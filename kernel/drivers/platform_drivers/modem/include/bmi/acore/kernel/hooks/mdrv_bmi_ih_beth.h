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


#ifndef __MDRV_BMI_IH_BETH_H__
#define __MDRV_BMI_IH_BETH_H__

#include "hooks/mdrv_bmi_ih.h"
#include <linux/skbuff.h>
#include <linux/platform_device.h>
#include <linux/phy.h>

/**
 * @brief 支持产品线phy定制reset
 *
 * @par 描述:
 * 支持产品线phy定制reset
 *
 * @param[in]  intf , phy接口类型
 * @param[in]  ret , 出参。0：执行基线phy_reset流程; !0：不执行；
 *
 */
DECLARE_IOT_HOOK(iot_hook_beth_reset_phy,
    IH_PROTO(int intf, int *ret),
    IH_ARGS(intf, ret));

/**
 * @brief 产品线读取自定义节点属性
 *
 * @par 描述:
 * 产品线读取自定义节点属性
 *
 * @param[in]  np ，设备节点
 *
 */
DECLARE_IOT_HOOK(iot_hook_beth_config_dt,
    IH_PROTO(struct device_node *np),
    IH_ARGS(np));

/**
 * @brief 支持产品线定制mdio读流程
 *
 * @par 描述:
 * 支持产品线定制mdio读流程
 *
 * @param[in]  bus ， mdio总线
 * @param[in]  phyaddr ， 总线地址
 * @param[in]  dev_reg ， 总线寄存器
 * @param[in]  retval ， 出参，读取的值
 * @param[in]  ret ， 出参，0:使用基线读流程；!0：使用产品线定制的读流程
 *
 */
DECLARE_IOT_HOOK(iot_hook_beth_mdio_read,
    IH_PROTO(struct mii_bus *bus, int phyaddr, int dev_reg, int *retval, int *ret),
    IH_ARGS(bus, phyaddr, dev_reg, retval, ret));

/**
 * @brief 支持产品线定制mdio写流程
 *
 * @par 描述:
 * 支持产品线定制mdio写流程
 *
 * @param[in]  bus ， mdio总线
 * @param[in]  phyaddr ， 总线地址
 * @param[in]  dev_reg ， 总线寄存器
 * @param[in]  phydata ， 写入的数据
 * @param[in]  ret ， 出参，0:使用基线写流程；!0：使用产品线定制的写流程
 *
 */
DECLARE_IOT_HOOK(iot_hook_beth_mdio_write,
    IH_PROTO(struct mii_bus *bus, int phyaddr, int dev_reg, u16 phydata, int *ret),
    IH_ARGS(bus, phyaddr, dev_reg, phydata, ret));

/**
 * @brief 支持产品线在probe之后增加配置
 *
 * @par 描述:
 * 支持产品线在probe之后增加配置
 *
 * @param[in]  intf ， phy接口类型
 * @param[in]  portno ， pfa端口号
 * @param[in]  portno_wan ， pfa端口号
 * @param[in]  phydev ， phy设备结构体
 *
 */
DECLARE_IOT_HOOK(iot_hook_beth_probe_finish,
    IH_PROTO(int intf, int portno, int portno_wan, struct phy_device *phydev),
    IH_ARGS(intf, portno, portno_wan, phydev));

/**
 * @brief 支持产品线在网卡open之前增加配置
 *
 * @par 描述:
 * 支持产品线在网卡open之前增加配置
 *
 * @param[in]  intf ， phy接口类型
 * @param[in]  ret ， 出参；0：正常open网卡；!0：不打开网卡
 *
 */
DECLARE_IOT_HOOK(iot_hook_beth_before_open,
    IH_PROTO(int intf, int *ret),
    IH_ARGS(intf, ret));

/**
 * @brief 支持产品线在网卡open之后增加配置
 *
 * @par 描述:
 * 支持产品线在网卡open之后增加配置
 *
 * @param[in]  dev ， 设备结构体
 * @param[in]  ndev ，网络设备结构体
 *
 */
DECLARE_IOT_HOOK(iot_hook_beth_after_open,
    IH_PROTO(struct device *dev, struct net_device *ndev),
    IH_ARGS(dev, ndev));

/**
 * @brief 支持产品线增加phy配置
 *
 * @par 描述:
 * 支持产品线增加phy配置
 *
 * @param[in]  ndev ， 网络设备结构体
 * @param[in]  phydev ， phy设备结构体
 * @param[in]  ret ， 出参，!0：使用switch_phy,需要执行adjust_link
 *
 */
DECLARE_IOT_HOOK(iot_hook_beth_config_phy,
    IH_PROTO(struct net_device *ndev, struct phy_device *phydev, int *ret),
    IH_ARGS(ndev, phydev, ret));

/**
 * @brief 支持产品线在网卡release之前增加配置
 *
 * @par 描述:
 * 支持产品线在网卡release之前增加配置
 *
 * @param[in]  ndev ， 网络设备结构体
 *
 */
DECLARE_IOT_HOOK(iot_hook_beth_before_release,
    IH_PROTO(struct net_device *ndev),
    IH_ARGS(ndev));

/**
 * @brief 支持产品线在网卡tx软转之前增加配置
 *
 * @par 描述:
 * 支持产品线在网卡tx软转之前增加配置
 *
 * @param[in]  ndev ， 网络设备结构体
 * @param[in]  dev ， 设备结构体
 * @param[in]  skb ， 数据包
 * @param[in]  nopaged_len ， 报文头长度
 *
 */
DECLARE_IOT_HOOK(iot_hook_beth_before_xmit,
    IH_PROTO(struct net_device *ndev, struct device *dev, struct sk_buff *skb, unsigned int *nopaged_len),
    IH_ARGS(ndev, dev, skb, nopaged_len));

/**
 * @brief 支持产品线在网卡tx硬转之前增加配置
 *
 * @par 描述:
 * 支持产品线在网卡tx硬转之前增加配置
 *
 * @param[in]  ndev ， 网络设备结构体
 * @param[in]  dev ， 设备结构体
 * @param[in]  buf_addr ， 数据地址
 * @param[in]  nopaged_len ， 报文头长度
 * @param[in]  l2_hdr_offset ， 地址头偏移
 *
 */
DECLARE_IOT_HOOK(iot_hook_beth_before_pfa_xmit,
    IH_PROTO(struct net_device *ndev, struct device *dev, unsigned long long *buf_addr, unsigned int *nopaged_len, unsigned int *l2_hdr_offset),
    IH_ARGS(ndev, dev, buf_addr, nopaged_len, l2_hdr_offset));

/**
 * @brief 支持产品线在网卡rx收包之前增加配置
 *
 * @par 描述:
 * 支持产品线在网卡rx收包之前增加配置
 *
 * @param[in]  ndev ， 网络设备结构体
 * @param[in]  dev ， 设备结构体
 * @param[in]  skb ， 数据包
 * @param[in]  frame_len ， 帧长度
 * @param[in]  ret ， 出参，0：正常收包； !0：停止收包
 *
 */
DECLARE_IOT_HOOK(iot_hook_beth_before_rx_trans,
    IH_PROTO(struct net_device *ndev, struct device *dev, struct sk_buff *skb, unsigned int *frame_len, int *ret),
    IH_ARGS(ndev, dev, skb, frame_len, ret));

/**
 * @brief 支持产品线在网卡配置td之前增加配置
 *
 * @par 描述:
 * 支持产品线在网卡配置td之前增加配置
 *
 * @param[in]  ndev ， 网络设备结构体
 * @param[in]  skb ， 数据包
 * @param[in]  intf ， phy接口类型
 * @param[in]  portno_wan、portno ， pfa端口号
 * @param[in]  dma ， dma地址
 *
 */
DECLARE_IOT_HOOK(iot_hook_beth_before_config_td,
    IH_PROTO(struct net_device *ndev, struct sk_buff *skb, int intf, unsigned int *portno_wan, unsigned int *portno, dma_addr_t *dma),
    IH_ARGS(ndev, skb, intf, portno_wan, portno, dma));

/**
 * @brief 支持phy上电
 *
 * @par 描述:
 * 支持phy上电
 *
 * @param[in]  ndev ， 网络设备结构体
 *
 */
DECLARE_IOT_HOOK(iot_hook_beth_phy_power_on,
    IH_PROTO(struct net_device *ndev),
    IH_ARGS(ndev));

/**
 * @brief 支持phy下电
 *
 * @par 描述:
 * 支持phy下电
 *
 * @param[in]  ndev ， 网络设备结构体
 *
 */
DECLARE_IOT_HOOK(iot_hook_beth_phy_power_off,
    IH_PROTO(struct net_device *ndev),
    IH_ARGS(ndev));

/**
 * @brief 支持产品线在TX clean中增加配置
 *
 * @par 描述:
 * 支持产品线在TX clean中增加配置
 *
 * @param[in]  skb ， 数据包
 * @param[in]  ret ， 出参， 0：释放数据包内存；!0：不释放数据包内存
 *
 */
DECLARE_IOT_HOOK(iot_hook_beth_tx_clean,
    IH_PROTO(struct sk_buff *skb, int *ret),
    IH_ARGS(skb, ret));

/**
 * @brief 支持产品线在suspend之前增加配置
 *
 * @par 描述:
 * 支持产品线在suspend之前增加配置
 *
 * @param[in]  ndev ， 网络设备结构体
 *
 */
DECLARE_IOT_HOOK(iot_hook_beth_before_suspend,
    IH_PROTO(struct net_device *ndev),
    IH_ARGS(ndev));

/**
 * @brief ioctl入口
 *
 * @par 描述:
 * ioctl入口
 *
 * @param[in]  ndev ， 网络设备结构体
 * @param[in]  rq ， 将信息传递给驱动的结构体
 * @param[in]  cmd , ioctl 命令
 * @param[in]  ret , 出参
 *
 */
DECLARE_IOT_HOOK(iot_hook_beth_ioctl,
    IH_PROTO(struct net_device *ndev, struct ifreq *rq, int cmd, int *ret),
    IH_ARGS(ndev, rq, cmd, ret));


#endif