/* Copyright (c) 2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef HDMIRX_PHY_H
#define HDMIRX_PHY_H

#include "hdmirx_struct.h"

#define RTERM_VALUE 9 /* 9: rterm value */
#define SLEEP_TIME_CNT 10
#define FIX_EQ_CNT 7
#define PHY_STATE_OFF_TIME_CNT 300
#define PHY_STATE_HANDLE_EQ_TIME_CNT 300
#define PHY_STATE_HANDLE_EQ_REPEAT_CNT 3
#define PHY_STATE_20_AUTO_EQ_TIME_CNT 300
#define PHY_STATE_20_AUTO_EQ_REPEAT_CNT 2
#define PHY_STATE_21_AUTO_EQ_TIME_CNT 60
#define PHY_STATE_21_AUTO_EQ_REPEAT_CNT 1
#define PHY_STATE_21_AUTO_DFE_TIME_CNT 15

typedef enum {
	PHY_STATE_OFF,
	PHY_STATE_HANDLE_EQ,
	PHY_STATE_AUTO_EQ,
	PHY_STATE_DFE,
	PHY_STATE_DONE,
	PHY_STATE_MAX
} phy_state;

typedef enum ext_hdmirx_damix_type {
	HDMIRX_DAMIX_PLL,
	HDMIRX_DAMIX_CLK,
	HDMIRX_DAMIX_AUTO_EQ,
	HDMIRX_DAMIX_FIX_EQ,
	HDMIRX_DAMIX_DFE,
	HDMIRX_DAMIX_MAX
}hdmirx_damix_type;

typedef enum ext_hdmirx_damix_status {
	HDMIRX_DAMIX_STATUS_FAIL,
	HDMIRX_DAMIX_STATUS_DONE,
	HDMIRX_DAMIX_STATUS_MAX
} hdmirx_damix_status;

typedef enum {
	PHY_EQ_STATE_OFF,
	PHY_EQ_STATE_HANDLE_CHECKING,
	PHY_EQ_STATE_HANDLE_DONE,
	PHY_EQ_STATE_HANDLE_TIMEOUT,
	PHY_EQ_STATE_AUTO_CHECKING,
	PHY_EQ_STATE_AUTO_DONE,
	PHY_EQ_STATE_AUTO_TIMEOUT,
	PHY_EQ_STATE_MAX
} phy_eq_state;

typedef enum {
	PHY_DFE_STATE_OFF,
	PHY_DFE_STATE_CHECKING,
	PHY_DFE_STATE_DONE,
	PHY_DFE_STATE_TIMEOUT,
	PHY_DFE_STATE_MAX
} phy_dfe_state;

typedef enum ext_hdmirx_dfe_mode {
	HDMIRX_DFE_MODE_HARDWARE,
	HDMIRX_DFE_MODE_SOFTWARE,
	HDMIRX_DFE_MODE_FORCE,
	HDMIRX_DFE_MODE_DISABLE,
	HDMIRX_DFE_MODE_MAX
} hdmirx_dfe_mode;

typedef struct {
	bool run;
	phy_state state;
	phy_eq_state eq_state;
	phy_dfe_state dfe_state;
	hdmirx_input_type type;
	hdmirx_dfe_mode dfe_mode;
	uint8_t repeat_cnt;
	uint32_t wait_cnt;
	bool force_dfe;
	uint8_t dfe_code;
	uint32_t fpga_phy_status;
	uint32_t fpga_phy_init;
	uint32_t fpga_phy_cnt;
	bool is_ckdt;
} hdmirx_phy_ctx;

static uint32_t g_pd_cfg[10][8] = { /* 10 power mode cfg for HDMI input , 8 for regs */
	/* RG_PD_RT, RG_PD_PHDAC, RG_PD_LDO, RG_PD_LANE, RG_PD_CLK, RG_PD_BG, RG_PD_PLL, RG_PD_LDO_NODIE */
	{ 0xf, 0xf, 0x1, 0xf, 0x1, 0x1, 0x1, 0x1 }, /* OFF */
	{ 0x0, 0x8, 0x0, 0x8, 0x0, 0x0, 0x0, 0x0 }, /* TMDS1.4 */
	{ 0x0, 0xf, 0x0, 0x8, 0x0, 0x0, 0x0, 0x0 }, /* TMDS2.0 */
	{ 0x0, 0xf, 0x0, 0x8, 0x1, 0x0, 0x0, 0x0 }, /* FRL3L3G */
	{ 0x0, 0xf, 0x0, 0x8, 0x1, 0x0, 0x0, 0x0 }, /* FRL3L6G */
	{ 0x0, 0xf, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0 }, /* FRL4L6G */
	{ 0x0, 0xf, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0 }, /* FRL4L8G */
	{ 0x0, 0xf, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0 }, /* FRL4L10G */
	{ 0x0, 0xf, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0 }, /* FRL4L12G */
	{ 0xf, 0xf, 0x1, 0xf, 0x1, 0x1, 0x1, 0x1 }, /* MAX */
};

int hdmirx_phy_init(struct hdmirx_ctrl_st *hdmirx);
bool hdmirx_ctrl_set_phy_power(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type);
void hdmirx_phy_proc(struct hdmirx_ctrl_st *hdmirx);
void hdmirx_ctrl_phy_handler(struct hdmirx_ctrl_st *hdmirx);

int hdmirx_fpga_phy_clear(struct hdmirx_ctrl_st *hdmirx);
int hdmirx_fpga_phy_init(struct hdmirx_ctrl_st *hdmirx);
void hdmirx_phy_ckdt_init(void);
void phy_clear_init(void);
void hdmirx_rterm_ctrl_set(struct hdmirx_ctrl_st *hdmirx, uint8_t rterm);
void hdmirx_phy_set_en(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type, bool en);
phy_eq_state  hdmirx_phy_get_eq_state(struct hdmirx_ctrl_st *hdmirx);
void phy_set_power(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type);
#endif