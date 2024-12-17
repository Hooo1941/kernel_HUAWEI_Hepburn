/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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

#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include <securec.h>
#include "dkmd_log.h"
#include "dkmd_object.h"
#include "dkmd_connector.h"
#include "dpu_connector.h"

#include "hdmitx_ctrl.h"
#include "hdmitx_phy.h"
#include "hdmitx_ddc.h"
#include "hdmitx_connector.h"
#include "hdmitx_edid.h"
#include "hdmitx_frl.h"
#include "hdmitx_avgen.h"
#include "hdmitx_vrr.h"
#include "hdmitx_core_config.h"
#include "hdmitx_infoframe.h"
#include "hdmitx_ctrl_dev.h"
#include "hdmitx_drv.h"
#include "hdmitx_aon.h"

#define TMDS_FRL_SEL 0x98
#define DSS2DPTX_SEL 0x300
#define HDMITX_PHY_ISO_EN 0x34c
#define DSS2DPTX_STREAM0_IN_SEL_MASK GENMASK(3, 0)
#define DSS2DPTX_STREAM1_IN_SEL_MASK GENMASK(7, 4)
#define DSS2DPTX_STREAM2_IN_SEL_MASK GENMASK(11, 8)
#define DSS2DPTX_STREAM3_IN_SEL_MASK GENMASK(15, 12)
#define HDMITX_HPD_HIGH_RESHOLD    100
#define HDMITX_HPD_LOW_RESHOLD     50
#define HDMITX_HPD_MAX_LOW_RESHOLD 100

#define HDMITX_DDC_RATE_MASK GENMASK(12, 4)

static struct pinctrl_data g_scl_pctrl;
static struct pinctrl_data g_sda_pctrl;

static struct pinctrl_cmd_desc scl_pinctrl_init_cmds[] = {
	{DTYPE_PINCTRL_GET, &g_scl_pctrl, 0},
	{DTYPE_PINCTRL_STATE_GET, &g_scl_pctrl, DTYPE_PINCTRL_STATE_DEFAULT},
	{DTYPE_PINCTRL_SET, &g_scl_pctrl, DTYPE_PINCTRL_STATE_DEFAULT},
};

static struct pinctrl_cmd_desc scl_pinctrl_finit_cmds[] = {
	{DTYPE_PINCTRL_PUT, &g_scl_pctrl, 0},
};

static struct pinctrl_cmd_desc sda_pinctrl_init_cmds[] = {
	{DTYPE_PINCTRL_GET, &g_sda_pctrl, 0},
	{DTYPE_PINCTRL_STATE_GET, &g_sda_pctrl, DTYPE_PINCTRL_STATE_DEFAULT},
	{DTYPE_PINCTRL_SET, &g_sda_pctrl, DTYPE_PINCTRL_STATE_DEFAULT},
};

static struct pinctrl_cmd_desc sda_pinctrl_finit_cmds[] = {
	{DTYPE_PINCTRL_PUT, &g_sda_pctrl, 0},
};

static void dump_edid(u8 *edid, int cnt)
{
	int i;

	if (edid == NULL) {
		dpu_pr_err("edid is null");
		return;
	}

	dpu_pr_info("<<<-------------------------block[%d]--------------------------->>>:", cnt);

	for (i = 0; i < EDID_LENGTH;) {
		if (!(i % 16))
			dpu_pr_info("EDID[%04x]: 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,"
				"\n\t 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,",
				i, edid[i], edid[i + 1], edid[i + 2], edid[i + 3], edid[i + 4], edid[i + 5],
				edid[i + 6], edid[i + 7], edid[i + 8], edid[i + 9], edid[i + 10], edid[i + 11],
				edid[i + 12], edid[i + 13], edid[i + 14], edid[i + 15]);

		i += 16;

		if (i == 128)
			dpu_pr_info("<<<-------------------------------------------------------------->>>");
	}
}

static void set_band_mode(u32 vic, u32 color_fmt, u32 color_dpth, struct band_mode *band)
{
	band->vic = vic;
	band->color_format = color_fmt;
	band->color_depth = color_dpth;
}

static void copy_valid_mode(struct hdmitx_valid_mode *dst,
	const struct hdmitx_valid_mode *src)
{
	struct list_head head = dst->head;

	*dst = *src;
	dst->head = head;
}

static s8 *get_color_format_name(u32 color_format)
{
	switch (color_format) {
	case RGB444:
		return "RGB444";
	case YCBCR422:
		return "YCBCR422";
	case YCBCR444:
		return "YCBCR444";
	case YCBCR420:
		return "YCBCR420";
	default:
		break;
	}

	return "UNKNOWN COLOR FORMAT";
}

static s8 *get_color_depth_name(u32 color_depth)
{
	switch (color_depth) {
	case CD_24:
		return "CD_24";
	case CD_30:
		return "CD_30";
	case CD_36:
		return "CD_36";
	case CD_48:
		return "CD_48";
	default:
		break;
	}

	return "UNKNOWN COLOR DEPTH";
}

static void print_valid_modes(const struct hdmitx_valid_mode *mode)
{
	struct hdmitx_timing_mode *timing_mode = NULL;
	s8 *color_format = NULL;
	s8 *color_depth = NULL;

	timing_mode = hdmitx_modes_get_timing_mode_by_vic(mode->band.vic);

	color_format = get_color_format_name(mode->band.color_format);
	color_depth = get_color_depth_name(mode->band.color_depth);

	dpu_pr_debug("[%d] %s %s %s", mode->band.vic,
		(timing_mode != NULL) ? timing_mode->name : "DETAIL TIMING",
		color_format, color_depth);
}

static s32 connector_create_valid_mode(struct hdmitx_connector *connector,
	struct hdmitx_valid_mode *valid_mode)
{
	struct hdmitx_valid_mode *nmode = NULL;

	nmode = kmalloc(sizeof(struct hdmitx_valid_mode), GFP_KERNEL);
	if (nmode == NULL) {
		dpu_pr_err("kmalloc fail");
		return -1;
	}

	if (memset_s(nmode, sizeof(*nmode), 0, sizeof(*nmode)) != EOK) {
		dpu_pr_err("memset_s fail");
		kfree(nmode);
		nmode = NULL;
		return -1;
	}

	valid_mode->valid = true;
	copy_valid_mode(nmode, valid_mode);
	print_valid_modes(valid_mode);

	list_add_tail(&nmode->head, &connector->valid_modes);

	return 0;
}

static void connector_destroy_valid_mode(struct hdmitx_connector *connector,
	struct hdmitx_valid_mode *valid_mode)
{
	list_del(&valid_mode->head);
	kfree(valid_mode);
	valid_mode = NULL;
}

static void connector_check_valid_mode_by_tmds(const struct hdmitx_connector *connector,
	struct hdmitx_valid_mode *valid_mode)
{
	const struct src_hdmitx *src = &connector->src_cap.hdmi;
	const struct hdmitx_display_info *info = &connector->display_info;

	/* tmds */
	if (valid_mode->tmds_clock < min(info->max_tmds_clock, src->max_tmds_clock)) {
		valid_mode->tmds_encode = true;

		if (valid_mode->tmds_clock > HDMITX14_MAX_TMDS_CLK) {
			if (connector->scdc.present && src->scdc_present) {
				valid_mode->hdmi_mode = HDMITX_MODE_20;
				valid_mode->tmds_scdc_en = true;
			} else {
				/* edid error */
				valid_mode->tmds_encode = false;
			}
		} else {
			valid_mode->hdmi_mode = (info->has_hdmi_infoframe && src->hdmi_support) ?
				HDMITX_MODE_14 : HDMITX_MODE_DVI;
			valid_mode->tmds_scdc_en = !!((valid_mode->hdmi_mode == HDMITX_MODE_14) &&
				src->scdc_lte_340mcsc && connector->scdc.lte_340mcsc);
		}
	} else {
		valid_mode->tmds_encode = false;
		valid_mode->tmds_scdc_en = false;
		valid_mode->hdmi_mode = HDMITX_MODE_21;
	}
}

static void connector_check_valid_mode_by_frl(const struct hdmitx_connector *connector,
	const struct hdmitx_display_mode *dis_mode, struct hdmitx_valid_mode *valid_mode)
{
	struct frl_requirements *frl_req = NULL;
	struct src_hdmitx src = connector->src_cap.hdmi;
	struct hdmitx_display_info info = connector->display_info;
	struct dsc_property dsc = connector->dsc;
	bool dsc_support = false;

	/* frl & dsc */
	if (connector->scdc.present && connector->display_info.max_frl_rate && src.max_frl_rate) {
		frl_req = hdmitx_modes_get_frl_req_by_band(&valid_mode->band);
		if (frl_req != NULL) {
			valid_mode->frl.max_frl_rate = min(info.max_frl_rate, src.max_frl_rate);
			valid_mode->frl.min_frl_rate = frl_req->min_frl_rate;
			valid_mode->frl.frl_uncompress = !!(frl_req->frl_uncompress &&
				frl_req->min_frl_rate <= valid_mode->frl.max_frl_rate);
			valid_mode->frl.ffe_levels = src.ffe_levels;
			//  to fix:
			//  dsc_support ,connector_dsc_check(...)

			valid_mode->dsc.frl_compress = (frl_req->frl_compress && dsc_support);

			if (dsc_support) {
				valid_mode->dsc.max_dsc_frl_rate = min(dsc.dsc_max_rate, src.max_frl_rate);
				valid_mode->dsc.min_dsc_frl_rate = frl_req->min_dsc_frl_rate;
				valid_mode->dsc.hcactive = frl_req->dsc_hcactive;
				valid_mode->dsc.hcblank = frl_req->dsc_hcblank;
			}
		}
	}
}

static bool connector_check_valid_mode(struct hdmitx_connector *connector,
	const struct hdmitx_display_mode *dis_mode, struct hdmitx_valid_mode *valid_mode)
{
	struct src_hdmitx *src = &connector->src_cap.hdmi;
	valid_mode->tmds_encode = false;
	valid_mode->frl.frl_uncompress = false;
	valid_mode->dsc.frl_compress = false;

	connector_check_valid_mode_by_tmds(connector, valid_mode);
	connector_check_valid_mode_by_frl(connector, dis_mode, valid_mode);

	if (dis_mode->detail.pixel_clock > src->max_pixel_clock) {
		dpu_pr_debug("now pixel clock %u, max is %u", dis_mode->detail.pixel_clock, src->max_pixel_clock);
		return false;
	}

	if ((!valid_mode->tmds_encode) &&
		(!valid_mode->frl.frl_uncompress) &&
		(!valid_mode->dsc.frl_compress))
		return false;

	if (connector_create_valid_mode(connector, valid_mode)) {
		dpu_pr_err("create valid mode fail");
		return false;
	}

	return true;
}

static void connector_check_valid_mode_set_color(struct hdmitx_connector *connector,
	struct hdmitx_display_mode *dis_mode, struct hdmitx_valid_mode *valid_mode, u32 color_cap)
{
	if (connector_check_valid_mode(connector, dis_mode, valid_mode)) {
		dis_mode->color_cap |= color_cap;
		dis_mode->status = MODE_OK;
	}
}

static void connector_add_420_valid_mode(struct hdmitx_connector *connector,
	struct hdmitx_display_mode *dis_mode)
{
	bool flag = false;
	bool hdmi_support = false;
	struct hdmitx_valid_mode valid_mode;
	struct color_depth *depth = &connector->color.depth;
	struct src_hdmitx *src = &connector->src_cap.hdmi;
	struct hdmi_detail *detail = &dis_mode->detail;

	if (memset_s(&valid_mode, sizeof(valid_mode), 0x0, sizeof(valid_mode)) != EOK) {
		dpu_pr_err("memset_s failed");
		return;
	}

	hdmi_support = connector->display_info.has_hdmi_infoframe && connector->src_cap.hdmi.hdmi_support;

	dis_mode->color_cap &= ~COLOR_Y420_24;
	flag = hdmi_support && depth->y420_24 && src->ycbcr420;
	if (flag) {
		set_band_mode(dis_mode->vic, YCBCR420, CD_24, &valid_mode.band);
		valid_mode.tmds_clock = detail->pixel_clock / 2; /* 1/2 times */
		connector_check_valid_mode_set_color(connector, dis_mode, &valid_mode, COLOR_Y420_24);
	}

	dis_mode->color_cap &= ~COLOR_Y420_30;
	flag = hdmi_support && depth->y420_30 && src->ycbcr420 && src->bpc_30;
	if (flag) {
		set_band_mode(dis_mode->vic, YCBCR420, CD_30, &valid_mode.band);
		valid_mode.tmds_clock = detail->pixel_clock * 5 / 8; /* 5/8: 0.625 times */
		connector_check_valid_mode_set_color(connector, dis_mode, &valid_mode, COLOR_Y420_30);
	}

	dis_mode->color_cap &= ~COLOR_Y420_36;
	flag = hdmi_support && depth->y420_36 && src->ycbcr420 && src->bpc_36;
	if (flag) {
		set_band_mode(dis_mode->vic, YCBCR420, CD_36, &valid_mode.band);
		valid_mode.tmds_clock = detail->pixel_clock * 3 / 4; /* 3/4: 0.75 times */
		connector_check_valid_mode_set_color(connector, dis_mode, &valid_mode, COLOR_Y420_36);
	}

	dis_mode->color_cap &= ~COLOR_Y420_48;
	flag = hdmi_support && depth->y420_48 && src->ycbcr420 && src->bpc_48;
	if (flag) {
		set_band_mode(dis_mode->vic, YCBCR420, CD_48, &valid_mode.band);
		valid_mode.tmds_clock = detail->pixel_clock;
		connector_check_valid_mode_set_color(connector, dis_mode, &valid_mode, COLOR_Y420_48);
	}
}

static void connector_add_rgb_valid_mode(struct hdmitx_connector *connector,
	struct hdmitx_display_mode *dis_mode)
{
	struct hdmitx_valid_mode valid_mode;
	struct color_depth *depth = &connector->color.depth;
	struct src_hdmitx *src = &connector->src_cap.hdmi;
	struct hdmi_detail *detail = &dis_mode->detail;
	bool hdmi_support = false;
	bool flag = false;

	if (memset_s(&valid_mode, sizeof(valid_mode), 0x0, sizeof(struct hdmitx_valid_mode)) != EOK) {
		dpu_pr_err("memset_s failed");
		return;
	}

	hdmi_support = connector->display_info.has_hdmi_infoframe && connector->src_cap.hdmi.hdmi_support;

	dis_mode->color_cap &= ~COLOR_RGB_24;
	set_band_mode(dis_mode->vic, RGB444, CD_24, &valid_mode.band);
	valid_mode.tmds_clock = detail->pixel_clock;
	connector_check_valid_mode_set_color(connector, dis_mode, &valid_mode, COLOR_RGB_24);

	dis_mode->color_cap &= ~COLOR_RGB_30;
	flag = hdmi_support && depth->rgb_30 && src->bpc_30;
	if (flag) {
		set_band_mode(dis_mode->vic, RGB444, CD_30, &valid_mode.band);
		valid_mode.tmds_clock = detail->pixel_clock * 5 / 4; /* 5/4: 1.25 times */
		connector_check_valid_mode_set_color(connector, dis_mode, &valid_mode, COLOR_RGB_30);
	}

	dis_mode->color_cap &= ~COLOR_RGB_36;
	flag = hdmi_support && depth->rgb_36 && src->bpc_36;
	if (flag) {
		set_band_mode(dis_mode->vic, RGB444, CD_36, &valid_mode.band);
		valid_mode.tmds_clock = detail->pixel_clock * 3 / 2; /* 3/2: 1.5 times */
		connector_check_valid_mode_set_color(connector, dis_mode, &valid_mode, COLOR_RGB_36);
	}

	dis_mode->color_cap &= ~COLOR_RGB_48;
	flag = hdmi_support && depth->rgb_48 && src->bpc_48;
	if (flag) {
		set_band_mode(dis_mode->vic, RGB444, CD_48, &valid_mode.band);
		valid_mode.tmds_clock = detail->pixel_clock << 1; /* 2 times */
		connector_check_valid_mode_set_color(connector, dis_mode, &valid_mode, COLOR_RGB_48);
	}
}

static void connector_add_444_valid_mode(struct hdmitx_connector *connector,
	struct hdmitx_display_mode *dis_mode)
{
	struct hdmitx_valid_mode valid_mode;
	struct color_depth *depth = &connector->color.depth;
	struct src_hdmitx *src = &connector->src_cap.hdmi;
	struct hdmi_detail *detail = &dis_mode->detail;
	bool hdmi_support = false;
	bool flag = false;

	if (memset_s(&valid_mode, sizeof(valid_mode), 0x0, sizeof(valid_mode)) != EOK) {
		dpu_pr_err("memset_s failed");
		return;
	}

	hdmi_support = connector->display_info.has_hdmi_infoframe &&
				   connector->src_cap.hdmi.hdmi_support;

	dis_mode->color_cap &= ~COLOR_Y444_24;
	flag = hdmi_support && depth->y444_24 && src->ycbcr444;
	if (flag) {
		set_band_mode(dis_mode->vic, YCBCR444, CD_24, &valid_mode.band);
		valid_mode.tmds_clock = detail->pixel_clock;
		if (connector_check_valid_mode(connector, dis_mode, &valid_mode))
			dis_mode->color_cap |= COLOR_Y444_24;
	}

	dis_mode->color_cap &= ~COLOR_Y444_30;
	flag = hdmi_support && depth->y444_30 && src->bpc_30 && src->ycbcr444;
	if (flag) {
		set_band_mode(dis_mode->vic, YCBCR444, CD_30, &valid_mode.band);
		valid_mode.tmds_clock = detail->pixel_clock * 5 / 4; /* 5/4: 1.25 times */
		connector_check_valid_mode_set_color(connector, dis_mode, &valid_mode, COLOR_Y444_30);
	}

	dis_mode->color_cap &= ~COLOR_Y444_36;
	flag = hdmi_support && depth->y444_36 && src->bpc_36 && src->ycbcr444;
	if (flag) {
		set_band_mode(dis_mode->vic, YCBCR444, CD_36, &valid_mode.band);
		valid_mode.tmds_clock = detail->pixel_clock * 3 / 2; /* 3/2: 1.5 times */
		connector_check_valid_mode_set_color(connector, dis_mode, &valid_mode, COLOR_Y444_36);
	}

	dis_mode->color_cap &= ~COLOR_Y444_48;
	if (hdmi_support && depth->y444_48 && src->bpc_48 && src->ycbcr444) {
		set_band_mode(dis_mode->vic, YCBCR444, CD_48, &valid_mode.band);
		valid_mode.tmds_clock = detail->pixel_clock << 1; /* 2 times */
		connector_check_valid_mode_set_color(connector, dis_mode, &valid_mode, COLOR_Y444_48);
	}
}

static void connector_add_422_valid_mode(struct hdmitx_connector *connector,
	struct hdmitx_display_mode *dis_mode)
{
	struct hdmitx_valid_mode valid_mode;
	struct src_hdmitx *src = &connector->src_cap.hdmi;
	struct hdmi_detail *detail = &dis_mode->detail;
	bool hdmi_support = false;

	if (memset_s(&valid_mode, sizeof(valid_mode), 0x0, sizeof(valid_mode)) != EOK) {
		dpu_pr_err("memset_s failed");
		return;
	}

	hdmi_support = connector->display_info.has_hdmi_infoframe &&
				   connector->src_cap.hdmi.hdmi_support;

	dis_mode->color_cap &= ~COLOR_Y422;
	if (hdmi_support && connector->color.format.ycbcr422 && src->ycbcr422) {
		set_band_mode(dis_mode->vic, YCBCR422, CD_24, &valid_mode.band);
		valid_mode.tmds_clock = detail->pixel_clock;
		if (connector_check_valid_mode(connector, dis_mode, &valid_mode)) {
			dis_mode->color_cap |= COLOR_Y422;
			dis_mode->status = MODE_OK;
		}
	}
}

static void connector_add_n420_valid_mode(struct hdmitx_connector *connector,
	struct hdmitx_display_mode *dis_mode)
{
	connector_add_rgb_valid_mode(connector, dis_mode);
	connector_add_444_valid_mode(connector, dis_mode);
	connector_add_422_valid_mode(connector, dis_mode);
}

static s32 connector_add_validmodes(struct hdmitx_connector *connector)
{
	struct list_head *n = NULL;
	struct list_head *pos = NULL;
	struct hdmitx_display_mode *dis_mode = NULL;

	if (connector == NULL)
		return -1;

	if (list_empty(&connector->probed_modes))
		return 0;

	list_for_each_safe(pos, n, &connector->probed_modes) {
		dis_mode = list_entry(pos, struct hdmitx_display_mode, head);
		if (dis_mode == NULL)
			continue;

		if ((dis_mode->parse_type & MODE_TYPE_Y420VDB) != MODE_TYPE_Y420VDB)
			connector_add_n420_valid_mode(connector, dis_mode);

		if ((dis_mode->parse_type & MODE_TYPE_Y420VDB) ||
			(dis_mode->parse_type & MODE_TYPE_VDB_Y420CMDB))
			connector_add_420_valid_mode(connector, dis_mode);
	}

	return 0;
}

static s32 connector_destroy_modes(struct hdmitx_connector *connector)
{
	struct list_head *n = NULL;
	struct list_head *p = NULL;
	struct hdmitx_valid_mode *valid_mode = NULL;
	struct hdmitx_display_mode *dis_mode = NULL;

	if (!list_empty(&connector->valid_modes)) {
		list_for_each_safe(p, n, &connector->valid_modes) {
			valid_mode = list_entry(p, struct hdmitx_valid_mode, head);
			if (valid_mode != NULL) {
				connector_destroy_valid_mode(connector, valid_mode);
				valid_mode = NULL;
			}
		}
	}

	if (!list_empty(&connector->probed_modes)) {
		list_for_each_safe(p, n, &connector->probed_modes) {
			dis_mode = list_entry(p, struct hdmitx_display_mode, head);
			if (dis_mode != NULL) {
				list_del(&dis_mode->head);
				hdmitx_modes_destroy_mode(dis_mode);
			}
		}
	}
	connector->probed_mode_cnt = 0;

	return 0;
}

static s32 connector_edid_info_reset(struct hdmitx_connector *connector)
{
	if (memset_s(&connector->audio, sizeof(connector->audio), 0, sizeof(connector->audio)) != EOK)
		goto err_memset;

	if (memset_s(&connector->base, sizeof(connector->base), 0, sizeof(connector->base)) != EOK)
		goto err_memset;

	if (memset_s(&connector->color, sizeof(connector->color), 0, sizeof(connector->color)) != EOK)
		goto err_memset;

	if (memset_s(&connector->dolby, sizeof(connector->dolby), 0, sizeof(connector->dolby)) != EOK)
		goto err_memset;

	if (memset_s(&connector->cuva, sizeof(connector->cuva), 0, sizeof(connector->cuva)) != EOK)
		goto err_memset;

	if (memset_s(&connector->dsc, sizeof(connector->dsc), 0, sizeof(connector->dsc)) != EOK)
		goto err_memset;

	if (memset_s(&connector->hdr, sizeof(connector->hdr), 0, sizeof(connector->hdr)) != EOK)
		goto err_memset;

	if (memset_s(&connector->latency, sizeof(connector->latency), 0, sizeof(connector->latency)) != EOK)
		goto err_memset;

	if (memset_s(&connector->scdc, sizeof(connector->scdc), 0, sizeof(connector->scdc)) != EOK)
		goto err_memset;

	if (memset_s(&connector->timing, sizeof(connector->timing), 0, sizeof(connector->timing)) != EOK)
		goto err_memset;

	if (memset_s(&connector->vrr, sizeof(connector->vrr), 0, sizeof(connector->vrr)) != EOK)
		goto err_memset;

	if (memset_s(&connector->display_info, sizeof(connector->display_info),
				 0, sizeof(connector->display_info)) != EOK)
		goto err_memset;

	return 0;
err_memset:
	dpu_pr_err("memset failed");
	return -1;
}

static void connector_edid_reset(struct hdmitx_connector *connector)
{
	mutex_lock(&connector->mutex);
	if (connector->edid_src_type == EDID_FROM_SINK && connector->edid_raw)
		kfree(connector->edid_raw);

	connector->edid_raw = NULL;
	connector->edid_size = 0;
	connector->detail_vic_base = VIC_DETAIL_TIMING_BASE;

	if (connector_edid_info_reset(connector))
		dpu_pr_err("edid info reset failed");

	connector->edid_src_type = EDID_EMPTY;

	if (connector_destroy_modes(connector))
		dpu_pr_err("destroy mode fail");

	mutex_unlock(&connector->mutex);
}

static void connector_set_edid_info(struct hdmitx_connector *connector,
	const u8 *edid, u32 size, s32 type)
{
	mutex_lock(&connector->mutex);
	connector->edid_size = (s32)size;
	connector->edid_raw = (struct edid *)edid;
	connector->edid_src_type = type;
	/* add modes */
	if (!drv_hdmitx_edid_add_modes(connector, connector->edid_raw))
		dpu_pr_err("no mode");

	if (connector_add_validmodes(connector))
		dpu_pr_err("add valid modes fail");
	mutex_unlock(&connector->mutex);
}

static u8 *connector_get_edid_base_block(struct hdmitx_ddc *ddc)
{
	u32 i;
	u8 *edid = NULL;

	edid = kmalloc(EDID_LENGTH, GFP_KERNEL);
	if (edid == NULL) {
		dpu_pr_err("kmalloc fail");
		return NULL;
	}

	if (memset_s(edid, EDID_LENGTH, 0, EDID_LENGTH) != EOK) {
		dpu_pr_err("memset_s fail");
		kfree(edid);
		edid = NULL;
		return NULL;
	}

	/* base block fetch */
	for (i = 0; i < 4; i++) { /* max retry 4 times. */
		if (hdmitx_ddc_edid_read(ddc, edid, 0, EDID_LENGTH) != 0) {
			kfree(edid);
			edid = NULL;
			return NULL;
		}
		/* &connector->edid_corrupt */
		if (drv_hdmitx_edid_block_is_valid(edid, 0, false))
			break;

		if (i == 0 && drv_hdmitx_edid_data_is_zero(edid, EDID_LENGTH)) {
			dpu_pr_err("data_is_zero");
			kfree(edid);
			edid = NULL;
			return NULL;
		}
	}

	if (i == 4) { /* retry 4 times. */
		kfree(edid);
		edid = NULL;
		dpu_pr_err("retry 4 times");
		return NULL;
	}

	return edid;
}

static u8 *connector_get_edid_ext_block(struct hdmitx_ddc *ddc, u8 *base_edid, u32 *valid_extensions)
{
	u32 i, j;
	u32 extensions_num = *valid_extensions;
	u8 *edid = NULL;
	u8 *block = NULL;

	edid = kmalloc((extensions_num + 1) * EDID_LENGTH, GFP_KERNEL);
	if (edid == NULL) {
		dpu_pr_err("kmalloc fail");
		return NULL;
	}

	if (memset_s(edid, (extensions_num + 1) * EDID_LENGTH, 0, (extensions_num + 1) * EDID_LENGTH) != EOK) {
		dpu_pr_err("memset_s fail");
		kfree(edid);
		edid = NULL;
		return NULL;
	}

	if (memcpy_s(edid, (extensions_num + 1) * EDID_LENGTH, base_edid, EDID_LENGTH) != EOK) {
		kfree(edid);
		edid = NULL;
		dpu_pr_err("memcpy_s failed");
		return NULL;
	}

	for (j = 1; j <= extensions_num; j++) {
		block = edid + j * EDID_LENGTH;
		for (i = 0; i < 4; i++) { /* max retry 4 times. */
			if (hdmitx_ddc_edid_read(ddc, block, j, EDID_LENGTH) != 0) {
				kfree(edid);
				edid = NULL;
				return NULL;
			}

			if (drv_hdmitx_edid_block_is_valid(block, j, false))
				break;
		}

		if (i == 4) /* retry 4 times. */
			extensions_num--;
	}

	for (j = 1; j <= extensions_num; j++) {
		block = edid + j * EDID_LENGTH;
		dump_edid(block, j);
	}

	*valid_extensions = extensions_num;
	return edid;
}

static u8 *connector_get_fixed_edid(u8 *raw_edid, u32 valid_extensions)
{
	u32 i;
	u8 *edid = NULL;
	u8 *block = NULL;
	u8 *base = NULL;

	raw_edid[EDID_LENGTH - 1] += raw_edid[0x7e] - valid_extensions;
	raw_edid[0x7e] = valid_extensions;

	edid = kmalloc((valid_extensions + 1) * EDID_LENGTH, GFP_KERNEL);
	if (edid == NULL) {
		dpu_pr_err("kmalloc fail");
		return NULL;
	}

	if (memset_s(edid, (valid_extensions + 1) * EDID_LENGTH, 0, (valid_extensions + 1) * EDID_LENGTH) != EOK) {
		dpu_pr_err("memset_s fail");
		kfree(edid);
		edid = NULL;
		return NULL;
	}

	base = edid;
	for (i = 0; i <= valid_extensions; i++) {
		block = raw_edid + i * EDID_LENGTH;
		if (!drv_hdmitx_edid_block_is_valid(block, i, false))
			continue;

		if (memcpy_s(base, (valid_extensions + 1 - i) * EDID_LENGTH, block, EDID_LENGTH) != EOK) {
			kfree(edid);
			edid = NULL;
			return NULL;
		}
		base += EDID_LENGTH;
	}

	return edid;
}

static struct edid *hdmitx_connector_get_edid(struct hdmitx_connector *connector, struct hdmitx_ddc *ddc)
{
	u32 size;
	u32 valid_extensions;
	u8 *edid = NULL;
	u8 *new_edid = NULL;
	u64 temp;

	edid = connector_get_edid_base_block(ddc);
	if (edid == NULL) {
		dpu_pr_err("get base block fail");
		goto out;
	}
	dump_edid(edid, 0);

	/* if there's no extensions, we're done */
	valid_extensions = edid[0x7e];
	size = EDID_LENGTH;
	if (valid_extensions == 0) {
		connector_set_edid_info(connector, edid, size, EDID_FROM_SINK);
		return (struct edid *)edid;
	}

	new_edid = connector_get_edid_ext_block(ddc, edid, &valid_extensions);
	if (new_edid == NULL) {
		dpu_pr_err("get ext blocks fail");
		goto free;
	}
	kfree(edid);
	edid = new_edid;

	if (valid_extensions != edid[0x7e]) {
		new_edid = connector_get_fixed_edid(edid, valid_extensions);
		if (new_edid == NULL) {
			dpu_pr_err("get fixed edid fail");
			goto free;
		}
		kfree(edid);
		edid = new_edid;
	}
	temp = (valid_extensions + 1) * EDID_LENGTH;
	size = (u32)temp;

	connector_set_edid_info(connector, edid, size, EDID_FROM_SINK);

	return (struct edid *)edid;

free:
	kfree(edid);
	edid = NULL;
out:
	mutex_lock(&connector->mutex);
	connector->edid_size = 0;
	mutex_unlock(&connector->mutex);
	return NULL;
}


bool hdmitx_connector_is_scdc_present(struct hdmitx_ctrl *hdmitx)
{
	if (hdmitx == NULL) {
		dpu_pr_err("ptr is null");
		return false;
	}

	if (!hdmitx->hdmitx_enable) {
		dpu_pr_info("scdc unkonw, no plug in");
		return false;
	} else {
		return hdmitx->hdmitx_connector->scdc.present;
	}
}

static void connector_source_tmds_capability_init(struct src_hdmitx *src)
{
	bool tmds_clock_600m = false;

	/* tmds */
	src->dvi_support = true;
	src->hdmi_support = true;
	tmds_clock_600m = true;
	if (tmds_clock_600m) {
		src->max_tmds_clock = HDMITX20_MAX_TMDS_CLK;
	} else {
		src->max_tmds_clock = HDMITX14_MAX_TMDS_CLK;
	}

	/* scdc */
	src->scdc_present = true;
	src->scdc_lte_340mcsc = false;
}

static void connector_source_frl_dsc_capability_init(struct src_hdmitx *src)
{
	u32 max_slice;
	u32 max_pixel_clk_per_line;

	/* frl */
	src->max_frl_rate = FRL_RATE_12G4L;
	src->ffe_levels = 3;

	/* dsc */
	src->dsc_support = true;
	src->native_y420 = false;
	src->dsc_10bpc = true;
	src->dsc_12bpc = false;
	max_slice = DSC_CAP_MAX_SLICE_CNT_3;
	switch (max_slice) {
	case DSC_CAP_MAX_SLICE_CNT_0:
		break;
	case DSC_CAP_MAX_SLICE_CNT_1:
		src->max_silce_count = 1; /* 1 slices */
		break;
	case DSC_CAP_MAX_SLICE_CNT_2:
		src->max_silce_count = 2; /* 2 slices */
		break;
	case DSC_CAP_MAX_SLICE_CNT_3:
		src->max_silce_count = 4; /* 4 slices */
		break;
	case DSC_CAP_MAX_SLICE_CNT_4:
		src->max_silce_count = 8; /* 8 slices */
		break;
	case DSC_CAP_MAX_SLICE_CNT_5:
		src->max_silce_count = 12; /* 12 slices */
		break;
	case DSC_CAP_MAX_SLICE_CNT_6:
		src->max_silce_count = 16; /* 16 slices */
		break;
	default:
		break;
	}

	src->max_dsc_frl_rate = src->max_frl_rate;

	max_pixel_clk_per_line = 0; // udp to fix, need confirm after
	if (max_pixel_clk_per_line == 0) {
		src->max_pixel_clk_per_slice = 340; /* 340Mhz */
	} else if (max_pixel_clk_per_line == 1) {
		src->max_pixel_clk_per_slice = 400; /* 400Mhz */
	}
}

static void connector_source_csc_capability_init(struct src_hdmitx *src)
{
	u32 pixel_clock_level;
	/* color depth */
	src->bpc_30 = true;
	src->bpc_36 = false;
	src->bpc_48 = false;

	/* csc */
	src->rgb2yuv = false;
	src->ycbcr444 = false;
	src->ycbcr422 = false;
	src->ycbcr420 = true;
	src->yuv2rgb = false;
	src->dither_support = false;

	/* pixel_clock */
	pixel_clock_level = HDMITX21_8K120_MAX_PIXEL_CLK_LEVEL;
	if (pixel_clock_level == HDMITX14_MAX_PIXEL_CLK_LEVEL) {
		src->max_pixel_clock = HDMITX14_MAX_PIXEL_CLK;
	} else if (pixel_clock_level == HDMITX20_MAX_PIXEL_CLK_LEVEL) {
		src->max_pixel_clock = HDMITX20_MAX_PIXEL_CLK;
	} else if (pixel_clock_level == HDMITX21_8K30_MAX_PIXEL_CLK_LEVEL) {
		src->max_pixel_clock = HDMITX21_8K30_MAX_PIXEL_CLK;
	} else if (pixel_clock_level == HDMITX21_8K60_MAX_PIXEL_CLK_LEVEL) {
		src->max_pixel_clock = HDMITX21_8K60_MAX_PIXEL_CLK;
	} else if (pixel_clock_level == HDMITX21_8K120_MAX_PIXEL_CLK_LEVEL) {
		src->max_pixel_clock = HDMITX21_8K120_MAX_PIXEL_CLK;
	} else {
		src->max_pixel_clock = HDMITX21_8K120_MAX_PIXEL_CLK;
	}
}

static void hdmitx_source_capability_init(struct hdmitx_connector *connector)
{
	struct source_capability *src = &connector->src_cap;

	src->cec_support = false;
	src->hdcp14_support = true;
	src->hdcp2x_support = true;

	connector_source_tmds_capability_init(&src->hdmi);
	connector_source_frl_dsc_capability_init(&src->hdmi);
	connector_source_csc_capability_init(&src->hdmi);

	src->hpd.fillter_en = true;
	src->hpd.high_reshold = HDMITX_HPD_HIGH_RESHOLD;
	src->hpd.low_reshold = HDMITX_HPD_LOW_RESHOLD;
}

bool hdmitx_connector_search_mode(struct hdmitx_connector *connector, struct hdmitx_mode_config *mode)
{
	struct list_head *n = NULL;
	struct list_head *p = NULL;
	struct hdmitx_valid_mode *tmp_mode = NULL;
	bool ret = false;

	if ((connector == NULL) || (mode == NULL))
		return false;

	mutex_lock(&connector->mutex);

	if (list_empty(&connector->valid_modes)) {
		mutex_unlock(&connector->mutex);
		return false;
	}

	list_for_each_safe(p, n, &connector->valid_modes) {
		tmp_mode = list_entry(p, struct hdmitx_valid_mode, head);
		if (tmp_mode->band.vic == mode->band.vic &&
			tmp_mode->band.color_format == mode->band.color_format &&
			tmp_mode->band.color_depth == mode->band.color_depth &&
			tmp_mode->valid) {
			mode->band = tmp_mode->band;
			mode->dsc = tmp_mode->dsc;
			mode->frl = tmp_mode->frl;
			mode->hdmi_mode = tmp_mode->hdmi_mode;
			mode->tmds_scdc_en = tmp_mode->tmds_scdc_en;
			mode->tmds_encode = tmp_mode->tmds_encode;
			mode->tmds_clock = tmp_mode->tmds_clock;
			ret = true;
			break;
		}
		dpu_pr_info("tmp_mode->band vic = %u, color_format = %d", tmp_mode->band.vic,
					tmp_mode->band.color_format);
	}

	mutex_unlock(&connector->mutex);

	dpu_pr_info("mode->band vic %u, color_format %u, color_depth %u", mode->band.vic,
		mode->band.color_format, mode->band.color_depth);
	dpu_pr_info("mode->dsc  frl_compress %u, min_dsc_frl_rate %u, max_dsc_frl_rate %u, bpp_target %u\n"
		"slice_width %u, hcactive %u, hcblank %u, slice_count %u",
		mode->dsc.frl_compress, mode->dsc.min_dsc_frl_rate, mode->dsc.max_dsc_frl_rate, mode->dsc.bpp_target,
		mode->dsc.slice_width, mode->dsc.hcactive, mode->dsc.hcblank, mode->dsc.slice_count);
	dpu_pr_info("mode->frl frl_uncompress %u, min_frl_rate %u, max_frl_rate %u, ffe_levels %u",
		mode->frl.frl_uncompress, mode->frl.min_frl_rate, mode->frl.max_frl_rate, mode->frl.ffe_levels);
	dpu_pr_info("hdmi_mode %u, tmds_scdc_en %u, tmds_encode %u, tmds_clock %u",
		mode->hdmi_mode, mode->tmds_scdc_en, mode->tmds_encode, mode->tmds_clock);

	return ret;
}

s32 hdmitx_ctrl_tmds_set_scramble(struct hdmitx_ctrl *hdmitx)
{
	struct hdmitx_connector *connector = NULL;
	struct hdmitx_mode_config mode_config;

	if (hdmitx == NULL) {
		dpu_pr_err("null ptr err");
		return -1;
	}
	connector = hdmitx->hdmitx_connector;
	mode_config.band.vic = hdmitx->select_mode.vic;
	mode_config.band.color_format = HDMITX_COLOR_FORMAT_RGB;
	mode_config.band.color_depth = (u32)g_hdmitx_color_depth_debug;
	if (!hdmitx_connector_search_mode(connector, &mode_config)) {
		dpu_pr_err("mode(vic=%u,c_fmt=%u,c_depth=%u) isn't avail, please set an avail-mode\n",
			mode_config.band.vic, mode_config.band.color_format, mode_config.band.color_depth);
		return -1;
	}

	/*
	 * When the timing is switched, the scrambling is disabled firstly.
	 * For example, the 4K TV is played, and the HDMITX cable is removed
	 * and then inserted into a non-4K TV. As a result, the scrambling
	 * is not disabled, and no output is displayed on the TV. So must
	 * be disable scramble firstly.
	 */
	core_hw_enable_tmds_scramble(hdmitx->base, false);
	hdmitx_ddc_scdc_set_scrambling(hdmitx->ddc, false);

	/*
	 * When you operate SCDC, check whether the RXs support. Avoid the
	 * crash caused by the SCDC that some RXs does not support the SCDC
	 * operation. Force output just for debug.
	 */
	if (!connector->scdc.present) {
		dpu_pr_err("not support scdc");
		return -1;
	}

	if (mode_config.tmds_scdc_en && hdmitx->mode == HDMITX_WORK_MODE_TMDS) {
		hdmitx_ddc_scdc_set_scrambling(hdmitx->ddc, true);
		core_hw_enable_tmds_scramble(hdmitx->base, true);
		dpu_pr_info("tmds need scramble");
	} else {
		hdmitx_ddc_scdc_set_scrambling(hdmitx->ddc, false);
		dpu_pr_info("no need scramble");
	}
	dpu_pr_info("sink scramble status:%u", hdmitx_ddc_scdc_get_scrambling_status(hdmitx->ddc));

	return 0;
}

int hdmitx_connector_init(struct hdmitx_ctrl *hdmitx)
{
	struct hdmitx_connector *connector = NULL;

	dpu_check_and_return(!hdmitx, -EINVAL, err, "[HDMI] hdmitx is NULL");
	connector = kmalloc(sizeof(struct hdmitx_connector), GFP_KERNEL);
	if (connector == NULL) {
		dpu_pr_err("kmalloc fail");
		return -1;
	}

	if (memset_s(connector, sizeof(*connector), 0, sizeof(*connector)) != EOK) {
		dpu_pr_err("memset_s fail");
		return -1;
	}

	connector->detail_vic_base = VIC_DETAIL_TIMING_BASE;

	mutex_init(&connector->mutex);
	INIT_LIST_HEAD(&connector->valid_modes);
	INIT_LIST_HEAD(&connector->probed_modes);
	connector->name = "hdmitx-connector0";

	hdmitx_source_capability_init(connector);
	hdmitx->hdmitx_connector = connector;
	return 0;
}

void hdmitx_subsys_init(struct hdmitx_ctrl *hdmitx)
{
	struct hdmitx_private *hdmitx_priv = NULL;
	dpu_check_and_no_retval(!hdmitx, err, "null pointer");
	dpu_pr_info("+");

	hdmitx_priv = to_hdmitx_private(hdmitx->connector->conn_info);

	// diable iso
	hdmi_writel(hdmitx_priv->hsdt1_sys_ctrl_base, HDMITX_PHY_ISO_EN, 0x0);

	// enable hdcp clk 
	core_enable_hdcp_clk(hdmitx_priv->hsdt1_sub_harden_base);
	// dss2hdmi choose stream0, now config static for udp verify, then need config dynamic between dp and hdmi;
	// video stream select dss post channel(2,3,4,5)
	hdmi_write_bits(hdmitx_priv->hsdt1_sys_ctrl_base, DSS2DPTX_SEL, DSS2DPTX_STREAM2_IN_SEL_MASK, 0xa);

	core_enable_memory(hdmitx->sysctrl_base);
	// dis reset pwd
	core_reset_pwd(hdmitx->base, false);

	// disable oe
	drv_hdmitx_phy_off(hdmitx->phy);

	dpu_pr_info("-");
}

static int hdmitx_ddc_gpio_init(struct hdmitx_ctrl *hdmitx)
{
	struct hdmitx_private *hdmitx_priv = NULL;
	dpu_check_and_return(!hdmitx, -1, err, "null pointer");
	hdmitx_priv = to_hdmitx_private(hdmitx->connector->conn_info);
	dpu_pr_info("+");
	// config ddc pinctrl
	if (peri_pinctrl_cmds_tx(hdmitx_priv->device, scl_pinctrl_init_cmds, ARRAY_SIZE(scl_pinctrl_init_cmds)) != 0) {
		dpu_pr_err("Init scl pinctrl failed");
		return -1;
	}

	if (peri_pinctrl_cmds_tx(hdmitx_priv->device, sda_pinctrl_init_cmds, ARRAY_SIZE(sda_pinctrl_init_cmds)) != 0) {
		dpu_pr_err("Init sda pinctrl failed");
		return -1;
	}
	dpu_pr_info("-");
	return 0;
}

static int hdmitx_update_panel_info(struct hdmitx_ctrl *hdmitx)
{
	struct hdmi_detail *mdtd = NULL;
	struct dkmd_object_info *upload_pinfo = NULL;
	struct dkmd_connector_info *pinfo = NULL;

	mdtd = &(hdmitx->select_mode.detail);
	pinfo = hdmitx->connector->conn_info;
	upload_pinfo = pinfo->base.comp_obj_info;

	upload_pinfo->xres = pinfo->base.xres = mdtd->h_active;
	upload_pinfo->yres = pinfo->base.yres = mdtd->v_active;
	upload_pinfo->fps = pinfo->base.fps = mdtd->field_rate;

	upload_pinfo->dsc_out_width = pinfo->base.dsc_out_width = pinfo->base.xres;
	upload_pinfo->dsc_out_height = pinfo->base.dsc_out_height = pinfo->base.yres;

	hdmitx->connector->ldi.h_back_porch = mdtd->h_back;
	hdmitx->connector->ldi.h_front_porch = mdtd->h_front;
	hdmitx->connector->ldi.h_pulse_width = mdtd->h_sync;

	hdmitx->connector->ldi.v_back_porch = mdtd->v_back;
	hdmitx->connector->ldi.v_front_porch = mdtd->v_front;
	hdmitx->connector->ldi.v_pulse_width = mdtd->v_sync;
	hdmitx->connector->ldi.pxl_clk_rate_div = 1;

	upload_pinfo->width = pinfo->base.width = 530;
	upload_pinfo->height = pinfo->base.height = 300;

	hdmitx->connector->ldi.pxl_clk_rate = mdtd->pixel_clock * 1000;

	return 0;
}

static void hdmitx_tmds_phy_set(const struct hdmitx_ctrl *hdmitx)
{
	u32 color_format;
	u32 color_depth;
	u32 pixel_clk;
	struct phy_tmds tmds;

	if (hdmitx == NULL) {
		dpu_pr_err("hdmitx_ctrl ptr is null");
		return;
	}
	color_format = HDMITX_COLOR_FORMAT_RGB;
	color_depth = (u32)g_hdmitx_color_depth_debug;
	pixel_clk = hdmitx->select_mode.detail.pixel_clock; // to fix: here stub, need modify

	if (color_format == HDMITX_COLOR_FORMAT_YCBCR420) {
		tmds.pixel_clk = pixel_clk / 2; /* pixel clock need divid by 2. */
		tmds.yuv_420 = true;
	} else {
		tmds.pixel_clk = pixel_clk;
		tmds.yuv_420 = false;
	}
	tmds.tmds_clk = hdmitx_modes_get_tmds_clk(pixel_clk, color_depth, color_format);
	tmds.color_depth = color_depth;
	tmds.pcb_len = PCB_LEN_1;
	drv_hdmitx_phy_configure_tmds(hdmitx->phy, &tmds);
}

static void hdmitx_frl_tmds_select(const struct hdmitx_ctrl *hdmitx)
{
	hdmi_writel(hdmitx->sysctrl_base, TMDS_FRL_SEL, hdmitx->mode);
	dpu_pr_info("[HDMI] select hdmitx->mode = %u (frl: 1, tmds: 0).\n", hdmitx->mode);
}

static int hdmitx_frl_mode_start(struct hdmitx_ctrl *hdmitx)
{
	int ret;
	dpu_pr_info("+");
	dpu_check_and_return(!hdmitx, -EINVAL, err, "[HDMI] hdmitx is NULL");

	// fpga phy init
	if (hdmitx->connector->conn_info->base.fpga_flag == 1)
		drv_hdmitx_phy_configure_frl_fpga(hdmitx);

	// frl mode config
	hdmitx_frl_tmds_select(hdmitx);

	// select timing before training
	hdmitx_timing_mock(hdmitx);

	// start frl link training
	ret = hdmitx_frl_start(hdmitx);
	if (ret) {
		dpu_pr_err("link training failed");
		hdmitx_frl_stop(hdmitx->frl);
		hdmitx_frl_deinit(hdmitx->frl);
		hdmitx->frl = NULL;
		return -1;
	}

	// choose timing by kernel and config timing
	hdmitx_timing_config(hdmitx);

	// update timing info to dss
	hdmitx_update_panel_info(hdmitx);
	// dpc pll config
	ret = hdmitx_dpc_pll_init(hdmitx);
	if (ret) {
		dpu_pr_err("frl mode hdmitx dpc pll init failed, hdmitx frl link failed");
		hdmitx_frl_stop(hdmitx->frl);
		hdmitx_frl_deinit(hdmitx->frl);
		hdmitx->frl = NULL;
		hdmitx_dpc_pll_deinit(hdmitx);
		return -1;
	}

	// init infoframe from status
	hdmitx_init_infoframe(hdmitx);
	hdmitx_video_config(hdmitx);
	hdmitx->video_transfer_enable = true;
	hdmitx_switch_set_state(&hdmitx->sdev, HOT_PLUG_IN);
	dpu_pr_info("-");
	return 0;
}

static int hdmitx_tmds_mode_start(struct hdmitx_ctrl *hdmitx)
{
	int ret = 0;
	dpu_pr_info("+");
	dpu_check_and_return(!hdmitx, -EINVAL, err, "[HDMI] hdmitx is NULL");

	// frl mode config
	hdmitx_frl_tmds_select(hdmitx);

	// choose timing by kernel and config timing
	hdmitx_timing_config(hdmitx);

	// update timing info to dss
	hdmitx_update_panel_info(hdmitx);

	// dpc pll config
	ret = hdmitx_dpc_pll_init(hdmitx);
	if (ret) {
		dpu_pr_err("tmds mode hdmitx dpc pll init failed, hdmitx tmds link failed");
		hdmitx_dpc_pll_deinit(hdmitx);
		return -1;
	}

	drv_hdmitx_crg_set(hdmitx);

	// start tmds phy configure
	hdmitx_tmds_phy_set(hdmitx);

	// init infoframe from status
	hdmitx_init_infoframe(hdmitx);
	hdmitx_video_config(hdmitx);
	core_hdmitx_controller_soft_reset(hdmitx);
	// oe enable
	drv_hdmitx_phy_on(hdmitx->phy);
	hdmitx->video_transfer_enable = true;
	hdmitx_switch_set_state(&hdmitx->sdev, HOT_PLUG_IN);
	dpu_pr_info("-");
	return 0;
}

static int hdmitx_handle_hotplug(struct hdmitx_ctrl *hdmitx)
{
	int ret = 0;
	dpu_pr_info("+");
	dpu_check_and_return(!hdmitx, -EINVAL, err, "[HDMI] hdmitx is NULL");
	if (hdmitx->video_transfer_enable == true) {
		dpu_pr_info("[HDMI] video_transfer_enable already true");
		return -1;
	}

	hdmitx->hdmitx_enable = true;
	hdmitx->mode = g_hdmitx_frl_tmds_mode_debug; // to fix

	dpu_pr_info("[HDMI] hdmi mode is %u", hdmitx->mode);

	// aon init
	ret = hdmitx_aon_init(hdmitx);
	if (ret) {
		dpu_pr_err("[HDMI] hdmitx_aon_init failed!");
		return ret;
	}

	// subsys init
	hdmitx_subsys_init(hdmitx);

	// connfig II2C gpio
	if (hdmitx_ddc_gpio_init(hdmitx)) {
		dpu_pr_err("[HDMI] hdmitx config ddc gpio failed");
		return -1;
	}

	// get edid and add modes
	if (hdmitx_connector_get_edid(hdmitx->hdmitx_connector, hdmitx->ddc) == NULL) {
		dpu_pr_err("[HDMI] hdmitx get edid failed");
		return -1;
	}

	if (hdmitx->mode == HDMITX_WORK_MODE_FRL) {
		ret = hdmitx_frl_mode_start(hdmitx);
		if (ret) {
			dpu_pr_err("[HDMI] frl mode link failed and try tmds mode to link");
			hdmitx->mode = HDMITX_WORK_MODE_TMDS;
			ret = hdmitx_tmds_mode_start(hdmitx);
		}
	} else {
		ret = hdmitx_tmds_mode_start(hdmitx);
		if (ret)
			dpu_pr_err("[HDMI] tmds mode link failed");
	}

	dpu_pr_info("-");
	return ret;
}

void hdmitx_hotplug()
{
	struct dkmd_connector_info *pinfo = NULL;
	struct dkmd_object_info *upload_pinfo = NULL;
	struct hdmitx_ctrl *hdmitx = g_hdmitx;
	dpu_check_and_no_retval(!hdmitx, err, "[HDMI] hdmitx is NULL");

	pinfo = hdmitx->connector->conn_info;
	upload_pinfo = pinfo->base.comp_obj_info;

	if (hdmitx_handle_hotplug(hdmitx)) {
		dpu_pr_err("[HDMI] hdmitx hotplug failed");
		hdmitx_hotunplug();
		return;
	}
	mutex_lock(pinfo->base.pluggable_connect_mutex);
	upload_pinfo->connect_status = pinfo->base.connect_status = HDMITX_CONNECTED;
	mutex_unlock(pinfo->base.pluggable_connect_mutex);
	return;
}

void hdmitx_hotunplug()
{
	struct dkmd_connector_info *pinfo = NULL;
	struct dkmd_object_info *upload_pinfo = NULL;
	struct hdmitx_ctrl *hdmitx = g_hdmitx;
	struct hdmitx_private *hdmitx_priv = NULL;
	dpu_check_and_no_retval(!hdmitx, err, "[HDMI] hdmitx is NULL");
	dpu_pr_info("+\n");

	if (hdmitx->hdmitx_enable == false) {
		dpu_pr_info("[HDMI] hdmitx_enable already false");
		return;
	}

	hdmitx_priv = to_hdmitx_private(hdmitx->connector->conn_info);
	pinfo = hdmitx->connector->conn_info;
	upload_pinfo = pinfo->base.comp_obj_info;

	mutex_lock(pinfo->base.pluggable_connect_mutex);
	upload_pinfo->connect_status = pinfo->base.connect_status = HDMITX_DISCONNECTED;
	mutex_unlock(pinfo->base.pluggable_connect_mutex);

	hdmitx_timing_gen_disable(hdmitx);

	/**
	* solve underflow problem when hdmi plug in after first hdmi plug out, 41.6ms is wait hdmi send
	* frm_end to dss then dss can work normal, Consider the minimum frame rate of 24hz.
	*/
	usleep_range(41500, 41600);
	peri_pinctrl_cmds_tx(hdmitx_priv->device, scl_pinctrl_finit_cmds, ARRAY_SIZE(scl_pinctrl_finit_cmds));
	peri_pinctrl_cmds_tx(hdmitx_priv->device, sda_pinctrl_finit_cmds, ARRAY_SIZE(sda_pinctrl_finit_cmds));
	hdmitx_dpc_pll_deinit(hdmitx);
	if (hdmitx->mode == HDMITX_WORK_MODE_FRL && hdmitx->video_transfer_enable) {
		hdmitx_frl_stop(hdmitx->frl);
		hdmitx_frl_deinit(hdmitx->frl);
		hdmitx->frl = NULL;
	}
	core_disable_memory(hdmitx->sysctrl_base);
	core_reset_pwd(hdmitx->base, true);
	core_reset_req_all(hdmitx->sysctrl_base);
	hdmitx_aon_deinit(hdmitx);
	hdmitx->video_transfer_enable = false;
	hdmitx->hdmitx_enable = false;
	connector_edid_reset(hdmitx->hdmitx_connector);
	hdmitx_switch_set_state(&hdmitx->sdev, HOT_PLUG_OUT);
	dpu_pr_info("-");
	return;
}