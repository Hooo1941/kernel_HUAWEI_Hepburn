/** @file
 * Copyright (c) 2020-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "hdmirx_init.h"
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_device.h>
#include <linux/of_address.h>

#include "hdmirx_struct.h"
#include "hdcp/hdmirx_hdcp.h"
#include "hdcp/hdmirx_hdcp_tee.h"
#include "phy/hdmirx_phy.h"
#include "control/hdmirx_ctrl.h"
#include "link/hdmirx_link_hpd.h"
#include "link/hdmirx_link_edid.h"
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include "hdmirx_common.h"
#include "control/hdmirx_ctrl_irq.h"
#include "video/hdmirx_video.h"
#include "packet/hdmirx_packet.h"
#include "link/hdmirx_link_frl.h"
#include "video/hdmirx_video_dsc.h"
#include "dpu_conn_mgr.h"

#include <platform_include/see/efuse_driver.h>

static struct hdmirx_chr_dev_st *g_dpu_hdmirx_dev;
static struct task_struct *g_hdmirx_main_task = NULL;

static struct hdmirx_chr_dev_st *hdmirx_get_dev(void)
{
	return g_dpu_hdmirx_dev;
}

struct hdmirx_ctrl_st *hdmirx_get_dev_ctrl(void)
{
	if (g_dpu_hdmirx_dev == NULL) {
		disp_pr_info("[hdmirx]g_dpu_hdmirx_dev NULL\n");
		return NULL;
	}

	return &(g_dpu_hdmirx_dev->hdmirx_ctrl);
}

uint32_t hdmirx_get_layer_fmt(void)
{
	struct hdmirx_ctrl_st *hdmirx = hdmirx_get_dev_ctrl();

	return hdmirx_ctrl_get_layer_fmt(hdmirx);
}

void hdmirx_yuv422_set(uint32_t enable)
{
	struct hdmirx_ctrl_st *hdmirx = hdmirx_get_dev_ctrl();

	hdmirx_ctrl_yuv422_set(hdmirx, enable);
}

int hdmirx_display_dss_ready_func(uint32_t type, uint32_t port, uint32_t stamp)
{
	struct hdmirx_ctrl_st *hdmirx = hdmirx_get_dev_ctrl();
	if (hdmirx == NULL) {
		disp_pr_err("[hdmirx]hdmirx NULL\n");
		return -1;
	}
	if (!hdmirx->start_display_dss_ready)
		disp_pr_info("start display dss ready\n");

	hdmirx->start_display_dss_ready = true;
	return 0;
}

void hdmirx_display_hdmi_ready(struct hdmirx_ctrl_st *hdmirx)
{
	if (hdmirx == NULL) {
		disp_pr_err("[hdmirx]hdmirx NULL\n");
		return;
	}
	if (!hdmirx->start_display_hdmi_ready)
		disp_pr_info("start display hdmi ready\n");

	hdmirx->start_display_hdmi_ready = true;
}

bool hdmirx_display_is_ready(struct hdmirx_ctrl_st *hdmirx)
{
	static int cnt = 0;
	cnt++;
	if (cnt >= 100) {
		disp_pr_info("dss_ready: %d, hdmi_ready: %d, mute:%d\n",
			hdmirx->start_display_dss_ready, hdmirx->start_display_hdmi_ready, hdmirx_ctrl_get_sys_mute(hdmirx));
		cnt = 0;
	}

	return (hdmirx->start_display_dss_ready &&
			hdmirx->start_display_hdmi_ready);
}

static int hdmirx_pericrg_reset(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_set_reg(hdmirx->hdmirx_pericrg_base + 0x004, 0x4000, 32, 0);
	return 0;
}

static int hdmirx_hsdt1_clk_reset(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x34, 1, 1, 23);
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x34, 1, 1, 30);
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x80, 1, 1, 16);
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x80, 1, 1, 18);
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x34, 1, 1, 24);
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x34, 1, 1, 29);
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x34, 1, 1, 28);
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x10C, 0x50000000, 32, 0);
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x34, 1, 1, 26);
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x80, 1, 1, 15);
	disp_pr_info("[hdmirx] clk reset success\n");

	return 0;
}

static int hdmirx_clk_enable(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_set_reg(hdmirx->hdmirx_gpio010_base + 0x010, 0x100010, 32, 0);
	mdelay(2);
	hdmirx_set_reg(hdmirx->hdmirx_gpio010_base + 0x004, 0x40006, 32, 0);
	hdmirx_set_reg(hdmirx->hdmirx_pericrg_base + 0x000, 0x4000, 32, 0); // no
	return 0;
}

static int hdmirx_pll_ref_div(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_set_reg(hdmirx->hdmirx_sysctrl_base + 0x0004, 0xf1f, 32, 0);
	return 0;
}

static int hdmirx_hsdt1_crg_set(struct hdmirx_ctrl_st *hdmirx)
{
	unsigned int value;

	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x84, 1, 1, 17); // no

	disp_pr_info("[hdmirx] hsdt1 crg set\n");

	// APB clock
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x10, 1, 1, 2);
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x54, 1, 1, 22);

	// HDMI Media Clk, AON Clk 24Mhz, link, ref djtag
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x30, 1, 1, 23);  // open HDMI APB/DJTAG  // no
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x30, 1, 1, 30);  // open HDMI APB  // no
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x84, 1, 1, 16);  // reset HDMI DJTAG
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x84, 1, 1, 18);  // reset HDMI APB
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x30, 1, 1, 24);  // open HDMI 24m clk/PHY REF // no
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x30, 1, 1, 29); // no
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x30, 1, 1, 28);  //	HDMI media Clk link clock // no

	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x10C, 0x50005000, 32, 0); // no
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x30, 1, 1, 26); // open hdmi_link_clk // no

	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x104, 0x18000000, 32, 0); // no
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x84, 1, 1, 15);  // reset HDMI com rst
	// PLL5 525MHz clock for media.
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x620, 2, 2, 0); //
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x620, 1, 6, 2);  //
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x620, 0x36, 12, 8);  //
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x620, 0x7, 3, 20);  //
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x620, 0x3, 3, 23);  //
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x624, 0xb00000, 25, 0);  //
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x648, 0x8c000004, 32, 0);  //
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x64c, 0x00b40000, 32, 0);  //
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x650, 0x20100fa8, 32, 0);  //
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x654, 0x2404ff20, 32, 0);
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x658, 0x0004013f, 32, 0);
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x65c, 0x00004046, 32, 0);

	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x620, 1, 1, 0);
	mdelay(5);
	value = inp32(hdmirx->hdmirx_hsdt1_crg_base + 0x620);
	if (!(value & (1 << 26))) {
		disp_pr_err("[hdmirx] hsdt1 crg set fail\n");
		return -1;
	}
	hdmirx_set_reg(hdmirx->hdmirx_hsdt1_crg_base + 0x620, 0, 1, 1);

	disp_pr_info("[hdmirx] hsdt1 crg set success\n");

	return 0;
}

static int hdmirx_ddc_i2c_slaver_set(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_set_reg(hdmirx->hdmirx_ioc_base + 0x68, 1, 32, 0);
	hdmirx_set_reg(hdmirx->hdmirx_ioc_base + 0x6C, 1, 32, 0);
	hdmirx_set_reg(hdmirx->hdmirx_iocg_base + 0x868, 0x28, 32, 0);
	hdmirx_set_reg(hdmirx->hdmirx_iocg_base + 0x86C, 0x28, 32, 0);

	hdmirx_set_reg(hdmirx->hdmirx_iocg_base + 0xF34, 0x20, 32, 0); // asic
	hdmirx_set_reg(hdmirx->hdmirx_ioc_base + 0x734, 0x2, 32, 0);

	disp_pr_info("[hdmirx] hdmirx_ddc_i2c_slaver_set success\n");

	return 0;
}

static int hdmirx_edid_init(struct hdmirx_ctrl_st *hdmirx)
{
	return hdmirx_link_edid_init(hdmirx);
}

int hdmirx_irq_enable(struct hdmirx_ctrl_st *hdmirx)
{
	if (hdmirx == NULL) {
		disp_pr_err("[hdmirx]hdmirx NULL\n");
		return -1;
	}
	hdmirx_video_detect_enable(hdmirx);
	hdmirx_packet_depack_enable(hdmirx);
	hdmirx_video_dsc_init(hdmirx);
	return 0;
}

static int hdmirx_phy_in_thread(struct hdmirx_ctrl_st *hdmirx)
{
	if (hdmirx->fpga_flag == 0) {
		hdmirx_ctrl_phy_handler(hdmirx);
		hdmirx_phy_proc(hdmirx);
	} else {
		hdmirx_fpga_phy_init(hdmirx);
	}
	return 0;
}

static int hdmirx_start_display(struct hdmirx_ctrl_st *hdmirx)
{
	if ((!hdmirx->start_display) && hdmirx_display_is_ready(hdmirx)) {
		hdmirx_ctrl_source_select_hdmi(hdmirx);
		mdelay(200);
		hdmirx_video_start_display(hdmirx);
		hdmirx->start_display = true;
	}
	return 0;
}

static int hdmirx_main_task_thread(void *p)
{
	struct hdmirx_ctrl_st *hdmirx = (struct hdmirx_ctrl_st *)p;

	if (hdmirx == NULL) {
		disp_pr_err("[hdmirx]hdmirx null\n");
		return -1;
	}

	while (!kthread_should_stop()) {
		msleep(10);
		hdmirx_phy_in_thread(hdmirx);
		hdmirx_frl_process(hdmirx);
		hdmirx_start_display(hdmirx);
		hdmirx_hpd_retry(hdmirx);
	}
	return 0;
}

void hdmirx_create_main_task(struct hdmirx_ctrl_st *hdmirx)
{
	if (hdmirx == NULL) {
		disp_pr_err("[hdmirx]hdmirx NULL\n");
		return;
	}
	if (g_hdmirx_main_task == NULL) {
		g_hdmirx_main_task = kthread_create(hdmirx_main_task_thread, hdmirx, "hdmirx_main_task");
		if (IS_ERR(g_hdmirx_main_task)) {
			disp_pr_err("Unable to start kernel hdmirx_main_task\n");
			g_hdmirx_main_task = NULL;
			return;
		}
	}
	wake_up_process(g_hdmirx_main_task);
}

void hdmirx_destroy_main_task(struct hdmirx_ctrl_st *hdmirx)
{
	if (g_hdmirx_main_task) {
		kthread_stop(g_hdmirx_main_task);
		g_hdmirx_main_task = NULL;

		disp_pr_info("stop main task thread succ\n");
	}
}

/* for hotplug out */
int hdmirx_off(struct hdmirx_ctrl_st *hdmirx)
{
	disp_check_and_return((hdmirx == NULL), -EINVAL, err, "[hdmirx] hdmirx is NULL\n");

	disp_pr_info("[hdmirx]hdmi rx off enter\n");
	hdmirx_destroy_main_task(hdmirx);
	hdmirx_fpga_phy_clear(hdmirx);
	hdmirx_ctrl_irq_disable(hdmirx);
	if (hdmirx->fpga_flag == 1) {
		// phy reset
		hdmirx_set_reg(hdmirx->hdmirx_sysctrl_base + 0x084, 0x3ff, 32, 0);
		disp_pr_info("[hdmirx]reset phy 0x3ff\n");
	}

	// set true: hpd down
	hdmirx_link_hpd_set(hdmirx, false);

	// ctrl reset
	hdmirx_hsdt1_clk_reset(hdmirx);
	hdmirx_pericrg_reset(hdmirx);
	disp_pr_info("[hdmirx] hdmi rx off exit\n");
	return 0;
}

static int hdmirx_clk_off(struct hdmirx_ctrl_st *hdmirx)
{
	int i;
	disp_pr_info("[hdmirx]hdmirx_clk_off\n");
	for (i = 0; i < CLK_HDMI_MAX_IDX; i++)
		clk_disable_unprepare(hdmirx->clk_hdmirx[i]);

	return 0;
}

static int hdmirx_clk_on(struct hdmirx_ctrl_st *hdmirx)
{
	int i;
	disp_pr_info("[hdmirx]hdmirx_clk_on\n");
	for (i = 0; i < CLK_HDMI_MAX_IDX; i++) {
		if (clk_prepare_enable(hdmirx->clk_hdmirx[i]) != 0) {
			dpu_pr_err("[hdmirx] enable %d failed!\n", i);
			return -1;
		}
	}
	hdmirx_clk_enable(hdmirx);
	return 0;
}

void hdmirx_para_set(struct hdmirx_ctrl_st *hdmirx)
{
	if (g_eq_para) {
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ1, REG_EQ_COARSE_SYMBOL_DET_TIMES, 0x1f);
		/* 0x1f: hdmi pat detect times */
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ7, REG_HDMI_PAT_DET_TIMES1, 0x1f);
		/* 0xf0: counter period times */
	}
	if (g_pll_para) {
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_CLK_DETECT1, REG_CLK_STABLE_TIMES, 0xf0);
		/* 0xf0: ref lock timer */
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_PLL_LOCK0, REG_REF_LOCK_TIMER, 0xf0);
		/* 0x1: pll lock times */
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_PLL_LOCK0, REG_PLL_LOCK_TIMES, 0x1);
	}

	if (g_ddc_para) {
		/* set scl filter/sda filter is 84ns */
		hdmirx_reg_write(hdmirx->hdmirx_aon_base, REG_DDC_FILT_CTL, DDC_FILT_SEL, 0x2); /* 0x2 : 84ns */
		/* set sda_in/sda_out delay is 250ns */
		hdmirx_reg_write(hdmirx->hdmirx_aon_base, REG_DDC_DLY_CTL, SDA_IN_DLY_CNT, 0x6);  /* 0x6 : 6*42ns */
		hdmirx_reg_write(hdmirx->hdmirx_aon_base, REG_DDC_DLY_CTL, SDA_OUT_DLY_CNT, 0x6); /* 0x6 : 6*42ns */
		/* set sda timeout thresholds */
		hdmirx_reg_write(hdmirx->hdmirx_aon_base, REG_DDC_TOUT_RES, DDC_TOUT_RESHOLD, 0x64); /* 0x64 : timeout 100ms */
		hdmirx_reg_write(hdmirx->hdmirx_aon_base, REG_DDC_STRETCH_CTL, SCL_HOLD_RESH, 0x14); /* 0x14 : threshold 20us */
	}
}

static int hdmirx_phy_init_all(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_fpga_phy_clear(hdmirx);

	hdmirx_phy_ckdt_init();

	phy_clear_init();

	phy_set_power(hdmirx, HDMIRX_INPUT_OFF);

	return 0;
}

static int hdmirx_3v3_pd(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x1c0c8, 0x21FFC000, 32, 0); // 3.3v
	return 0;
}

int hdmirx_ctx_init(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx->pwrstatus = true;
	hdmirx->start_display_hdmi_ready = false;
	hdmirx->start_display_dss_ready = false;
	hdmirx->start_display = false;
	hdmirx->phy_done = false;

	return 0;
}
bool hdmirx_phy_is_calibrated(struct hdmirx_ctrl_st *hdmirx)
{
	uint32_t val[3] = { 0, 0, 0 };
	struct efuse_item item = { 0 };
	uint32_t ret;
	item.buf = val;
	item.buf_size = sizeof(val) / sizeof(uint32_t);
	item.item_vid = EFUSE_KERNEL_PHY_CALI;
	ret = efuse_read_item(&item);
	if (ret != EFUSE_OK) {
		dpu_pr_err("[HDMIRX] Read phy calibration efuse fail");
		return false;
	}
	return (val[2] & 0x1) ? true : false;
}

int hdmirx_rterm_fix(struct hdmirx_ctrl_st *hdmirx)
{
	uint32_t val = 0;
	struct efuse_item item = { 0 };
	uint32_t ret;

	if (!hdmirx_phy_is_calibrated(hdmirx)) {
		dpu_pr_info("[HDMIRX] phy_calibration is 0");
		return 0;
	}
	item.buf = &val;
	item.buf_size = sizeof(val) / sizeof(uint32_t);
	item.item_vid = EFUSE_KERNEL_HDMI_RTERM;
	ret = efuse_read_item(&item);
	if (ret != EFUSE_OK) {
		dpu_pr_err("[HDMIRX] Read rterm efuse fail");
		return -EINVAL;
	}
	disp_pr_info("[hdmirx]hdmirx_rterm_fix : %d\n", val);
	hdmirx_rterm_ctrl_set(hdmirx, val);
	return 0;
}
/* for hotplug in */
int hdmirx_on(struct hdmirx_ctrl_st *hdmirx)
{
	disp_check_and_return((hdmirx == NULL), -EINVAL, err, "[hdmirx] hdmirx is NULL\n");

	if (hdmirx->is_power_off == false) {
		disp_pr_info("[hdmirx]power 5v had already on\n");
		return 0;
	}

	disp_pr_info("[hdmirx]hdmi rx on enter\n");
	hdmirx_clk_on(hdmirx);
	// clock set
	hdmirx_hsdt1_crg_set(hdmirx);
	// Pll ref div
	hdmirx_pll_ref_div(hdmirx);

	hdmirx_ddc_i2c_slaver_set(hdmirx);

	hdmirx_rterm_fix(hdmirx);

	hdmirx_edid_init(hdmirx);

	hdmirx_hdcp_init(hdmirx);

	hdmirx_ctrl_init(hdmirx);

	hdmirx_ctrl_irq_enable(hdmirx);

	hdmirx_ctx_init(hdmirx);

	if (hdmirx->fpga_flag == 0) {
		hdmirx_link_frl_init(hdmirx);
		disp_pr_info("[hdmirx]fpga_flag frl_init\n");
	}

	hdmirx_phy_init_all(hdmirx);

	hdmirx_para_set(hdmirx);

	disp_pr_info("[hdmirx] hdmi rx on success. hpd need cmd\n");

	hdmirx_3v3_pd(hdmirx);
	hdmirx->is_power_off = false;
	return 0;
}

void hdmirx_power5v_on(struct hdmirx_ctrl_st *hdmirx_ctrl)
{
	disp_pr_info("[hdmirx]power 5v on\n");

	hdmirx_on(hdmirx_ctrl);

	// set true: hpd up
	hdmirx_link_hpd_set(hdmirx_ctrl, true);

	if (hdmirx_ctrl->fpga_flag == 0)
		hdmirx_link_frl_ready(hdmirx_ctrl);

	hdmirx_create_main_task(hdmirx_ctrl);
}

void hdmirx_hotplug_in(void)
{
	disp_pr_info("[hdmirx]hdmi insert\n");

	if (g_dpu_hdmirx_dev == NULL) {
		disp_pr_err("[hdmirx]g_dpu_hdmirx_dev NULL\n");
		return;
	}
	hdmirx_on(&(g_dpu_hdmirx_dev->hdmirx_ctrl));
}

void hdmirx_power5v_off(struct hdmirx_ctrl_st *hdmirx_ctrl)
{
	if (hdmirx_ctrl->is_power_off == true) {
		disp_pr_info("[hdmirx]power 5v had already off\n");
		return;
	}
	disp_pr_info("[hdmirx]power 5v off\n");

	hdmirx_video_stop_display(hdmirx_ctrl);

	hdmirx_off(hdmirx_ctrl);

	hdmirx_phy_ckdt_init();

	hdmirx_clk_off(hdmirx_ctrl);

	disp_pr_info("[hdmirx]hdmi dss not ready\n");
	hdmirx_ctrl->pwrstatus = false;
	hdmirx_ctrl->start_display_hdmi_ready = false;
	hdmirx_ctrl->start_display_dss_ready = false;
	hdmirx_ctrl->start_display = false;
	hdmirx_ctrl->is_power_off = true;
}

void hdmirx_hotplug_out(void)
{
	disp_pr_info("[hdmirx]hdmi out\n");

	if (g_dpu_hdmirx_dev == NULL) {
		disp_pr_err("[hdmirx]g_dpu_hdmirx_dev NULL\n");
		return;
	}

	hdmirx_power5v_off(&(g_dpu_hdmirx_dev->hdmirx_ctrl));
}

void hdmirx_cmd_hpd_set(void)
{
	struct hdmirx_ctrl_st *hdmirx;
	disp_pr_info("[hdmirx]hpd set\n");

	hdmirx = hdmirx_get_dev_ctrl();
	if (hdmirx == NULL) {
		disp_pr_err("[hdmirx]g_dpu_hdmirx_dev NULL\n");
		return;
	}

	hdmirx_link_hpd_set(hdmirx, true);
}

void hdmirx_cmd_phy_init(void)
{
	struct hdmirx_ctrl_st *hdmirx;
	disp_pr_info("[hdmirx]phy init\n");

	hdmirx = hdmirx_get_dev_ctrl();
	if (hdmirx == NULL) {
		disp_pr_err("[hdmirx]g_dpu_hdmirx_dev NULL\n");
		return;
	}

	hdmirx_create_main_task(hdmirx);
}

void hdmirx_cmd_frl_init(void)
{
	struct hdmirx_ctrl_st *hdmirx;
	disp_pr_info("[hdmirx]frl init\n");

	hdmirx = hdmirx_get_dev_ctrl();
	if (hdmirx == NULL) {
		disp_pr_err("[hdmirx]g_dpu_hdmirx_dev NULL\n");
		return;
	}

	hdmirx_link_frl_init(hdmirx);
}

void hdmirx_start_hdcp_init(void)
{
	if (g_dpu_hdmirx_dev == NULL) {
		disp_pr_err("[hdmirx]g_dpu_hdmirx_dev NULL\n");
		return;
	}
	hdmirx_start_hdcp(&(g_dpu_hdmirx_dev->hdmirx_ctrl));
}

void hdmirx_cmd_hpd_flt_phy(void)
{
	struct hdmirx_ctrl_st *hdmirx;
	disp_pr_info("[hdmirx]hpd_flt_phy\n");

	hdmirx = hdmirx_get_dev_ctrl();
	if (hdmirx == NULL) {
		disp_pr_err("[hdmirx]g_dpu_hdmirx_dev NULL\n");
		return;
	}

	hdmirx_link_hpd_set(hdmirx, true);
	mdelay(10);
	hdmirx_link_frl_ready(hdmirx);
	hdmirx_create_main_task(hdmirx);
}

void hdmirx_cmd_flt_ready(void)
{
	struct hdmirx_ctrl_st *hdmirx;
	disp_pr_info("[hdmirx]flt_ready\n");

	hdmirx = hdmirx_get_dev_ctrl();
	if (hdmirx == NULL) {
		disp_pr_err("[hdmirx]g_dpu_hdmirx_dev NULL\n");
		return;
	}

	hdmirx_link_frl_ready(hdmirx);
}

#ifdef CONFIG_DKMD_DEBUG_ENABLE
EXPORT_SYMBOL_GPL(hdmirx_yuv422_set);
EXPORT_SYMBOL_GPL(hdmirx_hotplug_in);
EXPORT_SYMBOL_GPL(hdmirx_hotplug_out);
EXPORT_SYMBOL_GPL(hdmirx_cmd_hpd_set);
EXPORT_SYMBOL_GPL(hdmirx_cmd_phy_init);
EXPORT_SYMBOL_GPL(hdmirx_cmd_frl_init);
EXPORT_SYMBOL_GPL(hdmirx_start_hdcp_init);
EXPORT_SYMBOL_GPL(hdmirx_cmd_hpd_flt_phy);
EXPORT_SYMBOL_GPL(hdmirx_cmd_flt_ready);
#endif

static bool is_valid_irq = false;
static irqreturn_t hdmirx_hpd_gpio_irq(int irq, void *ptr)
{
	disp_pr_info("[hdmirx] hpd gpio irq\n");
	is_valid_irq = !is_valid_irq;
	return IRQ_WAKE_THREAD;
}

static uint32_t hdmirx_get_is_plug(struct hdmirx_chr_dev_st *hdmirx_chr_dev)
{
	uint32_t is_plug = 0;
	struct hdmirx_ctrl_st *hdmirx_ctrl = NULL;
	hdmirx_ctrl = &(hdmirx_chr_dev->hdmirx_ctrl);

	if (hdmirx_ctrl->fpga_flag == 1) {
		is_plug = (inp32(hdmirx_chr_dev->hdmirx_ctrl.hi_gpio14_base + 0xdc) >> 2) & 0x1;
	} else {
		is_plug = (inp32(hdmirx_chr_dev->hdmirx_ctrl.hi_gpio35_base + 0xdc) >> 7) & 0x1;
	}
	return is_plug;
}

static irqreturn_t hdmirx_hpd_gpio_irq_thread(int irq, void *ptr)
{
	struct hdmirx_chr_dev_st *hdmirx_chr_dev = ptr;
	struct hdmirx_ctrl_st *hdmirx_ctrl = NULL;
	int gpio_high= 0;

	if (hdmirx_chr_dev == NULL) {
		disp_pr_err("[hdmirx]parameter invalid, ptr null\n");
		return 0;
	}
	hdmirx_ctrl = &(hdmirx_chr_dev->hdmirx_ctrl);

	gpio_high = gpio_get_value_cansleep(hdmirx_ctrl->hpd_gpio);

	disp_pr_info("[hdmirx] gpio_high %d\n", gpio_high);
	mdelay(5);
	if (!is_valid_irq) {
		disp_pr_info("[hdmirx] ignore invalid irq\n");
		return 0;
	}
	is_valid_irq = false;

	if (gpio_high == 0) {
		mdelay(100); // ignore short irq
		gpio_high = gpio_get_value_cansleep(hdmirx_ctrl->hpd_gpio);
		disp_pr_info("[hdmirx]mdelay gpio_high %d\n", gpio_high);
		if (gpio_high != 0) {
			disp_pr_info("[hdmirx] ignore the irq  gpio_high=%d\n", gpio_high);
			hdmirx_ctrl->pwrstatus = true;
			return 0;
		}

		// FPGA not call hdmirx_power5v_off;
		disp_pr_info("[hdmirx]hdmi hotplug out\n");
		hdmirx_ctrl->pwrstatus = false;
	} else {
		// FPGA not call hdmirx_power5v_on;
		disp_pr_info("[hdmirx]hdmi hotplug in\n");
		hdmirx_ctrl->pwrstatus = true;
	}

	return 0;
}

static int hdmirx_init_hpd_gpio(struct hdmirx_chr_dev_st *hdmirx_chr_dev)
{
	int ret;
	int irq_num;
	int temp;
	struct hdmirx_ctrl_st *hdmirx = &(hdmirx_chr_dev->hdmirx_ctrl);

	if (hdmirx->fpga_flag == 1) {
		// for FPGA
		hdmirx_set_reg(hdmirx_chr_dev->hdmirx_ctrl.hi_gpio14_base + 0x10, 0x001F0004, 32, 0);
		temp = inp32(hdmirx_chr_dev->hdmirx_ctrl.hi_gpio14_base + 0x10);
		disp_pr_info("gpio config is 0x%x\n", temp); // 0000_0004
		hdmirx_set_reg(hdmirx_chr_dev->hdmirx_ctrl.hi_gpio14_base + 0x88, 0, 1, 2);
		disp_pr_info("hi_gpio14_base 0x88 config\n");

		hdmirx_chr_dev->hdmirx_ctrl.hpd_gpio = GPI0_GROUP_14 * GPIO_NUM + GPI0_PIN_2; // 14 * 8 + 2 = 114; 14 * 32 + 2 = 450
		disp_pr_info("hpd_gpio 114\n");
	} else {
		// for asic
		hdmirx_set_reg(hdmirx_chr_dev->hdmirx_ctrl.hi_gpio35_base + 0x24, 0x001F0004, 32, 0);
		temp = inp32(hdmirx_chr_dev->hdmirx_ctrl.hi_gpio35_base + 0x24);

		disp_pr_info("gpio config is 0x%x\n", temp); // 0000_0004

		disp_pr_info("hi_gpio35_base 0x88 config\n");

		hdmirx_chr_dev->hdmirx_ctrl.hpd_gpio = GPI0_GROUP_35 * GPIO_NUM + GPI0_PIN_7; // 35 * 8 + 7 = 287;
		disp_pr_info("hpd_gpio 287\n");
	}

	ret = devm_gpio_request(&(hdmirx_chr_dev->pdev->dev), hdmirx_chr_dev->hdmirx_ctrl.hpd_gpio, "gpio_hpd");
	if (ret) {
		disp_pr_err("[hdmirx] Fail[%d] request gpio:%d\n", ret, hdmirx_chr_dev->hdmirx_ctrl.hpd_gpio);
		return ret;
	}

	ret = gpio_direction_input(hdmirx_chr_dev->hdmirx_ctrl.hpd_gpio);
	if (ret < 0) {
		disp_pr_err("[hdmirx] Failed to set gpio_down direction\n");
		return ret;
	}

	irq_num = gpio_to_irq(hdmirx_chr_dev->hdmirx_ctrl.hpd_gpio);
	if (irq_num < 0) {
		disp_pr_err("[hdmirx] Failed to get dp_hpd_gpio irq\n");
		ret = -EINVAL;
		return ret;
	}
	ret = devm_request_threaded_irq(&(hdmirx_chr_dev->pdev->dev),
		irq_num, hdmirx_hpd_gpio_irq, hdmirx_hpd_gpio_irq_thread,
		IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
		"hpd_gpio", (void *)hdmirx_chr_dev);
	if (ret) {
		disp_pr_err("[hdmirx] Failed to request press interupt handler\n");
		return ret;
	}
	hdmirx_chr_dev->hdmirx_ctrl.hpd_irq = irq_num;
	disp_pr_info("[hdmirx] hpd_irq %d\n", irq_num);
	hdmirx_set_reg(hdmirx_chr_dev->hdmirx_ctrl.hi_gpio35_base + 0x88, 0, 1, 7);

	return 0;
}
static int hdmirx_config_clk_init(struct device_node *np, struct hdmirx_ctrl_st *hdmirx_ctrl)
{
	int32_t i;
	disp_pr_info("clk +++\n");
	for (i = 0; i < CLK_HDMI_MAX_IDX; i++) {
		hdmirx_ctrl->clk_hdmirx[i] = of_clk_get(np, i);
		if (IS_ERR_OR_NULL(hdmirx_ctrl->clk_hdmirx[i])) {
			dpu_pr_err("hdmi of clk get %d failed, maybe have no clk!", i);
			return ERROR;
		}
	}
	return 0;
}

static int hdmirx_config_base_addr_init(struct device_node *np, struct hdmirx_ctrl_st *hdmirx_ctrl)
{
	char __iomem *hdmirx_base_addrs[HDMIRX_BASE_ADDR_MAX] = {NULL};
	int i = 0;

	disp_pr_info("+++\n");

	for (i = 0; i < HDMIRX_BASE_ADDR_MAX; i++) {
		hdmirx_base_addrs[i] = of_iomap(np, i);
		if (!hdmirx_base_addrs[i]) {
			pr_err("get hdmi rx base fail\n");
			return -EINVAL;
		}
	}
	hdmirx_ctrl->hdmirx_aon_base = hdmirx_base_addrs[HDMIRX_AON_BASE_ID];
	hdmirx_ctrl->hdmirx_pwd_base = hdmirx_base_addrs[HDMIRX_PWD_BASE_ID];
	hdmirx_ctrl->hdmirx_sysctrl_base = hdmirx_base_addrs[HDMIRX_SYSCTRL_BASE_ID];
	hdmirx_ctrl->hdmirx_hsdt1_crg_base = hdmirx_base_addrs[HDMIRX_HSDT1_CRG_BASE_ID];
	hdmirx_ctrl->hsdt1_sysctrl_base = hdmirx_base_addrs[HDMIRX_HSDT1_SYSCTRL_BASE_ID];
	hdmirx_ctrl->hdmirx_ioc_base = hdmirx_base_addrs[HDMIRX_IOC_BASE_ID];
	hdmirx_ctrl->hi_gpio14_base = hdmirx_base_addrs[HDMIRX_HI_GPIO14_BASE_ID];
	hdmirx_ctrl->hi_gpio35_base = hdmirx_base_addrs[HDMIRX_HI_GPIO35_BASE_ID];
	hdmirx_ctrl->hdmirx_iocg_base = hdmirx_base_addrs[HDMIRX_IOCG_BASE_ID];
	hdmirx_ctrl->hdmirx_pericrg_base = hdmirx_base_addrs[HDMIRX_PERICRG_BASE_ID];
	hdmirx_ctrl->hdmirx_gpio262_base_1 = hdmirx_base_addrs[HDMIRX_GPIO262_BASE_ID];
	hdmirx_ctrl->hdmirx_gpio262_base_2 = hdmirx_base_addrs[HDMIRX_GPIO262_BASE_2_ID];
	hdmirx_ctrl->hdmirx_gpio010_base = hdmirx_base_addrs[HDMIRX_GPIO010_BASE_ID];
	disp_pr_info("---\n");

	return 0;
}

static int hdmirx_fpga_flag_set(struct device_node *np, struct hdmirx_ctrl_st *hdmirx_ctrl)
{
	uint32_t ret;
	ret = of_property_read_u32(np, "fpga_flag", &hdmirx_ctrl->fpga_flag);
	if (ret) {
		dpu_pr_info("get fpga_flag failed!\n");
		hdmirx_ctrl->fpga_flag = 0;
	}
	dpu_pr_info("get fpga_flag %d!\n", hdmirx_ctrl->fpga_flag);

	return 0;
}

static int hdmirx_drv_init_hdmi_config(struct hdmirx_chr_dev_st *hdmirx_dev)
{
	struct device_node *np = hdmirx_dev->pdev->dev.of_node;
	struct hdmirx_ctrl_st *hdmirx_ctrl = &(hdmirx_dev->hdmirx_ctrl);
	disp_pr_info("+++\n");

	hdmirx_ctrl->pwrstatus = false;
	hdmirx_ctrl->is_power_off = true;

	hdmirx_fpga_flag_set(np, hdmirx_ctrl);

	hdmirx_ctrl_irq_config_init(np, hdmirx_ctrl);

	hdmirx_config_base_addr_init(np, hdmirx_ctrl);

	hdmirx_config_clk_init(np, hdmirx_ctrl);

	disp_pr_info("---\n");

	return 0;
}

static int hdmirx_device_init(struct hdmirx_chr_dev_st *hdmirx_dev)
{
	struct hdmirx_ctrl_st *hdmirx = &(hdmirx_dev->hdmirx_ctrl);

	disp_pr_info("+++\n");

	hdmirx_drv_init_hdmi_config(hdmirx_dev);

	hdmirx_hdcp_tee_init();

	hdmirx_ctrl_irq_setup(hdmirx);

	hdmirx_ctrl_interface_init(&(hdmirx->interface_impl));

	disp_pr_info("---\n");

	return 0;
}

void hdmirx_drv_init_component(struct hdmirx_chr_dev_st *hdmirx_dev)
{
}

static int hdmirx_drv_open(struct inode *inode, struct file *filp)
{
	disp_check_and_return(unlikely(!inode), -EINVAL, err, "inode is null");
	disp_check_and_return(unlikely(!filp), -EINVAL, err, "filp is null");
	disp_pr_info("+++\n");
	if (!filp->private_data) {
		filp->private_data = hdmirx_get_dev();
		if (!filp->private_data)
			disp_pr_err("private_data null\n");
	}

	disp_pr_info("---\n");
	return 0;
}

static ssize_t hdmirx_debug_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return strlen(buf);
}

static ssize_t hdmirx_debug_store(struct device *device,
			struct device_attribute *attr, const char *buf, size_t count)
{
	return count;
}

static struct device_attribute hdmirx_attrs[] = {
	__ATTR(hdmirx_debug, S_IRUGO | S_IWUSR, hdmirx_debug_show, hdmirx_debug_store),

	/* TODO: other attrs */
};

static int hdmirx_drv_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static long hdmirx_drv_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct hdmirx_chr_dev_st *hdmirx_dev = NULL;
	disp_check_and_return(unlikely(!filp), -EINVAL, err, "filp is null");
	disp_check_and_return(unlikely(!filp->private_data), -EINVAL, err, "private_data is null");
	disp_check_and_return(unlikely(!argp), -EINVAL, err, "argp is null");

	hdmirx_dev = (struct hdmirx_chr_dev_st *)filp->private_data;
	switch (cmd) {
	case DISP_INPUTSRC_GET_TIMING_INFO:
		return hdmirx_ctrl_get_timinginfo(&(hdmirx_dev->hdmirx_ctrl.interface_impl), argp);
	default:
		disp_pr_info("unsupported cmd=%#x", cmd);
		return -EINVAL;
	}
	return 0;
}

static struct file_operations hdmirx_fops = {
	.owner = THIS_MODULE,
	.open = hdmirx_drv_open, // hisi_offline_open,
	.release = hdmirx_drv_release, // hisi_offline_release,
	.unlocked_ioctl = hdmirx_drv_ioctl, // hisi_offline_adaptor_ioctl,
	.compat_ioctl =  hdmirx_drv_ioctl, // hisi_offline_adaptor_ioctl,
};

// hisi_drv_offline_create_chrdev
static void hdmirx_drv_create_chrdev(struct hdmirx_chr_dev_st *hdmirx_dev)
{
	/* init chrdev info */
	hdmirx_dev->chrdev.name = HISI_HDMIRX_CHR_NAME;
	hdmirx_dev->chrdev.fops = &hdmirx_fops;
	hdmirx_dev->chrdev.drv_data = hdmirx_dev;

	dkmd_create_chrdev(&hdmirx_dev->chrdev);
	dkmd_create_attrs(hdmirx_dev->chrdev.chr_dev, hdmirx_attrs, ARRAY_SIZE(hdmirx_attrs));
}

static int hdmirx_drv_probe(struct platform_device *pdev)
{
	struct hdmirx_chr_dev_st *hdmirx_dev = NULL;

	disp_pr_info("+++\n");

	if (pdev == NULL) {
		disp_pr_err("[hdmirx] pdev null\n");
		return -1;
	}

	hdmirx_dev = devm_kzalloc(&pdev->dev, sizeof(*hdmirx_dev), GFP_KERNEL);
	if (!hdmirx_dev) {
		disp_pr_err("[hdmirx] devm_kzalloc fail\n");
		return -1;
	}
	hdmirx_dev->pdev = pdev;
	hdmirx_dev->index = 0;

	hdmirx_device_init(hdmirx_dev);

	/* create chrdev */
	hdmirx_drv_create_chrdev(hdmirx_dev);
	g_dpu_hdmirx_dev = hdmirx_dev;

	hdmirx_init_hpd_gpio(hdmirx_dev);

	disp_pr_info("---\n");
	return 0;
}

static int hdmirx_drv_remove(struct platform_device *pdev)
{
	disp_pr_info("+++\n");

	disp_pr_info("---\n");
	return 0;
}

static const struct of_device_id device_match_table[] = {
	{
		.compatible = DTS_NAME_HDMIRX0,
		.data = NULL,
	},
	{},
};
MODULE_DEVICE_TABLE(of, device_match_table);

static struct platform_driver g_dpu_hdmirx_driver = {
	.probe = hdmirx_drv_probe,
	.remove = hdmirx_drv_remove,
	.suspend = NULL,
	.resume = NULL,
	.shutdown = NULL,
	.driver = {
		.name = HISI_CONNECTOR_HDMIRX_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(device_match_table),
	},
};

static int __init hdmirx_module_init(void)
{
	int ret;

	ret = platform_driver_register(&g_dpu_hdmirx_driver);
	if (ret) {
		disp_pr_err("platform_driver_register failed, error=%d\n", ret);
		return ret;
	}

	return ret;
}

module_init(hdmirx_module_init);

MODULE_DESCRIPTION("Hisilicon HDMI RX Driver");
MODULE_LICENSE("GPL v2");
