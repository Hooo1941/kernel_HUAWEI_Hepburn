/*
 * lcd_kit_utils.c
 *
 * lcdkit utils function for lcd driver
 *
 * Copyright (c) 2018-2022 Huawei Technologies Co., Ltd.
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

#include "lcd_kit_utils.h"
#include "lcd_kit_disp.h"
#include "lcd_kit_parse.h"
#include "lcd_kit_local_hbm.h"
#include "lcd_kit_panel_pfm.h"
#include "display_engine_interface.h"
#include "lcd_kit_elvss_control.h"
#ifdef LCD_FACTORY_MODE
#include "lcd_kit_factory.h"
#endif
#include "lcd_kit_sysfs_dkmd.h"
#include "lcd_kit_adapt.h"
#ifdef CONFIG_CPLD_DRIVER
#include <huawei_platform/hw_cpld/cpld_driver.h>
#endif
#include <huawei_platform/fingerprint_interface/fingerprint_interface.h>
#include <linux/sort.h>
#include <securec.h>
#ifdef CONFIG_POWER_DUBAI
#include <huawei_platform/log/hwlog_kernel.h>
#endif
/* el ctrl share mem info */
#define SUB_RESERVED_EL_CTRL_INFO_BASE               0x1096C000
#define SUB_RESERVED_EL_CTRL_INFO_SIZE               (0x1000)

#if defined CONFIG_HUAWEI_DSM
extern struct dsm_client *lcd_dclient;
#endif

#define LCD_VER_INDEX 0
#define LCD_HOR_INDEX 1
#define LCD_POS_MAX 2

static char *dsi_name[DSI_MAX] = { "lcd-kit,connector-id0", "lcd-kit,connector-id1", "lcd-kit,connector-id2" };
bool g_skip_power_on_off = false;
unsigned int mipi_err_number = 0;

int lcd_kit_get_dsi_index(unsigned int *dsi_index, unsigned int connector_id)
{
	int i;
	unsigned int dsi[DSI_MAX] = { DSI0_INDEX, DSI1_INDEX, DSI2_INDEX };

	for (i = 0; i < DSI_MAX; i++) {
		if ((connector_id & dsi[i]) > 0) {
			*dsi_index = i;
			return LCD_KIT_OK;
		}
	}

	return LCD_KIT_FAIL;
}

int lcd_kit_get_dual_dsi_index(unsigned int *dsi0_index, unsigned int *dsi1_index,
	unsigned int connector_id)
{
	int ret;
	ret = lcd_kit_get_dsi_index(dsi0_index, connector_id);
	if (ret) {
		LCD_KIT_ERR("get dsi0 index error\n");
		return ret;
	}
	connector_id &= (~(1 << (*dsi0_index)));
	ret = lcd_kit_get_dsi_index(dsi1_index, connector_id);
	if (ret) {
		LCD_KIT_ERR("get dsi1 index error\n");
		return ret;
	}

	return ret;
}

unsigned int lcd_kit_get_esd_support(int panel_id)
{
	if (runmode_is_factory())
		return common_info->esd.fac_esd_support || disp_info->check_mipi.support;
	else
		return common_info->esd.support;
}

void lcd_esd_enable(int panel_id, unsigned int enable)
{
	if (lcd_kit_get_esd_support(panel_id)) {
		disp_info->esd_enable = enable;
		msleep(LCD_KIT_ESD_TIME);
	}
	LCD_KIT_INFO("esd_enable = %d\n", disp_info->esd_enable);
}

static void lcd_dmd_report_err(uint32_t dmd_num,
	const char *dmd_info, int info_len)
{
	if (!dmd_info) {
		LCD_KIT_ERR("info is NULL Pointer\n");
		return;
	}
#if defined(CONFIG_HUAWEI_DSM)
	if (lcd_dclient && !dsm_client_ocuppy(lcd_dclient)) {
		dsm_client_record(lcd_dclient, dmd_info);
		dsm_client_notify(lcd_dclient, dmd_num);
	} else {
		LCD_KIT_INFO("unocuppy dsm client\n");
		dsm_client_unocuppy(lcd_dclient);
		msleep(3);
		if (!dsm_client_ocuppy(lcd_dclient)) {
			dsm_client_record(lcd_dclient, dmd_info);
			dsm_client_notify(lcd_dclient, dmd_num);
		}
	}
#endif
}

static void lcd_kit_pcd_dmd_report(uint8_t *pcd_read_val, uint32_t val_len,
	int panel_id)
{
	int ret;
	char err_info[DMD_DET_ERR_LEN] = {0};
	uint32_t dmd_number;

	if (val_len < PCD_READ_LEN) {
		LCD_KIT_ERR("val len err\n");
		return;
	}
	if (!pcd_read_val) {
		LCD_KIT_ERR("pcd_read_val is NULL\n");
		return;
	}
	ret = snprintf(err_info, DMD_DET_ERR_LEN,
		"Panel = %s PCD detect error Value is 0x%x 0x%x 0x%x\n",
			disp_info->compatible, pcd_read_val[0], pcd_read_val[1], pcd_read_val[2]);
	if (ret < 0) {
		LCD_KIT_ERR("snprintf error\n");
		return;
	}
	dmd_number = (panel_id == PRIMARY_PANEL) ?
		DSM_LCD_PANEL_CRACK_ERROR_NO : DSM_LCD_SUB_PANEL_CRACK_ERROR_NO;
	lcd_dmd_report_err(dmd_number, err_info, DMD_DET_ERR_LEN);
}

static int lcd_kit_judge_pcd_dmd(uint32_t panel_id, uint8_t *read_val,
	uint32_t *expect_val, int cnt, uint32_t compare_mode)
{
	int i;
	uint32_t exp_pcd_value_mask;

	exp_pcd_value_mask = disp_info->pcd_errflag.exp_pcd_mask;
	if (read_val == NULL || expect_val == NULL) {
		LCD_KIT_ERR("read_val or expect_val is NULL\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("pcd check compare mode is %d\n", compare_mode);
	if (compare_mode == PCD_COMPARE_MODE_EQUAL) {
		for (i = 0; i < cnt; i++) {
			if ((uint32_t)read_val[i] != expect_val[i])
				return LCD_KIT_FAIL;
		}
	} else if (compare_mode == PCD_COMPARE_MODE_BIGGER) {
		if ((uint32_t)read_val[0] < expect_val[0])
			return LCD_KIT_FAIL;
	} else if (compare_mode == PCD_COMPARE_MODE_MASK) {
		if (((uint32_t)read_val[0] & exp_pcd_value_mask) == expect_val[0])
			return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

int lcd_kit_pcd_compare_result(uint32_t panel_id, uint8_t *read_val, int ret)
{
	uint32_t *expect_value = NULL;
	uint32_t expect_value_cnt;
	uint8_t pcd_result = PCD_ERRFLAG_SUCCESS;

	expect_value = disp_info->pcd_errflag.pcd_value.buf;
	expect_value_cnt = disp_info->pcd_errflag.pcd_value.cnt;
	if (ret == LCD_KIT_OK) {
		if (lcd_kit_judge_pcd_dmd(panel_id, read_val, expect_value,
			expect_value_cnt,
			disp_info->pcd_errflag.pcd_value_compare_mode) == \
			LCD_KIT_OK) {
			lcd_kit_pcd_dmd_report(read_val, LCD_KIT_PCD_SIZE, panel_id);
			pcd_result |= PCD_FAIL;
			LCD_KIT_ERR("[ERROR]pcd REG read result is 0x%x 0x%x 0x%x\n",
				read_val[0], read_val[1], read_val[2]);
		} else {
			LCD_KIT_INFO("pcd REG read result is 0x%x 0x%x 0x%x\n",
				read_val[0], read_val[1], read_val[2]);
		}
	} else {
		LCD_KIT_ERR("read pcd err\n");
	}
	LCD_KIT_INFO("pcd check result is %d\n", pcd_result);
	return (int)pcd_result;
}

int lcd_kit_check_pcd_errflag_check(void)
{
	uint8_t pcd_result = PCD_ERRFLAG_SUCCESS;
	int ret;
	uint8_t read_pcd[LCD_KIT_PCD_SIZE] = {0};
	uint8_t read_errflag[LCD_KIT_ERRFLAG_SIZE] = {0};
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	int i;

	int panel_id = lcd_kit_get_active_panel_id();
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!adapt_ops->mipi_rx || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("mipi_rx or mipi_tx is NULL\n");
		return LCD_KIT_FAIL;
	}

	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return LCD_KIT_FAIL;
	}

	if (disp_info->pcd_errflag.pcd_support) {
		/* init code */
		ret = adapt_ops->mipi_rx(panel_id, NULL, read_pcd,
			LCD_KIT_PCD_SIZE - 1,
			&disp_info->pcd_errflag.read_pcd_cmds);
		if (ret) {
			LCD_KIT_ERR("mipi_rx fail\n");
			return ret;
		}
		pcd_result = lcd_kit_pcd_compare_result(panel_id, read_pcd, ret);
	}
	/* Reserve interface, redevelop when needed */
	if (disp_info->pcd_errflag.errflag_support) {
		ret = adapt_ops->mipi_rx(panel_id, NULL, read_errflag,
			LCD_KIT_ERRFLAG_SIZE - 1,
			&disp_info->pcd_errflag.read_errflag_cmds);
		if (ret) {
			LCD_KIT_ERR("mipi_rx fail\n");
			return ret;
		}
		for (i = 0; i < LCD_KIT_ERRFLAG_SIZE; i++) {
			if (read_errflag[i] != 0) {
				pcd_result |= ERRFLAG_FAIL;
				break;
			}
		}
	}
	return (int)pcd_result;
}

int lcd_kit_judge_value(unsigned int judge_type, char read_value,
	unsigned int expect_value)
{
	int ret = LCD_KIT_OK;

	switch (judge_type) {
	case LCD_KIT_UNEQUAL:
		if (read_value != expect_value)
			ret = LCD_KIT_FAIL;
		break;
	case LCD_KIT_EQUAL:
		if (read_value == expect_value)
			ret = LCD_KIT_FAIL;
		break;
	case LCD_KIT_LESS:
		if (read_value < expect_value)
			ret = LCD_KIT_FAIL;
		break;
	case LCD_KIT_MORE:
		if (read_value >= expect_value)
			ret = LCD_KIT_FAIL;
		break;
	default:
		if (read_value != expect_value)
			ret = LCD_KIT_FAIL;
		break;
	}
	return ret;
}

void lcd_kit_print_mipi_error_reg(int panel_id)
{
	int i;
	int ret = LCD_KIT_OK;
	char read_value[MAX_MIPI_ERR_READ_COUNT] = {0};
	unsigned int *expect_value = NULL;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_rx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_rx!\n");
		return;
	}
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return;
	}
	ret = adapt_ops->mipi_rx(panel_id, NULL, read_value,
		MAX_MIPI_ERR_READ_COUNT - 1, &disp_info->check_mipi.sub_cmds);
	if (ret) {
		LCD_KIT_INFO("mipi_rx fail\n");
		return;
	}

	expect_value = disp_info->check_mipi.sub_value.buf;
	for (i = 0; i < disp_info->check_mipi.sub_value.cnt; i++)
		LCD_KIT_INFO("[ERROR]check mipi sub%d reg read_value[%d] = 0x%x, expect_value = 0x%x\n",
			i, i, read_value[i], expect_value[i]);
}

void lcd_kit_check_mipi_error(int panel_id)
{
	int ret = LCD_KIT_OK;
	int i;
	unsigned int max_err_number;
	char read_value[MAX_MIPI_ERR_READ_COUNT] = {0};
	unsigned int *expect_value = NULL;
	unsigned int *judge_type = NULL;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	char err_info[DMD_DET_ERR_LEN] = {0};
	uint32_t dmd_number;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_rx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_rx!\n");
		return;
	}

	expect_value = disp_info->check_mipi.value.buf;
	judge_type = disp_info->check_mipi.judge_type.buf;
	max_err_number = disp_info->check_mipi.max_number;
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return;
	}
	ret = adapt_ops->mipi_rx(panel_id, NULL, read_value,
		MAX_MIPI_ERR_READ_COUNT - 1, &disp_info->check_mipi.cmds);
	if (ret) {
		LCD_KIT_INFO("mipi_rx fail\n");
		return;
	}

	for (i = 0; i < disp_info->check_mipi.value.cnt; i++) {
		LCD_KIT_INFO("check mipi sub%d reg read_value[%d] = 0x%x, expect_value = 0x%x\n",
			i, i, read_value[i], expect_value[i]);
		if (lcd_kit_judge_value(judge_type[i], read_value[i], expect_value[i])) {
			mipi_err_number += 1;
			ret = snprintf(err_info, DMD_DET_ERR_LEN,
				"Panel = %s Normal check mipi read_value=0x%x expect_value=0x%x\n",
				disp_info->compatible, read_value[i], expect_value[i]);
			if (mipi_err_number >= max_err_number) {
				mipi_err_number = 0;
				dmd_number = (panel_id == PRIMARY_PANEL) ?
					DSM_LCD_MIPI_CHECK_ERROR_NO : DSM_LCD_SUB_MIPI_CHECK_ERROR_NO;
				lcd_dmd_report_err(dmd_number, err_info, DMD_DET_ERR_LEN);
				lcd_kit_print_mipi_error_reg(panel_id);
			}
			return;
		}
	}
}

void lcd_kit_ddic_lv_detect_dmd_report(
	u8 reg_val[DETECT_LOOPS][DETECT_NUM][VAL_NUM], int panel_id)
{
#if defined (CONFIG_HUAWEI_DSM) && defined(LCD_FACTORY_MODE)
	int i;
	int ret;
	unsigned int len;
	char err_info[DMD_DET_ERR_LEN] = {0};
	uint32_t dmd_number;

	if (!reg_val) {
		LCD_KIT_ERR("reg_val is NULL\n");
		return;
	}
	ret = snprintf(err_info, DMD_DET_ERR_LEN,
		"panel = %s ddic lvd error detect error \0", disp_info->compatible);
	if (ret < 0) {
		LCD_KIT_ERR("snprintf error\n");
		return;
	}
	for (i = 0; i < DETECT_LOOPS; i++) {
		len = strlen(err_info);
		if (len >= DMD_DET_ERR_LEN) {
			LCD_KIT_ERR("strlen error\n");
			return;
		}
		ret = snprintf(err_info + len, DMD_DET_ERR_LEN - len,
			"%d: %x %x, %x %x, %x %x, %x %x ",
			i + DET_START,
			reg_val[i][DET1_INDEX][VAL_1], reg_val[i][DET1_INDEX][VAL_0],
			reg_val[i][DET2_INDEX][VAL_1], reg_val[i][DET2_INDEX][VAL_0],
			reg_val[i][DET3_INDEX][VAL_1], reg_val[i][DET3_INDEX][VAL_0],
			reg_val[i][DET4_INDEX][VAL_1], reg_val[i][DET4_INDEX][VAL_0]);
		if (ret < 0) {
			LCD_KIT_ERR("snprintf error\n");
			return;
		}
	}
	dmd_number = (panel_id == PRIMARY_PANEL) ?
		DSM_LCD_LVD_DETECT_ERROR_NO : DSM_LCD_SUB_LVD_DETECT_ERROR_NO;
	lcd_dmd_report_err(dmd_number, err_info, DMD_DET_ERR_LEN);
#else
	(void)reg_val;
#endif
}

int lcd_kit_gpio_pcd_errflag_check(void)
{
	int ret = LCD_KIT_OK;

	return ret;
}

void check_panel_on_state(void)
{
#ifdef CONFIG_CPLD_DRIVER
	cpld_show_all_reg_data();
#endif
	return;
}

bool is_dual_mipi_panel(unsigned int panel_type)
{
	return (panel_type & (LCD_KIT_DUAL_MIPI_VIDEO | LCD_KIT_DUAL_MIPI_CMD));
}

int lcd_kit_mipi_set_backlight(int panel_id, unsigned int level)
{
	unsigned int bl_level;
	int ret = LCD_KIT_OK;

	bl_level = level;
	ret = common_ops->set_mipi_backlight(panel_id, NULL, bl_level);
	display_engine_brightness_update(bl_level);
#if defined(CONFIG_FINGERPRINT)
	fp_on_lcd_backlight_change(panel_id, bl_level);
#endif
	lcd_kit_bl_pfm_ctrl(panel_id, bl_level, disp_info->fps.current_fps);
	return ret;
}

int lcd_kit_get_bl_set_type(int panel_id)
{
	struct dpu_panel_info *panel_info = lcd_kit_get_dpu_pinfo(panel_id);

	if (!panel_info) {
		LCD_KIT_ERR("panel_info is null!\n");
		return LCD_KIT_FAIL;
	}

	if (panel_info->bl_info.bl_type & BL_SET_BY_PWM)
		return BL_SET_BY_PWM;
	else if (panel_info->bl_info.bl_type & BL_SET_BY_BLPWM)
		return BL_SET_BY_BLPWM;
	else if (panel_info->bl_info.bl_type & BL_SET_BY_MIPI)
		return BL_SET_BY_MIPI;
	else
		return BL_SET_BY_NONE;
}

int lcd_kit_blpwm_set_backlight(int panel_id, unsigned int bl_level)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_bl_ops *bl_ops = lcd_kit_get_bl_ops();;

	if (!bl_ops) {
		LCD_KIT_ERR("bl_ops is null!\n");
		return LCD_KIT_FAIL;
	}
	if (bl_ops->set_backlight)
		ret = bl_ops->set_backlight(bl_level);

	return ret;
}

bool lcd_kit_get_panel_on_state(int panel_id)
{
	return (disp_info->lcd_power_seq == LCD_KIT_POWER_ON);
}

bool lcd_kit_get_panel_off_state(int panel_id)
{
	return (disp_info->lcd_power_seq == LCD_KIT_POWER_OFF ||
			disp_info->lcd_power_seq == LCD_KIT_POWER_OFF_SEQ ||
			disp_info->lcd_power_seq == LCD_KIT_POWER_ON_SEQ ||
			disp_info->lcd_power_seq == LCD_KIT_FAKE_POWER_OFF);
}

void lcd_kit_recovery_display(int panel_id)
{
	return;
}

void lcd_kit_set_mipi_link(int panel_id, int link_state)
{
	int ret;
	unsigned int connector_id;
	unsigned int cmd_type;
	unsigned int dsi0_index;
	unsigned int dsi1_index;

	if (lcd_kit_get_panel_off_state(panel_id)) {
		LCD_KIT_ERR("panel is power off\n");
		return;
	}
	connector_id = DPU_PINFO->connector_id;
	cmd_type = DPU_PINFO->type;
	if (is_dual_mipi_panel(cmd_type)) {
		ret = lcd_kit_get_dual_dsi_index(&dsi0_index, &dsi1_index, connector_id);
		if (ret) {
			LCD_KIT_ERR("get dual dsi index error\n");
			return;
		}
		LCD_KIT_DEBUG("dual %d %d send cmd\n", dsi0_index, dsi1_index);
		if (link_state == LCD_KIT_DSI_LP_MODE) {
			mipi_dsi_set_lp_mode(dsi0_index, DPU_PINFO->type);
			mipi_dsi_set_lp_mode(dsi1_index, DPU_PINFO->type);
		} else {
			mipi_dsi_set_hs_mode(dsi0_index, DPU_PINFO->type);
			mipi_dsi_set_hs_mode(dsi1_index, DPU_PINFO->type);
		}
	} else {
		ret = lcd_kit_get_dsi_index(&dsi0_index, connector_id);
		if (ret) {
			LCD_KIT_ERR("get dsi0 index error\n");
			return;
		}
		LCD_KIT_DEBUG("single %d send cmd\n", dsi0_index);
		if (link_state == LCD_KIT_DSI_LP_MODE) {
			mipi_dsi_set_lp_mode(dsi0_index, DPU_PINFO->type);
		} else {
			mipi_dsi_set_hs_mode(dsi0_index, DPU_PINFO->type);
		}
	}
}

static void lcd_kit_init_sema(int panel_id)
{
	sema_init(&DISP_LOCK->lcd_kit_sem, 1);
	sema_init(&DISP_LOCK->thp_second_poweroff_sem, 1);
}

static void lcd_kit_get_barcode_2d_cmds(int panel_id, struct device_node *np)
{
	int i;
	char name[DMD_DET_ERR_LEN] = {0};

	for (i = 0; i < SN_CODE_READ_TIMES; i++) {
		int ret = snprintf_s(name, DMD_DET_ERR_LEN, DMD_DET_ERR_LEN - 1,
			"lcd-kit,barcode-2d-cmds%d", i);
		if (ret < 0) {
			LCD_KIT_ERR("%s snprintf fail\n", name);
			continue;
		}
		lcd_kit_parse_dcs_cmds(np, name, "lcd-kit,barcode-2d-cmds-state",
			&disp_info->oeminfo.barcode_2d.sn_cmds[i]);
	}
}

static void lcd_kit_parse_oem_info(int panel_id, struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,sn-code-multiple-reads",
		&disp_info->oeminfo.multiple_reads, 0);
	lcd_kit_parse_u32(np, "lcd-kit,oem-info-support",
		&disp_info->oeminfo.support, 0);
	if (disp_info->oeminfo.support) {
		lcd_kit_parse_u32(np, "lcd-kit,oem-barcode-2d-support",
			&disp_info->oeminfo.barcode_2d.support, 0);
		if (disp_info->oeminfo.barcode_2d.support) {
			lcd_kit_parse_u32(np,
				"lcd-kit,oem-barcode-2d-block-num",
				&disp_info->oeminfo.barcode_2d.block_num, 3);
			if (disp_info->oeminfo.multiple_reads) {
				lcd_kit_get_barcode_2d_cmds(panel_id, np);
				return;
			}
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,barcode-2d-cmds",
				"lcd-kit,barcode-2d-cmds-state",
				&disp_info->oeminfo.barcode_2d.cmds);
		}
	}
}

static void parse_fps_char(int panel_id, struct device_node *np)
{
	disp_info->fps.fps_1_cmd = (char *)of_get_property(np,
			"lcd-kit,fps-1-cmd", NULL);
	disp_info->fps.fps_10_cmd = (char *)of_get_property(np,
		"lcd-kit,fps-10-cmd", NULL);
	disp_info->fps.fps_15_cmd = (char *)of_get_property(np,
		"lcd-kit,fps-15-cmd", NULL);
	disp_info->fps.fps_18_cmd = (char *)of_get_property(np,
		"lcd-kit,fps-18-cmd", NULL);
	disp_info->fps.fps_20_cmd = (char *)of_get_property(np,
		"lcd-kit,fps-20-cmd", NULL);
	disp_info->fps.fps_24_cmd = (char *)of_get_property(np,
		"lcd-kit,fps-24-cmd", NULL);
	disp_info->fps.fps_30_cmd = (char *)of_get_property(np,
		"lcd-kit,fps-30-cmd", NULL);
	disp_info->fps.fps_40_cmd = (char *)of_get_property(np,
		"lcd-kit,fps-40-cmd", NULL);
	disp_info->fps.fps_45_cmd = (char *)of_get_property(np,
		"lcd-kit,fps-45-cmd", NULL);
	disp_info->fps.fps_48_cmd = (char *)of_get_property(np,
		"lcd-kit,fps-48-cmd", NULL);
	disp_info->fps.fps_60_cmd = (char *)of_get_property(np,
		"lcd-kit,fps-60-cmd", NULL);
	disp_info->fps.fps_72_cmd = (char *)of_get_property(np,
		"lcd-kit,fps-72-cmd", NULL);
	disp_info->fps.fps_90_cmd = (char *)of_get_property(np,
		"lcd-kit,fps-90-cmd", NULL);
	disp_info->fps.fps_120_cmd = (char *)of_get_property(np,
		"lcd-kit,fps-120-cmd", NULL);
	disp_info->fps.fps_144_cmd = (char *)of_get_property(np,
		"lcd-kit,fps-144-cmd", NULL);
}

static void parse_fps_cmd(int panel_id, struct device_node *np)
{
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-1-cmds",	"lcd-kit,fps-to-1-cmds-state",
		&disp_info->fps.fps_to_1_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-10-cmds", "lcd-kit,fps-to-10-cmds-state",
		&disp_info->fps.fps_to_10_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-15-cmds", "lcd-kit,fps-to-15-cmds-state",
		&disp_info->fps.fps_to_15_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-18-cmds", "lcd-kit,fps-to-18-cmds-state",
		&disp_info->fps.fps_to_18_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-20-cmds", "lcd-kit,fps-to-20-cmds-state",
		&disp_info->fps.fps_to_20_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-24-cmds", "lcd-kit,fps-to-24-cmds-state",
		&disp_info->fps.fps_to_24_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-30-cmds", "lcd-kit,fps-to-30-cmds-state",
		&disp_info->fps.fps_to_30_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-40-cmds", "lcd-kit,fps-to-40-cmds-state",
		&disp_info->fps.fps_to_40_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-45-cmds", "lcd-kit,fps-to-45-cmds-state",
		&disp_info->fps.fps_to_45_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-48-cmds", "lcd-kit,fps-to-48-cmds-state",
		&disp_info->fps.fps_to_48_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-60-cmds", "lcd-kit,fps-to-60-cmds-state",
		&disp_info->fps.fps_to_60_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-72-cmds", "lcd-kit,fps-to-72-cmds-state",
		&disp_info->fps.fps_to_72_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-90-cmds", "lcd-kit,fps-to-90-cmds-state",
		&disp_info->fps.fps_to_90_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-120-cmds", "lcd-kit,fps-to-120-cmds-state",
		&disp_info->fps.fps_to_120_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-144-cmds", "lcd-kit,fps-to-144-cmds-state",
		&disp_info->fps.fps_to_144_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-360-cmds", "lcd-kit,fps-to-360-cmds-state",
		&disp_info->fps.fps_to_360_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-432-cmds", "lcd-kit,fps-to-432-cmds-state",
		&disp_info->fps.fps_to_432_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,refresh-cmds", "lcd-kit,refresh-cmds-state",
		&disp_info->fps.refresh_cmds);
	common_info->backlight.bl_refresh_cmds = disp_info->fps.refresh_cmds;
}

static void parse_fps_bl(int panel_id, struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,bl-to-1-hz",
		&disp_info->fps.bl_to_1_hz, 0);
}

static void parse_fps_aod_bl(int panel_id, struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,aod-bl-to-1-hz",
		&disp_info->fps.aod_bl_to_1_hz, 0);
}

static void parse_fps_mipi(int panel_id, struct device_node *np)
{
	int i;
	int j;
	int table_num;
	struct fps_dsi_timming_table dsi_timming_table[] = {
		{FPS_30, "lcd-kit,fps-30-dsi-timming"},
		{FPS_45, "lcd-kit,fps-45-dsi-timming"},
		{FPS_48, "lcd-kit,fps-48-dsi-timming"},
		{FPS_60, "lcd-kit,fps-60-dsi-timming"},
		{FPS_72, "lcd-kit,fps-72-dsi-timming"},
		{FPS_90, "lcd-kit,fps-90-dsi-timming"},
		{FPS_120, "lcd-kit,fps-120-dsi-timming"},
		{FPS_144, "lcd-kit,fps-144-dsi-timming"},
	};

	table_num = sizeof(dsi_timming_table) / sizeof(dsi_timming_table[0]);
	for (i = 0; i < disp_info->fps.panel_support_fps_list.cnt; i++) {
		for (j = 0; j < table_num; j++) {
			if (dsi_timming_table[j].fps_rate == disp_info->fps.panel_support_fps_list.buf[i])
				lcd_kit_parse_array_data(np, dsi_timming_table[j].name,
					&disp_info->fps.fps_dsi_timming[i]);
		}
	}
}

static void parse_fps(int panel_id, struct device_node *np)
{
	parse_fps_char(panel_id, np);
	parse_fps_cmd(panel_id, np);
	parse_fps_bl(panel_id, np);
	parse_fps_aod_bl(panel_id, np);
	parse_fps_mipi(panel_id, np);
}

static void lcd_kit_parse_fps(int panel_id, struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,fps-support",
		&disp_info->fps.support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,default-fps",
		&disp_info->fps.default_fps, 0);
	if (disp_info->fps.support) {
		disp_info->fps.panel_support_fps_list.cnt = SEQ_NUM;
		lcd_kit_parse_array_data(np, "lcd-kit,panel-support-fps-list",
			&disp_info->fps.panel_support_fps_list);
		parse_fps(panel_id, np);
	}
}

static void lcd_kit_parse_pwrkey_press(int panel_id, struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,pwrkey-press-support",
		&disp_info->pwrkey_press.support, 0);
	if (disp_info->pwrkey_press.support) {
		lcd_kit_parse_u32(np, "lcd-kit,pwrkey-press-pwroff-flag",
			&disp_info->pwrkey_press.power_off_flag, 0);
		lcd_kit_parse_u32(np, "lcd-kit,pwrkey-press-esd-status",
			&disp_info->pwrkey_press.esd_status, 0);
		lcd_kit_parse_u32(np, "lcd-kit,pwrkey-press-rst-addr",
			&disp_info->pwrkey_press.rst_addr, 0);
		lcd_kit_parse_u32(np, "lcd-kit,pwrkey-press-rst-time1",
			&disp_info->pwrkey_press.configtime1, 0);
		lcd_kit_parse_u32(np, "lcd-kit,pwrkey-press-rst-time2",
			&disp_info->pwrkey_press.configtime2, 0);
		lcd_kit_parse_u32(np, "lcd-kit,pwrkey-press-rst-time3",
			&disp_info->pwrkey_press.configtime3, 0);
		lcd_kit_parse_u32(np, "lcd-kit,pwrkey-press-rst-time4",
			&disp_info->pwrkey_press.configtime4, 0);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,quick-power-off-cmds",
			"lcd-kit,quick-power-off-cmds-state", &disp_info->pwrkey_press.cmds);
	}
}

static void lcd_kit_parse_updt_fps_te(int panel_id, struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,updt-fps-te-support",
		&disp_info->updt_fps_te.support, 0);
	if (disp_info->updt_fps_te.support) {
		lcd_kit_parse_u32(np, "lcd-kit,updt-fps-te-cmds-delay",
			&disp_info->updt_fps_te.cmds_delay, 0);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,updt-fps-te-60-cmds",
			"lcd-kit,updt-fps-te-60-cmds-state", &disp_info->updt_fps_te.fps_60_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,updt-fps-te-120-cmds",
			"lcd-kit,updt-fps-te-120-cmds-state", &disp_info->updt_fps_te.fps_120_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,updt-fps-te-144-cmds",
			"lcd-kit,updt-fps-te-144-cmds-state", &disp_info->updt_fps_te.fps_144_cmds);
	}
}

static void lcd_kit_parse_luminance(int panel_id, struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,imax-lcd-luminance",
		&disp_info->max_luminance, 0);
	lcd_kit_parse_u32(np, "lcd-kit,imin-lcd-luminance",
		&disp_info->min_luminance, 0);
}

static void lcd_kit_parse_disp(int panel_id, struct device_node *np)
{
	/* lcd oeminfo */
	lcd_kit_parse_oem_info(panel_id, np);
	/* fps */
	lcd_kit_parse_fps(panel_id, np);
	/* pwrkey press */
	lcd_kit_parse_pwrkey_press(panel_id, np);
	/* te mode ctrl */
	lcd_kit_parse_updt_fps_te(panel_id, np);
	/* luminance */
	lcd_kit_parse_luminance(panel_id, np);
}

static void lcd_kit_init_display(int panel_id, struct device_node *np)
{
	if (!np) {
		LCD_KIT_ERR("np is null\n");
		return;
	}

	/* init lcd power seq */
	if (panel_id == PRIMARY_PANEL)
		disp_info->lcd_power_seq = PUBLIC_INFO->lcd_power_seq;
	else
		disp_info->lcd_power_seq = PUBLIC_INFO->sec_lcd_power_seq;
	if (disp_info->lcd_power_seq == LCD_KIT_POWER_ON)
		lcd_kit_set_aod_panel_state(panel_id, 1);
	else
		lcd_kit_set_aod_panel_state(panel_id, 0);
	/* dubai backlight def 0 */
	disp_info->jank_last_bl_level = 0;
	lcd_kit_parse_disp(panel_id, np);
}

static void lcd_kit_parse_connector_id(struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	unsigned int connector_num = 0;
	unsigned int dsi_id = 0;
	unsigned int connector_id = 0;
	int i;

	lcd_kit_parse_u32(np, "lcd-kit,connector-num", &connector_num, 1);
	for (i = 0; i < connector_num; i++) {
		lcd_kit_parse_u32(np, dsi_name[i], &dsi_id, 0);
		connector_id |= (1 << dsi_id);
	}
	pinfo->connector_id = connector_id;

	LCD_KIT_INFO("connector_id = %d!\n", connector_id);
}

static void lcd_kit_parse_base_pinfo(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,panel-xres",
		&pinfo->xres, 1440);
	lcd_kit_parse_u32(np, "lcd-kit,panel-yres",
		&pinfo->yres, 2560);
	lcd_kit_parse_u32(np, "lcd-kit,panel-width",
		&pinfo->width, 73);
	lcd_kit_parse_u32(np, "lcd-kit,panel-height",
		&pinfo->height, 130);
	lcd_kit_parse_u32(np, "lcd-kit,panel-cmd-type",
		&pinfo->type, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-bpp",
		&pinfo->bpp, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-bgr-fmt",
		&pinfo->bgr_fmt, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-orientation",
		&pinfo->orientation, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-vsyn-ctr-type",
		&pinfo->vsync_ctrl_type, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-fps",
		&pinfo->fps, 60);
	lcd_kit_parse_u32(np, "lcd-kit,te-index",
		&pinfo->lcd_te_idx, 0);
#ifdef LCD_FACTORY_MODE
	pinfo->aod_enable = 0;
#else
	lcd_kit_parse_u8(np, "lcd-kit,sensorhub-aod-support",
		&pinfo->aod_enable, 0);
#endif
	lcd_kit_parse_u8(np, "lcd-kit,update-core-clk-support",
		&pinfo->update_core_clk_support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,is-fake-panel",
		&pinfo->fake_panel_flag, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-split-swap-enable",
		&pinfo->panel_split_swap_enable, 0);
	pinfo->lcd_name = common_info->panel_name;
	lcd_kit_parse_connector_id(np, pinfo);
	/* for second panel */
	if (panel_id == SECONDARY_PANEL)
		pinfo->type |= SECONDARY_PANEL_CMD_TYPE;
}

static void lcd_kit_parse_bl_info(struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,panel-bl-type",
		&pinfo->bl_info.bl_type, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-bl-min",
		&pinfo->bl_info.bl_min, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-bl-max",
		&pinfo->bl_info.bl_max, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-bl-def",
		&pinfo->bl_info.bl_default, 0);
}

static void lcd_kit_parse_mipi(struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	unsigned int pxl_clk_rate = 0;

	lcd_kit_parse_u32(np, "lcd-kit,mipi-h-sync-area", &pinfo->mipi.hsa, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-h-back-porch", &pinfo->mipi.hbp, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-h-line-time", &pinfo->mipi.hline_time, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-dpi-h-size", &pinfo->mipi.dpi_hsize, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-v-sync-area", &pinfo->mipi.vsa, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-v-back-porch", &pinfo->mipi.vbp, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-v-front-porch", &pinfo->mipi.vfp, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-phy-mode", &pinfo->mipi.phy_mode, 0);
	lcd_kit_parse_u8(np, "lcd-kit,mipi-lane-nums", &pinfo->mipi.lane_nums, 0);
	lcd_kit_parse_u8(np, "lcd-kit,mipi-vc", &pinfo->mipi.vc, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-dsi-bit-clk", &pinfo->mipi.dsi_bit_clk, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-pxl-clk", &pxl_clk_rate, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-pxl-clk-div", &pinfo->mipi.pxl_clk_rate_div, 1);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-dsi-upt-support", &pinfo->mipi.dsi_bit_clk_upt_support, 0);
	lcd_kit_parse_u8(np, "lcd-kit,mipi-non-continue-enable", &pinfo->mipi.non_continue_en, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-clk-post-adjust", &pinfo->mipi.clk_post_adjust, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-clk-pre-adjust", &pinfo->mipi.clk_pre_adjust, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-clk-t-hs-prepare-adjust", &pinfo->mipi.clk_t_hs_prepare_adjust, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-clk-t-lpx-adjust", &pinfo->mipi.clk_t_lpx_adjust, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-data-t-lpx-adjust", &pinfo->mipi.data_t_lpx_adjust, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-clk-t-hs-trail-adjust", &pinfo->mipi.clk_t_hs_trial_adjust, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-clk-t-hs-exit-adjust", &pinfo->mipi.clk_t_hs_exit_adjust, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-clk-t-hs-zero-adjust", &pinfo->mipi.clk_t_hs_zero_adjust, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-data-t-hs-zero-adjust", &pinfo->mipi.data_t_hs_zero_adjust, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-data-t-hs-trail-adjust", &pinfo->mipi.data_t_hs_trial_adjust, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-rg-vrefsel-eq-adjust", &pinfo->mipi.rg_vrefsel_eq_adjust, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-rg-vrefsel-vcm-adjust", &pinfo->mipi.rg_vrefsel_vcm_adjust, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-data-lane-lp2hs-time-adjust",
		&pinfo->mipi.data_lane_lp2hs_time_adjust, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-data-t-hs-prepare-adjust",
		&pinfo->mipi.data_t_hs_prepare_adjust, 0);
	lcd_kit_parse_u8(np, "lcd-kit,mipi-color-mode", &pinfo->mipi.color_mode, 0);
	lcd_kit_parse_u8(np, "lcd-kit,mipi-dsi-version", &pinfo->mipi.dsi_version, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-max-tx-esc-clk", &pinfo->mipi.max_tx_esc_clk, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-burst-mode", &pinfo->mipi.burst_mode, 0);

	pinfo->mipi.pxl_clk_rate = (unsigned long long)pxl_clk_rate;
	// change MHz to Hz
	pinfo->mipi.pxl_clk_rate = pinfo->mipi.pxl_clk_rate * 1000000UL;
	pinfo->mipi.max_tx_esc_clk = pinfo->mipi.max_tx_esc_clk * 1000000;
	pinfo->mipi.dsi_bit_clk_upt = pinfo->mipi.dsi_bit_clk;
	pinfo->mipi.dsi_bit_clk_default = pinfo->mipi.dsi_bit_clk;
}

static void lcd_kit_parse_dsc_param(struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,vesa-dsc-version",
		&pinfo->input_dsc_info.dsc_version, 0);
	lcd_kit_parse_u32(np, "lcd-kit,vesa-dsc-format",
		&pinfo->input_dsc_info.format, 0);
	lcd_kit_parse_u32(np, "lcd-kit,vesa-dsc-bpc",
		&pinfo->input_dsc_info.dsc_bpc, 0);
	lcd_kit_parse_u32(np, "lcd-kit,vesa-dsc-bpp",
		&pinfo->input_dsc_info.dsc_bpp, 0);
	lcd_kit_parse_u32(np, "lcd-kit,vesa-slice-height",
		&pinfo->input_dsc_info.slice_height, 0);
	lcd_kit_parse_u32(np, "lcd-kit,vesa-slice-width",
		&pinfo->input_dsc_info.slice_width, 0);
	lcd_kit_parse_u32(np, "lcd-kit,vesa-dsc-blk-pred-en",
		&pinfo->input_dsc_info.block_pred_enable, 0);
	lcd_kit_parse_u32(np, "lcd-kit,vesa-dsc-line-buff-depth",
		&pinfo->input_dsc_info.linebuf_depth, 0);
	lcd_kit_parse_u32(np, "lcd-kit,vesa-dsc-gen-rc-params",
		&pinfo->input_dsc_info.gen_rc_params, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-ifbc-type",
		&pinfo->ifbc_type, 0);

	pinfo->input_dsc_info.pic_width = pinfo->xres;
	pinfo->input_dsc_info.pic_height = pinfo->yres;
	return;
}

static void lcd_kit_dump_cmd(struct dsi_cmd_desc *cmd)
{
	unsigned int i;

	LCD_KIT_DEBUG("cmd->dtype = 0x%x\n", cmd->dtype);
	LCD_KIT_DEBUG("cmd->vc = 0x%x\n", cmd->vc);
	LCD_KIT_DEBUG("cmd->wait = 0x%x\n", cmd->wait);
	LCD_KIT_DEBUG("cmd->waittype = 0x%x\n", cmd->waittype);
	LCD_KIT_DEBUG("cmd->dlen = 0x%x\n", cmd->dlen);
	LCD_KIT_DEBUG("cmd->payload:\n");
	for (i = 0; i < cmd->dlen; i++)
		LCD_KIT_DEBUG("0x%x\n", cmd->payload[i]);
}

int lcd_kit_cmds_to_dsi_cmds(struct lcd_kit_dsi_cmd_desc *lcd_kit_cmds,
	struct dsi_cmd_desc *cmd, bool to_wait)
{
	if (cmd == NULL) {
		LCD_KIT_ERR("cmd is null!\n");
		return LCD_KIT_FAIL;
	}
	if (lcd_kit_cmds == NULL) {
		LCD_KIT_ERR("lcd_kit_cmds is null point!\n");
		return LCD_KIT_FAIL;
	}
	cmd->dtype = lcd_kit_cmds->dtype;
	cmd->vc =  lcd_kit_cmds->vc;
	if (to_wait)
		cmd->wait = lcd_kit_cmds->wait;
	cmd->waittype =  lcd_kit_cmds->waittype;
	cmd->dlen =  lcd_kit_cmds->dlen;
	cmd->payload = lcd_kit_cmds->payload;
	lcd_kit_dump_cmd(cmd);
	return LCD_KIT_OK;
}

static int lcd_kit_tx_el_ctrl_cmds(int panel_id, struct lcd_kit_dsi_panel_cmds *cmds)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = lcd_kit_get_adapt_ops();

	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!adapt_ops->mipi_tx) {
		LCD_KIT_ERR("mipi_tx  is NULL\n");
		return LCD_KIT_FAIL;
	}

	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return LCD_KIT_FAIL;
	}

	ret = adapt_ops->mipi_tx(panel_id, NULL, cmds);
	if (ret) {
		LCD_KIT_ERR("cmds tx failed\n");
		return LCD_KIT_FAIL;
	}
	return ret;
}

static int lcd_kit_el_ctrl_open(int panel_id)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_dsi_panel_cmds *el_ctrl_open_cmds = NULL;

	el_ctrl_open_cmds = &common_info->el_ctrl_info.el_ctrl_open_cmds;
	if (!el_ctrl_open_cmds) {
		LCD_KIT_WARNING("el_ctrl_open_cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	ret = lcd_kit_tx_el_ctrl_cmds(panel_id, el_ctrl_open_cmds);
	if (ret)
		LCD_KIT_ERR("el ctrl open cmds TX fail\n");
	return ret;
}

int lcd_kit_el_ctrl_close(int panel_id)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_dsi_panel_cmds *el_ctrl_close_cmds = NULL;

	el_ctrl_close_cmds = &common_info->el_ctrl_info.el_ctrl_close_cmds;
	if (!el_ctrl_close_cmds) {
		LCD_KIT_WARNING("el_ctrl_open_cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	ret = lcd_kit_tx_el_ctrl_cmds(panel_id, el_ctrl_close_cmds);
	if (ret)
		LCD_KIT_ERR("el ctrl close cmds TX fail\n");
	return ret;
}

bool lcd_kit_el_ctrl_is_support(int panel_id, unsigned int bl_level)
{
	return common_info->el_ctrl_info.support == 1 && common_info->el_ctrl_info.el_init_done == 1;
}

static void lcd_kit_get_el_ctrl_cmds(int panel_id, unsigned int bl_level, struct dpu_elvss_state *el_ctrl_state)
{
	if (!common_info->el_ctrl_info.elvss_ctrl_cmds.cmds ||
			!common_info->el_ctrl_info.vint2_ctrl_cmds.cmds) {
		LCD_KIT_INFO("el ctrl cmds is NULL\n");
		return;
	}

	/* need update payload[1] in 2nd cmd both for elvss cmds and vint2 cmds */
	common_info->el_ctrl_info.elvss_ctrl_cmds.cmds[1].payload[1] = (char)el_ctrl_state->exp_elvss_state;
	if (common_info->el_ctrl_info.ddic_type == DDIC_TYPE_HXX) {
		common_info->el_ctrl_info.vint2_ctrl_cmds.cmds[1].payload[1] = (char)el_ctrl_state->exp_vint2_state;
	} else if (common_info->el_ctrl_info.ddic_type == DDIC_TYPE_CXX) {
		common_info->el_ctrl_info.vint2_ctrl_cmds.cmds[1].payload[1] = (char)(el_ctrl_state->exp_vint2_state & 0xFF);
		common_info->el_ctrl_info.vint2_ctrl_cmds.cmds[2].payload[1] = common_info->el_ctrl_info.vint2_ctrl_cmds.cmds[2].payload[1] & 0xFC |
					(char)((el_ctrl_state->exp_vint2_state & 0x300) >> 8);
	}
}

static void lcd_kit_set_el_ctrl_state(int panel_id, int bl_level, struct dpu_elvss_state *el_ctrl_state)
{
	if (el_ctrl_state->event == SEND_OPEN_CMD || el_ctrl_state->event == SEND_CLOSE_CMD) {
		el_ctrl_state->cur_ddic_enable = el_ctrl_state->exp_ddic_enable;
	} else if (el_ctrl_state->event == SEND_EL_CTRL_CMD) {
		el_ctrl_state->cur_elvss_state = el_ctrl_state->exp_elvss_state;
		el_ctrl_state->cur_vint2_state = el_ctrl_state->exp_vint2_state;
		el_ctrl_state->force_once_send = false;
		el_ctrl_state->cmds_retry = false;
	}
	el_ctrl_state->bl = bl_level;
}

static void report_effect_time_to_dubai(struct dpu_elvss_state *el_ctrl_state)
{
	uint64_t time_diff = 0;
	if (!el_ctrl_state || el_ctrl_state->into_effect_time == 0) {
		LCD_KIT_ERR("ptr is null or start_time is 0\n");
		return;
	}

	if (el_ctrl_state->into_effect_time >= el_ctrl_state->outo_effect_time) {
		LCD_KIT_ERR("into_time abnormal\n");
		return;
	}

	time_diff = el_ctrl_state->outo_effect_time - el_ctrl_state->into_effect_time;

	if (time_diff < TIME_SUM_MAX && (el_ctrl_state->total_effect_time + time_diff < TIME_SUM_MAX))
		el_ctrl_state->total_effect_time += time_diff;

	if (time_diff < DUBAI_TIME_LIMIT && el_ctrl_state->total_effect_time < DUBAI_TIME_LIMIT)
		return;

#ifdef CONFIG_POWER_DUBAI
	HWDUBAI_LOGE("DUBAI_TAG_EPS_LCD_EL", "into_effect_time=%llu outo_effect_time=%llu total_time=%llu",
		el_ctrl_state->first_into_time, el_ctrl_state->outo_effect_time, el_ctrl_state->total_effect_time);
	LCD_KIT_DEBUG("1st_in:%llu ms in:%llu ms,out:%llu ms, total:%llu ms", el_ctrl_state->first_into_time,
		el_ctrl_state->into_effect_time, el_ctrl_state->outo_effect_time, el_ctrl_state->total_effect_time);
#endif
	el_ctrl_state->first_into_time = 0;
	el_ctrl_state->into_effect_time = 0;
	el_ctrl_state->outo_effect_time = 0;
	el_ctrl_state->total_effect_time = 0;
}

static void lcd_kit_check_el_ctrl_state(int panel_id, int bl_level, struct dpu_elvss_state *el_ctrl_state)
{
	int ret = LCD_KIT_OK;

	switch (el_ctrl_state->event) {
	case SEND_OPEN_CMD:
		LCD_KIT_INFO("send el ctrl open cmds\n");
		ret = lcd_kit_el_ctrl_open(panel_id);
		if (ret == LCD_KIT_OK) {
			lcd_kit_set_el_ctrl_state(panel_id, bl_level, el_ctrl_state);
			el_ctrl_state->into_effect_time = (uint64_t)ktime_to_ms(ktime_get_real());
			if (el_ctrl_state->first_into_time == 0)
				el_ctrl_state->first_into_time = el_ctrl_state->into_effect_time;
		}
		break;
	case SEND_CLOSE_CMD:
		LCD_KIT_INFO("send el ctrl close cmds\n");
		ret = lcd_kit_el_ctrl_close(panel_id);
		if (ret == LCD_KIT_OK) {
			lcd_kit_set_el_ctrl_state(panel_id, bl_level, el_ctrl_state);
			el_ctrl_state->outo_effect_time = (uint64_t)ktime_to_ms(ktime_get_real());
#ifdef CONFIG_POWER_DUBAI
			report_effect_time_to_dubai(el_ctrl_state);
#endif
		}
		break;
	case SEND_EL_CTRL_CMD:
		LCD_KIT_INFO("el ctrl cmds\n");
		lcd_kit_get_el_ctrl_cmds(panel_id, bl_level, el_ctrl_state);
		break;
	case SEND_EL_CTRL_NONE:
		LCD_KIT_INFO("none el ctrl\n");
		break;
	default:
		LCD_KIT_INFO("none el ctrl\n");
		break;
	}
}

static void lcd_kit_get_el_cmds(int panel_id, int bl_level, struct dsi_cmds *dsi_cmds)
{
	int i, cmd_cnt;
	struct dpu_elvss_state *el_ctrl_state;
	bool need_update;

	el_ctrl_state = get_elvss_info(panel_id, bl_level);
	if (!el_ctrl_state) {
		LCD_KIT_ERR("el ctrl state is NULL\n");
		return;
	}

	if (el_ctrl_state->event != SEND_EL_CTRL_CMD) {
		LCD_KIT_INFO("not ready to send el ctrl cmds\n");
		return;
	}
	LCD_KIT_INFO("ddic:0x%x-0x%x, elvss:0x%x-0x%x, vint2:0x%x-0x%x\n",
		el_ctrl_state->cur_ddic_enable, el_ctrl_state->exp_ddic_enable, el_ctrl_state->cur_elvss_state,
		el_ctrl_state->exp_elvss_state, el_ctrl_state->cur_vint2_state, el_ctrl_state->exp_vint2_state);
	lcd_kit_check_el_ctrl_state(panel_id, bl_level, el_ctrl_state);
	cmd_cnt = dsi_cmds->cmd_num;
	for (i = cmd_cnt; i < cmd_cnt + common_info->el_ctrl_info.elvss_ctrl_cmds.cmd_cnt; i++) {
		lcd_kit_cmds_to_dsi_cmds(&common_info->el_ctrl_info.elvss_ctrl_cmds.cmds[i - cmd_cnt],
			&dsi_cmds->cmds[i], true);
		dsi_cmds->cmd_num += 1;
	}

	cmd_cnt = dsi_cmds->cmd_num;
	for (i = cmd_cnt; i < cmd_cnt + common_info->el_ctrl_info.vint2_ctrl_cmds.cmd_cnt; i++) {
		lcd_kit_cmds_to_dsi_cmds(&common_info->el_ctrl_info.vint2_ctrl_cmds.cmds[i - cmd_cnt],
			&dsi_cmds->cmds[i], true);
		dsi_cmds->cmd_num += 1;
	}
	lcd_kit_set_el_ctrl_state(panel_id, bl_level, el_ctrl_state);
}

void lcd_kit_get_bl_cmds(int panel_id, int bl_level, struct dsi_cmds *dsi_cmds)
{
	int i;

	switch (common_info->backlight.order) {
	case BL_BIG_ENDIAN:
		if (common_info->backlight.bl_max <= 0xFF) {
			common_info->backlight.bl_cmd.cmds[0].payload[1] = bl_level;
		} else if (common_info->backlight.ext_flag) {
			/* change bl level to dsi cmds */
			common_info->backlight.bl_cmd.cmds[0].payload[1] = common_info->backlight.write_offset;
			common_info->backlight.bl_cmd.cmds[0].payload[2] = (bl_level >> 8) & 0xFF;
			common_info->backlight.bl_cmd.cmds[0].payload[3] = bl_level & 0xFF;
		} else {
			/* change bl level to dsi cmds */
			common_info->backlight.bl_cmd.cmds[0].payload[1] = ((bl_level >> 8) & 0xFF) | common_info->backlight.write_offset;
			common_info->backlight.bl_cmd.cmds[0].payload[2] = bl_level & 0xFF;
		}
		break;
	case BL_LITTLE_ENDIAN:
		if (common_info->backlight.bl_max <= 0xFF) {
			common_info->backlight.bl_cmd.cmds[0].payload[1] = bl_level;
		} else if (common_info->backlight.ext_flag) {
			/* change bl level to dsi cmds */
			common_info->backlight.bl_cmd.cmds[0].payload[1] = bl_level & 0xFF;
			common_info->backlight.bl_cmd.cmds[0].payload[2] = (bl_level >> 8) & 0xFF;
			common_info->backlight.bl_cmd.cmds[0].payload[3] = common_info->backlight.write_offset;
		} else {
			/* change bl level to dsi cmds */
			common_info->backlight.bl_cmd.cmds[0].payload[1] = bl_level & 0xFF;
			common_info->backlight.bl_cmd.cmds[0].payload[2] = ((bl_level >> 8) & 0xFF) | common_info->backlight.write_offset;
		}
		break;
	default:
		LCD_KIT_ERR("not support order\n");
		break;
	}

	for (i = 0; i < common_info->backlight.bl_cmd.cmd_cnt; i++) {
		lcd_kit_cmds_to_dsi_cmds(&common_info->backlight.bl_cmd.cmds[i],
			&dsi_cmds->cmds[i], true);
		dsi_cmds->cmd_num += 1;
	}

	if (lcd_kit_el_ctrl_is_support(panel_id, bl_level))
		lcd_kit_get_el_cmds(panel_id, bl_level, dsi_cmds);
}

int lcd_kit_set_el_ctrl_cmds(int panel_id, unsigned int bl_level)
{
	int ret = LCD_KIT_OK;
	struct dpu_elvss_state *el_ctrl_state;
	struct lcd_kit_dsi_panel_cmds *elvss_ctrl_cmds = NULL;
	struct lcd_kit_dsi_panel_cmds *vint2_ctrl_cmds = NULL;
	bool need_update;

	el_ctrl_state = get_elvss_info(panel_id, bl_level);
	if (!el_ctrl_state) {
		LCD_KIT_ERR("el ctrl state is NULL\n");
		return LCD_KIT_FAIL;
	}

	if (el_ctrl_state->event == SEND_EL_CTRL_NONE) {
		LCD_KIT_DEBUG("el ctrl not ready\n");
		return LCD_KIT_FAIL;
	}

	LCD_KIT_INFO("ddic:0x%x-0x%x, elvss:0x%x-0x%x, vint2:0x%x-0x%x\n",
		el_ctrl_state->cur_ddic_enable, el_ctrl_state->exp_ddic_enable, el_ctrl_state->cur_elvss_state,
		el_ctrl_state->exp_elvss_state, el_ctrl_state->cur_vint2_state, el_ctrl_state->exp_vint2_state);
	lcd_kit_check_el_ctrl_state(panel_id, bl_level, el_ctrl_state);

	elvss_ctrl_cmds = &common_info->el_ctrl_info.elvss_ctrl_cmds;
	vint2_ctrl_cmds = &common_info->el_ctrl_info.vint2_ctrl_cmds;
	if (!elvss_ctrl_cmds || !vint2_ctrl_cmds) {
		LCD_KIT_ERR("elvss_ctrl_cmds or vint2_ctrl_cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (el_ctrl_state->event == SEND_EL_CTRL_CMD) {
		LCD_KIT_DEBUG("start to tx elvss vint2 cmds\n");
		ret = lcd_kit_tx_el_ctrl_cmds(panel_id, elvss_ctrl_cmds);
		ret = lcd_kit_tx_el_ctrl_cmds(panel_id, vint2_ctrl_cmds);
		if (ret == LCD_KIT_OK)
			lcd_kit_set_el_ctrl_state(panel_id, bl_level, el_ctrl_state);
	}
	return ret;
}

void lcd_kit_get_fixed_te2_cmds(int panel_id, struct dsi_cmds *dsi_cmds)
{
	int i;

	for (i = 0; i < common_info->proximity_flicker.lcd_fixed_cmd.cmd_cnt; i++) {
		lcd_kit_cmds_to_dsi_cmds(&common_info->proximity_flicker.lcd_fixed_cmd.cmds[i],
			&dsi_cmds->cmds[i], true);
		dsi_cmds->cmd_num += 1;
	}
}

void lcd_kit_get_follow_te2_cmds(int panel_id, struct dsi_cmds *dsi_cmds)
{
	int i;

	for (i = 0; i < common_info->proximity_flicker.lcd_follow_cmd.cmd_cnt; i++) {
		lcd_kit_cmds_to_dsi_cmds(&common_info->proximity_flicker.lcd_follow_cmd.cmds[i],
			&dsi_cmds->cmds[i], true);
		dsi_cmds->cmd_num += 1;
	}
}

static void parse_mipi_porch_info(int panel_id,
	struct device_node *np, struct dpu_panel_info *pinfo)
{
	int i;
	struct lcd_kit_array_data *fps_timing;

	pinfo->mipi.mipi_frm_rate_mode_num = disp_info->fps.panel_support_fps_list.cnt;
	for (i = 0; i < pinfo->mipi.mipi_frm_rate_mode_num; i++) {
		fps_timing = &disp_info->fps.fps_dsi_timming[i];
		pinfo->mipi.mipi_timing_modes[i].mipi_timing.hsa = fps_timing->buf[FPS_HSA];
		LCD_KIT_INFO("pinfo->mipi.mipi_timing_modes[%d].mipi_timing.hsa = %d\n", i, pinfo->mipi.mipi_timing_modes[i].mipi_timing.hsa);
		pinfo->mipi.mipi_timing_modes[i].mipi_timing.hbp = fps_timing->buf[FPS_HBP];
		LCD_KIT_INFO("pinfo->mipi.mipi_timing_modes[%d].mipi_timing.hbp = %d\n", i, pinfo->mipi.mipi_timing_modes[i].mipi_timing.hbp);
		pinfo->mipi.mipi_timing_modes[i].mipi_timing.dpi_hsize = fps_timing->buf[FPS_HSIZE];
		LCD_KIT_INFO("pinfo->mipi.mipi_timing_modes[%d].mipi_timing.dpi_hsize = %d\n", i, pinfo->mipi.mipi_timing_modes[i].mipi_timing.dpi_hsize);
		pinfo->mipi.mipi_timing_modes[i].mipi_timing.hline_time = fps_timing->buf[FPS_HLINE];
		LCD_KIT_INFO("pinfo->mipi.mipi_timing_modes[%d].mipi_timing.hline_time = %d\n", i, pinfo->mipi.mipi_timing_modes[i].mipi_timing.hline_time);
		pinfo->mipi.mipi_timing_modes[i].mipi_timing.vsa = fps_timing->buf[FPS_VSA];
		LCD_KIT_INFO("pinfo->mipi.mipi_timing_modes[%d].mipi_timing.vsa = %d\n", i, pinfo->mipi.mipi_timing_modes[i].mipi_timing.vsa);
		pinfo->mipi.mipi_timing_modes[i].mipi_timing.vbp = fps_timing->buf[FPS_VBP];
		LCD_KIT_INFO("pinfo->mipi.mipi_timing_modes[%d].mipi_timing.vbp = %d\n", i, pinfo->mipi.mipi_timing_modes[i].mipi_timing.vbp);
		pinfo->mipi.mipi_timing_modes[i].mipi_timing.vfp = fps_timing->buf[FPS_VFP];
		LCD_KIT_INFO("pinfo->mipi.mipi_timing_modes[%d].mipi_timing.vfp = %d\n", i, pinfo->mipi.mipi_timing_modes[i].mipi_timing.vfp);
		pinfo->mipi.mipi_timing_modes[i].dsi_bit_clk_default = fps_timing->buf[FPS_DSI_CLK];
		LCD_KIT_INFO("pinfo->mipi.mipi_timing_modes[%d].dsi_bit_clk_default = %d\n", i, pinfo->mipi.mipi_timing_modes[i].dsi_bit_clk_default);
		pinfo->mipi.mipi_timing_modes[i].frame_rate = disp_info->fps.panel_support_fps_list.buf[i];
		LCD_KIT_INFO("pinfo->mipi.mipi_timing_modes[%d].frame_rate = %d\n", i, pinfo->mipi.mipi_timing_modes[i].frame_rate);
	}
}

static void parse_ltps_info(int panel_id,
	struct device_node *np, struct dpu_panel_info *pinfo)
{
	LCD_KIT_INFO("parse ltps info\n");
	lcd_kit_parse_u32(np, "lcd-kit,ltpo-ver", &pinfo->dfr_info.oled_info.ltpo_info.ver, 0);
	lcd_kit_parse_u32(np, "lcd-kit,dfr-mode", &pinfo->dfr_info.dfr_mode, 0);
	LCD_KIT_INFO("pinfo->dfr_info.dfr_mode = %d\n", pinfo->dfr_info.dfr_mode);
	if (pinfo->dfr_info.dfr_mode != DFR_MODE_LONG_V && pinfo->dfr_info.dfr_mode != DFR_MODE_LONG_H
		&& pinfo->dfr_info.dfr_mode != DFR_MODE_LONG_VH) {
		LCD_KIT_INFO("not support ltps fps\n");
		return;
	}
	parse_mipi_porch_info(panel_id, np, pinfo);
}

static void parse_ltpo_base_cmd(int panel_id,
	struct device_node *np, struct dpu_panel_info *pinfo)
{
	int i;
	int j;
	int cmd_num;
	struct lcd_kit_dsi_panel_cmds ltpo_base_cmd[] = {
		disp_info->fps.fps_to_90_cmds,
		disp_info->fps.fps_to_120_cmds,
		disp_info->fps.refresh_cmds,
		disp_info->fps.fps_to_1_cmds,
		disp_info->fps.bl_cmds,
		disp_info->fps.fps_to_360_cmds,
		disp_info->fps.fps_to_432_cmds,
	};

	cmd_num = sizeof(ltpo_base_cmd) / sizeof(ltpo_base_cmd[0]);
	for (i = 0; i < cmd_num; i++) {
		pinfo->dfr_info.oled_info.ltpo_info.dsi_cmds[i].cmd_num = ltpo_base_cmd[i].cmd_cnt;
		for (j = 0; j < ltpo_base_cmd[i].cmd_cnt; j++)
			lcd_kit_cmds_to_dsi_cmds(&ltpo_base_cmd[i].cmds[j],
				&pinfo->dfr_info.oled_info.ltpo_info.dsi_cmds[i].cmds[j], true);
	}
}

static void safe_fps_cmd_to_dsi_cmds(int panel_id,
	struct dpu_panel_info *pinfo, unsigned int frame_rate, int num)
{
	int i;
	int j;
	int cmd_num;
	struct fps_cmd_table fps_safe_cmd[] = {
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

	cmd_num = sizeof(fps_safe_cmd) / sizeof(fps_safe_cmd[0]);
	for (i = 0; i < cmd_num; i++) {
		if (frame_rate == fps_safe_cmd[i].val) {
			pinfo->dfr_info.oled_info.ltpo_info.safe_frm_rates[num].dsi_cmds.cmd_num = fps_safe_cmd[i].cmds->cmd_cnt;
			for (j = 0; j < fps_safe_cmd[i].cmds->cmd_cnt; j++)
				lcd_kit_cmds_to_dsi_cmds(&fps_safe_cmd[i].cmds->cmds[j],
					&pinfo->dfr_info.oled_info.ltpo_info.safe_frm_rates[num].dsi_cmds.cmds[j], true);
			break;
		}
	}
}

static void parse_ltpo_safe_cmd(int panel_id,
	struct device_node *np, struct dpu_panel_info *pinfo)
{
	int i;
	struct lcd_kit_array_data safe_freq;

	lcd_kit_parse_array_data(np, "lcd-kit,safe-frame-info", &safe_freq);
	pinfo->dfr_info.oled_info.ltpo_info.safe_frm_rates_num = safe_freq.cnt;
	if (safe_freq.cnt > MAX_SAFE_FREQ_COUNT) {
		LCD_KIT_ERR("safe_freq.cnt is %d too big!\n", safe_freq.cnt);
		return;
	}
	LCD_KIT_INFO("safe_freq.cnt is %d!\n", safe_freq.cnt);
	for (i = 0; i < safe_freq.cnt; i++) {
		pinfo->dfr_info.oled_info.ltpo_info.safe_frm_rates[i].safe_frm_rate = safe_freq.buf[i];
		LCD_KIT_INFO("safe_freq.buf[%d] is %d!\n", i, safe_freq.buf[i]);
		safe_fps_cmd_to_dsi_cmds(panel_id, pinfo, safe_freq.buf[i], i);
	}
}

/* LTPO 120hz dimming seq */
static void parse_ltpo_120hz_dimming_seq(int panel_id,
	struct device_node *np, struct dpu_panel_info *pinfo)
{
	struct lcd_kit_array_data dimming_seq;
	struct lcd_kit_array_data repeat_num;
	int i;

	if ((lcd_kit_parse_array_data(np, "lcd-kit,dimming-seq", &dimming_seq) == LCD_KIT_OK) &&
		(lcd_kit_parse_array_data(np, "lcd-kit,repeat-num", &repeat_num) == LCD_KIT_OK)) {
		pinfo->dfr_info.oled_info.ltpo_info.dimming_sequence[DIMMING_SEQ_120HZ_INDEX].dimming_seq_num = dimming_seq.cnt;
		for (i = 0; i < dimming_seq.cnt && dimming_seq.buf != NULL; i++) {
			pinfo->dfr_info.oled_info.ltpo_info.dimming_sequence[DIMMING_SEQ_120HZ_INDEX].dimming_seq_list[i].frm_rate = dimming_seq.buf[i];
			pinfo->dfr_info.oled_info.ltpo_info.dimming_sequence[DIMMING_SEQ_120HZ_INDEX].dimming_seq_list[i].repeat_num = repeat_num.buf[i];
			LCD_KIT_INFO("120hz frm_rate[%d] is %d!\n", i, dimming_seq.buf[i]);
			LCD_KIT_INFO("120hz repeat_num[%d] is %d!\n", i, repeat_num.buf[i]);
		}
		pinfo->dfr_info.oled_info.ltpo_info.dimming_sequence[DIMMING_SEQ_120HZ_INDEX].type = DIMMING_SEQ_120HZ_INDEX;
	}
}

/* LTPO 90hz dimming seq */
static void parse_ltpo_90hz_dimming_seq(int panel_id,
	struct device_node *np, struct dpu_panel_info *pinfo)
{
	struct lcd_kit_array_data dimming_seq;
	struct lcd_kit_array_data repeat_num;
	int i;

	if ((lcd_kit_parse_array_data(np, "lcd-kit,90hz-dimming-seq", &dimming_seq) == LCD_KIT_OK) &&
		(lcd_kit_parse_array_data(np, "lcd-kit,90hz-repeat-num", &repeat_num) == LCD_KIT_OK)) {
		pinfo->dfr_info.oled_info.ltpo_info.dimming_sequence[DIMMING_SEQ_90HZ_INDEX].dimming_seq_num = dimming_seq.cnt;
		for (i = 0; i < dimming_seq.cnt && dimming_seq.buf != NULL; i++) {
			pinfo->dfr_info.oled_info.ltpo_info.dimming_sequence[DIMMING_SEQ_90HZ_INDEX].dimming_seq_list[i].frm_rate = dimming_seq.buf[i];
			pinfo->dfr_info.oled_info.ltpo_info.dimming_sequence[DIMMING_SEQ_90HZ_INDEX].dimming_seq_list[i].repeat_num = repeat_num.buf[i];
			LCD_KIT_INFO("90hz frm_rate[%d] is %d!\n", i, dimming_seq.buf[i]);
			LCD_KIT_INFO("90hz repeat_num[%d] is %d!\n", i, repeat_num.buf[i]);
		}
		pinfo->dfr_info.oled_info.ltpo_info.dimming_sequence[DIMMING_SEQ_90HZ_INDEX].type = DIMMING_SEQ_90HZ_INDEX;
	}
}

static int parse_ltpo_te_info(struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	struct lcd_kit_array_data te_freq;
	struct lcd_kit_array_data te_mask;
	int i;
	int ret;

	ret = lcd_kit_parse_array_data(np, "lcd-kit,te-freq-info", &te_freq);
	if (ret != LCD_KIT_OK)
		 return LCD_KIT_FAIL;

	ret = lcd_kit_parse_array_data(np, "lcd-kit,te-mask-info", &te_mask);
	if (ret != LCD_KIT_OK)
		 return LCD_KIT_FAIL;

	LCD_KIT_INFO("te_freq.cnt is %d, te_mask.cnt is %d!\n", te_freq.cnt, te_mask.cnt);

	// te_freq.cnt and te_mask.cnt must be equal
	if (te_freq.cnt > TE_FREQ_NUM_MAX || te_mask.cnt > TE_FREQ_NUM_MAX || te_freq.cnt != te_mask.cnt) {
		LCD_KIT_ERR("te cnt para config error!\n");
		return LCD_KIT_FAIL;
	}

	pinfo->dfr_info.oled_info.ltpo_info.te_freq_num = te_freq.cnt;
	pinfo->dfr_info.oled_info.ltpo_info.te_mask_num = te_mask.cnt;
	for (i = 0; i < te_freq.cnt; i++) {
		pinfo->dfr_info.oled_info.ltpo_info.te_freqs[i] = te_freq.buf[i];
		pinfo->dfr_info.oled_info.ltpo_info.te_masks[i] = te_mask.buf[i];
		LCD_KIT_INFO("te_freq.buf[%d] is %d!\n", i, te_freq.buf[i]);
		LCD_KIT_INFO("te_mask.buf[%d] is %d!\n", i, te_mask.buf[i]);
	}

	return LCD_KIT_OK;
}

static void parse_ltpo1_info(int panel_id,
	struct device_node *np, struct dpu_panel_info *pinfo)
{
	int ret;

	struct lcd_kit_array_data dimming_seq;
	struct lcd_kit_array_data repeat_num;

	ret = parse_ltpo_te_info(np, pinfo);
	if (ret != LCD_KIT_OK)
		 return;

	parse_ltpo_120hz_dimming_seq(panel_id, np, pinfo);
	parse_ltpo_base_cmd(panel_id, np, pinfo);
	parse_ltpo_safe_cmd(panel_id, np, pinfo);
}

static void parse_ltpo2_info(int panel_id,
	struct device_node *np, struct dpu_panel_info *pinfo)
{
	int ret;

	struct lcd_kit_array_data dimming_seq_120hz;
	struct lcd_kit_array_data repeat_num_120hz;
	struct lcd_kit_array_data dimming_seq_90hz;
	struct lcd_kit_array_data repeat_num_90hz;

	ret = parse_ltpo_te_info(np, pinfo);
	if (ret != LCD_KIT_OK)
		 return;

	parse_ltpo_120hz_dimming_seq(panel_id, np, pinfo);
	parse_ltpo_90hz_dimming_seq(panel_id, np, pinfo);
	parse_ltpo_base_cmd(panel_id, np, pinfo);
	parse_ltpo_safe_cmd(panel_id, np, pinfo);
}

static int cmp_u32(const void *a, const void *b)
{
	if (*(const uint32_t *)a < *(const uint32_t *)b)
		return 1;
	else if (*(const uint32_t *)a > *(const uint32_t *)b)
		return -1;
	else
		return 0;
}

static void sort_fps_list(struct dpu_panel_info *pinfo)
{
	unsigned int i;

	if (pinfo->dfr_info.dfr_mode == DFR_MODE_LONGH_TE_SKIP_BY_MCU)
		return;

	sort(pinfo->dfr_info.oled_info.fps_sup_seq, pinfo->dfr_info.oled_info.fps_sup_num,
		sizeof(uint32_t), cmp_u32, NULL);
	for (i = 0; i < pinfo->dfr_info.oled_info.fps_sup_num; i++)
		LCD_KIT_INFO("pinfo->dfr_info.oled_info.fps_sup_seq[%d] is %d\n", i, pinfo->dfr_info.oled_info.fps_sup_seq[i]);
}

static void parse_ltpo_info(int panel_id,
	struct device_node *np, struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,ltpo-ver", &pinfo->dfr_info.oled_info.ltpo_info.ver, 0);
	lcd_kit_parse_u32(np, "lcd-kit,dfr-mode", &pinfo->dfr_info.dfr_mode, 0);
	lcd_kit_parse_u32(np, "lcd-kit,dimming-mode", &pinfo->dfr_info.oled_info.ltpo_info.dimming_mode, 0);
	lcd_kit_parse_u32(np, "lcd-kit,te2-fixed-120hz-support",
		&pinfo->dfr_info.oled_info.ltpo_info.te2_enable, 0);

#ifdef LCD_FACTORY_MODE
	pinfo->dfr_info.dfr_mode = 0;
	pinfo->dfr_info.oled_info.ltpo_info.dimming_enable = 0;
#else
	pinfo->dfr_info.oled_info.ltpo_info.dimming_enable = 1;
#endif
	lcd_kit_parse_u32(np, "lcd-kit,panel-ddic-type", &pinfo->dfr_info.ddic_type, 0);
	sort_fps_list(pinfo);
	if (pinfo->dfr_info.oled_info.ltpo_info.ver == PANEL_LTPO_V1)
		parse_ltpo1_info(panel_id, np, pinfo);
	if (pinfo->dfr_info.oled_info.ltpo_info.ver == PANEL_LTPO_V2)
		parse_ltpo2_info(panel_id, np, pinfo);
	if (pinfo->dfr_info.dfr_mode == DFR_MODE_LONGH_TE_SKIP_BY_MCU)
		parse_mipi_porch_info(panel_id, np, pinfo);
}

static void lcd_kit_parse_pinfo_fps(int panel_id,
	struct device_node *np, struct dpu_panel_info *pinfo)
{
	int i;

	if (!disp_info->fps.support) {
		LCD_KIT_INFO("not support fps\n");
		return;
	}
	lcd_kit_parse_u32(np, "lcd-kit,oled-type",
		&pinfo->dfr_info.oled_info.oled_type, 0);
	LCD_KIT_INFO("oled_type %d\n", pinfo->dfr_info.oled_info.oled_type);
	pinfo->fps = disp_info->fps.default_fps;
	LCD_KIT_INFO("pinfo->fps %d\n", pinfo->fps);
	disp_info->fps.current_fps = disp_info->fps.default_fps;
	LCD_KIT_INFO("disp_info->fps.current_fps %d\n", disp_info->fps.current_fps);
	pinfo->dfr_info.oled_info.fps_sup_num = disp_info->fps.panel_support_fps_list.cnt;
	LCD_KIT_INFO("pinfo->dfr_info.oled_info.fps_sup_num %d\n",
		pinfo->dfr_info.oled_info.fps_sup_num);
	for (i = 0; i < pinfo->dfr_info.oled_info.fps_sup_num; i++) {
		pinfo->dfr_info.oled_info.fps_sup_seq[i] = disp_info->fps.panel_support_fps_list.buf[i];
		LCD_KIT_INFO("pinfo->dfr_info.oled_info.fps_sup_seq[%d] is %d\n",
			i, pinfo->dfr_info.oled_info.fps_sup_seq[i]);
	}
	if (pinfo->dfr_info.oled_info.oled_type == LTPS)
		parse_ltps_info(panel_id, np, pinfo);
	if (pinfo->dfr_info.oled_info.oled_type == LTPO)
		parse_ltpo_info(panel_id, np, pinfo);
}

static void lcd_kit_parse_utils(int panel_id,
	struct device_node *np, struct dpu_panel_info *pinfo)
{
	LCD_KIT_INFO("enter\n");
	/* pcd errflag */
	lcd_kit_parse_u32(np, "lcd-kit,pcd-errflag-check-support",
		&disp_info->pcd_errflag.pcd_errflag_check_support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,pcd-cmds-support",
		&disp_info->pcd_errflag.pcd_support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,gpio-errflag",
		&disp_info->pcd_errflag.gpio_errflag, 0);
	lcd_kit_parse_u32(np, "lcd-kit,gpio-pcd",
		&disp_info->pcd_errflag.gpio_pcd, 0);
	lcd_kit_parse_u32(np, "lcd-kit,pcd-power-on-support",
		&disp_info->pcd_errflag.power_on_support, 1);
	if (disp_info->pcd_errflag.pcd_support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,start-pcd-check-cmds",
			"lcd-kit,start-pcd-check-cmds-state",
			&disp_info->pcd_errflag.start_pcd_check_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,pcd-read-cmds",
			"lcd-kit,pcd-read-cmds-state",
			&disp_info->pcd_errflag.read_pcd_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,switch-page-cmds",
			"lcd-kit,switch-page-cmds-state",
			&disp_info->pcd_errflag.switch_page_cmds);
		lcd_kit_parse_u32(np, "lcd-kit,pcd-check-reg-value-compare-mode",
			&disp_info->pcd_errflag.pcd_value_compare_mode, 0);
		lcd_kit_parse_u32(np, "lcd-kit,exp-pcd-mask",
			&disp_info->pcd_errflag.exp_pcd_mask, 0);
		lcd_kit_parse_u32(np, "lcd-kit,pcd-det-num",
			&disp_info->pcd_errflag.pcd_det_num, 1);
		lcd_kit_parse_array_data(np, "lcd-kit,pcd-check-reg-value",
			&disp_info->pcd_errflag.pcd_value);
	}
	lcd_kit_parse_u32(np, "lcd-kit,errflag-cmds-support",
		&disp_info->pcd_errflag.errflag_support, 0);
	if (disp_info->pcd_errflag.errflag_support)
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,errflag-read-cmds",
			"lcd-kit,errflag-read-cmds-state",
			&disp_info->pcd_errflag.read_errflag_cmds);
	lcd_kit_parse_u32(np, "lcd-kit,camera-prox", &disp_info->camera_prox, 0);
}

static void lcd_kit_parse_panel_dieid(int panel_id, struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,panel-dieid-support",
		&disp_info->panel_dieid_info.support, 0);

	if (disp_info->panel_dieid_info.support)
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-dieid-cmds",
			"lcd-kit,panel-dieid-cmds-state",
				&disp_info->panel_dieid_info.cmds);
}

static void lcd_kit_parse_power_off_optimize(int panel_id, struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,pwr-off-time-optimize-support",
		&disp_info->pwr_off_optimize_info.support, 0);

	if (disp_info->pwr_off_optimize_info.support)
		lcd_kit_parse_u32(np, "lcd-kit,pwr-off-time-work-delay",
			&disp_info->pwr_off_optimize_info.delay_time, 100);
}

static void lcd_kit_parse_panel_version_info(int panel_id, struct device_node *np)
{
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-version-enter-cmds",
		"lcd-kit,panel-version-enter-cmds-state",
		&disp_info->panel_version.enter_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-version-cmds",
		"lcd-kit,panel-version-cmds-state",
		&disp_info->panel_version.cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-version-exit-cmds",
		"lcd-kit,panel-version-exit-cmds-state",
		&disp_info->panel_version.exit_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-secend-version-enter-cmds",
		"lcd-kit,panel-secend-version-enter-cmds-state",
		&disp_info->panel_version.secend_enter_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-secend-version-cmds",
		"lcd-kit,panel-secend-version-cmds-state",
		&disp_info->panel_version.secend_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-secend-version-exit-cmds",
		"lcd-kit,panel-secend-version-exit-cmds-state",
		&disp_info->panel_version.secend_exit_cmds);
}

static void lcd_kit_parse_panel_version_value(int panel_id, struct device_node *np)
{
	disp_info->panel_version.value_number = 0;
	if (disp_info->panel_version.cmds.cmds->dlen == 1)
		disp_info->panel_version.value_number = 1;
	if (disp_info->panel_version.cmds.cmds->dlen > 1)
		disp_info->panel_version.value_number = disp_info->panel_version.cmds.cmds->dlen -
			disp_info->panel_version.cmds.cmds->payload[1];
	if (disp_info->panel_version.secend_cmds.cmds != NULL) {
		disp_info->panel_version.secend_value_number =
			disp_info->panel_version.secend_cmds.cmds->dlen -
			disp_info->panel_version.secend_cmds.cmds->payload[1];
		disp_info->panel_version.value_number += disp_info->panel_version.secend_value_number;
		LCD_KIT_INFO("value_num=%d secend_value_number = %d\n",
			disp_info->panel_version.value_number,
			disp_info->panel_version.secend_value_number);
	}
	lcd_kit_parse_arrays_data(np, "lcd-kit,panel-version-value",
		&disp_info->panel_version.value, disp_info->panel_version.value_number);
}

static void lcd_kit_parse_panel_version_name(int panel_id, struct device_node *np)
{
	int i;
	int ret = 0;
	char *name[VERSION_NUM_MAX] = {NULL};

	disp_info->panel_version.version_number = (u32)disp_info->panel_version.value.cnt;
	LCD_KIT_INFO("Panel version value_num = %d version_num = %d\n",
		disp_info->panel_version.value_number, disp_info->panel_version.version_number);
	if (disp_info->panel_version.version_number > 0) {
		ret = lcd_kit_parse_string_array(np, "lcd-kit,panel-version",
			(const char **)&name[0], disp_info->panel_version.version_number);
		if (ret < 0)
			LCD_KIT_INFO("Panel version parse fail\n");
		for (i = 0; i < (int)disp_info->panel_version.version_number; i++) {
			strncpy(disp_info->panel_version.lcd_version_name[i],
				name[i], LCD_PANEL_VERSION_SIZE - 1);
			LCD_KIT_INFO("Panel version name[%d] = %s\n",
				i, name[i]);
		}
	}
}

static void lcd_kit_parse_panel_version(int panel_id, struct device_node *np)
{
	/* panel version */
	lcd_kit_parse_u32(np, "lcd-kit,panel-version-support",
		&disp_info->panel_version.support, 0);
	if (disp_info->panel_version.support) {
		lcd_kit_parse_u32(np, "lcd-kit,panel-version-read-at-uefi",
			&disp_info->panel_version.read_at_uefi, 0);
		lcd_kit_parse_panel_version_info(panel_id, np);
		lcd_kit_parse_panel_version_value(panel_id, np);
		lcd_kit_parse_panel_version_name(panel_id, np);
	}
}

static void lcd_kit_parse_esd_info(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	unsigned int esd_support;

	esd_support = lcd_kit_get_esd_support(panel_id);
	if (esd_support) {
		pinfo->esd_enable = esd_support;
		disp_info->esd_enable = esd_support;
		lcd_kit_parse_u32(np, "lcd-kit,esd-recovery-max-count",
			&pinfo->esd_info.esd_recovery_max_count, 10);
		/* esd_check_max_count set 3 times as default */
		lcd_kit_parse_u32(np, "lcd-kit,esd-check-max-count",
			&pinfo->esd_info.esd_check_max_count, 3);
		lcd_kit_parse_u8(np, "lcd-kit,esd-timing-ctrl",
			&pinfo->esd_info.esd_timing_ctrl, 0);
		lcd_kit_parse_u32(np, "lcd-kit,esd-check-time-period",
			&pinfo->esd_info.esd_check_time_period, 0);
		lcd_kit_parse_u32(np, "lcd-kit,esd-first-check-delay",
			&pinfo->esd_info.esd_first_check_delay, 0);
	}
}

static void lcd_kit_parse_check_mipi(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,check-mipi-support",
		&disp_info->check_mipi.support, 0);

	if (disp_info->check_mipi.support) {
		LCD_KIT_INFO("lcd_kit_parse_check_mipi\n");
		lcd_kit_parse_u32(np, "lcd-kit,check-mipi-max-number",
			&disp_info->check_mipi.max_number, 0);
		lcd_kit_parse_array_data(np, "lcd-kit,check-mipi-judge-type",
			&disp_info->check_mipi.judge_type);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,check-mipi-cmds",
			"lcd-kit,check-mipi-cmds-state", &disp_info->check_mipi.cmds);
		lcd_kit_parse_array_data(np, "lcd-kit,check-mipi-value",
			&disp_info->check_mipi.value);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,check-mipi-sub-cmds",
			"lcd-kit,check-mipi-sub-cmds-state", &disp_info->check_mipi.sub_cmds);
		lcd_kit_parse_array_data(np, "lcd-kit,check-mipi-sub-value",
			&disp_info->check_mipi.sub_value);
	}
}

static void lcd_kit_parse_dirty_region(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	if (common_info->dirty_region.support) {
		LCD_KIT_INFO("dirty_region_support = %d %d\n", common_info->dirty_region.support);
		pinfo->user_info.dirty_region_updt_support = common_info->dirty_region.support;
		lcd_kit_parse_dirty_info(np, "lcd-kit,dirty-left-align",
			&pinfo->user_info.left_align);
		lcd_kit_parse_dirty_info(np, "lcd-kit,dirty-right-align",
			&pinfo->user_info.right_align);
		lcd_kit_parse_dirty_info(np, "lcd-kit,dirty-top-align",
			&pinfo->user_info.top_align);
		lcd_kit_parse_dirty_info(np, "lcd-kit,dirty-bott-align",
			&pinfo->user_info.bottom_align);
		lcd_kit_parse_dirty_info(np, "lcd-kit,dirty-width-align",
			&pinfo->user_info.w_align);
		lcd_kit_parse_dirty_info(np, "lcd-kit,dirty-height-align",
			&pinfo->user_info.h_align);
		lcd_kit_parse_dirty_info(np, "lcd-kit,dirty-width-min",
			&pinfo->user_info.w_min);
		lcd_kit_parse_dirty_info(np, "lcd-kit,dirty-height-min",
			&pinfo->user_info.h_min);
		lcd_kit_parse_dirty_info(np, "lcd-kit,dirty-top-start",
			&pinfo->user_info.top_start);
		lcd_kit_parse_dirty_info(np, "lcd-kit,dirty-bott-start",
			&pinfo->user_info.bottom_start);
	}
}

static void lcd_kit_parse_project_id(int panel_id, struct device_node *np)
{
	int ret = 0;

	lcd_kit_parse_u32(np, "lcd-kit,project-id-support",
		&disp_info->project_id.support, 0);

	if (disp_info->project_id.support) {
		disp_info->project_id.default_project_id = (char *)of_get_property(np,
			"lcd-kit,default-project-id", NULL);
		LCD_KIT_INFO("project_id is %s", disp_info->project_id.default_project_id);
	}
}

static void lcd_kit_parse_quick_power_on_fold_state_cmd(int id_index, int dsi_index, int cmd_index,
	int panel_id, struct device_node *np, struct dpu_panel_info *pinfo)
{
	int i, ppc_index;
	int ret;
	char buf[MAX_STR_LEN] = {0};
	char state_buf[MAX_STR_LEN] = {0};
	/* [id_index][dsi_index][cmd_index] */
	struct lcd_kit_dsi_panel_cmds *ppc_on_cmds = NULL;

	ret = snprintf_s(buf, MAX_STR_LEN, MAX_STR_LEN - 1, "lcd-kit,panel-fold-state%d-ver%d-dsi%d-cmds%d", id_index,
		common_info->ppc_para.soft_id_number, dsi_index, cmd_index);
	if (ret < 0) {
		LCD_KIT_ERR("%s snprintf cmd buf fail\n", __func__);
		return;
	}

	ret = snprintf_s(state_buf, MAX_STR_LEN, MAX_STR_LEN - 1, "lcd-kit,panel-fold-state%d-ver%d-dsi%d-cmds%d-state", id_index,
		common_info->ppc_para.soft_id_number, dsi_index, cmd_index);
	if (ret < 0) {
		LCD_KIT_ERR("%s snprintf state buf fail\n", __func__);
		return;
	}

	ppc_index = id_index * common_info->ppc_para.dsi_cnt * common_info->ppc_para.cmds_cnt +
		dsi_index * common_info->ppc_para.cmds_cnt + cmd_index;
	if (ppc_index < 0 || ppc_index >= common_info->ppc_para.id_cnt * common_info->ppc_para.dsi_cnt
		* common_info->ppc_para.cmds_cnt) {
			LCD_KIT_ERR("%s ppc_index overflow\n", __func__);
			return;
	}

	ppc_on_cmds = &common_info->ppc_on_cmds[ppc_index];
	if (!ppc_on_cmds) {
		LCD_KIT_ERR("ppc_on_cmds is null\n");
		return;
	}

	lcd_kit_parse_dcs_cmds(np, buf, state_buf, ppc_on_cmds);

	LCD_KIT_INFO("ppc_on_cmds cnt is %d\n", ppc_on_cmds->cmd_cnt);
}

static void lcd_kit_parse_online_switch_fold_state_cmd(int id_index, int dsi_index, int cmd_index,
	int panel_id, struct device_node *np, struct dpu_panel_info *pinfo)
{
	int i, cmd_cnt;
	int ret;
	char buf[MAX_STR_LEN] = {0};
	char state_buf[MAX_STR_LEN] = {0};
	/* [id_index][dsi_index][cmd_index] */
	struct lcd_kit_dsi_panel_cmds ppc_dsi_cmds;

	ret = snprintf_s(buf, MAX_STR_LEN, MAX_STR_LEN - 1, "lcd-kit,ppc-fold-state%d-ver%d-dsi%d-cmds%d", id_index,
		common_info->ppc_para.soft_id_number, dsi_index, cmd_index);
	if (ret < 0) {
		LCD_KIT_ERR("%s snprintf cmd buf fail\n", __func__);
		return;
	}

	ret = snprintf_s(state_buf, MAX_STR_LEN, MAX_STR_LEN - 1, "lcd-kit,ppc-fold-state%d-ver%d-dsi%d-cmds%d-state", id_index,
		common_info->ppc_para.soft_id_number, dsi_index, cmd_index);
	if (ret < 0) {
		LCD_KIT_ERR("%s snprintf state buf fail\n", __func__);
		return;
	}

	lcd_kit_parse_dcs_cmds(np, buf, state_buf, &ppc_dsi_cmds);
	cmd_cnt = ppc_dsi_cmds.cmd_cnt;
	pinfo->ppc_config_id_dsi_cmds[id_index][dsi_index].dsi_cmds[cmd_index].cnt = cmd_cnt;
	pinfo->ppc_config_id_dsi_cmds[id_index][dsi_index].dsi_cmds[cmd_index].cmds = kzalloc(cmd_cnt * sizeof(struct dsi_cmd_desc), GFP_KERNEL);
	if (!pinfo->ppc_config_id_dsi_cmds[id_index][dsi_index].dsi_cmds[cmd_index].cmds) {
		LCD_KIT_ERR("ppc_config_id_dsi_cmds is null\n");
		return;
	}

	LCD_KIT_INFO("ppc_dsi_cmds cnt is %d\n", cmd_cnt);
	for (i = 0; i < cmd_cnt; i++)
		lcd_kit_cmds_to_dsi_cmds(&ppc_dsi_cmds.cmds[i],
			&pinfo->ppc_config_id_dsi_cmds[id_index][dsi_index].dsi_cmds[cmd_index].cmds[i], true);
}

static void lcd_kit_parse_fold_state_cmd_sub(int id_index, int dsi_index, int cmd_index,
	int panel_id, struct device_node *np, struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_quick_power_on_fold_state_cmd(id_index, dsi_index, cmd_index,
		panel_id, np, pinfo);

	lcd_kit_parse_online_switch_fold_state_cmd(id_index, dsi_index, cmd_index,
		panel_id, np, pinfo);
}

static int lcd_kit_init_ppc_para(int panel_id, struct dpu_panel_info *pinfo)
{
	common_info->ppc_on_cmds = kzalloc(PPC_CONFIG_ID_CNT * PPC_CONFIG_ID_DSI_CNT
		* PPC_CONFIG_ID_CMDS_CNT * sizeof(struct lcd_kit_dsi_panel_cmds), GFP_KERNEL);
	if (!common_info->ppc_on_cmds) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}

	common_info->ppc_para.id_cnt = PPC_CONFIG_ID_CNT;
	common_info->ppc_para.dsi_cnt = PPC_CONFIG_ID_DSI_CNT;
	common_info->ppc_para.cmds_cnt = PPC_CONFIG_ID_CMDS_CNT;
	common_info->ppc_para.ppc_config_id = PPC_CONFIG_ID_G_MODE;
	common_info->ppc_para.panel_partial_ctrl_support = pinfo->panel_partial_ctrl_support;

	return LCD_KIT_OK;
}

static void lcd_kit_parse_fold_state_cmd(int panel_id, struct device_node *np, struct dpu_panel_info *pinfo)
{
	int id_index, dsi_index, cmd_index;
	int ret;

	// common_info and pinfo share same config fold state
	ret = lcd_kit_init_ppc_para(panel_id, pinfo);
	if (ret == LCD_KIT_FAIL) {
		LCD_KIT_ERR("init ppc para fail\n");
		return;
	}

	for (id_index = 0; id_index < PPC_CONFIG_ID_CNT; id_index++) {
		for (dsi_index = 0; dsi_index < PPC_CONFIG_ID_DSI_CNT; dsi_index++) {
			for (cmd_index = 0; cmd_index < PPC_CONFIG_ID_CMDS_CNT; cmd_index++) {
				LCD_KIT_INFO("fold test id_index is %d, dsi_index is %d, cmd_index  is is %d\n", id_index, dsi_index, cmd_index);
				lcd_kit_parse_fold_state_cmd_sub(id_index, dsi_index, cmd_index, panel_id, np, pinfo);
			}
		}
	}
}
 
static void lcd_kit_parse_fold_ppc_config_id(int panel_id, struct device_node *np, struct dpu_panel_info *pinfo)
{
	int id_index, rect_index;
	int ret;
	char buf[MAX_STR_LEN] = {0};
	/* [fold_state] */
	struct lcd_kit_array_data fold_state_id_rect3[PPC_CONFIG_ID_CNT];
 
	for (id_index = 0; id_index < PPC_CONFIG_ID_CNT; id_index++) {
		ret = memset_s(buf, sizeof(buf), 0, sizeof(buf));
		if (ret < 0) {
			LCD_KIT_ERR("%s memset config id buf fail\n", __func__);
			return;
		}
 
		ret = snprintf_s(buf, MAX_STR_LEN, MAX_STR_LEN - 1, "lcd-kit,panel-fold-state%d-id-rect", id_index);
		if (ret < 0) {
			LCD_KIT_ERR("%s snprintf config id buf fail\n", __func__);
			return;
		}
 
		lcd_kit_parse_array_data(np, buf, &fold_state_id_rect3[id_index]);
	}
 
	for (id_index = 0, rect_index = 0; id_index < PPC_CONFIG_ID_CNT; id_index++, rect_index = 0) {
		pinfo->ppc_config_id_rect_info[id_index].id = fold_state_id_rect3[id_index].buf[rect_index++];
		pinfo->ppc_config_id_rect_info[id_index].rect.left = fold_state_id_rect3[id_index].buf[rect_index++];
		pinfo->ppc_config_id_rect_info[id_index].rect.top = fold_state_id_rect3[id_index].buf[rect_index++];
		pinfo->ppc_config_id_rect_info[id_index].rect.right = fold_state_id_rect3[id_index].buf[rect_index++];
		pinfo->ppc_config_id_rect_info[id_index].rect.bottom = fold_state_id_rect3[id_index].buf[rect_index++];
	}
}
 
static void lcd_kit_parse_fold_state_param(int panel_id, struct device_node *np, struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,panel-local-partial-support", &pinfo->panel_partial_ctrl_support, 0);
	if (!pinfo->panel_partial_ctrl_support) {
		LCD_KIT_INFO("panel partial ctrl not support\n");
		return;
	}

	lcd_kit_parse_u32(np, "lcd-kit,ppc-software-id-support", &common_info->ppc_para.soft_id_support, 0);
	LCD_KIT_INFO("ppc_software_id_support = %d\n", common_info->ppc_para.soft_id_support);

	lcd_kit_parse_u32(np, "lcd-kit,ppc-enable-panel-estv-support", &pinfo->ppc_enable_panel_estv_support, 0);
#ifdef LCD_FACTORY_MODE
	pinfo->ppc_enable_panel_estv_support = 0;
#endif

	pinfo->ppc_enable_panel_estv_support_factory = 0;
#ifdef LCD_FACTORY_MODE
	lcd_kit_parse_u32(np, "lcd-kit,ppc-enable-panel-estv-support-factory", &pinfo->ppc_enable_panel_estv_support_factory, 0);
	lcd_kit_parse_u32(np, "lcd-kit,ppc-panel-estv-wait-te-cnt", &pinfo->ppc_panel_estv_wait_te_cnt, 0);
#endif

	lcd_kit_parse_u32(np, "lcd-kit,hs-pkt-discontin-support", &pinfo->hs_pkt_discontin_support, 0);
	lcd_kit_parse_fold_state_cmd(panel_id, np, pinfo);
	lcd_kit_parse_fold_ppc_config_id(panel_id, np, pinfo);
}

static void lcd_kit_panel_parse_tcon_update_info(int panel_id, struct device_node *np)
{
	/* tcon update support */
	lcd_kit_parse_u32(np, "lcd-kit,tcon-update-support",
		&disp_info->update_info.support, 0);
	if (disp_info->update_info.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,update-get-version-cmds",
			"lcd-kit,update-get-version-cmds-state",
			&disp_info->update_info.get_version_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,update-get-status-cmds",
			"lcd-kit,update-get-status-cmds-state",
			&disp_info->update_info.get_status_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,update-start-cmds",
			"lcd-kit,update-start-cmds-state",
			&disp_info->update_info.start_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,update-end-cmds",
			"lcd-kit,update-end-cmds-state",
			&disp_info->update_info.end_cmds);
	}
}

static void lcd_kit_parse_special_config(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u8(np, "lcd-kit,updt-fps-te-support",
		&pinfo->panel_spcl_cfg.lcd_updt_fps_te_support, 0);
	lcd_kit_parse_u8(np, "lcd-kit,panel-pfm-support",
		&pinfo->panel_spcl_cfg.lcd_updt_fps_pfm_support, 0);
}

static void lcd_kit_parse_el_ctrl_cmds(int panel_id, struct device_node *np)
{
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-el-ctrl-open-cmds",
			"lcd-kit,panel-el-ctrl-open-cmds-state", &common_info->el_ctrl_info.el_ctrl_open_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-el-ctrl-close-cmds",
			"lcd-kit,panel-el-ctrl-close-cmds-state", &common_info->el_ctrl_info.el_ctrl_close_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-el-ctrl-cmds",
			"lcd-kit,panel-el-ctrl-cmds-state", &common_info->el_ctrl_info.elvss_ctrl_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-vint2-ctrl-cmds",
			"lcd-kit,panel-vint2-ctrl-cmds-state", &common_info->el_ctrl_info.vint2_ctrl_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-lut-inputtable-read-cmds",
			"lcd-kit,panel-lut-inputtable-read-cmds-state", &common_info->el_ctrl_info.lut_input_read_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-elvss-outputtable-read-cmds",
			"lcd-kit,panel-elvss-outputtable-read-cmds-state", &common_info->el_ctrl_info.elvss_output_read_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-vint2-outputtable-read-cmds",
			"lcd-kit,panel-vint2-outputtable-read-cmds-state", &common_info->el_ctrl_info.vint2_output_read_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-elvss-imterpolation-cmds",
			"lcd-kit,panel-elvss-imterpolation-cmds-state", &common_info->el_ctrl_info.elvss_interpolation_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-vint2-imterpolation-cmds",
			"lcd-kit,panel-vint2-imterpolation-cmds-state", &common_info->el_ctrl_info.vint2_interpolation_cmds);
	lcd_kit_parse_array_data(np, "lcd-kit,panel-el-ctrl-oem-value",
			&common_info->el_ctrl_info.el_ctrl_oem_value);
	lcd_kit_parse_array_data(np, "lcd-kit,panel-el-ctrl-level",
			&common_info->el_ctrl_info.el_ctrl_level);
}

static void lcd_kit_parse_el_ctrl_info(int panel_id, struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,panel-el-ctrl-support", &common_info->el_ctrl_info.support, 0);
	if (!common_info->el_ctrl_info.support) {
		LCD_KIT_WARNING("panel el ctrl not support\n");
		return;
	}
	lcd_kit_parse_u32(np, "lcd-kit,panel-el-ctrl-form-length",
		&common_info->el_ctrl_info.el_ctrl_forms.el_ctrl_form_len, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-el-ctrl-ddic-type",
		&common_info->el_ctrl_info.ddic_type, 1);
	lcd_kit_parse_u32(np, "lcd-kit,panel-el-ctrl-bl-threshold",
		&common_info->el_ctrl_info.bl_threshold, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-el-ctrl-als-threshold",
		&common_info->el_ctrl_info.als_threshold, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-el-ctrl-temp-threshold",
		&common_info->el_ctrl_info.temp_threshold, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-el-ctrl-elvss-min-step",
		&common_info->el_ctrl_info.elvss_min_step, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-el-ctrl-diming-fast-mode",
		&common_info->el_ctrl_info.diming_fast_mode, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-el-ctrl-diming-mid-mode",
		&common_info->el_ctrl_info.diming_med_mode, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-el-ctrl-diming-slow-mode",
		&common_info->el_ctrl_info.diming_slow_mode, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-el-ctrl-min-offset-bl",
		&common_info->el_ctrl_info.min_offset_brightness, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-el-ctrl-max-offset-bl",
		&common_info->el_ctrl_info.max_offset_brightness, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-el-ctrl-min-offset-value",
		&common_info->el_ctrl_info.min_offset_value, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-el-ctrl-max-offset-value",
		&common_info->el_ctrl_info.max_offset_value, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-el-ctrl-bias-reset-bl",
		&common_info->el_ctrl_info.restore_bias_brightness, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-el-ctrl-elvss-min",
		&common_info->el_ctrl_info.elvss_min, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-el-ctrl-elvss-max",
		&common_info->el_ctrl_info.elvss_max, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-el-ctrl-vint2-min",
		&common_info->el_ctrl_info.vint2_min, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-el-ctrl-vint2-max",
		&common_info->el_ctrl_info.vint2_max, 0);

	lcd_kit_parse_el_ctrl_cmds(panel_id, np);
}

static void lcd_kit_init_pinfo(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return;
	}

	lcd_kit_parse_base_pinfo(panel_id, np, pinfo);
	lcd_kit_parse_mipi(np, pinfo);
	lcd_kit_parse_bl_info(np, pinfo);
	lcd_kit_parse_dsc_param(np, pinfo);
	lcd_kit_parse_pinfo_fps(panel_id, np, pinfo);
	lcd_kit_parse_utils(panel_id, np, pinfo);
	lcd_kit_parse_panel_dieid(panel_id, np);
	lcd_kit_parse_power_off_optimize(panel_id, np);
	lcd_kit_parse_panel_version(panel_id, np);
	lcd_kit_parse_check_mipi(panel_id, np, pinfo);
	lcd_kit_parse_esd_info(panel_id, np, pinfo);
	lcd_kit_parse_dirty_region(panel_id, np, pinfo);
	lcd_kit_parse_spr_info(panel_id, np, pinfo);
	lcd_kit_parse_project_id(panel_id, np);
	lcd_kit_panel_parse_tcon_update_info(panel_id, np);
	lcd_kit_parse_special_config(panel_id, np, pinfo);
	lcd_kit_parse_fold_state_param(panel_id, np, pinfo);
	lcd_kit_parse_el_ctrl_info(panel_id, np);
}

bool lcd_kit_support(void)
{
	ssize_t ret;
	struct device_node *lcdkit_np = NULL;
	const char *support_type = NULL;

	lcdkit_np = of_find_compatible_node(NULL, NULL, DTS_COMP_LCD_KIT_PANEL_TYPE);
	if (!lcdkit_np) {
		LCD_KIT_ERR("not find device node!\n");
		return false;
	}
	ret = of_property_read_string(lcdkit_np, "support_lcd_type", &support_type);
	if (ret) {
		LCD_KIT_ERR("failed to get support_lcd_type\n");
		return false;
	}
	if (!strncmp(support_type, "LCD_KIT", strlen("LCD_KIT"))) {
		LCD_KIT_INFO("lcd_kit is support!\n");
		return true;
	} else {
		LCD_KIT_INFO("lcd_kit is not support!\n");
		return false;
	}
}

static int lcd_kit_get_project_id(char *buff)
{
	int panel_id = lcd_kit_get_active_panel_id();

	if (lcd_kit_get_display_type() != NORMAL_TYPE)
		panel_id = SECONDARY_PANEL;

	if (!buff) {
		LCD_KIT_ERR("buff is null\n");
		return LCD_KIT_FAIL;
	}
	if (!disp_info->project_id.support) {
		LCD_KIT_ERR("project_id is not support\n");
		return LCD_KIT_FAIL;
	}
	if (disp_info->project_id.default_project_id) {
		strncpy(buff, disp_info->project_id.default_project_id, strlen(disp_info->project_id.default_project_id));
		return LCD_KIT_OK;
	}
	return LCD_KIT_FAIL;
}

static int lcd_kit_get_online_status(int panel_id)
{
	int status = LCD_ONLINE;

	if (!strncmp(disp_info->compatible, LCD_KIT_DEFAULT_PANEL,
		strlen(disp_info->compatible)))
		status = LCD_OFFLINE;
	LCD_KIT_INFO("online status = %d\n", status);
	return status;
}

int lcd_kit_get_status_by_type(int type, int *status)
{
	int ret;
	int panel_id = lcd_kit_get_active_panel_id();

	if (status == NULL) {
		LCD_KIT_ERR("status is null\n");
		return LCD_KIT_FAIL;
	}
	if (type == LCD_ONLINE_TYPE) {
		*status = lcd_kit_get_online_status(panel_id);
		ret = LCD_KIT_OK;
	} else if (type == PT_STATION_TYPE) {
#ifdef LCD_FACTORY_MODE
		*status = lcd_kit_get_pt_station_status(panel_id);
#endif
		ret = LCD_KIT_OK;
	} else {
		LCD_KIT_ERR("not support type\n");
		ret = LCD_KIT_FAIL;
	}

	return ret;
}

void lcd_kit_mipi_error_dmd_report(
	u8 reg_val[MIPI_DETECT_LOOP][MIPI_DETECT_NUM][VAL_NUM], int panel_id)
{
	int ret;
	char err_info[DMD_DET_ERR_LEN] = { 0 };
	uint32_t dmd_number;

	LCD_KIT_INFO("mipi detect DMD report\n");
	if (!reg_val) {
		LCD_KIT_ERR("reg_val is NULL\n");
		return;
	}
	ret = snprintf(err_info, DMD_DET_ERR_LEN,
		"panel = %s mipi error detect error", disp_info->compatible);
	dmd_number = (panel_id == PRIMARY_PANEL) ?
		DSM_LCD_MIPI_CHECK_ERROR_NO : DSM_LCD_SUB_MIPI_CHECK_ERROR_NO;
	lcd_dmd_report_err(dmd_number, err_info, DMD_DET_ERR_LEN);
	LCD_KIT_INFO(" with DSM_LCD_MIPI_TRANSMIT_ERROR_NO \n");
}

void lcd_kit_check_sum2_dmd_report(
	u8 reg_val[CHECK_SUM2_NUM][CHECKSUM2_VAL_NUM], int panel_id)
{
	int ret;
	char err_info[DMD_DET_ERR_LEN] = { 0 };
	uint32_t dmd_number;

	LCD_KIT_INFO("check_sum2 detect DMD report\n");
	if (!reg_val) {
		LCD_KIT_ERR("reg_val is NULL\n");
		return;
	}
	ret = snprintf(err_info, DMD_DET_ERR_LEN,
		"panel = %s check sum2 detect error", disp_info->compatible);
	dmd_number = (panel_id == PRIMARY_PANEL) ?
		DSM_LCD_CHECKSUM_ERROR_NO : DSM_LCD_SUB_CHECKSUM_ERROR_NO;
	lcd_dmd_report_err(dmd_number, err_info, DMD_DET_ERR_LEN);

	LCD_KIT_INFO(" with DSM_LCD_CHECKSUM_ERROR_NO \n");
}

void lcd_kit_mbist_detect_dmd_report(
	u8 reg_val[MBIST_LOOPS][DETECT_NUM][VAL_NUM], int panel_id)
{
	int ret;
	char err_info[DMD_DET_ERR_LEN] = { 0 };
	uint32_t dmd_number;

	if (!reg_val) {
		LCD_KIT_ERR("reg_val is NULL\n");
		return;
	}
	ret = snprintf(err_info, DMD_DET_ERR_LEN,
		"panel = %s mbist detect error", disp_info->compatible);
	dmd_number = (panel_id == PRIMARY_PANEL) ?
		DSM_LCD_MBIST_ERROR_NO : DSM_LCD_SUB_MBIST_ERROR_NO;
	lcd_dmd_report_err(dmd_number, err_info, DMD_DET_ERR_LEN);

	LCD_KIT_INFO(" with DSM_LCD_ESD_STATUS_ERROR_NO \n");
}

void lcd_kit_sn_reprocess(uint32_t panel_id, char *buff, unsigned int len)
{
	int i;
	int j = 0;

	if (!buff) {
		LCD_KIT_ERR("buff is NULL\n");
		return;
	}
	if (!common_info->sn_code.reprocess_support) {
		LCD_KIT_INFO("no need reprocess\n");
		return;
	}
	if (len < SN_CODE_23_NUMB_SIZE) {
		LCD_KIT_ERR("len is error\n");
		return;
	}

	for (i = 0; (i < SN_CODE_23_NUMB_SIZE) && (j < SN_CODE_2_NONE_SIZE); i++) {
		LCD_KIT_INFO("sn read:%d-%d\n", i, buff[i]);
		if (buff[i] == 0) {
			buff[i] = '0';
			LCD_KIT_INFO("sn reprocess:%d-%d\n", i, buff[i]);
			j++;
		}
	}
	return;
}

int get_sn_code_multiple_reads(int panel_id, unsigned char *out)
{
	int ret;
	int i;
	int j;
	char value[SN_CODE_FRONT_BIT + 1] = {0};

	for (i = 0; i < SN_CODE_READ_TIMES; i++) {
		ret = lcd_kit_cmds_rx(panel_id, NULL, value, SN_CODE_FRONT_BIT,
			&disp_info->oeminfo.barcode_2d.sn_cmds[i]);
		if (ret != 0) {
			LCD_KIT_ERR("get sn_code %d times read error!\n", i);
			continue;
		}
		for (j = 0; j < SN_CODE_FRONT_BIT; j++) // Inverted value
			out[SN_CODE_FRONT_BIT * i + SN_CODE_FRONT_BIT - 1 - j] = value[j];
	}
	return LCD_KIT_OK;
}

static int lcd_kit_get_sn_code(int panel_id)
{
	int ret;
	char read_value[OEM_INFO_SIZE_MAX + 1] = {0};

	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return LCD_KIT_FAIL;
	}
	if (common_info && common_info->sn_code.support) {
		if (disp_info && disp_info->oeminfo.barcode_2d.support) {
			if (disp_info->oeminfo.multiple_reads)
				ret = get_sn_code_multiple_reads(panel_id, read_value);
			else
				ret = lcd_kit_cmds_rx(panel_id, NULL, read_value,
					OEM_INFO_SIZE_MAX,
					&disp_info->oeminfo.barcode_2d.cmds);
			if (ret != 0) {
				LCD_KIT_ERR("get sn_code error!\n");
				return LCD_KIT_FAIL;
			}
			LCD_KIT_INFO("do lcd_kit_sn_reprocess\n");
			lcd_kit_sn_reprocess(panel_id, read_value, OEM_INFO_SIZE_MAX);
			memcpy(disp_info->sn_code, read_value, LCD_KIT_SN_CODE_LENGTH);
			disp_info->sn_code_length = LCD_KIT_SN_CODE_LENGTH;
			return LCD_KIT_OK;
		}
	}
	return LCD_KIT_FAIL;
}

static int lcd_get_panel_dieid_info(uint32_t panel_id, char *buff)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = lcd_kit_get_adapt_ops();

	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!adapt_ops->mipi_rx) {
		LCD_KIT_ERR("mipi_rx or mipi_tx is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!buff) {
		LCD_KIT_ERR("buff is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return LCD_KIT_FAIL;
	}
	ret = adapt_ops->mipi_rx(panel_id, NULL, buff,	PANEL_DIEID_DATA_SIZE_MAX,
		&disp_info->panel_dieid_info.cmds);
	if (ret) {
		LCD_KIT_ERR("get panel dieid info fail\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

int lcd_kit_power_off_event_skip_send(int panel_id, void *hld, uint32_t event,
	uint32_t enable)
{
	if (!common_ops->is_power_event)
		return false;
	return disp_info->pwr_off_optimize_info.support && g_skip_power_on_off &&
		(enable == 0) && common_ops->is_power_event(event);
}

void lcd_kit_notify_panel_switch(void)
{
	if (PUBLIC_INFO->product_type == NORMAL_TYPE) {
		LCD_KIT_ERR("normal type not notify panel switch\n");
		return;
	}
	LCD_KIT_INFO("notify panel switch\n");
	g_skip_power_on_off = true;
}

bool lcd_kit_get_skip_power_on_off(void)
{
	return g_skip_power_on_off;
}

void lcd_kit_set_skip_power_on_off(bool status)
{
	g_skip_power_on_off = status;
}

static int lcd_kit_panel_version_read_secend(uint32_t panel_id, struct lcd_kit_adapt_ops *adapt_ops)
{
	int ret = LCD_KIT_OK;

	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return LCD_KIT_FAIL;
	}

	if (disp_info->panel_version.secend_enter_cmds.cmds != NULL) {
		ret = adapt_ops->mipi_tx(panel_id, NULL, &disp_info->panel_version.secend_enter_cmds);
		if (ret)
			return LCD_KIT_FAIL;
	}
	if (disp_info->panel_version.secend_cmds.cmds != NULL) {
		ret = adapt_ops->mipi_rx(panel_id, NULL,
			(uint8_t *)disp_info->panel_version.read_value +
				(int)disp_info->panel_version.value_number -
				disp_info->panel_version.secend_value_number,
			VERSION_VALUE_NUM_MAX - 1,
			&disp_info->panel_version.secend_cmds);
		if (ret)
			return LCD_KIT_FAIL;
	}
	if (disp_info->panel_version.secend_exit_cmds.cmds != NULL) {
		ret = adapt_ops->mipi_tx(panel_id, NULL, &disp_info->panel_version.secend_exit_cmds);
		if (ret)
			return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

static int lcd_kit_panel_version_read(uint32_t panel_id)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = lcd_kit_get_adapt_ops();

	if (panel_id != PRIMARY_PANEL) {
		LCD_KIT_ERR("not support start up to read other panel \n");
		return LCD_KIT_FAIL;
	}
	if (!adapt_ops || !adapt_ops->mipi_rx || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("mipi_rx or mipi_tx is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return LCD_KIT_FAIL;
	}

	if (disp_info->panel_version.enter_cmds.cmds != NULL) {
		ret = adapt_ops->mipi_tx(panel_id, NULL, &disp_info->panel_version.enter_cmds);
		if (ret)
			return LCD_KIT_FAIL;
	}
	ret = adapt_ops->mipi_rx(panel_id, NULL, (uint8_t *)disp_info->panel_version.read_value,
		VERSION_VALUE_NUM_MAX - 1, &disp_info->panel_version.cmds);
	if (ret) {
		LCD_KIT_ERR("mipi read panel version info fail\n");
		return LCD_KIT_FAIL;
	}
	if (disp_info->panel_version.exit_cmds.cmds != NULL) {
		ret = adapt_ops->mipi_tx(panel_id, NULL,
			&disp_info->panel_version.exit_cmds);
		if (ret)
			return LCD_KIT_FAIL;
	}
	ret = lcd_kit_panel_version_read_secend(panel_id, adapt_ops);
	return LCD_KIT_OK;
}

static void panel_version_cat(uint32_t panel_id, u32 i)
{
	memcpy(disp_info->panel_version.lcd_panel_version,
		" VER:", strlen(" VER:") + 1);
	strncat(disp_info->panel_version.lcd_panel_version,
		disp_info->panel_version.lcd_version_name[i],
		strlen(disp_info->panel_version.lcd_version_name[i]));
	LCD_KIT_INFO("Panel version is %s\n",
		disp_info->panel_version.lcd_panel_version);
}

static int lcd_kit_parse_panel_version_by_uefi(uint32_t panel_id)
{
	unsigned int panel_version;
	int i;
	int index;

	if (panel_id == PRIMARY_PANEL)
		panel_version = PUBLIC_INFO->primary_panel_version;
	else if (panel_id == SECONDARY_PANEL)
		panel_version = PUBLIC_INFO->secondary_panel_version;

	if (!panel_version) {
		LCD_KIT_ERR("uefi panel version is null\n");
		return LCD_KIT_FAIL;
	}

	for (i = 0; i < disp_info->panel_version.value_number; i++) {
		index = disp_info->panel_version.value_number - i - 1;
		disp_info->panel_version.read_value[index] = panel_version & 0xFF;
		panel_version = panel_version >> 8;
	}

	return LCD_KIT_OK;
}

static int panel_version_compare(uint32_t panel_id)
{
	unsigned int i;
	unsigned int j;

	for (i = 0; i < disp_info->panel_version.version_number; i++) {
		for (j = 0; j < disp_info->panel_version.value_number; j++) {
			LCD_KIT_INFO("read_value[%d]:0x%x, expected_value[%d].buf[%d]:0x%x\n",
				j, disp_info->panel_version.read_value[j], i, j,
				disp_info->panel_version.value.arry_data[i].buf[j]);
			if (disp_info->panel_version.read_value[j] !=
				disp_info->panel_version.value.arry_data[i].buf[j])
				break;

			if (j == (disp_info->panel_version.value_number - 1)) {
				panel_version_cat(panel_id, i);
				return LCD_KIT_OK;
			}
		}
	}

	LCD_KIT_INFO("panel_version not find\n");
	return LCD_KIT_FAIL;
}

void lcd_kit_color_calib_check(int panel_id)
{
	common_info->c_calib.need_color_calib = PUBLIC_INFO->need_color_calib;
	LCD_KIT_INFO("need_color_calib = %u",
		common_info->c_calib.need_color_calib);
}

int lcd_kit_panel_version_init(int panel_id)
{
	int ret;
	if (!disp_info->panel_version.support) {
		LCD_KIT_INFO("not support panel version\n");
		return LCD_KIT_FAIL;
	}

	if (disp_info->panel_version.read_at_uefi)
		ret = lcd_kit_parse_panel_version_by_uefi(panel_id);
	else
		ret = lcd_kit_panel_version_read(panel_id);

	if (ret) {
		LCD_KIT_ERR("read panel version cmd fail\n");
		return LCD_KIT_FAIL;
	}
	ret = panel_version_compare(panel_id);
	return ret;
}

static void parse_esd_enable(int panel_id, const char *command)
{
	if (strncmp("esd_enable:", command, strlen("esd_enable:")))
		return;
	if (command[strlen("esd_enable:")] == '0')
		lcd_esd_enable(panel_id, 0);
	else
		lcd_esd_enable(panel_id, 1);
}


int lcd_kit_parse_switch_cmd(int panel_id, const char *command)
{
	parse_esd_enable(panel_id, command);
	return LCD_KIT_OK;
}

int lcd_kit_get_aod_panel_state(int panel_id)
{
#if defined(CONFIG_DKMD_DPU_AOD)
	int aod_state = dkmd_get_lcd_status();

	if (panel_id == PRIMARY_PANEL)
		return aod_state & BIT(PRIMARY_PANEL_AOD_BIT);
	else if (panel_id == SECONDARY_PANEL)
		return aod_state & BIT(SECONDARY_PANEL_AOD_BIT);
	else
		return 0;
#else
	return 0;
#endif
}

void lcd_kit_set_aod_panel_state(int panel_id, uint32_t status)
{
	LCD_KIT_INFO("+\n");
#if defined(CONFIG_DKMD_DPU_AOD)
	if (panel_id == PRIMARY_PANEL)
		dkmd_set_lcd_status(PRIMARY_PANEL_AOD_BIT, status);
	else if (panel_id == SECONDARY_PANEL)
		dkmd_set_lcd_status(SECONDARY_PANEL_AOD_BIT, status);
	else
		return;
#else
	return;
#endif
}

static bool is_in_power_off_seq(int panel_id)
{
	return disp_info->lcd_power_seq >= LCD_KIT_POWER_OFF_HS_SEQ &&
		disp_info->lcd_power_seq <= LCD_KIT_POWER_OFF;
}

static bool lcd_kit_panel_event_skip_delay(int panel_id, void *hld,
	uint32_t event, uint32_t data)
{
	return lcd_kit_get_aod_panel_state(panel_id) && !is_in_power_off_seq(panel_id);
}

int lcd_kit_get_power_status(int panel_id)
{
	return lcd_kit_get_aod_panel_state(panel_id);
}

int lcd_kit_dvdd_hpm_ctrl(void)
{
	int ret = LCD_KIT_OK;
	int panel_id = lcd_kit_get_active_panel_id();

	if (PUBLIC_INFO->dvdd_avs_volt > 0)
		LCD_KIT_INFO("lcd dvdd voltage hpm_val = %d\n", PUBLIC_INFO->dvdd_avs_volt);

	return ret;
}

static int lcd_kit_dvdd_vmin_set(int panel_id, struct lcd_kit_dsi_panel_cmds *dvdd_cmds)
{
	struct lcd_kit_array_data *dvdd_vmin_value = &common_info->avs_dvdd.dvdd_vmin_value;
	struct lcd_kit_array_data *dvdd_vmin_cmds = &common_info->avs_dvdd.dvdd_vmin_cmds;
	uint32_t dvdd_avs_vmin = PUBLIC_INFO->dvdd_avs_vmin;
	uint32_t horaxis;
	uint32_t vertaxis;
	uint32_t ret = LCD_KIT_FAIL;
	uint32_t i;
	if (!dvdd_avs_vmin || !dvdd_vmin_value || !dvdd_vmin_cmds) {
		LCD_KIT_ERR("avs number or struct is abnormal\n");
		return LCD_KIT_FAIL;
	}
	if (common_info->avs_dvdd.dvdd_vmin_cmds_position.cnt != LCD_POS_MAX) {
		LCD_KIT_ERR("horaxis or vertaxis is null\n");
		return LCD_KIT_FAIL;
	}
	horaxis = common_info->avs_dvdd.dvdd_vmin_cmds_position.buf[LCD_HOR_INDEX];
	vertaxis = common_info->avs_dvdd.dvdd_vmin_cmds_position.buf[LCD_VER_INDEX];

	for (i = 0; i < dvdd_vmin_value->cnt; i++) {
		if (dvdd_avs_vmin == dvdd_vmin_value->buf[i]) {
			dvdd_cmds->cmds[vertaxis].payload[horaxis] = dvdd_vmin_cmds->buf[i];
			LCD_KIT_INFO("dvdd_cmds->cmds[%d].payload[%d] = %x\n", vertaxis, horaxis, dvdd_cmds->cmds[vertaxis].payload[horaxis]);
			ret = LCD_KIT_OK;
			break;
		}
	}
	return ret;
}
 
static int lcd_kit_dvdd_avs_ctrl(int panel_id, void *hld)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	struct lcd_kit_dsi_panel_cmds *dvdd_cmds = &common_info->avs_dvdd.dvdd_on_tx_cmds;
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!common_info->avs_dvdd.lcd_avs_support) {
		LCD_KIT_ERR("not support avs\n");
		return LCD_KIT_FAIL;
	}
	if (!adapt_ops || !dvdd_cmds) {
		LCD_KIT_ERR("error, exit avs\n");
		return LCD_KIT_FAIL;
	}
	ret = lcd_kit_dvdd_vmin_set(panel_id, dvdd_cmds);
	if (ret) {
		LCD_KIT_ERR("set vmin failed\n");
		return LCD_KIT_FAIL;
	}
	if (!adapt_ops->mipi_rx || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("mipi_rx or mipi_tx is null\n");
		return LCD_KIT_FAIL;
	}
	ret = adapt_ops->mipi_tx(panel_id, hld, dvdd_cmds);
	if (ret) {
		LCD_KIT_ERR("dvdd_avs_vmin wirte error\n");
		return LCD_KIT_FAIL;
	}
	return ret;
}

static bool lcd_kit_exec_ext_ts(void)
{
	if (lcd_kit_get_display_type() != NORMAL_TYPE)
		return true;
	return false;
}

static int lcd_kit_eink_power_ctrl(int enable)
{
	return LCD_KIT_FAIL;
}

#define MAX_EL_CTRL_INFO_LEN 1024
#define LUT_READ_STEP 3
#define LUT_READ_STEP_1 8
#define LUT_READ_STEP_2 4
#define LUT_FORM_STEP 2
static int lcd_kit_get_lut_input_form(int panel_id, char *read_value, int length)
{
	uint32_t *lut_table = NULL;
	int i, j;
	int lut_offset;

	switch (common_info->el_ctrl_info.ddic_type) {
	case DDIC_TYPE_CXX:
		lut_offset = 0;
		break;
	case DDIC_TYPE_HXX:
		lut_offset = 2;
		break;
	case DDIC_TYPE_NONE:
		LCD_KIT_ERR("ddic type invalid\n");
		break;
	default:
		lut_offset = 2;
		break;
	}

	lut_table = (uint32_t *)kzalloc(length * sizeof(uint32_t), GFP_KERNEL);
	if (!lut_table) {
		LCD_KIT_ERR("lut table is NULL\n");
		return LCD_KIT_FAIL;
	}

/*
 *  el ctrl lut input form example
 *  read_value:   bit0-bit11  bit12-bit23  bit24-bit35  bit36-bit47 ...
 *  input_form:   form[0]     form[1]      form[2]      form[4]     ...
 */
	for (i = 0, j = 0; i < MAX_EL_CTRL_INFO_LEN && j < length; i = i + LUT_READ_STEP, j = j + LUT_FORM_STEP) {
		lut_table[j] = (read_value[i] + ((read_value[i + 1] & 0x0F) << LUT_READ_STEP_1)) << lut_offset;
		if (j + 1 < length)
			lut_table[j + 1] = (((read_value[i + 1] & 0xF0) >> LUT_READ_STEP_2) + (read_value[i + 2] << LUT_READ_STEP_2)) << lut_offset;
	}
	lut_table[0] = 0;
	common_info->el_ctrl_info.el_ctrl_forms.lut_form = lut_table;
	return LCD_KIT_OK;
}

static int lcd_kit_get_elvss_input_form(int panel_id, char *read_value, int length)
{
	uint32_t *elvss_input_form = NULL;
	int i;

	elvss_input_form = (uint32_t *)kzalloc(length * sizeof(uint32_t), GFP_KERNEL);
	if (!elvss_input_form) {
		LCD_KIT_ERR("elvss input form is NULL\n");
		return LCD_KIT_FAIL;
	}
/*
 *  el ctrl elvss input form example
 *  read_value:   value[0]  value[1]  value[2]  value[3]  value[4]  value[5] ...
 *  input_form:   form[0]   form[1]   form[2]   form[3]   form[4]   form[5]  ...
 */
	for (i = 0; i < length; i++)
		elvss_input_form[i] = (uint32_t)read_value[i];

	common_info->el_ctrl_info.el_ctrl_forms.elvss_input_form = elvss_input_form;
	return LCD_KIT_OK;
}

#define VINT2_READ_STEP   8
#define VINT2_READ_STEP_1 1
#define VINT2_READ_STEP_2 7
#define VINT2_FORM_STEP   2
static int lcd_kit_get_vint2_input_form(int panel_id, char *read_value, int length)
{
	uint32_t *vint2_input_form = NULL;
	int i, j;

	vint2_input_form = kzalloc(length * sizeof(uint32_t), GFP_KERNEL);
	if (!vint2_input_form) {
		LCD_KIT_ERR("vint2 input form is NULL\n");
		return LCD_KIT_FAIL;
	}
/*
 *  el ctrl vint2 input form example
 *  read_value:   value[1]  value[7]  value[9]  value[15]  value[17]  value[23] ...
 *  input_form:   form[0]   form[1]   form[2]   form[3]    form[4]    form[5]   ...
 */
	for (i = 0, j = 0; i < MAX_EL_CTRL_INFO_LEN & j < length; i = i + VINT2_READ_STEP, j = j + VINT2_FORM_STEP) {
		vint2_input_form[j] = (uint32_t)read_value[i + VINT2_READ_STEP_1];
		if (j + 1 < length)
			vint2_input_form[j + 1] = (uint32_t)read_value[i + VINT2_READ_STEP_2];
	}
	common_info->el_ctrl_info.el_ctrl_forms.vint2_input_form = vint2_input_form;
	return LCD_KIT_OK;
}

static int lcd_kit_get_lut_input(int panel_id, struct lcd_kit_adapt_ops *adapt_ops,
	int el_ctrl_form_len)
{
	int ret = LCD_KIT_OK;
	char lut_read_value[MAX_EL_CTRL_INFO_LEN] = {0};

	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return LCD_KIT_FAIL;
	}

	ret = adapt_ops->mipi_rx(panel_id, NULL, lut_read_value, MAX_EL_CTRL_INFO_LEN - 1,
		&common_info->el_ctrl_info.lut_input_read_cmds);
	if (ret) {
		LCD_KIT_ERR("mipi rx failed\n");
		return LCD_KIT_FAIL;
	}

	ret = lcd_kit_get_lut_input_form(panel_id, lut_read_value, el_ctrl_form_len);
	if (ret) {
		LCD_KIT_ERR("get lut input form failed\n");
		return LCD_KIT_FAIL;
	}

	return ret;
}

static int lcd_kit_get_elvss_input(int panel_id, struct lcd_kit_adapt_ops *adapt_ops,
	int el_ctrl_form_len)
{
	int ret = LCD_KIT_OK;
	char elvss_read_value[MAX_EL_CTRL_INFO_LEN] = {0};

	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return LCD_KIT_FAIL;
	}

	ret = adapt_ops->mipi_rx(panel_id, NULL, elvss_read_value, MAX_EL_CTRL_INFO_LEN - 1,
		&common_info->el_ctrl_info.elvss_output_read_cmds);
	if (ret) {
		LCD_KIT_ERR("mipi rx failed\n");
		return LCD_KIT_FAIL;
	}

	ret = lcd_kit_get_elvss_input_form(panel_id, elvss_read_value, el_ctrl_form_len);
	if (ret) {
		LCD_KIT_ERR("get elvss input form failed\n");
		return LCD_KIT_FAIL;
	}

	return ret;
}

static int lcd_kit_get_vint2_input(int panel_id, struct lcd_kit_adapt_ops *adapt_ops,
	int el_ctrl_form_len)
{
	int ret = LCD_KIT_OK;
	char vint2_read_value[MAX_EL_CTRL_INFO_LEN] = {0};

	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on!\n");
		return LCD_KIT_FAIL;
	}

	ret = adapt_ops->mipi_rx(panel_id, NULL, vint2_read_value, MAX_EL_CTRL_INFO_LEN - 1,
		&common_info->el_ctrl_info.vint2_output_read_cmds);
	if (ret) {
		LCD_KIT_ERR("mipi rx failed\n");
		return LCD_KIT_FAIL;
	}

	ret = lcd_kit_get_vint2_input_form(panel_id, vint2_read_value, el_ctrl_form_len);
	if (ret) {
		LCD_KIT_ERR("get vint2 input form failed\n");
		return LCD_KIT_FAIL;
	}

	return ret;
}

static int lcd_kit_get_hxx_el_ctrl_info(int panel_id)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	int el_ctrl_form_len = common_info->el_ctrl_info.el_ctrl_forms.el_ctrl_form_len;

	LCD_KIT_INFO("el ctrl form len is %d\n", el_ctrl_form_len);

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
 
	if (!adapt_ops->mipi_rx || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("mipi_rx or mipi_tx is NULL\n");
		return LCD_KIT_FAIL;
	}

	ret = lcd_kit_get_lut_input(panel_id, adapt_ops, el_ctrl_form_len);
	if (ret) {
		LCD_KIT_ERR("get lut input fail\n");
		return LCD_KIT_FAIL;
	}
	ret = lcd_kit_get_elvss_input(panel_id, adapt_ops, el_ctrl_form_len);
	if (ret) {
		LCD_KIT_ERR("get elvss input fail\n");
		return LCD_KIT_FAIL;
	}
	ret = lcd_kit_get_vint2_input(panel_id, adapt_ops, el_ctrl_form_len);
	if (ret) {
		LCD_KIT_ERR("get vint2 input fail\n");
		return LCD_KIT_FAIL;
	}

	LCD_KIT_INFO("get hxx el ctrl info success\n");
	return ret;
}

#define EL_CTRL_SHARE_MEN_COUNT 26
static int lcd_kit_parse_share_mem(int panel_id, char *ram_read_value)
{
	int ret = LCD_KIT_OK;
	void *vir_addr = NULL;
	int i;

	if (common_info->el_ctrl_info.ddic_type != DDIC_TYPE_CXX) {
		LCD_KIT_WARNING("not cxx ddic type\n");
		return LCD_KIT_FAIL;
	}
	vir_addr = (void *)ioremap_wc(SUB_RESERVED_EL_CTRL_INFO_BASE,
		SUB_RESERVED_EL_CTRL_INFO_SIZE);
	if (!vir_addr) {
		LCD_KIT_ERR("mem ioremap error!\n");
		return LCD_KIT_FAIL;
	}
	ret = memcpy_s((void *)ram_read_value, EL_CTRL_SHARE_MEN_COUNT * sizeof(char),
			(void *)vir_addr, EL_CTRL_SHARE_MEN_COUNT * sizeof(char));
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("copy share mem fail\n");
		iounmap(vir_addr);
		return LCD_KIT_FAIL;
	}
	iounmap(vir_addr);
	for (i = 0; i < EL_CTRL_SHARE_MEN_COUNT; i++)
		LCD_KIT_DEBUG("el ctrl read value[%d] = %d\n", i, ram_read_value[i]);

	return ret;
}

#define ELVSS_READ_COUNT 4
#define ELVSS_TABLE_COUNT 13
#define LUT_READ_COUNT 40
#define EXTENSION_TABLE_COUNT 100
static int lcd_kit_get_cxx_vint2_info(int panel_id, struct lcd_kit_adapt_ops *adapt_ops,
	uint32_t *vint2_table, char *ram_read_value)
{
	int ret = LCD_KIT_OK;
	int i;
	char vint2_read_value[ELVSS_READ_COUNT] = {0};
	uint32_t vint2_offset = 0;

	ret = adapt_ops->mipi_rx(panel_id, NULL, vint2_read_value,
		ELVSS_READ_COUNT - 1, &common_info->el_ctrl_info.vint2_output_read_cmds);
	if (ret)
		return LCD_KIT_FAIL;

	/* vint2 */
	vint2_offset = ram_read_value[ELVSS_TABLE_COUNT] + (((ram_read_value[ELVSS_TABLE_COUNT + 1] & 0xFE) >> 1) << 8);
	for (i = 0; i < ELVSS_TABLE_COUNT; i++) {
		vint2_table[i] = vint2_read_value[0] + ((vint2_read_value[1] & 0x03) << 8) + ram_read_value[i] + (((vint2_offset & BIT(i)) >> i) << 8);
		LCD_KIT_DEBUG("vint2_table[%d] = 0x%x\n", i, vint2_table[i]);
	}

	return ret;
}

static int lcd_kit_get_cxx_elvss_info(int panel_id, struct lcd_kit_adapt_ops *adapt_ops,
	uint32_t *elvss_table, char *ram_read_value)
{
	int ret = LCD_KIT_OK;
	int i, j;
	uint32_t elvss_base_th = 0;
	char elvss_read_value[ELVSS_READ_COUNT] = {0};

	ret = adapt_ops->mipi_rx(panel_id, NULL, elvss_read_value,
		ELVSS_READ_COUNT - 1, &common_info->el_ctrl_info.elvss_output_read_cmds);
	if (ret)
		return LCD_KIT_FAIL;

	/* elvss */
	elvss_base_th = (uint32_t)elvss_read_value[0];
	if (elvss_base_th >= (ELVSS_TABLE_COUNT - 1))
		elvss_base_th = ELVSS_TABLE_COUNT - 1;
	LCD_KIT_INFO("elvss_base_th = %d\n", elvss_base_th);
	for (i = 0, j = ELVSS_TABLE_COUNT + 2; i < elvss_base_th && j < EL_CTRL_SHARE_MEN_COUNT; i = i + 2, j++) {
		elvss_table[i] = elvss_read_value[1] + (ram_read_value[j] & 0x0F);
		LCD_KIT_DEBUG("elvss_table[%d] = 0x%x\n", i, elvss_table[i]);
		if (i + 1 < elvss_base_th) {
			elvss_table[i + 1] = elvss_read_value[1] + ((ram_read_value[j] & 0xF0) >> 4);
			LCD_KIT_DEBUG("elvss_table[%d] = 0x%x\n", i + 1, elvss_table[i + 1]);
		}
	}

	if ((elvss_base_th % 2) == 1 && elvss_base_th < (ELVSS_TABLE_COUNT - 1)) {
		elvss_table[elvss_base_th] = elvss_read_value[2] + ((ram_read_value[j] & 0xF0) >> 4);
		LCD_KIT_DEBUG("elvss_table[%d] = 0x%x\n", elvss_base_th, elvss_table[elvss_base_th]);
		for (i = elvss_base_th + 1; i < ELVSS_TABLE_COUNT && j < EL_CTRL_SHARE_MEN_COUNT; i = i + 2, j++) {
			elvss_table[i] = elvss_read_value[2] + (ram_read_value[j] & 0x0F);
			LCD_KIT_DEBUG("elvss_table[%d] = 0x%x\n", i, elvss_table[i]);
			if (i + 1 < (ELVSS_TABLE_COUNT)) {
				elvss_table[i + 1] = elvss_read_value[2] + ((ram_read_value[j] & 0xF0) >> 4);
				LCD_KIT_DEBUG("elvss_table[%d] = 0x%x\n", i + 1, elvss_table[i + 1]);
			}
		}
	} else {
		for (i = elvss_base_th; i < ELVSS_TABLE_COUNT && j < EL_CTRL_SHARE_MEN_COUNT; i = i + 2, j++) {
			elvss_table[i] = elvss_read_value[2] + (ram_read_value[j] & 0x0F);
			LCD_KIT_DEBUG("elvss_table[%d] = 0x%x\n", i, elvss_table[i]);
			if (i + 1 < (ELVSS_TABLE_COUNT)) {
				elvss_table[i + 1] = elvss_read_value[2] + ((ram_read_value[j] & 0xF0) >> 4);
				LCD_KIT_DEBUG("elvss_table[%d] = 0x%x\n", i + 1, elvss_table[i + 1]);
			}
		}
	}

	return ret;
}

static int lcd_kit_get_cxx_lut_info(int panel_id, struct lcd_kit_adapt_ops *adapt_ops,
	uint32_t *lut_table, char *ram_read_value)
{
	int ret = LCD_KIT_OK;
	int i, j;
	char lut_read_value[LUT_READ_COUNT] = {0};

	ret = adapt_ops->mipi_rx(panel_id, NULL, lut_read_value,
		LUT_READ_COUNT - 1, &common_info->el_ctrl_info.lut_input_read_cmds);
	if (ret)
		return LCD_KIT_FAIL;

	for (i = 1, j = 0; i < ELVSS_TABLE_COUNT && j < LUT_READ_COUNT; i++, j = j + 2) {
		lut_table[i] = (((lut_read_value[j] & 0x1F) << 8) + lut_read_value[j + 1]) >> 1;
		LCD_KIT_DEBUG("lut_table[%d] = %d\n", i, lut_table[i]);
	}
	return ret;
}

static int lcd_kit_set_el_crtl_form(int panel_id, uint32_t *final_elvss_table, uint32_t *final_vint2_table,
	uint32_t *final_lut_table, int table_length)
{
	int ret = LCD_KIT_OK;

	if (table_length == 0 || table_length >= EXTENSION_TABLE_COUNT) {
		LCD_KIT_ERR("cxx el ctrl length is 0 or 100\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("table_length = %d\n", table_length);
	common_info->el_ctrl_info.el_ctrl_forms.el_ctrl_form_len = table_length;
	common_info->el_ctrl_info.el_ctrl_forms.vint2_input_form = kzalloc(table_length * sizeof(uint32_t), GFP_KERNEL);
	common_info->el_ctrl_info.el_ctrl_forms.elvss_input_form = kzalloc(table_length * sizeof(uint32_t), GFP_KERNEL);
	common_info->el_ctrl_info.el_ctrl_forms.lut_form = kzalloc(table_length * sizeof(uint32_t), GFP_KERNEL);
	if (!common_info->el_ctrl_info.el_ctrl_forms.vint2_input_form ||
			!common_info->el_ctrl_info.el_ctrl_forms.elvss_input_form ||
			!common_info->el_ctrl_info.el_ctrl_forms.lut_form) {
		LCD_KIT_ERR("malloc el ctrl mem failed\n");
		return LCD_KIT_FAIL;
	}

	ret = memcpy_s(common_info->el_ctrl_info.el_ctrl_forms.vint2_input_form, table_length * sizeof(uint32_t),
		final_vint2_table, table_length * sizeof(uint32_t));
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("copy vint2 form fail\n");
		return LCD_KIT_FAIL;
	}
	ret = memcpy_s(common_info->el_ctrl_info.el_ctrl_forms.elvss_input_form, table_length * sizeof(uint32_t),
		final_elvss_table, table_length * sizeof(uint32_t));
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("copy elvss form fail\n");
		return LCD_KIT_FAIL;
	}
	ret = memcpy_s(common_info->el_ctrl_info.el_ctrl_forms.lut_form, table_length * sizeof(uint32_t),
		final_lut_table, table_length * sizeof(uint32_t));
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("copy lut form fail\n");
		return LCD_KIT_FAIL;
	}

	return ret;
}

struct el_ctrl_final_table_info {
	uint32_t elvss_inter_step;
	uint32_t vint2_inter_step;
	char elvss_interpolation[ELVSS_READ_COUNT];
	char vint2_interpolation[ELVSS_READ_COUNT];
	uint32_t final_elvss_table[EXTENSION_TABLE_COUNT];
	uint32_t final_vint2_table[EXTENSION_TABLE_COUNT];
	uint32_t final_lut_table[EXTENSION_TABLE_COUNT];
};

static int lcd_kit_get_cxx_interpolation_info(int panel_id, struct lcd_kit_adapt_ops *adapt_ops, uint32_t *vint2_table,
		uint32_t *elvss_table, uint32_t *lut_table, char *ram_read_value)
{
	int ret = LCD_KIT_OK;
	int i, j, z;
	struct el_ctrl_final_table_info *table_info;
	int table_length = 0;
	uint32_t elvss_steps = 0;

	table_info = kzalloc(sizeof(struct el_ctrl_final_table_info), GFP_KERNEL);
	if (!table_info) {
		LCD_KIT_ERR("el ctrl final table info is NULL\n");
		return LCD_KIT_FAIL;
	}

	ret = adapt_ops->mipi_rx(panel_id, NULL, table_info->elvss_interpolation,
		ELVSS_READ_COUNT - 1, &common_info->el_ctrl_info.elvss_interpolation_cmds);
	if (ret)
		return LCD_KIT_FAIL;
	ret = adapt_ops->mipi_rx(panel_id, NULL, table_info->vint2_interpolation,
		ELVSS_READ_COUNT - 1, &common_info->el_ctrl_info.vint2_interpolation_cmds);
	if (ret)
		return LCD_KIT_FAIL;

	table_info->elvss_inter_step = table_info->elvss_interpolation[0] + (table_info->elvss_interpolation[1] << 8);
	table_info->vint2_inter_step = table_info->vint2_interpolation[0] + (table_info->vint2_interpolation[1] << 8);

	/* interpolation */
	for (i = 0, j = 0; i < ELVSS_TABLE_COUNT - 1 && j < EXTENSION_TABLE_COUNT; i++) {
		if (elvss_table[i] >= elvss_table[i + 1] || (table_info->elvss_inter_step & BIT(i)) || (table_info->vint2_inter_step & BIT(i))) {
			table_info->final_elvss_table[j] = elvss_table[i];
			table_info->final_vint2_table[j] = vint2_table[i];
			table_info->final_lut_table[j] = lut_table[i];
			j++;
		} else if ((table_info->elvss_inter_step & BIT(i)) == 0 && (table_info->vint2_inter_step & BIT(i)) == 0) {
			elvss_steps = elvss_table[i + 1] - elvss_table[i];
			for (z = 0; z < elvss_steps; z++) {
				table_info->final_elvss_table[j] = elvss_table[i] + z;
				table_info->final_vint2_table[j] = vint2_table[i] + (uint32_t)(z * (vint2_table[i + 1] - vint2_table[i]) / elvss_steps);
				table_info->final_lut_table[j] = lut_table[i] + (uint32_t)(z * (lut_table[i + 1] - lut_table[i]) / elvss_steps);
				j++;
			}
		}
	}
	table_info->final_elvss_table[j] = elvss_table[ELVSS_TABLE_COUNT - 1];
	table_info->final_vint2_table[j] = vint2_table[ELVSS_TABLE_COUNT - 1];
	table_info->final_lut_table[j] = lut_table[ELVSS_TABLE_COUNT - 1];
	table_length = j + 1;

	ret = lcd_kit_set_el_crtl_form(panel_id, table_info->final_elvss_table, table_info->final_vint2_table,
		table_info->final_lut_table, table_length);
	if (ret) {
		kfree(table_info);
		return LCD_KIT_FAIL;
	}
	kfree(table_info);

	return ret;
}

static int lcd_kit_get_cxx_el_ctrl_info(int panel_id)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	char ram_read_value[EL_CTRL_SHARE_MEN_COUNT] = {0};
	uint32_t vint2_table[ELVSS_TABLE_COUNT] = {0};
	uint32_t elvss_table[ELVSS_TABLE_COUNT] = {0};
	uint32_t lut_table[ELVSS_TABLE_COUNT] = {0};

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_rx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_rx!\n");
		return LCD_KIT_FAIL;
	}
	ret = lcd_kit_parse_share_mem(panel_id, ram_read_value);
	if (ret) {
		LCD_KIT_ERR("get cxx vint2 info fail\n");
		return LCD_KIT_FAIL;
	}

	ret = lcd_kit_get_cxx_vint2_info(panel_id, adapt_ops, vint2_table, ram_read_value);
	if (ret) {
		LCD_KIT_ERR("get cxx vint2 info fail\n");
		return LCD_KIT_FAIL;
	}
	ret = lcd_kit_get_cxx_elvss_info(panel_id, adapt_ops, elvss_table, ram_read_value);
	if (ret) {
		LCD_KIT_ERR("get cxx elvss info fail\n");
		return LCD_KIT_FAIL;
	}
	ret = lcd_kit_get_cxx_lut_info(panel_id, adapt_ops, lut_table, ram_read_value);
	if (ret) {
		LCD_KIT_ERR("get cxx lut info fail\n");
		return LCD_KIT_FAIL;
	}
	ret = lcd_kit_get_cxx_interpolation_info(panel_id, adapt_ops, vint2_table, elvss_table, lut_table, ram_read_value);
	if (ret) {
		LCD_KIT_ERR("get cxx interpolation info fail\n");
		return LCD_KIT_FAIL;
	}

	return ret;
}

static int lcd_kit_el_ctrl_set_level(int panel_id)
{
	int ret = LCD_KIT_OK;
	int i;

	if (PUBLIC_INFO->fake_compatible_panel) {
		LCD_KIT_WARNING("panel sn not compatible\n");
		return LCD_KIT_FAIL;
	}

	if (!common_info->el_ctrl_info.el_ctrl_level.buf || !common_info->el_ctrl_info.el_ctrl_oem_value.buf) {
		LCD_KIT_ERR("buf is NULL\n");
		return LCD_KIT_FAIL;
	}

	if (common_info->el_ctrl_info.el_ctrl_level.cnt == 0 ||
		common_info->el_ctrl_info.el_ctrl_oem_value.cnt == 0) {
		LCD_KIT_ERR("cnt is 0\n");
		return LCD_KIT_FAIL;
	}

	if (PUBLIC_INFO->calibrate_level == EL_CTRL_OEM_DEFAULT) {
		LCD_KIT_WARNING("el ctrl oeminfo invalid\n");
		return LCD_KIT_OK;
	}

	for (i = 0; i < common_info->el_ctrl_info.el_ctrl_oem_value.cnt; i++) {
		if (common_info->el_ctrl_info.el_ctrl_oem_value.buf[i] == PUBLIC_INFO->calibrate_level) {
			common_info->el_ctrl_info.max_offset_value = common_info->el_ctrl_info.el_ctrl_level.buf[i];
			break;
		}
	}
	LCD_KIT_INFO("common_info->el_ctrl_info.max_offset_value = %d\n", common_info->el_ctrl_info.max_offset_value);
	if (i == common_info->el_ctrl_info.el_ctrl_oem_value.cnt)
		common_info->el_ctrl_info.max_offset_value = 0;

	if (common_info->el_ctrl_info.max_offset_value == 0) {
		LCD_KIT_WARNING("max offset value is 0, not support el ctrl\n");
		return LCD_KIT_FAIL;
	}

	return ret;
}

static int lcd_kit_get_el_ctrl_info(int panel_id)
{
	int ret = LCD_KIT_OK;
	
	if (!common_info->el_ctrl_info.support) {
		LCD_KIT_INFO("El ctrl not support\n");
		return LCD_KIT_FAIL;
	}

	if (common_info->el_ctrl_info.ddic_type == DDIC_TYPE_CXX) {
		ret = lcd_kit_get_cxx_el_ctrl_info(panel_id);
		if (ret)
			return LCD_KIT_FAIL;
	} else if (common_info->el_ctrl_info.ddic_type == DDIC_TYPE_HXX) {
		ret = lcd_kit_get_hxx_el_ctrl_info(panel_id);
		if (ret)
			return LCD_KIT_FAIL;
	}

	ret = lcd_kit_el_ctrl_set_level(panel_id);
	if (ret)
		return LCD_KIT_FAIL;

	common_info->el_ctrl_info.validate_done = 0;
	common_info->el_ctrl_info.el_init_done = 1;
	return ret;
}

void clear_el_ctrl_input_info(int panel_id)
{
	if (!common_info->el_ctrl_info.support)
		return;
	kfree(common_info->el_ctrl_info.el_ctrl_forms.lut_form);
	kfree(common_info->el_ctrl_info.el_ctrl_forms.elvss_input_form);
	kfree(common_info->el_ctrl_info.el_ctrl_forms.vint2_input_form);
}

struct lcd_kit_ops g_lcd_ops = {
	.lcd_kit_support = lcd_kit_support,
	.get_project_id = lcd_kit_get_project_id,
	.get_status_by_type = lcd_kit_get_status_by_type,
#ifdef LCD_FACTORY_MODE
	.get_pt_station_status = lcd_kit_get_pt_station_status,
#endif
	.get_sn_code = lcd_kit_get_sn_code,
	.get_panel_dieid = lcd_get_panel_dieid_info,
	.event_skip_send = lcd_kit_power_off_event_skip_send,
	.notify_panel_switch = lcd_kit_notify_panel_switch,
	.panel_event_skip_delay = lcd_kit_panel_event_skip_delay,
	.get_panel_power_status = lcd_kit_get_power_status,
	.dvdd_hpm_ctrl = lcd_kit_dvdd_hpm_ctrl,
	.dvdd_avs_ctrl = lcd_kit_dvdd_avs_ctrl,
	.exec_ext_ts = lcd_kit_exec_ext_ts,
	.eink_power_ctrl = lcd_kit_eink_power_ctrl,
	.get_panel_id = lcd_kit_get_active_panel_id,
	.get_el_ctrl_info = lcd_kit_get_el_ctrl_info,
};

int lcd_kit_utils_init(int panel_id, struct device_node *np, struct dpu_panel_info *pinfo)
{
	/* init sem */
	lcd_kit_init_sema(panel_id);
	/* init display */
	lcd_kit_init_display(panel_id, np);
	/* init dpu pinfo */
	lcd_kit_init_pinfo(panel_id, np, pinfo);
	/* register lcd ops */
	lcd_kit_ops_register(&g_lcd_ops);
	/* init display engine params */
	lcd_kit_display_engine_param_init(panel_id, np);
	return LCD_KIT_OK;
}

