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

#include "relay_core.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <mdrv_usb.h>
#include <product_config.h>
#include <adrv_usb.h>
#include <bsp_relay.h>
#include "relay_acm.h"


#define THIS_MODU mod_relay
#define RELAY_GET_MAJOR_TYPE(dev_type) (((dev_type)&0xff00) >> 8)
#define RELAY_GET_MINOR_TYPE(dev_type) (((dev_type)&0x00ff))

#define RELAY_PARSE_DEV_TYPE(dev_type, major_type, minor_type) \
    do {                                                       \
        (major_type) = RELAY_GET_MAJOR_TYPE((dev_type));       \
        (minor_type) = RELAY_GET_MINOR_TYPE((dev_type));       \
    } while (0)

struct vdev_relay_func {
    int (*reg_enablecb)(unsigned int minor_type, relay_enable_cb func, void *private);
    int (*reg_disablecb)(unsigned int minor_type, relay_enable_cb func);
    int (*relay_open)(unsigned int minor_type);
    int (*relay_close)(unsigned int minor_type);
    int (*relay_write)(unsigned int minor_type, char *addr, unsigned int size);
    int (*relay_ioctl)(unsigned int minor_type, unsigned int cmd, void *para, unsigned int para_size);
};

struct vdev_relay_func g_vdev_relay_funcs[RELAY_MAX_TYPE] = {
    [RELAY_ACM_TYPE] = {
        .reg_enablecb = relay_acm_reg_enablecb,
        .reg_disablecb = relay_acm_reg_disablecb,
        .relay_open = relay_acm_dev_open,
        .relay_close = relay_acm_dev_close,
        .relay_write = relay_acm_dev_write,
        .relay_ioctl = relay_acm_dev_ioctl,
    },
};

void relay_usb_enable(void)
{
    unsigned int minor_type;

    for (minor_type = RELAY_DEV_ACM_CTRL; minor_type <= RELAY_DEV_SKYTONE; minor_type++) {
        relay_acm_enable(minor_type);
    }
    return;
}

void relay_usb_disable(void)
{
    unsigned int minor_type;

    for (minor_type = RELAY_DEV_ACM_CTRL; minor_type <= RELAY_DEV_SKYTONE; minor_type++) {
        relay_acm_disable(minor_type);
    }
    return;
}


int relay_reg_enablecb(unsigned int dev_type, relay_enable_cb func, void *private)
{
    unsigned int major_type;
    unsigned int minor_type;

    RELAY_PARSE_DEV_TYPE(dev_type, major_type, minor_type);
    if (major_type < RELAY_MAX_TYPE && g_vdev_relay_funcs[major_type].reg_enablecb != NULL) {
        return g_vdev_relay_funcs[major_type].reg_enablecb(minor_type, func, private);
    }
    relay_err("major type: %d overflow\n", major_type);
    return 0;
}

int relay_reg_disablecb(unsigned int dev_type, relay_disable_cb func)
{
    unsigned int major_type;
    unsigned int minor_type;

    RELAY_PARSE_DEV_TYPE(dev_type, major_type, minor_type);
    if (major_type < RELAY_MAX_TYPE && g_vdev_relay_funcs[major_type].reg_disablecb != NULL) {
        return g_vdev_relay_funcs[major_type].reg_disablecb(minor_type, func);
    }
    relay_err("major type: %d overflow\n", major_type);
    return 0;
}

int relay_dev_open(unsigned int dev_type)
{
    unsigned int major_type;
    unsigned int minor_type;

    RELAY_PARSE_DEV_TYPE(dev_type, major_type, minor_type);
    if (major_type < RELAY_MAX_TYPE && g_vdev_relay_funcs[major_type].relay_open != NULL) {
        return g_vdev_relay_funcs[major_type].relay_open(minor_type);
    }
    relay_err("major type: %d overflow\n", major_type);
    return 0;
}

int relay_dev_close(unsigned int dev_type)
{
    unsigned int major_type;
    unsigned int minor_type;

    RELAY_PARSE_DEV_TYPE(dev_type, major_type, minor_type);
    if (major_type < RELAY_MAX_TYPE && g_vdev_relay_funcs[major_type].relay_close != NULL) {
        return g_vdev_relay_funcs[major_type].relay_close(minor_type);
    }
    relay_err("major type: %d overflow\n", major_type);
    return 0;
}

int relay_dev_write(unsigned int dev_type, char *addr, unsigned int size)
{
    unsigned int major_type;
    unsigned int minor_type;

    RELAY_PARSE_DEV_TYPE(dev_type, major_type, minor_type);
    if (major_type < RELAY_MAX_TYPE && g_vdev_relay_funcs[major_type].relay_write != NULL) {
        return g_vdev_relay_funcs[major_type].relay_write(minor_type, addr, size);
    }
    relay_err("major type: %d overflow\n", major_type);
    return 0;
}

int relay_dev_ioctl(unsigned int dev_type, unsigned int cmd, void *para, unsigned int para_size)
{
    unsigned int major_type;
    unsigned int minor_type;

    RELAY_PARSE_DEV_TYPE(dev_type, major_type, minor_type);
    if (major_type < RELAY_MAX_TYPE && g_vdev_relay_funcs[major_type].relay_ioctl != NULL) {
        return g_vdev_relay_funcs[major_type].relay_ioctl(minor_type, cmd, para, para_size);
    }
    relay_err("major type: %d overflow\n", major_type);
    return 0;
}

static int relay_init(void)
{
    relay_err("[init] start\n");


    if (relay_acm_init() < 0) {
        return -1;
    }

    mdrv_usb_reg_enablecb(relay_usb_enable);
    mdrv_usb_reg_disablecb(relay_usb_disable);

    relay_err("[init] ok\n");
    return 0;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
static int usb_relay_init(void)
{
    return relay_init();
}
#endif

int bsp_usb_relay_init(void)
{
    return relay_init();
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(usb_relay_init);
#endif
