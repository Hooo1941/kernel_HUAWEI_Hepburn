// SPDX-License-Identifier: GPL-2.0
/*
 * mt5735_fw.c
 *
 * mt5735 mtp, sram driver
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
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

#include "mt5735.h"
#include "mt5735_mtp.h"

#define HWLOG_TAG wireless_mt5735_fw
HWLOG_REGIST();

static int mt5735_get_major_fw_version(u16 *fw)
{
	return mt5735_read_word(MT5735_MTP_MAJOR_ADDR, fw);
}

static int mt5735_get_minor_fw_version(u16 *fw)
{
	return mt5735_read_word(MT5735_MTP_MINOR_ADDR, fw);
}

static int mt5735_mtp_version_check(struct mt5735_dev_info *di)
{
	int ret;
	u16 major_fw_ver = 0;
	u16 minor_fw_ver = 0;

	ret = mt5735_get_major_fw_version(&major_fw_ver);
	if (ret)
		return ret;
	hwlog_info("[version_check] major_fw=0x%04x\n", major_fw_ver);

	ret = mt5735_get_minor_fw_version(&minor_fw_ver);
	if (ret)
		return ret;
	hwlog_info("[version_check] minor_fw=0x%04x\n", minor_fw_ver);

	if ((major_fw_ver != di->fw_mtp.fw_maj_ver) ||
		(minor_fw_ver != di->fw_mtp.fw_min_ver))
		return -ENXIO;

	return 0;
}

static int mt5735_mtp_pwr_cycle_chip(void)
{
	int ret;

	ret = mt5735_write_byte(0x5808, 0x95); /* disable wtd */
	ret += mt5735_write_byte(0x5800, 0x01); /* clear wtd flag */
	ret += mt5735_write_byte(0x5244, 0x57); /* write key to map */
	ret += mt5735_write_byte(0x5200, 0x20); /* hold M0 */
	ret += mt5735_write_word(0x5218, 0x0FFF); /* select sram map addr */
	ret += mt5735_write_byte(0x5208, 0x08); /* select mtp map addr */
	if (ret) {
		hwlog_err("pwr_cycle_chip: failed\n");
		return -EIO;
	}

	msleep(DT_MSLEEP_50MS); /* for power on, typically 20ms */
	return 0;
}

static int mt5735_fw_sram_write_data(u16 addr, const u8 *data, u16 len)
{
	int ret;
	u8 size_to_write;
	u16 remaining = len;
	u16 sram_addr = addr;
	u16 wr_already = 0;
	u8 wr_buff[MT5735_MTP_PGM_SIZE] = { 0 };

	while (remaining > 0) {
		size_to_write = remaining > MT5735_MTP_PGM_SIZE ?
			MT5735_MTP_PGM_SIZE : remaining;
		memcpy(wr_buff, data + wr_already, size_to_write);
		ret = mt5735_write_block(sram_addr, wr_buff, size_to_write);
		if (ret) {
			hwlog_err("sram_write_data: failed, addr=0x%04x\n", sram_addr);
			return ret;
		}

		sram_addr += size_to_write;
		wr_already += size_to_write;
		remaining -= size_to_write;
	}

	return 0;
}

static int mt5735_mtp_load_bootloader(struct mt5735_dev_info *di)
{
	int ret;
	u16 remaining = ARRAY_SIZE(g_mt5735_bootloader);
	u16 chip_id = 0;
	u16 addr = MT5735_BTLOADR_ADDR;

	ret = mt5735_fw_sram_write_data(addr, g_mt5735_bootloader, remaining);
	if (ret) {
		hwlog_err("load_bootloader: sram_write_data failed\n");
		return ret;
	}

	ret = mt5735_write_byte(0x5200, 0x80); /* reset M0 */
	if (ret) {
		hwlog_err("load_bootloader: reset M0 failed\n");
		return ret;
	}
	msleep(DT_MSLEEP_50MS); /* for power on, typically 20ms */
	ret = mt5735_read_word(MT5735_MTP_CHIP_ID_ADDR, &chip_id);
	if (ret)
		return ret;

	hwlog_info("[load_bootloader] chip_id=0x%x\n", chip_id);
	if (chip_id != MT5735_CHIP_ID)
		return -ENXIO;

	hwlog_info("[load_bootloader] succ\n");
	return 0;
}

static int mt5735_mtp_status_check(u16 expect_status)
{
	int i;
	int ret;
	u16 status = 0;

	/* wait for 10ms*50=500ms for status check, typically 300ms */
	for (i = 0; i < 50; i++) {
		power_usleep(DT_USLEEP_10MS);
		ret = mt5735_read_word(MT5735_MTP_STATUS_ADDR, &status);
		if (ret) {
			hwlog_err("status_check: read failed\n");
			return ret;
		}
		if (status == expect_status)
			return 0;
	}

	hwlog_err("status_check: status=0x%x\n", status);
	return -ENXIO;
}

static int mt5735_mtp_crc_check(struct mt5735_dev_info *di)
{
	int ret;

	ret = mt5735_mtp_load_bootloader(di);
	if (ret) {
		hwlog_err("crc_check: load bootloader failed\n");
		return ret;
	}

	/* write mtp start address */
	ret = mt5735_write_word(0x0002, 0x0000);
	/* write 16-bit MTP data size */
	ret += mt5735_write_word(0x0004, (u16)ARRAY_SIZE(g_mt5735_mtp));
	/* write the 16-bit CRC */
	ret += mt5735_write_word(0x0006, di->fw_mtp.fw_crc);
	/* start MTP data CRC-16 check */
	ret += mt5735_write_word(MT5735_MTP_CRC_ADDR, MT5735_MTP_CRC_CMD);
	ret += mt5735_mtp_status_check(MT5735_MTP_CRC_OK);
	if (ret) {
		hwlog_err("crc_check: failed\n");
		return ret;
	}

	hwlog_err("[crc_check] succ\n");
	return 0;
}

static int mt5735_mtp_crc_calc(const u8 *buf, u16 mtp_size)
{
	int i, j;
	u16 crc = MT5735_MTP_CRC_INIT;

	/* 2byte cycle, 1byte = 8bit */
	for (j = 0; j < mtp_size; j += 2) {
		crc ^= (buf[j + 1] << 8);
		for (i = 0; i < 8; i++)
			crc = (crc & MT5735_MTP_CRC_HIGHEST_BIT) ?
				(((crc << 1) & POWER_MASK_WORD) ^ MT5735_MTP_CRC_SEED) : (crc << 1);
		crc ^= (buf[j] << 8);
		for (i = 0; i < 8; i++)
			crc = (crc & MT5735_MTP_CRC_HIGHEST_BIT) ?
				(((crc << 1) & POWER_MASK_WORD) ^ MT5735_MTP_CRC_SEED) : (crc << 1);
	}

	hwlog_info("[mtp_crc_calc] crc=0x%x\n", crc);
	return crc;
}

static int mt5735_check_mtp_match(struct mt5735_dev_info *di)
{
	int ret;

	ret = mt5735_mtp_version_check(di);
	if (ret)
		return ret;

	ret = mt5735_mtp_pwr_cycle_chip();
	if (ret)
		return ret;

	ret = mt5735_mtp_crc_check(di);
	if (ret)
		return ret;

	return 0;
}

static int mt5735_fw_write_mtp_data(u16 addr, const u8 *buff, u16 len)
{
	int i;
	int ret;
	u16 chksum;

	chksum = addr + len;
	for (i = 0; i < len; i++)
		chksum += buff[i];

	ret = mt5735_write_word(MT5735_MTP_PGM_ADDR_ADDR, addr);
	if (ret) {
		hwlog_err("write_mtp_data: write addr failed\n");
		return ret;
	}

	ret = mt5735_write_word(MT5735_MTP_PGM_LEN_ADDR, len);
	if (ret) {
		hwlog_err("write_mtp_data: write len failed\n");
		return ret;
	}

	ret = mt5735_write_word(MT5735_MTP_PGM_CHKSUM_ADDR, chksum);
	if (ret) {
		hwlog_err("write_mtp_data: write checksum failed\n");
		return ret;
	}

	ret = mt5735_fw_sram_write_data(MT5735_MTP_PGM_DATA_ADDR, buff, len);
	if (ret) {
		hwlog_err("write_mtp_data: sram_write_data failed\n");
		return ret;
	}

	ret = mt5735_write_byte(MT5735_MTP_PGM_CMD_ADDR, MT5735_MTP_PGM_CMD);
	if (ret) {
		hwlog_err("write_mtp_data: start programming failed\n");
		return ret;
	}

	ret = mt5735_mtp_status_check(MT5735_MTP_PGM_OK);
	if (ret) {
		hwlog_err("write_mtp_data: check mtp status failed\n");
		return ret;
	}

	return 0;
}

static int mt5735_mtp_load_fw(struct mt5735_dev_info *di, const u8 *mtp_data, u16 remaining)
{
	u32 ret;
	u16 wr_already = 0;
	u16 addr = 0;
	u16 size_to_write;
	u8 buff[MT5735_MTP_BUFF_SIZE] = { 0 };

	hwlog_info("[load_fw] mtp_size=%d\n", remaining);
	while (remaining > 0) {
		memset(buff, 0, MT5735_MTP_BUFF_SIZE);
		size_to_write = remaining > MT5735_MTP_BUFF_SIZE ?
			MT5735_MTP_BUFF_SIZE : remaining;
		memcpy(buff, (mtp_data + wr_already), size_to_write);
		ret = mt5735_fw_write_mtp_data(addr, buff, size_to_write);
		if (ret) {
			hwlog_err("load_fw: write mtp data failed\n");
			return ret;
		}
		wr_already += size_to_write;
		addr += size_to_write;
		remaining -= size_to_write;
	}

	return 0;
}

static int mt5735_fw_program_mtp(struct mt5735_dev_info *di, const u8 *mtp_data, u16 mtp_size)
{
	int ret;

	if (di->g_val.mtp_latest)
		return 0;

	mt5735_disable_irq_nosync(di);
	wlps_control(di->ic_type, WLPS_PROC_OTP_PWR, true);
	(void)mt5735_chip_enable(true, di);
	msleep(DT_MSLEEP_100MS); /* for power on, typically 50ms */

	ret = mt5735_mtp_pwr_cycle_chip();
	if (ret) {
		hwlog_err("program_mtp: power cycle chip failed\n");
		goto exit;
	}
	ret = mt5735_mtp_load_bootloader(di);
	if (ret) {
		hwlog_err("program_mtp: load bootloader failed\n");
		goto exit;
	}
	ret = mt5735_mtp_load_fw(di, mtp_data, mtp_size);
	if (ret) {
		hwlog_err("program_mtp: load fw failed\n");
		goto exit;
	}
	wlps_control(di->ic_type, WLPS_PROC_OTP_PWR, false);
	power_usleep(DT_USLEEP_10MS); /* for power off, typically 5ms */
	wlps_control(di->ic_type, WLPS_PROC_OTP_PWR, true);
	power_usleep(DT_USLEEP_10MS); /* for power on, typically 5ms */
	ret = mt5735_check_mtp_match(di);
	if (ret) {
		hwlog_err("program_mtp: mtp mismatch\n");
		goto exit;
	}

	di->g_val.mtp_latest = true;
	hwlog_info("[program_mtp] succ\n");

exit:
	mt5735_enable_irq(di);
	wlps_control(di->ic_type, WLPS_PROC_OTP_PWR, false);
	return ret;
}

static int mt5735_fw_rx_program_mtp(unsigned int proc_type, void *dev_data)
{
	int ret;
	struct mt5735_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	hwlog_info("[rx_program_mtp] type=%u\n", proc_type);
	if (!di->g_val.mtp_chk_complete)
		return -EPERM;
	di->g_val.mtp_chk_complete = false;
	ret = mt5735_fw_program_mtp(di, g_mt5735_mtp, (u16)ARRAY_SIZE(g_mt5735_mtp));
	if (!ret)
		hwlog_info("[rx_program_mtp] succ\n");
	di->g_val.mtp_chk_complete = true;

	return ret;
}

static int mt5735_fw_check_mtp(void *dev_data)
{
	int ret = 0;
	u16 vrect = 0;
	struct mt5735_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (di->g_val.mtp_latest)
		return 0;

	if (power_cmdline_is_factory_mode() && mt5735_rx_is_tx_exist(di) &&
		!mt5735_mtp_version_check(di))
		goto succ;

	mt5735_disable_irq_nosync(di);
	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, true);
	msleep(DT_MSLEEP_100MS); /* for power on, typically 50ms */

	(void)mt5735_read_word(MT5735_MTP_GET_VRECT, &vrect);
	hwlog_info("[check_mtp] vrect=%u\n", vrect);

	ret = mt5735_check_mtp_match(di);
	if (ret)
		goto exit;

succ:
	di->g_val.mtp_latest = true;
	hwlog_info("[check_mtp] mtp latest\n");

exit:
	mt5735_enable_irq(di);
	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);
	return ret;
}

int mt5735_fw_sram_update(void *dev_data)
{
	int ret;
	u16 minor_fw_ver = 0;

	ret = mt5735_get_minor_fw_version(&minor_fw_ver);
	if (ret) {
		hwlog_err("sram_update: get minor fw_ver failed\n");
		return ret;
	}
	hwlog_info("mtp_version=0x%x\n", minor_fw_ver);

	return 0;
}

static int mt5735_fw_get_mtp_status(unsigned int *status, void *dev_data)
{
	int ret;
	struct mt5735_dev_info *di = dev_data;

	if (!di || !status)
		return -EINVAL;

	di->g_val.mtp_chk_complete = false;
	ret = mt5735_fw_check_mtp(di);
	if (!ret)
		*status = WIRELESS_FW_PROGRAMED;
	else
		*status = WIRELESS_FW_ERR_PROGRAMED;
	di->g_val.mtp_chk_complete = true;

	return 0;
}

static void mt5735_fw_mtp_check_work(struct work_struct *work)
{
	int i;
	int ret;
	struct mt5735_dev_info *di = container_of(work,
		struct mt5735_dev_info, mtp_check_work.work);

	if (!di)
		return;

	if (!power_cmdline_is_factory_mode() && mt5735_rx_is_tx_exist(di)) {
		hwlog_info("[mtp_check_work] tx exist\n");
		di->g_val.mtp_chk_complete = true;
		return;
	}

	power_wakeup_lock(di->fw_wakelock, false);
	di->g_val.mtp_chk_complete = false;
	ret = mt5735_fw_check_mtp(di);
	if (!ret) {
		hwlog_info("[mtp_check_work] succ\n");
		goto exit;
	}

	/* program for 3 times until it's ok */
	for (i = 0; i < 3; i++) {
		ret = mt5735_fw_program_mtp(di, g_mt5735_mtp, (u16)ARRAY_SIZE(g_mt5735_mtp));
		if (ret)
			continue;
		hwlog_info("[mtp_check_work] update mtp succ, cnt=%d\n", i + 1);
		goto exit;
	}
	hwlog_err("mtp_check_work: update mtp failed\n");

exit:
	di->g_val.mtp_chk_complete = true;
	power_wakeup_unlock(di->fw_wakelock, false);
}

void mt5735_fw_mtp_check(struct mt5735_dev_info *di)
{
	u32 mtp_check_delay;

	if (power_cmdline_is_powerdown_charging_mode() ||
		(!power_cmdline_is_factory_mode() && mt5735_rx_is_tx_exist(di))) {
		di->g_val.mtp_chk_complete = true;
		return;
	}

	if (!power_cmdline_is_factory_mode())
		mtp_check_delay = di->mtp_check_delay.user;
	else
		mtp_check_delay = di->mtp_check_delay.fac;
	INIT_DELAYED_WORK(&di->mtp_check_work, mt5735_fw_mtp_check_work);
	di->fw_wakelock = power_wakeup_source_register(di->dev, "wl_fw_wakelock");
	schedule_delayed_work(&di->mtp_check_work, msecs_to_jiffies(mtp_check_delay));
}

ssize_t mt5735_fw_write(void *dev_data, const char *buf, size_t size)
{
	int hdr_size, crc;
	struct power_fw_hdr *hdr = NULL;
	struct mt5735_dev_info *di = (struct mt5735_dev_info *)dev_data;

	if (!di || !buf)
		return -EINVAL;

	hdr = (struct power_fw_hdr *)buf;
	hdr_size = sizeof(struct power_fw_hdr);
	crc = mt5735_mtp_crc_calc((const u8 *)hdr + hdr_size, (u16)hdr->bin_size);
	hwlog_info("[fw_write] bin_size=%ld version_id=0x%x crc_id=0x%x\n",
		hdr->bin_size, hdr->version_id, hdr->crc_id);

	if ((hdr->unlock_val != WLTRX_UNLOCK_VAL) || (hdr->fw_size != hdr->bin_size) ||
		(hdr->crc_id != crc)) {
		hwlog_err("fw_write: config mismatch\n");
		return -EINVAL;
	}

	di->fw_mtp.fw_min_ver = hdr->version_id;
	di->fw_mtp.fw_crc = hdr->crc_id;
	di->g_val.mtp_latest = false;

	(void)mt5735_fw_check_mtp(di);
	if (di->g_val.mtp_latest)
		return size;

	(void)mt5735_fw_program_mtp(di, (const u8 *)hdr + hdr_size, (u16)hdr->bin_size);

	return size;
}

static bool mt5735_fw_is_mtp_chk_complete(void *dev_data)
{
	struct mt5735_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("is_mtp_chk_complete: di null\n");
		return false;
	}

	return di->g_val.mtp_chk_complete;
}

static struct wireless_fw_ops g_mt5735_fw_ops = {
	.ic_name       = "mt5735",
	.program_fw    = mt5735_fw_rx_program_mtp,
	.get_fw_status = mt5735_fw_get_mtp_status,
	.check_fw      = mt5735_fw_check_mtp,
	.write_fw      = mt5735_fw_write,
	.is_mtp_chk_complete = mt5735_fw_is_mtp_chk_complete,
};

int mt5735_fw_ops_register(struct mt5735_dev_info *di)
{
	if (!di)
		return -ENODEV;

	di->fw_mtp.fw_maj_ver = MT5735_MTP_MAJOR_VER;
	di->fw_mtp.fw_min_ver = MT5735_MTP_MINOR_VER;
	di->fw_mtp.fw_crc = MT5735_MTP_CRC;
	g_mt5735_fw_ops.dev_data = (void *)di;

	return wireless_fw_ops_register(&g_mt5735_fw_ops, di->ic_type);
}
