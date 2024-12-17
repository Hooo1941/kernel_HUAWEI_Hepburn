/* Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kmod.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/gfp.h>
#include <linux/types.h>
#include <securec.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "drv_panel_mcu.h"
#include "upgrade_channel_manage.h"
#include "../common/log.h"

#define SLAVE_DEVICE_ADDRESS (0x42)
#define I2C_SEL_MCU (4)

#define BYTE_SIZE (8)
#define HUNDRED (100)

#define LENGTH_TRANSMISSION (4096)
#define I2C_RETRY_CNT (3)
#define DELAY_TIME (15)
#define RETRY_TIME (500)
#define RESET_DELAY_TIME (2000)
#define ERASE_DELAY_TIME (10 * 1000)
#define BOOTCTL_DELAY_TIME (1000)
#define CHECKSUM_DELAY_TIME (3000)

static int32_t g_process = 0;

static int32_t drv_panel_mcu_flash_write(uint8_t device_address, uint8_t write_type,
	const uint8_t *data, uint32_t length)
{
	int32_t i = 0;
	int32_t retry_cnt = I2C_RETRY_CNT;
	int32_t err = RET_FAIL;
	uint8_t *kmalloc_buf;
	int32_t ret = 0;
	struct i2c_adapter *i2c_file = NULL;
	struct i2c_msg msg[1];

	i2c_file = i2c_get_adapter(I2C_SEL_MCU);
	if (!i2c_file) {
		cdc_err("UPDATE_ERR:can not get i2c_get_adapter!\n");
		return RET_FAIL;
	}

	kmalloc_buf = kmalloc((length + 1), GFP_KERNEL);
	if (!kmalloc_buf) {
		cdc_err("UPDATE_ERR:can not malloc!\n");
		return RET_FAIL;
	}

	ret = memset_s(kmalloc_buf, sizeof(kmalloc_buf), 0, sizeof(char));
	if (ret != 0)
		cdc_err("memset_s error");

	kmalloc_buf[0] = write_type & 0xff;
	for (i = 0; i < length; i++)
		kmalloc_buf[i + 1] = *(data + i);

	msg[0].addr = device_address;
	msg[0].flags = 0;
	msg[0].len = 1 + length;
	msg[0].buf = (char *)kmalloc_buf;

	do {
		err = i2c_transfer(i2c_file, msg, 1);
		if (err < 0)
			mdelay(RETRY_TIME);
	} while ((err < 0) && (--retry_cnt));
	if (err < 0) {
		cdc_err("%s i2c transfer write error, error=%d", __func__, err);
		kfree(kmalloc_buf);
		kmalloc_buf = NULL;
		return err;
	}

#ifdef TEST_DEBUG
	for (i = 0; i < length + 1; i++)
		cdc_info("test write buf %dth data :0x%x", i, msg[0].buf[i]);
#endif
	kfree(kmalloc_buf);
	kmalloc_buf = NULL;
	return RET_SUCCESS;
}

static int32_t drv_panel_mcu_flash_read(uint8_t device_address, uint8_t read_type,
	uint8_t *data, uint32_t length)
{
	int32_t err = RET_FAIL;
	int32_t retry_cnt = I2C_RETRY_CNT;
	uint8_t register_addr = read_type;
	struct i2c_adapter *i2c_file = NULL;

	struct i2c_msg msgs[] = {
		{
			.addr = device_address,
			.flags = 0,
			.len = 1,
			.buf = &register_addr,
		},
		{
			.addr = device_address,
			.flags = I2C_M_RD,
			.len = length,
			.buf = data,
		},
	};

	i2c_file = i2c_get_adapter(I2C_SEL_MCU);
	if (!i2c_file) {
		cdc_err("UPDATE_ERR: can not get i2c_get_adapter!\n");
		return RET_FAIL;
	}

	do {
		err = i2c_transfer(i2c_file, msgs, 0x02);
		if (err < 0)
			mdelay(RETRY_TIME);
	} while ((err < 0) && (--retry_cnt));
	if (err < 0) {
		cdc_err("%s i2c transfer read error, error=%d", __func__, err);
		return err;
	}

#ifdef TEST_DEBUG
	int32_t i = 0;
	for (i = 0; i < length; i++)
		cdc_info("test read buf %dth data :0x%x", i, data[i]);
#endif
	return RET_SUCCESS;
}

static int32_t check_nrc_value(uint8_t *value, uint32_t *status)
{
	int32_t i;
	int32_t len = value[0];
	int32_t sid = value[1];
	if ((len == LENGTH_ERR_FRAME) && (sid == RETURN_ERR_SID)) {
		for (i = 1; i <= LENGTH_ERR_FRAME; i++)
			*status |= (uint32_t)value[i] << ((LENGTH_ERR_FRAME - i) * BYTE_SIZE);
		cdc_err("UPDATE_ERR: len %d, nrc 0x%x", len, *status);
		return RET_WAIT;
	} else {
		*status = 0x01;
		cdc_info("UPDATE_SUCC: len %d, sid 0x%x", len, sid);
	}
	return RET_SUCCESS;
}

static int32_t drv_panel_mcu_multiple_read(uint8_t read_type, uint8_t *data, uint32_t length, uint32_t *status)
{
	int32_t ret = 0;
	int32_t read_retry = I2C_RETRY_CNT;
	do {
		ret = drv_panel_mcu_flash_read(SLAVE_DEVICE_ADDRESS, read_type, data, length);
		if (ret < 0) {
			cdc_err("UPDATE_ERR: transmit address/length I2C read error");
			return RET_WAIT;
		}
		ret = check_nrc_value(data, status);
		if (ret < 0 && (*status & 0xFF) != RETURN_WAIT_SID) {
			ret = RET_FAIL;
			break;
		}
		if (ret == RET_SUCCESS && *status == 0x01) {
			switch (read_type) {
			case I2C_CTR_GETSLOT:
				*status = data[length-1];
				ret = (data[length-1] == 0x00 || data[length-1] == 0x01 || data[length-1] == 0xFF) ?
					   RET_SUCCESS : RET_FAIL;
				break;
			case I2C_CTR_ADDLEN:
				*status = data[length-1];
				ret = (data[length-1] == 0x00) ? RET_SUCCESS : RET_FAIL;
				break;
			case I2C_CTR_CRC:
				*status = data[length-1];
				ret = (data[length-1] == 0x00) ? RET_SUCCESS : RET_FAIL;
				break;
			case I2C_CTR_SETSLOT:
				*status = data[length-1];
				ret = (data[length-1] == 0x00) ? RET_SUCCESS : RET_FAIL;
				break;
			default:
				break;
			}
		}
		if (ret == RET_SUCCESS || ret == RET_FAIL)
			break;
		mdelay(RETRY_TIME);
	} while ((ret < 0) && (read_retry--));
	return ret;
}

static int32_t drv_panel_get_version(uint8_t reg_type, uint32_t read_length, char *version, uint32_t *status)
{
	int32_t i;
	int32_t ret;
	unsigned long temp = 0;
	uint8_t data[MCU_GET_VERSION_LENGTH];
	int32_t offset = 4;
	int32_t versionlength = 0;
	uint8_t mcu_status[read_length];

	if ((!version) || (!status))
		return RET_FAIL;

	if (I2C_CTR_BOOTVER == reg_type) {
		/* get boot version */
		temp = MCU_GET_BOOTVERSION;
		for (i = 0; i < MCU_GET_VERSION_LENGTH; i++)
			data[MCU_GET_VERSION_LENGTH - i - 1] = (uint8_t)((temp >> (i * BYTE_SIZE)) & 0xFF);
	} else if (I2C_CTR_APPVER == reg_type) {
		/* get app version */
		temp = MCU_GET_APPVERSION;
		for (i = 0; i < MCU_GET_VERSION_LENGTH; i++)
			data[MCU_GET_VERSION_LENGTH - i - 1] = (uint8_t)((temp >> (i * BYTE_SIZE)) & 0xFF);
	}

	ret = drv_panel_mcu_flash_write(SLAVE_DEVICE_ADDRESS, reg_type, data, MCU_GET_VERSION_LENGTH);
	if (ret < 0) {
		*status = ((uint32_t)RETURN_ERR_SID << SHIFT_SIXTEEN) | ((uint32_t)SDID_GETVERSION << SHIFT_EIGHT) |
				  ((uint32_t)RETURN_TIMEOUT_SID);
		cdc_err("UPDATE_ERR:i2c transfer version commands error");
		return RET_WAIT;
	}

	mdelay(DELAY_TIME);

	ret = drv_panel_mcu_multiple_read(reg_type, mcu_status, read_length, status);
	if (ret == RET_SUCCESS) {
		cdc_info("UPDATE_SUCC: i2c get version succ");
		if (read_length == MCU_GET_VERSION_READ) {
			versionlength = APPVERSION_LENGTH;
		} else {
			versionlength = BOOTVERSION_LENGTH;
		}

		ret = strncpy_s(version, VERSION_LEN, (mcu_status + offset), versionlength);
		if (ret != 0) {
			cdc_err("strncpy_s error");
			return RET_FAIL;
		}
		version[versionlength] = '\0';
	}
	return ret;
}

int32_t drv_panel_mcu_get_ota_version(char *boot_ver, char *app_ver, uint32_t *status)
{
	int32_t ret = 0;
	if (!boot_ver || !app_ver)
		return RET_FAIL;
	cdc_info("Entry %s +.\n", __func__);

	ret = drv_panel_get_version(I2C_CTR_BOOTVER, MCU_GET_BOOTVERSION_READ, boot_ver, status);
	if (ret < 0) {
		cdc_err("UPDATE_ERR: i2c get boot version error");
		return ret;
	}

	ret = drv_panel_get_version(I2C_CTR_APPVER, MCU_GET_VERSION_READ, app_ver, status);
	if (ret < 0) {
		cdc_err("UPDATE_ERR: i2c get app version error");
		return ret;
	}

	cdc_info("Exit %s +.\n", __func__);
	return ret;
}

int32_t drv_panel_mcu_extended_session(uint32_t *status)
{
	int32_t i;
	int32_t ret;
	uint8_t write_buffer[MCU_START_UPDATE_LENGTH];
	uint8_t mcu_status[MCU_START_UPDATE_READ];
	unsigned long temp = MCU_EXTENDED_SESSION;

	if (!status) {
		cdc_err("UPDATE_ERR: status is null");
		return RET_FAIL;
	}
	cdc_info("Entry %s +\n", __func__);

	for (i = 0; i < MCU_START_UPDATE_LENGTH; i++)
		write_buffer[MCU_START_UPDATE_LENGTH - i - 1] =
			(uint8_t)((temp >> (i * BYTE_SIZE)) & 0xFF);

	ret = drv_panel_mcu_flash_write(SLAVE_DEVICE_ADDRESS, I2C_CTR_EXTENDED,
		write_buffer, MCU_START_UPDATE_LENGTH);
	if (ret < 0) {
		*status = ((uint32_t)RETURN_ERR_SID << SHIFT_SIXTEEN) | ((uint32_t)SDID_EXTENDED << SHIFT_EIGHT) |
			((uint32_t)RETURN_TIMEOUT_SID);
		cdc_err("UPDATE_ERR: %s I2C write error", __func__);
		return RET_WAIT;
	}

	mdelay(DELAY_TIME);

	ret = drv_panel_mcu_multiple_read(I2C_CTR_EXTENDED, mcu_status, MCU_START_UPDATE_READ, status);
	if (ret == RET_SUCCESS)
		cdc_info("UPDATE_INFO:%s succ", __func__);
	cdc_info("Exit %s +\n", __func__);
	return ret;
}

int32_t drv_panel_mcu_program_session(uint32_t *status)
{
	int32_t i;
	int32_t ret;
	uint8_t write_buffer[MCU_START_UPDATE_LENGTH];
	uint8_t mcu_status[MCU_START_UPDATE_READ];
	unsigned long temp = MCU_PROGRAM_SESSION;

	if (!status) {
		cdc_err("UPDATE_ERR: status is null");
		return RET_FAIL;
	}
	cdc_info("Entry %s +\n", __func__);

	for (i = 0; i < MCU_START_UPDATE_LENGTH; i++)
		write_buffer[MCU_START_UPDATE_LENGTH - i - 1] =
			(uint8_t)((temp >> (i * BYTE_SIZE)) & 0xFF);

	ret = drv_panel_mcu_flash_write(SLAVE_DEVICE_ADDRESS, I2C_CTR_PROGRAM,
		write_buffer, MCU_START_UPDATE_LENGTH);
	if (ret < 0) {
		*status = ((uint32_t)RETURN_ERR_SID << SHIFT_SIXTEEN) | ((uint32_t)SDID_EXTENDED << SHIFT_EIGHT) |
			((uint32_t)RETURN_TIMEOUT_SID);
		cdc_err("UPDATE_ERR: %s I2C write error", __func__);
		return RET_WAIT;
	}

	mdelay(RESET_DELAY_TIME);

	ret = drv_panel_mcu_multiple_read(I2C_CTR_PROGRAM, mcu_status, MCU_START_UPDATE_READ, status);
	if (ret == RET_SUCCESS)
		cdc_info("UPDATE_INFO:%s succ", __func__);
	cdc_info("Exit %s +\n", __func__);
	return ret;
}

int32_t drv_panel_mcu_get_slot(uint32_t *status)
{
	int32_t i;
	int32_t ret;
	uint8_t write_buffer[MCU_GET_SLOT_LENGTH];
	uint8_t mcu_status[MCU_GET_SLOT_READ];
	unsigned long temp = MCU_GET_SLOT;

	if (!status) {
		cdc_err("UPDATE_ERR: mcu start update status is null");
		return RET_FAIL;
	}
	cdc_info("Entry %s +\n", __func__);

	for (i = 0; i < MCU_GET_SLOT_LENGTH; i++)
		write_buffer[MCU_GET_SLOT_LENGTH - i - 1] =
			(uint8_t)((temp >> (i * BYTE_SIZE)) & 0xFF);

	ret = drv_panel_mcu_flash_write(SLAVE_DEVICE_ADDRESS, I2C_CTR_GETSLOT,
		write_buffer, MCU_GET_SLOT_LENGTH);
	if (ret < 0) {
		*status = ((uint32_t)RETURN_ERR_SID << SHIFT_SIXTEEN) | ((uint32_t)SDID_GETSLOT << SHIFT_EIGHT) |
			((uint32_t)RETURN_TIMEOUT_SID);
		cdc_err("UPDATE_ERR: %s I2C write error", __func__);
		return RET_WAIT;
	}

	mdelay(DELAY_TIME);

	ret = drv_panel_mcu_multiple_read(I2C_CTR_GETSLOT, mcu_status, MCU_GET_SLOT_READ, status);
	if (ret < 0)
		*status = ((uint32_t)RETURN_ERR_SID << SHIFT_SIXTEEN) | ((uint32_t)SDID_GETSLOT << SHIFT_EIGHT) |
			((uint32_t)(*status) & 0xFF);
	cdc_info("UPDATE_INFO:%s succ", __func__);
	cdc_info("Exit %s +\n", __func__);
	return ret;
}

int32_t drv_panel_mcu_get_seed(uint32_t *seed, uint32_t *status)
{
	int32_t i;
	int32_t ret;
	uint8_t write_buffer[MCU_GET_SEED_LENGTH];
	uint8_t mcu_status[MCU_GET_SEED_READ];
	unsigned long temp = MCU_GET_SEED;

	if (!seed || !status) {
		cdc_err("UPDATE_ERR: mcu get seed is null");
		return RET_FAIL;
	}
	cdc_info("Entry %s +\n", __func__);

	for (i = 0; i < MCU_GET_SEED_LENGTH; i++)
		write_buffer[MCU_GET_SEED_LENGTH - i - 1] =
			(uint8_t)((temp >> (i * BYTE_SIZE)) & 0xFF);

	ret = drv_panel_mcu_flash_write(SLAVE_DEVICE_ADDRESS, I2C_CTR_SEED,
		write_buffer, MCU_GET_SEED_LENGTH);
	if (ret < 0) {
		*status = ((uint32_t)RETURN_ERR_SID << SHIFT_SIXTEEN) | ((uint32_t)SDID_AUTHENTICATION << SHIFT_EIGHT) |
			((uint32_t)RETURN_TIMEOUT_SID);
		*seed = 0;
		cdc_err("UPDATE_ERR: %s I2C write error", __func__);
		return RET_WAIT;
	}

	mdelay(DELAY_TIME);

	ret = drv_panel_mcu_multiple_read(I2C_CTR_SEED, mcu_status, MCU_GET_SEED_READ, status);
	if (ret == RET_SUCCESS) {
		*seed = (((uint32_t)mcu_status[3] << SHIFT_TWENTYFOUR) | ((uint32_t)mcu_status[4] << SHIFT_SIXTEEN) |
			((uint32_t)mcu_status[5] << SHIFT_EIGHT) | ((uint32_t)mcu_status[6] & 0xFF));
		cdc_info("UPDATE_INFO:%s succ", __func__);
	}
	cdc_info("Exit %s +\n", __func__);
	return ret;
}

int32_t drv_panel_mcu_send_key(uint32_t key, uint32_t *status)
{
	int32_t i;
	int32_t ret ;
	int32_t offset = 4;
	uint8_t write_buffer[MCU_SEND_KEY_LENGTH];
	uint8_t read_buffer[MCU_SEND_KEY_READ];
	unsigned long temp = MCU_SEND_KEY;

	if (!status) {
		cdc_err("UPDATE_ERR: mcu start update status is null");
		return RET_FAIL;
	}
	cdc_info("Entry %s +\n", __func__);

	for (i = 0; i < MCU_SEND_KEY_LENGTH; i++) {
		if (i < offset) {
			write_buffer[MCU_SEND_KEY_LENGTH - i - 1] =
				(uint8_t)((key >> (BYTE_SIZE * i)) & 0xFF);
		} else {
			write_buffer[MCU_SEND_KEY_LENGTH - i - 1] =
				(uint8_t)((temp >> ((i - offset) * BYTE_SIZE)) & 0xFF);
		}
	}

	ret = drv_panel_mcu_flash_write(SLAVE_DEVICE_ADDRESS, I2C_CTR_KEY,
		write_buffer, MCU_SEND_KEY_LENGTH);
	if (ret < 0) {
		*status = ((uint32_t)RETURN_ERR_SID << SHIFT_SIXTEEN) | ((uint32_t)SDID_AUTHENTICATION << SHIFT_EIGHT) |
			((uint32_t)RETURN_TIMEOUT_SID);
		cdc_err("UPDATE_ERR: %s I2C write error", __func__);
		return RET_WAIT;
	}

	mdelay(DELAY_TIME);

	ret = drv_panel_mcu_multiple_read(I2C_CTR_KEY, read_buffer, MCU_SEND_KEY_READ, status);
	if (ret == RET_SUCCESS)
		cdc_info("UPDATE_INFO:%s succ", __func__);
	cdc_info("Exit %s +\n", __func__);
	return ret;
}

int32_t drv_panel_mcu_erase_flash(uint32_t address, uint32_t len, uint32_t *status)
{
	int32_t i;
	int32_t ret;
	int32_t offset = 6;
	int32_t value_offset = 3;
	int32_t length_index = 13;
	int32_t address_index = 9;
	uint8_t write_buffer[MCU_REASE_LENGTH];
	uint8_t read_buffer[MCU_REASE_READ];
	unsigned long temp = MCU_REASE;

	if (!status) {
		cdc_err("UPDATE_ERR:mcu erase flash status is null");
		return RET_FAIL;
	}
	cdc_info("Entry %s +\n", __func__);

	for (i = address_index - value_offset; i <= address_index ; i++)
		write_buffer[i] = (uint8_t)((address >> (BYTE_SIZE * (address_index - i))) & 0xFF);
	for (i = length_index - value_offset; i <= length_index ; i++)
		write_buffer[i] = (uint8_t)((len >> (BYTE_SIZE * (length_index - i))) & 0xFF);
	for (i = 0; i < offset; i++)
		write_buffer[offset - i - 1] = (uint8_t)((temp >> (i * BYTE_SIZE)) & 0xFF);

	ret = drv_panel_mcu_flash_write(SLAVE_DEVICE_ADDRESS, I2C_CTR_ADDLEN,
		write_buffer, MCU_REASE_LENGTH);
	if (ret < 0) {
		*status = ((uint32_t)RETURN_ERR_SID << SHIFT_SIXTEEN) | ((uint32_t)SDID_REASE << SHIFT_EIGHT) |
			((uint32_t)RETURN_TIMEOUT_SID);
		cdc_err("UPDATE_ERR: %s I2C write error", __func__);
		return RET_WAIT;
	}

	mdelay(ERASE_DELAY_TIME);

	ret = drv_panel_mcu_multiple_read(I2C_CTR_ADDLEN, read_buffer, MCU_REASE_READ, status);
	if (ret < 0)
		*status = ((uint32_t)RETURN_ERR_SID << SHIFT_SIXTEEN) | ((uint32_t)SDID_REASE << SHIFT_EIGHT) |
			((uint32_t)(*status) & 0xFF);
	g_process = 0;
	cdc_info("UPDATE_INFO:%s succ", __func__);
	cdc_info("Exit %s +\n", __func__);
	return ret;
}


int32_t drv_panel_mcu_request_download(uint32_t address, uint32_t len, uint32_t *status)
{
	int32_t i;
	int32_t ret;
	int32_t offset = 4;
	int32_t value_offset = 3;
	int32_t length_index = 11;
	int32_t address_index = 7;
	uint8_t read_buffer[MCU_DOWNLOAD_READ];
	uint8_t write_buffer[MCU_DOWNLOAD_LENGTH];
	unsigned long temp = MCU_DOWNLOAD;

	if (!status) {
		cdc_err("UPDATE_ERR:mcu request download status is null");
		return RET_FAIL;
	}
	cdc_info("Entry %s +\n", __func__);

	for (i = address_index - value_offset; i <= address_index ; i++)
		write_buffer[i] = (uint8_t)((address >> (BYTE_SIZE * (address_index - i))) & 0xFF);
	for (i = length_index - value_offset; i <= length_index ; i++)
		write_buffer[i] = (uint8_t)((len >> (BYTE_SIZE * (length_index - i))) & 0xFF);
	for (i = 0; i < offset; i++)
		write_buffer[offset - i - 1] = (uint8_t)((temp >> (i * BYTE_SIZE)) & 0xFF);

	ret = drv_panel_mcu_flash_write(SLAVE_DEVICE_ADDRESS, I2C_CTR_DOWNLOAD,
		write_buffer, MCU_DOWNLOAD_LENGTH);
	if (ret < 0) {
		*status = ((uint32_t)RETURN_ERR_SID << SHIFT_SIXTEEN) | ((uint32_t)SDID_DOWNLOAD << SHIFT_EIGHT) |
			((uint32_t)RETURN_TIMEOUT_SID);
		cdc_err("UPDATE_ERR: %s I2C write error", __func__);
		return RET_WAIT;
	}

	mdelay(DELAY_TIME);

	ret = drv_panel_mcu_multiple_read(I2C_CTR_DOWNLOAD, read_buffer, MCU_DOWNLOAD_READ, status);
	if (ret == RET_SUCCESS)
		cdc_info("UPDATE_INFO:%s succ", __func__);

	g_process = 0;
	cdc_info("Exit %s +\n", __func__);
	return ret;
}

static int32_t drv_panel_mcu_write_for_update(const uint8_t *version_bin, uint32_t len, uint32_t time, uint32_t *status)
{
	int32_t i;
	int32_t ret;
	int32_t offset = 5;
	uint32_t length = MCU_TRANSFER_LENGTH;
	uint8_t write_buffer[MCU_TRANSFER_LENGTH];
	uint8_t read_buffer[MCU_TRANSFER_READ];

	if (!version_bin || !status) {
		cdc_err("UPDATE_ERR: version_bin is null");
		return RET_FAIL;
	}
	const uint8_t *p_curdata = version_bin;

	write_buffer[0] = TRANSFER_TYPE;
	write_buffer[1] = (uint8_t)(((len + 2) >> BYTE_SIZE) & 0xFF);
	write_buffer[2] = (uint8_t)((len + 2) & 0xFF);
	write_buffer[3] = SDID_TRANSFER;
	write_buffer[4] = time & 0xFF;
	if (len >= LENGTH_TRANSMISSION) {
		for (i = offset; i < MCU_TRANSFER_LENGTH; i++)
			write_buffer[i] = p_curdata[i - offset];
		length = MCU_TRANSFER_LENGTH;
	} else {
		for (i = offset; i < len + offset; i++)
			write_buffer[i] = p_curdata[i - offset];
		length = len + offset;
	}

	ret = drv_panel_mcu_flash_write(SLAVE_DEVICE_ADDRESS, I2C_CTR_DATA,
		write_buffer, length);
	if (ret < 0) {
		*status = ((uint32_t)RETURN_ERR_SID << SHIFT_SIXTEEN) | ((uint32_t)SDID_TRANSFER << SHIFT_EIGHT) |
			((uint32_t)RETURN_TIMEOUT_SID);
		cdc_err("UPDATE_ERR: %s I2C write error", __func__);
		return RET_WAIT;
	}

	mdelay(DELAY_TIME);

	ret = drv_panel_mcu_multiple_read(I2C_CTR_DATA, read_buffer, MCU_TRANSFER_READ, status);
	if (ret == RET_SUCCESS)
		cdc_info("UPDATE_INFO: %s succ", __func__);
	return ret;
}

int32_t drv_panel_mcu_ota_update(const uint8_t *version_bin, uint32_t len, uint32_t *status)
{
	int32_t i;
	int32_t ret = 0;
	int32_t time;
	int32_t last_data;

	if ((NULL == version_bin) || (!status)) {
		cdc_err("UPDATE_ERR: drv_panel_mcu_ota_update parameter error!");
		return RET_FAIL;
	}
	cdc_info("Entry %s +\n", __func__);

	const uint8_t *p_curdata = version_bin;
	time = len / LENGTH_TRANSMISSION;
	for (i = 0; i < time; i++) {
		ret = drv_panel_mcu_write_for_update((p_curdata + i * LENGTH_TRANSMISSION), LENGTH_TRANSMISSION, i + 1, status);
		if (ret < 0) {
			cdc_err("UPDATE_ERR: %dth data transfer failed", i);
			return ret;
		}
		g_process = (i * HUNDRED)/time;
	}

	last_data = len % LENGTH_TRANSMISSION;
	if (last_data > 0) {
		ret = drv_panel_mcu_write_for_update(p_curdata + time * LENGTH_TRANSMISSION, last_data, i + 1, status);
		if (ret < 0) {
			cdc_err("UPDATE_ERR: last 4KB data transfer failed");
			return ret;
		}
	}
	g_process = HUNDRED - 1;
	cdc_info("Exit %s +\n", __func__);
	return ret;
}

int32_t drv_panel_mcu_exit_transfer(uint32_t *status)
{
	int32_t i;
	int32_t ret;
	uint8_t write_buffer[MCU_EXIT_LENGTH];
	uint8_t read_buffer[MCU_EXIT_READ];
	unsigned long temp = MCU_EXIT_TRANSFER;

	if (!status) {
		cdc_err("UPDATE_ERR:mcu request download status is null");
		return RET_FAIL;
	}
	cdc_info("Entry %s +\n", __func__);

	for (i = 0; i < MCU_EXIT_LENGTH; i++)
		write_buffer[MCU_EXIT_LENGTH - i - 1] =
			(uint8_t)((temp >> (i * BYTE_SIZE)) & 0xFF);

	ret = drv_panel_mcu_flash_write(SLAVE_DEVICE_ADDRESS, I2C_CTR_EXIT,
		write_buffer, MCU_EXIT_LENGTH);
	if (ret < 0) {
		*status = ((uint32_t)RETURN_ERR_SID << SHIFT_SIXTEEN) | ((uint32_t)SDID_EXIT << SHIFT_EIGHT) |
			((uint32_t)RETURN_TIMEOUT_SID);
		cdc_err("UPDATE_ERR: %s I2C write error", __func__);
		return RET_WAIT;
	}

	mdelay(DELAY_TIME);

	ret = drv_panel_mcu_multiple_read(I2C_CTR_EXIT, read_buffer, MCU_EXIT_READ, status);
	if (ret == RET_SUCCESS)
		cdc_info("UPDATE_INFO:%s succ", __func__);
	cdc_info("Exit %s +\n", __func__);
	return ret;
}

int32_t drv_panel_mcu_crc_checksum(uint32_t crc_val, uint32_t *status)
{
	int32_t i;
	int32_t ret;
	int32_t offset = 4;
	uint8_t write_buffer[MCU_CRC_LENGTH];
	uint8_t read_buffer[MCU_CRC_READ];
	unsigned long temp = MCU_CRC;

	if (!status) {
		cdc_err("UPDATE_ERR:mcu request download status is null");
		return RET_FAIL;
	}
	cdc_info("Entry %s +\n", __func__);

	for (i = 0; i < MCU_CRC_LENGTH; i++) {
		if (i < offset) {
			write_buffer[MCU_CRC_LENGTH - i - 1] =
				(uint8_t)((crc_val >> (BYTE_SIZE * i)) & 0xFF);
		} else {
			write_buffer[MCU_CRC_LENGTH - i - 1] =
				(uint8_t)((temp >> ((i - offset) * BYTE_SIZE)) & 0xFF);
		}
	}

	ret = drv_panel_mcu_flash_write(SLAVE_DEVICE_ADDRESS, I2C_CTR_CRC,
		write_buffer, MCU_CRC_LENGTH);
	if (ret < 0) {
		*status = ((uint32_t)RETURN_ERR_SID << SHIFT_SIXTEEN) | ((uint32_t)SDID_CRC << SHIFT_EIGHT) |
			((uint32_t)RETURN_TIMEOUT_SID);
		cdc_err("UPDATE_ERR: %s I2C write error", __func__);
		return RET_WAIT;
	}

	mdelay(CHECKSUM_DELAY_TIME);

	ret = drv_panel_mcu_multiple_read(I2C_CTR_CRC, read_buffer, MCU_CRC_READ, status);
	if (ret < 0) {
		*status = ((uint32_t)RETURN_ERR_SID << SHIFT_SIXTEEN) | ((uint32_t)SDID_CRC << SHIFT_EIGHT) |
			((uint32_t)(*status) & 0xFF);
	} else {
		g_process = HUNDRED;
		cdc_info("UPDATE_INFO:%s succ", __func__);
	}
	cdc_info("Exit %s +\n", __func__);
	return ret;
}

int32_t drv_panel_mcu_get_process(void)
{
	return g_process;
}

int32_t drv_panel_mcu_set_slot(uint32_t partition, uint32_t *status)
{
	int32_t i;
	int32_t ret ;
	int32_t offset = 1;
	uint8_t write_buffer[MCU_SET_SLOT_LENGTH];
	uint8_t read_buffer[MCU_SET_SLOT_READ];
	unsigned long temp = MCU_SET_SLOT;

	if (!status) {
		cdc_err("UPDATE_ERR: mcu bootctl status is null");
		return RET_FAIL;
	}
	cdc_info("Entry %s +\n", __func__);

	for (i = 0; i < MCU_SET_SLOT_LENGTH; i++) {
		if (i < offset) {
			write_buffer[MCU_SET_SLOT_LENGTH - i - 1] =
				(uint8_t)(partition & 0xFF);
		} else {
			write_buffer[MCU_SET_SLOT_LENGTH - i - 1] =
				(uint8_t)((temp >> ((i - offset) * BYTE_SIZE)) & 0xFF);
		}
	}

	ret = drv_panel_mcu_flash_write(SLAVE_DEVICE_ADDRESS, I2C_CTR_SETSLOT,
		write_buffer, MCU_SET_SLOT_LENGTH);
	if (ret < 0) {
		*status = ((uint32_t)RETURN_ERR_SID << SHIFT_SIXTEEN) | ((uint32_t)SDID_SETSLOT << SHIFT_EIGHT) |
			((uint32_t)RETURN_TIMEOUT_SID);
		cdc_err("UPDATE_ERR: %s I2C write error", __func__);
		return RET_WAIT;
	}

	mdelay(BOOTCTL_DELAY_TIME);

	ret = drv_panel_mcu_multiple_read(I2C_CTR_SETSLOT, read_buffer, MCU_SET_SLOT_READ, status);
	if (ret < 0)
		*status = ((uint32_t)RETURN_ERR_SID << SHIFT_SIXTEEN) | ((uint32_t)SDID_SETSLOT << SHIFT_EIGHT) |
			((uint32_t)(*status) & 0xFF);
	cdc_info("UPDATE_INFO:%s succ", __func__);
	cdc_info("Exit %s +\n", __func__);
	return ret;
}

int32_t drv_panel_mcu_reset(uint32_t *status)
{
	int32_t i;
	int32_t ret;
	uint8_t write_buffer[MCU_RESET_LENGTH];
	uint8_t read_buffer[MCU_RESET_READ];
	unsigned long temp = MCU_RESET;

	if (!status) {
		cdc_err("UPDATE_ERR:mcu reset status is null");
		return RET_FAIL;
	}
	cdc_info("Entry %s +\n", __func__);

	for (i = 0; i < MCU_RESET_LENGTH; i++)
		write_buffer[MCU_RESET_LENGTH - i - 1] =
			(uint8_t)((temp >> (i * BYTE_SIZE)) & 0xFF);

	ret = drv_panel_mcu_flash_write(SLAVE_DEVICE_ADDRESS, I2C_CTR_RESET,
		write_buffer, MCU_RESET_LENGTH);
	if (ret < 0) {
		*status = ((uint32_t)RETURN_ERR_SID << SHIFT_SIXTEEN) | ((uint32_t)SDID_MCURESET << SHIFT_EIGHT) |
			((uint32_t)RETURN_TIMEOUT_SID);
		cdc_err("UPDATE_ERR: %s I2C write error", __func__);
		return RET_WAIT;
	}

	mdelay(RESET_DELAY_TIME);

	ret = drv_panel_mcu_multiple_read(I2C_CTR_RESET, read_buffer, MCU_RESET_READ, status);
	if (ret == RET_SUCCESS) {
		g_process = HUNDRED;
		cdc_info("UPDATE_INFO:%s succ", __func__);
	}
	cdc_info("Exit %s +\n", __func__);
	return ret;
}

int32_t drv_panel_mcu_check_status(uint32_t *status)
{
	int32_t ret = RET_FAIL;
	uint8_t write_buffer[MCU_CHECK_STATUS_WRITE];
	uint8_t read_buffer[MCU_CHECK_STATUS_READ];

	if (!status) {
		cdc_err("UPDATE_ERR: panel mcu check status is null");
		return RET_FAIL;
	}
	cdc_info("Entry %s +\n", __func__);

	write_buffer[0] = ((uint32_t)REG_CHECK_STATUS << 1)|0x01;

	ret = drv_panel_mcu_flash_write(SLAVE_DEVICE_ADDRESS, GENERAL_REGISTER,
		write_buffer, MCU_CHECK_STATUS_WRITE);
	if (ret < 0) {
		*status = RETURN_TIMEOUT_SID;
		cdc_err("UPDATE_ERR: %s I2C write error", __func__);
		return RET_WAIT;
	}
	mdelay(DELAY_TIME);

	ret = drv_panel_mcu_flash_read(SLAVE_DEVICE_ADDRESS, GENERAL_REGISTER,
		read_buffer, MCU_CHECK_STATUS_READ);
	if (ret < 0) {
		*status = RETURN_TIMEOUT_SID;
		cdc_err("UPDATE_ERR: panel mcu check status I2C read error");
		return RET_FAIL;
	}

	*status = read_buffer[0];
	cdc_info("Exit %s +\n", __func__);
	return ret;
}
