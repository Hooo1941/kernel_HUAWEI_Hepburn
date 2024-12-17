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

#ifndef __BSP_DFXCERT_H__
#define __BSP_DFXCERT_H__

#include <product_config_drv_ap.h>
#include <linux/notifier.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DFXCERT_PORT_SUPPORT (0x5A5A5A5A)
#define DFXCERT_PORT_NOT_SUPPORT (0xA5A5A5A5)

enum port_ctrl_e {
    USB_3G_APP = 0,
    USB_APP,
    USB_GPS,
    USB_ADB,
    OM_COMPONENT,
    MFG_COMPONENT,
    OM_FULLDUMP_UPLOAD,
    PORT_AT_PHYNUM_UPDATE_IMEI,
    PORT_AT_TMODE,
    DIAG_SOCKET,
    UART_ASHELL,
    NVA_DEBUG,
    DFXCERT_PORT_MAX = 32,
};

/* *
 * @brief 证书端口授权状态获取接口。
 *
 * @par 描述:
 * 证书端口授权状态获取接口。
 *
 * @param[in]  port, 获取当前端口授权状态，枚举值可参考port_ctrl_e。
 *
 * @retval DFXCERT_PORT_SUPPORT，表示授权使能端口。
 * @retval DFXCERT_PORT_NOT_SUPPORT，表示不使能端口,其他异常也会返回该值。
 *
 * @attention
 * <ul><li>MBB平台USER版本商用芯片会根据证书有无及校验状态返回端口使能状态，其他生命周期芯片不需要证书，直接返回DFXCERT_PORT_SUPPORT。ENG版本固定返回DFXCERT_PORT_SUPPORT。</li></ul>
 * <ul><li>Phone平台不支持该接口。</li></ul>
 *
 */
#ifndef BSP_CONFIG_PHONE_TYPE
int bsp_dfxcert_status_get(enum port_ctrl_e port);
#else
static inline int bsp_dfxcert_status_get(enum port_ctrl_e port)
{
#ifdef CONFIG_DFXCERT_VERIFY
    return DFXCERT_PORT_NOT_SUPPORT;
#else
    return DFXCERT_PORT_SUPPORT;
#endif
}
#endif

/* *
 * @brief 证书校验状态更新通知接口。
 *
 * @par 描述:
 * 证书校验状态更新通知接口。
 *
 * @param[in]  nb, 通知结构体，见linux/notifier.h。
 *
 * @retval 0，表示注册成功。
 * @retval 非0，表示注册失败。
 *
 * @attention
 * <ul><li>Phone平台不支持该接口。</li></ul>
 *
 */
#ifndef BSP_CONFIG_PHONE_TYPE
int bsp_dfxcert_verify_notifier_register(struct notifier_block *nb);
#else
static inline int bsp_dfxcert_verify_notifier_register(struct notifier_block *nb)
{
    return 0;
}
#endif

#ifdef __cplusplus
}
#endif

#endif
