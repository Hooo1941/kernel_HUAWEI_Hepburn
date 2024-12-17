/*
 * h01_3223_tcon_fw_update.c
 *
 * lcdkit utils function for lcd driver
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

#define H01_13P2_FW_NAME_TAIL "_fw.bin"
#define H01_13P2_IC_DATA_LEN 13
#define H01_13P2_FW_NAME_LEN 32
#define H01_13P2_FW_VER_INFO_LEN 128
#define H01_13P2_FW_PACKE_LEN 90
#define H01_13P2_FW_UPD_RETRY_TIME 16
#define H01_13P2_FW_UPD_SUCC BIT(0)
#define H01_13P2_FW_UPD_FAIL BIT(1)
#define H01_13P2_FW_UPD_DELAY 500

typedef struct {
	int (*func_ptr)(uint32_t panel_id, struct lcd_kit_tcon_update *upd,
		struct lcd_kit_adapt_ops *ops);
} h01_13p2_upd_event_handle;

static struct h01_13p2_tcon_upd_time {
	ktime_t start;
	ktime_t end;
	s64 actual_time;
} h01_13p2_tcon_upd_time;

static struct h01_13p2_fw_info {
	char name[H01_13P2_FW_NAME_LEN];
	const struct firmware *fw;
	struct device *dev;
} h01_13p2_fw_info;

static char h01_13p2_ic_data[H01_13P2_IC_DATA_LEN];
static struct h01_13p2_fw_info g_h01_13p2_fw_info;

static int h01_13p2_upd_dsi_cmds_tx_sub(uint32_t panel_id, struct dsi_cmd_desc dsi_cmd,
	char *payload, struct lcd_kit_tcon_update *upd, struct lcd_kit_adapt_ops *ops)
{
	int i;
	int ret;
	struct h01_13p2_tcon_upd_time upd_time;
	uint32_t cmds_num;
	uint32_t cmds_offset = 0;
	unsigned int dsi0_index;

	lcd_kit_get_dsi_index(&dsi0_index, DPU_PINFO->connector_id);

	cmds_num = g_h01_13p2_fw_info.fw->size / H01_13P2_FW_PACKE_LEN +
		((g_h01_13p2_fw_info.fw->size % H01_13P2_FW_PACKE_LEN) ? 1 : 0);
	LCD_KIT_INFO("cmd_num is %u!\n", cmds_num);

	upd_time.start = ktime_get();
	down(&DISP_LOCK->lcd_kit_sem);
	for (i = 0; i < cmds_num; i++) {
		memset_s(payload, dsi_cmd.dlen, 0, dsi_cmd.dlen);
		payload[0] = (i == 0 ? 0x2C : 0x3C);

		if (g_h01_13p2_fw_info.fw->size - cmds_offset >= H01_13P2_FW_PACKE_LEN) {
			memcpy_s(payload + 1, H01_13P2_FW_PACKE_LEN,
				g_h01_13p2_fw_info.fw->data + cmds_offset, H01_13P2_FW_PACKE_LEN);
			cmds_offset += H01_13P2_FW_PACKE_LEN;
		} else {
			memcpy_s(payload + 1, g_h01_13p2_fw_info.fw->size - cmds_offset,
				g_h01_13p2_fw_info.fw->data + cmds_offset,
				g_h01_13p2_fw_info.fw->size - cmds_offset);
			cmds_offset += (g_h01_13p2_fw_info.fw->size - cmds_offset);
		}

		ret = mipi_dsi_cmds_tx_for_usr(dsi0_index, &dsi_cmd, 1, DPU_PINFO->type);
		if (ret < 0) {
			LCD_KIT_ERR("mipi_dsi_cmds_tx_for_usr failed\n");
			goto error;
		}
	}
	ret = LCD_KIT_OK;

error:
	up(&DISP_LOCK->lcd_kit_sem);
	upd_time.end = ktime_get();
	upd_time.actual_time = ktime_to_ms(ktime_sub(upd_time.end, upd_time.start));
	LCD_KIT_INFO("actual_time %u ms!\n", (unsigned int)(upd_time.actual_time));
	return ret;
}

static int h01_13p2_upd_dsi_cmds_tx(uint32_t panel_id,
	struct lcd_kit_tcon_update *upd, struct lcd_kit_adapt_ops *ops)
{
	int ret = LCD_KIT_OK;
	struct dsi_cmd_desc dsi_cmd;
	char *payload = NULL;

	if (g_h01_13p2_fw_info.fw->data == NULL ||
		g_h01_13p2_fw_info.fw->size == 0) {
		LCD_KIT_ERR("data is NULL or size = 0\n");
		return LCD_KIT_FAIL;
	}

	payload = kzalloc(H01_13P2_FW_PACKE_LEN + 1, GFP_KERNEL);
	if (!payload) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}

	memset_s(&dsi_cmd, sizeof(struct dsi_cmd_desc), 0, sizeof(struct dsi_cmd_desc));
	dsi_cmd.payload = payload;
	dsi_cmd.dlen = H01_13P2_FW_PACKE_LEN + 1;
	dsi_cmd.dtype = DTYPE_DCS_LWRITE; /* long write 0x39 */

	ret = h01_13p2_upd_dsi_cmds_tx_sub(panel_id, dsi_cmd, payload, upd, ops);

	kfree(payload);
	payload = NULL;
	return ret;
}

static void h01_13p2_release_firmware(void)
{
	if (g_h01_13p2_fw_info.fw) {
		release_firmware(g_h01_13p2_fw_info.fw);
		g_h01_13p2_fw_info.fw = NULL;
	}
}

static int h01_13p2_fw_version_check(void)
{
	char ic_text[H01_13P2_FW_VER_INFO_LEN] = { 0 };
	char bin_text[H01_13P2_FW_VER_INFO_LEN] = { 0 };
	int i;

	for (i = 0; i < H01_13P2_IC_DATA_LEN; i++) {
		sprintf_s(ic_text + (i << 1), H01_13P2_FW_VER_INFO_LEN - (i << 1),
			"%02x", h01_13p2_ic_data[i]);
		sprintf_s(bin_text + (i << 1), H01_13P2_FW_VER_INFO_LEN - (i << 1),
			"%02x", g_h01_13p2_fw_info.fw->data[i]);
	}
	LCD_KIT_INFO("ic [%u]='%s'\n", H01_13P2_IC_DATA_LEN, ic_text);
	LCD_KIT_INFO("bin[%u]='%s'\n", H01_13P2_IC_DATA_LEN, bin_text);

	/* check data & version */
	if (h01_13p2_ic_data[8] != g_h01_13p2_fw_info.fw->data[8])
		return LCD_KIT_FAIL;

	return ((h01_13p2_ic_data[4] == g_h01_13p2_fw_info.fw->data[4]) &&
		(h01_13p2_ic_data[5] == g_h01_13p2_fw_info.fw->data[5]) &&
		(h01_13p2_ic_data[6] == g_h01_13p2_fw_info.fw->data[6]) &&
		(h01_13p2_ic_data[7] == g_h01_13p2_fw_info.fw->data[7])) ? LCD_KIT_OK : LCD_KIT_FAIL;
}

static int h01_13p2_get_fw_name(int panel_id)
{
	int ret;

	memset_s(&g_h01_13p2_fw_info, sizeof(g_h01_13p2_fw_info),
		0, sizeof(g_h01_13p2_fw_info));

	ret = strncpy_s(g_h01_13p2_fw_info.name, H01_13P2_FW_NAME_LEN,
		disp_info->compatible, strlen(disp_info->compatible));
	if (ret) {
		LCD_KIT_ERR("parse fw name head faile\n");
		return LCD_KIT_FAIL;
	}
	ret = strncat_s(g_h01_13p2_fw_info.name, H01_13P2_FW_NAME_LEN,
		H01_13P2_FW_NAME_TAIL, strlen(H01_13P2_FW_NAME_TAIL));
	if (ret) {
		LCD_KIT_ERR("parse fw name tail faile\n");
		return LCD_KIT_FAIL;
	}

	return LCD_KIT_OK;
}

static int h01_13p2_fw_update_check(uint32_t panel_id,
	struct lcd_kit_tcon_update *upd, struct lcd_kit_adapt_ops *ops)
{
	int ret;

	/* get fw name */
	ret = h01_13p2_get_fw_name(panel_id);
	if (ret) {
		LCD_KIT_ERR("get_fw_name failed, ret = %d\n", ret);
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("firmware name = %s!\n", g_h01_13p2_fw_info.name);

	/* request_firmware */
	if (!g_h01_13p2_fw_info.fw && !g_h01_13p2_fw_info.dev) {
		ret = request_firmware(&g_h01_13p2_fw_info.fw,
			g_h01_13p2_fw_info.name, g_h01_13p2_fw_info.dev);
		if (ret) {
			LCD_KIT_ERR("request_firmware failed, ret = %d\n", ret);
			return LCD_KIT_FAIL;
		}
	}
	/* firmware size check */
	if (g_h01_13p2_fw_info.fw->size < H01_13P2_FW_VER_INFO_LEN) {
		LCD_KIT_ERR("firmware size is invalid fw_size = %zu\n",
			g_h01_13p2_fw_info.fw->size);
		return LCD_KIT_FAIL;
	}

	/* read ic version data */
	ret = ops->mipi_rx(panel_id, NULL, h01_13p2_ic_data,
		sizeof(h01_13p2_ic_data), &upd->get_version_cmds);
	if (ret)
		return LCD_KIT_FAIL;

	/* check version */
	ret = h01_13p2_fw_version_check();
	if (ret == LCD_KIT_OK) {
		LCD_KIT_INFO("same version 0x%02x 0x%02x and data is also same\n",
			g_h01_13p2_fw_info.fw->data[8], h01_13p2_ic_data[8]);
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("diff version need update tcon fw\n");

	return LCD_KIT_OK;
}

static int h01_13p2_fw_update_start(uint32_t panel_id,
	struct lcd_kit_tcon_update *upd, struct lcd_kit_adapt_ops *ops)
{
	int ret;

	LCD_KIT_INFO("+\n");
	ret = ops->mipi_tx(panel_id, NULL, &upd->start_cmds);
	if (ret)
		return LCD_KIT_FAIL;

	LCD_KIT_INFO("-\n");
	return ret;
}

static int h01_13p2_fw_update_do(uint32_t panel_id,
	struct lcd_kit_tcon_update *upd, struct lcd_kit_adapt_ops *ops)
{
	int ret;

	LCD_KIT_INFO("+\n");
	ret = h01_13p2_upd_dsi_cmds_tx(panel_id, upd, ops);
	if (ret)
		return LCD_KIT_FAIL;

	LCD_KIT_INFO("-\n");
	return ret;
}

static int h01_13p2_fw_update_end(uint32_t panel_id,
	struct lcd_kit_tcon_update *upd, struct lcd_kit_adapt_ops *ops)
{
	int ret;

	LCD_KIT_INFO("+\n");
	ret = ops->mipi_tx(panel_id, NULL, &upd->end_cmds);
	if (ret)
		return LCD_KIT_FAIL;

	LCD_KIT_INFO("-\n");
	return ret;
}

static int h01_13p2_fw_update_result(uint32_t panel_id,
	struct lcd_kit_tcon_update *upd, struct lcd_kit_adapt_ops *ops)
{
	int ret;
	int retry_times = H01_13P2_FW_UPD_RETRY_TIME;
	uint8_t read_status = 0;

	LCD_KIT_INFO("+\n");
	while (retry_times > 0) {
		ret = ops->mipi_rx(panel_id, NULL, &read_status, 1, &upd->get_status_cmds);
		if (ret)
			LCD_KIT_ERR("mipi_rx error\n");

		LCD_KIT_INFO("read_status 0x%02x\n", read_status);
		if (!read_status) {
			mdelay(H01_13P2_FW_UPD_DELAY);
			retry_times--;
		} else if (read_status & H01_13P2_FW_UPD_SUCC) {
			break;
		} else {
			return LCD_KIT_FAIL;
		}
	}
	LCD_KIT_INFO("-\n");
	return  retry_times > 0 ? LCD_KIT_OK : LCD_KIT_FAIL;
}

static const h01_13p2_upd_event_handle g_h01_13p2_upd_handle[] = {
	{ h01_13p2_fw_update_start },
	{ h01_13p2_fw_update_do },
	{ h01_13p2_fw_update_end },
	{ h01_13p2_fw_update_result },
};

static bool h01_13p2_need_update_fw(uint32_t panel_id)
{
	int ret;
	struct lcd_kit_tcon_update *upd = &disp_info->update_info;
	struct lcd_kit_adapt_ops *ops = NULL;

	ops = lcd_kit_get_adapt_ops();
	if (!ops) {
		LCD_KIT_ERR("can not get adapt_ops\n");
		ret = LCD_KIT_FAIL;
		return (ret != LCD_KIT_FAIL);
	}

	LCD_KIT_INFO("+\n");
	ret = h01_13p2_fw_update_check(panel_id, upd, ops);
	h01_13p2_release_firmware();
	LCD_KIT_INFO("-\n");
	return (ret == LCD_KIT_OK);
}

static int h01_13p2_update_fw(uint32_t panel_id)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_tcon_update *upd = &disp_info->update_info;
	int i = 0;
	int table_num = sizeof(g_h01_13p2_upd_handle) / sizeof(h01_13p2_upd_event_handle);
	struct lcd_kit_adapt_ops *ops = NULL;

	ops = lcd_kit_get_adapt_ops();
	if (!ops) {
		LCD_KIT_ERR("can not get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}

	if (!ops->mipi_rx || !ops->mipi_tx) {
		LCD_KIT_ERR("mipi_rx or mipi_tx is NULL\n");
		return LCD_KIT_FAIL;
	}

	LCD_KIT_INFO("+\n");
	if (h01_13p2_fw_update_check(panel_id, upd, ops) != LCD_KIT_OK) {
		LCD_KIT_INFO("no need do update\n");
		goto upd_end;
	}
	/* update bin status is doing */
	upd->tcon_upd_status = TCON_UPDATE_DOING;
	for (i = 0; i < table_num && g_h01_13p2_upd_handle[i].func_ptr; i++) {
		ret = g_h01_13p2_upd_handle[i].func_ptr(panel_id, upd, ops);
		if (ret) {
			/* update bin status is failed */
			upd->tcon_upd_status = TCON_UPDATE_FAIL;
			/* update bin switch is diable */
			upd->tcon_upd_switch = TCON_UPDATE_DISABLE;
			h01_13p2_release_firmware();
			return LCD_KIT_FAIL;
		}
	}

upd_end:
	/* update bin status is success */
	upd->tcon_upd_status = TCON_UPDATE_SUCCESS;
	/* update bin switch is disable */
	upd->tcon_upd_switch = TCON_UPDATE_DISABLE;
	h01_13p2_release_firmware();
	LCD_KIT_INFO("-\n");
	return LCD_KIT_OK;
}

static struct lcd_kit_panel_ops h01_13p2_ops = {
	.lcd_kit_need_update_fw = h01_13p2_need_update_fw,
	.lcd_kit_update_fw = h01_13p2_update_fw,
};

static int h01_13p2_probe(struct platform_device *pdev)
{
	int ret;
	g_h01_13p2_fw_info.dev = &(pdev->dev);

	ret = lcd_kit_panel_ops_register(&h01_13p2_ops);
	if (ret) {
		LCD_KIT_ERR("register lcd_kit_panel_ops failed\n");
		return LCD_KIT_FAIL;
	}

	return LCD_KIT_OK;
}
