/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022.
 * Description: parse power setting of device, provide common power management
 *
 * This file is released under the GPLv2.
 */
#include "sensor_power.h"
#include "cam_log.h"
#include "hw_buck.h"
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include <huawei_platform/sensor/hw_comm_pmic.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/types.h>
/***
 *  POWER EXAMPLE of device tree, power down sequence is always reversed:
 *
 *    MUST: vendor,cam-power-seq-type = "clock-rxphy", "ldo-iovdd", "gpio-rst", "clock-mclk", "hwpower-pmic";
 *    MUST: vendor,cam-power-seq-cfg-val = <80000000 1800000 0 19200000>;
 *
 *    following properties are optional, default value is 0:
 *    OPTIONAL: vendor,cam-power-seq-cfg-val-off = <0 0 0 0 0>; // gpio is inverted to on value
 *    OPTIONAL: vendor,cam-power-seq-val = <0 0 0 0 1>; // MUST when hwpower-xx present in seq-type
 *    OPTIONAL: vendor,cam-power-seq-delay = <0 1 1 10 0>;
 *    OPTIONAL: vendor,cam-power-seq-delay-off = <0 1 1 1 0>;
 *
 *  legacy vendor,cam-power-seq-type supported are listed inside 'lg_pair', check it
 *  legacy vendor,cam-power-seq-type strings are firstly mapped to generic strings
 *
 *  NEWly added gpio/ldo/clock/hwpower could use more generic method:
 *  gpio-xx example:
 *    gpios = <&gpioX Y 0>;
 *    gpio-ctrl-types = "mipisw";
 *    vendor,cam-power-seq-type = "gpio-mipisw"; // suffix of gpio- must exist in @gpio-ctrl-types
 *
 *  clock-xx example:
 *    available legacy-sensor-index - sensor mclk:
 *        0 - &clk_gate_isp_snclk0,   1 - &clk_gate_isp_snclk1
 *        2 - &clk_gate_isp_snclk2,   3 - &clk_gate_isp_snclk3
 *        20 - &clk_gate_ao_camera,   30 - &clk_gate_ao_tof
 *    clocks = <&clk_gate_isp_snclkx &clk_gate_rxx>;
 *    clock-names = "mclk", "rxphy";
 *    vendor,cam-power-seq-type = "clock-mclk", "clock-rxphy"; // suffix of clock- must exist in @clock-names
 *
 *  ldo-xx example:
 *    avdd-supply = <&ldoXX>;
 *    iovdd-supply = <&ldoXX>;
 *    vendor,cam-power-seq-type = "ldo-avdd", "ldo-iovdd"; // suffix of ldo- must exist as suffix-supply
 *
 *  hwpower-xx example:
 *    device board pmic/buck config, support pmic/pmic2/cam_buck/buck/boost_5v
 *    pmic/pmic2/cam_buck type also need vendor,cam-power-seq-val propety
 *
 *  RESTRICTIONS:
 *    gpio-ctrl-types name: "mipisw", "reset", "fsin", "btb_check"
 *      MUST be the same if these functions exist,
 *      for these gpio are reused outside normal power sequence
 **/

#define SEN_POWER_OFF 0
#define SEN_POWER_ON 1

#define GPIO_HIGH 1
#define GPIO_LOW 0

static DEFINE_MUTEX(shared_lock); // protect concurrent enable/disable power ref
static LIST_HEAD(shared_list);

int hisp_extra_pinctrl_index(const char *name);
int hisp_extra_pinctrl_select_state(int idx, const char *state);

struct power_setting;
struct power_setting_group;
typedef int (*power_config_t)(struct power_setting *, bool on);
typedef int (*power_enable_t)(struct power_setting *);
typedef void (*power_disable_t)(struct power_setting *);

struct shared_data {
	struct list_head list;
	uint32_t enable_count;
	/* unique id to distinguish different data of same type
	 * gpio type, just use gpio num
	 * pmic type, just use pmic num
	 */
	uint32_t id;

	// custom_config is called when enable_count switch between 0/1
	power_config_t custom_config;
};

struct power_setting {
	const char *dts_name;
	const char *name; // for legacy dts_name, converted to generic name
	struct power_setting_group *grp;
	uint32_t seq_val;
	uint32_t val_on;
	uint32_t val_on_max;
	uint32_t val_off;
	uint32_t val_off_max;
	uint32_t delay_on;
	uint32_t delay_off;

	void *private;
	power_enable_t enable;
	power_disable_t disable;
};

struct sensor_gpio {
	const char *name;
	int gpio;
	int ivalue;
	int ovalue;
	int delay_on;
	int delay_off;
};
enum sensor_gpio_type {
	SG_RESET,
	SG_MIPISW,
	SG_FSIN,
	SG_BTBCHECK,
	SG_MAX,
};

static const struct parse_seq_string {
	const char *seq_type;
	const char *seq_val;
	const char *seq_cfg_val;
	const char *seq_cfg_val_max;
	const char *seq_cfg_val_off;
	const char *seq_cfg_val_off_max;
	const char *seq_delay;
	const char *seq_delay_off;
} parse_flag [] = {
	[PS_SENSOR] = {
		"vendor,cam-power-seq-type", "vendor,cam-power-seq-val",
		"vendor,cam-power-seq-cfg-val", "vendor,cam-power-seq-cfg-val-max",
		"vendor,cam-power-seq-cfg-val-off", "vendor,cam-power-seq-cfg-val-off-max",
		"vendor,cam-power-seq-delay", "vendor,cam-power-seq-delay-off"
	},
	[PS_VCM] = {
		"vendor,vcm-power-seq-type", "vendor,vcm-power-seq-val",
		"vendor,vcm-power-seq-cfg-val", "vendor,vcm-power-seq-cfg-val-max",
		"vendor,vcm-power-seq-cfg-val-off", "vendor,vcm-power-seq-cfg-val-off-max",
		"vendor,vcm-power-seq-delay", "vendor,vcm-power-seq-delay-off"
	},
	[PS_PUBLIC] = {
		"vendor,public-power-seq-type", "vendor,public-power-seq-val",
		"vendor,public-power-seq-cfg-val", "vendor,public-power-seq-cfg-val-max",
		"vendor,public-power-seq-cfg-val-off", "vendor,public-power-seq-cfg-val-off-max",
		"vendor,public-power-seq-delay", "vendor,public-power-seq-delay-off"
	},
};

struct power_setting_array {
	unsigned int nsettings;
	struct power_setting *settings;
};

struct power_setting_group {
	struct device *dev;
	struct mutex power_lock;
	uint32_t power_ref;
	uint32_t reset_not_btb_check_flag;
	struct sensor_gpio sgpio[SG_MAX];
	struct power_setting_array pst_array[PS_MAX];
};

struct legacy_to_generic {
	const char *legacy;
	const char *generic;
};
// prefix sensor_ is stripped to reduce compare length
// DO NOT add new pairs, use GENERIC seq-type instead
static const struct legacy_to_generic lg_pair[] = {
	{ "pwdn",        "gpio-pwdn" },
	{ "vcm_pwdn",    "gpio-vcm_pwdn" },
	{ "avdd1_en",    "gpio-avdd1_en" },
	{ "avdd2_en",    "gpio-avdd2_en" },
	{ "dvdd0_en",    "gpio-dvdd0-en" },
	{ "dvdd1_en",    "gpio-dvdd1-en" },
	{ "iovdd_en",    "gpio-iovdd-en" },
	{ "ois",         "gpio-ois" },
	{ "mipi_sw",     "gpio-mipisw" },
	{ "laser_xshut", "gpio-laser_xshut" }, // no sensor_ prefix case
	{ "afvdd_en",    "gpio-afvdd_en" },
	{ "rst",         "gpio-reset" },
	{ "rst2",        "gpio-reset2" },
	{ "rst3",        "gpio-reset3" },
	{ "vcm_avdd",    "ldo-vcm" },
	{ "vcm_avdd2",   "ldo-vcm2" },
	{ "avdd",        "ldo-avdd" },
	{ "avdd1",       "ldo-avdd1" },
	{ "avdd2",       "ldo-avdd2" },
	{ "dvdd",        "ldo-dvdd" },
	{ "dvdd2",       "ldo-dvdd2" },
	{ "ois_drv",     "ldo-oisdrv" },
	{ "iovdd",       "ldo-iovdd" },
	{ "afvdd",       "ldo-afvdd" },
	{ "afvdd",       "ldo-afvdd" },
	{ "mclk",        "clock-mclk" },
	{ "tof_hi6562",  "clock-tof_hi6562" },
	{ "pmic",        "hwpower-pmic" },
	{ "pmic2",       "hwpower-pmic2" },
	{ "cam_buck",    "hwpower-cam_buck" },
	{ "buck",        "hwpower-buck" },
	{ "boot_5v",     "hwpower-boost_5v" },
};

static bool is_generic_name(const char *name);
static const char *to_generic_name(const char *name)
{
	uint32_t idx;

	if (!name || is_generic_name(name))
		return name;

	if (strncmp(name, "sensor_", strlen("sensor_")) == 0)
		name += strlen("sensor_");

	for (idx = 0; idx < ARRAY_SIZE(lg_pair); ++idx)
		if (strcmp(name, lg_pair[idx].legacy) == 0)
			return lg_pair[idx].generic;

	cam_err("power-seq: %s not found generic name", name);
	return name;
}

static int sensor_get_seq_data(struct device *dev, int idx,
			       struct power_setting *data, enum power_setting_type ps_type)
{
	const struct parse_seq_string* seq_str;

	seq_str = &parse_flag[ps_type];
	if (!seq_str)
		return -EINVAL;
	if (of_property_read_string_index(dev->of_node, seq_str->seq_type, idx,
					  &data->dts_name)) {
		dev_err(dev, "%pOF get index:%u of %s failed", dev->of_node,
			idx, parse_flag[ps_type].seq_type);
		return -ENODATA;
	}
	if (of_property_read_u32_index(dev->of_node, seq_str->seq_cfg_val, idx,
				       &data->val_on)) {
		dev_err(dev, "%pOF get index:%u of %s failed", dev->of_node,
			idx, seq_str->seq_cfg_val);
		return -ENODATA;
	}

	// optional fields:
	(void)of_property_read_u32_index(dev->of_node, seq_str->seq_val, idx,
					 &data->seq_val);
	(void)of_property_read_u32_index(dev->of_node, seq_str->seq_cfg_val_max, idx,
				       &data->val_on_max);
	(void)of_property_read_u32_index(dev->of_node, seq_str->seq_cfg_val_off, idx,
					 &data->val_off);
	(void)of_property_read_u32_index(dev->of_node, seq_str->seq_cfg_val_off_max, idx,
					 &data->val_off_max);
	(void)of_property_read_u32_index(dev->of_node, seq_str->seq_delay, idx,
					 &data->delay_on);
	(void)of_property_read_u32_index(dev->of_node, seq_str->seq_delay_off, idx,
					 &data->delay_off);
	data->name = to_generic_name(data->dts_name);
	return 0;
}

static struct shared_data *get_shared_data(power_config_t config, uint32_t id)
{
	struct shared_data *data;

	list_for_each_entry(data, &shared_list, list) {
		if (data->custom_config != config)
			continue;
		if (data->id == id)
			return data;
	}

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data)
		return NULL;

	data->id = id;
	data->custom_config = config;
	list_add_tail(&data->list, &shared_list);
	return data;
}

/* sensor_ref_enable - ref count managed enable, protected by shared_lock */
static int sensor_ref_enable(struct power_setting *pset)
{
	int ret = 0;
	struct shared_data *data = pset->private;

	mutex_lock(&shared_lock);
	if (++data->enable_count == 1)
		ret = data->custom_config(pset, true); // true: power on
	cam_debug("power-seq-on: %s enable count: %u", pset->name,
		  data->enable_count);
	mutex_unlock(&shared_lock);
	return ret;
}

/* sensor_ref_disable - ref count managed disable, protected by shared_lock */
static void sensor_ref_disable(struct power_setting *pset)
{
	struct shared_data *data = pset->private;

	mutex_lock(&shared_lock);
	if (data->enable_count == 1)
		data->custom_config(pset, false); // false: power off
	if (data->enable_count > 0)
		--data->enable_count;
	cam_debug("power-seq-off: %s enable count: %u", pset->name,
		  data->enable_count);
	mutex_unlock(&shared_lock);
}

static int sensor_pmic_config(struct power_setting *pset, bool on)
{
	struct shared_data *data = pset->private;
	struct hw_comm_pmic_cfg_t fp_pmic_ldo_set = { 0 };

	/* 0:pmic1/ldo1 1:pmic2/ldo2 2:cam_buck */
	fp_pmic_ldo_set.pmic_num = data->id;
	fp_pmic_ldo_set.pmic_power_type = pset->seq_val; /* 0~4:ldo 5~6:buck */
	fp_pmic_ldo_set.pmic_power_voltage = on ? pset->val_on : pset->val_off;
	fp_pmic_ldo_set.pmic_power_state = on ? SEN_POWER_ON : SEN_POWER_OFF;
	return hw_pmic_power_cfg(MAIN_CAM_PMIC_REQ, &fp_pmic_ldo_set);
}

static int sensor_buck_config(struct power_setting *pset, bool on)
{
	int ret = 0;
	struct hw_buck_ctrl_t *buck = hw_get_buck_ctrl();
	uint32_t val = on ? pset->val_on : pset->val_off;
	int state = on ? SEN_POWER_ON : SEN_POWER_OFF;

	if (buck && buck->func_tbl && buck->func_tbl->buck_power_config)
		ret = buck->func_tbl->buck_power_config(buck, val, state);
	else
		cam_err("failed to get buck ctrl");
	return ret;
}

/* boost on/off config, v300 reuse this power for lightstrap, v350 don't inherit */
static int sensor_boost_config(struct power_setting *pset, bool on)
{
	uint32_t val = on ? BOOST_5V_ENABLE : BOOST_5V_DISABLE;

	(void)pset;
	return boost_5v_enable(val, BOOST_CTRL_CAMERA);
}

static struct shared_data *devm_hwpower_get(struct device *dev,
					    const char *name)
{
	uint32_t i;
	static const struct hwpower_subtype {
		const char *power_type;
		power_config_t power_config;
		uint32_t id;
	} subtypes[] = {
		{ "pmic",     sensor_pmic_config,  0 }, // 0: pmic number for pmic1
		{ "pmic2",    sensor_pmic_config,  1 }, // 1: pmic number for pmic2
		{ "cam_buck", sensor_pmic_config,  2 }, // 2: pmic number for cam_buck
		{ "buck",     sensor_buck_config,  0 },
		{ "boost_5v", sensor_boost_config, 0 },
	};

	(void)dev;
	for (i = 0; i < ARRAY_SIZE(subtypes); ++i)
		if (strcmp(subtypes[i].power_type, name) == 0)
			break;
	if (i == ARRAY_SIZE(subtypes))
		return NULL;

	return get_shared_data(subtypes[i].power_config, subtypes[i].id);
}

static int sensor_pinctrl_config(struct power_setting *pset, bool on)
{
	struct shared_data *data = pset->private;
	const char *state = on ? "default" : "idle";

	return hisp_extra_pinctrl_select_state((int)data->id, state);
}

static struct shared_data *devm_extra_pinctrl_get(struct device *dev, const char *name)
{
	int id;

	id = hisp_extra_pinctrl_index(name);
	if (id < 0)
		return NULL;

	return get_shared_data(sensor_pinctrl_config, id);
}

static int sensor_gpio_input_output(uint32_t gpio, int *val, const char *name,
				    bool input)
{
	int ret;

	if (!name)
		name = "unknown";

	ret = gpio_request(gpio, name);
	if (ret != 0) {
		cam_err("fail to request gpio:%u for function:%s", gpio, name);
		return ret;
	}

	if (input) {
		ret = gpio_direction_input(gpio);
		*val = gpio_get_value(gpio);
	} else {
		ret = gpio_direction_output(gpio, *val);
	}

	if (ret != 0)
		cam_err("fail to set gpio:%u as %s for function:%s", gpio,
			(input ? "input" : "output"), name);

	gpio_free(gpio);
	return ret;
}

static int sensor_gpio_input(uint32_t gpio, const char *name)
{
	int val = 0;

	if (sensor_gpio_input_output(gpio, &val, name, true) != 0)
		return -EIO;
	return val;
}

static int sensor_gpio_output(uint32_t gpio, int val, const char *name)
{
	return sensor_gpio_input_output(gpio, &val, name, false);
}

static int sensor_gpio_config(struct power_setting *pset, bool on)
{
	struct shared_data *data = pset->private;
	uint32_t output = on ? pset->val_on : pset->val_off;

	return sensor_gpio_output(data->id, (int)output, pset->name);
}

static int sensor_get_named_gpio(struct device *dev, const char *name)
{
	int idx;
	int cnt;
	int ret;
	const char *gpio_name = NULL;
	struct gpio_desc *gd;
	struct device_node *np = dev->of_node;

	// check if gpio is provided via ${name}-gpio[s] = <&gpiox x x>
	gd = gpiod_get(dev, name, GPIOD_ASIS);
	if (!IS_ERR_OR_NULL(gd)) {
		ret = desc_to_gpio(gd);
		gpiod_put(gd);
		return ret;
	}

	// else should be provided via huawei,gpio-ctrl-types
	cnt = of_property_count_strings(np, "huawei,gpio-ctrl-types");
	for (idx = 0; idx < cnt; ++idx) {
		ret = of_property_read_string_index(
			np, "huawei,gpio-ctrl-types", idx, &gpio_name);
		if (ret < 0)
			return ret;
		if (strcmp(gpio_name, name) == 0)
			return of_get_gpio(np, idx);
	}
	return -ENOENT;
}

/**
 * devm_gpio_get - fake devm interface, not really allocating devres
 * @dev: device to get gpio @name from
 * @name: gpio name to get

 * @return the shared_data in linked list, same gpio number share same data
 */
static struct shared_data *devm_gpio_get(struct device *dev, const char *name)
{
	int gpio;

	gpio = sensor_get_named_gpio(dev, name);
	if (gpio <= 0) {
		dev_err(dev, "%pOF get gpio:%s failed", dev->of_node, name);
		return NULL;
	}
	return get_shared_data(&sensor_gpio_config, gpio);
}

static int sensor_clk_enable(struct power_setting *pset)
{
	int ret = 0;
	struct clk *clk = pset->private;

	if (pset->val_on) { // incase some clk is fixed freq
		ret = clk_set_rate(clk, pset->val_on);
		if (ret)
			return ret;
	}
	return clk_prepare_enable(clk);
}

static void sensor_clk_disable(struct power_setting *pset)
{
	struct clk *clk = pset->private;

	if (pset->val_off) // without explicit off value, just disable
		clk_set_rate(clk, pset->val_off);
	clk_disable_unprepare(clk);
}

static int sensor_ldo_enable(struct power_setting *pset)
{
	int ret = 0;
	struct regulator *supply = pset->private;

	if (pset->val_on) {
		if (pset->val_on_max)
			// The val_on_max voltage regulation range is added.
			ret = regulator_set_voltage(supply, (int)pset->val_on, (int)pset->val_on_max);
		else
			ret = regulator_set_voltage(supply, (int)pset->val_on, (int)pset->val_on);
		if (ret)
			return ret;
	}
	return regulator_enable(supply);
}

/**
 * sensor_ldo_disable - disable regulator, set voltage to [0, INT_MAX]
 * regulator core will find intersection of all consumers requested voltage range,
 * and it will fail if it can't find, so make the disabled consumer more tolerant.
 * regulator core will clamp [0, INT_MAX] to regulator's capability
 **/
static void sensor_ldo_disable(struct power_setting *pset)
{
	struct regulator *supply = pset->private;

	if (pset->val_off)
		regulator_set_voltage(supply, pset->val_off, INT_MAX);
	else
		regulator_set_voltage(supply, 0, INT_MAX);

	regulator_disable(supply);
}

enum devm_get_type {
	DEVM_GET_CLOCK,
	DEVM_GET_REGULATOR,
	DEVM_GET_GPIO,
	DEVM_GET_HWPOWER,
	DEVM_GET_PINCTRL,
};

struct power_type_ops {
	const char *prefix;
	enum devm_get_type type;
	power_enable_t enable;
	power_disable_t disable;
};

static struct power_type_ops generic_typeops[] = {
	{ "clock-",   DEVM_GET_CLOCK,     sensor_clk_enable, sensor_clk_disable },
	{ "ldo-",     DEVM_GET_REGULATOR, sensor_ldo_enable, sensor_ldo_disable },
	{ "gpio-",    DEVM_GET_GPIO,      sensor_ref_enable, sensor_ref_disable },
	{ "hwpower-", DEVM_GET_HWPOWER,   sensor_ref_enable, sensor_ref_disable },
	{ "pinctrl-", DEVM_GET_PINCTRL,   sensor_ref_enable, sensor_ref_disable },
};

static struct power_type_ops *find_generic_ops(const char *name)
{
	uint32_t i;
	struct power_type_ops *ops;

	for (i = 0; i < ARRAY_SIZE(generic_typeops); ++i) {
		ops = &generic_typeops[i];
		if (strncmp(ops->prefix, name, strlen(ops->prefix)) == 0)
			return ops;
	}
	return NULL;
}

static bool is_generic_name(const char *name)
{
	return find_generic_ops(name) != NULL;
}

static int parse_single_power_setting(struct device *dev,
				      struct power_setting *pset)
{
	const char *suffix_name;
	struct power_type_ops *ops = find_generic_ops(pset->name);

	if (!ops) {
		dev_err(dev, "power setting seq-type: %s not matched",
			pset->dts_name);
		return -EINVAL;
	}

	pset->enable = ops->enable;
	pset->disable = ops->disable;
	suffix_name = pset->name + strlen(ops->prefix);
	switch (ops->type) {
	case DEVM_GET_CLOCK:
		pset->private = devm_clk_get(dev, suffix_name);
		break;
	case DEVM_GET_REGULATOR:
		pset->private = devm_regulator_get(dev, suffix_name);
		break;
	case DEVM_GET_GPIO:
		pset->private = devm_gpio_get(dev, suffix_name);
		break;
	case DEVM_GET_HWPOWER:
		pset->private = devm_hwpower_get(dev, suffix_name);
		break;
	case DEVM_GET_PINCTRL:
		pset->private = devm_extra_pinctrl_get(dev, suffix_name);
		break;
	default:
		pset->private = NULL;
		return -EFAULT;
	}
	if (IS_ERR_OR_NULL(pset->private)) {
		dev_err(dev, "%pOF fail to get %s, generic name: %s",
			dev->of_node, pset->dts_name, pset->name);
		return -EFAULT;
	}
	return 0;
}

static inline bool pset_group_invalid(struct power_setting_group *grp,
		enum power_setting_type ps_type)
{
	if (!grp || grp->pst_array[ps_type].nsettings <= 0) {
		cam_err("giving invalid power group");
		return true;
	}
	return false;
}

static int get_pset_gpio_settings(struct power_setting_group *grp, struct sensor_gpio *sg)
{
	struct shared_data *sd;
	struct power_setting *pset;
	struct power_setting *end;
	struct power_setting_array *psta;

	psta = &grp->pst_array[PS_SENSOR];
	pset = psta->settings;
	end = psta->settings + psta->nsettings;

	while (pset != end) {
		if (strncmp(pset->name, "gpio-", strlen("gpio-")) == 0) {
			sd = pset->private;
			if (sd->id == sg->gpio) {
				sg->ovalue = (int)pset->val_on;
				sg->delay_on = (int)pset->delay_on;
				sg->delay_off = (int)pset->delay_off;
				return 0;
			}
		}
		++pset;
	}
	cam_err("not found gpio setting");
	return -EINVAL;
}

static void parse_reused_gpios(struct power_setting_group *grp)
{
	uint32_t idx;
	struct sensor_gpio *sg;
	static struct gpio_name_type_pair {
		const char *name;
		uint32_t type;
	} pair[] = {
		{ "reset",     SG_RESET },
		{ "mipisw",    SG_MIPISW },
		{ "fsin",      SG_FSIN },
		{ "btb_check", SG_BTBCHECK },
	};

	for (idx = 0; idx < ARRAY_SIZE(pair); ++idx) {
		sg = &grp->sgpio[pair[idx].type];
		sg->name = pair[idx].name;
		sg->gpio = sensor_get_named_gpio(grp->dev, pair[idx].name);
		get_pset_gpio_settings(grp, sg);
	}
}

/* sensor_reused_gpio_input - read input value of reused sgpio[tag] */
static inline void sensor_reused_gpio_input(struct power_setting_group *grp,
					    uint32_t tag)
{
	struct sensor_gpio *sg = &grp->sgpio[tag];

	if (sg->gpio <= 0)
		sg->ivalue = -ENOENT;
	else
		sg->ivalue = sensor_gpio_input(sg->gpio, sg->name);
}

static bool parse_power_setting_by_type(struct device *dev,
		struct power_setting_group *grp, enum power_setting_type type)
{
	int i;
	int nsettings;
	bool val_off_exist; // SEQ_CFG_VAL_OFF property exist
	bool val_off_max_exist; // SEQ_CFG_VAL_OFF_MAX property exist

	struct power_setting *pset;
	struct power_setting_array *pset_arr;
	const struct parse_seq_string* seq_str;

	seq_str = &parse_flag[type];
	if (!seq_str)
		return false;
	nsettings = of_property_count_strings(dev->of_node, seq_str->seq_type);
	if (nsettings <= 0) {
		dev_err(dev, "%pOF has no prop: %s", dev->of_node, seq_str->seq_type);
		return false;
	}
	val_off_exist = of_property_count_u32_elems(dev->of_node, seq_str->seq_cfg_val_off) == nsettings;
	if (val_off_exist)
		dev_info(dev, "%pOF has prop: %s", dev->of_node, seq_str->seq_cfg_val_off);

	val_off_max_exist = of_property_count_u32_elems(dev->of_node, seq_str->seq_cfg_val_off_max) == nsettings;
	if (val_off_max_exist)
		dev_info(dev, "%pOF has prop: %s", dev->of_node, seq_str->seq_cfg_val_off_max);

	pset_arr = &grp->pst_array[type];
	pset_arr->settings = devm_kzalloc(dev, nsettings * sizeof(*pset), GFP_KERNEL);
	pset_arr->nsettings = nsettings;
	if (!pset_arr->settings)
		return false;
	pset_arr = &grp->pst_array[type];
	for (i = 0; i < pset_arr->nsettings; ++i) {
		pset = &pset_arr->settings[i];
		pset->grp = grp;
		// returning NULL cause probe fail, device resource fwk will help free grp
		if (sensor_get_seq_data(dev, i, pset, type) != 0)
			return false;
		if (parse_single_power_setting(dev, pset) != 0)
			return false;

		// NOTE: for some unknown reason, on value of gpio is flipped when output
		// just flip it here, for gpio config convenience
		// if cfg-val-off exists, just use off value instead of opposite val_on
		if (strncmp(pset->name, "gpio-", strlen("gpio-")) == 0) {
			pset->val_on = (pset->val_on + 1) % 2;
			pset->val_on_max = (pset->val_on_max + 1) % 2;
			if (!val_off_exist)
				pset->val_off = pset->val_on ^ 1U;
			if (!val_off_max_exist)
				pset->val_off_max = pset->val_on_max ^ 1U;
		}
	}
	return true;
}

/**
 * parse_power_setting - parse power setting from dts
 *
 * return ptr of power_setting_group or NULL if fail to parse.
 */
struct power_setting_group *parse_power_setting(struct device *dev)
{
	int type;
	struct power_setting_group *grp;

	if (!dev)
		return NULL;

	grp = devm_kzalloc(dev, sizeof(*grp), GFP_KERNEL);
	if (!grp)
		return NULL;
	grp->dev = dev;

	mutex_init(&grp->power_lock);

	if (!parse_power_setting_by_type(dev, grp, PS_SENSOR))
		return NULL;

	for (type = PS_VCM; type < PS_MAX; ++type)
		parse_power_setting_by_type(dev, grp, type);

	(void)of_property_read_u32(dev->of_node, "huawei,reset_not_btb_check",
		&grp->reset_not_btb_check_flag);
	parse_reused_gpios(grp);
	return grp;
}

/**
 * sensor_do_power_setting - for [beg, end), do enable/disable
 *	beg may behind end, then will do power_setting backward
 *
 * @return: for disable always 0, for enable, 0 if success, negative if failed.
 *	    if enable failed for a power_setting, previously enabled will be disabled
 */
static int sensor_do_power_setting(struct power_setting *beg,
				   struct power_setting *end, bool on)
{
	uint32_t delay;
	int step = beg < end ? 1 : -1;
	struct power_setting *pset = beg;

	while (pset != end) {
		if (on) {
			cam_debug("power-seq-on: %s val:%u", pset->name,
				  pset->val_on);
			if (pset->enable(pset) != 0)
				goto power_up_fail;
			delay = pset->delay_on;
		} else {
			cam_debug("power-seq-off: %s val:%u", pset->name,
				  pset->val_off);
			pset->disable(pset);
			delay = pset->delay_off;
		}
		if (delay > 2) // larger than 2ms, use sleep, will yield cpu
			msleep(delay);
		else if (delay != 0) // else use delay
			mdelay(delay);
		pset += step;
	}
	return 0;

power_up_fail:
	cam_err("power-seq-on: %s val: %u failed", pset->name, pset->val_on);
	sensor_do_power_setting(pset - step, beg - step, false);
	return -EINVAL;
}

int sensor_enable_power_setting(struct power_setting_group *grp)
{
	int ret = 0;
	struct power_setting* beg;
	struct power_setting* end;
	struct power_setting_array *psta;

	if (pset_group_invalid(grp, PS_SENSOR))
		return -EINVAL;

	mutex_lock(&grp->power_lock);
	if (grp->power_ref == 0) {
		if (grp->reset_not_btb_check_flag == 0)
			sensor_reused_gpio_input(grp, SG_RESET);
		ret = sensor_mipi_switch(grp);
		if (ret)
			goto err_out;

		psta = &grp->pst_array[PS_SENSOR];
		beg = psta->settings;
		end = psta->settings + psta->nsettings;
		ret = sensor_do_power_setting(beg, end, true);
		if (ret)
			goto err_out;
	}
	++grp->power_ref;
	dev_info(grp->dev, "power_up: power-seq-on, ref: %u", grp->power_ref);
err_out:
	if (ret)
		dev_err(grp->dev, "power_up: power-seq-on failed: %d", ret);
	mutex_unlock(&grp->power_lock);
	return ret;
}

void sensor_disable_power_setting(struct power_setting_group *grp)
{
	struct power_setting* beg;
	struct power_setting* end;
	struct power_setting_array *psta;

	if (pset_group_invalid(grp, PS_SENSOR))
		return;

	mutex_lock(&grp->power_lock);
	if (grp->power_ref == 1) {
		psta = &grp->pst_array[PS_SENSOR];
		beg = psta->settings + psta->nsettings - 1;
		end = psta->settings - 1;
		sensor_do_power_setting(beg, end, false);

		if (grp->reset_not_btb_check_flag == 0)
			sensor_reused_gpio_input(grp, SG_RESET);
	}
	if (grp->power_ref > 0)
		--grp->power_ref;
	dev_info(grp->dev, "power_down: power-seq-off, ref:%u", grp->power_ref);
	mutex_unlock(&grp->power_lock);
}

int sensor_enable_vcm_power_setting(struct power_setting_group* grp)
{
	int ret;
	struct power_setting* beg;
	struct power_setting* end;
	struct power_setting_array *psta;

	if (pset_group_invalid(grp, PS_VCM))
		return -EINVAL;

	mutex_lock(&grp->power_lock);

	psta = &grp->pst_array[PS_VCM];
	beg = psta->settings;
	end = psta->settings + psta->nsettings;
	ret = sensor_do_power_setting(beg, end, true);
	mutex_unlock(&grp->power_lock);
	return ret;
}

void sensor_disable_vcm_power_setting(struct power_setting_group* grp)
{
	struct power_setting* beg;
	struct power_setting* end;
	struct power_setting_array *psta;
	if (pset_group_invalid(grp, PS_VCM))
		return;

	mutex_lock(&grp->power_lock);
	psta = &grp->pst_array[PS_VCM];
	beg = psta->settings + psta->nsettings - 1;
	end = psta->settings - 1;
	sensor_do_power_setting(beg, end, false);
	mutex_unlock(&grp->power_lock);
}

int sensor_ext_vcm_power_on_off(struct power_setting_group *grp, bool on)
{
	if (pset_group_invalid(grp, PS_VCM))
		return -EINVAL;

	if (on)
		return sensor_enable_vcm_power_setting(grp);
	sensor_disable_vcm_power_setting(grp);
	return 0;
}

int sensor_enable_power_setting_by_type(struct power_setting_group* grp, enum power_setting_type ps_type)
{
	int ret;
	struct power_setting* beg;
	struct power_setting* end;
	struct power_setting_array *psta;

	if (pset_group_invalid(grp, ps_type))
		return -EINVAL;

	mutex_lock(&grp->power_lock);

	psta = &grp->pst_array[ps_type];
	beg = psta->settings;
	end = psta->settings + psta->nsettings;
	ret = sensor_do_power_setting(beg, end, true);
	mutex_unlock(&grp->power_lock);
	return ret;
}

void sensor_disable_power_setting_by_type(struct power_setting_group* grp, enum power_setting_type ps_type)
{
	struct power_setting* beg;
	struct power_setting* end;
	struct power_setting_array *psta;
	if (pset_group_invalid(grp, ps_type))
		return;

	mutex_lock(&grp->power_lock);
	psta = &grp->pst_array[ps_type];
	beg = psta->settings + psta->nsettings - 1;
	end = psta->settings - 1;
	sensor_do_power_setting(beg, end, false);
	mutex_unlock(&grp->power_lock);
}

int sensor_power_on_off_by_type(struct power_setting_group *grp, bool on, enum power_setting_type ps_type)
{
	if (ps_type == PS_PUBLIC && grp && grp->pst_array[ps_type].nsettings <= 0) {
		cam_info("%s: no public power", __func__);
		return 0;
	}
	if (pset_group_invalid(grp, ps_type))
		return -EINVAL;

	if (on)
		return sensor_enable_power_setting_by_type(grp, ps_type);
	sensor_disable_power_setting_by_type(grp, ps_type);
	return 0;
}

int sensor_mipi_switch(struct power_setting_group *grp)
{
	struct sensor_gpio *sg;

	if (pset_group_invalid(grp, PS_SENSOR))
		return -EINVAL;

	sg = &grp->sgpio[SG_MIPISW];
	if (sg->gpio > 0)
		return sensor_gpio_output(sg->gpio, sg->ovalue, sg->name);
	return 0;
}

int sensor_release_hold(struct power_setting_group *grp, bool release)
{
	int ret;
	int val;
	int delay;
	struct sensor_gpio *sg;

	if (pset_group_invalid(grp, PS_SENSOR))
		return -EINVAL;

	sg = &grp->sgpio[SG_RESET];
	if (sg->gpio <= 0) {
		cam_err("%s: reset gpio not found", __func__);
		return -ENOENT;
	}

	// ovalue value is disreset/release, inverted ovalue is hold
	val = release ? sg->ovalue : (sg->ovalue + 1) % 2;
	dev_info(grp->dev, "power-seq: %s", (release ? "release" : "hold"));
	ret = sensor_gpio_output(sg->gpio, val, sg->name);

	delay = release ? sg->delay_on : sg->delay_off;
	cam_info("gpio %s  delay %u ms", sg->name, delay);
	mdelay(delay);
	return ret;
}

int sensor_btb_check(struct power_setting_group *grp)
{
	if (pset_group_invalid(grp, PS_SENSOR))
		return -EINVAL;
	sensor_reused_gpio_input(grp, SG_BTBCHECK);
	if (grp->sgpio[SG_BTBCHECK].ivalue == GPIO_HIGH ||
	    grp->sgpio[SG_RESET].ivalue == GPIO_HIGH) {
		grp->sgpio[SG_RESET].ivalue = GPIO_LOW;
		return GPIO_HIGH;
	}
	return grp->sgpio[SG_BTBCHECK].ivalue;
}

int sensor_fsin_check(struct power_setting_group *grp)
{
	if (pset_group_invalid(grp, PS_SENSOR))
		return -EINVAL;
	sensor_reused_gpio_input(grp, SG_FSIN);
	return grp->sgpio[SG_FSIN].ivalue;
}
