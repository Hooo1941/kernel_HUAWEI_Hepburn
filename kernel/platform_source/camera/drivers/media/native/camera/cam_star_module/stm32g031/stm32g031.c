// SPDX-License-Identifier: GPL-2.0
/* stm32g031.c
 *
 * stm32g031 driver
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

#include "stm32g031.h"
#include "stm32g031_fw.h"
#include "cam_log.h"
#include "../cam_sensormotor_i2c.h"
#include "../cam_star_module_adapter.h"
#include <securec.h>
#include <linux/string.h>
#include <chipset_common/hwpower/hardware_ic/buck_boost.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <chipset_common/hwmanufac/dev_detect/dev_detect.h>
#endif

#define HWLOG_TAG cam_star_stm32g031
HWLOG_REGIST();

#define STM32G031_FW_VER                           0x01
#define I2C_MAX_WAIT_RETRY                         30
#define CALIB_I2C_MAX_WAIT_RETRY_TIMES                   300
#define CALIB_MCU_READY_MAX_WAIT_RETRY_TIMES             10
#define BST_VOL_6V                                 6000
#define CAM_SENSORMOTOR_HEIGHT_DATA_LEN            20
#define CAM_SENSORMOTOR_CALIB_DATA_MAX_LEN         292 // 2*(2*72 + 2)
#define CAM_SENSORMOTOR_CALIB_ADD_INFO_MAX_LEN     12 // 2*6

static int g_ref_count = 0;
static struct mutex power_lock_mutex;

static int stm32g031_get_info(struct cam_star_module_device_info *di,
	struct sensormotor_info *info)
{
	if (!di)
		return -EINVAL;
	int rc = 0;
	rc = strncpy_s(info->name, HWSENSORMOTOR_DEVICE_NAME_SIZE - 1, di->ic_name, strlen(di->ic_name));
	hwlog_info("ic_name: %s\n", info->name);
	return rc;
}

int stm32g031_power_on(struct cam_star_module_device_info *di)
{
	int rc = 0;
	u8 val = 0;
	int retry = 0;
	bool boost_status = 0;

	if (!di)
		return -EINVAL;
	if (di->power_state == POWER_ON) {
		hwlog_info("stmg031 already power on.\n");
		return rc;
	}

	rc += gpio_direction_output(di->power_setting.mcu_reset_gpio, HIGH);
	rc += gpio_direction_output(di->power_setting.motor_en_gpio, HIGH);
	boost_status = buck_boost_set_enable(HIGH, BBST_USER_BOOST_HX);
	hwlog_info("%s: buck_boost_set_enable high status=%d\r\n", __func__, (int)boost_status);
	if (!boost_status)
		hwlog_err("%s: buck_boost_set_enable fail\r\n", __func__);
	buck_boost_set_vout(BST_VOL_6V, BBST_USER_BOOST_HX);

	do {
		msleep(3);
		i2c_u8_read_byte(di->client, STM32G031_STAR_CSTATE, &val);
		hwlog_info("%s retry times: %d\n", __func__, retry);
	} while ((retry++ < I2C_MAX_WAIT_RETRY) && (val & 0x01));

	if (retry >= I2C_MAX_WAIT_RETRY) {
		hwlog_err("%s fail, CPU is busy!\n", __func__);
		return -EINVAL;
	}

	di->power_state = POWER_ON;
	hwlog_info("%s success.\n", __func__);
	return 0;
}

static int stm32g031_power_off(struct cam_star_module_device_info *di, enum power_off_state_t state)
{
	int rc = 0;
	u8 val = 0;
	bool boost_status = 0;
	if (!di)
		return -EINVAL;

	if (di->power_state == POWER_OFF) {
		hwlog_info("stmg031 already power off.\n");
		return rc;
	}

	if (state == POWEROFF_TRY_MCU_STATE) {
		hwlog_info("%s try mcu state first.\n", __func__);
		rc = i2c_u8_read_byte(di->client, STM32G031_STAR_CSTATE, &val);
		if (rc) {
			hwlog_err("%s i2c failed.\n", __func__);
			return rc;
		}
		if (val != 0) {
			hwlog_err("%s MCU is not idle.\n", __func__);
			return -EINVAL;
		}
	} else if (state == POWEROFF_DIR_WITHOUT_TRY) {
		hwlog_info("%s directly without try to wait for MCU ready.\n");
	} else {
		hwlog_err("invalid power_off state.\n");
		return -EINVAL;
	}

	buck_boost_set_vout(0, BBST_USER_BOOST_HX);
	boost_status = buck_boost_set_enable(LOW, BBST_USER_BOOST_HX);
	hwlog_info("%s: buck_boost_set_enable low status=%d\r\n", __func__, (int)boost_status);
	if (!boost_status)
		hwlog_err("%s: buck_boost_set_enable fail\r\n", __func__);
	gpio_direction_output(di->power_setting.motor_en_gpio, LOW);
	gpio_direction_output(di->power_setting.mcu_reset_gpio, LOW);

	di->power_state = POWER_OFF;
	di->normal_calib_state = NORMAL;
	hwlog_info("%s success.\n", __func__);
	return rc;
}

static int stm32g031_power_off_with_retry(struct cam_star_module_device_info *di)
{
	int rc = 0;
	u8 val = 0;
	int retry = 0;
	bool boost_status = 0;
	if (!di)
		return -EINVAL;

	do {
		i2c_u8_read_byte(di->client, STM32G031_STAR_CSTATE, &val);
		msleep(100);
		hwlog_info("%s retry times: %d\n", __func__, retry);
	} while ((retry++ < I2C_MAX_WAIT_RETRY) && (val != 0));

	if (retry >= I2C_MAX_WAIT_RETRY) {
		hwlog_err("%s fail, CPU is busy!\n", __func__);
		return -EINVAL;
	}

	buck_boost_set_vout(0, BBST_USER_BOOST_HX);
	boost_status = buck_boost_set_enable(LOW, BBST_USER_BOOST_HX);
	hwlog_info("%s: buck_boost_set_enable low status=%d\r\n", __func__, (int)boost_status);
	if (!boost_status)
		hwlog_err("%s: buck_boost_set_enable fail\r\n", __func__);
	gpio_direction_output(di->power_setting.motor_en_gpio, LOW);
	gpio_direction_output(di->power_setting.mcu_reset_gpio, LOW);

	di->power_state = POWER_OFF;
	di->normal_calib_state = NORMAL;
	hwlog_info("%s success.\n", __func__);
	return rc;
}

int stm32g031_set_bootloader(struct cam_star_module_device_info *di)
{
	int rc = 0;
	bool boost_status = 0;
	if (!di)
		return -EINVAL;

	hwlog_info("%s\n", __func__);
	if (di->power_state == BOOTLOADER) {
		hwlog_info("already in bootloader state.\n");
		return rc;
	}
	gpio_direction_output(di->power_setting.mcu_reset_gpio, LOW);
	msleep(5);
	gpio_direction_output(di->power_setting.mcu_reset_gpio, HIGH);
	gpio_direction_output(di->power_setting.mcu_boot_gpio, HIGH);
	gpio_direction_output(di->power_setting.motor_en_gpio, HIGH);
	boost_status = buck_boost_set_enable(HIGH, BBST_USER_BOOST_HX);
	hwlog_info("%s: buck_boost_set_enable high status=%d\r\n", __func__, (int)boost_status);
	if (!boost_status)
		hwlog_err("%s: buck_boost_set_enable fail\r\n", __func__);
	buck_boost_set_vout(BST_VOL_6V, BBST_USER_BOOST_HX);
	msleep(5);
	gpio_direction_input(di->power_setting.mcu_boot_gpio);

	di->power_state = BOOTLOADER;
	return rc;
}

static int stm32g031_matchid(struct cam_star_module_device_info *di)
{
	int rc;
	u8 ack = 0;
	u8 data[3] = {0};
	if (!di)
		return -EINVAL;

	/* write ver cmd and wait ack */
	rc = stm32g031_fw_write_cmd(di, STM32G031_FW_MATCHID_CMD);
	if (rc) {
		hwlog_err("stm32g031 not work bootloader mode.\n");
		return -EIO;
	}

	/* get data and wait ack */
	rc = i2c_read_block_without_cmd(di->client, data, 3);
	rc += i2c_read_block_without_cmd(di->client, &ack, 1);
	hwlog_info("ack = 0x%x\n", ack);
	if (ack != STM32G031_FW_ACK_VAL) {
		hwlog_err("bootloader not ack.\n");
		return -EINVAL;
	}
	if (rc) {
		hwlog_err("not work bootloader mode\n");
		return -EIO;
	}

	hwlog_info("stm32g031 id: %u %u %u\n", data[0], data[1], data[2]);
	if (data[0] == 0x01 && data[1] == 0x04 && data[2] == 0x66) {
		hwlog_info("%s success!\n", __func__);
		return 0;
	} else {
		hwlog_err("%s failed\n", __func__);
		return -EINVAL;
	}

	hwlog_info("stm32g031 now working on bootloader mode.\n");
	return 0;
}

static irqreturn_t stm32g031_interrupt(int irq, struct cam_star_module_device_info *di)
{
	return IRQ_HANDLED;
}

static int stm32g031_irq_init(struct cam_star_module_device_info *di)
{
	int rc;
	int gpio = di->power_setting.mcu_int_gpio;
	if (!di)
		return -EINVAL;
	gpio_direction_input(gpio);
	di->irq = gpio_to_irq(gpio);
	if (di->irq < 0) {
		hwlog_err("gpio %d map to irq fail\n", gpio);
		return -EINVAL;
	}
	hwlog_info("stm32g031 irq = %d\n", di->irq);

	rc = request_threaded_irq(di->irq, NULL, (irq_handler_t)stm32g031_interrupt,
			IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "stm32g031", (void *)di);
	if (rc) {
		hwlog_err("gpio irq request fail\n");
		di->irq = -1;
		return rc;
	}
	disable_irq_nosync(di->irq);
	return 0;
}

static int stm32g031_download_firmware(struct cam_star_module_device_info *di)
{
	int rc = 0;
	if (!di)
		return -EINVAL;
	hwlog_info("enter %s\n, fw version which is to update is: %d", __func__, di->fw_state.fw_ver_id);
	if (di->power_state != BOOTLOADER) {
		hwlog_err("not in bootloader state!\n");
		return -EINVAL;
	}
	rc = stm32g031_fw_update_check(di);
	if (rc) {
		hwlog_err("cam_star stm32g031_fw_update_check failed.\n");
		return rc;
	}

	return rc;
}

static int stm32g031_set_code(struct cam_star_module_device_info *di, u16 code)
{
	int rc = 0;
	u8 buf[LEN_U16_DAT] = {0};
	if (!di)
		return -EINVAL;

	buf[0] = code & 0xff;
	buf[1] = code >> 8;
	rc = i2c_u8_write_byte(di->client, STM32G031_MOTOR_TARGET_POS_LOW, buf[0]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc += i2c_u8_write_byte(di->client, STM32G031_MOTOR_TARGET_POS_HIGH, buf[1]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	di->mcu_state.target_pos = code;
	return rc;
}

static int stm32g031_set_speed(struct cam_star_module_device_info *di, u16 speed)
{
	int rc = 0;
	u8 buf[LEN_U16_DAT] = {0};
	if (!di)
		return -EINVAL;

	buf[0] = speed & 0xff;
	buf[1] = speed >> 8;
	rc = i2c_u8_write_byte(di->client, STM32G031_MOTOR_SPEED_LOW, buf[0]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc += i2c_u8_write_byte(di->client, STM32G031_MOTOR_SPEED_HIGH, buf[1]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	di->mcu_state.motor_speed = speed;
	return rc;
}

static int stm32g031_start(struct cam_star_module_device_info *di)
{
	int rc = 0;
	if (!di)
		return -EINVAL;
	if (di->power_state == POWER_OFF) {
		hwlog_err("stm32g031 is not powered on, please power on first.\n");
		return -EINVAL;
	}

	if (di->normal_calib_state != NORMAL) {
		hwlog_err("stm3g031 is not in normal mode!\n");
		return -EINVAL;
	}

	rc = i2c_u8_write_byte(di->client, STM32G031_MOTOR_CTL, START_STEPPING);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	return rc;
}

static int stm32g031_stop(struct cam_star_module_device_info *di)
{
	int rc = 0;
	if (!di)
		return -EINVAL;
	if (di->power_state == POWER_OFF) {
		hwlog_err("stm32g031 is not powered on, please power on first.\n");
		return -EINVAL;
	}

	rc = i2c_u8_write_byte(di->client, STM32G031_MOTOR_CTL, STOP_STEPPING);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	return rc;
}

static int stm32g031_get_real_height(struct cam_star_module_device_info *di, u16 *real_code)
{
	int rc = 0;
	u8 buf[LEN_U16_DAT] = {0};
	if (!di)
		return -EINVAL;
	rc = i2c_u8_read_byte(di->client, STM32G031_CURR_REAL_HEIGHT_LOW, &buf[0]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, STM32G031_CURR_REAL_HEIGHT_HIGH, &buf[1]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	*real_code = buf[0] | (buf[1] << 8);
	return rc;
}

static int stm32g031_set_calib_mode(struct cam_star_module_device_info *di, enum calib_mode_t state)
{
	int rc = 0;
	u8 val = 0;
	int retry = 0;
	u8 buf[LEN_U8_DAT] = {0};
	if (!di)
		return -EINVAL;

	if (di->normal_calib_state == CALIB) {
		hwlog_err("stm32g031 already in calib mode.\n");
		return rc;
	}

	/* set calib mode util STM32G031_MOTOR_CSTATE is idle */
	do {
		i2c_u8_read_byte(di->client, STM32G031_MOTOR_CSTATE, &val);
		msleep(100);
		hwlog_info("%s retry times: %d\n", __func__, retry);
	} while ((retry++ < I2C_MAX_WAIT_RETRY) && (val != 0));

	if (retry >= I2C_MAX_WAIT_RETRY) {
		hwlog_err("%s fail, CPU is busy!\n", __func__);
		return -EINVAL;
	}

	buf[0] = state & 0x01;
	rc = i2c_u8_write_byte(di->client, STM32G031_CALI_CTL, buf[0]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	di->normal_calib_state = state;
	return rc;
}

static int stm32g031_calib_set_step(struct cam_star_module_device_info *di, int step)
{
	int rc = 0;
	u8 buf[LEN_U16_DAT] = {0};
	if (!di)
		return -EINVAL;

	if (di->normal_calib_state != CALIB) {
		hwlog_err("stm32g031 is not in calib mode.\n");
		return -EINVAL;
	}
	hwlog_info("%s step: %d\n", __func__, step);
	if (step < 0) {
		di->mcu_state.step_backward_flag = 1;
		step = abs(step);
	} else {
		di->mcu_state.step_backward_flag = 0;
	}

	buf[0] = step & 0xff;
	buf[1] = step >> 8;
	rc = i2c_u8_write_byte(di->client, STM32G031_CALI_STEP_CNT_SET_LOW, buf[0]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc += i2c_u8_write_byte(di->client, STM32G031_CALI_STEP_CNT_SET_HIGH, buf[1]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	di->mcu_state.target_step = step;

	return rc;
}

static int stm32g031_calib_set_speed(struct cam_star_module_device_info *di, u16 speed)
{
	int rc = 0;
	u8 buf[LEN_U16_DAT] = {0};
	if (!di)
		return -EINVAL;

	if (di->normal_calib_state != CALIB) {
		hwlog_err("stm32g031 is not in calib mode.\n");
		return -EINVAL;
	}

	buf[0] = speed & 0xff;
	buf[1] = speed >> 8;
	rc = i2c_u8_write_byte(di->client, STM32G031_CALI_SPEED_CNT_SET_LOW, buf[0]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc += i2c_u8_write_byte(di->client, STM32G031_CALI_SPEED_CNT_SET_HIGH, buf[1]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	di->mcu_state.motor_speed = speed;
	return rc;
}

static int stm32g031_calib_start(struct cam_star_module_device_info *di)
{
	int rc = 0;
	u8 val = 0;
	int retry = 0;
	if (!di)
		return -EINVAL;
	if (di->power_state == POWER_OFF) {
		hwlog_err("stm32g031 is not powered on, please power on first.\n");
		return -EINVAL;
	}

	if (di->normal_calib_state != CALIB) {
		hwlog_err("stm3g031 is not in calib mode!\n");
		return -EINVAL;
	}

	if (di->mcu_state.step_backward_flag == 0) {
		/* step forward */
		rc = i2c_u8_write_byte(di->client, STM32G031_CALI_CMD, STM32G031_CALI_MOVE_FORWARD_STEP);
		if (rc) {
			hwlog_err("%s i2c failed.\n", __func__);
			return rc;
		}
	} else if (di->mcu_state.step_backward_flag == 1) {
		/* step backward */
		rc = i2c_u8_write_byte(di->client, STM32G031_CALI_CMD, STM32G031_CALI_MOVE_BACKWARD_STEP);
		if (rc) {
			hwlog_err("%s i2c failed.\n", __func__);
			return rc;
		}
	} else {
		hwlog_err("invalid step_backward_flag.\n");
		return -EINVAL;
	}

	do {
		i2c_u8_read_byte(di->client, STM32G031_CALI_STATE, &val);
		msleep(10);
		hwlog_info("%s retry times: %d\n", __func__, retry);
	} while ((retry++ < CALIB_I2C_MAX_WAIT_RETRY_TIMES) && (val & 0x01));

	if (retry >= CALIB_I2C_MAX_WAIT_RETRY_TIMES) {
		hwlog_err("%s fail, CPU is busy!\n", __func__);
		return -EINVAL;
	}

	return rc;
}

static int stm32g031_calib_get_real_height(struct cam_star_module_device_info *di, int16_t *real_code)
{
	int rc = 0;
	u8 buf[LEN_U16_DAT] = {0};
	u8 val = 0;
	int retry = 0;
	if (!di)
		return -EINVAL;

	if (di->normal_calib_state != CALIB) {
		hwlog_err("stm3g031 is not in calib mode!\n");
		return -EINVAL;
	}

	rc = i2c_u8_write_byte(di->client, STM32G031_CALI_CMD, STM32G031_CALI_GET_CURRENT_HEIGHT);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	do {
		i2c_u8_read_byte(di->client, STM32G031_CALI_STATE, &val);
		msleep(10);
		hwlog_info("%s retry times: %d\n", __func__, retry);
	} while ((retry++ < CALIB_I2C_MAX_WAIT_RETRY_TIMES) && (val & 0x01));

	if (retry >= CALIB_I2C_MAX_WAIT_RETRY_TIMES) {
		hwlog_err("%s fail, CPU is busy!\n", __func__);
		return -EINVAL;
	}

	rc = i2c_u8_read_byte(di->client, STM32G031_CALI_REAL_HEIGHT_LOW, &buf[0]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, STM32G031_CALI_REAL_HEIGHT_HIGH, &buf[1]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	*real_code = buf[0] | (buf[1] << 8);
	return rc;
}

static int stm32g031_calib_get_real_tmr(struct cam_star_module_device_info *di, int16_t *real_code)
{
	int rc = 0;
	u8 buf[LEN_U16_DAT] = {0};
	u8 val = 0;
	int retry = 0;
	if (!di)
		return -EINVAL;

	if (di->normal_calib_state != CALIB) {
		hwlog_err("stm3g031 is not in calib mode!\n");
		return -EINVAL;
	}

	rc = i2c_u8_write_byte(di->client, STM32G031_CALI_CMD, STM32G031_CALI_GET_CURRENT_TMR);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	do {
		i2c_u8_read_byte(di->client, STM32G031_CALI_STATE, &val);
		msleep(10);
		hwlog_info("%s retry times: %d\n", __func__, retry);
	} while ((retry++ < CALIB_I2C_MAX_WAIT_RETRY_TIMES) && (val & 0x01));

	if (retry >= CALIB_I2C_MAX_WAIT_RETRY_TIMES) {
		hwlog_err("%s fail, CPU is busy!\n", __func__);
		return -EINVAL;
	}

	rc = i2c_u8_read_byte(di->client, STM32G031_CALI_TMR_DATA_LOW, &buf[0]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, STM32G031_CALI_TMR_DATA_HIGH, &buf[1]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	*real_code = buf[0] | (buf[1] << 8);
	return rc;
}

int stm32g031_calib_write_flash(struct cam_star_module_device_info *di, u16 data)
{
	int rc = 0;
	u8 val = 0;
	u8 readback = 0;
	int retry = 0;

	/* step 2: send flash addr */
	hwlog_info("%s flash_offset before this write: %u",
		__func__, di->calib_otp.flash_offset);
	rc = i2c_u8_write_byte(di->client, STM32G031_CALI_FLASH_DATA_ADDR_L,
		di->calib_otp.flash_offset & 0xff);
	if (rc) {
		hwlog_err("%s write flash addr-low i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, STM32G031_CALI_FLASH_DATA_ADDR_L, &readback);
	hwlog_info("%s flash_offset_low readback: %u", __func__, readback);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	rc = i2c_u8_write_byte(di->client, STM32G031_CALI_FLASH_DATA_ADDR_H,
		di->calib_otp.flash_offset >> 8);
	if (rc) {
		hwlog_err("%s write flash addr-high i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, STM32G031_CALI_FLASH_DATA_ADDR_H, &readback);
	hwlog_info("%s flash_offset_high readback: %u", __func__, readback);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	di->calib_otp.flash_offset += 2;
	hwlog_info("%s flash_offset after this write: %u",
		__func__, di->calib_otp.flash_offset);

	/* step 3: send data to write to flash */
	hwlog_info("%s write_data: %d", __func__, data);
	rc = i2c_u8_write_byte(di->client, STM32G031_CALI_FLASH_DATA_VALUE_L,
		data & 0xff);
	if (rc) {
		hwlog_err("%s write flash data low i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, STM32G031_CALI_FLASH_DATA_VALUE_L, &readback);
	hwlog_info("%s flash_data_low readback: %u", __func__, readback);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	rc = i2c_u8_write_byte(di->client, STM32G031_CALI_FLASH_DATA_VALUE_H,
		data >> 8);
	if (rc) {
		hwlog_err("%s write flash data high i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, STM32G031_CALI_FLASH_DATA_VALUE_H, &readback);
	hwlog_info("%s flash_data_high readback: %u", __func__, readback);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	/* step 4: write to MCU RAM */
	rc = i2c_u8_write_byte(di->client, STM32G031_CALI_CMD, 0x32);
	if (rc) {
		hwlog_err("%s write to MCU RAM i2c failed.\n", __func__);
		return rc;
	}
	
	/* step 5: wait for MCU ready */
	do {
		i2c_u8_read_byte(di->client, STM32G031_CALI_STATE, &val);
		msleep(1);
		hwlog_info("%s wait for MCU ready, retry times: %d\n", __func__, retry);
	} while ((retry++ < CALIB_MCU_READY_MAX_WAIT_RETRY_TIMES) && (val & 0x03 != 0));
	if (retry >= CALIB_MCU_READY_MAX_WAIT_RETRY_TIMES) {
		hwlog_err("%s fail, CPU is busy!\n", __func__);
		return -EINVAL;
	}

	return rc;
}

static int stm32g031_calib_set_info(struct cam_star_module_device_info *di)
{
	int rc = 0;
	int i = 0;
	int retry = 0;
	u8 val = 0;
	hwlog_info("enter %s", __func__);
	if (!di)
		return -EINVAL;

	/* step 1: enter calib mode and release write protect */
	rc = i2c_u8_write_byte(di->client, STM32G031_CALI_CTL,
		CALIB_AND_RELEASE_WRITE_PROTECT);
	if (rc) {
		hwlog_err("%s enter calib mode i2c failed.\n", __func__);
		return rc;
	}

	/* step 2: erase flash */
	rc = i2c_u8_write_byte(di->client, STM32G031_CALI_CMD, 0x30);
	if (rc) {
		hwlog_err("%s erase flash i2c failed.\n", __func__);
		return rc;
	}

	/* add wait for MCU ready */
	do {
		i2c_u8_read_byte(di->client, STM32G031_CALI_STATE, &val);
		msleep(1);
		hwlog_info("%s retry times: %d\n", __func__, retry);
	} while ((retry++ < CALIB_MCU_READY_MAX_WAIT_RETRY_TIMES) && (val & 0x03 != 0));
	if (retry >= CALIB_MCU_READY_MAX_WAIT_RETRY_TIMES) {
		hwlog_err("%s fail, CPU is busy!\n", __func__);
		return -EINVAL;
	}

	/* step 3: set data num_of_pair */
	rc = stm32g031_calib_write_flash(di, di->calib_otp.calib_info.num_of_pair);
	if (rc) {
		hwlog_err("%s set data num_of_pair failed.\n", __func__);
		return rc;
	}

	/* step 4: set data system_accuracy_err */
	rc = stm32g031_calib_write_flash(di, di->calib_otp.calib_info.system_accuracy_err);
	if (rc) {
		hwlog_err("%s set data system_accuracy_err failed.\n", __func__);
		return rc;
	}

	/* step 5: set data distance and corresponding tmr data */
	for (i = 0; i < CALIB_INFO_NUM_OF_PAIR; ++i) {
		/* set data: distance */
		rc = stm32g031_calib_write_flash(di, di->calib_otp.calib_info.dis_tmr_pair[i].dis);
		if (rc) {
			hwlog_err("%s write distance flash failed.\n", __func__);
			return rc;
		}

		/* set data: tmr */
		rc = stm32g031_calib_write_flash(di, di->calib_otp.calib_info.dis_tmr_pair[i].tmr);
		if (rc) {
			hwlog_err("%s write tmr flash failed.\n", __func__);
			return rc;
		}
	}

	return rc;
}

static int stm32g031_calib_set_add_info(struct cam_star_module_device_info *di)
{
	int rc = 0;
	int i = 0;
	int retry = 0;
	u8 val = 0;
	hwlog_info("enter %s", __func__);
	if (!di)
		return -EINVAL;

	/* step 3: set calib_add_info data */
	for (i = 0; i < sizeof(di->calib_otp.calib_add_info); ++i) {
		/* set data */
		rc = stm32g031_calib_write_flash(di, di->calib_otp.calib_add_info[i]);
		if (rc) {
			hwlog_err("%s write distance flash failed.\n", __func__);
			return rc;
		}
	}

	/* step 5: open writing protect */
	rc = i2c_u8_write_byte(di->client, STM32G031_CALI_CTL, 0x01);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	/* step 6: force MCU to reload flash data */
	rc = i2c_u8_write_byte(di->client, STM32G031_CALI_CMD, 0x3F);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	/* step 7: wait for MCU ready */
	do {
		i2c_u8_read_byte(di->client, STM32G031_CALI_STATE, &val);
		msleep(1);
		hwlog_info("%s retry times: %d\n", __func__, retry);
	} while ((retry++ < CALIB_MCU_READY_MAX_WAIT_RETRY_TIMES) && (val & 0x03 != 0));
	if (retry >= CALIB_MCU_READY_MAX_WAIT_RETRY_TIMES) {
		hwlog_err("%s fail, CPU is busy!\n", __func__);
		return -EINVAL;
	}

	return rc;
}

static int stm32g031_calib_crc_check(struct cam_star_module_device_info *di)
{
	int rc = 0;
	u8 val = 0;
	int i = 0;
	int offset = 0;
	int retry = 0;
	u8 buf_cal[2] = {0};
	u8 buf_rec[2] = {0};
	hwlog_info("enter %s", __func__);
	if (!di)
		return -EINVAL;

	/* start calculating flash CRC */
	rc = i2c_u8_write_byte(di->client, STM32G031_CALI_CMD, 0x35);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	/* wait for MCU ready */
	do {
		i2c_u8_read_byte(di->client, STM32G031_CALI_STATE, &val);
		msleep(10);
		hwlog_info("%s retry times: %d\n", __func__, retry);
	} while ((retry++ < CALIB_I2C_MAX_WAIT_RETRY_TIMES) && (val & 0x03 != 0));
	if (retry >= I2C_MAX_WAIT_RETRY) {
		hwlog_err("%s fail, CPU is busy!\n", __func__);
		return -EINVAL;
	}

	/* check CRC */
	rc = i2c_u8_read_byte(di->client, STM32G031_CALI_CRC_CALC_VALUE_L, &buf_cal[0]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, STM32G031_CALI_CRC_CALC_VALUE_H, &buf_cal[1]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, STM32G031_CALI_CRC_REC_VALUE_L, &buf_rec[0]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, STM32G031_CALI_CRC_REC_VALUE_H, &buf_rec[1]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	if (buf_cal[0] == buf_rec[0] && buf_cal[1] == buf_rec[1]) {
		hwlog_info("%s CRC check success!\n", __func__);
		di->calib_otp.crc_readout = buf_rec[0] | buf_rec[1] << 8;
	} else {
		hwlog_err("%s CRC check failed!\n");
	}

	return rc;
}

int stm32g031_calib_read_flash(struct cam_star_module_device_info *di,
	u16 offset, u16 *read_value)
{
	int rc = 0;
	u8 val = 0;
	u8 calib_mode;
	u8 readback = 0;
	int i = 0;
	int retry = 0;
	u8 buf[2] = {0};
	hwlog_info("enter %s", __func__);
	if (!di)
		return -EINVAL;

	/* read calib mode reg */
	rc = i2c_u8_read_byte(di->client, STM32G031_CALI_CTL, &calib_mode);
	hwlog_info("%s calib_mode: %u", __func__, calib_mode & 0x01);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	/* flash addr to read */
	rc = i2c_u8_write_byte(di->client, STM32G031_CALI_FLASH_DATA_ADDR_L, offset & 0xff);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, STM32G031_CALI_FLASH_DATA_ADDR_L, &readback);
	hwlog_info("%s flash_offset_low readback: %u", __func__, readback);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	rc = i2c_u8_write_byte(di->client, STM32G031_CALI_FLASH_DATA_ADDR_H, offset >> 8);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, STM32G031_CALI_FLASH_DATA_ADDR_H, &readback);
	hwlog_info("%s flash_offset_high readback: %u", __func__, readback);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	/* flash read cmd */
	rc = i2c_u8_write_byte(di->client, STM32G031_CALI_CMD, 0x31);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	/* wait for MCU ready */
	do {
		i2c_u8_read_byte(di->client, STM32G031_CALI_STATE, &val);
		msleep(10);
		hwlog_info("%s retry times: %d\n", __func__, retry);
	} while ((retry++ < CALIB_I2C_MAX_WAIT_RETRY_TIMES) && (val & 0x03 != 0));
	if (retry >= I2C_MAX_WAIT_RETRY) {
		hwlog_err("%s fail, CPU is busy!\n", __func__);
		return -EINVAL;
	}

	/* get value */
	rc = i2c_u8_read_byte(di->client, STM32G031_CALI_FLASH_DATA_VALUE_L, &buf[0]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, STM32G031_CALI_FLASH_DATA_VALUE_H, &buf[1]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	*read_value = buf[0] | buf[1] << 8;
	hwlog_info("%s read_value: %u", __func__, *read_value);

	return rc;
}

static int stm32g031_calib_get_info(struct cam_star_module_device_info *di, u16 *read_value, int *size_read_value)
{
	int rc = 0;
	int i = 0;
	int size = 0;
	u16 offset = 0;
	u16 *readout = NULL;
	hwlog_info("enter %s", __func__);
	if (!di)
		return -EINVAL;

	size = sizeof(struct calib_info_t);

	for (i = 0; i < size; ++i) {
		rc = stm32g031_calib_read_flash(di, offset, read_value);
		if (rc) {
			hwlog_err("%s read_flash fail\n");
			return rc;
		}
		offset += 2;
		read_value += 1;
	}

	*size_read_value = size;
	hwlog_info("%s calib_info size: %d\n", __func__, *size_read_value);

	return rc;
}

static int stm32g031_open(struct cam_star_module_device_info *di)
{
	(void)di;
	hwlog_info("%s device open\n", __func__);
	mutex_lock(&power_lock_mutex);
	g_ref_count++;
	mutex_unlock(&power_lock_mutex);
	return 0;
}

static int stm32g031_close(struct cam_star_module_device_info *di)
{
	int rc = 0;
	(void)di;
	if (!di)
		return -EINVAL;
	hwlog_info("%s device close\n", __func__);
	mutex_lock(&power_lock_mutex);

	if (g_ref_count)
		g_ref_count--;

	if ((g_ref_count == 0) && di->mcu_state.target_pos != 0) {
		hwlog_err("%s has not move back, now moving it back to zero.\n",
			__func__);

		rc = stm32g031_set_code(di, 0);
		if (rc != 0)
			hwlog_err("%s failed to stm32g031_set_code to zero\n", __func__);
	}

	if ((g_ref_count == 0) && di->power_state == POWER_ON) {
		hwlog_err("%s is still on power-on state, now power it off\n",
			__func__);

		rc = stm32g031_power_off(di, POWEROFF_DIR_WITHOUT_TRY);
		if (rc != 0)
			hwlog_err("%s failed to stm32g031 power off\n", __func__);
	}

	di->normal_calib_state = NORMAL;

	mutex_unlock(&power_lock_mutex);
	return rc;
}

static struct star_api_fn_t stm32g031_api_func_tbl = {
	.open = stm32g031_open,
	.close = stm32g031_close,
	.get_info = stm32g031_get_info,
	.power_on = stm32g031_power_on,
	.power_off = stm32g031_power_off,
	.set_bootloader = stm32g031_set_bootloader,
	.matchid = stm32g031_matchid,
	.download_firmware = stm32g031_download_firmware,
	.set_code = stm32g031_set_code,
	.set_speed = stm32g031_set_speed,
	.start = stm32g031_start,
	.stop = stm32g031_stop,
	.get_real_height = stm32g031_get_real_height,
	.set_calib_mode = stm32g031_set_calib_mode,
	.calib_set_step = stm32g031_calib_set_step,
	.calib_set_speed = stm32g031_calib_set_speed,
	.calib_start = stm32g031_calib_start,
	.calib_get_real_height = stm32g031_calib_get_real_height,
	.calib_get_real_tmr = stm32g031_calib_get_real_tmr,
	.calib_set_info = stm32g031_calib_set_info,
	.calib_set_add_info = stm32g031_calib_set_add_info,
	.calib_read_info = stm32g031_calib_get_info,
};

/* MCU vendor info */
static ssize_t stm32g031_vendor_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_star_module_device_info *di = NULL;
	hwlog_info("enter stm32g031_vendor_show\n");
	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}

	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	} else {
		return scnprintf(buf, PAGE_SIZE, "%s\n", di->ic_name);
	}
}

static DEVICE_ATTR(vendor, 0664, stm32g031_vendor_show, NULL);

/* firmware version_id which is to update attr */
static ssize_t stm32g031_fw_version_id_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_star_module_device_info *di = NULL;
	hwlog_info("enter stm32g031_fw_version_id_show\n");
	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}

	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	} else {
		return scnprintf(buf, PAGE_SIZE, "%u\n", di->fw_state.fw_ver_id);
	}
}

static ssize_t stm32g031_fw_version_id_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di;
	unsigned int val = 0;
	int rc = 0;
	hwlog_info("enter stm32g031_fw_version_id_store\n");

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}

	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&di->lock);
	if (sscanf_s(buf, "%u\n", &val) != 1) {
		hwlog_err("cpy buf fail.\n");
		mutex_unlock(&di->lock);
		return -1;
	}
	if (val < 0 || val > 65535) {
		hwlog_err("%s input unvalid value = %u\n", __func__, val);
		mutex_unlock(&di->lock);
		return count;
	}
	hwlog_info("%s input value = %u\n", __func__, val);
	di->fw_state.fw_ver_id = val;
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(fw_version_id, 0664, stm32g031_fw_version_id_show, stm32g031_fw_version_id_store);

/* firmware updata state attr
* 0: UPDATE_WHEN_NOT_EQUAL
* 1: UPDATE_WHEN_GREATER_THAN
*/
static ssize_t stm32g031_fw_update_state_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_star_module_device_info *di = NULL;
	hwlog_info("enter stm32g031_fw_update_state_show\n");
	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}

	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	} else {
		return scnprintf(buf, PAGE_SIZE, "%u\n", di->fw_update_state);
	}
}

static ssize_t stm32g031_fw_update_state_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di;
	unsigned int val = 0;
	int rc = 0;
	hwlog_info("enter stm32g031_fw_update_state_store\n");

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}

	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&di->lock);
	if (sscanf_s(buf, "%u\n", &val) != 1) {
		hwlog_err("cpy buf fail.\n");
		mutex_unlock(&di->lock);
		return -1;
	}
	if (val != 0 && val != 1) {
		hwlog_err("%s input unvalid value = %u\n", __func__, val);
		mutex_unlock(&di->lock);
		return count;
	}
	hwlog_info("%s input value = %u\n", __func__, val);
	di->fw_update_state = val;
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(fw_update_state, 0664, stm32g031_fw_update_state_show, stm32g031_fw_update_state_store);

/* power attr
* power on: echo 1
* power off: echo 0
*/
static ssize_t stm32g031_power_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_star_module_device_info *di = NULL;
	hwlog_info("enter stm32g031_power_show\n");
	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}
	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	} else {
		return scnprintf(buf, PAGE_SIZE, "%u\n", di->power_state);
	}
}

static ssize_t stm32g031_power_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	unsigned int val = 0;
	int rc = 0;
	hwlog_info("enter stm32g031_power_store\n");

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}
	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&di->lock);
	if (sscanf_s(buf, "%u\n", &val) != 1) {
		mutex_unlock(&di->lock);
		return -1;
	}
	if (val != 0 && val != 1) {
		hwlog_err("%s input unvalid value = %u\n", __func__, val);
		mutex_unlock(&di->lock);
		return count;
	}
	hwlog_info("%s input value = %u\n", __func__, val);
	if (val == 1) {
		rc = stm32g031_power_on(di);
	} else {
		stm32g031_power_off_with_retry(di);
	}
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(power, 0664, stm32g031_power_show, stm32g031_power_store);

/* set_bootloader attr
* set_bootloader: echo 1
*/
static ssize_t stm32g031_set_bootloader_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	unsigned int val = 0;
	int rc = 0;
	hwlog_info("enter stm32g031_set_bootloader_store\n");

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}
	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&di->lock);
	if (sscanf_s(buf, "%u\n", &val) != 1) {
		mutex_unlock(&di->lock);
		return -1;
	}
	if (val != 1) {
		hwlog_err("%s input unvalid value = %u, this mode only support 1.\n", __func__, val);
		mutex_unlock(&di->lock);
		return count;
	}
	hwlog_info("%s input value = %u\n", __func__, val);
	rc = stm32g031_set_bootloader(di);
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(set_bootloader, 0664, NULL, stm32g031_set_bootloader_store);

/* download_firmware attr
*  download_firmware: echo 1
*  not support cat
*/
static ssize_t stm32g031_download_firmware_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	unsigned int val = 0;
	int rc = 0;
	hwlog_info("enter stm32g031_download_firmware_store\n");

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}
	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&di->lock);
	if (sscanf_s(buf, "%u\n", &val) != 1) {
		mutex_unlock(&di->lock);
		return -1;
	}
	if (val != 1) {
		hwlog_err("%s input unvalid value = %u, this mode only support 1.\n", __func__, val);
		mutex_unlock(&di->lock);
		return count;
	}
	hwlog_info("%s input value = %u\n", __func__, val);
	rc = stm32g031_download_firmware(di);
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(download_firmware, 0664, NULL, stm32g031_download_firmware_store);

/* set_height attr
* set_height: echo u16 height
*/
static ssize_t stm32g031_set_height_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_star_module_device_info *di = NULL;
	hwlog_info("enter stm32g031_set_height_show\n");

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}
	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -EINVAL;
	} else
		return scnprintf(buf, PAGE_SIZE, "%u\n", di->mcu_state.target_pos);
}

static ssize_t stm32g031_set_height_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	u16 val = 0;
	int rc = 0;
	hwlog_info("enter stm32g031_set_height_store\n");

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}
	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&di->lock);
	if (sscanf_s(buf, "%u\n", &val) != 1) {
		mutex_unlock(&di->lock);
		return -1;
	}

	hwlog_info("%s input value = %u\n", __func__, val);
	rc = stm32g031_set_code(di, val);
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(set_height, 0664, stm32g031_set_height_show, stm32g031_set_height_store);

/* set_speed attr
* set_speed: echo u16 speed
*/
static ssize_t stm32g031_set_speed_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_star_module_device_info *di = NULL;
	hwlog_info("enter stm32g031_set_speed_show\n");

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}
	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -EINVAL;
	}

	return scnprintf(buf, PAGE_SIZE, "%u\n", di->mcu_state.motor_speed);
}

static ssize_t stm32g031_set_speed_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	u16 val = 0;
	int rc = 0;
	hwlog_info("enter stm32g031_set_speed_store\n");

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}
	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&di->lock);
	if (sscanf_s(buf, "%u\n", &val) != 1) {
		mutex_unlock(&di->lock);
		return -1;
	}

	hwlog_info("%s input value = %u\n", __func__, val);
	rc = stm32g031_set_speed(di, val);
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(set_speed, 0664, stm32g031_set_speed_show, stm32g031_set_speed_store);

/* start attr
* start: echo 1
*/
static ssize_t stm32g031_start_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	unsigned int val = 0;
	int rc = 0;
	hwlog_info("enter stm32g031_start_store\n");

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}
	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&di->lock);
	if (sscanf_s(buf, "%u\n", &val) != 1) {
		mutex_unlock(&di->lock);
		return -1;
	}

	if (val != 1) {
		hwlog_err("%s input unvalid value = %u, this mode only support 1.\n", __func__, val);
		mutex_unlock(&di->lock);
		return -EINVAL;
	}

	hwlog_info("%s input value = %u\n", __func__, val);
	rc = stm32g031_start(di);
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(start, 0664, NULL, stm32g031_start_store);

/* stop attr
* stop: echo 1
*/
static ssize_t stm32g031_stop_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	unsigned int val = 0;
	int rc = 0;
	hwlog_info("enter stm32g031_stop_store\n");

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}
	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&di->lock);
	if (sscanf_s(buf, "%u\n", &val) != 1) {
		mutex_unlock(&di->lock);
		return -1;
	}

	if (val != 1) {
		hwlog_err("%s input unvalid value = %u, this mode only support 1.\n", __func__, val);
		mutex_unlock(&di->lock);
		return -EINVAL;
	}

	hwlog_info("%s input value = %u\n", __func__, val);
	rc = stm32g031_stop(di);
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(stop, 0664, NULL, stm32g031_stop_store);

/* get_real_height attr
* get_real_height: get_real_height
*/
static ssize_t stm32g031_get_real_height_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_star_module_device_info *di = NULL;
	u16 real_code = 0;
	int rc = 0;
	hwlog_info("enter stm32g031_get_real_height_show\n");

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}
	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -EINVAL;
	}

	rc = stm32g031_get_real_height(di, &real_code);
	return scnprintf(buf, PAGE_SIZE, "%u\n", real_code);
}
static DEVICE_ATTR(get_real_height, 0664, stm32g031_get_real_height_show, NULL);

/* set_calib_mode attr
* set calib mode: echo 1
* set calib mode: echo 0
*/
static ssize_t stm32g031_set_calib_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_star_module_device_info *di = NULL;
	hwlog_info("enter stm32g031_set_calib_mode_show", __func__);

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}
	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -EINVAL;
	} else
		return scnprintf(buf, PAGE_SIZE, "%u\n", di->normal_calib_state);
}

static ssize_t stm32g031_set_calib_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	unsigned int val = 0;
	int rc = 0;
	hwlog_info("enter stm32g031_set_calib_mode_store\n");

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}
	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&di->lock);
	if (sscanf_s(buf, "%u\n", &val) != 1) {
		mutex_unlock(&di->lock);
		return -1;
	}

	if (val != 0 && val != 1) {
		hwlog_err("cam_star stm32g031 unsupported mode: %u.\n", val);
		mutex_unlock(&di->lock);
		return -EINVAL;
	}
	hwlog_info("%s input value = %u\n", __func__, val);
	rc = stm32g031_set_calib_mode(di, val);
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(set_calib_mode, 0664, stm32g031_set_calib_mode_show, stm32g031_set_calib_mode_store);

/* calib_info attr
* set calib info to MCU: echo 1
* get calib info: cat, and return CRC result.
*/
static ssize_t stm32g031_calib_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_star_module_device_info *di = NULL;
	int rc = 0;
	int size = 0;
	hwlog_info("enter stm32g031_calib_info_show", __func__);

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}
	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&di->lock);
	rc = stm32g031_calib_get_info(di, (u16 *)buf, &size);
	if (rc) {
		hwlog_err("stm32g031_calib_get_info fail\n");
		mutex_unlock(&di->lock);
		return -EINVAL;
	}

	mutex_unlock(&di->lock);
	return size;
}

static ssize_t stm32g031_calib_info_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	unsigned int val = 0;
	int rc = 0;
	char *calib_info = NULL;
	int i = 0; // i index the calib_info array
	char *token = NULL;
	char *next_token = NULL;
	u16 value = 0;
	hwlog_info("enter stm32g031_calib_info_store\n");

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}
	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&di->lock);
	calib_info = kmalloc(CAM_SENSORMOTOR_CALIB_DATA_MAX_LEN, GFP_KERNEL);
	if (calib_info == NULL) {
		hwlog_err("%s kmalloc error!\n", __func__);
		mutex_unlock(&di->lock);
		return -EINVAL;
	}
	/* parse input string */
	hwlog_info("%s buf input value = %s\n", __func__, buf);
	if (memcpy_s(calib_info, CAM_SENSORMOTOR_CALIB_DATA_MAX_LEN,
		buf, CAM_SENSORMOTOR_CALIB_DATA_MAX_LEN) != EOK) {
		hwlog_err("%s memcpy failed.\n");
		mutex_unlock(&di->lock);
		return -EINVAL;
	}
	hwlog_info("%s calib_info input value = %s\n", __func__, calib_info);
	/* parse first tag: num_of_pair */
	token = strtok_s(calib_info, " ", &next_token);
	if (token) {
		rc = kstrtou16(token, 10, &(di->calib_otp.calib_info.num_of_pair));
		hwlog_info("%s num_of_pair: %u", __func__, di->calib_otp.calib_info.num_of_pair);
	} else {
		hwlog_err("%s input invalid 1! No separator BLANK!\n");
		mutex_unlock(&di->lock);
		return -EINVAL;
	}

	if (di->calib_otp.calib_info.num_of_pair > CALIB_INFO_NUM_OF_PAIR) {
		hwlog_err("%s input num_of_pair larger than max 72!\n");
		di->calib_otp.calib_info.num_of_pair = CALIB_INFO_NUM_OF_PAIR;
	}

	/* parse second tag: system_accuracy_err */
	hwlog_info("lyl calib_info 2: %s, strlen(token): %d, next_token:%s\n", calib_info, strlen(token), &next_token);
	token = strtok_s(NULL, " ", &next_token);
	if (token) {
		rc = kstrtou16(token, 10, &(di->calib_otp.calib_info.system_accuracy_err));
		hwlog_info("%s system_accuracy_err: %u", __func__, di->calib_otp.calib_info.system_accuracy_err);
	} else {
		hwlog_err("%s input invalid 2! No separator BLANK!\n");
		mutex_unlock(&di->lock);
		return -EINVAL;
	}
	/* parse distance and corresponding tmr value */
	while (token && i < (di->calib_otp.calib_info.num_of_pair) * 2) {
		token = strtok_s(NULL, " ", &next_token);
		rc = kstrtou16(token, 10, &value);
		hwlog_info("%s input calib_info: %s\n", __func__, token);
		if ( i % 2 == 0) {
			di->calib_otp.calib_info.dis_tmr_pair[i / 2].dis = value;
			hwlog_info("index of pair: %d, dis: %d", i / 2, value);
		} else {
			di->calib_otp.calib_info.dis_tmr_pair[i / 2].tmr = value;
			hwlog_info("index of pair: %d, tmr: %d", i / 2, value);
		}
		i++;
	}

	/* set calib info to MCU */
	rc = stm32g031_calib_set_info(di);
	kfree(calib_info);
	mutex_unlock(&di->lock);
	return rc;
}
static DEVICE_ATTR(calib_info, 0664, stm32g031_calib_info_show, stm32g031_calib_info_store);

/* calib_add_info attr
* set calib additional info to MCU: echo 1
*/
static ssize_t stm32g031_calib_add_info_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	unsigned int val = 0;
	int rc = 0;
	char *calib_add_info = NULL;
	int i = 1; // i index the calib_add_info array
	char *token = NULL;
	char *next_token = NULL;
	u16 value = 0;
	hwlog_info("enter stm32g031_calib_info_store\n");

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}
	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&di->lock);
	calib_add_info = kmalloc(CAM_SENSORMOTOR_CALIB_ADD_INFO_MAX_LEN, GFP_KERNEL);
	if (calib_add_info == NULL) {
		hwlog_err("%s kmalloc error!\n", __func__);
		mutex_unlock(&di->lock);
		return NULL;
	}

	/* parse input string */
	hwlog_info("%s buf input value = %s\n", __func__, buf);
	if (memcpy_s(calib_add_info, CAM_SENSORMOTOR_CALIB_ADD_INFO_MAX_LEN,
		buf, CAM_SENSORMOTOR_CALIB_ADD_INFO_MAX_LEN) != EOK) {
		hwlog_err("%s memcpy failed.\n");
		mutex_unlock(&di->lock);
		return -EINVAL;
	}
	hwlog_info("%s calib_add_info input value = %s\n", __func__, calib_add_info);
	token = strtok_s(calib_add_info, " ", &next_token);
	if (token) {
		rc = kstrtou16(token, 10, &(di->calib_otp.calib_add_info[0]));
		hwlog_info("%s crc_check: %u", __func__, di->calib_otp.calib_add_info[0]);
	} else {
		hwlog_err("%s input invalid 1! No separator BLANK!\n");
		mutex_unlock(&di->lock);
		return -EINVAL;
	}

	while (token && i < sizeof(di->calib_otp.calib_add_info)) {
		token = strtok_s(NULL, " ", &next_token);
		if (!token) {
			hwlog_err("%s token is null.\n", __func__);
			break;
		}
		rc = kstrtou16(token, 10, &value);
		di->calib_otp.calib_add_info[i] = value;
		hwlog_info("%s index: %d, calib_add_info: %s, value: %u", __func__, i, token, value);
		i++;
	}

	/* set calib info to MCU */
	rc = stm32g031_calib_set_add_info(di);
	kfree(calib_add_info);
	mutex_unlock(&di->lock);
	return rc;
}
static DEVICE_ATTR(calib_add_info, 0664, NULL, stm32g031_calib_add_info_store);

/* calib_set_step attr
* calib_set_step: echo int16 height
*/
static ssize_t stm32g031_calib_set_step_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	int val = 0;
	int rc = 0;
	hwlog_info("enter stm32g031_calib_set_step_store\n");

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}
	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&di->lock);
	if (sscanf_s(buf, "%d\n", &val) != 1) {
		mutex_unlock(&di->lock);
		return -1;
	}

	hwlog_info("%s input value = %d\n", __func__, val);
	rc = stm32g031_calib_set_step(di, val);
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(calib_set_step, 0664, NULL, stm32g031_calib_set_step_store);

/* calib_set_speed attr
* calib_set_speed: echo u16 height
*/
static ssize_t stm32g031_calib_set_speed_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	u16 val = 0;
	int rc = 0;
	hwlog_info("enter stm32g031_calib_set_speed_store\n");

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}
	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&di->lock);
	if (sscanf_s(buf, "%u\n", &val) != 1) {
		mutex_unlock(&di->lock);
		return -1;
	}

	hwlog_info("%s input value = %u\n", __func__, val);
	rc = stm32g031_calib_set_speed(di, val);
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(calib_set_speed, 0664, NULL, stm32g031_calib_set_speed_store);

/* calib_start attr
* calib_start: echo 1
*/
static ssize_t stm32g031_calib_start_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	u16 val = 0;
	int rc = 0;
	hwlog_info("enter stm32g031_calib_start_store\n");

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}
	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&di->lock);
	if (sscanf_s(buf, "%u\n", &val) != 1) {
		mutex_unlock(&di->lock);
		return -1;
	}
	if (val != 1) {
		hwlog_err("%s input unvalid value = %u, this mode only support 1.\n", __func__, val);
		mutex_unlock(&di->lock);
		return -EINVAL;
	}

	hwlog_info("%s input value = %u\n", __func__, val);
	rc = stm32g031_calib_start(di);
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(calib_start, 0664, NULL, stm32g031_calib_start_store);

/* calib_get_real_height attr
* calib_get_real_height: calib_get_real_height
*/
static ssize_t stm32g031_calib_get_real_height_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_star_module_device_info *di = NULL;
	int16_t real_code = 0;
	int rc = 0;
	hwlog_info("enter stm32g031_calib_get_real_height_show\n");

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}
	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&di->lock);
	rc = stm32g031_calib_get_real_height(di, &real_code);
	mutex_unlock(&di->lock);
	return scnprintf(buf, PAGE_SIZE, "%d\n", real_code);
}
static DEVICE_ATTR(calib_get_real_height, 0664, stm32g031_calib_get_real_height_show, NULL);

/* calib_get_real_tmr attr
* calib_get_real_tmr: calib_calib_get_real_tmr
*/
static ssize_t stm32g031_calib_get_real_tmr_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_star_module_device_info *di = NULL;
	int16_t real_code = 0;
	int rc = 0;
	hwlog_info("enter stm32g031_calib_get_real_tmr_show\n");

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}
	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&di->lock);
	rc = stm32g031_calib_get_real_tmr(di, &real_code);
	mutex_unlock(&di->lock);
	return scnprintf(buf, PAGE_SIZE, "%d\n", real_code);
}
static DEVICE_ATTR(calib_get_real_tmr, 0664, stm32g031_calib_get_real_tmr_show, NULL);

/* AT sysfs */

/* save_height_value attr
* save_height_value_show: return saved height value
* save_height_value_store: read_real_height every 20ms, and save it.
*/
static ssize_t stm32g031_save_height_value_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_star_module_device_info *di = NULL;
	int size = -1;
	hwlog_info("enter stm32g031_save_height_value_show\n");

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}
	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&di->lock);
	if (di->atcmd_param.save_height_value != 1) {
		hwlog_err("not save height value\n");
		mutex_unlock(&di->lock);
		return -1;
	}
	size = sizeof(di->atcmd_param.height_value);
	hwlog_err("lyl 1:%d, 2: %d\n", CAM_SENSORMOTOR_HEIGHT_DATA_LEN * sizeof(int), sizeof(di->atcmd_param.height_value));
	if (memcpy_s(buf, CAM_SENSORMOTOR_HEIGHT_DATA_LEN * sizeof(int),
		&(di->atcmd_param.height_value), sizeof(di->atcmd_param.height_value)) != EOK) {
		hwlog_err("%s memcpy failed\n", __func__);
		mutex_unlock(&di->lock);
		return -EINVAL;
	}
	hwlog_err("lyl 3\n");
	mutex_unlock(&di->lock);
	return size;
}

static ssize_t stm32g031_save_height_value_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	u16 val = 0;
	int rc = 0;
	u16 real_code = 0;
	hwlog_info("enter stm32g031_save_height_value_store\n");

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}
	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&di->lock);
	if (sscanf_s(buf, "%u\n", &val) != 1) {
		mutex_unlock(&di->lock);
		return -1;
	}

	hwlog_info("%s input value = %u\n", __func__, val);
	if (val != 0 && val != 1) {
		hwlog_err("invalid input value!\n");
		mutex_unlock(&di->lock);
		return -EINVAL;
	}
	di->atcmd_param.save_height_value = val;

	if (di->atcmd_param.save_height_value == 1) {
		hwlog_info("save_height_value is:");
		for (int i = 0; i < CAM_SENSORMOTOR_HEIGHT_DATA_LEN; ++i) {
			rc = stm32g031_get_real_height(di, &real_code);
			if (rc) {
				hwlog_err("save_height_value failed!\n");
				di->atcmd_param.height_value[i] = -1;
			} else {
				di->atcmd_param.height_value[i] = real_code;
			}
			hwlog_info("%d\n", di->atcmd_param.height_value[i]);
			msleep(20);
		}
	}
	mutex_unlock(&di->lock);
	return rc ? -1 : count;
}
static DEVICE_ATTR(save_height_value, 0664, stm32g031_save_height_value_show, stm32g031_save_height_value_store);

static struct attribute *stm32g031_attributes[] = {
	&dev_attr_vendor.attr,
	&dev_attr_fw_version_id.attr,
	&dev_attr_fw_update_state.attr,
	&dev_attr_power.attr,
	&dev_attr_set_bootloader.attr,
	&dev_attr_download_firmware.attr,
	&dev_attr_set_height.attr,
	&dev_attr_set_speed.attr,
	&dev_attr_start.attr,
	&dev_attr_stop.attr,
	&dev_attr_get_real_height.attr,
	NULL,
};

static struct attribute *stm32g031_calib_attrs[] = {
	&dev_attr_set_calib_mode.attr,
	&dev_attr_calib_info.attr,
	&dev_attr_calib_add_info.attr,
	&dev_attr_calib_set_step.attr,
	&dev_attr_calib_set_speed.attr,
	&dev_attr_calib_start.attr,
	&dev_attr_calib_get_real_height.attr,
	&dev_attr_calib_get_real_tmr.attr,
	NULL,
};

static struct attribute *stm32g031_atcmd_attrs[] = {
	&dev_attr_save_height_value.attr,
	NULL,
};

static const struct attribute_group stm32g031_attr_group = {
	.name = "normal",
	.attrs = stm32g031_attributes,
};

static const struct attribute_group stm32g031_calib_group = {
	.name = "calib",
	.attrs = stm32g031_calib_attrs,
};

static const struct attribute_group stm32g031_atcmd_group = {
	.name = "ATCMD",
	.attrs = stm32g031_atcmd_attrs,
};

static const struct attribute_group *star_groups[] = {
	&stm32g031_attr_group,
	&stm32g031_calib_group,
	&stm32g031_atcmd_group,
	NULL,
};

static void hisp_notify_intf_rpmsg_cb(struct star_notify_intf_t *i,
											struct star_event_t *star_ev)
{
	if (!i || !(i->vtbl) || !(i->vtbl->rpmsg_cb)) {
		hwlog_err("func %s: param is NULL\n", __func__);
		return;
	}

	i->vtbl->rpmsg_cb(i, star_ev);
}

int stm32g031_probe(struct i2c_client *client,
	const struct i2c_device_id *id, struct cam_star_module_device_info *di)
{
	int rc = 0;
	struct device_node *np = NULL;

	if (!client || !client->dev.of_node || !id || !di)
		return -ENODEV;

	/* set bootloader mode */
	stm32g031_set_bootloader(di);
	/* matchid */
	if (stm32g031_matchid(di)) {
		hwlog_err("stm32g031 matchid failed.\n");
		/* power off */
		stm32g031_power_off_with_retry(di);
		return -EINVAL;
	}

	rc = stm32g031_irq_init(di);
	if (rc) {
		hwlog_err("cam_stm32g031 irq init failed.\n");
		return rc;
	}
	/* add di ioctl intf */
	di->api_func_tbl = &stm32g031_api_func_tbl;
	di->star_attr_group = star_groups;

	/* init th MCU state */
	di->power_state = POWER_OFF;
	di->normal_calib_state = NORMAL;
	/* init fw version id */
	di->fw_state.fw_ver_id = STM32G031_FW_VER;

	return 0;
}
EXPORT_SYMBOL(stm32g031_probe);

int stm32g031_remove(struct i2c_client *client)
{
	struct cam_star_module_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	return 0;
}
EXPORT_SYMBOL(stm32g031_remove);
