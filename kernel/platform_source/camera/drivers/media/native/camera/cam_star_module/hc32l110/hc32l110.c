// SPDX-License-Identifier: GPL-2.0
/* hc32l110.c
 *
 * hc32l110 driver
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
#include "cam_log.h"
#include "../cam_sensormotor_dmd_report.h"
#include "../cam_sensormotor_i2c.h"
#include "../cam_star_module_adapter.h"
#include <securec.h>
#include <linux/string.h>
#include <chipset_common/hwpower/hardware_ic/buck_boost.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <chipset_common/hwmanufac/dev_detect/dev_detect.h>
#endif

#define HWLOG_TAG cam_star_hc32l110
HWLOG_REGIST();

static int g_ref_count = 0;
static struct mutex power_lock_mutex;

static int hc32l110_get_info(struct cam_star_module_device_info *di,
	struct sensormotor_info *info)
{
	if (!di)
		return -EINVAL;
	int rc = 0;
	rc = strncpy_s(info->name, HWSENSORMOTOR_DEVICE_NAME_SIZE - 1, di->ic_name, strlen(di->ic_name));
	hwlog_info("ic_name: %s\n", info->name);
	return rc;
}

int hc32l110_read_mcu_i2c_debug_data(struct cam_star_module_device_info *di)
{
	int rc = 0;
	u8 cmd[LEN_U8_REG] = { 0 };
	u8 buf[16] = {0};
	if (!di)
		return -EINVAL;

	cmd[0] = HC32L110_STACKED_LR_LL;
	rc =  i2c_read_block(di->client, cmd, sizeof(cmd), buf, sizeof(buf));
	if (rc) {
		hwlog_err("%s i2c_read_block failed.\n", __func__);
		return rc;
	}
	hwlog_info("%s i2c_key_data: 0x%08x, 0x%08x, 0x%08x, 0x%08x\n", __func__,
		(uint32_t)(buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24),
		(uint32_t)(buf[4] | buf[5] << 8 | buf[6] << 16 | buf[7] << 24),
		(uint32_t)(buf[8] | buf[9] << 8 | buf[10] << 16 | buf[11] << 24),
		(uint32_t)(buf[12] | buf[13] << 8 | buf[14] << 16 | buf[15] << 24));

	return rc;
}

static int hc32l110_get_mcu_irq(struct cam_star_module_device_info *di,
	u8 *isr_evt_val, u8 *isr_err1_val, u8 *isr_err2_val)
{
	int rc = 0;
	if (!di)
		return -EINVAL;

	rc = i2c_u8_read_byte(di->client, HC32L110_STAR_ISR_EVT, isr_evt_val);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return IRQ_HANDLED;
	}
	rc = i2c_u8_read_byte(di->client, HC32L110_STAR_ISR_ERR1, isr_err1_val);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return IRQ_HANDLED;
	}
	rc = i2c_u8_read_byte(di->client, HC32L110_STAR_ISR_ERR2, isr_err2_val);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return IRQ_HANDLED;
	}
	return rc;
}

int hc32l110_power_on(struct cam_star_module_device_info *di)
{
	int rc = 0;
	u8 val = 0;
	int retry = 0;

	if (!di)
		return -EINVAL;

	if (hc32l110_get_power_state() == POWER_ON) {
		hwlog_info("%s already power_on, return.\n", __func__);
		return rc;
	}
	rc = hc32l110_power_state_machine(di, EJECT);
	if (rc) {
		hwlog_err("%s power_state_machine error\n", __func__);
		return rc;
	}

	hwlog_info("%s success.\n", __func__);

	return 0;
}

static int hc32l110_power_off(struct cam_star_module_device_info *di, enum power_off_state_t state)
{
	int rc = 0;
	u8 val = 0;
	bool boost_status = 0;
	if (!di)
		return -EINVAL;

	if (hc32l110_get_power_state() == POWER_OFF) {
		hwlog_info("%s already power_off, return.\n", __func__);
		return rc;
	}
	if (state == POWEROFF_TRY_MCU_STATE) {
		hwlog_info("%s try mcu state first.\n", __func__);
		rc = hc32l110_power_state_machine(di, RETRACTION);
		if (rc) {
			hwlog_err("%s power_state_machine error\n", __func__);
			return rc;
		}
	} else {
		hwlog_err("invalid power_off state.\n");
		return -EINVAL;
	}

	di->normal_calib_state = NORMAL;
	hwlog_info("%s success.\n", __func__);
	return rc;
}

int hc32l110_set_bootloader(struct cam_star_module_device_info *di)
{
	int rc = 0;
	if (!di)
		return -EINVAL;

	hwlog_info("%s\n", __func__);
	/* add delay for DBC to linkage */
	msleep(di->bootloader_delay);
	/* from POWER_OFF to BOOTLOADER, power sequence 2 */
	rc = hc32l110_power_state_machine(di, TURN_TO_BOOTLOADER);
	if (rc)
		hwlog_err("%s power_state_machine error\n", __func__);

	return rc;
}

int hc32l110_exit_bootloader(struct cam_star_module_device_info *di)
{
	int rc = 0;

	hwlog_info("%s\n", __func__);
	/* from BOOTLOADER to PROBE, power sequence 1 */
	rc = hc32l110_power_state_machine(di, EXT_MATCH_BOOTLOADER_ID);
	if (rc)
		hwlog_err("%s power_state_machine error\n", __func__);

	return rc;
}

static int hc32l110_read_bootloaderid(struct cam_star_module_device_info *di)
{
	int rc;
	u8 ack = 0;
	u8 data[1] = {0};

	/* write ver cmd and wait ack */
	rc = hc32l110_fw_write_cmd(di, HC32L110_FW_MATCHID_CMD);
	if (rc) {
		hwlog_err("hc32l110 not work bootloader mode.\n");
		return -EIO;
	}

	/* get data and wait ack */
	rc = i2c_read_block_without_cmd(di->client, data, 1);
	rc += i2c_read_block_without_cmd(di->client, &ack, 1);
	hwlog_info("ack = 0x%x\n", ack);
	if (ack != HC32L110_FW_ACK_VAL) {
		hwlog_err("bootloader not ack.\n");
		return -EINVAL;
	}
	if (rc) {
		hwlog_err("not work bootloader mode\n");
		return -EIO;
	}

	di->bootloader_id = data[0];
	hwlog_info("hc32l110 bootloader_id: %u, now working on bootloader mode\n", di->bootloader_id);

	return 0;
}

static int hc32l110_wait_for_cali_state_ready(struct cam_star_module_device_info *di,
	int sleep_time, int max_retry_times, u8 *readout_value)
{
	int rc = 0;
	int retry_times = 0;

	do {
		cam_usleep(sleep_time * 1000);
		rc = i2c_u8_read_byte(di->client, HC32L110_CALI_STATE, readout_value);
		if (rc) {
			hwlog_err("%s i2c transfer fail\n", __func__);
			return -EINVAL;
		}
		if (*readout_value & 0x02) {
			hwlog_err("%s CALI_CMD error\n", __func__);
			return -EINVAL;
		}
		hwlog_info("%s retry times: %d\n", __func__, retry_times);
	} while ((retry_times++ < max_retry_times) && (*readout_value & 0x01));

	if (retry_times >= max_retry_times) {
		hwlog_err("%s fail, CPU is busy!\n", __func__);
		return -EINVAL;
	}

	return rc;
}

static int hc32l110_read_log_reg(struct cam_star_module_device_info *di)
{
	int rc = 0;
	int i = 0;
	u8 val = 0;
	u8 cmd[LEN_U8_REG] = { 0 };
	u8 buf[4] = {0};

	for (; i < CAM_SENSORMOTOR_LOG_NUM_MAX; ++i) {
		rc = i2c_u8_read_byte(di->client, HC32L110_STAR_LOG_STATUS, &val);
		if (rc) {
			hwlog_err("%s i2c failed.\n", __func__);
			return rc;
		}
		if (!val) {
			hwlog_info("%s have no log in fifo now.\n", __func__);
			break;
		}

		rc = i2c_u8_write_byte(di->client, HC32L110_STAR_LOG_GET_ONE_DATA, 1);
		if (rc) {
			hwlog_err("%s i2c failed.\n", __func__);
			return rc;
		}

		cmd[0] = HC32L110_STAR_LOG_INFO;
		rc =  i2c_read_block(di->client, cmd, sizeof(cmd), buf, sizeof(buf));
		if (rc) {
			hwlog_err("%s i2c_read_block failed.\n", __func__);
			return rc;
		}

		switch (buf[0]) {
		case MOTOR:
			hwlog_info("%s log index: %d, <motor_log_info> addr: %u, data: %d\n",
				__func__, i, buf[1], (int16_t)(buf[2] & 0xFF | buf[3] << 8));
			break;
		case TMR:
			hwlog_info("%s log index: %d, <tmr_log_info> addr: 0x%x, data: %d\n",
				__func__, i, buf[1], (int16_t)(buf[2] & 0xFF | buf[3] << 8));
			break;
		case HEIGHT:
			hwlog_info("%s log index: %d, <height_log_info> addr: 0x%x, data: %d\n",
				__func__, i, buf[1], (int16_t)(buf[2] & 0xFF | buf[3] << 8));
			break;
		case PERI:
			hwlog_info("%s log index: %d, <peri_log_info> addr: 0x%x, data: 0x%x\n",
				__func__, i, buf[1], buf[2] & 0xFF | buf[3] << 8);
			break;
		case ERROR:
			hwlog_info("%s log index: %d, [ERR_LOG: catalog: 0x%x, index: 0x%x, argument: 0x%x]\n",
				__func__, i, buf[1], buf[2], buf[3]);
			break;
		default:
			hwlog_err("%s invalid log_info: %u\n", __func__, buf[0]);
			break;
		}
	}

	return rc;
}

static void hisp_notify_intf_rpmsg_cb(struct star_notify_intf_t *i,
											struct star_event_t *star_ev)
{
	if (!i || !(i->vtbl) || !(i->vtbl->rpmsg_cb)) {
		hwlog_err("func %s: param is NULL\n", __func__);
		return;
	}

	i->vtbl->rpmsg_cb(i, star_ev);
}

static irqreturn_t hc32l110_irq_handler(int irq, void *info)
{
	struct cam_star_module_device_info *di = (struct cam_star_module_device_info *)info;
	int rc = 0;
	u8 isr_evt_val = 0;
	u8 isr_err1_val = 0;
	u8 isr_err2_val = 0;
	struct star_event_t star_ev;
	if (!di)
		return IRQ_HANDLED;

	if (di->irq == irq) {
		rc = hc32l110_get_mcu_irq(di, &isr_evt_val, &isr_err1_val, &isr_err2_val);
		if (rc) {
			hwlog_err("%s failed.\n", __func__);
			return rc;
		}

		di->isr_err = (isr_evt_val & 0xFF) | (isr_err1_val << 8) | (isr_err2_val << 16);
		hwlog_info("%s isr_evt_val: 0x%02x, isr_err_val: 0x%04x\n", __func__, isr_evt_val, (isr_err1_val << 8) | isr_err2_val);
		star_ev.kind = di->isr_err;
		hisp_notify_intf_rpmsg_cb(&di->notify, &star_ev);

		rc = hc32l110_read_log_reg(di);
		if (rc)
			hwlog_err("%s failed.\n", __func__);

		/* clear int */
		if (isr_evt_val & 0xFD) {
			rc = i2c_u8_write_byte(di->client, HC32L110_STAR_ISR_EVT, isr_evt_val);
			if (rc) {
				hwlog_err("%s i2c failed.\n", __func__);
				return IRQ_HANDLED;
			}
			/* record move_done flag in interrupt handler */
			di->move_done = isr_evt_val & 0x01;
		}
		if (isr_err1_val) {
			rc = i2c_u8_write_byte(di->client, HC32L110_STAR_ISR_ERR1, isr_err1_val);
			if (rc) {
				hwlog_err("%s i2c failed.\n", __func__);
				return IRQ_HANDLED;
			}
		}
		if (isr_err2_val) {
			rc = i2c_u8_write_byte(di->client, HC32L110_STAR_ISR_ERR2, isr_err2_val);
			if (rc) {
				hwlog_err("%s i2c failed.\n", __func__);
				return IRQ_HANDLED;
			}
		}
	}

	return IRQ_HANDLED;
}

static int hc32l110_irq_init(struct cam_star_module_device_info *di)
{
	int rc = 0;
	if (!di)
		return -EINVAL;

	di->irq = gpio_to_irq(di->power_setting.mcu_int_gpio);
	if (di->irq < 0) {
		hwlog_err("%s mcu_int_gpio map to irq fail\n", __func__);
		return -EINVAL;
	}
	hwlog_info("%s hc32l110 irq = %d\n", __func__, di->irq);
	rc = request_threaded_irq(di->irq, NULL, (irq_handler_t)hc32l110_irq_handler,
			IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "hc32l110", (void *)di);
	if (rc) {
		hwlog_err("gpio irq request fail\n");
		di->irq = -1;
		return rc;
	}
	di->int_state = INT_DISABLE;
	disable_irq_nosync(di->irq);
	return 0;
}

int hc32l110_mcu_fw_reg_print(struct cam_star_module_device_info *di)
{
	int rc;
	u8 main_ver = 0;
	u8 sub_ver = 0;
	u8 patch_ver = 0;

	rc = i2c_u8_read_byte(di->client, HC32L110_SW_MAIN_VER, &main_ver);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, HC32L110_SW_SUB_VER, &sub_ver);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, HC32L110_SW_PATCH_VER, &patch_ver);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	hwlog_info("%s: %u.%u.%u\n", __func__, main_ver, sub_ver, patch_ver);
	return rc;
}

static int hc32l110_download_firmware(struct cam_star_module_device_info *di)
{
	int rc = 0;
	int retry = 0;
	if (!di)
		return -EINVAL;
	hwlog_info("enter %s, fw version which is to update is: %d\n", __func__, di->fw_state.fw_ver_id);

	/* PROBE to BOOTLOADER: power sequence 2, now state change to BOOTLOADER */
	rc = hc32l110_power_state_machine(di, TURN_TO_BOOTLOADER);
	if (rc) {
		hwlog_err("%s power_state_machine error\n", __func__);
		return rc;
	}

	for (retry = 0; retry < READ_BOOTLOADERID_RETRY; ++retry) {
		cam_usleep(3000);
		rc = hc32l110_read_bootloaderid(di);
		if (rc == 0)
			break;
		else
			hwlog_err("hc32l110_read_bootloaderid failed, retry times: %d.\n", retry);
	}
	if (retry >= READ_BOOTLOADERID_RETRY) {
		hwlog_err("hc32l110_read_bootloaderid failed, retry times: %d.\n", retry);
		cam_sensormotor_dsm_set_bootloader_err(di->dsm_client);
		goto done;
	}

	rc = hc32l110_fw_update_check(di);
	if (rc) {
		hwlog_err("hc32l110_fw_update_check failed.\n");
		cam_sensormotor_dsm_download_fw_err(di->dsm_client);
	}

done:
	/* exit BOOTLOADER and POWER_OFF: power sequence 3, now state change to POWER_OFF */
	rc = hc32l110_power_state_machine(di, DOWNLOAD_FW);
	if (rc)
		hwlog_err("%s power_state_machine error\n", __func__);

	return rc;
}

static int hc32l110_stop(struct cam_star_module_device_info *di)
{
	int rc = 0;
	if (!di)
		return -EINVAL;
	if (hc32l110_get_power_state() != POWER_ON) {
		hwlog_err("hc32l110 is not powered on, please power on first.\n");
		return -EINVAL;
	}

	rc = i2c_u8_write_byte(di->client, HC32L110_MOTOR_CTL, STOP_STEPPING);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	return rc;
}

static int hc32l110_set_code(struct cam_star_module_device_info *di, u16 code)
{
	int rc = 0;
	u8 val = 0;
	u8 buf[LEN_U16_DAT] = {0};
	if (!di)
		return -EINVAL;

	cam_usleep(6000);
	/* wait HC32L110_MOTOR_CSTATE is idle */
	rc = i2c_u8_read_byte(di->client, HC32L110_MOTOR_CSTATE, &val);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	if (val) {
		hwlog_info("%s stop before set_code\n", __func__);
		hc32l110_stop(di);
	}

	buf[0] = code & 0xff;
	buf[1] = code >> 8;
	rc = i2c_u8_write_byte(di->client, HC32L110_MOTOR_TARGET_POS_LOW, buf[0]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc += i2c_u8_write_byte(di->client, HC32L110_MOTOR_TARGET_POS_HIGH, buf[1]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	di->mcu_state.target_pos = code;
	return rc;
}

static int hc32l110_set_speed(struct cam_star_module_device_info *di, u16 speed)
{
	int rc = 0;
	u8 buf[LEN_U16_DAT] = {0};
	if (!di)
		return -EINVAL;

	buf[0] = speed & 0xff;
	buf[1] = speed >> 8;
	rc = i2c_u8_write_byte(di->client, HC32L110_MOTOR_SPEED_LOW, buf[0]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc += i2c_u8_write_byte(di->client, HC32L110_MOTOR_SPEED_HIGH, buf[1]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	di->mcu_state.motor_speed = speed;
	return rc;
}

static int hc32l110_read_mcu_key_data(struct cam_star_module_device_info *di)
{
	int rc = 0;
	u8 cmd[LEN_U8_REG] = { 0 };
	u8 buf[16] = {0};
	if (!di)
		return -EINVAL;

	cmd[0] = HC32L110_START_TMR_VOLT_L;
	rc =  i2c_read_block(di->client, cmd, sizeof(cmd), buf, sizeof(buf));
	if (rc) {
		hwlog_err("%s i2c_read_block failed.\n", __func__);
		return rc;
	}
	hwlog_info("%s key_data: %d, %d, %d, %d%%, %d, %d, 0x%02x, 0x%02x, %d\n", __func__,
		(int16_t)(buf[0] | buf[1] << 8), (int16_t)(buf[2] | buf[3] << 8),
		(int16_t)(buf[4] | buf[5] << 8), (buf[6] | buf[7] << 8) / 100,
		(int16_t)(buf[8] | buf[9] << 8), (int16_t)(buf[14] | buf[15] << 8),
		buf[10], buf[11], buf[12] | buf[13] << 8);

	return rc;
}

static int hc32l110_start(struct cam_star_module_device_info *di)
{
	int rc = 0;
	if (!di)
		return -EINVAL;
	if (hc32l110_get_power_state() != POWER_ON) {
		hwlog_err("hc32l110 is not powered on, please power on first.\n");
		return -EINVAL;
	}

	if (di->normal_calib_state != NORMAL) {
		hwlog_err("hc32l110 is not in normal mode!\n");
		return -EINVAL;
	}

	rc = hc32l110_read_mcu_key_data(di);
	if (rc) {
		hwlog_err("%s hc32l110_read_mcu_key_data failed.\n", __func__);
		return rc;
	}

	rc = i2c_u8_write_byte(di->client, HC32L110_MOTOR_CTL, START_STEPPING);
	if (rc)
		hwlog_err("%s i2c failed.\n", __func__);

	return rc;
}

static int hc32l110_get_real_height(struct cam_star_module_device_info *di, u16 *real_code)
{
	int rc = 0;
	u8 cmd[LEN_U8_REG] = { 0 };
	u8 buf[LEN_U16_DAT] = { 0 };
	if (!di)
		return -EINVAL;

	cmd[0] = HC32L110_CURR_REAL_HEIGHT_LOW;
	rc =  i2c_read_block(di->client, cmd, sizeof(cmd), buf, sizeof(buf));
	if (rc) {
		hwlog_err("%s i2c_read_block failed.\n", __func__);
		return rc;
	}
	*real_code = buf[0] | (buf[1] << 8);
	return rc;
}

static int hc32l110_set_calib_mode(struct cam_star_module_device_info *di, enum calib_mode_t state)
{
	int rc = 0;
	u8 val = 0xFF;
	int retry = 0;
	u8 buf[LEN_U8_DAT] = {0};
	if (!di)
		return -EINVAL;

	rc = i2c_u8_read_byte(di->client, HC32L110_STAR_STATUS, &val);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	hwlog_info("%s star_status: %u\n", __func__, val);
	if ((val / 2) == state) {
		hwlog_info("%s normal_caib_state need not change, return.\n", __func__);
		return rc;
	}
	/* set calib mode util HC32L110_MOTOR_CSTATE is idle */
	do {
		i2c_u8_read_byte(di->client, HC32L110_MOTOR_CSTATE, &val);
		msleep(100);
		hwlog_info("%s retry times: %d\n", __func__, retry);
		retry++;
	} while ((retry < I2C_MAX_WAIT_RETRY) && (val != 0));

	if (retry >= I2C_MAX_WAIT_RETRY) {
		hwlog_err("%s fail, CPU is busy!\n", __func__);
		return -EINVAL;
	}

	buf[0] = state & 0x01;
	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_CTL, buf[0]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	do {
		i2c_u8_read_byte(di->client, HC32L110_STAR_STATUS, &val);
		cam_usleep(5000);
		hwlog_info("%s retry times: %d, val: %u\n", __func__, retry, val);
		retry++;
	} while ((retry < 30) && (val != (state * 2)));

	if (retry >= 30) {
		hwlog_err("%s fail, CPU is busy!\n", __func__);
		return -EINVAL;
	}

	di->normal_calib_state = state;
	return rc;
}

static int hc32l110_calib_set_step(struct cam_star_module_device_info *di, int step)
{
	int rc = 0;
	u8 buf[LEN_U16_DAT] = {0};
	if (!di)
		return -EINVAL;

	hwlog_info("%s step: %d\n", __func__, step);
	if (step < 0) {
		di->mcu_state.step_backward_flag = 1;
		step = abs(step);
	} else {
		di->mcu_state.step_backward_flag = 0;
	}

	buf[0] = step & 0xff;
	buf[1] = step >> 8;
	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_STEP_CNT_SET_LOW, buf[0]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc += i2c_u8_write_byte(di->client, HC32L110_CALI_STEP_CNT_SET_HIGH, buf[1]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	di->mcu_state.target_step = step;

	return rc;
}

static int hc32l110_calib_set_speed(struct cam_star_module_device_info *di, u16 speed)
{
	int rc = 0;
	u8 buf[LEN_U16_DAT] = {0};
	if (!di)
		return -EINVAL;

	buf[0] = speed & 0xff;
	buf[1] = speed >> 8;
	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_SPEED_CNT_SET_LOW, buf[0]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc += i2c_u8_write_byte(di->client, HC32L110_CALI_SPEED_CNT_SET_HIGH, buf[1]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	di->mcu_state.motor_speed = speed;
	return rc;
}

static int hc32l110_calib_start(struct cam_star_module_device_info *di)
{
	int rc = 0;
	u8 val = 0xFF;
	if (!di)
		return -EINVAL;
	if (hc32l110_get_power_state() != POWER_ON) {
		hwlog_err("hc32l110 is not powered on, please power on first.\n");
		return -EINVAL;
	}

	if (di->mcu_state.step_backward_flag == 0) {
		/* step forward */
		rc = i2c_u8_write_byte(di->client, HC32L110_CALI_CMD, HC32L110_CALI_MOVE_FORWARD_STEP);
		if (rc) {
			hwlog_err("%s i2c failed.\n", __func__);
			return rc;
		}
	} else if (di->mcu_state.step_backward_flag == 1) {
		/* step backward */
		rc = i2c_u8_write_byte(di->client, HC32L110_CALI_CMD, HC32L110_CALI_MOVE_BACKWARD_STEP);
		if (rc) {
			hwlog_err("%s i2c failed.\n", __func__);
			return rc;
		}
	} else {
		hwlog_err("invalid step_backward_flag.\n");
		return -EINVAL;
	}

	rc = hc32l110_wait_for_cali_state_ready(di, 100, 30, &val);
	if (rc) {
		hwlog_err("%s wait_for_cali_state_ready fail\n", __func__);
		return rc;
	}

	rc = hc32l110_read_mcu_key_data(di);
	if (rc)
		hwlog_err("%s hc32l110_read_mcu_key_data failed.\n", __func__);

	return rc;
}

static int hc32l110_calib_get_real_height(struct cam_star_module_device_info *di, int16_t *real_code)
{
	int rc = 0;
	u8 cmd[LEN_U8_REG] = { 0 };
	u8 buf[LEN_U16_DAT] = { 0 };
	u8 val = 0xFF;
	if (!di)
		return -EINVAL;

	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_CMD, HC32L110_CALI_GET_CURRENT_HEIGHT);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	rc = hc32l110_wait_for_cali_state_ready(di, 5, 5, &val);
	if (rc) {
		hwlog_err("%s wait_for_cali_state_ready fail\n", __func__);
		return rc;
	}

	cmd[0] = HC32L110_CALI_REAL_HEIGHT_LOW;
	rc =  i2c_read_block(di->client, cmd, sizeof(cmd), buf, sizeof(buf));
	if (rc) {
		hwlog_err("%s i2c_read_block failed.\n", __func__);
		return rc;
	}

	*real_code = buf[0] | (buf[1] << 8);
	hwlog_info("%s real_height: %d", __func__, *real_code);

	return rc;
}

static int hc32l110_calib_get_real_tmr(struct cam_star_module_device_info *di, int16_t *real_code)
{
	int rc = 0;
	u8 cmd[LEN_U8_REG] = { 0 };
	u8 buf[LEN_U16_DAT] = { 0 };
	u8 val = 0xFF;
	if (!di)
		return -EINVAL;

	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_CMD, HC32L110_CALI_GET_CURRENT_TMR);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	rc = hc32l110_wait_for_cali_state_ready(di, 5, 5, &val);
	if (rc) {
		hwlog_err("%s wait_for_cali_state_ready fail\n", __func__);
		return rc;
	}

	cmd[0] = HC32L110_CALI_TMR_DATA_LOW;
	rc =  i2c_read_block(di->client, cmd, sizeof(cmd), buf, sizeof(buf));
	if (rc) {
		hwlog_err("%s i2c_read_block failed.\n", __func__);
		return rc;
	}

	*real_code = buf[0] | (buf[1] << 8);
	hwlog_info("%s real_tmr: %d", __func__, *real_code);
	return rc;
}

int hc32l110_calib_write_flash_data(struct cam_star_module_device_info *di, u16 data)
{
	int rc = 0;
	u8 val = 0xFF;
	u8 flash_readback_l = 0;
	u8 flash_readback_h = 0;

	/* step 2: send flash addr */
	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_FLASH_DATA_ADDR_L,
		di->calib_otp.flash_offset & 0xff);
	if (rc) {
		hwlog_err("%s write flash addr-low i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, HC32L110_CALI_FLASH_DATA_ADDR_L, &flash_readback_l);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_FLASH_DATA_ADDR_H,
		di->calib_otp.flash_offset >> 8);
	if (rc) {
		hwlog_err("%s write flash addr-high i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, HC32L110_CALI_FLASH_DATA_ADDR_H, &flash_readback_h);
	hwlog_info("%s flash_offset readback: %u", __func__, flash_readback_l & 0xFF | flash_readback_h << 8);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	di->calib_otp.flash_offset += 2;
	hwlog_info("%s flash_offset after this write: %u",
		__func__, di->calib_otp.flash_offset);

	/* step 3: send data to write to flash */
	hwlog_info("%s write_data: %lu", __func__, data);
	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_FLASH_DATA_VALUE_L,
		data & 0xff);
	if (rc) {
		hwlog_err("%s write flash data low i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, HC32L110_CALI_FLASH_DATA_VALUE_L, &flash_readback_l);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_FLASH_DATA_VALUE_H,
		data >> 8);
	if (rc) {
		hwlog_err("%s write flash data high i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, HC32L110_CALI_FLASH_DATA_VALUE_H, &flash_readback_h);
	hwlog_info("%s flash_data readback: %u", __func__, flash_readback_l & 0xFF | flash_readback_h << 8);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	/* step 4: write to MCU RAM */
	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_CMD, 0x32);
	if (rc) {
		hwlog_err("%s write to MCU RAM i2c failed.\n", __func__);
		return rc;
	}
	
	/* step 5: wait for MCU ready */
	rc = hc32l110_wait_for_cali_state_ready(di, 1, 10, &val);
	if (rc)
		hwlog_err("%s wait_for_cali_state_ready fail\n", __func__);

	return rc;
}

static int hc32l110_calib_crc_check(struct cam_star_module_device_info *di)
{
	int rc = 0;
	u8 val = 0xFF;
	int i = 0;
	int offset = 0;
	u8 buf_cal[2] = {0};
	u8 buf_rec[2] = {0};
	hwlog_info("enter %s", __func__);
	if (!di)
		return -EINVAL;

	/* start calculating flash CRC */
	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_CMD, 0x35);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	/* wait for MCU ready */
	rc = hc32l110_wait_for_cali_state_ready(di, 1, 10, &val);
	if (rc && (!(val & 0x02))) {
		hwlog_err("%s wait_for_cali_state_ready fail\n", __func__);
		return rc;
	}

	/* check CRC */
	rc = i2c_u8_read_byte(di->client, HC32L110_CALI_CRC_CALC_VALUE_L, &buf_cal[0]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, HC32L110_CALI_CRC_CALC_VALUE_H, &buf_cal[1]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, HC32L110_CALI_CRC_REC_VALUE_L, &buf_rec[0]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, HC32L110_CALI_CRC_REC_VALUE_H, &buf_rec[1]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	if (buf_cal[0] == buf_rec[0] && buf_cal[1] == buf_rec[1]) {
		hwlog_info("%s CRC check success! mcu_cal_crc: 0x%x\n",
			__func__, buf_cal[0] | buf_cal[1] << 8);
		di->calib_otp.crc_readout = buf_rec[0] | buf_rec[1] << 8;
	} else {
		di->crc_check = CRC_FAIL;
		hwlog_err("%s CRC check failed! mcu_cal_crc: 0x%x, flash_crc: 0x%x\n",
			__func__, buf_cal[0] | buf_cal[1] << 8, buf_rec[0] | buf_rec[1] << 8);
	}

	return rc;
}

static int hc32l110_calib_erase_flash(struct cam_star_module_device_info *di)
{
	int rc = 0;
	u8 val = 0xFF;
	hwlog_info("enter %s", __func__);
	if (!di)
		return -EINVAL;

	/* step 1: enter calib mode and release write protect */
	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_CTL,
		CALIB_AND_RELEASE_WRITE_PROTECT);
	if (rc) {
		hwlog_err("%s enter calib mode i2c failed.\n", __func__);
		return rc;
	}

	/* step 2: erase flash */
	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_CMD, 0x30);
	if (rc) {
		hwlog_err("%s erase flash i2c failed.\n", __func__);
		return rc;
	}

	/* add wait for MCU ready */
	rc = hc32l110_wait_for_cali_state_ready(di, 10, 5, &val);
	if (rc)
		hwlog_err("%s wait_for_cali_state_ready fail\n", __func__);

	return rc;
}

static int hc32l110_calib_reload_flash(struct cam_star_module_device_info *di)
{
	int rc = 0;
	u8 val = 0xFF;
	/* step 5: open writing protect */
	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_CTL, 0x01);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	/* step 6: force MCU to reload flash data */
	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_CMD, 0x3F);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	/* step 7: wait for MCU ready */
	rc = hc32l110_wait_for_cali_state_ready(di, 10, 5, &val);
	if (rc || (val & 0x04)) {
		hwlog_err("%s flash reload fail\n", __func__);
		rc = hc32l110_calib_crc_check(di);
		if (rc)
			hwlog_err("%s i2c failed.\n", __func__);
		return -EINVAL;
	} else {
		hwlog_info("%s flash reload success\n", __func__);
	}
	return rc;
}

static int hc32l110_calib_write_eeprom_data(struct cam_star_module_device_info *di, u16 data)
{
	int rc = 0;
	u8 val = 0xFF;

	/* send eeprom addr */
	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_E2PROM_DATA_ADDR_L,
		di->calib_otp.flash_offset & 0xff);
	if (rc) {
		hwlog_err("%s write flash addr-low i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_E2PROM_DATA_ADDR_H,
		di->calib_otp.flash_offset >> 8);
	if (rc) {
		hwlog_err("%s write flash addr-high i2c failed.\n", __func__);
		return rc;
	}

	di->calib_otp.flash_offset += 2;
	hwlog_info("%s flash_offset after this write: %u",
		__func__, di->calib_otp.flash_offset);

	/* send data to write to eeprom */
	hwlog_info("%s write_data: %lu", __func__, data);
	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_E2PROM_DATA_VALUE_L,
		data & 0xff);
	if (rc) {
		hwlog_err("%s write flash data low i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_E2PROM_DATA_VALUE_H,
		data >> 8);
	if (rc) {
		hwlog_err("%s write flash data high i2c failed.\n", __func__);
		return rc;
	}

	/* write to MCU RAM */
	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_CMD, 0x44);
	if (rc) {
		hwlog_err("%s write to MCU RAM i2c failed.\n", __func__);
		return rc;
	}
	
	/* wait for MCU ready */
	rc = hc32l110_wait_for_cali_state_ready(di, 1, 10, &val);
	if (rc)
		hwlog_err("%s wait_for_cali_state_ready fail\n", __func__);

	return rc;
}

static int hc32l110_calib_write_backup_eeprom_data(struct cam_star_module_device_info *di)
{
	int rc = 0;
	int i = 0;
	u8 val = 0xFF;

	di->calib_otp.flash_offset = 0x400;
	/* enter calib mode and release write protect */
	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_CTL,
		CALIB_AND_RELEASE_WRITE_PROTECT);
	if (rc) {
		hwlog_err("%s enter calib mode i2c failed.\n", __func__);
		return rc;
	}

	/* add wait for MCU ready */
	rc = hc32l110_wait_for_cali_state_ready(di, 10, 5, &val);
	if (rc) {
		hwlog_err("%s wait_for_cali_state_ready fail\n", __func__);
		return rc;
	}

	/* set data num_of_pair */
	rc = hc32l110_calib_write_eeprom_data(di, di->calib_otp.calib_info.num_of_pair);
	if (rc) {
		hwlog_err("%s set data num_of_pair failed.\n", __func__);
		return rc;
	}

	/* set data system_accuracy_err */
	rc = hc32l110_calib_write_eeprom_data(di, di->calib_otp.calib_info.system_accuracy_err);
	if (rc) {
		hwlog_err("%s set data system_accuracy_err failed.\n", __func__);
		return rc;
	}

	/* set data distance and corresponding tmr data */
	for (i = 0; i < CALIB_INFO_NUM_OF_PAIR; ++i) {
		/* set data: distance */
		rc = hc32l110_calib_write_eeprom_data(di, di->calib_otp.calib_info.dis_tmr_pair[i].dis);
		if (rc) {
			hwlog_err("%s write distance eeprom failed.\n", __func__);
			return rc;
		}

		/* set data: tmr */
		rc = hc32l110_calib_write_eeprom_data(di, di->calib_otp.calib_info.dis_tmr_pair[i].tmr);
		if (rc) {
			hwlog_err("%s write tmr eeprom failed.\n", __func__);
			return rc;
		}
	}
	/* step 3: set calib_add_info data */
	for (i = 0; i < NUM_OF_CALIB_ADD_INFO; ++i) {
		/* set data */
		hwlog_info("%s index: %d, calib_add_info: %u", __func__, i, di->calib_otp.calib_add_info[i]);
		rc = hc32l110_calib_write_eeprom_data(di, di->calib_otp.calib_add_info[i]);
		if (rc) {
			hwlog_err("%s write distance eeprom failed.\n", __func__);
			return rc;
		}
	}

	return rc;
}

static int hc32l110_calib_set_info(struct cam_star_module_device_info *di)
{
	int rc = 0;
	int i = 0;
	u8 val = 0xFF;
	hwlog_info("enter %s", __func__);
	if (!di)
		return -EINVAL;

	di->calib_otp.flash_offset = 0;
	rc = hc32l110_calib_erase_flash(di);
	if (rc) {
		hwlog_err("%s hc32l110_calib_erase_flash failed.\n", __func__);
		return rc;
	}

	/* step 3: set data num_of_pair */
	di->calib_otp.calib_info.num_of_pair /= 2; // 2 only for double_routine
	rc = hc32l110_calib_write_flash_data(di, di->calib_otp.calib_info.num_of_pair);
	if (rc) {
		hwlog_err("%s set data num_of_pair failed.\n", __func__);
		return rc;
	}

	/* step 4: set data system_accuracy_err */
	rc = hc32l110_calib_write_flash_data(di, di->calib_otp.calib_info.system_accuracy_err);
	if (rc) {
		hwlog_err("%s set data system_accuracy_err failed.\n", __func__);
		return rc;
	}

	/* step 5: set data distance and corresponding tmr data */
	for (i = 0; i < CALIB_INFO_NUM_OF_PAIR; ++i) {
		/* set data: distance */
		rc = hc32l110_calib_write_flash_data(di, di->calib_otp.calib_info.dis_tmr_pair[i].dis);
		if (rc) {
			hwlog_err("%s write distance flash failed.\n", __func__);
			return rc;
		}

		/* set data: tmr */
		rc = hc32l110_calib_write_flash_data(di, di->calib_otp.calib_info.dis_tmr_pair[i].tmr);
		if (rc) {
			hwlog_err("%s write tmr flash failed.\n", __func__);
			return rc;
		}
	}

	return rc;
}

static int hc32l110_calib_set_add_info(struct cam_star_module_device_info *di)
{
	int rc = 0;
	int i = 0;
	u8 val = 0xFF;
	hwlog_info("enter %s", __func__);
	if (!di)
		return -EINVAL;

	/* step 3: set calib_add_info data */
	for (i = 0; i < NUM_OF_CALIB_ADD_INFO; ++i) {
		/* set data */
		hwlog_info("%s index: %d, calib_add_info: %u", __func__, i, di->calib_otp.calib_add_info[i]);
		rc = hc32l110_calib_write_flash_data(di, di->calib_otp.calib_add_info[i]);
		if (rc) {
			hwlog_err("%s write distance flash failed.\n", __func__);
			return rc;
		}
	}

	rc = hc32l110_calib_reload_flash(di);
	if (rc) {
		hwlog_err("%s hc32l110_calib_reload_flash failed.\n", __func__);
		return rc;
	}

	rc = hc32l110_calib_write_backup_eeprom_data(di);
	if (rc) {
		hwlog_err("%s hc32l110_calib_write_backup_eeprom_data failed.\n", __func__);
		return rc;
	}

	return rc;
}

int hc32l110_calib_read_flash(struct cam_star_module_device_info *di,
	u16 offset, u16 *read_value)
{
	int rc = 0;
	u8 val = 0xFF;
	u8 calib_mode;
	u8 readback_l = 0;
	u8 readback_h = 0;
	int i = 0;
	u8 buf[2] = {0};
	hwlog_info("enter %s", __func__);
	if (!di)
		return -EINVAL;

	/* read calib mode reg */
	rc = i2c_u8_read_byte(di->client, HC32L110_CALI_CTL, &calib_mode);
	hwlog_info("%s calib_mode: %u", __func__, calib_mode & 0x01);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	/* flash addr to read */
	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_FLASH_DATA_ADDR_L, offset & 0xff);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, HC32L110_CALI_FLASH_DATA_ADDR_L, &readback_l);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_FLASH_DATA_ADDR_H, offset >> 8);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, HC32L110_CALI_FLASH_DATA_ADDR_H, &readback_h);
	hwlog_info("%s flash_offset readback: %u", __func__, readback_l & 0xFF | readback_h << 8);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	/* flash read cmd */
	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_CMD, 0x31);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	/* wait for MCU ready */
	rc = hc32l110_wait_for_cali_state_ready(di, 1, 10, &val);
	if (rc) {
		hwlog_err("%s wait_for_cali_state_ready fail\n", __func__);
		return rc;
	}

	/* get value */
	rc = i2c_u8_read_byte(di->client, HC32L110_CALI_FLASH_DATA_VALUE_L, &buf[0]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, HC32L110_CALI_FLASH_DATA_VALUE_H, &buf[1]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	*read_value = buf[0] | buf[1] << 8;
	hwlog_info("%s read_value: %u", __func__, *read_value);

	return rc;
}

static int hc32l110_calib_get_info(struct cam_star_module_device_info *di, u16 *read_value, int *size_read_value)
{
	int rc = 0;
	int i = 0;
	int size = 0;
	u16 offset = 0;
	u16 *readout = NULL;
	hwlog_info("enter %s", __func__);
	if (!di)
		return -EINVAL;

	size = sizeof(struct calib_info_t) / sizeof(u16);

	for (i = 0; i < size; ++i) {
		rc = hc32l110_calib_read_flash(di, offset, read_value);
		if (rc) {
			hwlog_err("%s read_flash fail\n");
			return rc;
		}
		hwlog_info("%s index: %d, read_flash: %u, read_value: %p\n", __func__, i, *read_value, read_value);
		offset += 2;
		read_value += 1;
	}

	*size_read_value = size;
	hwlog_info("%s calib_info size: %d\n", __func__, *size_read_value);

	return rc;
}

static int hc32l110_calib_read_eeprom(struct cam_star_module_device_info *di,
	u16 *readout_value)
{
	int rc = 0;
	u8 val = 0xFF;
	u8 buf[2] = {0};
	hwlog_info("enter %s", __func__);
	if (!di)
		return -EINVAL;

	/* eeprom addr to read */
	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_E2PROM_DATA_ADDR_L, di->calib_otp.flash_offset & 0xff);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_E2PROM_DATA_ADDR_H, di->calib_otp.flash_offset >> 8);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	/* flash read cmd */
	rc = i2c_u8_write_byte(di->client, HC32L110_CALI_CMD, 0x42);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	/* wait for MCU ready */
	rc = hc32l110_wait_for_cali_state_ready(di, 1, 10, &val);
	if (rc) {
		hwlog_err("%s wait_for_cali_state_ready fail\n", __func__);
		return rc;
	}

	/* get value */
	rc = i2c_u8_read_byte(di->client, HC32L110_CALI_E2PROM_DATA_VALUE_L, &buf[0]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc = i2c_u8_read_byte(di->client, HC32L110_CALI_E2PROM_DATA_VALUE_H, &buf[1]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	*readout_value = buf[0] | buf[1] << 8;
	hwlog_info("%s readout_value: %u", __func__, *readout_value);

	return rc;
}

static int hc32l110_calib_read_calib_data_type(struct cam_star_module_device_info *di,
	u16 offset, u16 *read_value)
{
	int rc = 0;
	if (!di)
		return -EINVAL;

	rc = hc32l110_calib_read_flash(di, offset, read_value);
	if (rc) {
		hwlog_err("%s read_flash fail\n", __func__);
		return rc;
	}
	hwlog_info("%s calib_data_type addr: 0x%x, calib_data_type: %u\n", __func__, offset, *read_value);

	return rc;
}

static void hc32l110_crc_check(u16 data,  u16 *crc)
{
	int rc = 0;
	int j = 0;
	*crc ^= (u8)data;
	for (j = 0; j < 8; j++) {
		if ((*crc & 0x01) != 0) {
			*crc >>= 1;
			*crc ^= 0xA001;
		} else {
			*crc >>= 1;
		}
	}

	*crc ^= (u8)(data >> 8);
	for (j = 0; j < 8; j++) {
		if ((*crc & 0x01) != 0) {
			*crc >>= 1;
			*crc ^= 0xA001;
		} else
		{
			*crc >>= 1;
		}
	}
}

static int hc32l110_calib_write_flash_data_from_eeprom_single_routine(
	struct cam_star_module_device_info *di)
{
	int rc = 0;
	int i = 0;
	u16 num_of_pair = 0;
	u16 eeprom_readout_value = 0;
	u16 data_length = 0;
	int tmp_num = 0;
	u16 crc = 0xFFFF;
	u16 eeprom_data_length = 0;
	u16 dis_tmr_point72_offset = 0x120;
	u16 calib_data_type_offset = 0x126;
	u16 *eeprom_data = NULL;
	eeprom_data = kmalloc(36 * 2, GFP_KERNEL);
	if (eeprom_data == NULL) {
		hwlog_err("%s kmalloc error!\n", __func__);
		return NULL;
	}
	/* read num_of_pair from eeprom data */
	if (di->maintenance_state == REPAIRED)
		di->calib_otp.flash_offset = 0;
	else
		di->calib_otp.flash_offset = 0x0400;

	rc = hc32l110_calib_read_eeprom(di, &num_of_pair);
	if (rc) {
		hwlog_err("hc32l110_calib_read_eeprom failed\n", __func__);
		kfree(eeprom_data);
		eeprom_data = NULL;
		return rc;
	}
	/* routine 1: num_of_pair + system_acc + num_of_pair dis and tmr */
	data_length = (num_of_pair + 1) * 2;
	for (i = 0; i < data_length; ++i) {
		/* read eeprom data */
		rc = hc32l110_calib_read_eeprom(di, &eeprom_readout_value);
		if (rc) {
			hwlog_err("hc32l110_calib_read_eeprom failed\n", __func__);
			kfree(eeprom_data);
			eeprom_data = NULL;
			return rc;
		}
		if (i > 1) {
			eeprom_data[eeprom_data_length] = eeprom_readout_value;
			eeprom_data_length++;
		}
		hc32l110_crc_check(eeprom_readout_value,  &crc);
		/* write flash data */
		rc = hc32l110_calib_write_flash_data(di, eeprom_readout_value);
		if (rc) {
			hwlog_err("%s hc32l110_calib_write_flash_data failed.\n", __func__);
			kfree(eeprom_data);
			eeprom_data = NULL;
			return rc;
		}
	}
	/* routine 2: copy num_of_pair dis and tmr to make double routine */
	for (i = 0; i < eeprom_data_length; ++i) {
		hc32l110_crc_check(eeprom_data[i], &crc);
		/* write flash data */
		rc = hc32l110_calib_write_flash_data(di, eeprom_data[i]);
		if (rc) {
			hwlog_err("%s hc32l110_calib_write_flash_data failed.\n", __func__);
			kfree(eeprom_data);
			eeprom_data = NULL;
			return rc;
		}
	}

	/* Fill in 0xFF to satisfy a multiple of 4 */
	data_length += eeprom_data_length;
	for (i = 0; i < (4 - (data_length % 4)); ++i) {
		/* write flash data */
		rc = hc32l110_calib_write_flash_data(di, 0xFFFF);
		if (rc) {
			hwlog_err("%s hc32l110_calib_write_flash_data failed.\n", __func__);
			kfree(eeprom_data);
			eeprom_data = NULL;
			return rc;
		}
	}
	if (di->maintenance_state != REPAIRED)
		dis_tmr_point72_offset = 0x520;

	if (di->calib_otp.flash_offset <= dis_tmr_point72_offset) {
		di->calib_otp.flash_offset = dis_tmr_point72_offset;
		for (i = 0; i < 2; ++i) {
			/* write flash data */
			rc = hc32l110_calib_write_flash_data(di, 0xFFFF);
			if (rc) {
				hwlog_err("%s hc32l110_calib_write_flash_data failed.\n", __func__);
				kfree(eeprom_data);
				eeprom_data = NULL;
				return rc;
			}
		}
	}
	/* write CRC to flash data */
	rc = hc32l110_calib_write_flash_data(di, crc);
	if (rc) {
		hwlog_err("%s hc32l110_calib_write_flash_data failed.\n", __func__);
		kfree(eeprom_data);
		eeprom_data = NULL;
		return rc;
	}
	/* write calib_data_type to flash data */
	if (di->maintenance_state != REPAIRED)
		calib_data_type_offset = 0x526;
	di->calib_otp.flash_offset = calib_data_type_offset;
	rc = hc32l110_calib_write_flash_data(di, 2);
	if (rc) {
		hwlog_err("%s hc32l110_calib_write_flash_data failed.\n", __func__);
		kfree(eeprom_data);
		eeprom_data = NULL;
		return rc;
	}

	kfree(eeprom_data);
	eeprom_data = NULL;
	return rc;
}

static int hc32l110_calib_write_flash_data_from_eeprom_double_routine(
	struct cam_star_module_device_info *di)
{
	int rc = 0;
	int i = 0;
	u16 num_of_pair = 0;
	u16 eeprom_readout_value = 0;
	u16 data_length = 0;
	u16 dis_tmr_point72_offset = 0x120;
	u16 crc_data_offset = 0x124;
	u16 calib_data_type_offset = 0x126;
	/* read num_of_pair from eeprom data */
	if (di->maintenance_state != REPAIRED)
		di->calib_otp.flash_offset = 0x0400;
	else
		di->calib_otp.flash_offset = 0;
	rc = hc32l110_calib_read_eeprom(di, &num_of_pair);
	if (rc) {
		hwlog_err("hc32l110_calib_read_eeprom failed\n", __func__);
		return rc;
	}
	data_length = (num_of_pair * 2) * 2 + 2; // num_of_pair means single routine pair num
	for (i = 0; i < data_length; ++i) {
		/* read eeprom data */
		rc = hc32l110_calib_read_eeprom(di, &eeprom_readout_value);
		if (rc) {
			hwlog_err("hc32l110_calib_read_eeprom failed\n", __func__);
			return rc;
		}
		/* write flash data */
		rc = hc32l110_calib_write_flash_data(di, eeprom_readout_value);
		if (rc) {
			hwlog_err("%s hc32l110_calib_write_flash_data failed.\n", __func__);
			return rc;
		}
	}

	/* Fill in 0xFF to satisfy a multiple of 4 */
	for (i = 0; i < (4 - (data_length % 4)); ++i) {
		/* write flash data */
		rc = hc32l110_calib_write_flash_data(di, 0xFFFF);
		if (rc) {
			hwlog_err("%s hc32l110_calib_write_flash_data failed.\n", __func__);
			return rc;
		}
	}
	if (di->maintenance_state != REPAIRED)
		dis_tmr_point72_offset = 0x520;
	if (di->calib_otp.flash_offset <= dis_tmr_point72_offset) {
		di->calib_otp.flash_offset = dis_tmr_point72_offset;
		for (i = 0; i < 2; ++i) {
			/* write flash data */
			rc = hc32l110_calib_write_flash_data(di, 0xFFFF);
			if (rc) {
				hwlog_err("%s hc32l110_calib_write_flash_data failed.\n", __func__);
				return rc;
			}
		}
	}

	/* read CRC from eeprom data */
	if (di->maintenance_state != REPAIRED)
		crc_data_offset = 0x524;
	di->calib_otp.flash_offset = crc_data_offset;
	rc = hc32l110_calib_read_eeprom(di, &eeprom_readout_value);
	if (rc) {
		hwlog_err("hc32l110_calib_read_eeprom failed\n", __func__);
		return rc;
	}
	/* write CRC to flash data */
	rc = hc32l110_calib_write_flash_data(di, eeprom_readout_value);
	if (rc) {
		hwlog_err("%s hc32l110_calib_write_flash_data failed.\n", __func__);
		return rc;
	}
	/* write calib_data_type to flash data */
	if (di->maintenance_state != REPAIRED)
		calib_data_type_offset = 0x526;
	di->calib_otp.flash_offset = calib_data_type_offset;
	rc = hc32l110_calib_write_flash_data(di, 2);
	if (rc) {
		hwlog_err("%s hc32l110_calib_write_flash_data failed.\n", __func__);
		return rc;
	}
	return rc;
}

static int hc32l110_calib_write_flash_data_from_eeprom(struct cam_star_module_device_info *di)
{
	int rc = 0;
	u16 eeprom_readout_value = 0;
	u16 double_routine_flag = 0;
	u16 data_length = 0;
	u16 use_dual_routine_offset = 0x128;
	int i = 0;

	/* erase flash data */
	rc = hc32l110_calib_erase_flash(di);
	if (rc) {
		hwlog_err("%s hc32l110_calib_erase_flash failed.\n", __func__);
		return rc;
	}
	/* read double_routine_flag from eeprom data */
	if (di->maintenance_state != REPAIRED)
		use_dual_routine_offset = 0x528;
	di->calib_otp.flash_offset = use_dual_routine_offset;
	rc = hc32l110_calib_read_eeprom(di, &double_routine_flag);
	if (rc) {
		hwlog_err("hc32l110_calib_read_eeprom failed\n", __func__);
		return rc;
	}
	if (double_routine_flag == 1) {
		hwlog_info("%s eeprom double_routine\n", __func__);
		rc = hc32l110_calib_write_flash_data_from_eeprom_double_routine(di);
		if (rc) {
			hwlog_err("hc32l110_calib_write_flash_data_from_eeprom_double_routine failed\n", __func__);
			return rc;
		}
	} else {
		hwlog_info("%s eeprom single_routine\n", __func__);
		rc = hc32l110_calib_write_flash_data_from_eeprom_single_routine(di);
		if (rc) {
			hwlog_err("hc32l110_calib_write_flash_data_from_eeprom_single_routine failed\n", __func__);
			return rc;
		}
	}

	/* reload flash data */
	rc = hc32l110_calib_reload_flash(di);
	if (rc) {
		hwlog_err("%s hc32l110_calib_reload_flash failed.\n", __func__);
		return rc;
	}
	/* clear 0x16 bit0 interrupt to avoid empty calib data int report */
	rc = i2c_u8_write_byte(di->client, HC32L110_STAR_ISR_ERR1, 1);
	if (rc) {
		hwlog_err("%s i2c_u8_write_byte failed.\n", __func__);
		return rc;
	}

	return rc;
}

static int hc32l110_reload_calib_data_from_eeprom(struct cam_star_module_device_info *di)
{
	int rc = 0;
	u16 calib_data_type_offset = HC32L110_CALI_DATA_TYPE;
	u16 calib_data_type = 0;
	u16 eeprom_readout_value = 0;

	/* POWER_OFF to POWER_ON after download fw: power sequence 4 */
	rc = hc32l110_power_state_machine(di, RELOAD_EEPROM_TO_FLASH);
	if (rc) {
		hwlog_err("%s power_state_machine error\n", __func__);
		return rc;
	}

	rc = hc32l110_set_calib_mode(di, CALIB_MODE);
	if (rc) {
		hwlog_err("%s hc32l110_set_calib_mode fail\n", __func__);
		goto power_off;
	}

	rc = hc32l110_calib_read_calib_data_type(di, calib_data_type_offset, &calib_data_type);
	if (rc) {
		hwlog_err("%s hc32l110_calib_read_calib_data_type fail\n", __func__);
		goto exit_calib_mode;
	}

	if (calib_data_type == CALIB_DATA_FROM_CDT) {
		hwlog_info("%s already have calib data in flash\n", __func__);
	} else if (calib_data_type == CALIB_DATA_FROM_SUPPLIER) {
		hwlog_info("%s already reload calib data from eeprom to flash\n", __func__);
	} else {
		/* reload eeprom to flash */
		rc = hc32l110_calib_write_flash_data_from_eeprom(di);
		if (rc)
			hwlog_err("hc32l110_calib_write_flash_data_from_eeprom failed\n", __func__);
	}

exit_calib_mode:
	rc = hc32l110_set_calib_mode(di, NORMAL_MODE);
	if (rc)
		hwlog_err("%s hc32l110_set_calib_mode fail\n");

power_off:
	/* POWER_ON to POWER_OFF after reload eeprom to flash: power sequence 5 */
	rc = hc32l110_power_state_machine(di, RETRACTION);
	if (rc)
		hwlog_err("%s power_state_machine error\n", __func__);

	return rc;
}

static int hc32l110_set_repaired_tag(struct cam_star_module_device_info *di)
{
	int rc = 0;
	if (!di)
		return -EINVAL;

	di->maintenance_state = REPAIRED;

	rc = hc32l110_set_calib_mode(di, CALIB_MODE);
	if (rc)
		hwlog_err("%s hc32l110_set_calib_mode fail\n", __func__);

	/* reload eeprom to flash */
	rc = hc32l110_calib_write_flash_data_from_eeprom(di);
	if (rc)
		hwlog_err("hc32l110_calib_write_flash_data_from_eeprom failed\n", __func__);

	rc = hc32l110_set_calib_mode(di, NORMAL_MODE);
	if (rc)
		hwlog_err("%s hc32l110_set_calib_mode fail\n");

	rc = i2c_u8_write_byte(di->client, HC32L110_STAR_FUNC_CFG, 0x0d);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	return rc;
}

static int hc32l110_set_maintenance_delta(struct cam_star_module_device_info *di, u16 delta)
{
	int rc = 0;
	u8 buf[LEN_U16_DAT] = {0};
	if (!di)
		return -EINVAL;

	/* set maintenance tag */
	rc = i2c_u8_write_byte(di->client, HC32L110_STAR_FUNC_CFG, 0x0d);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	di->maintenance_state = REPAIRED;
	/* set maintenance delta */
	buf[0] = delta & 0xff;
	buf[1] = delta >> 8;
	rc = i2c_u8_write_byte(di->client, HC32L110_SYS_ERR_HEIGHT_AMEND_LOW, buf[0]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}
	rc += i2c_u8_write_byte(di->client, HC32L110_SYS_ERR_HEIGHT_AMEND_HIGH, buf[1]);
	if (rc) {
		hwlog_err("%s i2c failed.\n", __func__);
		return rc;
	}

	return rc;
}

static int hc32l110_flash_crc_check(struct cam_star_module_device_info *di)
{
	int rc = 0;
	u8 buf[LEN_U16_DAT] = {0};
	if (!di)
		return -EINVAL;

	/* POWER_OFF to POWER_ON: power sequence 4 */
	rc = hc32l110_power_state_machine(di, RELOAD_EEPROM_TO_FLASH);
	if (rc) {
		hwlog_err("%s power_state_machine error\n", __func__);
		return rc;
	}

	rc = hc32l110_set_calib_mode(di, CALIB_MODE);
	if (rc) {
		hwlog_err("%s hc32l110_set_calib_mode fail\n", __func__);
		goto power_off;
	}

	rc = hc32l110_calib_crc_check(di);
	if (rc) {
		hwlog_err("%s hc32l110_calib_crc_check fail\n", __func__);
		goto exit_calib_mode;
	}

	if (di->crc_check != CRC_FAIL)
		goto exit_calib_mode;
	else
		cam_sensormotor_dsm_flash_crc_check_err(di->dsm_client);

	hwlog_info("%s failed, now reload eeprom to flash...\n", __func__);
	/* reload eeprom to flash */
	rc = hc32l110_calib_write_flash_data_from_eeprom(di);
	if (rc)
		hwlog_err("hc32l110_calib_write_flash_data_from_eeprom failed\n", __func__);

exit_calib_mode:
	rc = hc32l110_set_calib_mode(di, NORMAL_MODE);
	if (rc)
		hwlog_err("%s hc32l110_set_calib_mode fail\n");

power_off:
	/* POWER_ON to POWER_OFF after reload eeprom to flash: power sequence 5 */
	rc = hc32l110_power_state_machine(di, RETRACTION);
	if (rc)
		hwlog_err("%s power_state_machine error\n", __func__);

	return rc;
}

static int hc32l110_open(struct cam_star_module_device_info *di)
{
	(void)di;
	hwlog_info("%s device open\n", __func__);
	mutex_lock(&power_lock_mutex);
	g_ref_count++;
	mutex_unlock(&power_lock_mutex);
	return 0;
}

static int hc32l110_close(struct cam_star_module_device_info *di)
{
	int rc = 0;
	if (!di)
		return -EINVAL;
	hwlog_info("%s device close\n", __func__);
	mutex_lock(&power_lock_mutex);

	if (g_ref_count)
		g_ref_count--;

	if ((g_ref_count == 0) && di->mcu_state.target_pos != 0) {
		hwlog_err("%s has not move back, now moving it back to zero.\n",
			__func__);

		rc = hc32l110_set_code(di, 0);
		if (rc != 0)
			hwlog_err("%s failed to hc32l110_set_code to zero\n", __func__);
		rc = hc32l110_start(di);
		if (rc != 0)
			hwlog_err("%s failed to hc32l110_start\n", __func__);

		cam_usleep(10000);
	}

	if ((g_ref_count == 0) && (hc32l110_get_power_state() != POWER_OFF)) {
		hwlog_err("%s is still not on power-off state, now power it off\n",
			__func__);

		/* power sequence 3, now state change to POWER_OFF */
		rc = hc32l110_power_state_machine(di, RETRACTION);
		if (rc)
			hwlog_err("%s power_state_machine error\n", __func__);
	}

	di->normal_calib_state = NORMAL;

	mutex_unlock(&power_lock_mutex);
	return rc;
}
static struct star_api_fn_t hc32l110_api_func_tbl = {
	.open = hc32l110_open,
	.close = hc32l110_close,
	.get_info = hc32l110_get_info,
	.power_on = hc32l110_power_on,
	.power_off = hc32l110_power_off,
	.set_bootloader = hc32l110_set_bootloader,
	.matchid = hc32l110_read_bootloaderid,
	.download_firmware = hc32l110_download_firmware,
	.set_code = hc32l110_set_code,
	.set_speed = hc32l110_set_speed,
	.start = hc32l110_start,
	.stop = hc32l110_stop,
	.get_real_height = hc32l110_get_real_height,
	.set_calib_mode = hc32l110_set_calib_mode,
	.calib_set_step = hc32l110_calib_set_step,
	.calib_set_speed = hc32l110_calib_set_speed,
	.calib_start = hc32l110_calib_start,
	.calib_get_real_height = hc32l110_calib_get_real_height,
	.calib_get_real_tmr = hc32l110_calib_get_real_tmr,
	.calib_set_info = hc32l110_calib_set_info,
	.calib_set_add_info = hc32l110_calib_set_add_info,
	.calib_read_info = hc32l110_calib_get_info,
	.reload_calib_data_from_eeprom = hc32l110_reload_calib_data_from_eeprom,
	.read_mcu_i2c_debug_data = hc32l110_read_mcu_i2c_debug_data,
	.set_repaired_tag = hc32l110_set_repaired_tag,
	.set_maintenance_delta = hc32l110_set_maintenance_delta,
	.flash_crc_check = hc32l110_flash_crc_check,
};

#ifdef CONFIG_SYSFS
/* MCU vendor info */
static ssize_t hc32l110_vendor_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_star_module_device_info *di = NULL;
	hwlog_info("enter hc32l110_vendor_show\n");
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
static DEVICE_ATTR(vendor, 0664, hc32l110_vendor_show, NULL);
/* firmware version_id which is to update attr */
static ssize_t hc32l110_fw_version_id_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_star_module_device_info *di = NULL;
	hwlog_info("enter hc32l110_fw_version_id_show\n");
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

static ssize_t hc32l110_fw_version_id_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di;
	unsigned int val = 0;
	int rc = 0;
	hwlog_info("enter hc32l110_fw_version_id_store\n");

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
static DEVICE_ATTR(fw_version_id, 0664, hc32l110_fw_version_id_show, hc32l110_fw_version_id_store);
/* firmware updata state attr
* 0: UPDATE_WHEN_NOT_EQUAL
* 1: UPDATE_WHEN_GREATER_THAN
*/
static ssize_t hc32l110_fw_update_state_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_star_module_device_info *di = NULL;
	hwlog_info("enter hc32l110_fw_update_state_show\n");
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

static ssize_t hc32l110_fw_update_state_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di;
	unsigned int val = 0;
	int rc = 0;
	hwlog_info("enter hc32l110_fw_update_state_store\n");

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

	hwlog_info("%s input value = %u\n", __func__, val);
	di->fw_update_state = val;
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(fw_update_state, 0664, hc32l110_fw_update_state_show, hc32l110_fw_update_state_store);
/* power attr
* power on: echo 1
* power off: echo 0
*/
static ssize_t hc32l110_power_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_star_module_device_info *di = NULL;
	hwlog_info("enter hc32l110_power_show\n");
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

static ssize_t hc32l110_power_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	unsigned int val = 0;
	int rc = 0;
	hwlog_info("enter hc32l110_power_store\n");

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
	if (val == 1)
		rc = hc32l110_power_on(di);
	else
		rc = hc32l110_power_off(di, POWEROFF_TRY_MCU_STATE);

	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(power, 0664, hc32l110_power_show, hc32l110_power_store);
/* set_bootloader_delay attr
* set_bootloader_delay: ms
*/
static ssize_t hc32l110_set_bootloader_delay_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	unsigned int val = 0;
	int rc = 0;
	hwlog_info("enter hc32l110_set_bootloader_delay_store\n");

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
	if (val <= 0) {
		hwlog_err("%s input unvalid value = %u, this mode only support > 0.\n", __func__, val);
		mutex_unlock(&di->lock);
		return count;
	}
	hwlog_info("%s input value = %u\n", __func__, val);
	di->bootloader_delay = val;
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(set_bootloader_delay, 0664, NULL, hc32l110_set_bootloader_delay_store);
/* set_bootloader attr
* set_bootloader: echo 1
*/
static ssize_t hc32l110_set_bootloader_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	unsigned int val = 0;
	int rc = 0;
	hwlog_info("enter hc32l110_set_bootloader_store\n");

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
	rc = hc32l110_set_bootloader(di);
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(set_bootloader, 0664, NULL, hc32l110_set_bootloader_store);
/* exit_bootloader attr
* exit_bootloader: echo 1
*/
static ssize_t hc32l110_exit_bootloader_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	unsigned int val = 0;
	int rc = 0;
	hwlog_info("enter hc32l110_exit_bootloader_store\n");

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
	rc = hc32l110_exit_bootloader(di);
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(exit_bootloader, 0664, NULL, hc32l110_exit_bootloader_store);
/* download_firmware attr
*  download_firmware: echo 1
*  not support cat
*/
static ssize_t hc32l110_download_firmware_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	unsigned int val = 0;
	int rc = 0;
	hwlog_info("enter hc32l110_download_firmware_store\n");

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
	rc = hc32l110_download_firmware(di);
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(download_firmware, 0664, NULL, hc32l110_download_firmware_store);
/* set_height attr
* set_height: echo u16 height
*/
static ssize_t hc32l110_set_height_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_star_module_device_info *di = NULL;
	hwlog_info("enter hc32l110_set_height_show\n");

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

static ssize_t hc32l110_set_height_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	u16 val = 0;
	int rc = 0;
	hwlog_info("enter hc32l110_set_height_store\n");

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
	rc = hc32l110_set_code(di, val);
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(set_height, 0664, hc32l110_set_height_show, hc32l110_set_height_store);
/* set_speed attr
* set_speed: echo u16 speed
*/
static ssize_t hc32l110_set_speed_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_star_module_device_info *di = NULL;
	hwlog_info("enter hc32l110_set_speed_show\n");

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

static ssize_t hc32l110_set_speed_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	u16 val = 0;
	int rc = 0;
	hwlog_info("enter hc32l110_set_speed_store\n");

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
	rc = hc32l110_set_speed(di, val);
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(set_speed, 0664, hc32l110_set_speed_show, hc32l110_set_speed_store);
/* start attr
* start: echo 1
*/
static ssize_t hc32l110_start_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	unsigned int val = 0;
	int rc = 0;
	hwlog_info("enter hc32l110_start_store\n");

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
	rc = hc32l110_start(di);
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(start, 0664, NULL, hc32l110_start_store);
/* stop attr
* stop: echo 1
*/
static ssize_t hc32l110_stop_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	unsigned int val = 0;
	int rc = 0;
	hwlog_info("enter hc32l110_stop_store\n");

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
	rc = hc32l110_stop(di);
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(stop, 0664, NULL, hc32l110_stop_store);
/* get_real_height attr
* get_real_height: get_real_height
*/
static ssize_t hc32l110_get_real_height_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_star_module_device_info *di = NULL;
	u16 real_code = 0;
	int rc = 0;
	hwlog_info("enter hc32l110_get_real_height_show\n");

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -EINVAL;
	}
	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -EINVAL;
	}

	rc = hc32l110_get_real_height(di, &real_code);
	return scnprintf(buf, PAGE_SIZE, "%u\n", real_code);
}
static DEVICE_ATTR(get_real_height, 0664, hc32l110_get_real_height_show, NULL);
/* set_calib_mode attr
* set calib mode: echo 1
* set calib mode: echo 0
*/
static ssize_t hc32l110_set_calib_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_star_module_device_info *di = NULL;
	hwlog_info("enter hc32l110_set_calib_mode_show", __func__);

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

static ssize_t hc32l110_set_calib_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	unsigned int val = 0;
	int rc = 0;
	hwlog_info("enter hc32l110_set_calib_mode_store\n");

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
		hwlog_err("cam_star hc32l110 unsupported mode: %u.\n", val);
		mutex_unlock(&di->lock);
		return -EINVAL;
	}
	hwlog_info("%s input value = %u\n", __func__, val);
	rc = hc32l110_set_calib_mode(di, val);
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(set_calib_mode, 0664, hc32l110_set_calib_mode_show, hc32l110_set_calib_mode_store);
/* calib_info attr
* set calib info to MCU: echo 1
* get calib info: cat, and return CRC result.
*/
static ssize_t hc32l110_calib_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_star_module_device_info *di = NULL;
	int rc = 0;
	int size = 0;
	hwlog_info("enter hc32l110_calib_info_show", __func__);

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
	rc = hc32l110_calib_get_info(di, (u16 *)buf, &size);
	if (rc) {
		hwlog_err("hc32l110_calib_get_info fail\n");
		mutex_unlock(&di->lock);
		return -EINVAL;
	}

	mutex_unlock(&di->lock);
	return size;
}

static int atoi(const char *s)
{
	int n = 0;
	int neg = 0;
	while (isspace(*s)) s++;
	switch (*s) {
	case '-': neg = 1;
	case '+': s++;
	}
	/* Compute n as a negative number to avoid overflow on INT_MIN */
	while (isdigit(*s))
		n = 10 * n - (*s++ - '0');
	return neg ? n : -n;
}

static ssize_t hc32l110_calib_info_store(struct device *dev,
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
	char *endptr = NULL;
	
	u16 value = 0;
	hwlog_info("enter hc32l110_calib_info_store\n");

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
	calib_info = kmalloc(count + CAM_SENSORMOTOR_CALIB_DATA_MAX_LEN, GFP_KERNEL);
	if (calib_info == NULL) {
		hwlog_err("%s kmalloc error!\n", __func__);
		mutex_unlock(&di->lock);
		return -EINVAL;
	}
	/* parse input string */
	hwlog_info("%s buf input value = %s, count: %d\n", __func__, buf, count);
	if (strcpy_s(calib_info, count + CAM_SENSORMOTOR_CALIB_DATA_MAX_LEN, buf) != EOK) {
		hwlog_err("%s strcpy_s failed.\n");
		kfree(calib_info);
		calib_info = NULL;
		mutex_unlock(&di->lock);
		return -EINVAL;
	}
	hwlog_info("%s calib_info input value = %s\n", __func__, calib_info);
	/* parse first tag: num_of_pair */
	token = strtok_s(calib_info, " ", &next_token);
	if (token) {
		rc = kstrtou16(token, 10, &(di->calib_otp.calib_info.num_of_pair));
		di->calib_otp.calib_info.num_of_pair *= 2; // 2 only for double_routine

		hwlog_info("%s num_of_pair: %u", __func__, di->calib_otp.calib_info.num_of_pair);
	} else {
		hwlog_err("%s input invalid 1! No separator BLANK!\n");
		kfree(calib_info);
		calib_info = NULL;
		mutex_unlock(&di->lock);
		return -EINVAL;
	}

	if (di->calib_otp.calib_info.num_of_pair > CALIB_INFO_NUM_OF_PAIR) {
		hwlog_err("%s input num_of_pair larger than max 72!\n");
		di->calib_otp.calib_info.num_of_pair = CALIB_INFO_NUM_OF_PAIR;
	}

	/* parse second tag: system_accuracy_err */
	token = strtok_s(NULL, " ", &next_token);
	if (token) {
		rc = kstrtou16(token, 10, &(di->calib_otp.calib_info.system_accuracy_err));
		hwlog_info("%s system_accuracy_err: %u", __func__, di->calib_otp.calib_info.system_accuracy_err);
	} else {
		hwlog_err("%s input invalid 2! No separator BLANK!\n");
		kfree(calib_info);
		calib_info = NULL;
		mutex_unlock(&di->lock);
		return -EINVAL;
	}
	/* parse distance and corresponding tmr value */
	while (token && i < (di->calib_otp.calib_info.num_of_pair) * 2) {
		token = strtok_s(NULL, " ", &next_token);
		if (token) {
			hwlog_info("%s input calib_info: %s\n", __func__, token);
			value = (u16)atoi(token);

			if (i % 2 == 0) {
				di->calib_otp.calib_info.dis_tmr_pair[i / 2].dis = value;
				hwlog_info("index of pair: %d, dis: %lu", i / 2, value);
			} else {
				di->calib_otp.calib_info.dis_tmr_pair[i / 2].tmr = value;
				hwlog_info("index of pair: %d, tmr: %lu", i / 2, value);
			}
			i++;
		}
	}

	/* set calib info to MCU */
	rc = hc32l110_calib_set_info(di);
	kfree(calib_info);
	calib_info = NULL;
	mutex_unlock(&di->lock);
	return rc;
}
static DEVICE_ATTR(calib_info, 0664, hc32l110_calib_info_show, hc32l110_calib_info_store);
/* calib_add_info attr
* set calib additional info to MCU: echo 1
*/
static ssize_t hc32l110_calib_add_info_store(struct device *dev,
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
	unsigned long value = 0;
	hwlog_info("enter hc32l110_calib_info_store\n");

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
	calib_add_info = kmalloc(count + CAM_SENSORMOTOR_CALIB_DATA_MAX_LEN, GFP_KERNEL);
	if (calib_add_info == NULL) {
		hwlog_err("%s kmalloc error!\n", __func__);
		mutex_unlock(&di->lock);
		return NULL;
	}

	/* parse input string */
	hwlog_info("%s buf input value = %s, count:%d\n", __func__, buf, count);
	if (strcpy_s(calib_add_info, count + CAM_SENSORMOTOR_CALIB_DATA_MAX_LEN, buf) != EOK) {
		hwlog_err("%s strcpy_s failed.\n");
		kfree(calib_add_info);
		calib_add_info = NULL;
		mutex_unlock(&di->lock);
		return -EINVAL;
	}
	hwlog_info("%s calib_add_info input value = %s\n", __func__, calib_add_info);
	token = strtok_s(calib_add_info, " ", &next_token);
	if (token) {
		di->calib_otp.calib_add_info[0] = (u16)atoi(token);
		hwlog_info("%s crc_check: %u", __func__, di->calib_otp.calib_add_info[0]);
	} else {
		hwlog_err("%s input invalid 1! No separator BLANK!\n");
		kfree(calib_add_info);
		calib_add_info = NULL;
		mutex_unlock(&di->lock);
		return -EINVAL;
	}

	while (token && i < NUM_OF_CALIB_ADD_INFO) {
		token = strtok_s(NULL, " ", &next_token);
		if (!token) {
			hwlog_err("%s token is null.\n", __func__);
			break;
		}
		rc = kstrtoul(token, 10, &value);
		di->calib_otp.calib_add_info[i] = value;
		hwlog_info("%s index: %d, calib_add_info: %s, value: %u", __func__, i, token, value);
		i++;
	}

	/* set calib info to MCU */
	rc = hc32l110_calib_set_add_info(di);
	kfree(calib_add_info);
	calib_add_info = NULL;
	mutex_unlock(&di->lock);
	return rc;
}
static DEVICE_ATTR(calib_add_info, 0664, NULL, hc32l110_calib_add_info_store);
/* calib_set_step attr
* calib_set_step: echo int16 height
*/
static ssize_t hc32l110_calib_set_step_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	int val = 0;
	int rc = 0;
	hwlog_info("enter hc32l110_calib_set_step_store\n");

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
	rc = hc32l110_calib_set_step(di, val);
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(calib_set_step, 0664, NULL, hc32l110_calib_set_step_store);
/* calib_set_speed attr
* calib_set_speed: echo u16 height
*/
static ssize_t hc32l110_calib_set_speed_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	u16 val = 0;
	int rc = 0;
	hwlog_info("enter hc32l110_calib_set_speed_store\n");

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
	rc = hc32l110_calib_set_speed(di, val);
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(calib_set_speed, 0664, NULL, hc32l110_calib_set_speed_store);
/* calib_start attr
* calib_start: echo 1
*/
static ssize_t hc32l110_calib_start_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	u16 val = 0;
	int rc = 0;
	hwlog_info("enter hc32l110_calib_start_store\n");

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
	rc = hc32l110_calib_start(di);
	mutex_unlock(&di->lock);
	return rc ? -1 : count;

	return -EPERM;
}
static DEVICE_ATTR(calib_start, 0664, NULL, hc32l110_calib_start_store);

/* calib_get_real_height attr
* calib_get_real_height: calib_get_real_height
*/
static ssize_t hc32l110_calib_get_real_height_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_star_module_device_info *di = NULL;
	int16_t real_code = 0;
	int rc = 0;
	hwlog_info("enter hc32l110_calib_get_real_height_show\n");

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
	rc = hc32l110_calib_get_real_height(di, &real_code);
	mutex_unlock(&di->lock);
	return scnprintf(buf, PAGE_SIZE, "%d\n", real_code);
}
static DEVICE_ATTR(calib_get_real_height, 0664, hc32l110_calib_get_real_height_show, NULL);

/* calib_get_real_tmr attr
* calib_get_real_tmr: calib_calib_get_real_tmr
*/
static ssize_t hc32l110_calib_get_real_tmr_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_star_module_device_info *di = NULL;
	int16_t real_code = 0;
	int rc = 0;
	hwlog_info("enter hc32l110_calib_get_real_tmr_show\n");

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
	rc = hc32l110_calib_get_real_tmr(di, &real_code);
	mutex_unlock(&di->lock);
	return scnprintf(buf, PAGE_SIZE, "%d\n", real_code);
}
static DEVICE_ATTR(calib_get_real_tmr, 0664, hc32l110_calib_get_real_tmr_show, NULL);

/* AT sysfs */

/* save_height_value attr
* save_height_value_show: return saved height value
* save_height_value_store: read_real_height every 20ms, and save it.
*/
static ssize_t hc32l110_save_height_value_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_star_module_device_info *di = NULL;
	int size = -1;
	hwlog_info("enter hc32l110_save_height_value_show\n");

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
	hwlog_info("%s 1:%d, 2: %d\n", __func__,
		CAM_SENSORMOTOR_HEIGHT_DATA_LEN * sizeof(int), sizeof(di->atcmd_param.height_value));
	if (memcpy_s(buf, CAM_SENSORMOTOR_HEIGHT_DATA_LEN * sizeof(int),
		&(di->atcmd_param.height_value), sizeof(di->atcmd_param.height_value)) != EOK) {
		hwlog_err("%s memcpy failed\n", __func__);
		mutex_unlock(&di->lock);
		return -EINVAL;
	}

	mutex_unlock(&di->lock);
	return size;
}

static ssize_t hc32l110_save_height_value_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	u16 val = 0;
	int rc = 0;
	u16 real_code = 0;
	hwlog_info("enter hc32l110_save_height_value_store\n");

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
			rc = hc32l110_get_real_height(di, &real_code);
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
static DEVICE_ATTR(save_height_value, 0664, hc32l110_save_height_value_show, hc32l110_save_height_value_store);

/* i2c_u8_read attr
* i2c_u8_read_show: return i2c_u8_read u8 value
* i2c_u8_read_store: which u8 reg need to i2c_u8_read.
*/
static ssize_t hc32l110_i2c_u8_read_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_star_module_device_info *di = NULL;
	u8 val = 0;
	int rc = 0;
	hwlog_info("enter %s, need to echo reg first!\n", __func__);

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
	rc = i2c_u8_read_byte(di->client, di->i2c_read_write_op.reg_u8, &val);
	hwlog_info("reg need to read: 0x%0x, readout val: 0x%0x, rc = %d\n", di->i2c_read_write_op.reg_u8, val, rc);
	mutex_unlock(&di->lock);
	return scnprintf(buf, PAGE_SIZE, "0x%x\n", val);
}

static ssize_t hc32l110_i2c_u8_read_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	u8 val = 0;
	int rc = 0;
	u16 real_code = 0;
	hwlog_info("enter %s, echo reg which need to read\n", __func__);

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
	if (sscanf_s(buf, "%x\n", &val) != 1) {
		mutex_unlock(&di->lock);
		return -1;
	}

	hwlog_info("%s input value = 0x%x\n", __func__, val);
	di->i2c_read_write_op.reg_u8 = val;
	mutex_unlock(&di->lock);
	return rc ? -1 : count;
}
static DEVICE_ATTR(i2c_u8_read, 0664, hc32l110_i2c_u8_read_show, hc32l110_i2c_u8_read_store);

/* i2c_u8_write attr
* i2c_u8_write_store: which u8 reg need to i2c_u8_write.
*/
static ssize_t hc32l110_i2c_u8_write_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	int rc = 0;
	u16 real_code = 0;
	char *token = NULL;
	char *next_token = NULL;
	char input_buf[20];
	int param_count = 0;

	strcpy_s(input_buf, 20, buf);
	hwlog_info("buf: %s\n", buf);
	hwlog_info("enter %s, count: %d, input_buf: %s\n", __func__, count, input_buf);

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
	token = strtok_s(input_buf, " ", &next_token);
	while (token && (param_count < 2)) {
		param_count++;
		hwlog_info("token: %s, next_token: %s\n", token, next_token);
		if (param_count == 1)
			kstrtou8(token, 16, &(di->i2c_read_write_op.reg_u8));
		else if (param_count == 2)
			kstrtou8(token, 16, &(di->i2c_read_write_op.data_u8));
		else
			hwlog_err("only need two input param!\n");

		token = strtok_s(NULL, " ", &next_token);
	}

	hwlog_info("%s input reg: 0x%x, value: 0x%x\n",
		__func__, di->i2c_read_write_op.reg_u8, di->i2c_read_write_op.data_u8);
	rc = i2c_u8_write_byte(di->client, di->i2c_read_write_op.reg_u8, di->i2c_read_write_op.data_u8);
	if (rc)
		hwlog_err("%s i2c failed.\n", __func__);
	mutex_unlock(&di->lock);
	return rc ? -1 : count;
}
static DEVICE_ATTR(i2c_u8_write, 0664, NULL, hc32l110_i2c_u8_write_store);

/* exception_log_print attr
* exception_log_print: .
*/
static ssize_t hc32l110_exception_log_print_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct cam_star_module_device_info *di = NULL;
	u16 val = 0;
	int rc = 0;
	u16 real_code = 0;
	hwlog_info("enter %s\n", __func__);

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
	if (val != 1) {
		hwlog_err("invalid input value!\n");
		mutex_unlock(&di->lock);
		return -EINVAL;
	}

	rc = hc32l110_read_log_reg(di);

	mutex_unlock(&di->lock);
	return rc ? -1 : count;
}
static DEVICE_ATTR(exception_log_print, 0664, NULL, hc32l110_exception_log_print_store);

static struct attribute *hc32l110_attributes[] = {
	&dev_attr_vendor.attr,
	&dev_attr_fw_version_id.attr,
	&dev_attr_fw_update_state.attr,
	&dev_attr_power.attr,
	&dev_attr_set_bootloader.attr,
	&dev_attr_set_bootloader_delay.attr,
	&dev_attr_exit_bootloader.attr,
	&dev_attr_download_firmware.attr,
	&dev_attr_set_height.attr,
	&dev_attr_set_speed.attr,
	&dev_attr_start.attr,
	&dev_attr_stop.attr,
	&dev_attr_get_real_height.attr,
	NULL,
};

static struct attribute *hc32l110_calib_attrs[] = {
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

static struct attribute *hc32l110_atcmd_attrs[] = {
	&dev_attr_save_height_value.attr,
	NULL,
};

static struct attribute *hc32l110_debug_attrs[] = {
	&dev_attr_i2c_u8_read.attr,
	&dev_attr_i2c_u8_write.attr,
	&dev_attr_exception_log_print.attr,
	NULL,
};

static const struct attribute_group hc32l110_attr_group = {
	.name = "normal",
	.attrs = hc32l110_attributes,
};

static const struct attribute_group hc32l110_calib_group = {
	.name = "calib",
	.attrs = hc32l110_calib_attrs,
};

static const struct attribute_group hc32l110_atcmd_group = {
	.name = "ATCMD",
	.attrs = hc32l110_atcmd_attrs,
};

static const struct attribute_group hc32l110_debug_group = {
	.name = "debug",
	.attrs = hc32l110_debug_attrs,
};

static const struct attribute_group *star_groups[] = {
	&hc32l110_attr_group,
	&hc32l110_calib_group,
	&hc32l110_atcmd_group,
	&hc32l110_debug_group,
	NULL,
};
#endif /* CONFIG_SYSFS */

int hc32l110_probe(struct i2c_client *client,
	const struct i2c_device_id *id, struct cam_star_module_device_info *di)
{
	int rc = 0;
	struct device_node *np = NULL;

	if (!client || !client->dev.of_node || !id || !di)
		return -ENODEV;

	rc = hc32l110_irq_init(di);
	if (rc) {
		hwlog_err("cam_hc32l110 irq init failed.\n");
		return rc;
	}
	/* add di ioctl intf */
	di->api_func_tbl = &hc32l110_api_func_tbl;
	di->star_attr_group = star_groups;

	/* init th MCU state */
	di->normal_calib_state = NORMAL;
	/* init fw version id */
	di->fw_state.fw_ver_id = HC32L110_FW_VER;
	/* init power state machine */
	hc32l110_power_state_machine_init();

	return 0;
}
EXPORT_SYMBOL(hc32l110_probe);

int hc32l110_remove(struct i2c_client *client)
{
	struct cam_star_module_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	return 0;
}
EXPORT_SYMBOL(hc32l110_remove);
