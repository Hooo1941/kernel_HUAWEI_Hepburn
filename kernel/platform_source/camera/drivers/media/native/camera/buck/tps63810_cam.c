/*
 * tps63810_cam.c
 *
 * driver function of tps63810
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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

#include <linux/delay.h>
#include "hw_buck.h"
#include "tps63810_cam.h"
#include "../cam_log.h"
#include "../platform/sensor_commom.h"

#define PIN_VSEL_HIGH 1
#define PIN_VSEL_LOW  0

static struct mutex buck_mut_tps63810 = __MUTEX_INITIALIZER(buck_mut_tps63810);

static u8 g_dev_id;

static int tps63810_read_mask(struct hw_buck_ctrl_t *buck_ctrl,
	u8 reg, u8 mask, u8 shift, u8 *value)
{
	int ret;
	u8 val;
	struct hw_buck_i2c_client *i2c_client = NULL;
	struct hw_buck_i2c_fn_t *i2c_func = NULL;

	if (!buck_ctrl || !buck_ctrl->buck_i2c_client ||
		!buck_ctrl->buck_i2c_client->i2c_func_tbl) {
		cam_err("input param is null\n");
		return -1;
	}
	i2c_client = buck_ctrl->buck_i2c_client;
	i2c_func = buck_ctrl->buck_i2c_client->i2c_func_tbl;

	ret = i2c_func->i2c_read(i2c_client, reg, &val);
	if (ret < 0) {
		cam_err("%s: read reg fail, ret = %d", __func__, ret);
		return ret;
	}

	val &= mask;
	val >>= shift;
	*value = val;

	return 0;
}

static int tps63810_write_mask(struct hw_buck_ctrl_t *buck_ctrl,
	u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret;
	u8 val = 0;
	struct hw_buck_i2c_client *i2c_client = NULL;
	struct hw_buck_i2c_fn_t *i2c_func = NULL;

	if (!buck_ctrl || !buck_ctrl->buck_i2c_client ||
		!buck_ctrl->buck_i2c_client->i2c_func_tbl) {
		cam_err("input param is null\n");
		return false;
	}
	i2c_client = buck_ctrl->buck_i2c_client;
	i2c_func = buck_ctrl->buck_i2c_client->i2c_func_tbl;

	ret = i2c_func->i2c_read(i2c_client, reg, &val);
	if (ret < 0) {
		cam_err("%s: read reg fail, ret = %d", __func__, ret);
		return ret;
	}

	val &= ~mask;
	val |= ((value << shift) & mask);

	ret = i2c_func->i2c_write(i2c_client, reg, val);
	if (ret < 0) {
		cam_err("%s: write reg fail, ret = %d", __func__, ret);
		return ret;
	}
	return 0;
}

static int tps63810_get_dt_data(struct hw_buck_ctrl_t *buck_ctrl)
{
	int rc;

	if (!buck_ctrl) {
		cam_err("%s buck ctrl is NULL", __func__);
		return -1;
	}
	rc = hw_buck_get_dt_data(buck_ctrl);
	if (rc < 0)
		return -1;

	return 0;
}

static int tps63810_match(struct hw_buck_ctrl_t *buck_ctrl)
{
	cam_info("%s enter", __func__);
	return 0;
}

static int tps63810_set_vout1(struct hw_buck_ctrl_t *buck_ctrl, u8 vset)
{
	int ret;
	struct hw_buck_i2c_client *i2c_client = NULL;
	struct hw_buck_i2c_fn_t *i2c_func = NULL;

	if (!buck_ctrl || !buck_ctrl->buck_i2c_client ||
		!buck_ctrl->buck_i2c_client->i2c_func_tbl) {
		cam_err("input param is null\n");
		return -1;
	}
	i2c_client = buck_ctrl->buck_i2c_client;
	i2c_func = buck_ctrl->buck_i2c_client->i2c_func_tbl;

	ret = i2c_func->i2c_write(i2c_client, TPS63810_VOUT1_ADDR, vset);
	if (ret < 0) {
		cam_err("%s: write vout1 fail, ret = %d", __func__, ret);
		return ret;
	}

	return 0;
}

static int tps63810_set_vout2(struct hw_buck_ctrl_t *buck_ctrl, u8 vset)
{
	int ret;
	struct hw_buck_i2c_client *i2c_client = NULL;
	struct hw_buck_i2c_fn_t *i2c_func = NULL;

	if (!buck_ctrl || !buck_ctrl->buck_i2c_client ||
		!buck_ctrl->buck_i2c_client->i2c_func_tbl) {
		cam_err("input param is null\n");
		return false;
	}
	i2c_client = buck_ctrl->buck_i2c_client;
	i2c_func = buck_ctrl->buck_i2c_client->i2c_func_tbl;

	ret = i2c_func->i2c_write(i2c_client, TPS63810_VOUT2_ADDR, vset);
	if (ret < 0) {
		cam_err("%s: write vout1 fail, ret = %d", __func__, ret);
		return ret;
	}
	return 0;
}

static bool tps63810_set_enable(struct hw_buck_ctrl_t *buck_ctrl)
{
	u32 ctrl_val;

	if (!buck_ctrl) {
		cam_err("input param is null\n");
		return false;
	}

	switch (g_dev_id) {
	case TPS63810_DEVID_TI:
		ctrl_val = TPS63810_CONTROL_ENABLE;
		(void)tps63810_write_mask(buck_ctrl, TPS63810_CONTROL_ADDR,
			TPS63810_CONTROL_MASK,
			TPS63810_CONTROL_SHIFT, ctrl_val);
		break;
	case TPS63810_DEVID_AU:
		ctrl_val = AU8310_CONTROL_ENABLE;
		(void)tps63810_write_mask(buck_ctrl, AU8310_CONTROL_ADDR,
			AU8310_CONTROL_MASK,
			AU8310_CONTROL_SHIFT, ctrl_val);
		break;
	case TPS63810_DEVID_SGM:
		ctrl_val = SGM62110_CONTROL_ENABLE;
		(void)tps63810_write_mask(buck_ctrl, SGM62110_CONTROL_ADDR,
			SGM62110_CONTROL_MASK,
			SGM62110_CONTROL_SHIFT, ctrl_val);
		break;
	default:
		/* RT6160 no need to config */
		break;
	}

	return true;
}

static void tps63810_get_vout_info(struct hw_buck_ctrl_t *buck_ctrl,
	unsigned int *vout_min, unsigned int *vout_max, unsigned int *vout_step)
{
	if (!buck_ctrl) {
		cam_err("%s buck_ctrl is NULL", __func__);
		return;
	}

	switch (g_dev_id) {
	case TPS63810_DEVID_TI:
		if (!buck_ctrl->buck_info.pin_vsel) {
			if (buck_ctrl->buck_info.vsel_range == TPS63810_LOW_RANGE) {
				*vout_min = TPS63810_VOUT1_LR_MIN;
				*vout_max = TPS63810_VOUT1_LR_MAX;
			} else {
				*vout_min = TPS63810_VOUT1_HR_MIN;
				*vout_max = TPS63810_VOUT1_HR_MAX;
			}
			*vout_step = TPS63810_VOUT1_STEP;
		} else {
			if (buck_ctrl->buck_info.vsel_range == TPS63810_LOW_RANGE) {
				*vout_min = TPS63810_VOUT2_LR_MIN;
				*vout_max = TPS63810_VOUT2_LR_MAX;
			} else {
				*vout_min = TPS63810_VOUT2_HR_MIN;
				*vout_max = TPS63810_VOUT2_HR_MAX;
			}
			*vout_step = TPS63810_VOUT2_STEP;
		}
		break;
	case TPS63810_DEVID_RT:
		if (!buck_ctrl->buck_info.pin_vsel) {
			*vout_min = RT6160_VOUT1_MIN;
			*vout_max = RT6160_VOUT1_MAX;
			*vout_step = RT6160_VOUT1_STEP;
		} else {
			*vout_min = RT6160_VOUT2_MIN;
			*vout_max = RT6160_VOUT2_MAX;
			*vout_step = RT6160_VOUT2_STEP;
		}
		break;
	case TPS63810_DEVID_AU:
	case TPS63810_DEVID_SGM:
		if (!buck_ctrl->buck_info.pin_vsel) {
			if (buck_ctrl->buck_info.vsel_range == AU8310_LOW_RANGE) {
				*vout_min = AU8310_VOUT1_LR_MIN;
				*vout_max = AU8310_VOUT1_LR_MAX;
			} else {
				*vout_min = AU8310_VOUT1_HR_MIN;
				*vout_max = AU8310_VOUT1_HR_MAX;
			}
			*vout_step = AU8310_VOUT1_STEP;
		} else {
			if (buck_ctrl->buck_info.vsel_range == AU8310_LOW_RANGE) {
				*vout_min = AU8310_VOUT2_LR_MIN;
				*vout_max = AU8310_VOUT2_LR_MAX;
			} else {
				*vout_min = AU8310_VOUT2_HR_MIN;
				*vout_max = AU8310_VOUT2_HR_MAX;
			}
			*vout_step = AU8310_VOUT2_STEP;
		}
		break;
	default:
		return;
	}
}

static int tps63810_power_config(struct hw_buck_ctrl_t *buck_ctrl,
				u32 voltage,
				int state)
{
	int ret;
	u8 vset;
	unsigned int vout_min;
	unsigned int vout_max;
	unsigned int vout_step;

	cam_info("%s enter, voltage:%u state:%d", __func__, voltage, state);

	if (!buck_ctrl) {
		cam_err("%s buck_ctrl is NULL", __func__);
		return -1;
	}

	if (state != POWER_OFF && state != POWER_ON) {
		cam_err("%s state [%u] error", __func__, state);
		return -1;
	}

	ret = gpio_request(buck_ctrl->buck_info.en_pin, "tps63810");
	if (ret < 0) {
		cam_err("failed to request gpio[%d]", buck_ctrl->buck_info.en_pin);
		return -1;
	}
	ret = gpio_direction_output(buck_ctrl->buck_info.en_pin, state);
	if (ret < 0)
		cam_err("failed to set gpio high[%d]", buck_ctrl->buck_info.en_pin);

	gpio_free(buck_ctrl->buck_info.en_pin);

	if (state == POWER_OFF)
		return 0;

	hw_camdrv_msleep(2);

	tps63810_set_enable(buck_ctrl);

	tps63810_get_vout_info(buck_ctrl, &vout_min, &vout_max, &vout_step);

	voltage = voltage / 1000; /* uv to mv */
	if ((voltage < vout_min) || (voltage > vout_max)) {
		cam_err("vout out of range %u,%u\n", vout_min, vout_max);
		return -1;
	}

	vset = (u8)((voltage - vout_min) / vout_step);
	cam_info("dev_id=%u pin_vsel=%u vsel_range=%u vset=%u\n",
		g_dev_id, buck_ctrl->buck_info.pin_vsel,
		buck_ctrl->buck_info.vsel_range, vset);

	if (buck_ctrl->buck_info.pin_vsel == PIN_VSEL_LOW)
		return tps63810_set_vout1(buck_ctrl, vset);

	return tps63810_set_vout2(buck_ctrl, vset);
}

static int tps63810_init(struct hw_buck_ctrl_t *buck_ctrl)
{
	struct hw_buck_i2c_client *i2c_client = NULL;
	struct hw_buck_i2c_fn_t *i2c_func = NULL;
	int ret = 0;

	cam_info("%s enter", __func__);

	if (!buck_ctrl) {
		cam_err("%s buck_ctrl is NULL", __func__);
		return -1;
	}

	ret = gpio_request(buck_ctrl->buck_info.en_pin, "tps63810");
	if (ret < 0) {
		cam_err("failed to request gpio[%d]", buck_ctrl->buck_info.en_pin);
		return ret;
	}
	ret = gpio_direction_output(buck_ctrl->buck_info.en_pin, 1); /* pull high */
	if (ret < 0) {
		cam_err("failed to set gpio high[%d]", buck_ctrl->buck_info.en_pin);
		gpio_free(buck_ctrl->buck_info.en_pin);
		return ret;
	}
	hw_camdrv_msleep(2);

	tps63810_set_enable(buck_ctrl);

	i2c_client = buck_ctrl->buck_i2c_client;
	i2c_func = buck_ctrl->buck_i2c_client->i2c_func_tbl;

	ret = i2c_func->i2c_read(i2c_client, TPS63810_DEVID_ADDR, &g_dev_id);
	if (ret < 0) {
		cam_err("%s: read CHIP ID failed, ret = %d", __func__, ret);
		goto EXIT;
	}
	cam_info("%s chip id=0x%x", __func__, g_dev_id);

EXIT:
	if (gpio_direction_output(buck_ctrl->buck_info.en_pin, 0))  /* pull low */
		cam_err("%s: gpio set error",__func__);

	gpio_free(buck_ctrl->buck_info.en_pin);

	return ret;
}

static int tps63810_exit(struct hw_buck_ctrl_t *buck_ctrl)
{
	int ret;

	cam_info("%s enter", __func__);
	if (!buck_ctrl) {
		cam_err("%s buck_ctrl is NULL", __func__);
		return -1;
	}

	ret = gpio_request(buck_ctrl->buck_info.en_pin, "tps63810");
	if (ret < 0) {
		cam_err("failed to request gpio[%d]", buck_ctrl->buck_info.en_pin);
		return -1;
	}
	ret = gpio_direction_output(buck_ctrl->buck_info.en_pin, 0); /* pull low */
	if (ret < 0)
		cam_err("failed to set gpio high[%d]", buck_ctrl->buck_info.en_pin);

	gpio_free(buck_ctrl->buck_info.en_pin);
	return 0;
}


static struct hw_buck_i2c_client tps63810_i2c_client;


static struct hw_buck_fn_t tps63810_func_tbl = {
	.buck_power_config = tps63810_power_config,
	.buck_get_dt_data = tps63810_get_dt_data,
	.buck_match = tps63810_match,
	.buck_init = tps63810_init,
	.buck_exit = tps63810_exit,
};

/* driver_data */
static struct hw_buck_ctrl_t tps63810_ctrl = {
	.buck_i2c_client = &tps63810_i2c_client,
	.func_tbl = &tps63810_func_tbl,
	.buck_mutex = &buck_mut_tps63810,
};

static const struct i2c_device_id tps63810_id[] = {
	/* name, driver_data */
	{"tps63810", (unsigned long)&tps63810_ctrl },
	{}
};

static const struct of_device_id tps63810_dt_match[] = {
	{.compatible = "vendor,tps63810"},
	{}
};

MODULE_DEVICE_TABLE(of, tps63810_dt_match);

static struct i2c_driver tps63810_i2c_driver = {
	.probe    = hw_buck_i2c_probe,
	.remove   = hw_buck_remove,
	.id_table = tps63810_id,
	.driver = {
		.name           = "tps63810",
		.of_match_table = tps63810_dt_match,
	},
};

static int __init tps63810_module_init(void)
{
	cam_info("%s enter", __func__);
	return i2c_add_driver(&tps63810_i2c_driver);
}

static void __exit tps63810_module_exit(void)
{
	cam_info("%s enter", __func__);
	i2c_del_driver(&tps63810_i2c_driver);
}

module_init(tps63810_module_init);
module_exit(tps63810_module_exit);

MODULE_DESCRIPTION("TPS63810 BUCK");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Native Technologies Co., Ltd.");
