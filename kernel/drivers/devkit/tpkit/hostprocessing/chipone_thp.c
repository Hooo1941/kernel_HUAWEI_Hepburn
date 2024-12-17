/*
 * Thp driver code for chipone
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <securec.h>
#include "huawei_pen_clk.h"
#include "huawei_thp.h"

#define MOVE_0BIT 0
#define MOVE_8BIT 8
#define MOVE_16BIT 16
#define MOVE_24BIT 24
#define MASK_8BIT 0xFF
#define SINGLE_CLICK 0x0001
#define DOUBLE_CLICK 0x0002
#define BOTH_CLICK 0x0003
#define SPEED_HZ_DRW (8 * 1000 * 1000)
#define SPEED_HZ_TRS (6 * 1000 * 1000)
#define SPEED_HZ_DEFAULT 0
#define TI_UN_CON 1000
#define DETECT_RETRY_TIME 3
#define DETECT_RETRY_DELAY_MS 6
#define CTS_DRV_VERSION "v1.0"
#define CTS_IC_NAME "chipone"
#define CTS_DEV_NODE_NAME "chipone_thp"
#define CTS_PROJECT_ID_LEN 10

/* commands */
#define CTS_DRW_RD_CMD 0x61
#define CTS_DRW_WR_CMD 0x60
#define CTS_NORM_RD_CMD 0xF1
#define CTS_NORM_WR_CMD 0xF0

/* hwid */
#define CTS_IC_HWID 0x00009268
#define CTS_IC_HWID_MASK 0x0000FFFF

/* registers */
#define CTS_REG_HWID 0x70000
#define CTS_REG_HWID_9288 0x870000
#define CTS_REG_CMD_SEL 0x74000
#define CTS_REG_FLASH_ADDR 0x74004
#define CTS_REG_SRAM_ADDR 0x74008
#define CTS_REG_DATA_LENGTH 0x7400C
#define CTS_REG_START_DEXC 0x74010
#define CTS_REG_BUSY 0x74024
#define CTS_PROJECT_ID_ADDR 0x0000000E
#define CTS_RST_FLASH_VOL_REG1 0x71034
#define CTS_RST_FLASH_VOL_REG2 0x74084

/* buffer size */
#define CTS_SPI_BUF_SIZ (THP_MAX_FRAME_SIZE - 128)
#define CTS_FRAME_SIZ 1400
#define CTS_DRW_HEAD_SIZ sizeof(struct cts_drw_head)
#define CTS_DRW_TAIL_SIZ sizeof(struct cts_drw_tail)
#define CHIPONE_MAX_LOG_SIZE 128
#define CTS_TCS_HEAD_SIZ sizeof(struct cts_tcs_head)
#define CTS_TCS_TAIL_SIZ sizeof(struct cts_tcs_tail)
#define CTS_FRAME_MAX_SIZ (THP_MAX_FRAME_SIZE - 4 * 1024)
#define CTS_FRAME_MIN_SIZ (CTS_TCS_HEAD_SIZ + CTS_TCS_TAIL_SIZ)

/* retries count */
#define CTS_ENTER_DRW_RETRIES 3
#define CTS_WAIT_BUS_RETRIES 3
#define THP_RESET_WAIT_TIME 10
#define CTS_FLASH_BUSY_DELAY 1

#define CTS_SWITCH_INT_DELAY_TIME_MS 2

/* log buf */
# define STR_START (g_logbuf + offset)
# define STR_SIZ ((sizeof(g_logbuf) - offset))

/* gesture flag */
#define ENABLE_SINGLE_CLICK 0x01
#define ENABLE_DOUBLE_CLICK 0x02
#define ENABLE_STYLUS_CLICK 0x04

/* pen clock */
#define CLK_ENABLE 1
#define CLK_DISABLE 0

enum cts_ic_type {
	ICNT9268 = 0,
	ICNT9288 = 1,
	ICNL9922C = 2,   // 0x99C220,
};

/* structs for communication */
#pragma pack(push, 1)
struct cts_drw_head {
	uint8_t rwcmd;
	uint8_t addr[3];
	uint8_t len[3];
	uint8_t crc[2];
	uint8_t wait[4];
};

struct cts_drw_tail {
	uint8_t crc[2];
	uint8_t wait[1];
	uint8_t ack[2];
};

struct cts_tcs_head {
	uint8_t  dummy[8];
	uint16_t curr_size;
	uint16_t next_size;
};

struct cts_tcs_tail {
	uint8_t ecode;
	uint8_t cmd[2];
	uint8_t crc[2];
};
#pragma pack(pop)

static uint8_t g_logbuf[CHIPONE_MAX_LOG_SIZE];

/* Get frame data command */
static uint8_t get_frame_cmd[] = {
	CTS_NORM_RD_CMD, 0x05, 0x51, 0x70, 0x00, 0xAA, 0x4A
};
static uint8_t single_click_cmd[] = {
	CTS_NORM_WR_CMD, 0x01, 0x37, 0x02, 0x00, 0x2F, 0xB1,
	0x01, 0x00, 0x03, 0x86
};
static uint8_t double_click_cmd[] = {
	CTS_NORM_WR_CMD, 0x01, 0x37, 0x02, 0x00, 0x2F, 0xB1,
	0x02, 0x00, 0x03, 0x8C
};
static uint8_t both_click_cmd[] = {
	CTS_NORM_WR_CMD, 0x01, 0x37, 0x02, 0x00, 0x2F, 0xB1,
	0x03, 0x00, 0x00, 0x0A
};
static uint8_t sleep_cmd[] = {
	CTS_NORM_WR_CMD, 0x0A, 0x31, 0x01, 0x00, 0x54, 0xA7,
	0x02, 0x0F, 0x80,
};

static void touch_hw_reset(struct thp_device *tdev);
#ifdef CONFIG_HUAWEI_SHB_THP
static int touch_driver_switch_int_sh(struct thp_device *tdev);
static int touch_driver_switch_int_ap(struct thp_device *tdev);
static void touch_driver_poweroff_status(struct thp_device *tdev);
#endif

static inline void put_unaligned_be16(void *p, uint16_t v)
{
	uint8_t *pu8 = (uint8_t *)p;

	pu8[0] = (v >> MOVE_8BIT) & MASK_8BIT;
	pu8[1] = (v >> MOVE_0BIT) & MASK_8BIT;
}

static inline void put_unaligned_be24(void *p, uint32_t v)
{
	uint8_t *pu8 = (uint8_t *)p;

	pu8[0] = (v >> MOVE_16BIT) & MASK_8BIT;
	pu8[1] = (v >> MOVE_8BIT) & MASK_8BIT;
	pu8[2] = (v >> MOVE_0BIT) & MASK_8BIT;
}

static inline uint16_t get_unaligned_be16(const void *p)
{
	const uint8_t *pu8 = (const uint8_t *)p;

	return ((pu8[0] << MOVE_8BIT) | pu8[1]);
}

static inline uint16_t get_unaligned_le16(const void *p)
{
	const uint8_t *pu8 = (const uint8_t *)p;

	return (pu8[0] | (pu8[1] << MOVE_8BIT));
}

static inline uint32_t get_unaligned_le32(const void *p)
{
	const uint8_t *pu8 = (const uint8_t *)p;

	return (pu8[0] | (pu8[1] << MOVE_8BIT) |
		(pu8[2] << MOVE_16BIT) | (pu8[3] << MOVE_24BIT));
}

static inline long tvdiff2ms(struct timeval *start, struct timeval *end)
{
	return (end->tv_sec - start->tv_sec) * TI_UN_CON +
			(end->tv_usec - start->tv_usec) / TI_UN_CON;
}

/* calculate 16bit crc for data */
const static uint16_t crc16_table[] = {
	0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
	0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
	0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
	0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
	0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
	0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
	0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
	0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
	0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
	0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
	0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
	0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
	0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
	0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
	0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
	0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
	0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
	0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
	0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
	0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
	0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
	0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
	0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
	0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
	0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
	0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
	0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
	0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
	0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
	0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
	0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
	0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202,
};

static unsigned int g_thp_udfp_status;
static unsigned int chipone_stylus_status;
static unsigned int chipone_ring_setting_switch;
static unsigned int g_aod_touch_status;

static void touch_driver_log_buf(char *prefix, uint8_t *data, size_t datalen)
{
	int offset = 0;
	int i;

	offset += snprintf_s(STR_START, STR_SIZ, STR_SIZ - 1, "%s", prefix);
	if (data && datalen) {
		for (i = 0; i < datalen && offset < sizeof(g_logbuf); i++)
			offset += snprintf_s(STR_START, STR_SIZ, STR_SIZ - 1, " %02x", data[i]);
	}

	thp_log_info("%s", g_logbuf);
}

static uint16_t touch_driver_crc16(const uint8_t *data, size_t len)
{
	uint16_t crc16 = 0;
	uint8_t index;

	while (len) {
		index = (((crc16 >> MOVE_8BIT) ^ *data) & MASK_8BIT);
		crc16 = (crc16 << MOVE_8BIT) ^ crc16_table[index];
		data++;
		len--;
	}

	return crc16;
}

static void pen_clk_control(int status, u32 panel_id)
{
	unsigned int stylus3_status = thp_get_status(THP_STATUS_STYLUS3, panel_id);
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	u32 i;
	bool flag = true;
	bool no_need_operate_pen_clk;

	for (i = 0; i <= cd->dual_thp; i++)
		flag = flag && thp_get_core_data(i)->suspended;

	/*
	 * need_extra_system_clk: need clk from soc.
	 * stylus3_status: the link status of pen
	 * support_fw_xtal_switch: if true, open pen clk when screen on and close pen clk when screen off
	 */
	no_need_operate_pen_clk = (!cd->need_extra_system_clk) ||
		(!stylus3_status && cd->support_fw_xtal_switch);
	if (no_need_operate_pen_clk)
		return;
	if (status) {
		thp_pen_clk_enable();
	} else if (flag) {
		thp_pen_clk_disable();
		if (stylus3_status)
			start_pen_clk_timer();
	}
}

/* power api */
static int touch_driver_power_init(u32 panel_id)
{
	int ret;
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	if (cd->power_cfg.power_supply_mode)
		return 0;

	ret = thp_power_supply_get(THP_VCC, panel_id);
	if (ret)
		thp_log_err("%s: fail to get vcc power\n", __func__);
	ret = thp_power_supply_get(THP_IOVDD, panel_id);
	if (ret)
		thp_log_err("%s: fail to get power\n", __func__);
	return 0;
}

static int touch_driver_power_release(u32 panel_id)
{
	int ret;
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	if (cd->power_cfg.power_supply_mode)
		return 0;

	ret = thp_power_supply_put(THP_VCC, panel_id);
	if (ret)
		thp_log_err("%s: fail to release vcc power\n", __func__);
	ret = thp_power_supply_put(THP_IOVDD, panel_id);
	if (ret)
		thp_log_err("%s: fail to release power\n", __func__);
	return ret;
}

static int touch_driver_power_on(struct thp_device *tdev)
{
	int ret;
	struct thp_core_data *cd = thp_get_core_data(tdev->thp_core->panel_id);

	if (cd->power_cfg.power_supply_mode)
		return 0;
	thp_log_info("%s:called\n", __func__);

	gpio_direction_input(tdev->gpios->irq_gpio);
	gpio_set_value(tdev->gpios->rst_gpio, GPIO_LOW);
	ret = thp_power_supply_ctrl(THP_VCC, THP_POWER_ON, 1, cd->panel_id); /* delay 1ms */
	if (ret)
		thp_log_err("%s:power ctrl fail\n", __func__);
	ret = thp_power_supply_ctrl(THP_IOVDD, THP_POWER_ON,
		cd->iovdd_power_on_delay_ms, cd->panel_id);
	if (ret)
		thp_log_err("%s:power ctrl vddio fail\n", __func__);
	gpio_set_value(tdev->gpios->cs_gpio, GPIO_HIGH);

	touch_hw_reset(tdev);

	return ret;
}

static int touch_driver_power_off(struct thp_device *tdev)
{
	int ret;
	struct thp_core_data *cd = thp_get_core_data(tdev->thp_core->panel_id);

	if (cd->power_cfg.power_supply_mode)
		return 0;
	thp_log_debug("%s: in\n", __func__);

	gpio_set_value(tdev->gpios->rst_gpio, GPIO_LOW);
	mdelay(tdev->timing_config.boot_reset_low_delay_ms);
	gpio_set_value(tdev->gpios->cs_gpio, GPIO_LOW);

	ret = thp_power_supply_ctrl(THP_IOVDD, THP_POWER_OFF, 5, cd->panel_id);
	if (ret)
		thp_log_err("%s:power ctrl fail\n", __func__);
	ret = thp_power_supply_ctrl(THP_VCC, THP_POWER_OFF, 5, cd->panel_id);
	if (ret)
		thp_log_err("%s:power ctrl vcc fail\n", __func__);

	return ret;
}

/* transfer spi data */
static int touch_driver_spi_transfer(struct thp_device *tdev,
	void *tbuf, void *rbuf, size_t len, uint32_t speed)
{
	struct spi_transfer xfer = {
		.tx_buf = tbuf,
		.rx_buf = rbuf,
		.len = len,
		.speed_hz = speed,
	};
	struct spi_message msg;
	int rc = 0;

	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);

	rc = thp_spi_sync(tdev->sdev, &msg);
	if (rc)
		thp_log_err("%s: spi_sync failed: rc=%d\n", __func__, rc);

	return rc;
}

/* hardware reset */
static void touch_hw_reset(struct thp_device *tdev)
{
	thp_log_info("%s: called\n", __func__);
	gpio_set_value(tdev->gpios->rst_gpio, GPIO_HIGH);
	thp_time_delay(tdev->timing_config.boot_reset_hi_delay_ms);
	gpio_set_value(tdev->gpios->rst_gpio, GPIO_LOW);
	thp_time_delay(tdev->timing_config.boot_reset_low_delay_ms);
	gpio_set_value(tdev->gpios->rst_gpio, GPIO_HIGH);
	thp_time_delay(tdev->timing_config.boot_reset_hi_delay_ms);
}

/* enter drw mode */
static int touch_enter_drw_mode(struct thp_device *tdev)
{
	uint8_t enter_drw_cmd[] = { 0xCC, 0x33, 0x55, 0x5A };
	int cmd_size = sizeof(enter_drw_cmd);
	int retries = CTS_ENTER_DRW_RETRIES;
	int rc;

	thp_log_info("%s: called\n", __func__);
	thp_bus_lock();
	memcpy(tdev->tx_buff, enter_drw_cmd, cmd_size);
	while (retries--) {
		rc = touch_driver_spi_transfer(tdev,
			tdev->tx_buff, tdev->rx_buff, cmd_size, SPEED_HZ_DRW);
		if (!rc)
			break;
		thp_log_err("%s: Enter drw mode failed: rc=%d, retry %d\n",
			__func__, rc, retries);
		thp_time_delay(tdev->thp_core->send_cmd_retry_delay_ms);
	}
	thp_bus_unlock();
	if (rc)
		thp_log_err("%s: Enter drw mode failed: rc=%d\n", __func__, rc);

	return rc;
}

/* read reg value in drw mode */
static int touch_drw_read_reg(struct thp_device *tdev,
	uint32_t reg_addr, size_t rlen)
{
	struct cts_drw_head *psend = (struct cts_drw_head *)tdev->tx_buff;
	uint8_t *pdata = tdev->rx_buff + sizeof(struct cts_drw_head);
	uint16_t total_len = rlen + sizeof(struct cts_drw_head) +
		sizeof(struct cts_drw_tail);
	uint16_t crc16_calc;
	uint16_t crc16_recv;
	int rc;

	thp_log_info("%s: called\n", __func__);
	if (!rlen || (total_len > CTS_SPI_BUF_SIZ)) {
		thp_log_err("%s: Invalid read size %ld\n", __func__, rlen);
		return -EINVAL;
	}

	thp_bus_lock();
	memset(tdev->tx_buff, 0, total_len);
	psend->rwcmd = CTS_DRW_RD_CMD;
	put_unaligned_be24(&psend->addr[0], reg_addr);
	put_unaligned_be24(&psend->len[0], rlen);
	crc16_calc = touch_driver_crc16(&psend->rwcmd,
		offsetof(struct cts_drw_head, crc));
	put_unaligned_be16(&psend->crc[0], (uint16_t)~crc16_calc);
	rc = touch_driver_spi_transfer(tdev, tdev->tx_buff,
		tdev->rx_buff, total_len, SPEED_HZ_DRW);
	if (rc) {
		thp_log_err("Drw read reg %#07x failed: rc=%d", reg_addr, rc);
	} else {
		crc16_calc = touch_driver_crc16(pdata, rlen);
		crc16_recv = ~get_unaligned_be16(pdata + rlen);
		if (crc16_calc != crc16_recv) {
			thp_log_err("%s: crc error: calc %#06x != %#06x recv\n",
				__func__, crc16_calc, crc16_recv);
			rc = -EINVAL;
		}
	}
	thp_bus_unlock();

	return rc;
}

/* write reg value in drw mode */
static int touch_driver_drw_write_reg(struct thp_device *tdev,
	uint32_t reg_addr, uint8_t *wdata, size_t wlen)
{
	struct cts_drw_head *psend = (struct cts_drw_head *)tdev->tx_buff;
	uint8_t *pdata = tdev->tx_buff + sizeof(struct cts_drw_head);
	uint16_t total_len = wlen + CTS_DRW_HEAD_SIZ + CTS_DRW_TAIL_SIZ;
	uint16_t crc16_calc;
	int rc;

	thp_log_info("%s", __func__);
	if (!wlen || (total_len > CTS_SPI_BUF_SIZ)) {
		thp_log_err("Invalid write size %ld\n", wlen);
		return -EINVAL;
	}

	thp_bus_lock();
	rc = memset_s(tdev->tx_buff, total_len, 0, total_len);
	if (rc) {
		thp_log_err("%s: memset fail\n", __func__);
		thp_bus_unlock();
		return rc;
	}
	psend->rwcmd = CTS_DRW_WR_CMD;
	put_unaligned_be24(&psend->addr[0], reg_addr);
	put_unaligned_be24(&psend->len[0], wlen);
	crc16_calc = touch_driver_crc16(&psend->rwcmd, offsetof(struct cts_drw_head, crc));
	put_unaligned_be16(&psend->crc[0], (uint16_t)~crc16_calc);
	rc = memcpy_s(pdata, wlen, wdata, wlen);
	if (rc) {
		thp_log_err("memcpy failed, rc = %d\n", rc);
		thp_bus_unlock();
		return rc;
	}
	crc16_calc = touch_driver_crc16(wdata, wlen);
	put_unaligned_be16(pdata + wlen, (uint16_t)~crc16_calc);
	rc = touch_driver_spi_transfer(tdev, tdev->tx_buff, tdev->rx_buff,
		total_len, SPEED_HZ_DRW);
	if (rc)
		thp_log_err("Drw write reg %#07x failed: rc=%d\n", reg_addr, rc);
	thp_bus_unlock();

	return rc;
}

/* get hardware id */
static int touch_driver_get_hwid(struct thp_device *tdev, uint32_t *hwid)
{
	uint32_t ret_hwid;
	int rc;

	thp_log_info("%s: called\n", __func__);
	rc = touch_enter_drw_mode(tdev);
	if (rc) {
		thp_log_err("%s: Enter drw mode failed: rc=%d\n", __func__, rc);
		return rc;
	}

	rc = touch_drw_read_reg(tdev, CTS_REG_HWID, sizeof(uint32_t));
	if (rc) {
		thp_log_err("%s: Read @%06x failed: rc=%d\n", __func__, CTS_REG_HWID, rc);
		return rc;
	}
	ret_hwid = get_unaligned_le32(tdev->rx_buff + sizeof(struct cts_drw_head));
	thp_log_info("%s: hwid = %#06x@%#6x\n", __func__, ret_hwid, CTS_REG_HWID);
	*hwid = ret_hwid & CTS_IC_HWID_MASK;

	return 0;
}

static int touch_driver_get_hwid_icnt9288(struct thp_device *tdev, uint32_t *hwid)
{
	uint32_t ret_hwid;
	int rc;

	thp_log_info("%s", __func__);
	rc = touch_enter_drw_mode(tdev);
	if (rc) {
		thp_log_err("Enter drw mode failed: rc=%d\n", rc);
		return rc;
	}
	rc = touch_drw_read_reg(tdev, CTS_REG_HWID_9288, sizeof(uint32_t));
	if (rc) {
		thp_log_err("Read hwid @%06x failed: rc=%d\n", CTS_REG_HWID_9288, rc);
		return rc;
	}
	ret_hwid = get_unaligned_le32(tdev->rx_buff + sizeof(struct cts_drw_head));
	if ((ret_hwid & CTS_IC_HWID_MASK) == CTS_IC_HWID) {
		thp_log_info("hwid = %#06x@%#6x\n", ret_hwid, CTS_REG_HWID_9288);
		*hwid = ret_hwid;
		return 0;
	}

	return 0;
}

/* free tdev and buffers */
static int touch_driver_dev_free(struct thp_device *tdev)
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

	return 0;
}

/* malloc tdev and buffers */
static struct thp_device *touch_driver_dev_malloc(void)
{
	struct thp_device *tdev = NULL;

	thp_log_info("%s", __func__);
	tdev = kzalloc(sizeof(struct thp_device), GFP_KERNEL);
	if (!tdev)
		goto err_touch_driver_dev_free;
	tdev->tx_buff = kzalloc(CTS_SPI_BUF_SIZ, GFP_KERNEL);
	if (!tdev->tx_buff)
		goto err_touch_driver_dev_free;
	tdev->rx_buff = kzalloc(CTS_SPI_BUF_SIZ, GFP_KERNEL);
	if (!tdev->rx_buff)
		goto err_touch_driver_dev_free;

	return tdev;

err_touch_driver_dev_free:
	touch_driver_dev_free(tdev);
	thp_log_err("%s: Malloc thp_device failed\n", __func__);

	return NULL;
}

static int thp_parse_feature_ic_config(struct device_node *thp_node,
	struct thp_core_data *cd)
{
	int rc;

	thp_log_info("%s: called\n", __func__);
	cd->support_vendor_ic_type = 0;
	rc = of_property_read_u32(thp_node, "support_vendor_ic_type",
		&cd->support_vendor_ic_type);
	if (!rc)
		thp_log_info("%s:support_vendor_ic_type parsed:%u\n",
			__func__, cd->support_vendor_ic_type);

	return rc;
}

/* thp callback, init */
static int touch_driver_init(struct thp_device *tdev)
{
	struct thp_core_data *cd = tdev->thp_core;
	struct device_node *cts_node;
	int rc;

	thp_log_info("%s", __func__);
	cts_node = of_get_child_by_name(cd->thp_node, CTS_DEV_NODE_NAME);
	if (!cts_node) {
		thp_log_err("%s node NOT found in dts", CTS_DEV_NODE_NAME);
		return -ENODEV;
	}
	if (tdev->sdev->master->flags & SPI_MASTER_HALF_DUPLEX) {
		thp_log_err("Full duplex not supported by master");
		return -EIO;
	}
	rc = thp_parse_spi_config(cts_node, cd);
	if (rc)
		thp_log_err("Spi config parse failed: rc=%d", rc);

	rc = thp_parse_timing_config(cts_node, &tdev->timing_config);
	if (rc)
		thp_log_err("Timing config parse failed: rc=%d", rc);

	rc = thp_parse_feature_config(cts_node, cd);
	if (rc)
		thp_log_err("%s: feature_config fail\n", __func__);

	rc = of_property_read_u32(cts_node, "send_cmd_retry_delay_ms",
		&cd->send_cmd_retry_delay_ms);
	if (rc)
		cd->send_cmd_retry_delay_ms = 0;
	thp_log_info("%s: send_cmd_retry_delay_ms %u\n", __func__,
		cd->send_cmd_retry_delay_ms);

	rc = of_property_read_u32(cts_node, "support_fw_xtal_switch",
		&cd->support_fw_xtal_switch);
	if (rc)
		cd->support_fw_xtal_switch = 0;
	thp_log_info("%s: support_fw_xtal_switch %u\n",
		__func__, cd->support_fw_xtal_switch);

	rc = of_property_read_u32(cts_node, "need_extra_system_clk",
		&cd->need_extra_system_clk);
	if (rc) {
		cd->need_extra_system_clk = 0;
	thp_log_info("%s: need_extra_system_clk %u\n",
		__func__, cd->need_extra_system_clk);
	}
	return 0;
}

/* thp callback, detect */
static int touch_driver_detect(struct thp_device *tdev)
{
	uint32_t hwid;
	int rc;
	struct thp_core_data *cd = tdev->thp_core;

	thp_log_info("%s", __func__);
	rc = touch_driver_power_init(tdev->thp_core->panel_id);
	if (rc)
		thp_log_err("%s: power init failed\n", __func__);
	rc = touch_driver_power_on(tdev);
	if (rc)
		thp_log_err("%s: power on failed\n", __func__);

	thp_time_delay(tdev->timing_config.boot_reset_after_delay_ms);

	if (cd->support_vendor_ic_type == ICNT9288)
		rc = touch_driver_get_hwid_icnt9288(tdev, &hwid);
	else
		rc = touch_driver_get_hwid(tdev, &hwid);
	if (rc) {
		thp_log_err("Get hwid failed: rc=%d\n", rc);
		goto err;
	}
	if ((hwid & CTS_IC_HWID_MASK) != CTS_IC_HWID) {
		thp_log_err("Mismatch hwid, got %#06x while %#06x expected\n",
				hwid, CTS_IC_HWID);
		goto err;
	}
	pen_clk_control(CLK_ENABLE, tdev->thp_core->panel_id);

	return 0;
err:
	rc = touch_driver_power_off(tdev);
	if (rc)
		thp_log_err("%s: power off failed\n", __func__);
	rc = touch_driver_power_release(tdev->thp_core->panel_id);
	if (rc < 0)
		thp_log_err("%s: power ctrl Failed\n", __func__);

	if (tdev->thp_core->fast_booting_solution)
		touch_driver_dev_free(tdev);
	return -ENODEV;
}

static int touch_driver_check_frame(uint8_t *frame, unsigned int frame_size)
{
	struct cts_tcs_head *head;
	struct cts_tcs_tail *tail;
	uint16_t curr_size, next_size;
	uint16_t crc16_calc, crc16_recv;

	head = (struct cts_tcs_head *)frame;
	curr_size = head->curr_size;
	next_size = head->next_size;
	if ((curr_size < CTS_FRAME_MIN_SIZ) ||
			(curr_size > CTS_FRAME_MAX_SIZ)) {
		thp_log_err("Invalid current frame size: %d\n", curr_size);
		return -1;
	}
	if ((next_size < CTS_FRAME_MIN_SIZ) ||
			(next_size > CTS_FRAME_MAX_SIZ)) {
		thp_log_err("Invalid next frame size: %d\n", next_size);
		return -1;
	}
	if ((curr_size + CTS_TCS_TAIL_SIZ) > frame_size) {
		thp_log_err("Mismatched curr_size %d, read size %d\n",
			curr_size, frame_size);
		return -1;
	}
	tail = (struct cts_tcs_tail *)(frame + curr_size);
	crc16_calc = touch_driver_crc16(frame,
		curr_size + offsetof(struct cts_tcs_tail, crc));
	crc16_recv = get_unaligned_le16((uint16_t *)tail->crc);
	if (crc16_calc != crc16_recv) {
		thp_log_err("Crc error: calc %#06x != %#06x recv",
			crc16_calc, crc16_recv);
		return -1;
	}
	if ((tail->cmd[0] != 0x05) || (tail->cmd[1] != 0x51)) {
		thp_log_err("Mismatched reply! cmd[2] = %02x,%02x\n",
			tail->cmd[0], tail->cmd[1]);
		return -1;
	}

	return 0;
}

static int touch_driver_get_frame_data(struct thp_device *tdev, unsigned int *frame_len)
{
	static unsigned int read_len = CTS_FRAME_MAX_SIZ;
	struct cts_tcs_head *head;
	int rc;

	rc = memcpy_s(tdev->tx_buff, sizeof(get_frame_cmd), get_frame_cmd, sizeof(get_frame_cmd));
	if (rc) {
		thp_log_err("%s:cpy get frame cmd fail\n", __func__);
		return rc;
	}
	rc = touch_driver_spi_transfer(tdev, tdev->tx_buff, tdev->rx_buff,
			read_len + CTS_TCS_TAIL_SIZ, SPEED_HZ_DEFAULT);
	if (rc) {
		thp_log_err("Get data frame failed: rc=%d\n", rc);
		return rc;
	}
	rc = touch_driver_check_frame(tdev->rx_buff, read_len + CTS_TCS_TAIL_SIZ);
	if (rc) {
		thp_log_err("Check frame failed\n");
		read_len = CTS_FRAME_MAX_SIZ;
		return rc;
	}
	head = (struct cts_tcs_head *)(tdev->rx_buff);
	read_len = head->next_size;
	*frame_len = head->curr_size + CTS_TCS_TAIL_SIZ;

	return 0;
}

/* thp callback, get_frame */
static int touch_driver_get_frame(struct thp_device *tdev,
	char *buf, unsigned int len)
{
	unsigned int frame_len;
	unsigned int copy_len;
	int rc = 0;

	thp_bus_lock();
	rc = touch_driver_get_frame_data(tdev, &frame_len);
	if (rc) {
		thp_log_err("Get frame failed: rc=%d\n", rc);
		thp_bus_unlock();
		return rc;
	}
	copy_len = frame_len > len ? len : frame_len;
	memcpy_s(buf, copy_len, tdev->rx_buff, copy_len);
	if (frame_len < len)
		memset_s(buf + frame_len, len - frame_len, 0, len - frame_len);
	thp_bus_unlock();

	return 0;
}

static int touch_driver_write_rst(struct thp_device *tdev)
{
	int rc;
	uint8_t rst_reg[] = { 0x00 };

	rc = touch_driver_drw_write_reg(tdev, CTS_RST_FLASH_VOL_REG1,
		rst_reg, sizeof(rst_reg));
	if (rc) {
		thp_log_err("Reset flash voltage reg1(0x%x) failed rc=%d\n",
			CTS_RST_FLASH_VOL_REG1, rc);
		return rc;
	}
	rc = touch_driver_drw_write_reg(tdev, CTS_RST_FLASH_VOL_REG2,
		rst_reg, sizeof(rst_reg));
	if (rc) {
		thp_log_err("Reset flash voltage reg2(0x%x) failed rc=%d\n",
			CTS_RST_FLASH_VOL_REG2, rc);
		return rc;
	}
	thp_time_delay(THP_RESET_WAIT_TIME);
	return rc;
}

static int touch_driver_write_project_id_cmd(struct thp_device *tdev)
{
	int rc;
	uint8_t cmd_sel[] = { 0x01 };
	uint8_t flash_addr[] = { 0x00, 0x14, 0x0F, 0x00 };
	uint8_t sram_addr[] = { 0x00, 0x00, 0x00, 0x00 };
	uint8_t data_length[] = { 0x00, 0x02, 0x00, 0x00 };
	uint8_t start_dexc[] = { 0x01 };

	rc = touch_driver_drw_write_reg(tdev,
		CTS_REG_CMD_SEL, cmd_sel, sizeof(cmd_sel));
	if (rc) {
		thp_log_err("Set mass read failed: rc=%d", rc);
		return rc;
	}
	rc = touch_driver_drw_write_reg(tdev,
		CTS_REG_FLASH_ADDR, flash_addr, sizeof(flash_addr));
	if (rc) {
		thp_log_err("Set flash addr failed: rc=%d\n", rc);
		return rc;
	}
	rc = touch_driver_drw_write_reg(tdev,
		CTS_REG_SRAM_ADDR, sram_addr, sizeof(sram_addr));
	if (rc) {
		thp_log_err("Set sram addr failed: rc=%d\n", rc);
		return rc;
	}
	rc = touch_driver_drw_write_reg(tdev,
		CTS_REG_DATA_LENGTH, data_length, sizeof(data_length));
	if (rc) {
		thp_log_err("Set data length failed: rc=%d\n", rc);
		return rc;
	}
	rc = touch_driver_drw_write_reg(tdev,
		CTS_REG_START_DEXC, start_dexc, sizeof(start_dexc));
	if (rc) {
		thp_log_err("Start copy flash data failed: rc=%d\n", rc);
		return rc;
	}
	return rc;
}

/* thp callback, get_project id */
static int touch_driver_get_project_id(struct thp_device *tdev,
	char *buf, unsigned int len)
{
	uint32_t proj_id_addr = CTS_PROJECT_ID_ADDR;
	int retries = CTS_WAIT_BUS_RETRIES;
	uint8_t *proj_id_ptr;
	uint32_t flash_busy;
	int copy_len;
	int rc;

	thp_log_info("[%s]", __func__);
	if (!buf || !len) {
		thp_log_err("Invalid buf or len for project id");
		return -EINVAL;
	}

	touch_driver_write_rst(tdev);
	touch_driver_write_project_id_cmd(tdev);

	while (retries--) {
		rc = touch_drw_read_reg(tdev, CTS_REG_BUSY, sizeof(uint32_t));
		if (rc) {
			thp_log_err("Check flash busy failed: rc=%d, retries=%d", rc, retries);
			return rc;
		}
		flash_busy = get_unaligned_le32(tdev->rx_buff + CTS_DRW_HEAD_SIZ);
		if (flash_busy) {
			mdelay(CTS_FLASH_BUSY_DELAY);
		} else {
			break;
		}
	}
	if (flash_busy) {
		thp_log_err("Check flash busy failed: rc=%d\n", rc);
		return rc;
	}
	rc = touch_drw_read_reg(tdev, proj_id_addr, CTS_PROJECT_ID_LEN);
	if (rc) {
		thp_log_err("Read sram data failed: rc=%d\n", rc);
		return rc;
	}
	proj_id_ptr = tdev->rx_buff + sizeof(struct cts_drw_head);
	touch_driver_log_buf("PROJID", proj_id_ptr, CTS_PROJECT_ID_LEN);

	memset_s(buf, len, 0, len);
	if (len >= CTS_PROJECT_ID_LEN)
		copy_len = CTS_PROJECT_ID_LEN;
	else
		copy_len = len;

	rc = memcpy_s(buf, copy_len, proj_id_ptr, copy_len);
	if (rc) {
		thp_log_err("copy project id fail, ret = %d\n", rc);
		return rc;
	}

	return 0;
}

/* thp callback, resume */
static int touch_driver_resume(struct thp_device *tdev)
{
	int ret;

	if ((!tdev) || (!tdev->thp_core) || (!tdev->thp_core->sdev)) {
		thp_log_err("%s: Invalid tdev\n", __func__);
		return -EINVAL;
	}
	thp_log_info("%s, ic_type %d\n", __func__,
		tdev->thp_core->support_vendor_ic_type);
	pen_clk_control(CLK_ENABLE, tdev->thp_core->panel_id);
	if (g_thp_udfp_status ||
		(tdev->thp_core->easy_wakeup_info.sleep_mode ==
		TS_GESTURE_MODE) || chipone_stylus_status ||
		g_aod_touch_status || chipone_ring_setting_switch ||
		tdev->thp_core->power_cfg.power_supply_mode) {
		if (tdev->thp_core->support_shb_thp) {
#ifdef CONFIG_HUAWEI_SHB_THP
			ret = touch_driver_switch_int_ap(tdev);
			thp_log_info("%s: switch to ap\n", __func__);
#endif
		} else {
			thp_log_info("%s:gesture support without shb\n", __func__);
		}
		touch_hw_reset(tdev);
	} else {
		ret = touch_driver_power_on(tdev);
	}
	thp_time_delay(tdev->timing_config.resume_reset_after_delay_ms);
	thp_log_info("%s: called end\n", __func__);
	return ret;
}

static int touch_driver_chose_suspend_cmd(struct thp_device *tdev, uint8_t* copy_cmd, int size)
{
	uint8_t *cmd;
	size_t cmd_len = 0;
	struct thp_core_data *cd;
	uint16_t param = 0x00000;
	int rc;

	if ((!tdev) || (!tdev->thp_core) || (!tdev->thp_core->sdev)) {
		thp_log_err("Invalid tdev");
		return -EINVAL;
	}

	cd = tdev->thp_core;
	if (cd->easy_wakeup_info.aod_mode)
		param |= ENABLE_SINGLE_CLICK;
	if (cd->easy_wakeup_info.sleep_mode == TS_GESTURE_MODE)
		param |= ENABLE_DOUBLE_CLICK;
	thp_bus_lock();
	if (param) {
		switch (param) {
		case 0x0001:
			cmd = single_click_cmd;
			cmd_len = sizeof(single_click_cmd);
			break;
		case 0x0002:
			cmd = double_click_cmd;
			cmd_len = sizeof(double_click_cmd);
			break;
		case 0x0003:
			cmd = both_click_cmd;
			cmd_len = sizeof(both_click_cmd);
			break;
		default:
			thp_log_err("BUG!!");
			thp_bus_unlock();
			return -EIO;
		}
	} else {
		cmd = sleep_cmd;
		cmd_len = sizeof(sleep_cmd);
	}
	rc = memcpy_s(copy_cmd, size, cmd, cmd_len);
	if (rc) {
		thp_log_info("copy suspend cmd failed, ret = %d\n", rc);
		return rc;
	}

	return cmd_len;
}

static int touch_driver_suspend_cmd_transfer(struct thp_device *tdev)
{
	struct cts_tcs_tail *tail;
	uint16_t crc16_calc, crc16_recv;
	int rc;
	int cmd_len;
	uint8_t cmd[12];
	cmd_len = touch_driver_chose_suspend_cmd(tdev, cmd, sizeof(cmd));
	if (cmd_len < 0) {
		thp_log_err("thp choose suspend cmd fail\n");
		return cmd_len;
	}

	memcpy(tdev->tx_buff, cmd, cmd_len);
	rc = touch_driver_spi_transfer(tdev, tdev->tx_buff, tdev->rx_buff,
		cmd_len, SPEED_HZ_DEFAULT);
	if (rc) {
		thp_log_err("Send cmd failed: rc=%d\n", rc);
		thp_bus_unlock();
		return -EIO;
	}
	memset_s(tdev->tx_buff, CTS_TCS_TAIL_SIZ, 0, CTS_TCS_TAIL_SIZ);
	rc = touch_driver_spi_transfer(tdev, tdev->tx_buff, tdev->rx_buff,
		CTS_TCS_TAIL_SIZ, SPEED_HZ_DEFAULT);
	if (rc) {
		thp_log_err("Recv cmd failed: rc=%d\n", rc);
		thp_bus_unlock();
		return -EIO;
	}
	tail = (struct cts_tcs_tail *)tdev->rx_buff;
	if (tail->ecode || (tail->cmd[0] != cmd[1]) || (tail->cmd[1] != cmd[2])) {
		thp_log_err("Tcs check failed: ecode=%d, cmd[2]={%02x,%02x}\n",
			tail->ecode, tail->cmd[0], tail->cmd[1]);
		thp_bus_unlock();
		return -EIO;
	}
	crc16_calc = touch_driver_crc16((uint8_t *)tail, offsetof(struct cts_tcs_tail, crc));
	crc16_recv = tail->crc[0] | (tail->crc[1] << 8);
	if (crc16_calc != crc16_recv) {
		thp_log_err("crc error: calc %#06x != %#06x recv\n", crc16_calc, crc16_recv);
		thp_bus_unlock();
		return -EIO;
	}
	thp_bus_unlock();
	return 0;
}

/* thp callback, suspend */
static int touch_driver_suspend(struct thp_device *tdev)
{
	int rc;
	struct thp_core_data *cd = tdev->thp_core;
	g_thp_udfp_status = thp_get_status(THP_STATUS_UDFP, cd->panel_id);
	chipone_stylus_status = thp_get_status(THP_STATUS_STYLUS, cd->panel_id) |
		cd->suspend_stylus3_status;
	g_aod_touch_status = cd->aod_touch_status;
	chipone_ring_setting_switch = cd->ring_setting_switch;
	thp_log_info("%s:gesture_status:%u,finger_status:%u\n",
		__func__, cd->easy_wakeup_info.sleep_mode, g_thp_udfp_status);
	thp_log_info("%s:ring_support:%u,ring_switch:%u,phone_status:%u,ring_setting_switch:%u\n",
		__func__, cd->support_ring_feature, cd->ring_switch,
		cd->phone_status, chipone_ring_setting_switch);
	thp_log_info("%s:stylus_status:%u,aod_touch_status:%u\n", __func__,
		chipone_stylus_status, cd->aod_touch_status);

	if (g_thp_udfp_status || chipone_ring_setting_switch ||
		(cd->easy_wakeup_info.sleep_mode == TS_GESTURE_MODE) ||
		chipone_stylus_status || cd->aod_touch_status) {
			if (tdev->thp_core->support_shb_thp) {
#ifdef CONFIG_HUAWEI_SHB_THP
				touch_driver_poweroff_status(tdev);
				rc = touch_driver_switch_int_sh(tdev);
#endif
			} else {
				thp_log_info("%s:gesture support without shb\n", __func__);
			}
	} else {
		touch_driver_suspend_cmd_transfer(tdev);
		if (tdev->thp_core->support_shb_thp)
			/* 0:all function was closed */
			tdev->thp_core->poweroff_function_status = 0;
		touch_driver_power_off(tdev);
	}
	thp_do_time_delay(tdev->timing_config.before_pen_clk_disable_delay_ms);
	pen_clk_control(CLK_DISABLE, tdev->thp_core->panel_id);
	return 0;
}

/* thp callback, exit */
static void touch_driver_exit(struct thp_device *tdev)
{
	thp_log_info("%s: called\n", __func__);
	touch_driver_dev_free(tdev);
}

#ifdef CONFIG_HUAWEI_SHB_THP
#define INPUTHUB_POWER_SWITCH_START_BIT 9
#define INPUTHUB_POWER_SWITCH_START_OFFSET 1
static void touch_driver_poweroff_status(struct thp_device *tdev)
{
	struct thp_core_data *cd = thp_get_core_data(tdev->thp_core->panel_id);
	unsigned int finger_status = !!(thp_get_status(THP_STATUS_UDFP, cd->panel_id));
	unsigned int stylus_status = (thp_get_status(THP_STATUS_STYLUS, cd->panel_id)) |
		(thp_get_status(THP_STATUS_STYLUS3, cd->panel_id));

	cd->poweroff_function_status =
		(cd->double_click_switch << THP_DOUBLE_CLICK_ON) |
		(finger_status << THP_TPUD_ON) |
		(cd->ring_setting_switch << THP_RING_SUPPORT) |
		(cd->ring_switch << THP_RING_ON) |
		(stylus_status << THP_PEN_MODE_ON) |
		(cd->phone_status << THP_PHONE_STATUS) |
		(cd->single_click_switch << THP_SINGLE_CLICK_ON) |
		(cd->volume_side_status << THP_VOLUME_SIDE_STATUS_LEFT);
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

static int touch_driver_switch_int_sh(struct thp_device *tdev)
{
	uint8_t cmd_enable[] = {
		CTS_NORM_WR_CMD, 0x22, 0x31, 0x01, 0x00, 0x5B, 0x07,
		0x01, 0x05, 0x80
	};
	uint8_t *cmd;
	size_t cmd_len = sizeof(cmd);
	struct cts_tcs_tail *tail;
	uint16_t crc16_calc, crc16_recv;
	int rc;

	cmd = cmd_enable;
	cmd_len = sizeof(cmd_enable);

	if (tdev->thp_core->use_ap_sh_common_int && tdev->thp_core->support_pinctrl)
		thp_pinctrl_select_lowpower(tdev->thp_core->thp_dev);

	thp_bus_lock();
	memcpy(tdev->tx_buff, cmd, cmd_len);
	rc = touch_driver_spi_transfer(tdev, tdev->tx_buff, tdev->rx_buff, cmd_len, SPEED_HZ_DEFAULT);
	if (rc) {
		thp_log_err("%s:send cmd failed: rc=%d\n", __func__, rc);
		thp_bus_unlock();
		return -EIO;
	}
	thp_time_delay(CTS_SWITCH_INT_DELAY_TIME_MS);
	memset(tdev->tx_buff, 0, CTS_TCS_TAIL_SIZ);
	rc = touch_driver_spi_transfer(tdev, tdev->tx_buff, tdev->rx_buff,
		CTS_TCS_TAIL_SIZ, SPEED_HZ_DEFAULT);
	if (rc) {
		thp_log_err("%s:recv cmd failed: rc=%d\n", __func__, rc);
		thp_bus_unlock();
		return -EIO;
	}
	tail = (struct cts_tcs_tail *)tdev->rx_buff;
	if (tail->ecode || (tail->cmd[0] != cmd[1]) || (tail->cmd[1] != cmd[2])) {
		thp_log_err("Tcs check failed: ecode=%d, cmd[2]={%02x,%02x}\n",
			tail->ecode, tail->cmd[0], tail->cmd[1]);
		thp_bus_unlock();
		return -EIO;
	}
	crc16_calc = touch_driver_crc16((uint8_t *)tail, offsetof(struct cts_tcs_tail, crc));
	crc16_recv = tail->crc[0] | (tail->crc[1] << 8);
	if (crc16_calc != crc16_recv) {
		thp_log_err("crc error: calc %#06x != %#06x recv\n", crc16_calc, crc16_recv);
		thp_bus_unlock();
		return -EIO;
	}
	thp_bus_unlock();
	thp_log_info("%s: send: switch int sh\n", __func__);
	return 0;
}

static int touch_driver_switch_int_ap(struct thp_device *tdev)
{
	uint8_t cmd_disable[] = {
		CTS_NORM_WR_CMD, 0x22, 0x31, 0x01, 0x00, 0x5B, 0x07,
		0x00, 0x00, 0x00
	};
	uint8_t *cmd;
	size_t cmd_len = sizeof(cmd);
	struct cts_tcs_tail *tail;
	uint16_t crc16_calc, crc16_recv;
	int rc;

	cmd = cmd_disable;
	cmd_len = sizeof(cmd_disable);

	if (tdev->thp_core->use_ap_sh_common_int && tdev->thp_core->support_pinctrl)
		thp_pinctrl_select_normal(tdev->thp_core->thp_dev);

	thp_bus_lock();
	memcpy(tdev->tx_buff, cmd, cmd_len);
	rc = touch_driver_spi_transfer(tdev, tdev->tx_buff, tdev->rx_buff, cmd_len, SPEED_HZ_DEFAULT);
	if (rc) {
		thp_log_err("Send cmd failed: rc=%d\n", rc);
		thp_bus_unlock();
		return -EIO;
	}
	thp_time_delay(CTS_SWITCH_INT_DELAY_TIME_MS);
	memset(tdev->tx_buff, 0, CTS_TCS_TAIL_SIZ);
	rc = touch_driver_spi_transfer(tdev, tdev->tx_buff, tdev->rx_buff, CTS_TCS_TAIL_SIZ, SPEED_HZ_DEFAULT);
	if (rc) {
		thp_log_err("Recv cmd failed: rc=%d\n", rc);
		thp_bus_unlock();
		return -EIO;
	}
	tail = (struct cts_tcs_tail *)tdev->rx_buff;
	if (tail->ecode || (tail->cmd[0] != cmd[1]) || (tail->cmd[1] != cmd[2])) {
		thp_log_err("Tcs check failed: ecode=%d, cmd[2]={%02x,%02x}\n",
			tail->ecode, tail->cmd[0], tail->cmd[1]);
		thp_bus_unlock();
		return -EIO;
	}
	crc16_calc = touch_driver_crc16((uint8_t *)tail, offsetof(struct cts_tcs_tail, crc));
	crc16_recv = tail->crc[0] | (tail->crc[1] << 8);
	if (crc16_calc != crc16_recv) {
		thp_log_err("crc error: calc %#06x != %#06x recv",
			crc16_calc, crc16_recv);
		thp_bus_unlock();
		return -EIO;
	}
	thp_bus_unlock();
	thp_log_info("%s: send: switch int ap\n", __func__);
	return 0;
}
#endif

/* thp ops */
static struct thp_device_ops cts_dev_ops = {
	.init = touch_driver_init,
	.detect = touch_driver_detect,
	.get_frame = touch_driver_get_frame,
	.get_project_id = touch_driver_get_project_id,
	.resume = touch_driver_resume,
	.suspend = touch_driver_suspend,
	.exit = touch_driver_exit,
#ifdef CONFIG_HUAWEI_SHB_THP
	.switch_int_sh = touch_driver_switch_int_sh,
	.switch_int_ap = touch_driver_switch_int_ap,
#endif
};

/* module init */
static int thp_module_init(u32 panel_id)
{
	struct thp_device *tdev;
	int rc;
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	tdev = touch_driver_dev_malloc();
	if (!tdev) {
		rc = -ENOMEM;
		thp_log_err("%s: Malloc for thp device failed: rc=%d\n",
			__func__, rc);
		return rc;
	}
	tdev->ic_name = CTS_IC_NAME;
	tdev->dev_node_name = CTS_DEV_NODE_NAME;
	tdev->ops = &cts_dev_ops;
	if (cd && cd->fast_booting_solution) {
		thp_send_detect_cmd(tdev, panel_id, NO_SYNC_TIMEOUT);
		/*
		 * The thp_register_dev will be called later to complete
		 * the real detect action.If it fails, the detect function will
		 * release the resources requested here.
		 */
		return 0;
	}
	rc = thp_register_dev(tdev, panel_id);
	if (rc) {
		rc = -EFAULT;
		thp_log_err("%s: Register thp device failed: rc=%d\n",
			__func__, rc);
		touch_driver_dev_free(tdev);
		return rc;
	}
	thp_log_info("%s: THP dirver registered\n", __func__);

	return 0;
}

static int __init touch_driver_module_init(void)
{
	int rc = 0;
	struct device_node *dn = NULL;
	u32 i;

	thp_log_info("%s: called\n", __func__);
	for (i = 0; i < TOTAL_PANEL_NUM; i++) {
		dn = get_thp_node(i);
		if (dn && of_get_child_by_name(dn, CTS_DEV_NODE_NAME))
			rc = thp_module_init(i);
	}
	return rc;
}

/* module exit */
static void __exit touch_driver_module_exit(void)
{
	thp_log_info("%s: called, do nothing", __func__);
}

#ifdef CONFIG_HUAWEI_THP_QCOM
late_initcall(touch_driver_module_init);
#else
module_init(touch_driver_module_init);
#endif
module_exit(touch_driver_module_exit);
MODULE_AUTHOR("huawei");
MODULE_DESCRIPTION("huawei driver");
MODULE_LICENSE("GPL");

