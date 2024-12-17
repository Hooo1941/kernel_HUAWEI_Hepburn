/*
 * linear_hapitcs.c
 *
 * pmu linear haptics driver
 *
 * Copyright (c) 2023 Huawei Technologies Co., Ltd.
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
#include "huawei_platform/vibrator/pmu_linear_haptics.h"
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/platform_drivers/lpcpu_idle_sleep.h>
#include <linux/kthread.h>
#include <platform_include/basicplatform/linux/spmi_platform.h>
#include <platform_include/basicplatform/linux/mfd/pmic_platform.h>
#include <platform_include/basicplatform/linux/of_platform_spmi.h>

#ifdef CONFIG_BOOST_5V
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include <huawei_platform/sensor/hw_comm_pmic.h>
#endif
#include <huawei_platform/power/wireless/wireless_transmitter.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_status.h>

#define PMU_LINEAR_HAPTIC_CLK_GATE         0x4E
#define PMU_LINEAR_HAPTIC_LRA_ANA_CFG2     0x108F
#define PMU_LINEAR_HAPTIC_LRA_ANA_VALUE    0x0

#define PMU_LINEAR_HAPTIC_BRAKE_TIME_OUT   100
#define PMU_LINEAR_HAPTIC_STATUS           0x104F
#define PMU_LINEAR_HAPTIC_ON               0x1000
#define PMU_LINEAR_HAPTIC_POWER_ON         1
#define PMU_LINEAR_HAPTIC_POWER_OFF        0
#define PMU_BOOST_POWER_ON_DELAY_TIME      2
#define PMU_BOOST_POWER_OFF_DELAY_TIME     5

#define LINEAR_VIBRATOR_SPEL_DUTY_BASE_REG           0x1064
#define LINEAR_VIBRATOR_SPLICE_CFG_LADD_REG_NUM      20
#define LINEAR_VIBRATOR_SPLICE_SPEL_TIME_REG_NUM     20
#define LINEAR_VIBRATOR_SPLICE_CFG_DUTY_REG_NUM      41
#define LINEAR_VIBRATOR_RESET_CFG_REG                0x0143
#define LINEAR_VIBRATOR_RESET_VAL                    0xAC
#define LINEAR_VIBRATOR_RESTORE_VAL                  0x5A
#define LINEAR_VIBRATOR_LADD_PARA_REG                0x102B
#define LINEAR_VIBRATOR_SPEL_TIME_REG0               0x1050
#define HAPTIC_REPEAT_US_TIME                        42480
#define LINEAR_VIBRATOR_SPLICE_CFG_PARA_REG_NUM      81
#define LINEAR_VIBRATOR_MODE_HAPTICS                 0x02

struct linear_vibrator_splice_haptics_lib {
	u32 ladd_para[LINEAR_VIBRATOR_SPLICE_CFG_LADD_REG_NUM];
	u32 spel_para[LINEAR_VIBRATOR_SPLICE_SPEL_TIME_REG_NUM];
	u32 duty_para[LINEAR_VIBRATOR_SPLICE_CFG_DUTY_REG_NUM];
};

static struct task_struct *splice_haptic_thread;
static bool enable_short_splice_long_haptic = false;
static struct linear_vibrator_splice_haptics_lib pmu_linear_splice_cfg;
static u32 haptic_state;

static void pmu_linear_haptic_write_u8(
	u8 vibrator_set, const u32 vibrator_address)
{
#ifdef CONFIG_PMIC_21V900_PMU
	sub_pmic_reg_write(vibrator_address, vibrator_set);
#endif
}

static u8 pmu_linear_haptic_read_u8(const u32 vibrator_address)
{
#ifdef CONFIG_PMIC_21V900_PMU
	return sub_pmic_reg_read(vibrator_address);
#else
	return 0;
#endif
}

static void pmu_linear_haptic_set_clock(int pmic_vibrator_clk_onoff)
{
#ifdef CONFIG_PMIC_21V900_PMU
	pmic_write_reg(PMU_LINEAR_HAPTIC_CLK_GATE, pmic_vibrator_clk_onoff);
	if (pmic_vibrator_clk_onoff)
		sub_pmic_reg_write(PMU_LINEAR_HAPTIC_LRA_ANA_CFG2,
			PMU_LINEAR_HAPTIC_LRA_ANA_VALUE);
#endif
}

static int pmu_linear_haptic_power_on_off(int pmic_vibrator_power_onoff)
{
	int retry = PMU_LINEAR_HAPTIC_BRAKE_TIME_OUT;

	if (pmic_vibrator_power_onoff) {
#ifdef CONFIG_BOOST_5V
		boost_5v_enable(BOOST_5V_ENABLE, BOOST_CTRL_MOTOER);
#endif
		mdelay(PMU_BOOST_POWER_ON_DELAY_TIME);
		pr_info("pmu_linear pmu boost 5v on\n");
	} else {
		while (pmu_linear_haptic_read_u8(PMU_LINEAR_HAPTIC_STATUS) && retry) {
			mdelay(1);
			retry--;
		}
		pr_info("pmu_linear vibrator status is 0x%x,retry is %d\n",
			pmu_linear_haptic_read_u8(PMU_LINEAR_HAPTIC_STATUS), retry);
		pmu_linear_haptic_set_clock(pmic_vibrator_power_onoff);
#ifdef CONFIG_BOOST_5V
		boost_5v_enable(BOOST_5V_DISABLE, BOOST_CTRL_MOTOER);
#endif
		mdelay(PMU_BOOST_POWER_OFF_DELAY_TIME);
		pr_info("pmu_linear pmu boost 5v off\n");
	}
	return 0;
}

static void pmu_linear_haptic_set_mode(u8 mode)
{
	pmu_linear_haptic_write_u8(mode, PMU_LINEAR_HAPTIC_ON);
}

static void pmu_linear_haptic_play_time_effect(void)
{
	u8 i;
	u32 cfg_reg;
	u32 spel_reg;
	u32 duty_reg;

	for (i = 0; i < LINEAR_VIBRATOR_SPLICE_CFG_LADD_REG_NUM; i++) {
		cfg_reg = LINEAR_VIBRATOR_LADD_PARA_REG + i;
		spel_reg = LINEAR_VIBRATOR_SPEL_TIME_REG0 + i;
		/* configs time register and duty registers */
		pmu_linear_haptic_write_u8(pmu_linear_splice_cfg.ladd_para[i], cfg_reg);
		pmu_linear_haptic_write_u8(pmu_linear_splice_cfg.spel_para[i], spel_reg);
	}

	for (i = 0; i < LINEAR_VIBRATOR_SPLICE_CFG_DUTY_REG_NUM; i++) {
		duty_reg = LINEAR_VIBRATOR_SPEL_DUTY_BASE_REG + i;
		/* configs duty registers */
		pmu_linear_haptic_write_u8(pmu_linear_splice_cfg.duty_para[i], duty_reg);
	}

	pr_info("pmu_linear %s\n", __func__);
}

void pmu_linear_haptic_play_splice_haptic_effect(unsigned              int state)
{
	if (!enable_short_splice_long_haptic)
		return;

	haptic_state = state;
	if (state) {
		pmu_linear_haptic_power_on_off(PMU_LINEAR_HAPTIC_POWER_ON);
#ifdef CONFIG_PMIC_21V900_PMU
		pmu_linear_haptic_set_clock(PMU_LINEAR_HAPTIC_POWER_ON);
		pmu_linear_haptic_play_time_effect();
#endif
		lpcpu_idle_sleep_vote(ID_MOTOR_LRA, 1);
		if (splice_haptic_thread) {
			wake_up_process(splice_haptic_thread);
			pr_info("pmu_linear wake up haptic effect thread\n");
		}
	} else {
		pmu_linear_haptic_write_u8(LINEAR_VIBRATOR_RESET_VAL,
			LINEAR_VIBRATOR_RESET_CFG_REG);
		pmu_linear_haptic_power_on_off(PMU_LINEAR_HAPTIC_POWER_OFF);
		pmu_linear_haptic_write_u8(LINEAR_VIBRATOR_RESTORE_VAL,
			LINEAR_VIBRATOR_RESET_CFG_REG);
		lpcpu_idle_sleep_vote(ID_MOTOR_LRA, 0);
	}
}

bool pmu_linear_haptic_check_splice_haptic(unsigned            int state)
{
	static bool is_wireless_opened = false;
	int wireless_channel_state;
	bool wireless_open_flag;

	if (!enable_short_splice_long_haptic)
		return false;

	if (state) {
		wireless_open_flag = wireless_tx_get_tx_open_flag();
		wireless_channel_state = wlrx_get_wireless_channel_state();
		pr_info("pmu_linear wireless open flag = %u, channel state %d",
			wireless_open_flag, wireless_channel_state);
		is_wireless_opened = (wireless_channel_state ==
			WIRELESS_CHANNEL_ON) || wireless_open_flag;
	}

	return is_wireless_opened;
}

static int linear_vibrator_splice_haptic_function(void *data)
{
	int repeat_log_limit = 0;

	(void)data;
	while (!kthread_should_stop()) {
		if (haptic_state) {
			if (repeat_log_limit == 0) {
				pr_info("pmu_linear repeat splice haptic work\n");
				repeat_log_limit = -1;
			}
			pmu_linear_haptic_set_mode(LINEAR_VIBRATOR_MODE_HAPTICS);
			usleep_accurate(HAPTIC_REPEAT_US_TIME);
		} else {
			repeat_log_limit = 0;
			pr_info("pmu_linear stop splice work, set wait\n");
			set_current_state(TASK_INTERRUPTIBLE);
			schedule();
		}
	}
	return 0;
}

static int pmu_linear_haptics_thread_init(void)
{
	splice_haptic_thread = kthread_create(linear_vibrator_splice_haptic_function,
		NULL, "splice_haptic-thread");
	if (IS_ERR(splice_haptic_thread)) {
		pr_err("pmu_linear haptic_thread init failed\n");
		return -EFAULT;
	}
	sched_set_fifo(splice_haptic_thread);
	return 0;
}

static int pmu_linear_haptics_splice_haptics_cfg(struct device_node *dn)
{
	const __be32 *mux = NULL;
	int ret;
	unsigned int size;

	mux = of_get_property(dn, "splice-haptics-cfg", &size);
	if (mux == NULL) {
		pr_info("pmu_linear could not support splice haptic lib\n");
		return -EINVAL;
	}

	if (size != (sizeof(*mux) * LINEAR_VIBRATOR_SPLICE_CFG_PARA_REG_NUM)) {
		pr_err("pmu_linear splice haptic lib data is bad\n");
		return -EINVAL;
	}

	pr_info("pmu_linear num of splice elements is %u\n", size);

	ret = of_property_read_u32_array(
		dn, "splice-haptics-cfg", (u32 *)&pmu_linear_splice_cfg,
		LINEAR_VIBRATOR_SPLICE_CFG_PARA_REG_NUM);
	if (ret) {
		pr_err("pmu_linear could not read splice-haptics-cfg table\n");
		return ret;
	}

	return 0;
}

static int __init pmu_linear_haptics_init(void)
{
	struct device_node *pmu_haptics = NULL;
	bool short_splice_long_haptic_flag;
	int ret;

	pmu_haptics = of_find_compatible_node(NULL, NULL, "hisilicon,pmic-vibrator");
	if (!pmu_haptics) {
		pr_info("pmu_linear %s, can't find node pmic-vibrator\n", __func__);
		return -1;
	}

	short_splice_long_haptic_flag =
		of_property_read_bool(pmu_haptics, "enable_short_splice_long_haptic");
	pr_info("pmu_linear enable_short_splice_long_haptic status is %u\n", short_splice_long_haptic_flag);

	if (!short_splice_long_haptic_flag)
		return 0;

	ret = pmu_linear_haptics_splice_haptics_cfg(pmu_haptics);
	if (ret) {
		pr_info("pmu_linear cannt get splice cfg, do not support splice haptics\n");
		return -EINVAL;
	}
	ret = pmu_linear_haptics_thread_init();
	if (ret) {
		pr_err("pmu_linear pmu linear thread init fail\n");
		return -EINVAL;
	}

	enable_short_splice_long_haptic = true;
	return 0;
}

static void __exit pmu_linear_vibrator_exit(void)
{
	enable_short_splice_long_haptic = false;
}

module_init(pmu_linear_haptics_init);
module_exit(pmu_linear_vibrator_exit);

MODULE_DESCRIPTION("Linear Vibrator driver");
MODULE_LICENSE("GPL");

