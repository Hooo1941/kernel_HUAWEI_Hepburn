/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
#ifndef _BSP_USB_H_
#define _BSP_USB_H_

#include <osl_types.h>
#include <mdrv_udi.h>

/* IOCTL CMD 定义 */
#define RELAY_ACM_IOCTL_SET_WRITE_CB      0x7F001000
#define RELAY_ACM_IOCTL_SET_READ_CB       0x7F001001
#define RELAY_ACM_IOCTL_SET_EVT_CB        0x7F001002
#define RELAY_ACM_IOCTL_SET_FREE_CB       0x7F001003
#define RELAY_ACM_IOCTL_SET_WRITE_INFO_CB 0x7F001004
#define RELAY_ACM_IOCTL_SET_DATA_READ_CB  0x7F001005

#define RELAY_ACM_IOCTL_WRITE_ASYNC       0x7F001010
#define RELAY_ACM_IOCTL_GET_RD_BUFF       0x7F001011
#define RELAY_ACM_IOCTL_RETURN_BUFF       0x7F001012
#define RELAY_ACM_IOCTL_RELLOC_READ_BUFF  0x7F001013
#define RELAY_ACM_IOCTL_GET_WRITE_NUM     0x7F001014

#define RELAY_ACM_MODEM_IOCTL_SET_MSC_READ_CB 0x7F001030
#define RELAY_ACM_MODEM_IOCTL_MSC_WRITE_CMD   0x7F001031
#define RELAY_ACM_MODEM_IOCTL_SET_REL_IND_CB  0x7F001032
#define RELAY_ACM_MODEM_IOCTL_SET_SWITCH_MODE_CB    0x7F001033

#define RELAY_ACM_MODEM_IOCTL_SET_TRANS_MODE  0x7F001040
#define RELAY_ACM_MODEM_IOCTL_SET_IFACE_INFO  0x7F001041
#define RELAY_ACM_MODEM_IOCTL_ENABLE_ESCAPE_DETECT  0x7F001042

typedef void (*usb_modem_msc_read_cb)(struct modem_msc_stru *modem_msc);
typedef void (*acm_modem_switch_mode_cb)(void);

typedef void (*USB_ENABLE_CB_T)(void);
typedef void (*USB_DISABLE_CB_T)(void);

/**
 * @brief 注册usb枚举完成回调
 *
 * @par 描述:
 * 注册usb枚举完成回调
 *
 * @param[in]  pFunc 回调指针
 *
 * @retval 0  处理成功
 * @retval <0 处理失败
 */
unsigned int BSP_USB_RegUdiEnableCB(USB_ENABLE_CB_T pFunc);

/**
 * @brief 注册usb拔出完成回调
 *
 * @par 描述:
 * 注册usb拔出完成回调
 *
 * @param[in]  pFunc 回调指针
 *
 * @retval 0  处理成功
 * @retval <0 处理失败
 */
unsigned int BSP_USB_RegUdiDisableCB(USB_DISABLE_CB_T pFunc);

#endif /* _BSP_USB_H_ */
