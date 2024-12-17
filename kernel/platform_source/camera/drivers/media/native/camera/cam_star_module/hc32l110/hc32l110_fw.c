// SPDX-License-Identifier: GPL-2.0
/* hc32l110_fw.c
 *
 * hc32l110 firmware driver
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
#include "../cam_sensormotor_i2c.h"
#include "cam_log.h"
#include <linux/firmware.h>
#include <linux/delay.h>
#include <securec.h>


#define HWLOG_TAG cam_star_hc32l110_fw
HWLOG_REGIST();

int hc32l110_fw_write_cmd(struct cam_star_module_device_info *di, u16 cmd)
{
	int i;
	u8 ack;
	u8 buf[HC32L110_FW_CMD_SIZE] = { 0 };

	/* fill cmd */
	buf[0] = cmd >> 8;
	buf[1] = cmd & 0xFF;

	if (i2c_write_block(di->client, buf, HC32L110_FW_CMD_SIZE))
		return -EIO;

	for (i = 0; i < HC32L110_FW_ACK_COUNT; i++) {
		ack = 0;
		cam_usleep(500);
		(void)i2c_read_block_without_cmd(di->client, &ack, 1);
		if (ack == HC32L110_FW_ACK_VAL) {
			hwlog_info("write_cmd succ: i = %d cmd = %x ack = %x\n", i, cmd, ack);
			return 0;
		}
	}

	hwlog_err("write_cmd fail: i = %d cmd = %x ack = %x\n", i, cmd, ack);
	return -EIO;
}

int hc32l110_fw_write_addr(struct cam_star_module_device_info *di, u32 addr)
{
	int i;
	u8 ack;
	u8 buf[HC32L110_FW_ADDR_SIZE] = { 0 };

	/* fill address */
	buf[0] = addr >> 24;
	buf[1] = (addr >> 16) & 0xFF;
	buf[2] = (addr >> 8) & 0xFF;
	buf[3] = (addr >> 0) & 0xFF;
	buf[4] = buf[0];
	for (i = 1; i < HC32L110_FW_ADDR_SIZE - 1; i++)
		buf[4] ^= buf[i];

	if (i2c_write_block(di->client, buf, HC32L110_FW_ADDR_SIZE))
		return -EIO;

	for (i = 0; i < HC32L110_FW_ACK_COUNT; i++) {
		ack = 0;
		cam_usleep(500);
		(void)i2c_read_block_without_cmd(di->client, &ack, 1);
		if (ack == HC32L110_FW_ACK_VAL) {
			hwlog_info("write_addr succ: i = %d addr = %x ack = %x\n", i, addr, ack);
			return 0;
		}
	}

	hwlog_err("write_addr fail: i = %d addr = %x ack = %x\n", i, addr, ack);
	return -EIO;
}

static int hc32l110_fw_erase(struct cam_star_module_device_info *di)
{
	int ret;
	int i, j, k, l;
	u8 ack;
	u8 buf[HC32L110_FW_ERASE_CMD1_SIZE] = { 0 };
	u8 index_buf[HC32L110_ERASE_PAGE_NUM * 2 + 1] = {0};

	ret = hc32l110_fw_write_cmd(di, HC32L110_FW_ERASE_CMD);
	if (ret)
		return ret;

	// number of pages - 1: HC32L110_ERASE_PAGE_NUM - 1
	buf[0] = (HC32L110_ERASE_PAGE_NUM - 1) >> 8;
	buf[1] = (HC32L110_ERASE_PAGE_NUM - 1) & 0xFF;
	buf[2] = buf[0] ^ buf[1];
	if (i2c_write_block(di->client, buf, HC32L110_FW_ERASE_CMD1_SIZE))
		return -EIO;

	for (i = 0; i < HC32L110_FW_ACK_COUNT; i++) {
		ack = 0;
		cam_usleep(500);
		(void)i2c_read_block_without_cmd(di->client, &ack, 1);
		if (ack == HC32L110_FW_ACK_VAL) {
			hwlog_info("erase cmd1 succ: i = %d ack = 0x%x\n", i, ack);
			break;
		}
	}
	if (i >= HC32L110_FW_ACK_COUNT) {
		hwlog_err("erase cmd1 failed!\n");
		return -EIO;
	}
	// index of page, jump page (HC32L110_ERASE_PAGE_START+(HC32L110_ERASE_PAGE_NUM-1))=62
	for (j = 0; j <= HC32L110_ERASE_PAGE_NUM - 2; j++) {
		index_buf[2 * j] =  (HC32L110_ERASE_PAGE_START + j) >> 8;
		index_buf[2 * j + 1] = (HC32L110_ERASE_PAGE_START + j) & 0xFF;
	}
	index_buf[HC32L110_ERASE_PAGE_NUM * 2 - 2] = (HC32L110_ERASE_PAGE_START + HC32L110_ERASE_PAGE_NUM) >> 8;
	index_buf[HC32L110_ERASE_PAGE_NUM * 2 - 1] = (HC32L110_ERASE_PAGE_START + HC32L110_ERASE_PAGE_NUM) & 0xFF;
	// check sum
	index_buf[HC32L110_ERASE_PAGE_NUM * 2] = index_buf[0];
	for (l = 1; l < HC32L110_ERASE_PAGE_NUM * 2; l++)
		index_buf[HC32L110_ERASE_PAGE_NUM * 2] ^= index_buf[l];

	if (i2c_write_block(di->client, index_buf, HC32L110_ERASE_PAGE_NUM * 2 + 1))
		return -EIO;

	// 0.6S~1.2S
	for (k = 0; k < HC32L110_FW_ERASE_ACK_COUNT; k++) {
		ack = 0;
		msleep(540);
		(void)i2c_read_block_without_cmd(di->client, &ack, 1);
		if (ack == HC32L110_FW_ACK_VAL) {
			hwlog_info("erase succ: i = %d ack = 0x%x\n", i, ack);
			return 0;
		}
	}

	hwlog_err("erase fail\n");
	return -EIO;
}

static int hc32l110_fw_read_data(struct cam_star_module_device_info *di, u32 addr,
	u8 *data, int len)
{
	int i;
	int ret;
	u8 ack = 0;
	u8 temp[HC32L110_FW_READ_OPTOPN_SIZE] = { 0 };

	ret = hc32l110_fw_write_cmd(di, HC32L110_FW_READ_CMD);
	ret += hc32l110_fw_write_addr(di, addr);
	if (ret)
		return -EIO;

	/* buf content: (len of data need to read - 1) + checksum */
	if (di->bootloader_id == 0x20) { // only for bootloader1.1
		temp[0] = len;
		temp[1] = temp[0];
	} else {
		temp[0] = len - 1;
		temp[1] = 0xFF ^ temp[0];
	}

	if (i2c_write_block(di->client, temp, HC32L110_FW_READ_OPTOPN_SIZE))
		return -EIO;

	for (i = 0; i < HC32L110_FW_ACK_COUNT; i++) {
		ack = 0;
		cam_usleep(2000);
		(void)i2c_read_block_without_cmd(di->client, &ack, 1);
		if (ack == HC32L110_FW_ACK_VAL) {
			hwlog_info("send read num succ: i=%d len=%d ack=%x\n", i, len, ack);
			break;
		}
	}

	if (i > HC32L110_FW_ACK_COUNT) {
		hwlog_err("read data fail\n");
		return -EIO;
	}

	return i2c_read_block_without_cmd(di->client, data, len);
}

static int hc32l110_fw_write_data(struct cam_star_module_device_info *di,
	const u8 *data, int len)
{
	int i;
	u8 ack;
	u8 checksum = len - 1;
	u8 buf[HC32L110_FW_PAGE_SIZE + 2] = {0};

	if ((len > HC32L110_FW_PAGE_SIZE) || (len <= 0)) {
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

	for (i = 0; i < HC32L110_FW_ACK_COUNT; i++) {
		ack = 0;

		cam_usleep(7000);

		(void)i2c_read_block_without_cmd(di->client, &ack, 1);
		if (ack == HC32L110_FW_ACK_VAL) {
			hwlog_info("write_data succ: i=%d len=%d ack=%x\n", i, len, ack);
			return 0;
		}
	}

	hwlog_err("write_data fail\n");
	return -EIO;
}

static int hc32l110_fw_set_checksum_byte(struct cam_star_module_device_info *di)
{
	int ret;
	u8 buf[LEN_U32_DAT] = {0};
	buf[0] = di->fw_state.version_id & 0xff;
	buf[1] = di->fw_state.version_id >> 8;
	buf[2] = di->fw_state.version_id >> 16;
	buf[3] = di->fw_state.version_id >> 24;

	ret = hc32l110_fw_write_cmd(di, HC32L110_FW_WRITE_CMD);
	ret += hc32l110_fw_write_addr(di, HC32L110_FW_VERSION_ID_ADDR);
	ret += hc32l110_fw_write_data(di, buf, sizeof(buf));
	return ret;
}

static int hc32l110_download_firmware(struct cam_star_module_device_info *di)
{
	const struct firmware *hc32l110_firmware = NULL;
	const char *fw_name = "hc32l110_firmware.bin";
	uint32_t data_size = 0;
	int rc = 0;

	rc = request_firmware(&hc32l110_firmware, fw_name, di->dev);
	if (rc || hc32l110_firmware == NULL) {
		hwlog_err("firmware request failed!\n");
		return rc;
	} else {
		hwlog_info("firmware request succeeded!\n");
	}
	data_size = (uint32_t)hc32l110_firmware->size;
	if (data_size <= 0) {
		hwlog_err("firmware size error!\n");
		return -EINVAL;
	}
	di->fw_state.fw_size = data_size;
	hwlog_info("hc32l110_firmware size:%d\n", data_size);
	di->fw_state.fw_data = (u8 *)kzalloc(data_size, GFP_KERNEL);
	if (!di->fw_state.fw_data)
		return -ENOMEM;

	rc = memcpy_s(di->fw_state.fw_data, data_size, hc32l110_firmware->data, hc32l110_firmware->size);
	if (rc) {
		hwlog_err("firmware memcpy failed!\n");
		kfree(di->fw_state.fw_data);
		return -ENOMEM;
	}
	release_firmware(hc32l110_firmware);
	return rc;
}

static int hc32l110_write_firmware_data(struct cam_star_module_device_info *di)
{
	int ret;
	int size;
	int offset = 0;
	int remaining = di->fw_state.fw_size;
	u32 addr = HC32L110_FW_WRITE_ADDR;
	hwlog_info("%s start write firmware\n", __func__);

	while (remaining > 0) {
		ret = hc32l110_fw_write_cmd(di, HC32L110_FW_WRITE_CMD);
		ret += hc32l110_fw_write_addr(di, addr);
		size = (remaining > HC32L110_FW_PAGE_SIZE) ? HC32L110_FW_PAGE_SIZE : remaining;
		ret += hc32l110_fw_write_data(di, di->fw_state.fw_data + offset, size);
		if (ret) {
			hwlog_err("write firmware data fail\n");
			return ret;
		}

		offset += size;
		remaining -= size;
		addr += size;
	}
	hwlog_info("%s end write firmware\n", __func__);
	return 0;
}

int hc32l110_fw_check_bootloader_mode(struct cam_star_module_device_info *di)
{
	int ret;
	u8 ack = 0;
	u8 data = 0;

	/* write ver cmd and wait ack */
	ret = hc32l110_fw_write_cmd(di, HC32L110_FW_GET_VER_CMD);
	if (ret) {
		hwlog_err("not work bootloader mode\n");
		return -EIO;
	}

	/* get data and wait ack */
	ret = i2c_read_block_without_cmd(di->client, &data, 1);
	ret += i2c_read_block_without_cmd(di->client, &ack, 1);
	hwlog_info("get_data=0x%x ack=0x%x\n", data, ack);
	if (ret) {
		hwlog_err("not work bootloader mode\n");
		return -EIO;
	}

	hwlog_info("work bootloader mode\n");
	return 0;
}

static int hc32l110_fw_program_begin(struct cam_star_module_device_info *di)
{
	int i;

	for (i = 0; i < HC32L110_FW_RETRY_COUNT; i++) {
		if (!hc32l110_fw_check_bootloader_mode(di))
			return 0;
	}

	return -EINVAL;
}

static int hc32l110_fw_update(struct cam_star_module_device_info *di)
{
	int i;

	for (i = 0; i < HC32L110_FW_RETRY_COUNT; i++) {
		hwlog_info("fw update start, try times i = %d\n", i);
		if (hc32l110_fw_program_begin(di)) // need to del bootloader id
			continue;

		if (hc32l110_download_firmware(di))
			continue;

		if (hc32l110_fw_erase(di))
			continue;

		if (hc32l110_write_firmware_data(di))
			continue;

		if (hc32l110_fw_set_checksum_byte(di))
			continue;

		hwlog_info("fw update success.");
		return 0;
	}

	return -EINVAL;
}

static bool hc32l110_fw_need_update(struct cam_star_module_device_info *di)
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
		} else {
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

int hc32l110_fw_update_check(struct cam_star_module_device_info *di)
{
	u8 buf[LEN_U32_DAT] = {0};
	int rc = 0;

	if (hc32l110_fw_program_begin(di))
		return -EINVAL;

	if (hc32l110_fw_read_data(di, HC32L110_FW_VERSION_ID_ADDR, buf, sizeof(buf)))
		return -EINVAL;

	di->fw_state.version_id = (buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24));
	/* empty SOC readout 0xFFFF */
	hwlog_info("%s mcu fw version_id: %u\n", __func__, di->fw_state.version_id);

	if (!hc32l110_fw_need_update(di))
		hwlog_info("don't need update firmware\n");
	else
		rc = hc32l110_fw_update(di);

	return rc;
}
