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
#include <linux/types.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <platform_include/basicplatform/linux/switch.h>
#include <dpu/soc_dpu_define.h>
#include <securec.h>
#include "peri/dkmd_peri.h"
#include "dp_aux.h"
#include "dp_drv.h"
#include "dkmd_connector.h"
#include "drm_dp_helper_additions.h"
#include "dp_link_layer_interface.h"
#include "dpu_dp_dbg.h"
#include "dp_ctrl_dev.h"
#include "hidptx/hidptx_reg.h"
#include "hidptx/hidptx_dp_core.h"
#include "controller/dp_avgen_base.h"
#include "dp_ctrl_config.h"
#include "dpu_conn_mgr.h"
#include "psr_config_base.h"
#include "dvfs.h"
#include <platform_include/display/linux/dpu_dss_dp.h>
#include "hdcp_common.h"

struct dp_hook_ops *g_dp_hook_ops;

extern int dpu_multi_dptx_hpd_trigger(TCA_IRQ_TYPE_E irq_type, TCPC_MUX_CTRL_TYPE mode,
								TYPEC_PLUG_ORIEN_E typec_orien, uint32_t dp_id, int port_id);

int dpu_dptx_get_spec(void *data, unsigned int size, unsigned int *ext_acount)
{
	struct dp_private *dp_priv = NULL;
	struct dkmd_connector_info *pinfo = NULL;
	struct dp_ctrl *dptx = NULL;
	struct edid_audio *audio_info = NULL;

	dpu_check_and_return(!data, -EINVAL,  err, "[DP] data is null pointer!");
	dpu_check_and_return(!g_dkmd_dp_devive[GFX_DP], -EINVAL, err, "[DP] dp device is null pointer!");
	pinfo = platform_get_drvdata(g_dkmd_dp_devive[GFX_DP]);
	dpu_check_and_return(!pinfo, -EINVAL, err, "[DP] get connector info failed!");
	dp_priv = to_dp_private(pinfo);
	dpu_check_and_return(!dp_priv, -EINVAL, err, "[DP] dp_priv is null pointer\n");
	dptx = &dp_priv->dp[MASTER_DPTX_IDX];
	dpu_check_and_return(!dptx, -EINVAL, err, "[DP] dptx is null pointer");

	audio_info = &dptx->edid_info.audio;
	if (size < sizeof(struct edid_audio_info) * audio_info->ext_acount) {
		dpu_pr_err("[DP] size is error!size %d ext_acount %d", size, audio_info->ext_acount);
		return -EINVAL;
	}

	dpu_check_and_return(!audio_info->spec, -EINVAL, err, "[DP] audio_info->spec is null pointer");
	if (memcpy_s(data, size, audio_info->spec, sizeof(struct edid_audio_info) * audio_info->ext_acount) != EOK) {
		dpu_pr_err("[DP] memcpy buffer error!");
		return -EINVAL;
	}

	*ext_acount = audio_info->ext_acount;

	dpu_pr_info("[DP] get spec success");

	return 0;
}

int dpu_dptx_set_aparam(unsigned int channel_num, unsigned int data_width, unsigned int sample_rate)
{
	struct dp_private *dp_priv = NULL;
	struct dkmd_connector_info *pinfo = NULL;
	struct dp_ctrl *dptx = NULL;
	uint8_t orig_sample_freq = 0;
	uint8_t sample_freq = 0;
	struct audio_params *aparams = NULL;

	dpu_check_and_return(!g_dkmd_dp_devive[GFX_DP], -EINVAL, err, "[DP] dp device is null pointer!");
	pinfo = platform_get_drvdata(g_dkmd_dp_devive[GFX_DP]);
	dpu_check_and_return(!pinfo, -EINVAL, err, "[DP] get connector info failed!");
	dp_priv = to_dp_private(pinfo);
	dpu_check_and_return(!dp_priv, -EINVAL, err, "[DP] dp_priv is null pointer\n");
	dptx = &dp_priv->dp[MASTER_DPTX_IDX];
	dpu_check_and_return(!dptx, -EINVAL, err, "[DP] dptx is null pointer");

	if (channel_num > DPTX_CHANNEL_NUM_MAX || data_width > DPTX_DATA_WIDTH_MAX) {
		dpu_pr_err("[DP] input param is invalid. channel_num=%d data_width=%d", channel_num, data_width);
		return -EINVAL;
	}
	aparams = &dptx->aparams;

	dpu_pr_info("[DP] set aparam. channel_num=%d data_width=%d sample_rate=%d",
		channel_num, data_width, sample_rate);

	switch (sample_rate) {
	case 32000:
		orig_sample_freq = IEC_ORIG_SAMP_FREQ_32K;
		sample_freq = IEC_SAMP_FREQ_32K;
		break;
	case 44100:
		orig_sample_freq = IEC_ORIG_SAMP_FREQ_44K;
		sample_freq = IEC_SAMP_FREQ_44K;
		break;
	case 48000:
		orig_sample_freq = IEC_ORIG_SAMP_FREQ_48K;
		sample_freq = IEC_SAMP_FREQ_48K;
		break;
	case 88200:
		orig_sample_freq = IEC_ORIG_SAMP_FREQ_88K;
		sample_freq = IEC_SAMP_FREQ_88K;
		break;
	case 96000:
		orig_sample_freq = IEC_ORIG_SAMP_FREQ_96K;
		sample_freq = IEC_SAMP_FREQ_96K;
		break;
	case 176400:
		orig_sample_freq = IEC_ORIG_SAMP_FREQ_176K;
		sample_freq = IEC_SAMP_FREQ_176K;
		break;
	case 192000:
		orig_sample_freq = IEC_ORIG_SAMP_FREQ_192K;
		sample_freq = IEC_SAMP_FREQ_192K;
		break;
	default:
		dpu_pr_info("[DP] invalid sample_rate");
		return -EINVAL;
	}

	aparams->iec_samp_freq = sample_freq;
	aparams->iec_orig_samp_freq = orig_sample_freq;
	aparams->num_channels = (uint8_t)channel_num;
	aparams->data_width = (uint8_t)data_width;

	mutex_lock(&dptx->dptx_mutex);
	if (dptx->power_saving_mode || pinfo->base.connect_status == DP_DISCONNECTED) {
		dpu_pr_info("[DP] dptx has already off, not set aparam");
	} else {
		if (dptx->dptx_audio_config)
			dptx->dptx_audio_config(dptx);
		dpu_pr_info("[DP] set aparam success");
	}
	mutex_unlock(&dptx->dptx_mutex);

	return 0;
}


int dptx_phy_rate_to_bw(uint8_t rate)
{
	switch (rate) {
	case DPTX_PHYIF_CTRL_RATE_RBR:
		return DP_LINK_BW_1_62;
	case DPTX_PHYIF_CTRL_RATE_HBR:
		return DP_LINK_BW_2_7;
	case DPTX_PHYIF_CTRL_RATE_HBR2:
		return DP_LINK_BW_5_4;
	case DPTX_PHYIF_CTRL_RATE_HBR3:
		return DP_LINK_BW_8_1;
	case DPTX_PHYIF_CTRL_EDP_RATE_R216:
		return DP_LINK_BW_2_16;
	case DPTX_PHYIF_CTRL_EDP_RATE_R243:
		return DP_LINK_BW_2_43;
	case DPTX_PHYIF_CTRL_EDP_RATE_R324:
		return DP_LINK_BW_3_24;
	case DPTX_PHYIF_CTRL_EDP_RATE_R432:
		return DP_LINK_BW_4_32;
	default:
		dpu_pr_err("[DP] Invalid rate 0x%x", rate);
		return -EINVAL;
	}
}

int dptx_bw_to_phy_rate(uint8_t bw)
{
	switch (bw) {
	case DP_LINK_BW_1_62:
		return DPTX_PHYIF_CTRL_RATE_RBR;
	case DP_LINK_BW_2_7:
		return DPTX_PHYIF_CTRL_RATE_HBR;
	case DP_LINK_BW_5_4:
		return DPTX_PHYIF_CTRL_RATE_HBR2;
	case DP_LINK_BW_8_1:
		return DPTX_PHYIF_CTRL_RATE_HBR3;
	case DP_LINK_BW_2_16:
		return DPTX_PHYIF_CTRL_EDP_RATE_R216;
	case DP_LINK_BW_2_43:
		return DPTX_PHYIF_CTRL_EDP_RATE_R243;
	case DP_LINK_BW_3_24:
		return DPTX_PHYIF_CTRL_EDP_RATE_R324;
	case DP_LINK_BW_4_32:
		return DPTX_PHYIF_CTRL_EDP_RATE_R432;
	default:
		dpu_pr_err("[DP] Invalid bw 0x%x", bw);
		return -EINVAL;
	}
}

struct dp_ctrl* dpu_get_dptx_by_portid(int port_id)
{
	int ret = 0;
	struct dp_ctrl *dptx = NULL;
	struct dp_private *dp_priv = NULL;
	struct dkmd_connector_info *pinfo = NULL;
	uint32_t dev_index = GFX_DP; // dev_index: 0 1 2

	if (dp_ctrl_is_bypass_by_pg((uint32_t)port_id)) {
		dpu_pr_err("[DP] port_id: %d. edp is unavailable by pg, so power on bypass", port_id);
		return NULL;
	}

	ret = get_dp_devive_index(port_id, &dev_index);
	if (ret == -1) {
		dpu_pr_err("[DP] get_dp_devive_index: dev_index get error\n");
		return NULL;
	}
	dpu_pr_info("[DP] get_dp_devive_index done: dev_index is %u\n", dev_index);

	dpu_check_and_return(!g_dkmd_dp_devive[dev_index], NULL, err, "[DP] dp device is null pointer!");
	pinfo = platform_get_drvdata(g_dkmd_dp_devive[dev_index]);

	dpu_check_and_return(!pinfo, NULL, err, "[DP] get connector info failed!");
	dp_priv = to_dp_private(pinfo);

	dptx = &dp_priv->dp[0];
	return dptx;
}

/*
 * audio/video Parameters Reset
 */
void dptx_audio_params_reset(struct audio_params *params)
{
	dpu_check_and_no_retval((params == NULL), err, "[DP] null pointer!");

	memset(params, 0x0, sizeof(struct audio_params));
	params->iec_channel_numcl0 = 8;
	params->iec_channel_numcr0 = 4;
	params->use_lut = 1;
	params->iec_samp_freq = 3;
	params->iec_word_length = 11;
	params->iec_orig_samp_freq = 12;
	params->data_width = 16;
	params->num_channels = 2;
	params->inf_type = 1;
	params->ats_ver = 17;
	params->mute = 0;
}

void dptx_video_params_reset(struct video_params *params)
{
	dpu_check_and_no_retval((params == NULL), err, "[DP] null pointer!");

	memset(params, 0x0, sizeof(struct video_params));

	/* 6 bpc should be default - use 8 bpc for MST calculation */
	params->bpc = COLOR_DEPTH_8;
	params->dp_dsc_info.dsc_info.dsc_bpp = 8; /* DPTX_BITS_PER_PIXEL */
	params->pix_enc = RGB;
	params->mode = 1;
	params->colorimetry = ITU601;
	params->dynamic_range = CEA;
	params->aver_bytes_per_tu = 30;
	params->aver_bytes_per_tu_frac = 0;
	params->init_threshold = 15;
	params->pattern_mode = RAMP;
	params->refresh_rate = 60000;
	params->video_format = VCEA;
}

bool dptx_check_low_temperature(struct dp_ctrl *dptx)
{
	return dpu_dvfs_check_low_temperature();
}

static int dp_ceil(uint64_t a, uint64_t b)
{
	if (b == 0)
		return -1;

	if (a % b != 0)
		return (int)(a / b + 1);

	return (int)(a / b);
}

void dp_send_cable_notification(struct dp_ctrl *dptx, int val)
{
	int state = 0;
	struct dtd *mdtd = NULL;
	struct video_params *vparams = NULL;

	dpu_check_and_no_retval(!dptx, err, "[DP] dptx is NULL!");

	/* just send tx0 plugin sigal to hal */
	if (dptx->id != MASTER_DPTX_IDX)
		return;

	state = dptx->sdev.state;
	switch_set_state(&dptx->sdev, val);
	if (dptx->video_transfer_enable && val == HOT_PLUG_IN &&
		!(dptx->current_link_rate == dptx->link.rate && dptx->current_link_lanes == dptx->link.lanes)) {
		dpu_pr_warn("[DP] pre rate: %d, pre lanes: %d, cur rate: %d, cur lanes: %d",
			dptx->current_link_rate, dptx->current_link_lanes, dptx->link.rate, dptx->link.lanes);
		dpu_pr_info("[DP] link training lane or rate changed, report plugout and then plugin to hwc");
		switch_set_state(&dptx->sdev, HOT_PLUG_OUT);
		mdelay(100);
		switch_set_state(&dptx->sdev, HOT_PLUG_IN);
	}

	if (dptx->edid_info.audio.basic_audio == 0x1) {
		if (val == HOT_PLUG_OUT || val == HOT_PLUG_OUT_VR)
			switch_set_state(&dptx->dp_switch, 0);
		else if (val == HOT_PLUG_IN || val == HOT_PLUG_IN_VR)
			switch_set_state(&dptx->dp_switch, 1);
	} else {
		dpu_pr_info("[DP] basic_audio(%ud) no support!", dptx->edid_info.audio.basic_audio);
		dpu_pr_info("[DP] dp_switch state: %d, val: %d", dptx->dp_switch.state, val);
		if (dptx->dp_switch.state == 1 && (val == HOT_PLUG_OUT || val == HOT_PLUG_OUT_VR)) {
			switch_set_state(&dptx->dp_switch, 0);
			dpu_pr_info("[DP] audio cable state switched to 0");
		}
	}

	vparams = &(dptx->vparams);
	mdtd = &(dptx->vparams.mdtd);
	dp_update_external_display_timming_info(mdtd->h_active, mdtd->v_active, vparams->m_fps);

	dpu_pr_info("[DP] cable state %s %d",
		dptx->sdev.state == state ? "is same" : "switched to", dptx->sdev.state);
}

int dpu_dptx_switch_source(uint32_t user_mode, uint32_t user_format)
{
	int ret = 0;
	struct dp_private *dp_priv = NULL;
	struct dkmd_connector_info *pinfo = NULL;
	struct dkmd_object_info *upload_pinfo = NULL;
	struct dp_ctrl *dptx = NULL;
	bool pre_source;

	dpu_check_and_return(!g_dkmd_dp_devive, -EINVAL, err, "[DP] dp device is null pointer!");
	pinfo = platform_get_drvdata(g_dkmd_dp_devive[GFX_DP]);
	dpu_check_and_return(!pinfo, -EINVAL, err, "[DP] get connector info failed!");
	upload_pinfo = pinfo->base.comp_obj_info;
	dp_priv = to_dp_private(pinfo);
	dpu_check_and_return(!dp_priv, -EINVAL, err, "[DP] dp_priv is null pointer\n");
	dptx = &dp_priv->dp[MASTER_DPTX_IDX];
	dpu_check_and_return(!dptx, -EINVAL, err, "[DP] dptx is null pointer");

	dpu_pr_info("[DP] switch source starts");

	mutex_lock(&dptx->dptx_mutex);

	if (!dptx->dptx_enable) {
		dpu_pr_err("[DP] dptx has already off\n");
		ret = -EINVAL;
		goto fail;
	}

	if (!dptx->video_transfer_enable) {
		dpu_pr_err("[DP] dptx never transfer video\n");
		ret = -EINVAL;
		goto fail;
	}

	if ((dptx->same_source == dpu_dp_get_current_dp_source_mode()) && (!user_mode) && (!user_format)) {
		dpu_pr_err("[DP] dptx don't switch source when the dest mode is same as current!!!\n");
		ret = -EINVAL;
		goto fail;
	}

	pre_source = dptx->same_source;
	dptx->user_mode = user_mode;
	dptx->user_mode_format = (enum video_format_type) user_format;
	dptx->same_source = dpu_dp_get_current_dp_source_mode();
	dpu_pr_info("[DP] dptx user switch: mode %d; format %d; pre_source %d; same_source %d.\n",
		dptx->user_mode, dptx->user_mode_format, pre_source, dptx->same_source);

	/* dptx user switch PC mode:1080P */
	if ((dptx->user_mode != 0) || (pre_source != dptx->same_source)) {
		/* DP plug out */
		dpu_pr_info("[DP] dptx user switch source,if PC mode:1080P");
		mutex_lock(pinfo->base.pluggable_connect_mutex);
		upload_pinfo->connect_status = pinfo->base.connect_status = DP_DISCONNECTED;
		mutex_unlock(pinfo->base.pluggable_connect_mutex);
		if (dptx->handle_hotunplug)
			dptx->handle_hotunplug(dptx);
		mutex_unlock(&dptx->dptx_mutex);

		msleep(1000);

		mutex_lock(&dptx->dptx_mutex);
		if (dptx->dp_dis_reset)
			dptx->dp_dis_reset(dptx, true);

		if (dptx->dptx_core_on)
			dptx->dptx_core_on(dptx);

		/* connect_status need update before dp set uevent to hwc */
		mutex_lock(pinfo->base.pluggable_connect_mutex);
		upload_pinfo->connect_status = pinfo->base.connect_status = DP_CONNECTED;
		mutex_unlock(pinfo->base.pluggable_connect_mutex);
		if (dptx->handle_hotplug)
			ret = dptx->handle_hotplug(dptx);

		dp_imonitor_set_param(DP_PARAM_HOTPLUG_RETVAL, &ret);
	}
fail:
	mutex_unlock(&dptx->dptx_mutex);

	dpu_pr_info("[DP] switch source ends");

	return ret;
}

#ifdef CONFIG_DKMD_DEBUG_ENABLE
EXPORT_SYMBOL(dpu_dptx_set_aparam);
EXPORT_SYMBOL_GPL(dpu_dptx_switch_source);
#endif

struct ppll7_cfg_param {
	uint64_t refdiv;
	uint64_t fbdiv;
	uint64_t frac;
	uint64_t postdiv1;
	uint64_t postdiv2;
	int post_div;
};

static uint64_t dp_pixel_ppll7_param_calc(uint64_t pixel_clock_cur, struct ppll7_cfg_param *cfg)
{
	int i, ceil_temp;
	uint64_t ppll7_freq_divider, vco_freq_output;
	int freq_divider_list[22] = {
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		12, 14, 15, 16, 20, 21, 24,
		25, 30, 36, 42, 49
	};
	int postdiv1_list[22] = {
		1, 2, 3, 4, 5, 6, 7, 4, 3, 5, 4,
		7, 5, 4, 5, 7, 6, 5, 6, 6, 7, 7
	};
	int postdiv2_list[22] = {
		1, 1, 1, 1, 1, 1, 1, 2, 3, 2, 3,
		2, 3, 4, 4, 3, 4, 5, 5, 6, 6, 7
	};

	/* Fractional PLL can not output the so small clock */
	cfg->post_div = 1;
	if (pixel_clock_cur * (uint64_t)freq_divider_list[21] < VCO_MIN_FREQ_OUPUT) {
		cfg->post_div = 2;
		pixel_clock_cur *= 2; /* multiple frequency */
	}

	ceil_temp = dp_ceil(VCO_MIN_FREQ_OUPUT, pixel_clock_cur);
	if (ceil_temp < 0)
		return pixel_clock_cur;

	ppll7_freq_divider = (uint64_t)ceil_temp;
	for (i = 0; i < 22; i++) {
		if (freq_divider_list[i] >= (int)ppll7_freq_divider) {
			ppll7_freq_divider = (uint64_t)freq_divider_list[i];
			cfg->postdiv1 = (uint64_t)postdiv1_list[i] - 1;
			cfg->postdiv2 = (uint64_t)postdiv2_list[i] - 1;
			dpu_pr_info("[DP] postdiv1=0x%llx, postdiv2=0x%llx\n", cfg->postdiv1, cfg->postdiv2);
			break;
		}
	}

	vco_freq_output = ppll7_freq_divider * pixel_clock_cur;
	if (vco_freq_output == 0)
		return pixel_clock_cur;

	dpu_pr_info("[DP] vco_freq_output=%llu\n", vco_freq_output);
	ceil_temp = dp_ceil(400000000UL, vco_freq_output);
	if (ceil_temp < 0)
		return pixel_clock_cur;

	cfg->refdiv = ((vco_freq_output * (uint64_t)ceil_temp) >= 494000000UL) ? 1 : 2;
	dpu_pr_info("[DP] refdiv=0x%llx\n", cfg->refdiv);

	cfg->fbdiv = vco_freq_output * (uint64_t)ceil_temp * cfg->refdiv / SYS_FREQ;
	dpu_pr_info("[DP] fbdiv=0x%llx\n", cfg->fbdiv);

	cfg->frac = ((uint64_t)ceil_temp * vco_freq_output - SYS_FREQ / \
		cfg->refdiv * cfg->fbdiv) * cfg->refdiv * 0x1000000; /* 0x1000000 is 2^24 */
	cfg->frac = (uint64_t)cfg->frac / SYS_FREQ;

	dpu_pr_info("[DP] frac=0x%llx\n", cfg->frac);

	return pixel_clock_cur;
}

static void dp_pixel_ppll7_set_reg(char __iomem *peri_crg_base, struct ppll7_cfg_param *cfg)
{
	uint32_t ppll7ctrl0, ppll7ctrl1;
	uint32_t ppll7ctrl0_val, ppll7ctrl1_val;

	ppll7ctrl0 = inp32(peri_crg_base + MIDIA_PPLL7_CTRL0);
	ppll7ctrl0 &= ~MIDIA_PPLL7_FREQ_DEVIDER_MASK;

	ppll7ctrl0_val = 0x0;
	ppll7ctrl0_val |= (uint32_t)((cfg->postdiv2 << 23) | (cfg->postdiv1 << 20) |
		(cfg->fbdiv << 8) | (cfg->refdiv << 2));
	ppll7ctrl0_val &= MIDIA_PPLL7_FREQ_DEVIDER_MASK;

	ppll7ctrl0 |= ppll7ctrl0_val;
	outp32(peri_crg_base + MIDIA_PPLL7_CTRL0, ppll7ctrl0);

	ppll7ctrl1 = inp32(peri_crg_base + MIDIA_PPLL7_CTRL1);
	ppll7ctrl1 &= ~MIDIA_PPLL7_FRAC_MODE_MASK;

	ppll7ctrl1_val = 0x0;
	ppll7ctrl1_val |= (uint32_t)(1 << 25 | 0 << 24 | cfg->frac);
	ppll7ctrl1_val &= MIDIA_PPLL7_FRAC_MODE_MASK;

	ppll7ctrl1 |= ppll7ctrl1_val;
	outp32(peri_crg_base + MIDIA_PPLL7_CTRL1, ppll7ctrl1);
}

static int dp_pxl_ppll7_init(struct dp_ctrl *dptx, struct dpu_connector *connector, uint64_t pixel_clock)
{
	int ret = 0;
	uint64_t pixel_clock_cur;
	struct ppll7_cfg_param cfg;

	dpu_check_and_return(!connector || !pixel_clock, -EINVAL, err, "[DP] input err!\n");

	pixel_clock_cur = dp_pixel_ppll7_param_calc(pixel_clock, &cfg);
	dp_pixel_ppll7_set_reg(connector->peri_crg_base, &cfg);

	/* need vote voltage */
	if (cfg.post_div == 0) {
		dpu_pr_err("[DP] Illegal clk parameter");
		return -EINVAL;
	}
	ret = clk_set_rate(dptx->clk_dpctrl_pixel, DEFAULT_MIDIA_PPLL7_CLOCK_FREQ / cfg.post_div);
	if (ret < 0)
		dpu_pr_err("[DP] %s clk_dpctrl_pixel clk_set_rate(%llu) failed, error=%d!\n",
			connector->conn_info->base.name, pixel_clock_cur, ret);

	dpu_pr_info("[DP] clk_dpctrl_pixel:ori[%llu]->[%llu]->[%llu]\n",
		pixel_clock, pixel_clock_cur, (uint64_t)clk_get_rate(dptx->clk_dpctrl_pixel));

	return ret;
}

int dptx_dss_plugin(struct dp_ctrl *dptx, enum dptx_hot_plug_type etype)
{
	int ret = 0;
	struct dp_private *dp_priv = NULL;

	dpu_check_and_return(!dptx, -EINVAL, err, "[DP] dptx is null pointer\n");
	dp_priv = to_dp_private(dptx->connector->conn_info);
	dpu_check_and_return(!dp_priv, -EINVAL, err, "[DP] dp_priv is null pointer\n");

	/* config pll */
	if (dptx->is_all_pll_config_by_dp_soft)
		ret = dptx_pixel_pll_init(dptx,  dptx->connector->ldi.pxl_clk_rate);
	else
		ret = dp_pxl_ppll7_init(dptx, dptx->connector, dptx->connector->ldi.pxl_clk_rate);

	/* for dp test, would be deleted */
	dpu_pr_info("[DP] enable dp colorbar!");
	dptx_writel(dptx, dptx_dp_color_bar_stream(0), DPTX_COLOR_BAR_STREAM_EN);

	dp_send_cable_notification(dptx, etype);

	dptx->video_transfer_enable = true;

	return ret;
}

static int dp_ctrl_on(struct dkmd_connector_info *pinfo)
{
	struct dp_private *dp_priv = to_dp_private(pinfo);
	struct dp_ctrl *dptx = NULL;
	struct dpu_connector *connector = NULL;
	int connector_cnt = 1;
	int i = 0;

	dpu_check_and_return(!dp_priv, -EINVAL, err, "[DP] dp_priv is null pointer\n");
	dpu_pr_info("[DP] hsdt1 ctrl set PLL r&w mode");
	dptx_pixel_pll_preinit();

	connector = get_primary_connector(pinfo);
	if (connector->bind_connector)
		connector_cnt = 2;

	connector->enable_ser_vp_sync = true;
	for (i = 0; (i < connector_cnt) && (i < MAX_DPTX_COUNT); i++) {
		dptx = &dp_priv->dp[i];

		dpu_pr_info("[DP] dp%d (portid:%d) enter!\n", i, dptx->port_id);
		mutex_lock(&dptx->dptx_mutex);
		if (dptx->dptx_enable && dptx->power_saving_mode && dptx->dptx_power_handler) {
			dpu_pr_info("[DP] dptx has already blank, power on for unblank!");
			if (!dptx->dptx_power_handler(dptx, true))
				dptx->power_saving_mode = false;
			else
				dpu_pr_warn("[DP] dptx_power_handler failed");
		}
		mutex_unlock(&dptx->dptx_mutex);

		dpu_pr_info("[DP] dp%d (portid:%d) exit!\n", i, dptx->port_id);
	}

	return 0;
}

static int dp_ctrl_off(struct dkmd_connector_info *pinfo)
{
	struct dp_private *dp_priv = to_dp_private(pinfo);
	struct dp_ctrl *dptx = NULL;
	struct dpu_connector *connector = NULL;
	int connector_cnt = 1;
	int i = 0;

	dpu_check_and_return(!dp_priv, -EINVAL, err, "[DP] dp_priv is null pointer\n");
	connector = get_primary_connector(pinfo);
	if (connector->bind_connector)
		connector_cnt = 2;

	for (i = 0; (i < connector_cnt) && (i < MAX_DPTX_COUNT); i++) {
		dptx = &dp_priv->dp[i];

		dpu_pr_info("[DP] dp%d (portid:%d) enter!\n", i, dptx->port_id);
		mutex_lock(&dptx->dptx_mutex);
		if (dptx->dptx_enable && !dptx->power_saving_mode && dptx->video_transfer_enable) {
			dpu_pr_info("[DP] dptx %d has already connected, power off for blank!", dptx->port_id);
			if (dptx->dptx_power_handler)
				dptx->dptx_power_handler(dptx, false);

			/* reset dprx of 983 */
			if (dptx->dp_info && dptx->dp_info->is_dprx_ready && dptx->dp_info->dp_connect_ctrl_reset_dprx != NULL) {
				dpu_pr_info("[DP] dp_connect_ctrl_reset_dprx entry");
				dptx->dp_info->dp_connect_ctrl_reset_dprx(dptx->port_id);
			}

			dptx->power_saving_mode = true;
		}
		mutex_unlock(&dptx->dptx_mutex);

		/* set dp power saving mode */
		if (dptx->dptx_combophy_power_saving_handler)
			dptx->dptx_combophy_power_saving_handler(dptx);

		dpu_pr_info("[DP] dp%d (portid:%d) exit!\n", i, dptx->port_id);
	}
	connector->enable_ser_vp_sync = false;

	return 0;
}

static void dpu_dp_timing_isr_handle(struct dp_ctrl *dptx, struct dkmd_isr *isr_ctrl, uint32_t timing_gen_irq_status)
{
	uint32_t val = 0;
	int retval;

	dpu_check_and_no_retval(!dptx, err, "[DP] NULL Pointer\n");
	if (timing_gen_irq_status & DPTX_IRQ_VSYNC) {
		dpu_pr_debug("[DP] DPTX_IRQ_VSYNC received");
		dkmd_isr_notify_listener(isr_ctrl, DSI_INT_VSYNC);
	}

	if ((timing_gen_irq_status & DPTX_IRQ_STREAM0_UNDERFLOW) && !dptx->dptx_underflow_clear) {
		dpu_pr_warn("[DP] [DP-IRQ] DPTX_IRQ_STREAM0_UNDERFLOW");
		dptx->dptx_underflow_clear = true;
		val = dptx_readl(dptx, DPTX_INTR_ACPU_TIMING_GEN_ENABLE);
		val &= ~DPTX_IRQ_STREAM0_UNDERFLOW;
		dptx_writel(dptx, DPTX_INTR_ACPU_TIMING_GEN_ENABLE, val);
		dptx_global_intr_clear(dptx);
		dkmd_isr_notify_listener(isr_ctrl, DSI_INT_UNDER_FLOW);
	}

	if (timing_gen_irq_status & DPTX_PSR_ACTIVE_SDP_INTR)
		dpu_pr_debug("[DP] TX PSR active SDP");

	if (timing_gen_irq_status & DPTX_PSR_INACTIVE_SDP_INTR)
		dpu_pr_debug("[DP] TX PSR inactive SDP");

	if (timing_gen_irq_status & DPTX_PSR_UPDATE_INTR)
		dpu_pr_debug("[DP] TX PSR update");

	if (timing_gen_irq_status & DPTX_PSR_ENTRY_INTR) {
		if (dptx->psr_params.ml_close_require && dptx->psr_params.psr_version == DP_PSR_IS_SUPPORTED) {
			retval = dptx_psr_ml_config(dptx, LANE_POWER_MODE_P3);
			if (retval)
				dpu_pr_err("[DP] Config mainlink status P3 fail");
		}
		dpu_pr_info("[DP] TX PSR entry");
	}

	if (timing_gen_irq_status & DPTX_PSR_EXIT_INTR)
		dpu_pr_info("[DP] TX PSR exit");
}

static void dpu_dp_intr_handle(struct dp_ctrl *dptx, struct dkmd_isr *isr_ctrl, uint32_t intr_status)
{
	uint32_t val = 0;

	dpu_check_and_no_retval(!dptx, err, "[DP] NULL Pointer\n");
	if (intr_status & DPTX_IRQ_STREAM0_FRAME) {
		dpu_pr_debug("[DP] DPTX_IRQ_STREAM0_FRAME irq received");
		val = dptx_readl(dptx, DPTX_INTR_ACPU_TIMING_GEN_ENABLE);
		val |= DPTX_IRQ_STREAM0_UNDERFLOW;
		dptx_writel(dptx, DPTX_INTR_ACPU_TIMING_GEN_ENABLE, val);
		dkmd_isr_notify_listener(isr_ctrl, DSI_INT_VACT0_START);
	}

	if (intr_status & DPTX_IRQ_HPD_SHORT) {
		dpu_pr_info("[DP-IRQ] DPTX_IRQ_HPD_SHORT\n");
		if (dptx->dptx_hpd_irq_handler)
			dptx->dptx_hpd_irq_handler(dptx);
	}

	if (intr_status & DPTX_SU_UPDATE)
		dpu_pr_debug("[DP] TX PSR2 SU update");

	if (intr_status & DPTX_PSR2_VACTIVE_START)
		dpu_pr_debug("[DP] TX PSR2 vactive start");
}

static irqreturn_t dpu_dp_isr(int32_t irq, void *ptr)
{
	uint32_t timing_gen_irq_status, intr_status, sdp_intr_status;
	struct dpu_connector *connector = NULL;
	struct dkmd_isr *isr_ctrl = (struct dkmd_isr *)ptr;
	struct dp_private *dp_priv = NULL;
	struct dp_ctrl *dptx = NULL;

	dpu_check_and_return(!isr_ctrl, IRQ_NONE, err, "[DP] isr_ctrl is null pointer");
	connector = (struct dpu_connector *)isr_ctrl->parent;
	dp_priv = to_dp_private(connector->conn_info);
	dpu_check_and_return(!dp_priv, IRQ_NONE, err, "[DP] dp_priv is null pointer");
	dptx = &dp_priv->dp[MASTER_DPTX_IDX];

	timing_gen_irq_status = dptx_readl(dptx, DPTX_INTR_ACPU_TIMING_GEN_MASKED_STATUS) &
		dptx_readl(dptx, DPTX_INTR_ACPU_TIMING_GEN_ORIGINAL_STATUS);
	dptx_writel(dptx, DPTX_INTR_ACPU_TIMING_GEN_ORIGINAL_STATUS, timing_gen_irq_status);

	intr_status = dptx_readl(dptx, DPTX_INTR_MASKED_STATUS) & dptx_readl(dptx, DPTX_INTR_ORIGINAL_STATUS);
	dptx_writel(dptx, DPTX_INTR_ORIGINAL_STATUS, intr_status);

	sdp_intr_status = dptx_readl(dptx, DPTX_SDP_INTR_MASKED_STATUS) & dptx_readl(dptx, DPTX_SDP_INTR_ORIGINAL_STATUS);
	dptx_writel(dptx, DPTX_SDP_INTR_ORIGINAL_STATUS, sdp_intr_status);

	if (timing_gen_irq_status) {
		if (dptx->dpu_dp_timing_isr_handle)
			dptx->dpu_dp_timing_isr_handle(dptx, isr_ctrl, timing_gen_irq_status);
		else
			dpu_dp_timing_isr_handle(dptx, isr_ctrl, timing_gen_irq_status);
	}

	if (intr_status) {
		if (dptx->dpu_dp_intr_handle)
			dptx->dpu_dp_intr_handle(dptx, isr_ctrl, intr_status);
		else
			dpu_dp_intr_handle(dptx, isr_ctrl, intr_status);
	}

	return IRQ_HANDLED;
}

static void dp_ctrl_isr_handle(struct dpu_connector *connector, struct dkmd_isr *isr_ctrl, bool enable)
{
	struct dp_private *dp_priv = to_dp_private(connector->conn_info);
	struct dp_ctrl *dptx = &dp_priv->dp[MASTER_DPTX_IDX];

	mutex_lock(&dptx->dptx_mutex);
	if (!dptx->video_transfer_enable) {
		dpu_pr_info("[DP] dptx has already off, id: %d\n", dptx->port_id);
		isr_ctrl->handle_func(isr_ctrl, DKMD_ISR_DISABLE);
		mutex_unlock(&dptx->dptx_mutex);
		return;
	}
	/* 1. interrupt mask */
	dptx_global_intr_dis(dptx);
	if (enable) {
		/* 2. enable irq */
		isr_ctrl->handle_func(isr_ctrl, DKMD_ISR_ENABLE);

		/* 3. interrupt clear */
		dptx_global_intr_clear(dptx);

		/* 4. interrupt umask, enable all top-level interrupts */
		dptx_global_intr_en(dptx);
	} else {
		/* 2. disable irq */
		isr_ctrl->handle_func(isr_ctrl, DKMD_ISR_DISABLE);

		/* 3. interrupt clear */
		dptx_global_intr_clear(dptx);
	}
	mutex_unlock(&dptx->dptx_mutex);
}

static int32_t dp_ctrl_isr_enable(struct dpu_connector *connector, const void *value)
{
	struct dkmd_isr *isr_ctrl = (struct dkmd_isr *)value;

	dp_ctrl_isr_handle(connector, isr_ctrl, true);

	return 0;
}

static int32_t dp_ctrl_isr_disable(struct dpu_connector *connector, const void *value)
{
	struct dkmd_isr *isr_ctrl = (struct dkmd_isr *)value;

	dp_ctrl_isr_handle(connector, isr_ctrl, false);

	return 0;
}

static int32_t dp_ctrl_isr_setup(struct dpu_connector *connector, const void *value)
{
	struct dkmd_isr *isr_ctrl = (struct dkmd_isr *)value;

	if (unlikely((!connector) || (!value))) {
		dpu_pr_err("[DP] connector or value is null\n");
		return -1;
	}

	(void)snprintf(isr_ctrl->irq_name, sizeof(isr_ctrl->irq_name), "irq_dp_%u", connector->connector_id);
	isr_ctrl->irq_no = connector->connector_irq;
	isr_ctrl->isr_fnc = dpu_dp_isr;
	isr_ctrl->parent = connector;
	/* fake int mask for dptx */
	isr_ctrl->unmask = ~(DSI_INT_VSYNC | DSI_INT_VACT0_START | DSI_INT_UNDER_FLOW);

	return 0;
}

static void dptx_enable_dpu_pipeline(struct dp_ctrl *dptx)
{
	uint32_t reg;
	uint32_t val;
	struct dpu_connector *pinfo = NULL;
	uint32_t rx2tx_delay = 0x10;

	dpu_check_and_no_retval((dptx == NULL), err, "[DP] NULL Pointer\n");
	pinfo = dptx->connector;

	if (dptx->source_is_external) {
		reg = dptx_readl(dptx, DPTX_GCTL0);
		reg &= ~DPTX_CFG_TIMING_GEN_MODE_SEL;
		if (dptx->arsr_enable) {
			reg |= DPTX_HSYNC_VSYNC_FOLLOW_MODE << DPTX_CFG_TIMING_GEN_MODE_SEL_OFFSET;
		} else {
			reg |= DPTX_HSYNC_VSYNC_HSYNC_FOLLOW_MODE << DPTX_CFG_TIMING_GEN_MODE_SEL_OFFSET;
		}
		dptx_writel(dptx, DPTX_GCTL0, reg);

		reg = 0;
		reg |= (rx2tx_delay + FRM_START_LINE_NUM_VAL) << DPTX_CFG_FRM_START_DELAY_OFFSET;
		reg |= rx2tx_delay << DPTX_CFG_RX2TX_DELAY_OFFSET;
		val = (pinfo->conn_info->base.xres + pinfo->ldi.h_back_porch + \
			pinfo->ldi.h_front_porch + pinfo->ldi.h_pulse_width) / 2;
		val = val / 2 * 2;
		reg |= val << DPTX_CFG_PIXEL_NUM_TIMING_MODE_SEL1_OFFSET;
		dptx_writel(dptx, DPTX_TIMING_MODE1_CTRL, reg);

		reg = 0;
		val = pinfo->ldi.v_back_porch + pinfo->ldi.v_front_porch + pinfo->ldi.v_pulse_width - rx2tx_delay;
		reg |= (val << DPTX_TIMING_GEN_VFP_SHIFT);
		dptx_writel(dptx, dptx_timing_gen_config3_stream(0), reg);
	}
}

static void enable_dptx_ldi(struct dkmd_connector_info *pinfo)
{
	struct dpu_connector *connector = get_primary_connector(pinfo);
	struct dp_private *dp_priv = to_dp_private(connector->conn_info);
	int connector_cnt = 1;
	int i = 0;

	if (connector->bind_connector)
		connector_cnt = 2;

	for (i = 0; (i < connector_cnt) && (i < MAX_DPTX_COUNT); i++) {
		struct dp_ctrl *dptx = &dp_priv->dp[i];
		if (unlikely(!dptx)) {
			dpu_pr_warn("dptx is null");
			continue;
		}

		if (!dptx->power_fake_saving_mode) {
			dptx_enable_dpu_pipeline(dptx);
			enable_dptx_timing_gen(dptx);
		}
	}
}

static void disable_dptx_timing_gen(struct dkmd_connector_info *pinfo)
{
	struct dpu_connector *connector = get_primary_connector(pinfo);
	struct dp_private *dp_priv = to_dp_private(connector->conn_info);
	int connector_cnt = 1;
	int i = 0;

	if (connector->bind_connector)
		connector_cnt = 2;

	for (i = 0; (i < connector_cnt) && (i < MAX_DPTX_COUNT); i++) {
		struct dp_ctrl *dptx = &dp_priv->dp[i];
		mutex_lock(&dptx->dptx_mutex);
		if (dptx->power_saving_mode || !dptx->video_transfer_enable) {
			dpu_pr_warn("[DP] DP is off, cannot config media transfer");
			mutex_unlock(&dptx->dptx_mutex);
			continue;
		}
		if (dptx->dptx_triger_media_transfer)
			dptx->dptx_triger_media_transfer(dptx, false);
		mutex_unlock(&dptx->dptx_mutex);
	}
}

static int32_t dp_ctrl_dptx_reset(struct dpu_connector *connector, const void *value)
{
	struct dp_private *dp_priv = to_dp_private(connector->conn_info);
	int connector_cnt = 1;
	int i = 0;
	if (connector->bind_connector)
		connector_cnt = 2;

	for (i = 0; (i < connector_cnt) && (i < MAX_DPTX_COUNT); i++) {
		struct dp_ctrl *dptx = &dp_priv->dp[i];

		dpu_pr_info("[DP] dp%d clear enter\n", i);

		mutex_lock(&dptx->dptx_mutex);
		if (!dptx->video_transfer_enable || dptx->power_saving_mode) {
			dpu_pr_info("[DP] dptx has already off");
			dptx->dptx_underflow_clear = false;
			mutex_unlock(&dptx->dptx_mutex);
			return 0;
		}
		msleep(100);
		dptx_enable_dpu_pipeline(dptx);
		enable_dptx_timing_gen(dptx);
		dptx->dptx_underflow_clear = false;
		mutex_unlock(&dptx->dptx_mutex);

		dpu_pr_info("[DP] dp%d clear exit\n", i);
	}

	return 0;
}

static int32_t dp_hdcp_increase_counter(struct dpu_connector *connector, const void *value)
{
	(void)connector;
	(void)value;
	hdcp_increase_counter();
	return 0;
}

static int32_t dp_hdcp_decrease_counter(struct dpu_connector *connector, const void *value)
{
	(void)connector;
	(void)value;
	hdcp_decrease_counter();
	return 0;
}

static struct connector_ops_handle_data dp_ops_table[] = {
	{ SETUP_ISR, dp_ctrl_isr_setup },
	{ ENABLE_ISR, dp_ctrl_isr_enable },
	{ DISABLE_ISR, dp_ctrl_isr_disable },
	{ DO_CLEAR, dp_ctrl_dptx_reset },
	{ SET_BACKLIGHT, dp_set_backlight},
	{ HDCP_INCREASE_COUNTER, dp_hdcp_increase_counter },
	{ HDCP_DECREASE_COUNTER, dp_hdcp_decrease_counter },
};

static int32_t dp_ctrl_ops_handle(struct dkmd_connector_info *pinfo, uint32_t ops_cmd_id, void *value)
{
	struct dpu_connector *connector = NULL;

	dpu_check_and_return(!pinfo, -EINVAL, err, "[DP] pinfo is NULL!");
	connector = get_primary_connector(pinfo);
	dpu_check_and_return(!connector, -EINVAL, err, "[DP] connector is NULL!");

	return dkdm_connector_hanlde_func(dp_ops_table, ARRAY_SIZE(dp_ops_table), ops_cmd_id, connector, value);
}

static void dp_ctrl_ldi_setup(struct dpu_connector *connector, uint32_t dp_mode)
{
	struct dkmd_connector_info *pinfo = connector->conn_info;

	/* set default ldi parameter */
	pinfo->base.width = 16000;
	pinfo->base.height = 9000;
	pinfo->base.fps = 60;
	pinfo->base.is_pluggable = dp_mode == EDP_MODE ? 0 : 1;
	pinfo->ifbc_type = IFBC_TYPE_NONE;
	if (pinfo->base.fpga_flag == 1) {
		pinfo->base.xres = 1280;
		pinfo->base.yres = 720;

		connector->ldi.h_back_porch = 220;
		connector->ldi.h_front_porch = 110;
		connector->ldi.h_pulse_width = 40;
		connector->ldi.hsync_plr = 1;

		connector->ldi.v_back_porch = 20;
		connector->ldi.v_front_porch = 30;
		connector->ldi.v_pulse_width = 5;
		connector->ldi.vsync_plr = 1;

		connector->ldi.pxl_clk_rate = 27000000UL;
		connector->ldi.pxl_clk_rate_div = 1;
	} else {
		pinfo->base.xres = 1920;
		pinfo->base.yres = 1080;

		connector->ldi.h_back_porch = 148;
		connector->ldi.h_front_porch = 88;
		connector->ldi.h_pulse_width = 44;
		connector->ldi.hsync_plr = 1;

		connector->ldi.v_back_porch = 36;
		connector->ldi.v_front_porch = 4;
		connector->ldi.v_pulse_width = 5;
		connector->ldi.vsync_plr = 1;

		connector->ldi.pxl_clk_rate = 148500000UL;
		connector->ldi.pxl_clk_rate_div = 1;
	}
}

static void dp_ctrl_clk_setup(struct dpu_connector *connector)
{
	int ret = 0;

	ret = clk_set_rate(connector->connector_clk[CLK_DPCTRL_16M], 16000000UL);
	if (ret < 0)
		dpu_pr_err("[DP] CLK_DPCTRL_16M clk_set_rate failed, error=%d!\n", ret);

	ret = clk_set_rate(connector->connector_clk[CLK_DPCTRL_PCLK], 288000000UL);
	if (ret < 0)
		dpu_pr_err("[DP] CLK_DPCTRL_PCLK clk_set_rate failed, error=%d!\n", ret);
}

static void dp_device_params_init(struct dpu_connector *connector, struct dp_ctrl *dptx)
{
	errno_t err_ret;

	mutex_init(&dptx->dptx_mutex);
	init_waitqueue_head(&dptx->dptxq);
	init_waitqueue_head(&dptx->waitq);
	atomic_set(&(dptx->sink_request), 0);
	atomic_set(&(dptx->shutdown), 0);
	atomic_set(&(dptx->c_connect), 0);

	dptx->dummy_dtds_present = false;
	dptx->selected_est_timing = NONE;
	dptx->dptx_vr = false;
	dptx->dptx_enable = false;
	dptx->video_transfer_enable = false;
	dptx->power_saving_mode = false;
	dptx->power_fake_saving_mode = false;
	dptx->dptx_detect_inited = false;
	dptx->user_mode = 0;
	dptx->detect_times = 0;
	dptx->dptx_plug_type = DP_PLUG_TYPE_NORMAL;
	dptx->user_mode_format = VCEA;
	dptx->dsc_decoders = DSC_DEFAULT_DECODER;
	dptx->dsc_ifbc_type = IFBC_TYPE_VESA3X_DUAL; /* IFBC_TYPE_VESA2X_DUAL for fpga */
	dptx->same_source = dpu_dp_get_current_dp_source_mode();
	dptx->max_edid_timing_hactive = 0;
	dptx->bstatus = 0;
	dptx->edid_try_count = MAX_AUX_RETRY_COUNT;
	dptx->edid_try_delay = AUX_RETRY_DELAY_TIME;

	dptx->base = connector->connector_base;
	dpu_pr_info("[DP] dp_device_params_init dp_mode = %d\n", dptx->connector->conn_info->base.mode);
	if (dptx->connector->conn_info->base.mode == EDP_MODE) {
		err_ret = memcpy_s(dptx->combophy_pree_swing, sizeof(dptx->combophy_pree_swing),
			connector->combophy_ctrl.edp_combophy_pree_swing,
			sizeof(connector->combophy_ctrl.edp_combophy_pree_swing));
		if (err_ret != EOK)
			dpu_pr_err("[DP] copy dptx phy parameters fail, ret = %d\n", err_ret);
	} else {
		err_ret = memcpy_s(dptx->combophy_pree_swing, sizeof(dptx->combophy_pree_swing),
			connector->combophy_ctrl.combophy_pree_swing, sizeof(connector->combophy_ctrl.combophy_pree_swing));
		if (err_ret != EOK)
			dpu_pr_err("[DP] copy dptx phy parameters fail, ret = %d\n", err_ret);
	}

	memcpy_s(dptx->combophy_ssc_ppm, sizeof(dptx->combophy_ssc_ppm),
		connector->combophy_ctrl.combophy_ssc_ppm, sizeof(dptx->combophy_ssc_ppm));
}

static int dp_device_buf_alloc(struct dp_ctrl *dptx)
{
	memset(&(dptx->edid_info), 0, sizeof(struct edid_information));

	dptx->edid = kzalloc(DPTX_DEFAULT_EDID_BUFLEN, GFP_KERNEL);
	if (dptx->edid == NULL) {
		dpu_pr_err("[DP] dptx base is NULL!\n");
		return -ENOMEM;
	}

	return 0;
}

static int dp_get_port_id_by_connector_id(uint32_t connector_id)
{
	int port_id = 0;

	switch (connector_id) {
	case CONNECTOR_ID_DP:
		port_id = 0;
		break;
	case CONNECTOR_ID_DP1:
		port_id = 1;
		break;
	case CONNECTOR_ID_DP2:
		port_id = 2;
		break;
	case CONNECTOR_ID_DP3:
		port_id = 3;
		break;
	case CONNECTOR_ID_EDP:
		port_id = 4;
		break;
	case CONNECTOR_ID_EDP1:
		port_id = 5;
		break;
	default:
		dpu_pr_err("[DP] connector_id %u is invalid!\n", connector_id);
		break;
	}
	dpu_pr_info("[DP] port_id is %d!\n", port_id);
	return port_id;
}

static void hdmi_audio_dev_name_init(struct dp_ctrl *dptx)
{
	dpu_check_and_no_retval(!dptx, err, "[DP] dptx is null pointer!");

	if (strcmp(dptx->sdev.name, DTS_GFX_DP_NAME) == 0)
		dptx->dp_switch.name = "hdmi_audio";
	else if (strcmp(dptx->sdev.name, DTS_GFX_DP1_NAME) == 0)
		dptx->dp_switch.name = "hdmi_audio1";
	else if (strcmp(dptx->sdev.name, DTS_GFX_DP2_NAME) == 0)
		dptx->dp_switch.name = "hdmi_audio2";
	else
		dptx->dp_switch.name = "hdmi_audio";

	dpu_pr_info("[DP] dptx->dp_switch.name = %s", dptx->dp_switch.name);
	return;
}

void dpu_dp_hook_register(struct dp_hook_ops *ops)
{
	g_dp_hook_ops = ops;
}

static irqreturn_t dptx_hpd_gpio_thread_irq(int32_t irq, void *ptr)
{
	uint32_t is_pluged = 0;
	struct dp_ctrl *dptx = NULL;
	struct dp_private *dp_priv = NULL;

	dpu_pr_info("[DP] +\n");
	dptx = (struct dp_ctrl *)ptr;
	dpu_check_and_return(!dptx, IRQ_NONE, err, "DPTX is null!");

	dp_priv = to_dp_private(dptx->connector->conn_info);
	dpu_check_and_return(!dp_priv, IRQ_NONE, err, "DPTX is null!");

	is_pluged = (uint32_t)gpio_get_value(dp_priv->hpd_gpio);
	dpu_pr_info("[DP] is_pluged %d\n", is_pluged);

	if (is_pluged == 1) {
		mdelay(100);
		is_pluged = (uint32_t)gpio_get_value(dp_priv->hpd_gpio);
		dpu_pr_info("[DP] mdelay is_pluged %d\n", is_pluged);
		if (is_pluged != 1) {
			dpu_pr_info("[DP] ignore the irq is_pluged %d\n", is_pluged);
			return IRQ_NONE;
		}
		dpu_pr_info("[DP] port %d hotplug in!\n", dptx->port_id);
		dpu_multi_dptx_hpd_trigger(TCA_IRQ_HPD_IN, TCPC_DP, DP_PLUG_TYPE_NORMAL, MASTER_DPTX_IDX, dptx->port_id);
	} else {
		dpu_pr_info("[DP] hotplug out!\n");
		dpu_multi_dptx_hpd_trigger(TCA_IRQ_HPD_OUT, TCPC_DP, DP_PLUG_TYPE_NORMAL, MASTER_DPTX_IDX, dptx->port_id);
	}
	return IRQ_NONE;
}

static int32_t dptx_hpd_gpio_setup(struct dp_ctrl *dptx)
{
	int32_t ret;
	int32_t irq_num;
	struct dp_private *dp_priv = NULL;

	mutex_init(&dptx->dptx_mutex);
	dp_priv = to_dp_private(dptx->connector->conn_info);
	dpu_check_and_return(!dp_priv, IRQ_NONE, err, "DPTX is null!");

	dpu_pr_info("[DP] hpd_gpio = %#x,+\n", dp_priv->hpd_gpio);

	ret = gpio_request(dp_priv->hpd_gpio, "hpd_gpio");
	if (ret) {
		dpu_pr_err("[DP] Fail[%d] request gpio:%d\n", ret, dp_priv->hpd_gpio);
		return ret;
	}

	ret = gpio_direction_input(dp_priv->hpd_gpio);
	if (ret < 0) {
		gpio_free(dp_priv->hpd_gpio);
		dpu_pr_err("[DP] Failed to set gpio direction\n");
		return ret;
	}

	irq_num = (int32_t)gpio_to_irq(dp_priv->hpd_gpio);
	if (irq_num < 0) {
		gpio_free(dp_priv->hpd_gpio);
		dpu_pr_err("[DP] Failed to get dp_cable_det_gpio irq\n");
		ret = -EINVAL;
		return ret;
	}

	ret = request_threaded_irq(dp_priv->hpd_irq_no, NULL, dptx_hpd_gpio_thread_irq,
			IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "dptx_hpd_gpio", (void *)dptx);
	if (ret) {
		gpio_free(dp_priv->hpd_gpio);
		dpu_pr_err("[DP] Failed to request press interupt handler!\n");
		return ret;
	}
	dp_priv->hpd_irq_no = (uint32_t)irq_num;
	dpu_pr_info("[DP] dp_cable_det_gpio irq %d\n", irq_num);
	/* default enable irq */
	disable_irq(dp_priv->hpd_irq_no);
	dpu_pr_info("[DP] -\n");
	return ret;
}

void dp_ctrl_default_setup(struct dpu_connector *connector)
{
	int ret = 0;
	struct dp_private *dp_priv = NULL;
	struct dp_ctrl *dptx = NULL;
	int connector_cnt = 1;
	struct dpu_connector *cnct = NULL;
	int i = 0;

	dp_priv = to_dp_private(connector->conn_info);

	dpu_check_and_no_retval(!dp_priv, err, "[DP] dp_priv is null pointer\n");
	if (connector->bind_connector) {
		connector_cnt = 2;
		dpu_pr_info("[DP] dual edp ctrl default config\n");
	}

	for (i = 0; (i < connector_cnt) && (i < MAX_DPTX_COUNT); i++) {
		dptx = &dp_priv->dp[i];
		if (i == 0) {
			cnct = connector;
			dptx->port_id = dp_get_port_id_by_connector_id(connector->connector_id);
			dptx->sdev.name = connector->conn_info->base.name;
			dpu_pr_info("[DP] dptx->sdev.name = %s", dptx->sdev.name);
			hdmi_audio_dev_name_init(dptx);
		} else {
			cnct = connector->bind_connector;
			dptx->port_id = dp_get_port_id_by_connector_id(connector->bind_connector->connector_id);
		}
		dpu_pr_info("[DP] dp_ctrl_default_setup: --%d-- port id = %d", i, dptx->port_id);

		dptx->connector = cnct;
		dptx->irq = (uint32_t)(cnct->connector_irq);
		cnct->connector_base = dptx->hidptx_base;
		dpu_pr_info("[DP] dp_ctrl_default_setup mode = %d", dptx->connector->conn_info->base.mode);
		dp_ctrl_ldi_setup(cnct, dptx->connector->conn_info->base.mode);
		dp_ctrl_clk_setup(cnct);
		dp_device_params_init(cnct, dptx);
		dptx->id = (uint8_t)i;
		dptx->source_is_external = false;
		dptx->arsr_enable = false;
		dptx->hook_ops = g_dp_hook_ops;

		dptx_link_layer_init(dptx);

		/* The default security attribute after the offset address of the HSDT1 CRG region is 0x800.
		If the attribute is not changed, the DP/HDMI PLL needs to be configured under the security core.
		Therefore, the software changes the attribute to non-secure. */
		dptx_pixel_pll_preinit();

		if (dptx->dptx_default_params_from_core)
			dptx->dptx_default_params_from_core(dptx);

		ret = dp_device_buf_alloc(dptx);
		if (ret != 0)
			dpu_pr_err("[DP] dp device alloc failed!\n");

		dptx->is_dptx_ready = true;
		dp_graphic_debug_node_init(dptx);
		dpu_pr_info("[DP] dp dp_priv->dp_hpd_plug: %d, dptx->port_id: %d\n", dp_priv->dp_hpd_plug, dptx->port_id);
		if (dp_priv->dp_hpd_plug && dptx_hpd_gpio_setup(dptx))
			dpu_pr_err("gpio setup failed");
	}

	connector->on_func = dp_ctrl_on;
	connector->off_func = dp_ctrl_off;
	connector->ops_handle_func = dp_ctrl_ops_handle;

	connector->conn_info->enable_ldi = enable_dptx_ldi;
	connector->conn_info->disable_ldi = disable_dptx_timing_gen;
}

MODULE_LICENSE("GPL");
