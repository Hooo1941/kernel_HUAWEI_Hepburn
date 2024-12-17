
/*
 * lcd_kit_sysfs_dkmd.c
 *
 * lcdkit sys node function for lcd driver
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

#include "lcd_kit_sysfs_dkmd.h"
#include "lcd_kit_sysfs_fac.h"
#include "lcd_kit_common.h"
#include "lcd_kit_disp.h"
#include "lcd_kit_sysfs.h"
#include "lcd_kit_utils.h"
#include "lcd_kit_adapt.h"
#ifdef LCD_FACTORY_MODE
#include "lcd_kit_factory.h"
#endif
#include "lcd_kit_panel.h"
#include <securec.h>

#define PANEL_INFO_LIMIT 3
#define MAX_BUF 60

static unsigned int sncode_panel_id;
static int lcd_get_project_id(int panel_id, char *oem_data);
static int lcd_get_2d_barcode(int panel_id, char *oem_data);

static unsigned int oem_info_type = LCD_KIT_FAIL;
static struct oem_info_cmd oem_read_cmds[] = {
	{ PROJECT_ID_TYPE, lcd_get_project_id },
	{ BARCODE_2D_TYPE, lcd_get_2d_barcode },
};

static ssize_t lcd_model_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;
	int panel_id = lcd_kit_get_active_panel_id();

	if (common_ops->get_panel_name)
		ret = common_ops->get_panel_name(panel_id, buf);
	return ret;
}

static ssize_t sec_lcd_model_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;

	if (PUBLIC_INFO->product_type == NORMAL_TYPE)
		return ret;

	if (common_ops->get_panel_name)
		ret = common_ops->get_panel_name(SECONDARY_PANEL, buf);

	return ret;
}

#define AVG_ON_TIME_MAX 100
static ssize_t lcd_avg_on_time_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int i;
	int count = 0;
	int sum = 0;
	int ret;
	char str_ret[AVG_ON_TIME_MAX] = {0};
	char str_tmp[AVG_ON_TIME_MAX] = {0};
	int panel_id = lcd_kit_get_active_panel_id();

	for (i = 0; i < TIME_COUNT; i++) {
		LCD_KIT_INFO("%d ", disp_info->lcd_on_time.delta[i]);
		if (disp_info->lcd_on_time.delta[i] != 0) {
			sum += disp_info->lcd_on_time.delta[i];
			count++;
		}
		snprintf(str_tmp, sizeof(str_tmp), "%d,", disp_info->lcd_on_time.delta[i]);
		strncat(str_ret, str_tmp, strlen(str_tmp));
	}
	LCD_KIT_INFO("\n");
	disp_info->lcd_on_time.avg = sum / count;
	LCD_KIT_INFO("disp_info->lcd_on_time.avg is %d\n", disp_info->lcd_on_time.avg);
	snprintf(str_tmp, sizeof(str_tmp), "avg:%d", disp_info->lcd_on_time.avg);
	strncat(str_ret, str_tmp, strlen(str_tmp));
	ret = snprintf(buf, PAGE_SIZE, "%s\n", str_ret);
	LCD_KIT_INFO("str_ret = %s\n", str_ret);
	return ret;
}

static ssize_t lcd_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", 1);
}

static ssize_t lcd_panel_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;
	int panel_id = lcd_kit_get_active_panel_id();

	if (common_ops->get_panel_info)
		ret = common_ops->get_panel_info(panel_id, buf);
	return ret;
}

static int lcd_get_project_id(int panel_id, char *oem_data)
{
	int i;
	struct lcd_kit_ops *lcd_ops = NULL;
	char project_id[PROJECT_ID_LENGTH + 1] = {0};

	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null\n");
		return LCD_KIT_FAIL;
	}

	if (lcd_ops->get_project_id &&
		lcd_ops->get_project_id(project_id)) {
		oem_data[0] = PROJECT_ID_TYPE;
		oem_data[1] = OEM_INFO_BLOCK_NUM;
		for (i = 0; i < PROJECT_ID_LENGTH; i++)
			oem_data[i + 2] = project_id[i];

		return LCD_KIT_OK;
	} else {
		return LCD_KIT_FAIL;
	}
}

static int lcd_get_2d_barcode(int panel_id, char *oem_data)
{
	char read_value[OEM_INFO_SIZE_MAX + 1];
	int ret = LCD_KIT_OK;

	memset(read_value, 0, OEM_INFO_SIZE_MAX + 1);

	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return LCD_KIT_FAIL;
	}
	if (disp_info->oeminfo.barcode_2d.support) {
		if (disp_info->oeminfo.multiple_reads)
			ret = get_sn_code_multiple_reads(panel_id, read_value);
		else
			ret = lcd_kit_cmds_rx(panel_id, NULL, read_value, OEM_INFO_SIZE_MAX,
				&disp_info->oeminfo.barcode_2d.cmds);

		LCD_KIT_INFO("do lcd_kit_sn_reprocess\n");
		lcd_kit_sn_reprocess(panel_id, read_value, OEM_INFO_SIZE_MAX);
		oem_data[0] = BARCODE_2D_TYPE;
		oem_data[1] = disp_info->oeminfo.barcode_2d.block_num;
		strncat(oem_data, read_value, strlen(read_value));
	}

	return ret;
}

static ssize_t lcd_oem_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	int i;
	char oem_info_data[OEM_INFO_SIZE_MAX];
	char str_oem[OEM_INFO_SIZE_MAX + 1] = {0};
	char str_tmp[OEM_INFO_SIZE_MAX + 1] = {0};
	int panel_id = lcd_kit_get_active_panel_id();

	if (!disp_info->oeminfo.support) {
		LCD_KIT_ERR("oeminfo is not support\n");
		return LCD_KIT_FAIL;
	}
	if (oem_info_type == -1) {
		LCD_KIT_ERR("first write ddic_oem_info, then read\n");
		return LCD_KIT_FAIL;
	}
	memset(oem_info_data, 0, OEM_INFO_SIZE_MAX);
	if (lcd_kit_get_panel_on_state(panel_id)) {
		for (i = 0; i < ARRAY_SIZE(oem_read_cmds); i++) {
			if (oem_info_type != oem_read_cmds[i].type)
				continue;
			LCD_KIT_INFO("oeminfo type = %d\n", oem_info_type);
			if (oem_read_cmds[i].func)
				(*oem_read_cmds[i].func)(panel_id, oem_info_data);
		}
	} else {
		LCD_KIT_ERR("panel is power off\n");
	}
	/* parse data */
	memset(str_oem, 0, sizeof(str_oem));
	for (i = 0; i < oem_info_data[1]; i++) {
		memset(str_tmp, 0, sizeof(str_tmp));
		snprintf(str_tmp, sizeof(str_tmp), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,",
		oem_info_data[0 + i * BARCODE_BLOCK_LEN], oem_info_data[1 + i * BARCODE_BLOCK_LEN],
		oem_info_data[2 + i * BARCODE_BLOCK_LEN], oem_info_data[3 + i * BARCODE_BLOCK_LEN],
		oem_info_data[4 + i * BARCODE_BLOCK_LEN], oem_info_data[5 + i * BARCODE_BLOCK_LEN],
		oem_info_data[6 + i * BARCODE_BLOCK_LEN], oem_info_data[7 + i * BARCODE_BLOCK_LEN],
		oem_info_data[8 + i * BARCODE_BLOCK_LEN], oem_info_data[9 + i * BARCODE_BLOCK_LEN],
		oem_info_data[10 + i * BARCODE_BLOCK_LEN], oem_info_data[11 + i * BARCODE_BLOCK_LEN],
		oem_info_data[12 + i * BARCODE_BLOCK_LEN], oem_info_data[13 + i * BARCODE_BLOCK_LEN],
		oem_info_data[14 + i * BARCODE_BLOCK_LEN], oem_info_data[15 + i * BARCODE_BLOCK_LEN]);
		strncat(str_oem, str_tmp, strlen(str_tmp));
	}
	LCD_KIT_INFO("str_oem = %s\n", str_oem);
	ret = snprintf(buf, PAGE_SIZE, "%s\n", str_oem);
	return ret;
}

static ssize_t lcd_oem_info_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	char *str_oem = NULL;
	char *token = NULL;
	char oem_info[OEM_INFO_SIZE_MAX] = {0};
	int i = 0;
	int panel_id = lcd_kit_get_active_panel_id();

	if (!disp_info->oeminfo.support) {
		LCD_KIT_ERR("oem info is not support\n");
		return LCD_KIT_FAIL;
	}

	if (strlen(buf) < OEM_INFO_SIZE_MAX) {
		str_oem = (char *)buf;
		token = strsep(&str_oem, ",");
		while (token) {
			oem_info[i++] = (unsigned char)simple_strtol(token, NULL, 0);
			token = strsep(&str_oem, ",");
		}
	} else {
		memcpy(oem_info, "INVALID", strlen("INVALID") + 1);
		LCD_KIT_INFO("invalid cmd\n");
	}

	LCD_KIT_INFO("write Type=0x%x , data len=%d\n", oem_info[0], oem_info[1]);
	oem_info_type = oem_info[0];

	return count;
}

static ssize_t lcd_panel_sncode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int i;
	ssize_t ret;
	struct lcd_kit_disp_info *dinfo;
	char oem_info[OEM_INFO_SIZE_MAX] = {0};
	char oem_info_tmp[OEM_INFO_SIZE_MAX] = {0};

	dinfo = lcd_kit_get_disp_info(sncode_panel_id);
	memset(oem_info, 0, sizeof(oem_info));
	for (i = 0; i < sizeof(dinfo->sn_code); i++) {
		memset(oem_info_tmp, 0, sizeof(oem_info_tmp));
		ret = snprintf(oem_info_tmp, sizeof(oem_info_tmp), "%d,", dinfo->sn_code[i]);
		if (ret < 0) {
			LCD_KIT_ERR("snprintf fail\n");
			return LCD_KIT_FAIL;
		}
		strncat(oem_info, oem_info_tmp, strlen(oem_info_tmp));
	}

	ret = snprintf(buf, PAGE_SIZE, "%s\n", oem_info);
	if (ret < 0) {
		LCD_KIT_ERR("snprintf fail\n");
		return LCD_KIT_FAIL;
	}

	return ret;
}

static ssize_t lcd_panel_sncode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int panel_id = 0;
	int ret;

	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}

	ret = kstrtouint(buf, 10, &panel_id);
	if (ret) {
		LCD_KIT_ERR("invalid parameter!\n");
		return ret;
	}

	sncode_panel_id = panel_id;
	LCD_KIT_INFO("panel id is %u\n", panel_id);
	return ret;
}

static ssize_t lcd_panel_version_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;
	int panel_id = lcd_kit_get_active_panel_id();

	if (!disp_info->panel_version.support) {
		ret = snprintf(buf, PAGE_SIZE, "VER:NA\n");
		return ret;
	}

	ret = snprintf(buf, PAGE_SIZE, "%s\n", disp_info->panel_version.lcd_panel_version);
	return ret;
}

static int fps_list_cat(char *str, int len)
{
	int ret;
	int i;
	int fps_rate;
	int panel_id = lcd_kit_get_active_panel_id();
	char tmp_str[MAX_BUF] = {0};

	ret = snprintf(str, len, "current_fps:%d;default_fps:%d;support_fps_list:",
		disp_info->fps.current_fps, disp_info->fps.default_fps);
	for (i = 0; i < disp_info->fps.panel_support_fps_list.cnt; i++) {
		fps_rate = (int)disp_info->fps.panel_support_fps_list.buf[i];
		if (i > 0)
			strncat(str, ",", strlen(","));
		ret += snprintf(tmp_str, sizeof(tmp_str), "%d", fps_rate);
		strncat(str, tmp_str, strlen(tmp_str));
	}
	return ret;
}

static int lcd_get_panel_num(void)
{
	if (lcd_kit_get_display_type() != NORMAL_TYPE)
		return DUAL_PANELS;
	return SINGLE_PANEL;
}

static int product_panel_num_cat(char *str, int len)
{
	int ret;
	int panel_id = lcd_kit_get_active_panel_id();
	int panel_num = lcd_get_panel_num();
	char tmp_str[MAX_BUF] = {0};

	strncat(str, ";product_panel_num:", strlen(";product_panel_num:"));
	ret = snprintf(tmp_str, sizeof(tmp_str), "%d", panel_num);
	strncat(str, tmp_str, strlen(tmp_str));
	strncat(str, ";current_panel_id:", strlen(";current_panel_id:"));
	ret += snprintf(tmp_str, sizeof(tmp_str), "%d", panel_id);
	strncat(str, tmp_str, strlen(tmp_str));
	return ret;
}

static int panel_id_type_fps_cat(char *str, int len)
{
	int ret;
	int panel_id;
	int fps_rate;
	char tmp_str[MAX_BUF] = {0};
	int i;
	int j;
	char oled_type[PANEL_MAX] = {0};
	int panel_num = lcd_get_panel_num();

	for (i = 0; i < panel_num; i++) {
		panel_id = i;
		strncat(str, ";panel_id:", strlen(";panel_id:"));
		ret = snprintf(tmp_str, sizeof(tmp_str), "%d", i);
		strncat(str, tmp_str, strlen(tmp_str));
		strncat(str, ";panel_type:", strlen(";panel_type:"));
		get_oled_type(panel_id, oled_type, PANEL_MAX);
		strncat(str, oled_type, strlen(oled_type));
		strncat(str, ";support_fps:", strlen(";support_fps:"));
		for (j = 0; j < disp_info->fps.panel_support_fps_list.cnt; j++) {
			fps_rate = disp_info->fps.panel_support_fps_list.buf[j];
			if (j > 0)
				strncat(str, ",", strlen(","));
			ret += snprintf(tmp_str, sizeof(tmp_str), "%d", fps_rate);
			strncat(str, tmp_str, strlen(tmp_str));
		}
	}
	return ret;
}

static int fps_scence_show(char *str, int len)
{
	int ret;

	ret = fps_list_cat(str, len);
	ret += product_panel_num_cat(str, len);
	ret += panel_id_type_fps_cat(str, len);
	return ret;
}

static ssize_t lcd_fps_scence_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	char fps_str[LCD_REG_LENGTH_MAX] = {0};
	int panel_id = lcd_kit_get_active_panel_id();

	if (disp_info->fps.support) {
		fps_scence_show(fps_str, LCD_REG_LENGTH_MAX);
	} else {
		ret = snprintf(fps_str, sizeof(fps_str), "lcd_fps=%d",
			disp_info->fps.default_fps);
	}
	ret = snprintf(buf, PAGE_SIZE, "%s\n", fps_str);
	LCD_KIT_INFO("%s\n", fps_str);
	return ret;
}

static ssize_t lcd_fps_scence_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	return count;
}

static ssize_t lcd_fps_order_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	char fps_str[LCD_REG_LENGTH_MAX] = {0};
	char str_tmp[MAX_BUF] = {0};
	int i;
	int fps_rate;
	int panel_id = lcd_kit_get_active_panel_id();

	if (!buf || !attr) {
		LCD_KIT_ERR("lcd_fps_order_show buf NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!disp_info->fps.support) {
		ret = snprintf(buf, PAGE_SIZE, "0\n");
		return ret;
	} else {
		ret = snprintf(fps_str, sizeof(fps_str), "1,%d,%d", disp_info->fps.default_fps,
			disp_info->fps.panel_support_fps_list.cnt);
		for (i = 0; i < disp_info->fps.panel_support_fps_list.cnt; i++) {
			fps_rate = disp_info->fps.panel_support_fps_list.buf[i];
			if (i == 0)
				strncat(fps_str, ",", strlen(","));
			else
				strncat(fps_str, ";", strlen(";"));
			ret += snprintf(str_tmp, sizeof(str_tmp), "%d:%d", fps_rate, ORDER_DELAY);
			strncat(fps_str, str_tmp, strlen(str_tmp));
		}
	}
	LCD_KIT_INFO("%s\n", fps_str);
	ret = snprintf(buf, PAGE_SIZE, "%s\n", fps_str);
	return ret;
}

static ssize_t lcd_product_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int panel_id = lcd_kit_get_active_panel_id();

	if (!buf || !attr) {
		LCD_KIT_ERR("buf is NULL\n");
		return LCD_KIT_FAIL;
	}

	if (common_info->panel_product_type_switch)
		return snprintf(buf, PAGE_SIZE, "%u\n", PRODUCT_TYPE_GREENLAND_RECOVERY);

	return snprintf(buf, PAGE_SIZE, "%u\n", PUBLIC_INFO->product_type);
}

static ssize_t lcd_amoled_pcd_check_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int panel_id = lcd_kit_get_active_panel_id();
	int ret = LCD_KIT_OK;
	int check_result = 0;

	if (!buf || !attr) {
		LCD_KIT_ERR("buf is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (disp_info->pcd_errflag.pcd_support ||
		disp_info->pcd_errflag.errflag_support) {
		check_result = lcd_kit_check_pcd_errflag_check();
		ret = snprintf(buf, PAGE_SIZE, "%d\n", check_result);
		LCD_KIT_INFO("pcd_errflag, the check_result = %d\n",
			check_result);
	}

	return ret;
}

#ifdef LCD_FACTORY_MODE
static int lcd_judge_ddic_lv_detect(int panel_id, unsigned int pic_index, int loop)
{
	int i;
	unsigned char expect_value;
	int ret;
	unsigned int *detect_value = NULL;
	unsigned char read_value[MAX_REG_READ_COUNT] = {0};
	struct lcd_kit_array_data congfig_data;

	congfig_data = FACT_INFO->ddic_lv_detect.value[pic_index];
	detect_value = congfig_data.buf;
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return LCD_KIT_FAIL;
	}
	if (FACT_INFO->ddic_lv_detect.rd_cmds[pic_index].cmds == NULL) {
		LCD_KIT_INFO("read ddic lv detect cmd is NULL\n");
		return LCD_KIT_FAIL;
	}
	/* delay 2s, or ddic reg value invalid */
	ssleep(2);
	ret = lcd_kit_cmds_rx(panel_id, NULL, read_value, MAX_REG_READ_COUNT - 1,
		&FACT_INFO->ddic_lv_detect.rd_cmds[pic_index]);
	if (ret) {
		LCD_KIT_INFO("read ddic lv detect cmd error\n");
		return ret;
	}
	for (i = 0; i < congfig_data.cnt; i++) {
		/* obtains the value of the first byte */
		expect_value = detect_value[i] & 0xFF;
		if ((i < VAL_NUM) && (loop < DETECT_LOOPS))
			FACT_INFO->ddic_lv_detect.reg_val[loop][pic_index][i] =
				read_value[i];
		if (read_value[i] != expect_value) {
			FACT_INFO->ddic_lv_detect.err_flag[pic_index]++;
			LCD_KIT_ERR("pic: %u, read_val:%d = 0x%x, but expect_val = 0x%x\n",
				pic_index, i, read_value[i], expect_value);
			return LCD_KIT_FAIL;
		} else {
			LCD_KIT_INFO("pic: %u, read_val:%d = 0x%x, expect_val = 0x%x\n",
				pic_index, i, read_value[i], expect_value);
		}
	}
	return ret;
}

static int lcd_ddic_lv_detect_test(int panel_id)
{
	int i;
	int ret;
	static int count;
	static int loop_num;
	int err_record = 0;
	unsigned int pic_index;
	unsigned int *error_flag = NULL;

	pic_index = FACT_INFO->ddic_lv_detect.pic_index;
	error_flag = FACT_INFO->ddic_lv_detect.err_flag;
	if (pic_index >= DETECT_NUM) {
		LCD_KIT_ERR("pic number not support\n");
		return LCD_KIT_FAIL;
	}
	ret = lcd_judge_ddic_lv_detect(panel_id, pic_index, loop_num);
	if (ret)
		LCD_KIT_ERR("ddic lv detect judge fail\n");
	count++;
	if (count >= DETECT_NUM) {
		loop_num++;
		count = 0;
	}
	LCD_KIT_INFO("count = %d, loop_num = %d\n", count, loop_num);
	if (loop_num >= DETECT_LOOPS) {
		for (i = 0; i < DETECT_NUM; i++) {
			LCD_KIT_INFO("pic : %d, err_num = %d\n", i, error_flag[i]);
			if (error_flag[i] > ERR_THRESHOLD)
				err_record++;
			FACT_INFO->ddic_lv_detect.err_flag[i] = 0;
		}
		if (err_record >= i)
			lcd_kit_ddic_lv_detect_dmd_report(
				FACT_INFO->ddic_lv_detect.reg_val, panel_id);
		loop_num = 0;
	}
	return ret;
}

static int lcd_ddic_lv_detect_set(int panel_id, unsigned int pic_index)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_dsi_panel_cmds *enter_cmds = NULL;

	pic_index = pic_index - DET_START;
	enter_cmds = FACT_INFO->ddic_lv_detect.enter_cmds;
	if ((pic_index >= DET1_INDEX) && (pic_index < DETECT_NUM)) {
		if (enter_cmds[pic_index].cmds == NULL) {
			LCD_KIT_ERR("send ddic lv detect cmd is NULL\n");
			return LCD_KIT_FAIL;
		}
		ret = lcd_kit_cmds_tx(panel_id, NULL, &enter_cmds[pic_index]);
		if (ret) {
			LCD_KIT_ERR("send enter cmd error\n");
			return ret;
		}
		FACT_INFO->ddic_lv_detect.pic_index = pic_index;
		LCD_KIT_INFO("set picture : %u\n", pic_index);
	} else {
		FACT_INFO->ddic_lv_detect.pic_index = INVALID_INDEX;
		LCD_KIT_INFO("set picture : %u unknown\n", pic_index);
	}
	return ret;
}

static ssize_t lcd_ddic_lv_detect_test_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int result;
	ssize_t ret = LCD_KIT_OK;
	int panel_id = lcd_kit_get_active_panel_id();

	if (FACT_INFO->ddic_lv_detect.support) {
		if (lcd_kit_get_panel_on_state(panel_id)) {
			result = lcd_ddic_lv_detect_test(panel_id);
			ret = snprintf(buf, PAGE_SIZE, "%d\n", result);
		}
		lcd_kit_recovery_display(panel_id);
	}

	return ret;
}

static ssize_t lcd_ddic_lv_detect_test_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret;
	unsigned int index = 0;
	int panel_id = lcd_kit_get_active_panel_id();

	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	ret = kstrtouint(buf, 10, &index);
	if (ret) {
		LCD_KIT_ERR("invalid parameter\n");
		return ret;
	}

	LCD_KIT_INFO("picture index = %u\n", index);
	if (lcd_kit_get_panel_on_state(panel_id))
		ret = lcd_ddic_lv_detect_set(panel_id, index);

	return ret;
}
#endif

static void lcd_get_parse_info_data(char *parse_data, char *buf)
{
	int i;
	char str_tmp[OEM_INFO_SIZE_MAX + 1] = {0};
	/* parse data */
	for (i = 0; i < PANEL_INFO_LIMIT; i++) {
		memset(str_tmp, 0, sizeof(str_tmp));
		snprintf(str_tmp, sizeof(str_tmp),
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,",
			buf[0 + i * BARCODE_BLOCK_LEN], buf[1 + i * BARCODE_BLOCK_LEN],
			buf[2 + i * BARCODE_BLOCK_LEN], buf[3 + i * BARCODE_BLOCK_LEN],
			buf[4 + i * BARCODE_BLOCK_LEN], buf[5 + i * BARCODE_BLOCK_LEN],
			buf[6 + i * BARCODE_BLOCK_LEN], buf[7 + i * BARCODE_BLOCK_LEN],
			buf[8 + i * BARCODE_BLOCK_LEN], buf[9 + i * BARCODE_BLOCK_LEN],
			buf[10 + i * BARCODE_BLOCK_LEN], buf[11 + i * BARCODE_BLOCK_LEN],
			buf[12 + i * BARCODE_BLOCK_LEN], buf[13 + i * BARCODE_BLOCK_LEN],
			buf[14 + i * BARCODE_BLOCK_LEN], buf[15 + i * BARCODE_BLOCK_LEN]);
		strncat(parse_data, str_tmp, strlen(str_tmp));
	}
}

static ssize_t lcd_panel_dieid_info_show(struct device* dev,
	struct device_attribute* attr, char *buf)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_ops *lcd_ops = NULL;
	char dieid_info_data[PANEL_DIEID_INFO_SIZE_MAX] = {0};
	char str_dieid[PANEL_DIEID_INFO_SIZE_MAX + 1] = {0};
	int panel_id = lcd_kit_get_active_panel_id();
	unsigned int panel_state;

	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null\n");
		return LCD_KIT_FAIL;
	}

	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is power off!\n");
		return LCD_KIT_FAIL;
	}

	if (disp_info->panel_dieid_info.support) {
		ret = lcd_ops->get_panel_dieid(panel_id, dieid_info_data);
		if (ret) {
			LCD_KIT_ERR("get_panel_dieid fail!\n");
			return ret;
		}
		strncat(str_dieid, dieid_info_data, strlen(dieid_info_data));
	} else {
		LCD_KIT_ERR("panel_dieid info is not support\n");
		return LCD_KIT_FAIL;
	}

	memset(str_dieid, 0, sizeof(str_dieid));
	lcd_get_parse_info_data(str_dieid, dieid_info_data);
	LCD_KIT_INFO("str_dieid = %s\n", str_dieid);
	ret = snprintf(buf, PAGE_SIZE, "%s", str_dieid);
	return ret;
}

static ssize_t lcd_camera_prox_show(struct device* dev,
	struct device_attribute* attr, char *buf)
{
	int panel_id = PRIMARY_PANEL;

	if (disp_info->camera_prox)
		return snprintf(buf, PAGE_SIZE, "%d\n", 1);
	else
		return snprintf(buf, PAGE_SIZE, "%d\n", 0);
}

static ssize_t lcd_panel_dimming_switch_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret;
	unsigned int on_off = 0;
	int panel_id = lcd_kit_get_active_panel_id();
	unsigned int dsi0_index;
	unsigned int connector_id;

	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	ret = kstrtouint(buf, 10, &on_off);
	if (ret) {
		LCD_KIT_ERR("invalid parameter\n");
		return ret;
	}

	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_INFO("lcd is not power on, skip\n");
		return ret;
	}

	connector_id = DPU_PINFO->connector_id;
	ret = lcd_kit_get_dsi_index(&dsi0_index, connector_id);
	if (ret) {
		LCD_KIT_ERR("get dsi0 index error\n");
		return LCD_KIT_FAIL;
	}

	LCD_KIT_INFO("enter, on_off is = %u\n", on_off);
	if (on_off == 1) {
		dkmd_dfr_enable_dimming(dsi0_index, DPU_PINFO->type);
	} else if (on_off == 0) {
		dkmd_dfr_disable_dimming(dsi0_index, DPU_PINFO->type);
	} else {
		LCD_KIT_ERR("invalid param = %u\n", on_off);
	}

	return ret;
}

static ssize_t lcd_func_switch_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret;
	int panel_id = lcd_kit_get_active_panel_id();

	ret = snprintf(buf, PAGE_SIZE,
						"esd_enable=%u\n",
						disp_info->esd_enable);
	return ret;
}

static ssize_t lcd_func_switch_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	char cmd[MAX_BUF] = {0};
	int panel_id = lcd_kit_get_active_panel_id();

	if (!buf) {
		LCD_KIT_ERR("NULL Pointer!\n");
		return LCD_KIT_FAIL;
	}
	if (strlen(buf) >= MAX_BUF) {
		LCD_KIT_ERR("buf overflow!\n");
		return LCD_KIT_FAIL;
	}
	if (!sscanf(buf, "%s", cmd)) {
		LCD_KIT_ERR("bad command(%s)\n", cmd);
		return count;
	}
	lcd_kit_parse_switch_cmd(panel_id, cmd);
	return count;
}

static ssize_t lcd_tcon_update_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;
	int panel_id = lcd_kit_get_active_panel_id();

	if (!buf) {
		LCD_KIT_ERR("tcon update show buf null!\n");
		return LCD_KIT_FAIL;
	}

	if (disp_info->update_info.support)
		ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			"%d\n", disp_info->update_info.tcon_upd_status);

	return ret;
}

static ssize_t lcd_tcon_update_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = LCD_KIT_OK;
	unsigned int upd_switch = 0;
	int panel_id = lcd_kit_get_active_panel_id();

	if (!buf) {
		LCD_KIT_ERR("tcon update store buf null!\n");
		return LCD_KIT_FAIL;
	}
	if (strlen(buf) >= MAX_BUF) {
		LCD_KIT_ERR("buf overflow!\n");
		return LCD_KIT_FAIL;
	}
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is power off!\n");
		return LCD_KIT_FAIL;
	}
	ret = sscanf_s(buf, "%u", &upd_switch);
	if (ret != 1) {
		LCD_KIT_ERR("sscanf return invalid:%d\n", ret);
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("tcon update switch set %u\n", upd_switch);
	if (disp_info->update_info.support && upd_switch)
		disp_info->update_info.tcon_upd_switch = TCON_UPDATE_ENABLE;
	return count;
}

static ssize_t lcd_tcon_update_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;
	int panel_id = lcd_kit_get_active_panel_id();
	struct lcd_kit_tcon_update *upd = &disp_info->update_info;
	struct lcd_kit_panel_ops *ops = lcd_kit_panel_get_ops();

	if (!buf) {
		LCD_KIT_ERR("tcon update info show buf null!\n");
		return LCD_KIT_FAIL;
	}

	if (!upd || !upd->support) {
		LCD_KIT_ERR("upd is null or not support\n");
		return LCD_KIT_FAIL;
	}

	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is power off!\n");
		return LCD_KIT_FAIL;
	}

	if (ops && ops->lcd_kit_need_update_fw) {
		if (ops->lcd_kit_need_update_fw(panel_id))
			ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", 1);
		else
			ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", 0);
	}
	return ret;
}


static int lcd_check_support(int index)
{
	int panel_id = PRIMARY_PANEL;
	switch (index) {
	case LCD_MODEL_INDEX:
		return SYSFS_SUPPORT;
	case SEC_LCD_MODEL_INDEX:
		return SYSFS_SUPPORT;
	case LCD_AVG_ON_TIME_INDEX:
		return SYSFS_SUPPORT;
	case LCD_TYPE_INDEX:
		return SYSFS_SUPPORT;
	case PANEL_INFO_INDEX:
		return SYSFS_SUPPORT;
	case DDIC_OEM_INDEX:
		return disp_info->oeminfo.support;
	case PANEL_SNCODE_INDEX:
		return common_info->sn_code.support;
	case FPS_SCENCE_INDEX:
		return SYSFS_SUPPORT;
	case LCD_PRODUCT_TYPE_INDEX:
		return SYSFS_SUPPORT;
	case PCD_CHECK_INDEX:
		return SYSFS_SUPPORT;
	case PANEL_DIEID_INDEX:
		return disp_info->panel_dieid_info.support;
	case CAMERA_PROX_INDEX:
		return SYSFS_SUPPORT;
	case LCD_DIMMING_INDEX:
		return SYSFS_SUPPORT;
	case PANEL_VERSION_INDEX:
		return SYSFS_SUPPORT;
	case FUNC_SWITCH_INDEX:
		return SYSFS_SUPPORT;
#ifdef LCD_FACTORY_MODE
	case DDIC_LV_DETECT_INDEX:
		return FACT_INFO->ddic_lv_detect.support;
#endif
	case TCON_UPDATE_INDEX:
	case TCON_UPDATE_INFO_INDEX:
		return disp_info->update_info.support;
	default:
		return SYSFS_NOT_SUPPORT;
	}
}

struct lcd_kit_sysfs_ops g_lcd_sysfs_ops = {
	.check_support = lcd_check_support,
	.model_show = lcd_model_show,
	.sec_model_show = sec_lcd_model_show,
	.type_show = lcd_type_show,
	.panel_info_show = lcd_panel_info_show,
	.ddic_oem_info_show = lcd_oem_info_show,
	.ddic_oem_info_store = lcd_oem_info_store,
	.panel_sncode_show = lcd_panel_sncode_show,
	.panel_sncode_store = lcd_panel_sncode_store,
	.panel_version_show = lcd_panel_version_show,
	.fps_scence_show = lcd_fps_scence_show,
	.fps_scence_store = lcd_fps_scence_store,
	.fps_order_show = lcd_fps_order_show,
	.product_type_show = lcd_product_type_show,
	.pcd_check_show = lcd_amoled_pcd_check_show,
	.panel_dieid_info_show = lcd_panel_dieid_info_show,
	.camera_prox_show = lcd_camera_prox_show,
	.panel_dimming_switch_store = lcd_panel_dimming_switch_store,
	.func_switch_show = lcd_func_switch_show,
	.func_switch_store = lcd_func_switch_store,
	.avg_on_time_show = lcd_avg_on_time_show,
#ifdef LCD_FACTORY_MODE
	.ddic_lv_detect_test_show = lcd_ddic_lv_detect_test_show,
	.ddic_lv_detect_test_store = lcd_ddic_lv_detect_test_store,
#endif
	.tcon_update_show = lcd_tcon_update_show,
	.tcon_update_store = lcd_tcon_update_store,
	.tcon_update_info_show = lcd_tcon_update_info_show,
};

int lcd_kit_sysfs_init(struct device *dev)
{
	int ret;

	lcd_kit_sysfs_ops_register(&g_lcd_sysfs_ops);
	ret = lcd_kit_create_sysfs(&dev->kobj);
	if (ret) {
		LCD_KIT_ERR("create sysfs node fail\n");
		return LCD_KIT_FAIL;
	}

#ifdef LCD_FACTORY_MODE
	/* create factory fs */
	lcd_kit_sysfs_fac_init(dev);
#endif

	return LCD_KIT_OK;
}
