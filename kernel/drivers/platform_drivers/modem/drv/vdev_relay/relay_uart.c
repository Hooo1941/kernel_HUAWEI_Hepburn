/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
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
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS"
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
#include <linux/slab.h>
#include <securec.h>
#include "relay_uart.h"

#define THIS_MODU mod_relay

void relay_uart_read_done_cb(unsigned int minor_type);
#define UART_READ_DONE_CB(_name)                \
    static void uart_read_done_cb_##_name(void) \
    {                                          \
        relay_uart_read_done_cb(_name);         \
    }

UART_READ_DONE_CB(RELAY_DEV_HSUART);
UART_READ_DONE_CB(RELAY_DEV_CMUX0);
UART_READ_DONE_CB(RELAY_DEV_CMUX1);
UART_READ_DONE_CB(RELAY_DEV_CMUX2);
UART_READ_DONE_CB(RELAY_DEV_CMUX3);
#define UART_READ_DONE_CB_NAME(_name) (uart_read_done_cb_##_name)

void relay_uart_free_cb(unsigned int minor_type, char *addr, unsigned int size);
#define UART_FREE_CB(_name)                                                    \
    static void uart_free_cb_##_name(char *addr, unsigned int size) \
    {                                                                               \
        relay_uart_free_cb(_name, addr, size);                   \
    }

UART_FREE_CB(RELAY_DEV_HSUART);
UART_FREE_CB(RELAY_DEV_CMUX0);
UART_FREE_CB(RELAY_DEV_CMUX1);
UART_FREE_CB(RELAY_DEV_CMUX2);
UART_FREE_CB(RELAY_DEV_CMUX3);
#define UART_FREE_CB_NAME(_name) (uart_free_cb_##_name)

void relay_uart_msc_read_cb(unsigned int minor_type, struct modem_msc_stru *msc);
#define UART_MSC_READ_CB(_name) \
    static void uart_msc_read_cb_##_name(struct modem_msc_stru *msc)     \
    {                                                         \
        relay_uart_msc_read_cb(_name, msc); \
    }

UART_MSC_READ_CB(RELAY_DEV_HSUART);
UART_MSC_READ_CB(RELAY_DEV_CMUX0);
UART_MSC_READ_CB(RELAY_DEV_CMUX1);
UART_MSC_READ_CB(RELAY_DEV_CMUX2);
UART_MSC_READ_CB(RELAY_DEV_CMUX3);
#define UART_MSC_READ_CB_NAME(_name) (uart_msc_read_cb_##_name)

void relay_uart_switch_mode_cb(unsigned int minor_type);
#define UART_SWITCH_MODE_CB(_name) \
    static void uart_switch_mode_cb_##_name(void)     \
    {                                                         \
        relay_uart_switch_mode_cb(_name); \
    }

UART_SWITCH_MODE_CB(RELAY_DEV_HSUART);
UART_SWITCH_MODE_CB(RELAY_DEV_CMUX0);
UART_SWITCH_MODE_CB(RELAY_DEV_CMUX1);
UART_SWITCH_MODE_CB(RELAY_DEV_CMUX2);
UART_SWITCH_MODE_CB(RELAY_DEV_CMUX3);
#define UART_SWITCH_MODE_CB_NAME(_name) (uart_switch_mode_cb_##_name)

void relay_uart_water_line_cb(unsigned int minor_type, water_level_enum level);
#define UART_WATER_LINE_CB(_name) \
    static void uart_water_line_cb_##_name(water_level_enum level)     \
    {                                                         \
        relay_uart_water_line_cb(_name, level); \
    }

UART_WATER_LINE_CB(RELAY_DEV_HSUART);
#define UART_WATER_LINE_CB_NAME(_name) (uart_water_line_cb_##_name)

void relay_uart_ready_cb(unsigned int minor_type, port_ready_enum status);
#define UART_READY_CB(_name) \
    static void uart_ready_cb_##_name(port_ready_enum status)     \
    {                                                         \
        relay_uart_ready_cb(_name, status); \
    }

UART_READY_CB(RELAY_DEV_HSUART);
UART_READY_CB(RELAY_DEV_CMUX0);
UART_READY_CB(RELAY_DEV_CMUX1);
UART_READY_CB(RELAY_DEV_CMUX2);
UART_READY_CB(RELAY_DEV_CMUX3);
#define UART_READY_CB_NAME(_name) (uart_ready_cb_##_name)

static struct relay_uart_device g_uart_devices[RELAY_DEV_UART_MAX] =
{
    [RELAY_DEV_HSUART] = {
        .adp_dev.name = "HSUART",
        .adp_dev.udi_id = UDI_UART_0_ID,
        .funcs.read_done_cb = UART_READ_DONE_CB_NAME(RELAY_DEV_HSUART),
        .funcs.free_cb = UART_FREE_CB_NAME(RELAY_DEV_HSUART),
        .funcs.msc_read_cb = UART_MSC_READ_CB_NAME(RELAY_DEV_HSUART),
        .funcs.switch_mode_cb = UART_SWITCH_MODE_CB_NAME(RELAY_DEV_HSUART),
        .funcs.water_line_cb = UART_WATER_LINE_CB_NAME(RELAY_DEV_HSUART),
        .funcs.ready_cb = UART_READY_CB_NAME(RELAY_DEV_HSUART),
    },
    [RELAY_DEV_CMUX0] = {
        .adp_dev.name = "CMUX0",
        .adp_dev.udi_id = UDI_CMUX_AT_ID,
        .funcs.read_done_cb = UART_READ_DONE_CB_NAME(RELAY_DEV_CMUX0),
        .funcs.free_cb = UART_FREE_CB_NAME(RELAY_DEV_CMUX0),
        .funcs.msc_read_cb = UART_MSC_READ_CB_NAME(RELAY_DEV_CMUX0),
        .funcs.switch_mode_cb = UART_SWITCH_MODE_CB_NAME(RELAY_DEV_CMUX0),
        .funcs.water_line_cb = NULL,
        .funcs.ready_cb = UART_READY_CB_NAME(RELAY_DEV_CMUX0),
    },
    [RELAY_DEV_CMUX1] = {
        .adp_dev.name = "CMUX1",
        .adp_dev.udi_id = UDI_CMUX_MDM_ID,
        .funcs.read_done_cb = UART_READ_DONE_CB_NAME(RELAY_DEV_CMUX1),
        .funcs.free_cb = UART_FREE_CB_NAME(RELAY_DEV_CMUX1),
        .funcs.msc_read_cb = UART_MSC_READ_CB_NAME(RELAY_DEV_CMUX1),
        .funcs.switch_mode_cb = UART_SWITCH_MODE_CB_NAME(RELAY_DEV_CMUX1),
        .funcs.water_line_cb = NULL,
        .funcs.ready_cb = UART_READY_CB_NAME(RELAY_DEV_CMUX1),
    },
    [RELAY_DEV_CMUX2] = {
        .adp_dev.name = "CMUX2",
        .adp_dev.udi_id = UDI_CMUX_EXT_ID,
        .funcs.read_done_cb = UART_READ_DONE_CB_NAME(RELAY_DEV_CMUX2),
        .funcs.free_cb = UART_FREE_CB_NAME(RELAY_DEV_CMUX2),
        .funcs.msc_read_cb = UART_MSC_READ_CB_NAME(RELAY_DEV_CMUX2),
        .funcs.switch_mode_cb = UART_SWITCH_MODE_CB_NAME(RELAY_DEV_CMUX2),
        .funcs.water_line_cb = NULL,
        .funcs.ready_cb = UART_READY_CB_NAME(RELAY_DEV_CMUX2),
    },
    [RELAY_DEV_CMUX3] = {
        .adp_dev.name = "CMUX3",
        .adp_dev.udi_id = UDI_CMUX_GPS_ID,
        .funcs.read_done_cb = UART_READ_DONE_CB_NAME(RELAY_DEV_CMUX3),
        .funcs.free_cb = UART_FREE_CB_NAME(RELAY_DEV_CMUX3),
        .funcs.msc_read_cb = UART_MSC_READ_CB_NAME(RELAY_DEV_CMUX3),
        .funcs.switch_mode_cb = UART_SWITCH_MODE_CB_NAME(RELAY_DEV_CMUX3),
        .funcs.water_line_cb = NULL,
        .funcs.ready_cb = UART_READY_CB_NAME(RELAY_DEV_CMUX3),
    },
};

void relay_uart_up_deliver(struct relay_uart_device *uart_dev, unsigned int cmd, void *buf, unsigned int len)
{
    int ret;

    if (uart_dev->adp_dev.opend == RELAY_OPEND_MAGIC) {
        uart_dev->stat.read_cb_sched++;
        if (uart_dev->cbs.read_cb != NULL) {
            ret = uart_dev->cbs.read_cb(uart_dev->adp_dev.private, cmd, buf, len);
            if (ret) {
                uart_dev->stat.read_done_fail++;
            }
        } else {
            uart_dev->stat.no_read_cb++;
        }
    } else {
        uart_dev->adp_dev.base_stat.port_not_open++;
    }
}

void relay_uart_switch_mode_cb(unsigned int minor_type)
{
    unsigned int mode = 0;
    struct relay_uart_device *uart_dev = NULL;

    if (minor_type >= RELAY_DEV_UART_MAX) {
        relay_err("minor_type:%d overflow\n", minor_type);
        return;
    }

    uart_dev = &g_uart_devices[minor_type];
    uart_dev->stat.switch_mode++;
    relay_uart_up_deliver(uart_dev, RELAY_UART_NOTIFY_SWITCH_MODE, (void*)&mode, sizeof(unsigned int));
}

void relay_uart_msc_read_cb(unsigned int minor_type, struct modem_msc_stru *msc)
{
    struct relay_uart_device *uart_dev = NULL;

    if (minor_type >= RELAY_DEV_UART_MAX) {
        relay_err("minor_type:%d overflow\n", minor_type);
        return;
    }

    uart_dev = &g_uart_devices[minor_type];
    uart_dev->stat.msc_read++;
    relay_uart_up_deliver(uart_dev, RELAY_UART_ACK_MSC, (void*)msc, sizeof(struct modem_msc_stru));
}

void relay_uart_ready_cb(unsigned int minor_type, port_ready_enum status)
{
    struct relay_uart_device *uart_dev = NULL;

    if (minor_type >= RELAY_DEV_UART_MAX) {
        relay_err("minor_type:%d overflow\n", minor_type);
        return;
    }

    uart_dev = &g_uart_devices[minor_type];
    uart_dev->stat.notify_ready++;
    relay_uart_up_deliver(uart_dev, RELAY_UART_NOTIFY_READY, (void*)&status, sizeof(status));
}

void relay_uart_water_line_cb(unsigned int minor_type, water_level_enum level)
{
    struct relay_uart_device *uart_dev = NULL;

    if (minor_type >= RELAY_DEV_UART_MAX) {
        relay_err("minor_type:%d overflow\n", minor_type);
        return;
    }

    uart_dev = &g_uart_devices[minor_type];
    relay_uart_up_deliver(uart_dev, RELAY_UART_NOTIFY_WATER_LINE, (void*)&level, sizeof(level));
}

void relay_uart_read_done_cb(unsigned int minor_type)
{
    struct relay_uart_device *uart_dev = NULL;
    dialup_uart_wr_async_info buf_info = { 0 };
    unsigned int udi_handler;
    int ret;

    if (minor_type >= RELAY_DEV_UART_MAX) {
        relay_err("minor_type:%d overflow\n", minor_type);
        return;
    }

    uart_dev = &g_uart_devices[minor_type];
    uart_dev->stat.read_done++;
    udi_handler = uart_dev->adp_dev.udi_handler;
    ret = mdrv_udi_ioctl(udi_handler, UART_IOCTL_GET_RD_BUFF, &buf_info);
    if (ret) {
        uart_dev->stat.read_buff_get_fail++;
        relay_err("minor_type:%d, get read buff fail ret:%d\n", minor_type, ret);
        return;
    }

    uart_dev->stat.read_done_len += buf_info.buffer_size;
    relay_uart_up_deliver(uart_dev, RELAY_UART_ACK_ASYNC, buf_info.p_buffer, buf_info.buffer_size);

    ret = mdrv_udi_ioctl(udi_handler, UART_IOCTL_RETURN_BUFF, &buf_info);
    if (ret) {
        uart_dev->stat.ret_buf_fail++;
        relay_err("minor_type:%d, return fail ret:%d\n", minor_type, ret);
    } else {
        uart_dev->stat.ret_buf++;
    }
}

void relay_uart_free_cb(unsigned int minor_type, char *addr, unsigned int size)
{
    struct relay_uart_device *uart_dev = NULL;

    if (minor_type >= RELAY_DEV_UART_MAX) {
        relay_err("minor_type:%d overflow\n", minor_type);
        return;
    }

    uart_dev = &g_uart_devices[minor_type];

    if (addr == NULL) {
        relay_err("para err\n");
        return;
    }

    if (uart_dev->cbs.free_cb) {
        uart_dev->cbs.free_cb(addr); /* diag port reture buff */
    } else {
        kfree(addr); /* vcom port free direct */
    }

    uart_dev->stat.write_done++;
    uart_dev->stat.write_done_len += size;
    return;
}

static int relay_uart_set_udi_ioctl(struct relay_uart_device *uart_dev)
{
    int ret = 0;

    /* set usb ready cb */
    ret = mdrv_udi_ioctl(uart_dev->adp_dev.udi_handler, UART_IOCTL_SET_READY_CB, uart_dev->funcs.ready_cb);
    if (ret) {
        relay_err("dev %s, set read cb failed\n", uart_dev->adp_dev.name);
        return ret;
    }

    /* set usb read cb */
    ret = mdrv_udi_ioctl(uart_dev->adp_dev.udi_handler, UART_IOCTL_SET_READ_CB, uart_dev->funcs.read_done_cb);
    if (ret) {
        relay_err("dev %s, set read cb failed\n", uart_dev->adp_dev.name);
        return ret;
    }

    /* set write cb */
    ret = mdrv_udi_ioctl(uart_dev->adp_dev.udi_handler, UART_IOCTL_SET_FREE_CB, uart_dev->funcs.free_cb);
    if (ret) {
        relay_err("dev %s, set write cb failed\n", uart_dev->adp_dev.name);
        return ret;
    }

    /* set water cb */
    if (uart_dev->funcs.water_line_cb != NULL) {
        ret = mdrv_udi_ioctl(uart_dev->adp_dev.udi_handler, UART_IOCTL_SET_WATER_CB, uart_dev->funcs.water_line_cb);
        if (ret) {
            relay_err("dev %s, set water cb failed\n", uart_dev->adp_dev.name);
            return ret;
        }
    }

    /* set switch mode cb */
    ret = mdrv_udi_ioctl(uart_dev->adp_dev.udi_handler, UART_IOCTL_SWITCH_MODE_CB, uart_dev->funcs.switch_mode_cb);
    if (ret) {
        relay_err("dev %s, set switch mode cb failed\n", uart_dev->adp_dev.name);
        return ret;
    }

    /* set msc read cb */
    ret = mdrv_udi_ioctl(uart_dev->adp_dev.udi_handler, UART_IOCTL_SET_MSC_READ_CB, uart_dev->funcs.msc_read_cb);
    if (ret) {
        relay_err("dev %s, set msc read cb failed\n", uart_dev->adp_dev.name);
        return ret;
    }

    return ret;
}

int relay_uart_dev_stat_init(struct relay_uart_device *uart_dev)
{
    int ret;

    ret = memset_s(&uart_dev->stat, sizeof(uart_dev->stat), 0, sizeof(struct relay_uart_stat));
    if (ret) {
        relay_info("memset_s fail:%d\n", ret);
        return ret;
    }

    ret = memset_s(&uart_dev->adp_dev.base_stat, sizeof(uart_dev->adp_dev.base_stat), 0, sizeof(struct relay_base_stat));
    if (ret) {
        relay_info("memset_s fail:%d\n", ret);
        return ret;
    }
    return 0;
}

int relay_uart_dev_open(unsigned int minor_type)
{
    struct relay_uart_device *uart_dev = NULL;
    struct udi_open_param param;
    unsigned int udi_id;
    int ret = -EINVAL;
    int handler;

    if (minor_type >= RELAY_DEV_UART_MAX) {
        relay_err("minor_type:%d overflow\n", minor_type);
        return ret;
    }

    uart_dev = &g_uart_devices[minor_type];
    uart_dev->stat.open++;
    if (uart_dev->adp_dev.opend == RELAY_OPEND_MAGIC) {
        relay_err("minor_type:%d has opend\n", minor_type);
        uart_dev->stat.open_again++;
        return ret;
    }

    udi_id = uart_dev->adp_dev.udi_id;
    param.devid = udi_id;
    param.private = 0;
    handler = mdrv_udi_open(&param);
    if (handler == UDI_INVALID_HANDLE) {
        relay_info("udi_id:%d open fail:%d\n", udi_id, ret);
        uart_dev->stat.open_fail++;
        return ret;
    }

    uart_dev->adp_dev.udi_handler = handler;
    ret = relay_uart_set_udi_ioctl(uart_dev);
    if (ret) {
        relay_info("minor_type:%d set ioctl fail:%d\n", minor_type, ret);
        uart_dev->stat.ioctl_fail++;
        return ret;
    }

    uart_dev->adp_dev.opend = RELAY_OPEND_MAGIC;
    uart_dev->stat.open_succ++;
    relay_info("minor_type:%d open succ\n", minor_type);
    return ret;
}

int relay_uart_dev_close(unsigned int minor_type)
{
    struct relay_uart_device *uart_dev = NULL;
    int ret;
    int handle;

    if (minor_type >= RELAY_DEV_UART_MAX) {
        relay_err("minor_type:%d overflow\n", minor_type);
        return -EINVAL;
    }

    uart_dev = &g_uart_devices[minor_type];
    uart_dev->stat.close++;
    if (uart_dev->adp_dev.opend != RELAY_OPEND_MAGIC) {
        relay_info("minor_type:%d not open\n", minor_type);
        uart_dev->stat.close_again++;
        return -EINVAL;
    }

    handle = uart_dev->adp_dev.udi_handler;
    ret = mdrv_udi_close(handle);
    if (ret) {
        relay_info("minor_type:%d handler:%d close fail:%d\n", minor_type, handle, ret);
        uart_dev->stat.close_fail++;
        return ret;
    }

    uart_dev->adp_dev.udi_handler = UDI_INVALID_HANDLE;
    uart_dev->adp_dev.opend = 0;
    uart_dev->stat.close_succ++;
    return ret;
}

int relay_uart_dev_write(unsigned int minor_type, char *addr, unsigned int size)
{
    return 0;
}

int relay_uart_dev_write_async(struct relay_uart_device *uart_dev, int handle, void *para)
{
    int ret;

    uart_dev->stat.write++;
    ret = mdrv_udi_ioctl(handle, UART_IOCTL_WRITE_ASYNC, para);
    if (ret < 0) {
        uart_dev->stat.write_fail++;
    }
    return ret;
}

int relay_uart_set_flow_ctrl(unsigned int minor_type, int handle, void *para, unsigned int para_size)
{
    if (para_size != sizeof(uart_flow_ctrl_info)) {
        relay_err("minor_type:%d, para size error\n", minor_type);
        return -EINVAL;
    }
    return mdrv_udi_ioctl(handle, UART_IOCTL_SET_FLOW_CONTROL, para);
}

int relay_uart_write_wlen(unsigned int minor_type, int handle, void *para, unsigned int para_size)
{
    if (para_size != sizeof(uart_wlen_enum)) {
        relay_err("minor_type:%d, para size error\n", minor_type);
        return -EINVAL;
    }
    return mdrv_udi_ioctl(handle, UART_IOCTL_SET_WLEN, para);
}

int relay_uart_write_stp(unsigned int minor_type, int handle, void *para, unsigned int para_size)
{
    if (para_size != sizeof(uart_stp2_enum)) {
        relay_err("minor_type:%d, para size error\n", minor_type);
        return -EINVAL;
    }
    return mdrv_udi_ioctl(handle, UART_IOCTL_SET_STP2, para);
}

int relay_uart_write_eps(unsigned int minor_type, int handle, void *para, unsigned int para_size)
{
    if (para_size != sizeof(uart_parity_enum)) {
        relay_err("minor_type:%d, para size error\n", minor_type);
        return -EINVAL;
    }
    return mdrv_udi_ioctl(handle, UART_IOCTL_SET_EPS, para);
}

int relay_uart_write_baud(unsigned int minor_type, int handle, void *para, unsigned int para_size)
{
    if (para_size != sizeof(unsigned int)) {
        relay_err("minor_type:%d, para size error\n", minor_type);
        return -EINVAL;
    }
    return mdrv_udi_ioctl(handle, UART_IOCTL_SET_BAUD, para);
}

int relay_uart_set_cmux_info(unsigned int minor_type, int handle, void *para, unsigned int para_size)
{
    if (para_size != sizeof(uart_cmux_info)) {
        relay_err("minor_type:%d, para size error\n", minor_type);
        return -EINVAL;
    }
    return mdrv_udi_ioctl(handle, UART_IOCTL_SET_CMUX_INFO, para);
}

int relay_uart_set_cmux_dlc(unsigned int minor_type, int handle, void *para, unsigned int para_size)
{
    if (para_size != sizeof(unsigned int)) {
        relay_err("minor_type:%d, para size error\n", minor_type);
        return -EINVAL;
    }
    return mdrv_udi_ioctl(handle, UART_IOCTL_SET_CMUX_DLC, para);
}

int relay_uart_enable_escape_detect(unsigned int minor_type, int handle, void *para, unsigned int para_size)
{
    if (para_size != sizeof(unsigned int)) {
        relay_err("minor_type:%d, para size error\n", minor_type);
        return -EINVAL;
    }
    return mdrv_udi_ioctl(handle, UART_IOCTL_ENABLE_ESCAPE_DETECT, para);
}

int relay_uart_write_msc(unsigned int minor_type, int handle, void *para, unsigned int para_size)
{
    if (para_size != sizeof(struct modem_msc_stru)) {
        relay_err("minor_type:%d, para size error\n", minor_type);
        return -EINVAL;
    }
    return mdrv_udi_ioctl(handle, UART_IOCTL_MSC_WRITE_CMD, para);
}

int relay_uart_dev_ioctl(unsigned int minor_type, unsigned int cmd, void *para, unsigned int para_size)
{
    int handle;
    struct relay_uart_device *uart_dev = NULL;

    if (minor_type >= RELAY_DEV_UART_MAX || para == NULL) {
        relay_err("minor_type:%d, param err\n", minor_type);
        return -EINVAL;
    }

    uart_dev = &g_uart_devices[minor_type];
    if (uart_dev->adp_dev.opend != RELAY_OPEND_MAGIC) {
        relay_info("minor_type:%d, dev is not open\n", minor_type);
        uart_dev->stat.droped++;
        return -EINVAL;
    }

    handle = uart_dev->adp_dev.udi_handler;
    switch (cmd) {
        case RELAY_UART_REG_READ_CB:
            uart_dev->cbs.read_cb = (mci_uart_read_cb)para;
            return 0;
        case RELAY_UART_REG_WRITE_CB:
            uart_dev->cbs.free_cb = (mci_uart_free_cb)para;
            return 0;
        case RELAY_UART_WRITE_ASYNC: {
            dialup_uart_wr_async_info info;
            info.p_buffer = para;
            info.buffer_size = para_size;
            return relay_uart_dev_write_async(uart_dev, handle, (void*)&info);
        }
        case RELAY_UART_WRITE_MSC:
            return relay_uart_write_msc(minor_type, handle, para, para_size);
        case RELAY_UART_WRITE_FLOW_CTRL:
            return relay_uart_set_flow_ctrl(minor_type, handle, para, para_size);
        case RELAY_UART_WRITE_WLEN:
            return relay_uart_write_wlen(minor_type, handle, para, para_size);
        case RELAY_UART_WRITE_STP:
            return relay_uart_write_stp(minor_type, handle, para, para_size);
        case RELAY_UART_WRITE_EPS:
            return relay_uart_write_eps(minor_type, handle, para, para_size);
        case RELAY_UART_WRITE_BAUD:
            return relay_uart_write_baud(minor_type, handle, para, para_size);
        case RELAY_UART_SET_CMUX_INFO:
            return relay_uart_set_cmux_info(minor_type, handle, para, para_size);
        case RELAY_UART_SET_CMUX_DLC:
            return relay_uart_set_cmux_dlc(minor_type, handle, para, para_size);
        case RELAY_UART_ENABLE_CMUX:
            return mdrv_udi_ioctl(handle, UART_IOCTL_ENABLE_CMUX, para);
        case RELAY_UART_ENABLE_ESCAPE_DETECT:
            return relay_uart_enable_escape_detect(minor_type, handle, para, para_size);
        case RELAY_UART_QUERY_READY:
            return mdrv_udi_ioctl(handle, UART_IOCTL_QUERY_READY, para);
        default:
            relay_err("cmd is error\n");
            return -EINVAL;
    }
}

int relay_uart_reg_enablecb(unsigned int minor_type, relay_enable_cb func, void *private)
{
    struct relay_uart_device *uart_dev = NULL;

    if (minor_type >= RELAY_DEV_UART_MAX || func == NULL) {
        relay_err("minor_type:%d param err\n", minor_type);
        return -EINVAL;
    }

    uart_dev = &g_uart_devices[minor_type];
    uart_dev->cbs.enable_cb = func;
    uart_dev->adp_dev.private = private;
    if (uart_dev->adp_dev.b_enable == RELAY_ENABLE_MAGIC) {
        uart_dev->cbs.enable_cb(private);
    }
    return 0;
}

int relay_uart_reg_disablecb(unsigned int minor_type, relay_disable_cb func)
{
    struct relay_uart_device *uart_dev = NULL;

    if (minor_type >= RELAY_DEV_UART_MAX || func == NULL) {
        relay_err("minor_type:%d param err\n", minor_type);
        return -EINVAL;
    }

    uart_dev = &g_uart_devices[minor_type];
    uart_dev->cbs.disable_cb = func;
    return 0;
}

void relay_uart_enable(void)
{
    struct relay_uart_device *uart_dev = NULL;

    uart_dev = &g_uart_devices[RELAY_DEV_HSUART];
    uart_dev->stat.enable++;
    if (uart_dev->cbs.enable_cb != NULL) {
        uart_dev->cbs.enable_cb(uart_dev->adp_dev.private);
    } else {
        uart_dev->stat.enable_no_cb++;
        relay_info("enable cb is null\n");
    }
    uart_dev->adp_dev.b_enable = RELAY_ENABLE_MAGIC;
}

void relay_cmux_enable(void)
{
    unsigned int minor_type;
    struct relay_uart_device *uart_dev = NULL;

    for (minor_type = RELAY_DEV_CMUX0; minor_type <= RELAY_DEV_CMUX3; minor_type++) {
        uart_dev = &g_uart_devices[minor_type];
        uart_dev->stat.enable++;
        if (uart_dev->cbs.enable_cb != NULL) {
            uart_dev->cbs.enable_cb(uart_dev->adp_dev.private);
        } else {
            uart_dev->stat.enable_no_cb++;
            relay_info("enable cb is null\n");
        }
        uart_dev->adp_dev.b_enable = RELAY_ENABLE_MAGIC;
    }
}

void relay_uart_init(void)
{
    int ret;
    unsigned int type;
    struct relay_uart_device *uart_dev = NULL;

    bsp_dialup_register_enable_cb(relay_uart_enable);
#ifdef CONFIG_CMUX
    bsp_cmux_register_enable_cb(relay_cmux_enable);
#endif

    for (type = 0; type < RELAY_DEV_UART_MAX; type++) {
        uart_dev = &g_uart_devices[type];
        ret = relay_uart_dev_stat_init(uart_dev);
        if (ret < 0) {
            return;
        }
    }
}

void relay_uart_info_show(unsigned int minor_type)
{
    struct relay_uart_device *uart_dev = NULL;

    if (minor_type >= RELAY_DEV_ACM_MAX) {
        relay_err("minor_type: %d overflow\n", minor_type);
        return;
    }
    uart_dev = &g_uart_devices[minor_type];

    relay_err("read_cb             : %s\n", uart_dev->cbs.read_cb ? "good" : "null");
    relay_err("free_cb             : %s\n", uart_dev->cbs.free_cb ? "good" : "null");
    relay_err("enable_cb           : %s\n", uart_dev->cbs.enable_cb ? "good" : "null");
    relay_err("disable_cb          : %s\n", uart_dev->cbs.disable_cb ? "good" : "null");

    relay_err("open                : %u\n", uart_dev->stat.open);
    relay_err("open_again          : %u\n", uart_dev->stat.open_again);
    relay_err("open_succ           : %u\n", uart_dev->stat.open_succ);
    relay_err("open_fail           : %u\n", uart_dev->stat.open_fail);
    relay_err("ioctl_fail          : %u\n", uart_dev->stat.ioctl_fail);
    relay_err("close               : %u\n", uart_dev->stat.close);
    relay_err("close_again         : %u\n", uart_dev->stat.close_again);
    relay_err("close_succ          : %u\n", uart_dev->stat.close_succ);
    relay_err("close_fail          : %u\n", uart_dev->stat.close_fail);

    relay_err("enable              : %u\n", uart_dev->stat.enable);
    relay_err("enable_no_cb        : %u\n", uart_dev->stat.enable_no_cb);

    relay_err("droped num          : %u\n", uart_dev->stat.droped);
    relay_err("ret_buf             : %u\n", uart_dev->stat.ret_buf);
    relay_err("ret_buf_fail        : %u\n", uart_dev->stat.ret_buf_fail);

    relay_err("read_done           : %u\n", uart_dev->stat.read_done);
    relay_err("read_done_len       : %u\n", uart_dev->stat.read_done_len);
    relay_err("read_done_fail      : %u\n", uart_dev->stat.read_done_fail);
    relay_err("read_buff_get_fail  : %u\n", uart_dev->stat.read_buff_get_fail);
    relay_err("read_cb_sched       : %u\n", uart_dev->stat.read_cb_sched);
    relay_err("write               : %u\n", uart_dev->stat.write);
    relay_err("write_len           : %u\n", uart_dev->stat.write_len);
    relay_err("write_fail          : %u\n", uart_dev->stat.write_fail);
    relay_err("write_done          : %u\n", uart_dev->stat.write_done);
    relay_err("write_done_len      : %u\n", uart_dev->stat.write_done_len);
    relay_err("msc_read            : %u\n", uart_dev->stat.msc_read);
    relay_err("notify_ready        : %u\n", uart_dev->stat.notify_ready);
    relay_err("switch_mode         : %u\n", uart_dev->stat.switch_mode);
}

