/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: define for vol keypad
 * Create: 2022-8-17
 */

#ifndef _KEYPAD_H
#define _KEYPAD_H

#ifdef CONFIG_VOLUMEKEY_COMBINATION
#define VOL_DOWN_BIT  (0)
#define VOL_UP_BIT    (1)

#define VOL_UPDOWN_PRESS   ((1<<VOL_DOWN_BIT) | (1<<VOL_UP_BIT))
#ifdef CONFIG_KEYBOARD_PMIC_VOLUME_KEY
extern void pmic_gpio_key_vol_updown_press_set_zero(void);
extern int pmic_gpio_key_vol_updown_press_get(void);
extern int is_pmic_gpio_key_vol_updown_pressed(void);
#endif
extern void gpio_key_vol_updown_press_set_zero(void);
extern int gpio_key_vol_updown_press_get(void);
extern int is_gpio_key_vol_updown_pressed(void);
#endif
#endif
