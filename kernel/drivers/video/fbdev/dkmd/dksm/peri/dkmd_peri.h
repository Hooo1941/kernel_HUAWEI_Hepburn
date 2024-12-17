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

#ifndef _DKMD_PERI_H_
#define _DKMD_PERI_H_

#include <linux/wait.h>
#include <linux/kernel.h>
#include <linux/bitops.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/regulator/consumer.h>

#include "dkmd_connector.h"

#define GEN_VID_LP_CMD BIT(24) /* vid lowpwr cmd write */
/* dcs read/write */
#define DTYPE_DCS_WRITE 0x05 /* short write, 0 parameter */
#define DTYPE_DCS_WRITE1 0x15 /* short write, 1 parameter */
#define DTYPE_DCS_WRITE2 0x07 /* short write, 2 parameter */
#define DTYPE_DCS_READ 0x06 /* read */
#define DTYPE_DCS_LWRITE 0x39 /* long write */
#define DTYPE_DSC_LWRITE 0x0A /* dsc dsi1.2 vase3x long write */

/* generic read/write */
#define DTYPE_GEN_WRITE 0x03 /* short write, 0 parameter */
#define DTYPE_GEN_WRITE1 0x13 /* short write, 1 parameter */
#define DTYPE_GEN_WRITE2 0x23 /* short write, 2 parameter */
#define DTYPE_GEN_LWRITE 0x29 /* long write */
#define DTYPE_GEN_READ 0x04 /* long read, 0 parameter */
#define DTYPE_GEN_READ1 0x14 /* long read, 1 parameter */
#define DTYPE_GEN_READ2 0x24 /* long read, 2 parameter */

#define DTYPE_TEAR_ON 0x35 /* set tear on */
#define DTYPE_MAX_PKTSIZE 0x37 /* set max packet size */
#define DTYPE_NULL_PKT 0x09 /* null packet, no data */
#define DTYPE_BLANK_PKT 0x19 /* blankiing packet, no data */

#define DTYPE_CM_ON 0x02 /* color mode off */
#define DTYPE_CM_OFF 0x12 /* color mode on */
#define DTYPE_PERIPHERAL_OFF 0x22
#define DTYPE_PERIPHERAL_ON 0x32

#define MIPIDSI_CMD_PKT_STATUS_OFFSET 0x0014
#define MIPIDSI_PHY_STATUS_OFFSET 0x009C
#define MIPIDSI_INT_ST0_OFFSET 0x00A8
#define MIPIDSI_INT_ST1_OFFSET 0x00AC

#define dsi_hdr_dtype(dtype) ((dtype) & 0x03f)
#define dsi_hdr_vc(vc) (((vc) & 0x03) << 6)
#define dsi_hdr_data1(data) (((data) & 0x0ff) << 8)
#define dsi_hdr_data2(data) (((data) & 0x0ff) << 16)
#define dsi_hdr_wc(wc) (((wc) & 0x0ffff) << 8)

#define dsi_pld_data1(data) ((data) & 0x0ff)
#define dsi_pld_data2(data) (((data) & 0x0ff) << 8)
#define dsi_pld_data3(data) (((data) & 0x0ff) << 16)
#define dsi_pld_data4(data) (((data) & 0x0ff) << 24)

enum {
	WAIT_TYPE_US = 0,
	WAIT_TYPE_MS,
};

/* dtype for vcc */
enum {
	DTYPE_VCC_GET,
	DTYPE_VCC_PUT,
	DTYPE_VCC_ENABLE,
	DTYPE_VCC_DISABLE,
	DTYPE_VCC_SET_VOLTAGE,
};

/* pinctrl operation */
enum {
	DTYPE_PINCTRL_GET,
	DTYPE_PINCTRL_STATE_GET,
	DTYPE_PINCTRL_SET,
	DTYPE_PINCTRL_PUT,
};

/* pinctrl state */
enum {
	DTYPE_PINCTRL_STATE_DEFAULT,
	DTYPE_PINCTRL_STATE_IDLE,
};

/* dtype for gpio */
enum {
	DTYPE_GPIO_REQUEST,
	DTYPE_GPIO_FREE,
	DTYPE_GPIO_INPUT,
	DTYPE_GPIO_OUTPUT,
};

enum panel_ops_cmd {
	PANEL_OPS_CMD_START = PIPE_OPS_CMD_MAX,

	CHECK_LCD_STATUS ,
	HANDLE_MIPI_ULPS,
	MIPI_DSI_PARTIAL_UPDATE,
	RESET_PARTIAL_UPDATE,
	LCD_SEND_HS_CMD,
	LCD_SET_DISPLAY_REGION,
	REGIST_POSTPROCESS,
	DUMP_EXCEPTION_INFO,
	SET_REFRESH_STATISTIC,
	GET_STATISTIC_CLEAR_FLAG,
	SET_PPC_CONFIG_ID,
	UPDATE_SAFE_FRM_RATE,
	PANEL_OPS_MAX,
	MIPI_DSI_BIT_CLK_UPT
};

struct gpio_desc {
	int32_t dtype;
	int32_t waittype;
	int32_t wait;
	char *label;
	uint32_t *gpio;
	int32_t value;
};

struct vcc_desc {
	int32_t dtype;
	char *label;
	struct regulator **regulator;
	int32_t min_uv;
	int32_t max_uv;
	int32_t waittype;
	int32_t wait;
};

struct pinctrl_data {
	struct pinctrl *p;
	struct pinctrl_state *pinctrl_def;
	struct pinctrl_state *pinctrl_idle;
};

struct pinctrl_cmd_desc {
	int32_t dtype;
	struct pinctrl_data *pctrl_data;
	int32_t mode;
};

int32_t peri_gpio_cmds_tx(struct gpio_desc *cmds, int32_t cnt);
int32_t peri_vcc_cmds_tx(struct platform_device *pdev, struct vcc_desc *cmds, int32_t cnt);
int32_t peri_pinctrl_cmds_tx(struct platform_device *pdev, struct pinctrl_cmd_desc *cmds, int32_t cnt);

int32_t pipeline_next_on(struct platform_device *pdev, struct dkmd_connector_info *pinfo);
int32_t pipeline_next_off(struct platform_device *pdev, struct dkmd_connector_info *pinfo);
int32_t pipeline_next_ops_handle(struct platform_device *pdev, struct dkmd_connector_info *pinfo,
	uint32_t ops_cmd_id, void *value);
bool check_addr_status_is_valid(const char __iomem* check_addr, uint32_t status,
	uint32_t udelay_time, uint32_t times);

#endif
