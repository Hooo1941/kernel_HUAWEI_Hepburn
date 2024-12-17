/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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
#ifndef _PINCTRL_DRIVER_H_
#define _PINCTRL_DRIVER_H_

#define PINCONFIG_NF_VAL 0xffff
#define PINCTRL_SHMEM_SIZE 0x1800
#define PIN_NAME_LEN 10

enum {
    MODEM_PIN_INIT = 0,
    MODEM_PIN_NORMAL,
    MODEM_PIN_POWERDOWN,
    MODEM_PIN_MODE_BUTT,
};

/**
  * @brief modem管脚控制组件枚举
  * 同步修改include/drv/ccore/fusion/mdrv_pinctrl.h
  */
enum modem_pin_ctrl_module {
    MODEM_PIN_CP_DRV = 0,  /**< modem底软控制*/
    MODEM_PIN_CP_FESS,  /**< FESS控制*/
    MODEM_PIN_CP_BUTT,
};

struct pinctrl {
    unsigned int gpio;
    unsigned int iomg1_off;
    unsigned int iomg1_val;
    unsigned int iocg_off;
    unsigned int iocg_val;
    unsigned int gpio_dir;
    unsigned int gpio_val;
    unsigned int iomg2_off;
    unsigned int iomg2_val;
    unsigned int reserve[3];
};

struct pinctrl_property {
    unsigned int gpio_grp;
    unsigned int gpio_bit;
    unsigned int iomg1_off;
    unsigned int iomg1_val;
    unsigned int iocg_off;
    unsigned int iocg_val;
    unsigned int gpio_dir;
    unsigned int gpio_val;
};

struct pinctrl_property_extend_info {
    struct pinctrl_property base;
    unsigned int iomg2_off;
    unsigned int iomg2_val;
};

struct pinctrl_property_ops {
    unsigned int extend_enabled;
    unsigned int max_pins;
    unsigned int element_size;
    void (*data_convert)(struct pinctrl *pctl, const struct pinctrl_property *prop, unsigned int len);
};

struct pinctrl_head {
    unsigned int offset;
    unsigned int size;
};

struct modem_pinctrl_head {
    u32 pinctrl_module_count;
    struct pinctrl_head head[MODEM_PIN_CP_BUTT][MODEM_PIN_MODE_BUTT];
};

int modem_pintrl_dts_parse(void *fdt);

#endif
