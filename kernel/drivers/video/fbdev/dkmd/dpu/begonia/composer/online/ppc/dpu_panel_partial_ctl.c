/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2023. All rights reserved.
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

#include "dpu/soc_dpu_interface.h"
#include "dpu_panel_partial_ctl.h"
#include "dkmd_log.h"
#include "dkmd_peri.h"
#include "cmdlist_interface.h"
#include "dkmd_cmdlist.h"
#include "panel_mgr.h"
#include "dpu_conn_mgr.h"
#include "dpu_connector.h"
#include "dpu_comp_config_utils.h"
#include "dpu_ppc_status_control.h"

#define DLEN_MAX 1024
#define PPC_SET_ACTIVE_RECT_TIMEOUT_MS 150
#define PPC_SET_ACTIVE_RECT_WAIT_CNT 3

static int32_t dpu_init_cmdlist(struct dpu_comp_ppc_ctrl *ppc_ctrl, uint32_t ppc_config_id, uint32_t dsi_idx)
{
	int32_t ret;
	struct panel_partial_ctrl *priv = (struct panel_partial_ctrl *)ppc_ctrl->priv_data;

	dpu_check_and_return(ppc_config_id > PPC_CONFIG_ID_TOTAL_CMDLIST_CNT, -1, err, "invalid cmdlist id");

	priv->header_cmdlist_ids[ppc_config_id][dsi_idx] =
		cmdlist_create_user_client(priv->cmdlist_scene_id, SCENE_NOP_TYPE, 0, 0);
	if (unlikely(priv->header_cmdlist_ids[ppc_config_id][dsi_idx] == 0)) {
		dpu_pr_err("scene_id=%u, create header cmdlist fail", priv->cmdlist_scene_id);
		return -1;
	}

	priv->reg_cmdlist_ids[ppc_config_id][dsi_idx] =
		cmdlist_create_user_client(priv->cmdlist_scene_id, REGISTER_CONFIG_TYPE, 0, PAGE_SIZE);
	if (unlikely(priv->reg_cmdlist_ids[ppc_config_id][dsi_idx] == 0)) {
		dkmd_cmdlist_release_locked(priv->cmdlist_scene_id, priv->header_cmdlist_ids[ppc_config_id][dsi_idx]);
		dpu_pr_err("scene_id=%u create reg cmdlist fail", priv->cmdlist_scene_id);
		return -1;
	}

	dpu_pr_info("ppc_config_id:%u, dsi_idx:%u, reg_cmdlist_ids %u",
				ppc_config_id, dsi_idx, priv->reg_cmdlist_ids[ppc_config_id][dsi_idx]);

	ret = cmdlist_append_client(priv->cmdlist_scene_id,
		priv->header_cmdlist_ids[ppc_config_id][dsi_idx], priv->reg_cmdlist_ids[ppc_config_id][dsi_idx]);
	if (unlikely(ret != 0)) {
		dkmd_cmdlist_release_locked(priv->cmdlist_scene_id, priv->header_cmdlist_ids[ppc_config_id][dsi_idx]);
		dpu_pr_err("append reg(%u) cmdlist fail", priv->reg_cmdlist_ids[ppc_config_id][dsi_idx]);
		return -1;
	}

	return 0;
}

static int32_t cmdlist_mipi_dsi_swrite_to(struct dsi_cmd_desc *cm, uint32_t dsi_offset,
	uint32_t cmdlist_id, uint32_t scene_id)
{
	uint32_t hdr = 0;
	uint32_t len;

	if ((cm->dlen != 0) && (cm->payload == 0)) {
		dpu_pr_err("NO payload error!");
		return 0;
	}

	/* mipi dsi short write with 0, 1 2 parameters, total 3 param */
	if (cm->dlen > 2) {
		dpu_pr_err("cm->dlen is invalid");
		return -1;
	}
	len = cm->dlen;

	hdr |= dsi_hdr_dtype(cm->dtype);
	hdr |= dsi_hdr_vc(cm->vc);
	if (len == 1) {
		dpu_pr_info("len = 1, data1 = %#x!\n", (uint32_t)(cm->payload[0]));
		hdr |= dsi_hdr_data1((uint32_t)(cm->payload[0]));
		hdr |= dsi_hdr_data2(0);
	} else if (len == 2) {
		dpu_pr_info("len = 2, data1 = %#x data2 = %#x!\n", (uint32_t)(cm->payload[0]), (uint32_t)(cm->payload[1]));
		hdr |= dsi_hdr_data1((uint32_t)(cm->payload[0]));
		hdr |= dsi_hdr_data2((uint32_t)(cm->payload[1]));
	} else {
		dpu_pr_info("len = ?\n");
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
			dpu_pr_info("pld1 = %#x!\n", pld);
		} else {
			for (j = i; j < cm->dlen; j++)
				pld |= (((uint32_t)cm->payload[j] & 0x0ff) << ((j - i) * 8));
			dpu_pr_info("pld2 = %#x!\n", pld);
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

static int32_t dpu_write_dcs_cmds_to_cmdlist(uint32_t dsi_offset, struct dpu_ppc_config_id_cmds *dcs_cmds,
		uint32_t start_idx, uint32_t cmd_cnt, uint32_t cmdlist_id, uint32_t scene_id)
{
	uint32_t i = start_idx;
	uint32_t cnt = 0;

	dpu_check_and_return(!dcs_cmds, -1, err, "dcs_cmds is null");
	dpu_pr_info("dcs_cmds->cnt:%u", dcs_cmds->cnt);
	cnt = start_idx + cmd_cnt < dcs_cmds->cnt ? start_idx + cmd_cnt : dcs_cmds->cnt;

	if (i >= cnt) {
		dpu_pr_info("start_idx is beyond cnt. start_idx: %u, cnt:%u", i, cnt);
		return -1;
	}

	dpu_pr_info("start_idx: %u, cnt:%u", i, cnt);

	for (; i < cnt; ++i) {
		if (dcs_cmds->cmds[i].dtype == DTYPE_DCS_WRITE || dcs_cmds->cmds[i].dtype == DTYPE_DCS_WRITE1) {
			cmdlist_mipi_dsi_swrite_to(&dcs_cmds->cmds[i], dsi_offset, cmdlist_id, scene_id);
		} else if (dcs_cmds->cmds[i].dtype == DTYPE_GEN_LWRITE || dcs_cmds->cmds[i].dtype == DTYPE_DCS_LWRITE) {
			cmdlist_mipi_dsi_lwrite_to(&dcs_cmds->cmds[i], dsi_offset, cmdlist_id, scene_id);
		} else {
			dpu_pr_err("invalid dtype:%d", dcs_cmds->cmds[i].dtype);
			return -1;
		}
	}

	return 0;
}

static int32_t dpu_get_connector_id_sec_part(uint32_t ppc_config_id)
{
	if (ppc_config_id <= PPC_CONFIG_ID_F_MODE_SECOND_PART_IDX)
		return PPC_CONFIG_ID_F_MODE;
	if (ppc_config_id <= PPC_CONFIG_ID_M_MODE_SECOND_PART_IDX)
		return PPC_CONFIG_ID_M_MODE;
	return PPC_CONFIG_ID_G_MODE;
}


static int32_t dpu_set_cmdlist_reg(struct dpu_comp_ppc_ctrl *ppc_ctrl, uint32_t ppc_config_id, uint32_t dsi_idx)
{
	struct dpu_connector *connector = NULL;
	struct dpu_panel_ops *entry_pops = NULL;
	struct dpu_panel_info *panel_info = NULL;
	struct panel_partial_ctrl *priv = (struct panel_partial_ctrl *)ppc_ctrl->priv_data;
	struct dkmd_connector_info *pinfo = ppc_ctrl->dpu_comp->conn_info;
	uint32_t connector_id = 0, real_connector_id = 0, start_idx = 0, part_idx = 0;
	int32_t ret = 0;

	dpu_check_and_return(ppc_config_id > PPC_CONFIG_ID_TOTAL_CMDLIST_CNT, -1, err, " is invalid");
	dpu_check_and_return(!pinfo, -1, err, "pinfo is null");

	entry_pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_return(!entry_pops, -1, err, "entry pops is null\n");

	panel_info = entry_pops->get_panel_info();
	dpu_check_and_return(!panel_info, -1, err, "panel info is null\n");

	part_idx = ppc_config_id < PPC_CONFIG_ID_FIRST_PART_MAX_IDX ?
								PPC_CONFIG_ID_1ST_PART_CMD : PPC_CONFIG_ID_2ND_PART_CMD;

	if (ppc_config_id <= PPC_CONFIG_ID_F_MODE_7TH_PART_IDX) {
		real_connector_id = PPC_CONFIG_ID_F_MODE;
		start_idx = DCS_CMD_MAX_CNT_FOR_CMDLIST * (ppc_config_id - PPC_CONFIG_ID_F_MODE_1ST_PART_IDX);
	} else if (ppc_config_id <= PPC_CONFIG_ID_M_MODE_7TH_PART_IDX) {
		real_connector_id = PPC_CONFIG_ID_M_MODE;
		start_idx = DCS_CMD_MAX_CNT_FOR_CMDLIST * (ppc_config_id - PPC_CONFIG_ID_M_MODE_1ST_PART_IDX);
	} else if (ppc_config_id <= PPC_CONFIG_ID_G_MODE_7TH_PART_IDX) {
		real_connector_id = PPC_CONFIG_ID_G_MODE;
		start_idx = DCS_CMD_MAX_CNT_FOR_CMDLIST * (ppc_config_id - PPC_CONFIG_ID_G_MODE_1ST_PART_IDX);
	} else if (ppc_config_id < PPC_CONFIG_ID_TOTAL_CMDLIST_CNT) {
		real_connector_id = dpu_get_connector_id_sec_part(ppc_config_id);
		start_idx = 0;
	} 

	connector = get_primary_connector(pinfo);
	dpu_check_and_return(!connector, -1, err, "connector is null");

	if (unlikely(connector->connector_id >= CONNECTOR_ID_MAX)) {
		dpu_pr_warn("connector->connector_id=%u is out of range", connector->connector_id);
		return -1;
	}

	if (dsi_idx == PPC_1ST_DSI_IDX)
		connector_id = connector->connector_id;
	else if (dsi_idx == PPC_2ND_DSI_IDX)
		connector_id = connector->bind_connector->connector_id;
	else
		dpu_pr_err("invalid dsi index:%u", dsi_idx);

	dpu_pr_info("connector->connector_id:%u, part_idx:%u, ppc_config_id:%u, dsi_idx:%u",
					connector->connector_id, part_idx, ppc_config_id, dsi_idx);

	ret = dpu_write_dcs_cmds_to_cmdlist(g_connector_offset[connector_id],
			&(panel_info->ppc_config_id_dsi_cmds[real_connector_id][dsi_idx].dsi_cmds[part_idx]), start_idx,
			DCS_CMD_MAX_CNT_FOR_CMDLIST, priv->reg_cmdlist_ids[ppc_config_id][dsi_idx], priv->cmdlist_scene_id);
	if (ret != 0)
		return -1;

	if ((ppc_config_id < PPC_CONFIG_ID_FIRST_PART_MAX_IDX) && (dsi_idx == 0)) {
		dpu_pr_warn("real_conn_id:%u, cmdlist_cnt:%u", real_connector_id,
				priv->first_part_cmdlist_cnt[real_connector_id]);
		++(priv->first_part_cmdlist_cnt[real_connector_id]);
	}

	return 0;
}

static int32_t dpu_ppc_init_cmdlist(struct dpu_comp_ppc_ctrl *ppc_ctrl)
{
	struct dpu_connector *connector = NULL;
	struct dkmd_connector_info *pinfo = ppc_ctrl->dpu_comp->conn_info;
	uint32_t ppc_config_id = 0;
	dpu_pr_info("+");

	connector = get_primary_connector(pinfo);
	for (ppc_config_id = 0; ppc_config_id < PPC_CONFIG_ID_TOTAL_CMDLIST_CNT; ++ppc_config_id) {
		dpu_init_cmdlist(ppc_ctrl, ppc_config_id, PPC_1ST_DSI_IDX);
		dpu_set_cmdlist_reg(ppc_ctrl, ppc_config_id, PPC_1ST_DSI_IDX);
		if (connector->bind_connector) {
			dpu_pr_debug("write ddic cmds to dual mipi");
			dpu_init_cmdlist(ppc_ctrl, ppc_config_id, PPC_2ND_DSI_IDX);
			dpu_set_cmdlist_reg(ppc_ctrl, ppc_config_id, PPC_2ND_DSI_IDX);
		}
	}

	return 0;
}

static int32_t panel_partial_ctrl_cmd_done_isr_notify(struct notifier_block *self,
	unsigned long action, void *data)
{
	struct dkmd_listener_data *listener_data = (struct dkmd_listener_data *)data;
	struct dpu_comp_ppc_ctrl *ppc_ctrl = (struct dpu_comp_ppc_ctrl *)(listener_data->data);

	dpu_pr_info("+");

	ppc_ctrl->dacc_send_cmd_done = 1;

	if (ppc_ctrl->dacc_send_cmd_done)
		wake_up_interruptible_all(&(ppc_ctrl->wait));

	return 0;
}

static struct notifier_block panel_partial_ctrl_cmd_done_isr_notifier = {
	.notifier_call = panel_partial_ctrl_cmd_done_isr_notify,
};

static void panel_partial_ctrl_cmd_done_listener(struct dpu_comp_ppc_ctrl *ppc_ctrl)
{
	struct dkmd_isr *isr_ctrl = &ppc_ctrl->dpu_comp->comp_mgr->mdp_isr_ctrl;
	struct panel_partial_ctrl *priv = (struct panel_partial_ctrl *)ppc_ctrl->priv_data;

	dpu_pr_info("+");

	priv->notifier = &panel_partial_ctrl_cmd_done_isr_notifier;

	dkmd_isr_register_listener(isr_ctrl, priv->notifier, NOTIFY_PPC_DONE, ppc_ctrl);
}

struct panel_partial_ctrl g_panel_partial_ctrl = { 0 };

static void dpu_ppc_get_panel_estv_info(struct dpu_comp_ppc_ctrl *ppc_ctrl)
{
	struct dpu_panel_ops *entry_pops = NULL;
	struct dpu_panel_info *panel_info = NULL;
	struct panel_partial_ctrl *priv = (struct panel_partial_ctrl *)ppc_ctrl->priv_data;
	struct dkmd_connector_info *pinfo = ppc_ctrl->dpu_comp->conn_info;

	dpu_check_and_no_retval(!pinfo, err, "pinfo is null");

	entry_pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_no_retval(!entry_pops, err, "entry pops is null\n");

	panel_info = entry_pops->get_panel_info();
	dpu_check_and_no_retval(!panel_info, err, "panel info is null\n");

	priv->ppc_enable_panel_estv_support = panel_info->ppc_enable_panel_estv_support;
	priv->ppc_enable_panel_estv_support_factory = panel_info->ppc_enable_panel_estv_support_factory;
	priv->ppc_panel_estv_wait_te_cnt = panel_info->ppc_panel_estv_wait_te_cnt;

	dpu_pr_info("panel_estv_support:%u, panel_estv_support_factory:%u, panel_estv_wait_te_cnt:%u",
		priv->ppc_enable_panel_estv_support,
		priv->ppc_enable_panel_estv_support_factory,
		priv->ppc_panel_estv_wait_te_cnt);
}

int32_t dpu_ppc_setup_priv_data(struct dpu_comp_ppc_ctrl *ppc_ctrl)
{
	struct panel_partial_ctrl *priv;
	char __iomem *dpu_base = NULL;
	struct dpu_connector *connector = NULL;
	struct dkmd_connector_info *pinfo = NULL;
	int i;

	dpu_check_and_return(!ppc_ctrl, -1, err, "ppc_ctrl is null");
	dpu_check_and_return(!ppc_ctrl->dpu_comp, -1, err, "dpu_comp is null");
	dpu_check_and_return(!ppc_ctrl->dpu_comp->comp_mgr, -1, err, "comp_mgr is null");
	pinfo = ppc_ctrl->dpu_comp->conn_info;

	dpu_pr_info("index:%u", ppc_ctrl->dpu_comp->comp.index);

	if (ppc_ctrl->dpu_comp->comp.index != DEVICE_COMP_PRIMARY_ID) {
		dpu_pr_info("no need init ppc. index:%u", ppc_ctrl->dpu_comp->comp.index);
		return 0;
	}

	connector = get_primary_connector(pinfo);
	dpu_base = ppc_ctrl->dpu_comp->comp_mgr->dpu_base;

	ppc_ctrl->priv_data = (void*)&g_panel_partial_ctrl;
	priv = (struct panel_partial_ctrl *)ppc_ctrl->priv_data;
	priv->assist_scene_id = DPU_SCENE_OFFLINE_2;
	priv->cmdlist_scene_id = DPU_SCENE_ONLINE_3;
	priv->first_part_cmdlist_cnt[PPC_CONFIG_ID_F_MODE] = 0;
	priv->first_part_cmdlist_cnt[PPC_CONFIG_ID_M_MODE] = 0;
	priv->first_part_cmdlist_cnt[PPC_CONFIG_ID_G_MODE] = 0;
	ppc_ctrl->dacc_send_cmd_done = 0;
	// ppc status can be switched before first present of dpu power on
	process_ppc_event(ppc_ctrl->dpu_comp, PPC_EVENT_DPU_POWER_ON);
	init_waitqueue_head(&ppc_ctrl->wait);
	init_waitqueue_head(&ppc_ctrl->ppc_cmd_start_wq);

	if (dpu_ppc_init_cmdlist(ppc_ctrl) != 0)
		return -1;

	/* set default F mode phy_addr */
	for (i = 0; i < DCS_CMDLIST_PACKET_NUM; ++i) {
		priv->cmdlist_phy_addr[i][PPC_1ST_DSI_IDX] = cmdlist_get_phy_addr(priv->cmdlist_scene_id,
								priv->reg_cmdlist_ids[i][PPC_1ST_DSI_IDX]);
		if (connector->bind_connector)
			priv->cmdlist_phy_addr[i][PPC_2ND_DSI_IDX] = cmdlist_get_phy_addr(priv->cmdlist_scene_id,
									priv->reg_cmdlist_ids[i][PPC_2ND_DSI_IDX]);
	}

	priv->cmdlist_phy_addr[PPC_CONFIG_ID_F_MODE_SECOND_PART_IDX][PPC_1ST_DSI_IDX] =
		cmdlist_get_phy_addr(priv->cmdlist_scene_id,
		priv->reg_cmdlist_ids[PPC_CONFIG_ID_F_MODE_SECOND_PART_IDX][PPC_1ST_DSI_IDX]);
	priv->cmdlist_phy_addr[PPC_CONFIG_ID_F_MODE_SECOND_PART_IDX][PPC_2ND_DSI_IDX] =
		cmdlist_get_phy_addr(priv->cmdlist_scene_id,
		priv->reg_cmdlist_ids[PPC_CONFIG_ID_F_MODE_SECOND_PART_IDX][PPC_2ND_DSI_IDX]);

	if ((priv->cmdlist_phy_addr[PPC_CONFIG_ID_F_MODE_SECOND_PART_IDX][PPC_1ST_DSI_IDX] == 0 ) ||
		(priv->cmdlist_phy_addr[PPC_CONFIG_ID_F_MODE_SECOND_PART_IDX][PPC_2ND_DSI_IDX] == 0)) {
		dpu_pr_err("invalid TOTAL_CMDLIST_CNT %d, id1:%d, id2:%d, commit failed\n", PPC_CONFIG_ID_FIRST_PART_MAX_IDX,
				priv->reg_cmdlist_ids[PPC_CONFIG_ID_F_MODE_SECOND_PART_IDX][PPC_CONFIG_ID_1ST_PART_CMD],
				priv->reg_cmdlist_ids[PPC_CONFIG_ID_F_MODE_SECOND_PART_IDX][PPC_CONFIG_ID_2ND_PART_CMD]);
		return -1;
	}

	dpu_ppc_get_panel_estv_info(ppc_ctrl);

	/* enable ppc */
	dpu_ppc_init_interactive_reg(dpu_base, priv);

	/* init 1st part */
	dpu_ppc_set_1st_part_cmd_addr(dpu_base, priv, PPC_CONFIG_ID_F_MODE_1ST_PART_IDX);

	/* init 2nd part */
	dpu_ppc_set_2nd_part_cmd_addr(dpu_base, priv, PPC_CONFIG_ID_F_MODE_SECOND_PART_IDX);

	panel_partial_ctrl_cmd_done_listener(ppc_ctrl);

	return 0;
}

void dpu_ppc_release_priv_data(struct dpu_comp_ppc_ctrl *ppc_ctrl)
{
	struct dpu_composer *dpu_comp = NULL;
	struct panel_partial_ctrl *priv = NULL;
	struct dkmd_isr *isr_ctrl = NULL;
	char __iomem *dpu_base = NULL;
	struct dkmd_connector_info *pinfo = NULL;
	struct dpu_connector *connector = NULL;
	uint32_t ppc_config_id;

	dpu_check_and_no_retval(!ppc_ctrl, err, "ppc_ctrl is null");
	dpu_check_and_no_retval(!ppc_ctrl->dpu_comp, err, "dpu_comp is null");
	dpu_check_and_no_retval(!ppc_ctrl->priv_data, err, "ppc_ctrl->priv_data is null");

	pinfo = ppc_ctrl->dpu_comp->conn_info;
	dpu_pr_info("+");

	process_ppc_event(ppc_ctrl->dpu_comp, PPC_EVENT_DPU_POWER_OFF);

	dpu_comp = ppc_ctrl->dpu_comp;
	priv = (struct panel_partial_ctrl *)ppc_ctrl->priv_data;
	isr_ctrl = &dpu_comp->comp_mgr->mdp_isr_ctrl;
	dpu_base = dpu_comp->comp_mgr->dpu_base;

	connector = get_primary_connector(pinfo);
	for (ppc_config_id = 0; ppc_config_id < PPC_CONFIG_ID_TOTAL_CMDLIST_CNT; ++ppc_config_id) {
		if (priv->header_cmdlist_ids[ppc_config_id][PPC_1ST_DSI_IDX] != 0) {
			dkmd_cmdlist_release_locked(priv->cmdlist_scene_id, priv->header_cmdlist_ids[ppc_config_id][PPC_1ST_DSI_IDX]);
			priv->header_cmdlist_ids[ppc_config_id][PPC_1ST_DSI_IDX] = 0;
		}

		if (connector->bind_connector) {
			dpu_pr_info("release cmdlist for dual mipi");
			if (priv->header_cmdlist_ids[ppc_config_id][PPC_2ND_DSI_IDX] != 0) {
				dkmd_cmdlist_release_locked(priv->cmdlist_scene_id, priv->header_cmdlist_ids[ppc_config_id][PPC_2ND_DSI_IDX]);
				priv->header_cmdlist_ids[ppc_config_id][PPC_2ND_DSI_IDX] = 0;
			}
		}
	}

	/* disable ppc */
	outp32(DPU_DM_LAYER_HEIGHT_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 1), 0);

	dkmd_isr_unregister_listener(isr_ctrl, priv->notifier, NOTIFY_PPC_DONE);
}

int32_t dpu_ppc_set_active_rect(struct dpu_comp_ppc_ctrl *ppc_ctrl, uint32_t ppc_config_id)
{
	struct panel_partial_ctrl *priv = NULL;
	char __iomem *dpu_base = NULL;
	uint32_t first_part_cmdlist_cnt;
	uint32_t real_ppc_config_id = 0;
	uint32_t sec_part_config_id = 0;
	struct dpu_connector *connector = NULL;
	struct dkmd_connector_info *pinfo = NULL;
	int32_t ret;
	uint32_t i;
	uint32_t delay_count = 0;

	dpu_check_and_return(!ppc_ctrl, -1, err, "ppc_ctrl is null");
	dpu_check_and_return(!ppc_ctrl->dpu_comp, -1, err, "dpu_comp is null");
	dpu_check_and_return(!ppc_ctrl->dpu_comp->comp_mgr, -1, err, "comp_mgr is null");

	pinfo = ppc_ctrl->dpu_comp->conn_info;
	dpu_check_and_return(!pinfo, -1, err, "pinfo is null");
	dpu_base = ppc_ctrl->dpu_comp->comp_mgr->dpu_base;
	dpu_pr_info("ppc_config_id:%u", ppc_config_id);

	priv = (struct panel_partial_ctrl *)ppc_ctrl->priv_data;

	if (ppc_config_id == PPC_CONFIG_ID_F_MODE) {
		real_ppc_config_id = PPC_CONFIG_ID_F_MODE_1ST_PART_IDX;
		sec_part_config_id = PPC_CONFIG_ID_F_MODE_SECOND_PART_IDX;
	} else if (ppc_config_id == PPC_CONFIG_ID_M_MODE) {
		real_ppc_config_id = PPC_CONFIG_ID_M_MODE_1ST_PART_IDX;
		sec_part_config_id = PPC_CONFIG_ID_M_MODE_SECOND_PART_IDX;
	} else if (ppc_config_id == PPC_CONFIG_ID_G_MODE) {
		real_ppc_config_id = PPC_CONFIG_ID_G_MODE_1ST_PART_IDX;
		sec_part_config_id = PPC_CONFIG_ID_G_MODE_SECOND_PART_IDX;
	} else {
		dpu_pr_warn("invalid ppc_config_id:%u", ppc_config_id);
		return -1;
	}

	first_part_cmdlist_cnt = priv->first_part_cmdlist_cnt[ppc_config_id];
	dpu_pr_info("con_id:%u, real_con_id:%u, cmdlist_cnt:%u", ppc_config_id, real_ppc_config_id, first_part_cmdlist_cnt);

	outp32(DPU_DM_LAYER_MASK_Y1_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 1),
			first_part_cmdlist_cnt);

	connector = get_primary_connector(pinfo);
	for (i = 0; i < DCS_CMDLIST_PACKET_NUM; ++i) {
		priv->cmdlist_phy_addr[real_ppc_config_id + i][PPC_1ST_DSI_IDX] = cmdlist_get_phy_addr(priv->cmdlist_scene_id,
								priv->reg_cmdlist_ids[real_ppc_config_id + i][PPC_1ST_DSI_IDX]);
		if (connector->bind_connector)
			priv->cmdlist_phy_addr[real_ppc_config_id + i][PPC_2ND_DSI_IDX] = cmdlist_get_phy_addr(priv->cmdlist_scene_id,
									priv->reg_cmdlist_ids[real_ppc_config_id + i][PPC_2ND_DSI_IDX]);
	}

	dpu_ppc_set_1st_part_cmd_addr(dpu_base, priv, real_ppc_config_id);

	priv->cmdlist_phy_addr[sec_part_config_id][PPC_1ST_DSI_IDX] = cmdlist_get_phy_addr(priv->cmdlist_scene_id,
		priv->reg_cmdlist_ids[sec_part_config_id][PPC_1ST_DSI_IDX]);
	if (connector->bind_connector)
		priv->cmdlist_phy_addr[sec_part_config_id][PPC_2ND_DSI_IDX] = cmdlist_get_phy_addr(priv->cmdlist_scene_id,
		priv->reg_cmdlist_ids[sec_part_config_id][PPC_2ND_DSI_IDX]);
	dpu_ppc_set_2nd_part_cmd_addr(dpu_base, priv, sec_part_config_id);

	outp32(DPU_DM_LAYER_MASK_Y0_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 1), 1);

	while (true) {
		ret = (int32_t)wait_event_interruptible_timeout(ppc_ctrl->wait, ppc_ctrl->dacc_send_cmd_done,
			(long)msecs_to_jiffies(PPC_SET_ACTIVE_RECT_TIMEOUT_MS));
		if ((ret == -ERESTARTSYS) && (++delay_count <= PPC_SET_ACTIVE_RECT_WAIT_CNT))
			usleep_range(1000, 1100);
		else
			break;
	}

	dpu_pr_info("ppc_ctrl->dacc_send_cmd_done:%u", ppc_ctrl->dacc_send_cmd_done);
	ppc_ctrl->dacc_send_cmd_done = 0;

	if (ret == 0) {
		dpu_pr_warn("ppc set active rect timeout ret = %d!", ret);
		return -1;
	}

	return 0;
}

#ifdef CONFIG_DKMD_DEBUG_ENABLE

extern struct composer_manager *g_composer_manager;
int32_t dpu_set_active_rect(uint32_t ppc_config_id)
{
	uint32_t i, first_part_cmdlist_cnt;
	uint32_t real_ppc_config_id = 0;
	uint32_t sec_part_config_id = 0;
	char __iomem *dpu_base = NULL;
	struct panel_partial_ctrl *priv = NULL;
	struct dpu_connector *connector =
							get_primary_connector(g_composer_manager->dpu_comps[DEVICE_COMP_PRIMARY_ID]->conn_info);

	dpu_base = g_composer_manager->dpu_base;
	priv = &g_panel_partial_ctrl;

	if (ppc_config_id == PPC_CONFIG_ID_F_MODE) {
		real_ppc_config_id = PPC_CONFIG_ID_F_MODE_1ST_PART_IDX;
		sec_part_config_id = PPC_CONFIG_ID_F_MODE_SECOND_PART_IDX;
	} else if (ppc_config_id == PPC_CONFIG_ID_M_MODE) {
		real_ppc_config_id = PPC_CONFIG_ID_M_MODE_1ST_PART_IDX;
		sec_part_config_id = PPC_CONFIG_ID_M_MODE_SECOND_PART_IDX;
	} else if (ppc_config_id == PPC_CONFIG_ID_G_MODE) {
		real_ppc_config_id = PPC_CONFIG_ID_G_MODE_1ST_PART_IDX;
		sec_part_config_id = PPC_CONFIG_ID_G_MODE_SECOND_PART_IDX;
	} else {
		dpu_pr_warn("invalid ppc_config_id:%u", ppc_config_id);
		return -1;
	}

	first_part_cmdlist_cnt = priv->first_part_cmdlist_cnt[ppc_config_id];
	dpu_pr_info("con_id:%u, real_con_id:%u, cmdlist_cnt:%u", ppc_config_id, real_ppc_config_id,
																first_part_cmdlist_cnt);
	outp32(DPU_DM_LAYER_MASK_Y1_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 1),
			first_part_cmdlist_cnt);

	for (i = 0; i < DCS_CMDLIST_PACKET_NUM; ++i) {
		priv->cmdlist_phy_addr[real_ppc_config_id + i][PPC_1ST_DSI_IDX] = cmdlist_get_phy_addr(priv->cmdlist_scene_id,
								priv->reg_cmdlist_ids[real_ppc_config_id + i][PPC_1ST_DSI_IDX]);
		dpu_pr_info("first dsi cmdlist_phy_addr:%u, reg_cmdlist_ids:%u",
						priv->cmdlist_phy_addr[real_ppc_config_id + i][PPC_1ST_DSI_IDX],
						priv->reg_cmdlist_ids[real_ppc_config_id + i][PPC_1ST_DSI_IDX]);
		if (connector->bind_connector)
			priv->cmdlist_phy_addr[real_ppc_config_id + i][PPC_2ND_DSI_IDX] =
										cmdlist_get_phy_addr(priv->cmdlist_scene_id,
										priv->reg_cmdlist_ids[real_ppc_config_id + i][PPC_2ND_DSI_IDX]);
		dpu_pr_info("second dsi cmdlist_phy_addr:%u, reg_cmdlist_ids:%u",
						priv->cmdlist_phy_addr[real_ppc_config_id + i][PPC_2ND_DSI_IDX],
						priv->reg_cmdlist_ids[real_ppc_config_id + i][PPC_2ND_DSI_IDX]);
	}

	dpu_ppc_set_1st_part_cmd_addr(dpu_base, priv, real_ppc_config_id);

	priv->cmdlist_phy_addr[sec_part_config_id][PPC_1ST_DSI_IDX] = cmdlist_get_phy_addr(priv->cmdlist_scene_id,
		priv->reg_cmdlist_ids[sec_part_config_id][PPC_1ST_DSI_IDX]);
	if (connector->bind_connector)
		priv->cmdlist_phy_addr[sec_part_config_id][PPC_2ND_DSI_IDX] = cmdlist_get_phy_addr(priv->cmdlist_scene_id,
		priv->reg_cmdlist_ids[sec_part_config_id][PPC_2ND_DSI_IDX]);
	dpu_ppc_set_2nd_part_cmd_addr(dpu_base, priv, sec_part_config_id);

	outp32(DPU_DM_LAYER_MASK_Y0_ADDR(dpu_base + g_dm_tlb_info[priv->assist_scene_id].dm_data_addr, 1), 1);

	mdelay(5);

	return 0;
}
EXPORT_SYMBOL(dpu_set_active_rect);

#endif
