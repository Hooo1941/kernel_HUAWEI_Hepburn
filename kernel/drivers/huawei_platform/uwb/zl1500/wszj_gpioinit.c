/*
 * vendor_exception.c
 *
 * Copyright (C) 2018 Huawei Device Co., Ltd.
 * This file is licensed under GPL.
 *
 * Register vendor exceptions.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
#include <linux/spinlock.h>
#include <linux/reboot.h>
#include <linux/clk.h>
#include <linux/pm_wakeup.h>
#include <linux/of.h>
#include <linux/gpio.h>
#include <linux/of_platform.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/mtd/hisi_nve_interface.h>

#include "wszj_gpioinit.h"
#include "securec.h"
#include <linux/mfd/hisi_pmic.h>
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/clk.h>

#define HWLOG_TAG UWB

#define PMU_HI_6421 1
#define PMU_HI_6423 2
#define PMU_SIM_0 0
#define PMU_SIM_1 1
#define DELAY_AFTER_1V2 5
#define DELAY_AFTER_2V85 20
#define DELAY_POWER_OFF_AFTER_1V2 31
#define DELAY_POWER_OFF_AFTER_RST 2

static pmu_reg_control_stu g_uwb_1v2_powron_by_hi6421 = {0x02a, 0};
static pmu_reg_control_stu g_uwb_285vol_adj_by_hi6423 = {0x04d, 0x2d};
static pmu_reg_control_stu g_uwb_1v2vol_adj_by_hi6421 = {0x06f, 0x05};

static pmu_reg_control_stu g_uwb_ldo53_enb_by_hi6423 = {0x02b, 0x01};
static pmu_reg_control_stu g_uwb_sim0_enb_by_hi6423 = {0x670, 0x0f};
static pmu_reg_control_stu g_uwb_sim1_enb_by_hi6423 = {0x672, 0x0f};
static pmu_reg_control_stu g_uwb_sim0_out_by_hi6423 = {0x671, 0x22};
static pmu_reg_control_stu g_uwb_sim1_out_by_hi6423 = {0x673, 0x22};

static pmu_reg_control_stu g_uwb_38m_clk_enb_by_hi6421 = {0x043, 0x01};
static pmu_reg_control_stu g_uwb_38m_clk_freq_by_hi6421 = {0x0aa, 0x4};
static pmu_reg_control_stu g_uwb_32k_clk_enb_by_hi6421 = {0x04a, 0x1};
static pmu_reg_control_stu g_uwb_38m_clk_vpp_adj_by_hi6421 = {0x0a9, 0x02};

HWLOG_REGIST();

static int uwb_pmureg_operate_by_bit(int pmuid, int regaddr, int bitpos, int op)
{
	hwlog_info("UWB: %s, in\n", __func__);
	int ret = UWB_FAIL;
	int value = 0;
	if (regaddr <= 0 || bitpos < 0 || op < 0) {
		hwlog_err("UWB: %s: reg_addr[%x], bit_pos[%d], op[%d], error!\n", __func__, regaddr, bitpos, op);
		return UWB_FAIL;
	}

	if (pmuid == PMU_HI_6421)
		ret = pmic_read_reg(regaddr);
	if (pmuid == PMU_HI_6423)
		ret = sub_pmic_reg_read(regaddr);

	if (op == CLR_BIT) {
		ret &= ~(1<<(bitpos));
	} else {
		ret |= 1<<(bitpos);
	}

	if (pmuid == PMU_HI_6421) {
		pmic_write_reg(regaddr, ret);
		hwlog_info("UWB: %s: pmic_write_reg: 0x%x, value: 0x%x\n", __func__, regaddr, ret);

		value = pmic_read_reg(regaddr);
		hwlog_info("UWB: %s: pmic_read_reg: 0x%x, value: 0x%x\n", __func__, regaddr, value);
		return UWB_SUCC;
	}
	if (pmuid == PMU_HI_6423) {
		sub_pmic_reg_write(regaddr, ret);
		hwlog_info("UWB: %s: sub_pmic_reg_write: 0x%x, value: 0x%x\n", __func__, regaddr, ret);

		value = sub_pmic_reg_read(regaddr);
		hwlog_info("UWB: %s: sub_pmic_reg_read: 0x%x, value: 0x%x\n", __func__, regaddr, value);
		return UWB_SUCC;
	}
	hwlog_err("UWB: %s: no match pmuId\n", __func__);

	return UWB_FAIL;
}

static int uwb_pmureg_operate_by_byte(int pmuid, int regaddr, int byte)
{
	hwlog_info("UWB: %s, in\n", __func__);
	int ret = UWB_FAIL;
	int value = 0;
	if (regaddr <= 0 || byte < 0) {
		hwlog_err("UWB: %s: reg_addr[%x], op[%d], error!\n", __func__, regaddr, byte);
		return UWB_FAIL;
	}
	if (pmuid == PMU_HI_6421) {
		pmic_write_reg(regaddr, byte);
		hwlog_info("UWB: %s: pmic_write_reg: 0x%x, value: 0x%x\n", __func__, regaddr, byte);

		value = pmic_read_reg(regaddr);
		hwlog_info("UWB: %s: pmic_read_reg: 0x%x, value: 0x%x\n", __func__, regaddr, value);
		return UWB_SUCC;
	}
	if (pmuid == PMU_HI_6423) {
		sub_pmic_reg_write(regaddr, byte);
		hwlog_info("UWB: %s: sub_pmic_reg_write: 0x%x, value: 0x%x\n", __func__, regaddr, byte);

		value = sub_pmic_reg_read(regaddr);
		hwlog_info("UWB: %s: sub_pmic_reg_read: 0x%x, value: 0x%x\n", __func__, regaddr, value);
		return UWB_SUCC;
	}
	hwlog_err("UWB: %s: no match pmuid\n", __func__);

	return UWB_FAIL;
}

void uwb_hardware_reset(struct wszj_data *wszj)
{
	gpio_set_value(wszj->reset_gpio, RESET_ON);
	msleep(RESET_DELAY);
	gpio_set_value(wszj->reset_gpio, RESET_OFF);
	msleep(POWER_DELAY);
	return ;
}

static int uwb_ldo_enable(uwb_power_supply uwbldo)
{
	int status = UWB_FAIL;
	/* 2v85 adjust */
	status = regulator_set_voltage(uwbldo.ldo_reg54, 2850000, 2850000);
	if (status) {
		hwlog_err("UWB:%s, 2v85 poweradj fail, error: 0x%x\n", __func__, status);
		return UWB_FAIL;
	}
	/* 1v2 adjust */
	status = regulator_set_voltage(uwbldo.ldo_reg25, 1200000, 1200000);
	if (status) {
		hwlog_err("UWB:%s, 1v2 poweradj fail, error: 0x%x\n", __func__, status);
		return UWB_FAIL;
	}
	/* 1v8 adjust */
	status = regulator_set_voltage(uwbldo.ldo_reg3, 1800000, 1800000);
	if (status) {
		hwlog_err("UWB:%s, rf 1v8 poweradj fail, error: 0x%x\n", __func__, status);
		return UWB_FAIL;
	}
	/* 1v2 power on */
	status = regulator_enable(uwbldo.ldo_reg25);
	if (status) {
		hwlog_err("UWB:%s, 2v85 poweron fail, error: 0x%x\n", __func__, status);
		return UWB_FAIL;
	}
	/* 2v85 power on */
	status = regulator_enable(uwbldo.ldo_reg54);
	if (status) {
		hwlog_err("UWB:%s, 2v85 poweron fail, error: 0x%x\n", __func__, status);
		return UWB_FAIL;
	}
	status = regulator_enable(uwbldo.ldo_reg53);
	if (status) {
		hwlog_err("UWB:%s, ldo53 poweron fail, error: 0x%x\n", __func__, status);
		return UWB_FAIL;
	}
	/* 1v8 power on */
	status = regulator_enable(uwbldo.ldo_reg3);
	if (status) {
		hwlog_err("UWB:%s, rf 1v8 poweron fail, error: 0x%x\n", __func__, status);
		return UWB_FAIL;
	}
	msleep(DELAY_AFTER_1V2);
	return UWB_SUCC;
}

static int uwb_ldo_disable(uwb_power_supply uwbldo)
{
	int status = UWB_FAIL;
	status = regulator_disable(uwbldo.ldo_reg25);
	if (status) {
		hwlog_err("UWB:%s, ldo25 poweroff fail, error: 0x%x\n", __func__, status);
		return UWB_FAIL;
	}
	msleep(DELAY_POWER_OFF_AFTER_1V2);
	status = regulator_disable(uwbldo.ldo_reg54);
	if (status) {
		hwlog_err("UWB:%s, ldo54 poweroff fail, error: 0x%x\n", __func__, status);
		return UWB_FAIL;
	}
	status = regulator_disable(uwbldo.ldo_reg53);
	if (status) {
		hwlog_err("UWB:%s, ldo53 poweroff fail, error: 0x%x\n", __func__, status);
		return UWB_FAIL;
	}
	status = regulator_disable(uwbldo.ldo_reg3);
	if (status) {
		hwlog_err("UWB:%s, ldo3 poweroff fail, error: 0x%x\n", __func__, status);
		return UWB_FAIL;
	}
	return UWB_SUCC;
}

static int uwb_chip_poweron(struct wszj_data *spidev, uwb_power_supply uwbldo)
{
	hwlog_info("UWB: %s, in\n", __func__);
	int ret = UWB_FAIL;

	if (uwbldo.ldo_reg25 == NULL || uwbldo.ldo_reg3 == NULL || uwbldo.ldo_reg54 == NULL || uwbldo.ldo_reg53 == NULL) {
		hwlog_err("UWB:%s, input error\n", __func__);
		return UWB_FAIL;
	}
	ret = uwb_ldo_enable(uwbldo);
	if (ret != UWB_SUCC) {
		hwlog_err("UWB:%s, uwb_ldo_enable fail, error: 0x%x\n", __func__, ret);
		return UWB_FAIL;
	}

	/* 2v85 power on */
	if (uwbldo.gpio_enable2v85 == 1) {
		gpio_direction_output(spidev->power_gpio, SET_BIT);
	} else {
		hwlog_err("UWB:%s, ldo54 not need gpio\n", __func__);
	}
	msleep(DELAY_AFTER_2V85);
	uwb_hardware_reset(spidev);
	return UWB_SUCC;
}

static int uwb_chip_poweroff(struct wszj_data *spidev, uwb_power_supply uwbldo)
{
	hwlog_info("UWB: %s, in\n", __func__);
	int ret = UWB_FAIL;

	if (uwbldo.ldo_reg25 == NULL || uwbldo.ldo_reg3 == NULL) {
		hwlog_err("UWB:%s, input error\n", __func__);
		return UWB_FAIL;
	}
	ret = uwb_ldo_disable(uwbldo);
	if (ret != UWB_SUCC) {
		hwlog_err("UWB:%s, UwbLdoDisable fail, error: 0x%x\n", __func__, ret);
		return UWB_FAIL;
	}
	if (uwbldo.gpio_enable2v85 == 1) {
		gpio_direction_output(spidev->power_gpio, CLR_BIT); // 2v85 power off
	} else {
		hwlog_err("UWB:%s, ldo54 not need gpio\n", __func__);
	}
	return UWB_SUCC;
}

int uwb_sim_levelshift_init(int sim_id)
{
	hwlog_info("UWB: %s, in\n", __func__);
	int ret = UWB_FAIL;
	switch (sim_id) {
	case PMU_SIM_0:
		ret = uwb_pmureg_operate_by_byte(PMU_HI_6423, g_uwb_sim0_out_by_hi6423.addr, g_uwb_sim0_out_by_hi6423.value);
		if (ret != UWB_SUCC) {
			hwlog_err("UWB: %s: Set Direction fail, error: 0x%x\n", __func__, ret);
			return UWB_FAIL;
		}
		ret = uwb_pmureg_operate_by_byte(PMU_HI_6423, g_uwb_sim0_enb_by_hi6423.addr, g_uwb_sim0_enb_by_hi6423.value);
		if (ret != UWB_SUCC) {
			hwlog_err("UWB: %s: SIM0 enable fail, error: 0x%x\n", __func__, ret);
			return UWB_FAIL;
		}
		break;
	case PMU_SIM_1:
		ret = uwb_pmureg_operate_by_byte(PMU_HI_6423, g_uwb_sim1_out_by_hi6423.addr, g_uwb_sim1_out_by_hi6423.value);
		if (ret != UWB_SUCC) {
			hwlog_err("UWB: %s: Set Direction fail, error: 0x%x\n", __func__, ret);
			return UWB_FAIL;
		}
		ret = uwb_pmureg_operate_by_byte(PMU_HI_6423, g_uwb_sim1_enb_by_hi6423.addr, g_uwb_sim1_enb_by_hi6423.value);
		if (ret != UWB_SUCC) {
			hwlog_err("UWB: %s: SIM0 enable fail, error: 0x%x\n", __func__, ret);
			return UWB_FAIL;
		}
		break;
	default:
		hwlog_err("UWB: %s:not match sim_id, sim_id: 0x%x\n", __func__, sim_id);
		break;
	}
	return UWB_SUCC;
}

int uwb_extra_clk_enable(uwb_power_supply uwbldo)
{
	hwlog_info("UWB: %s, in\n", __func__);
	int ret = UWB_FAIL;
	int value;

	if (uwbldo.clk32kb == NULL) {
		hwlog_err("UWB:%s, input error\n", __func__);
		return UWB_FAIL;
	}

	/* enable 32k clk */
	value = pmic_read_reg(0x4a);
	hwlog_info("UWB: %s: value1: 0x%x\n", __func__, value);
	ret = clk_prepare_enable(uwbldo.clk32kb);
	if (ret) {
		hwlog_err("%s: 32K clk enable fail fail, err: %d", ret);
		return UWB_FAIL;
	}
	value = pmic_read_reg(0x4a);
	hwlog_info("UWB: %s: value2: 0x%x\n", __func__, value);

	/* enable 38m clk */
	if (uwbldo.freq2div38m == 1) {
		ret = uwb_pmureg_operate_by_byte(PMU_HI_6421, g_uwb_38m_clk_freq_by_hi6421.addr, g_uwb_38m_clk_freq_by_hi6421.value);
		hwlog_info("UWB:%s, 78.6M need 2 div\n", __func__);
	}
	ret = uwb_pmureg_operate_by_byte(PMU_HI_6421, g_uwb_38m_clk_vpp_adj_by_hi6421.addr, g_uwb_38m_clk_vpp_adj_by_hi6421.value);
	if (ret != UWB_SUCC) {
		hwlog_err("UWB: %s, 38m clk adjust Vpp = 1V, error: 0x%x\n", __func__, ret);
		return UWB_FAIL;
	}
	ret = uwb_pmureg_operate_by_byte(PMU_HI_6421, g_uwb_38m_clk_enb_by_hi6421.addr, SET_BIT);
	if (ret != UWB_SUCC) {
		hwlog_err("UWB: %s, 38m clk enable fail, error: 0x%x\n", __func__, ret);
		return UWB_FAIL;
	}
	return UWB_SUCC;
}

int uwb_extra_clk_disable(uwb_power_supply uwbldo)
{
	hwlog_info("UWB: %s, in\n", __func__);
	int ret = UWB_FAIL;

	if (uwbldo.clk32kb == NULL) {
		hwlog_err("UWB:%s, input error\n", __func__);
		return UWB_FAIL;
	}
	ret = uwb_pmureg_operate_by_byte(PMU_HI_6421, g_uwb_38m_clk_enb_by_hi6421.addr, CLR_BIT);
	if (ret != UWB_SUCC) {
		hwlog_err("UWB: %s, 38m disable fail, error: 0x%x\n", __func__, ret);
		return UWB_FAIL;
	}
	clk_disable_unprepare(uwbldo.clk32kb);
	return UWB_SUCC;
}

static void uwb_gpio_deinit(struct wszj_data *spidev)
{
	gpio_set_value(spidev->reset_gpio, UWB_GPIO_LOW);
	gpio_set_value(spidev->wakeup_gpio, UWB_GPIO_LOW);
}

int uwb_open_init(struct wszj_data *spidev, uwb_power_supply uwbldo)
{
	hwlog_info("UWB: %s, in\n", __func__);
	int ret = UWB_FAIL;
	if (spidev == NULL) {
		hwlog_err("UWB: %s, uwb_open_init spidev NULL\n", __func__);
		return ret;
	}
	ret = uwb_extra_clk_enable(uwbldo);
	if (ret == UWB_FAIL) {
	    hwlog_err("UWB: %s, uwb_extra_clk_enable fail\n", __func__);
	    return ret;
	}
	ret = uwb_chip_poweron(spidev, uwbldo);
	if (ret == UWB_FAIL) {
	    hwlog_err("UWB: %s, uwb_chip_poweron fail\n", __func__);
	    return ret;
	}
	return UWB_SUCC;
}

int uwb_close_deinit(struct wszj_data *spidev, uwb_power_supply uwbldo)
{
	hwlog_info("UWB: %s, in\n", __func__);
	int ret = UWB_FAIL;
	if (spidev == NULL) {
		hwlog_err("UWB: %s, uwb_close_deinit spidev NULL\n", __func__);
		return ret;
	}
	uwb_gpio_deinit(spidev);
	msleep(DELAY_POWER_OFF_AFTER_RST);
	ret = uwb_chip_poweroff(spidev, uwbldo);
	if (ret == UWB_FAIL) {
		hwlog_err("UWB: %s, uwb_chip_poweroff fail\n", __func__);
		return ret;
	}
	ret = uwb_extra_clk_disable(uwbldo);
	if (ret == UWB_FAIL) {
		hwlog_err("UWB: %s, uwb_extra_clk_disable fail\n", __func__);
		return ret;
	}
	return UWB_SUCC;
}

