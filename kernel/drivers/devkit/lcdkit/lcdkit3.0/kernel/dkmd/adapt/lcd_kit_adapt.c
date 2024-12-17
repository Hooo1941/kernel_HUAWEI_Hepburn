/*
 * lcd_kit_adapt.c
 *
 * lcdkit adapt function for lcd driver
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

#include "lcd_kit_adapt.h"
#include "lcd_kit_common.h"
#include "lcd_kit_disp.h"
#include "lcd_kit_power.h"
#include "lcd_kit_ext_power.h"
#include "lcd_kit_utils.h"
#include <securec.h>

static bool lcd_kit_cmd_is_write(struct dsi_cmd_desc *cmd)
{
	bool is_write = true;

	switch (cmd->dtype) {
	case DTYPE_DCS_WRITE:
	case DTYPE_DCS_WRITE1:
	case DTYPE_DCS_LWRITE:
	case DTYPE_DSC_LWRITE:
	case DTYPE_GEN_WRITE:
	case DTYPE_GEN_WRITE1:
	case DTYPE_GEN_WRITE2:
	case DTYPE_GEN_LWRITE:
		is_write = true;
		break;
	case DTYPE_DCS_READ:
	case DTYPE_GEN_READ:
	case DTYPE_GEN_READ1:
	case DTYPE_GEN_READ2:
		is_write = false;
		break;
	default:
		is_write = false;
		break;
	}
	return is_write;
}

static int lcd_kit_dsi_cmd_rx(int panel_id, unsigned char *out,
	int out_len, struct dsi_cmd_desc *cmd)
{
	int ret;
	unsigned int dsi_index;
	unsigned int connector_id;

	if (!cmd) {
		LCD_KIT_ERR("cmd is null\n");
		return LCD_KIT_FAIL;
	}

	if (lcd_kit_get_panel_off_state(panel_id)) {
		LCD_KIT_ERR("panel is power off\n");
		return LCD_KIT_FAIL;
	}

	connector_id = DPU_PINFO->connector_id;
	ret = lcd_kit_get_dsi_index(&dsi_index, connector_id);
	if (ret) {
		LCD_KIT_ERR("get dsi index error\n");
		return LCD_KIT_FAIL;
	}

	LCD_KIT_DEBUG("rx %d send cmd\n", dsi_index);
	mipi_dsi_cmds_rx_for_usr(dsi_index, out, out_len, cmd, false, DPU_PINFO->type);

	return LCD_KIT_OK;
}

bool lcd_kit_is_dual_mipi_panel(int panel_id)
{
	unsigned int cmd_type;
	cmd_type = DPU_PINFO->type;
	return is_dual_mipi_panel(cmd_type);
}

static int lcd_kit_dual_dsi_cmd_rx(int panel_id, unsigned char *out0, unsigned char *out1,
	int out_len, struct dsi_cmd_desc *cmd)
{
	int ret;
	unsigned int connector_id;
	unsigned int cmd_type;
	unsigned int dsi0_index;
	unsigned int dsi1_index;

	if (!cmd) {
		LCD_KIT_ERR("cmd is null\n");
		return LCD_KIT_FAIL;
	}

	if (lcd_kit_get_panel_off_state(panel_id)) {
		LCD_KIT_ERR("panel is power off\n");
		return LCD_KIT_FAIL;
	}

	connector_id = DPU_PINFO->connector_id;
	cmd_type = DPU_PINFO->type;
	if (is_dual_mipi_panel(cmd_type)) {
		ret = lcd_kit_get_dual_dsi_index(&dsi0_index, &dsi1_index, connector_id);
		if (ret) {
			LCD_KIT_ERR("get dual dsi index error\n");
			return LCD_KIT_FAIL;
		}
		LCD_KIT_INFO("dual %d %d send cmd\n", dsi0_index, dsi1_index);
		mipi_dual_dsi_cmds_rx_for_usr(dsi0_index, out0, dsi1_index,
			out1, out_len, cmd, false, DPU_PINFO->type);
	} else {
		LCD_KIT_ERR("dual panel error\n");
		return LCD_KIT_FAIL;
	}

	return LCD_KIT_OK;
}

static int lcd_kit_single_dsi_cmd_tx(int panel_id, struct dsi_cmd_desc *cmd, int lock)
{
	int ret;
	unsigned int dsi0_index;
	unsigned int connector_id;

	connector_id = DPU_PINFO->connector_id;
	ret = lcd_kit_get_dsi_index(&dsi0_index, connector_id);
	if (ret) {
		LCD_KIT_ERR("get dsi0 index error\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_DEBUG("single %d send cmd\n", dsi0_index);
	if (lock == NO_LOCK) {
		LCD_KIT_DEBUG("enter no lock\n");
		mipi_dsi_cmds_tx_for_usr_isr_safe(dsi0_index, cmd, 1, DPU_PINFO->type);
	} else {
		LCD_KIT_DEBUG("enter lock\n");
		mipi_dsi_cmds_tx_for_usr(dsi0_index, cmd, 1, DPU_PINFO->type);
	}

	return ret;
}

/* same cmds, master+slave TX at the same time */
static int lcd_kit_dual_dsi_cmd_tx(int panel_id, struct dsi_cmd_desc *cmd0,
	struct dsi_cmd_desc *cmd1, int lock)
{
	int ret;
	unsigned int dsi0_index;
	unsigned int dsi1_index;
	unsigned int connector_id;

	connector_id = DPU_PINFO->connector_id;
	ret = lcd_kit_get_dual_dsi_index(&dsi0_index, &dsi1_index, connector_id);
	if (ret) {
		LCD_KIT_ERR("get dual dsi index error\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_DEBUG("dual %d %d send cmd\n", dsi0_index, dsi1_index);
	if (lock == NO_LOCK) {
		LCD_KIT_DEBUG("enter no lock\n");
		mipi_dual_dsi_cmds_tx_for_usr_isr_safe(dsi0_index, cmd0, 1, dsi1_index,
			cmd1, 1, 0, DPU_PINFO->type);
	} else {
		LCD_KIT_DEBUG("enter lock\n");
		mipi_dual_dsi_cmds_tx_for_usr(dsi0_index, cmd0, 1, dsi1_index,
			cmd1, 1, 0, DPU_PINFO->type);
	}

	return ret;
}

static int lcd_kit_dsi_cmd_tx(int panel_id, struct dsi_cmd_desc *cmd, int lock)
{
	int ret;
	unsigned int cmd_type;

	if (!cmd) {
		LCD_KIT_ERR("cmd is null\n");
		return LCD_KIT_FAIL;
	}

	if (lcd_kit_get_panel_off_state(panel_id)) {
		LCD_KIT_ERR("panel is power off\n");
		return LCD_KIT_FAIL;
	}

	cmd_type = DPU_PINFO->type;
	if (is_dual_mipi_panel(cmd_type)) {
		return lcd_kit_dual_dsi_cmd_tx(panel_id, cmd, cmd, lock);
	} else {
		return lcd_kit_single_dsi_cmd_tx(panel_id, cmd, lock);
	}
}

#define MAX_WAIT_TIME 20
static void wait_no_lock_completion(int panel_id)
{
	reinit_completion(&disp_info->lcd_completion.tx_no_lock_done);
	LCD_KIT_INFO("wait no_lock_done!\n");
	if (!wait_for_completion_timeout(&disp_info->lcd_completion.tx_no_lock_done,
		msecs_to_jiffies(MAX_WAIT_TIME)))
		LCD_KIT_ERR("wait tx no lock time out!\n");
}

/* same cmds, master+slave TX at the same time */
int lcd_kit_cmds_tx(int panel_id, void *hld, struct lcd_kit_dsi_panel_cmds *cmds)
{
	int i;
	int link_state;
	struct dsi_cmd_desc dsi_cmd;
	unsigned int dsi0_index;
	unsigned int connector_id;
	int ret;

	if (cmds == NULL || cmds->cmds == NULL || cmds->cmd_cnt <= 0) {
		LCD_KIT_ERR("cmds is null, or cmd_cnt <= 0!\n");
		return LCD_KIT_FAIL;
	}

	memset(&dsi_cmd, 0, sizeof(dsi_cmd));
	link_state = cmds->link_state;
	connector_id = DPU_PINFO->connector_id;
	ret = lcd_kit_get_dsi_index(&dsi0_index, connector_id);
	if (ret) {
		LCD_KIT_ERR("get dsi0 index error\n");
		return LCD_KIT_FAIL;
	}

	if (disp_info->lcd_completion.is_in_tx_no_lock == true)
		wait_no_lock_completion(panel_id);
	down(&DISP_LOCK->lcd_kit_sem);
	/* switch to LP mode */
	if (cmds->link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(panel_id, LCD_KIT_DSI_LP_MODE);
	for (i = 0; i < cmds->cmd_cnt; i++) {
		lcd_kit_cmds_to_dsi_cmds(&cmds->cmds[i], &dsi_cmd, false);
		lcd_kit_dsi_cmd_tx(panel_id, &dsi_cmd, LOCK);
		if (cmds->cmds[i].waittype & SELF_REFRESH_FLAG)
			dkmd_dfr_send_refresh(dsi0_index, DPU_PINFO->type);
		lcd_kit_delay(cmds->cmds[i].wait, cmds->cmds[i].waittype, true);
	}

	LCD_KIT_DEBUG("send cmd len = %d\n", cmds->cmd_cnt);
	/* switch to HS mode */
	if (cmds->link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(panel_id, LCD_KIT_DSI_HS_MODE);
	up(&DISP_LOCK->lcd_kit_sem);

	return LCD_KIT_OK;
}

int lcd_kit_cmds_tx_no_lock(int panel_id, void *hld, struct lcd_kit_dsi_panel_cmds *cmds)
{
	int i;
	int ret;
	struct dsi_cmd_desc dsi_cmd;
	unsigned int dsi0_index;
	int link_state = cmds->link_state;
	unsigned int connector_id = DPU_PINFO->connector_id;

	if (cmds == NULL || cmds->cmds == NULL || cmds->cmd_cnt <= 0) {
		LCD_KIT_ERR("cmds is null, or cmd_cnt <= 0!\n");
		return LCD_KIT_FAIL;
	}

	memset(&dsi_cmd, 0, sizeof(dsi_cmd));
	ret = lcd_kit_get_dsi_index(&dsi0_index, connector_id);
	if (ret) {
		LCD_KIT_ERR("get dsi0 index error\n");
		return LCD_KIT_FAIL;
	}
	disp_info->lcd_completion.is_in_tx_no_lock = true;
	/* switch to LP mode */
	if (cmds->link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(panel_id, LCD_KIT_DSI_LP_MODE);
	for (i = 0; i < cmds->cmd_cnt; i++) {
		lcd_kit_cmds_to_dsi_cmds(&cmds->cmds[i], &dsi_cmd, false);
		lcd_kit_dsi_cmd_tx(panel_id, &dsi_cmd, NO_LOCK);
		if (cmds->cmds[i].waittype & SELF_REFRESH_FLAG)
			dkmd_dfr_send_refresh(dsi0_index, DPU_PINFO->type);
		lcd_kit_delay(cmds->cmds[i].wait, cmds->cmds[i].waittype, true);
	}

	LCD_KIT_INFO("send cmd len = %d\n", cmds->cmd_cnt);
	/* switch to HS mode */
	if (cmds->link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(panel_id, LCD_KIT_DSI_HS_MODE);
	complete_all(&disp_info->lcd_completion.tx_no_lock_done);
	disp_info->lcd_completion.is_in_tx_no_lock = false;
	return LCD_KIT_OK;
}

/* same cmds, master+slave TX and only master RX at the same time */
int lcd_kit_cmds_rx(int panel_id, void *hld, unsigned char *out, int out_len,
	struct lcd_kit_dsi_panel_cmds *cmds)
{
	int ret = LCD_KIT_OK;
	int i, j, link_state;
	int cnt = 0;
	struct dsi_cmd_desc dsi_cmd;
	unsigned char tmp[BUF_MAX] = {0};
	unsigned int dsi0_index;
	unsigned int connector_id;

	if (cmds == NULL || cmds->cmds == NULL || cmds->cmd_cnt <= 0) {
		LCD_KIT_ERR("cmds is null, or cmd_cnt <= 0!\n");
		return LCD_KIT_FAIL;
	}

	if (out == NULL) {
		LCD_KIT_ERR("out is null!\n");
		return LCD_KIT_FAIL;
	}

	connector_id = DPU_PINFO->connector_id;
	ret = lcd_kit_get_dsi_index(&dsi0_index, connector_id);
	if (ret) {
		LCD_KIT_ERR("get dsi0 index error\n");
		return LCD_KIT_FAIL;
	}
	memset(&dsi_cmd, 0, sizeof(dsi_cmd));
	link_state = cmds->link_state;
	down(&DISP_LOCK->lcd_kit_sem);
	if (link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(panel_id, link_state);
	for (i = 0; i < cmds->cmd_cnt; i++) {
		memset(tmp, 0, sizeof(tmp));
		lcd_kit_cmds_to_dsi_cmds(&cmds->cmds[i], &dsi_cmd, false);
		if (lcd_kit_cmd_is_write(&dsi_cmd)) {
			ret = lcd_kit_dsi_cmd_tx(panel_id, &dsi_cmd, LOCK);
			if (cmds->cmds[i].waittype & SELF_REFRESH_FLAG)
				dkmd_dfr_send_refresh(dsi0_index, DPU_PINFO->type);
		} else {
			ret = lcd_kit_dsi_cmd_rx(panel_id, tmp, BUF_MAX, &dsi_cmd);
			for (j = 0; (j < dsi_cmd.dlen) && (cnt <= out_len); j++)
				out[cnt++] = tmp[j];
		}
		lcd_kit_delay(cmds->cmds[i].wait, cmds->cmds[i].waittype, true);
	}

	LCD_KIT_DEBUG("send cmd len = %d\n", cmds->cmd_cnt);
	if (link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(panel_id, LCD_KIT_DSI_HS_MODE);
	up(&DISP_LOCK->lcd_kit_sem);

	return ret;
}

/* same cmds, master+slave TX and master+slave RX at the same time */
int lcd_kit_dual_cmds_rx(int panel_id, void *hld, unsigned char *out0, unsigned char *out1,
	int out_len, struct lcd_kit_dsi_panel_cmds *cmds)
{
	int ret = LCD_KIT_OK;
	int i, j, link_state;
	int cnt0 = 0;
	int cnt1 = 0;
	struct dsi_cmd_desc dsi_cmd;
	unsigned char tmp0[BUF_MAX] = {0};
	unsigned char tmp1[BUF_MAX] = {0};

	if (cmds == NULL || cmds->cmds == NULL || cmds->cmd_cnt <= 0) {
		LCD_KIT_ERR("cmds is null, or cmd_cnt <= 0!\n");
		return LCD_KIT_FAIL;
	}

	if (out0 == NULL || out1 == NULL) {
		LCD_KIT_ERR("out is null!\n");
		return LCD_KIT_FAIL;
	}

	memset(&dsi_cmd, 0, sizeof(dsi_cmd));
	link_state = cmds->link_state;
	down(&DISP_LOCK->lcd_kit_sem);
	if (link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(panel_id, link_state);
	for (i = 0; i < cmds->cmd_cnt; i++) {
		memset(tmp0, 0, sizeof(tmp0));
		memset(tmp1, 0, sizeof(tmp1));
		lcd_kit_cmds_to_dsi_cmds(&cmds->cmds[i], &dsi_cmd, false);
		if (lcd_kit_cmd_is_write(&dsi_cmd)) {
			ret = lcd_kit_dsi_cmd_tx(panel_id, &dsi_cmd, LOCK);
		} else {
			ret = lcd_kit_dual_dsi_cmd_rx(panel_id, tmp0, tmp1, BUF_MAX, &dsi_cmd);
			for (j = 0; (j < dsi_cmd.dlen) && (cnt0 <= out_len) && (cnt1 <= out_len); j++) {
				out0[cnt0++] = tmp0[j];
				out1[cnt1++] = tmp1[j];
			}
		}
		lcd_kit_delay(cmds->cmds[i].wait, cmds->cmds[i].waittype, true);
	}

	LCD_KIT_INFO("send cmd len = %d\n", cmds->cmd_cnt);
	if (link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(panel_id, LCD_KIT_DSI_HS_MODE);
	up(&DISP_LOCK->lcd_kit_sem);

	return ret;
}

/* different cmds, master+slave TX and master+slave RX at the same time */
int lcd_kit_dual_diff_cmds_rx(int panel_id, unsigned char *out0, unsigned char *out1,
	int out_len, struct lcd_kit_dsi_panel_cmds *cmds0, struct lcd_kit_dsi_panel_cmds *cmds1)
{
	int ret = LCD_KIT_OK;
	int i, j, link_state;
	int cnt0 = 0;
	int cnt1 = 0;
	struct dsi_cmd_desc dsi_cmd0;
	struct dsi_cmd_desc dsi_cmd1;
	unsigned char tmp0[BUF_MAX] = {0};
	unsigned char tmp1[BUF_MAX] = {0};

	memset_s(&dsi_cmd0, sizeof(dsi_cmd0), 0, sizeof(dsi_cmd0));
	memset_s(&dsi_cmd1, sizeof(dsi_cmd1), 0, sizeof(dsi_cmd1));
	link_state = cmds0->link_state;
	down(&DISP_LOCK->lcd_kit_sem);
	if (link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(panel_id, link_state);
	for (i = 0; i < cmds0->cmd_cnt; i++) {
		memset_s(tmp0, sizeof(tmp0), 0, sizeof(tmp0));
		memset_s(tmp1, sizeof(tmp1), 0, sizeof(tmp1));
		lcd_kit_cmds_to_dsi_cmds(&cmds0->cmds[i], &dsi_cmd0, false);
		lcd_kit_cmds_to_dsi_cmds(&cmds1->cmds[i], &dsi_cmd1, false);
		if (lcd_kit_cmd_is_write(&dsi_cmd0)) {
			ret += lcd_kit_dual_dsi_cmd_tx(panel_id, &dsi_cmd0, &dsi_cmd1, LOCK);
		} else {
			ret += lcd_kit_dual_dsi_cmd_rx(panel_id, tmp0, tmp1, BUF_MAX, &dsi_cmd0);
			for (j = 0; (j < dsi_cmd0.dlen) && (cnt0 <= out_len) && (cnt1 <= out_len); j++) {
				out0[cnt0++] = tmp0[j];
				out1[cnt1++] = tmp1[j];
			}
		}
		lcd_kit_delay(cmds0->cmds[i].wait, cmds0->cmds[i].waittype, true);
	}

	LCD_KIT_INFO("send cmds0 len = %d\n", cmds0->cmd_cnt);
	if (link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(panel_id, LCD_KIT_DSI_HS_MODE);
	up(&DISP_LOCK->lcd_kit_sem);

	return ret;
}

/* master+slave TX different or same cmds */
int lcd_kit_dual_diff_cmds_tx(int panel_id, void *hld, struct lcd_kit_dsi_panel_cmds *cmds0,
	struct lcd_kit_dsi_panel_cmds *cmds1)
{
	int ret = LCD_KIT_OK;
	int i, link_state;
	struct dsi_cmd_desc dsi_cmd0;
	struct dsi_cmd_desc dsi_cmd1;

	memset_s(&dsi_cmd0, sizeof(dsi_cmd0), 0, sizeof(dsi_cmd0));
	memset_s(&dsi_cmd1, sizeof(dsi_cmd1), 0, sizeof(dsi_cmd1));
	link_state = cmds0->link_state;
	down(&DISP_LOCK->lcd_kit_sem);
	if (link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(panel_id, link_state);
	for (i = 0; i < cmds0->cmd_cnt; i++) {
		lcd_kit_cmds_to_dsi_cmds(&cmds0->cmds[i], &dsi_cmd0, false);
		lcd_kit_cmds_to_dsi_cmds(&cmds1->cmds[i], &dsi_cmd1, false);
		ret += lcd_kit_dual_dsi_cmd_tx(panel_id, &dsi_cmd0, &dsi_cmd1, LOCK);
		lcd_kit_delay(cmds0->cmds[i].wait, cmds0->cmds[i].waittype, true);
	}

	LCD_KIT_DEBUG("send cmds0 len = %d\n", cmds0->cmd_cnt);
	if (link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(panel_id, LCD_KIT_DSI_HS_MODE);
	up(&DISP_LOCK->lcd_kit_sem);

	return ret;
}

static int lcd_kit_gpio_enable(int panel_id, unsigned int type)
{
	lcd_kit_gpio_tx(panel_id, type, GPIO_REQ);
	lcd_kit_gpio_tx(panel_id, type, GPIO_HIGH);
	return LCD_KIT_OK;
}

static int lcd_kit_gpio_disable(int panel_id, unsigned int type)
{
	lcd_kit_gpio_tx(panel_id, type, GPIO_LOW);
	lcd_kit_gpio_tx(panel_id, type, GPIO_FREE);
	return LCD_KIT_OK;
}

static int lcd_kit_gpio_disable_plugin(unsigned int type)
{
	lcd_kit_gpio_tx_plugin(type, GPIO_LOW);
	lcd_kit_gpio_tx_plugin(type, GPIO_FREE);
	return LCD_KIT_OK;
}

static int lcd_kit_gpio_enable_nolock(int panel_id, unsigned int type)
{
	lcd_kit_gpio_tx(panel_id, type, GPIO_REQ);
	lcd_kit_gpio_tx(panel_id, type, GPIO_HIGH);
	lcd_kit_gpio_tx(panel_id, type, GPIO_FREE);
	return LCD_KIT_OK;
}

static int lcd_kit_gpio_disable_nolock(int panel_id, unsigned int type)
{
	lcd_kit_gpio_tx(panel_id, type, GPIO_REQ);
	lcd_kit_gpio_tx(panel_id, type, GPIO_LOW);
	lcd_kit_gpio_tx(panel_id, type, GPIO_FREE);
	return LCD_KIT_OK;
}

static int lcd_kit_regulator_enable(int panel_id, unsigned int type, int enable)
{
	int ret = LCD_KIT_OK;

	switch (type) {
	case LCD_KIT_VCI:
	case LCD_KIT_IOVCC:
	case LCD_KIT_VDD:
		/* to ensure ap power vote is right */
		if (lcd_kit_get_power_status(panel_id) == 0 && enable != 0 &&
			common_info->fake_power_off_state == true)
			ret = lcd_kit_pmu_ctrl(panel_id, type, 0);
		ret = lcd_kit_pmu_ctrl(panel_id, type, enable);
		break;
	default:
		ret = LCD_KIT_FAIL;
		LCD_KIT_ERR("regulator type:%d not support\n", type);
		break;
	}
	return ret;
}

static int lcd_kit_buf_trans(const char *inbuf, int inlen,
	char **outbuf, int *outlen)
{
	char *buf = NULL;
	int bufsize;
	int i;

	if (!inbuf) {
		LCD_KIT_ERR("inbuf is null point\n");
		return LCD_KIT_FAIL;
	}
	/* The property is 4bytes long per element in cells: <> */
	bufsize = inlen / 4;
	if (bufsize <= 0) {
		LCD_KIT_ERR("bufsize is less 0\n");
		return LCD_KIT_FAIL;
	}
	/* If use bype property: [], this division should be removed */
	buf = kzalloc(sizeof(char) * bufsize, GFP_KERNEL);
	if (!buf) {
		LCD_KIT_ERR("buf is null point\n");
		return LCD_KIT_FAIL;
	}
	// For use cells property: <>
	for (i = 0; i < bufsize; i++)
		buf[i] = inbuf[i * 4 + 3];
	*outbuf = buf;
	*outlen = bufsize;
	return LCD_KIT_OK;
}

static int lcd_kit_check_nv_info(uint32_t panel_id, char *info,
	uint32_t info_len)
{
	if (!info || info_len == 0) {
		LCD_KIT_ERR("invalid info\n");
		return LCD_KIT_FAIL;
	}
	if (memset_s(&disp_info->nv_info, sizeof(disp_info->nv_info), 0,
		sizeof(disp_info->nv_info)) != EOK) {
		LCD_KIT_ERR("memset_s fail\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

static int lcd_kit_read_nv_info(uint32_t panel_id, int nv_num,
	int valid_size, char *info, uint32_t info_len)
{
	int ret;

	if (lcd_kit_check_nv_info(panel_id, info, info_len)) {
		LCD_KIT_ERR("check nv info fail\n");
		return LCD_KIT_FAIL;
	}
	disp_info->nv_info.nv_operation = NV_READ;
	disp_info->nv_info.nv_number = nv_num;
	disp_info->nv_info.valid_size = valid_size;
	if (strncpy_s(disp_info->nv_info.nv_name, sizeof(disp_info->nv_info.nv_name),
		"LCD_STA", sizeof("LCD_STA")) != EOK) {
		LCD_KIT_ERR("strncpy_s fail\n");
		return LCD_KIT_FAIL;
	}
	disp_info->nv_info.nv_name[sizeof(disp_info->nv_info.nv_name) - 1] = '\0';
	ret = nve_direct_access_interface(&disp_info->nv_info);
	if (ret != 0) {
		LCD_KIT_ERR("read nv %d fail\n", nv_num);
		return LCD_KIT_FAIL;
	}
	if (memcpy_s(info, info_len, disp_info->nv_info.nv_data, valid_size) != EOK) {
		LCD_KIT_ERR("memcpy_s fail\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

static int lcd_kit_write_nv_info(uint32_t panel_id, int nv_num,
	int valid_size, char *info, uint32_t info_len)
{
	int ret;

	if (lcd_kit_check_nv_info(panel_id, info, info_len)) {
		LCD_KIT_ERR("check nv info fail\n");
		return LCD_KIT_FAIL;
	}
	disp_info->nv_info.nv_operation = NV_WRITE;
	disp_info->nv_info.nv_number = nv_num;
	disp_info->nv_info.valid_size = valid_size;
	if (strncpy_s(disp_info->nv_info.nv_name, sizeof(disp_info->nv_info.nv_name),
		"LCD_STA", sizeof("LCD_STA")) != EOK) {
		LCD_KIT_ERR("strncpy_s fail\n");
		return LCD_KIT_FAIL;
	}
	disp_info->nv_info.nv_name[sizeof(disp_info->nv_info.nv_name) - 1] = '\0';
	if (memcpy_s(disp_info->nv_info.nv_data, sizeof(disp_info->nv_info.nv_data),
		info, info_len) != EOK) {
		LCD_KIT_ERR("memcpy_s fail\n");
		return LCD_KIT_FAIL;
	}
	ret = nve_direct_access_interface(&disp_info->nv_info);
	if (ret != 0) {
		LCD_KIT_ERR("write nv %d fail\n", nv_num);
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

struct lcd_kit_adapt_ops adapt_ops = {
	.mipi_tx = lcd_kit_cmds_tx,
	.mipi_tx_no_lock = lcd_kit_cmds_tx_no_lock,
	.dual_mipi_diff_cmd_tx = lcd_kit_dual_diff_cmds_tx,
	.mipi_rx = lcd_kit_cmds_rx,
	.gpio_enable = lcd_kit_gpio_enable,
	.gpio_disable = lcd_kit_gpio_disable,
	.gpio_disable_plugin = lcd_kit_gpio_disable_plugin,
	.gpio_enable_nolock = lcd_kit_gpio_enable_nolock,
	.gpio_disable_nolock = lcd_kit_gpio_disable_nolock,
	.regulator_enable = lcd_kit_regulator_enable,
	.buf_trans = lcd_kit_buf_trans,
	.read_nv_info = lcd_kit_read_nv_info,
	.write_nv_info= lcd_kit_write_nv_info,
};

int lcd_kit_adapt_init(void)
{
	int ret;

	ret = lcd_kit_adapt_register(&adapt_ops);
	return ret;
}
