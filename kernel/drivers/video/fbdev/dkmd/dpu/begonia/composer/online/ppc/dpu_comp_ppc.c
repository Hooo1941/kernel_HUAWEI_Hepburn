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
#include "dpu_comp_ppc.h"
#include "dpu_panel_partial_ctl.h"
#include "dkmd_log.h"
#include "dkmd_notify.h"

void dpu_comp_ppc_ctrl_setup(struct dpu_composer *dpu_comp, struct comp_online_present *present)
{
	struct dpu_comp_ppc_ctrl* ppc_ctrl = &present->ppc_ctrl;
	dpu_pr_info("+");

	if (!is_ppc_support(&(dpu_comp->conn_info->base)))
		return;

	ppc_ctrl->dpu_comp = dpu_comp;

	ppc_ctrl->setup_data = dpu_ppc_setup_priv_data;
	ppc_ctrl->release_data = dpu_ppc_release_priv_data;
	ppc_ctrl->set_active_rect = dpu_ppc_set_active_rect;

	return;
}

int32_t dpu_comp_preprocess_set_active_rect(struct dpu_composer *dpu_comp, uint32_t ppc_config_id, bool *is_need_switch)
{
	bool alsc_need_update = false;
	struct dkmd_connector_info *pinfo = dpu_comp->conn_info;
	struct dkmd_event event = {0};

	if (!pinfo) {
		dpu_pr_err("pinfo is null");
		return -1;
	}

	if (!is_ppc_support(&pinfo->base)) {
		dpu_pr_warn("isnot support ppc");
		return -1;
	}

	dpu_pr_debug("ppc_config_id_record=%d ppc_config_id=%d ppc_config_id_active=%d",
		pinfo->ppc_config_id_record, ppc_config_id, pinfo->ppc_config_id_active);

	// M2G or G2M, ALSC sensor coordinate need convert
	if ((pinfo->ppc_config_id_record == PPC_CONFIG_ID_G_MODE && ppc_config_id == PPC_CONFIG_ID_M_MODE) ||
		(pinfo->ppc_config_id_record == PPC_CONFIG_ID_M_MODE && ppc_config_id == PPC_CONFIG_ID_G_MODE))
		alsc_need_update = true;

	pinfo->ppc_config_id_record = ppc_config_id;

	event.data = NULL;
	event.value = pinfo->ppc_config_id_record;
	dkmd_notifier_call_chain(DKMD_EVENT_SET_ACTIVE_RECT, (void *)&event);

	if (alsc_need_update)
		dpu_effect_update_alsc_coord(dpu_comp->alsc);

	if (!dpu_comp->comp.power_on) {
		dpu_pr_warn("comp %u is power off ppc_config_id:%u", dpu_comp->comp.index, ppc_config_id);
		return -1;
	}

	if (pinfo->ppc_config_id_record == pinfo->ppc_config_id_active) {
		dpu_pr_warn("input ppc_config_id_record:%u,dkmd ppc_config_id_active:%u neednot switch",
			pinfo->ppc_config_id_record, pinfo->ppc_config_id_active);
		return 0;
	}

	*is_need_switch = true;

	return 0;
}
