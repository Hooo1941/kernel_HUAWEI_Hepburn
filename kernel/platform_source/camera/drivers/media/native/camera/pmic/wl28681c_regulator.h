/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * Description: wl28681c driver
 */
#ifndef __WL28681C_REGISTERS_H__
#define __WL28681C_REGISTERS_H__

#define VERSION 0.3
#define wl28681c_err(reg, message, ...) \
	pr_err("%s: " message, (reg)->rdesc->name, ##__VA_ARGS__)
#define wl28681c_debug(reg, message, ...) \
	pr_debug("%s: " message, (reg)->rdesc->name, ##__VA_ARGS__)

/* Registers */
#define WL28681C_REG_NUM   (WL28681C_MINT3-WL28681C_PRODUCT_ID+1)

#define WL28681C_REG_CHIPID                     0x00
#define WL28681C_PRODUCT_ID                     0x00
#define WL28681C_SILICON_REV_ID                 0x01
#define WL28681C_IOUT                           0x02
#define WL28681C_LDO_EN                         0x03
#define WL28681C_LDO1_VOUT                      0x04
#define WL28681C_LDO2_VOUT                      0x05
#define WL28681C_LDO3_VOUT                      0x06
#define WL28681C_LDO4_VOUT                      0x07
#define WL28681C_LDO5_VOUT                      0x08
#define WL28681C_LDO6_VOUT                      0x09
#define WL28681C_LDO7_VOUT                      0x0a
#define WL28681C_LDO1_LDO2_SEQ                  0x0b
#define WL28681C_LDO3_LDO4_SEQ                  0x0c
#define WL28681C_LDO5_LDO6_SEQ                  0x0d
#define WL28681C_LDO7_SEQ                       0x0e
#define WL28681C_SEQ_STATUS                     0x0f
#define WL28681C_DISCHARGE_RESISTORS            0x10
#define WL28681C_SOFT_RESET                     0x11
#define WL28681C_I2C_ADDR                       0x12
#define WL28681C_LDO_COMP0                      0x13
#define WL28681C_LDO_COMP1                      0x14
#define WL28681C_INTERRUPT1                     0x15
#define WL28681C_INTERRUPT2                     0x16
#define WL28681C_INTERRUPT3                     0x17
#define WL28681C_STATUS1                        0x18
#define WL28681C_STATUS2                        0x19
#define WL28681C_STATUS3                        0x1a
#define WL28681C_STATUS4                        0x1b
#define WL28681C_MINT1                          0x1c
#define WL28681C_MINT2                          0x1d
#define WL28681C_MINT3                          0x1e

#define WL28681C_CHIP_ID_VAL                    0x0D
#define WL28681C_SLAVE_ADDRESS                  0x35

#define CHIP_ENABLE_BIT     BIT(7)
#define MAX_REG_NAME        20
#define WL28681C_VSEL_SHIFT  0
#define WL28681C_VSEL_MASK   (0xff << 0)

enum wl28681c_regulators {
	WL28681C_REGULATOR_LDO1 = 0,
	WL28681C_REGULATOR_LDO2,
	WL28681C_REGULATOR_LDO3,
	WL28681C_REGULATOR_LDO4,
	WL28681C_REGULATOR_LDO5,
	WL28681C_REGULATOR_LDO6,
	WL28681C_REGULATOR_LDO7,
	WL28681C_MAX_REGULATORS,
};

struct regulator_dev *wl28681c_get_rdev(int index);

int wl28681c_regulator_enable_warpper(struct regulator_dev *rdev);
int wl28681c_regulator_disable_warpper(struct regulator_dev *rdev);
int wl28681c_regulator_is_enabled_warpper(struct regulator_dev *rdev);
int wl28681c_regulator_set_voltage_warpper(struct regulator_dev *rdev, int min_uv, int max_uv, unsigned int *selector);
int wl28681c_regulator_get_voltage_warpper(struct regulator_dev *rdev);

int wl28681c_regulator_i2c_init(void);
void wl28681c_regulator_i2c_exit(void);

#endif
/* __wl28681c_REGISTERS_H__ */
