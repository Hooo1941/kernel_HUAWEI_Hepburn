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
#ifndef _BSP_RELAY_H_
#define _BSP_RELAY_H_

#include <osl_types.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum relay_dev_major_type {
    RELAY_ACM_TYPE,
    RELAY_PCIE_TYPE,
    RELAY_UART_TYPE,
    RELAY_NET_TYPE,
    RELAY_MAX_TYPE,
};

enum relay_uart_minor_type {
    RELAY_DEV_HSUART,
    RELAY_DEV_CMUX0,
    RELAY_DEV_CMUX1,
    RELAY_DEV_CMUX2,
    RELAY_DEV_CMUX3,
    RELAY_DEV_UART_MAX,
};

enum relay_acm_minor_type {
    RELAY_DEV_ACM_CTRL,
    RELAY_DEV_AT,
    RELAY_DEV_OM,
    RELAY_DEV_MODEM,
    RELAY_DEV_SKYTONE,
    RELAY_DEV_PCIE_AT,
    RELAY_DEV_ACM_MAX,
};

enum relay_net_minor_type {
    RELAY_DEV_UNET_CTRL,
    RELAY_DEV_UNET_AT,
    RELAY_DEV_PNET_CTRL,
    RELAY_NET_MAX_TYPE,
};

#define RELAY_BUILD_TYPE_ID(dev, type) (((dev) << 8) | ((type) & 0x00ff))

enum relay_dev_type {
    RELAY_ACM_DEV_CTRL_TYPE = RELAY_BUILD_TYPE_ID(RELAY_ACM_TYPE, RELAY_DEV_ACM_CTRL),
    RELAY_ACM_DEV_AT_TYPE = RELAY_BUILD_TYPE_ID(RELAY_ACM_TYPE, RELAY_DEV_AT),
    RELAY_ACM_DEV_OM_TYPE = RELAY_BUILD_TYPE_ID(RELAY_ACM_TYPE, RELAY_DEV_OM),
    RELAY_ACM_DEV_MODEM_TYPE = RELAY_BUILD_TYPE_ID(RELAY_ACM_TYPE, RELAY_DEV_MODEM),
    RELAY_ACM_DEV_SKYTONE_TYPE = RELAY_BUILD_TYPE_ID(RELAY_ACM_TYPE, RELAY_DEV_SKYTONE),
    RELAY_PCIE_DEV_AT_TYPE = RELAY_BUILD_TYPE_ID(RELAY_ACM_TYPE, RELAY_DEV_PCIE_AT),
    RELAY_UART_DEV_HSUART_TYPE = RELAY_BUILD_TYPE_ID(RELAY_UART_TYPE, RELAY_DEV_HSUART),
    RELAY_UART_DEV_CMUX0_TYPE = RELAY_BUILD_TYPE_ID(RELAY_UART_TYPE, RELAY_DEV_CMUX0),
    RELAY_UART_DEV_CMUX1_TYPE = RELAY_BUILD_TYPE_ID(RELAY_UART_TYPE, RELAY_DEV_CMUX1),
    RELAY_UART_DEV_CMUX2_TYPE = RELAY_BUILD_TYPE_ID(RELAY_UART_TYPE, RELAY_DEV_CMUX2),
    RELAY_UART_DEV_CMUX3_TYPE = RELAY_BUILD_TYPE_ID(RELAY_UART_TYPE, RELAY_DEV_CMUX3),
    RELAY_UNET_DEV_AT_TYPE = RELAY_BUILD_TYPE_ID(RELAY_NET_TYPE, RELAY_DEV_UNET_AT),
    RELAY_NET_DEV_USB_TYPE = RELAY_BUILD_TYPE_ID(RELAY_NET_TYPE, RELAY_DEV_UNET_CTRL),
    RELAY_NET_DEV_PCIE_TYPE = RELAY_BUILD_TYPE_ID(RELAY_NET_TYPE, RELAY_DEV_PNET_CTRL),
    RELAY_INVAL_DEV_ID = 0xFFFF
};

struct relay_write_info {
    char *virt_addr;
    char *phy_addr;
    unsigned int size;
    void *priv;
};

enum relay_evt {
    RELAY_EVT_DEV_SUSPEND = 0,
    RELAY_EVT_DEV_READY = 1,
    RELAY_EVT_DEV_BOTTOM
};

struct relay_read_buff_info {
    unsigned int buff_size;
    unsigned int buff_num;
};

enum relay_net_cmd_type {
    RELAY_NET_AT_RECV,
    RELAY_NET_STAT_CHG,
    RELAY_NET_ACK_CFG,
    RELAY_NET_REG_READ_CB,
    RELAY_NET_AT_RESPONSE,
    RELAY_NET_WRITE_CFG,
    RELAY_NET_QUERY_READY,
    RELAY_NET_NOTIFY_READY,
    RELAY_NET_MAX_CMD,
};

enum relay_acm_cmd_type {
    RELAY_ACM_WRITE_ASYNC,
    RELAY_ACM_ACK_ASYNC,
    RELAY_ACM_WRITE_MSC,
    RELAY_ACM_ACK_MSC,
    RELAY_ACM_REG_READ_CB,
    RELAY_ACM_REG_WRITE_CB,
    RELAY_ACM_REG_EVT_CB,
    RELAY_ACM_QUERY_READY,
    RELAY_ACM_NOTIFY_READY,
    RELAY_ACM_NOTIFY_SWITCH_MODE,
    RELAY_ACM_SET_TRANS_MODE,
    RELAY_ACM_SET_IFACE_INFO,
    RELAY_ACM_ENABLE_ESCAPE_DETECT,
    RELAY_ACM_MAX_CMD,
};

enum relay_uart_cmd_type {
    RELAY_UART_WRITE_FLOW_CTRL,
    RELAY_UART_WRITE_STP,
    RELAY_UART_WRITE_MSC,
    RELAY_UART_WRITE_EPS,
    RELAY_UART_WRITE_WLEN,
    RELAY_UART_WRITE_BAUD,
    RELAY_UART_WRITE_ASYNC,
    RELAY_UART_ACK_ASYNC,
    RELAY_UART_ACK_MSC,
    RELAY_UART_REG_READ_CB,
    RELAY_UART_REG_WRITE_CB,
    RELAY_UART_QUERY_READY,
    RELAY_UART_SET_CMUX_INFO,
    RELAY_UART_SET_CMUX_DLC,
    RELAY_UART_ENABLE_CMUX,
    RELAY_UART_ENABLE_ESCAPE_DETECT,
    RELAY_UART_NOTIFY_READY,
    RELAY_UART_NOTIFY_SWITCH_MODE,
    RELAY_UART_NOTIFY_WATER_LINE,
    RELAY_UART_MAX_CMD_TYPE,
};

typedef void (*relay_enable_cb)(void *private);
typedef void (*relay_disable_cb)(void *private);
typedef int (*relay_acm_read_cb)(void *private, unsigned int type, void *addr, int size);
typedef void (*relay_acm_write_donecb)(char *virt_addr, char *phy_addr, int size);
typedef void (*relay_acm_event_cb)(enum relay_evt port_state);

typedef int (*mci_uart_read_cb)(void *private, unsigned int type, void *addr, int size);
typedef void (*mci_uart_free_cb)(char *buff);

typedef int (*relay_net_read_cb)(void *private, unsigned int type, void *buf, unsigned int size);

int bsp_relay_reg_enablecb(unsigned int relay_id, relay_enable_cb func, void *private);
int bsp_relay_reg_disablecb(unsigned int relay_id, relay_disable_cb func);
int bsp_relay_open(unsigned int relay_id);
int bsp_relay_close(unsigned int relay_id);
int bsp_relay_write(unsigned int relay_id, char *addr, unsigned int size);
int bsp_relay_ioctl(unsigned int relay_id, unsigned int cmd, void *para, unsigned int para_size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _BSP_RELAY_H_ */
