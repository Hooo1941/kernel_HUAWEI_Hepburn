/*
 * Thp driver code for focaltech
 *
 * Copyright (c) 2012-2050 Huawei Technologies Co., Ltd.
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

#define FOCALTECH_IC_NAME "focaltech"
#define THP_FOCALTECH_DEV_NODE_NAME "focaltech"

#define DEFAULT_TOUCH_PANEL 0
#define DATA_HEAD 0x3F
#define DATA_MASK 0xFF
#define COMOND_HEAD 0xC0
#define COMOND_READ_ONE 0x81
#define COMOND_WRITE_ONE 0x01
#define COMOND_ADDR_RESET 0x55
#define COMMOND_READ_DATA 0x3A
#define COMMOND_READ_CHIP_ID_H 0xA3
#define COMOND_ADDR_DETECT 0x90
#define COMMAND_READ_PID_3683G 0xC2
#define COMMAND_READ_PID_3680 0x8B
#define COMMAND_ADDR_PID 0x8A
#define COMMAND_READ_PID_WAIT 0x8C
#define COMMAND_WRITE_PID_ONE 0x00
#define COMMAND_WRITE_PID_TWO 0x2E
#define COMMAND_WRITE_PID_THR 0x03
#define LENGTH_DETECT 2
#define READ_LEN 4
#define CHIP_DETECT_FAILE_ONE 0x00
#define CHIP_DETECT_FAILE_TWO 0xFF
#define CHIP_DETECT_FAILE_THR 0xEF
#define CHIP_DETECT_RETRY_NUMB 3
#define DELAY_AFTER_FIRST_BYTE 30
#define PID_WAIT_VALUE 0xA5
#define POWER_CTRL_DELAY 10
#define FTS_IC_NAME_LEN 2
#define FTS_PRODUCT_NAME_LEN 4
#define FTS_READ_CMD 0xA0
#define FTS_WRITE_CMD 0x00
#define SPI_DUMMY_BYTE 0x07
#define DATA_CRC_EN 0x20
#define CRC_REG_VALUE 0xFFFF
#define CRC_CHECK_BIT_SIZE 8
#define CRC_CHECK_CODE 0x8408
#define CRC_BUF_MIN_SIZE 2
#define CRC_LOW_BIT_CHECK 0x01
#define CRC_BIT_CHECK 1
#define NO_OFFSET 0x00
#define ADDR_OFFSET 0x02
#define ADDR_RAWDATA 0x01
#define ADDR_FRAME 0x3A
#define DELAY8_AFTER_RST 8
#define DELAY12_AFTER_RST 12
#define GET_HIGH_BIT 8
#define SPI_RETRY_NUMBER 3
#define CS_HIGH_DELAY 150
#define FTS_REG_GESTURE_EN 0xD0
#define FTS_REG_GESTURE 0xD1
#define FTS_REG_GESTURE_DATA 0xD3
#define FTS_REG_LEN 1
#define FTS_OFF_STS 3
#define FTS_GESTURE_DATA_LEN 30
#define FTS_GESTURE_STR_LEN_MAX 152
#define FTS_GESTURE_ID_BIT 2
#define FTS_GESTURE_ID_VALUE 0x24
#define FTS_GESTURE_STYLUS_ID_VALUE 0x25
#define GESTURE_DATA_ENABLE 1
#define GESTURE_INFO_BIT 3
#define GESTURE_INFO_VALUE 0x80
#define BT_CONNECT_CMD_RETRY 3
#define STYLUS3_CONNECT 2
#define FTS_REG_BT 0xC5
#define SLEEP_RETRY_NUM 3
#define SLEEP_BUF_BIT 3
#define WAIT_FOR_SPI_BUS_READ_DELAY 5
#define PID_RETRY_NUMBER 3
#define PID_WAIT_RETRY_NUMBER 10
#define DELAY10_READ 10
#define DELAY20_WRITE 20
#define DOUBLE_TAP_FLAG 2
#define PID_OFFSET 2
#define DATALEN_SIZE (THP_MAX_FRAME_SIZE - SPI_DUMMY_BYTE - ADDR_OFFSET)
#define SPI_INDEX 2
#define RST_HIGH_DELAY 1
#define INVALID_ADDRESS 0X000
#define COMMAND_ADDR_PID_ONE 0xAC
#define COMMAND_READ_PID 0x03
#define OFFSET_0 0
#define OFFSET_1 1
#define OFFSET_2 2
#define MOVE_16BIT 16
#define MOVE_8BIT 8

/*
 * Here we use "fts_ic_type" to identify focal ICs' types:
 * 1.FT8615_SERIES includes ft8615 and ft8719,
 * and "support_vendor_ic_type" is 0 in dts.
 * 2.FT8720_SERIES includes ft8720 and ft8009,
 * but ft8009 has some differences in ecc and get_frame command,
 * and this series should config "support_vendor_ic_type" to 1.
 * 3.FT3680_SERIES and UNIVERSAL_SERIES belongs to no tddi,
 * but FT3680_SERIES no flash should config "support_vendor_ic_type" to 2.
 * 4.The future focal ic almost belongs to UNIVERSAL_SERIES,
 * so we have to config the "support_vendor_ic_type" to 3 in dts.
 */
enum fts_ic_type {
	FT8615_SERIES,
	FT8720_SERIES = 1,
	FT3680_SERIES = 2,
	UNIVERSAL_SERIES = 3,
};

enum fts_gesture_event {
	FTS_DOUBLE_CLICK_WAKEUP_EVENT,
	FTS_STYLUS_WAKEUP_TO_MEMO_EVENT = 1,
};

enum fts_stylus_status_cmd {
	FTS_STYLUS_CMD_DISCONNECT = 0,
	FTS_STYLUS_CMD_CONNECT = 1,
};

enum fts_return_status {
	FTS_STATUS_SUCCESS = 0,
	FTS_STYLUS_FAIL = 1,
	FTS_STYLUS_CONTINUE = 2,
	FTS_STYLUS_EXIT = 3,
};

static bool is_ic_ft8009;
static unsigned int g_aod_touch_status;
static unsigned int g_thp_udfp_status;
/*
 * Use this command with caution
 * To solve the problem that the PID burning mode is incorrect in the production line
 * The correct way to read the PID using C2 mode
 */
static u32 workarounds_get_pid_from_addr;

/* spi interface */
static int touch_driver_spi_transfer(const char *tx_buf,
	char *rx_buf, unsigned int len)
{
	int rc;
	bool flag;
	struct thp_core_data *cd = thp_get_core_data(DEFAULT_TOUCH_PANEL);
	struct spi_device *sdev = cd->sdev;
	struct spi_message msg;
	struct spi_transfer xfer1 = {
		.tx_buf = tx_buf,
		.rx_buf = rx_buf,
		.len = len,
	};
	struct spi_transfer xfer[SPI_INDEX];

	unsigned int gesture_status = cd->easy_wakeup_info.sleep_mode;
	bool no_need_work_in_suspend = cd->suspended && (!cd->need_work_in_suspend) &&
		!(is_pt_test_mode(cd->thp_dev) || (cd->support_gesture_mode &&
		((gesture_status == TS_GESTURE_MODE) || g_thp_udfp_status ||
		g_aod_touch_status)));
	if (no_need_work_in_suspend)
		return 0;
	flag = (cd->support_vendor_ic_type == FT8720_SERIES) ||
		(cd->support_vendor_ic_type == FT3680_SERIES) ||
		(cd->support_vendor_ic_type == UNIVERSAL_SERIES);
	if (flag) {
		spi_message_init(&msg);
		spi_message_add_tail(&xfer1, &msg);
	} else {
		memset(xfer, 0, sizeof(xfer));
		spi_message_init(&msg);
		xfer[0].tx_buf = &tx_buf[0];
		if (rx_buf)
			xfer[0].rx_buf = &rx_buf[0];
		xfer[0].len = 1;
		xfer[0].delay_usecs = DELAY_AFTER_FIRST_BYTE;
		spi_message_add_tail(&xfer[0], &msg);
		if (len > 1) {
			xfer[1].tx_buf = &tx_buf[1];
			if (rx_buf)
				xfer[1].rx_buf = &rx_buf[1];
			xfer[1].len = len - 1;
			spi_message_add_tail(&xfer[1], &msg);
		}
	}

	mutex_lock(&cd->spi_mutex);
	if (!cd->not_support_cs_control)
		thp_spi_cs_set(GPIO_HIGH);
	rc = spi_sync(sdev, &msg);
	mutex_unlock(&cd->spi_mutex);

	return rc;
}

static int touch_driver_hardware_reset_to_rom_tddi(struct thp_device *tdev)
{
	int rc;
	int index = 0;
	unsigned char cmd[3]; /* 3 byte once */

	if (!tdev) {
		thp_log_info("%s: input dev null\n", __func__);
		return -EINVAL;
	}
	/* reset */
	gpio_direction_output(tdev->gpios->rst_gpio, THP_RESET_LOW);
	thp_do_time_delay(tdev->timing_config.boot_reset_low_delay_ms);

	gpio_set_value(tdev->gpios->rst_gpio, THP_RESET_HIGH);
	thp_do_time_delay(tdev->timing_config.boot_reset_hi_delay_ms);

	mdelay(8); /* 8ms sequence delay */

	cmd[index++] = COMOND_HEAD;
	cmd[index++] = COMOND_WRITE_ONE;
	cmd[index++] = COMOND_ADDR_RESET;
	rc = touch_driver_spi_transfer(cmd, cmd, ARRAY_SIZE(cmd));
	if (rc) {
		thp_log_err("%s:write 0x55 command fail\n", __func__);
		return rc;
	}

	mdelay(8); /* 8ms sequence delay */
	return 0;
}

static int touch_driver_hardware_reset_to_rom(struct thp_device *tdev)
{
	int rc;
	int index = 0;
	unsigned char cmd[4] = { 0x0 }; /* 4 byte once */

	/* reset */
#ifndef CONFIG_HUAWEI_THP_MTK
	gpio_direction_output(tdev->gpios->rst_gpio, THP_RESET_LOW);
	if (workarounds_get_pid_from_addr != INVALID_ADDRESS)
		mdelay(tdev->timing_config.boot_reset_low_delay_ms);
	else
		thp_do_time_delay(tdev->timing_config.boot_reset_low_delay_ms);
	gpio_set_value(tdev->gpios->rst_gpio, THP_RESET_HIGH);
	if (workarounds_get_pid_from_addr != INVALID_ADDRESS)
		mdelay(tdev->timing_config.boot_reset_hi_delay_ms);
	else
		thp_do_time_delay(tdev->timing_config.boot_reset_hi_delay_ms);
#else
	if (tdev->thp_core->support_pinctrl == 0) {
		thp_log_info("%s: not support pinctrl\n", __func__);
		return -EINVAL;
	}
	pinctrl_select_state(tdev->thp_core->pctrl,
		tdev->thp_core->mtk_pinctrl.reset_low);
	thp_do_time_delay(tdev->timing_config.boot_reset_low_delay_ms);
	pinctrl_select_state(tdev->thp_core->pctrl,
		tdev->thp_core->mtk_pinctrl.reset_high);
	thp_do_time_delay(tdev->timing_config.boot_reset_hi_delay_ms);
#endif
	if (workarounds_get_pid_from_addr != INVALID_ADDRESS)
		mdelay(DELAY12_AFTER_RST);
	else
		mdelay(DELAY8_AFTER_RST);

	cmd[index++] = COMOND_ADDR_RESET;
	cmd[index++] = FTS_WRITE_CMD;
	cmd[index++] = 0x00; /* high bit */
	cmd[index++] = 0x00; /* low bit */
	rc = touch_driver_spi_transfer(cmd, cmd, ARRAY_SIZE(cmd));
	if (rc) {
		thp_log_err("%s:write 0x55 command fail\n", __func__);
		return rc;
	}
	if (workarounds_get_pid_from_addr != INVALID_ADDRESS)
		mdelay(DELAY12_AFTER_RST);
	else
		mdelay(DELAY8_AFTER_RST);
	return 0;
}

static u8 cal_ecc_xor8(u8 *buf, u16 buflen)
{
	u16 i;
	u8 result = 0;

	for (i = 0; i < buflen; i++)
		result ^= buf[i];

	return result;
}

static int rdata_crc_check(u8 *rdata, u16 rlen)
{
	u16 i;
	u16 j;
	u16 crc_read;
	u16 crc_calc = CRC_REG_VALUE;

	if (rdata == NULL) {
		thp_log_err("%s: rdata is NULL\n", __func__);
		return -EIO;
	}
	if (rlen < CRC_BUF_MIN_SIZE) {
		thp_log_err("%s: thp_check_frame_valid buf less than 2: %d\n",
			__func__, rlen);
		return -EIO;
	}
	for (i = 0; i < (rlen - CRC_BUF_MIN_SIZE); i++) {
		crc_calc ^= rdata[i];
		for (j = 0; j < CRC_CHECK_BIT_SIZE; j++) {
			if (crc_calc & CRC_LOW_BIT_CHECK)
				crc_calc = (crc_calc >> CRC_BIT_CHECK) ^
					CRC_CHECK_CODE;
			else
				crc_calc = (crc_calc >> CRC_BIT_CHECK);
		}
	}
	/* rlen - 1 is to get the last element of rdata array */
	crc_read = (u16)(rdata[rlen - 1] << CRC_CHECK_BIT_SIZE) +
		rdata[rlen - CRC_BUF_MIN_SIZE];
	if (crc_calc != crc_read) {
		thp_log_err("%s: ecc fail:cal %x != buf %x\n",
			__func__, crc_calc, crc_read);
		return -EIO;
	}
	return 0;
}

static int touch_driver_enabled_gesture(u8 addr, u8 value)
{
	int i;
	int rc;
	u8 w_buf[SPI_DUMMY_BYTE + FTS_REG_LEN];
	u8 r_buf[SPI_DUMMY_BYTE + FTS_REG_LEN];
	u32 txlen = 0;

	thp_log_info("%s: called\n", __func__);
	w_buf[txlen++] = addr;
	w_buf[txlen++] = FTS_WRITE_CMD;
	w_buf[txlen++] = (FTS_REG_LEN >> GET_HIGH_BIT) & 0xFF;
	w_buf[txlen++] = FTS_REG_LEN & 0xFF;
	txlen = SPI_DUMMY_BYTE + FTS_REG_LEN;
	w_buf[SPI_DUMMY_BYTE] = value;

	for (i = 0; i < SPI_RETRY_NUMBER; i++) {
		rc = touch_driver_spi_transfer(w_buf, r_buf, txlen);
		if ((rc == 0) && ((r_buf[FTS_OFF_STS] & FTS_READ_CMD) == 0)) {
			thp_log_info("%s: success\n", __func__);
			break;
		}
		thp_log_err("%s: failed addr:%x, status:%x, retry:%d, ret:%d",
			__func__, FTS_REG_GESTURE_EN, r_buf[FTS_OFF_STS], i, rc);
		rc = -EIO;
		udelay(CS_HIGH_DELAY);
	}
	return rc;
}

static int touch_driver_check_frame_valid(u8 *buf, u16 len)
{
	u16 ecc_buf;
	u16 ecc_cal;
	bool flag;
	struct thp_core_data *cd = thp_get_core_data(DEFAULT_TOUCH_PANEL);

	flag = ((cd->support_vendor_ic_type == FT8720_SERIES) ||
		(cd->support_vendor_ic_type == FT3680_SERIES) ||
		(cd->support_vendor_ic_type == UNIVERSAL_SERIES)) &&
		(!is_ic_ft8009);
	if (flag)
		return rdata_crc_check(buf, len + ADDR_OFFSET);
	if (len < 2) {
		thp_log_err("%s: buf len < 2 %d\n", __func__, len);
		return -EIO;
	}
	ecc_cal = (u16)cal_ecc_xor8(buf, len - 2);
	ecc_buf = (u16)((buf[len - 2] << 8) + buf[len - 1]);
	if (ecc_cal != ecc_buf) {
		thp_log_err("ecc fail:cal %x != buf %x\n", ecc_cal, ecc_buf);
		return -EIO;
	}

	return 0;
}

static void touch_driver_projectid_to_ic_type(struct thp_core_data *cd)
{
	char temp_buf[FTS_IC_NAME_LEN + 1] = {0};

	strncpy(temp_buf, cd->project_id + FTS_PRODUCT_NAME_LEN,
		FTS_IC_NAME_LEN);
	if (!strncmp("9D", temp_buf, FTS_IC_NAME_LEN)) {
		is_ic_ft8009 = true;
		cd->support_vendor_ic_type = FT8720_SERIES;
		cd->spi_config.mode = 0;
		cd->sdev->mode = 0;
		thp_log_info("%s: ic is 9D, spi reconfiged\n", __func__);
	}
}

static int thp_parse_extra_config(struct device_node *thp_node,
	struct thp_core_data *cd)
{
	int rc;
	unsigned int value = 0;

	if (!thp_node || !cd) {
		thp_log_info("%s: input null\n", __func__);
		return -ENODEV;
	}
	rc = of_property_read_u32(thp_node, "reset_status_in_suspend_mode",
		&value);
	if (!rc) {
		cd->reset_status_in_suspend_mode = value;
		thp_log_info("%s: reset_status_in_suspend_mode %u\n",
			__func__, value);
	}
	cd->fts_gesture_no_offset = 0;
	rc = of_property_read_u32(thp_node, "fts_gesture_no_offset", &value);
	if (!rc) {
		cd->fts_gesture_no_offset = value;
		thp_log_info("%s: gesture no offset %u\n", __func__, value);
	}

	workarounds_get_pid_from_addr = INVALID_ADDRESS;
	rc = of_property_read_u32(thp_node, "workarounds_get_pid_from_addr",
		&value);
	if (!rc) {
		workarounds_get_pid_from_addr = value;
		thp_log_info("%s: workarounds_get_pid_from_addr read\n",
			__func__);
	}

	return 0;
}

static int touch_driver_power_init(u32 panel_id)
{
	int rc;

	thp_log_info("%s:called\n", __func__);
	rc = thp_power_supply_get(THP_IOVDD, panel_id);
	if (rc)
		thp_log_err("%s: fail to get vcc power\n", __func__);
	rc = thp_power_supply_get(THP_VCC, panel_id);
	if (rc)
		thp_log_err("%s: fail to get power\n", __func__);

	return rc;
}

static int touch_driver_power_release(u32 panel_id)
{
	int rc;

	thp_log_info("%s:called\n", __func__);
	rc = thp_power_supply_put(THP_VCC, panel_id);
	if (rc)
		thp_log_err("%s: fail to release vcc power\n", __func__);
	rc = thp_power_supply_put(THP_IOVDD, panel_id);
	if (rc)
		thp_log_err("%s: fail to release power\n", __func__);

	return rc;
}

static int touch_driver_power_on(struct thp_device *tdev)
{
	int rc;
	struct thp_core_data *cd = thp_get_core_data(DEFAULT_TOUCH_PANEL);

	thp_log_info("%s:called\n", __func__);
	rc = thp_power_supply_ctrl(THP_VCC, THP_POWER_ON,
		POWER_CTRL_DELAY, tdev->thp_core->panel_id);
	if (rc)
		thp_log_err("%s:power ctrl vcc fail\n", __func__);

	rc = thp_power_supply_ctrl(THP_IOVDD, THP_POWER_ON,
		POWER_CTRL_DELAY, tdev->thp_core->panel_id);
	if (rc)
		thp_log_err("%s:power ctrl iovdd fail\n", __func__);

	if (!cd->not_support_cs_control)
		gpio_set_value(tdev->gpios->cs_gpio, GPIO_LOW);
	if (!cd->not_support_cs_control)
		gpio_set_value(tdev->gpios->cs_gpio, GPIO_HIGH);

	thp_do_time_delay(tdev->timing_config.suspend_reset_after_delay_ms);
	gpio_direction_output(tdev->gpios->rst_gpio, GPIO_LOW);
	if (rc)
		thp_log_err("%s:rst gpio fail\n", __func__);
	gpio_set_value(tdev->gpios->rst_gpio, GPIO_HIGH);

	return rc;
}

static void touch_driver_power_off(struct thp_device *tdev)
{
	int rc;
	struct thp_core_data *cd = thp_get_core_data(DEFAULT_TOUCH_PANEL);

	thp_log_info("%s:called\n", __func__);
	if (!cd->not_support_cs_control)
		gpio_set_value(tdev->gpios->cs_gpio, GPIO_LOW);
	gpio_set_value(tdev->gpios->rst_gpio, GPIO_LOW);
	thp_do_time_delay(tdev->timing_config.suspend_reset_after_delay_ms);
	rc = thp_power_supply_ctrl(THP_VCC, THP_POWER_OFF,
		POWER_CTRL_DELAY, tdev->thp_core->panel_id);
	if (rc)
		thp_log_err("%s:power ctrl vcc fail\n", __func__);
	rc = thp_power_supply_ctrl(THP_IOVDD, THP_POWER_OFF,
		POWER_CTRL_DELAY, tdev->thp_core->panel_id);
	if (rc)
		thp_log_err("%s:power ctrl fail\n", __func__);
}

static int touch_driver_init(struct thp_device *tdev)
{
	struct thp_core_data *cd = NULL;
	struct device_node *touch_node = NULL;

	thp_log_info("%s: called\n", __func__);
	if (!tdev) {
		thp_log_info("%s: input dev null\n", __func__);
		return -EINVAL;
	}
	cd = tdev->thp_core;
	touch_node = of_get_child_by_name(cd->thp_node,
		THP_FOCALTECH_DEV_NODE_NAME);
	if (!touch_node) {
		thp_log_info("%s: dev not config in dts\n",
				__func__);
		return -ENODEV;
	}

	if (thp_parse_spi_config(touch_node, cd))
		thp_log_err("%s: spi config parse fail\n",
			__func__);

	if (thp_parse_timing_config(touch_node, &tdev->timing_config))
		thp_log_err("%s: timing config parse fail\n",
			__func__);

	if (thp_parse_feature_config(touch_node, cd))
		thp_log_err("%s: feature_config fail\n", __func__);
	if (thp_parse_extra_config(touch_node, cd))
		thp_log_err("%s: extra_config fail\n", __func__);

	touch_driver_projectid_to_ic_type(cd);

	return 0;
}

static int touch_driver_spi_read_check(u8 *w_buf, u8 *r_buf, u32 txlen,
	u8 *data, u8 addr, u32 datalen)
{
	int rc;
	int i;

	thp_log_info("%s: called\n", __func__);
	for (i = 0; i < SPI_RETRY_NUMBER; i++) {
		rc = touch_driver_spi_transfer(w_buf, r_buf, txlen);
		if ((rc == 0) && ((r_buf[FTS_OFF_STS] & FTS_READ_CMD) == 0)) {
			memcpy(data, &r_buf[SPI_DUMMY_BYTE], datalen);
			thp_log_info("%s:chip id read data:0x%X,0x%X,0x%X,0x%X\n",
				__func__, data[0], data[1], data[2], data[3]);
			/* crc check */
			rc = rdata_crc_check(r_buf + SPI_DUMMY_BYTE, txlen - SPI_DUMMY_BYTE);
			if (rc < 0) {
				thp_log_info("%s: data read addr:%x crc abnormal,retry:%d",
					__func__, addr, i);
				udelay(CS_HIGH_DELAY);
				continue;
			}
			break;
		}
		rc = -EIO;
		thp_log_info("data read addr:%x status:%x,retry:%d,rc:%d",
			addr, r_buf[FTS_OFF_STS], i, rc);
		udelay(CS_HIGH_DELAY);
	}

	return rc;
}

static int touch_driver_spi_read(u8 addr, u8 *data, u32 datalen)
{
	struct thp_core_data *cd = thp_get_core_data(DEFAULT_TOUCH_PANEL);
	struct thp_device *tdev = cd->thp_dev;
	u8 *w_buf = NULL;
	u8 *r_buf = NULL;
	int rc;
	u32 txlen = 0;
	bool flag;

	thp_log_info("%s: called\n", __func__);
	flag = !tdev || !tdev->tx_buff || !tdev->rx_buff || !data;
	if (flag) {
		thp_log_info("%s: tdev/tx_buff/rx_buff/data null\n", __func__);
		return -EINVAL;
	}

	if ((!datalen) || (datalen >= DATALEN_SIZE)) {
		thp_log_info("%s: data_len: %u illegal\n", __func__, datalen);
		return -EINVAL;
	}

	w_buf = tdev->tx_buff;
	r_buf = tdev->rx_buff;
	memset(tdev->tx_buff, 0xFF, THP_MAX_FRAME_SIZE);
	memset(tdev->rx_buff, 0, THP_MAX_FRAME_SIZE);

	w_buf[txlen++] = addr;
	w_buf[txlen++] = FTS_READ_CMD;
	w_buf[txlen++] = (datalen >> GET_HIGH_BIT) & 0xFF;
	w_buf[txlen++] = datalen & 0xFF;
	txlen = SPI_DUMMY_BYTE + datalen + ADDR_OFFSET;
	rc = touch_driver_spi_read_check(w_buf, r_buf,
		txlen, data, addr, datalen);
	if (rc) {
		thp_log_err("%s: spi read check fail\n", __func__);
		return rc;
	}
	udelay(CS_HIGH_DELAY);

	return rc;
}

/* free tdev and buffers */
static void touch_driver_dev_free(struct thp_device *tdev)
{
	thp_log_info("%s: called\n", __func__);
	if (tdev) {
		kfree(tdev->rx_buff);
		tdev->rx_buff = NULL;
		kfree(tdev->tx_buff);
		tdev->tx_buff = NULL;
		kfree(tdev);
		tdev = NULL;
	}
}

static void touch_driver_chip_detect_exit(struct thp_device *tdev)
{
	if (tdev->thp_core->self_control_power) {
		touch_driver_power_off(tdev);
		touch_driver_power_release(DEFAULT_TOUCH_PANEL);
	}
	if (tdev->thp_core->fast_booting_solution)
		touch_driver_dev_free(tdev);
}

static int touch_driver_chip_detect_no_tddi(struct thp_device *tdev,
	unsigned char *cmd_no_tddi)
{
	int rc;
	int i;
	bool flag;

	thp_log_info("%s: called\n", __func__);
	touch_driver_power_init(DEFAULT_TOUCH_PANEL);
	touch_driver_power_on(tdev);
	thp_do_time_delay(tdev->timing_config.boot_reset_hi_delay_ms);
	flag = (tdev->thp_core->support_vendor_ic_type == FT3680_SERIES) ||
			(tdev->thp_core->support_vendor_ic_type == UNIVERSAL_SERIES);
	for (i = 0; i < CHIP_DETECT_RETRY_NUMB; i++) {
		if (flag) {
			rc = touch_driver_hardware_reset_to_rom(tdev);
			if (rc) {
				thp_log_err("%s:write 0x55 command fail\n", __func__);
				continue;
			}

			rc = touch_driver_spi_read(COMOND_ADDR_DETECT, cmd_no_tddi, LENGTH_DETECT);
			if (rc) {
				thp_log_err("%s:read 0x90 command fail\n", __func__);
				goto exit;
			}
			if ((cmd_no_tddi[0] == CHIP_DETECT_FAILE_ONE) ||
				(cmd_no_tddi[0] == CHIP_DETECT_FAILE_TWO) ||
				(cmd_no_tddi[0] == CHIP_DETECT_FAILE_THR)) {
				thp_log_err("%s:chip id read fail, rc=%d, i=%d, data=%x\n",
					__func__, rc, i, cmd_no_tddi[0]);
			} else {
				thp_log_info("%s:chip id read success, chip id:0x%X,0x%X\n",
					__func__, cmd_no_tddi[0], cmd_no_tddi[1]);
				return NO_ERR;
			}
			msleep(50); /* Delay 50ms to retry if reading ic id failed */
		}
	}
exit:
	touch_driver_chip_detect_exit(tdev);

	return -EIO;
}

static int touch_driver_bt_handler(struct thp_device *tdev, bool delay_enable);

static void touch_driver_stylus_connect_in_power(struct thp_device *tdev)
{
	if (tdev->thp_core->send_bt_status_to_fw)
		if (touch_driver_bt_handler(tdev, true))
			thp_log_err("power on send stylus3 connect status fail\n");
}

static enum fts_return_status touch_driver_chip_detect_b2_series(struct thp_device *tdev)
{
	int rc;
	/* 20 bytes for spi transfer */
	unsigned char cmd1[20] = {0};
	u32 txlen = 0;

	rc = touch_driver_hardware_reset_to_rom(tdev);
	if (rc) {
		thp_log_err("%s:write 0x55 command fail\n",
			__func__);
		return FTS_STYLUS_CONTINUE;
	}

	cmd1[txlen++] = COMOND_ADDR_DETECT;
	cmd1[txlen++] = FTS_READ_CMD;
	cmd1[txlen++] = 0x00; /* high bit */
	cmd1[txlen++] = 0x02; /* low bit */
	txlen = SPI_DUMMY_BYTE + ADDR_OFFSET;
	if (FTS_READ_CMD & DATA_CRC_EN)
		txlen = txlen + ADDR_OFFSET;

	rc = touch_driver_spi_transfer(cmd1, cmd1, txlen);
	if (rc) {
		thp_log_err("%s:write 0x90 command fail\n",
			__func__);
		return FTS_STYLUS_EXIT;
	}

	if ((cmd1[SPI_DUMMY_BYTE] == CHIP_DETECT_FAILE_ONE) ||
	    (cmd1[SPI_DUMMY_BYTE] == CHIP_DETECT_FAILE_TWO) ||
	    (cmd1[SPI_DUMMY_BYTE] == CHIP_DETECT_FAILE_THR)) {
		thp_log_err(
			"%s:chip id read fail, rc=%d, data=%x\n",
			__func__, rc, cmd1[SPI_DUMMY_BYTE]);
	} else {
		thp_log_info(
			"%s:chip id read success, chip id:0x%X,0x%X\n",
			__func__, cmd1[SPI_DUMMY_BYTE],
			cmd1[SPI_DUMMY_BYTE + 1]);
		touch_driver_stylus_connect_in_power(tdev);
		return FTS_STATUS_SUCCESS;
	}
	return FTS_STYLUS_FAIL;
}

static enum fts_return_status touch_driver_chip_detect_other_series(struct thp_device *tdev)
{
	int rc;
	unsigned char cmd[3]; /* 3 bytes once */

	rc = touch_driver_hardware_reset_to_rom_tddi(tdev);
	if (rc) {
		thp_log_err("write 0x55 command fail\n");
		return FTS_STYLUS_CONTINUE;
	}

	cmd[0] = COMOND_HEAD;
	cmd[1] = COMOND_READ_ONE;
	cmd[2] = COMOND_ADDR_DETECT;
	rc = touch_driver_spi_transfer(cmd, cmd,
		ARRAY_SIZE(cmd));
	if (rc) {
		thp_log_err("%s:write 0x90 command fail\n",
			__func__);
		return FTS_STYLUS_EXIT;
	}
	msleep(10); /* 10ms sequential */

	cmd[0] = DATA_HEAD;
	cmd[1] = DATA_MASK;
	cmd[2] = DATA_MASK;
	rc = touch_driver_spi_transfer(cmd, cmd,
		ARRAY_SIZE(cmd));
	if (rc) {
		thp_log_err("%s:read 0x90 data fail\n",
			__func__);
		return FTS_STYLUS_EXIT;
	}

	if ((cmd[1] == CHIP_DETECT_FAILE_ONE) ||
		(cmd[1] == CHIP_DETECT_FAILE_TWO) ||
		(cmd[1] == CHIP_DETECT_FAILE_THR)) {
		thp_log_err("%s: chip id read fail rc=%d, data=%x\n",
			__func__, rc, cmd[1]);
	} else {
		thp_log_info("%s:chip id read success 0x%X%X\n",
			__func__, cmd[1], cmd[2]);
		return FTS_STATUS_SUCCESS;
	}
	return FTS_STYLUS_FAIL;
}

static int touch_driver_chip_detect(struct thp_device *tdev)
{
	int i;
	/* 20 bytes for spi transfer */
	unsigned char cmd1[20] = {0};
	enum fts_return_status status;

	thp_log_info("%s: called\n", __func__);
	if (!tdev) {
		thp_log_info("%s: input dev null\n", __func__);
		return -EINVAL;
	}
	if (tdev->thp_core->self_control_power)
		return touch_driver_chip_detect_no_tddi(tdev, cmd1);
	for (i = 0; i < CHIP_DETECT_RETRY_NUMB; i++) {
		if (tdev->thp_core->support_vendor_ic_type == FT8720_SERIES)
			status = touch_driver_chip_detect_b2_series(tdev);
		else
			status = touch_driver_chip_detect_other_series(tdev);

		if (status == FTS_STATUS_SUCCESS)
			return 0;
		else if (status == FTS_STYLUS_FAIL)
			thp_log_info("%s:chip id read fail, i=%d\n", __func__, i);
		else if (status == FTS_STYLUS_CONTINUE)
			continue;
		else if (status == FTS_STYLUS_EXIT)
			goto exit;
		/* Delay 50ms to retry if reading ic id failed */
		msleep(50);
	}
exit:
	touch_driver_chip_detect_exit(tdev);

	return -EIO;
}

static int touch_driver_get_frame(struct thp_device *tdev,
	char *buf, unsigned int len)
{
	int ret;
	unsigned char *w_buf = NULL;
	unsigned char *r_buf = NULL;
	bool flag;
	u32 txlen = 0;

	if (!tdev) {
		thp_log_info("%s: input dev null\n", __func__);
		return -EINVAL;
	}
	if (!tdev->tx_buff || !tdev->rx_buff) {
		thp_log_err("%s: out of memory\n", __func__);
		return -EINVAL;
	}
	if (!buf) {
		thp_log_info("%s: input buf null\n", __func__);
		return -EINVAL;
	}

	if ((!len) || (len >= THP_MAX_FRAME_SIZE - 1)) {
		thp_log_info("%s: read len: %u illegal\n", __func__, len);
		return -EINVAL;
	}

	w_buf = tdev->tx_buff;
	r_buf = tdev->rx_buff;
	memset(tdev->tx_buff, 0xFF, THP_MAX_FRAME_SIZE);
	memset(tdev->rx_buff, 0, THP_MAX_FRAME_SIZE);

	flag = (tdev->thp_core->support_vendor_ic_type == FT8720_SERIES) ||
		(tdev->thp_core->support_vendor_ic_type == FT3680_SERIES) ||
		(tdev->thp_core->support_vendor_ic_type == UNIVERSAL_SERIES);
	if (flag) {
		w_buf[txlen++] = is_ic_ft8009 ? ADDR_RAWDATA : ADDR_FRAME;
		w_buf[txlen++] = FTS_READ_CMD;
		w_buf[txlen++] = (len >> 8) & 0xFF; /* get high 8 bit */
		w_buf[txlen++] = len & 0xFF; /* get low 8 bit */
		txlen = SPI_DUMMY_BYTE + len;
		txlen = txlen + ADDR_OFFSET;

		ret = touch_driver_spi_transfer(w_buf, r_buf, txlen);
		if (ret < 0) {
			thp_log_err("spi transfer fail\n");
			return ret;
		}

		ret = touch_driver_check_frame_valid(r_buf + SPI_DUMMY_BYTE,
			txlen - SPI_DUMMY_BYTE - ADDR_OFFSET);
		if (ret < 0) {
			ret = touch_driver_spi_transfer(w_buf, r_buf, txlen);
			if (ret < 0) {
				thp_log_err("spi transfer fail\n");
				return ret;
			}
		}
		memcpy(buf, r_buf + SPI_DUMMY_BYTE, len);
	} else {
		w_buf[0] = COMMOND_READ_DATA;
		ret = touch_driver_spi_transfer(w_buf, r_buf, len + 1);
		if (ret < 0) {
			thp_log_err("spi transfer fail\n");
			return ret;
		}
		ret = touch_driver_check_frame_valid(r_buf + 1, len);
		if (ret < 0) {
			ret = touch_driver_spi_transfer(w_buf, r_buf, len + 1);
			if (ret < 0) {
				thp_log_err("spi transfer fail\n");
				return ret;
			}
		}
		memcpy(buf, r_buf + 1, len);
	}
	return 0;
}

static void touch_driver_power_on_tddi(struct thp_device *tdev)
{
	if (!tdev->thp_core->not_support_cs_control)
		gpio_set_value(tdev->gpios->cs_gpio, GPIO_HIGH);
#ifdef CONFIG_HUAWEI_THP_QCOM
	if (tdev->thp_core->support_control_cs_off &&
		(!IS_ERR_OR_NULL(tdev->thp_core->qcom_pinctrl.cs_high))) {
		pinctrl_select_state(tdev->thp_core->pctrl, tdev->thp_core->qcom_pinctrl.cs_high);
		thp_log_info("%s: cs to high\n", __func__);
	}
#endif
	gpio_set_value(tdev->gpios->rst_gpio, GPIO_HIGH);
}

static void touch_driver_power_off_tddi(struct thp_device *tdev)
{
	gpio_set_value(tdev->gpios->rst_gpio, GPIO_LOW);
	if (!tdev->thp_core->not_support_cs_control)
		gpio_set_value(tdev->gpios->cs_gpio, GPIO_LOW);
#ifdef CONFIG_HUAWEI_THP_QCOM
	if (tdev->thp_core->support_control_cs_off &&
		(!IS_ERR_OR_NULL(tdev->thp_core->qcom_pinctrl.cs_low))) {
		pinctrl_select_state(tdev->thp_core->pctrl, tdev->thp_core->qcom_pinctrl.cs_low);
		thp_log_info("%s: cs to low\n", __func__);
	}
#endif
}

static int touch_driver_resume(struct thp_device *tdev)
{
	u32 gesture_status;
	struct thp_core_data *cd = thp_get_core_data(DEFAULT_TOUCH_PANEL);
	bool no_power_off_flag;
	bool error_flag = (!tdev) || (!tdev->thp_core);

	thp_log_info("%s: called\n", __func__);
	if (error_flag) {
		thp_log_info("%s: input dev null\n", __func__);
		return -EINVAL;
	}
	gesture_status = cd->easy_wakeup_info.sleep_mode;
	no_power_off_flag = (gesture_status == TS_GESTURE_MODE) ||
		g_thp_udfp_status || g_aod_touch_status || is_pt_test_mode(tdev);
#ifndef CONFIG_HUAWEI_THP_MTK
	if (cd->self_control_power) {
		if (no_power_off_flag) {
			thp_log_info("%s: gesture mode in\n", __func__);
			gpio_set_value(tdev->gpios->cs_gpio, GPIO_HIGH);
			gpio_set_value(tdev->gpios->rst_gpio, GPIO_LOW);
			mdelay(RST_HIGH_DELAY);
			gpio_set_value(tdev->gpios->rst_gpio, GPIO_HIGH);
		} else {
			touch_driver_power_on(tdev);
		}
	} else {
		touch_driver_power_on_tddi(tdev);
	}
#else
	if (tdev->thp_core->support_pinctrl == 0) {
		thp_log_info("%s: not support pinctrl\n", __func__);
		return -EINVAL;
	}
	pinctrl_select_state(tdev->thp_core->pctrl,
		tdev->thp_core->mtk_pinctrl.cs_high);
	if (tdev->thp_core->reset_status_in_suspend_mode)
		thp_log_info("%s: reset retains high\n", __func__);
	else
		pinctrl_select_state(tdev->thp_core->pctrl,
			tdev->thp_core->mtk_pinctrl.reset_high);
#endif
	thp_do_time_delay(tdev->timing_config.resume_reset_after_delay_ms);

	return 0;
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
	return 0;
}

static int touch_driver_send_fw_cmd(struct thp_device *tdev, enum fts_stylus_status_cmd value)
{
	int i;
	int ret;
	unsigned char w_buf[SPI_DUMMY_BYTE + FTS_REG_LEN];
	unsigned char r_buf[SPI_DUMMY_BYTE + FTS_REG_LEN];
	u32 index = 0;
	u32 txlen;

	thp_log_info("%s: called; value: %d\n", __func__, value);
	w_buf[index++] = FTS_REG_BT;
	w_buf[index++] = FTS_WRITE_CMD;
	w_buf[index++] = 0x00;
	w_buf[index++] = 0x01;
	txlen = SPI_DUMMY_BYTE + FTS_REG_LEN;
	w_buf[SPI_DUMMY_BYTE] = value;

	for (i = 0; i < SPI_RETRY_NUMBER; i++) {
		ret = touch_driver_spi_transfer(w_buf, r_buf, txlen);
		if ((ret == 0) && ((r_buf[FTS_OFF_STS] & FTS_READ_CMD) == 0))
			break;

		thp_log_info("data write addr:%x,status:%x,retry:%d,ret:%d",
			FTS_REG_BT, r_buf[FTS_OFF_STS], i, ret);
		ret = -EIO;
		udelay(CS_HIGH_DELAY);
	}
	if (ret < 0)
		thp_log_err("data write addr:%x fail,status:%x,ret:%d",
			FTS_REG_BT, r_buf[FTS_OFF_STS], ret);

	return ret;
}

static int touch_driver_bt_handler(struct thp_device *tdev,
	bool delay_enable)
{
	unsigned int stylus3_status;
	int i;
	int ret;

	stylus3_status = atomic_read(&tdev->thp_core->last_stylus3_status);

	thp_log_info("%s enter, delay_enable = %d, stylus3_status = %d\n",
		__func__, delay_enable, stylus3_status);
	for (i = 0; i < BT_CONNECT_CMD_RETRY; i++) {
		if ((stylus3_status & STYLUS3_CONNECT) == STYLUS3_CONNECT)
			ret = touch_driver_send_fw_cmd(tdev, FTS_STYLUS_CMD_CONNECT);
		else
			ret = touch_driver_send_fw_cmd(tdev, FTS_STYLUS_CMD_DISCONNECT);

		if (!ret)
			break;
	}
	if (i == BT_CONNECT_CMD_RETRY) {
		thp_log_err("%s, send stylus status fail, retry = %d\n", __func__, i);
		ret = -EINVAL;
	}

	return ret;
}

static void touch_driver_enter_gesture_mode(struct thp_device *tdev)
{
	int i;
	int ret;
	unsigned char w_buf[SPI_DUMMY_BYTE + FTS_REG_LEN];
	unsigned char r_buf[SPI_DUMMY_BYTE + FTS_REG_LEN];
	u32 index = 0;
	u32 txlen;

	thp_log_info("%s: called\n", __func__);
	w_buf[index++] = FTS_REG_GESTURE_EN;
	w_buf[index++] = FTS_WRITE_CMD;
	w_buf[index++] = (FTS_REG_LEN >> GET_HIGH_BIT) & 0xFF;
	w_buf[index++] = FTS_REG_LEN & 0xFF;
	txlen = SPI_DUMMY_BYTE + FTS_REG_LEN;
	w_buf[SPI_DUMMY_BYTE] = GESTURE_DATA_ENABLE;

	for (i = 0; i < SPI_RETRY_NUMBER; i++) {
		ret = touch_driver_spi_transfer(w_buf, r_buf, txlen);
		if ((ret == 0) && ((r_buf[FTS_OFF_STS] & FTS_READ_CMD) == 0))
			break;
		thp_log_info("data write addr:%x,status:%x,retry:%d,ret:%d",
			FTS_REG_GESTURE_EN, r_buf[FTS_OFF_STS], i, ret);
		ret = -EIO;
		udelay(CS_HIGH_DELAY);
	}
	if (ret < 0)
		thp_log_err("data write addr:%x fail,status:%x,ret:%d",
			FTS_REG_GESTURE_EN, r_buf[FTS_OFF_STS], ret);
	mutex_lock(&tdev->thp_core->thp_wrong_touch_lock);
	tdev->thp_core->easy_wakeup_info.off_motion_on = true;
	mutex_unlock(&tdev->thp_core->thp_wrong_touch_lock);
}

static void parse_gesture_info(u8 *gesture_buffer)
{
	int cnt = 0;
	char str[FTS_GESTURE_STR_LEN_MAX];
	int str_len_max = FTS_GESTURE_STR_LEN_MAX;
	int i;

	if ((gesture_buffer[GESTURE_INFO_BIT] & GESTURE_INFO_VALUE) ==
		GESTURE_INFO_VALUE) {
		for (i = 0; i < FTS_GESTURE_DATA_LEN; i++)
			cnt += snprintf(str + cnt, str_len_max - cnt, "%02X ",
				gesture_buffer[i]);
		thp_log_info("%s: %s\n", __func__, str);
	}
}

static int touch_driver_gesture_data_identify(unsigned char *gesture_data)
{
	int offset_bit;
	int gesture_data_val = 0;
	struct thp_core_data *cd = thp_get_core_data(DEFAULT_TOUCH_PANEL);

	/*
	 * distinguish historical projects. fts_gesture_no_offset is
	 * 0: Inndicates the historical item logic. Use gesture_data[2] to determine the gesture function value.
	 * 1: Use gesture_data[0] to determine the gesture function value.
	 */
	if (cd->fts_gesture_no_offset) {
		offset_bit = 0;
	} else {
		if (gesture_data[0] != GESTURE_DATA_ENABLE) {
			thp_log_err("gesture is disabled");
			return -EINVAL;
		}
		offset_bit = FTS_GESTURE_ID_BIT;
	}
	gesture_data_val = gesture_data[offset_bit];
	if (gesture_data_val == FTS_GESTURE_ID_VALUE)
		return FTS_DOUBLE_CLICK_WAKEUP_EVENT;
	else if (gesture_data_val == FTS_GESTURE_STYLUS_ID_VALUE)
		return FTS_STYLUS_WAKEUP_TO_MEMO_EVENT;

	return  -EINVAL;
}

static int touch_driver_read_gesture_data(void)
{
	int ret;
	int i;
	unsigned char w_buf[SPI_DUMMY_BYTE + FTS_GESTURE_DATA_LEN];
	unsigned char r_buf[SPI_DUMMY_BYTE + FTS_GESTURE_DATA_LEN];
	u32 txlen = 0;
	unsigned char *gesture_data = NULL;

	thp_log_info("%s: called\n", __func__);
	w_buf[txlen++] = FTS_REG_GESTURE_DATA;
	w_buf[txlen++] = FTS_READ_CMD;
	w_buf[txlen++] = (FTS_GESTURE_DATA_LEN >> GET_HIGH_BIT) & 0xFF;
	w_buf[txlen++] = FTS_GESTURE_DATA_LEN & 0xFF;
	txlen = SPI_DUMMY_BYTE + FTS_GESTURE_DATA_LEN + ADDR_OFFSET;
	for (i = 0; i < SPI_RETRY_NUMBER; i++) {
		ret = touch_driver_spi_transfer(w_buf, r_buf, txlen);
		if ((ret == 0) && ((r_buf[FTS_OFF_STS] & FTS_READ_CMD) == 0)) {
			ret = touch_driver_check_frame_valid(r_buf +
				SPI_DUMMY_BYTE,
				txlen - SPI_DUMMY_BYTE - ADDR_OFFSET);
			if (ret < 0) {
				thp_log_info("addr:%x crc abnormal,retry:%d",
					FTS_REG_GESTURE_DATA, i);
				udelay(CS_HIGH_DELAY);
				continue;
			}
			break;
		}
		thp_log_info("addr:%x status:%x,ret:%d",
			FTS_REG_GESTURE_DATA, r_buf[FTS_OFF_STS], ret);
		ret = -EIO;
		udelay(CS_HIGH_DELAY);
	}
	if (ret < 0) {
		thp_log_err("data read addr:%x %s,status:%x,ret:%d",
			FTS_REG_GESTURE_DATA,
			(i >= SPI_RETRY_NUMBER) ? "crc abnormal" : "fail",
			r_buf[FTS_OFF_STS], ret);
		return ret;
	}
	gesture_data = r_buf + SPI_DUMMY_BYTE;
	parse_gesture_info(gesture_data);

	return touch_driver_gesture_data_identify(gesture_data);
}

static int touch_driver_gesture_event_dispatch(unsigned int *gesture_wakeup_value,
	enum fts_gesture_event fts_gesture)
{
	if (!gesture_wakeup_value) {
		thp_log_err("%s: gesture_wakeup_value is null\n", __func__);
		return -EINVAL;
	}

	switch (fts_gesture) {
	case FTS_DOUBLE_CLICK_WAKEUP_EVENT:
		*gesture_wakeup_value = TS_DOUBLE_CLICK;
		break;
	case FTS_STYLUS_WAKEUP_TO_MEMO_EVENT:
		*gesture_wakeup_value = TS_STYLUS_WAKEUP_TO_MEMO;
		break;
	default:
		thp_log_err("%s:invalid  value\n", __func__);
		return -EINVAL;
	}
	return 0;
}

int touch_driver_report_gesture(struct thp_device *tdev,
	unsigned int *gesture_wakeup_value)
{
	int gesture_data;

	if ((!tdev) || (!tdev->thp_core) || (!tdev->thp_core->sdev)) {
		thp_log_info("%s: input dev null\n", __func__);
		return -EINVAL;
	}
	if ((!gesture_wakeup_value) ||
		(!tdev->thp_core->support_gesture_mode)) {
		thp_log_err("%s, gesture not support\n", __func__);
		return -EINVAL;
	}
	gesture_data = touch_driver_read_gesture_data();
	if (gesture_data < 0) {
		thp_log_err("%s: gesture data read or write fail\n", __func__);
		return -EINVAL;
	}
	mutex_lock(&tdev->thp_core->thp_wrong_touch_lock);
	if (tdev->thp_core->easy_wakeup_info.off_motion_on == true) {
		tdev->thp_core->easy_wakeup_info.off_motion_on = false;
		if (touch_driver_gesture_event_dispatch(gesture_wakeup_value, gesture_data) < 0)
			thp_log_err("%s, set gesture_wakeup_value fail\n", __func__);
	}
	mutex_unlock(&tdev->thp_core->thp_wrong_touch_lock);
	return 0;
}

static int touch_driver_set_cmd(unsigned char *tx_buf,
	unsigned char *rx_buf, unsigned int len)
{
	int rc;
	int index;

	for (index = 0; index < SPI_RETRY_NUMBER; index++) {
		rc = touch_driver_spi_transfer(tx_buf, rx_buf, len);
		if (rc < 0) {
			thp_log_info("%s: lowpower fail", __func__);
			msleep(1);
			continue;
		}
		break;
	}

	return rc;
}

static void pt_mode_set(void)
{
	int rc;
	unsigned char sleep_cmd[] = {0x52, 0x00, 0x00, 0x00};
	unsigned char r_buf[READ_LEN] = {0};

	rc = touch_driver_set_cmd(sleep_cmd, r_buf, sizeof(sleep_cmd));
	if (rc < 0)
		thp_log_err("%s: failed to set pt mode\n", __func__);
}

static bool touch_driver_gesture_flag(struct thp_device *tdev)
{
	u32 gesture_status;
	bool gesture_flag;

	thp_log_info("%s: called\n", __func__);
	gesture_status = tdev->thp_core->easy_wakeup_info.sleep_mode;
	gesture_flag = (gesture_status == TS_GESTURE_MODE) ||
		g_thp_udfp_status || g_aod_touch_status;

	return gesture_flag;
}

static int gesture_process_oled(struct thp_device *tdev)
{
	u8 reg_value = 0;
	int rc;
	struct thp_core_data *cd = thp_get_core_data(DEFAULT_TOUCH_PANEL);
	u32 gesture_status = cd->easy_wakeup_info.sleep_mode;

	/*
	 * when bit0 is set to 1, it indicates single click
	 * when bit1 is set to 1, it indicates double click
	 * when bit4 is set to 1, it indicates finger print
	 */
	if (gesture_status == TS_GESTURE_MODE) {
		thp_log_info("enable double click");
		reg_value |= 1 << 1;
	}
	if (g_thp_udfp_status) {
		thp_log_info("enable finger print");
		reg_value |= 1 << 4;
	}
	if (g_aod_touch_status) {
		thp_log_info("enable single click");
		reg_value |= 1 << 0;
	}
	thp_log_info("%s: enabled gesture 0xd1 %u\n", __func__, reg_value);
	rc = touch_driver_enabled_gesture(FTS_REG_GESTURE, reg_value);
	if (rc)
		thp_log_err("%s: enabled gesture failed\n", __func__);
	touch_driver_enter_gesture_mode(tdev);

	return rc;
}

static int gesture_process_tddi(struct thp_device *tdev)
{
	bool gesture_status = tdev->thp_core->easy_wakeup_info.sleep_mode;

	if (gesture_status == TS_GESTURE_MODE &&
		tdev->thp_core->lcd_gesture_mode_support) {
		thp_log_info("%s: TS_GESTURE_MODE\n", __func__);
		touch_driver_enter_gesture_mode(tdev);
		return NO_ERR;
	}

	return -EIO;
}

static int touch_driver_suspend(struct thp_device *tdev)
{
	int pt_test_station;
	bool error_flag = (!tdev) || (!tdev->thp_core);
	struct thp_core_data *cd = thp_get_core_data(DEFAULT_TOUCH_PANEL);

	thp_log_info("%s: called\n", __func__);
	if (error_flag) {
		thp_log_err("%s: input dev null\n", __func__);
		return -EINVAL;
	}

	g_aod_touch_status = cd->aod_touch_status;
	g_thp_udfp_status = thp_get_status(THP_STATUS_UDFP, DEFAULT_TOUCH_PANEL);
	if (touch_driver_gesture_flag(tdev)) {
		if (tdev->thp_core->self_control_power)
			return gesture_process_oled(tdev);
		else
			return gesture_process_tddi(tdev);
	}
	pt_test_station = is_pt_test_mode(tdev);
#ifndef CONFIG_HUAWEI_THP_MTK
	if (pt_test_station) {
		pt_mode_set();
		thp_log_info("%s: PT sleep mode\n", __func__);
	} else {
		if (tdev->thp_core->self_control_power)
			touch_driver_power_off(tdev);
		else
			touch_driver_power_off_tddi(tdev);
	}
#else
	if (tdev->thp_core->support_pinctrl == 0) {
		thp_log_info("%s: not support pinctrl\n", __func__);
		return -EINVAL;
	}
	if (pt_test_station) {
		thp_log_info("%s: PT sleep mode\n", __func__);
	} else {
		if (tdev->thp_core->reset_status_in_suspend_mode)
			thp_log_info("%s: reset retains high\n", __func__);
		else
			pinctrl_select_state(tdev->thp_core->pctrl,
				tdev->thp_core->mtk_pinctrl.reset_low);
	}
	pinctrl_select_state(tdev->thp_core->pctrl,
		tdev->thp_core->mtk_pinctrl.cs_low);
#endif
	thp_do_time_delay(tdev->timing_config.suspend_reset_after_delay_ms);

	return 0;
}

static void touch_driver_exit(struct thp_device *tdev)
{
	thp_log_info("%s: called\n", __func__);
	touch_driver_dev_free(tdev);
}

static int touch_driver_second_poweroff(void)
{
	struct thp_core_data *cd = thp_get_core_data(DEFAULT_TOUCH_PANEL);
	struct thp_device *tdev = cd->thp_dev;
	int ret;
	int i;
	bool flag;
	unsigned char *w_buf = NULL;
	unsigned char *r_buf = NULL;
	u32 txlen = 0;

	thp_log_info("%s: called\n", __func__);
	flag = !tdev || !tdev->tx_buff || !tdev->rx_buff;
	if (flag) {
		thp_log_err("%s: tdev/tx_buff/rx_buff null\n", __func__);
		return -EINVAL;
	}

	w_buf = tdev->tx_buff;
	r_buf = tdev->rx_buff;

	for (i = 0; i < SLEEP_RETRY_NUM; i++) {
		udelay(150); /* delay 150us */
		w_buf[txlen++] = 0x62; /* power off twice command */
		w_buf[txlen++] = FTS_WRITE_CMD; /* 0x00  write */
		w_buf[txlen++] = 0x00;
		w_buf[txlen++] = 0x00;
		ret = touch_driver_spi_transfer(w_buf, r_buf, txlen);
		if ((ret < 0) || ((r_buf[SLEEP_BUF_BIT] & FTS_READ_CMD) != 0)) {
			thp_log_err("data write addr:%x fail,status:%x,ret:%d",
				0x62, r_buf[SLEEP_BUF_BIT], ret);
			continue;
		} else {
			break;
		}
	}
	return ret;
}

/*spi_v2 read/write protocol*/
static int touch_driver_spi_write(u8 addr, u8 *data, u32 datalen)
{
	struct thp_core_data *cd = thp_get_core_data(DEFAULT_TOUCH_PANEL);
	struct thp_device *tdev = cd->thp_dev;
	int ret = 0;
	int i = 0;
	unsigned char *w_buf = NULL;
	unsigned char *r_buf = NULL;
	u32 txlen = 0;
	bool flag;

	thp_log_info("%s: called\n", __func__);
	flag = (!tdev || !tdev->tx_buff || !tdev->rx_buff || !data);
	if (flag) {
		thp_log_info("%s: tdev/tx_buff/rx_buff null\n", __func__);
		return -ENOMEM;
	}

	if (datalen >= DATALEN_SIZE) {
		thp_log_info("%s: data_len: %u illegal\n", __func__, datalen);
		return -ENOMEM;
	}

	w_buf = tdev->tx_buff;
	r_buf = tdev->rx_buff;
	memset(tdev->tx_buff, DATA_MASK, THP_MAX_FRAME_SIZE);
	memset(tdev->rx_buff, 0, THP_MAX_FRAME_SIZE);

	w_buf[txlen++] = addr;
	w_buf[txlen++] = FTS_WRITE_CMD;
	w_buf[txlen++] = (datalen >> MOVE_8BIT) & DATA_MASK;
	w_buf[txlen++] = datalen & DATA_MASK;
	if (datalen > 0) {
		memcpy(&w_buf[SPI_DUMMY_BYTE], data, datalen);
		txlen = SPI_DUMMY_BYTE + datalen;
	}

	for (i = 0; i < SPI_RETRY_NUMBER; i++) {
		ret = touch_driver_spi_transfer(w_buf, r_buf, txlen);
		flag = (!ret) && (!(r_buf[FTS_OFF_STS] & FTS_READ_CMD));
		if (flag)
			break;
		thp_log_info("data write(addr:%x),status:%x,retry:%d,ret:%d",
			addr, r_buf[FTS_OFF_STS], i, ret);
		ret = -EIO;
		udelay(CS_HIGH_DELAY);
	}

	udelay(CS_HIGH_DELAY);
	return ret;
}

static int get_project_form_addr_enter(struct thp_device *tdev, char *buf, unsigned int len)
{
	int rc;
	u8 data[THP_PROJECT_ID_LEN] = { 0 };
	u8 cmd[THP_PROJECT_ID_LEN] = { 0 };
	u8 pid_data[THP_PROJECT_ID_LEN + PID_OFFSET] = { 0 };

	thp_log_info("%s: called\n", __func__);
	rc = touch_driver_hardware_reset_to_rom(tdev);
	if (rc) {
		thp_log_err("%s:write 0x55 command fail\n", __func__);
		return rc;
	}

	rc = touch_driver_spi_read(COMOND_ADDR_DETECT, data, LENGTH_DETECT);
	if (rc) {
		thp_log_err("%s:write 0x90 command fail\n", __func__);
		return rc;
	}

	if ((data[OFFSET_0] == CHIP_DETECT_FAILE_ONE) ||
		(data[OFFSET_0] == CHIP_DETECT_FAILE_TWO) ||
		(data[OFFSET_0] == CHIP_DETECT_FAILE_THR)) {
		thp_log_err(
			"%s:chip id read fail, ret=%d, data=%x\n",
			__func__, rc, data[OFFSET_0]);
		return -EINVAL;
	}
	thp_log_info("%s:chip id read success, chip id:0x%X,0x%X\n",
		__func__, data[OFFSET_0], data[OFFSET_1]);

	cmd[OFFSET_0] = (workarounds_get_pid_from_addr >> MOVE_16BIT) & DATA_MASK;
	cmd[OFFSET_1] = (workarounds_get_pid_from_addr >> MOVE_8BIT) & DATA_MASK;
	cmd[OFFSET_2] = workarounds_get_pid_from_addr & DATA_MASK;
	rc = touch_driver_spi_write(COMMAND_ADDR_PID_ONE, cmd, FTS_OFF_STS);
	if (rc) {
		thp_log_err("write 0x%x fail!\n", COMMAND_ADDR_PID_ONE);
		return rc;
	}
	mdelay(DELAY20_WRITE);

	rc = touch_driver_spi_read(COMMAND_READ_PID, pid_data, THP_PROJECT_ID_LEN);
	if (rc) {
		thp_log_err("read project id fail\n");
		return rc;
	}
	strncpy(buf, pid_data, len);
	thp_log_info("touch_driver_get_project_id %s\n", buf);
	return rc;
}

static int touch_driver_get_project_code_from_addr(struct thp_device *tdev, char *buf, unsigned int len)
{
	int i = 0;
	int rc;

	thp_log_info("%s: called\n", __func__);
	for (i = 0; i < CHIP_DETECT_RETRY_NUMB; i++) {
		rc = get_project_form_addr_enter(tdev, buf, len);
		if (!rc)
			break;
	}

	gpio_direction_output(tdev->gpios->rst_gpio, THP_RESET_LOW);
	thp_do_time_delay(tdev->timing_config.boot_reset_low_delay_ms);

	gpio_set_value(tdev->gpios->rst_gpio, THP_RESET_HIGH);
	thp_do_time_delay(tdev->timing_config.boot_reset_hi_delay_ms);
	return rc;
}

static int touch_driver_read_project(struct thp_device *tdev, char *buf, unsigned int len)
{
	int rc;
	u8 cmd[THP_PROJECT_ID_LEN + PID_OFFSET] = { 0 };

	rc = touch_driver_hardware_reset_to_rom(tdev);
	if (rc) {
		thp_log_err("%s:write 0x55 command fail\n",
			__func__);
		return -EINVAL;
	}

	rc = touch_driver_spi_read(COMMAND_READ_PID_3683G, cmd, THP_PROJECT_ID_LEN);
	if (rc) {
		thp_log_err("read project id fail\n");
	} else {
		cmd[THP_PROJECT_ID_LEN + 1] = '\0';
		strncpy(buf, cmd, len);
		thp_log_info("%s: %s\n", __func__, buf);
	}
	rc = gpio_direction_output(tdev->gpios->rst_gpio, THP_RESET_LOW);
	if (rc)
		thp_log_err("%s:rst gpio fail\n", __func__);
	thp_do_time_delay(tdev->timing_config.boot_reset_low_delay_ms);

	gpio_set_value(tdev->gpios->rst_gpio, THP_RESET_HIGH);
	thp_do_time_delay(tdev->timing_config.boot_reset_hi_delay_ms);

	return rc;
}

static int touch_driver_get_project_id(struct thp_device *tdev, char *buf, unsigned int len)
{
	int rc = 0;

	if (!tdev || !tdev->thp_core) {
		thp_log_info("%s: tdev/tdev->thp_core null\n", __func__);
		return -EINVAL;
	}

	if (workarounds_get_pid_from_addr != INVALID_ADDRESS)
		return touch_driver_get_project_code_from_addr(tdev, buf, len);

	if (tdev->thp_core->support_vendor_ic_type == FT3680_SERIES) {
		strncpy(buf, tdev->thp_core->project_id_dummy, len);
		thp_log_info("%s: %s\n", __func__, buf);
		return NO_ERR;
	} else if (tdev->thp_core->support_vendor_ic_type == UNIVERSAL_SERIES) {
		rc = touch_driver_read_project(tdev, buf, len);
		if (rc)
			thp_log_err("%s:read project fail\n", __func__);
	}

	return rc;
}

struct thp_device_ops fts_dev_ops = {
	.init = touch_driver_init,
	.detect = touch_driver_chip_detect,
	.get_frame = touch_driver_get_frame,
	.resume = touch_driver_resume,
	.suspend = touch_driver_suspend,
	.exit = touch_driver_exit,
	.spi_transfer = touch_driver_spi_transfer,
	.chip_wrong_touch = touch_driver_wrong_touch,
	.chip_gesture_report = touch_driver_report_gesture,
	.second_poweroff = touch_driver_second_poweroff,
	.get_project_id = touch_driver_get_project_id,
	.bt_handler = touch_driver_bt_handler,
};

/* malloc tdev and buffers */
static struct thp_device *touch_driver_dev_malloc(void)
{
	struct thp_device *tdev = NULL;

	thp_log_info("%s: called\n", __func__);
	tdev = kzalloc(sizeof(*tdev), GFP_KERNEL);
	if (!tdev)
		goto err_touch_driver_dev_free;
	tdev->tx_buff = kzalloc(THP_MAX_FRAME_SIZE, GFP_KERNEL);
	if (!tdev->tx_buff)
		goto err_touch_driver_dev_free;
	tdev->rx_buff = kzalloc(THP_MAX_FRAME_SIZE, GFP_KERNEL);
	if (!tdev->rx_buff)
		goto err_touch_driver_dev_free;

	return tdev;

err_touch_driver_dev_free:
	touch_driver_dev_free(tdev);
	thp_log_err("%s: Malloc thp_device failed\n", __func__);

	return NULL;
}

static int __init touch_driver_module_init(void)
{
	int rc = 0;
	struct thp_device *tdev = NULL;
	struct thp_core_data *cd = thp_get_core_data(DEFAULT_TOUCH_PANEL);

	tdev = touch_driver_dev_malloc();
	if (!tdev) {
		thp_log_err("%s: thp device malloc fail\n", __func__);
		return -ENOMEM;
	}

	tdev->ic_name = FOCALTECH_IC_NAME;
	tdev->dev_node_name = THP_FOCALTECH_DEV_NODE_NAME;
	tdev->ops = &fts_dev_ops;
	if (cd && cd->fast_booting_solution) {
		thp_send_detect_cmd(tdev, DEFAULT_TOUCH_PANEL, NO_SYNC_TIMEOUT);
		/*
		 * The thp_register_dev will be called later to complete
		 * the real detect action.If it fails, the detect function will
		 * release the resources requested here.
		 */
		return 0;
	}
	rc = thp_register_dev(tdev, DEFAULT_TOUCH_PANEL);
	if (rc) {
		thp_log_err("%s: register fail\n", __func__);
		touch_driver_dev_free(tdev);
	}

	return rc;
}

static void __exit touch_driver_module_exit(void)
{
	thp_log_info("%s: called\n", __func__);
};

module_init(touch_driver_module_init);
module_exit(touch_driver_module_exit);
MODULE_AUTHOR("focaltech");
MODULE_DESCRIPTION("focaltech driver");
MODULE_LICENSE("GPL");
