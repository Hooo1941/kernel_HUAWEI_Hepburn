/*
 * high_power_smartpa.c
 *
 * high_power_smartpa driver
 *
 * Copyright (c) 2022 Huawei Technologies Co., Ltd.
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

#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/of_device.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pwm.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/compat.h>
#include <linux/workqueue.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/of_address.h>

#include <huawei_platform/log/hw_log.h>
#include <securec.h>
#include "../smartpakit.h"

#define HWLOG_TAG smartpakit
HWLOG_REGIST();

#define RETRY_TIMES 3
#define MAX_TIMER_EXPIRES 3000
#define HIGH_LEVEL_OFFSET 16
#define RGG_INFO_SIZE 2

enum pvdd_voltage_mode {
	LOW_VOLTAGE = 0,
	HIGH_VOLTAGE,
};

enum add_timer_status {
	NOT_ADD_TIMER = 0,
	NEED_ADD_TIMER,
	POWER_ON_DEL_TIMER,
	POWER_OFF_DEL_TIMER,
};

enum pwm_param_ctrl_index {
	PWM_CTRL_ACCORD_BY_SPK_VOLUME_INDEX,
	PWM_SET_DUTY_CYCLE_BY_XML_PARAM,
	PWM_PARAM_CTRL_INDEX_MAX,
};

enum {
	PWM_USE_STANDARD_OPS_MODE, /* in qcom or other paltrom */
	PWM_USE_IO_MAP_OPS_MODE, /* in roma use iomap */
	NOT_USE_PWM_MODE,
};

enum {
	PINCTRL_GET_OPS,
	PINCTRL_STATE_GET_OPS,
	PINCTRL_SET_OPS,
	PINCTRL_PUT_OPS,
	CLK_GET_OPS,
	CLK_SET_OPS,
	CLK_PUT_OPS,
	IO_REMAP_OPS,
	GET_PWM_DEVICE_OPS,
	PARSE_PWM_CONFIG_DTSI,
	PWM_CONFIG_OPS,
	PWM_ENABLE_OPS,
	PWM_DISBALE_OPS,
	PWM_SET_DUTY_SPECIAL_SCENE,
	OPS_TYPE_MAX,
};

enum {
	PINCTRL_STATE_DEFAULT_MODE,
	PINCTRL_STATE_IDLE_MODE,
	CLK_STATE_OPEN_MODE,
	CLK_STATE_CLOSE_MODE,
	STANDARD_PWM_MODE,
	IOMAP_PWM_MODE,
	GET_PWM_IOMAP_MODE,
	DELETE_PWM_IOMAP_MODE,
	PWM_SKIP_MODE,
};

struct pwm_regs {
	u32 base_addr;
	u32 reg_len;
	u32 in_ctrl;
	u32 in_div;
	u32 out_ctrl;
	u32 out_div;
	u32 out_cfg;
};

struct standard_pwm_ctl_data {
	unsigned int pwm_period_usecs;
	unsigned int pwm_max_level;
	/* PWM params */
	struct pwm_device *pwm;
};

struct iomap_pwm_ctl_data {
	void __iomem *pwm_base;
	struct clk *pwm_clk;
	struct pwm_regs reg_offset;
	unsigned int precision;
	unsigned int out_div;
	unsigned int in_div;
	unsigned int pwm_input_ena;
};

struct pwm_ops_data {
	struct pinctrl *p;
	struct pinctrl_state *pinctrl_def;
	struct pinctrl_state *pinctrl_idle;
	struct standard_pwm_ctl_data stand_ctl;
	struct iomap_pwm_ctl_data iomap_ctl;
	struct device *dev;
	struct device_node *np;
	unsigned int pwm_level;
	bool no_need_pinctl;
	/* it get from param directly, not relevant with spk volume index */
	unsigned int special_pwm_level; /* use for special sence */
};

struct pwm_ops_cmd_desc {
	int ops_type;
	struct pwm_ops_data *pwm_ops_data;
	int ops_mode;
};

typedef int (*pwm_ops_func)(struct pwm_ops_cmd_desc *cmds);
static struct pwm_ops_data pwm_ops_ctrl;

static struct pwm_ops_cmd_desc pwm_pinctrl_init_cmds[] = {
	{PINCTRL_GET_OPS, &pwm_ops_ctrl, PWM_SKIP_MODE},
	{PINCTRL_STATE_GET_OPS, &pwm_ops_ctrl, PINCTRL_STATE_DEFAULT_MODE},
	{PINCTRL_STATE_GET_OPS, &pwm_ops_ctrl, PINCTRL_STATE_IDLE_MODE},
};

static struct pwm_ops_cmd_desc pwm_pinctrl_normal_cmds[] = {
	{PINCTRL_SET_OPS, &pwm_ops_ctrl, PINCTRL_STATE_DEFAULT_MODE},
};

static struct pwm_ops_cmd_desc pwm_pinctrl_lowpower_cmds[] = {
	{PINCTRL_SET_OPS, &pwm_ops_ctrl, PINCTRL_STATE_IDLE_MODE},
};

static struct pwm_ops_cmd_desc pwm_pinctrl_deinit_cmds[] = {
	{PINCTRL_PUT_OPS, &pwm_ops_ctrl, PWM_SKIP_MODE},
};

static struct pwm_ops_cmd_desc pwm_clk_init_cmds[] = {
	{CLK_GET_OPS, &pwm_ops_ctrl, PWM_SKIP_MODE},
};

static struct pwm_ops_cmd_desc pwm_clk_open_cmds[] = {
	{CLK_SET_OPS, &pwm_ops_ctrl, CLK_STATE_OPEN_MODE},
};

static struct pwm_ops_cmd_desc pwm_clk_close_cmds[] = {
	{CLK_SET_OPS, &pwm_ops_ctrl, CLK_STATE_CLOSE_MODE},
};

static struct pwm_ops_cmd_desc pwm_clk_deinit_cmds[] = {
	{CLK_PUT_OPS, &pwm_ops_ctrl, PWM_SKIP_MODE},
};

static struct pwm_ops_cmd_desc pwm_get_io_map_cmds[] = {
	{IO_REMAP_OPS, &pwm_ops_ctrl, GET_PWM_IOMAP_MODE},
};

static struct pwm_ops_cmd_desc pwm_put_io_map_cmds[] = {
	{IO_REMAP_OPS, &pwm_ops_ctrl, DELETE_PWM_IOMAP_MODE},
};

static struct pwm_ops_cmd_desc pwm_get_device_cmds[] = {
	{GET_PWM_DEVICE_OPS, &pwm_ops_ctrl, PWM_SKIP_MODE},
};

static struct pwm_ops_cmd_desc standard_pwm_parse_dtsi_cmds[] = {
	{PARSE_PWM_CONFIG_DTSI, &pwm_ops_ctrl, STANDARD_PWM_MODE},
};

static struct pwm_ops_cmd_desc iomap_pwm_parse_dtsi_cmds[] = {
	{PARSE_PWM_CONFIG_DTSI, &pwm_ops_ctrl, IOMAP_PWM_MODE},
};

static struct pwm_ops_cmd_desc iomap_pwm_parse_default_cmds[] = {
	{PARSE_PWM_CONFIG_DTSI, &pwm_ops_ctrl, PWM_SKIP_MODE},
};

static struct pwm_ops_cmd_desc standard_pwm_enable_cmds[] = {
	{PWM_ENABLE_OPS, &pwm_ops_ctrl, STANDARD_PWM_MODE},
};

static struct pwm_ops_cmd_desc standard_pwm_disable_cmds[] = {
	{PWM_DISBALE_OPS, &pwm_ops_ctrl, STANDARD_PWM_MODE},
};

static struct pwm_ops_cmd_desc standard_pwm_config_cmds[] = {
	{PWM_CONFIG_OPS, &pwm_ops_ctrl, STANDARD_PWM_MODE},
};

static struct pwm_ops_cmd_desc standard_pwm_set_duty_special_cmds[] = {
	{PWM_SET_DUTY_SPECIAL_SCENE, &pwm_ops_ctrl, STANDARD_PWM_MODE},
};

static struct pwm_ops_cmd_desc iomap_pwm_enable_cmds[] = {
	{PWM_ENABLE_OPS, &pwm_ops_ctrl, IOMAP_PWM_MODE},
};

static struct pwm_ops_cmd_desc iomap_pwm_disable_cmds[] = {
	{PWM_DISBALE_OPS, &pwm_ops_ctrl, IOMAP_PWM_MODE},
};

static struct pwm_ops_cmd_desc iomap_pwm_config_cmds[] = {
	{PWM_CONFIG_OPS, &pwm_ops_ctrl, IOMAP_PWM_MODE},
};

static struct pwm_ops_cmd_desc iomap_pwm_set_duty_special_cmds[] = {
	{PWM_SET_DUTY_SPECIAL_SCENE, &pwm_ops_ctrl, IOMAP_PWM_MODE},
};

static int smartpakit_high_power_pwm_control(struct smartpakit_priv *pakit_priv);
static void smartpakit_high_power_del_timer(struct smartpakit_priv *pakit_priv);

static int pwm_get_pinctrl(struct pwm_ops_cmd_desc *cmds)
{
	int ret;

	if (cmds->pwm_ops_data->dev == NULL) {
		hwlog_err("%s: dev is NULL\n", __func__);
		return -EINVAL;
	}

	if (cmds->pwm_ops_data->no_need_pinctl)
		return 0;

	cmds->pwm_ops_data->p = devm_pinctrl_get(cmds->pwm_ops_data->dev);
	ret = IS_ERR_OR_NULL(cmds->pwm_ops_data->p);
	if (ret) {
		hwlog_err("%s: get pwm pinctrl error\n", __func__);
		return ret;
	}

	return 0;
}

static int pwm_pinctrl_get_state(struct pwm_ops_cmd_desc *cmds)
{
	if (cmds->pwm_ops_data->no_need_pinctl)
		return 0;

	if (cmds->ops_mode == PINCTRL_STATE_DEFAULT_MODE) {
		cmds->pwm_ops_data->pinctrl_def =
		pinctrl_lookup_state(cmds->pwm_ops_data->p,
			"pwm_pin_default");
		if (IS_ERR_OR_NULL(cmds->pwm_ops_data->pinctrl_def)) {
			hwlog_err("failed to get pinctrl def!\n");
			return -EINVAL;
		}
	} else if (cmds->ops_mode == PINCTRL_STATE_IDLE_MODE) {
		cmds->pwm_ops_data->pinctrl_idle =
		pinctrl_lookup_state(cmds->pwm_ops_data->p,
			"pwm_pin_idle");
		if (IS_ERR_OR_NULL(cmds->pwm_ops_data->pinctrl_idle)) {
			hwlog_err("failed to get pinctrl idle!\n");
			return -EINVAL;
		}
	}

	return 0;
}

static int pwm_pinctrl_set_state(struct pwm_ops_cmd_desc *cmds)
{
	if (cmds->pwm_ops_data->no_need_pinctl)
		return 0;

	if (cmds->ops_mode == PINCTRL_STATE_DEFAULT_MODE) {
		if (cmds->pwm_ops_data->p && cmds->pwm_ops_data->pinctrl_def)
			return pinctrl_select_state(cmds->pwm_ops_data->p,
				cmds->pwm_ops_data->pinctrl_def);
	} else if (cmds->ops_mode == PINCTRL_STATE_IDLE_MODE) {
		if (cmds->pwm_ops_data->p && cmds->pwm_ops_data->pinctrl_idle)
			return pinctrl_select_state(cmds->pwm_ops_data->p,
				cmds->pwm_ops_data->pinctrl_idle);
	}

	return 0;
}

static int pwm_put_pinctrl(struct pwm_ops_cmd_desc *cmds)
{
	if (cmds->pwm_ops_data->no_need_pinctl)
		return 0;

	if (cmds->pwm_ops_data->p)
		pinctrl_put(cmds->pwm_ops_data->p);

	return 0;
}

static int pwm_get_clk(struct pwm_ops_cmd_desc *cmds)
{
	struct clk *pwm_clk = NULL;
	struct device_node *np = cmds->pwm_ops_data->np;

	if (np == NULL) {
		hwlog_err("%s: np is NULL\n", __func__);
		return -EINVAL;
	}

	/* get pwm clk resource */
	pwm_clk = of_clk_get(np, 0);
	if (IS_ERR_OR_NULL(pwm_clk)) {
		hwlog_err("%s:%s clock not found: %d!\n", __func__, np->name,
		(int)PTR_ERR(pwm_clk));
		return -EINVAL;
	}

	cmds->pwm_ops_data->iomap_ctl.pwm_clk = pwm_clk;

	return 0;
}

static int pwm_ops_clk(struct pwm_ops_cmd_desc *cmds)
{
	int ret;
	struct clk *pwm_clk = cmds->pwm_ops_data->iomap_ctl.pwm_clk;

	if (IS_ERR_OR_NULL(pwm_clk)) {
		hwlog_err("%s: pwm_clk not found\n", __func__);
		return -EINVAL;
	}

	hwlog_info("%s: ops_mode:%d\n", __func__, cmds->ops_mode);
	if (cmds->ops_mode == CLK_STATE_OPEN_MODE) {
		ret = clk_prepare(pwm_clk);
		if (ret) {
			hwlog_err("%s: pwm_clk clk_prepare failed:%d\n",
				__func__, ret);
			return -EINVAL;
		}

		ret = clk_enable(pwm_clk);
		if (ret) {
			hwlog_err("%s: pwm_clk clk_enable failed:%d\n",
				__func__, ret);
			return -EINVAL;
		}
	} else if (cmds->ops_mode == CLK_STATE_CLOSE_MODE) {
		clk_disable(pwm_clk);
		clk_unprepare(pwm_clk);
	}

	return 0;
}

static int pwm_put_clk(struct pwm_ops_cmd_desc *cmds)
{
	struct clk *pwm_clk = cmds->pwm_ops_data->iomap_ctl.pwm_clk;

	if (IS_ERR_OR_NULL(pwm_clk)) {
		hwlog_err("%s: pwm_clk not found\n", __func__);
		return -EINVAL;
	}

	clk_disable(pwm_clk);
	clk_unprepare(pwm_clk);

	return 0;
}

static int pwm_get_io_reg_map(struct pwm_ops_cmd_desc *cmds)
{
	void __iomem *pwm_base = NULL;
	unsigned int mem_len;
	unsigned int mem_addr;

	mem_addr = cmds->pwm_ops_data->iomap_ctl.reg_offset.base_addr;
	mem_len = cmds->pwm_ops_data->iomap_ctl.reg_offset.reg_len;
	hwlog_info("%s: mem_addr:0x%x, mem_len:0x%x", __func__, mem_addr, mem_len);
	pwm_base = ioremap(mem_addr, mem_len);
	if (IS_ERR_OR_NULL(pwm_base)) {
		hwlog_err("%s: failed to get pwm_base resource\n", __func__);
		return -EINVAL;
	}
	cmds->pwm_ops_data->iomap_ctl.pwm_base = pwm_base;

	return 0;
}

static int pwm_delete_io_reg_map(struct pwm_ops_cmd_desc *cmds)
{
	void __iomem *pwm_base = cmds->pwm_ops_data->iomap_ctl.pwm_base;

	if (IS_ERR_OR_NULL(pwm_base))
		return 0;

	iounmap(pwm_base);
	return 0;
}

static int pwm_ops_io_reg_map(struct pwm_ops_cmd_desc *cmds)
{
	if (cmds->ops_mode == GET_PWM_IOMAP_MODE)
		return pwm_get_io_reg_map(cmds);
	else if (cmds->ops_mode == DELETE_PWM_IOMAP_MODE)
		return pwm_delete_io_reg_map(cmds);

	return 0;
}

static int get_pwm_device(struct pwm_ops_cmd_desc *cmds)
{
	struct pwm_device *pwm = NULL;
	struct device_node *np = cmds->pwm_ops_data->np;
	struct device *dev = cmds->pwm_ops_data->dev;

	if (np == NULL || dev == NULL) {
		hwlog_err("%s: np or dev is NULL\n", __func__);
		return -EINVAL;
	}

	pwm = devm_of_pwm_get(dev, np, NULL);
	if (IS_ERR_OR_NULL(pwm)) {
		hwlog_err("%s: get pwm fail %d, need probe_defer\n",
			__func__, IS_ERR(pwm));
		return -EPROBE_DEFER;
	}

	cmds->pwm_ops_data->stand_ctl.pwm = pwm;

	return 0;
}

static int parse_config_dsti_for_standard_pwm(struct pwm_ops_cmd_desc *cmds)
{
	int ret;
	struct device_node *np = cmds->pwm_ops_data->np;

	if (np == NULL)
		return -EINVAL;

	ret = smartpakit_get_prop_of_u32_type(np, "pwm-period-usecs",
		&cmds->pwm_ops_data->stand_ctl.pwm_period_usecs, true);
	ret += smartpakit_get_prop_of_u32_type(np, "pwm-max-level",
		&cmds->pwm_ops_data->stand_ctl.pwm_max_level, true);
	if (ret)
		return ret;

	if (cmds->pwm_ops_data->stand_ctl.pwm_max_level == 0) {
		hwlog_info("%s:pwm parms invaild, pls check\n", __func__);
		return -EINVAL;
	}

	hwlog_info("%s: pwm_period_usecs %u, pwm_max_level:%u\n",
		__func__, cmds->pwm_ops_data->stand_ctl.pwm_period_usecs,
		cmds->pwm_ops_data->stand_ctl.pwm_max_level);

	return 0;
}

static int parse_config_dsti_for_iomap_pwm(struct pwm_ops_cmd_desc *cmds)
{
	int ret;
	struct device_node *np = cmds->pwm_ops_data->np;

	if (np == NULL)
		return -EINVAL;

	ret = of_property_read_u32_array(np, "reg_map",
		(u32 *)&cmds->pwm_ops_data->iomap_ctl.reg_offset,
		RGG_INFO_SIZE);
	ret += smartpakit_get_prop_of_u32_type(np, "in_ctrl_offset",
		&cmds->pwm_ops_data->iomap_ctl.reg_offset.in_ctrl, true);
	ret += smartpakit_get_prop_of_u32_type(np, "in_div_offset",
		&cmds->pwm_ops_data->iomap_ctl.reg_offset.in_div, true);
	ret += smartpakit_get_prop_of_u32_type(np, "out_ctrl_offset",
		&cmds->pwm_ops_data->iomap_ctl.reg_offset.out_ctrl, true);
	ret += smartpakit_get_prop_of_u32_type(np, "out_div_offset",
		&cmds->pwm_ops_data->iomap_ctl.reg_offset.out_div, true);
	ret += smartpakit_get_prop_of_u32_type(np, "out_cfg_offset",
		&cmds->pwm_ops_data->iomap_ctl.reg_offset.out_cfg, true);
	ret += smartpakit_get_prop_of_u32_type(np, "pwm_precision",
		&cmds->pwm_ops_data->iomap_ctl.precision, true);
	ret += smartpakit_get_prop_of_u32_type(np, "pwm_out_div",
		&cmds->pwm_ops_data->iomap_ctl.out_div, false);
	ret += smartpakit_get_prop_of_u32_type(np, "pwm_in_div",
		&cmds->pwm_ops_data->iomap_ctl.in_div, false);
	ret += smartpakit_get_prop_of_u32_type(np, "pwm_input_ena",
		&cmds->pwm_ops_data->iomap_ctl.pwm_input_ena, false);
	if (ret)
		return ret;

	return 0;
}

static int parse_pwm_config_dsti(struct pwm_ops_cmd_desc *cmds)
{
	struct device_node *np = cmds->pwm_ops_data->np;

	if (np == NULL) {
		hwlog_err("%s: np is NULL\n", __func__);
		return -EINVAL;
	}

	if (of_property_read_bool(np, "no_need_pwm_pinctrl"))
		cmds->pwm_ops_data->no_need_pinctl = true;
	else
		cmds->pwm_ops_data->no_need_pinctl = false;

	hwlog_info("%s:no_need_pinctl:%d\n", __func__,
		cmds->pwm_ops_data->no_need_pinctl);
	if (cmds->ops_mode == STANDARD_PWM_MODE)
		return parse_config_dsti_for_standard_pwm(cmds);
	else if (cmds->ops_mode == IOMAP_PWM_MODE)
		return parse_config_dsti_for_iomap_pwm(cmds);

	return 0;
}

static int set_pwm_config_for_standard_mode(
	struct standard_pwm_ctl_data *stand_ctl,
	unsigned int pwm_level)
{
	int ret;
	unsigned int duty;
	unsigned int period_ns;
	unsigned int period_usecs;
	struct pwm_device *pwm = NULL;

	if (IS_ERR_OR_NULL(stand_ctl))
		return -EINVAL;

	pwm = stand_ctl->pwm;
	if (IS_ERR_OR_NULL(pwm))
		return -EINVAL;

	period_usecs = stand_ctl->pwm_period_usecs;
	if (pwm_level > stand_ctl->pwm_max_level)
		pwm_level = stand_ctl->pwm_max_level;

	period_ns = period_usecs * NSEC_PER_USEC;
	duty = pwm_level * period_ns;
	duty /= stand_ctl->pwm_max_level;
	hwlog_info("%s: duty:%u, period_ns:%u\n", __func__, duty, period_ns);
	ret = pwm_config(pwm, duty, period_ns);
	if (ret) {
		hwlog_err("%s: failed to change pwm config, ret:%d\n",
			__func__, ret);
		return ret;
	}

	return 0;
}

static int pwm_ctrl_config_for_standard_mode(struct pwm_ops_cmd_desc *cmds)
{
	unsigned int pwm_level = cmds->pwm_ops_data->pwm_level;
	struct standard_pwm_ctl_data *stand_ctl = &cmds->pwm_ops_data->stand_ctl;

	return set_pwm_config_for_standard_mode(stand_ctl, pwm_level);
}

static int set_pwm_config_for_iomap_mode(struct iomap_pwm_ctl_data *iomap_ctl,
	unsigned int pwm_level)
{
	unsigned int value;
	unsigned int out_precision;
	void *reg_addr = NULL;
	void __iomem *pwm_base = NULL;

	if (IS_ERR_OR_NULL(iomap_ctl))
		return -EINVAL;

	out_precision = iomap_ctl->precision;
	pwm_base = iomap_ctl->pwm_base;
	if (IS_ERR_OR_NULL(pwm_base))
		return -EINVAL;

	if (pwm_level > out_precision - 1)
		pwm_level = out_precision - 1;

	hwlog_info("%s: pwm_level:%u, out_precision:%u\n", __func__,
		pwm_level, out_precision);
	value = (pwm_level << HIGH_LEVEL_OFFSET) | (out_precision - pwm_level);
	reg_addr = pwm_base + iomap_ctl->reg_offset.out_cfg;
	writel(value, reg_addr);

	return 0;
}

static int pwm_ctrl_config_for_iomap_mode(struct pwm_ops_cmd_desc *cmds)
{
	unsigned int pwm_level = cmds->pwm_ops_data->pwm_level;
	struct iomap_pwm_ctl_data *iomap_ctl = &cmds->pwm_ops_data->iomap_ctl;

	return set_pwm_config_for_iomap_mode(iomap_ctl, pwm_level);
}

static int pwm_ctrl_config(struct pwm_ops_cmd_desc *cmds)
{
	int ret = 0;

	if (cmds->ops_mode == STANDARD_PWM_MODE)
		ret = pwm_ctrl_config_for_standard_mode(cmds);
	else if (cmds->ops_mode == IOMAP_PWM_MODE)
		ret = pwm_ctrl_config_for_iomap_mode(cmds);

	return ret;
}

static int pwm_ctrl_enable_for_standard_mode(struct pwm_ops_cmd_desc *cmds)
{
	int ret;
	struct pwm_device *pwm = cmds->pwm_ops_data->stand_ctl.pwm;

	if (IS_ERR_OR_NULL(pwm))
		return -EINVAL;

	ret = pwm_ctrl_config_for_standard_mode(cmds);
	if (ret)
		return ret;

	hwlog_info("%s: pwm_enable\n",  __func__);
	ret = pwm_enable(pwm);
	if (ret) {
		hwlog_err("%s: failed to enable pwm, ret:%d\n",
			__func__, ret);
		return ret;
	}
	return 0;
}

static int pwm_ctrl_enable_for_iomap_mode(struct pwm_ops_cmd_desc *cmds)
{
	void *reg_addr = NULL;
	void __iomem *pwm_base = cmds->pwm_ops_data->iomap_ctl.pwm_base;
	struct clk *pwm_clk = cmds->pwm_ops_data->iomap_ctl.pwm_clk;

	if (IS_ERR_OR_NULL(pwm_base))
		return -EINVAL;

	if (IS_ERR_OR_NULL(pwm_clk))
		return -EINVAL;

	hwlog_info("%s: pwm_enable\n",  __func__);
	pwm_ctrl_config_for_iomap_mode(cmds);
	/* enable pwm out */
	reg_addr = pwm_base + cmds->pwm_ops_data->iomap_ctl.reg_offset.out_ctrl;
	writel(0x1, reg_addr);

	/* set pwm out div */
	reg_addr = pwm_base + cmds->pwm_ops_data->iomap_ctl.reg_offset.out_div;
	writel(cmds->pwm_ops_data->iomap_ctl.out_div, reg_addr);
	if (!cmds->pwm_ops_data->iomap_ctl.pwm_input_ena)
		return 0;

	/* enable pwm in  */
	reg_addr = pwm_base + cmds->pwm_ops_data->iomap_ctl.reg_offset.in_ctrl;
	writel(0x1, reg_addr);

	/* set pwm in div */
	reg_addr = pwm_base + cmds->pwm_ops_data->iomap_ctl.reg_offset.in_div;
	writel(cmds->pwm_ops_data->iomap_ctl.in_div, reg_addr);

	return 0;
}

static int pwm_ctrl_enable(struct pwm_ops_cmd_desc *cmds)
{
	int ret = 0;

	if (cmds->ops_mode == STANDARD_PWM_MODE)
		ret = pwm_ctrl_enable_for_standard_mode(cmds);
	else if (cmds->ops_mode == IOMAP_PWM_MODE)
		ret = pwm_ctrl_enable_for_iomap_mode(cmds);

	return ret;
}

static int pwm_ctrl_disable_for_standard_mode(struct pwm_ops_cmd_desc *cmds)
{
	struct pwm_device *pwm = cmds->pwm_ops_data->stand_ctl.pwm;

	if (IS_ERR_OR_NULL(pwm))
		return -EINVAL;

	hwlog_info("%s: pwm_disable\n",  __func__);
	pwm_disable(pwm);

	return 0;
}

static int pwm_ctrl_disable_for_iomap_mode(struct pwm_ops_cmd_desc *cmds)
{
	void *reg_addr = NULL;
	void __iomem *pwm_base = cmds->pwm_ops_data->iomap_ctl.pwm_base;
	struct clk *pwm_clk = cmds->pwm_ops_data->iomap_ctl.pwm_clk;

	if (IS_ERR_OR_NULL(pwm_base))
		return -EINVAL;

	if (IS_ERR_OR_NULL(pwm_clk))
		return -EINVAL;

	hwlog_info("%s: pwm_disable\n",  __func__);
	/* disable pwm out */
	reg_addr = pwm_base + cmds->pwm_ops_data->iomap_ctl.reg_offset.out_ctrl;
	writel(0x0, reg_addr);
	if (!cmds->pwm_ops_data->iomap_ctl.pwm_input_ena)
		return 0;

	/* disable pwm in */
	reg_addr = pwm_base + cmds->pwm_ops_data->iomap_ctl.reg_offset.in_ctrl;
	writel(0x0, reg_addr);

	return 0;
}

static int pwm_ctrl_disable(struct pwm_ops_cmd_desc *cmds)
{
	int ret = 0;

	if (cmds->ops_mode == STANDARD_PWM_MODE)
		ret = pwm_ctrl_disable_for_standard_mode(cmds);
	else if (cmds->ops_mode == IOMAP_PWM_MODE)
		ret = pwm_ctrl_disable_for_iomap_mode(cmds);

	return ret;
}

static int pwm_set_duty_special_sence(struct pwm_ops_cmd_desc *cmds)
{
	int ret = 0;
	unsigned int pwm_level = cmds->pwm_ops_data->special_pwm_level;
	struct standard_pwm_ctl_data *stand_ctl = &cmds->pwm_ops_data->stand_ctl;
	struct iomap_pwm_ctl_data *iomap_ctl = &cmds->pwm_ops_data->iomap_ctl;

	hwlog_info("%s: pwm_level: %u\n", __func__, pwm_level);

	if (cmds->ops_mode == STANDARD_PWM_MODE)
		ret = set_pwm_config_for_standard_mode(stand_ctl, pwm_level);
	else if (cmds->ops_mode == IOMAP_PWM_MODE)
		ret = set_pwm_config_for_iomap_mode(iomap_ctl, pwm_level);

	return ret;
}

static pwm_ops_func pwm_ops_funcs[OPS_TYPE_MAX] = {
	pwm_get_pinctrl,
	pwm_pinctrl_get_state,
	pwm_pinctrl_set_state,
	pwm_put_pinctrl,
	pwm_get_clk,
	pwm_ops_clk,
	pwm_put_clk,
	pwm_ops_io_reg_map,
	get_pwm_device,
	parse_pwm_config_dsti,
	pwm_ctrl_config,
	pwm_ctrl_enable,
	pwm_ctrl_disable,
	pwm_set_duty_special_sence,
};

static int pwm_cmds_ops(struct pwm_ops_cmd_desc *cmds, int cnt)
{
	int ret = 0;
	int i;
	struct pwm_ops_cmd_desc *cm = NULL;

	cm = cmds;
	for (i = 0; i < cnt; i++) {
		if (cm->ops_type >= OPS_TYPE_MAX)
			continue;

		if (pwm_ops_funcs[cm->ops_type] == NULL)
			continue;

		ret = pwm_ops_funcs[cm->ops_type](cm);
		if (ret != 0) {
			hwlog_err("ret:%d err ops type: %d index: %d!\n",
				ret, cm->ops_type, i);
			return ret;
		}
		cm++;
	}

	return ret;
}

bool is_high_power_smartpa(struct smartpakit_priv *pakit_priv)
{
	if (pakit_priv == NULL)
		return false;

	if (pakit_priv->cust == NULL)
		return false;

	if (strcmp(pakit_priv->cust, "high_power") == 0)
		return true;

	return false;
}

bool smartpakit_high_power_is_power_down(struct smartpakit_priv *pakit_priv,
	struct smartpakit_i2c_priv *i2c_priv)
{
	struct smartpakit_pa_ctl_sequence *sequence = NULL;

	if ((pakit_priv == NULL) || (i2c_priv == NULL)) {
		hwlog_err("%s: invalid arg, file or buf is NULL\n", __func__);
		return false;
	}

	if (!i2c_priv->high_power_pa)
		return false;

	if (!is_high_power_smartpa(pakit_priv))
		return false;

	sequence = &pakit_priv->high_power_config.power_on_seq;
	if (sequence->pa_poweron_flag != SMARTPA_REGS_FLAG_NEED_RESUME)
		return true;

	return false;
}

static int smartpakit_get_high_power_config_param(const void __user *arg,
	int compat_mode, struct smartpakit_high_power_set_param *param)
{
	unsigned int param_size;

#ifdef CONFIG_COMPAT
	if (compat_mode == 0) {
#endif /* CONFIG_COMPAT */
		/* for system/lib64/x.so(64 bits) */
#ifndef CONFIG_FINAL_RELEASE
		hwlog_info("%s: copy_from_user b64\n", __func__);
#endif
		if (copy_from_user(param, arg, sizeof(*param))) {
			hwlog_err("%s: get param failed\n", __func__);
			goto config_param_err_out;
		}

#ifdef CONFIG_COMPAT
	} else {
		struct smartpakit_high_power_set_param_compat s_compat;

		param_size = sizeof(s_compat);
		/* for system/lib/x.so(32 bits) */
#ifndef CONFIG_FINAL_RELEASE
		hwlog_info("%s: copy_from_user32\n", __func__);
#endif
		if (copy_from_user(&s_compat, arg, param_size)) {
			hwlog_err("%s: get param failed\n", __func__);
			goto config_param_err_out;
		}

		param->pwm_level = s_compat.pwm_level;
		param->voltage_mode = s_compat.voltage_mode;
		param->config_param.pa_ctl_mask = s_compat.config_param.pa_ctl_mask;
		param->config_param.param_num = s_compat.config_param.param_num;
		param->config_param.params = compat_ptr(s_compat.config_param.params_ptr);
	}
#endif /* CONFIG_COMPAT */

	if (smartpakit_is_param_valid(&param->config_param))
		return 0;

config_param_err_out:
	return -EFAULT;
}

static int smartpakit_get_high_power_param_config_node(
	struct smartpakit_high_power_set_param *param,
	struct smartpakit_param_node **node)
{
	struct smartpakit_param_node *par_node = NULL;
	unsigned int par_size;

	par_size = sizeof(int) * param->config_param.param_num;
	par_node = kzalloc(par_size, GFP_KERNEL);
	if (par_node == NULL)
		return -ENOMEM;

	if (copy_from_user(par_node, (void __user *)param->config_param.params,
		par_size)) {
		hwlog_err("%s: get param data failed\n", __func__);
		kfree(par_node);
		return -EIO;
	}
	*node = par_node;

	return 0;
}

static int smartpakit_high_power_store_param_node(
	struct smartpakit_pa_ctl_sequence *power_seq,
	struct smartpakit_pa_ctl_sequence *sequence,
	unsigned int pa_poweron_flag)
{
	unsigned int param_size;

	if ((power_seq == NULL) || (sequence == NULL) ||
		(sequence->node == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	power_seq->pa_ctl_mask = 1;
	power_seq->pa_poweron_flag = pa_poweron_flag;
	power_seq->pa_ctl_num = 1;

	power_seq->param_num = sequence->param_num_of_one_pa;
	param_size = sizeof(struct smartpakit_param_node) * power_seq->param_num;
	power_seq->node = kzalloc(param_size, GFP_KERNEL);
	if (power_seq->node == NULL)
		return -ENOMEM;

	if (memcpy_s(power_seq->node, param_size,
		sequence->node, param_size) != EOK) {
		hwlog_err("%s: memcpy_s is failed\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static int smartpakit_high_power_save_poweron_regs(
	struct smartpakit_priv *pakit_priv,
	struct smartpakit_pa_ctl_sequence *sequence)
{
	struct smartpakit_pa_ctl_sequence *seq = NULL;

	if ((pakit_priv == NULL) || (sequence == NULL) ||
		(sequence->node == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}
	hwlog_info("%s: pa_poweron_flag 0x%x\n", __func__,
		sequence->pa_poweron_flag);

	if (sequence->pa_poweron_flag == SMARTPA_REGS_FLAG_POWER_OFF) {
		seq = &(pakit_priv->high_power_config.power_on_seq);
		smartpakit_reset_pa_ctl_sequence(seq);
		seq = &(pakit_priv->high_power_config.power_off_seq);
		smartpakit_reset_pa_ctl_sequence(seq);
		return smartpakit_high_power_store_param_node(seq,
			sequence, SMARTPA_REGS_FLAG_POWER_OFF);
	}

	if (sequence->pa_poweron_flag == SMARTPA_REGS_FLAG_NEED_RESUME) {
		seq = &(pakit_priv->high_power_config.power_on_seq);
		smartpakit_reset_pa_ctl_sequence(seq);
		return smartpakit_high_power_store_param_node(seq,
			sequence, SMARTPA_REGS_FLAG_NEED_RESUME);
	}

	return 0;
}

static void pwm_ctrl_delay(unsigned int delay)
{
	if (delay == 0)
		return;

	if (delay > SMARTPAKIT_DELAY_US_TO_MS)
		mdelay(delay / SMARTPAKIT_DELAY_US_TO_MS);
	else
		udelay(delay);
}

static int smartpakit_high_power_set_duty_by_param(
	struct smartpakit_priv *pakit_priv,
	struct smartpakit_param_node *node)
{
	int ret;
	unsigned int pwm_type = pakit_priv->high_power_config.pwm_type;
	pwm_ops_ctrl.special_pwm_level = node->value;

	if (pakit_priv->high_power_config.no_need_pwm)
		return 0;

	hwlog_info("%s: special_pwm_level:%u, pwm_enabled:%d\n", __func__,
		node->value, pakit_priv->high_power_config.pwm_enabled);
	if (node->value == 0)
		return 0;

	if (pwm_type == PWM_USE_STANDARD_OPS_MODE) {
		ret = pwm_cmds_ops(standard_pwm_set_duty_special_cmds,
			ARRAY_SIZE(standard_pwm_set_duty_special_cmds));
		if (ret)
			return ret;
	} else if (pwm_type == PWM_USE_IO_MAP_OPS_MODE) {
		if (!pakit_priv->high_power_config.pwm_enabled)
			return 0;

		ret = pwm_cmds_ops(iomap_pwm_set_duty_special_cmds,
			ARRAY_SIZE(iomap_pwm_set_duty_special_cmds));
		if (ret)
			return ret;
	}

	return 0;
}

static int smartpakit_high_power_handle_pwm_ctl_node(
	struct smartpakit_priv *pakit_priv,
	struct smartpakit_param_node *node)
{
	int ret;

	switch (node->index) {
	case PWM_CTRL_ACCORD_BY_SPK_VOLUME_INDEX:
		ret = smartpakit_high_power_pwm_control(pakit_priv);
		if (ret < 0) {
			hwlog_err("%s: set pwm err\n", __func__);
			return ret;
		}
		break;
	case PWM_SET_DUTY_CYCLE_BY_XML_PARAM:
		ret = smartpakit_high_power_set_duty_by_param(pakit_priv, node);
		if (ret < 0) {
			hwlog_err("%s: set duty err\n", __func__);
			return ret;
		}
		break;
	default:
		break;
	}

	pwm_ctrl_delay(node->delay);
	return 0;
}

static bool smartpakit_high_pa_is_skip_set_sequence(
	struct smartpakit_priv *pakit_priv,
	struct smartpakit_pa_ctl_sequence *new_seq,
	struct smartpakit_pa_ctl_sequence *old_seq)
{
	unsigned int regs_size;
	unsigned int num = new_seq->param_num;

	if (new_seq->pa_poweron_flag != SMARTPA_REGS_FLAG_NEED_RESUME)
		return false;

	if (!pakit_priv->high_power_config.need_check_repeat_seqs)
		return false;

	regs_size = sizeof(struct smartpakit_param_node) * num;
	if ((new_seq->node != NULL) && (old_seq->node != NULL) &&
		(old_seq->param_num == num) && (num > 0) &&
		(memcmp(new_seq->node, old_seq->node, regs_size) == 0)) {
		hwlog_info("%s: not need re-set the same seqs\n", __func__);
		return true;
	}

	return false;
}

static int smartpakit_high_power_sequence_ctl(
	struct smartpakit_priv *pakit_priv,
	struct smartpakit_pa_ctl_sequence *sequence,
	bool skip_set_seqs)
{
	int i;
	int ret = 0;
	struct smartpakit_param_node *node = NULL;

	hwlog_info("%s: skip_set_seqs:%d", __func__, skip_set_seqs);
	for (i = 0; i < (int)sequence->param_num; i++) {
		node = &sequence->node[i];
		if (node == NULL)
			continue;

		if (node->node_type == SMARTPAKIT_PARAM_NODE_TYPE_SKIP)
			continue;

		if (node->node_type == SMARTPAKIT_PARAM_NODE_TYPE_PWM_CTL) {
			ret = smartpakit_high_power_handle_pwm_ctl_node(
				pakit_priv, node);
			continue;
		}

		if (skip_set_seqs)
			continue;

		ret = smartpakit_simple_pa_gpio_node_ctl(pakit_priv, node);
		if (ret < 0) {
			hwlog_err("%s: node %d ops err\n", __func__, i);
			continue;
		}
	}

	return ret;
}

static int smartpakit_high_power_write_config_check_params(
	struct smartpakit_priv *pakit_priv, const void __user *arg)
{
	if ((pakit_priv == NULL) || (arg == NULL)) {
		hwlog_err("%s: invalid arg, file or buf is NULL\n", __func__);
		return -EINVAL;
	}

	if (!is_high_power_smartpa(pakit_priv))
		return -EINVAL;

	if (!(pakit_priv->high_power_config.probe_success))  {
		hwlog_err("%s: high_power init failed, exit\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static int smartpakit_high_power_init_param_sequence(
	struct smartpakit_high_power_set_param *param,
	struct smartpakit_pa_ctl_sequence *sequence)
{
	if (memset_s(param, sizeof(struct smartpakit_high_power_set_param), 0,
		sizeof(struct smartpakit_high_power_set_param)) != EOK) {
		hwlog_err("%s: memset_s param is failed\n", __func__);
		return -EINVAL;
	}

	if (memset_s(sequence, sizeof(struct smartpakit_pa_ctl_sequence), 0,
		sizeof(struct smartpakit_pa_ctl_sequence)) != EOK) {
		hwlog_err("%s: memset_s sequence is failed\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static bool check_timer_expires_is_normal(unsigned int timer_expires)
{
	if ((timer_expires == 0) || (timer_expires > MAX_TIMER_EXPIRES))
		return false;

	return true;
}

static int smartpakit_high_power_set_sequence_ctl(
	struct smartpakit_priv *pakit_priv,
	struct smartpakit_high_power_set_param *param,
	struct smartpakit_pa_ctl_sequence *sequence)
{
	int ret;
	bool skip_set_seqs = false;
	struct smartpakit_high_power_config *config =
		&(pakit_priv->high_power_config);

	hwlog_info("%s:timer_start status: %d\n", __func__, config->timer_start);
	if (param->voltage_mode == LOW_VOLTAGE &&
		check_timer_expires_is_normal(config->timer_expires)) {
		config->timer.expires = jiffies +
			msecs_to_jiffies(config->timer_expires);
		if ((!config->timer_start) &&
			(param->need_add_timer == NEED_ADD_TIMER)) {
			add_timer(&(config->timer));
			config->timer_start = true;
			hwlog_info("%s: start timer..\n", __func__);
			return 0;
		}

		if (config->timer_start) {
			if (param->need_add_timer == NOT_ADD_TIMER) {
				hwlog_info("%s:timer already start\n", __func__);
				return 0;
			}

			if (param->need_add_timer == POWER_ON_DEL_TIMER ||
				param->need_add_timer == POWER_OFF_DEL_TIMER)
				smartpakit_high_power_del_timer(pakit_priv);
		}
	}

	if (param->voltage_mode == HIGH_VOLTAGE)
		smartpakit_high_power_del_timer(pakit_priv);

	skip_set_seqs = smartpakit_high_pa_is_skip_set_sequence(pakit_priv,
		sequence, &(pakit_priv->high_power_config.power_on_seq));

	ret = smartpakit_high_power_sequence_ctl(pakit_priv, sequence, skip_set_seqs);
	if (ret < 0)
		return ret;

	return 0;
}

int smartpakit_high_power_config_write(struct smartpakit_priv *pakit_priv,
	const void __user *arg, int compat_mode)
{
	int ret;
	struct smartpakit_param_node *node = NULL;
	struct smartpakit_high_power_set_param param;
	struct smartpakit_pa_ctl_sequence sequence;

	ret = smartpakit_high_power_write_config_check_params(pakit_priv, arg);
	if (ret)
		return ret;

	mutex_lock(&(pakit_priv->high_power_config.ops_lock));
	ret = smartpakit_high_power_init_param_sequence(&param, &sequence);
	if (ret < 0)
		goto config_write_out;

	ret = smartpakit_get_high_power_config_param(arg, compat_mode, &param);
	if (ret < 0)
		goto config_write_out;

	ret = smartpakit_get_high_power_param_config_node(&param, &node);
	if (ret < 0)
		goto config_write_out_free_node;

	pakit_priv->high_power_config.pwm_level = param.pwm_level;
	pakit_priv->high_power_config.voltage_mode = param.voltage_mode;
	hwlog_info("%s: pwm_level:%u, voltage_mode:%u, need_add_timer:%u\n",
		__func__, param.pwm_level, param.voltage_mode,
		param.need_add_timer);
	ret = smartpakit_fill_param_sequence(&sequence,
		&param.config_param, node); // node is assign to sequence->node
	if (ret < 0)
		goto config_write_out_free_node;

	smartpakit_print_node_info(sequence.node, sequence.param_num);
	ret = smartpakit_high_power_set_sequence_ctl(pakit_priv, &param,
		&sequence);
	if (ret < 0)
		goto config_write_out_free_node;

	ret = smartpakit_high_power_save_poweron_regs(pakit_priv, &sequence);
config_write_out_free_node:
	smartpakit_kfree_ops(node);
config_write_out:
	mutex_unlock(&(pakit_priv->high_power_config.ops_lock));
	return ret;
}

static int smartpakit_high_power_enable_pwm(struct smartpakit_priv *pakit_priv)
{
	int ret;
	unsigned int pwm_type = pakit_priv->high_power_config.pwm_type;

	if (pakit_priv->high_power_config.no_need_pwm)
		return 0;

	ret = pwm_cmds_ops(pwm_pinctrl_normal_cmds,
		ARRAY_SIZE(pwm_pinctrl_normal_cmds));
	if (ret)
		return ret;

	if (pwm_type == PWM_USE_STANDARD_OPS_MODE) {
		ret = pwm_cmds_ops(standard_pwm_enable_cmds,
			ARRAY_SIZE(standard_pwm_enable_cmds));
		if (ret)
			return ret;
	} else if (pwm_type == PWM_USE_IO_MAP_OPS_MODE) {
		ret = pwm_cmds_ops(pwm_clk_open_cmds,
			ARRAY_SIZE(pwm_clk_open_cmds));
		if (ret)
			return ret;

		ret = pwm_cmds_ops(iomap_pwm_enable_cmds,
			ARRAY_SIZE(iomap_pwm_enable_cmds));
		if (ret)
			return ret;
	}

	return 0;
}

static int smartpakit_high_power_disable_pwm(struct smartpakit_priv *pakit_priv)
{
	int ret;
	unsigned int pwm_type = pakit_priv->high_power_config.pwm_type;

	if (pakit_priv->high_power_config.no_need_pwm)
		return 0;

	if (pwm_type == PWM_USE_STANDARD_OPS_MODE) {
		ret = pwm_cmds_ops(standard_pwm_disable_cmds,
			ARRAY_SIZE(standard_pwm_disable_cmds));
		if (ret)
			return ret;

		ret = pwm_cmds_ops(pwm_pinctrl_lowpower_cmds,
			ARRAY_SIZE(pwm_pinctrl_lowpower_cmds));
		if (ret)
			return ret;
	} else if (pwm_type == PWM_USE_IO_MAP_OPS_MODE) {
		ret = pwm_cmds_ops(iomap_pwm_disable_cmds,
			ARRAY_SIZE(iomap_pwm_disable_cmds));
		if (ret)
			return ret;

		ret = pwm_cmds_ops(pwm_pinctrl_lowpower_cmds,
			ARRAY_SIZE(pwm_pinctrl_lowpower_cmds));
		if (ret)
			return ret;

		ret = pwm_cmds_ops(pwm_clk_close_cmds,
			ARRAY_SIZE(pwm_clk_close_cmds));
		if (ret)
			return ret;
	}

	return 0;
}

static int smartpakit_high_power_config_pwm(struct smartpakit_priv *pakit_priv)
{
	int ret;
	unsigned int pwm_type = pakit_priv->high_power_config.pwm_type;

	if (pakit_priv->high_power_config.no_need_pwm)
		return 0;

	if (pwm_type == PWM_USE_STANDARD_OPS_MODE) {
		ret = pwm_cmds_ops(standard_pwm_config_cmds,
			ARRAY_SIZE(standard_pwm_config_cmds));
		if (ret)
			return ret;
	} else if (pwm_type == PWM_USE_IO_MAP_OPS_MODE) {
		if (!pakit_priv->high_power_config.pwm_enabled)
			return 0;

		ret = pwm_cmds_ops(iomap_pwm_config_cmds,
			ARRAY_SIZE(iomap_pwm_config_cmds));
		if (ret)
			return ret;
	}

	return 0;
}

static int smartpakit_high_power_pwm_control(struct smartpakit_priv *pakit_priv)
{
	int ret;
	unsigned int pwm_level;

	if (pakit_priv == NULL) {
		hwlog_err("%s: Invalid Params\n", __func__);
		return -EINVAL;
	}

	pwm_level = pakit_priv->high_power_config.pwm_level;
	pwm_ops_ctrl.pwm_level = pakit_priv->high_power_config.pwm_level;
	hwlog_info("%s: pwm_level:%u, pwm_enabled:%d\n", __func__,
		pwm_level, pakit_priv->high_power_config.pwm_enabled);

	if (pwm_level != 0 && !pakit_priv->high_power_config.pwm_enabled) {
		hwlog_info("%s: pwm_enable\n",  __func__);
		ret = smartpakit_high_power_enable_pwm(pakit_priv);
		if (ret) {
			hwlog_err("%s: failed to enable pwm, ret:%d\n",
				__func__, ret);
			return  ret;
		}

		pakit_priv->high_power_config.pwm_enabled = true;
	}

	ret = smartpakit_high_power_config_pwm(pakit_priv);
	if (ret)
		return ret;

	if (pwm_level == 0 && pakit_priv->high_power_config.pwm_enabled) {
		hwlog_info("%s: pwm_disable\n",  __func__);
		ret = smartpakit_high_power_disable_pwm(pakit_priv);
		if (ret)
			return ret;

		pakit_priv->high_power_config.pwm_enabled = false;
		return 0;
	}

	return 0;
}

static int smartpakit_high_power_parse_pwm_config(struct device_node *node,
	struct smartpakit_priv *pakit_priv)
{
	int ret;
	unsigned int pwm_type = pakit_priv->high_power_config.pwm_type;

	if (pakit_priv->high_power_config.no_need_pwm)
		return 0;

	if (pwm_type == PWM_USE_STANDARD_OPS_MODE) {
		ret = pwm_cmds_ops(standard_pwm_parse_dtsi_cmds,
		ARRAY_SIZE(standard_pwm_parse_dtsi_cmds));
		if (ret)
			return ret;
	} else if (pwm_type == PWM_USE_IO_MAP_OPS_MODE) {
		ret = pwm_cmds_ops(iomap_pwm_parse_dtsi_cmds,
			ARRAY_SIZE(iomap_pwm_parse_dtsi_cmds));
		if (ret)
			return ret;
	} else {
		ret = pwm_cmds_ops(iomap_pwm_parse_default_cmds,
			ARRAY_SIZE(iomap_pwm_parse_default_cmds));
		if (ret)
			return ret;
	}

	return 0;
}

static int smartpakit_high_power_get_pwm_device(struct device *dev,
	struct device_node *node, struct smartpakit_priv *pakit_priv)
{
	int ret;
	unsigned int i;
	unsigned int retry = RETRY_TIMES;
	unsigned int retry_delay = 1; /* DEFAULT delay 1ms */
	unsigned int pwm_type = pakit_priv->high_power_config.pwm_type;

	if (pakit_priv->high_power_config.no_need_pwm)
		return 0;

	hwlog_info("%s: pwm_type:%d\n", __func__, pwm_type);
	if (pwm_type == PWM_USE_STANDARD_OPS_MODE) {
		for (i = 0; i < retry; i++) {
			ret = pwm_cmds_ops(pwm_get_device_cmds,
				ARRAY_SIZE(pwm_get_device_cmds));
			if (ret == 0)
				break;

			mdelay(retry_delay);
		}
	} else if (pwm_type == PWM_USE_IO_MAP_OPS_MODE) {
		ret = pwm_cmds_ops(pwm_get_io_map_cmds,
			ARRAY_SIZE(pwm_get_io_map_cmds));
		if (ret)
			return ret;

		ret = pwm_cmds_ops(pwm_clk_init_cmds,
			ARRAY_SIZE(pwm_clk_init_cmds));
		if (ret)
			return ret;
	}

	return 0;
}

static int smartpakit_high_power_pwm_register(struct device *dev,
	struct device_node *node, struct smartpakit_priv *pakit_priv)
{
	int ret = 0;

	hwlog_info("%s: enter\n", __func__);
	if (pakit_priv->high_power_config.no_need_pwm)
		return 0;

	ret = smartpakit_high_power_get_pwm_device(dev,
		node, pakit_priv);
	if (ret)
		return ret;

	ret = pwm_cmds_ops(pwm_pinctrl_init_cmds,
		ARRAY_SIZE(pwm_pinctrl_init_cmds));
	if (ret)
		return ret;

	return ret;
}

static int smartpakit_high_power_parse_pa_index(struct device_node *node,
	struct smartpakit_priv *pakit_priv)
{
	int ret;
	const char *pa_index_str = "high_power_pa_index";

	ret = smartpakit_get_prop_of_u32_type(node, pa_index_str,
		&(pakit_priv->high_power_config.pa_index), false);

	return ret;
}

static int smartpakit_high_power_parse_timer_expires(struct device_node *node,
	struct smartpakit_priv *pakit_priv)
{
	int ret;
	const char *timer_expires_str = "timer_expires";

	ret = smartpakit_get_prop_of_u32_type(node, timer_expires_str,
		&(pakit_priv->high_power_config.timer_expires), false);
	hwlog_info("%s: timer_expires:%u\n", __func__,
		pakit_priv->high_power_config.timer_expires);

	return ret;
}

static void smartpakit_high_power_init_ops_ctrl(struct device *dev,
	struct device_node *node, struct pwm_ops_data *pwm_ops_data)
{
	if (memset_s(pwm_ops_data, sizeof(struct pwm_ops_data), 0,
		sizeof(struct pwm_ops_data)) != EOK) {
		hwlog_err("%s: memset_s pwm_ops_data is failed\n", __func__);
		return;
	}

	pwm_ops_data->np = node;
	pwm_ops_data->dev = dev;
}

static int smartpakit_high_power_parse_pwm_type(struct device_node *node,
	struct smartpakit_priv *pakit_priv)
{
	int ret;
	const char *pwm_type_str = "pwm_type";

	ret = smartpakit_get_prop_of_u32_type(node, pwm_type_str,
		&(pakit_priv->high_power_config.pwm_type), false);

	pakit_priv->high_power_config.no_need_pwm =
		of_property_read_bool(node, "no_need_pwm");
	hwlog_info("%s: pwm_type:%d, no_need_pwm:%d", __func__,
		pakit_priv->high_power_config.pwm_type,
		pakit_priv->high_power_config.no_need_pwm);

	return ret;
}

static int smartpakit_high_power_parse_config_info(struct device_node *node,
	struct smartpakit_priv *pakit_priv)
{
	const char *check_repeat_seqs_str = "need_check_repeat_seqs";

	pakit_priv->high_power_config.need_check_repeat_seqs =
		of_property_read_bool(node, check_repeat_seqs_str);

	hwlog_info("%s: need_check_repeat_seqs:%d", __func__,
		pakit_priv->high_power_config.need_check_repeat_seqs);

	return 0;
}

static int smartpakit_high_power_parse_dt_info(struct device *dev,
	struct device_node *node,
	struct smartpakit_priv *pakit_priv)
{
	int ret;

	smartpakit_high_power_init_ops_ctrl(dev, node, &pwm_ops_ctrl);
	ret = smartpakit_high_power_parse_pwm_type(node, pakit_priv);
	if (ret)
		return ret;

	ret = smartpakit_high_power_parse_pwm_config(node, pakit_priv);
	if (ret)
		return ret;

	ret = smartpakit_high_power_pwm_register(dev, node, pakit_priv);
	if (ret)
		return ret;

	ret = smartpakit_high_power_parse_pa_index(node, pakit_priv);
	if (ret)
		return ret;

	ret = smartpakit_high_power_parse_timer_expires(node, pakit_priv);
	if (ret)
		return ret;

	ret = smartpakit_high_power_parse_config_info(node, pakit_priv);
	if (ret)
		return ret;

	return 0;
}

void smartpakit_high_power_reset(struct smartpakit_priv *pakit_priv)
{
	struct smartpakit_pa_ctl_sequence *sequence = NULL;

	if (pakit_priv == NULL)
		return;

	if (!is_high_power_smartpa(pakit_priv))
		return;

	sequence = &pakit_priv->high_power_config.power_on_seq;
	if (sequence->pa_poweron_flag == SMARTPA_REGS_FLAG_POWER_OFF)
		return;

	hwlog_info("%s: reset chips\n", __func__);
	mutex_lock(&(pakit_priv->high_power_config.ops_lock));
	sequence = &(pakit_priv->high_power_config.power_off_seq);
	smartpakit_high_power_sequence_ctl(pakit_priv, sequence, false);
	sequence = &(pakit_priv->high_power_config.power_on_seq);
	smartpakit_high_power_sequence_ctl(pakit_priv, sequence, false);
	mutex_unlock(&(pakit_priv->high_power_config.ops_lock));
}

static void smartpakit_high_power_del_timer(struct smartpakit_priv *pakit_priv)
{
	struct smartpakit_high_power_config *config =
		&(pakit_priv->high_power_config);

	if (!config->timer_start)
		return;

	del_timer(&(config->timer));
	config->timer_start = false;
}

static void smartpakit_high_power_wq_handler(struct work_struct *data)
{
	struct smartpakit_pa_ctl_sequence *sequence = NULL;
	struct smartpakit_high_power_config *config = container_of(data,
		struct smartpakit_high_power_config, work);
	struct smartpakit_priv *pakit_priv = container_of(config,
		struct smartpakit_priv, high_power_config);

	hwlog_info("%s: workin\n", __func__);
	mutex_lock(&(pakit_priv->high_power_config.ops_lock));
	if (!config->timer_start) {
		hwlog_info("%s: timer_start is cancel, ignore\n", __func__);
		goto wq_handler_exit;
	}
	sequence = &(pakit_priv->high_power_config.power_on_seq);
	smartpakit_high_power_sequence_ctl(pakit_priv, sequence, false);
	smartpakit_high_power_del_timer(pakit_priv);
wq_handler_exit:
	mutex_unlock(&(pakit_priv->high_power_config.ops_lock));
}

static void smartpakit_high_power_timer_fun(struct timer_list *t)
{
	struct smartpakit_high_power_config *config = from_timer(config, t, timer);

	hwlog_info("%s: time is expired\n", __func__);
	queue_work(config->wq, &(config->work));
}

static int smartpakit_high_power_delay_work_init(struct smartpakit_priv *pakit_priv)
{
	struct smartpakit_high_power_config *config =
		&(pakit_priv->high_power_config);

	config->wq = create_singlethread_workqueue(
		"pa_high_power_wq");
	if (!pakit_priv->high_power_config.wq) {
		hwlog_err("%s : create wq failed", __func__);
		return -ENOMEM;
	}

	INIT_WORK(&(config->work),  smartpakit_high_power_wq_handler);
	timer_setup(&(config->timer), smartpakit_high_power_timer_fun, 0);
	return 0;
}

int smartpakit_high_power_init(struct device *dev,
	struct device_node *node,
	struct smartpakit_priv *pakit_priv)
{
	int ret;
	const char *high_power_str = "high_power_config";
	struct device_node *child_node = NULL;

	if (dev == NULL || node == NULL || pakit_priv == NULL)
		return 0;

	if (!is_high_power_smartpa(pakit_priv))
		return 0;

	child_node = of_get_child_by_name(node, high_power_str);
	if (child_node == NULL) {
		hwlog_err("%s: high power node not existed, skip\n", __func__);
		return -ENOENT;
	}

	pakit_priv->high_power_config.timer_start = false;
	ret = smartpakit_high_power_parse_dt_info(dev, child_node, pakit_priv);
	if (ret)
		goto init_err;

	ret = smartpakit_high_power_delay_work_init(pakit_priv);
	if (ret)
		goto init_err;

	pakit_priv->high_power_config.probe_success = true;
	mutex_init(&(pakit_priv->high_power_config.ops_lock));
	return 0;
init_err:
	pakit_priv->high_power_config.probe_success = false;
	return ret;
}

static void smartpakit_high_power_pwm_resources_free(
	struct smartpakit_priv *pakit_priv)
{
	if (pakit_priv->high_power_config.no_need_pwm)
		return;

	pwm_cmds_ops(pwm_pinctrl_deinit_cmds,
		ARRAY_SIZE(pwm_pinctrl_deinit_cmds));
	pwm_cmds_ops(pwm_clk_deinit_cmds,
		ARRAY_SIZE(pwm_clk_deinit_cmds));
	pwm_cmds_ops(pwm_put_io_map_cmds,
		ARRAY_SIZE(pwm_put_io_map_cmds));
}

void smartpakit_high_power_deinit(struct smartpakit_priv *pakit_priv)
{
	struct smartpakit_pa_ctl_sequence *sequence = NULL;

	if (pakit_priv == NULL)
		return;

	if (!is_high_power_smartpa(pakit_priv))
		return;

	smartpakit_high_power_pwm_resources_free(pakit_priv);
	smartpakit_high_power_del_timer(pakit_priv);
	sequence = &(pakit_priv->high_power_config.power_off_seq);
	smartpakit_reset_pa_ctl_sequence(sequence);
	sequence = &(pakit_priv->high_power_config.power_on_seq);
	smartpakit_reset_pa_ctl_sequence(sequence);
	destroy_workqueue(pakit_priv->high_power_config.wq);
}

int smartpakit_high_power_get_info(struct smartpakit_priv *pakit_priv,
	void __user *arg)
{
	struct smartpakit_high_power_info info;

	if ((pakit_priv == NULL) || (arg == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if (!is_high_power_smartpa(pakit_priv))
		return -EINVAL;

	if (memset_s(&info, sizeof(info), 0, sizeof(info)) != EOK) {
		hwlog_err("%s: memset_s is failed\n", __func__);
		return -EINVAL;
	}

	info.pa_index = pakit_priv->high_power_config.pa_index;
	if (copy_to_user((void *)arg, &info, sizeof(info))) {
		hwlog_err("%s: copy kit info to user failed\n", __func__);
		return -EIO;
	}

	hwlog_info("%s: pa_index=%u\n", __func__, info.pa_index);
	return 0;
}

