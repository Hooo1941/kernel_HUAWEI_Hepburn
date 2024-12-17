/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
* Description: lcdkit parse function for lcdkit
* Create: 2023-06-13
*/

#ifndef _LCD_KIT_BRIDGE_H_
#define _LCD_KIT_BRIDGE_H_

#define BRIDGE_DISABLE 0
#define BRIDGE_ENABLE  1
#define BRIDGE_REFCLK_ON  2
#define BRIDGE_REFCLK_OFF  3

struct lcd_kit_bridge_ops {
	void (*pre_enable)(void);
	void (*enable)(void);
	void (*disable)(void);
	void (*refclk_on)(void);
	void (*refclk_off)(void);
};

/* function declare */
struct lcd_kit_bridge_ops *lcd_kit_get_bridge_ops(void);
int lcd_kit_bridge_register(struct lcd_kit_bridge_ops *ops);
int lcd_kit_bridge_unregister(struct lcd_kit_bridge_ops *ops);
#endif
