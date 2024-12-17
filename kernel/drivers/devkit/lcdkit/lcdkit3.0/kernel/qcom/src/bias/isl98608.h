/*
 * isl98608.h
 *
 * lcdkit backlight function for lcd driver
 *
 * Copyright (c) 2016-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef __ISL98608_H
#define __ISL98608_H

#define DTS_COMP_ISL98608 "isl98608_phy"

#define ISL98608_VOL_55        0x00     // 5.5V
#define ISL98608_VOL_56        0x02     // 5.6V
#define ISL98608_VOL_57        0x04     // 5.7V
#define ISL98608_VOL_58        0x06     // 5.8V
#define ISL98608_VOL_59        0x08     // 5.9V
#define ISL98608_VOL_60        0x0A     // 6.0V

#define ISL98608_VBST_VOL_64   0x0F     // 6.4V
#define ISL98608_REG_ENABLE_VAL 0x27

#define ISL98608_REG_ENABLE    0x05
#define ISL98608_REG_VBST      0x06
#define ISL98608_REG_VPOS      0x09
#define ISL98608_REG_VNEG      0x08

#define ISL98608_REG_VOL_MASK  0x1F
#define ISL98608_SHUTDOWN_BIT  (1 << 5)
#define ISL98608_VP_BIT        (1 << 2)
#define ISL98608_VN_BIT        (1 << 1)
#define ISL98608_VBST_BIT      (1 << 0)

#define VSP_ENABLE 1
#define VSN_ENABLE 1
#define VSP_DISABLE 0
#define VSN_DISABLE 0

struct isl98608_voltage {
	u32 voltage;
	int value;
};

struct isl98608_configure_info {
	char *lcd_name;
	int enable_cmd;
	int vbst_cmd;
	int vpos_cmd;
	int vneg_cmd;
	int vsp_enable;
	int vsn_enable;
};

struct isl98608_device_info {
	struct device *dev;
	struct i2c_client *client;
	struct isl98608_configure_info config;
};

bool check_isl98608_device(void);
int isl98608_set_voltage(void);

int __init isl98608_module_init(void);
void __exit isl98608_exit(void);

#endif
