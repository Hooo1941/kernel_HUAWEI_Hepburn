// SPDX-License-Identifier: GPL-2.0
/*
 * mt5806_fw.c
 *
 * mt5806 mtp, sram driver
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

#include "mt5806.h"
#include "mt5806_mtp.h"

#define HWLOG_TAG wireless_mt5806_fw
HWLOG_REGIST();

static void mt5806_fw_default_psy(struct mt5806_dev_info *di)
{
	/* depend on dts */
	switch (di->default_psy_type) {
	case MT5806_DEFAULT_POWEROFF:
		break;
	case MT5806_DEFAULT_POWERON:
		wlps_control(di->ic_type, WLPS_TX_PWR_SW, true);
		break;
	case MT5806_DEFAULT_LOWPOWER:
		wlps_control(di->ic_type, WLPS_TX_PWR_SW, true);
		power_msleep(DT_MSLEEP_20MS, 0, NULL); /* delay for power on */
		mt5806_tx_lowpower_enable(true, di);
		break;
	default:
		hwlog_err("default_psy: type error\n");
		break;
	}
}

static int mt5806_get_major_fw_version(struct mt5806_dev_info *di, u16 *fw)
{
	return mt5806_read_word(di, MT5806_MTP_MAJOR_ADDR, fw);
}

static int mt5806_get_minor_fw_version(struct mt5806_dev_info *di, u16 *fw)
{
	u8 min_h = 0;
	u8 min_l = 0;
	int ret;

	ret = mt5806_read_byte(di, MT5806_MTP_MINOR_ADDR, &min_l);
	ret += mt5806_read_byte(di, MT5806_MTP_MINOR_ADDR_H, &min_h);
	if (ret)
		return -EIO;

	*fw = (min_h << 8) | min_l;
	return 0;
}

static int mt5806_mtp_version_check(struct mt5806_dev_info *di)
{
	int ret;
	u16 major_fw_ver = 0;
	u16 minor_fw_ver = 0;

	ret = mt5806_get_major_fw_version(di, &major_fw_ver);
	if (ret)
		return ret;
	hwlog_info("[version_check] major_fw=0x%04x\n", major_fw_ver);

	ret = mt5806_get_minor_fw_version(di, &minor_fw_ver);
	if (ret)
		return ret;
	hwlog_info("[version_check] minor_fw=0x%04x\n", minor_fw_ver);

	if (minor_fw_ver != MT5806_MTP_MINOR_VER)
		return -ENXIO;

	return 0;
}

static int mt5806_mtp_pwr_cycle_chip(struct mt5806_dev_info *di)
{
	int ret;

	ret = mt5806_write_byte(di, MT5806_PMU_WDGEN_ADDR, MT5806_WDG_DISABLE); /* disable wtd, try 3 times */
	ret += mt5806_write_byte(di, MT5806_PMU_WDGEN_ADDR, MT5806_WDG_DISABLE);
	ret += mt5806_write_byte(di, MT5806_PMU_WDGEN_ADDR, MT5806_WDG_DISABLE);
	ret += mt5806_write_byte(di, MT5806_PMU_FLAG_ADDR, MT5806_WDT_INTFALG); /* clear wtd flag */

	ret += mt5806_write_byte(di, MT5806_SYS_KEY_ADDR, MT5806_KEY_VAL); /* write key to map */
	ret += mt5806_write_byte(di, MT5806_M0_CTRL_ADDR, MT5806_M0_HOLD_VAL); /* hold M0 */
	ret += mt5806_write_byte(di, MT5806_M0_CLK_ADDR, MT5806_M0_CLK_DISABLE_VAL); /* disalbe dma clk */
	ret += mt5806_write_byte(di, MT5806_M0_PWM_ADDR, MT5806_M0_PWM_DISABLE_VAL); /* disable pwm */
	ret += mt5806_write_byte(di, MT5806_M0_DMA_ADDR, MT5806_M0_DMA_DISABLE_VAL); /* disable dma */
	ret += mt5806_write_byte(di, MT5806_CODE_REMAP_ADDR, MT5806_CODE_REMAP_VAL); /* select mtp map addr */
	ret += mt5806_write_block(di, MT5806_BTLOADR_ADDR, (u8 *)g_mt5806_bootloop,
		(u8)ARRAY_SIZE(g_mt5806_bootloop)); /*load boot_loop */

	ret += mt5806_write_byte(di, MT5806_M0_CTRL_ADDR, MT5806_M0_RST_VAL); /* reset M0 */

	ret += mt5806_write_byte(di, MT5806_SYS_KEY_ADDR, MT5806_KEY_VAL); /* write key to map */
	ret += mt5806_write_byte(di, MT5806_M0_CTRL_ADDR, MT5806_M0_HOLD_VAL); /* hold M0 */
	ret += mt5806_write_byte(di, MT5806_CODE_REMAP_ADDR, MT5806_CODE_REMAP_VAL); /* select mtp map addr */
	if (ret) {
		hwlog_err("pwr_cycle_chip: failed\n");
		return -EIO;
	}

	(void)power_msleep(DT_MSLEEP_50MS, 0, NULL); /* for power on, typically 50ms */
	return 0;
}

static int mt5806_mtp_load_bootloader(struct mt5806_dev_info *di)
{
	int ret;
	int remaining = ARRAY_SIZE(g_mt5806_bootloader);
	int size_to_wr;
	int wr_already = 0;
	u16 chip_id = 0;
	u16 addr = MT5806_BTLOADR_ADDR;
	u8 wr_buff[MT5806_MTP_PGM_SIZE] = { 0 };

	while (remaining > 0) {
		size_to_wr = remaining > MT5806_MTP_PGM_SIZE ? MT5806_MTP_PGM_SIZE : remaining;
		memcpy(wr_buff, g_mt5806_bootloader + wr_already, size_to_wr);
		ret = mt5806_write_block(di, addr, wr_buff, size_to_wr);
		if (ret) {
			hwlog_err("load_bootloader: failed, addr=0x%04x\n", addr);
			return ret;
		}
		addr += size_to_wr;
		wr_already += size_to_wr;
		remaining -= size_to_wr;
	}

	ret = mt5806_write_byte(di, MT5806_M0_CTRL_ADDR, MT5806_M0_RST_VAL);
	if (ret) {
		hwlog_err("load_bootloader: reset M0 failed\n");
		return ret;
	}
	(void)power_msleep(DT_MSLEEP_50MS, 0, NULL); /* for power on, typically 50ms */
	ret = mt5806_read_word(di, MT5806_BOOTLOADER_CHIPID_ADDR, &chip_id);
	if (ret)
		return ret;

	hwlog_info("[load_bootloader] chip_id=0x%x\n", chip_id);
	if (chip_id != MT5806_CHIP_ID)
		return -ENXIO;

	hwlog_info("[load_bootloader] succ\n");
	return 0;
}

static int mt5806_mtp_status_check(struct mt5806_dev_info *di, u16 expect_status)
{
	int i;
	int ret;
	u16 status = 0;

	/* wait for 10ms*50=500ms for status check, typically 300ms */
	for (i = 0; i < 50; i++) {
		power_usleep(DT_USLEEP_10MS);
		ret = mt5806_read_word(di, MT5806_BOOT_STATUS_ADDR, &status);
		if (ret) {
			hwlog_err("status_check: read failed\n");
			return ret;
		}
		if (status == expect_status)
			return 0;
	}

	return -ENXIO;
}

static int mt5806_mtp_crc_check(struct mt5806_dev_info *di)
{
	int ret;
	u16 verify_result;

	ret = mt5806_write_word(di, MT5806_FW_LENGTH_ADDR, MT5806_APPLIB_LENGTH);
	ret += mt5806_write_word(di, MT5806_FW_CRC16VALUE_ADDR, MT5806_MTP_APPLIB_CRC);
	ret += mt5806_write_dword_mask(di, MT5806_TX_CMD_ADDR, MT5806_TX_CMD_FW_VERIFY,
		MT5806_TX_CMD_FW_VERIFY_SHIFT, MT5806_TX_CMD_VAL);

	(void)power_msleep(DT_MSLEEP_100MS, 0, NULL); /* for power on, typically 100ms */

	ret += mt5806_read_word(di, MT5806_FW_VERIFY_STATUS_ADDR, &verify_result);
	if (ret) {
		hwlog_err("crc_check: cmd error\n");
		return ret;
	}

	if (verify_result == MT5806_FW_VERIFY_STATUS_OK_VAL) {
		hwlog_info("[crc_check] succ\n");
		return 0;
	}
	hwlog_err("crc_check: error %x\n", verify_result);
	return -ENXIO;
}

static int mt5806_check_mtp_match(struct mt5806_dev_info *di)
{
	int ret;

	ret = mt5806_mtp_version_check(di);
	if (ret)
		return ret;

	ret = mt5806_mtp_crc_check(di);
	if (ret)
		return ret;

	return 0;
}

static int mt5806_mtp_crc_verify(struct mt5806_dev_info *di, u16 len, u16 crc, u16 reg)
{
	int ret;

	ret = mt5806_write_word(di, MT5806_BOOT_PGM_VERIFY_ADDR, crc);
	ret += mt5806_write_word(di, MT5806_BOOT_PGM_LEN_ADDR, len);
	ret += mt5806_write_dword(di, MT5806_BOOT_PGM_ADDR_ADDR, reg);
	ret += mt5806_write_word(di, MT5806_BOOT_CTRL_ADDR, MT5806_BOOT_CTRL_CRC_VERIFY_CMD);
	(void)power_msleep(DT_MSLEEP_20MS, 0, NULL); /* for power on, typically 20ms */
	ret += mt5806_mtp_status_check(di, MT5806_BOOT_STATUS_CRC_OK_VAL);
	if (ret) {
		hwlog_err("mtp_crc_verify: failed\n");
		return -EIO;
	}
	hwlog_info("[mtp_crc_verify] successed\n");

	return 0;
}

static int mt5806_mtp_load_fw(struct mt5806_dev_info *di, u16 start_addr, const u8 *data, u16 len)
{
	int i;
	int ret;
	u16 addr = start_addr; /* start from adrr */
	u16 remaining = len;
	u16 wr_size = 0;
	u16 chksum = 0;

	if (!di)
		return -ENODEV;

	while (remaining > 0) {
		wr_size = remaining > MT5806_MTP_PGM_SIZE ? MT5806_MTP_PGM_SIZE : remaining;
		ret = mt5806_write_dword(di, MT5806_BOOT_PGM_ADDR_ADDR, addr);
		if (ret) {
			hwlog_err("load_fw: write addr failed\n");
			return ret;
		}
		ret = mt5806_write_word(di, MT5806_BOOT_PGM_LEN_ADDR, wr_size);
		if (ret) {
			hwlog_err("load_fw: write len failed\n");
			return ret;
		}
		chksum = addr + wr_size;
		for (i = 0; i < wr_size; i++)
			chksum += data[addr - start_addr + i];
		ret = mt5806_write_word(di, MT5806_BOOT_PGM_VERIFY_ADDR, chksum);
		if (ret) {
			hwlog_err("load_fw: write checksum failed\n");
			return ret;
		}
		ret = mt5806_write_block(di, MT5806_BOOT_PGM_BUFFER_ADDR,
			(u8 *)&data[addr - start_addr], wr_size);
		if (ret) {
			hwlog_err("load_fw: write mtp_data failed\n");
			return ret;
		}

		ret = mt5806_write_word(di, MT5806_BOOT_CTRL_ADDR, MT5806_BOOT_CTRL_WRITE_CMD);
		if (ret) {
			hwlog_err("load_fw: start programming failed\n");
			return ret;
		}
		ret = mt5806_mtp_status_check(di, MT5806_BOOT_STATUS_WRITE_OK_VAL);
		if (ret) {
			hwlog_err("load_fw: check mtp status failed\n");
			return ret;
		}
		addr += wr_size;
		remaining -= wr_size;
	}

	return 0;
}

static int mt5806_mtp_erase(struct mt5806_dev_info *di, u16 cmd, u16 time)
{
	int ret;

	ret = mt5806_write_word(di, MT5806_BOOT_CTRL_ADDR, cmd);
	(void)power_msleep(time, 0, NULL);
	ret += mt5806_mtp_status_check(di, MT5806_BOOT_STATUS_ERASE_OK_VAL);
	if (ret) {
		hwlog_err("mtp_erase: MTP erase failed\n");
		return -EIO;
	}

	return 0;
}

static int mt5806_fw_write_q(struct mt5806_dev_info *di)
{
	int ret;
	u16 addr = MT5806_BOOT_QF_BUFFER1_ADDR; /* start from adrr 0x7F00 */
	u16 chksum = 0;
	u32 q_data = 0;

	if (!di)
		return -ENODEV;

	if (di->tx_q_factor.tx_q_flag == MT5806_Q_CALI_DYNAMIC)
		addr = MT5806_BOOT_QF_BUFFER0_ADDR;

	q_data = di->tx_q_factor.tx_q_cnt | (di->tx_q_factor.tx_q_width << 16);
	ret = mt5806_write_dword(di, MT5806_BOOT_PGM_ADDR_ADDR, addr);
	if (ret) {
		hwlog_err("fw_write_q: write addr failed\n");
		return ret;
	}
	ret = mt5806_write_word(di, MT5806_BOOT_PGM_LEN_ADDR, MT5806_BOOT_FACTOR_DATA_LEN);
	if (ret) {
		hwlog_err("fw_write_q: write len failed\n");
		return ret;
	}
	/* Calculate checksum in single byte */
	chksum = addr + MT5806_BOOT_FACTOR_DATA_LEN + (q_data & 0xff) + ((q_data >> 8) & 0xff) +
		((q_data >> 16) & 0xff) + ((q_data >> 24) & 0xff);
	ret = mt5806_write_word(di, MT5806_BOOT_PGM_VERIFY_ADDR, chksum);
	if (ret) {
		hwlog_err("fw_write_q: write checksum failed\n");
		return ret;
	}
	ret = mt5806_write_dword(di, MT5806_BOOT_PGM_BUFFER_ADDR, q_data);
	if (ret) {
		hwlog_err("fw_write_q: write mtp_data failed\n");
		return ret;
	}
	ret = mt5806_write_word(di, MT5806_BOOT_CTRL_ADDR, MT5806_BOOT_CTRL_WRITE_CMD);
	if (ret) {
		hwlog_err("fw_write_q: start programming failed\n");
		return ret;
	}
	ret = mt5806_mtp_status_check(di, MT5806_BOOT_STATUS_WRITE_OK_VAL);
	if (ret) {
		hwlog_err("fw_write_q: check mtp status failed\n");
		return ret;
	}

	return 0;
}

static int mt5806_fw_load_app(struct mt5806_dev_info *di)
{
	int ret;

	ret = mt5806_mtp_erase(di, MT5806_BOOT_CTRL_APP_ERASE_CMD, MT5806_BOOT_APP_ERASE_TIME_MS);
	if (ret) {
		hwlog_err("fw_load_app: chip erase app failed\n");
		return ret;
	}
	ret = mt5806_mtp_load_fw(di, MT5806_BOOT_APP_ADDR, g_mt5806_mtp_app,
		(u16)ARRAY_SIZE(g_mt5806_mtp_app));
	if (ret) {
		hwlog_err("fw_load_app: load fw app failed\n");
		return ret;
	}

	return 0;
}

static int mt5806_fw_load_lib(struct mt5806_dev_info *di)
{
	int ret;

	ret = mt5806_mtp_erase(di, MT5806_BOOT_CTRL_LIB_ERASE_CMD, MT5806_BOOT_LIB_ERASE_TIME_MS);
	if (ret) {
		hwlog_err("fw_load_lib: chip erase lib failed\n");
		return ret;
	}
	ret = mt5806_mtp_load_fw(di, MT5806_BOOT_LIB_ADDR, g_mt5806_mtp_lib,
		(u16)ARRAY_SIZE(g_mt5806_mtp_lib));
	if (ret) {
		hwlog_err("fw_load_lib: load fw lib failed\n");
		return ret;
	}

	return 0;
}

static int mt5806_fw_check_lib_app_load(struct mt5806_dev_info *di)
{
	int ret;

	ret = mt5806_mtp_crc_verify(di, MT5806_LIB_LENGTH, MT5806_MTP_LIB_CRC, MT5806_BOOT_LIB_ADDR);
	if (ret) {
		hwlog_info("[check_lib_app_load] need reload lib\n");
		ret = mt5806_fw_load_lib(di);
		if (ret) {
			hwlog_err("check_lib_app_load: load fw lib failed\n");
			return ret;
		}
	}
	hwlog_info("[check_lib_app_load] need reload app\n");
	ret = mt5806_fw_load_app(di);
	if (ret) {
		hwlog_err("check_lib_app_load: load fw app failed\n");
		return ret;
	}

	return 0;
}

static int mt5806_fw_load_mtp(struct mt5806_dev_info *di)
{
	int ret;

	ret = mt5806_mtp_pwr_cycle_chip(di);
	if (ret) {
		hwlog_err("fw_load_mtp: power cycle chip failed\n");
		goto exit;
	}
	ret = mt5806_mtp_load_bootloader(di);
	if (ret) {
		hwlog_err("fw_load_mtp: load bootloader failed\n");
		goto exit;
	}

	ret = mt5806_fw_check_lib_app_load(di);
	if (ret) {
		hwlog_err("fw_load_mtp: load fw lib_app failed\n");
		goto exit;
	}

	return 0;
exit:
	return ret;
}

static int mt5806_fw_check_q_data(struct mt5806_dev_info *di)
{
	int ret;
	u16 q_cnt = 0;
	u16 q_width = 0;

	if (di->tx_q_factor.tx_q_flag == MT5806_Q_CALI_DYNAMIC) {
		ret = mt5806_read_word(di, MT5806_TX_CALI_DYN_CNT_ADDR, &q_cnt);
		ret += mt5806_read_word(di, MT5806_TX_CALI_DYN_WIDTH_ADDR, &q_width);
	} else {
		ret = mt5806_read_word(di, MT5806_TX_CALI_FAC_CNT_ADDR, &q_cnt);
		ret += mt5806_read_word(di, MT5806_TX_CALI_FAC_WIDTH_ADDR, &q_width);
	}
	hwlog_info("[q_factor_check] cali_cnt=%d, cali_wid=%d, no_rx_cnt=%d, no_rx_wid=%d\n",
		q_cnt, q_width, di->tx_q_factor.tx_q_cnt, di->tx_q_factor.tx_q_width);
	if (!ret && (q_cnt == di->tx_q_factor.tx_q_cnt) && (q_width == di->tx_q_factor.tx_q_width))
		return 0;

	di->q_cali_result = MT5806_Q_CALI_FAIL;
	return -EINVAL;
}

int mt5806_fw_program_q_data(struct mt5806_dev_info *di)
{
	int ret = 0;

	mt5806_disable_irq_nosync(di);
	di->q_cali_result = MT5806_Q_WRITE_MTP;

	ret = mt5806_mtp_pwr_cycle_chip(di);
	if (ret) {
		hwlog_err("program_q_data: power cycle chip failed\n");
		goto exit;
	}
	ret = mt5806_mtp_load_bootloader(di);
	if (ret) {
		hwlog_err("program_q_data: load bootloader failed\n");
		goto exit;
	}
	if (di->tx_q_factor.tx_q_flag == MT5806_Q_CALI_DYNAMIC)
		ret = mt5806_mtp_erase(di, MT5806_BOOT_CTRL_DYN_DATA_CMD, MT5806_BOOT_FACTOR_ERASE_TIME_MS);
	else
		ret = mt5806_mtp_erase(di, MT5806_BOOT_CTRL_FAC_DATA_CMD, MT5806_BOOT_FACTOR_ERASE_TIME_MS);
	if (ret) {
		hwlog_err("program_q_data: erase q_val failed\n");
		goto exit;
	}
	ret = mt5806_fw_write_q(di);
	if (ret) {
		hwlog_err("program_q_data: load q_val failed\n");
		goto exit;
	}

	wlps_control(di->ic_type, WLPS_TX_PWR_SW, false);
	power_usleep(DT_USLEEP_20MS); /* for power off, typically 20ms */
	wlps_control(di->ic_type, WLPS_TX_PWR_SW, true);
	power_usleep(DT_USLEEP_20MS); /* for power on, typically 20ms */

	ret = mt5806_check_mtp_match(di);
	if (ret) {
		hwlog_err("program_q_data: mtp mismatch\n");
		goto exit;
	}
	ret = mt5806_fw_check_q_data(di);
	if (ret)
		goto exit;

	di->q_cali_result = MT5806_Q_CALI_SUCC;
	hwlog_info("[program_q_data] load qf succ\n");
exit:
	power_event_bnc_notify(mt5806_tx_get_bnt_wltx_type(di->ic_type),
		POWER_NE_WLTX_TX_Q_CALIBRATION, &di->q_cali_result);
	mt5806_enable_irq(di);
	wlps_control(di->ic_type, WLPS_TX_PWR_SW, false);
	mt5806_fw_default_psy(di);
	return ret;
}

static int mt5806_fw_get_cali_q_val(struct mt5806_dev_info *di)
{
	int ret;
	u16 q_cnt_fac = 0;
	u16 q_width_fac = 0;
	u16 q_cnt_dyn = 0;
	u16 q_width_dyn = 0;

	ret = mt5806_read_word(di, MT5806_TX_CALI_FAC_CNT_ADDR, &q_cnt_fac);
	ret += mt5806_read_word(di, MT5806_TX_CALI_FAC_WIDTH_ADDR, &q_width_fac);
	ret += mt5806_read_word(di, MT5806_TX_CALI_DYN_CNT_ADDR, &q_cnt_dyn);
	ret += mt5806_read_word(di, MT5806_TX_CALI_DYN_WIDTH_ADDR, &q_width_dyn);
	if (!ret)
		hwlog_info("[get_cali_q_val] factory: cnt=%u, width=%u, dynamic: cnt=%u, width=%u\n",
			q_cnt_fac, q_width_fac, q_cnt_dyn, q_width_dyn);

	return ret;
}

static int mt5806_fw_program_mtp(struct mt5806_dev_info *di)
{
	int ret;

	if (di->g_val.mtp_latest)
		return 0;

	mt5806_disable_irq_nosync(di);
	wlps_control(di->ic_type, WLPS_TX_PWR_SW, true);
	(void)mt5806_chip_enable(true, di);
	(void)power_msleep(DT_MSLEEP_100MS, 0, NULL); /* for power on, typically 100ms */

	ret = mt5806_fw_load_mtp(di);
	if (ret) {
		hwlog_err("program_mtp: mt5806_fw_load_mtp failed\n");
		goto exit;
	}
	wlps_control(di->ic_type, WLPS_TX_PWR_SW, false);
	power_usleep(DT_USLEEP_20MS); /* for power off, typically 20ms */
	wlps_control(di->ic_type, WLPS_TX_PWR_SW, true);
	power_usleep(DT_USLEEP_20MS); /* for power on, typically 20ms */

	ret = mt5806_check_mtp_match(di);
	if (ret) {
		hwlog_err("program_mtp: mtp mismatch\n");
		goto exit;
	}

	di->g_val.mtp_latest = true;
	hwlog_info("[program_mtp] succ\n");

exit:
	mt5806_enable_irq(di);
	wlps_control(di->ic_type, WLPS_TX_PWR_SW, false);
	return ret;
}

static int mt5806_fw_rx_program_mtp(unsigned int proc_type, void *dev_data)
{
	int ret;
	struct mt5806_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	hwlog_info("[rx_program_mtp] type=%u\n", proc_type);
	if (!di->g_val.mtp_chk_complete)
		return -EINVAL;
	di->g_val.mtp_chk_complete = false;
	ret = mt5806_fw_program_mtp(di);
	if (!ret)
		hwlog_info("[rx_program_mtp] succ\n");
	di->g_val.mtp_chk_complete = true;

	return ret;
}

static int mt5806_fw_check_mtp(void *dev_data)
{
	int ret;
	struct mt5806_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (di->g_val.mtp_latest)
		return 0;

	mt5806_disable_irq_nosync(di);
	wlps_control(di->ic_type, WLPS_TX_PWR_SW, true);
	(void)power_msleep(DT_MSLEEP_100MS, 0, NULL); /* for power on, typically 100ms */

	ret = mt5806_check_mtp_match(di);
	if (ret)
		goto exit;

	ret = mt5806_fw_get_cali_q_val(di);
	if (ret)
		hwlog_err("check_mtp: get q data failed\n");

	di->g_val.mtp_latest = true;
	hwlog_info("[check_mtp] mtp latest\n");

exit:
	mt5806_enable_irq(di);
	wlps_control(di->ic_type, WLPS_TX_PWR_SW, false);
	return ret;
}

int mt5806_fw_sram_update(void *dev_data)
{
	int ret;
	u16 minor_fw_ver = 0;
	struct mt5806_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = mt5806_get_minor_fw_version(di, &minor_fw_ver);
	if (ret) {
		hwlog_err("sram_update: get minor fw_ver failed\n");
		return ret;
	}
	hwlog_info("[sram_update] mtp_version=0x%x\n", minor_fw_ver);

	return 0;
}

static int mt5806_fw_get_mtp_status(unsigned int *status, void *dev_data)
{
	int ret;
	struct mt5806_dev_info *di = dev_data;

	if (!di || !status)
		return -EINVAL;

	di->g_val.mtp_chk_complete = false;
	ret = mt5806_fw_check_mtp(di);
	if (!ret)
		*status = WIRELESS_FW_PROGRAMED;
	else
		*status = WIRELESS_FW_ERR_PROGRAMED;
	di->g_val.mtp_chk_complete = true;

	return 0;
}

void mt5806_fw_mtp_check_work(struct work_struct *work)
{
	int i;
	int ret;
	struct mt5806_dev_info *di = NULL;

	if (!work)
		return;

	di = container_of(work, struct mt5806_dev_info, mtp_check_work.work);
	if (!di) {
		hwlog_err("mtp_check_work: di null\n");
		return;
	}
	di->g_val.mtp_chk_complete = false;
	ret = mt5806_fw_check_mtp(di);
	if (!ret) {
		hwlog_info("[mtp_check_work] succ\n");
		goto exit;
	}

	/* program for 3 times until it's ok */
	for (i = 0; i < 3; i++) {
		ret = mt5806_fw_program_mtp(di);
		if (ret)
			continue;
		hwlog_info("[mtp_check_work] update mtp succ, cnt=%d\n", i + 1);
		goto exit;
	}
	hwlog_err("mtp_check_work: update mtp failed\n");

exit:
	di->g_val.mtp_chk_complete = true;
	mt5806_fw_default_psy(di);
}

static struct wireless_fw_ops g_mt5806_fw_ops = {
	.ic_name       = "mt5806",
	.program_fw    = mt5806_fw_rx_program_mtp,
	.get_fw_status = mt5806_fw_get_mtp_status,
	.check_fw      = mt5806_fw_check_mtp,
};

int mt5806_fw_ops_register(struct wltrx_ic_ops *ops, struct mt5806_dev_info *di)
{
	if (!ops || !di)
		return -ENODEV;

	ops->fw_ops = kzalloc(sizeof(*(ops->fw_ops)), GFP_KERNEL);
	if (!ops->fw_ops)
		return -ENOMEM;

	memcpy(ops->fw_ops, &g_mt5806_fw_ops, sizeof(g_mt5806_fw_ops));
	ops->fw_ops->dev_data = (void *)di;

	return wireless_fw_ops_register(ops->fw_ops, di->ic_type);
}
