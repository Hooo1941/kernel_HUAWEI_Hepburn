// SPDX-License-Identifier: GPL-2.0
/*
 * hc32l110_state_machine.c
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

#include "hc32l110.h"
#include "hc32l110_fw.h"
#include "hc32l110_state_machine.h"
#include "../cam_sensormotor_i2c.h"
#include "../cam_star_module_adapter.h"
#include "cam_log.h"
#include <chipset_common/hwpower/hardware_ic/buck_boost.h>
#include <chipset_common/log/hw_log.h>
#include <linux/string.h>
#include <linux/firmware.h>
#include <linux/delay.h>
#include <securec.h>

#define HWLOG_TAG cam_star_hc32l110_state_machine
HWLOG_REGIST();

static struct power_state_machine_t power_state_machine;

static int hc32l110_enable_irq(struct cam_star_module_device_info *di)
{
	int rc = 0;
	if (!di)
		return -EINVAL;

	if (di->int_state == INT_DISABLE) {
		enable_irq(di->irq);
		di->int_state = INT_ENABLE;
		hwlog_info("%s\n", __func__);
	}
	return rc;
}

static int hc32l110_disable_irq(struct cam_star_module_device_info *di)
{
	int rc = 0;
	if (!di)
		return -EINVAL;

	if (di->int_state == INT_ENABLE) {
		disable_irq(di->irq);
		di->int_state = INT_DISABLE;
		hwlog_info("%s\n", __func__);
	}
	return rc;
}

static int hc32l110_wakeup_mcu(struct cam_star_module_device_info *di)
{
	int rc = 0;
	rc = hc32l110_enable_irq(di);
	return rc;
}

static int hc32l110_sleep_mcu(struct cam_star_module_device_info *di)
{
	int rc = 0;
	rc = hc32l110_disable_irq(di);
	return rc;
}

static int hc32l110_power_sequence1(struct cam_star_module_device_info *di)
{
	int rc = 0;
	bool boost_status = 0;
	if (!di)
		return -EINVAL;

	hwlog_info("enter %s\n", __func__);
	gpio_direction_output(di->power_setting.mcu_reset_gpio, LOW);
	cam_usleep(10000);
	gpio_direction_output(di->power_setting.motor_en_gpio, LOW);
	buck_boost_set_vout(0, BBST_USER_BOOST_HX);
	boost_status = buck_boost_set_enable(LOW, BBST_USER_BOOST_HX);
	hwlog_info("%s: buck_boost_set_enable low status=%d\r\n", __func__, (int)boost_status);
	if (!boost_status)
		hwlog_err("%s: buck_boost_set_enable fail\r\n", __func__);
	return rc;
}

static int hc32l110_power_sequence2(struct cam_star_module_device_info *di)
{
	int rc = 0;
	bool boost_status = 0;
	u8 val = 0xFF;
	int retry = 0;
	if (!di)
		return -EINVAL;

	hwlog_info("enter %s\n", __func__);
	gpio_direction_output(di->power_setting.motor_en_gpio, HIGH);
	gpio_direction_output(di->power_setting.mcu_reset_gpio, LOW);
	cam_usleep(20000);
	gpio_direction_output(di->power_setting.mcu_int_gpio, LOW);
	gpio_direction_output(di->power_setting.mcu_reset_gpio, HIGH);
	hc32l110_disable_irq(di);
	gpio_direction_output(di->power_setting.mcu_int_gpio, LOW);
	gpio_direction_output(di->power_setting.mcu_reset_gpio, HIGH);
	boost_status = buck_boost_set_enable(HIGH, BBST_USER_BOOST_HX);
	hwlog_info("%s: buck_boost_set_enable high status=%d\r\n", __func__, (int)boost_status);
	if (!boost_status)
		hwlog_err("%s: buck_boost_set_enable fail\r\n", __func__);
	buck_boost_set_vout(BST_VOL_6V, BBST_USER_BOOST_HX);
	cam_usleep(30000);
	gpio_direction_input(di->power_setting.mcu_int_gpio);
	return rc;
}

static int hc32l110_power_sequence3(struct cam_star_module_device_info *di)
{
	int rc = 0;
	bool boost_status = 0;
	u8 val = 0xFF;
	int retry = 0;
	if (!di)
		return -EINVAL;

	hwlog_info("enter %s\n", __func__);
	rc = hc32l110_fw_write_cmd(di, HC32L110_FW_GO_CMD);
	rc += hc32l110_fw_write_addr(di, HC32L110_FW_WRITE_ADDR);

	cam_usleep(5000);
	do {
		i2c_u8_read_byte(di->client, HC32L110_STAR_CSTATE, &val);
		cam_usleep(10000);
		hwlog_info("%s retry times: %d, 0x13: 0x%x\n", __func__, retry, val);
		retry++;
	} while ((retry <= 10) && (val != 0));

	if (retry >= 10) {
		hwlog_err("%s fail, CPU is busy!\n", __func__);
		return -EINVAL;
	}

	rc = hc32l110_mcu_fw_reg_print(di);
	if (rc)
		hwlog_err("hc32l110_mcu_fw_reg_print failed.\n");

	hc32l110_sleep_mcu(di);
	gpio_direction_output(di->power_setting.mcu_reset_gpio, LOW);
	gpio_direction_output(di->power_setting.motor_en_gpio, LOW);
	buck_boost_set_vout(0, BBST_USER_BOOST_HX);
	boost_status = buck_boost_set_enable(LOW, BBST_USER_BOOST_HX);
	hwlog_info("%s: buck_boost_set_enable low status=%d\r\n", __func__, (int)boost_status);
	if (!boost_status)
		hwlog_err("%s: buck_boost_set_enable fail\r\n", __func__);
	cam_usleep(10000);
	return rc;
}

static int hc32l110_power_sequence4(struct cam_star_module_device_info *di)
{
	int rc = 0;
	bool boost_status = 0;
	if (!di)
		return -EINVAL;

	hwlog_info("enter %s\n", __func__);
	gpio_direction_output(di->power_setting.motor_en_gpio, HIGH);
	gpio_direction_output(di->power_setting.mcu_reset_gpio, HIGH);
	boost_status = buck_boost_set_enable(HIGH, BBST_USER_BOOST_HX);
	hwlog_info("%s: buck_boost_set_enable high status=%d\r\n", __func__, (int)boost_status);
	if (!boost_status)
		hwlog_err("%s: buck_boost_set_enable fail\r\n", __func__);
	buck_boost_set_vout(BST_VOL_6V, BBST_USER_BOOST_HX);
	msleep(30);
	hc32l110_wakeup_mcu(di);

	return rc;
}

static int hc32l110_power_sequence5(struct cam_star_module_device_info *di)
{
	int rc = 0;
	bool boost_status = 0;
	u8 val = 0xFF;
	int retry = 0;
	if (!di)
		return -EINVAL;

	hwlog_info("enter %s\n", __func__);
	do {
		i2c_u8_read_byte(di->client, HC32L110_STAR_CSTATE, &val);
		hwlog_info("%s retry times: %d, 0x13: 0x%x\n", __func__, retry, val);
		if (val == 0)
			break;
		msleep(100);
		retry++;
	} while ((retry <= I2C_MAX_WAIT_RETRY));

	if (retry >= I2C_MAX_WAIT_RETRY) {
		hwlog_err("%s fail, CPU is busy\n", __func__);
		rc = -EINVAL;
	}

	hc32l110_sleep_mcu(di);
	gpio_direction_output(di->power_setting.mcu_reset_gpio, LOW);
	gpio_direction_output(di->power_setting.motor_en_gpio, LOW);
	buck_boost_set_vout(0, BBST_USER_BOOST_HX);
	boost_status = buck_boost_set_enable(LOW, BBST_USER_BOOST_HX);
	hwlog_info("%s: buck_boost_set_enable low status=%d\r\n", __func__, (int)boost_status);
	if (!boost_status)
		hwlog_err("%s: buck_boost_set_enable fail\r\n", __func__);
	cam_usleep(10000);
	return rc;
}

static int hc32l110_power_sequence6(struct cam_star_module_device_info *di)
{
	int rc = 0;
	u8 val = 0;
	int retry = 0;
	if (!di)
		return -EINVAL;

	hwlog_info("enter %s\n", __func__);
	rc = hc32l110_fw_write_cmd(di, HC32L110_FW_GO_CMD);
	rc += hc32l110_fw_write_addr(di, HC32L110_FW_WRITE_ADDR);
	hc32l110_enable_irq(di);
	cam_usleep(5000);
	return rc;
}

struct power_state_transformer_t star_power_state_transformer[] = {
	{ PROBE, TURN_TO_BOOTLOADER, BOOTLOADER, hc32l110_power_sequence2 }, /* transformer 1，开机自动download fw时切换到BootLoader */
	{ BOOTLOADER, EXT_MATCH_BOOTLOADER_ID, PROBE, hc32l110_power_sequence1 }, /* transformer 2 */
	{ BOOTLOADER, DOWNLOAD_FW, POWER_OFF, hc32l110_power_sequence3}, /* transformer 3 */
	{ POWER_OFF, TURN_TO_BOOTLOADER, BOOTLOADER, hc32l110_power_sequence2 }, /* transformer 4, 开机之后切换到BootLoader */
	{ POWER_ON, RETRACTION, POWER_OFF, hc32l110_power_sequence5 }, /* transformer 5 */
	{ POWER_OFF, EJECT, POWER_ON, hc32l110_power_sequence4 }, /* transformer 6 */
	{ POWER_ON, TURN_TO_BOOTLOADER, BOOTLOADER, hc32l110_power_sequence2 }, /* transformer 7， 开着camera切换到BootLoader */
	{ BOOTLOADER, EJECT, POWER_ON, hc32l110_power_sequence6 }, /* transformer 8 */
	{ POWER_OFF, RELOAD_EEPROM_TO_FLASH, POWER_ON, hc32l110_power_sequence4 }, /* transformer 9, reload eeprom to flash after download fw */
};

/* find transformer in state machine */
struct power_state_transformer_t *find_trans(enum work_state_t work_state)
{
	int i = 0;
	for (i = 0; i < power_state_machine.trans_num; i++) {
		if ((power_state_machine.power_state_transformer[i].current_state == power_state_machine.power_state)
			&& (power_state_machine.power_state_transformer[i].event_id == work_state))
			return &power_state_machine.power_state_transformer[i];
	}
	return NULL;
}

int hc32l110_power_state_machine(struct cam_star_module_device_info *di, enum work_state_t work_state)
{
	int rc = 0;
	struct power_state_transformer_t *power_state_transformer;
	struct power_operation_t power_operation;
	if (!di)
		return -EINVAL;

	power_state_transformer = find_trans(work_state);
	if (!power_state_transformer) {
		hwlog_err("power_state_machine current state: %d can not match any transformer!\n", power_state_machine.power_state);
		return -EINVAL;
	}

	/* update power_state_machine */
	power_state_machine.power_state = power_state_transformer->next_state;
	hwlog_info("%s now in state: %u", __func__, power_state_machine.power_state);
	power_operation = power_state_transformer->power_operation;
	if (!power_operation.action) {
		hwlog_err("power_state_transformer change to state: %d, action is NULL!\n", power_state_machine.power_state);
		return -EINVAL;
	}
	rc = power_operation.action(di);

	return rc;
}

int hc32l110_get_power_state(void)
{
	return power_state_machine.power_state;
}

int hc32l110_power_state_machine_init(void)
{
	int rc = 0;
	power_state_machine.power_state = PROBE;
	power_state_machine.power_state_transformer = star_power_state_transformer;
	power_state_machine.trans_num = sizeof(star_power_state_transformer) / sizeof(star_power_state_transformer[0]);
	return rc;
}