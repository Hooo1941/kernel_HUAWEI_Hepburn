/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * Description: cam pmic dev driver
 */

#ifndef __ET5907_REGULATOR_H__
#define __ET5907_REGULATOR_H__

#include <linux/regulator/machine.h>

#define ET5907_REG_CONTROL        0x1E
#define ET5907_REG_CHIPID         0x00
#define ET5907_REG_VERID          0x01

/* 2 LDO_ILIMIT Register ----LDO Current Limit Selection. Default = 0x7F */
#define ET5907_REG_LDO_ILIMIT     0x02

/* LDOs Chip enable control register. Default = 0x00 */
#define ET5907_REG_LDO_EN         0x03

/* Regulator LDO VOLTAGE SET Registers VOUT =0.600V + [d*6mV] */
#define ET5907_REG_LDO1_VSET      0x04
#define ET5907_REG_LDO2_VSET      0x05

/* Regulator LDO VOLTAGE SET Registers VOUT ==1.200V + [d*10mV] */
#define ET5907_REG_LDO3_VSET      0x06
#define ET5907_REG_LDO4_VSET      0x07
#define ET5907_REG_LDO5_VSET      0x08
#define ET5907_REG_LDO6_VSET      0x09
#define ET5907_REG_LDO7_VSET      0x0A

/* Power sequence setting register. Default = 0x00 */
#define ET5907_REG_LDO12_SEQ      0x0B
#define ET5907_REG_LDO34_SEQ      0x0C
#define ET5907_REG_LDO56_SEQ      0x0D
#define ET5907_REG_LDO7_SEQ       0x0E

/* Power sequence setting and status register. */
#define ET5907_REG_SEQ_CTR        0x0F

/* Discharge Resistor Selection. Default = 0x7 */
#define ET5907_REG_LDO_DIS        0x10
#define ET5907_REG_RESET          0x11
#define ET5907_REG_I2C_ADDR       0x12
#define ET5907_REG_UVP_INT        0x15
#define ET5907_REG_OCP_INT        0x16

/* bit definitions of  TSD AND UVLO INTERRUPT REGISTER */
#define ET5907_REG_TSD_UVLO_INT   0x17
#define ET5907_TSD_UVLO_LDO12     (1 << 0)
#define ET5907_TSD_UVLO_LDO34     (1 << 1)
#define ET5907_TSD_UVLO_LDO5      (1 << 2)
#define ET5907_TSD_UVLO_LDO6      (1 << 3)
#define ET5907_TSD_UVLO_LDO7      (1 << 4)
#define ET5907_TSD_UVLO_VSYS      (1 << 5)
#define ET5907_TSD_TSD_WRN        (1 << 6)
#define ET5907_TSD_TSD            (1 << 7)
#define ET5907_REG_UVP_STAU       0x18
#define ET5907_REG_OCP_STAU       0x19
#define ET5907_REG_TSD_UVLO_STAU  0x1A
#define ET5907_REG_SUSD_STAU      0x1B
#define ET5907_REG_UVP_INTMA      0x1C
#define ET5907_REG_OCP_INTMA      0x1D
#define ET5907_REG_TSD_UVLO_INTMA 0x1E

/* bit definitions of  TSD AND UVLO MASK INTERRUPT REGISTER */
#define ET5907_MASK_UVLO_LDO12    (1 << 0)
#define ET5907_MASK_UVLO_LDO34    (1 << 1)
#define ET5907_MASK_UVLO_LDO5     (1 << 2)
#define ET5907_MASK_UVLO_LDO6     (1 << 3)
#define ET5907_MASK_UVLO_LDO7     (1 << 4)
#define ET5907_MASK_UVLO_VSYS     (1 << 5)
#define ET5907_MASK_TSD_WRN       (1 << 6)
#define ET5907_MASK_TSD           (1 << 7)

#define et5907_ldo_reg_index(n)   (ET5907_REG_LDO1_VSET + (n))

/* DEVICE IDs */
#define ET5907_DEVICE_ID          0x01
#define ET5907_BIT_0              (1 << 0)
#define ET5907_BIT_1              (1 << 1)
#define ET5907_BIT_2              (1 << 2)
#define ET5907_BIT_3              (1 << 3)
#define ET5907_BIT_4              (1 << 4)
#define ET5907_BIT_5              (1 << 5)
#define ET5907_BIT_6              (1 << 6)
#define ET5907_BIT_7              (1 << 7)

/* ET5907 REGULATOR IDs */
enum {
	ET5907_ID_REGULATOR1,
	ET5907_ID_REGULATOR2,
	ET5907_ID_REGULATOR3,
	ET5907_ID_REGULATOR4,
	ET5907_ID_REGULATOR5,
	ET5907_ID_REGULATOR6,
	ET5907_ID_REGULATOR7,
	ET5907_MAX_REGULATORS,
};

enum et5907_chip_id {
	ET5907,
};

struct et5907_pdata {
	struct device_node *reg_node[ET5907_MAX_REGULATORS];
	struct regulator_init_data *init_data[ET5907_MAX_REGULATORS];
};

struct et5907_regulator_data {
	int id;
	const char *name;
	struct regulator_init_data *init_data;
	struct device_node *of_node;
};

struct et5907_platform_data {
	int num_regulators;
	struct et5907_regulator_data *regulators;
};

struct et5907_regulator {
	struct device *dev;
	struct regmap *regmap;
	struct et5907_pdata *pdata;
	struct regulator_dev *rdev[ET5907_MAX_REGULATORS];
	int num_regulator;
	int chip_irq;
	int chip_id;
	int reset_gpio;
	struct delayed_work irq_event_work;
	int irq_num;
};

struct regulator_dev *et5907_get_rdevs(int index);

int et5907_regulator_enable_warpper(struct regulator_dev *rdev);
int et5907_regulator_disable_warpper(struct regulator_dev *rdev);
int et5907_regulator_is_enabled_warpper(struct regulator_dev *rdev);
int et5907_regulator_set_voltage_warpper(struct regulator_dev *rdev, int min_uv, int max_uv, unsigned *selector);
int et5907_regulator_get_voltage_warpper(struct regulator_dev *rdev);

int et5907_regulator_i2c_init(void);
void et5907_regulator_i2c_exit(void);

#endif
/* __ET5907_REGISTERS_H__ */
