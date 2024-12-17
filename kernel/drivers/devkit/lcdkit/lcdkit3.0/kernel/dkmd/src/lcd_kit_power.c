/*
 * lcd_kit_power.c
 *
 * lcdkit power function for lcd driver
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

#include "lcd_kit_power.h"
#include "lcd_kit_disp.h"
#include "lcd_kit_common.h"

#ifdef LCD_FACTORY_MODE
#include "lcd_kit_factory.h"
#endif

#define ENABLE_MAX_TH 100
/* ldo regulator */
static struct regulator *lcd_iovcc;
static struct regulator *lcd_vci;
static struct regulator *lcd_vdd;
/* global gpio */
unsigned int g_lcd_kit_gpio;

/*
 * power type
 */
/* ldo power */
static struct vcc_desc iovcc_init_cmds[] = {
	/* vcc get */
	{ DTYPE_VCC_GET, IOVCC_NAME, &lcd_iovcc, 0, 0, WAIT_TYPE_MS, 0 },
	/* io set voltage */
	{ DTYPE_VCC_SET_VOLTAGE, IOVCC_NAME, &lcd_iovcc, 1800000, 1800000, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc vci_init_cmds[] = {
	/* vcc get */
	{ DTYPE_VCC_GET, VCI_NAME, &lcd_vci, 0, 0, WAIT_TYPE_MS, 0 },
	/* vcc set voltage */
	{ DTYPE_VCC_SET_VOLTAGE, VCI_NAME, &lcd_vci, 2800000, 2800000, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc vdd_init_cmds[] = {
	/* vcc get */
	{ DTYPE_VCC_GET, VDD_NAME, &lcd_vdd, 0, 0, WAIT_TYPE_MS, 0 },
	/* vcc set voltage */
	{ DTYPE_VCC_SET_VOLTAGE, VDD_NAME, &lcd_vdd, 1250000, 1250000, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc vci_finit_cmds[] = {
	/* vcc put */
	{ DTYPE_VCC_PUT, VCI_NAME, &lcd_vci, 0, 0, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc iovcc_finit_cmds[] = {
	/* vcc put */
	{ DTYPE_VCC_PUT, IOVCC_NAME, &lcd_iovcc, 0, 0, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc vdd_finit_cmds[] = {
	/* vcc put */
	{ DTYPE_VCC_PUT, VDD_NAME, &lcd_vdd, 0, 0, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc iovcc_enable_cmds[] = {
	/* vcc enable */
	{ DTYPE_VCC_ENABLE, IOVCC_NAME, &lcd_iovcc, 0, 0, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc vci_enable_cmds[] = {
	/* vcc enable */
	{ DTYPE_VCC_ENABLE, VCI_NAME, &lcd_vci, 0, 0, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc vdd_enable_cmds[] = {
	/* vcc enable */
	{ DTYPE_VCC_ENABLE, VDD_NAME, &lcd_vdd, 0, 0, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc iovcc_disable_cmds[] = {
	/* vcc disable */
	{ DTYPE_VCC_DISABLE, IOVCC_NAME, &lcd_iovcc, 0, 0, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc vci_disable_cmds[] = {
	/* vcc disable */
	{ DTYPE_VCC_DISABLE, VCI_NAME, &lcd_vci, 0, 0, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc vdd_disable_cmds[] = {
	/* vcc disable */
	{ DTYPE_VCC_DISABLE, VDD_NAME, &lcd_vdd, 0, 0, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc vdd_set_cmds[] = {
	/* vcc set voltage */
	{ DTYPE_VCC_SET_VOLTAGE, VDD_NAME, &lcd_vdd, 1250000, 1250000, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc vdd_get_cmds[] = {
	/* get voltage */
	{ DTYPE_VCC_GET_VOLTAGE, VDD_NAME, &lcd_vdd, 0, 0, WAIT_TYPE_MS, 0 },
};

#ifdef LCD_FACTORY_MODE
static struct vcc_desc vdd_dynamic_set_cmds[] = {
	/* set voltage */
	{ DTYPE_VCC_SET_VOLTAGE, VDD_NAME, &lcd_vdd, 1250000, 1250000, WAIT_TYPE_MS, 0 },
};
#endif

/* gpio power */
static struct gpio_desc gpio_req_cmds[] = {
	{
		DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_NAME, &g_lcd_kit_gpio, 0
	},
};

static struct gpio_desc gpio_free_cmds[] = {
	{
		DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_NAME, &g_lcd_kit_gpio, 0
	},
};

static struct gpio_desc gpio_high_cmds[] = {
	{
		DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_NAME, &g_lcd_kit_gpio, 1
	},
};

static struct gpio_desc gpio_low_cmds[] = {
	{
		DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_NAME, &g_lcd_kit_gpio, 0
	},
};

struct gpio_power_arra gpio_power[] = {
	{ GPIO_REQ, ARRAY_SIZE(gpio_req_cmds), gpio_req_cmds },
	{ GPIO_HIGH, ARRAY_SIZE(gpio_high_cmds), gpio_high_cmds },
	{ GPIO_LOW, ARRAY_SIZE(gpio_low_cmds), gpio_low_cmds },
	{ GPIO_FREE, ARRAY_SIZE(gpio_free_cmds), gpio_free_cmds },
};

static int lcd_kit_gpio_tx_check_param(struct gpio_desc *cmds, int index)
{
	if ((cmds == NULL) || (cmds->label == NULL)) {
		LCD_KIT_ERR("cmds or cmds->label is NULL! index=%d\n", index);
		return LCD_KIT_FAIL;
	}

	if (!gpio_is_valid(*(cmds->gpio))) {
		LCD_KIT_ERR("gpio invalid, dtype=%d, lable=%s, gpio=%d!\n",
			cmds->dtype, cmds->label, *(cmds->gpio));
		return LCD_KIT_FAIL;
	}

	return LCD_KIT_OK;
}

int lcd_kit_gpio_cmds_tx(struct gpio_desc *cmds, int cnt)
{
	int i;
	struct gpio_desc *cm = cmds;

	for (i = 0; i < cnt; i++) {
		if (lcd_kit_gpio_tx_check_param(cm, i))
			return LCD_KIT_FAIL;

		LCD_KIT_INFO("gpio = %d, type = %d\n", *(cm->gpio), cm->dtype);
		if (cm->dtype == DTYPE_GPIO_INPUT) {
			if (gpio_direction_input(*(cm->gpio)) != 0) {
				LCD_KIT_ERR("failed to gpio_direction_input, lable=%s, gpio=%d!\n",
					cm->label, *(cm->gpio));
				return LCD_KIT_FAIL;
			}
		} else if (cm->dtype == DTYPE_GPIO_OUTPUT) {
			if (gpio_direction_output(*(cm->gpio), cm->value) != 0) {
				LCD_KIT_ERR("failed to gpio_direction_output, label%s, gpio=%d!\n",
					cm->label, *(cm->gpio));
				return LCD_KIT_FAIL;
			}
		} else if (cm->dtype == DTYPE_GPIO_REQUEST) {
			if (gpio_request(*(cm->gpio), cm->label) != 0) {
				LCD_KIT_ERR("failed to gpio_request, lable=%s, gpio=%d!\n",
					cm->label, *(cm->gpio));
				return LCD_KIT_FAIL;
			}
		} else if (cm->dtype == DTYPE_GPIO_FREE) {
			gpio_free(*(cm->gpio));
		} else {
			LCD_KIT_ERR("dtype=%x NOT supported\n", cm->dtype);
			return LCD_KIT_FAIL;
		}

		lcd_kit_delay(cm->wait, cm->waittype, false);
		cm++;
	}

	return LCD_KIT_OK;
}

typedef struct {
	unsigned int power_type;
	unsigned int (*func_ptr)(int panel_id);
} lcd_gpio_event_table;

static unsigned int set_lcd_vci_gpio(int panel_id)
{
	if (power_hdl->lcd_vci.buf == NULL)
		LCD_KIT_ERR("null pointer\n");
	return power_hdl->lcd_vci.buf[POWER_NUM];
}

static unsigned int set_lcd_iovcc_gpio(int panel_id)
{
	if (power_hdl->lcd_iovcc.buf == NULL)
		LCD_KIT_ERR("null pointer\n");
	return power_hdl->lcd_iovcc.buf[POWER_NUM];
}

static unsigned int set_lcd_vsp_gpio(int panel_id)
{
	if (power_hdl->lcd_vsp.buf == NULL)
		LCD_KIT_ERR("null pointer\n");
	return power_hdl->lcd_vsp.buf[POWER_NUM];
}

static unsigned int set_lcd_vsn_gpio(int panel_id)
{
	if (power_hdl->lcd_vsn.buf == NULL)
		LCD_KIT_ERR("null pointer\n");
	return power_hdl->lcd_vsn.buf[POWER_NUM];
}

static unsigned int set_lcd_rst_gpio(int panel_id)
{
	if (power_hdl->lcd_rst.buf == NULL)
		LCD_KIT_ERR("null pointer\n");
	return power_hdl->lcd_rst.buf[POWER_NUM];
}

static unsigned int set_lcd_backlight_gpio(int panel_id)
{
	if (power_hdl->lcd_backlight.buf == NULL)
		LCD_KIT_ERR("null pointer\n");
	return power_hdl->lcd_backlight.buf[POWER_NUM];
}

static unsigned int set_lcd_vdd_gpio(int panel_id)
{
	if (power_hdl->lcd_vdd.buf == NULL)
		LCD_KIT_ERR("null pointer\n");
	return power_hdl->lcd_vdd.buf[POWER_NUM];
}

static unsigned int set_lcd_aod_gpio(int panel_id)
{
	if (power_hdl->lcd_aod.buf == NULL)
		LCD_KIT_ERR("null pointer\n");
	return power_hdl->lcd_aod.buf[POWER_NUM];
}

static unsigned int set_lcd_mipi_switch_gpio(int panel_id)
{
	if (power_hdl->lcd_mipi_switch.buf == NULL)
		LCD_KIT_ERR("null pointer\n");
	return power_hdl->lcd_mipi_switch.buf[POWER_NUM];
}

static unsigned int set_lcd_pmic_ctrl_gpio(int panel_id)
{
	if (power_hdl->lcd_pmic_ctrl.buf == NULL)
		LCD_KIT_ERR("null pointer\n");
	return power_hdl->lcd_pmic_ctrl.buf[POWER_NUM];
}

static unsigned int set_lcd_pmic_reset_gpio(int panel_id)
{
	if (power_hdl->lcd_pmic_reset.buf == NULL)
		LCD_KIT_ERR("null pointer\n");
	return power_hdl->lcd_pmic_reset.buf[POWER_NUM];
}

static unsigned int set_lcd_pmic_init_gpio(int panel_id)
{
	if (power_hdl->lcd_pmic_init.buf == NULL)
		LCD_KIT_ERR("null pointer\n");
	return power_hdl->lcd_pmic_init.buf[POWER_NUM];
}

static unsigned int set_lcd_boost_enable_gpio(int panel_id)
{
	if (power_hdl->lcd_boost_enable.buf == NULL)
		LCD_KIT_ERR("null pointer\n");
	return power_hdl->lcd_boost_enable.buf[POWER_NUM];
}

static unsigned int set_lcd_boost_ctrl_gpio(int panel_id)
{
	if (power_hdl->lcd_boost_ctrl.buf == NULL)
		LCD_KIT_ERR("null pointer\n");
	return power_hdl->lcd_boost_ctrl.buf[POWER_NUM];
}

static unsigned int set_lcd_boost_bypass_gpio(int panel_id)
{
	if (power_hdl->lcd_boost_bypass.buf == NULL)
		LCD_KIT_ERR("null pointer\n");
	return power_hdl->lcd_boost_bypass.buf[POWER_NUM];
}

	static lcd_gpio_event_table g_lcd_gpio_event_table[] = {
		{ LCD_KIT_VCI, set_lcd_vci_gpio },
		{ LCD_KIT_IOVCC, set_lcd_iovcc_gpio },
		{ LCD_KIT_VSP, set_lcd_vsp_gpio },
		{ LCD_KIT_VSN, set_lcd_vsn_gpio },
		{ LCD_KIT_RST, set_lcd_rst_gpio },
		{ LCD_KIT_BL, set_lcd_backlight_gpio },
		{ LCD_KIT_VDD, set_lcd_vdd_gpio },
		{ LCD_KIT_AOD, set_lcd_aod_gpio },
		{ LCD_KIT_MIPI_SWITCH, set_lcd_mipi_switch_gpio },
		{ LCD_KIT_PMIC_CTRL, set_lcd_pmic_ctrl_gpio },
		{ LCD_KIT_PMIC_RESET, set_lcd_pmic_reset_gpio },
		{ LCD_KIT_PMIC_INIT, set_lcd_pmic_init_gpio },
		{ LCD_KIT_BOOST_ENABLE, set_lcd_boost_enable_gpio },
		{ LCD_KIT_BOOST_CTRL, set_lcd_boost_ctrl_gpio },
		{ LCD_KIT_BOOST_BYPASS, set_lcd_boost_bypass_gpio },
	};

static void lcd_kit_init_gpio(int panel_id, unsigned int power_type)
{
	int table_num = sizeof(g_lcd_gpio_event_table) / sizeof(lcd_gpio_event_table);
	int i;

	for (i = 0; i < table_num; i++) {
		if ((power_type == g_lcd_gpio_event_table[i].power_type) &&
			(g_lcd_gpio_event_table[i].func_ptr != NULL)) {
			g_lcd_kit_gpio = g_lcd_gpio_event_table[i].func_ptr(panel_id);
			return;
		}
	}
	LCD_KIT_ERR("not support type:%u\n", power_type);
}

static bool is_in_power_off_seq(int panel_id)
{
	return disp_info->lcd_power_seq >= LCD_KIT_POWER_OFF_HS_SEQ &&
		disp_info->lcd_power_seq <= LCD_KIT_POWER_OFF;
}

int lcd_kit_gpio_tx(int panel_id, unsigned int type, unsigned int op)
{
	int i;
	int ret;
	struct gpio_power_arra *gpio_cm = NULL;

#if defined(CONFIG_DKMD_DPU_AOD)
	if (lcd_kit_get_aod_panel_state(panel_id) && (type != LCD_KIT_MIPI_SWITCH) &&
		!is_in_power_off_seq(panel_id)) {
		LCD_KIT_INFO("panel is power on, not need operate gpio, type = %d, op = %d\n", type, op);
		return LCD_KIT_OK;
	}
#endif
	lcd_kit_init_gpio(panel_id, type);
	for (i = 0; i < ARRAY_SIZE(gpio_power); i++) {
		if (gpio_power[i].oper == op) {
			gpio_cm = &gpio_power[i];
			break;
		}
	}

	if (i >= ARRAY_SIZE(gpio_power) || !gpio_cm) {
		LCD_KIT_ERR("not find gpio cm or gpio cm is null\n");
		return LCD_KIT_FAIL;
	}

	ret = lcd_kit_gpio_cmds_tx(gpio_cm->cm, gpio_cm->num);
	return ret;
}

static void cmds_tx_delay(int wait, int waittype)
{
	if (wait) {
		if (waittype == WAIT_TYPE_US)
			udelay(wait);
		else if (waittype == WAIT_TYPE_MS)
			mdelay(wait);
		else
			mdelay(wait * 1000);  /* ms */
	}
}

typedef struct {
	int ops_type;
	int (*func_ptr)(struct platform_device *pdev, struct vcc_desc *cm);
}reused_lcd_vcc_cmds_tx_table;

int lcd_kit_vcc_cmds_tx_get(struct platform_device *pdev, struct vcc_desc *cm)
{
	if (!pdev) {
		LCD_KIT_ERR("pdev is null!\n");
		return -EINVAL;
	}

	*(cm->regulator) = devm_regulator_get(&pdev->dev, cm->id);
	if (IS_ERR(*(cm->regulator))) {
		LCD_KIT_ERR("devm_regulator_get failed!, cm->id = %s\n", cm->id);
		return -1;
	}
	LCD_KIT_INFO("\ndevm_regulator_get success, cm->id = %s\n", cm->id);
	return 0;
}

int lcd_kit_vcc_cmds_tx_put(struct platform_device *pdev, struct vcc_desc *cm)
{
	(void)pdev;

	if (!IS_ERR(*(cm->regulator)))
		devm_regulator_put(*(cm->regulator));
	return 0;
}

int lcd_kit_vcc_cmds_tx_enable(struct platform_device *pdev, struct vcc_desc *cm)
{
	(void)pdev;

	if (!IS_ERR(*(cm->regulator))) {
		if (regulator_enable(*(cm->regulator)) != 0) {
			LCD_KIT_ERR("failed to enable %s regulator!\n", cm->id);
			return -1;
		}
		LCD_KIT_INFO("\nregulator_enable success, cm->id = %s\n", cm->id);
	}
	return 0;
}

int lcd_kit_vcc_cmds_tx_disable(struct platform_device *pdev, struct vcc_desc *cm)
{
	(void)pdev;

	if (!IS_ERR(*(cm->regulator))) {
		if (regulator_disable(*(cm->regulator)) != 0) {
			LCD_KIT_ERR("failed to disable %s regulator!\n", cm->id);
			return -1;
		}
		LCD_KIT_INFO("\nregulator_disable success, cm->id = %s\n", cm->id);
	}
	return 0;
}

int lcd_kit_vcc_cmds_tx_set_voltage(struct platform_device *pdev, struct vcc_desc *cm)
{
	(void)pdev;
	int ret = 0;

	if (!IS_ERR(*(cm->regulator))) {
		ret = regulator_set_voltage(*(cm->regulator), cm->min_uv, cm->max_uv);
		if (ret != 0) {
			LCD_KIT_ERR("failed to set %s regulator voltage!, ret is %d\n", cm->id, ret);
			return -1;
		}
		LCD_KIT_INFO("\nregulator_set_voltage success, cm->id = %s\n", cm->id);
	}
	return 0;
}

int lcd_kit_vcc_cmds_tx_get_voltage(struct platform_device *pdev, struct vcc_desc *cm)
{
	(void)pdev;
	int ret_volt = 0;

	if (!IS_ERR(*(cm->regulator))) {
		ret_volt = regulator_get_voltage(*(cm->regulator));
		cm->min_uv = ret_volt;
		cm->max_uv = ret_volt;
		LCD_KIT_INFO("\nregulator_get_voltage, cm->id = %s, ret_volt:%d\n", cm->id, ret_volt);
	}
	return 0;
}

#ifdef LCD_FACTORY_MODE
int lcd_kit_dynamic_ctrl_vdd(int panel_id, uint32_t volt_mode)
{
	int ret = LCD_KIT_OK;
	LCD_KIT_INFO("+\n");
	vdd_dynamic_set_cmds->min_uv = FACT_INFO->dynamic_volt.vdd_vol[volt_mode];
	vdd_dynamic_set_cmds->max_uv = FACT_INFO->dynamic_volt.vdd_vol[volt_mode];
	ret = lcd_kit_vcc_cmds_tx(NULL, vdd_dynamic_set_cmds,
		ARRAY_SIZE(vdd_dynamic_set_cmds));
	LCD_KIT_INFO("vdd_dynamic_set_cmds ret:%d\n", ret);
	ret = lcd_kit_vcc_cmds_tx(NULL, vdd_enable_cmds,
		ARRAY_SIZE(vdd_enable_cmds));
	LCD_KIT_INFO("vdd_enable_cmds ret:%d\n", ret);

	ret = lcd_kit_vcc_cmds_tx(NULL, vdd_get_cmds,
		ARRAY_SIZE(vdd_get_cmds));
	LCD_KIT_INFO("vdd_get_cmds ret:%d\n", ret);

	LCD_KIT_INFO("get :%d\n", vdd_get_cmds->min_uv);

	return ret;
}

int lcd_kit_dynamic_get_vdd(int panel_id)
{
	int ret = LCD_KIT_OK;
	LCD_KIT_INFO("+\n");

	ret = lcd_kit_vcc_cmds_tx(NULL, vdd_get_cmds,
		ARRAY_SIZE(vdd_get_cmds));
	LCD_KIT_INFO("vdd_get_cmds ret:%d\n", ret);

	LCD_KIT_INFO("get :%d\n", vdd_get_cmds->min_uv);

	return vdd_get_cmds->min_uv;
}

#endif

static const reused_lcd_vcc_cmds_tx_table g_reused_lcd_vcc_cmds_tx_table[] = {
	{ DTYPE_VCC_GET, lcd_kit_vcc_cmds_tx_get },
	{ DTYPE_VCC_PUT, lcd_kit_vcc_cmds_tx_put },
	{ DTYPE_VCC_ENABLE, lcd_kit_vcc_cmds_tx_enable },
	{ DTYPE_VCC_DISABLE, lcd_kit_vcc_cmds_tx_disable },
	{ DTYPE_VCC_SET_VOLTAGE, lcd_kit_vcc_cmds_tx_set_voltage },
	/* Note: the last item of the table. */
	{ DTYPE_VCC_GET_VOLTAGE, lcd_kit_vcc_cmds_tx_get_voltage },
};

int lcd_kit_vcc_cmds_tx(struct platform_device *pdev, struct vcc_desc *cmds, int cnt)
{
	struct vcc_desc *cm = cmds;
	int i, j;
	int ret = 0;
	int table_num = sizeof(g_reused_lcd_vcc_cmds_tx_table) / sizeof(reused_lcd_vcc_cmds_tx_table);

	for (i = 0; i < cnt; i++) {
		if (!cm || !cm->id) {
			LCD_KIT_ERR("cm is null or cmd->id is null!\n");
			return -1;
		}

		if ((cm->dtype < DTYPE_VCC_GET) || (cm->dtype > DTYPE_VCC_GET_VOLTAGE)) {
			LCD_KIT_ERR("dtype=%x NOT supported\n", cm->dtype);
			return -1;
		}

		for (j = 0; j < table_num; j++) {
			if ((cm->dtype == g_reused_lcd_vcc_cmds_tx_table[j].ops_type) &&
				(g_reused_lcd_vcc_cmds_tx_table[j].func_ptr) &&
				(g_reused_lcd_vcc_cmds_tx_table[j].func_ptr(pdev, cm) == -1))
				return -1;
		}

		cmds_tx_delay(cm->wait, cm->waittype);
		cm++;
	}

	return 0;
}

typedef struct {
	int ops_type;
	int (*func_ptr)(int panel_id, uint32_t enable);
}reused_lcd_pmu_ctrl_table;

int lcd_kit_pmu_ctrl_vci(int panel_id, uint32_t enable)
{
	int ret = LCD_KIT_OK;

	if (enable == 1) {
		ret = lcd_kit_vcc_cmds_tx(NULL, vci_enable_cmds,
			ARRAY_SIZE(vci_enable_cmds));
	} else if (enable == 0) {
		ret = lcd_kit_vcc_cmds_tx(NULL, vci_disable_cmds,
			ARRAY_SIZE(vci_disable_cmds));
	} else if (enable > ENABLE_MAX_TH) {
		vdd_set_cmds->min_uv = enable;
		vdd_set_cmds->max_uv = enable;
		ret = lcd_kit_vcc_cmds_tx(NULL, vdd_set_cmds,
			ARRAY_SIZE(vdd_set_cmds));
	}

	return ret;
}

int lcd_kit_pmu_ctrl_iovcc(int panel_id, uint32_t enable)
{
	int ret = LCD_KIT_OK;

	if (enable == 1) {
		ret = lcd_kit_vcc_cmds_tx(NULL, iovcc_enable_cmds,
			ARRAY_SIZE(iovcc_enable_cmds));
	} else if (enable == 0) {
		ret = lcd_kit_vcc_cmds_tx(NULL, iovcc_disable_cmds,
			ARRAY_SIZE(iovcc_disable_cmds));
	} else if (enable > ENABLE_MAX_TH) {
		vdd_set_cmds->min_uv = enable;
		vdd_set_cmds->max_uv = enable;
		ret = lcd_kit_vcc_cmds_tx(NULL, vdd_set_cmds,
			ARRAY_SIZE(vdd_set_cmds));
	}

	return ret;
}

int lcd_kit_pmu_ctrl_vdd(int panel_id, uint32_t enable)
{
	int ret = LCD_KIT_OK;

	if (enable == 1) {
		if (PUBLIC_INFO->dvdd_avs_volt > 0) {
			vdd_enable_cmds->min_uv = (int)(PUBLIC_INFO->dvdd_avs_volt);
			vdd_enable_cmds->max_uv = (int)(PUBLIC_INFO->dvdd_avs_volt);
		}
		ret = lcd_kit_vcc_cmds_tx(NULL, vdd_enable_cmds,
			ARRAY_SIZE(vdd_enable_cmds));
	} else if (enable == 0) {
		ret = lcd_kit_vcc_cmds_tx(NULL, vdd_disable_cmds,
			ARRAY_SIZE(vdd_disable_cmds));
	} else if (enable > ENABLE_MAX_TH) {
		if (PUBLIC_INFO->dvdd_avs_volt > 0) {
			vdd_set_cmds->min_uv = (int)(PUBLIC_INFO->dvdd_avs_volt);
			vdd_set_cmds->max_uv = (int)(PUBLIC_INFO->dvdd_avs_volt);
#ifdef LCD_FACTORY_MODE
			FACT_INFO->dynamic_volt.vdd_vol[VOLT_NOMAL] = (int)(PUBLIC_INFO->dvdd_avs_volt);
#endif
		} else {
			vdd_set_cmds->min_uv = enable;
			vdd_set_cmds->max_uv = enable;
#ifdef LCD_FACTORY_MODE
			FACT_INFO->dynamic_volt.vdd_vol[VOLT_NOMAL] = enable;
#endif
		}
		ret = lcd_kit_vcc_cmds_tx(NULL, vdd_set_cmds,
			ARRAY_SIZE(vdd_set_cmds));
	}

	return ret;
}

static const reused_lcd_pmu_ctrl_table g_reused_lcd_pmu_ctrl_table[] = {
	{ LCD_KIT_VCI, lcd_kit_pmu_ctrl_vci },
	{ LCD_KIT_IOVCC, lcd_kit_pmu_ctrl_iovcc },
	{ LCD_KIT_VDD, lcd_kit_pmu_ctrl_vdd },
};

int lcd_kit_pmu_ctrl(int panel_id, uint32_t type, uint32_t enable)
{
	int table_num = sizeof(g_reused_lcd_pmu_ctrl_table) / sizeof(reused_lcd_pmu_ctrl_table);

	LCD_KIT_INFO("\nlcd_kit_pmu_ctrl type = %d, enable = %d\n", type, enable);
	for (int i = 0; i < table_num; i++) {
		if ((type == g_reused_lcd_pmu_ctrl_table[i].ops_type) &&
			(g_reused_lcd_pmu_ctrl_table[i].func_ptr))
			return g_reused_lcd_pmu_ctrl_table[i].func_ptr(panel_id, enable);
	}
	LCD_KIT_ERR("error type\n");
	return LCD_KIT_FAIL;
}

static void lcd_kit_power_set(int panel_id, struct vcc_desc *cmds, int cnt)
{
	struct vcc_desc *cm = NULL;
	int i;

	cm = cmds;
	for (i = 0; i < cnt; i++) {
		if (cm->dtype == DTYPE_VCC_SET_VOLTAGE) {
			if (!strncmp(cm->id, VCI_NAME, strlen(cm->id))) {
				cm->min_uv = (int)power_hdl->lcd_vci.buf[POWER_VOL];
				cm->max_uv = (int)power_hdl->lcd_vci.buf[POWER_VOL];
			}
			if (!strncmp(cm->id, IOVCC_NAME, strlen(cm->id))) {
				cm->min_uv = (int)power_hdl->lcd_iovcc.buf[POWER_VOL];
				cm->max_uv = (int)power_hdl->lcd_iovcc.buf[POWER_VOL];
			}
			if (!strncmp(cm->id, VDD_NAME, strlen(cm->id))) {
				cm->min_uv = (int)power_hdl->lcd_vdd.buf[POWER_VOL];
				cm->max_uv = (int)power_hdl->lcd_vdd.buf[POWER_VOL];
			}
		}
		cm++;
	}
}

static int vci_process(int panel_id, struct platform_device *pdev)
{
	int ret = LCD_KIT_OK;

	if (power_hdl->lcd_vci.buf &&
		power_hdl->lcd_vci.buf[POWER_MODE] == REGULATOR_MODE) {
		/* judge vci is valid */
			/* init vci */
		lcd_kit_power_set(panel_id, vci_init_cmds,
			ARRAY_SIZE(vci_init_cmds));
		ret = lcd_kit_vcc_cmds_tx(pdev, vci_init_cmds,
			ARRAY_SIZE(vci_init_cmds));
		if (ret != 0) {
			LCD_KIT_ERR("LCD vci init failed!\n");
			return ret;
		}
		ret = lcd_kit_vcc_cmds_tx(pdev, vci_enable_cmds,
			ARRAY_SIZE(vci_enable_cmds));
		if (ret != 0) {
			LCD_KIT_ERR("LCD vci enable failed!\n");
			return ret;
		}
	}
	return ret;
}

static int iovcc_process(int panel_id, struct platform_device *pdev)
{
	int ret = LCD_KIT_OK;

	if (power_hdl->lcd_iovcc.buf &&
		power_hdl->lcd_iovcc.buf[POWER_MODE] == REGULATOR_MODE) {
		/* judge iovcc is valid */
			/* init iovcc */
		lcd_kit_power_set(panel_id, iovcc_init_cmds,
			ARRAY_SIZE(iovcc_init_cmds));
		ret = lcd_kit_vcc_cmds_tx(pdev, iovcc_init_cmds,
			ARRAY_SIZE(iovcc_init_cmds));
		if (ret != 0) {
			LCD_KIT_ERR("LCD iovcc init failed!\n");
			return ret;
		}
		ret = lcd_kit_vcc_cmds_tx(pdev, iovcc_enable_cmds,
			ARRAY_SIZE(iovcc_enable_cmds));
		if (ret != 0) {
			LCD_KIT_ERR("LCD iovcc enable failed!\n");
			return ret;
		}
	}
	return ret;
}

static int vdd_process(int panel_id, struct platform_device *pdev)
{
	int ret = LCD_KIT_OK;

	if (power_hdl->lcd_vdd.buf &&
		power_hdl->lcd_vdd.buf[POWER_MODE] == REGULATOR_MODE) {
		/* judge vdd is valid */
			/* init vdd */
		lcd_kit_power_set(panel_id, vdd_init_cmds,
			ARRAY_SIZE(vdd_init_cmds));
		ret = lcd_kit_vcc_cmds_tx(pdev, vdd_init_cmds,
			ARRAY_SIZE(vdd_init_cmds));
		if (ret != 0) {
			LCD_KIT_ERR("LCD vdd init failed!\n");
			return ret;
		}
		ret = lcd_kit_vcc_cmds_tx(pdev, vdd_enable_cmds,
			ARRAY_SIZE(vdd_enable_cmds));
		if (ret != 0) {
			LCD_KIT_ERR("LCD vdd enable failed!\n");
			return ret;
		}
	}
	return ret;
}

int lcd_kit_power_init(int panel_id, struct platform_device *pdev)
{
	int ret;

	if (!pdev) {
		LCD_KIT_ERR("pdev is null\n");
		return LCD_KIT_FAIL;
	}
	ret = vci_process(panel_id, pdev);
	if (ret != 0)
		return ret;
	ret = iovcc_process(panel_id, pdev);
	if (ret != 0)
		return ret;
	ret = vdd_process(panel_id, pdev);
	if (ret != 0)
		return ret;
	return ret;
}

int lcd_kit_power_finit(int panel_id, struct platform_device *pdev)
{
	int ret = LCD_KIT_OK;

	if (power_hdl->lcd_vci.buf &&
		power_hdl->lcd_vci.buf[POWER_MODE] == REGULATOR_MODE)
		ret = lcd_kit_vcc_cmds_tx(pdev, vci_finit_cmds, ARRAY_SIZE(vci_finit_cmds));
	if (power_hdl->lcd_iovcc.buf &&
		power_hdl->lcd_iovcc.buf[POWER_MODE] == REGULATOR_MODE)
		ret = lcd_kit_vcc_cmds_tx(pdev, iovcc_finit_cmds, ARRAY_SIZE(iovcc_finit_cmds));
	if (power_hdl->lcd_vdd.buf &&
		power_hdl->lcd_vdd.buf[POWER_MODE] == REGULATOR_MODE)
		ret = lcd_kit_vcc_cmds_tx(pdev, vdd_finit_cmds, ARRAY_SIZE(vdd_finit_cmds));
	return ret;
}

unsigned int lcd_kit_poweric_detect(struct device_node *np, unsigned int poweric_gpio)
{
	int ret;
	unsigned int pinid = 0;
	unsigned int poweric_gpio_val = POWERIC_DEFAULT_GPIO_VAL;
	unsigned int gpio;

	gpio = poweric_gpio;
	/* config poweric_gpio and read */
	ret = gpio_request(gpio, "poweric_gpio");
	if (ret) {
		LCD_KIT_ERR("lcd_kit_gpio[%u] request fail\n", gpio);
		return pinid;
	}

	ret = gpio_direction_input(gpio);
	if (ret) {
		gpio_free(gpio);
		LCD_KIT_ERR("lcd_kit_gpio[%u] direction input set fail\n", gpio);
		return pinid;
	}
	poweric_gpio_val = gpio_get_value(gpio);
	LCD_KIT_INFO("poweric_gpio is %u, get val is %u\n",
		gpio, poweric_gpio_val);

	if (poweric_gpio_val)
		pinid |= 0x400;

	return pinid;
}

/*
 * function: lcd_kit_get_ext_pinid
 * ---------------------------------------------
 * |  |  |   |   |  |  |  |  |  |  |  |  |  |  |
 * |Ext..|POWERIC|SECUR|SEC_P| ID2 | ID1 | ID0 |
 * |  |  |   |   |  |  |  |  |  |  |  |  |  |  |
 * ---------------------------------------------
 *
 * SEC_P   Secondary Panel  0x40
 * POWERIC poweric detect   0x400
 * Ext     reserved
 */
unsigned int lcd_kit_get_ext_pinid(struct device_node *np,
	unsigned int poweric_det_support, unsigned int poweric_gpio)
{
	if (!np) {
		LCD_KIT_ERR("np is NULL! error return\n");
		return LCD_KIT_FAIL;
	}

	unsigned int pinid = 0;
	unsigned int support;
	unsigned int gpio;

	support = poweric_det_support;
	gpio = poweric_gpio;

	/* poweric detect */
	if (support) {
		LCD_KIT_INFO("support poweric_detect\n");
		pinid |= lcd_kit_poweric_detect(np, gpio);
	}

	return pinid;
}

static unsigned int lcd_kit_get_pin_value(unsigned int gpio,
	unsigned int gpio_type)
{
	int ret;
	unsigned int gpio_value = BIT(1);

	ret = gpio_request(gpio, "lcd_kit_gpio");
	if (ret) {
		LCD_KIT_ERR("lcd_kit_gpio[%u] request fail\n", gpio);
		return gpio_value;
	}
	ret = gpio_direction_output(gpio, gpio_type);
	if (ret) {
		gpio_free(gpio);
		LCD_KIT_ERR("lcd_kit_gpio[%u] direction output set fail\n", gpio);
		return gpio_value;
	}
	ret = gpio_direction_input(gpio);
	if (ret) {
		gpio_free(gpio);
		LCD_KIT_ERR("lcd_kit_gpio[%u] direction input set fail\n", gpio);
		return gpio_value;
	}
	gpio_value = gpio_get_value(gpio);
	gpio_free(gpio);

	return gpio_value;
}

unsigned int lcd_kit_get_pinid(unsigned int gpio)
{
	int ret;
	unsigned int gpio_value = BIT(1);
	unsigned int gpio_pull_down_value;
	unsigned int gpio_pull_up_value;

	/* config gpio id to pull down and read */
	gpio_pull_down_value = lcd_kit_get_pin_value(gpio, 0);
	lcd_kit_delay(5, LCD_KIT_WAIT_MS, true);
	/* config gpio id to pull up and read */
	gpio_pull_up_value = lcd_kit_get_pin_value(gpio, 1);
	if (gpio_pull_down_value == gpio_pull_up_value)
		gpio_value = gpio_pull_down_value;
	LCD_KIT_INFO("gpio value = %u\n", gpio_value);

	return gpio_value;
}
