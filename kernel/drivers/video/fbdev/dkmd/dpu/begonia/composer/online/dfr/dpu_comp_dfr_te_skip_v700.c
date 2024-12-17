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
#include <linux/ktime.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/notifier.h>

#include "dpu_comp_dfr_te_skip.h"
#include <dpu/soc_dpu_define.h>
#include "cmdlist_interface.h"
#include "dpu_comp_mgr.h"
#include "dkmd_cmdlist.h"
#include "dkmd_lcd_interface.h"
#include "dkmd_dfr_interface.h"
#include "dkmd_peri.h"
#include "dpu_conn_mgr.h"
#include "panel_mgr.h"
#include "dpu_comp_maintain.h"

#define DLEN_MAX 1024

enum {
	CMDLIST_SEND_FRM,
	CMDLIST_REFRESH_FRM,
	CMDLIST_SEND_WITH_90HZ_DCS,
	CMDLIST_SEND_WITH_120HZ_DCS,
	CMDLIST_REFRESH_WITH_1HZ_DCS,
	CMDLIST_SEND_DCS_WITH_FRM,
	CMDLIST_TE2_FIXED_120HZ_DCS,
	CMDLIST_TE2_FOLLOWED_RF_DCS,
	CMDLIST_TYPE_MAX
};

struct dfr_te_skip_ctrl {
	bool resend_safe_frm_rate; /* need resend safe frame rate in the first frame commit after power on. */
	uint32_t active_id;
	uint32_t assist_scene_id;
	uint32_t cmdlist_scene_id;
	uint32_t header_cmdlist_ids[CMDLIST_TYPE_MAX + SAFE_FRM_RATE_MAX_NUM];
	uint32_t reg_cmdlist_ids[CMDLIST_TYPE_MAX + SAFE_FRM_RATE_MAX_NUM];
	struct notifier_block *notifier;
};

/* te skip mode implemented by dacc, only support one scene,
 * so we use global private data, if need support multi scenes,
 * please alloc it dynamiclly, and attach it to connector peri device.
 */
struct dfr_te_skip_ctrl g_te_skip_ctrl = { 0 };

static int32_t cmdlist_mipi_dsi_swrite_to(struct dsi_cmd_desc *cm, uint32_t dsi_offset,
	uint32_t cmdlist_id, uint32_t scene_id)
{
	uint32_t hdr = 0;
	uint32_t len;

	if ((cm->dlen != 0) && (cm->payload == 0)) {
		dpu_pr_err("NO payload error!\n");
		return 0;
	}

	/* mipi dsi short write with 0, 1 2 parameters, total 3 param */
	if (cm->dlen > 2) {
		dpu_pr_err("cm->dlen is invalid");
		return -EINVAL;
	}
	len = cm->dlen;

	hdr |= dsi_hdr_dtype(cm->dtype);
	hdr |= dsi_hdr_vc(cm->vc);
	if (len == 1) {
		hdr |= dsi_hdr_data1((uint32_t)(cm->payload[0]));
		hdr |= dsi_hdr_data2(0);
	} else if (len == 2) {
		hdr |= dsi_hdr_data1((uint32_t)(cm->payload[0]));
		hdr |= dsi_hdr_data2((uint32_t)(cm->payload[1]));
	} else {
		hdr |= dsi_hdr_data1(0);
		hdr |= dsi_hdr_data2(0);
	}

	/* used for low power cmds trans under video mode */
	hdr |= cm->dtype & GEN_VID_LP_CMD;

	dkmd_set_reg(scene_id, cmdlist_id, DPU_DSI_GEN_HP_HDR_ADDR(dsi_offset), hdr);

	dpu_pr_info("hdr = %#x!\n", hdr);
	return (int32_t)len;  /* 4 bytes */
}

static int32_t cmdlist_mipi_dsi_lwrite_to(struct dsi_cmd_desc *cm, uint32_t dsi_offset,
	uint32_t cmdlist_id, uint32_t scene_id)
{
	uint32_t hdr = 0;
	uint32_t i = 0;
	uint32_t j = 0;
	uint32_t pld = 0;

	if ((cm->dlen != 0) && (cm->payload == 0)) {
		dpu_pr_err("NO payload error!\n");
		return 0;
	}

	if (cm->dlen > DLEN_MAX) {
		dpu_pr_err("invalid dlen:%u\n", cm->dlen);
		return 0;
	}

	/* fill up payload, 4bytes set reg, remain 1 byte(8 bits) set reg */
	for (i = 0;  i < cm->dlen; i += 4) {
		if ((i + 4) <= cm->dlen) {
			pld = *((uint32_t *)(cm->payload + i));
		} else {
			for (j = i; j < cm->dlen; j++)
				pld |= (((uint32_t)cm->payload[j] & 0x0ff) << ((j - i) * 8));

			dpu_pr_info("pld = %#x!\n", pld);
		}

		dkmd_set_reg(scene_id, cmdlist_id, DPU_DSI_GEN_HP_PLD_DATA_ADDR(dsi_offset), pld);
		pld = 0;
	}

	/* fill up header */
	hdr |= dsi_hdr_dtype(cm->dtype);
	hdr |= dsi_hdr_vc(cm->vc);
	hdr |= dsi_hdr_wc(cm->dlen);

	/* used for low power cmds trans under video mode */
	hdr |= cm->dtype & GEN_VID_LP_CMD;
	dkmd_set_reg(scene_id, cmdlist_id, DPU_DSI_GEN_HP_HDR_ADDR(dsi_offset), hdr);

	dpu_pr_info("hdr = %#x!\n", hdr);

	return (int32_t)cm->dlen;
}

static void dfr_te_skip_print_dsi_cmds(struct dsi_cmds *cmds)
{
	uint32_t i, j;
	dpu_check_and_no_retval(!cmds, err, "cmds is null");

	for (i = 0; i < cmds->cmd_num; i++) {
		dpu_pr_debug("dtype: %u, vc: %u, dlen: %u", cmds->cmds[i].dtype, cmds->cmds[i].vc, cmds->cmds[i].dlen);
		dpu_pr_debug("payload: ");
		for (j = 0; j < cmds->cmds[i].dlen; j++)
			dpu_pr_debug("0x%x", cmds->cmds[i].payload[j]);
	}
}

static void dfr_te_skip_print_dfr_info(struct dfr_info *dinfo)
{
	uint32_t i;
	uint32_t j;
	struct dimming_sequence dimming_seq;
	dpu_check_and_no_retval(!dinfo, err, "dinfo is null");
	dpu_pr_debug("oled type:%d", dinfo->oled_info.oled_type);

	if (dinfo->oled_info.oled_type != PANEL_OLED_LTPO)
		return;

	dpu_pr_debug("ltpo version:%d, dfr mode:%d", dinfo->oled_info.ltpo_info.ver, dinfo->dfr_mode);
	dpu_pr_debug("ltpo te freq num:%d", dinfo->oled_info.ltpo_info.te_freq_num);
	dpu_pr_info("ltpo te freq:[%d, %d]", dinfo->oled_info.ltpo_info.te_freqs[0], dinfo->oled_info.ltpo_info.te_freqs[1]);
	dpu_pr_debug("ltpo te mask num:%d", dinfo->oled_info.ltpo_info.te_mask_num);
	dpu_pr_info("ltpo te mask:[%d, %d]", dinfo->oled_info.ltpo_info.te_masks[0], dinfo->oled_info.ltpo_info.te_masks[1]);

	if (dinfo->oled_info.ltpo_info.ver == PANEL_LTPO_V1) {
		dpu_pr_debug("ltpo te 90hz dcs cmd:");
		dfr_te_skip_print_dsi_cmds(&dinfo->oled_info.ltpo_info.dsi_cmds[PANEL_LTPO_DSI_CMD_TE_90HZ]);
		dpu_pr_debug("ltpo te 120hz dcs cmd:");
		dfr_te_skip_print_dsi_cmds(&dinfo->oled_info.ltpo_info.dsi_cmds[PANEL_LTPO_DSI_CMD_TE_120HZ]);
	}
	for (i = 0; i < DIMMING_TYPE_MAX; i++) {
		dimming_seq = dinfo->oled_info.ltpo_info.dimming_sequence[i];
		dpu_pr_info("%u dimming type, seq num: %u", dimming_seq.type, dimming_seq.dimming_seq_num);
		if (dimming_seq.type >= DIMMING_TYPE_MAX || dimming_seq.dimming_seq_num > DIMMING_SEQ_LEN_MAX)
			continue;
		for (j = 0; j < dimming_seq.dimming_seq_num; j++)
			dpu_pr_info("node %u: %u, %u", j, dimming_seq.dimming_seq_list[j].frm_rate,
				dimming_seq.dimming_seq_list[j].repeat_num);
	}

	dpu_pr_debug("refresh dcs cmd:");
	dfr_te_skip_print_dsi_cmds(&dinfo->oled_info.ltpo_info.dsi_cmds[PANEL_LTPO_DSI_CMD_REFRESH]);
	dpu_pr_debug("1hz refresh dcs cmd:");
	dfr_te_skip_print_dsi_cmds(&dinfo->oled_info.ltpo_info.dsi_cmds[PANEL_LTPO_DSI_CMD_REFRESH_1HZ]);
	/* safe frm rate cmd */
	for (i = 0; i < min(dinfo->oled_info.ltpo_info.safe_frm_rates_num, (uint32_t)SAFE_FRM_RATE_MAX_NUM); i++) {
		dpu_pr_debug("%u hz safe frm rate dcs cmd:", dinfo->oled_info.ltpo_info.safe_frm_rates[i].safe_frm_rate);
		dfr_te_skip_print_dsi_cmds(&dinfo->oled_info.ltpo_info.safe_frm_rates[i].dsi_cmds);
	}
}

static int32_t dfr_te_skip_create_cmdlist(struct dpu_comp_dfr_ctrl *dfr_ctrl, int32_t cmdlist_type)
{
	int32_t ret;
	struct dfr_te_skip_ctrl *priv = dfr_ctrl->priv_data;

	dpu_check_and_return(cmdlist_type >= CMDLIST_TYPE_MAX + SAFE_FRM_RATE_MAX_NUM, -1, err, "invalid cmdlist id");

	priv->header_cmdlist_ids[cmdlist_type] =
		cmdlist_create_user_client(priv->cmdlist_scene_id, SCENE_NOP_TYPE, 0, 0);
	if (unlikely(priv->header_cmdlist_ids[cmdlist_type] == 0)) {
		dpu_pr_err("scene_id=%u, create header cmdlist fail", priv->cmdlist_scene_id);
		return -1;
	}

	priv->reg_cmdlist_ids[cmdlist_type] =
		cmdlist_create_user_client(priv->cmdlist_scene_id, REGISTER_CONFIG_TYPE, 0, PAGE_SIZE);
	if (unlikely(priv->reg_cmdlist_ids[cmdlist_type] == 0)) {
		dkmd_cmdlist_release_locked(priv->cmdlist_scene_id, priv->header_cmdlist_ids[cmdlist_type]);
		dpu_pr_err("scene_id=%u create reg cmdlist fail", priv->cmdlist_scene_id);
		return -1;
	}

	ret = cmdlist_append_client(priv->cmdlist_scene_id,
		priv->header_cmdlist_ids[cmdlist_type], priv->reg_cmdlist_ids[cmdlist_type]);
	if (unlikely(ret != 0)) {
		dkmd_cmdlist_release_locked(priv->cmdlist_scene_id, priv->header_cmdlist_ids[cmdlist_type]);
		dpu_pr_err("append reg(%u) cmdlist fail", priv->reg_cmdlist_ids[cmdlist_type]);
		return -1;
	}

	return 0;
}

static int32_t dfr_te_skip_write_ddic_cmds_to_cmdlist(uint32_t dsi_offset, struct dsi_cmds *ddic_cmds,
	uint32_t cmdlist_id, uint32_t scene_id)
{
	int i = 0;
	dpu_check_and_return(!ddic_cmds, -1, err, "ddic_cmds is null");
	for (i = 0; i < (int)ddic_cmds->cmd_num; i++) {
		if (ddic_cmds->cmds[i].dtype == DTYPE_DCS_WRITE || ddic_cmds->cmds[i].dtype == DTYPE_DCS_WRITE1) {
			cmdlist_mipi_dsi_swrite_to(&ddic_cmds->cmds[i], dsi_offset, cmdlist_id, scene_id);
		} else if (ddic_cmds->cmds[i].dtype == DTYPE_GEN_LWRITE || ddic_cmds->cmds[i].dtype == DTYPE_DCS_LWRITE) {
			cmdlist_mipi_dsi_lwrite_to(&ddic_cmds->cmds[i], dsi_offset, cmdlist_id, scene_id);
		} else {
			dpu_pr_err("don't support dtype:%d", ddic_cmds->cmds[i].dtype);
			return -1;
		}
	}
	return 0;
}

#define BIT_LDI_EN 0
#define BIT_DUAL_LDI_EN 5
static int32_t dfr_te_skip_set_reg_dsi_ldi(struct dpu_comp_dfr_ctrl *dfr_ctrl, int32_t cmdlist_type)
{
	uint32_t val = 0;
	uint32_t connector_offset = 0;
	struct dpu_connector *connector = NULL;
	struct dfr_te_skip_ctrl *priv = dfr_ctrl->priv_data;
	struct dkmd_connector_info *pinfo = dfr_ctrl->dpu_comp->conn_info;

	dpu_check_and_return(cmdlist_type >= CMDLIST_TYPE_MAX, -1, err, "invalid cmdlist id");
	dpu_check_and_return(!pinfo, -1, err, "pinfo is null");

	connector = get_primary_connector(pinfo);
	dpu_check_and_return(!connector, -1, err, "connector is null");

	connector_offset = g_connector_offset[connector->connector_id];
	dkmd_set_reg(priv->cmdlist_scene_id, priv->reg_cmdlist_ids[cmdlist_type],
		DPU_DSI_LDI_FRM_MSK_UP_ADDR(connector_offset), 0x1);
	if (connector->bind_connector)
		dkmd_set_reg(priv->cmdlist_scene_id, priv->reg_cmdlist_ids[cmdlist_type],
			DPU_DSI_LDI_FRM_MSK_UP_ADDR(g_connector_offset[connector->bind_connector->connector_id]),
			0x1);

	val = inp32(DPU_DSI_LDI_CTRL_ADDR(connector->connector_base));
	if (connector->bind_connector) {
		val |= BIT(BIT_DUAL_LDI_EN);
		dkmd_set_reg(priv->cmdlist_scene_id, priv->reg_cmdlist_ids[cmdlist_type],
			DPU_DSI_LDI_CTRL_ADDR(connector_offset), val);
	} else {
		val |= BIT(BIT_LDI_EN);
		dkmd_set_reg(priv->cmdlist_scene_id, priv->reg_cmdlist_ids[cmdlist_type],
			DPU_DSI_LDI_CTRL_ADDR(connector_offset), val);
	}

	return 0;
}

static int32_t dfr_te_skip_set_reg_dsi_dcs(struct dpu_comp_dfr_ctrl *dfr_ctrl,
	struct dsi_cmds *dsi_cmds, int32_t cmdlist_type)
{
	struct dpu_connector *connector = NULL;
	struct dfr_te_skip_ctrl *priv = dfr_ctrl->priv_data;
	struct dkmd_connector_info *pinfo = dfr_ctrl->dpu_comp->conn_info;

	dpu_check_and_return(cmdlist_type >= CMDLIST_TYPE_MAX + SAFE_FRM_RATE_MAX_NUM, -1, err, "cmdlist type is invalid");
	dpu_check_and_return(!pinfo, -1, err, "pinfo is null");

	if (dsi_cmds->cmd_num == 0) {
		dpu_pr_err("dsi cmds is invalid");
		return 0;
	}
	connector = get_primary_connector(pinfo);
	dpu_check_and_return(!connector, -1, err, "connector is null");

	if (connector->bind_connector && likely(connector->bind_connector->connector_id < CONNECTOR_ID_MAX)) {
		dpu_pr_debug("write ddic cmds to dual mipi");
		dfr_te_skip_write_ddic_cmds_to_cmdlist(g_connector_offset[connector->bind_connector->connector_id],
		dsi_cmds, priv->reg_cmdlist_ids[cmdlist_type], priv->cmdlist_scene_id);
	}

	if (unlikely(connector->connector_id >= CONNECTOR_ID_MAX)) {
		dpu_pr_warn("connector->connector_id=%u is out of range", connector->connector_id);
		return -1;
	}
	return dfr_te_skip_write_ddic_cmds_to_cmdlist(g_connector_offset[connector->connector_id],
		dsi_cmds, priv->reg_cmdlist_ids[cmdlist_type], priv->cmdlist_scene_id);
}

static int32_t dfr_te_skip_setup_cmdlist(struct dpu_comp_dfr_ctrl *dfr_ctrl, int32_t cmdlist_type,
	int32_t dsi_dcs_type, bool set_ldi, bool set_dcs)
{
	struct dfr_te_skip_ctrl *priv = dfr_ctrl->priv_data;
	struct dfr_info *dinfo = dkmd_get_dfr_info(dfr_ctrl->dpu_comp->conn_info);
	dpu_check_and_return(!dinfo, -1, err, "dfr info is null");

	if (dfr_te_skip_create_cmdlist(dfr_ctrl, cmdlist_type) != 0)
		return -1;

	if (set_ldi && (dfr_te_skip_set_reg_dsi_ldi(dfr_ctrl, cmdlist_type) != 0))
		return -1;

	if (dsi_dcs_type >= PANEL_LTPO_DSI_CMD_MAX) {
		dpu_pr_err("dsi dcs type is invalid: %d", dsi_dcs_type);
		return -1;
	}
	if (set_dcs &&
		(dfr_te_skip_set_reg_dsi_dcs(dfr_ctrl, &dinfo->oled_info.ltpo_info.dsi_cmds[dsi_dcs_type], cmdlist_type) != 0))
		return -1;

	dpu_pr_info("cmdlist id: %u, cmdlist type: %d", priv->reg_cmdlist_ids[cmdlist_type], cmdlist_type);
	return 0;
}

static int32_t dfr_te_skip_setup_base_cmdlist(struct dpu_comp_dfr_ctrl *dfr_ctrl, struct dfr_info *dinfo)
{
	struct dfr_te_skip_ctrl *priv = dfr_ctrl->priv_data;

	dpu_check_and_return(!dinfo, -1, err, "dfr info is null");

	/* normal */
	if (dfr_te_skip_setup_cmdlist(dfr_ctrl, CMDLIST_SEND_FRM, 0, true, false) != 0)
		return -1;

	if (dinfo->oled_info.ltpo_info.te_freq_num == 2) {
		/* switch te to 90hz */
		if (dfr_te_skip_setup_cmdlist(dfr_ctrl, CMDLIST_SEND_WITH_90HZ_DCS,
			PANEL_LTPO_DSI_CMD_TE_90HZ, true, true) != 0)
			return -1;

		/* switch te to 120hz */
		if (dfr_te_skip_setup_cmdlist(dfr_ctrl, CMDLIST_SEND_WITH_120HZ_DCS,
			PANEL_LTPO_DSI_CMD_TE_120HZ, true, true) != 0)
			return -1;
	}

	priv->active_id = priv->reg_cmdlist_ids[CMDLIST_SEND_FRM];
	return 0;
}

static int32_t dfr_te_skip_setup_safe_frm_rate_cmds(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	struct dfr_info *dinfo = dkmd_get_dfr_info(dfr_ctrl->dpu_comp->conn_info);
	uint32_t safe_frm_rates_num;
	int32_t cmdlist_type;
	uint32_t i;
	dpu_check_and_return(!dinfo, -1, err, "dfr info is null");

	dpu_pr_debug("safe frm rate cmds setup");
	if (dinfo->oled_info.ltpo_info.safe_frm_rates_num > SAFE_FRM_RATE_MAX_NUM) {
		dpu_pr_warn("safe frm rates num exceed max num");
		dinfo->oled_info.ltpo_info.safe_frm_rates_num = SAFE_FRM_RATE_MAX_NUM;
	}
	safe_frm_rates_num = dinfo->oled_info.ltpo_info.safe_frm_rates_num;
	for (i = 0; i < safe_frm_rates_num; i++) {
		cmdlist_type = CMDLIST_TYPE_MAX + (int32_t)i;
		/* create safe frm rate cmdlist */
		if (dfr_te_skip_create_cmdlist(dfr_ctrl, cmdlist_type) != 0)
			return -1;
		if (dfr_te_skip_set_reg_dsi_dcs(dfr_ctrl, &dinfo->oled_info.ltpo_info.safe_frm_rates[i].dsi_cmds, cmdlist_type) != 0)
			return -1;
	}
	return 0;
}

static int32_t dfr_get_safe_frm_rate_idx(struct dpu_comp_dfr_ctrl *dfr_ctrl, uint32_t safe_frm_rate)
{
	struct dfr_info *dinfo = NULL;
	int32_t frm_index;

	dinfo = dkmd_get_dfr_info(dfr_ctrl->dpu_comp->conn_info);
	dpu_check_and_return(!dinfo, -1, err, "dinfo is null");

	dpu_pr_info("cur safe frm rate is %u", safe_frm_rate);
	if (dinfo->oled_info.ltpo_info.safe_frm_rates_num > SAFE_FRM_RATE_MAX_NUM) {
		dpu_pr_warn("safe frm rates num exceed max num");
		dinfo->oled_info.ltpo_info.safe_frm_rates_num = SAFE_FRM_RATE_MAX_NUM;
	}
	frm_index = (int32_t)dinfo->oled_info.ltpo_info.safe_frm_rates_num -1;
	for (; frm_index >= 0; frm_index--) {
		if (safe_frm_rate == dinfo->oled_info.ltpo_info.safe_frm_rates[frm_index].safe_frm_rate)
			break;
	}
	dpu_pr_info("cur safe_frm_rate_idx is %u", frm_index);
	return frm_index;
}


#define LTPO_VER1_TE_RATE 90
#define SAFE_FRM_RATE_MAX 0xffff
#define SAFE_FRM_RATE_DEFAULT 120
static int32_t dfr_te_skip_check_safe_frm_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl, uint32_t safe_frm_rate)
{
	uint32_t parse_frm_rate = safe_frm_rate;
	int32_t frm_index = 0;

	dpu_pr_debug("check safe frm rate enter");
	if (safe_frm_rate == SAFE_FRM_RATE_MAX) {
		dpu_pr_warn("safe_frm_rate is max, need set default safe frm rate!");
		parse_frm_rate = SAFE_FRM_RATE_DEFAULT;
	}

	if (dfr_ctrl->cur_safe_frm_rate == parse_frm_rate || parse_frm_rate == 0) {
		dpu_pr_debug("No need to send safe frm rate");
		if (g_debug_dpu_safe_frm_rate == 0)
			return -1;
		parse_frm_rate = g_debug_dpu_safe_frm_rate;
	}

	frm_index = dfr_get_safe_frm_rate_idx(dfr_ctrl, parse_frm_rate);
	if (frm_index == -1) {
		dpu_pr_warn("not support safe frm rate!\n");
		return -1;
	}
	dfr_ctrl->cur_safe_frm_rate = parse_frm_rate;
	/* cur te rate is 90Hz, return */
	if (dfr_ctrl->cur_te_rate == LTPO_VER1_TE_RATE) {
		dpu_pr_info("current te is %u, only save safe frm rate!", dfr_ctrl->cur_te_rate);
		return -1;
	}
	return frm_index;
}

static void dfr_te_update_safe_frm_rate_to_dacc(uint32_t safe_frm_rate,
	struct dfr_te_skip_ctrl *priv, struct composer_manager *comp_mgr, dma_addr_t phy_addr)
{
	/* scene 6, 0x1EC, update safe frm rate cmdlist addr */
	outp32(DPU_DM_LAYER_START_ADDR1_L_ADDR(comp_mgr->dpu_base +
		g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0), phy_addr);
	/* 0x1E4, used for safe frm rate */
	outp32(DPU_DM_LAYER_DMA_SEL_ADDR(comp_mgr->dpu_base +
		g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0), safe_frm_rate);
	return;
}

static void dfr_te_skip_update_safe_frm_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl,
	struct dfr_te_skip_ctrl *priv, struct composer_manager *comp_mgr, uint32_t safe_frm_rate)
{
	int32_t frm_index = 0;
	dma_addr_t phy_addr;
	frm_index = dfr_get_safe_frm_rate_idx(dfr_ctrl, safe_frm_rate);
	if (frm_index == -1) {
		dpu_pr_warn("No set safe frm rate!");
		return;
	}
	phy_addr = cmdlist_get_phy_addr(priv->cmdlist_scene_id, priv->reg_cmdlist_ids[frm_index + CMDLIST_TYPE_MAX]);
	if (phy_addr == 0) {
		dpu_pr_err("invalid safe frm cmdlist %u", frm_index + CMDLIST_TYPE_MAX);
		return;
	}
	dfr_te_update_safe_frm_rate_to_dacc(safe_frm_rate, priv, comp_mgr, phy_addr);
}

static int32_t dfr_te_skip_setup_dimming_cmdlist(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	dma_addr_t phy_addr;
	struct dfr_te_skip_ctrl *priv = dfr_ctrl->priv_data;
	char __iomem *dpu_base = dfr_ctrl->dpu_comp->comp_mgr->dpu_base;

	/* refresh */
	if (dfr_te_skip_setup_cmdlist(dfr_ctrl, CMDLIST_REFRESH_FRM,
		PANEL_LTPO_DSI_CMD_REFRESH, false, true) != 0) {
		dpu_pr_warn("create refresh cmdlist failed");
		return -1;
	}

	phy_addr = cmdlist_get_phy_addr(priv->cmdlist_scene_id, priv->reg_cmdlist_ids[CMDLIST_REFRESH_FRM]);
	if (phy_addr != 0) {
		/* 0x1E0, used for refresh cmdlist addr */
		outp32(DPU_DM_LAYER_START_ADDR0_L_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0), phy_addr);
	} else {
		dpu_pr_warn("invalid refresh cmdlist");
		return -1;
	}

	/* 1hz refresh default */
	if (dfr_te_skip_setup_cmdlist(dfr_ctrl, CMDLIST_REFRESH_WITH_1HZ_DCS,
		PANEL_LTPO_DSI_CMD_REFRESH_1HZ, false, true) != 0) {
		dpu_pr_warn("create 1hz refresh cmdlist failed");
		return -1;
	}
	/* handle safe frm rate cmd */
	if (dfr_te_skip_setup_safe_frm_rate_cmds(dfr_ctrl) != 0) {
		dpu_pr_warn("set safe_frm_rate cmd failed");
		return -1;
	}
	return 0;
}

static int32_t dfr_te_skip_dimming_done_isr_notify(struct notifier_block *self,
	unsigned long action, void *data)
{
	struct dkmd_listener_data *listener_data = (struct dkmd_listener_data *)data;
	struct dpu_comp_dfr_ctrl *dfr_ctrl = (struct dpu_comp_dfr_ctrl *)(listener_data->data);
	struct composer_manager *comp_mgr = dfr_ctrl->dpu_comp->comp_mgr;

	composer_mgr_disable_dimming(comp_mgr, dfr_ctrl->dpu_comp->comp.index);

	return 0;
}

static struct notifier_block dfr_te_skip_dimming_done_isr_notifier = {
	.notifier_call = dfr_te_skip_dimming_done_isr_notify,
};

static void dfr_te_skip_setup_dimming_listener(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	struct dkmd_isr *isr_ctrl = &dfr_ctrl->dpu_comp->comp_mgr->mdp_isr_ctrl;
	struct dfr_te_skip_ctrl *priv = (struct dfr_te_skip_ctrl *)dfr_ctrl->priv_data;

	priv->notifier = &dfr_te_skip_dimming_done_isr_notifier;

	dkmd_isr_register_listener(isr_ctrl, priv->notifier, NOTIFY_DIMMING_DONE, dfr_ctrl);
}

static void dfr_te_skip_write_dimming_seq_to_dacc(struct dimming_sequence *dimming_seq,
	char __iomem *dpu_base, struct dfr_te_skip_ctrl *priv)
{
	uint32_t i;
	char __iomem *dimming_seq_base = NULL;
	uint32_t dimming_seq_num = 0;
	uint32_t dimming_frame_info = 0;
	if (priv->assist_scene_id >= DPU_SCENE_MAX) {
		dpu_pr_warn("scene id is invalid");
		return;
	}

	dimming_seq_num = inp32(DPU_DM_LAYER_SBLK_TYPE_ADDR(dpu_base +
		g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0));
	switch (dimming_seq->type) {
	case DIMMING_NORMAL:
		/* bit0-bit8: normal dimming seq num */
		dimming_seq_num |= (dimming_seq->dimming_seq_num & 0xFF);
		/* write dimming seq list */
		dimming_seq_base =
			DPU_DM_LAYER_BOT_CROP_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0);
		break;
	case DIMMING_90HZ:
		/* bit8-bit16: 90HZ dimming seq num */
		dimming_seq_num |= (dimming_seq->dimming_seq_num & 0xFF) << 8;
		/* write dimming seq list */
		dimming_seq_base =
			DPU_DM_LAYER_RSV3_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0);
		break;
	default:
		dpu_pr_err("not support dimming type %d", dimming_seq->type);
		return;
	}

	dpu_pr_debug("dimming seq num is %u", dimming_seq_num);
	outp32(DPU_DM_LAYER_SBLK_TYPE_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0),
			dimming_seq_num);
	for (i = 0; i < dimming_seq->dimming_seq_num; i += 2) {
		dimming_frame_info = 0;
		dimming_frame_info |= dimming_seq->dimming_seq_list[i].frm_rate & 0xFF;
		dimming_frame_info |= (dimming_seq->dimming_seq_list[i].repeat_num & 0xFF) << 8;
		dimming_frame_info |= (dimming_seq->dimming_seq_list[i + 1].frm_rate & 0xFF) << 16;
		dimming_frame_info |= (dimming_seq->dimming_seq_list[i + 1].repeat_num & 0xFF) << 24;
		outp32(dimming_seq_base, dimming_frame_info);
		dimming_seq_base += 4;
	}
}

static void dfr_te_skip_setup_dimming(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	int32_t i;
	char __iomem *dpu_base = NULL;
	struct dfr_te_skip_ctrl *priv = dfr_ctrl->priv_data;
	struct dfr_info *dinfo = dkmd_get_dfr_info(dfr_ctrl->dpu_comp->conn_info);
	struct dimming_sequence *dimming_seq = NULL;

	dpu_check_and_no_retval(!dinfo, err, "dfr info is null");

	if (unlikely(priv->assist_scene_id >= DPU_SCENE_MAX)) {
		dpu_pr_warn("priv->assist_scene_id=%u is out of range", priv->assist_scene_id);
		return;
	}

	if (dfr_te_skip_setup_dimming_cmdlist(dfr_ctrl) != 0) {
		dpu_pr_warn("setup dimming cmdlist failed");
		return;
	}

	dpu_base = dfr_ctrl->dpu_comp->comp_mgr->dpu_base;
	for (i = 0; i < DIMMING_TYPE_MAX; i++) {
		dimming_seq = &dinfo->oled_info.ltpo_info.dimming_sequence[i];
		if (!dimming_seq || dimming_seq->dimming_seq_num == 0 ||
			dimming_seq->dimming_seq_num > DIMMING_SEQ_LEN_MAX) {
			dpu_pr_warn("dimming_seq is null or dimming seq num is invalid");
			continue;
		}
		dfr_te_skip_write_dimming_seq_to_dacc(dimming_seq, dpu_base, priv);
	}

	/* enable frm dimming */
	if (dinfo->oled_info.ltpo_info.dimming_enable)
		outp32(DPU_DM_LAYER_OV_MODE_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0), 1);

	dfr_te_skip_setup_dimming_listener(dfr_ctrl);
}

static int32_t dfr_te_skip_get_dsi_cmds(struct dpu_comp_dfr_ctrl *dfr_ctrl,
	struct disp_effect_params *effect_params, struct dsi_cmds *ddic_cmds)
{
	struct dkmd_connector_info *pinfo = dfr_ctrl->dpu_comp->conn_info;
	dpu_check_and_return(!pinfo, -1, err, "pinfo is null");
	dpu_check_and_return(!pinfo->get_ddic_cmds, -1, err, "get_ddic_cmds func is null");

	pinfo->get_ddic_cmds(pinfo, effect_params, ddic_cmds);

	dpu_check_and_return(!ddic_cmds, -1, err, "ddic_cmds is null");

	if (ddic_cmds->cmd_num == 0) {
		dpu_pr_warn("no ddic cmds");
		return -1;
	}

	if (ddic_cmds->cmd_num > DDIC_CMDS_MAX_LEN) {
		dpu_pr_warn("ddic cmds len exceed threshold, current len is %u", ddic_cmds->cmd_num);
		return -1;
	}

	return 0;
}

static int32_t dfr_te_skip_setup_te2_cmdlist(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	dma_addr_t phy_addr;

	struct dsi_cmds fixed_120hz_cmds = { 0 };
	struct dsi_cmds followed_rf_cmds = { 0 };
	struct disp_effect_params effect_params = { 0 };

	struct dpu_connector *connector = NULL;
	struct dfr_te_skip_ctrl *priv = dfr_ctrl->priv_data;
	struct dkmd_connector_info *pinfo = dfr_ctrl->dpu_comp->conn_info;
	char __iomem *dpu_base = dfr_ctrl->dpu_comp->comp_mgr->dpu_base;

	connector = get_primary_connector(pinfo);
	dpu_check_and_return(!connector, -1, err, "connector is null");

	effect_params.effect_num = 1;
	effect_params.params[0].effect_type = DISP_EFFECT_TE2_FIXED;
	if (dfr_te_skip_get_dsi_cmds(dfr_ctrl, &effect_params, &fixed_120hz_cmds) != 0) {
		dpu_pr_err("get dsi cmds error");
		return -1;
	}

	effect_params.params[0].effect_type = DISP_EFFECT_TE2_FOLLOWED;
	if (dfr_te_skip_get_dsi_cmds(dfr_ctrl, &effect_params, &followed_rf_cmds) != 0) {
		dpu_pr_err("get dsi cmds error");
		return -1;
	}

	if (dfr_te_skip_create_cmdlist(dfr_ctrl, CMDLIST_TE2_FIXED_120HZ_DCS) != 0)
		return -1;

	if (dfr_te_skip_create_cmdlist(dfr_ctrl, CMDLIST_TE2_FOLLOWED_RF_DCS) != 0)
		return -1;

	dfr_te_skip_write_ddic_cmds_to_cmdlist(g_connector_offset[connector->connector_id],
		&fixed_120hz_cmds, priv->reg_cmdlist_ids[CMDLIST_TE2_FIXED_120HZ_DCS], priv->cmdlist_scene_id);

	dfr_te_skip_write_ddic_cmds_to_cmdlist(g_connector_offset[connector->connector_id],
		&followed_rf_cmds, priv->reg_cmdlist_ids[CMDLIST_TE2_FOLLOWED_RF_DCS], priv->cmdlist_scene_id);

	phy_addr = cmdlist_get_phy_addr(priv->cmdlist_scene_id, priv->reg_cmdlist_ids[CMDLIST_TE2_FIXED_120HZ_DCS]);
	if (phy_addr != 0) {
		outp32(DPU_DM_LAYER_START_ADDR2_L_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 1), phy_addr);
	} else {
		dpu_pr_warn("invalid te2 fixed 120hz cmdlist");
		return -1;
	}

	phy_addr = cmdlist_get_phy_addr(priv->cmdlist_scene_id, priv->reg_cmdlist_ids[CMDLIST_TE2_FOLLOWED_RF_DCS]);
	if (phy_addr != 0) {
		outp32(DPU_DM_LAYER_START_ADDR3_L_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 1), phy_addr);
	} else {
		dpu_pr_warn("invalid te2 followed rf cmdlist");
		return -1;
	}

	dpu_pr_info("setup ok");
	return 0;
}

void dfr_te_skip_setup_priv_data(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	struct dfr_te_skip_ctrl *priv;
	char __iomem *dpu_base = NULL;
	struct dfr_info *dinfo = NULL;
	uint32_t init_config = 0;

	dpu_check_and_no_retval(!dfr_ctrl, err, "dfr_ctrl is null");
	dpu_check_and_no_retval(!dfr_ctrl->dpu_comp, err, "dpu_comp is null");

	dinfo = dkmd_get_dfr_info(dfr_ctrl->dpu_comp->conn_info);

	dpu_check_and_no_retval(!dinfo, err, "dfr info is null");
	dpu_check_and_no_retval(!dfr_ctrl->dpu_comp->comp_mgr, err, "comp_mgr is null");

	dpu_base = dfr_ctrl->dpu_comp->comp_mgr->dpu_base;

	dpu_check_and_no_retval(dinfo->dfr_mode != DFR_MODE_TE_SKIP_BY_MCU,
		err, "invalid dfr mode:%d", dinfo->dfr_mode);

	dpu_check_and_no_retval(dinfo->oled_info.oled_type != PANEL_OLED_LTPO,
		err, "invalid oled type:%d", dinfo->oled_info.oled_type);

	dfr_te_skip_print_dfr_info(dinfo);

	dfr_ctrl->priv_data = (void*)&g_te_skip_ctrl;
	priv = (struct dfr_te_skip_ctrl *)dfr_ctrl->priv_data;
	priv->assist_scene_id = DPU_SCENE_OFFLINE_2;
	priv->cmdlist_scene_id = DPU_SCENE_ONLINE_3;
	dfr_ctrl->pre_te_rate = 0;
	dfr_ctrl->pre_te_mask_num = 0;

	if (dfr_te_skip_setup_base_cmdlist(dfr_ctrl, dinfo) != 0)
		return;

	/* clear safe frm rate */
	outp32(DPU_DM_LAYER_DMA_SEL_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0), 0);

	/* enable dacc dfr */
	outp32(DPU_DM_LAYER_HEIGHT_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0), 1);

	/* disable frm dimming */
	outp32(DPU_DM_LAYER_OV_MODE_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0), 0);

	/* clear DFR_SEND_EFFECT_DCS flag */
	outp32(DPU_DM_LAYER_OV_PATTERN_RGB_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0), 0);

	/* clear SEND_DCS_INDIRECT flag */
	outp32(DPU_DM_LAYER_STRIDE0_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0), 0);
	/* init frame rate and te */
	outp32(DPU_DM_LAYER_MASK_Y0_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0), 0);
	outp32(DPU_DM_LAYER_MASK_Y1_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0), 0);
	/* init mask te num */
	outp32(DPU_DM_LAYER_STRIDE2_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0), 0);

	/* init config 0x1F4 */
	init_config = inp32(DPU_DM_LAYER_STRIDE1_ADDR(dpu_base + g_dm_tlb_info[DPU_SCENE_OFFLINE_2].dm_data_addr, 0));
	/* bit0      : ddic_type */
	dinfo->ddic_type == DDIC_TYPE_H01 ? (init_config |= BIT(0)):(init_config &= ~(BIT(0)));
	/* bit1~bit9 : fps */
	init_config &= 0xFFFFFE01;
	init_config |= (dfr_ctrl->dpu_comp->conn_info->base.fps & 0xFF) << 1;

	dpu_pr_info("dfr_te_skip_setup_priv_data init_config = %u", init_config);

	outp32(DPU_DM_LAYER_STRIDE1_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0), init_config);

	dfr_te_skip_setup_dimming(dfr_ctrl);

	if (dinfo->oled_info.ltpo_info.te2_enable) {
		/* enable te2 in dmcu */
		if (!dfr_te_skip_setup_te2_cmdlist(dfr_ctrl))
			outp32(DPU_DM_LAYER_START_ADDR1_L_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 1), 1);
	}

	/* do not set safe frame rate in 90Hz TE case */
	if (dfr_ctrl->cur_te_rate != LTPO_VER1_TE_RATE)
		priv->resend_safe_frm_rate = true;
}

void dfr_te_skip_release_priv_data(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	struct dpu_composer *dpu_comp = NULL;
	struct dfr_te_skip_ctrl *priv = NULL;
	struct dkmd_isr *isr_ctrl = NULL;
	char __iomem *dpu_base = NULL;
	int32_t i;

	dpu_check_and_no_retval(!dfr_ctrl, err, "dfr_ctrl is null");
	dpu_check_and_no_retval(!dfr_ctrl->dpu_comp, err, "dfr info is null");
	dpu_check_and_no_retval(!dfr_ctrl->priv_data, err, "dfr_ctrl->priv_data is null");

	dpu_comp = dfr_ctrl->dpu_comp;
	priv = (struct dfr_te_skip_ctrl *)dfr_ctrl->priv_data;
	isr_ctrl = &dpu_comp->comp_mgr->mdp_isr_ctrl;
	dpu_base = dpu_comp->comp_mgr->dpu_base;

	dpu_pr_info("+");

	for (i = 0; i < CMDLIST_TYPE_MAX + SAFE_FRM_RATE_MAX_NUM; i++) {
		if (priv->header_cmdlist_ids[i] != 0) {
			dkmd_cmdlist_release_locked(priv->cmdlist_scene_id, priv->header_cmdlist_ids[i]);
			priv->header_cmdlist_ids[i] = 0;
		}
	}

	dkmd_isr_unregister_listener(isr_ctrl, priv->notifier, NOTIFY_DIMMING_DONE);
	/* disable frm dimming */
	outp32(DPU_DM_LAYER_OV_MODE_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0), 0);

	/* disable dacc dfr */
	outp32(DPU_DM_LAYER_HEIGHT_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0), 0);

	/* disable te2 ctrl */
	outp32(DPU_DM_LAYER_START_ADDR1_L_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 1), 0);

	composer_mgr_disable_dimming(dpu_comp->comp_mgr, dpu_comp->comp.index);
}

static uint32_t dfr_te_skip_get_te_mask(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	int i;
	struct dfr_info *dinfo = dkmd_get_dfr_info(dfr_ctrl->dpu_comp->conn_info);

	dpu_check_and_return(!dinfo, 0, err, "dfr info is null");
	dpu_check_and_return(dinfo->oled_info.ltpo_info.te_mask_num == 0, 0, err, "invalid te mask num");

	if (dfr_ctrl->cur_frm_rate == 0)
		return dinfo->oled_info.ltpo_info.te_masks[0];

	for (i = 0; i < TE_FREQ_NUM_MAX; i++) {
		if (dinfo->oled_info.ltpo_info.te_freqs[i] % dfr_ctrl->cur_frm_rate == 0)
			return dinfo->oled_info.ltpo_info.te_masks[i];
	}

	dpu_pr_err("frm rate is %uhz, no valid te freq to devide", dfr_ctrl->cur_frm_rate);

	// use te mask default value 0
	return 0;
}

/* todo, 120 & 90 divided strategy */
static uint32_t dfr_te_skip_get_te_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	int i;
	struct dfr_info *dinfo = dkmd_get_dfr_info(dfr_ctrl->dpu_comp->conn_info);

	dpu_check_and_return(!dinfo, 0, err, "dfr info is null");
	dpu_check_and_return(dinfo->oled_info.ltpo_info.te_freq_num == 0, 0, err, "invalid te freq");

	if (dfr_ctrl->cur_frm_rate == 0)
		return dinfo->oled_info.ltpo_info.te_freqs[0];

	for (i = 0; i < TE_FREQ_NUM_MAX; i++) {
		if (dinfo->oled_info.ltpo_info.te_freqs[i] % dfr_ctrl->cur_frm_rate == 0)
			return dinfo->oled_info.ltpo_info.te_freqs[i];
	}

	dpu_pr_err("frm rate is %uhz, no valid te freq to devide", dfr_ctrl->cur_frm_rate);
	return 0;
}

static void dfr_te_skip_switch_te_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	struct dfr_te_skip_ctrl *priv = dfr_ctrl->priv_data;

	if ((dfr_ctrl->pre_te_rate == 0 || dfr_ctrl->pre_te_rate == dfr_ctrl->cur_te_rate) &&
		(dfr_ctrl->resend_switch_cmds == false)) {
		priv->active_id = priv->reg_cmdlist_ids[CMDLIST_SEND_FRM];
		return;
	}

	switch (dfr_ctrl->cur_te_rate) {
	case 120:
		priv->active_id = priv->reg_cmdlist_ids[CMDLIST_SEND_WITH_120HZ_DCS];
		break;
	case 90:
		priv->active_id = priv->reg_cmdlist_ids[CMDLIST_SEND_WITH_90HZ_DCS];
		break;
	default:
		dpu_pr_err("invalid te freq: %u", dfr_ctrl->cur_te_rate);
		break;
	}
}

int32_t dfr_te_skip_switch_frm_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl, uint32_t frame_rate)
{
	char __iomem *dpu_base = dfr_ctrl->dpu_comp->comp_mgr->dpu_base;
	struct dkmd_connector_info *pinfo = dfr_ctrl->dpu_comp->conn_info;
	struct dfr_info *dinfo = dkmd_get_dfr_info(dfr_ctrl->dpu_comp->conn_info);
	struct dfr_te_skip_ctrl *priv = dfr_ctrl->priv_data;
	struct dpu_panel_ops *pops = get_panel_ops(dfr_ctrl->dpu_comp->conn_info->base.id);
	uint32_t dimming_status;
	uint32_t current_config = 0;

	dpu_check_and_return(!pinfo, -1, err, "pinfo is null");
	dpu_check_and_return(!dinfo, -1, err, "dfr info is null");
	dpu_check_and_return(!pops, -1, err, "pops is null");
	if (unlikely(priv->assist_scene_id >= DPU_SCENE_MAX)) {
		dpu_pr_warn("priv->assist_scene_id=%u is out of range", priv->assist_scene_id);
		return -1;
	}

	if (dfr_ctrl->pre_te_rate == 0 && dfr_ctrl->cur_te_rate == LTPO_VER1_TE_RATE) {
		dpu_pr_info("resume 90hz te");
		pipeline_next_ops_handle(pinfo->conn_device, pinfo, DCS_CMD_TX_FOR_PLATFORM,
			&dinfo->oled_info.ltpo_info.dsi_cmds[PANEL_LTPO_DSI_CMD_TE_90HZ]);
		outp32(DPU_DM_LAYER_OV_MODE_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0), 0);
	}

	dfr_ctrl->pre_te_mask_num = dfr_ctrl->cur_te_mask_num;
	dfr_ctrl->pre_te_rate = dfr_ctrl->cur_te_rate;
	dfr_ctrl->pre_frm_rate = dfr_ctrl->cur_frm_rate;
	dfr_ctrl->cur_frm_rate = frame_rate;
	dfr_ctrl->cur_te_rate = dfr_te_skip_get_te_rate(dfr_ctrl);
	dfr_ctrl->cur_te_mask_num = dfr_te_skip_get_te_mask(dfr_ctrl);

	if (dfr_ctrl->pre_frm_rate != dfr_ctrl->cur_frm_rate) {
		dpu_pr_info("switch frame rate from %dhz to %dhz", dfr_ctrl->pre_frm_rate, dfr_ctrl->cur_frm_rate);
		dpu_pr_info("switch te rate from %u to %u", dfr_ctrl->pre_te_rate, dfr_ctrl->cur_te_rate);
		dpu_pr_info("switch te mask num from %u to %u", dfr_ctrl->pre_te_mask_num, dfr_ctrl->cur_te_mask_num);
		if (pops->set_current_fps)
			pops->set_current_fps(dfr_ctrl->cur_frm_rate);
	}

	dfr_te_skip_switch_te_rate(dfr_ctrl);
	/* update frame rate to dacc */
	outp32(DPU_DM_LAYER_MASK_Y0_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0),
		dfr_ctrl->cur_frm_rate);

	/* update te rate to dacc */
	outp32(DPU_DM_LAYER_MASK_Y1_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0),
		dfr_ctrl->cur_te_rate);

	/* update te mask to dacc */
	outp32(DPU_DM_LAYER_STRIDE2_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0),
		dfr_ctrl->cur_te_mask_num);

	if (dfr_ctrl->resend_switch_cmds) {
		dfr_ctrl->resend_switch_cmds = false;
		if (priv->active_id == priv->reg_cmdlist_ids[CMDLIST_SEND_WITH_120HZ_DCS])
			current_config |= BIT(0);
		else if (priv->active_id == priv->reg_cmdlist_ids[CMDLIST_SEND_WITH_90HZ_DCS])
			current_config |= BIT(1);
		outp32(DPU_DM_LAYER_START_ADDR3_L_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0),
			current_config);
	}
	/* ltpo1.0: when te rate is 90Hz, disable dimming */
	if (dfr_ctrl->pre_te_rate == dfr_ctrl->cur_te_rate ||
		(dfr_ctrl->pre_te_rate != 90 && dfr_ctrl->cur_te_rate != 90))
		return 0;
	dpu_pr_info("cur te rate is %u, switch dimming status", dfr_ctrl->cur_te_rate);
	dimming_status = dfr_ctrl->cur_te_rate == 90 ? 0 : 1;
	outp32(DPU_DM_LAYER_OV_MODE_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0), dimming_status);
	return 0;
}

static void dfr_te_skip_check_send_safe_frm_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl,
	struct dfr_te_skip_ctrl *priv, struct composer_manager *comp_mgr)
{
	uint32_t pre_safe_frm_rate = 0;
	struct comp_online_present *present = (struct comp_online_present *)dfr_ctrl->dpu_comp->present_data;
	if (priv->assist_scene_id >= DPU_SCENE_MAX) {
		dpu_pr_warn("scene id is invalid");
		return;
	}

	if (priv->resend_safe_frm_rate) {
		priv->resend_safe_frm_rate = false;
		dpu_pr_info("Need resend safe frm rate in the first frame commit after power on");
		outp32(DPU_DM_LAYER_START_ADDR2_L_ADDR(comp_mgr->dpu_base +
			g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0), 1);
	}

	/* when idle, need send safe frm rate by commit */
	pre_safe_frm_rate = inp32(DPU_DM_LAYER_DMA_SEL_ADDR(comp_mgr->dpu_base +
		g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0));
	if (dfr_ctrl->cur_safe_frm_rate != pre_safe_frm_rate) {
		dpu_pr_info("set safe frm rate by commit");
		dfr_te_skip_update_safe_frm_rate(dfr_ctrl, priv, comp_mgr, dfr_ctrl->cur_safe_frm_rate);

		/* for the frame rate statistics in maintain module */
		if (dfr_ctrl->cur_safe_frm_rate > 0)
			present->comp_maintain.self_refresh_period_us = PERIOD_US_1HZ / dfr_ctrl->cur_safe_frm_rate;
		else
			dpu_pr_warn("meet invalide safe frame rate(%u) when set it to maintain!", dfr_ctrl->cur_safe_frm_rate);
	}
}

int32_t dfr_te_skip_commit(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	struct composer_manager *comp_mgr = dfr_ctrl->dpu_comp->comp_mgr;
	char __iomem *dpu_base = comp_mgr->dpu_base;
	struct dfr_te_skip_ctrl *priv = dfr_ctrl->priv_data;
	dma_addr_t phy_addr = 0;
	struct dfr_info *dinfo = dkmd_get_dfr_info(dfr_ctrl->dpu_comp->conn_info);

	dpu_check_and_return(!dinfo, -1, err, "dfr info is null");
	dpu_check_and_return(!priv, -1, err, "priv is null");

	dpu_pr_debug("active id: %u", priv->active_id);
	phy_addr = cmdlist_get_phy_addr(priv->cmdlist_scene_id, priv->active_id);
	if (phy_addr == 0) {
		dpu_pr_err("invalid active cmdlist %u, commit failed", priv->active_id);
		return -1;
	}

	if (priv->assist_scene_id >= DPU_SCENE_MAX) {
		dpu_pr_err("invalid assist_scene_id %u", priv->assist_scene_id );
		return -1;
	}

	/* scene 6, 0x1D8, used for cmdlist addr */
	outp32(DPU_DM_LAYER_STRETCH3_LINE_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0), phy_addr);

	/* notifiy frame update to dacc  */
	outp32(SOC_DACC_CLR_RISCV_START_ADDR(dpu_base + DACC_OFFSET + DMC_OFFSET, 6), 1);

	if (dinfo->oled_info.ltpo_info.dimming_enable)
		composer_mgr_enable_dimming(comp_mgr, dfr_ctrl->dpu_comp->comp.index);

	/* check send safe frm rate */
	dfr_te_skip_check_send_safe_frm_rate(dfr_ctrl, priv, comp_mgr);
	return 0;
}

static int32_t dfr_te_skip_config_cmdlist(struct dpu_comp_dfr_ctrl *dfr_ctrl, struct dsi_cmds *ddic_cmds,
	uint32_t cmdlist_id, uint32_t scene_id)
{
	struct dkmd_connector_info *pinfo = dfr_ctrl->dpu_comp->conn_info;
	struct dpu_connector *connector = NULL;

	dpu_check_and_return(!pinfo, -1, err, "pinfo is null");
	connector = get_primary_connector(pinfo);
	dpu_check_and_return(!connector, -1, err, "connector is null");

	if (connector->bind_connector) {
		dpu_pr_debug("write ddic cmds to dual mipi");
		dfr_te_skip_write_ddic_cmds_to_cmdlist(g_connector_offset[connector->bind_connector->connector_id],
			ddic_cmds, cmdlist_id, scene_id);
	}
	return dfr_te_skip_write_ddic_cmds_to_cmdlist(g_connector_offset[connector->connector_id],
		ddic_cmds, cmdlist_id, scene_id);
}

static int32_t dfr_te_skip_get_dsi_cmds_with_frame(struct dpu_comp_dfr_ctrl *dfr_ctrl,
	struct disp_effect_params *effect_params, uint32_t cmdlist_id, uint32_t scene_id)
{
	struct dsi_cmds ddic_cmds = { 0 };

	/* get ddic cmds and write cmdlist */
	if (dfr_te_skip_get_dsi_cmds(dfr_ctrl, effect_params, &ddic_cmds) != 0) {
		dpu_pr_err("get ddic cmds error\n");
		return -1;
	}
	if (dfr_te_skip_config_cmdlist(dfr_ctrl, &ddic_cmds, cmdlist_id, scene_id) == -1) {
		dpu_pr_err("ddic cmds config cmdlist error\n");
		return -1;
	}
	return 0;
}

static int32_t dfr_te_skip_recreate_cmdlist(struct dpu_comp_dfr_ctrl *dfr_ctrl, uint32_t cmdlist_type)
{
	struct dfr_te_skip_ctrl *priv = dfr_ctrl->priv_data;
	uint32_t header_cmdlist_id = 0;
	dpu_check_and_return(!priv, -1, err, "priv is null");
	header_cmdlist_id = priv->header_cmdlist_ids[cmdlist_type];

	if (header_cmdlist_id != 0) {
		dkmd_cmdlist_release_locked(priv->cmdlist_scene_id, header_cmdlist_id);
		priv->header_cmdlist_ids[cmdlist_type] = 0;
	}
	if (dfr_te_skip_create_cmdlist(dfr_ctrl, (int32_t)cmdlist_type) != 0) {
		dpu_pr_err("create cmdlist error !\n");
		return -1;
	}
	return (int32_t)priv->reg_cmdlist_ids[cmdlist_type];
}

int32_t dfr_te_skip_send_dcs_cmds(struct dpu_comp_dfr_ctrl *dfr_ctrl, struct disp_effect_params *effect_params)
{
	struct composer_manager *comp_mgr = NULL;
	struct dpu_bl_ctrl *bl_ctrl = NULL;
	struct dfr_te_skip_ctrl *priv = NULL;
	struct dkmd_connector_info *pinfo = NULL;
	int32_t cmdlist_id;
	dma_addr_t phy_addr;

	dpu_check_and_return(!dfr_ctrl, -1, err, "dfr_ctrl is null");
	dpu_check_and_return(!effect_params, -1, err, "effect_params is null");
	dpu_check_and_return(!dfr_ctrl->dpu_comp, -1, err, "dpu_composer is null");
	dpu_check_and_return(effect_params->effect_num > EFFECT_PARAMS_MAX_NUM, -1, err, "effect num exceeds max num");
	if ((effect_params->delay != 0 || effect_params->effect_num == 0) && (g_debug_dpu_send_dcs_cmds == 0)) {
		dpu_pr_debug("Nothing to send !\n");
		return 0;
	}

	comp_mgr = dfr_ctrl->dpu_comp->comp_mgr;
	bl_ctrl = &dfr_ctrl->dpu_comp->bl_ctrl;
	priv = dfr_ctrl->priv_data;
	pinfo = dfr_ctrl->dpu_comp->conn_info;

	cmdlist_id = dfr_te_skip_recreate_cmdlist(dfr_ctrl, CMDLIST_SEND_DCS_WITH_FRM);
	if (cmdlist_id == -1) {
		dpu_pr_err("recreate cmdlist error !\n");
		return -1;
	}
	/* debug update bl_level */
	if (g_debug_dpu_send_dcs_cmds)
		dpu_backlight_debug_level(effect_params);
	dpu_backlight_update_level(bl_ctrl, effect_params);

	/* get dsi cmds and write cmdlist */
	if (dfr_te_skip_get_dsi_cmds_with_frame(dfr_ctrl, effect_params,
		(uint32_t)cmdlist_id, priv->cmdlist_scene_id) == -1) {
		dpu_pr_err("get dsi cmds or write cmdlist error\n");
		return -1;
	}

	phy_addr = cmdlist_get_phy_addr(priv->cmdlist_scene_id, (uint32_t)cmdlist_id);
	if (phy_addr == 0) {
		dpu_pr_err("invalid active cmdlist %u, commit failed\n", cmdlist_id);
		return -1;
	}
	/* 0x1DC, used for send dcs cmds with frame */
	outp32(DPU_DM_LAYER_START_ADDR3_H_ADDR(comp_mgr->dpu_base +
		g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0), phy_addr);

	/* update register */
	outp32(DPU_DM_LAYER_OV_PATTERN_RGB_ADDR(comp_mgr->dpu_base +
		g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0), 1);

	pipeline_next_ops_handle(pinfo->conn_device, pinfo, TRACE_SCREEN_BL, &bl_ctrl->bl_level);

	dpu_pr_info("send_dcs_cmds success, bl_level = %u\n", bl_ctrl->bl_level);
	return 0;
}

int32_t dfr_te_skip_send_dcs_cmds_by_riscv(struct dpu_comp_dfr_ctrl *dfr_ctrl, uint32_t bl_level)
{
	struct dpu_composer *dpu_comp = NULL;
	struct dfr_te_skip_ctrl *priv = NULL;
	struct composer_manager *comp_mgr = NULL;
	struct dfr_info *dinfo = NULL;
	uint32_t pre_safe_frm_rate = 0;
	struct comp_online_present *present = NULL;
	struct dkmd_connector_info *pinfo;

	dpu_check_and_return(!dfr_ctrl, -1, err, "dfr_ctrl is null");
	present = (struct comp_online_present *)dfr_ctrl->dpu_comp->present_data;
	dpu_check_and_return(!present, -1, err, "present is null");
	dinfo = dkmd_get_dfr_info(dfr_ctrl->dpu_comp->conn_info);
	dpu_check_and_return(!dinfo, -1, err, "dfr info is null");

	dpu_comp = dfr_ctrl->dpu_comp;
	priv = dfr_ctrl->priv_data;
	dpu_check_and_return(!dpu_comp, -1, err, "dpu_comp is null");
	dpu_check_and_return(!priv, -1, err, "priv is null");
	comp_mgr = dpu_comp->comp_mgr;
	pinfo = dpu_comp->conn_info;

	if (dinfo->oled_info.ltpo_info.dimming_enable)
		composer_mgr_enable_dimming(comp_mgr, dpu_comp->comp.index);

	dpu_pr_debug("update dcs cmds register is true");

	/* when idle, need send safe frm rate by backlight setting */
	pre_safe_frm_rate = inp32(DPU_DM_LAYER_DMA_SEL_ADDR(comp_mgr->dpu_base +
		g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 0));
	if (pre_safe_frm_rate != 0 && dfr_ctrl->cur_safe_frm_rate != pre_safe_frm_rate) {
		dpu_pr_info("update safe frm rate by backlight setting");
		dfr_te_skip_update_safe_frm_rate(dfr_ctrl, priv, comp_mgr, dfr_ctrl->cur_safe_frm_rate);

		/* for the frame rate statistics in maintain module */
		if (dfr_ctrl->cur_safe_frm_rate > 0) {
			present->comp_maintain.self_refresh_period_us = PERIOD_US_1HZ / dfr_ctrl->cur_safe_frm_rate;
		} else {
			dpu_pr_warn("meet invalide safe frame rate(%u) when set it to maintain!", dfr_ctrl->cur_safe_frm_rate);
		}
	}

	pipeline_next_ops_handle(pinfo->conn_device, pinfo, SET_BACKLIGHT, &bl_level);
	return 0;
}

int32_t dfr_te_skip_set_safe_frm_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl, uint32_t safe_frm_rate)
{
	struct composer_manager *comp_mgr = NULL;
	struct dfr_te_skip_ctrl *priv = NULL;
	dma_addr_t phy_addr;
	int32_t frm_index;
	int32_t cmdlist_id;
	struct comp_online_present *present = NULL;

	dpu_check_and_return(!dfr_ctrl, -1, err, "dfr_ctrl is null");
	dpu_check_and_return(!dfr_ctrl->dpu_comp, -1, err, "dpu_composer is null");
	dpu_check_and_return(!dfr_ctrl->dpu_comp->comp_mgr, -1, err, "comp mgr is null");
	dpu_check_and_return(!dfr_ctrl->priv_data, -1, err, "priv_data is null");

	present = (struct comp_online_present *)dfr_ctrl->dpu_comp->present_data;
	comp_mgr = dfr_ctrl->dpu_comp->comp_mgr;
	priv = dfr_ctrl->priv_data;
	frm_index = dfr_te_skip_check_safe_frm_rate(dfr_ctrl, safe_frm_rate);
	/* check safe frm_idx */
	if (frm_index == -1) {
		dpu_pr_info("No need to set safe frm rate");
		return 0;
	}

	dpu_pr_info("safe frm rate update, frm rate is %u, frm_index is %u", dfr_ctrl->cur_safe_frm_rate, frm_index);
	cmdlist_id = frm_index + CMDLIST_TYPE_MAX;
	if (priv->header_cmdlist_ids[cmdlist_id] == 0) {
		dpu_pr_warn("safe frm rate cmdlist is not inited");
		return -1;
	}
	phy_addr = cmdlist_get_phy_addr(priv->cmdlist_scene_id, priv->reg_cmdlist_ids[cmdlist_id]);
	if (phy_addr == 0) {
		dpu_pr_warn("invalid safe frm cmdlist id %u", cmdlist_id);
		return -1;
	}

	/* check power & idle */
	down(&comp_mgr->power_sem);
	mutex_lock(&comp_mgr->idle_lock);
	if (!composer_check_power_status(dfr_ctrl->dpu_comp) || dfr_ctrl->dpu_comp->comp_idle_flag > 0 ) {
		dpu_pr_warn("already power off or idle, idle func flag is %u", dfr_ctrl->dpu_comp->comp_idle_flag);
		mutex_unlock(&comp_mgr->idle_lock);
		up(&comp_mgr->power_sem);
		return 0;
	}
	dfr_te_update_safe_frm_rate_to_dacc(dfr_ctrl->cur_safe_frm_rate, priv, comp_mgr, phy_addr);
	mutex_unlock(&comp_mgr->idle_lock);
	up(&comp_mgr->power_sem);

	/* for the frame rate statistics in maintain module */
	if (dfr_ctrl->cur_safe_frm_rate > 0) {
		present->comp_maintain.self_refresh_period_us = PERIOD_US_1HZ / dfr_ctrl->cur_safe_frm_rate;
	} else {
		dpu_pr_warn("meet invalide safe frame rate(%u) when set it to maintain!", dfr_ctrl->cur_safe_frm_rate);
	}

	return 0;
}

void dfr_te_skip_self_refresh(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	uint32_t current_config = 0;
	if (dfr_ctrl && dfr_ctrl->dpu_comp && dfr_ctrl->dpu_comp->comp_mgr &&
	    dfr_ctrl->dpu_comp->comp_mgr->dpu_base) {
		char __iomem *dpu_base = dfr_ctrl->dpu_comp->comp_mgr->dpu_base;
		current_config = inp32(DPU_DM_LAYER_STRIDE1_ADDR(dpu_base + g_dm_tlb_info[DPU_SCENE_OFFLINE_2].dm_data_addr, 0));
		/* bit10 1 self refresh */
		current_config |= BIT(10);
		dpu_pr_info("dfr_te_skip_self_refresh current_config = %u", current_config);
		outp32(DPU_DM_LAYER_STRIDE1_ADDR(dpu_base + g_dm_tlb_info[DPU_SCENE_OFFLINE_2].dm_data_addr, 0), current_config);
	}
}

void dfr_te_skip_enable_dimming(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	uint32_t current_config = 0;
	if (dfr_ctrl && dfr_ctrl->dpu_comp && dfr_ctrl->dpu_comp->comp_mgr &&
	    dfr_ctrl->dpu_comp->comp_mgr->dpu_base) {
		char __iomem *dpu_base = dfr_ctrl->dpu_comp->comp_mgr->dpu_base;
		current_config = inp32(DPU_DM_LAYER_STRIDE1_ADDR(dpu_base + g_dm_tlb_info[DPU_SCENE_OFFLINE_2].dm_data_addr, 0));
		/* bit11 0  enable dimming */
		current_config &= ~(BIT(11));
		dpu_pr_info("dfr_te_skip_enable_dimming current_config = %u", current_config);
		outp32(DPU_DM_LAYER_STRIDE1_ADDR(dpu_base + g_dm_tlb_info[DPU_SCENE_OFFLINE_2].dm_data_addr, 0), current_config);
	}
}

void dfr_te_skip_disable_dimming(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	uint32_t current_config = 0;
	if (dfr_ctrl && dfr_ctrl->dpu_comp && dfr_ctrl->dpu_comp->comp_mgr &&
	    dfr_ctrl->dpu_comp->comp_mgr->dpu_base) {
		char __iomem *dpu_base = dfr_ctrl->dpu_comp->comp_mgr->dpu_base;
		current_config = inp32(DPU_DM_LAYER_STRIDE1_ADDR(dpu_base + g_dm_tlb_info[DPU_SCENE_OFFLINE_2].dm_data_addr, 0));
		/* bit11 1 disable dimming */
		current_config |= BIT(11);
		dpu_pr_info("dfr_te_skip_disable_dimming current_config = %u", current_config);
		outp32(DPU_DM_LAYER_STRIDE1_ADDR(dpu_base + g_dm_tlb_info[DPU_SCENE_OFFLINE_2].dm_data_addr, 0), current_config);
	}
}
static void dfr_te_skip_set_safe_frm_rate_status(struct dpu_comp_dfr_ctrl *dfr_ctrl, bool status, int32_t disable_type)
{
	uint32_t current_config = 0;
	if (dfr_ctrl && dfr_ctrl->dpu_comp && dfr_ctrl->dpu_comp->comp_mgr &&
	    dfr_ctrl->dpu_comp->comp_mgr->dpu_base) {
		char __iomem *dpu_base = dfr_ctrl->dpu_comp->comp_mgr->dpu_base;
		current_config = inp32(DPU_DM_LAYER_STRIDE1_ADDR(dpu_base + g_dm_tlb_info[DPU_SCENE_OFFLINE_2].dm_data_addr, 0));
		/* bit12: safe frm rate status 0: enable; 1: disable */
		status ? (current_config &= ~BIT(12)) : (current_config |= BIT(12));
		dpu_pr_info("set safe frm rate status, current_config = %u", current_config);
		outp32(DPU_DM_LAYER_STRIDE1_ADDR(dpu_base + g_dm_tlb_info[DPU_SCENE_OFFLINE_2].dm_data_addr, 0), current_config);
		/* enable safe frm rate, set safe frm rate right now */
		if (status) {
			outp32(DPU_DM_LAYER_START_ADDR2_L_ADDR(dpu_base + g_dm_tlb_info[DPU_SCENE_OFFLINE_2].dm_data_addr, 0), 1);
		} else if (disable_type == LHBM_DISABLE) {
			/* lhbm, bit13: notify dacc min frm rate is 120 */
			current_config |= BIT(13);
			dpu_pr_info("notify dacc min frm rate updated");
			outp32(DPU_DM_LAYER_STRIDE1_ADDR(dpu_base + g_dm_tlb_info[DPU_SCENE_OFFLINE_2].dm_data_addr, 0), current_config);
		}
	}
}

void dfr_te_skip_enable_safe_frm_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	dfr_te_skip_set_safe_frm_rate_status(dfr_ctrl, true, INVALID_DISABLE);
}

void dfr_te_skip_disable_safe_frm_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl, int32_t disable_type)
{
	dfr_te_skip_set_safe_frm_rate_status(dfr_ctrl, false, disable_type);
}
