/*
 * lcd_kit_sysfs_fac.c
 *
 * lcdkit factory sysfs function for lcd driver
 *
 * Copyright (c) 2022-2024 Huawei Technologies Co., Ltd.
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
#include "lcd_kit_common.h"
#include "lcd_kit_factory.h"
#include "lcd_kit_utils.h"
#include "lcd_kit_disp.h"
#include "lcd_kit_adapt.h"
#include "lcd_kit_sysfs_fac.h"
#include<linux/jiffies.h>
#include <securec.h>
#include "lcd_kit_power.h"

static ssize_t lcd_inversion_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int panel_id = lcd_kit_get_active_panel_id();

	return lcd_kit_inversion_get_mode(panel_id, buf);
}

static ssize_t lcd_inversion_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long val;
	int ret;
	int panel_id = lcd_kit_get_active_panel_id();

	if (!buf) {
		LCD_KIT_ERR("lcd inversion mode store  buf is NULL\n");
		return LCD_KIT_FAIL;
	}

	val = simple_strtoul(buf, NULL, 0);
	if (lcd_kit_get_panel_on_state(panel_id)) {
		ret = lcd_kit_inversion_set_mode(panel_id, NULL, val);
		if (ret)
			LCD_KIT_ERR("inversion mode set failed\n");
	}

	return count;
}

static ssize_t lcd_scan_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int panel_id = lcd_kit_get_active_panel_id();

	return lcd_kit_scan_get_mode(panel_id, buf);
}

static ssize_t lcd_scan_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long val;
	int ret;
	int panel_id = lcd_kit_get_active_panel_id();

	if (!buf) {
		LCD_KIT_ERR("lcd scan mode store buf is NULL\n");
		return LCD_KIT_FAIL;
	}

	val = simple_strtoul(buf, NULL, 0);
	if (lcd_kit_get_panel_on_state(panel_id)) {
		ret = lcd_kit_scan_set_mode(panel_id, NULL, val);
		if (ret)
			LCD_KIT_ERR("scan mode set failed\n");
	}

	return count;
}

static ssize_t lcd_check_reg_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;
	int panel_id = lcd_kit_get_active_panel_id();

	if (lcd_kit_get_panel_on_state(panel_id))
		ret = lcd_kit_check_reg(panel_id, NULL, buf);

	return ret;
}

static int lcd_checksum_compare(unsigned char *read_value, unsigned int *value,
	int len)
{
	int i;
	int err_no = 0;
	unsigned char tmp;

	for (i = 0; i < len; i++) {
		tmp = (unsigned char)value[i];
		if (read_value[i] != tmp) {
			LCD_KIT_ERR("gram check error\n");
			LCD_KIT_ERR("read_value[%d]:0x%x\n", i, read_value[i]);
			LCD_KIT_ERR("expect_value[%d]:0x%x\n", i, tmp);
			err_no++;
		}
	}
	return err_no;
}

static int lcd_dual_dsi_check_checksum(int panel_id)
{
	int ret;
	int err_cnt;
	int check_cnt;
	unsigned int pic_index;
	unsigned int checksum_size = FACT_INFO->checksum.read_value_num;
	unsigned char read_value0[LCD_KIT_CHECKSUM_SIZE_DKMD + 1] = { 0 };
	unsigned char read_value1[LCD_KIT_CHECKSUM_SIZE_DKMD + 1] = { 0 };
	if (FACT_INFO->checksum.value.arry_data == NULL ||
		FACT_INFO->checksum.sub_value.arry_data == NULL) {
		LCD_KIT_ERR("null pointer\n");
		return LCD_KIT_FAIL;
	}
	pic_index = FACT_INFO->checksum.pic_index;
	ret = lcd_kit_dual_cmds_rx(panel_id, NULL, read_value0, read_value1,
		checksum_size, &FACT_INFO->checksum.checksum_cmds);
	if (ret) {
		LCD_KIT_ERR("checksum read error\n");
		return LCD_KIT_FAIL;
	}
	check_cnt = FACT_INFO->checksum.value.arry_data[pic_index].cnt;
	if (check_cnt > checksum_size) {
		LCD_KIT_ERR("checksum count is larger than checksum size\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("dsi0 checksum val\n");
	err_cnt = lcd_checksum_compare(read_value0,
		FACT_INFO->checksum.value.arry_data[pic_index].buf, check_cnt);
	LCD_KIT_INFO("dsi1 checksum val\n");
	err_cnt += lcd_checksum_compare(read_value1,
		FACT_INFO->checksum.sub_value.arry_data[pic_index].buf, check_cnt);
	if (err_cnt) {
		LCD_KIT_ERR("err_cnt:%d\n", err_cnt);
		ret = LCD_KIT_FAIL;
	}
	return ret;
}

static int lcd_check_checksum(int panel_id)
{
	int ret;
	int err_cnt;
	int check_cnt;
	unsigned int *checksum = NULL;
	unsigned int pic_index;
	unsigned int checksum_size = FACT_INFO->checksum.read_value_num;
	unsigned char read_value[LCD_KIT_CHECKSUM_SIZE_DKMD + 1] = {0};

	if (FACT_INFO->checksum.value.arry_data == NULL) {
		LCD_KIT_ERR("null pointer\n");
		return LCD_KIT_FAIL;
	}
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return LCD_KIT_FAIL;
	}
	pic_index = FACT_INFO->checksum.pic_index;
	if (FACT_INFO->checksum.dual_dsi_support)
		return lcd_dual_dsi_check_checksum(panel_id);
	ret = lcd_kit_cmds_rx(panel_id, NULL, read_value, checksum_size,
		&FACT_INFO->checksum.checksum_cmds);
	if (ret) {
		LCD_KIT_ERR("checksum read error\n");
		return LCD_KIT_FAIL;
	}

	check_cnt = FACT_INFO->checksum.value.arry_data[pic_index].cnt;
	if (check_cnt > checksum_size) {
		LCD_KIT_ERR("checksum count is larger than checksum size\n");
		return LCD_KIT_FAIL;
	}

	checksum = FACT_INFO->checksum.value.arry_data[pic_index].buf;
	err_cnt = lcd_checksum_compare(read_value, checksum, check_cnt);
	if (err_cnt) {
		LCD_KIT_ERR("err_cnt:%d\n", err_cnt);
		ret = LCD_KIT_FAIL;
	}
	return ret;
}

static int lcd_kit_checksum_check(int panel_id)
{
	int ret;
	unsigned int pic_index;

	pic_index = FACT_INFO->checksum.pic_index;
	LCD_KIT_INFO("checksum pic num:%d\n", pic_index);
	if (pic_index > TEST_PIC_2) {
		LCD_KIT_ERR("checksum pic num unknown:%d\n", pic_index);
		return LCD_KIT_FAIL;
	}
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return LCD_KIT_FAIL;
	}
	ret = lcd_check_checksum(panel_id);
	if (ret)
		LCD_KIT_ERR("checksum error\n");

	if (ret && FACT_INFO->checksum.pic_index == TEST_PIC_2)
		FACT_INFO->checksum.status = LCD_KIT_CHECKSUM_END;

	if (FACT_INFO->checksum.status == LCD_KIT_CHECKSUM_END) {
		lcd_kit_cmds_tx(panel_id, NULL, &FACT_INFO->checksum.disable_cmds);
		LCD_KIT_INFO("gram checksum end, disable checksum\n");
		/* enable esd */
		lcd_esd_enable(panel_id, ENABLE);
	}

	return ret;
}

static ssize_t lcd_gram_check_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;
	int checksum_result;
	int panel_id = lcd_kit_get_active_panel_id();

	if (FACT_INFO->checksum.support) {
		if (lcd_kit_get_panel_on_state(panel_id)) {
			checksum_result = lcd_kit_checksum_check(panel_id);
			ret = snprintf(buf, PAGE_SIZE, "%d\n", checksum_result);
		}

		if (FACT_INFO->checksum.status == LCD_KIT_CHECKSUM_END) {
			if (FACT_INFO->checksum.clear_sram) {
				lcd_kit_recovery_display(panel_id);
				LCD_KIT_INFO("checksum recovery display\n");
			}
		}
	}

	return ret;
}

static void lcd_enable_checksum(int panel_id)
{
	/* disable esd */
	lcd_esd_enable(panel_id, DISABLE);
	FACT_INFO->checksum.status = LCD_KIT_CHECKSUM_START;
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return;
	}
	if (!FACT_INFO->checksum.special_support) {
		if (FACT_INFO->checksum.clear_sram) {
			LCD_KIT_INFO("before checksum, clear sram\n");
			lcd_kit_cmds_tx(panel_id, NULL, &FACT_INFO->checksum.clear_sram_cmds);
		} else {
			lcd_kit_cmds_tx(panel_id, NULL, &FACT_INFO->checksum.enable_cmds);
		}
	}
	FACT_INFO->checksum.pic_index = INVALID_INDEX;
	LCD_KIT_INFO("Enable checksum\n");
}

static int lcd_kit_checksum_set(int panel_id, int pic_index)
{
	int ret = LCD_KIT_OK;

	if (FACT_INFO->checksum.status == LCD_KIT_CHECKSUM_END) {
		if (pic_index == TEST_PIC_0) {
			LCD_KIT_INFO("start gram checksum\n");
			lcd_enable_checksum(panel_id);
			return ret;
		}
		LCD_KIT_INFO("pic index error\n");
		return LCD_KIT_FAIL;
	}
	if (pic_index == TEST_PIC_2)
		FACT_INFO->checksum.check_count++;
	if (FACT_INFO->checksum.check_count == CHECKSUM_CHECKCOUNT) {
		LCD_KIT_INFO("check 5 times, set checksum end\n");
		FACT_INFO->checksum.status = LCD_KIT_CHECKSUM_END;
		FACT_INFO->checksum.check_count = 0;
	}
	switch (pic_index) {
	case TEST_PIC_0:
	case TEST_PIC_1:
	case TEST_PIC_2:
		if (FACT_INFO->checksum.clear_sram && lcd_kit_get_panel_on_state(panel_id))
			lcd_kit_cmds_tx(panel_id, NULL, &FACT_INFO->checksum.enable_cmds);
		if (FACT_INFO->checksum.special_support && lcd_kit_get_panel_on_state(panel_id))
			lcd_kit_cmds_tx(panel_id, NULL, &FACT_INFO->checksum.enable_cmds);
		LCD_KIT_INFO("set pic [%d]\n", pic_index);
		FACT_INFO->checksum.pic_index = pic_index;
		break;
	default:
		LCD_KIT_INFO("set pic [%d] unknown\n", pic_index);
		FACT_INFO->checksum.pic_index = INVALID_INDEX;
		break;
	}
	return ret;
}

static ssize_t lcd_gram_check_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	unsigned long val = 0;
	int index;
	int panel_id = lcd_kit_get_active_panel_id();

	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}

	ret = kstrtoul(buf, 10, &val);
	if (ret)
		return ret;

	LCD_KIT_INFO("val = %ld\n", val);
	if (FACT_INFO->checksum.support) {
		if (lcd_kit_get_panel_on_state(panel_id)) {
			index = val - INDEX_START;
			ret = lcd_kit_checksum_set(panel_id, index);
		}
	}
	return count;
}

static ssize_t lcd_sleep_ctrl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int panel_id = lcd_kit_get_active_panel_id();

	return lcd_kit_get_sleep_mode(panel_id, buf);
}

static ssize_t lcd_sleep_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	unsigned long val = 0;
	int panel_id = lcd_kit_get_active_panel_id();

	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	ret = kstrtoul(buf, 0, &val);
	if (ret) {
		LCD_KIT_ERR("invalid parameter!\n");
		return ret;
	}

	if (lcd_kit_get_panel_on_state(panel_id)) {
		ret = lcd_kit_set_sleep_mode(panel_id, val);
	} else {
		LCD_KIT_ERR("panel is power off!\n");
		return LCD_KIT_FAIL;
	}

	return count;
}

static ssize_t lcd_poweric_det_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret;

	ret = snprintf(buf, PAGE_SIZE, "OK\n");

	return ret;
}

static ssize_t lcd_poweric_det_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	return count;
}

static ssize_t lcd_amoled_cmds_pcd_errflag(char *buf)
{
	int ret = LCD_KIT_OK;
	int check_result;
	int panel_id = lcd_kit_get_active_panel_id();

	if (buf == NULL) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}

	if (lcd_kit_get_panel_on_state(panel_id)) {
		check_result = lcd_kit_check_pcd_errflag_check();
		ret = snprintf(buf, PAGE_SIZE, "%d\n", check_result);
		LCD_KIT_INFO("pcd_errflag, the check_result = %d\n",
			check_result);
	}
	return ret;
}

static ssize_t lcd_amoled_gpio_pcd_errflag(char *buf)
{
	int ret;

	if (buf == NULL) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}

	ret = lcd_kit_gpio_pcd_errflag_check();
	LCD_KIT_INFO("pcd_errflag result_value = %d\n", ret);
	ret = snprintf(buf, PAGE_SIZE, "%d\n", ret);
	return ret;
}

static ssize_t lcd_amoled_pcd_errflag_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;
	int panel_id = lcd_kit_get_active_panel_id();

	if (buf == NULL) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}

	if (disp_info->pcd_errflag.pcd_support ||
		disp_info->pcd_errflag.errflag_support)
		ret = lcd_amoled_cmds_pcd_errflag(buf);
	else
		ret = lcd_amoled_gpio_pcd_errflag(buf);
	return ret;
}

int lcd_kit_pcd_detect(int panel_id, unsigned int val)
{
	int ret = LCD_KIT_OK;
	static unsigned int pcd_mode;

	if (!FACT_INFO->pcd_errflag_check.pcd_errflag_check_support) {
		LCD_KIT_ERR("pcd errflag not support!\n");
		return LCD_KIT_OK;
	}

	if (pcd_mode == val) {
		LCD_KIT_ERR("pcd detect already\n");
		return LCD_KIT_OK;
	}

	pcd_mode = val;
	if (pcd_mode == LCD_KIT_PCD_DETECT_OPEN && lcd_kit_get_panel_on_state(panel_id))
		ret = lcd_kit_cmds_tx(panel_id, NULL,
			&FACT_INFO->pcd_errflag_check.pcd_detect_open_cmds);
	else if (pcd_mode == LCD_KIT_PCD_DETECT_CLOSE && lcd_kit_get_panel_on_state(panel_id))
		ret = lcd_kit_cmds_tx(panel_id, NULL,
			&FACT_INFO->pcd_errflag_check.pcd_detect_close_cmds);

	LCD_KIT_INFO("pcd_mode %d, ret %d\n", pcd_mode, ret);
	return ret;
}

static ssize_t lcd_amoled_pcd_errflag_store(struct device *dev,
	struct device_attribute *attr, const char *buf)
{
	int ret;
	unsigned long val = 0;
	int panel_id = lcd_kit_get_active_panel_id();

	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	ret = kstrtoul(buf, 0, &val);
	if (ret) {
		LCD_KIT_ERR("invalid parameter\n");
		return ret;
	}
	if (lcd_kit_get_panel_on_state(panel_id)) {
		return lcd_kit_pcd_detect(panel_id, val);
	} else {
		LCD_KIT_ERR("panel is power off\n");
		return LCD_KIT_FAIL;
	}
}

static int lcd_kit_current_det(int panel_id)
{
	ssize_t current_check_result;
	unsigned char read = 0;

	if (!FACT_INFO->current_det.support) {
		LCD_KIT_INFO("current detect is not support! return!\n");
		return LCD_KIT_OK;
	}
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return LCD_KIT_FAIL;
	}

	lcd_kit_cmds_rx(panel_id, NULL, &read, 1, &FACT_INFO->current_det.detect_cmds);
	LCD_KIT_INFO("current detect, read value = 0x%x\n", read);
	/* buf[0] means current detect result */
	if ((read & FACT_INFO->current_det.value.buf[0]) == LCD_KIT_OK) {
		current_check_result = LCD_KIT_OK;
		LCD_KIT_ERR("no current over\n");
	} else {
		current_check_result = LCD_KIT_FAIL;
		LCD_KIT_ERR("current over:0x%x\n", read);
	}

	return current_check_result;
}

static int lcd_kit_lv_det(int panel_id)
{
	ssize_t lv_check_result;
	unsigned char read = 0;

	if (!FACT_INFO->lv_det.support) {
		LCD_KIT_INFO("current detect is not support! return!\n");
		return LCD_KIT_OK;
	}
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return LCD_KIT_FAIL;
	}

	lcd_kit_cmds_rx(panel_id, NULL, &read, 1, &FACT_INFO->lv_det.detect_cmds);

	LCD_KIT_INFO("current detect, read value = 0x%x\n", read);
	if ((read & FACT_INFO->lv_det.value.buf[0]) == LCD_KIT_OK) {
		lv_check_result = LCD_KIT_OK;
		LCD_KIT_ERR("no current over\n");
	} else {
		lv_check_result = LCD_KIT_FAIL;
		LCD_KIT_ERR("current over:0x%x\n", read);
	}

	return lv_check_result;
}

static ssize_t lcd_test_config_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	int panel_id = lcd_kit_get_active_panel_id();

	ret = lcd_kit_get_test_config(panel_id, buf);
	if (ret < 0) {
		LCD_KIT_ERR("not find test item\n");
		return ret;
	}

	if (lcd_kit_get_panel_on_state(panel_id)) {
		if (buf) {
			if (!strncmp(buf, "OVER_CURRENT_DETECTION", strlen("OVER_CURRENT_DETECTION"))) {
				ret = lcd_kit_current_det(panel_id);
				return snprintf(buf, PAGE_SIZE, "%d", ret);
			}
			if (!strncmp(buf, "OVER_VOLTAGE_DETECTION", strlen("OVER_VOLTAGE_DETECTION"))) {
				ret = lcd_kit_lv_det(panel_id);
				return snprintf(buf, PAGE_SIZE, "%d", ret);
			}
		}
	}

	return ret;
}

static ssize_t lcd_test_config_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int panel_id = lcd_kit_get_active_panel_id();

	if (lcd_kit_set_test_config(panel_id, buf) < 0)
		LCD_KIT_ERR("set_test_config failed\n");
	return count;
}

static ssize_t lcd_lv_detect_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	int panel_id = lcd_kit_get_active_panel_id();

	ret = lcd_kit_lv_det(panel_id);
	return snprintf(buf, PAGE_SIZE, "%d", ret);
}

static ssize_t lcd_current_detect_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	int panel_id = lcd_kit_get_active_panel_id();

	ret = lcd_kit_current_det(panel_id);
	return snprintf(buf, PAGE_SIZE, "%d", ret);
}

static ssize_t lcd_ldo_check_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	int len = sizeof(struct lcd_ldo);
	int panel_id = lcd_kit_get_active_panel_id();

	if (!FACT_INFO->ldo_check.support) {
		LCD_KIT_INFO("ldo check not support\n");
		return len;
	}

	return ret;
}

static int lcd_hor_line_test(int panel_id)
{
	int ret = LCD_KIT_OK;
	int count = HOR_LINE_TEST_TIMES;

	LCD_KIT_INFO("horizontal line test start\n");
	LCD_KIT_INFO("FACT_INFO->hor_line.duration = %d\n",
			FACT_INFO->hor_line.duration);
	/* disable esd */
	lcd_esd_enable(panel_id, DISABLE);
	/* disable elvdd detect */
	disp_info->elvdd_detect.support = 0;
	while (count--) {
		mdelay(30);
		/* test avdd */
		if (!lcd_kit_get_panel_on_state(panel_id)) {
			LCD_KIT_ERR("panel is power off\n");
			return LCD_KIT_FAIL;
		}

		if (FACT_INFO->hor_line.hl_cmds.cmds != NULL) {
			ret = lcd_kit_cmds_tx(panel_id, NULL, &FACT_INFO->hor_line.hl_cmds);
			if (ret)
				LCD_KIT_ERR("send avdd cmd error\n");
		}
		msleep(FACT_INFO->hor_line.duration * MILLISEC_TIME);
		// recovery display
		lcd_kit_recovery_display(panel_id);
	}
	/* enable esd */
	lcd_esd_enable(panel_id, ENABLE);
	LCD_KIT_INFO("horizontal line test end\n");
	return ret;
}

static ssize_t lcd_general_test_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;
	int panel_id = lcd_kit_get_active_panel_id();

	if (FACT_INFO->hor_line.support)
		ret = lcd_hor_line_test(panel_id);

	if (ret == 0)
		ret = snprintf(buf, PAGE_SIZE, "OK\n");
	else
		ret = snprintf(buf, PAGE_SIZE, "FAIL\n");

	return ret;
}

static ssize_t lcd_vertical_line_test_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	int panel_id = lcd_kit_get_active_panel_id();

	if (!FACT_INFO->vtc_line.support) {
		ret = snprintf(buf, PAGE_SIZE, "NOT_SUPPORT\n");
		return ret;
	}
	ret = snprintf(buf, PAGE_SIZE, "OK\n");
	return ret;
}

static int lcd_vtc_line_test(int panel_id, unsigned long pic_index)
{
	int ret = LCD_KIT_OK;

	switch (pic_index) {
	case PIC1_INDEX:
		/* disable esd */
		lcd_esd_enable(panel_id, DISABLE);
		/* disable elvdd detect */
		disp_info->elvdd_detect.support = 0;
		mdelay(20);
		if (FACT_INFO->vtc_line.vtc_cmds.cmds != NULL && lcd_kit_get_panel_on_state(panel_id)) {
			ret = lcd_kit_cmds_tx(panel_id, NULL, &FACT_INFO->vtc_line.vtc_cmds);
			if (ret)
				LCD_KIT_ERR("send vtc cmd error\n");
		}
		break;
	case PIC2_INDEX:
	case PIC3_INDEX:
	case PIC4_INDEX:
		LCD_KIT_INFO("picture:%lu display\n", pic_index);
		break;
	case PIC5_INDEX:
		// recovery
		lcd_kit_recovery_display(panel_id);
		/* enable esd */
		lcd_esd_enable(panel_id, ENABLE);
		break;
	default:
		LCD_KIT_ERR("pic number not support\n");
		break;
	}
	return ret;
}

static ssize_t lcd_vertical_line_test_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	unsigned long index = 0;
	int panel_id = lcd_kit_get_active_panel_id();

	ret = kstrtoul(buf, 10, &index);
	if (ret) {
		LCD_KIT_ERR("kstrtoul fail!\n");
		return ret;
	}
	LCD_KIT_INFO("index=%ld\n", index);
	if (FACT_INFO->vtc_line.support) {
		ret = lcd_vtc_line_test(panel_id, index);
		if (ret)
			LCD_KIT_ERR("vtc line test fail\n");
	}
	return count;
}

static ssize_t lcd_fps_cmd_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	char str[LCD_REG_LENGTH_MAX] = {0};
	int panel_id = lcd_kit_get_active_panel_id();

	if (disp_info->fps.support)
		ret = snprintf(str, sizeof(str),
			"FPS60:%s;FPS90:%s;FPS1:%s;FPS45:%s;FPS40:%s;FPS30:%s;FPS24:%s;FPS20:%s;FPS18:%s;FPS15:%s;FPS10:%s;FPS120:%s;FPS144:%s;FPS48:%s;FPS72:%s;",
			disp_info->fps.fps_60_cmd, disp_info->fps.fps_90_cmd,
			disp_info->fps.fps_1_cmd, disp_info->fps.fps_45_cmd,
			disp_info->fps.fps_40_cmd, disp_info->fps.fps_30_cmd,
			disp_info->fps.fps_24_cmd, disp_info->fps.fps_20_cmd,
			disp_info->fps.fps_18_cmd, disp_info->fps.fps_15_cmd,
			disp_info->fps.fps_10_cmd, disp_info->fps.fps_120_cmd,
			disp_info->fps.fps_144_cmd, disp_info->fps.fps_48_cmd,
			disp_info->fps.fps_72_cmd);
	ret = snprintf(buf, PAGE_SIZE, "%s\n", str);
	LCD_KIT_INFO("%s\n", str);
	return ret;
}

static int fps_cmd_send(int panel_id, struct lcd_kit_dsi_panel_cmds *cmds)
{
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (adapt_ops->mipi_tx == NULL) {
		LCD_KIT_ERR("mipi_tx is NULL\n");
		return LCD_KIT_FAIL;
	}

	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return LCD_KIT_FAIL;
	}

	return adapt_ops->mipi_tx(panel_id, NULL, cmds);
}


static void send_fps_lock_cmd(int panel_id, unsigned int val)
{
	int ret = LCD_KIT_OK;
	int table_num;
	int i;
	struct fps_cmd_table fps_lock_cmd[] = {
		{ FPS_1, &disp_info->fps.fps_to_1_cmds },
		{ FPS_10, &disp_info->fps.fps_to_10_cmds },
		{ FPS_15, &disp_info->fps.fps_to_15_cmds },
		{ FPS_18, &disp_info->fps.fps_to_18_cmds },
		{ FPS_20, &disp_info->fps.fps_to_20_cmds },
		{ FPS_24, &disp_info->fps.fps_to_24_cmds },
		{ FPS_30, &disp_info->fps.fps_to_30_cmds },
		{ FPS_40, &disp_info->fps.fps_to_40_cmds },
		{ FPS_45, &disp_info->fps.fps_to_45_cmds },
		{ FPS_48, &disp_info->fps.fps_to_48_cmds },
		{ FPS_60, &disp_info->fps.fps_to_60_cmds },
		{ FPS_72, &disp_info->fps.fps_to_72_cmds },
		{ FPS_90, &disp_info->fps.fps_to_90_cmds },
		{ FPS_120, &disp_info->fps.fps_to_120_cmds },
		{ FPS_144, &disp_info->fps.fps_to_144_cmds },
	};

	table_num = sizeof(fps_lock_cmd) / sizeof(fps_lock_cmd[0]);
	for (i = 0; i < table_num; i++) {
		if (val == fps_lock_cmd[i].val)
			ret = fps_cmd_send(panel_id, fps_lock_cmd[i].cmds);
	}
	disp_info->fps.current_fps = val;
	LCD_KIT_INFO("ret = %d", ret);
}

static ssize_t lcd_fps_cmd_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	int panel_id = lcd_kit_get_active_panel_id();
	unsigned int current_fps = 0;

	if (!buf) {
		LCD_KIT_ERR("buf is null!\n");
		return LCD_KIT_FAIL;
	}
	if (strlen(buf) >= MAX_BUF) {
		LCD_KIT_ERR("buf overflow!\n");
		return LCD_KIT_FAIL;
	}
	ret = kstrtouint(buf, 10, &current_fps);
	if (ret) {
		LCD_KIT_ERR("invalid parameter!\n");
		return ret;
	}
	LCD_KIT_INFO("val is %u\n", current_fps);
	send_fps_lock_cmd(panel_id, current_fps);
	return count;
}

static ssize_t lcd_standby_current_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	int panel_id = lcd_kit_get_active_panel_id();
	struct regulator *iovcc;
	struct regulator *vci;
	int iovcc_out_current = 0;
	int vci_out_current = 0;

	if (FACT_INFO->standby_current_det.support) {
		LCD_KIT_INFO("print iovdd %s, print vci %s\n",
			FACT_INFO->standby_current_det.iovcc_id,
			FACT_INFO->standby_current_det.vci_id);
		iovcc = regulator_get(dev,
			FACT_INFO->standby_current_det.iovcc_id);
		vci = regulator_get(dev,
			FACT_INFO->standby_current_det.vci_id);
		if (IS_ERR(iovcc) || IS_ERR(vci)) {
			LCD_KIT_ERR("iovdd or vci is error!!\n");
			return ret;
		}
		iovcc_out_current = regulator_get_current_limit(iovcc);
		vci_out_current = regulator_get_current_limit(vci);
		ret = snprintf(buf, PAGE_SIZE, "iovcc:%d, vci:%d\n",
			iovcc_out_current, vci_out_current);
	} else {
		ret = snprintf(buf, PAGE_SIZE, "NOT_SUPPORT\n");
	}

	return ret;
}

static ssize_t lcd_dynamic_volt_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	struct regulator *vdd;
	int panel_id = lcd_kit_get_active_panel_id();
	int vdd_out_voltage = 0;

	if (FACT_INFO->dynamic_volt.support) {
		vdd_out_voltage = lcd_kit_dynamic_get_vdd(panel_id);
		LCD_KIT_INFO("vdd_out_voltage is %u\n", vdd_out_voltage);
		ret = snprintf(buf, PAGE_SIZE, "vdd:%u\n", vdd_out_voltage);
	} else {
		ret = snprintf(buf, PAGE_SIZE, "NOT_SUPPORT\n");
	}

	return ret;
}

static ssize_t lcd_dynamic_volt_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = LCD_KIT_OK;
	unsigned int volt_index = 0;
	int panel_id = lcd_kit_get_active_panel_id();
	struct regulator *vdd;

	LCD_KIT_INFO("enter\n");
	if (!FACT_INFO->dynamic_volt.support) {
		LCD_KIT_ERR("dynamic voltage not support\n");
		return LCD_KIT_FAIL;
	}
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is power off\n");
		return LCD_KIT_FAIL;
	}
	if (!dev || !buf) {
		LCD_KIT_ERR("dev or buf is null\n");
		return LCD_KIT_FAIL;
	}
	/* decimal  parsing */
	ret = kstrtouint(buf, 10, &volt_index);
	if (ret) {
		LCD_KIT_ERR("str to int fail\n");
		return ret;
	}

	LCD_KIT_INFO("volt_index:%d\n", volt_index);
	switch (volt_index) {
	case LDO_VDD_VOLT_HIGHT:
		LCD_KIT_INFO("FACT_INFO->dynamic_volt.vdd_vol[HIGHT]:%u\n", FACT_INFO->dynamic_volt.vdd_vol[VOLT_HIGHT]);
		lcd_kit_dynamic_ctrl_vdd(panel_id, VOLT_HIGHT);
		break;
	case LDO_VDD_VOLT_LOW:
		LCD_KIT_INFO("FACT_INFO->dynamic_volt.vdd_vol[LOW]:%u\n", FACT_INFO->dynamic_volt.vdd_vol[VOLT_LOW]);
		lcd_kit_dynamic_ctrl_vdd(panel_id, VOLT_LOW);
		break;
	case LDO_VDD_VOLT_NOMAL:
		LCD_KIT_INFO("FACT_INFO->dynamic_volt.vdd_vol[NOMAL]:%u\n", FACT_INFO->dynamic_volt.vdd_vol[VOLT_NOMAL]);
		lcd_kit_dynamic_ctrl_vdd(panel_id, VOLT_NOMAL);
		break;

	default:
		LCD_KIT_INFO("not support volt_index:%d\n", volt_index);
		break;
	}

	return ret;
}

static int check_read_mipi_detect_reg_val(uint32_t panel_id, unsigned char *read_value,
	unsigned pic_num, int loop_num)
{
	int i;
	unsigned char except_value;
	unsigned int *detect_value = NULL;
	struct lcd_kit_array_data config_data;

	config_data = FACT_INFO->mipi_detect.value[pic_num];
	detect_value = config_data.buf;
	for (i = 0; i < config_data.cnt; i++) {
		except_value = detect_value[i] & 0xFF;
		LCD_KIT_INFO("pic: %u, read_value%d = 0x%x, except_value = 0x%x\n",
			pic_num, i, read_value[i], except_value);
		if (read_value[i] >= except_value) {
			FACT_INFO->mipi_detect.err_flag[pic_num]++;
			LCD_KIT_ERR("[ERROR]pic:%u, read_val:%d = 0x%x, but expect_val = 0x%x\n",
				pic_num, i, read_value[i], except_value);
			return LCD_KIT_FAIL;
		}
	}
	return LCD_KIT_OK;
}

static int lcd_kit_dual_mipi_detect(uint32_t panel_id, unsigned int pic_num, int loop_num)
{
	int ret;
	unsigned char read_value0[MIPI_READ_COUNT] = { 0 };
	unsigned char read_value1[MIPI_READ_COUNT] = { 0 };
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	LCD_KIT_INFO("enter\n");
	if (!FACT_INFO->mipi_detect.rd_cmds[pic_num].cmds) {
		LCD_KIT_ERR("mipi detect read cmd is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return LCD_KIT_FAIL;
	}
	ret = lcd_kit_dual_cmds_rx(panel_id, NULL, read_value0, read_value1,
		MIPI_READ_COUNT - 1, &FACT_INFO->mipi_detect.rd_cmds[pic_num]);
	if (ret) {
		LCD_KIT_ERR("mipi dual detect read error\n");
		return ret;
	}
	ret = check_read_mipi_detect_reg_val(panel_id, read_value0,
		pic_num, loop_num);
	if (ret) {
		LCD_KIT_ERR("mipi detect dsi0 read error\n");
		return ret;
	}
	ret = check_read_mipi_detect_reg_val(panel_id, read_value1,
		pic_num, loop_num);
	if (ret) {
		LCD_KIT_ERR("mipi detect dsi1 read error\n");
		return ret;
	}

	return ret;
}

static int lcd_judge_mipi_detect(uint32_t panel_id, unsigned int pic_num, int loop_num)
{
	int ret;
	unsigned char read_value[MIPI_READ_COUNT] = { 0 };
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	LCD_KIT_INFO("enter\n");
	if (!FACT_INFO->mipi_detect.rd_cmds[pic_num].cmds) {
		LCD_KIT_INFO("mipi detect read cmd is NULL\n");
		return LCD_KIT_FAIL;
	}
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops\n");
		return LCD_KIT_FAIL;
	}
	if (adapt_ops->mipi_rx == NULL) {
		LCD_KIT_ERR("mipi_rx is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return LCD_KIT_FAIL;
	}
	if (FACT_INFO->mipi_detect.dual_check) {
		ret = lcd_kit_dual_mipi_detect(panel_id, pic_num, loop_num);
		if (ret) {
			LCD_KIT_ERR("dual mipi detect read error\n");
			return ret;
		}
	} else {
		ret = adapt_ops->mipi_rx(panel_id, NULL, read_value,
			MIPI_READ_COUNT - 1, &FACT_INFO->mipi_detect.rd_cmds[pic_num]);
		if (ret) {
			LCD_KIT_ERR("mipi detect read error\n");
			return ret;
		}
		ret = check_read_mipi_detect_reg_val(panel_id, read_value, pic_num, loop_num);
	}

	return ret;
}

static void lcd_kit_modify_mipi_error_nv(int panel_id)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	char read_nve_value[LCD_NV_DATA_SIZE] = { 0 };

	LCD_KIT_INFO("+\n");
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return;
	}
	if (!adapt_ops->read_nv_info) {
		LCD_KIT_ERR("no read nv info!\n");
		return;
	}
	if (!adapt_ops->write_nv_info) {
		LCD_KIT_ERR("no write nv info!\n");
		return;
	}
	ret = adapt_ops->read_nv_info(panel_id, LCD_NV_NUM, sizeof(read_nve_value),
		read_nve_value, sizeof(read_nve_value));
	if (ret) {
		LCD_KIT_ERR("read nv info fail\n");
		return;
	}
	/* bit[2] is PRIMARY_PANEL SET_POWER_BY_NV_BYTE */
	if (panel_id == PRIMARY_PANEL) {
		read_nve_value[SET_POWER_BY_NV_BYTE] = NEED_SET_POWER_BY_NV;
	} else {
		LCD_KIT_ERR("is not PRIMARY_PANEL return\n");
		return;
	}
	ret = adapt_ops->write_nv_info(panel_id, LCD_NV_NUM, sizeof(read_nve_value),
		read_nve_value, sizeof(read_nve_value));
	if (ret) {
		LCD_KIT_ERR("write nv info fail\n");
		return;
	}
	LCD_KIT_INFO("-\n");
}

static int lcd_mipi_detect(uint32_t panel_id)
{
	int i;
	int ret;
	static int count;
	int err_record = 0;
	static int loop_num;
	unsigned int pic_num;
	unsigned int *err_flag = NULL;

	LCD_KIT_INFO("enter \n");
	pic_num = FACT_INFO->mipi_detect.pic_index;
	err_flag = FACT_INFO->mipi_detect.err_flag;
	if (pic_num >= MIPI_DETECT_NUM) {
		LCD_KIT_ERR("pic index out of bound");
		return LCD_KIT_FAIL;
	}
	ret = lcd_judge_mipi_detect(panel_id, pic_num, loop_num);
	if (ret)
		LCD_KIT_ERR("mipi detect judge fail\n");
	count++;
	if (count >= MIPI_DETECT_RING) {
		loop_num++;
		/* set initial value */
		count = 0;
	}
	LCD_KIT_INFO("count = %d, loop_num = %d\n", count, loop_num);
	if (loop_num >= MIPI_DETECT_LOOP) { // MIPI_DETECT_LOOP -> 30
		for (i = 0; i < MIPI_DETECT_NUM; i++) {
			LCD_KIT_INFO("pic : %d, err_num = %d\n",
				i, err_flag[i]);
			if (err_flag[i] >= FACT_INFO->mipi_detect.threshold)
				err_record++;
			/* set initial value */
			FACT_INFO->mipi_detect.err_flag[i] = 0;
		}
		LCD_KIT_INFO("pic_num = %d\n", pic_num);
		if (err_record && pic_num <= DET4_INDEX) // any picture has error, will report
			lcd_kit_mipi_error_dmd_report(FACT_INFO->mipi_detect.reg_val,
				panel_id);
		else if (err_record && pic_num > DET4_INDEX)
			lcd_kit_modify_mipi_error_nv(panel_id);
		/* set initial value */
		loop_num = 0;
	}
	LCD_KIT_INFO("exit\n");
	return ret;
}

static ssize_t lcd_mipi_detect_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int result;
	ssize_t ret = LCD_KIT_OK;
	int panel_id = lcd_kit_get_active_panel_id();

	LCD_KIT_INFO("enter \n");
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is power off\n");
		return LCD_KIT_FAIL;
	}

	if (FACT_INFO->mipi_detect.support) {
		result = lcd_mipi_detect(panel_id);
		ret = snprintf(buf, PAGE_SIZE, "%d\n", result);
	}

	/* disable esd */
	lcd_esd_enable(panel_id, ENABLE);
	return ret;
}

static int lcd_mipi_detect_set(uint32_t panel_id, unsigned int pic_num)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_dsi_panel_cmds *enter_cmds = NULL;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	LCD_KIT_INFO("enter\n");
	pic_num = pic_num - MIPI_DETECT_BASE;
	enter_cmds = FACT_INFO->mipi_detect.enter_cmds;
	if ((pic_num < MIPI_DETECT_START) || (pic_num >= MIPI_DETECT_NUM)) {
		LCD_KIT_ERR("set picture : %u unknown\n", pic_num);
		FACT_INFO->mipi_detect.pic_index = INVALID_INDEX;
		return ret;
	}
	if (!enter_cmds[pic_num].cmds) {
		LCD_KIT_ERR("pic %u enter cmds is null\n", pic_num);
		return LCD_KIT_FAIL;
	}
	/* disable esd */
	lcd_esd_enable(panel_id, DISABLE);
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops\n");
		return LCD_KIT_FAIL;
	}
	if (adapt_ops->mipi_tx == NULL) {
		LCD_KIT_ERR("mipi_tx is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return LCD_KIT_FAIL;
	}
	ret = adapt_ops->mipi_tx(panel_id, NULL, &enter_cmds[pic_num]);
	if (ret) {
		LCD_KIT_ERR("send mipi detect enter cmds failed\n");
		return ret;
	}
	LCD_KIT_INFO("set picture : %u\n", pic_num);
	FACT_INFO->mipi_detect.pic_index = pic_num;
	return ret;
}

static ssize_t lcd_mipi_detect_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret;
	unsigned int index = 0;
	int panel_id = lcd_kit_get_active_panel_id();

	LCD_KIT_INFO("enter\n");
	if (!FACT_INFO->mipi_detect.support) {
		LCD_KIT_ERR("mipi detect not support\n");
		return LCD_KIT_FAIL;
	}
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is power off\n");
		return LCD_KIT_FAIL;
	}
	if (!dev) {
		LCD_KIT_ERR("dev is null\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	/* decimal  parsing */
	ret = kstrtouint(buf, 10, &index);
	if (ret) {
		LCD_KIT_ERR("str to int fail\n");
		return ret;
	}
	LCD_KIT_INFO("picture index = %u\n", index);
	ret = lcd_mipi_detect_set(panel_id, index);
	return ret;
}

static int lcd_check_sum2_detect_set(uint32_t panel_id, unsigned int loop_num)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_dsi_panel_cmds *enter_cmds = FACT_INFO->check_sum2_detect.enter_cmds;
	struct qcom_panel_info *panel_info = NULL;
	struct lcd_kit_adapt_ops *adapt_ops = lcd_kit_get_adapt_ops();

	LCD_KIT_INFO("enter\n");
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops\n");
		return LCD_KIT_FAIL;
	}
	if (loop_num < MIPI_DETECT_START) {
		LCD_KIT_ERR("set loop : %u unknown\n", loop_num);
		FACT_INFO->check_sum2_detect.loop_index = INVALID_INDEX;
		return ret;
	}
	if (!enter_cmds[loop_num].cmds) {
		LCD_KIT_ERR("loop %u enter cmds is null\n", loop_num);
		return LCD_KIT_FAIL;
	}
	if (adapt_ops->mipi_tx == NULL) {
		LCD_KIT_ERR("mipi_tx is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return LCD_KIT_FAIL;
	}
	ret = adapt_ops->mipi_tx(panel_id, NULL, &enter_cmds[loop_num]);
	if (ret) {
		LCD_KIT_ERR("send check sum2 enter cmds failed\n");
		return ret;
	}
	LCD_KIT_INFO("set picture : %u\n", loop_num);
	return ret;
}

static void checksum2_compare(uint32_t panel_id, char expect_value[CHECKSUM2_VAL_NUM],
	unsigned char *read_value, int *count, int loop_num)
{
	if (!expect_value || !read_value || !count)
		return;

	if (read_value[1] == expect_value[1]) {
		LCD_KIT_ERR("[WARING]loop: %u, read_value0 = 0x%x, except_value0 = 0x%x",
		loop_num, read_value[0], expect_value[0]);
		LCD_KIT_ERR("[WARING]loop: %u, read_value1 = 0x%x, except_value1 = 0x%x",
		loop_num, read_value[1], expect_value[1]);
		return;
	}
	if (read_value[0] == expect_value[0]) {
		LCD_KIT_INFO("loop: %u, read_value0 = 0x%x, except_value0 = 0x%x",
		loop_num, read_value[0], expect_value[0]);
		LCD_KIT_INFO("loop: %u, read_value1 = 0x%x, except_value1 = 0x%x",
		loop_num, read_value[1], expect_value[1]);
	} else {
		(*count)++;
		FACT_INFO->check_sum2_detect.reg_val[loop_num][0] = read_value[0];
		FACT_INFO->check_sum2_detect.reg_val[loop_num][1] = read_value[1];
		LCD_KIT_ERR("[ERROR]loop: %u, read_value0 = 0x%x, except_value0 = 0x%x",
		loop_num, read_value[0], expect_value[0]);
		LCD_KIT_ERR("[ERROR]loop: %u, read_value1 = 0x%x, except_value1 = 0x%x",
		loop_num, read_value[1], expect_value[1]);
	}
}

static void check_read_check_sum2_reg_val(uint32_t panel_id,
	unsigned char *read_value, int *count, int loop_num)
{
	int i;
	unsigned int *detect_value = NULL;
	int detect_value_cnt;
	unsigned char expect_value[CHECKSUM2_VAL_NUM] = {0};
	struct lcd_kit_array_data config_data;

	config_data = FACT_INFO->check_sum2_detect.value[loop_num];
	detect_value = config_data.buf;
	detect_value_cnt = config_data.cnt;

	for (i = 0; i < CHECKSUM2_VAL_NUM && i < detect_value_cnt; i++)
		expect_value[i] = detect_value[i] & 0xFF;

	if (FACT_INFO->check_sum2_detect.model_switch)
		(*count) += lcd_checksum_compare(read_value, detect_value, detect_value_cnt);
	else
		checksum2_compare(panel_id, expect_value, read_value, count, loop_num);
}

static int dual_dsi_check_sum2_rx(uint32_t panel_id,
	int loop_num, int *count)
{
	int ret;
	unsigned char read_value0[CHECKSUM2_READ_COUNT] = { 0 };
	unsigned char read_value1[CHECKSUM2_READ_COUNT] = { 0 };
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	LCD_KIT_INFO("enter\n");
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return LCD_KIT_FAIL;
	}
	if (!FACT_INFO->check_sum2_detect.rd_cmds[loop_num].cmds) {
		LCD_KIT_ERR("check sum2 read cmd is NULL\n");
		return LCD_KIT_FAIL;
	}

	ret = lcd_kit_dual_cmds_rx(panel_id, NULL, read_value0, read_value1,
		CHECKSUM2_READ_COUNT - 1, &FACT_INFO->check_sum2_detect.rd_cmds[loop_num]);
	if (ret) {
		LCD_KIT_ERR("check sum2 dual detect read error\n");
		return ret;
	}
	check_read_check_sum2_reg_val(panel_id, read_value0, count, loop_num);

	check_read_check_sum2_reg_val(panel_id, read_value1, count, loop_num);

	return ret;
}

static int lcd_judge_check_sum2_detect(uint32_t panel_id,
	unsigned int loop_num, int *count)
{
	int ret;
	unsigned char read_value[CHECKSUM2_READ_COUNT] = { 0 };
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	unsigned long timeout;

	LCD_KIT_INFO("enter\n");
	timeout = jiffies + (CHECK_SUM2_LOOP_TIME * FACT_INFO->check_sum2_detect.time[loop_num] * HZ);
	if (!FACT_INFO->check_sum2_detect.rd_cmds[loop_num].cmds) {
		LCD_KIT_INFO("check sum2 detect read cmd is NULL\n");
		return LCD_KIT_FAIL;
	}
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops\n");
		return LCD_KIT_FAIL;
	}
	if (adapt_ops->mipi_rx == NULL) {
		LCD_KIT_ERR("mipi_rx is NULL\n");
		return LCD_KIT_FAIL;
	}
	msleep(FACT_INFO->check_sum2_detect.delay);
	do {
		if (!lcd_kit_get_panel_on_state(panel_id)) {
			LCD_KIT_ERR("panel is not power on!\n");
			return LCD_KIT_FAIL;
		}
		if (FACT_INFO->check_sum2_detect.dual_dsi_support) {
			ret = dual_dsi_check_sum2_rx(panel_id, loop_num, count);
			if (ret) {
				LCD_KIT_ERR("dual mipi detect read error\n");
				return ret;
			}
		} else {
			ret = adapt_ops->mipi_rx(panel_id, NULL, read_value,
				CHECKSUM2_READ_COUNT - 1, &FACT_INFO->check_sum2_detect.rd_cmds[loop_num]);
			if (ret) {
				LCD_KIT_ERR("check sum2 detect read error\n");
				return ret;
			}
			check_read_check_sum2_reg_val(panel_id, read_value, count, loop_num);
		}
		msleep(CHECK_SUM2_LOOP_DELAY);
	} while (time_before(jiffies, timeout));

	return ret;
}

static int lcd_check_sum2_detect(uint32_t panel_id)
{
	int ret;
	int count = 0;
	unsigned int loop_num;

	LCD_KIT_INFO("enter \n");
	loop_num = FACT_INFO->check_sum2_detect.loop_index;
	if (loop_num >= INVALID_INDEX) {
		LCD_KIT_ERR("pic index out of bound");
		return LCD_KIT_FAIL;
	}
	ret = lcd_judge_check_sum2_detect(panel_id, loop_num, &count);
	if (ret)
		LCD_KIT_ERR("check sum2 detect judge fail\n");

	LCD_KIT_INFO("loop_num = %d, error_count = %d\n", loop_num, count);
	if (count >= FACT_INFO->check_sum2_detect.threshold) {
		ret = LCD_KIT_FAIL;
		lcd_kit_check_sum2_dmd_report(FACT_INFO->check_sum2_detect.reg_val,
			panel_id);
		/* set initial value */
		count = 0;
	}
	LCD_KIT_INFO("exit\n");
	return ret;
}

static ssize_t lcd_check_sum2_detect_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int result;
	ssize_t ret = LCD_KIT_OK;
	int panel_id = lcd_kit_get_active_panel_id();

	LCD_KIT_INFO("enter \n");
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is power off\n");
		return LCD_KIT_FAIL;
	}
	if (FACT_INFO->check_sum2_detect.support) {
		lcd_check_sum2_detect_set(panel_id,
			FACT_INFO->check_sum2_detect.loop_index);
		result = lcd_check_sum2_detect(panel_id);
		ret = snprintf(buf, PAGE_SIZE, "%d\n", result);
	}
	/* disable esd */
	lcd_esd_enable(panel_id, ENABLE);
	return ret;
}

static ssize_t lcd_check_sum2_detect_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = LCD_KIT_OK;
	unsigned int index = 0;
	int panel_id = lcd_kit_get_active_panel_id();

	LCD_KIT_INFO("enter\n");
	if (!FACT_INFO->check_sum2_detect.support) {
		LCD_KIT_ERR("check sum2 detect not support\n");
		return LCD_KIT_FAIL;
	}
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is power off\n");
		return LCD_KIT_FAIL;
	}
	if (!dev) {
		LCD_KIT_ERR("dev is null\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	/* disable esd */
	lcd_esd_enable(panel_id, DISABLE);
	/* decimal  parsing */
	ret = kstrtouint(buf, 10, &index);
	if (ret) {
		LCD_KIT_ERR("str to int fail\n");
		return ret;
	}
	LCD_KIT_INFO("pic index = %u\n", index);
	if (index <= 0) {
		FACT_INFO->check_sum2_detect.loop_index = INVALID_INDEX;
		return LCD_KIT_FAIL;
	}
	FACT_INFO->check_sum2_detect.loop_index = index - MIPI_DETECT_BASE;
	return ret;
}

static int check_read_mbist_detect_reg_val(uint32_t panel_id, unsigned char *read_value,
	unsigned int pic_index, int loop)
{
	int i;
	unsigned char expect_value;
	unsigned int *detect_value = NULL;
	struct lcd_kit_array_data config_data;

	config_data = FACT_INFO->mbist_detect.value[pic_index];
	detect_value = config_data.buf;
	for (i = 0; i < config_data.cnt; i++) {
		/* obtains the value of the first byte */
		expect_value = detect_value[i] & 0xFF;
		LCD_KIT_INFO("pic: %u, read_val:%d = 0x%x, expect_val = 0x%x\n",
			pic_index, i, read_value[i], expect_value);
		if ((i < VAL_NUM) && (loop < MBIST_LOOPS))
			FACT_INFO->mbist_detect.reg_val[loop][pic_index][i] =
				read_value[i];
		if (read_value[i] != expect_value) {
			FACT_INFO->mbist_detect.err_flag[pic_index]++;
			LCD_KIT_ERR("[ERROR]pic: %u, read_val:%d = 0x%x, but expect_val = 0x%x\n",
				pic_index, i, read_value[i], expect_value);
			return LCD_KIT_FAIL;
		}
	}
	return LCD_KIT_OK;
}

static int lcd_kit_dual_mbist_detect(int panel_id, unsigned int pic_index, int loop)
{
	int ret;
	unsigned char read_value0[MAX_REG_READ_COUNT] = { 0 };
	unsigned char read_value1[MAX_REG_READ_COUNT] = { 0 };
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	LCD_KIT_INFO("enter\n");
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return LCD_KIT_FAIL;
	}
	if (!FACT_INFO->mbist_detect.rd_cmds[pic_index].cmds) {
		LCD_KIT_ERR("mbist detect read cmd is NULL\n");
		return LCD_KIT_FAIL;
	}

	ret = lcd_kit_dual_cmds_rx(panel_id, NULL, read_value0, read_value1,
		MAX_REG_READ_COUNT - 1, &FACT_INFO->mbist_detect.rd_cmds[pic_index]);
	if (ret) {
		LCD_KIT_ERR("mbist dual detect read error\n");
		return ret;
	}
	ret = check_read_mbist_detect_reg_val(panel_id, read_value0,
		pic_index, loop);
	if (ret) {
		LCD_KIT_ERR("mbist detect dsi0 read error\n");
		return ret;
	}
	ret = check_read_mbist_detect_reg_val(panel_id, read_value1,
		pic_index, loop);
	if (ret) {
		LCD_KIT_ERR("mbist detect dsi1 read error\n");
		return ret;
	}

	return ret;
}

static int lcd_judge_mbist_detect(int panel_id, unsigned int pic_index, int loop)
{
	int ret;
	unsigned char read_value[MAX_REG_READ_COUNT] = { 0 };
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	if (FACT_INFO->mbist_detect.rd_cmds[pic_index].cmds == NULL) {
		LCD_KIT_INFO("read mbist detect cmd is NULL\n");
		return LCD_KIT_FAIL;
	}
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops\n");
		return LCD_KIT_FAIL;
	}
	if (adapt_ops->mipi_rx == NULL) {
		LCD_KIT_ERR("mipi_rx is NULL\n");
		return LCD_KIT_FAIL;
	}
	/* delay 2s, or mbist reg value invalid */
	ssleep(2);
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return LCD_KIT_FAIL;
	}
	if (FACT_INFO->mbist_detect.dual_dsi_support) {
		ret = lcd_kit_dual_mbist_detect(panel_id, pic_index, loop);
		if (ret) {
			LCD_KIT_ERR("dual mbist detect read error\n");
			return ret;
		}
	} else {
		ret = adapt_ops->mipi_rx(panel_id, NULL, read_value,
			MAX_REG_READ_COUNT - 1, &FACT_INFO->mbist_detect.rd_cmds[pic_index]);
		if (ret) {
			LCD_KIT_ERR("mbist detect read error\n");
			return ret;
		}
		ret = check_read_mbist_detect_reg_val(panel_id, read_value, pic_index, loop);
	}

	return ret;
}

static int lcd_mbist_detect_test(int panel_id)
{
	int i;
	int ret;
	static int count;
	static int loop_num;
	int err_record = 0;
	unsigned int pic_index;
	unsigned int *error_flag = NULL;

	pic_index = FACT_INFO->mbist_detect.pic_index;
	error_flag = FACT_INFO->mbist_detect.err_flag;
	if (pic_index >= DETECT_NUM) {
		LCD_KIT_ERR("pic number not support\n");
		return LCD_KIT_FAIL;
	}
	ret = lcd_judge_mbist_detect(panel_id, pic_index, loop_num);
	if (ret)
		LCD_KIT_ERR("mbist detect judge fail\n");
	count++;
	if (count >= DETECT_NUM) {
		loop_num++;
		/* set initial value */
		count = 0;
	}
	LCD_KIT_INFO("count = %d, loop_num = %d\n", count, loop_num);
	if (loop_num >= MBIST_LOOPS) {
		for (i = 0; i < DETECT_NUM; i++) {
			LCD_KIT_INFO("pic : %d, err_num = %d\n",
				i, error_flag[i]);
			if (error_flag[i] >= FACT_INFO->mbist_detect.threshold)
				err_record++;
			/* set initial value */
			FACT_INFO->mbist_detect.err_flag[i] = 0;
		}
		if (err_record)
			lcd_kit_mbist_detect_dmd_report(
				FACT_INFO->mbist_detect.reg_val, panel_id);
		/* set initial value */
		loop_num = 0;
	}
	return ret;
}

static int lcd_mbist_detect_set(int panel_id, unsigned int pic_index)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_dsi_panel_cmds *enter_cmds = NULL;

	pic_index = pic_index - DET_START;
	enter_cmds = FACT_INFO->mbist_detect.enter_cmds;
	if ((pic_index >= DET1_INDEX) && (pic_index < DETECT_NUM)) {
		if (enter_cmds[pic_index].cmds == NULL) {
			LCD_KIT_ERR("send mbist detect cmd is NULL\n");
			return LCD_KIT_FAIL;
		}
		if (!lcd_kit_get_panel_on_state(panel_id)) {
			LCD_KIT_ERR("panel is not power on!\n");
			return LCD_KIT_FAIL;
		}
		ret = lcd_kit_cmds_tx(panel_id, NULL, &enter_cmds[pic_index]);
		if (ret) {
			LCD_KIT_ERR("send mbist detect cmd error\n");
			return ret;
		}
		LCD_KIT_INFO("set picture : %u\n", pic_index);
		FACT_INFO->mbist_detect.pic_index = pic_index;
	} else {
		LCD_KIT_INFO("set picture : %u unknown\n", pic_index);
		FACT_INFO->mbist_detect.pic_index = INVALID_INDEX;
	}
	return ret;
}

static ssize_t lcd_mbist_detect_test_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int result;
	ssize_t ret = LCD_KIT_OK;
	int panel_id = lcd_kit_get_active_panel_id();

	if (FACT_INFO->mbist_detect.support) {
		if (lcd_kit_get_panel_on_state(panel_id)) {
			result = lcd_mbist_detect_test(panel_id);
			ret = snprintf(buf, PAGE_SIZE, "%d\n", result);
		}
		lcd_kit_recovery_display(panel_id);
	}
	/* disable esd */
	lcd_esd_enable(panel_id, ENABLE);
	return ret;
}

static ssize_t lcd_mbist_detect_test_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret;
	unsigned int index = 0;
	int panel_id = lcd_kit_get_active_panel_id();

	if (!FACT_INFO->mbist_detect.support) {
		LCD_KIT_ERR("mbist detect not support\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	/* disable esd */
	lcd_esd_enable(panel_id, DISABLE);
	ret = kstrtouint(buf, 10, &index);
	if (ret) {
		LCD_KIT_ERR("invalid parameter\n");
		return ret;
	}

	LCD_KIT_INFO("picture index = %u\n", index);
	if (lcd_kit_get_panel_on_state(panel_id))
		ret = lcd_mbist_detect_set(panel_id, index);

	return ret;
}

static int lcd_kit_aod_detect_setting(uint32_t panel_id, uint32_t mode)
{
	int ret = LCD_KIT_OK;

	LCD_KIT_INFO("color aod detect mode is %u\n", mode);
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return LCD_KIT_FAIL;
	}
	FACT_INFO->oneside_mode.mode = (int)mode;
	if (mode == COLOR_AOD_DETECT_ENTER)
		ret = lcd_kit_cmds_tx(panel_id, NULL,
			&FACT_INFO->color_aod_det.enter_cmds);
	else if (mode == COLOR_AOD_DETECT_EXIT)
		ret = lcd_kit_cmds_tx(panel_id, NULL,
			&FACT_INFO->color_aod_det.exit_cmds);
	else
		LCD_KIT_ERR("color aod detect mode not support\n");
	return ret;
}

static int lcd_kit_oneside_setting(uint32_t panel_id, uint32_t mode)
{
	int ret = LCD_KIT_OK;

	LCD_KIT_INFO("oneside driver mode is %u\n", mode);
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return LCD_KIT_FAIL;
	}
	FACT_INFO->oneside_mode.mode = (int)mode;
	switch (mode) {
	case ONESIDE_DRIVER_LEFT:
		ret = lcd_kit_cmds_tx(panel_id, NULL,
			&FACT_INFO->oneside_mode.left_cmds);
		break;
	case ONESIDE_DRIVER_RIGHT:
		ret = lcd_kit_cmds_tx(panel_id, NULL,
			&FACT_INFO->oneside_mode.right_cmds);
		break;
	case ONESIDE_MODE_EXIT:
		ret = lcd_kit_cmds_tx(panel_id, NULL,
			&FACT_INFO->oneside_mode.exit_cmds);
		break;
	default:
		break;
	}
	return ret;
}

static ssize_t lcd_oneside_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;
	int panel_id = lcd_kit_get_active_panel_id();

	if (FACT_INFO->oneside_mode.support ||
		FACT_INFO->color_aod_det.support)
		ret = snprintf(buf, PAGE_SIZE, "%d\n",
			FACT_INFO->oneside_mode.mode);

	return ret;
}

static ssize_t lcd_oneside_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret;
	unsigned int mode = 0;
	int panel_id = lcd_kit_get_active_panel_id();

	if (!buf) {
		LCD_KIT_ERR("oneside setting store buf null!\n");
		return LCD_KIT_FAIL;
	}
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is power off!\n");
		return LCD_KIT_FAIL;
	}
	if (strlen(buf) >= MAX_BUF) {
		LCD_KIT_ERR("buf overflow!\n");
		return LCD_KIT_FAIL;
	}
	ret = sscanf(buf, "%u", &mode);
	if (!ret) {
		LCD_KIT_ERR("sscanf return invaild:%zd\n", ret);
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("oneside set %d\n", mode);
	if (FACT_INFO->oneside_mode.support)
		lcd_kit_oneside_setting(panel_id, mode);
	if (FACT_INFO->color_aod_det.support)
		lcd_kit_aod_detect_setting(panel_id, mode);
	return count;
}

static ssize_t lcd_pmic_detect_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	int panel_id = lcd_kit_get_active_panel_id();

	LCD_KIT_INFO("+\n");

	if (!FACT_INFO->pmic_detect.support) {
		LCD_KIT_ERR("pmic detect not support!\n");
		ret = snprintf(buf, PAGE_SIZE, "FAIL\n");
		return LCD_KIT_FAIL;
	}

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (adapt_ops->gpio_enable_nolock)
		adapt_ops->gpio_enable_nolock(panel_id, LCD_KIT_PMIC_RESET);
	lcd_kit_delay(LCD_PMIC_ENABLE_DELAY, LCD_KIT_WAIT_MS, true);
	if (adapt_ops->gpio_enable_nolock)
		adapt_ops->gpio_enable_nolock(panel_id, LCD_KIT_PMIC_CTRL);
	lcd_kit_delay(LCD_PMIC_ENABLE_DELAY, LCD_KIT_WAIT_MS, true);

	ret = snprintf(buf, PAGE_SIZE, "OK\n");
	LCD_KIT_INFO("-\n");
	return ret;
}

struct lcd_fact_ops g_fact_ops = {
	.inversion_mode_show = lcd_inversion_mode_show,
	.inversion_mode_store = lcd_inversion_mode_store,
	.scan_mode_show = lcd_scan_mode_show,
	.scan_mode_store = lcd_scan_mode_store,
	.check_reg_show = lcd_check_reg_show,
	.gram_check_show = lcd_gram_check_show,
	.gram_check_store = lcd_gram_check_store,
	.sleep_ctrl_show = lcd_sleep_ctrl_show,
	.sleep_ctrl_store = lcd_sleep_ctrl_store,
	.poweric_det_show = lcd_poweric_det_show,
	.poweric_det_store = lcd_poweric_det_store,
	.pcd_errflag_check_show = lcd_amoled_pcd_errflag_show,
	.pcd_errflag_check_store = lcd_amoled_pcd_errflag_store,
	.test_config_show = lcd_test_config_show,
	.test_config_store = lcd_test_config_store,
	.lv_detect_show = lcd_lv_detect_show,
	.current_detect_show = lcd_current_detect_show,
	.ldo_check_show = lcd_ldo_check_show,
	.general_test_show = lcd_general_test_show,
	.vtc_line_test_show = lcd_vertical_line_test_show,
	.vtc_line_test_store = lcd_vertical_line_test_store,
	.fps_cmd_show = lcd_fps_cmd_show,
	.fps_cmd_store = lcd_fps_cmd_store,
	.standby_current_show = lcd_standby_current_show,
	.mipi_detect_show = lcd_mipi_detect_show,
	.mipi_detect_store = lcd_mipi_detect_store,
	.check_sum2_detect_show = lcd_check_sum2_detect_show,
	.check_sum2_detect_store = lcd_check_sum2_detect_store,
	.mbist_detect_show = lcd_mbist_detect_test_show,
	.mbist_detect_store = lcd_mbist_detect_test_store,
	.oneside_mode_show = lcd_oneside_mode_show,
	.oneside_mode_store = lcd_oneside_mode_store,
	.dynamic_volt_show = lcd_dynamic_volt_show,
	.dynamic_volt_store = lcd_dynamic_volt_store,
	.pmic_detect_show = lcd_pmic_detect_show,
};

int lcd_kit_sysfs_fac_init(struct device *dev)
{
	int ret;

	lcd_fact_ops_register(&g_fact_ops);
	ret = lcd_create_fact_sysfs(&dev->kobj);
	if (ret)
		LCD_KIT_ERR("create factory sysfs fail\n");

	return LCD_KIT_OK;
}
