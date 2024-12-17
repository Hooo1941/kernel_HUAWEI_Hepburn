/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __MIPI_DSI_DEV_H__
#define __MIPI_DSI_DEV_H__

#include "dkmd_mipi_panel_info.h"

#define DPHYTX_TRSTOP_FLAG_TIMEOUT_TIMES (10)
#define DSI_ADDR_TO_OFFSET (0)
#define READ_MAX 100
#define BUF_MAX (4 * READ_MAX)
#define TX_CMD_MAX 1000

enum {
	DSI_LANE_NUMS_DEFAULT = 0,
	DSI_1_LANES_SUPPORT = BIT(0),
	DSI_2_LANES_SUPPORT = BIT(1),
	DSI_3_LANES_SUPPORT = BIT(2),
	DSI_4_LANES_SUPPORT = BIT(3),
};

enum {
	EN_DSI_TX_NORMAL_MODE = 0x0,
	EN_DSI_TX_LOW_PRIORITY_DELAY_MODE = 0x1,
	EN_DSI_TX_HIGH_PRIORITY_DELAY_MODE = 0x2,
	EN_DSI_TX_AUTO_MODE = 0xF,
};

enum MIPI_LP11_MODE {
	MIPI_NORMAL_LP11 = 0,
	MIPI_SHORT_LP11 = 1,
	MIPI_DISABLE_LP11 = 2,
};

enum V320_DPHY_VER {
	DPHY_VER_12 = 0,
	DPHY_VER_14,
	DPHY_VER_MAX
};

#define DSI_VIDEO_DST_FORMAT_RGB565 0
#define DSI_VIDEO_DST_FORMAT_RGB666 1
#define DSI_VIDEO_DST_FORMAT_RGB666_LOOSE 2
#define DSI_VIDEO_DST_FORMAT_RGB888 3

#define DSI_CMD_DST_FORMAT_RGB565 0
#define DSI_CMD_DST_FORMAT_RGB666 1
#define DSI_CMD_DST_FORMAT_RGB888 2

struct mipi_dsi_timing {
	uint32_t hsa;
	uint32_t hbp;
	uint32_t dpi_hsize;
	uint32_t width;
	uint32_t hline_time;

	uint32_t vsa;
	uint32_t vbp;
	uint32_t vactive_line;
	uint32_t vfp;
};

#define MAX_CMD_QUEUE_LOW_PRIORITY_SIZE 256
#define MAX_CMD_QUEUE_HIGH_PRIORITY_SIZE 128
#define CMD_AUTO_MODE_THRESHOLD (2 / 3)

struct dsi_delayed_cmd_queue {
	struct semaphore work_queue_sem;
	spinlock_t cmd_send_lock;
	struct dsi_cmd_desc cmd_queue_low_priority[MAX_CMD_QUEUE_LOW_PRIORITY_SIZE];
	uint32_t cmd_queue_low_priority_rd;
	uint32_t cmd_queue_low_priority_wr;
	spinlock_t cmd_queue_low_priority_lock;

	struct dsi_cmd_desc cmd_queue_high_priority[MAX_CMD_QUEUE_HIGH_PRIORITY_SIZE];
	uint32_t cmd_queue_high_priority_rd;
	uint32_t cmd_queue_high_priority_wr;
	spinlock_t cmd_queue_high_priority_lock;

	bool is_cmd_queue_low_priority_full;
	bool is_cmd_queue_low_priority_working;
	bool is_cmd_queue_high_priority_full;
	bool is_cmd_queue_high_priority_working;

	s64 timestamp_frame_start;
	s64 oneframe_time;
};

struct mipi_dual_dsi_param {
	uint32_t *value_out;
	char __iomem *dsi_base;
	struct dsi_cmd_desc *cmd;
	struct dsi_cmd_desc *p_cmdset;
	int is_timeout;
	int cmdset_cnt;
};

struct dpu_connector;
struct dkmd_connector_info;

void mipi_init(struct dpu_connector *connector);
int32_t mipi_dsi_cmds_tx(struct dsi_cmd_desc *cmds, int32_t cnt, char __iomem *dsi_base);
int32_t mipi_dsi_cmd_add(struct dsi_cmd_desc *cm, char __iomem *dsi_base);
int32_t mipi_dsi_lread_reg(uint32_t *out, int out_len, struct dsi_cmd_desc *cm, uint32_t len, char *dsi_base);
void delay_for_next_cmd_by_sleep(uint32_t wait, uint32_t waittype);
bool mipi_panel_check_reg(struct dpu_connector *connector);

int mipi_dsi_get_read_value(struct dsi_cmd_desc *dsi_cmd,
	uint8_t *dest, uint32_t *src, uint32_t len, bool little_endian_support);
bool mipi_dsi_cmd_is_read(struct dsi_cmd_desc *cm);
bool mipi_dsi_cmd_is_write(struct dsi_cmd_desc *cm);
int32_t mipi_dsi_fifo_idle_cycle(const char __iomem *dsi_base);
int mipi_dsi_fifo_is_full(const char __iomem *dsi_base);
void mipi_dsi_max_return_packet_size(struct dsi_cmd_desc *cm, char __iomem *dsi_base);
void mipi_dsi_sread_request(struct dsi_cmd_desc *cm, char __iomem *dsi_base);

void mipi_dsi_default_setup(struct dpu_connector *connector);
bool mipi_phy_status_check(const char __iomem *mipi_dsi_base, uint32_t expected_value);
void mipi_dsi_ulps_cfg(struct dpu_connector *connector, bool is_ulps);

void mipi_dsi_auto_ulps_config(struct mipi_dsi_timing *timing, struct dpu_connector *connector,
	char __iomem *mipi_dsi_base);
void mipi_auto_ulps_ctrl(struct dpu_connector *connector, bool is_auto_ulps);
int32_t  mipi_dual_dsi_cmds_tx(struct dsi_cmd_desc *cmd0, int cnt0, char __iomem *dsi_base_0,
	struct dsi_cmd_desc *cmd1, int cnt1, char __iomem *dsi_base_1, uint8_t tx_mode, bool need_check_fifo);
int32_t mipi_dual_dsi_cmds_rx(char __iomem *dsi_base_0, uint8_t *dsi0_out, char __iomem *dsi_base_1,
	uint8_t *dsi1_out, int out_len, struct dsi_cmd_desc *cmd,  bool little_endian_support);
void mipi_transfer_lock_init(void);

void mipi_dsi_dfr_update(struct dpu_connector *connector, int target_frmrate, int mode);
int32_t mipi_wait_ldi_vstate_idle(struct dpu_connector *connector, const void *value);

#endif
