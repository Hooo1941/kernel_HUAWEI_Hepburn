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
#ifndef __DPRX_H__
#define __DPRX_H__
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/regulator/driver.h>
#include <linux/compat.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/mount.h>
#include <linux/genalloc.h>

#include <linux/platform_drivers/usb/tca.h>

#include "hidprx_edid.h"
#include "hidprx_sdp.h"
#include "dkmd_rx.h"
#include "dkmd_chrdev.h"

#define DEV_NAME_DPRX "dpu_dp_receiver"
#define DTS_NAME_DPRX0 "dkmd,dpu_receiver0"
#define DTS_NAME_DPRX1 "dkmd,dpu_receiver1"

#define RX_CAPABILITY_SIZE (0x100)
#define RX_LINK_CFG_SIZE (0x100)
#define RX_LINK_DEVICE_STATUS_SIZE (0x100)
#define RX_SRC_DEVICE_SPECIFIC_SIZE (0x100)
#define RX_SINK_DEVICE_SPECIFIC_SIZE (0x100)
#define RX_EDP_SPECIFIC_SIZE (0x100)
#define RX_EVENT_STATUS_INDICATOR (0x200)
#define RX_EXTENDED_CAPABILITY_SIZE (0x100)

#define HISI_HIDPRX0_CHR_NAME    "hidp_rx0"
#define HISI_HIDPRX1_CHR_NAME    "hidp_rx1"
#define DPRX_LOG_TAG       "[DPRX %s]"

enum {
	DPRX_CRTL0 = 0,
	DPRX_CTRL1 = 1,
	DPRX_CTRL_NUMS_MAX = 2,
};

enum {
	DP_MODE = 0,
	EDP_MODE = 1,
	MODE_MAX = 2,
};

enum {
	DPRX_HSDT1_CRG_BASE = 0,
	DPRX_HSDT1_SYS_CTRL_BASE = 1,
	DPRX_CTRL_BASE = 2,
	DPRX_EDID_BASE = 3,
	IOC_PERI_BASE = 4,
	DPRX_BASE_ADDR_MAX,
};

/**
 * DPCD r/w operation is monitored in byte size.
 * So we should record current dpcd state to identify which bit changes.
 */
struct dpcd_status {
	uint8_t rx_caps[RX_CAPABILITY_SIZE]; /* 0x0-0xFF */
	uint8_t link_cfgs[RX_LINK_CFG_SIZE]; /* 0x100-0x1FF */
	uint8_t link_dev_status[RX_LINK_DEVICE_STATUS_SIZE]; /* 0x200-0x2FF */
	uint8_t src_dev_spec[RX_SRC_DEVICE_SPECIFIC_SIZE]; /* 0x300-0x3FF */
	uint8_t sink_dev_spec[RX_SINK_DEVICE_SPECIFIC_SIZE]; /* 0x400-0x4FF */
	uint8_t power_state; /* 0x600 */
	uint8_t edp_spec[RX_EDP_SPECIFIC_SIZE]; /* 0x700-0x7FF */
	uint8_t event_status_indicator[RX_EVENT_STATUS_INDICATOR]; /* 0x2000-0x21FF */
	uint8_t extended_rx_caps[RX_EXTENDED_CAPABILITY_SIZE]; /* 0x2200-0x22FF */
};

struct msa_params {
	uint32_t mvid;
	uint32_t nvid;
	uint16_t h_total;
	uint16_t v_total;
	uint16_t h_start;
	uint16_t v_start;
	uint16_t hsp;
	uint16_t hsw;
	uint16_t vsp;
	uint16_t vsw;
	uint16_t h_width;
	uint16_t v_height;
	uint8_t misc0;
	uint8_t misc1;
	bool msa_valid;
};

/* for test, TODO: be coincident with dptx */
enum pixel_enc_type {
	RGB = 0,
	YCBCR420 = 1,
	YCBCR422 = 2,
	YCBCR444 = 3,
	YONLY = 4,
	RAW = 5,
	PIXEL_ENC_INVALID
};
/* for test, TODO: be coincident with dptx */
enum color_depth {
	COLOR_DEPTH_INVALID = 0,
	COLOR_DEPTH_6 = 6,
	COLOR_DEPTH_7 = 7,
	COLOR_DEPTH_8 = 8,
	COLOR_DEPTH_10 = 10,
	COLOR_DEPTH_12 = 12,
	COLOR_DEPTH_14 = 14,
	COLOR_DEPTH_16 = 16
};

struct dprx_to_layer_format {
	enum pixel_enc_type color_space;
	enum color_depth bit_width;
	uint32_t layer_format;
};

struct video_params {
	enum pixel_enc_type pix_enc;
	enum color_depth bpc;
	bool video_format_valid;
};

struct dprx_debug_params {
	uint8_t max_lane_num;
	uint8_t max_link_rate;
	uint8_t adaptive_sync_en;
	uint8_t fec_en;
	uint8_t enhance_frame_en;
	uint8_t dsc_support;
	uint8_t phy_rate;
};

enum {
	CLK_DPRX_GTC,
	CLK_DPRX_AUX,
	CLK_DPRX_PCLK,
	CLK_DPRXPHY_PCLK,
	CLK_DPRXPHY_REF,
	CLK_DPRXPHY_AUX,
	CLK_DPRX_EDID,
	CLK_DPRX_ASYNC_BRG,
	CLK_DPRX_MAX,
};

typedef struct dpu_dispintf_info {
	uint16_t timing_code;
	uint16_t format;
	uint16_t xres;
	uint16_t yres;
	uint16_t vporch;
	uint16_t hporch;
	uint16_t framerate;
	uint8_t psr;
	uint8_t vrr;
} dss_intfinfo_t;

struct dprx_ctrl {
	struct mutex dprx_mutex; /* generic mutex for dprx */

	/* ctrl version: major_minor */
	uint8_t major_version;
	uint8_t minor_version;

	/*
	* work mode: 0 for dp and 1 for edp
	* ctrl id: 0-1-...
	*/
	uint32_t mode;
	uint32_t id;

	bool fpga_flag;
	struct dpu_dprx_device *device;

	void __iomem *base; /* ctrl base address */
	void __iomem *hsdt1_crg_base; /* hsdt1 crg base address */
	void __iomem *hsdt1_sys_ctrl_base; /* hsdt1_sys_ctrl_base address */
	void __iomem *edid_base; /* edid base address */
	void __iomem *ioc_peri_base; /* PERI IOMG to ctrl gpio map hpd */

	uint32_t irq;       /* irq num */
	uint32_t cable_det_irq;
	struct dpcd_status dpcds;
	uint8_t power_on;
	uint8_t video_on; /* start transfer data to dpu */
	uint8_t cable_det_status;
	uint8_t dsc_en;
	bool video_input_params_ready; /* get valid timing and format from input video */

	struct rx_edid edid; /* edid */
	struct rx_sdp_raw_data sdp_raw_data;

	struct video_params v_params;
	struct msa_params msa;
	struct pps_sdp pps_table; /* pps table */

	struct workqueue_struct *dprx_sdp_wq;
	struct work_struct dprx_sdp_work;
	struct dprx_debug_params debug_params;
	uint8_t intface_type;
	uint8_t typec_orien;
	uint8_t vactive_flag;
	uint32_t cable_det_gpio;
	uint32_t hpd_gpio;
	/*
	 * irq
	 */
	irqreturn_t (*dprx_irq_handler)(int irq, void *dev);
	irqreturn_t (*dprx_threaded_irq_handler)(int irq, void *dev);

	void (*local_event_notify)(struct dprx_ctrl *dprx, uint32_t event_id);

	raw_spinlock_t context_comm_lock;
	dss_intfinfo_t intf_info;
	struct clk *rx_clk[CLK_DPRX_MAX];
};

struct dpu_dprx_device {
	struct platform_device *pdev;
	struct dkmd_chrdev chrdev;
	struct dprx_ctrl dprx;
	struct rx_interface interface_impl;
};

struct dprx_description_handling {
	uint32_t bit_pos;
	char description[64];
	void (*oprate) (struct dprx_ctrl *dprx);
};
/*--------------------------------------------------------------------------*/
enum {
	DPRX_LOG_LVL_ERR = 0,
	DPRX_LOG_LVL_WARNING,
	DPRX_LOG_LVL_INFO,
	DPRX_LOG_LVL_DEBUG,
	DPRX_LOG_LVL_MAX,
};

enum hisi_dp_pixel_format {
	HISI_FB_PIXEL_FORMAT_RGBA_8888 = 6,
	HISI_FB_PIXEL_FORMAT_YUYV_422_PKG = 23,
	HISI_FB_PIXEL_FORMAT_RGBA_1010102 = 27,
	HISI_FB_PIXEL_FORMAT_YUV422_10BIT = 30,
	HISI_FB_PIXEL_FORMAT_YUVA444 = 39,
	HISI_FB_PIXEL_FORMAT_YUV444 = 40,
	HISI_FB_PIXEL_FORMAT_YUYV_420_PKG = 41,
	HISI_FB_PIXEL_FORMAT_YUYV_420_PKG_10BIT = 42,
};

extern uint32_t g_dprx_log_level;

enum {
	STANDARD_DP_INTERFACE = 0,
	TYPEC_INTERFACE,
	INVALID_RX_INTERFACE,
};

enum {
	TIMING_480P,
	TIMING_1080P,
};

#define dprx_pr_err(msg, ...)  \
	do { \
		pr_err(DPRX_LOG_TAG"[%s:%d]"msg, "E", __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define dprx_pr_warn(msg, ...)  \
	do { \
		if (g_dprx_log_level > DPRX_LOG_LVL_WARNING) \
			pr_warning(DPRX_LOG_TAG"[%s:%d]"msg, "W", __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define dprx_pr_info(msg, ...)  \
	do { \
		if (g_dprx_log_level > DPRX_LOG_LVL_INFO) \
			pr_err(DPRX_LOG_TAG"[%s:%d]"msg, "I", __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define dprx_pr_debug(msg, ...)  \
	do { \
		if (g_dprx_log_level > DPRX_LOG_LVL_DEBUG) \
			pr_err(DPRX_LOG_TAG"[%s:%d]"msg, "D", __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

/*--------------------------------------------------------------------------*/
/* temp use */
#define CONFIG_DPU_FB_DUMP_DSS_REG
#ifdef CONFIG_DPU_FB_DUMP_DSS_REG
#define outp32(addr, val) \
	do {\
		writel(val, addr);\
		pr_info("writel(0x%x, 0x%x);\n", addr, val);\
	} while (0)
#else
#define outp32(addr, val) writel(val, addr)
#endif

#define inp32(addr)         readl(addr)

#define dpu_check_and_return(condition, ret, level, msg, ...) \
	do { \
		if (condition) { \
			pr_##level(msg, ##__VA_ARGS__);\
			return ret; \
		} \
	} while (0)

#define dpu_check_and_no_retval(condition, level, msg, ...) \
	do { \
		if (condition) { \
			pr_##level(msg, ##__VA_ARGS__);\
			return; \
		} \
	} while (0)

/*--------------------------------------------------------------------------*/
static inline uint32_t dprx_readl(struct dprx_ctrl *dp, uint32_t offset)
{
	/*lint -e529*/
	uint32_t data = readl(dp->base + offset);
	/*lint +e529*/
	return data;
}

static inline void dprx_writel(struct dprx_ctrl *dp, uint32_t offset, uint32_t data)
{
	writel(data, dp->base + offset);
	dprx_pr_debug("writel(0x%x, 0x%x);\n", dp->base + offset, data);
}

uint32_t dprx_ctrl_get_layer_fmt(struct dprx_ctrl *dprx);

uint32_t dprx_get_layer_fmt(uint32_t src_fmt);

#endif
