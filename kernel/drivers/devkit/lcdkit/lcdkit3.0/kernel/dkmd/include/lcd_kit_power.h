/*
 * lcd_kit_power.h
 *
 * lcdkit power function for lcd driver head file
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#ifndef __LCD_KIT_POWER_H_
#define __LCD_KIT_POWER_H_
#include "lcd_kit_utils.h"

/*
 * macro
 */
/* vci */
#define VCI_NAME  "vci"
/* iovcc */
#define IOVCC_NAME "iovcc"
/* vdd */
#define VDD_NAME  "vdd"
/* gpio */
#define GPIO_NAME "gpio"

/* dtype for gpio */
enum {
	DTYPE_GPIO_REQUEST,
	DTYPE_GPIO_FREE,
	DTYPE_GPIO_INPUT,
	DTYPE_GPIO_OUTPUT,
};

enum {
	WAIT_TYPE_US = 0,
	WAIT_TYPE_MS,
};

enum gpio_operator {
	GPIO_REQ,
	GPIO_FREE,
	GPIO_HIGH,
	GPIO_LOW,
};

/* dtype for vcc */
enum {
	DTYPE_VCC_GET,
	DTYPE_VCC_PUT,
	DTYPE_VCC_ENABLE,
	DTYPE_VCC_DISABLE,
	DTYPE_VCC_SET_VOLTAGE,
	DTYPE_VCC_GET_VOLTAGE,
};

/* gpio desc */
struct gpio_desc {
	int dtype;
	int waittype;
	int wait;
	char *label;
	unsigned int *gpio;
	int value;
};

/* vcc desc */
struct vcc_desc {
	int dtype;
	char *id;
	struct regulator **regulator;
	int min_uv;
	int max_uv;
	int waittype;
	int wait;
};

struct gpio_power_arra {
	enum gpio_operator oper;
	unsigned int num;
	struct gpio_desc *cm;
};

int lcd_kit_vcc_cmds_tx(struct platform_device *pdev, struct vcc_desc *cmds, int cnt);
int lcd_kit_pmu_ctrl(int panel_id, uint32_t type, uint32_t enable);
int lcd_kit_gpio_tx(int panel_id, unsigned int type, unsigned int op);
int lcd_kit_power_finit(int panel_id, struct platform_device *pdev);
int lcd_kit_power_init(int panel_id, struct platform_device *pdev);
unsigned int lcd_kit_get_pinid(unsigned int gpio);
unsigned int lcd_kit_get_ext_pinid(struct device_node *np, unsigned int poweric_det_support, unsigned int poweric_gpio);
int lcd_kit_gpio_cmds_tx(struct gpio_desc *cmds, int cnt);
#ifdef LCD_FACTORY_MODE
int lcd_kit_dynamic_ctrl_vdd(int panel_id, uint32_t volt_mode);
int lcd_kit_dynamic_get_vdd(int panel_id);
#endif
#endif
