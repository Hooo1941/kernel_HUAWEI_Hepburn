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
#include <bsp_print.h>
#include "vdev.h"

#define THIS_MODU mod_vcom

typedef enum {
    HSUART_BRIDGE_TYPE,
    PPP_BRIDGE_TYPE,
    PCUI_BRIDGE_TYPE,
    CMUX0_BRIDGE_TYPE,
    CMUX1_BRIDGE_TYPE,
    CMUX2_BRIDGE_TYPE,
    CMUX3_BRIDGE_TYPE,
    MAX_BRIDGE_TYPE
} channel_type;

#define VDEV_IOT_MAGIC 'k'
#define VDEV_IOT_CMD _IOW(VDEV_IOT_MAGIC, 0, channel_type)
#define VDEV_IOT_RAWDATA _IOW(VDEV_IOT_MAGIC, 1, channel_type)

struct vdev_bridge_config {
    unsigned int dst_channel; // 桥接目的通道
    unsigned int bridge_mode;  // 0关闭， 1打开
};
unsigned int g_bridge_mode[MAX_BRIDGE_TYPE] = {0};

static const unsigned int g_vdev_bridge_dst_chn_map[MAX_BRIDGE_TYPE] = {
    VCOM_HSUART_ID, // HSUART_BRIDGE_TYPE
    VCOM_ACM_MODEM, // PPP_BRIDGE_TYPE
    VCOM_ACM_AT,    // PCUI_BRIDGE_TYPE
    VCOM_CMUX0_ID,  // CMUX0_BRIDGE_TYPE
    VCOM_CMUX1_ID,  // CMUX1_BRIDGE_TYPE
    VCOM_CMUX2_ID,  // CMUX2_BRIDGE_TYPE
    VCOM_CMUX3_ID,  // CMUX2_BRIDGE_TYPE
};

static int vdev_bridge_get_dst_chn(channel_type type, unsigned int *chn_id)
{
    if (type >= MAX_BRIDGE_TYPE || chn_id == NULL) {
        VCOM_PRINT_ERROR("dst channel not support\n");
        return -1;
    }
    *chn_id = g_vdev_bridge_dst_chn_map[type];
    return 0;
}

long vdev_cdev_change_bridge_mode(struct vcom_hd *hd, unsigned int cmd, unsigned long arg)
{
    long ret;
    channel_type type;
    struct vdev_bridge_config cfg = {0};

    ret = copy_from_user(&type, (void*)(uintptr_t)arg, sizeof(channel_type));
    if (ret) {
        VCOM_PRINT_ERROR("copy fail, ret: %lu\n", ret);
        return -EINVAL;
    }

    switch (cmd) {
        case VDEV_IOT_CMD:
            cfg.bridge_mode = 0;
            break;
        case VDEV_IOT_RAWDATA:
            cfg.bridge_mode = 1;
            break;
        default:
            VCOM_PRINT_ERROR("cmd not support\n");
            return -EINVAL;
    }

    if (vdev_bridge_get_dst_chn(type, &cfg.dst_channel) != 0) {
        return -EINVAL;
    }

    if (cfg.bridge_mode == g_bridge_mode[type]) {
        VCOM_PRINT_ERROR("bridge type %d, mode not change\n", type);
        return -EINVAL;
    }

    g_bridge_mode[type] = cfg.bridge_mode;

    ret = vcom_ioctl(hd, &cfg, sizeof(cfg), NOTIFY_BRIDGE_MODE);
    if (ret < 0) {
        VCOM_PRINT_ERROR("ioctl fail\n");
    }
    return ret;
}
