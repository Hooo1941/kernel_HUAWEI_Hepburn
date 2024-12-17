/*
 * high_voltage_converter.c
 *
 * high voltage converter driver
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
#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/slab.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/of_device.h>
#include <linux/pinctrl/consumer.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>

#include <huawei_platform/log/hw_log.h>
#include <securec.h>
#include "../smartpakit.h"

#if IS_ENABLED(CONFIG_HUAWEI_DSM_AUDIO)
#include <dsm_audio/dsm_audio.h>
#endif

#define HWLOG_TAG smartpakit
HWLOG_REGIST();

#define RETRY_TIMES 6
#define I2C_READ 0
#define I2C_WRITE 1

#if IS_ENABLED(CONFIG_HUAWEI_DSM_AUDIO)
#define DSM_BUF_SIZE DSM_SMARTPA_BUF_SIZE
#endif

struct high_vol_cnv_reg_ctl {
	unsigned int addr;
	unsigned int mask;
	union {
		unsigned int value;
		unsigned int chip_version;
	};
	unsigned int delay;
	unsigned int ctl_type;
};

struct high_vol_cnv_reg_ctl_sequence {
	unsigned int num;
	struct high_vol_cnv_reg_ctl *regs;
};

struct high_vol_cnv_gpio_irq {
	int gpio;
	int irq;
	unsigned int irqflags;
	char gpio_name[SMARTPAKIT_NAME_MAX];
	char irq_name[SMARTPAKIT_NAME_MAX];
	struct high_vol_cnv_reg_ctl_sequence *rw_sequence;
};

struct high_voltage_converter_priv {
	unsigned int num_by_same_i2c_addr;
	unsigned int iter;
	struct device *dev;
	struct i2c_client *i2c;
	/* version regs */
	struct high_vol_cnv_reg_ctl_sequence *version_regs_seq;
	/* init regs */
	struct high_vol_cnv_reg_ctl_sequence *init_regs_seq;
	/* dump regs */
	struct high_vol_cnv_reg_ctl_sequence *dump_regs_sequence;
	bool need_version_check;
	/* notice: ctl_type in regs use as mask in version_val_check_seq */
	struct high_vol_cnv_reg_ctl_sequence *version_val_check_seq;
	/* irq */
	struct high_vol_cnv_gpio_irq *irq_handler;
	struct work_struct irq_handle_work;
	unsigned int switch_num;
	bool switch_gpio_requested;
	struct smartpakit_switch_node *switch_ctl;
	/* reg map config */
	struct smartpakit_regmap_cfg *regmap_cfg;
	/* record wirten regs */
	struct high_vol_cnv_reg_ctl_sequence *record_wirten_seq;
	int probe_completed;
	struct pinctrl *pinctrl;
	struct mutex irq_handler_lock;
	struct mutex gpio_request_lock;
	/* irq filter regs */
	struct high_vol_cnv_reg_ctl_sequence *irq_filter_seq;
};

enum high_vol_cnv_reg_ctl_type {
	HIGH_VOL_CNV_REG_CTL_TYPE_R = 0,  /* read reg        */
	HIGH_VOL_CNV_REG_CTL_TYPE_W,      /* write reg       */
	HIGH_VOL_CNV_REG_CTL_TYPE_DELAY,  /* only time delay */
	HIGH_VOL_CNV_PARAM_NODE_TYPE_GPIO,
	HIGH_VOL_CNV_REG_CTL_TYPE_MAX,
};

#define IN_FUNCTION hwlog_info("%s comein\n", __func__)
#define OUT_FUNCTION hwlog_info("%s comeout\n", __func__)

static atomic_t cnv_probe_ilde_flag = ATOMIC_INIT(0);
static DEFINE_SPINLOCK(cnv_probe_spinlock);

static bool high_vol_cnv_probe_is_ilde(void)
{
	unsigned long flags;

	spin_lock_irqsave(&cnv_probe_spinlock, flags);
	if (atomic_read(&cnv_probe_ilde_flag) == 0) {
		atomic_set(&cnv_probe_ilde_flag, 1);
		spin_unlock_irqrestore(&cnv_probe_spinlock, flags);
		return true;
	}
	spin_unlock_irqrestore(&cnv_probe_spinlock, flags);
	return false;
}

static void high_vol_cnv_set_probe_ilde(void)
{
	unsigned long flags;

	spin_lock_irqsave(&cnv_probe_spinlock, flags);
	atomic_set(&cnv_probe_ilde_flag, 0);
	spin_unlock_irqrestore(&cnv_probe_spinlock, flags);
}

static inline int high_vol_cnv_regmap_read(struct regmap *regmap,
	unsigned int reg_addr, unsigned int *value)
{
	int ret = 0;
	int i;

	for (i = 0; i < RETRY_TIMES; i++) {
		ret = regmap_read(regmap, reg_addr, value);
		if (ret == 0)
			break;

		mdelay(1);
	}
	return ret;
}

static inline int high_vol_cnv_regmap_write(struct regmap *regmap,
	unsigned int reg_addr, unsigned int value)
{
	int ret = 0;
	int i;

	for (i = 0; i < RETRY_TIMES; i++) {
		ret = regmap_write(regmap, reg_addr, value);
		if (ret == 0)
			break;

		mdelay(1);
	}
	return ret;
}

static int high_vol_cnv_regmap_update_bits(struct regmap *regmap,
	unsigned int reg_addr, unsigned int mask, unsigned int value)
{
	int ret;
	int i;

	for (i = 0; i < RETRY_TIMES; i++) {
		ret = regmap_update_bits(regmap, reg_addr, mask, value);
		if (ret == 0)
			break;

		mdelay(1);
	}
	return ret;
}

static int high_vol_cnv_regmap_complex_write(struct smartpakit_regmap_cfg *cfg,
	unsigned int reg_addr, unsigned int mask, unsigned int value)
{
	int ret;

	if (cfg == NULL)
		return -EINVAL;

	if ((mask ^ cfg->value_mask) == 0)
		ret = high_vol_cnv_regmap_write(cfg->regmap, reg_addr, value);
	else
		ret = high_vol_cnv_regmap_update_bits(cfg->regmap, reg_addr,
			mask, value);

	return ret;
}

#if IS_ENABLED (CONFIG_HUAWEI_DSM_AUDIO)
static void high_vol_cnv_append_dsm_report(char *dst, char *fmt, ...)
{
	va_list args;
	unsigned int buf_len;
	char tmp_str[DSM_BUF_SIZE] = {0};

	if ((dst == NULL) || (fmt == NULL)) {
		hwlog_err("%s, dst or src is NULL\n", __func__);
		return;
	}

	va_start(args, fmt);
	vscnprintf(tmp_str, (unsigned long)DSM_BUF_SIZE, (const char *)fmt,
		args);
	va_end(args);

	buf_len = DSM_BUF_SIZE - strlen(dst) - 1;
	if (strlen(dst) < DSM_BUF_SIZE - 1) {
		if (strncat_s(dst, buf_len, tmp_str, strlen(tmp_str)) != EOK)
			hwlog_err("%s: strncat_s is failed\n", __func__);
	}
}
#endif

static void high_vol_cnv_dsm_report_by_i2c_error(int flag, int errno,
	struct i2c_err_info *info)
{
	char *report = NULL;

	unused(report);
	if (errno == 0)
		return;

#if IS_ENABLED (CONFIG_HUAWEI_DSM_AUDIO)
	report = kzalloc(sizeof(char) * DSM_BUF_SIZE, GFP_KERNEL);
	if (!report)
		return;

	if (flag == I2C_READ) /* read i2c error */
		high_vol_cnv_append_dsm_report(report, "read ");
	else /* flag write i2c error == 1 */
		high_vol_cnv_append_dsm_report(report, "write ");

	high_vol_cnv_append_dsm_report(report, "errno %d", errno);

	if (info != NULL)
		high_vol_cnv_append_dsm_report(report,
			" %u fail times of %u all times, err_details is 0x%lx",
			info->err_count, info->regs_num, info->err_details);

	audio_dsm_report_info(AUDIO_SMARTPA, DSM_SMARTPA_I2C_ERR, "%s", report);
	hwlog_info("%s: dsm report, %s\n", __func__, report);
	kfree(report);
#endif
}

static void high_vol_cnv_i2c_dsm_report(
	struct high_voltage_converter_priv *i2c_priv,
	int flag, int errno, struct i2c_err_info *info)
{
	if (i2c_priv->probe_completed == 0)
		return;
	high_vol_cnv_dsm_report_by_i2c_error(flag, errno, info);
}

static void high_vol_cnv_i2c_free_reg_ctl(
	struct high_vol_cnv_reg_ctl_sequence **reg_ctl)
{
	if (reg_ctl == NULL || *reg_ctl == NULL)
		return;

	smartpakit_kfree_ops((*reg_ctl)->regs);
	kfree(*reg_ctl);
	*reg_ctl = NULL;
}

static int high_vol_cnv_i2c_parse_reg_ctl(
	struct high_vol_cnv_reg_ctl_sequence **reg_ctl,
	struct device_node *node,
	const char *ctl_str)
{
	struct high_vol_cnv_reg_ctl_sequence *ctl = NULL;
	int count = 0;
	int val_num;
	int ret;

	if ((node == NULL) || (ctl_str == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	ctl = kzalloc(sizeof(*ctl), GFP_KERNEL);
	if (ctl == NULL)
		return -ENOMEM;

	ret = smartpakit_get_prop_of_u32_array(node, ctl_str,
		(u32 **)&ctl->regs, &count);
	if ((count <= 0) || (ret < 0)) {
		hwlog_err("%s: get %s failed or count is 0\n",
			__func__, ctl_str);
		ret = -EFAULT;
		goto parse_reg_ctl_err_out;
	}

	val_num = sizeof(struct high_vol_cnv_reg_ctl) / sizeof(unsigned int);
	if ((count % val_num) != 0) {
		hwlog_err("%s: count %d %% val_num %d != 0\n",
			__func__, count, val_num);
		ret = -EFAULT;
		goto parse_reg_ctl_err_out;
	}

	ctl->num = (unsigned int)(count / val_num);
	*reg_ctl = ctl;
	return 0;

parse_reg_ctl_err_out:
	high_vol_cnv_i2c_free_reg_ctl(&ctl);
	return ret;
}

static bool high_vol_cnv_i2c_is_reg_in_special_range(unsigned int reg_addr,
	int num, unsigned int *reg)
{
	int i;

	if ((num == 0) || (reg == NULL)) {
		hwlog_err("%s: invalid arg\n", __func__);
		return false;
	}

	for (i = 0; i < num; i++) {
		if (reg[i] == reg_addr)
			return true;
	}
	return false;
}

static struct smartpakit_regmap_cfg *high_vol_cnv_i2c_get_regmap_cfg(
	struct device *dev)
{
	struct high_voltage_converter_priv *i2c_priv = NULL;

	if (dev == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return NULL;
	}
	i2c_priv = dev_get_drvdata(dev);
	if ((i2c_priv == NULL) || (i2c_priv->regmap_cfg == NULL)) {
		hwlog_err("%s: regmap_cfg invalid argument\n", __func__);
		return NULL;
	}
	return i2c_priv->regmap_cfg;
}

static bool high_vol_cnv_i2c_writeable_reg(struct device *dev, unsigned int reg)
{
	struct smartpakit_regmap_cfg *cfg = NULL;

	cfg = high_vol_cnv_i2c_get_regmap_cfg(dev);
	if (cfg == NULL)
		return false;
	/* config writable or unwritable, can not config in dts at same time */
	if (cfg->num_writeable > 0)
		return high_vol_cnv_i2c_is_reg_in_special_range(reg,
			cfg->num_writeable, cfg->reg_writeable);
	if (cfg->num_unwriteable > 0)
		return !high_vol_cnv_i2c_is_reg_in_special_range(reg,
			cfg->num_unwriteable, cfg->reg_unwriteable);

	return true;
}

static bool high_vol_cnv_i2c_readable_reg(struct device *dev, unsigned int reg)
{
	struct smartpakit_regmap_cfg *cfg = NULL;

	cfg = high_vol_cnv_i2c_get_regmap_cfg(dev);
	if (cfg == NULL)
		return false;
	/* config readable or unreadable, can not config in dts at same time */
	if (cfg->num_readable > 0)
		return high_vol_cnv_i2c_is_reg_in_special_range(reg,
			cfg->num_readable, cfg->reg_readable);
	if (cfg->num_unreadable > 0)
		return !high_vol_cnv_i2c_is_reg_in_special_range(reg,
			cfg->num_unreadable, cfg->reg_unreadable);

	return true;
}

static bool high_vol_cnv_i2c_volatile_reg(struct device *dev, unsigned int reg)
{
	struct smartpakit_regmap_cfg *cfg = NULL;

	cfg = high_vol_cnv_i2c_get_regmap_cfg(dev);
	if (cfg == NULL)
		return false;
	/* config volatile or unvolatile, can not config in dts at same time */
	if (cfg->num_volatile > 0)
		return high_vol_cnv_i2c_is_reg_in_special_range(reg,
			cfg->num_volatile, cfg->reg_volatile);
	if (cfg->num_unvolatile > 0)
		return !high_vol_cnv_i2c_is_reg_in_special_range(reg,
			cfg->num_unvolatile, cfg->reg_unvolatile);

	return true;
}

static unsigned int high_vol_cnv_i2c_get_reg_value_mask(int val_bits)
{
	unsigned int mask;

	if (val_bits == SMARTPAKIT_REG_VALUE_B16)
		mask = SMARTPAKIT_REG_VALUE_M16;
	else if (val_bits == SMARTPAKIT_REG_VALUE_B24)
		mask = SMARTPAKIT_REG_VALUE_M24;
	else if (val_bits == SMARTPAKIT_REG_VALUE_B32)
		mask = SMARTPAKIT_REG_VALUE_M32;
	else /* SMARTPAKIT_REG_VALUE_B8 or other */
		mask = SMARTPAKIT_REG_VALUE_M8;

	return mask;
}

static void high_vol_cnv_i2c_free_regmap_cfg(struct smartpakit_regmap_cfg **cfg)
{
	if ((cfg == NULL) || (*cfg == NULL))
		return;

	smartpakit_kfree_ops((*cfg)->reg_writeable);
	smartpakit_kfree_ops((*cfg)->reg_unwriteable);
	smartpakit_kfree_ops((*cfg)->reg_readable);
	smartpakit_kfree_ops((*cfg)->reg_unreadable);
	smartpakit_kfree_ops((*cfg)->reg_volatile);
	smartpakit_kfree_ops((*cfg)->reg_unvolatile);
	smartpakit_kfree_ops((*cfg)->reg_defaults);

	kfree((*cfg));
	(*cfg) = NULL;
}

static int high_vol_cnv_i2c_parse_reg_defaults(struct device_node *node,
	struct smartpakit_regmap_cfg *cfg_info)
{
	const char *reg_defaults_str = "reg_defaults";

	return smartpakit_get_prop_of_u32_array(node, reg_defaults_str,
		(u32 **)&cfg_info->reg_defaults, &cfg_info->num_defaults);
}

static int high_vol_cnv_i2c_parse_special_regs_range(struct device_node *node,
	struct smartpakit_regmap_cfg *cfg_info)
{
	const char *reg_writeable_str   = "reg_writeable";
	const char *reg_unwriteable_str = "reg_unwriteable";
	const char *reg_readable_str    = "reg_readable";
	const char *reg_unreadable_str  = "reg_unreadable";
	const char *reg_volatile_str    = "reg_volatile";
	const char *reg_unvolatile_str  = "reg_unvolatile";
	int ret;

	cfg_info->num_writeable   = 0;
	cfg_info->num_unwriteable = 0;
	cfg_info->num_readable    = 0;
	cfg_info->num_unreadable  = 0;
	cfg_info->num_volatile    = 0;
	cfg_info->num_unvolatile  = 0;
	cfg_info->num_defaults    = 0;

	ret = smartpakit_get_prop_of_u32_array(node, reg_writeable_str,
		&cfg_info->reg_writeable, &cfg_info->num_writeable);
	ret += smartpakit_get_prop_of_u32_array(node, reg_unwriteable_str,
		&cfg_info->reg_unwriteable, &cfg_info->num_unwriteable);
	ret += smartpakit_get_prop_of_u32_array(node, reg_readable_str,
		&cfg_info->reg_readable, &cfg_info->num_readable);
	ret += smartpakit_get_prop_of_u32_array(node, reg_unreadable_str,
		&cfg_info->reg_unreadable, &cfg_info->num_unreadable);
	ret += smartpakit_get_prop_of_u32_array(node, reg_volatile_str,
		&cfg_info->reg_volatile, &cfg_info->num_volatile);
	ret += smartpakit_get_prop_of_u32_array(node, reg_unvolatile_str,
		&cfg_info->reg_unvolatile, &cfg_info->num_unvolatile);
	ret += high_vol_cnv_i2c_parse_reg_defaults(node, cfg_info);
	return ret;
}

static int high_vol_cnv_i2c_parse_remap_cfg(struct device_node *node,
	struct smartpakit_regmap_cfg **cfg)
{
	struct smartpakit_regmap_cfg *cfg_info = NULL;
	const char *reg_bits_str     = "reg_bits";
	const char *val_bits_str     = "val_bits";
	const char *cache_type_str   = "cache_type";
	const char *max_register_str = "max_register";
	int ret;

	cfg_info = kzalloc(sizeof(*cfg_info), GFP_KERNEL);
	if (cfg_info == NULL)
		return -ENOMEM;

	ret = of_property_read_u32(node, reg_bits_str,
		(u32 *)&cfg_info->cfg.reg_bits);
	if (ret < 0) {
		hwlog_err("%s: get reg_bits failed\n", __func__);
		ret = -EFAULT;
		goto high_vol_cnv_parse_remap_err_out;
	}

	ret = of_property_read_u32(node, val_bits_str,
		(u32 *)&cfg_info->cfg.val_bits);
	if (ret < 0) {
		hwlog_err("%s: get val_bits failed\n", __func__);
		ret = -EFAULT;
		goto high_vol_cnv_parse_remap_err_out;
	}
	cfg_info->value_mask = high_vol_cnv_i2c_get_reg_value_mask(
		cfg_info->cfg.val_bits);

	ret = of_property_read_u32(node, cache_type_str,
		(u32 *)&cfg_info->cfg.cache_type);
	if ((ret < 0) || (cfg_info->cfg.cache_type > REGCACHE_FLAT)) {
		hwlog_err("%s: get cache_type failed\n", __func__);
		ret = -EFAULT;
		goto high_vol_cnv_parse_remap_err_out;
	}

	ret = of_property_read_u32(node, max_register_str,
		&cfg_info->cfg.max_register);
	if (ret < 0) {
		hwlog_err("%s: get max_register failed\n", __func__);
		ret = -EFAULT;
		goto high_vol_cnv_parse_remap_err_out;
	}

	ret = high_vol_cnv_i2c_parse_special_regs_range(node, cfg_info);
	if (ret < 0)
		goto high_vol_cnv_parse_remap_err_out;

	*cfg = cfg_info;
	return 0;

high_vol_cnv_parse_remap_err_out:
	high_vol_cnv_i2c_free_regmap_cfg(&cfg_info);
	return ret;
}


static int high_vol_cnv_i2c_regmap_init(struct i2c_client *i2c,
	struct device_node *adc_node,
	struct high_voltage_converter_priv *i2c_priv)
{
	const char *regmap_cfg_str = "regmap_cfg";
	struct smartpakit_regmap_cfg *cfg = NULL;
	struct device_node *node = NULL;
	int ret;
	int val_num;

	node = of_get_child_by_name(adc_node, regmap_cfg_str);
	if (node == NULL) {
		hwlog_debug("%s: regmap_cfg not existed, skip\n", __func__);
		return 0;
	}

	ret = high_vol_cnv_i2c_parse_remap_cfg(node, &i2c_priv->regmap_cfg);
	if (ret < 0)
		return ret;

	cfg = i2c_priv->regmap_cfg;
	val_num = sizeof(struct reg_default) / sizeof(unsigned int);
	if (cfg->num_defaults > 0) {
		if ((cfg->num_defaults % val_num) != 0) {
			hwlog_err("%s: reg_defaults %d%%%d != 0\n",
				__func__, cfg->num_defaults, val_num);
			ret = -EFAULT;
			goto regmap_init_err_out;
		}
	}

#if !IS_ENABLED(CONFIG_FINAL_RELEASE)
	hwlog_info("%s: regmap_cfg get w%d,%d,r%d,%d,v%d,%d,default%d\n",
		__func__, cfg->num_writeable, cfg->num_unwriteable,
		cfg->num_readable, cfg->num_unreadable, cfg->num_volatile,
		cfg->num_unvolatile, cfg->num_defaults / val_num);
#endif
	/* set num_reg_defaults */
	if (cfg->num_defaults > 0) {
		cfg->num_defaults /= val_num;
		cfg->cfg.reg_defaults = cfg->reg_defaults;
		cfg->cfg.num_reg_defaults = (unsigned int)cfg->num_defaults;
	}

	cfg->cfg.writeable_reg = high_vol_cnv_i2c_writeable_reg;
	cfg->cfg.readable_reg  = high_vol_cnv_i2c_readable_reg;
	cfg->cfg.volatile_reg  = high_vol_cnv_i2c_volatile_reg;

	cfg->regmap = regmap_init_i2c(i2c, &cfg->cfg);
	if (IS_ERR(cfg->regmap)) {
		hwlog_err("%s: regmap_init_i2c regmap failed\n", __func__);
		ret = -EFAULT;
		goto regmap_init_err_out;
	}
	return 0;
regmap_init_err_out:
	high_vol_cnv_i2c_free_regmap_cfg(&i2c_priv->regmap_cfg);
	return ret;
}

static int high_vol_cnv_i2c_regmap_deinit(
	struct high_voltage_converter_priv *i2c_priv)
{
	struct smartpakit_regmap_cfg *cfg = NULL;

	if ((i2c_priv == NULL) || (i2c_priv->regmap_cfg == NULL) ||
		(i2c_priv->regmap_cfg->regmap == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}
	cfg = i2c_priv->regmap_cfg;
	regmap_exit(cfg->regmap);
	cfg->regmap = NULL;
	high_vol_cnv_i2c_free_regmap_cfg(&cfg);
	return 0;
}

static int get_high_vol_cnv_node(struct i2c_client *i2c,
	struct high_voltage_converter_priv *i2c_priv, struct device_node **node)
{
	int ret;
	char name[SMARTPAKIT_NAME_MAX] = {0};

	if (i2c_priv->num_by_same_i2c_addr == 1) {
		*node = i2c->dev.of_node;
		return 0;
	}

	ret = snprintf_s(name, SMARTPAKIT_NAME_MAX, SMARTPAKIT_NAME_MAX - 1,
		"converter%u", i2c_priv->iter);
	if (ret < 0) {
		hwlog_err("%s: set iter name failed\n", __func__);
		return ret;
	}

	*node = of_get_child_by_name(i2c->dev.of_node, name);
	if (!(*node)) {
		hwlog_err("%s: get node %s fail\n", __func__, name);
		return -EINVAL;
	}

	return 0;
}

static void high_vol_cnv_delay(unsigned int delay)
{
	if (delay == 0)
		return;

	if (delay > SMARTPAKIT_DELAY_US_TO_MS)
		msleep(delay / SMARTPAKIT_DELAY_US_TO_MS);
	else
		udelay(delay);
}

static int high_vol_cnv_get_pinctrl(struct high_voltage_converter_priv *i2c_priv)
{
	struct i2c_client *i2c = i2c_priv->i2c;

	if (i2c == NULL) {
		hwlog_err("%s: i2c_client is invaild\n", __func__);
		return -EINVAL;
	}

	if (IS_ERR_OR_NULL(i2c_priv->pinctrl)) {
		i2c_priv->pinctrl = devm_pinctrl_get(&(i2c->dev));
		if (IS_ERR_OR_NULL(i2c_priv->pinctrl)) {
			hwlog_err("%s: get pinctrl fail %d\n",
				__func__, IS_ERR_OR_NULL(i2c_priv->pinctrl));
			return -EINVAL;
		}
	}

	return 0;
}

static int high_vol_cnv_pinctrl_select_state(
	struct high_voltage_converter_priv *i2c_priv,
	const char *name)
{
	struct pinctrl_state *pin_state = NULL;
	int ret = high_vol_cnv_get_pinctrl(i2c_priv);
	if (ret < 0)
		return ret;

	if (!name) {
		hwlog_err("%s:name is NULL\n", __func__);
		return -EFAULT;
	}

	pin_state = pinctrl_lookup_state(i2c_priv->pinctrl, name);
	if (IS_ERR_OR_NULL(pin_state)) {
		hwlog_err("%s: could not get %s\n", __func__, name);
		return -ENOENT;
	}

	ret = pinctrl_select_state(i2c_priv->pinctrl, pin_state);
	if (ret) {
		hwlog_err("%s: can not set pins to %s state\n", __func__, name);
		return -ENOENT;
	}

	return 0;
}

static int high_vol_cnv_get_gpio_num_from_dt(struct device_node *node,
	const char *property_name)
{
	int gpio;
	int ret;

	gpio = of_get_named_gpio(node, property_name, 0);
	if (gpio < 0) {
		hwlog_info("%s: %s of_get_named_gpio failed, %d\n",
			__func__, property_name, gpio);
		ret = of_property_read_u32(node, property_name, (u32 *)&gpio);
		if (ret < 0) {
			hwlog_err("%s: %s read gpio prop failed, %d\n",
				__func__, property_name, ret);
				return ret;
		}
	}
	hwlog_info("%s: %s gpio %d\n", __func__, property_name, gpio);
	return gpio;
}

static int high_vol_cnv_save_switch_ctl_node(struct device_node *node,
	struct smartpakit_switch_node **switch_ctl, unsigned int num)
{
	struct smartpakit_switch_node *ctl = NULL;
	struct property *pp = NULL;
	const char *gpio_rst_str = "gpio_reset";
	int i = 0;
	int ret;
	unsigned int switch_node_size;

	switch_node_size = sizeof(*ctl) * num;
	ctl = kzalloc(switch_node_size, GFP_KERNEL);
	if (ctl == NULL)
		return -ENOMEM;

	for_each_property_of_node(node, pp) {
		if (strncmp(pp->name, gpio_rst_str, strlen(gpio_rst_str)) != 0)
			continue;

		ret = snprintf_s(ctl[i].name, (unsigned long)SMARTPAKIT_NAME_MAX,
			(unsigned long)SMARTPAKIT_NAME_MAX - 1,
			"high_vol_cnv_swtich_%d", i);
		if (ret < 0) {
			hwlog_err("%s: set gpio name failed\n", __func__);
			ret = -EFAULT;
			goto switch_ctl_node_err_out;
		}
		ctl[i].gpio = high_vol_cnv_get_gpio_num_from_dt(node, pp->name);
		if (!gpio_is_valid(ctl[i].gpio)) {
			hwlog_info("%s:gpio%d invaild\n", __func__, ctl[i].gpio);
			ret = -EFAULT;
			goto switch_ctl_node_err_out;
		}
		ctl[i].state = -1;
		i++;
	}
	*switch_ctl = ctl;
	return 0;
switch_ctl_node_err_out:
	kfree(ctl);
	return ret;
}

static int high_vol_cnv_parse_switch_ctl_gpio(struct device_node *node,
	struct high_voltage_converter_priv *i2c_priv)
{
	const char *gpio_reset_str = "gpio_reset";
	int ret;
	unsigned int gpio_num;

	gpio_num = (unsigned int)smartpakit_get_prop_num(node, gpio_reset_str);
	if (gpio_num == 0) {
		hwlog_err("%s: switch_ctl gpio num is 0\n", __func__);
		return -EINVAL;
	}
	ret = high_vol_cnv_save_switch_ctl_node(node, &i2c_priv->switch_ctl,
		gpio_num);
	if (ret < 0) {
		hwlog_err("%s: save switch ctl node err\n", __func__);
		goto err_out;
	}

	ret = smartpakit_save_switch_ctl_default_value(node,
		i2c_priv->switch_ctl, gpio_num);
	if (ret < 0) {
		hwlog_err("%s: save default value err\n", __func__);
		goto err_out;
	}
	i2c_priv->switch_num = gpio_num;
	return 0;
err_out:
	smartpakit_kfree_ops(i2c_priv->switch_ctl);
	i2c_priv->switch_num = 0;
	return ret;
}

static int high_vol_cnv_init_switch_ctl_gpio(
	struct high_voltage_converter_priv *i2c_priv)
{
	struct smartpakit_switch_node *ctl = NULL;
	int i;
	int ret;

	ctl = i2c_priv->switch_ctl;
	if ((i2c_priv->switch_num == 0) || (ctl == NULL))
		return 0;

	for (i = 0; i < (int)i2c_priv->switch_num; i++) {
		hwlog_info("%s: %d node gpio %d, state %d\n", __func__, i,
			ctl[i].gpio, ctl[i].state);
		if (!gpio_is_valid(ctl[i].gpio)) {
			hwlog_info("%s:gpio%d invaild\n", __func__, ctl[i].gpio);
			ret = -EFAULT;
			goto err_out;
		}
		if (gpio_request((unsigned int)ctl[i].gpio, ctl[i].name) < 0) {
			hwlog_err("%s: request gpio %d failed\n",
				__func__, ctl[i].gpio);
			ctl[i].gpio = 0;
			ret = -EFAULT;
			goto err_out;
		}

		if (ctl[i].state < 0)
			continue;
		gpio_direction_output((unsigned int)ctl[i].gpio, ctl[i].state);
	}
	i2c_priv->switch_gpio_requested = true;
	return 0;
err_out:
	i2c_priv->switch_num = 0;
	smartpakit_kfree_ops(i2c_priv->switch_ctl);
	return ret;
}

static int high_vol_cnv_parse_dt_switch_ctl_cfg(struct device_node *parent_node,
	struct high_voltage_converter_priv *i2c_priv)
{
	const char *switch_ctl_str = "switch_ctl";
	struct device_node *node = NULL;
	int ret;

	node = of_get_child_by_name(parent_node, switch_ctl_str);
	if (node == NULL) {
		hwlog_debug("%s: node not existed, skip\n", __func__);
		return 0;
	}

	if (of_property_read_bool(node, "switch_ctl_pinctrl")) {
		ret = high_vol_cnv_pinctrl_select_state(i2c_priv,
			"switch_active");
		if (ret) {
			hwlog_err("%s: ops switch_active pinctrl fail %d\n",
				__func__, ret);
			return -EINVAL;
		}
	}

	ret = high_vol_cnv_parse_switch_ctl_gpio(node, i2c_priv);
	if (ret < 0) {
		hwlog_err("%s: get switch ctl config err\n", __func__);
		return ret;
	}
	ret = high_vol_cnv_init_switch_ctl_gpio(i2c_priv);
	if (ret < 0) {
		hwlog_err("%s: get switch ctl init err\n", __func__);
		return ret;
	}

	return 0;
}

static void high_vol_cnv_free_switch_ctl_gpio(unsigned int ctl_num,
	struct smartpakit_switch_node *ctl, bool switch_gpio_requested)
{
	int i;

	if ((ctl_num == 0) || (ctl == NULL) || (!switch_gpio_requested))
		return;

	for (i = 0; i < (int)ctl_num; i++) {
		if (ctl[i].gpio > 0) {
			gpio_free((unsigned int)ctl[i].gpio);
			ctl[i].gpio = 0;
		}
	}

	kfree(ctl);
}

static void high_vol_cnv_free_switch_ctl(
	struct high_voltage_converter_priv *i2c_priv)
{
	if (i2c_priv == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return;
	}

	high_vol_cnv_free_switch_ctl_gpio(i2c_priv->switch_num,
		i2c_priv->switch_ctl, i2c_priv->switch_gpio_requested);
	i2c_priv->switch_num = 0;
	i2c_priv->switch_ctl = NULL;
	i2c_priv->switch_gpio_requested = false;
}

static void high_vol_cnv_i2c_free_irq_handler(
	struct high_vol_cnv_gpio_irq **irq_handler)
{
	if (irq_handler == NULL || *irq_handler == NULL)
		return;

	IN_FUNCTION;
	high_vol_cnv_i2c_free_reg_ctl(&((*irq_handler)->rw_sequence));
	kfree(*irq_handler);
	*irq_handler = NULL;
}

static int high_vol_cnv_i2c_parse_irq_handler_info(
	struct high_voltage_converter_priv *i2c_priv, struct device_node *node,
	struct high_vol_cnv_gpio_irq **irq)
{
	const char *gpio_irq_str        = "gpio_irq";
	const char *irq_flags_str       = "irq_flags";
	const char *rw_sequence_str     = "rw_sequence";
	struct high_vol_cnv_gpio_irq *irq_info = NULL;
	int ret;

	irq_info = kzalloc(sizeof(*irq_info), GFP_KERNEL);
	if (irq_info == NULL)
		return -ENOMEM;

	irq_info->gpio = high_vol_cnv_get_gpio_num_from_dt(node, gpio_irq_str);
	if (!gpio_is_valid(irq_info->gpio)) {
		hwlog_info("%s: gpio%d invaild\n", __func__, irq_info->gpio);
		ret = -EFAULT;
		goto parse_irq_handler_err_out;
	}

	ret = of_property_read_u32(node, irq_flags_str, &irq_info->irqflags);
	if (ret < 0) {
		hwlog_err("%s: irq_handler get irq_flags failed\n", __func__);
		goto parse_irq_handler_err_out;
	}

	ret = strcpy_s(irq_info->gpio_name, SMARTPAKIT_NAME_MAX - 1,
		"high_vol_cnv_irq_gpio");
	ret += strcpy_s(irq_info->irq_name, SMARTPAKIT_NAME_MAX - 1,
		"high_vol_cnv_irq");
	if (ret < 0) {
		hwlog_err("%s: set gpio_name/irq_name failed\n", __func__);
		goto parse_irq_handler_err_out;
	}

	/* rw_sequence configed in dts */
	if (of_property_read_bool(node, rw_sequence_str)) {
		ret = high_vol_cnv_i2c_parse_reg_ctl(&irq_info->rw_sequence,
			node, rw_sequence_str);
		if (ret < 0) {
			hwlog_err("%s: parse rw_sequence failed\n", __func__);
			goto parse_irq_handler_err_out;
		}
	}
	*irq = irq_info;
	return 0;
parse_irq_handler_err_out:
	high_vol_cnv_i2c_free_irq_handler(&irq_info);
	return ret;
}

static irqreturn_t high_vol_cnv_i2c_thread_irq(int irq, void *data)
{
	struct high_voltage_converter_priv *i2c_priv = NULL;

	hwlog_info("%s: enter, irq = %d\n", __func__, irq);

	i2c_priv = data;
	if (i2c_priv == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return IRQ_HANDLED;
	}

	if (i2c_priv->probe_completed == 0) {
		hwlog_err("%s: probe not completed, skip\n", __func__);
		return IRQ_HANDLED;
	}

	if (!work_busy(&i2c_priv->irq_handle_work)) {
		hwlog_info("%s: schedule_work\n", __func__);
		schedule_work(&i2c_priv->irq_handle_work);
	} else {
		hwlog_info("%s: work busy, skip\n", __func__);
	}

	hwlog_info("%s: enter end\n", __func__);
	return IRQ_HANDLED;
}

static int high_vol_cnv_i2c_request_threaded_irq(
	struct high_voltage_converter_priv *i2c_priv)
{
	struct high_vol_cnv_gpio_irq *irq_handler = NULL;
	int gpio;
	int ret;
	struct device *dev = NULL;

	irq_handler = i2c_priv->irq_handler;
	if (irq_handler == NULL || i2c_priv->i2c == NULL) {
		hwlog_info("%s: irq_handler or i2c not configed\n", __func__);
		return 0;
	}
	dev = &i2c_priv->i2c->dev;
	gpio = irq_handler->gpio;
	if (!gpio_is_valid(gpio)) {
		hwlog_err("%s: irq_handler gpio %d invalid\n", __func__, gpio);
		return -EFAULT;
	}

	ret = devm_gpio_request_one(dev, (unsigned int)gpio,
		(unsigned long)GPIOF_DIR_IN, irq_handler->gpio_name);
	if (ret < 0) {
		hwlog_err("%s: gpio set GPIOF_DIR_IN failed\n", __func__);
		high_vol_cnv_i2c_free_irq_handler(&(i2c_priv->irq_handler));
		return -EFAULT;
	}

	irq_handler->irq = gpio_to_irq((unsigned int)gpio);
	hwlog_info("%s: irq_handler get irq %d, irqflags=%u\n", __func__,
		irq_handler->irq, irq_handler->irqflags);
	ret = devm_request_threaded_irq(dev,
		(unsigned int)irq_handler->irq, NULL,
		high_vol_cnv_i2c_thread_irq,
		(unsigned long)(irq_handler->irqflags | IRQF_ONESHOT),
		irq_handler->irq_name, (void *)i2c_priv);
	if (ret < 0) {
		hwlog_err("%s: devm_request_threaded_irq failed\n", __func__);
		devm_gpio_free(i2c_priv->dev, i2c_priv->irq_handler->gpio);
		high_vol_cnv_i2c_free_irq_handler(&(i2c_priv->irq_handler));
		return -EFAULT;
	}
	return 0;
}

static void high_vol_cnv_i2c_reset_irq_handler_data(
	struct high_voltage_converter_priv *i2c_priv)
{
	int gpio;
	struct device *dev = NULL;

	if (i2c_priv == NULL || i2c_priv->i2c == NULL)
		return;

	if (i2c_priv->irq_handler == NULL)
		return;

	IN_FUNCTION;
	gpio = i2c_priv->irq_handler->gpio;
	dev = &i2c_priv->i2c->dev;

	if (!gpio_is_valid(gpio))
		return;

	devm_free_irq(dev,
		(unsigned int)(i2c_priv->irq_handler->irq),
		(void *)i2c_priv);

	devm_gpio_free(dev, gpio);
	i2c_priv->irq_handler->gpio = -EINVAL;
}

static int high_vol_cnv_parse_dt_irq_pinctrl(struct device_node *node,
	struct high_voltage_converter_priv *i2c_priv)
{
	const char *irq_pinctrl = "irq_pinctrl";
	const char *pin_ctrl_state_name = "irq_default";
	int ret;

	if (of_property_read_bool(node, irq_pinctrl)) {
		ret = high_vol_cnv_pinctrl_select_state(i2c_priv,
			pin_ctrl_state_name);
		if (ret) {
			hwlog_err("%s: ops switch_active pinctrl fail %d\n",
				__func__, ret);
			return -EINVAL;
		}
	}

	return 0;
}

static int high_vol_cnv_i2c_parse_dt_irq(struct device_node *parent_node,
	struct high_voltage_converter_priv *i2c_priv)
{
	const char *irq_handler_str = "irq_handler";
	struct device_node *node = NULL;
	int ret;

	node = of_get_child_by_name(parent_node, irq_handler_str);
	if (node == NULL) {
		hwlog_debug("%s: irq_handler not existed, skip\n", __func__);
		return 0;
	}
	ret = high_vol_cnv_i2c_parse_irq_handler_info(i2c_priv, node,
		&i2c_priv->irq_handler);
	if (ret < 0)
		hwlog_err("%s: parse irq handler info err\n", __func__);

	return ret;
}

static void high_vol_cnv_i2c_get_i2c_err_info(struct i2c_err_info *info,
	unsigned int index)
{
	if (index < I2C_STATUS_B64)
		info->err_details |= ((unsigned long int)1 << index);

	info->err_count++;
}

static int high_vol_cnv_check_i2c_regmap_cfg(
	struct high_voltage_converter_priv *i2c_priv)
{
	if ((i2c_priv == NULL) || (i2c_priv->regmap_cfg == NULL) ||
		(i2c_priv->regmap_cfg->regmap == NULL))
		return -EINVAL;
	return 0;
}

static int high_vol_cnv_gpio_node_ctl(
	struct high_voltage_converter_priv *i2c_priv,
	struct high_vol_cnv_reg_ctl *reg)
{
	int gpio;

	if ((i2c_priv == NULL) || (i2c_priv->switch_ctl == NULL)) {
		hwlog_info("%s: switch_ctl is invalid\n", __func__);
		return -EINVAL;
	}
	if (reg == NULL) {
		hwlog_err("%s: param node reg is invalid\n", __func__);
		return -EINVAL;
	}

	if (reg->addr >= i2c_priv->switch_num) {
		hwlog_err("%s: Invalid arg, gpio index %u>= switch num %u\n",
			__func__, reg->addr, i2c_priv->switch_num);
		return -EINVAL;
	}

	gpio = i2c_priv->switch_ctl[reg->addr].gpio;
	gpio_direction_output((unsigned int)gpio, (int)reg->value);
	if (reg->delay == 0)
		goto high_vol_cnv_gpio_node_ctl_exit;

	if (reg->delay > SMARTPAKIT_DELAY_US_TO_MS) {
		msleep(reg->delay / SMARTPAKIT_DELAY_US_TO_MS);
		hwlog_info("%s: gpio %d set %u, delay %u\n", __func__, gpio,
			reg->value, reg->delay);
	} else {
		udelay(reg->delay);
	}

high_vol_cnv_gpio_node_ctl_exit:
	return 0;
}

static int high_vol_cnv_i2c_reg_node_ops(struct smartpakit_regmap_cfg *cfg,
	struct high_vol_cnv_reg_ctl *reg,
	int index, unsigned int reg_num)
{
	int ret = 0;
	int value;

	switch (reg->ctl_type) {
	case HIGH_VOL_CNV_REG_CTL_TYPE_DELAY:
		if (reg->delay > 0)
			msleep(reg->delay);
		break;
	case HIGH_VOL_CNV_REG_CTL_TYPE_R:
		ret = high_vol_cnv_regmap_read(cfg->regmap, reg->addr, &value);
		hwlog_info("%s: read node %d/%u, reg[0x%x]:0x%x, ret:%d\n",
			__func__, index, reg_num, reg->addr, value, ret);
		break;
	case HIGH_VOL_CNV_REG_CTL_TYPE_W:
		high_vol_cnv_delay(reg->delay);
		ret = high_vol_cnv_regmap_complex_write(cfg, reg->addr,
			reg->mask, reg->value);
#if !IS_ENABLED(CONFIG_FINAL_RELEASE)
		hwlog_info("%s: w node %d/%u,reg[0x%x]:0x%x,"
			"mask:0x%x,ret:%d,delay:%d\n",
			__func__, index, reg_num, reg->addr, reg->value,
			reg->mask, ret, reg->delay);
#endif
		break;
	default:
		hwlog_err("%s: invalid argument\n", __func__);
		break;
	}
	return ret;
}

static void high_vol_cnv_i2c_set_record_regs(
	struct high_voltage_converter_priv *i2c_priv,
	struct high_vol_cnv_reg_ctl_sequence *seq)
{
	unsigned int size;

	if (i2c_priv == NULL) {
		hwlog_err("%s: i2c_priv invalid argument\n", __func__);
		return;
	}

	if (i2c_priv->record_wirten_seq != NULL)
		high_vol_cnv_i2c_free_reg_ctl(&i2c_priv->record_wirten_seq);

	size = sizeof(struct high_vol_cnv_reg_ctl_sequence);
	i2c_priv->record_wirten_seq = kzalloc(size, GFP_KERNEL);
	if (i2c_priv->record_wirten_seq == NULL)
		return;

	size = sizeof(struct high_vol_cnv_reg_ctl_sequence) * seq->num;

	i2c_priv->record_wirten_seq->regs = kzalloc(size, GFP_KERNEL);
	if (i2c_priv->record_wirten_seq->regs == NULL) {
		smartpakit_kfree_ops(i2c_priv->record_wirten_seq);
		return;
	}
	i2c_priv->record_wirten_seq->num = seq->num;
	if (memcpy_s(i2c_priv->record_wirten_seq->regs,
		size, seq->regs, size) != EOK)
		hwlog_err("%s: memcpy_s is failed\n", __func__);
}

static int high_vol_cnv_i2c_wirte_ctl_seq_regs(
	struct high_voltage_converter_priv *i2c_priv,
	struct high_vol_cnv_reg_ctl_sequence *seq,
	struct i2c_err_info *info)
{
	int i;
	int ret;
	int errno = 0;
	struct smartpakit_regmap_cfg *cfg = i2c_priv->regmap_cfg;

	for (i = 0; i < (int)(seq->num); i++) {
		/* gpio node */
		if (seq->regs[i].ctl_type == HIGH_VOL_CNV_PARAM_NODE_TYPE_GPIO) {
			ret = high_vol_cnv_gpio_node_ctl(i2c_priv,
				&(seq->regs[i]));
			if (ret == 0)
				continue;
			hwlog_err("%s: node %d, set gpio index%u %u err\n",
				__func__, i, seq->regs[i].addr,
				seq->regs[i].value);
			return ret;
		}

		/* regmap node */
		ret = high_vol_cnv_i2c_reg_node_ops(cfg,
			&(seq->regs[i]), i, seq->num);
		if (ret < 0) {
			hwlog_err("%s: ctl %d, reg 0x%x w/r 0x%x err, ret %d\n",
				__func__, i, seq->regs[i].addr,
				seq->regs[i].value, ret);
			high_vol_cnv_i2c_get_i2c_err_info(info, (unsigned int)i);
			errno = ret;
		}
	}
	info->regs_num = seq->num;
	high_vol_cnv_i2c_dsm_report(i2c_priv, I2C_WRITE, errno, info);
	high_vol_cnv_i2c_set_record_regs(i2c_priv, seq);
	return 0;
}

static int high_vol_cnv_i2c_ctrl_write_regs(
	struct high_voltage_converter_priv *i2c_priv,
	struct high_vol_cnv_reg_ctl_sequence *seq)
{
	struct i2c_err_info info = {
		.regs_num    = 0,
		.err_count   = 0,
		.err_details = 0,
	};

	if (high_vol_cnv_check_i2c_regmap_cfg(i2c_priv) < 0) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if ((seq == NULL) || (seq->num == 0)) {
		hwlog_err("%s: reg node is invalid\n", __func__);
		return -EINVAL;
	}

	return high_vol_cnv_i2c_wirte_ctl_seq_regs(i2c_priv, seq, &info);
}

static int high_vol_cnv_i2c_read_chip_version(
	struct high_voltage_converter_priv *i2c_priv)
{
	struct regmap *regmap = NULL;
	struct high_vol_cnv_reg_ctl_sequence *seq = NULL;
	struct high_vol_cnv_reg_ctl *regs = NULL;
	unsigned int reg_addr;
	unsigned int value;
	int ret;
	int i;
	unsigned int num;

	if (high_vol_cnv_check_i2c_regmap_cfg(i2c_priv) < 0) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if (i2c_priv->version_regs_seq == NULL)
		return 0;

	seq = i2c_priv->version_regs_seq;
	num = seq->num;
	regmap = i2c_priv->regmap_cfg->regmap;
	for (i = 0; i < (int)num; i++) {
		regs = &(seq->regs[i]);
		reg_addr = regs->addr;
		hwlog_info("%s: seq num %u  read version reg:0x%x\n",
			__func__, seq->num, reg_addr);
		ret = high_vol_cnv_regmap_read(regmap, reg_addr, &value);
		high_vol_cnv_i2c_dsm_report(i2c_priv, I2C_READ, ret, NULL);
		if (ret < 0) {
			hwlog_err("%s: read version regs:0x%x failed, %d\n",
				__func__, reg_addr, ret);
			return ret;
		}
		hwlog_info("%s:r reg 0x%x = 0x%x\n", __func__, reg_addr, value);
		if ((value & regs->mask) != regs->chip_version) {
			hwlog_err("%s:r reg 0x%x = 0x%x,"
				"mask:0x%x, chip_version:0x%x\n",
				__func__, reg_addr, value,
				regs->mask, regs->chip_version);
			return -EINVAL;
		}
	}
	return ret;
}

static int high_vol_cnv_i2c_parse_dt_reg_ctl(struct device_node *node,
	struct high_vol_cnv_reg_ctl_sequence **reg_ctl,
	const char *seq_str)
{
	int ret;

	if (!of_property_read_bool(node, seq_str)) {
		hwlog_debug("%s: %s not existed, skip\n", seq_str, __func__);
		return 0;
	}
	ret = high_vol_cnv_i2c_parse_reg_ctl(reg_ctl,
		node, seq_str);
	if (ret < 0) {
		hwlog_err("%s: parse %s failed\n", __func__, seq_str);
		goto parse_dt_reg_ctl_err_out;
	}

parse_dt_reg_ctl_err_out:
	return ret;
}

static int high_vol_cnv_parse_dts_sequence(struct device_node *node,
	struct high_voltage_converter_priv *i2c_priv)
{
	int ret;

	ret = high_vol_cnv_parse_dt_switch_ctl_cfg(node, i2c_priv);
	if (ret < 0)
		return ret;

	ret = high_vol_cnv_i2c_parse_dt_reg_ctl(node,
		&i2c_priv->version_regs_seq, "version_regs");
	if (ret < 0)
		return ret;

	ret = high_vol_cnv_i2c_parse_dt_reg_ctl(node,
		&i2c_priv->dump_regs_sequence, "dump_regs");
	if (ret < 0)
		return ret;

	ret = high_vol_cnv_i2c_parse_dt_reg_ctl(node,
		&i2c_priv->init_regs_seq, "init_regs");
	if (ret < 0)
		return ret;

	ret = high_vol_cnv_i2c_parse_dt_reg_ctl(node,
		&i2c_priv->irq_filter_seq, "irq_filter_regs");

	return ret;
}

static int high_vol_cnv_parse_irq_pinctrl(struct device_node *node,
	struct high_voltage_converter_priv *i2c_priv)
{
	int ret;

	ret = high_vol_cnv_parse_dt_irq_pinctrl(node, i2c_priv);
	if (ret < 0)
		return ret;

	ret = high_vol_cnv_i2c_parse_dt_irq(node, i2c_priv);
	if (ret < 0)
		return ret;

	return ret;
}

static int high_vol_cnv_init_regs_seq(
	struct high_voltage_converter_priv *i2c_priv)
{
	if (high_vol_cnv_check_i2c_regmap_cfg(i2c_priv) < 0) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if (i2c_priv->init_regs_seq != NULL)
		return high_vol_cnv_i2c_ctrl_write_regs(i2c_priv,
			i2c_priv->init_regs_seq);

	return 0;
}

static int high_vol_cnv_i2c_resource_init(struct i2c_client *i2c,
	struct high_voltage_converter_priv *i2c_priv)
{
	int ret;
	struct device_node *node = NULL;

	ret = get_high_vol_cnv_node(i2c, i2c_priv, &node);
	if (ret < 0)
		return ret;

	ret = high_vol_cnv_parse_dts_sequence(node, i2c_priv);
	if (ret < 0)
		return ret;

	/* int regmap */
	ret = high_vol_cnv_i2c_regmap_init(i2c, node, i2c_priv);
	if (ret < 0)
		return ret;

	ret = high_vol_cnv_i2c_read_chip_version(i2c_priv);
	if (ret < 0)
		return ret;

	ret = high_vol_cnv_parse_irq_pinctrl(node, i2c_priv);
	if (ret < 0)
		return ret;

	ret = high_vol_cnv_init_regs_seq(i2c_priv);
	if (ret < 0)
		return ret;

	ret = high_vol_cnv_i2c_request_threaded_irq(i2c_priv);
	if (ret < 0)
		return ret;

	return 0;
}

static int high_vol_cnv_i2c_check_status(struct i2c_client *i2c,
	const struct i2c_device_id *id)
{
	if (i2c == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	hwlog_info("%s: device %s, addr=0x%x, flags=0x%x\n", __func__,
		id->name, i2c->addr, i2c->flags);
	if (!i2c_check_functionality(i2c->adapter, I2C_FUNC_I2C)) {
		hwlog_err("%s: i2c check functionality error\n", __func__);
		return -ENODEV;
	}

	return 0;
}

static bool  high_vol_cnv_is_rw_sequence_exited(
	struct high_voltage_converter_priv *i2c_priv)
{
	if ((i2c_priv == NULL) ||
		(i2c_priv->irq_handler == NULL) ||
		(i2c_priv->irq_handler->rw_sequence == NULL))
		return false;

	return true;
}

static int high_vol_cnv_dump_rw_sequence_ops(
	struct high_voltage_converter_priv *i2c_priv,
	struct high_vol_cnv_reg_ctl *reg,
	struct list_head *dump_reg_list)
{
	struct smartpakit_reg_info reg_info;
	struct regmap *regmap = NULL;
	unsigned int reg_addr;
	unsigned int value = 0;
	int ret = 0;
	int ret_once;
	int j;

	regmap = i2c_priv->regmap_cfg->regmap;
	reg_addr = reg->addr;
	if (memset_s(&reg_info, sizeof(reg_info), 0, sizeof(reg_info)) != EOK) {
		hwlog_err("%s: memcpy_s is failed\n", __func__);
		return -EINVAL;
	}

	for (j = 0; j < (int)reg->value; j++) {
		ret_once = high_vol_cnv_regmap_read(regmap, reg_addr, &value);
		high_vol_cnv_i2c_dsm_report(i2c_priv, I2C_READ, ret_once, NULL);

		if (ret_once < 0) {
			ret += ret_once;
		} else {
			reg_info.reg_addr = reg_addr;
			reg_info.info = value;
			smartpakit_reg_info_add_list(&reg_info, dump_reg_list);
		}
		hwlog_info("%s:r reg 0x%x = 0x%x\n", __func__, reg_addr, value);

		reg_addr++;
	}
	return ret;
}

static int high_vol_cnv_dump_rw_sequence(
	struct high_voltage_converter_priv *i2c_priv,
	struct high_vol_cnv_reg_ctl_sequence *sequence,
	struct list_head *dump_reg_list)
{
	unsigned int reg_addr;
	unsigned int ctl_value;
	unsigned int ctl_type;
	int ret = 0;
	int ret_once;
	int i;
	struct regmap *regmap = NULL;

	if (high_vol_cnv_check_i2c_regmap_cfg(i2c_priv) < 0) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if ((sequence == NULL) || (sequence->num == 0) ||
		(sequence->regs == NULL))
		return 0;

	regmap = i2c_priv->regmap_cfg->regmap;
	for (i = 0; i < (int)sequence->num; i++) {
		reg_addr = sequence->regs[i].addr;
		ctl_value = sequence->regs[i].value;
		ctl_type = sequence->regs[i].ctl_type;

		if (ctl_type == HIGH_VOL_CNV_REG_CTL_TYPE_W) {
			ret_once = high_vol_cnv_regmap_write(regmap, reg_addr,
				ctl_value);
			high_vol_cnv_i2c_dsm_report(i2c_priv, I2C_WRITE,
				ret_once, NULL);
			ret += ret_once;
			hwlog_info("%s:w reg 0x%x 0x%x\n", __func__,
				reg_addr, ctl_value);
		} else if (ctl_type == HIGH_VOL_CNV_REG_CTL_TYPE_R) {
			ret += high_vol_cnv_dump_rw_sequence_ops(i2c_priv,
				&sequence->regs[i], dump_reg_list);
		} else {
			ret += high_vol_cnv_i2c_reg_node_ops(i2c_priv->regmap_cfg,
				&(sequence->regs[i]), i, sequence->num);
		}
	}

	return ret;
}

#if IS_ENABLED (CONFIG_HUAWEI_DSM_AUDIO)
static void high_vol_cnv_i2c_handler_irq_dsm_report(
	struct high_voltage_converter_priv *i2c_priv,
	struct list_head *dump_reg_list)
{
	char *report = NULL;
	struct smartpakit_reg_info *reg_info = NULL;

	if (i2c_priv == NULL || i2c_priv->i2c == NULL)
		return;

	report = kzalloc(sizeof(char) * DSM_BUF_SIZE, GFP_KERNEL);
	if (report == NULL)
		return;

	high_vol_cnv_append_dsm_report(report, "high_voltage_converter 0x%x:",
		i2c_priv->i2c->addr);
	list_for_each_entry(reg_info, dump_reg_list, list)
		high_vol_cnv_append_dsm_report(report, "reg 0x%x=0x%x,",
			reg_info->reg_addr, reg_info->info);
	/* dsm report */
	hwlog_info("%s: dsm report, %s\n", __func__, report);
	audio_dsm_report_info(AUDIO_CODEC, DSM_HIFI_AK4376_I2C_ERR,
		"high_voltage_converter,%s", report);
	kfree(report);
}
#endif

static bool high_vol_cnv_is_irq_filter_match(struct list_head *dump_reg_list,
	struct high_vol_cnv_reg_ctl *reg)
{
	unsigned int val;
	struct smartpakit_reg_info *reg_info = NULL;

	/* if match any irq filter in dtsi, it need handle irqs */
	list_for_each_entry(reg_info, dump_reg_list, list) {
		if (reg_info->reg_addr == reg->addr) {
			val = reg_info->info & reg->mask;
			if (val) {
				hwlog_info("%s:reg:0x%x,value:0x%x,mask:0x%x\n",
					__func__, reg_info->reg_addr,
					reg_info->info, reg_info->mask);
				hwlog_info("%s:filter match\n", __func__);
				return true;
			}
		}
	}

	return false;
}

static bool is_high_vol_cnv_irq_filter_hited(
	struct high_voltage_converter_priv *i2c_priv,
	struct list_head *dump_reg_list)
{
	int i;
	bool ret = false;
	struct high_vol_cnv_reg_ctl_sequence *seq = NULL;

	if (i2c_priv == NULL || dump_reg_list == NULL)
		return false;

	/* not config irq filter in dtsi, it need handle all the irqs */
	if (i2c_priv->irq_filter_seq == NULL)
		return true;

	seq = i2c_priv->irq_filter_seq;
	for (i = 0; i < (int)(seq->num); i++) {
		ret = high_vol_cnv_is_irq_filter_match(dump_reg_list,
			&(seq->regs[i]));
		if (ret)
			return ret;
	}

	return false;
}

static void high_vol_cnv_i2c_handler_irq(struct work_struct *work)
{
	struct high_voltage_converter_priv *i2c_priv = NULL;
	struct list_head dump_regs_list;

	i2c_priv = container_of(work, struct high_voltage_converter_priv,
		irq_handle_work);

	hwlog_info("%s: enter\n", __func__);
	mutex_lock(&i2c_priv->irq_handler_lock);
	INIT_LIST_HEAD(&dump_regs_list);
	if (high_vol_cnv_is_rw_sequence_exited(i2c_priv)) {
		hwlog_info("%s: dump rw sequence\n", __func__);
		if (high_vol_cnv_dump_rw_sequence(i2c_priv,
			i2c_priv->irq_handler->rw_sequence, &dump_regs_list) < 0)
			goto high_vol_cnv_handler_irq_err_out;
	}
#if IS_ENABLED (CONFIG_HUAWEI_DSM_AUDIO)
	if (is_high_vol_cnv_irq_filter_hited(i2c_priv, &dump_regs_list))
		high_vol_cnv_i2c_handler_irq_dsm_report(i2c_priv,
			&dump_regs_list);
#endif
	hwlog_info("%s: dump regs sequence\n", __func__);
	high_vol_cnv_dump_rw_sequence(i2c_priv,
		i2c_priv->dump_regs_sequence, &dump_regs_list);

	smartpakit_reg_info_del_list_all(&dump_regs_list);
high_vol_cnv_handler_irq_err_out:
	mutex_unlock(&i2c_priv->irq_handler_lock);
	hwlog_info("%s: enter end\n", __func__);
}

static void high_vol_cnv_adc_i2c_free(
	struct high_voltage_converter_priv *i2c_priv)
{
	if (i2c_priv == NULL) {
		hwlog_err("%s: i2c_priv invalid argument\n", __func__);
		return;
	}

	if (i2c_priv->regmap_cfg != NULL) {
		high_vol_cnv_i2c_regmap_deinit(i2c_priv);
		i2c_priv->regmap_cfg = NULL;
	}

	high_vol_cnv_i2c_free_reg_ctl(&i2c_priv->init_regs_seq);
	high_vol_cnv_i2c_free_reg_ctl(&i2c_priv->version_regs_seq);
	high_vol_cnv_i2c_free_reg_ctl(&i2c_priv->dump_regs_sequence);
	high_vol_cnv_i2c_free_reg_ctl(&i2c_priv->version_val_check_seq);
	high_vol_cnv_i2c_free_reg_ctl(&i2c_priv->irq_filter_seq);
	high_vol_cnv_i2c_free_reg_ctl(&i2c_priv->record_wirten_seq);
	high_vol_cnv_free_switch_ctl(i2c_priv);
	high_vol_cnv_i2c_reset_irq_handler_data(i2c_priv);
	high_vol_cnv_i2c_free_irq_handler(&i2c_priv->irq_handler);
}

static void high_vol_cnv_i2c_reset_priv_data(
	struct high_voltage_converter_priv *i2c_priv)
{
	if (i2c_priv == NULL)
		return;

	high_vol_cnv_adc_i2c_free(i2c_priv);
	i2c_priv->probe_completed = 0;
	i2c_priv->switch_gpio_requested = false;
}

static int high_vol_cnv_adc_enumrate(struct i2c_client *i2c,
	struct high_voltage_converter_priv *i2c_priv)
{
	int ret;
	unsigned int i;

	ret = of_property_read_u32(i2c->dev.of_node, "num_by_same_i2c_addr",
		&i2c_priv->num_by_same_i2c_addr);
	if (ret < 0)
		i2c_priv->num_by_same_i2c_addr = 1;

	for (i = 0; i < i2c_priv->num_by_same_i2c_addr; i++) {
		i2c_priv->iter = i;
		high_vol_cnv_i2c_reset_priv_data(i2c_priv);
		ret = high_vol_cnv_i2c_resource_init(i2c, i2c_priv);
		if (ret < 0)
			continue;

		return 0;
	}

	return ret;
}

static int smartpa_high_voltage_converter_probe(struct i2c_client *i2c,
	const struct i2c_device_id *id)
{
	struct high_voltage_converter_priv *i2c_priv = NULL;
	int ret;

	IN_FUNCTION;
	if (!high_vol_cnv_probe_is_ilde()) {
		hwlog_err("%s: cnv probe is not idle\n", __func__);
		return -EPROBE_DEFER;
	}

	ret = high_vol_cnv_i2c_check_status(i2c, id);
	if (ret < 0)
		return ret;

	i2c_priv = kzalloc(sizeof(*i2c_priv), GFP_KERNEL);
	if (i2c_priv == NULL)
		return -ENOMEM;

	i2c_priv->i2c = i2c;
	i2c_set_clientdata(i2c, i2c_priv);
	dev_set_drvdata(&i2c->dev, i2c_priv);
	mutex_init(&i2c_priv->irq_handler_lock);
	mutex_init(&i2c_priv->gpio_request_lock);
	INIT_WORK(&i2c_priv->irq_handle_work, high_vol_cnv_i2c_handler_irq);
	mutex_lock(&i2c_priv->gpio_request_lock);
	ret = high_vol_cnv_adc_enumrate(i2c, i2c_priv);
	if (ret < 0)
		goto converter_probe_err_out;

	i2c_priv->probe_completed = 1;
	mutex_unlock(&i2c_priv->gpio_request_lock);
	high_vol_cnv_set_probe_ilde();
	hwlog_info("%s: end success\n", __func__);
	return 0;
converter_probe_err_out:
	i2c_set_clientdata(i2c, NULL);
	dev_set_drvdata(&i2c->dev, NULL);
	high_vol_cnv_adc_i2c_free(i2c_priv);
	kfree(i2c_priv);
	mutex_unlock(&i2c_priv->gpio_request_lock);
	high_vol_cnv_set_probe_ilde();;
	hwlog_err("%s: end failed\n", __func__);
	return ret;
}

static int smartpa_high_voltage_converter_remove(struct i2c_client *i2c)
{
	struct high_voltage_converter_priv *i2c_priv = NULL;

	hwlog_info("%s: remove\n", __func__);
	if (i2c == NULL) {
		pr_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	i2c_priv = i2c_get_clientdata(i2c);
	if (i2c_priv == NULL) {
		hwlog_err("%s: i2c_priv invalid\n", __func__);
		return -EINVAL;
	}

	i2c_set_clientdata(i2c, NULL);
	dev_set_drvdata(&i2c->dev, NULL);
	high_vol_cnv_adc_i2c_free(i2c_priv);
	kfree(i2c_priv);
	return 0;
}

static const struct i2c_device_id smartpa_high_voltage_converter_i2c_id[] = {
	{"high_vol_cnv", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, smartpa_high_voltage_converter_i2c_id);

static const struct of_device_id smartpa_high_voltage_converter_of_match[] = {
	{ .compatible = "huawei,high_vol_cnv", },
	{},
};
MODULE_DEVICE_TABLE(of, smartpa_high_voltage_converter_of_match);

static struct i2c_driver smartpa_high_voltage_converter_driver = {
	.driver = {
		.name = "high_vol_cnv",
		.owner = THIS_MODULE,
		.of_match_table =
			of_match_ptr(smartpa_high_voltage_converter_of_match),
	},
	.probe = smartpa_high_voltage_converter_probe,
	.remove = smartpa_high_voltage_converter_remove,
	.id_table = smartpa_high_voltage_converter_i2c_id,
};

static int __init smartpa_high_voltage_converter_init(void)
{
	IN_FUNCTION;
	return i2c_add_driver(&smartpa_high_voltage_converter_driver);
}

static void __exit smartpa_high_voltage_converter_exit(void)
{
	IN_FUNCTION;
	i2c_del_driver(&smartpa_high_voltage_converter_driver);
}

fs_initcall(smartpa_high_voltage_converter_init);
module_exit(smartpa_high_voltage_converter_exit);

MODULE_DESCRIPTION("high voltage converter driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");
