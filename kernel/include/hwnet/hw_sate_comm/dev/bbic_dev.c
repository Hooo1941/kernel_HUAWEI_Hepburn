/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: This module is used to start the driver peripheral
 * and identify the peripheral chip type.
 * Author: lizhenzhou1@huawei.com
 * Create: 2022-11-26
 */

#include "bbic_dev.h"

#include <securec.h>

#include <log/hw_log.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/kthread.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/poll.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_drivers/lpcpu_idle_sleep.h>
#include <linux/reboot.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/version.h>

#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include <chipset_common/hwpower/hardware_ic/buck_boost.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG HW_SATE_DEV
HWLOG_REGIST();

#define EOK 0
#define MAX_NOR_MSG_LEN 100
#define MODEL_NUM 1
#define DEVICE_REG_TRY_MAX_COUNT 100
#define DEVICE_REG_SLEEP_INTERVAL 1000
#define SUCCESS 0
#define FAILURE 1

#define GPIO_LOW 0
#define GPIO_HIGH 1
#define GPIO_DIRECTION_INPUT 0
#define GPIO_DIRECTION_OUTPUT 1
#define SIM_SLOT_0 0
#define SIM_SLOT_1 1

#define SIM_AT_MODEM 0
#define SIM_AT_BBIC 1
#define SIM0_AT_BBIC 0
#define SIM1_AT_BBIC 1

#define BBIC_SLEEP_GPIO_LOCAL_STATUS_NORMAL 0
#define BBIC_SLEEP_GPIO_LOCAL_STATUS_INPUT 1
#define BBIC_RESET_TIME_MS 32
#define SIM_LDO_SWITCH_TIME_MS 5
#define PA_BST_VOL_4V8 4800
#define PA_BST_VOL_4V5 4500

#define BUCK_BOOST_ENABLE  0
#define BUCK_BOOST_DISABLE 1

#define BUCKBOOST_MIN_MICROVOLT 3450000
#define BUCKBOOST_MAX_MICROVOLT 3950000

static struct platform_device *g_hw_ps_device = NULL;
static bool g_bbic_pwr_status = false;
static int g_bbic_sleep_gpio_status = BBIC_SLEEP_GPIO_LOCAL_STATUS_NORMAL;
static struct regulator *ls_ldo_regulator = NULL;
static struct regulator *usim_ldo_regulator = NULL;
static struct regulator *core_ldo_regulator = NULL;
static struct regulator *sim0_card_ldo_regulator = NULL;
static struct regulator *sim1_card_ldo_regulator = NULL;
static struct regulator *sim1_card_back_ldo_regulator = NULL;
static struct regulator *bbic_xbuck3_regulator = NULL;
static struct regulator *bbic_vdd_buckboost_regulator = NULL;

static struct clk *hlct_clk = NULL;
static int g_sim_connect_state = SIM_AT_MODEM;
static int g_sim_at_bbic_state = SIM0_AT_BBIC;
static int g_bbic_chip_type = -1;

static struct ps_core_s *g_core_s = NULL;
static struct ps_core_s *get_core_data(void)
{
	return g_core_s;
}

void set_bbic_pwr_status(bool sta)
{
	g_bbic_pwr_status = sta;
}

bool get_bbic_pwr_status(void)
{
	return g_bbic_pwr_status;
}

void set_bbic_chip_type(int chip_type)
{
	g_bbic_chip_type = chip_type;
}

int get_bbic_chip_type(void)
{
	return g_bbic_chip_type;
}

static bool is_valid_bbic_chip_type(int type)
{
	if ((type != BBIC_CHIP_TYPE_HLCT) && (type != BBIC_CHIP_TYPE_WS))
		return false;

	return true;
}

bool ldo_is_valid(int ldo_num)
{
	if (ldo_num < 0)
		return false;

	return true;
}

static int parse_sleep_config(struct device_node *dev_node, struct ps_core_s *cd)
{
	int32_t value;

	if ((dev_node == NULL) || (cd == NULL)) {
		hwlog_info("%s, device node or core is NULL\n", __func__);
		return -1;
	}

	value = of_get_named_gpio(dev_node, "ap_wakeup_bb_gpio", 0);
	hwlog_info("%s, ap_wakeup_bb_gpio:%d\n", __func__, value);
	cd->ap_wakeup_bb = value;

	value = of_get_named_gpio(dev_node, "bb_wakeup_ap_gpio", 0);
	hwlog_info("%s, bb_wakeup_ap_gpio:%d\n", __func__, value);
	cd->bb_wakeup_ap = value;

	return 0;
}

static int parse_boot_mode_config(struct device_node *dev_node, struct ps_core_s *cd)
{
	int32_t value;

	if ((dev_node == NULL) || (cd == NULL)) {
		hwlog_info("%s, device node or core is NULL\n", __func__);
		return -1;
	}

	value = of_get_named_gpio(dev_node, "bb_reset_gpio", 0);
	hwlog_info("%s, bb_reset_gpio:%d\n", __func__, value);
	cd->bb_reset = value;

	value = of_get_named_gpio(dev_node, "bb_boot_mode0_gpio", 0);
	hwlog_info("%s, bb_boot_mode0_gpio:%d\n", __func__, value);
	cd->bb_boot_mode0 = value;

	value = of_get_named_gpio(dev_node, "bb_boot_mode3_gpio", 0);
	hwlog_info("%s, bb_boot_mode3_gpio:%d\n", __func__, value);
	cd->bb_boot_mode3 = value;

	return 0;
}

static int parse_pwr_config(struct device_node *dev_node, struct ps_core_s *cd)
{
	int32_t value;
	int32_t ret;

	if ((dev_node == NULL) || (cd == NULL)) {
		hwlog_info("%s, device node or core is NULL\n", __func__);
		return -1;
	}

	value = of_get_named_gpio(dev_node, "pwr_en_gpio", 0);
	hwlog_info("%s, pwr_en_gpio:%d\n", __func__, value);
	cd->pwr_en = value;

	value = of_get_named_gpio(dev_node, "usb_ls_en_gpio", 0);
	hwlog_info("%s, usb_ls_en_gpio:%d\n", __func__, value);
	cd->usb_ls_pwr_en = value;

	value = of_get_named_gpio(dev_node, "bds_mos_en_gpio", 0);
	hwlog_info("%s, bds_mos_en_gpio:%d\n", __func__, value);
	cd->bds_mos_en = value;

	if (get_bbic_chip_type() == BBIC_CHIP_TYPE_WS) {
		value = of_get_named_gpio(dev_node, "ws_vbst_5v_en_gpio", 0);
		hwlog_info("%s, ws_vbst_5v_en_gpio:%d\n", __func__, value);
		cd->ws_vbst_5v_en = value;

		value = of_get_named_gpio(dev_node, "ws_core_en_gpio", 0);
		hwlog_info("%s, ws_core_en_gpio:%d\n", __func__, value);
		cd->ws_core_en = value;

		value = of_get_named_gpio(dev_node, "ws_bds_bst_en_gpio", 0);
		hwlog_info("%s, ws_bds_bst_en_gpio:%d\n", __func__, value);
		cd->bds_bst_en_gpio = value;
	}

	value = of_get_named_gpio(dev_node, "ant_vc1_gpio", 0);
	hwlog_info("%s, ant_vc1_gpio:%d\n", __func__, value);
	cd->ant_vc1 = value;

	value = of_get_named_gpio(dev_node, "ant_vc2_gpio", 0);
	hwlog_info("%s, ant_vc2_gpio:%d\n", __func__, value);
	cd->ant_vc2 = value;

	value = of_get_named_gpio(dev_node, "ant_sw1_vc1_gpio", 0);
	hwlog_info("%s, ant_sw1_vc1_gpio:%d\n", __func__, value);
	cd->ant_sw1_vc1 = value;

	/* set default to adapt old version */
	cd->buck_boost_volt = PA_BST_VOL_4V5;
	ret = of_property_read_u32(dev_node, "bbic,buck_boost_vol",
		(unsigned int *)(&cd->buck_boost_volt));
	if (ret != 0) {
		cd->buck_boost_volt = PA_BST_VOL_4V5;
		hwlog_info("%s: failed to get buck_boost_vol,use default vol:%d, use default vol.\n", __func__, cd->buck_boost_volt);
	} else {
		hwlog_info("%s: succ to get buck_boost_vol = %d!\n", __func__, cd->buck_boost_volt);
	}

	return 0;
}

static int parse_usim_config(struct device_node *dev_node, struct ps_core_s *cd)
{
	int32_t value;

	if ((dev_node == NULL) || (cd == NULL)) {
		hwlog_info("%s, device node or core is NULL\n", __func__);
		return -1;
	}

	value = of_get_named_gpio(dev_node, "usim0_switch_gpio", 0);
	hwlog_info("%s, usim0_switch_gpio:%d\n", __func__, value);
	cd->usim0_switch = value;

	value = of_get_named_gpio(dev_node, "usim1_switch_gpio", 0);
	hwlog_info("%s, usim1_switch_gpio:%d\n", __func__, value);
	cd->usim1_switch = value;

	return 0;
}

static int parse_ls_core_supply_config(struct device_node *dev_node,
	struct ps_core_s *cd)
{
	int ret;
	ret = of_property_read_u32(dev_node, "bbic,ls_ldo_num",
		(unsigned int *)(&cd->ls_ldo_num));
	if (ret != 0) {
		cd->ls_ldo_num = -1;
		hwlog_info("%s: failed to get ls_ldo_num,use default value:%d, use default value.\n", __func__, cd->ls_ldo_num);
	} else {
		hwlog_info("%s: succ to get ls_ldo_num = %d!\n", __func__, cd->ls_ldo_num);
	}
	ret = of_property_read_u32(dev_node, "bbic,ls_ldo_vol",
		(unsigned int *)(&cd->ls_ldo_vol));
	if (ret != 0) {
		cd->ls_ldo_vol = -1;
		hwlog_info("%s: failed to get ls_ldo_vol,use default value:%d, use default value.\n", __func__, cd->ls_ldo_vol);
	} else {
		hwlog_info("%s: succ to get ls_ldo_vol = %d!\n", __func__, cd->ls_ldo_vol);
	}
	ret = of_property_read_u32(dev_node, "bbic,core_ldo_num",
		(unsigned int *)(&cd->core_ldo_num));
	if (ret != 0) {
		cd->ls_ldo_vol = -1;
		hwlog_info("%s: failed to get core_ldo_num,use default value:%d, use default value.\n", __func__, cd->core_ldo_num);
	} else {
		hwlog_info("%s: succ to get core_ldo_num = %d!\n", __func__, cd->core_ldo_num);
	}
	ret = of_property_read_u32(dev_node, "bbic,core_ldo_vol",
		(unsigned int *)(&cd->core_ldo_vol));
	if (ret != 0) {
		cd->core_ldo_vol = -1;
		hwlog_info("%s: failed to get core_ldo_vol,use default value:%d, use default value.\n", __func__, cd->core_ldo_vol);
	} else {
		hwlog_info("%s: succ to get core_ldo_vol = %d!\n", __func__, cd->core_ldo_vol);
	}
	ret = of_property_read_u32(dev_node, "bbic,vbuck3_vol", (unsigned int *)(&cd->vbuck3_vol));
	if (ret != 0) {
		cd->vbuck3_vol = -1;
		hwlog_info("%s: failed to get vbuck3_vol,use default value:%d, use default value.\n", __func__, cd->vbuck3_vol);
	} else {
		hwlog_info("%s: succ to get vbuck3_vol = %d!\n", __func__, cd->vbuck3_vol);
	}
	ret = of_property_read_u32(dev_node, "bbic,vbuck3_off", (unsigned int *)(&cd->vbuck3_off));
	if (ret != 0) {
		cd->vbuck3_off = -1;
		hwlog_info("%s: failed to get vbuck3_off,use default value:%d, use default value.\n", __func__, cd->vbuck3_off);
	} else {
		hwlog_info("%s: succ to get vbuck3_off = %d!\n", __func__, cd->vbuck3_off);
	}
	return 0;
}

static int parse_usim_ldo_supply_config(struct device_node *dev_node,
	struct ps_core_s *cd)
{
	int ret;

	if ((dev_node == NULL) || (cd == NULL)) {
		hwlog_info("%s: input null\n", __func__);
		return -1;
	}

	ret = of_property_read_u32(dev_node, "bbic,usim_ldo_num", (unsigned int *)(&cd->usim0_ldo_num));
	if (ret != 0) {
		cd->usim0_ldo_num = -1;
		hwlog_info("%s: fail to get usim0_ldo_num,default value:%d\n", __func__, cd->usim0_ldo_num);
	} else {
		hwlog_info("%s: succ to get usim0_ldo_num = %d!\n", __func__, cd->usim0_ldo_num);
	}
	ret = of_property_read_u32(dev_node, "bbic,usim_ldo_vol_1v8",
		(unsigned int *)(&cd->usim0_ldo_vol_1v8));
	if (ret != 0) {
		cd->usim0_ldo_vol_1v8 = -1;
		hwlog_info("%s: failed to get usim0_ldo_vol_1v8,use default value:%d, use default value.\n", __func__, cd->usim0_ldo_vol_1v8);
	} else {
		hwlog_info("%s: succ to get usim0_ldo_vol_1v8 = %d!\n", __func__, cd->usim0_ldo_vol_1v8);
	}
	ret = of_property_read_u32(dev_node, "bbic,usim_ldo_vol_3v", (unsigned int *)(&cd->usim0_ldo_vol_3v));
	if (ret != 0) {
		cd->usim0_ldo_vol_3v = -1;
		hwlog_info("%s: failed to get usim0_ldo_vol_3v,default value:%d\n", __func__, cd->usim0_ldo_vol_3v);
	} else {
		hwlog_info("%s: succ to get usim0_ldo_vol_3v = %d!\n", __func__, cd->usim0_ldo_vol_3v);
	}
	return 0;
}

static int parse_ldo_supply_config(struct device_node *dev_node,
	struct ps_core_s *cd)
{
	int ret;

	if ((dev_node == NULL) || (cd == NULL)) {
		hwlog_info("%s: input null\n", __func__);
		return -1;
	}

	ret = of_property_read_u32(dev_node, "bbic,sim0_ldo_vol", (unsigned int *)(&cd->usim0_card_ldo_vol));
	if (ret != 0) {
		cd->usim0_card_ldo_vol = -1;
		hwlog_info("%s: failed to get usim0_card_ldo_vol,use default value:%d, use default value.\n", __func__, cd->usim0_card_ldo_vol);
	} else {
		hwlog_info("%s: succ to get usim0_card_ldo_vol = %d!\n", __func__, cd->usim0_card_ldo_vol);
	}

	ret = of_property_read_u32(dev_node, "bbic,sim1_ldo_vol",
		(unsigned int *)(&cd->usim1_card_ldo_vol));
	if (ret != 0) {
		cd->usim1_card_ldo_vol = -1;
		hwlog_info("%s: failed to get usim1_card_ldo_vol,use default value:%d, use default value.\n", __func__, cd->usim1_card_ldo_vol);
	} else {
		hwlog_info("%s: succ to get usim1_card_ldo_vol = %d!\n", __func__, cd->usim1_card_ldo_vol);
	}
	ret = of_property_read_u32(dev_node, "bbic,vdd_buckboost_vol", (unsigned int *)(&cd->vdd_buckboost_vol));
	if (ret != 0) {
		cd->vdd_buckboost_vol = -1;
		hwlog_info("%s: failed to get vdd_buckboost_vol,default value:%d\n", __func__, cd->vdd_buckboost_vol);
	} else {
		hwlog_info("%s: succ to get vdd_buckboost_vol = %d!\n", __func__, cd->vdd_buckboost_vol);
	}
	ret = parse_usim_ldo_supply_config(dev_node, cd);
	ret = parse_ls_core_supply_config(dev_node, cd);
	return ret;
}

static int parse_dts_config(struct device_node *dev_node,
	struct ps_core_s *cd)
{
	int ret;

	hwlog_info("%s: enter \n", __func__);

	if ((dev_node == NULL) || (cd == NULL)) {
		hwlog_info("%s: input null\n", __func__);
		return -1;
	}
	ret = parse_ldo_supply_config(dev_node, cd);
	if (ret != 0)
		return ret;
	ret = parse_sleep_config(dev_node, cd);
	if (ret != 0)
		return ret;
	ret = parse_boot_mode_config(dev_node, cd);
	if (ret != 0)
		return ret;
	ret = parse_pwr_config(dev_node, cd);
	if (ret != 0)
		return ret;
	ret = parse_usim_config(dev_node, cd);
	if (ret != 0)
		return ret;

	cd->dev_node = dev_node;

	return 0;
}

static void print_pwr_gpio_value(void)
{
	struct ps_core_s *cd = get_core_data();
	int32_t rc;

	if (cd == NULL) {
		hwlog_info("%s: core data is null", __func__);
		return;
	}

	if (core_ldo_regulator != NULL) {
		rc = regulator_get_voltage(core_ldo_regulator);
		hwlog_info("%s: core_ldo_regulator, value:%d", __func__, rc);
	}
	if (ls_ldo_regulator != NULL) {
		rc = regulator_get_voltage(ls_ldo_regulator);
		hwlog_info("%s: ls_ldo_regulator, value:%d", __func__, rc);
	}

	rc = gpio_get_value(cd->pwr_en);
	hwlog_info("%s: gpio:%d, value:%d", __func__, cd->pwr_en, rc);

	rc = gpio_get_value(cd->bb_reset);
	hwlog_info("%s: gpio:%d, value:%d", __func__, cd->bb_reset, rc);

	rc = gpio_get_value(cd->usb_ls_pwr_en);
	hwlog_info("%s: gpio:%d, value:%d", __func__, cd->usb_ls_pwr_en, rc);

	rc = gpio_get_value(cd->bb_boot_mode0);
	hwlog_info("%s: gpio:%d, value:%d", __func__, cd->bb_boot_mode0, rc);

	rc = gpio_get_value(cd->bb_boot_mode3);
	hwlog_info("%s: gpio:%d, value:%d", __func__, cd->bb_boot_mode3, rc);

	rc = gpio_get_value(cd->bds_bst_en_gpio);
	hwlog_info("%s: gpio:%d, value:%d", __func__, cd->bds_bst_en_gpio, rc);
}

static int32_t boot_mode_gpio_ctrl(int boot_mode0, int boot_mode3)
{
	struct ps_core_s *cd = get_core_data();

	hwlog_info("%s: bootmode0:%d, bootmode3:%d", __func__, boot_mode0, boot_mode3);
	if (cd == NULL) {
		hwlog_err("%s: core data is null", __func__);
		return -1;
	}
	if ((!gpio_is_valid(cd->bb_boot_mode0)) || (!gpio_is_valid(cd->bb_boot_mode0))) {
		hwlog_err("%s: invalid boot mode gpio", __func__);
		return -1;
	}
	gpio_set_value(cd->bb_boot_mode0, boot_mode0);
	gpio_set_value(cd->bb_boot_mode3, boot_mode3);
	return 0;
}

int32_t set_boot_mode(int boot_mode)
{
	int32_t ret;
	int boot_mode0;
	int boot_mode1;
	int bbic_chip_type = get_bbic_chip_type();

	hwlog_info("%s: boot mode:%d, chip type:%d", __func__, boot_mode, bbic_chip_type);
	if (!is_valid_bbic_chip_type(bbic_chip_type)) {
		hwlog_err("%s: invalid bbic chip type:%d", __func__, bbic_chip_type);
		return -1;
	}
	switch (boot_mode) {
	case BBIC_LOAD_FROM_USB:
		boot_mode0 = GPIO_LOW;
		boot_mode1 = (bbic_chip_type == BBIC_CHIP_TYPE_HLCT) ? GPIO_LOW : GPIO_HIGH;
		break;
	case BBIC_LOAD_FROM_UART:
		boot_mode0 = GPIO_HIGH;
		boot_mode1 = GPIO_LOW;
		break;
	case BBIC_BOOT_NORMAL:
		ret = boot_mode_gpio_ctrl(GPIO_LOW, GPIO_HIGH);
		boot_mode0 = GPIO_LOW;
		boot_mode1 = (bbic_chip_type == BBIC_CHIP_TYPE_HLCT) ? GPIO_HIGH : GPIO_LOW;
		break;
	default:
		hwlog_err("%s: invalid boot mode:%d", __func__, boot_mode);
		return -1;
	}
	ret = boot_mode_gpio_ctrl(boot_mode0, boot_mode1);
	return ret;
}

int32_t hw_set_usim_switch(int sim_slot, int gpio_status)
{
	struct ps_core_s *cd = get_core_data();

	hwlog_info("%s, sim_slot:%d, gpio_status %d", __func__, sim_slot, gpio_status);
	if (cd == NULL) {
		hwlog_err("%s: core data is null", __func__);
		return -1;
	}
	if ((gpio_status != GPIO_HIGH) && (gpio_status != GPIO_LOW)) {
		hwlog_err("%s: invalid sim gpio status", __func__);
		return -1;
	}
	if ((sim_slot != SIM_SLOT_0) && (sim_slot != SIM_SLOT_1)) {
		hwlog_err("%s: invalid sim slot %d", __func__, sim_slot);
		return -1;
	}
	if (sim_slot == SIM_SLOT_0) {
		if (gpio_is_valid(cd->usim0_switch)) // XGPIO_007
			gpio_set_value(cd->usim0_switch, gpio_status);
	} else {
		if (gpio_is_valid(cd->usim1_switch)) // XGPIO_010
			gpio_set_value(cd->usim1_switch, gpio_status);
	}
	return 0;
}

static void sim_ldo_regulator_deinit(int sim_idx)
{
	int32_t ret = -1;

	hwlog_info("%s:sim idx:%d\n", __func__, sim_idx);
	if (sim_idx == SIM_SLOT_0) {
		if (sim0_card_ldo_regulator != NULL) {
			ret = regulator_disable(sim0_card_ldo_regulator);
			hwlog_info("%s:regulator_disable ret:%d, sim idx:%d.\n", __func__, ret, sim_idx);
			devm_regulator_put(sim0_card_ldo_regulator);
			sim0_card_ldo_regulator = NULL;
		}
	} else if (sim_idx == SIM_SLOT_1) {
		if (sim1_card_ldo_regulator != NULL) {
			ret = regulator_disable(sim1_card_ldo_regulator);
			hwlog_info("%s:regulator_disable ret:%d, sim idx:%d.\n", __func__, ret, sim_idx);
			devm_regulator_put(sim1_card_ldo_regulator);
			sim1_card_ldo_regulator = NULL;
		}
		if (sim1_card_back_ldo_regulator != NULL) {
			ret = regulator_disable(sim1_card_back_ldo_regulator);
			hwlog_info("%s:ldo_back regulator_disable ret:%d, sim idx:%d.\n", __func__, ret, sim_idx);
			devm_regulator_put(sim1_card_back_ldo_regulator);
			sim1_card_back_ldo_regulator = NULL;
		}
	} else {
		hwlog_err("%s:invalid sim index:%d\n", __func__, sim_idx);
	}
}

static int32_t sim_reset(int sim_idx)
{
	struct ps_core_s *cd = get_core_data();

	hwlog_info("%s: enter", __func__);
	if (cd == NULL) {
		hwlog_err("%s: core data is null", __func__);
		return -1;
	}
	if ((sim_idx != SIM_SLOT_0) && (sim_idx != SIM_SLOT_1)) {
		hwlog_err("%s:invalid sim slot %d\n", __func__, sim_idx);
		return -1;
	}
	if (sim_idx == SIM_SLOT_0) {
		sim_ldo_regulator_deinit(SIM_SLOT_0);
		msleep(SIM_LDO_SWITCH_TIME_MS);
		return hw_set_usim_switch(sim_idx, GPIO_LOW);
	}
	sim_ldo_regulator_deinit(SIM_SLOT_1);
	msleep(SIM_LDO_SWITCH_TIME_MS);
	return hw_set_usim_switch(sim_idx, GPIO_LOW);
}

static int32_t sim_card_ldo_power_on_ldo9(struct ps_core_s *cd, int vcc_idx)
{
	int32_t ret;

	hwlog_info("%s: vcc_idx:%d", __func__, vcc_idx);

	sim1_card_back_ldo_regulator = devm_regulator_get(cd->dev, "bbic_sim1_back_ldo");
	if (IS_ERR(sim1_card_back_ldo_regulator)) {
		hwlog_info("%s:No pmu ldo found for sim_vcc\n", __func__);
		return -1;
	}

	if (vcc_idx == BBIC_SIM_VCC_1V8)
		ret = regulator_set_voltage(sim1_card_back_ldo_regulator, cd->usim1_card_ldo_vol, cd->usim1_card_ldo_vol);
	else
		ret = regulator_set_voltage(sim1_card_back_ldo_regulator, cd->usim0_ldo_vol_3v, cd->usim0_ldo_vol_3v);
	if (ret != 0)
		hwlog_info("%s:regulator_set_voltage sim_vcc fail,ret = %d\n", __func__, ret);

	ret = regulator_set_mode(sim1_card_back_ldo_regulator, REGULATOR_MODE_NORMAL);
	if (ret != 0)
		hwlog_info("%s:regulator_set_mode sim_vcc fail,ret = %d\n", __func__, ret);

	ret = regulator_enable(sim1_card_back_ldo_regulator);
	if (ret != 0)
		hwlog_info("%s:regulator_enable,ret = %d\n", __func__, ret);
	return ret;
}

static int32_t sim0_card_ldo_ctrl(struct ps_core_s *cd, int vcc_idx)
{
	int32_t ret;

	if (sim0_card_ldo_regulator == NULL)
		sim0_card_ldo_regulator = devm_regulator_get(cd->dev, "bbic_sim0_ldo");

	if (IS_ERR(sim0_card_ldo_regulator)) {
		hwlog_info("%s:No pmu ldo found for sim_vcc\n", __func__);
		return -1;
	}
	if (vcc_idx == BBIC_SIM_VCC_1V8)
		ret = regulator_set_voltage(sim0_card_ldo_regulator, cd->usim0_card_ldo_vol, cd->usim0_card_ldo_vol);
	else
		ret = regulator_set_voltage(sim0_card_ldo_regulator, cd->usim0_ldo_vol_3v, cd->usim0_ldo_vol_3v);
	if (ret != 0)
		hwlog_info("%s:regulator_set_voltage sim_vcc fail,ret = %d\n", __func__, ret);

	ret = regulator_set_mode(sim0_card_ldo_regulator, REGULATOR_MODE_NORMAL);
	if (ret != 0)
		hwlog_info("%s:regulator_set_mode sim_vcc fail,ret = %d\n", __func__, ret);

	ret = regulator_enable(sim0_card_ldo_regulator);
	if (ret != 0)
		hwlog_info("%s:regulator_enable,ret = %d\n", __func__, ret);
	return ret;
}

// SIM card ldo
static int32_t sim_card_ldo_power_on(int sim_idx, int vcc_idx)
{
	int32_t ret;
	struct ps_core_s *cd = get_core_data();

	hwlog_info("%s: enter", __func__);
	if (cd == NULL) {
		hwlog_err("%s: core data is null", __func__);
		return -1;
	}
	if ((sim_idx != SIM_SLOT_0) && (sim_idx != SIM_SLOT_1)) {
		hwlog_err("%s:invalid sim slot %d\n", __func__, sim_idx);
		return -1;
	}
	if (sim_idx == SIM_SLOT_1) {
		if (sim1_card_ldo_regulator == NULL)
			sim1_card_ldo_regulator = devm_regulator_get(cd->dev, "bbic_sim1_ldo");

		if (IS_ERR(sim1_card_ldo_regulator)) {
			hwlog_info("%s:No pmu ldo found for sim_vcc\n", __func__);
			return -1;
		}
		if (vcc_idx == BBIC_SIM_VCC_1V8)
			ret = regulator_set_voltage(sim1_card_ldo_regulator, cd->usim1_card_ldo_vol, cd->usim1_card_ldo_vol);
		else
			ret = regulator_set_voltage(sim1_card_ldo_regulator, cd->usim0_ldo_vol_3v, cd->usim0_ldo_vol_3v);
		if (ret != 0)
			hwlog_info("%s:regulator_set_voltage sim_vcc fail,ret = %d\n", __func__, ret);

		ret = regulator_set_mode(sim1_card_ldo_regulator, REGULATOR_MODE_NORMAL);
		if (ret != 0)
			hwlog_info("%s:regulator_set_mode sim_vcc fail,ret = %d\n", __func__, ret);

		ret = regulator_enable(sim1_card_ldo_regulator);
		if (ret != 0)
			hwlog_info("%s:regulator_enable,ret = %d\n", __func__, ret);

		msleep(SIM_LDO_SWITCH_TIME_MS);
		sim_card_ldo_power_on_ldo9(cd, vcc_idx);
		return ret;
	}
	ret = sim0_card_ldo_ctrl(cd, vcc_idx);
	return ret;
}

// BBIC sim IO
static int32_t hw_usim_ldo_power_on(int vcc_idx)
{
	int32_t ret;
	struct ps_core_s *cd = get_core_data();

	hwlog_info("%s: enter", __func__);
	if (cd == NULL) {
		hwlog_err("%s: core data is null", __func__);
		return -1;
	}
	if ((vcc_idx != BBIC_SIM_VCC_1V8) && (vcc_idx != BBIC_SIM_VCC_3V)) {
		hwlog_err("%s: invalid vcc idx:%d", __func__, vcc_idx);
		return -1;
	}
	if (usim_ldo_regulator != NULL) {
		ret = regulator_disable(usim_ldo_regulator);
		hwlog_info("%s:usim ldo regulator disable,ret = %d\n", __func__, ret);
		devm_regulator_put(usim_ldo_regulator);
		usim_ldo_regulator = NULL;
	}
	usim_ldo_regulator = devm_regulator_get(cd->dev, "bbic_usim_ldo");
	if (IS_ERR(usim_ldo_regulator)) {
		hwlog_info("%s:No pmu ldo found for bbic_usim_ldo_num\n", __func__);
		return -1;
	}
	if (vcc_idx == BBIC_SIM_VCC_1V8) {
		ret = regulator_set_voltage(usim_ldo_regulator, cd->usim0_ldo_vol_1v8, cd->usim0_ldo_vol_1v8);
	} else {
		ret = regulator_set_voltage(usim_ldo_regulator, cd->usim0_ldo_vol_3v, cd->usim0_ldo_vol_3v);
	}
	if (ret != 0) {
		hwlog_info("%s:regulator_set_voltage usim0 ldo fail,ret = %d\n", __func__, ret);
		return ret;
	}
	ret = regulator_set_mode(usim_ldo_regulator, REGULATOR_MODE_NORMAL);
	if (ret != 0)
		hwlog_info("%s:regulator_set_mode usim0 fail,ret = %d\n", __func__, ret);

	ret = regulator_enable(usim_ldo_regulator);
	if (ret != 0)
		hwlog_info("%s:regulator_enable,ret = %d\n", __func__, ret);
	return ret;
}

int32_t hw_sim_vcc_switch_proc(int vcc_idx)
{
	int32_t ret = -1;

	if (g_sim_connect_state == SIM_AT_MODEM) {
		hwlog_err("%s invalid sim state:sim%d is at modem\n", __func__, g_sim_connect_state);
		return -1;
	}
	if (g_sim_at_bbic_state == SIM0_AT_BBIC) {
		sim_ldo_regulator_deinit(SIM_SLOT_0);
		ret = sim_card_ldo_power_on(SIM_SLOT_0, vcc_idx);
	} else if (g_sim_at_bbic_state == SIM1_AT_BBIC) {
		sim_ldo_regulator_deinit(SIM_SLOT_1);
		ret = sim_card_ldo_power_on(SIM_SLOT_1, vcc_idx);
	} else {
		hwlog_err("%s invalid sim state:sim%d\n", __func__, g_sim_at_bbic_state);
		return ret;
	}
	ret = hw_usim_ldo_power_on(vcc_idx);
	return ret;
}

static int32_t ls_ldo_power_on(struct ps_core_s *cd)
{
	int32_t ret;

	hwlog_info("%s: enter", __func__);
	ls_ldo_regulator = devm_regulator_get(cd->dev, "bbic_ls_ldo");
	if (IS_ERR(ls_ldo_regulator)) {
		hwlog_info("%s:No pmu ldo found for bbic_ls_ldo\n", __func__);
		return -1;
	}
	ret = regulator_set_voltage(ls_ldo_regulator, cd->ls_ldo_vol, cd->ls_ldo_vol);
	if (ret != 0)
		hwlog_info("%s:regulator_set_voltage ls_ldo_vol fail,ret = %d\n", __func__, ret);

	ret = regulator_set_mode(ls_ldo_regulator, REGULATOR_MODE_NORMAL);
	if (ret != 0)
		hwlog_info("%s:regulator_set_mode ls_ldo_vol fail,ret = %d\n", __func__, ret);

	ret = regulator_enable(ls_ldo_regulator);
	if (ret != 0)
		hwlog_info("%s:regulator_enable,ret = %d\n", __func__, ret);
	return ret;
}

static int32_t core_ldo_power_on(struct ps_core_s *cd)
{
	int32_t ret;

	hwlog_info("%s: enter", __func__);
	core_ldo_regulator = devm_regulator_get(cd->dev, "bbic_core_ldo");
	if (IS_ERR(core_ldo_regulator)) {
		hwlog_info("%s:No pmu ldo found for bbic_core_ldo\n", __func__);
		return -1;
	}
	ret = regulator_set_voltage(core_ldo_regulator, cd->core_ldo_vol, cd->core_ldo_vol);
	if (ret != 0)
		hwlog_info("%s:regulator_set_voltage core_ldo_regulator fail,ret = %d\n", __func__, ret);

	ret = regulator_set_mode(core_ldo_regulator, REGULATOR_MODE_NORMAL);
	if (ret != 0)
		hwlog_info("%s:regulator_set_mode core_ldo_regulator fail,ret = %d\n", __func__, ret);

	ret = regulator_enable(core_ldo_regulator);
	if (ret != 0)
		hwlog_info("%s:regulator_enable,ret = %d\n", __func__, ret);
	return ret;
}

static int32_t vdd_buckboost_power_on(struct ps_core_s *cd)
{
	int32_t ret;

	if (bbic_vdd_buckboost_regulator == NULL) {
		bbic_vdd_buckboost_regulator = devm_regulator_get(cd->dev, "bbic_vdd_buckboost");
		hwlog_info("%s:bbic_vdd_buckboost register", __func__);
	}
	if (IS_ERR(bbic_vdd_buckboost_regulator)) {
		hwlog_info("%s:No pmu ldo found for bbic_vdd\n", __func__);
	}
	ret = regulator_set_voltage(bbic_vdd_buckboost_regulator, cd->vdd_buckboost_vol, cd->vdd_buckboost_vol);
	if (ret != 0)
		hwlog_info("%s:regulator_set_voltage bbic_vdd fail,ret = %d\n", __func__, ret);
	ret = regulator_set_mode(bbic_vdd_buckboost_regulator, REGULATOR_MODE_NORMAL);
	if (ret != 0)
		hwlog_info("%s:regulator_set_mode bbic_vdd fail,ret = %d\n", __func__, ret);
 
	ret = regulator_enable(bbic_vdd_buckboost_regulator);
	if (ret != 0)
		hwlog_info("%s:regulator_bbic_vdd_enable,ret = %d\n", __func__, ret);

	return ret;
}

static int32_t vbuck3_vol_power_on(struct ps_core_s *cd)
{
	int32_t ret;

	hwlog_info("%s: enter", __func__);
	if (bbic_xbuck3_regulator == NULL) {
		bbic_xbuck3_regulator = devm_regulator_get(cd->dev, "bbic_vbuck3");
		hwlog_info("%s:xbuck3 register\n", __func__);
	}
	if (IS_ERR(bbic_xbuck3_regulator)) {
		hwlog_info("%s:No pmu ldo found for bbic_vbuck3\n", __func__);
		return -1;
	}
	ret = regulator_set_voltage(bbic_xbuck3_regulator, cd->vbuck3_vol, cd->vbuck3_vol);
	if (ret != 0)
		hwlog_info("%s:regulator_set_voltage bbic_vbuck3 fail,ret = %d\n", __func__, ret);

	ret = regulator_set_mode(bbic_xbuck3_regulator, REGULATOR_MODE_NORMAL);
	if (ret != 0)
		hwlog_info("%s:regulator_set_mode bbic_vbuck3 fail,ret = %d\n", __func__, ret);

	ret = regulator_enable(bbic_xbuck3_regulator);
	if (ret != 0)
		hwlog_info("%s:regulator_bbic_vbuck3_enable,ret = %d\n", __func__, ret);
	return ret;
}

// SIM switch modem to BBIC
int32_t hw_set_sim_switch_modem_to_bbic(int sim_idx)
{
	int32_t ret;

	if (g_sim_connect_state == SIM_AT_BBIC) {
		hwlog_err("%s: sim is already at bbic", __func__);
		return 0;
	}
	if ((sim_idx != SIM_SLOT_0) && (sim_idx != SIM_SLOT_1)) {
		hwlog_err("%s:invalid sim slot %d\n", __func__, sim_idx);
		return -1;
	}
	ret = hw_set_usim_switch(sim_idx, SIM_AT_BBIC);
	if (ret != 0) {
		hwlog_err("%s: sim switch is err %d", __func__, ret);
		return -1;
	}
	msleep(SIM_LDO_SWITCH_TIME_MS);
	ret = sim_card_ldo_power_on(sim_idx, BBIC_SIM_VCC_1V8);
	if (ret != 0) {
		hwlog_err("%s: sim card power on is err %d", __func__, ret);
		return -1;
	}
	ret = hw_usim_ldo_power_on(BBIC_SIM_VCC_1V8);
	if (ret != 0) {
		hwlog_err("%s: usim ldo power on is err %d", __func__, ret);
		return -1;
	}
	g_sim_connect_state = SIM_AT_BBIC;
	g_sim_at_bbic_state = sim_idx;
	return ret;
}

// SIM switch BBIC to modem
int32_t hw_set_sim_switch_bbic_to_modem(int sim_idx)
{
	int32_t ret;

	if (g_sim_connect_state == SIM_AT_MODEM) {
		hwlog_err("%s: sim is already at modem", __func__);
		return 0;
	}
	if ((sim_idx != SIM_SLOT_0) && (sim_idx != SIM_SLOT_1)) {
		hwlog_err("%s:invalid sim slot %d\n", __func__, sim_idx);
		return -1;
	}
	hwlog_info("%s: disable sim ldo, sim_idx:%d", __func__, sim_idx);
	if (sim_idx == SIM_SLOT_0) {
		sim_ldo_regulator_deinit(SIM_SLOT_0);
	} else if (sim1_card_ldo_regulator != NULL) {
		hwlog_info("%s: disable sim1 ldo regulator, sim_idx:%d", __func__, sim_idx);
		sim_ldo_regulator_deinit(SIM_SLOT_1);
	}
	hwlog_info("%s: sim ldo done, sim_idx:%d", __func__, sim_idx);
	msleep(SIM_LDO_SWITCH_TIME_MS);
	ret = hw_set_usim_switch(sim_idx, SIM_AT_MODEM);
	if (ret != 0) {
		hwlog_err("%s: sim switch is err %d", __func__, ret);
		return -1;
	}
	if (usim_ldo_regulator != NULL) {
		ret = regulator_disable(usim_ldo_regulator);
		devm_regulator_put(usim_ldo_regulator);
		usim_ldo_regulator = NULL;
	}
	hwlog_info("%s: hw_set_sim_switch_bbic_to_modem done", __func__);
	g_sim_connect_state = SIM_AT_MODEM;
	return ret;
}

int32_t hw_bbic_sleep_switch(int status)
{
	int32_t ret = -1;
	struct ps_core_s *cd = get_core_data();

	hwlog_info("%s: bbic sleep switch status %d, current status:%d", __func__, status, g_bbic_sleep_gpio_status);
	if (cd == NULL) {
		hwlog_err("%s: core data is null", __func__);
		return -1;
	}

	switch (status) {
	case BBIC_SLEEP_GPIO_SWITCH_TO_NORMAL:
		if (g_bbic_sleep_gpio_status != BBIC_SLEEP_GPIO_LOCAL_STATUS_NORMAL) {
			ret = gpio_direction_output(cd->ap_wakeup_bb, GPIO_HIGH);
			if (ret) {
				hwlog_err("%s: switch sleep gpio to normal err %d", __func__, ret);
				return ret;
			}
			g_bbic_sleep_gpio_status = BBIC_SLEEP_GPIO_LOCAL_STATUS_NORMAL;
		}
		break;
	case BBIC_SLEEP_GPIO_SWITCH_TO_INPUT:
		if (g_bbic_sleep_gpio_status != BBIC_SLEEP_GPIO_LOCAL_STATUS_INPUT) {
			ret = gpio_direction_input(cd->ap_wakeup_bb);
			if (ret) {
				hwlog_err("%s: switch sleep gpio to input err %d", __func__, ret);
				return ret;
			}
			g_bbic_sleep_gpio_status = BBIC_SLEEP_GPIO_LOCAL_STATUS_INPUT;
		}
		break;
	default:
		hwlog_info("%s invalid status %d\n", __func__, status);
	}
	return ret;
}

int32_t hw_bbic_sleep_gpio_status_query(void)
{
	int32_t ret = BBIC_SLEEP_GPIO_INVALID_STATUS;
	int32_t ap_wakeup_bb_status;
	int32_t bb_wakeup_ap_status;
	struct ps_core_s *cd = get_core_data();

	if (cd == NULL) {
		hwlog_err("%s: core data is null", __func__);
		return ret;
	}
	if ((!gpio_is_valid(cd->ap_wakeup_bb)) || (!gpio_is_valid(cd->bb_wakeup_ap))) {
		hwlog_err("%s: invalid sleep gpio", __func__);
		return ret;
	}
	ap_wakeup_bb_status = gpio_get_value(cd->ap_wakeup_bb);
	bb_wakeup_ap_status = gpio_get_value(cd->bb_wakeup_ap);
	hwlog_info("%s: ap_wakeup_bb_status:%d, bb_wakeup_ap_status:%d",
		__func__, ap_wakeup_bb_status, bb_wakeup_ap_status);
	if ((ap_wakeup_bb_status == GPIO_LOW) && (bb_wakeup_ap_status == GPIO_LOW))
		return BBIC_SLEEP_GPIO_ALL_LOW_STATUS;
	if ((ap_wakeup_bb_status == GPIO_HIGH) && (bb_wakeup_ap_status == GPIO_HIGH))
		return BBIC_SLEEP_GPIO_ALL_HIGH_STATUS;
	return ret;
}

static void rf_pwr_off_proc(struct ps_core_s *cd)
{
	int ret;

	hwlog_info("%s: enter", __func__);
	// GPIO 219/351/332 config
	if (gpio_is_valid(cd->ant_sw1_vc1))
		gpio_set_value(cd->ant_sw1_vc1, GPIO_HIGH);
	if (gpio_is_valid(cd->ant_vc1))
		gpio_set_value(cd->ant_vc1, GPIO_HIGH);
	if (gpio_is_valid(cd->ant_vc2))
		gpio_set_value(cd->ant_vc2, GPIO_LOW);

	if (gpio_is_valid(cd->bds_mos_en)) // GPIO_204 mos en
		gpio_set_value(cd->bds_mos_en, GPIO_LOW);
	// xbuck3 off
	if (bbic_xbuck3_regulator != NULL) {
		ret = regulator_set_voltage(bbic_xbuck3_regulator, cd->vbuck3_off, cd->vbuck3_off);
		if (ret != 0)
		hwlog_info("%s:regulator_set_voltage vbuck3_off fail,ret = %d\n", __func__, ret);
		ret = regulator_set_mode(bbic_xbuck3_regulator, REGULATOR_MODE_NORMAL);
		if (ret != 0)
		hwlog_info("%s:regulator_set_mode vbuck3_off fail,ret = %d\n", __func__, ret);
		ret = regulator_enable(bbic_xbuck3_regulator);
		if (ret != 0)
		hwlog_info("%s:regulator_vbuck3_off_enable,ret = %d\n", __func__, ret);
	}

	if (bbic_vdd_buckboost_regulator != NULL) {
		ret = regulator_set_voltage(bbic_vdd_buckboost_regulator, BUCKBOOST_MIN_MICROVOLT, BUCKBOOST_MAX_MICROVOLT);
		if (ret != 0)
		hwlog_info("%s:regulator_set_voltage bbic_vdd fail,ret = %d\n", __func__, ret);
		ret = regulator_set_mode(bbic_vdd_buckboost_regulator, REGULATOR_MODE_NORMAL);
		if (ret != 0)
		hwlog_info("%s:regulator_set_mode bbic_vdd fail,ret = %d\n", __func__, ret);
		ret = regulator_enable(bbic_vdd_buckboost_regulator);
		if (ret != 0)
		hwlog_info("%s:regulator_bbic_vdd_enable,ret = %d\n", __func__, ret);
	}
}

static int bbic_pwr_off_proc(struct ps_core_s *cd)
{
	int ret;
	if (gpio_is_valid(cd->bb_reset)) { // GPIO_006
		hwlog_info("%s, bbic reset", __func__);
		gpio_set_value(cd->bb_reset, GPIO_LOW);
	}
	msleep(BBIC_RESET_TIME_MS);
	if (gpio_is_valid(cd->usb_ls_pwr_en)) // XGPIO_001
		gpio_set_value(cd->usb_ls_pwr_en, GPIO_LOW);
	if ((get_bbic_chip_type() == BBIC_CHIP_TYPE_WS) && (gpio_is_valid(cd->ws_core_en))) // XGPIO_000
		gpio_set_value(cd->ws_core_en, GPIO_LOW);
	if (core_ldo_regulator != NULL) {
		ret = regulator_disable(core_ldo_regulator);
		hwlog_info("%s:core ldo regulator disable,ret = %d\n", __func__, ret);
		devm_regulator_put(core_ldo_regulator);
		core_ldo_regulator = NULL;
	}
	if (ls_ldo_regulator != NULL) {
		ret = regulator_disable(ls_ldo_regulator);
		hwlog_info("%s:ls ldo regulator disable,ret = %d\n", __func__, ret);
		devm_regulator_put(ls_ldo_regulator);
		ls_ldo_regulator = NULL;
	}
	msleep(1);

	if (gpio_is_valid(cd->bds_bst_en_gpio)) // GRL XGPIO_003
		gpio_set_value(cd->bds_bst_en_gpio, GPIO_LOW);

	if (get_bbic_chip_type() == BBIC_CHIP_TYPE_WS) { // GPIO_253
		ret = boost_5v_enable(BOOST_5V_DISABLE, BOOST_CTRL_HW_SATE);
		hwlog_info("%s: disable ws vbst 5v_en done, ret %d", __func__, ret);
	}
	if (get_bbic_chip_type() == BBIC_CHIP_TYPE_HLCT) {
		if (hlct_clk != NULL)
			clk_disable_unprepare(hlct_clk);
	}
	hwlog_info("%s: BBIC plcpu idle sleep vote 0", __func__);
	lpcpu_idle_sleep_vote(ID_BBIC, 0);
	buck_boost_set_vout(0, BBST_USER_BOOST_HW_SATE);
	buck_boost_set_enable(GPIO_LOW, BBST_USER_BOOST_HW_SATE);
	if (usim_ldo_regulator != NULL) {
		ret = regulator_disable(usim_ldo_regulator);
		hwlog_info("%s:usim ldo regulator disable,ret = %d\n", __func__, ret);
		devm_regulator_put(usim_ldo_regulator);
		usim_ldo_regulator = NULL;
	}
	rf_pwr_off_proc(cd);
	return ret;
}

static void rf_pwr_on_proc(struct ps_core_s *cd)
{
	int bbic_chip_type = get_bbic_chip_type();
	// GRL XGPIO_003 GPIO_006
	if (gpio_is_valid(cd->bds_bst_en_gpio)) // GRL XGPIO_003
		gpio_set_value(cd->bds_bst_en_gpio, GPIO_HIGH);
	if (gpio_is_valid(cd->bb_reset)) { // GPIO_006
		hwlog_info("%s, bbic reset", __func__);
		gpio_set_value(cd->bb_reset, GPIO_HIGH);
	}
	// GPIO 219/351/332 config
	hwlog_info("rf_pwr_on_proc: ant_sw1_vc1 is %d, ant_sw2_vc1 is %d, ant_sw2_vc2 is %d\n",
		gpio_is_valid(cd->ant_sw1_vc1), gpio_is_valid(cd->ant_vc1), gpio_is_valid(cd->ant_vc2));
	if (gpio_is_valid(cd->ant_sw1_vc1))
		gpio_set_value(cd->ant_sw1_vc1, GPIO_LOW);
	if (gpio_is_valid(cd->ant_vc1))
		gpio_set_value(cd->ant_vc1, GPIO_LOW);
	if (gpio_is_valid(cd->ant_vc2))
		gpio_set_value(cd->ant_vc2, GPIO_HIGH);

	if (gpio_is_valid(cd->bds_mos_en)) // GPIO_204 mos en
		gpio_set_value(cd->bds_mos_en, GPIO_HIGH);

	buck_boost_set_enable(GPIO_HIGH, BBST_USER_BOOST_HW_SATE);

	if (bbic_chip_type == BBIC_CHIP_TYPE_WS) {
		buck_boost_set_vout(cd->buck_boost_volt, BBST_USER_BOOST_HW_SATE);
	} else if (bbic_chip_type == BBIC_CHIP_TYPE_HLCT) {
		buck_boost_set_vout(PA_BST_VOL_4V8, BBST_USER_BOOST_HW_SATE);
	} else {
		hwlog_err("%s: bbic chip type err:%d", __func__, bbic_chip_type);
		return;
	}
}

static int32_t ldo_pwr_on_proc(struct ps_core_s *cd)
{
	int32_t ret;
	if (ldo_is_valid(cd->vbuck3_vol)) {
		ret = vbuck3_vol_power_on(cd);
		if (ret != 0) {
			hwlog_err("%s: vbuck3 pwr on fail, ret:%d", __func__, ret);
			return -1;
		}
	}
	if (ldo_is_valid(cd->core_ldo_vol)) {
		ret = core_ldo_power_on(cd);
		if (ret != 0) {
			hwlog_err("%s: core pwr on fail, ret:%d", __func__, ret);
			return -1;
		}
	}
	if (ldo_is_valid(cd->vdd_buckboost_vol)) {
		ret = vdd_buckboost_power_on(cd);
		if (ret != 0) {
			hwlog_err("%s: core pwr on fail, ret:%d", __func__, ret);
			return -1;
		}
	}
	return ret;
}

static int bbic_pwr_on_proc(struct ps_core_s *cd)
{
	int ret;
	if (get_bbic_chip_type() == BBIC_CHIP_TYPE_HLCT) {
		if (hlct_clk == NULL) {
			hwlog_err("%s: clk is null", __func__);
			return -1;
		}
		ret = clk_prepare_enable(hlct_clk);
		if (ret != 0) {
			hwlog_err("%s: hlct_clk enable fail, ret:%d", __func__, ret);
			return -1;
		}
	}
	hwlog_info("%s: BBIC plcpu idle sleep vote 1", __func__);
	lpcpu_idle_sleep_vote(ID_BBIC, 1);
	if (get_bbic_chip_type() == BBIC_CHIP_TYPE_WS) { // GPIO_253
		ret = boost_5v_enable(BOOST_5V_ENABLE, BOOST_CTRL_HW_SATE);
		if (ret) {
			hwlog_err("%s: set ws vbst 5v_en fail, ret %d", __func__, ret);
			return ret;
		}
	}
	msleep(1);
	ret = ldo_pwr_on_proc(cd);
	if (ret != 0) {
		hwlog_err("%s: ldo pwr on fail, ret:%d", __func__, ret);
		return -1;
	}
	if (get_bbic_chip_type() == BBIC_CHIP_TYPE_HLCT)
		msleep(1);
	if (ldo_is_valid(cd->ls_ldo_vol)) {
		ret = ls_ldo_power_on(cd);
		if (ret != 0) {
			hwlog_err("%s: ls pwr on fail, ret:%d", __func__, ret);
			return -1;
		}
	}
	if ((get_bbic_chip_type() == BBIC_CHIP_TYPE_WS) && (gpio_is_valid(cd->ws_core_en))) { // XGPIO_000
		gpio_set_value(cd->ws_core_en, GPIO_HIGH);
		hwlog_info("%s, ws_core_en valid %d", __func__, cd->ws_core_en);
	}
	if (gpio_is_valid(cd->usb_ls_pwr_en)) // XGPIO_001
		gpio_set_value(cd->usb_ls_pwr_en, GPIO_HIGH);

	msleep(BBIC_RESET_TIME_MS);
	rf_pwr_on_proc(cd);
	return ret;
}

int pwr_gpio_ctrl(bool status, bool is_need_set_bootmode)
{
	struct ps_core_s *cd = get_core_data();
	int32_t set_value = status ? GPIO_HIGH : GPIO_LOW;
	int ret;
	int boot_mode0 = GPIO_LOW;
	int boot_mode3 = GPIO_HIGH;
	int ap_sleep_status = (get_bbic_chip_type() == BBIC_CHIP_TYPE_WS) ? GPIO_HIGH : GPIO_LOW;
	hwlog_info("%s: enter, set_value %d, ap_sleep_status %d", __func__, set_value, ap_sleep_status);

	if (cd == NULL) {
		hwlog_err("%s: core data is null", __func__);
		return -1;
	}
	if (status) {
		if (is_need_set_bootmode) {
			if (get_bbic_chip_type() == BBIC_CHIP_TYPE_WS) {
				boot_mode0 = GPIO_LOW;
				boot_mode3 = GPIO_LOW;
			}
			if (gpio_is_valid(cd->bb_boot_mode0))
				gpio_set_value(cd->bb_boot_mode0, boot_mode0);
			if (gpio_is_valid(cd->bb_boot_mode3))
				gpio_set_value(cd->bb_boot_mode3, boot_mode3);
			msleep(1);
		}
		if (gpio_is_valid(cd->ap_wakeup_bb))
			gpio_set_value(cd->ap_wakeup_bb, ap_sleep_status);
		ret = bbic_pwr_on_proc(cd);
		hwlog_info("%s, bbic_pwr_on_proc done", __func__);
		set_bbic_pwr_status(true);
	} else {
		ret = bbic_pwr_off_proc(cd);
		if (gpio_is_valid(cd->bb_boot_mode0))
			gpio_set_value(cd->bb_boot_mode0, GPIO_LOW);
		if (gpio_is_valid(cd->bb_boot_mode3))
			gpio_set_value(cd->bb_boot_mode3, GPIO_LOW);
		if (gpio_is_valid(cd->ap_wakeup_bb))
			gpio_set_value(cd->ap_wakeup_bb, GPIO_LOW);
		if (ret != 0)
			hwlog_info("%s, bbic pwr off err:%d", __func__, ret);
		ret = hw_set_usim_switch(0, GPIO_LOW);
		if (ret != 0)
			hwlog_info("%s, set sim0 switch err:%d", __func__, ret);
		ret = hw_set_usim_switch(1, GPIO_LOW);
		if (ret != 0)
			hwlog_info("%s, set sim1 switch err:%d", __func__, ret);
		set_bbic_pwr_status(false);
	}
	return ret;
}

static int32_t ps_probe(struct platform_device *pdev)
{
	struct ps_core_s *ps_plat_d = NULL;
	int32_t err;

	hwlog_info("%s, enter\n", __func__);
	g_hw_ps_device = pdev;
	ps_plat_d = kzalloc(sizeof(struct ps_core_s), GFP_KERNEL);
	if (ps_plat_d == NULL) {
		hwlog_info("%s, no mem to allocate\n", __func__);
		return -1;
	}
	err = parse_dts_config(pdev->dev.of_node, ps_plat_d);
	if (err != 0) {
		hwlog_info("%s, parse_dts_config failed\n", __func__);
		goto err_core_init;
	}
	ps_plat_d->dev = &pdev->dev;
	dev_set_drvdata(&pdev->dev, ps_plat_d);

	hwlog_info("%s, parse dts done\n", __func__);

	if (get_bbic_chip_type() == BBIC_CHIP_TYPE_HLCT) {
		hlct_clk = devm_clk_get(&pdev->dev, "clk_pmu32kc");
		if (IS_ERR(hlct_clk)) {
			hwlog_err("%s, parse clk_pmu32kc err\n", __func__);
			goto err_core_init;
		}
	}

	/* get reference of pdev */
	ps_plat_d->pm_pdev = pdev;

	g_core_s = ps_plat_d;

	hwlog_info("%s is success!\n", __func__);
	return 0;

err_core_init:
	kfree(ps_plat_d);

	return -1;
}

static int32_t ps_remove(struct platform_device *pdev)
{
	struct ps_core_s *ps_plat_d = NULL;

	hwlog_info("%s, device removed\n", __func__);
	ps_plat_d = dev_get_drvdata(&pdev->dev);
	if (ps_plat_d == NULL) {
		hwlog_info("%s, ps_plat_d is null\n", __func__);
		return -1;
	}
	return 0;
}

static int init_gpio(int32_t gpio_id, char *gpio_name, int status, int gpio_direction)
{
	int32_t rc;

	hwlog_info("%s: enter, gpio:%d", __func__, gpio_id);

	rc = gpio_request(gpio_id, gpio_name);
	if (rc) {
		hwlog_err("%s: gpio %d failed %d\n",
			__func__, gpio_id, rc);
		return rc;
	}

	rc = gpio_get_value(gpio_id);
	hwlog_info("%s: get gpio %d value %d",
		__func__, gpio_id, rc);

	if (gpio_direction == GPIO_DIRECTION_INPUT) {
		rc = gpio_direction_input(gpio_id);
		if (rc) {
			hwlog_err("%s: input gpio %d input high err %d",
				__func__, gpio_id, rc);
			return rc;
		}
	} else {
		rc = gpio_direction_output(gpio_id, status);
		if (rc) {
			hwlog_err("%s: output gpio %d output high err %d",
				__func__, gpio_id, rc);
			return rc;
		}
	}

	rc = gpio_get_value(gpio_id);
	hwlog_info("%s: get value gpio %d value %d",
		__func__, gpio_id, rc);

	return 0;
}

static int32_t sleep_config_init(struct ps_core_s *cd)
{
	int32_t rc;

	rc = init_gpio(cd->ap_wakeup_bb, "bbic_ap_wakeup_bb", GPIO_LOW, GPIO_DIRECTION_OUTPUT);
	if (rc != 0) {
		hwlog_err("%s: init gpio %d failed %d\n",
			__func__, cd->ap_wakeup_bb, rc);
		return rc;
	}

	rc = init_gpio(cd->bb_wakeup_ap, "bbic_bb_wakeup_ap", GPIO_HIGH, GPIO_DIRECTION_INPUT);
	if (rc != 0) {
		hwlog_err("%s: init gpio %d failed %d\n",
			__func__, cd->bb_wakeup_ap, rc);
		return rc;
	}
	return 0;
}

static int32_t boot_mode_config_init(struct ps_core_s *cd)
{
	int32_t rc;

	rc = init_gpio(cd->bb_boot_mode0, "bbic_bb_boot_mode0", GPIO_LOW, GPIO_DIRECTION_OUTPUT);
	if (rc != 0) {
		hwlog_err("%s: init gpio %d failed %d\n",
			__func__, cd->bb_boot_mode0, rc);
		return rc;
	}

	rc = init_gpio(cd->bb_boot_mode3, "bbic_bb_boot_mode3", GPIO_LOW, GPIO_DIRECTION_OUTPUT);
	if (rc != 0) {
		hwlog_err("%s: init gpio %d failed %d\n",
			__func__, cd->bb_boot_mode3, rc);
		return rc;
	}

	rc = init_gpio(cd->bb_reset, "bbic_bb_reset", GPIO_LOW, GPIO_DIRECTION_OUTPUT);
	if (rc != 0) {
		hwlog_err("%s: init gpio %d failed %d\n",
			__func__, cd->bb_reset, rc);
		return rc;
	}

	return 0;
}

static int32_t pwr_config_init_ws(struct ps_core_s *cd)
{
	int32_t rc;

	rc = init_gpio(cd->pwr_en, "bbic_pwr_en", GPIO_HIGH, GPIO_DIRECTION_OUTPUT);
	if (rc != 0) {
		hwlog_err("%s: init gpio %d failed %d\n",
			__func__, cd->pwr_en, rc);
		return rc;
	}

	// temporary plan GPIO_338 always high
	if (gpio_is_valid(cd->pwr_en)) { // GPIO_338_MCAM_BUCK
		gpio_set_value(cd->pwr_en, GPIO_HIGH);
		hwlog_info("%s: set per_en done gpio %d", __func__, cd->pwr_en);
	}
	rc = init_gpio(cd->usb_ls_pwr_en, "bbic_usb_ls_pwr_en", GPIO_LOW, GPIO_DIRECTION_OUTPUT);
	if (rc != 0) {
		hwlog_err("%s: init gpio %d failed %d\n",
			__func__, cd->usb_ls_pwr_en, rc);
		return rc;
	}

	rc = init_gpio(cd->ws_core_en, "bbic_ws_core_en", GPIO_LOW, GPIO_DIRECTION_OUTPUT);
	if (rc != 0) {
		hwlog_err("%s: init gpio %d failed %d\n",
			__func__, cd->ws_core_en, rc);
		return rc;
	}
	rc = init_gpio(cd->ant_vc1, "bbic_ant_vc1", GPIO_HIGH, GPIO_DIRECTION_OUTPUT);
	if (rc != 0) // may inited by GG, here not return
		hwlog_err("%s: init gpio %d failed %d\n",
			__func__, cd->ant_vc1, rc);

	rc = init_gpio(cd->ant_vc2, "bbic_ant_vc2", GPIO_LOW, GPIO_DIRECTION_OUTPUT);
	if (rc != 0) // may inited by GG, here not return
		hwlog_err("%s: init gpio %d failed %d\n",
			__func__, cd->ant_vc2, rc);

	rc = init_gpio(cd->ant_sw1_vc1, "bbic_ant_sw1_vc1", GPIO_HIGH, GPIO_DIRECTION_OUTPUT);
	if (rc != 0) // may inited by GG, here not return
		hwlog_err("%s: init gpio %d failed %d\n",
			__func__, cd->ant_sw1_vc1, rc);

	return 0;
}

static int32_t pwr_config_init(struct ps_core_s *cd)
{
	int32_t rc;

	rc = init_gpio(cd->pwr_en, "bbic_pwr_en", GPIO_HIGH, GPIO_DIRECTION_OUTPUT);
	if (rc != 0) {
		hwlog_err("%s: init gpio %d failed %d\n",
			__func__, cd->pwr_en, rc);
		return rc;
	}

	// temporary plan GPIO_338 always high
	if (gpio_is_valid(cd->pwr_en)) // GPIO_338_MCAM_BUCK
		gpio_set_value(cd->pwr_en, GPIO_HIGH);

	rc = init_gpio(cd->usb_ls_pwr_en, "bbic_usb_ls_pwr_en", GPIO_LOW, GPIO_DIRECTION_OUTPUT);
	if (rc != 0) {
		hwlog_err("%s: init gpio %d failed %d\n",
			__func__, cd->usb_ls_pwr_en, rc);
		return rc;
	}

	rc = init_gpio(cd->ant_vc1, "bbic_ant_vc1", GPIO_HIGH, GPIO_DIRECTION_OUTPUT);
	if (rc != 0) // may inited by GG, here not return
		hwlog_err("%s: init gpio %d failed %d\n",
			__func__, cd->ant_vc1, rc);

	rc = init_gpio(cd->ant_vc2, "bbic_ant_vc2", GPIO_LOW, GPIO_DIRECTION_OUTPUT);
	if (rc != 0) // may inited by GG, here not return
		hwlog_err("%s: init gpio %d failed %d\n",
			__func__, cd->ant_vc2, rc);

	rc = init_gpio(cd->ant_sw1_vc1, "bbic_ant_sw1_vc1", GPIO_HIGH, GPIO_DIRECTION_OUTPUT);
	if (rc != 0) // may inited by GG, here not return
		hwlog_err("%s: init gpio %d failed %d\n",
			__func__, cd->ant_sw1_vc1, rc);

	return 0;
}

static int32_t usim_config_init(struct ps_core_s *cd)
{
	int32_t rc;

	rc = init_gpio(cd->usim0_switch, "bbic_usim0_switch", GPIO_LOW, GPIO_DIRECTION_OUTPUT);
	if (rc != 0) {
		hwlog_err("%s: init gpio %d failed %d\n",
			__func__, cd->usim0_switch, rc);
		return rc;
	}
	rc = init_gpio(cd->usim1_switch, "bbic_usim1_switch", GPIO_LOW, GPIO_DIRECTION_OUTPUT);
	if (rc != 0) {
		hwlog_err("%s: init gpio %d failed %d\n",
			__func__, cd->usim1_switch, rc);
		return rc;
	}

	return 0;
}

static int32_t hw_bbic_wakeup_pin_setmode(struct ps_core_s *cd)
{
	int ret;
	struct pinctrl *p = NULL;
	struct pinctrl_state *pinctrl_def = NULL;

	if (cd->dev == NULL) {
		hwlog_err("%s: dev node is null", __func__);
		return -1;
	}
	/* Get pinctrl node from dev */
	p = devm_pinctrl_get(cd->dev);
	if (IS_ERR(p)) {
		hwlog_err("%s: pinctrl is not config in dts!", __func__);
		return -1;
	}
	/* parse pinctrl node */
	pinctrl_def = pinctrl_lookup_state(p, PINCTRL_STATE_DEFAULT);
	if (IS_ERR(pinctrl_def)) {
		hwlog_err("%s: could not get defstate (%li)", __func__, PTR_ERR(pinctrl_def));
		return -1;
	}
	/* Set work mode as GPIO */
	ret = pinctrl_select_state(p, pinctrl_def);
	if (ret) {
		hwlog_err("%s: could not set pins to default state", __func__);
		return -1;
	}

	return 0;
}

static int32_t hw_bbic_driver_init(void)
{
	int32_t ret;
	struct ps_core_s *cd = NULL;

	hwlog_err("%s: enter", __func__);

	cd = get_core_data();
	if (cd == NULL) {
		hwlog_err("%s: core data is null", __func__);
		return -1;
	}

	hw_bbic_wakeup_pin_setmode(cd);

	ret = sleep_config_init(cd);
	if (ret != 0)
		return ret;
	ret = boot_mode_config_init(cd);
	if (ret != 0)
		return ret;

	if (get_bbic_chip_type() == BBIC_CHIP_TYPE_HLCT) {
		ret = pwr_config_init(cd);
	} else if (get_bbic_chip_type() == BBIC_CHIP_TYPE_WS) {
		ret = pwr_config_init_ws(cd);
	} else {
		hwlog_err("%s: invalid bbic chip type:%d", __func__, get_bbic_chip_type());
		return -1;
	}
	if (ret != 0)
		return ret;

	ret = usim_config_init(cd);
	if (ret != 0)
		return ret;

	return 0;
}

static struct of_device_id g_bbic_match_table[] = {
	{
		.compatible = DTS_BBIC_COMP_NAME,
		.data = NULL,
	},
	{},
};

static struct platform_driver g_bbic_platform_driver = {
	.probe = ps_probe,
	.remove = ps_remove,
	.driver = {
		.name = "huawei_sate",
		.owner = THIS_MODULE,
		.of_match_table = g_bbic_match_table,
	},
};

int32_t hw_bbic_dev_init(void)
{
	int ret;
	struct ps_core_s *cd = NULL;
	int count = 0;

	hwlog_info("%s, enter\n", __func__);
	ret = platform_driver_register(&g_bbic_platform_driver);
	if (ret) {
		hwlog_info("%s, Unable to register platform bbic driver.\n", __func__);
		return -1;
	}
	cd = get_core_data();
	while (cd == NULL && count < DEVICE_REG_TRY_MAX_COUNT) {
		msleep(DEVICE_REG_SLEEP_INTERVAL);
		count++;
		cd = get_core_data();
	}
	hwlog_info("%s, count:%d.\n", __func__, count);

	ret = hw_bbic_driver_init();
	if (ret) {
		hwlog_info("%s, driver init error.\n", __func__);
		return -1;
	}
	hwlog_info("%s done, ret:%d \n", __func__, ret);

	return ret;
}

void hw_bbic_dev_exit(void)
{
	platform_driver_unregister(&g_bbic_platform_driver);
}

MODULE_DESCRIPTION("Public serial Driver for bbic chips");
MODULE_LICENSE("GPL");
