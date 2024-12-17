/*
 * Thp driver code for raydium
 *
 * Copyright (c) 2012-2023 Huawei Technologies Co., Ltd.
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include "huawei_thp.h"

#define RAYDIUM_IC_NAME "raydium"
#define THP_RAYDIUM_DEV_NODE_NAME "raydium_thp"

#define RAYDIUM_RESET_LOW_DELAY_MSEC 1
#define RAYDIUM_RESET_HI_DELAY_MSEC 100
#define CHIP_DETECT_RETRY_NUMB 3

/* SPI bus slave address(ID) */
#define MASK_8BIT 0xFF
#define RAD_SPI_PDA_ADDRESS_LENGTH 5
#define RAYDIUM_SPI_EID (0x5A)
#define RAYDIUM_SPI_NID (0x39)
#define RAYDIUM_SPI_RID (0xB9)
#define RAD_SPI_PDA_MODE_ENABLE 0x80
/* Using byte mode due to data might be not word-aligment */
#define RAD_SPI_PDA_MODE_WORD_MODE 0x40


#define MAX_WRITE_PACKET_SIZE 128
#define MAX_READ_PACKET_SIZE 256
#define RAYDIUM_READ_FT_DATA_ADDR 0x2000019C
#define RAYDIUM_PDA_COMMAND_ADDR 0x20000288
#define RAYDIUM_PDA_STATUS_ADDR 0x20000290
#define RAYDIUM_MISCIER_ADDR 0x40000014
#define RAYDIUM_CHK_PID_CMD 0x50000064
#define RAYDIUM_CHK_SPI_CMD 0x500009BC
#define RAD_REPORT_STATUS_LEN 4
#define RAD_RAW_DATA_LEN 96
#define RAD_FRAME_DATA_LEN 124
#define RAD_SYS_CMD_LEN 2
#define RAD_FRAME_CRC_LEN 4
#define RAD_CRC_KEY 0xedb88320

#define TP_FRM_BUF_POS 0
#define TP_FRM_FT_CMD_POS 1
#define TP_FRM_IDX_POS 96
#define TP_FRM_SCAN_FREQ_POS 98
#define TP_FRM_SCAN_RATE_POS 100
#define TP_FRM_AFE_STATUS_POS 102
#define TP_FRM_SW_NOISE_POS 104
#define TP_FRM_GESTURE_POS 106
#define TP_FRM_CRC_POS 110

#define RAD_PALM_DISABLE 0x00
#define RAD_PALM_ENABLE 0x01
#define RAD_WAKE_UP 0x02

#define RAD_IC_NAME_LEN 2
#define RAD_PRODUCT_NAME_LEN 4
#define RAD_READ_CMD 0x80
#define RAD_WRITE_CMD 0x00
#define RAD_SEND_CMD_RETEY 30
#define SPI_RETRY_NUMBER 3
#define RAD_TX_BUFF_LEN 2
#define RAYDIUM_DMA_ENABLE 0x03
#define BIT_USED_BY_TSA 9
#define TSA_CHECK_TAG 0

/* Raydium host cmd */
#define RAYDIUM_HOST_CMD_NO_OP 0x00
#define RAYDIUM_HOST_CMD_PWR_SLEEP 0x30
#define RAYDIUM_HOST_CMD_DISPLAY_MODE 0x33
#define RAYDIUM_HOST_CMD_CALIBRATION 0x5C
#define RAYDIUM_HOST_CMD_TP_MODE 0x60
#define RAYDIUM_HOST_CMD_FT_MODE 0x61

/* FT APK literal */
#define RAD_HOST_CMD_POS 0x00
#define RAD_FT_CMD_POS 0x01
#define RAD_FT_CMD_LENGTH 0x02

/* FT APK data type */
#define RAYDIUM_FT_UPDATE 0x01

/* Raydium gesture cmd */
#define SYS_CMD_DOUBLE_TAP 0x40
#define SYS_CMD_SINGLE_TAP 0x41
#define SYS_CMD_LEN 4

int touch_driver_spi_set_address(struct spi_device *sdev,
	unsigned int u32_addr, unsigned char u8_mode)
{
	int ret;
	unsigned char u8_buf[RAD_SPI_PDA_ADDRESS_LENGTH];
	struct spi_message msg;
	struct spi_transfer xfer;
	unsigned char dma_rx_buf;

	u8_buf[0] = RAYDIUM_SPI_EID;
	u8_buf[1] = (u32_addr & 0x0000FF00) >> 8;
	u8_buf[2] = (u32_addr & 0x00FF0000) >> 16;
	u8_buf[3] = (u32_addr & 0xFF000000) >> 24;
	u8_buf[4] = u8_mode;

	memset(&xfer, 0, sizeof(xfer));
	xfer.tx_buf = u8_buf;
	xfer.rx_buf = &dma_rx_buf;
	xfer.len = RAD_SPI_PDA_ADDRESS_LENGTH;

	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);

	ret = thp_spi_sync(sdev, &msg);
	if (ret < 0) {
		thp_log_err("%s: SPI transfer failed, error = %d\n",
			__func__, ret);
		return -EINVAL;
	}
	return NO_ERR;
}

int touch_driver_spi_read_phase_nid(struct spi_device *sdev,
	struct spi_message msg, struct thp_device *tdev,
	unsigned char* u8_buf, struct spi_transfer xfer,
	unsigned int u32_addr, unsigned short u16_length)
{
	int ret;
	unsigned char dma_rx_buf;

	u8_buf[0] = RAYDIUM_SPI_NID;
	u8_buf[1] = u32_addr & MASK_8BIT;
	memset(&xfer, 0, sizeof(xfer));
	xfer.tx_buf = u8_buf;
	xfer.rx_buf = &dma_rx_buf;
	xfer.len = 2;

	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);

	ret = thp_spi_sync(sdev, &msg);
	if (ret < 0) {
		thp_log_err("%s: SPI transfer failed, error = %d\n",
			__func__, ret);
		return -EINVAL;
	}
	return NO_ERR;
}

int touch_driver_spi_read_phase_rid(struct spi_device *sdev,
	struct spi_message msg, struct thp_device *tdev,
	unsigned char *u8_buf, struct spi_transfer xfer,
	unsigned int u32_addr, unsigned char *u8_data,
	unsigned short u16_length)
{
	int ret;

	u8_buf[0] = RAYDIUM_SPI_RID;
	u8_buf[1] = u32_addr & MASK_8BIT;
	memset(&xfer, 0, sizeof(xfer));
	xfer.tx_buf = u8_buf;
	xfer.rx_buf = u8_data;
	xfer.len = 2 + u16_length; /* 2 means tx buff length */

	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);

	ret = thp_spi_sync(sdev, &msg);
	if (ret < 0) {
		thp_log_err("%s: error, bus_read thp_spi_sync failed, ret=%d\n",
			__func__, ret);
		return -EINVAL;
	}
	return NO_ERR;
}

void set_u8_mode_val(unsigned char *u8_mode, unsigned short u16_length)
{
	*u8_mode |= RAD_SPI_PDA_MODE_ENABLE;

	if (u16_length == 4)
		*u8_mode |= RAD_SPI_PDA_MODE_WORD_MODE;
	return;
}

int touch_driver_spi_read(struct thp_device *tdev, unsigned int u32_addr,
	char *u8_r_data, unsigned short u16_length)
{
	int ret;
	unsigned char u8_mode = RAYDIUM_DMA_ENABLE;
	unsigned char u8_buf[2];
	unsigned char u8_data[MAX_READ_PACKET_SIZE + 2];
	struct spi_device *sdev = tdev->thp_core->sdev;
	struct spi_message msg;
	struct spi_transfer xfer;

	thp_log_debug("%s: spi_addr=0x%02X, len=%d\n", __func__,
		(unsigned char)u32_addr, (unsigned char)u16_length);

	set_u8_mode_val(&u8_mode, u16_length);

	ret = thp_bus_lock();
	if (ret < 0) {
		thp_log_err("%s: get lock failed\n", __func__);
		return -EINVAL;
	}
	ret = touch_driver_spi_set_address(sdev, u32_addr, u8_mode);
	if (ret < 0) {
		thp_log_err("%s: spi set address failed, address 0x%x\n",
			__func__, u32_addr);
		goto error;
	}
	ret = touch_driver_spi_read_phase_nid(sdev, msg, tdev, u8_buf,
		xfer, u32_addr, u16_length);
	if (ret < 0) {
		thp_log_err("%s: spi read phase pid failed, address 0x%x\n",
			__func__, u32_addr);
		goto error;
	}
	ret = touch_driver_spi_read_phase_rid(sdev, msg, tdev, u8_buf,
		xfer, u32_addr, u8_data, u16_length);
	if (ret < 0) {
		thp_log_err("%s: spi read phase rid failed, address 0x%x\n",
			__func__, u32_addr);
		goto error;
	}
	memcpy(u8_r_data, u8_data + RAD_TX_BUFF_LEN, u16_length);
error:
	thp_bus_unlock();
	return ret;
}

int touch_driver_spi_write(struct thp_device *tdev, unsigned int u32_addr,
	const char *u8_w_data, unsigned short u16_length)
{
	int ret;
	unsigned char u8_mode = RAYDIUM_DMA_ENABLE;
	unsigned char u8_buf[MAX_WRITE_PACKET_SIZE + 2];
	struct spi_device *sdev = tdev->thp_core->sdev;
	struct spi_message msg;
	struct spi_transfer xfer;
	unsigned char dma_rx_buf;

	thp_log_debug("%s: spi_addr=0x%02X, len=%d\n", __func__,
		(unsigned char)u32_addr, (unsigned char)u16_length);

	set_u8_mode_val(&u8_mode, u16_length);

	ret = thp_bus_lock();
	if (ret < 0) {
		thp_log_err("%s: get lock failed\n", __func__);
		return -EINVAL;
	}
	ret = touch_driver_spi_set_address(sdev, u32_addr, u8_mode);
	if (ret < 0) {
		thp_log_err("%s: spi set address failed, address 0x%x\n",
			__func__, u32_addr);
		goto error;
	}

	u8_buf[0] = RAYDIUM_SPI_NID;
	u8_buf[1] = u32_addr & MASK_8BIT;
	memcpy(u8_buf + 2, u8_w_data, u16_length);
	memset(&xfer, 0, sizeof(xfer));
	xfer.tx_buf = u8_buf;
	xfer.rx_buf = &dma_rx_buf;
	xfer.len = 2 + u16_length;

	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);

	ret = thp_spi_sync(sdev, &msg);
	if (ret < 0)
		thp_log_err("%s: error, bus_write thp_spi_sync failed, ret=%d\n",
			__func__, ret);
error:
	thp_bus_unlock();
	return ret;
}

static int check_cmd_ack(struct thp_device *tdev)
{
	int ret;
	unsigned char r_buf[4];
	unsigned char u8_loop;

	for (u8_loop = 0; u8_loop < RAD_SEND_CMD_RETEY; u8_loop++) {
		mdelay(5); /* wait fw respone, retry delay time */
		ret = touch_driver_spi_read(tdev, RAYDIUM_PDA_COMMAND_ADDR, r_buf, 4);
		if (ret) {
			thp_log_err("%s: touch_driver_spi_read NG!\r\n", __func__);
			return -EINVAL;
		}

		if (r_buf[0] == 0) {
			thp_log_info("%s, read fw cmd success\n", __func__);
			break;
		}
	}
	return 0;
}

static int touch_driver_send_fw_cmd(struct thp_device *tdev, unsigned char *u8_cmd)
{
	int ret;
	unsigned int cmd_addr = RAYDIUM_PDA_COMMAND_ADDR;
	unsigned char w_buf[4];
	unsigned char r_buf[4];
	unsigned char u8_loop;

	thp_log_info("%s: cmd is %x\n", __func__, u8_cmd[0]);
	memcpy(w_buf, u8_cmd, SYS_CMD_LEN);

	ret = touch_driver_spi_write(tdev, cmd_addr, w_buf, 4);
	if (ret) {
		thp_log_err("%s: touch_driver_spi_write NG!\n", __func__);
		return -EINVAL;
	}
	/* SLEEP CMD bypass check status */
	if (w_buf[0] == RAYDIUM_HOST_CMD_PWR_SLEEP)
		return NO_ERR;
	/* Check if cmd was received and wait handle 25ms */
	ret = check_cmd_ack(tdev);
	if (ret) {
		thp_log_err("%s: touch_driver_spi_read NG!\r\n", __func__);
		return -EINVAL;
	}
	if (u8_loop == RAD_SEND_CMD_RETEY) {
		thp_log_err("%s send cmd fail, retry timeout!\n", __func__);
		return -EINVAL;
	}
	thp_log_info("%s send cmd success\n", __func__);
	return NO_ERR;
}

unsigned int bits_reverse(unsigned int u32_num, unsigned int bit_num)
{
	unsigned int reverse = 0;
	unsigned int u32_i = 0;

	for (u32_i = 0; u32_i < bit_num; u32_i++) {
		if (u32_num & (1 << u32_i))
			reverse |= 1 << ((bit_num - 1) - u32_i);
	}
	return reverse;
}

unsigned int rc_crc32(const char *buf, unsigned int u32_len,
	unsigned int u32_crc)
{
	unsigned int u32_i;
	unsigned char u8_flash_byte, u8_current, u8_j;

	for (u32_i = 0; u32_i < u32_len; u32_i++) {
		u8_flash_byte = buf[u32_i];
		u8_current = (unsigned char)bits_reverse(u8_flash_byte, 8);
		for (u8_j = 0; u8_j < 8; u8_j++) {
			if ((u32_crc ^ u8_current) & 0x01)
				u32_crc = (u32_crc >> 1) ^ RAD_CRC_KEY;
			else
				u32_crc >>= 1;
			u8_current >>= 1;
		}
	}
	return u32_crc;
}

unsigned int checksum(u8 *buf, unsigned int u32_len)
{
	unsigned int u32_i = 0;
	unsigned int u32_crc = 0;
	unsigned int *u32_value = NULL;

	u32_value = (unsigned int *)buf;

	for (u32_i = 0; u32_i < u32_len; u32_i++)
		u32_crc += u32_value[u32_i];
	return u32_crc;
}

static int touch_driver_check_frame_valid(u8 *buf, u16 len)
{
	unsigned int u32_crc_buf;
	unsigned int u32_crc_cal;

	u32_crc_cal = checksum(buf, len >> 2);
	memcpy(&u32_crc_buf, &buf[TP_FRM_CRC_POS], RAD_FRAME_CRC_LEN);
	if (u32_crc_cal != u32_crc_buf) {
		thp_log_err("CRC fail:cal(%x) != buf(%x)\n", u32_crc_cal, u32_crc_buf);
		return -EIO;
	}

	return NO_ERR;
}

static int touch_driver_hardware_reset(struct thp_device *tdev)
{
	if (!tdev) {
		thp_log_info("%s: input dev null\n", __func__);
		return -ENOMEM;
	}
	/* reset */
	gpio_direction_output(tdev->gpios->rst_gpio, THP_RESET_LOW);
	thp_do_time_delay(RAYDIUM_RESET_LOW_DELAY_MSEC);

	gpio_set_value(tdev->gpios->rst_gpio, THP_RESET_HIGH);
	thp_do_time_delay(RAYDIUM_RESET_HI_DELAY_MSEC);
	thp_log_info("%s: tp reset status:%u\n", __func__, tdev->gpios->rst_gpio);
	return NO_ERR;
}

static int touch_driver_check_spi_ready(struct thp_device *tdev)
{
	unsigned char u8_buf[4];
	int ret;

	ret = touch_driver_spi_read(tdev, RAYDIUM_CHK_SPI_CMD, u8_buf, 4);
	if (ret < 0) {
		thp_log_err("%s: failed\n", __func__);
		return -EINVAL;
	}
	thp_log_info("%s: check SPI : 0x%02X%02X\n", __func__, u8_buf[3], u8_buf[2]);
	if (u8_buf[3] != 0xF3 || u8_buf[2] != 0x03) {
		thp_log_err("%s: wrong chip id detected\n", __func__);
		return -EINVAL;
	}
	return NO_ERR;
}

static int touch_driver_check_pid(struct thp_device *tdev,
	char *buf, unsigned int len)
{
	unsigned char u8_buf[4];
	int ret;

	thp_log_info("%s: called\n", __func__);
	ret = touch_driver_spi_read(tdev, RAYDIUM_CHK_PID_CMD, u8_buf, 4);
	if (ret < 0) {
		thp_log_err("%s: check pid failed\n", __func__);
		return -EINVAL;
	}
	thp_log_info("RAD PID : 0x%02X%02X%02X%02X\n", u8_buf[3], u8_buf[2], u8_buf[1], u8_buf[0]);
	/*
	 * Background: The project id got from LCD driver cannot identify different flag used for tsa,
	 * need TP read from TDDI and assign value at last bit of project id.
	 * Modify description:
	 * 1. add configeration flag : pid_need_modify_for_tsa
	 * pid_need_modify_for_tsa is true means TP driver need read the last bit of project id.
	 * 2. u8_buf[0] : Last bit of project id read by spi.
	 */
	if (tdev->thp_core->pid_need_modify_for_tsa)
		tdev->thp_core->project_id[BIT_USED_BY_TSA] = u8_buf[TSA_CHECK_TAG];

	return NO_ERR;
}

static void touch_driver_dts_parse_process_template(int rc, unsigned int value, unsigned int *target,
	const char *func_name, const char *log)
{
	if (!rc) {
		*target = value;
		thp_log_info("%s:%s: %u\n", func_name, log, value);
	}
}

static void touch_driver_parse_ic_feature_config(struct device_node *rad_node, struct thp_device *tdev)
{
	u32 value;
	int rc;

	tdev->thp_core->pid_need_modify_for_tsa = 0;
	rc = of_property_read_u32(rad_node, "pid_need_modify_for_tsa", &value);
	touch_driver_dts_parse_process_template(rc, value, &(tdev->thp_core->pid_need_modify_for_tsa),
		__func__, "pid_need_modify_for_tsa");

	tdev->thp_core->spi_in_suspend_return_negative = 0;
	rc = of_property_read_u32(rad_node, "spi_in_suspend_return_negative", &value);
	touch_driver_dts_parse_process_template(rc, value, &(tdev->thp_core->spi_in_suspend_return_negative),
		__func__, "spi_in_suspend_return_negative");
}

static int touch_driver_init(struct thp_device *tdev)
{
	struct thp_core_data *cd = NULL;
	struct device_node *rad_node = NULL;

	thp_log_info("%s: called\n", __func__);
	if (!tdev) {
		thp_log_info("%s: input dev null\n", __func__);
		return -ENOMEM;
	}
	cd = tdev->thp_core;
	rad_node = of_get_child_by_name(cd->thp_node,
		THP_RAYDIUM_DEV_NODE_NAME);
	if (!rad_node) {
		thp_log_info("%s: dev not config in dts\n",
			__func__);
		return -ENODEV;
	}

	if (thp_parse_spi_config(rad_node, cd))
		thp_log_err("%s: spi config parse fail\n",
			__func__);
	if (thp_parse_timing_config(rad_node, &tdev->timing_config))
		thp_log_err("%s: timing config parse fail\n",
			__func__);
	touch_driver_parse_ic_feature_config(rad_node, tdev);
	return NO_ERR;
}

static int touch_driver_chip_detect(struct thp_device *tdev)
{
	int ret;
	unsigned char u8_i = 0;

	thp_log_info("%s: call\n", __func__);
	if (!tdev) {
		thp_log_info("%s: input dev null\n", __func__);
		return -ENOMEM;
	}

	for (u8_i = 0; u8_i < CHIP_DETECT_RETRY_NUMB; u8_i++) {
		ret = touch_driver_hardware_reset(tdev);
		if (ret) {
			thp_log_err(":%s hw reset fail\n", __func__);
			continue;
		}
		ret = touch_driver_check_spi_ready(tdev);
		if (ret < 0) {
			thp_log_err(":%s Check SPI failed\n", __func__);
			continue;
		}
		return NO_ERR;
	}
	if (tdev->thp_core->fast_booting_solution) {
		kfree(tdev->tx_buff);
		tdev->tx_buff = NULL;
		kfree(tdev->rx_buff);
		tdev->rx_buff = NULL;
		kfree(tdev);
		tdev = NULL;
	}
	return -EIO;
}

static int touch_driver_read_thp_frame(struct thp_device *tdev,
	char *rx_buf, unsigned int len)
{
	int ret;
	unsigned char u8_rbuffer[MAX_READ_PACKET_SIZE];

	memset(u8_rbuffer, 0x00, MAX_READ_PACKET_SIZE);

	ret = touch_driver_spi_read(tdev, RAYDIUM_READ_FT_DATA_ADDR,
		u8_rbuffer, len);
	if (ret < 0) {
		thp_log_err("%s: spi read fail\n", __func__);
		return -EINVAL;
	}

	if ((u8_rbuffer[RAD_FT_CMD_POS] & RAYDIUM_FT_UPDATE) == RAYDIUM_FT_UPDATE) {
		memcpy(rx_buf, u8_rbuffer + RAD_SYS_CMD_LEN, RAD_FRAME_DATA_LEN);
	} else {
		thp_log_err("%s wait FT_UPDATE ready fail\n", __func__);
		return -EINVAL;
	}
	return NO_ERR;
}

static int touch_driver_get_frame_input_check(struct thp_device *tdev,
	char *buf, unsigned int len)
{
	if (!tdev) {
		thp_log_info("%s: input dev null\n", __func__);
		return -ENOMEM;
	}
	if (!tdev->tx_buff || !tdev->rx_buff) {
		thp_log_err("%s: out of memory\n", __func__);
		return -ENOMEM;
	}
	if (!buf) {
		thp_log_info("%s: input buf null\n", __func__);
		return -ENOMEM;
	}

	if ((!len) || (len >= THP_MAX_FRAME_SIZE - 1)) {
		thp_log_info("%s: read len: %u illegal\n", __func__, len);
		return -ENOMEM;
	}
	return NO_ERR;
}
static int touch_driver_get_frame(struct thp_device *tdev,
	char *buf, unsigned int len)
{
	int ret;
	unsigned char *r_buf = NULL;

	ret = touch_driver_get_frame_input_check(tdev, buf, len);
	if (ret < 0) {
		thp_log_err("%s: input check error\n", __func__);
		return -EIO;
	}
	r_buf = tdev->rx_buff;
	memset(tdev->rx_buff, 0, THP_MAX_FRAME_SIZE);

	ret = touch_driver_read_thp_frame(tdev, r_buf, len + RAD_SYS_CMD_LEN);
	if (ret < 0) {
		thp_log_err("%s: spi read frame fail\n", __func__);
		return -EIO;
	}
	ret = touch_driver_check_frame_valid(r_buf, RAD_RAW_DATA_LEN);
	if (ret < 0) {
		ret = touch_driver_read_thp_frame(tdev, r_buf, len + RAD_SYS_CMD_LEN);
		if (ret < 0) {
			thp_log_err("%s: spi transfer fail\n", __func__);
			return -EIO;
		}
	}
	memcpy(buf, r_buf, len);
	return NO_ERR;
}

static int touch_driver_resume(struct thp_device *tdev)
{
	int ret;

	thp_log_info("%s: called\n", __func__);
	if (!tdev) {
		thp_log_info("%s: input dev null\n", __func__);
		return -EINVAL;
	}

	return NO_ERR;
}

static int touch_driver_wrong_touch(struct thp_device *tdev)
{
	if ((!tdev) || (!tdev->thp_core)) {
		thp_log_err("%s: input dev is null\n", __func__);
		return -EINVAL;
	}

	if (tdev->thp_core->support_gesture_mode) {
		mutex_lock(&tdev->thp_core->thp_wrong_touch_lock);
		tdev->thp_core->easy_wakeup_info.off_motion_on = true;
		mutex_unlock(&tdev->thp_core->thp_wrong_touch_lock);
		thp_log_info("%s, done\n", __func__);
	}
	return NO_ERR;
}

static int touch_driver_read_gesture_data(struct thp_device *tdev)
{
	int ret;
	int i;
	unsigned char *r_buf = NULL;

	thp_log_info("%s: called\n", __func__);
	r_buf = tdev->rx_buff;
	memset(tdev->rx_buff, 0, THP_MAX_FRAME_SIZE);

	for (i = 0; i < SPI_RETRY_NUMBER; i++) {
		ret = touch_driver_read_thp_frame(tdev, r_buf,
			RAD_FRAME_DATA_LEN + RAD_SYS_CMD_LEN);
		if ((ret == 0) && ((r_buf[TP_FRM_GESTURE_POS] != 0))) {
			ret = touch_driver_check_frame_valid(r_buf, RAD_RAW_DATA_LEN);
			if (ret < 0) {
				thp_log_info("crc abnormal,retry:%d", i);
				continue;
			}
			break;
		}
		thp_log_info("addr:%x status:%x,ret:%d",
			TP_FRM_GESTURE_POS, r_buf[TP_FRM_GESTURE_POS], ret);
		ret = -EIO;
	}
	if (ret < 0) {
		thp_log_err("data read %s,status:%x,ret:%d",
			(i >= SPI_RETRY_NUMBER) ? "crc abnormal" : "fail",
			r_buf[TP_FRM_GESTURE_POS], ret);
		return -EIO;
	}

	if (r_buf[TP_FRM_GESTURE_POS] == RAD_WAKE_UP)
		return NO_ERR;
	return -EIO;
}

int touch_driver_report_gesture(struct thp_device *tdev,
	unsigned int *gesture_wakeup_value)
{
	if ((!tdev) || (!tdev->thp_core) || (!tdev->thp_core->sdev)) {
		thp_log_info("%s: input dev null\n", __func__);
		return -ENOMEM;
	}
	if ((!gesture_wakeup_value) ||
		(!tdev->thp_core->support_gesture_mode)) {
		thp_log_err("%s, gesture not support\n", __func__);
		return -EINVAL;
	}
	if (touch_driver_read_gesture_data(tdev)) {
		thp_log_err("%s: gesture data read or write fail\n", __func__);
		return -EINVAL;
	}
	mutex_lock(&tdev->thp_core->thp_wrong_touch_lock);
	if (tdev->thp_core->easy_wakeup_info.off_motion_on == true) {
		tdev->thp_core->easy_wakeup_info.off_motion_on = false;
		*gesture_wakeup_value = TS_DOUBLE_CLICK;
	}
	mutex_unlock(&tdev->thp_core->thp_wrong_touch_lock);
	return NO_ERR;
}

#ifdef CONFIG_HUAWEI_SHB_THP
#define INPUTHUB_POWER_SWITCH_START_BIT 9
#define INPUTHUB_POWER_SWITCH_START_OFFSET 1
static void touch_driver_poweroff_status(struct thp_device *tdev)
{
	struct thp_core_data *cd = thp_get_core_data(tdev->thp_core->panel_id);

	cd->poweroff_function_status =
		((cd->double_click_switch << THP_DOUBLE_CLICK_ON) |
		(cd->ring_setting_switch << THP_RING_SUPPORT) |
		(cd->ring_switch << THP_RING_ON) |
		(cd->phone_status << THP_PHONE_STATUS) |
		(cd->single_click_switch << THP_SINGLE_CLICK_ON) |
		(cd->volume_side_status << THP_VOLUME_SIDE_STATUS_LEFT));
	if (cd->aod_display_support)
		cd->poweroff_function_status |=
		(cd->aod_touch_status << THP_AOD_TOUCH_STATUS);
	if ((cd->power_switch >= POWER_KEY_OFF_CTRL) &&
		(cd->power_switch < POWER_MAX_STATUS))
	/*
	 * cd->poweroff_function_status 0~8 bit saved function flag
	 * eg:double_click, finger_status, ring_switch,and so on.
	 * cd->poweroff_function_status 9~16 bit saved screen-on-off reason flag
	 * cd->power_switch is a value(1~8) which stand for screen-on-off reason
	 */
	cd->poweroff_function_status |=
		(1 << (INPUTHUB_POWER_SWITCH_START_BIT +
		cd->power_switch - INPUTHUB_POWER_SWITCH_START_OFFSET));
}
#endif

static int touch_driver_suspend_pt_mode(struct thp_device *tdev)
{
	int ret = 0;
	unsigned char u8_wbuffer[4];

	thp_log_info("%s: suspend PT mode\n", __func__);
	u8_wbuffer[0] = RAYDIUM_HOST_CMD_PWR_SLEEP;
	ret = touch_driver_send_fw_cmd(tdev, u8_wbuffer);
	if (ret < 0)
		thp_log_err("failed enable PT mode\n");
	return ret;
}

static int touch_driver_suspend_guesture_mode(struct thp_device *tdev)
{
	int ret = 0;
	unsigned char u8_wbuffer[4] = {0};

	thp_log_info("%s: gesture mode\n", __func__);
	/* support singe tap gesture */
	if (tdev->thp_core->aod_touch_status) {
		u8_wbuffer[0] = SYS_CMD_SINGLE_TAP;
		ret = touch_driver_send_fw_cmd(tdev, u8_wbuffer);
		if (ret < 0)
			thp_log_err("failed to enter single tap mode\n");
	}
	/* support double tap gesture */
	if (tdev->thp_core->easy_wakeup_info.sleep_mode == TS_GESTURE_MODE) {
		u8_wbuffer[0] = SYS_CMD_DOUBLE_TAP;
		ret = touch_driver_send_fw_cmd(tdev, u8_wbuffer);
		if (ret < 0)
			thp_log_err("failed to enter double tap mode\n");
	}
#ifdef CONFIG_HUAWEI_SHB_THP
	if (tdev->thp_core->support_shb_thp) {
		thp_log_info("set power off status\n");
		touch_driver_poweroff_status(tdev);
	}
#endif
	return ret;
}

static int touch_driver_suspend_poweroff_mode(struct thp_device *tdev)
{
	int ret = 0;
	unsigned char u8_wbuffer[4];

	thp_log_info("%s: poweroff mode\n", __func__);
	if (tdev->thp_core->support_shb_thp)
		/* 0:all function was closed */
		tdev->thp_core->poweroff_function_status = 0;
	u8_wbuffer[0] = RAYDIUM_HOST_CMD_PWR_SLEEP;
	ret = touch_driver_send_fw_cmd(tdev, u8_wbuffer);
	if (ret < 0)
		thp_log_err("failed to enter sleep mode\n");
	return ret;
}

static int touch_driver_suspend(struct thp_device *tdev)
{
	int ret;

	thp_log_info("%s: called\n", __func__);
	if (!tdev) {
		thp_log_info("%s: input dev null\n", __func__);
		return -ENOMEM;
	}

	if (is_pt_test_mode(tdev))
		ret = touch_driver_suspend_pt_mode(tdev);
	else if (tdev->thp_core->easy_wakeup_info.sleep_mode == TS_GESTURE_MODE ||
		tdev->thp_core->aod_touch_status)
		ret = touch_driver_suspend_guesture_mode(tdev);
	else
		ret = touch_driver_suspend_poweroff_mode(tdev);
	thp_do_time_delay(tdev->timing_config.suspend_reset_after_delay_ms);

	thp_log_info("%s: called end\n", __func__);
	return NO_ERR;
}

static void touch_driver_exit(struct thp_device *tdev)
{
	thp_log_info("%s: called\n", __func__);
	if (tdev) {
		kfree(tdev->tx_buff);
		tdev->tx_buff = NULL;
		kfree(tdev->rx_buff);
		tdev->rx_buff = NULL;
		kfree(tdev);
		tdev = NULL;
	}
}


struct thp_device_ops rad_dev_ops = {
	.init = touch_driver_init,
	.get_project_id = touch_driver_check_pid,
	.detect = touch_driver_chip_detect,
	.get_frame = touch_driver_get_frame,
	.resume = touch_driver_resume,
	.suspend = touch_driver_suspend,
	.exit = touch_driver_exit,
	.chip_wrong_touch = touch_driver_wrong_touch,
	.chip_gesture_report = touch_driver_report_gesture,
};

static int thp_module_init(u32 panel_id)
{
	int rc = 0;
	struct thp_device *dev = NULL;
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev) {
		thp_log_err("%s: thp device malloc fail\n", __func__);
		return -ENOMEM;
	}

	dev->tx_buff = kzalloc(THP_MAX_FRAME_SIZE, GFP_KERNEL);
	dev->rx_buff = kzalloc(THP_MAX_FRAME_SIZE, GFP_KERNEL);
	if (!dev->tx_buff || !dev->rx_buff) {
		thp_log_err("%s: out of memory\n", __func__);
		rc = -ENOMEM;
		goto err;
	}

	dev->ic_name = RAYDIUM_IC_NAME;
	dev->dev_node_name = THP_RAYDIUM_DEV_NODE_NAME;
	dev->ops = &rad_dev_ops;
	if (cd && cd->fast_booting_solution) {
		thp_send_detect_cmd(dev, panel_id, NO_SYNC_TIMEOUT);
		thp_log_info("%s: touch driver init send detect cmd succ\n", __func__);
		return NO_ERR;
	}
	rc = thp_register_dev(dev, panel_id);
	if (rc) {
		thp_log_err("%s: register fail\n", __func__);
		goto err;
	}

	return NO_ERR;
err:
	kfree(dev->tx_buff);
	dev->tx_buff = NULL;
	kfree(dev->rx_buff);
	dev->rx_buff = NULL;
	kfree(dev);
	dev = NULL;
	return rc;
}

static int __init touch_driver_module_init(void)
{
	int rc = 0;
	struct device_node *dn = NULL;
	u32 i;

	thp_log_info("%s: called\n", __func__);
	for (i = 0; i < TOTAL_PANEL_NUM; i++) {
		dn = get_thp_node(i);
		if (dn && of_get_child_by_name(dn, THP_RAYDIUM_DEV_NODE_NAME))
			rc = thp_module_init(i);
	}
	return rc;
}


static void __exit touch_driver_module_exit(void)
{
	thp_log_info("%s: called\n", __func__);
};

module_init(touch_driver_module_init);
module_exit(touch_driver_module_exit);
MODULE_AUTHOR("huawei");
MODULE_DESCRIPTION("huawei driver");
MODULE_LICENSE("GPL");
