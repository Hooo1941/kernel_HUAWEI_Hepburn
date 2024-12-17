/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: local hbm interface.
 * Author: guoleilei wx1018007
 * Create: 2023-07-10
 */

#ifndef __LCD_KIT_LOCAL_HBM__
#define __LCD_KIT_LOCAL_HBM__

#include <linux/kernel.h>

#include "lcd_kit_common.h"

int display_engine_gamma_code_print_for_c08(int panel_id);
int display_engine_circle_code_print_for_c08(int panel_id);
int display_engine_set_grayscale_to_common_info_for_c08(int panel_id, int grayscale);
int display_engine_local_hbm_set_dbv_for_c08(int panel_id, uint32_t dbv, bool is_on);
int display_engine_ddic_alpha_is_on_for_c08(int panel_id, void *hld, uint32_t alpha, bool is_on);
int display_engine_ddic_alpha_is_on_ctrl(int panel_id, void *hld, uint32_t alpha, bool is_on);
int display_engine_set_grayscale_to_common_info_ctrl(int panel_id, void *hld, int grayscale);
int display_engine_local_hbm_set_dbv_ctrl(int panel_id, uint32_t dbv,
	uint32_t dbv_threshold, bool is_on);
int display_engine_circle_code_print_ctrl(int panel_id);
void lcd_kit_local_hbm_cmds_parse(int panel_id, struct device_node *np);
bool display_engine_local_hbm_ddic_valid(int panel_id);
void display_engine_circle_cmds_set_payload(
	int panel_id, int index, int payload_num, char value, bool is_on);
bool display_engine_local_hbm_is_gamma_read(int panel_id);
int display_engine_copy_color_cmds_to_common_info_for_f01(int fps,
	struct color_cmds_rgb *color_cmds);
int display_engine_gamma_code_print_for_f01(int panel_id);
int display_engine_ddic_alpha_is_on_for_f01(int panel_id, void *hld, uint32_t alpha,
	bool is_on);
int display_engine_local_hbm_set_dbv_for_f01(int panel_id, uint32_t dbv,
	uint32_t dbv_threshold, bool is_on);
void display_engine_local_hbm_mipi_rx_fps_cmd(
	void *hld, int fps, uint8_t *hbm_gamma, size_t rgb_size, bool is_ext);
void display_engine_local_hbm_gamma_read_fps_for_f01(int panel_id, void *hld, int fps);
int display_engine_set_hbm_gamma_to_common_info_fps_for_f01(int panel_id,
	int fps, uint8_t *hbm_gamma, size_t rgb_size);
int display_engine_local_hbm_set_color_by_grayscale_ctrl(int panel_id, void *hld,
	int grayscale);
void display_engine_local_hbm_gamma_read_ctrl(int panel_id, void *hld);
#endif
