// SPDX-License-Identifier: GPL-2.0
/* stm32g031_fw.c
 *
 * stm32g031 firmware driver
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
#include "../cam_sensormotor_i2c.h"
#include "cam_log.h"
#include <linux/firmware.h>
#include <linux/delay.h>
#include <securec.h>


#define HWLOG_TAG cam_star_stm32g031_fw
HWLOG_REGIST();

int stm32g031_fw_write_cmd(struct cam_star_module_device_info *di, u16 cmd)
{
	int i;
	u8 ack;
	u8 buf[STM32G031_FW_CMD_SIZE] = { 0 };

	/* fill cmd */
	buf[0] = cmd >> 8;
	buf[1] = cmd & 0xFF;

	if (i2c_write_block(di->client, buf, STM32G031_FW_CMD_SIZE))
		return -EIO;

	for (i = 0; i < STM32G031_FW_ACK_COUNT; i++) {
		ack = 0;
		(void)i2c_read_block_without_cmd(di->client, &ack, 1);
		if (ack == STM32G031_FW_ACK_VAL) {
			hwlog_info("write_cmd succ: i=%d cmd=%x ack=%x\n", i, cmd, ack);
			return 0;
		}

		cam_usleep(1000);
	}

	hwlog_err("write_cmd fail: i=%d cmd=%x\n", i, cmd);
	return -EIO;
}

static int stm32g031_fw_write_addr(struct cam_star_module_device_info *di, u32 addr)
{
	int i;
	u8 ack;
	u8 buf[STM32G031_FW_ADDR_SIZE] = { 0 };

	/* fill address */
	buf[0] = addr >> 24;
	buf[1] = (addr >> 16) & 0xFF;
	buf[2] = (addr >> 8) & 0xFF;
	buf[3] = (addr >> 0) & 0xFF;
	buf[4] = buf[0];
	for (i = 1; i < STM32G031_FW_ADDR_SIZE - 1; i++)
		buf[4] ^= buf[i];

	if (i2c_write_block(di->client, buf, STM32G031_FW_ADDR_SIZE))
		return -EIO;

	for (i = 0; i < STM32G031_FW_ACK_COUNT; i++) {
		ack = 0;
		(void)i2c_read_block_without_cmd(di->client, &ack, 1);
		if (ack == STM32G031_FW_ACK_VAL) {
			hwlog_info("write_addr succ: i=%d addr=%x ack=%x\n", i, addr, ack);
			return 0;
		}

		cam_usleep(1000);
	}

	hwlog_err("write_addr fail: i=%d addr=%x\n", i, addr);
	return -EIO;
}

static int stm32g031_fw_erase(struct cam_star_module_device_info *di)
{
	int ret;
	int i, j, k, l;
	u8 ack;
	u8 buf[STM32G031_FW_ERASE_CMD1_SIZE] = { 0 };
	u8 index_buf[31 * 2 + 1] = {0};

	ret = stm32g031_fw_write_cmd(di, STM32G031_FW_ERASE_CMD);
	if (ret)
		return ret;

	// number of pages - 1: 31 - 1
	buf[0] = 0x00;
	buf[1] = 0x1E;
	buf[2] = buf[0] ^ buf[1];
	if (i2c_write_block(di->client, buf, STM32G031_FW_ERASE_CMD1_SIZE))
		return -EIO;

	for (i = 0; i < STM32G031_FW_ACK_COUNT; i++) {
		ack = 0;
		(void)i2c_read_block_without_cmd(di->client, &ack, 1);
		if (ack == STM32G031_FW_ACK_VAL) {
			hwlog_info("erase cmd1 succ: i=%d ack=%x\n", i, ack);
			break;
		}
		cam_usleep(1000);
	}
	if (i >= STM32G031_FW_ACK_COUNT) {
		hwlog_err("erase cmd1 failed!\n");
		return -EIO;
	}
	// index of page, jump page 30
	for (j = 0; j < 30; ++j) {
		index_buf[2 * j] =  0;
		index_buf[2 * j + 1] = j;
	}
	index_buf[60] = 0;
	index_buf[61] = 31;
	// check sum
	index_buf[62] = index_buf[0];
	for (l = 1; l < 31 * 2; l++)
		index_buf[62] ^= index_buf[l];

	if (i2c_write_block(di->client, index_buf, STM32G031_FW_ERASE_CMD2_SIZE))
		return -EIO;

	// 0.6S~1.2S
	for (k = 0; k < STM32G031_FW_ERASE_ACK_COUNT; k++) {
		ack = 0;
		(void)i2c_read_block_without_cmd(di->client, &ack, 1);
		if (ack == STM32G031_FW_ACK_VAL) {
			hwlog_info("erase succ: i=%d ack=%x\n", i, ack);
			return 0;
		}
		msleep(300);
	}

	hwlog_err("erase fail\n");
	return -EIO;
}

static int stm32g031_fw_read_data(struct cam_star_module_device_info *di, u32 addr,
	u8 *data, int len)
{
	int i;
	int ret;
	u8 ack = 0;
	u8 temp[STM32G031_FW_READ_OPTOPN_SIZE] = { 0 };

	ret = stm32g031_fw_write_cmd(di, STM32G031_FW_READ_CMD);
	ret += stm32g031_fw_write_addr(di, addr);
	if (ret)
		return -EIO;

	/* buf content: (len of data need to read - 1) + checksum */
	temp[0] = len - 1;
	temp[1] = 0xFF ^ temp[0];
	if (i2c_write_block(di->client, temp, STM32G031_FW_READ_OPTOPN_SIZE))
		return -EIO;

	for (i = 0; i < STM32G031_FW_ACK_COUNT; i++) {
		ack = 0;
		cam_usleep(2000);
		(void)i2c_read_block_without_cmd(di->client, &ack, 1);
		if (ack == STM32G031_FW_ACK_VAL) {
			hwlog_info("send read num succ: i=%d len=%d ack=%x\n", i, len, ack);
			break;
		}
	}

	if (i > STM32G031_FW_ACK_COUNT) {
		hwlog_err("read data fail\n");
		return -EIO;
	}

	return i2c_read_block_without_cmd(di->client, data, len);
}

static int stm32g031_fw_write_data(struct cam_star_module_device_info *di,
	const u8 *data, int len)
{
	int i;
	u8 ack;
	u8 checksum = len - 1;
	u8 buf[STM32G031_FW_PAGE_SIZE + 2] = {0};

	if ((len > STM32G031_FW_PAGE_SIZE) || (len <= 0)) {
		hwlog_err("data len illegal, len=%d\n", len);
		return -EINVAL;
	}

	/* buf content: (len of data - 1) + data + checksum */
	buf[0] = len - 1;
	for (i = 1; i <= len; i++) {
		buf[i] = data[i - 1];
		checksum ^= buf[i];
	}
	buf[len + 1] = checksum;

	if (i2c_write_block(di->client, buf, len + 2))
		return -EIO;

	for (i = 0; i < STM32G031_FW_ACK_COUNT; i++) {
		ack = 0;
		(void)i2c_read_block_without_cmd(di->client, &ack, 1);
		if (ack == STM32G031_FW_ACK_VAL) {
			hwlog_info("write_data succ: i=%d len=%d ack=%x\n", i, len, ack);
			return 0;
		}
		cam_usleep(500);
	}

	hwlog_err("write_data fail\n");
	return -EIO;
}

static int stm32g031_fw_modify_option_byte(struct cam_star_module_device_info *di)
{
	int i;
	int ret;
	u8 temp = 0;

	ret = stm32g031_fw_read_data(di, STM32G031_FW_BOOTN_ADDR, &temp, 1);
	if (ret)
		return -EIO;

	hwlog_info("option byte is 0x%x\n", temp);
	if (temp == STM32G031_FW_NBOOT_VAL)
		return 0;

	for (i = 0; i < STM32G031_FW_RETRY_COUNT; i++) {
		ret = stm32g031_fw_write_cmd(di, STM32G031_FW_WRITE_CMD);
		ret += stm32g031_fw_write_addr(di, STM32G031_FW_OPTION_ADDR);
		ret += stm32g031_fw_write_data(di, g_stm32g031_option_data, STM32G031_OPTION_SIZE);
		if (ret)
			return -EIO;

		/* wait for option byte set complete */
		msleep(30);
		ret = stm32g031_fw_read_data(di, STM32G031_FW_BOOTN_ADDR, &temp, 1);
		if (ret)
			return -EIO;

		hwlog_info("check option byte is 0x%x\n", temp);
		if (temp == STM32G031_FW_NBOOT_VAL)
			return 0;
	}

	return ret;
}

static int stm32g031_fw_set_checksum_byte(struct cam_star_module_device_info *di)
{
	int ret;
	u8 buf[LEN_U16_DAT] = {0};
	buf[0] = di->fw_state.version_id & 0xff;
	buf[1] = di->fw_state.version_id >> 8;

	ret = stm32g031_fw_write_cmd(di, STM32G031_FW_WRITE_CMD);
	ret += stm32g031_fw_write_addr(di, STM32G031_FW_VERSION_ID_ADDR);
	ret += stm32g031_fw_write_data(di, buf, 2);
	return ret;
}

static int stm32g031_download_firmware(struct cam_star_module_device_info *di)
{
	const struct firmware *stm32g031_firmware = NULL;
	const char *fw_name = "stm32g031_firmware.bin";
	uint32_t data_size = 0;
	int rc = 0;

	rc = request_firmware(&stm32g031_firmware, fw_name, di->dev);
	if (rc || stm32g031_firmware == NULL) {
		hwlog_err("firmware request failed!\n");
		return rc;
	} else {
		hwlog_info("firmware request succeeded!\n");
	}
	data_size = (uint32_t)stm32g031_firmware->size;
	if (data_size <= 0) {
		hwlog_err("firmware size error!\n");
		return -EINVAL;
	}
	di->fw_state.fw_size = data_size;
	hwlog_info("stm32g031_firmware size:%d\n", data_size);
	di->fw_state.fw_data = (u8 *)kzalloc(data_size, GFP_KERNEL);
	if (!di->fw_state.fw_data)
		return -ENOMEM;

	rc = memcpy_s(di->fw_state.fw_data, data_size, stm32g031_firmware->data, stm32g031_firmware->size);
	if (rc) {
		hwlog_err("firmware memcpy failed!\n");
		kfree(di->fw_state.fw_data);
		return -ENOMEM;
	}
	release_firmware(stm32g031_firmware);
	return rc;
}

static int stm32g031_write_firmware_data(struct cam_star_module_device_info *di)
{
	int ret;
	int size;
	int offset = 0;
	int remaining = di->fw_state.fw_size;
	u32 addr = STM32G031_FW_WRITE_ADDR;

	while (remaining > 0) {
		ret = stm32g031_fw_write_cmd(di, STM32G031_FW_WRITE_CMD);
		ret += stm32g031_fw_write_addr(di, addr);
		size = (remaining > STM32G031_FW_PAGE_SIZE) ? STM32G031_FW_PAGE_SIZE : remaining;
		ret += stm32g031_fw_write_data(di, di->fw_state.fw_data + offset, size);
		if (ret) {
			hwlog_err("write firmware data fail\n");
			return ret;
		}

		offset += size;
		remaining -= size;
		addr += size;
	}

	return 0;
}

int stm32g031_fw_check_bootloader_mode(struct cam_star_module_device_info *di)
{
	int ret;
	u8 ack = 0;
	u8 data = 0;

	/* write ver cmd and wait ack */
	ret = stm32g031_fw_write_cmd(di, STM32G031_FW_GET_VER_CMD);
	if (ret) {
		hwlog_err("not work bootloader mode\n");
		return -EIO;
	}

	/* get data and wait ack */
	ret = i2c_read_block_without_cmd(di->client, &data, 1);
	ret += i2c_read_block_without_cmd(di->client, &ack, 1);
	hwlog_info("get_data=%x ack=0x%x\n", data, ack);
	if (ret) {
		hwlog_err("not work bootloader mode\n");
		return -EIO;
	}

	hwlog_info("work bootloader mode\n");
	return 0;
}

static int stm32g031_fw_program_begin(struct cam_star_module_device_info *di)
{
	int i;

	for (i = 0; i < STM32G031_FW_RETRY_COUNT; i++) {
		if (!stm32g031_fw_check_bootloader_mode(di))
			return 0;
	}

	return -EINVAL;
}

static int stm32g031_fw_program_end(struct cam_star_module_device_info *di)
{
	int ret;

	/* enable pin pull low */
	(void)gpio_direction_output(di->power_setting.mcu_boot_gpio, 0);

	/* write go cmd */
	ret = stm32g031_fw_write_cmd(di, STM32G031_FW_GO_CMD);
	ret += stm32g031_fw_write_addr(di, STM32G031_FW_WRITE_ADDR);

	/* wait for program jump */
	msleep(30);

	return ret;
}

static int stm32g031_fw_update(struct cam_star_module_device_info *di)
{
	int i;

	for (i = 0; i < STM32G031_FW_RETRY_COUNT; i++) {
		hwlog_info("fw update start, try times i = %d\n", i);
		if (stm32g031_fw_program_begin(di))
			continue;

		if (stm32g031_download_firmware(di))
			continue;

		if (stm32g031_fw_modify_option_byte(di))
			continue;

		if (stm32g031_fw_erase(di))
			continue;

		if (stm32g031_write_firmware_data(di))
			continue;

		if (stm32g031_fw_set_checksum_byte(di))
			continue;

		if (stm32g031_fw_program_end(di))
			continue;

		hwlog_info("fw update success.");
		return 0;
	}

	// stm32g031_fw_change_mode(di, STM32G031_FW_GPIO_LOW);
	return -EINVAL;
}

static bool stm32g031_fw_need_update(struct cam_star_module_device_info *di)
{
	if (di->fw_update_state == UPDATE_WHEN_NOT_EQUAL) {
		hwlog_info("fw update when fw version not equal MCU fw version\n");
		if (di->fw_state.fw_ver_id == di->fw_state.version_id) {
			return false;
		} else {
			di->fw_state.version_id = di->fw_state.fw_ver_id;
			return true;
		}
	} else if (di->fw_update_state == UPDATE_WHEN_GREATER_THAN) {
		hwlog_info("fw update when fw version greater than MCU fw version\n");

		if (di->fw_state.fw_ver_id > di->fw_state.version_id) {
			di->fw_state.version_id = di->fw_state.fw_ver_id;
			return true;
		}
		else {
			return false;
		}
	} else if (di->fw_update_state == UPDATE_ALWAYS) {
		hwlog_info("fw update every time!\n");
		return true;
	} else {
		hwlog_info("hc32l110 do not update.\n");
		return false;
	}
}

int stm32g031_fw_update_check(struct cam_star_module_device_info *di)
{
	u8 buf[LEN_U16_DAT] = {0};
	int rc = 0;

	if (stm32g031_fw_program_begin(di))
		return -EINVAL;

	if (stm32g031_fw_read_data(di, STM32G031_FW_VERSION_ID_ADDR, buf, sizeof(buf)))
		return -EINVAL;

	di->fw_state.version_id = (buf[0] | (buf[1] << 8));
	/* empty SOC readout 0xFFFF */
	hwlog_info("%s MCU fw version_id: %x\n", __func__, di->fw_state.version_id);

	if (!stm32g031_fw_need_update(di)) {
		hwlog_info("don't need update firmware\n");
		return rc;
	}

	rc = stm32g031_fw_update(di);
	return rc;
}
