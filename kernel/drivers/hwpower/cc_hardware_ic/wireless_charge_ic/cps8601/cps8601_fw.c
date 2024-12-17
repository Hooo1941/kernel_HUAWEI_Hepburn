// SPDX-License-Identifier: GPL-2.0
/*
 * cps8601_fw.c
 *
 * cps8601 mtp, sram driver
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#include "cps8601.h"
#include "cps8601_mtp.h"

#define HWLOG_TAG wireless_cps8601_fw
HWLOG_REGIST();

static void cps8601_fw_default_psy(struct cps8601_dev_info *di)
{
	/* depend on dts */
	switch (di->default_psy_type) {
	case CPS8601_DEFAULT_POWEROFF:
		break;
	case CPS8601_DEFAULT_POWERON:
		wlps_control(di->ic_type, WLPS_TX_PWR_SW, true);
		break;
	case CPS8601_DEFAULT_LOWPOWER:
		wlps_control(di->ic_type, WLPS_TX_PWR_SW, true);
		power_msleep(DT_MSLEEP_20MS, 0, NULL); /* delay for power on */
		cps8601_tx_lowpower_enable(true, di);
		break;
	default:
		hwlog_err("default_psy: type error\n");
		break;
	}
}

static int cps8601_fw_check_mtp_crc(struct cps8601_dev_info *di)
{
	int i;
	int ret;
	u16 crc = 0;

	ret = cps8601_write_word_mask(di, CPS8601_TX_CMD_ADDR, CPS8601_TX_CMD_CRC_CHK,
		CPS8601_TX_CMD_CRC_CHK_SHIFT, CPS8601_TX_CMD_VAL);
	if (ret) {
		hwlog_err("check_mtp_crc: write cmd failed\n");
		return ret;
	}

	/* 100ms*10=1s timeout for status check */
	for (i = 0; i < 10; i++) {
		(void)power_msleep(DT_MSLEEP_100MS, 0, NULL);
		ret = cps8601_read_word(di, CPS8601_CRC_ADDR, &crc);
		if (ret) {
			hwlog_err("check_mtp_crc: get crc failed\n");
			return ret;
		}
		if (crc == CPS8601_MTP_CRC)
			return 0;
	}

	hwlog_err("check_mtp_crc: timeout, crc=0x%x\n", crc);
	return -EINVAL;
}

static int cps8601_fw_check_mtp_version(struct cps8601_dev_info *di)
{
	int ret;
	struct cps8601_chip_info info = { 0 };

	ret = cps8601_get_chip_info(di, &info);
	if (ret) {
		hwlog_err("check_mtp_version: get chip_info failed\n");
		return ret;
	}

	hwlog_info("[check_mtp_version] mtp_ver=0x%04x\n", info.mtp_ver);
	if (info.mtp_ver != CPS8601_MTP_VER) {
		hwlog_err("[check_mtp_version] ver=0x%04x\n", CPS8601_MTP_VER);
		return -EINVAL;
	}

	return 0;
}

static int cps8601_fw_get_cali_q_val(struct cps8601_dev_info *di)
{
	int ret;
	u8 q_cnt_fac = 0;
	u16 q_width_fac = 0;
	u8 q_cnt_dyn = 0;
	u16 q_width_dyn = 0;

	ret = cps8601_read_byte(di, CPS8601_TX_CALI_FAC_CNT_ADDR, &q_cnt_fac);
	ret += cps8601_read_word(di, CPS8601_TX_CALI_FAC_WIDTH_ADDR, &q_width_fac);
	ret += cps8601_read_byte(di, CPS8601_TX_CALI_DYN_CNT_ADDR, &q_cnt_dyn);
	ret += cps8601_read_word(di, CPS8601_TX_CALI_DYN_WIDTH_ADDR, &q_width_dyn);
	if (!ret)
		hwlog_info("[get_cali_q_val] factory: cnt=%u, width=%u, dynamic: cnt=%u, width=%u\n",
			q_cnt_fac, q_width_fac, q_cnt_dyn, q_width_dyn);

	return ret;
}

static int cps8601_fw_write_sram_data(struct cps8601_dev_info *di, u32 cur_addr,
	const u8 *data, int len)
{
	int ret;
	int size_to_wr;
	u32 wr_already = 0;
	u32 addr = cur_addr;
	int remaining = len;
	u8 wr_buff[CPS8601_FW_PAGE_SIZE] = { 0 };

	while (remaining > 0) {
		size_to_wr = remaining > CPS8601_FW_PAGE_SIZE ? CPS8601_FW_PAGE_SIZE : remaining;
		memcpy(wr_buff, data + wr_already, size_to_wr);
		ret = cps8601_hw_write_block(di, addr, wr_buff, size_to_wr);
		if (ret) {
			hwlog_err("write_sram_data: fail, addr=0x%x\n", addr);
			return ret;
		}
		addr += size_to_wr;
		wr_already += size_to_wr;
		remaining -= size_to_wr;
	}

	return 0;
}

static u16 cps8601_fw_crc_cal(const u8 *start, u16 len)
{
	u16 i, j;
	u16 crc_in = 0x0000;
	u16 crc_poly = CPS8601_FW_CRC_SEED;

	for (i = 0; i < len; i++) {
		crc_in ^= (*(start + i) << POWER_BITS_PER_BYTE);
		for (j = 0; j < POWER_BITS_PER_BYTE; j++) {
			if (crc_in & CPS8601_FW_CRC_HIGHEST_BIT)
				crc_in = (crc_in << 1) ^ crc_poly;
			else
				crc_in = crc_in << 1;
		}
	}

	return crc_in;
}

static int cps8601_fw_config_i2c(struct cps8601_dev_info *di)
{
	int ret;
	u32 data = 0;

	ret = cps8601_hw_write_dword(di, CPS8601_ACCESS_32BIT_REG_ADD,
		CPS8601_ENABLE_32BIT_ADD_ACCESS);
	power_msleep(DT_MSLEEP_100MS, 0, NULL);
	ret += cps8601_hw_read_dword(di, CPS8601_ACCESS_32BIT_REG_ADD, &data);
	if (ret || (data != CPS8601_ENABLE_32BIT_ADD_ACCESS)) {
		hwlog_err("config_i2c: check fail 0x%x\n", data);
		return -EINVAL;
	}
	power_msleep(DT_MSLEEP_25MS, 0, NULL);
	return cps8601_hw_write_dword(di, CPS8601_CLKCTRL_I2C_BUS_MODE,
		CPS8601_I2C_BUS_MODE_LITTLE_ENDIAN);
}

static int cps8601_fw_status_check(struct cps8601_dev_info *di, u32 cmd)
{
	int i;
	int ret;
	u8 status = 0;

	ret = cps8601_hw_write_dword(di, CPS8601_BOOTLOADER_CMD_ADD, cmd);
	if (ret) {
		hwlog_err("status_check: set check cmd failed\n");
		return ret;
	}

	/* wait for 50ms*10=500ms for status check */
	for (i = 0; i < 10; i++) {
		(void)power_msleep(DT_MSLEEP_50MS, 0, NULL);
		ret = cps8601_hw_read_block(di, CPS8601_BOOTLOADER_FLAG_ADD, &status, POWER_BYTE_LEN);
		if (ret) {
			hwlog_err("status_check: get status failed\n");
			return ret;
		}

		if (status == CPS8601_CHK_SUCC)
			return 0;

		if ((status == CPS8601_CHK_FAIL) || (status == CPS8601_CHK_ILLEGAL)) {
			hwlog_err("status_check: failed, stat=0x%x\n", status);
			return -EINVAL;
		}
	}

	hwlog_err("status_check: status=0x%x, program timeout\n", status);
	return -EINVAL;
}

static int cps8601_fw_check_bootloader(struct cps8601_dev_info *di)
{
	int ret;
	u32 data = 0;
	u16 crc_cal = 0;

	crc_cal = cps8601_fw_crc_cal(g_cps8601_bootloader, CPS8601_FW_BTL_SIZE);

	ret = cps8601_hw_write_dword(di, CPS8601_BOOTLOADER_ADDR_BUFFER0, CPS8601_FW_BTL_SIZE);
	power_usleep(DT_USLEEP_2MS);
	ret += cps8601_hw_read_dword(di, CPS8601_BOOTLOADER_ADDR_BUFFER0, &data);
	if (ret || (data != CPS8601_FW_BTL_SIZE)) {
		hwlog_err("check_bootloader: fw size write failed, data=0x%x\n", data);
		return -EINVAL;
	}
	power_msleep(DT_MSLEEP_25MS, 0, NULL);
	ret = cps8601_fw_status_check(di, CPS8601_CMD_CACL_CRC_TEST);
	if (ret) {
		hwlog_err("check_bootloader: cmd failed\n");
		return ret;
	}

	ret = cps8601_hw_read_dword(di, CPS8601_BOOTLOADER_ADDR_BUFFER0, &data);
	if (ret || (data != crc_cal)) {
		hwlog_err("check_bootloader: crc_failed, data=0x%x, expect=0x%x\n", data, crc_cal);
		return -EINVAL;
	}

	return 0;
}

static int cps8601_fw_program_prev_process(struct cps8601_dev_info *di)
{
	int ret;

	ret = cps8601_fw_config_i2c(di);
	if (ret) {
		hwlog_err("program_prev_process: config i2c failed\n");
		return ret;
	}

	hwlog_info("[program_prev_process] success\n");
	return 0;
}

static int cps8601_fw_load_bootloader(struct cps8601_dev_info *di)
{
	int ret;
	int i;
	const struct cps8601_fw_reg_para cps8601_fw_boot_cfg[CPS8601_FW_REG_CFG_SIZE] = {
		{ CPS8601_CLKCTRL_PASSWORD, CPS8601_ENABLE_PASSWORD, 2 },
		{ CPS8601_CLKCTRL_SYSCONFIG, CPS8601_RESET_MCU, 2 },
		{ CPS8601_DMACTRL_CHANNEL, CPS8601_DISABLE_ALL_CHANNEL, 2 },
		{ CPS8601_CLKCTRL_WATCHDOG_LOAD, CPS8601_WATCH_DOG_RELOAD_MAX, 300 },
		{ CPS8601_CLKCTRL_I2C_BUS_MODE, CPS8601_I2C_BUS_MODE_BIG_ENDIAN, 3 },
		{ CPS8601_BOOTLOADER_BOOT_ADD, 0, 0 },
		{ CPS8601_CLKCTRL_I2C_BUS_MODE, CPS8601_I2C_BUS_MODE_LITTLE_ENDIAN, 3 },
		{ CPS8601_CLKCTRL_TRIM_DIS, CPS8601_TRIMING_LOAD_DISABLED, 2 },
		{ CPS8601_CLKCTRL_SYSCONFIG, CPS8601_SET_BOOT_SOURCE_SRAM, 110 },
	};

	for (i = 0; i < CPS8601_FW_REG_CFG_SIZE; i++) {
		if (cps8601_fw_boot_cfg[i].addr != CPS8601_BOOTLOADER_BOOT_ADD) {
			ret = cps8601_hw_write_dword(di, cps8601_fw_boot_cfg[i].addr,
				cps8601_fw_boot_cfg[i].data);
			if (ret) {
				hwlog_err("load_bootloader: write reg=0x%x failed\n", cps8601_fw_boot_cfg[i].addr);
				return ret;
			}
			power_msleep(cps8601_fw_boot_cfg[i].interval, 0, NULL);
		} else {
			ret = cps8601_fw_write_sram_data(di, CPS8601_BOOTLOADER_BOOT_ADD,
				g_cps8601_bootloader, CPS8601_FW_BTL_SIZE);
			if (ret) {
				hwlog_err("load_bootloader: wirte bootloader error\n");
				return ret;
			}
		}
	}

	hwlog_info("[load_bootloader] check status\n");
	/* after successful download bootloader, check ic again */
	ret = cps8601_fw_config_i2c(di);
	if (ret) {
		hwlog_err("load_bootloader: config i2c failed\n");
		return ret;
	}
	ret = cps8601_fw_check_bootloader(di);
	if (ret) {
		hwlog_err("load_bootloader: crc failed\n");
		return ret;
	}

	hwlog_info("[load_bootloader] success\n");
	return 0;
}

static int cps8601_fw_write_mtp_data(struct cps8601_dev_info *di)
{
	int ret;
	int offset = 0;
	int remaining = CPS8601_FW_MTP_SIZE;
	int wr_size;

	while (remaining > 0) {
		wr_size = remaining > CPS8601_SRAM_MTP_BUFF_SIZE ? CPS8601_SRAM_MTP_BUFF_SIZE : remaining;
		ret = cps8601_fw_write_sram_data(di, CPS8601_BOOTLOADER_ADDR_BUFFER0,
			g_cps8601_mtp + offset, wr_size);
		if (ret) {
			hwlog_err("write_mtp_data: write mtp failed\n");
			return ret;
		}

		ret = cps8601_fw_status_check(di, CPS8601_CMD_PGM_BUFFER0);
		if (ret) {
			hwlog_err("write_mtp_data: check crc failed\n");
			return ret;
		}
		offset += wr_size;
		remaining -= wr_size;
	}

	return 0;
}

static int cps8601_fw_check_mtp_crc_cal(struct cps8601_dev_info *di)
{
	u32 data = 0;
	int ret;
	u16 crc_cal;

	crc_cal = cps8601_fw_crc_cal(g_cps8601_mtp, CPS8601_FW_MTP_SIZE);

	ret = cps8601_hw_write_dword(di, CPS8601_BOOTLOADER_ADDR_BUFFER0, CPS8601_FW_MTP_SIZE);
	power_usleep(DT_USLEEP_2MS);
	ret += cps8601_hw_read_dword(di, CPS8601_BOOTLOADER_ADDR_BUFFER0, &data);
	if (ret || (data != CPS8601_FW_MTP_SIZE)) {
		hwlog_err("check_mtp_crc_cal: fw size write failed, data=0x%x\n", data);
		return -EINVAL;
	}
	power_usleep(DT_USLEEP_2MS);
	ret = cps8601_fw_status_check(di, CPS8601_CMD_CACL_CRC_MTP);
	if (ret)
		return ret;

	ret = cps8601_hw_read_dword(di, CPS8601_BOOTLOADER_ADDR_BUFFER0, &data);
	power_usleep(DT_USLEEP_2MS);
	if (ret || (data != crc_cal)) {
		hwlog_err("check_mtp_crc_cal: mtp crc failed, read:0x%x, expect:0x%x\n", data, crc_cal);
		return -EINVAL;
	}

	return 0;
}

static int cps8601_fw_load_q_data(struct cps8601_dev_info *di)
{
	int ret;
	u32 q_data = di->tx_q_factor.tx_q_cnt | (di->tx_q_factor.tx_q_width << 16);

	ret = cps8601_hw_write_dword(di, CPS8601_BOOTLOADER_ADDR_BUFFER0, q_data);
	if (ret) {
		hwlog_err("load_q_data: failed\n");
		return ret;
	}
	power_msleep(DT_MSLEEP_10MS, 0, NULL);
	if (di->tx_q_factor.tx_q_flag == CPS8601_Q_CALI_DYNAMIC)
		ret = cps8601_fw_status_check(di, CPS8601_CMD_PGM_Q_FACTOR_DYNAMIC);
	else
		ret = cps8601_fw_status_check(di, CPS8601_CMD_PGM_Q_FACTOR_FACTORY);
	if (ret) {
		hwlog_err("load_q_data: check failed, ret=%d\n", ret);
		return ret;
	}

	hwlog_info("[load_q_data] success\n");
	return 0;
}

static int cps8601_fw_load_mtp(struct cps8601_dev_info *di)
{
	int ret;

	ret = cps8601_fw_status_check(di, CPS8601_CMD_PGM_ERASER);
	if (ret) {
		hwlog_err("load_mtp: erase failed\n");
		return ret;
	}
	power_msleep(DT_MSLEEP_10MS, 0, NULL);
	ret = cps8601_fw_write_mtp_data(di);
	if (ret) {
		hwlog_err("load_mtp: download failed\n");
		return ret;
	}

	ret = cps8601_fw_check_mtp_crc_cal(di);
	if (ret) {
		hwlog_err("load_mtp: crc failed\n");
		return ret;
	}
	ret = cps8601_fw_status_check(di, CPS8601_CMD_PGM_WR_FLAG);
	if (ret) {
		hwlog_err("load_mtp: write failed\n");
		return ret;
	}

	hwlog_info("[load_mtp] success\n");
	return 0;
}

static int cps8601_fw_progam_post_process(struct cps8601_dev_info *di)
{
	int ret;

	ret = cps8601_fw_config_i2c(di);
	if (ret) {
		hwlog_err("progam_post_process: config_i2c failed\n");
		return ret;
	}
	ret = cps8601_hw_write_dword(di, CPS8601_CLKCTRL_PASSWORD, CPS8601_ENABLE_PASSWORD);
	if (ret) {
		hwlog_err("progam_post_process: enable password failed\n");
		return ret;
	}
	power_usleep(DT_USLEEP_2MS);
	ret = cps8601_hw_write_dword(di, CPS8601_CLKCTRL_SYSCONFIG, CPS8601_RESET_ALL_SYSTEM);
	if (ret) {
		hwlog_err("progam_post_process: reset system failed\n");
		return ret;
	}

	wlps_control(di->ic_type, WLPS_TX_PWR_SW, false);
	power_usleep(DT_USLEEP_10MS);
	wlps_control(di->ic_type, WLPS_TX_PWR_SW, true);
	(void)power_msleep(DT_MSLEEP_100MS, 0, NULL);

	ret = cps8601_fw_check_mtp_version(di);
	if (ret) {
		hwlog_err("progam_post_process: mtp_version mismatch\n");
		return ret;
	}

	hwlog_info("[progam_post_process] success\n");
	return 0;
}

static int cps8601_fw_check_q_data(struct cps8601_dev_info *di)
{
	int ret;
	u8 q_cnt = 0;
	u16 q_width = 0;

	if (di->tx_q_factor.tx_q_flag == CPS8601_Q_CALI_DYNAMIC) {
		ret = cps8601_read_byte(di, CPS8601_TX_CALI_DYN_CNT_ADDR, &q_cnt);
		ret += cps8601_read_word(di, CPS8601_TX_CALI_DYN_WIDTH_ADDR, &q_width);
	} else {
		ret = cps8601_read_byte(di, CPS8601_TX_CALI_FAC_CNT_ADDR, &q_cnt);
		ret += cps8601_read_word(di, CPS8601_TX_CALI_FAC_WIDTH_ADDR, &q_width);
	}
	hwlog_info("[q_factor_check] cali_cnt=%d, cali_wid=%d, no_rx_cnt=%d, no_rx_wid=%d\n",
		q_cnt, q_width, di->tx_q_factor.tx_q_cnt, di->tx_q_factor.tx_q_width);
	if (!ret && (q_cnt == di->tx_q_factor.tx_q_cnt) && (q_width == di->tx_q_factor.tx_q_width))
		return 0;

	di->q_cali_result = CPS8601_Q_CALI_FAIL;
	return -EINVAL;
}

int cps8601_fw_program_q_data(struct cps8601_dev_info *di)
{
	int ret;

	cps8601_disable_irq_nosync(di);
	di->q_cali_result = CPS8601_Q_WRITE_MTP;

	ret = cps8601_fw_program_prev_process(di);
	if (ret)
		goto exit;

	ret = cps8601_fw_load_bootloader(di);
	if (ret)
		goto exit;

	ret = cps8601_fw_load_q_data(di);
	if (ret)
		goto exit;

	ret = cps8601_fw_progam_post_process(di);
	if (ret)
		goto exit;

	ret = cps8601_fw_check_q_data(di);
	if (ret)
		goto exit;

	di->q_cali_result = CPS8601_Q_CALI_SUCC;
	hwlog_info("[program_q_data] succ\n");

exit:
	power_event_bnc_notify(cps8601_tx_get_bnt_wltx_type(di->ic_type),
		POWER_NE_WLTX_TX_Q_CALIBRATION, &di->q_cali_result);
	cps8601_enable_irq(di);
	wlps_control(di->ic_type, WLPS_TX_PWR_SW, false);
	cps8601_fw_default_psy(di);
	return ret;
}

static int cps8601_fw_program_mtp(struct cps8601_dev_info *di)
{
	int ret;

	if (di->g_val.mtp_latest)
		return 0;

	cps8601_disable_irq_nosync(di);
	wlps_control(di->ic_type, WLPS_TX_PWR_SW, true);
	cps8601_chip_enable(true, di);
	(void)power_msleep(DT_MSLEEP_100MS, 0, NULL);

	ret = cps8601_fw_program_prev_process(di);
	if (ret)
		goto exit;

	ret = cps8601_fw_load_bootloader(di);
	if (ret)
		goto exit;

	ret = cps8601_fw_load_mtp(di);
	if (ret)
		goto exit;

	ret = cps8601_fw_progam_post_process(di);
	if (ret)
		goto exit;

	di->g_val.mtp_latest = true;
	hwlog_info("[program_mtp] succ\n");

exit:
	cps8601_enable_irq(di);
	wlps_control(di->ic_type, WLPS_TX_PWR_SW, false);
	return ret;
}

static int cps8601_fw_rx_program_mtp(unsigned int proc_type, void *dev_data)
{
	int ret;
	struct cps8601_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	hwlog_info("[rx_program_mtp] type=%u\n", proc_type);

	if (!di->g_val.mtp_chk_complete)
		return -EINVAL;

	di->g_val.mtp_chk_complete = false;
	ret = cps8601_fw_program_mtp(di);
	if (!ret)
		hwlog_info("[rx_program_mtp] succ\n");
	di->g_val.mtp_chk_complete = true;

	return ret;
}

static int cps8601_fw_check_mtp(void *dev_data)
{
	struct cps8601_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (di->g_val.mtp_latest)
		return 0;

	wlps_control(di->ic_type, WLPS_TX_PWR_SW, true);
	(void)power_msleep(DT_MSLEEP_100MS, 0, NULL);

	if (cps8601_fw_check_mtp_version(di)) {
		hwlog_err("check_mtp: mtp_ver mismatch\n");
		goto check_fail;
	}

	if (cps8601_fw_check_mtp_crc(di)) {
		hwlog_err("check_mtp: mtp_crc mismatch\n");
		goto check_fail;
	}

	if (cps8601_fw_get_cali_q_val(di))
		hwlog_err("check_mtp: get q data failed\n");

	di->g_val.mtp_latest = true;
	wlps_control(di->ic_type, WLPS_TX_PWR_SW, false);
	hwlog_info("[check_mtp] mtp latest\n");
	return 0;

check_fail:
	wlps_control(di->ic_type, WLPS_TX_PWR_SW, false);
	return -EINVAL;
}

int cps8601_fw_sram_update(void *dev_data)
{
	return 0;
}

static int cps8601_fw_get_mtp_status(unsigned int *status, void *dev_data)
{
	int ret;
	struct cps8601_dev_info *di = dev_data;

	if (!di || !status)
		return -EINVAL;

	di->g_val.mtp_chk_complete = false;
	ret = cps8601_fw_check_mtp(di);
	if (!ret)
		*status = WIRELESS_FW_PROGRAMED;
	else
		*status = WIRELESS_FW_ERR_PROGRAMED;
	di->g_val.mtp_chk_complete = true;

	return 0;
}

void cps8601_fw_mtp_check_work(struct work_struct *work)
{
	int i;
	int ret;
	struct cps8601_dev_info *di = NULL;

	if (!work)
		return;

	di = container_of(work, struct cps8601_dev_info, mtp_check_work.work);
	if (!di) {
		hwlog_err("mtp_check_work: di null\n");
		return;
	}

	di->g_val.mtp_chk_complete = false;
	ret = cps8601_fw_check_mtp(di);
	if (!ret) {
		hwlog_info("[mtp_check_work] succ\n");
		goto exit;
	}

	/* program for 3 times until it's ok */
	for (i = 0; i < 3; i++) {
		ret = cps8601_fw_program_mtp(di);
		if (ret)
			continue;
		hwlog_info("[mtp_check_work] update mtp succ, cnt=%d\n", i + 1);
		goto exit;
	}
	hwlog_err("mtp_check_work: update mtp failed\n");

exit:
	di->g_val.mtp_chk_complete = true;
	cps8601_fw_default_psy(di);
}

static struct wireless_fw_ops g_cps8601_fw_ops = {
	.ic_name                = "cps8601",
	.program_fw             = cps8601_fw_rx_program_mtp,
	.get_fw_status          = cps8601_fw_get_mtp_status,
	.check_fw               = cps8601_fw_check_mtp,
};

int cps8601_fw_ops_register(struct wltrx_ic_ops *ops, struct cps8601_dev_info *di)
{
	if (!ops || !di)
		return -ENODEV;

	ops->fw_ops = kzalloc(sizeof(*(ops->fw_ops)), GFP_KERNEL);
	if (!ops->fw_ops)
		return -ENOMEM;

	memcpy(ops->fw_ops, &g_cps8601_fw_ops, sizeof(g_cps8601_fw_ops));
	ops->fw_ops->dev_data = (void *)di;

	return wireless_fw_ops_register(ops->fw_ops, di->ic_type);
}
