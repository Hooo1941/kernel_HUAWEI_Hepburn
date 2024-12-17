// SPDX-License-Identifier: GPL-2.0
/*
 * cbm8580_power_saving.c
 *
 * Intelligent power protection driver
 *
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd.
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

#include "cbm8580_interface.h"

#include <linux/reboot.h>
#include <linux/rtc.h>
#include <linux/suspend.h>
#include <linux/timer.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG cbm8580_power_saving
HWLOG_REGIST();

extern struct rtc_wkalrm poweroff_rtc_alarm;

static int cbm8580_sub_cmd_verify(u8 *buf, u16 buf_len, u8 *data, u16 data_len)
{
	u16 data_sub_cmd = (u16)data[CBM8580_FW_SUB_CMD_INDEX] |
		(u16)(data[CBM8580_FW_SUB_CMD_INDEX + 1] <<
		CBM8580_FW_SINGLE_BIT_LEN);
	u16 sub_cmd = (u16)buf[CBM8580_FW_SUB_CMD_INDEX] |
		(u16)(buf[CBM8580_FW_SUB_CMD_INDEX + 1] <<
		CBM8580_FW_SINGLE_BIT_LEN);

	hwlog_info("%s buf: sub_cmd = 0x%x, data: sub_cmd = 0x%x\n",
		__func__, sub_cmd, data_sub_cmd);
	if (data_sub_cmd == sub_cmd)
		return CBM8580_RET_SUCCESS;

	return CBM8580_RET_FAILED;
}

static int cbm8580_single_cmd_operation(struct bq40z50_device_info *di, u8 *buf,
	u8 buf_len, u8 *data, u8 data_len)
{
	int ret;
	int retry = CBM8580_FW_RETRY_COUNT;

	while (retry--) {
		ret = bq40z50_write_block(di, CBM8580_FW_REG, buf, buf_len);
		if (ret) {
			hwlog_err("%s retry[%d] write buf fail, ret = %d\n",
				__func__, retry, ret);
			goto cmd_failed;
		}

		ret = bq40z50_read_block(di, CBM8580_FW_REG, data, data_len);
		if (ret) {
			hwlog_err("%s retry[%d] read block ret = %d\n",
				__func__, retry, ret);
			goto cmd_failed;
		}

		if (!cbm8580_sub_cmd_verify(buf, buf_len, data, data_len))
			return CBM8580_RET_SUCCESS;

		hwlog_err("%s retry[%d] read data fail\n", __func__, retry);
cmd_failed:
		power_msleep(DT_MSLEEP_10MS, 0, NULL);
	}

	return CBM8580_RET_FAILED;
}

static int cbm8580_bat_param_only_write(struct bq40z50_device_info *di, u8 *buf,
	u8 buf_len)
{
	int ret;
	int retry = CBM8580_FW_RETRY_COUNT;

	while (retry--) {
		ret = bq40z50_write_block(di, CBM8580_FW_REG, buf, buf_len);
		hwlog_info("%s write block sub_cmd = 0x%x %x, ret = %d\n", __func__,
			buf[CBM8580_FW_SUB_CMD_INDEX + 1], buf[CBM8580_FW_SUB_CMD_INDEX], ret);
		if (!ret)
			return CBM8580_RET_SUCCESS;

		power_msleep(DT_MSLEEP_10MS, 0, NULL);
	}

	hwlog_info("%s write block sub_cmd = 0x%x %x, ret = %d\n", __func__,
			buf[CBM8580_FW_SUB_CMD_INDEX + 1], buf[CBM8580_FW_SUB_CMD_INDEX], ret);
	return CBM8580_RET_FAILED;
}

static int cbm8580_write_rtc_time2coul(struct bq40z50_device_info *di, struct rtc_time *tm)
{
	int ret;
	int idx = 0;
	u8 rtc_time[CBM8580_FW_WRITE_RTC_LEN + 1] = { 0 };
	
	rtc_time[idx++] = CBM8580_FW_WRITE_RTC_LEN;
	rtc_time[idx++] = (u8)(CBM8580_FW_SUB_CMD_WRITE_RTC & CBM8580_FW_SINGLE_BIT_MASK);
	rtc_time[idx++] = (u8)(CBM8580_FW_SUB_CMD_WRITE_RTC >> CBM8580_FW_SINGLE_BIT_LEN);
	rtc_time[idx++] = tm->tm_year;
	rtc_time[idx++] = tm->tm_mon;
	rtc_time[idx++] = tm->tm_mday;
	rtc_time[idx++] = tm->tm_hour;
	rtc_time[idx++] = tm->tm_min;
	rtc_time[idx++] = tm->tm_sec;

	ret = cbm8580_bat_param_only_write(di, rtc_time, CBM8580_FW_WRITE_RTC_LEN + 1);
	if (ret) {
		hwlog_err("write time to cbm8580 err\n");
		return ret;
	}
	hwlog_info("write time to cbm8580 succeed\n");
	return CBM8580_RET_SUCCESS;
}

static int cbm8580_read_rtc_time_from_coul(struct bq40z50_device_info *di, struct rtc_time *tm)
{
	int ret;
	u8 buf[CBM8580_FW_READ_RTC_1_LEN + 1] = { CBM8580_FW_READ_RTC_1_LEN,
		(u8)(CBM8580_FW_SUB_CMD_READ_RTC & CBM8580_FW_SINGLE_BIT_MASK),
		(u8)(CBM8580_FW_SUB_CMD_READ_RTC >> CBM8580_FW_SINGLE_BIT_LEN),
		CBM8580_FW_READ_RTC_FORMAT_HEX };
	u8 rtc_time[CBM8580_FW_READ_RTC_2_LEN + 1] = { 0 };
	int idx = CBM8580_FW_RTC_DATA_INDEX;

	ret = cbm8580_single_cmd_operation(di, buf, CBM8580_FW_READ_RTC_1_LEN + 1,
		rtc_time, CBM8580_FW_READ_RTC_2_LEN + 1);
	if (ret) {
		hwlog_err("read time from cbm8580 err\n");
		return ret;
	}

	tm->tm_year = rtc_time[idx++];
	tm->tm_mon = rtc_time[idx++];
	tm->tm_mday = rtc_time[idx++];
	tm->tm_hour = rtc_time[idx++];
	tm->tm_min = rtc_time[idx++];
	tm->tm_sec = rtc_time[idx++];

	hwlog_info("read time from cbm8580 succeed\n");
	return CBM8580_RET_SUCCESS;
}

static int cbm8580_update_shutdown_flag(struct bq40z50_device_info *di, u8 flag)
{
	int ret;
	int retry = CBM8580_FW_RETRY_COUNT;
	u16 data;
	u8 buf[CBM8580_UPDATE_SHUTDOWN_FLAG_LEN] = { CBM8580_UPDATE_SHUTDOWN_FLAG_LEN - 1,
		(u8)(CBM8580_FW_SUB_CMD_SHUTDOWN_FLAG & CBM8580_FW_SINGLE_BIT_MASK),
		(u8)(CBM8580_FW_SUB_CMD_SHUTDOWN_FLAG >> CBM8580_FW_SINGLE_BIT_LEN), flag };

	while (retry--) {
		ret = bq40z50_write_block(di, CBM8580_FW_REG, buf, CBM8580_UPDATE_SHUTDOWN_FLAG_LEN);
		if (ret) {
			hwlog_err("%s retry[%d] write buf fail, ret = %d\n", __func__, retry, ret);
			power_msleep(DT_MSLEEP_10MS, 0, NULL);
			continue;
		}

		ret = bq40z50_read_word(di, CBM8580_OP_STAT_H_REG, &data);
		hwlog_info("%s retry[%d] read block ret = %d, data = 0x%04x, flag = %u\n",
				__func__, retry, ret, data, flag);
		if (!ret && (((data >> CBM8580_OP_STAT_H_SHUTDOWN_FLAG_BIT) & 0x1) == flag))
			return CBM8580_RET_SUCCESS;

		hwlog_err("%s retry[%d] read data fail\n", __func__, retry);
		power_msleep(DT_MSLEEP_10MS, 0, NULL);
	}

	return CBM8580_RET_FAILED;
}

static void cbm8580_rtc2coul(struct rtc_time *tm)
{
	tm->tm_mon = tm->tm_mon + 1;
	tm->tm_year = tm->tm_year + CBM8580_BASE_YEAR - CBM8580_CUR_YEAR;
}

static void cbm8580_coul2rtc(struct rtc_time *tm)
{
	tm->tm_mon = tm->tm_mon - 1;
	tm->tm_year = CBM8580_CUR_YEAR + tm->tm_year - CBM8580_BASE_YEAR;
}

static int has_wakealarm(struct device *dev, const void *data)
{
	struct rtc_device *candidate = to_rtc_device(dev);

	hwlog_info("RTCX: devname is %s!\n", dev_name(dev));
	if (!candidate->ops->read_alarm) {
		hwlog_err("RTCX: read_alarm not support!\n");
		return 0;
	}
	if (!device_may_wakeup(candidate->dev.parent)) {
		hwlog_err("RTCX: can_wakeup not support!\n");
		return 0;
	}
	hwlog_info("RTCX: has_wakealarm!\n");
	return 1;
}

static struct rtc_device *cbm8580_open_rtc_device(void)
{
	struct rtc_device *rtc = NULL;
	struct device *dev;

	dev = class_find_device(rtc_class, NULL, NULL, has_wakealarm);
	if (dev) {
		rtc = rtc_class_open(dev_name(dev));
		put_device(dev);
		hwlog_info("RTCX: dev ok(name:%s)!\n", dev_name(dev));
	}
	if (!rtc) {
		hwlog_err("RTCX: rtc_class_open failed!\n");
		return NULL;
	} else {
		hwlog_info("RTCX: rtc_class_open success!\n");
		return rtc;
	}
}

static void cbm8580_coul_save_rtc_time(struct bq40z50_device_info *di)
{
	struct rtc_time tm;

	if (!rtc_read_time(di->rtc_dev, &tm)) {
		cbm8580_rtc2coul(&tm);
		(void)cbm8580_write_rtc_time2coul(di, &tm);
		(void)cbm8580_update_shutdown_flag(di, CBM8580_SHUTDOWN_FLAG_ENABLE);
	}
}

static void cbm8580_rtc_hctosys(struct rtc_device *rtc)
{
	int err;
	struct rtc_time tm;
	struct timespec64 tv64 = {
		.tv_nsec = NSEC_PER_SEC >> 1,
	};

	hwlog_err("start to hctosys sync cbm8580 time\n");
	err = rtc_read_time(rtc, &tm);
	if (err) {
		hwlog_err("hctosys: unable to read the hardware clock, ret = %d\n", err);
		return;
	}

	hwlog_info("after: rtc time: %d, %d, %d, %d, %d, %d, %d\n",
		tm.tm_sec, tm.tm_min, tm.tm_hour, tm.tm_wday, tm.tm_mday, tm.tm_mon,
		tm.tm_year);
	tv64.tv_sec = rtc_tm_to_time64(&tm);

#if BITS_PER_LONG == 32
	if (tv64.tv_sec > INT_MAX) {
		err = -ERANGE;
		return;
	}
#endif

	err = do_settimeofday64(&tv64);
	if (err)
		hwlog_err("do_settimeofday64 err, ret = %d\n", err);

	hwlog_info("end to hctosys sync cbm8580 time\n");
	return;
}

static int cbm8580_time_compare_soc(struct rtc_time *tm, struct rtc_time *soc)
{
	if ((tm->tm_year > soc->tm_year) || (tm->tm_mon > soc->tm_mon) ||
		(tm->tm_mday > soc->tm_mday) || (tm->tm_hour > soc->tm_hour) ||
		(tm->tm_min > soc->tm_min) || (tm->tm_sec > soc->tm_sec))
		return CBM8580_RET_SUCCESS;

	return CBM8580_RET_FAILED;
}

static void cbm8580_sync_time_to_soc_work(struct work_struct *work)
{
	struct rtc_time tm;
	struct rtc_time soc_tm;
	struct bq40z50_device_info *di = container_of(work,
		struct bq40z50_device_info, soc_sync_rtc.work);

	if (!di)
		return;

	hwlog_info("%s enter\n", __func__);

	di->rtc_dev = cbm8580_open_rtc_device();
	if (!di->rtc_dev) {
		queue_delayed_work(di->sync_wq, &di->soc_sync_rtc,
			msecs_to_jiffies(DT_MSLEEP_1S));
		return;
	}

	/* start shooting clear shutdown flag, work queue not affect probe process */
	(void)cbm8580_update_shutdown_flag(di, CBM8580_SHUTDOWN_FLAG_DIEABLE);

	if (cbm8580_read_rtc_time_from_coul(di, &tm))
		return;

	hwlog_info("time from cbm8580: %d, %d, %d, %d, %d, %d, %d\n",
		tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_wday, tm.tm_hour,
		tm.tm_min, tm.tm_sec);

	cbm8580_coul2rtc(&tm);
	if (rtc_read_time(di->rtc_dev, &soc_tm))
		hwlog_err("soc read rtc time err\n");

	hwlog_info("before rtc time: %d, %d, %d, %d, %d, %d, %d\n",
		soc_tm.tm_year, soc_tm.tm_mon, soc_tm.tm_mday, soc_tm.tm_wday,
		soc_tm.tm_hour, soc_tm.tm_min, soc_tm.tm_sec);
	
	hwlog_info("cbm8580 time to trans rtc format: %d, %d, %d, %d, %d, %d, %d\n",
		tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_wday, tm.tm_hour, tm.tm_min,
		tm.tm_sec);

	if (cbm8580_time_compare_soc(&tm, &soc_tm))
		return;

	if (rtc_set_time(di->rtc_dev, &tm))
		hwlog_err("cbm8580 set rtc time err\n");

	cbm8580_rtc_hctosys(di->rtc_dev);
}

void cbm8580_power_saving_restore(struct bq40z50_device_info *di)
{
	if (!di || !di->power_saving_enable)
		return;
	
	di->sync_wq = create_workqueue("cbm8580_sync_time_wq");
	INIT_DELAYED_WORK(&di->soc_sync_rtc, cbm8580_sync_time_to_soc_work);
	queue_delayed_work(di->sync_wq, &di->soc_sync_rtc,
		msecs_to_jiffies(DT_MSLEEP_1S));
	hwlog_info("%s soc_sync_rtc work start run!\n", __func__);
}

void cbm8580_power_saving_shutdown(struct bq40z50_device_info *di)
{
	if (!di || !di->power_saving_enable)
		return;
	
	if (poweroff_rtc_alarm.enabled)
		return;

	cbm8580_coul_save_rtc_time(di);
}

void cbm8580_power_saving_free_resource(struct bq40z50_device_info *di)
{
	if (!di || !di->power_saving_enable)
		return;

	hwlog_info("%s enter. di->rtc_dev = 0x%x, work = 0x%x\n", __func__,
		di->rtc_dev, &di->soc_sync_rtc);
	cancel_delayed_work(&di->soc_sync_rtc);
	if (di->rtc_dev)
		rtc_class_close(di->rtc_dev);
}