/* SPDX-License-Identifier: GPL-2.0 */
/*
 * charge_state_code.c
 *
 * status process interface for charge
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

#include <chipset_common/hwpower/charger/charge_state_code.h>
#include <linux/syscalls.h>
#include <log/log_usertype.h>
#include <securec.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_time.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_error_handle.h>

#define HWLOG_TAG charge_state_code
HWLOG_REGIST();

#define CSC_FILE_BUFF_SIZE    1024
#define CSC_BD_BUFF_SIZE      70
#define CSC_RESERVE_SIZE      4
#define CSC_CODE_BASE_MAX     1000
#define CSC_CODE_NUM_MAX      100
#define CSC_FILE_NAME_SIZE    256
#define CSC_FILE_MAX_SIZE     512000 /* 500Kb */
#define CSC_FILE_PATH         "/log/charge_monitor/csc_log"
#define CSC_MAX_VAL           100

#define CHARGE_CSC_RD_BUF_SIZE 32

struct csc_map {
	enum charge_csc_code_list cur_state;
	bool en_cbd;
	int dsm_no;
	void (*action)(char *);
};

enum charge_csc_sysfs_type {
	CHARGE_CSC_SYSFS_BEGIN = 0,
	CHARGE_CSC_BD_CODE = CHARGE_CSC_SYSFS_BEGIN,
	CHARGE_CSC_FILE_CODE,
	CHARGE_CSC_CODE_DEBUG,
};

struct charge_csc_handle_dev {
	struct device *dev;
	struct notifier_block event_nb;
	char bd_code[CSC_BD_BUFF_SIZE];
	char file_code[CSC_FILE_BUFF_SIZE];
	char dsm_code[CSC_FILE_BUFF_SIZE];
	char file_name[CSC_FILE_NAME_SIZE];
	char start_time[CSC_FILE_NAME_SIZE];
	char stop_time[CSC_FILE_NAME_SIZE];
};

static struct charge_csc_handle_dev *g_csc_dev;

/* event:charging */
static struct csc_map csc_detail_map[] = {
	{ CSC_CHARGER_INSERT, true, false, .action = NULL },
	{ CSC_BC12_RESULT, true, false, .action = NULL },
	{ CSC_BUCK_CHARGE, true, false, .action = NULL },
	{ CSC_START_CHECK, true, false, .action = NULL },
	{ CSC_MODE_MATCH, true, false, .action = NULL },
	{ CSC_MODE_CHECK, true, false, .action = NULL },
	{ CSC_FSM_STATE, true, false, .action = NULL },
	{ CSC_DC_SUCC, true, false, .action = NULL },
	{ CSC_WL_START, true, false, .action = NULL },
};

/* read and write files */
static int charge_csc_get_file_size(char *file_name)
{
	struct kstat stat;

	(void)memset_s(&stat, sizeof(stat), 0, sizeof(stat));
	vfs_stat(file_name, &stat);

	return stat.size;
}

static void charge_csc_delete_file(char *path)
{
	int ret;

	if (!path) {
		hwlog_err("path is invalid\n");
		return;
	}
	hwlog_info("unlink file name: %s\n", path);

	ret = ksys_access(path, 0); /* 0: check file exist */
	if (ret) {
		hwlog_err("path is not exist\n");
		return;
	}

	ret = ksys_unlink(path);
	if (ret) {
		hwlog_err("unlink file failed\n");
		return;
	}
	hwlog_info("unlink file ok\n");
}

static void charge_csc_check_file(void)
{
	int file_size;
	struct charge_csc_handle_dev *l_dev = g_csc_dev;

	if (!l_dev || !l_dev->file_name)
		return;

	file_size = charge_csc_get_file_size(l_dev->file_name);

	hwlog_info("cs file stat size=%u\n", file_size);
	if (file_size > CSC_FILE_MAX_SIZE) {
		charge_csc_delete_file(l_dev->file_name);
		hwlog_info("file large need to delete\n");
	}
}

static struct file *charge_csc_open_file(char *file_name)
{
	struct file *fp = NULL;

	fp = filp_open(file_name, O_WRONLY | O_APPEND, 0644);
	if (!IS_ERR(fp))
		return fp;

	hwlog_info("file %s not exist\n", file_name);
	fp = filp_open(file_name, O_WRONLY | O_CREAT, 0644);
	if (IS_ERR(fp)) {
		hwlog_info("create %s file fail\n", file_name);
		return NULL;
	}

	return fp;
}

static int charge_csc_write_data_to_file(struct file *fp, const u8 *data, u32 size)
{
	char time_end[CSC_FILE_NAME_SIZE];
	struct tm ctm;
	loff_t pos = 0;
	int ret;
	struct charge_csc_handle_dev *l_dev = g_csc_dev;

	if (!l_dev)
		return -ENODEV;

	/* write stat timestamp */
	ret = vfs_write(fp, l_dev->start_time, strlen(l_dev->start_time), &pos);
	if (ret != strlen(l_dev->start_time)) {
		hwlog_err("write start time failed ret=%d\n", ret);
		return -EFAULT;
	}

	/* write data */
	ret = vfs_write(fp, data, size, &pos);
	if (ret != size) {
		hwlog_err("write data failed ret=%d\n", ret);
		return -EFAULT;
	}

	/* write stop timestamp */
	ret = vfs_write(fp, l_dev->stop_time, strlen(l_dev->stop_time), &pos);
	if (ret != strlen(l_dev->stop_time)) {
		hwlog_err("write stop time failed ret=%d\n", ret);
		return -EFAULT;
	}

	return 0;
}

static int charge_csc_open_write_file(char *file_name, const u8 *data, u32 size)
{
	int ret;
	struct file *fp = NULL;

	fp = charge_csc_open_file(file_name);
	if (!fp) {
		hwlog_err("open %s failed\n", file_name);
		return -EEXIST;
	}

	ret = vfs_llseek(fp, 0L, SEEK_END);
	if (ret < 0) {
		hwlog_err("lseek %s failed from end\n", file_name);
		filp_close(fp, NULL);
		return -EFAULT;
	}

	ret = charge_csc_write_data_to_file(fp, data, size);
	if (ret) {
		hwlog_err("write %s failed ret=%d\n", file_name, ret);
		filp_close(fp, NULL);
		return -EFAULT;
	}

	filp_close(fp, NULL);
	hwlog_info("wtire to file succ\n");
	return 0;
}

static int charge_csc_handle_file(const u8 *data, u32 size)
{
	int ret;
	mm_segment_t fs;
	struct charge_csc_handle_dev *l_dev = g_csc_dev;

	if (!l_dev)
		return -ENODEV;;

	if (!l_dev->file_name || !data || (size == 0))
		return -EIO;

	charge_csc_check_file();

	fs = get_fs();
	set_fs(KERNEL_DS);
	ret = charge_csc_open_write_file(l_dev->file_name, data, size);
	set_fs(fs);

	return ret;
}

static void charge_csc_link_code_para(char *link_code,
	unsigned int code, char *para, int max_size)
{
	int ret;
	int curr_size;
	int remain_size;
	int para_size = 0;

	curr_size = strlen(link_code);
	if (para) {
		para_size = strlen(para);
		hwlog_info("current para=%s\n", para);
	}

	if (curr_size + para_size + CSC_RESERVE_SIZE >= max_size) {
		hwlog_err("%s: buffer overflow\n", __func__);
		return;
	}
	remain_size = max_size - (curr_size + para_size + CSC_RESERVE_SIZE);

	if (para)
		ret = snprintf_s(link_code + curr_size, remain_size,
			remain_size - 1, "#%u-%s", code, para);
	else
		ret = snprintf_s(link_code + curr_size, remain_size,
			remain_size - 1, "#%u", code);

	if (ret < 0)
		hwlog_err("%s: snprintf_s fail\n", __func__);
}

static void charge_csc_set_file_code(unsigned int code, char *para)
{
	struct charge_csc_handle_dev *l_dev = g_csc_dev;

	if (!l_dev)
		return;

	hwlog_info("set to file, code=%u\n", code);
	charge_csc_link_code_para(l_dev->file_code, code, para, CSC_FILE_BUFF_SIZE);
}

static void charge_csc_set_bg_code(unsigned int code, char *para)
{
	struct charge_csc_handle_dev *l_dev = g_csc_dev;

	if (!l_dev)
		return;

	hwlog_info("set to bigdata, code=%u\n", code);
	charge_csc_link_code_para(l_dev->bd_code, code, para, CSC_BD_BUFF_SIZE);
}

static void charge_csc_set_dsm_code(unsigned int code, char *para)
{
	struct charge_csc_handle_dev *l_dev = g_csc_dev;

	if (!l_dev)
		return;

	hwlog_info("set to dsm, code=%u\n", code);
	charge_csc_link_code_para(l_dev->dsm_code, code, para, CSC_FILE_BUFF_SIZE);
}

void charge_state_code_set(unsigned int code, char *para)
{
	unsigned int i;
	struct charge_csc_handle_dev *l_dev = g_csc_dev;

	if (!l_dev)
		return;

	if (code > CSC_CODE_BASE_MAX) {
		hwlog_err("%s: invalid para\n", __func__);
		return;
	}

	for (i = 0; i < ARRAY_SIZE(csc_detail_map); i++) {
		if (code != csc_detail_map[i].cur_state)
			continue;

		charge_csc_set_file_code(code, para);

		if (csc_detail_map[i].en_cbd)
			charge_csc_set_bg_code(code, para);

		if (csc_detail_map[i].dsm_no)
			charge_csc_set_dsm_code(code, para);

		if (csc_detail_map[i].action)
			csc_detail_map[i].action;

		break;
	}
}

static void charge_csc_save_buff_to_file(void)
{
	int ret;
	int file_len;
	unsigned int log_usertype;
	struct charge_csc_handle_dev *l_dev = g_csc_dev;

	if (!l_dev)
		return;

	log_usertype = get_logusertype_flag();
	if (log_usertype != BETA_USER) {
		hwlog_info("commercial version or oversea beta does not write file\n");
		return;
	}

	file_len = strlen(l_dev->file_code);
	if (file_len <= 0) {
		hwlog_info("no data needs to be written\n");
		return;
	}

	hwlog_info("start write file, code=%s\n", l_dev->file_code);
	ret = charge_csc_handle_file(l_dev->file_code, file_len);
	if (ret < 0)
		hwlog_info("wtire to file fail\n");
}

static void charge_csc_reset_code(void)
{
	struct charge_csc_handle_dev *l_dev = g_csc_dev;
	struct tm ctm;

	if (!l_dev)
		return;

	hwlog_info("reset charge code buff\n");
	(void)memset_s(l_dev->file_code, CSC_FILE_BUFF_SIZE, 0, CSC_FILE_BUFF_SIZE);
	(void)memset_s(l_dev->bd_code, CSC_BD_BUFF_SIZE, 0, CSC_BD_BUFF_SIZE);
	(void)memset_s(l_dev->dsm_code, CSC_FILE_BUFF_SIZE, 0, CSC_FILE_BUFF_SIZE);
}

static void charge_csc_charging_start(void)
{
	int ret;
	struct charge_csc_handle_dev *l_dev = g_csc_dev;
	struct tm ctm;

	if (!l_dev)
		return;

	charge_csc_save_buff_to_file();
	charge_csc_reset_code();

	power_get_tm_time(&ctm);
	(void)memset_s(l_dev->start_time, CSC_FILE_NAME_SIZE, 0, CSC_FILE_NAME_SIZE);
	ret = snprintf_s(l_dev->start_time, CSC_FILE_NAME_SIZE, CSC_FILE_NAME_SIZE - 1,
		"\nstart time: %u-%u-%u %u:%u:%u\n",
		ctm.tm_year + PT_BASIC_YEAR, ctm.tm_mon + 1,
		ctm.tm_mday, ctm.tm_hour, ctm.tm_min, ctm.tm_sec);
	if (ret < 0)
		hwlog_err("%s: snprintf_s fail\n", __func__);
}

static void charge_csc_charging_stop(void)
{
	int ret;
	int dsm_len;
	struct tm ctm;
	struct charge_csc_handle_dev *l_dev = g_csc_dev;

	if (!l_dev)
		return;

	power_get_tm_time(&ctm);
	(void)memset_s(l_dev->stop_time, CSC_FILE_NAME_SIZE, 0, CSC_FILE_NAME_SIZE);
	ret = snprintf_s(l_dev->stop_time, CSC_FILE_NAME_SIZE, CSC_FILE_NAME_SIZE - 1,
		"\nstop time: %u-%u-%u %u:%u:%u\n",
		ctm.tm_year + PT_BASIC_YEAR, ctm.tm_mon + 1,
		ctm.tm_mday, ctm.tm_hour, ctm.tm_min, ctm.tm_sec);
	if (ret < 0)
		hwlog_err("%s: snprintf_s fail\n", __func__);

	/* write & report */
	charge_csc_save_buff_to_file();

	dsm_len = strlen(l_dev->dsm_code);
	if (dsm_len > 0)
		power_dsm_report_dmd(POWER_DSM_BATTERY,
			POWER_DSM_DIRECT_CHARGE_ERR_WITH_STD_CABLE, l_dev->dsm_code);

	charge_csc_reset_code();
}

static int charge_csc_event_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct charge_csc_handle_dev *l_dev = g_csc_dev;

	if (!l_dev)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_CHARGING_START:
		charge_csc_charging_start();
		break;
	case POWER_NE_CHARGING_STOP:
		charge_csc_charging_stop();
		break;
	default:
		return NOTIFY_OK;
	}

	hwlog_info("%s: receive event=%lu\n", __func__, event);
	return NOTIFY_OK;
}

static void charge_csc_set_filename(void)
{
	int ret;
	struct charge_csc_handle_dev *l_dev = g_csc_dev;

	if (!l_dev)
		return;

	(void)memset_s(l_dev->file_name, sizeof(l_dev->file_name), 0, sizeof(l_dev->file_name));
	ret = snprintf_s(l_dev->file_name, CSC_FILE_NAME_SIZE, CSC_FILE_NAME_SIZE - 1,
		"%s", CSC_FILE_PATH);
	if (ret < 0)
		hwlog_err("%s: snprintf_s fail\n", __func__);
}

#ifdef CONFIG_SYSFS
static ssize_t charge_csc_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t charge_csc_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info charge_csc_sysfs_field_tbl[] = {
	power_sysfs_attr_ro(charge_csc, 0440, CHARGE_CSC_BD_CODE, bd_code),
	power_sysfs_attr_ro(charge_csc, 0440, CHARGE_CSC_FILE_CODE, file_code),
	power_sysfs_attr_rw(charge_csc, 0644, CHARGE_CSC_CODE_DEBUG, code_debug),
};

#define CHARGE_CSC_SYSFS_ATTRS_SIZE  ARRAY_SIZE(charge_csc_sysfs_field_tbl)

static struct attribute *charge_csc_sysfs_attrs[CHARGE_CSC_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group charge_csc_sysfs_attr_group = {
	.attrs = charge_csc_sysfs_attrs,
};

static ssize_t charge_csc_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct charge_csc_handle_dev *l_dev = g_csc_dev;

	if (!l_dev)
		return -ENODEV;

	info = power_sysfs_lookup_attr(attr->attr.name,
		charge_csc_sysfs_field_tbl, CHARGE_CSC_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case CHARGE_CSC_BD_CODE:
		return scnprintf(buf, CHARGE_CSC_RD_BUF_SIZE, "%s\n",
			l_dev->bd_code);
	case CHARGE_CSC_FILE_CODE:
		return scnprintf(buf, CHARGE_CSC_RD_BUF_SIZE, "%s\n",
			l_dev->file_code);
	default:
		return 0;
	}
}

static ssize_t charge_csc_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct charge_csc_handle_dev *l_dev = g_csc_dev;
	long val = 0;
	unsigned int code;
	char para[CSC_BD_BUFF_SIZE];

	if (!l_dev)
		return -ENODEV;

	info = power_sysfs_lookup_attr(attr->attr.name,
		charge_csc_sysfs_field_tbl, CHARGE_CSC_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	if (count > CSC_BD_BUFF_SIZE) {
		hwlog_err("input too long\n");
		return -EINVAL;
	}

	switch (info->name) {
	case CHARGE_CSC_CODE_DEBUG:
		/* 2: the fields of "code para" */
		if (sscanf_s(buf, "%u %s", &code, para, sizeof(para)) != 2) {
			hwlog_err("unable to parse input:%s\n", buf);
			return -EINVAL;
		}
		hwlog_info("set code debug, buf=%s, code=%u, para=%s\n", buf, code, para);
		/* if para is n, invalid para */
		if (strcmp(para, "n") == 0)
			charge_state_code_set(code, NULL);
		else
			charge_state_code_set(code, para);
		break;
	default:
		break;
	}

	return count;
}

static struct device *charge_csc_sysfs_create_group(void)
{
	power_sysfs_init_attrs(charge_csc_sysfs_attrs,
		charge_csc_sysfs_field_tbl, CHARGE_CSC_SYSFS_ATTRS_SIZE);
	return power_sysfs_create_group("hw_power", "charge_csc",
		&charge_csc_sysfs_attr_group);
}

static void charge_csc_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &charge_csc_sysfs_attr_group);
}
#else
static inline struct device *charge_csc_sysfs_create_group(void)
{
	return NULL;
}

static inline void charge_csc_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static bool charge_csc_normal_mode(void)
{
	if (power_cmdline_is_powerdown_charging_mode() ||
		power_cmdline_is_recovery_mode() ||
		power_cmdline_is_erecovery_mode()) {
		hwlog_info("current mode not support charge status trace\n");
		return false;
	}

	return true;
}

static int __init charge_csc_handle_init(void)
{
	int ret;
	struct charge_csc_handle_dev *l_dev = NULL;

	hwlog_info("charge status handle init\n");
	if (!charge_csc_normal_mode())
		return 0;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_csc_dev = l_dev;
	l_dev->dev = charge_csc_sysfs_create_group();

	l_dev->event_nb.notifier_call = charge_csc_event_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_CHARGING, &l_dev->event_nb);
	if (ret < 0) {
		hwlog_err("charge_csc register notifier call fail\n");
		goto csc_bnc_reg_fail;
	}

	charge_csc_set_filename();
	charge_csc_reset_code();
	(void)memset_s(l_dev->stop_time, CSC_FILE_NAME_SIZE, 0, CSC_FILE_NAME_SIZE);
	(void)memset_s(l_dev->start_time, CSC_FILE_NAME_SIZE, 0, CSC_FILE_NAME_SIZE);
	return 0;

csc_bnc_reg_fail:
	kfree(l_dev);
	l_dev = NULL;
	g_csc_dev = NULL;
	return ret;
}

static void __exit charge_csc_handle_exit(void)
{
	struct charge_csc_handle_dev *l_dev = g_csc_dev;

	if (!l_dev)
		return;

	charge_csc_sysfs_remove_group(l_dev->dev);
	power_event_bnc_unregister(POWER_BNT_CHARGING, &l_dev->event_nb);
	kfree(l_dev);
	l_dev = NULL;
	g_csc_dev = NULL;
}

subsys_initcall_sync(charge_csc_handle_init);
module_exit(charge_csc_handle_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("charge state code module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
