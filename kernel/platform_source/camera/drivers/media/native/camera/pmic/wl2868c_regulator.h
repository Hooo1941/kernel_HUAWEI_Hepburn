/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * Description: wl2868c driver
 */
#ifndef __WL2868C_REGISTERS_H__
#define __WL2868C_REGISTERS_H__

#define VERSION 0.3
#define wl2868c_err(reg, message, ...) \
	pr_err("%s: " message, (reg)->rdesc->name, ##__VA_ARGS__)
#define wl2868c_debug(reg, message, ...) \
	pr_debug("%s: " message, (reg)->rdesc->name, ##__VA_ARGS__)

/* Registers */
#define WL2868C_REG_NUM   (RESERVED_2-WL2868C_DEVICE_D1+1)

#define WL2868C_REG_CHIPID                     0x00
#define WL2868C_DEVICE_D1                      0x00
#define WL2868C_DEVICE_D2                      0x01
#define WL2868C_DISCHARGE_RESISTORS            0x02
#define WL2868C_LDO1_VOUT                      0x03
#define WL2868C_LDO2_VOUT                      0x04
#define WL2868C_LDO3_VOUT                      0x05
#define WL2868C_LDO4_VOUT                      0x06
#define WL2868C_LDO5_VOUT                      0x07
#define WL2868C_LDO6_VOUT                      0x08
#define WL2868C_LDO7_VOUT                      0x09
#define WL2868C_LDO1_LDO2_SEQ                  0x0a
#define WL2868C_LDO3_LDO4_SEQ                  0x0b
#define WL2868C_LDO5_LDO6_SEQ                  0x0c
#define WL2868C_LDO7_SEQ                       0x0d
#define WL2868C_LDO_EN                         0x0e
#define WL2868C_SEQ_STATUS                     0x0f
#define WL2868C_LDO1_STATUS                    0x10
#define WL2868C_LDO1_OCP_CTL                   0x11
#define WL2868C_LDO2_STATUS                    0x12
#define WL2868C_LDO2_OCP_CTL                   0x13
#define WL2868C_LDO3_STATUS                    0x14
#define WL2868C_LDO3_OCP_CTL                   0x15
#define WL2868C_LDO4_STATUS                    0x16
#define WL2868C_LDO4_OCP_CTL                   0x17
#define WL2868C_LDO5_STATUS                    0x18
#define WL2868C_LDO5_OCP_CTL                   0x19
#define WL2868C_LDO6_STATUS                    0x1a
#define WL2868C_LDO6_OCP_CTL                   0x1b
#define WL2868C_LDO7_STATUS                    0x1c
#define WL2868C_LDO7_OCP_CTL                   0x1d
#define WL2868C_REPROGRAMMABLE_I2C_ADDRESS     0x1e
#define RESERVED_1                             0x1f
#define INT_LATCHED_CLR                        0x20
#define INT_EN_SET                             0x21
#define INT_LATCHED_STS                        0x22
#define INT_PENDING_STS                        0x23
#define UVLO_CTL                               0x24
#define RESERVED_2                             0x25


#define CHIP_ENABLE_BIT     BIT(7)
#define MAX_REG_NAME        20
#define WL2868C_VSEL_SHIFT  0
#define WL2868C_VSEL_MASK   (0xff << 0)

enum wl2868c_regulators {
	WL2868C_REGULATOR_LDO1 = 0,
	WL2868C_REGULATOR_LDO2,
	WL2868C_REGULATOR_LDO3,
	WL2868C_REGULATOR_LDO4,
	WL2868C_REGULATOR_LDO5,
	WL2868C_REGULATOR_LDO6,
	WL2868C_REGULATOR_LDO7,
	WL2868C_MAX_REGULATORS,
};

struct regulator_dev *wl2868c_get_rdev(int index);

int wl2868c_regulator_enable_warpper(struct regulator_dev *rdev);
int wl2868c_regulator_disable_warpper(struct regulator_dev *rdev);
int wl2868c_regulator_is_enabled_warpper(struct regulator_dev *rdev);
int wl2868c_regulator_set_voltage_warpper(struct regulator_dev *rdev, int min_uv, int max_uv, unsigned int *selector);
int wl2868c_regulator_get_voltage_warpper(struct regulator_dev *rdev);

int wl2868c_regulator_i2c_init(void);
void wl2868c_regulator_i2c_exit(void);

#endif
/* __WL2868C_REGISTERS_H__ */
