/* Copyright (c) 2023-2023, Hisilicon Tech. Co., Ltd. All rights reserved.
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
#include "dpu_effect_alsc.h"
#include "dpu_effect_alsc_debug.h"
#include "dpu_comp_mgr.h"
#include "gfxdev_mgr.h"

struct dpu_alsc g_dpu_alsc[DPU_PANEL_ID_MAX];

static void switch_alsc_status(struct dpu_alsc *alsc, uint32_t status)
{
	alsc->status = status;
}

static struct dpu_alsc *get_alsc_by_panel_id(uint32_t panel_id)
{
	if (panel_id >= DPU_PANEL_ID_MAX)
		return NULL;

	return &g_dpu_alsc[panel_id];
}

static void alsc_degamma_param_debug_print(const struct alsc_degamma_coef *degamma_coef)
{
	uint32_t i = 0;

	dpu_pr_debug("[ALSC] degamma params print begin");

	for (i = 0; i < ALSC_DEGAMMA_COEF_LEN; i++)
		dpu_pr_debug("r %u, g %u, b %u",
			degamma_coef->r_coef[i], degamma_coef->g_coef[i], degamma_coef->b_coef[i]);

	dpu_pr_debug("[ALSC] degamma params print end");
}

static void alsc_block_param_debug_print(const struct dpu_alsc *alsc)
{
	uint32_t i = 0;

	dpu_pr_debug("[ALSC] block params print begin");

	for (i = 0; i < ALSC_BLOCK_LEN; i++)
		dpu_pr_debug("block %u, addr %u, ave %u, coef %u %u %u %u",
			i, alsc->dkmd_alsc.addr_block[i], alsc->dkmd_alsc.ave_block[i], alsc->dkmd_alsc.coef_block_r[i],
			alsc->dkmd_alsc.coef_block_g[i], alsc->dkmd_alsc.coef_block_b[i], alsc->dkmd_alsc.coef_block_c[i]);

	dpu_pr_debug("[ALSC] block params print end");
}

static void alsc_param_debug_print(const struct dpu_alsc *alsc)
{
	dpu_pr_debug("[ALSC]status=%u action=%u "
		"alsc_en_by_dirty_region_limit=%u "
		"degamma_en=%u "
		"degamma_lut_sel=%u "
		"alsc_en=%u "
		"sensor_channel=%u "
		"postion=%#x "
		"size=%#x "
		"bl_param.coef_r2r=%u "
		"bl_param.coef_r2g=%u "
		"bl_param.coef_r2b=%u "
		"bl_param.coef_r2c=%u "
		"bl_param.coef_g2r=%u "
		"bl_param.coef_g2g=%u "
		"bl_param.coef_g2b=%u "
		"bl_param.coef_g2c=%u "
		"bl_param.coef_b2r=%u "
		"bl_param.coef_b2g=%u "
		"bl_param.coef_b2b=%u "
		"bl_param.coef_b2c=%u",
		alsc->status,
		alsc->action,
		alsc->alsc_en_by_dirty_region_limit,
		alsc->degamma_en,
		alsc->degamma_lut_sel,
		alsc->dkmd_alsc.alsc_en,
		alsc->dkmd_alsc.sensor_channel,
		alsc->dkmd_alsc.addr,
		alsc->dkmd_alsc.size,
		alsc->dkmd_alsc.bl_param.coef_r2r,
		alsc->dkmd_alsc.bl_param.coef_r2g,
		alsc->dkmd_alsc.bl_param.coef_r2b,
		alsc->dkmd_alsc.bl_param.coef_r2c,
		alsc->dkmd_alsc.bl_param.coef_g2r,
		alsc->dkmd_alsc.bl_param.coef_g2g,
		alsc->dkmd_alsc.bl_param.coef_g2b,
		alsc->dkmd_alsc.bl_param.coef_g2c,
		alsc->dkmd_alsc.bl_param.coef_b2r,
		alsc->dkmd_alsc.bl_param.coef_b2g,
		alsc->dkmd_alsc.bl_param.coef_b2b,
		alsc->dkmd_alsc.bl_param.coef_b2c);
}

void dpu_effect_update_alsc_coord(struct dpu_alsc *alsc)
{
	uint32_t x = 0;
	uint32_t y = 0;
	struct dkmd_connector_info *pinfo = NULL;
	struct dpu_composer *dpu_comp = NULL;
	struct composer *comp = NULL;
	struct dkmd_rect_coord *record_coord = NULL;
	struct dpu_ppc_config_id_rect_info ppc_rect_info[PPC_CONFIG_ID_CNT] = {0};

	dpu_check_and_no_retval(!alsc, err, "alsc is null!");

	comp = alsc->comp;
	dpu_check_and_no_retval(!comp, err, "comp is null!");

	if (!is_ppc_support(&comp->base)) {
		dpu_pr_debug("do not need update alsc coordinate");
		return;
	}

	if (alsc->dkmd_alsc.alsc_en == 0)
		return;

	dpu_comp = to_dpu_composer(comp);
	dpu_check_and_no_retval(!dpu_comp, err, "dpu_comp is null!");

	pinfo = dpu_comp->conn_info;
	dpu_check_and_no_retval(!pinfo, err, "pinfo is null!");
	dpu_check_and_no_retval(!pinfo->get_panel_ppc_config_id_rect_info,
		err, "get_panel_ppc_config_id_rect_info is null!");

	pinfo->get_panel_ppc_config_id_rect_info(pinfo, ppc_rect_info);
	record_coord = &ppc_rect_info[pinfo->ppc_config_id_record].rect;

	x = (uint32_t)(alsc->alsc_phy_rect.x);
	y = (uint32_t)(alsc->alsc_phy_rect.y);
	if (x < record_coord->left || y < record_coord->top) {
		dpu_pr_warn("do not need to update alsc coordinate, alsc x=%d y=%d record_coord={%d %d %d %d}",
			x, y, record_coord->left, record_coord->top, record_coord->right, record_coord->bottom);
		return;
	}
	dpu_pr_debug("alsc physical sensor x=%d y=%d", x, y);

	x -= record_coord->left;
	y -= record_coord->top;
	alsc->dkmd_alsc.addr = (y << ALSC_ADDR_Y_SHIFT) | x;

	dpu_pr_debug("alsc coordinate x=%d y=%d addr=%u", x, y, alsc->dkmd_alsc.addr);
}

static int32_t alsc_enable(struct dpu_alsc *alsc, const void *data)
{
	const struct dkmd_alsc *in_alsc = (const struct dkmd_alsc *)data;

	dpu_pr_debug("[ALSC]+");

	alsc->dkmd_alsc = *in_alsc;
	alsc->action |= BIT(ALSC_ENABLE);
	alsc->degamma_en = in_alsc->alsc_en;
	alsc->degamma_lut_sel = 0;
	alsc->alsc_en_by_dirty_region_limit = in_alsc->alsc_en;

	alsc->alsc_phy_rect.x = alsc->dkmd_alsc.addr & ALSC_ADDR_X_MASK;
	alsc->alsc_phy_rect.y = (alsc->dkmd_alsc.addr >> ALSC_ADDR_Y_SHIFT) & ALSC_ADDR_X_MASK;
	alsc->alsc_phy_rect.w = (alsc->dkmd_alsc.size & ALSC_SIZE_X_MASK) + 1;
	alsc->alsc_phy_rect.h = ((alsc->dkmd_alsc.size >> ALSC_SIZE_Y_SHIFT) & ALSC_SIZE_X_MASK) + 1;

	dpu_effect_update_alsc_coord(alsc);

	alsc_param_debug_print(alsc);

	alsc_block_param_debug_print(alsc);
	alsc_degamma_param_debug_print(&(alsc->dkmd_alsc.degamma_coef));

	return 0;
}

static int32_t alsc_update_bl_param(struct dpu_alsc *alsc, const void *data)
{
	const struct alsc_bl_param *bl_param = (const struct alsc_bl_param *)data;

	dpu_pr_debug("[ALSC]+");
	alsc->action |= BIT(ALSC_UPDATE_BL);
	alsc->dkmd_alsc.bl_param = *bl_param;
	alsc->bl_update_to_hwc = 1;

	alsc_param_debug_print(alsc);

	return 0;
}

static int32_t alsc_update_degamma_coef(struct dpu_alsc *alsc, const void *data)
{
	const struct alsc_degamma_coef *degamma_coef = (const struct alsc_degamma_coef *)data;

	dpu_pr_debug("[ALSC]+");
	alsc->action |= BIT(ALSC_UPDATE_DEGAMMA);
	alsc->dkmd_alsc.degamma_coef = *degamma_coef;
	alsc_degamma_param_debug_print(&(alsc->dkmd_alsc.degamma_coef));

	/* degamma lut reg attr is RW, need delay one frame
	 * disable alsc first, and after the next frame, enable alsc and configure degamma
	 */
	alsc->action |= BIT(ALSC_DISABLE);

	return 0;
}

static int32_t alsc_blank(struct dpu_alsc *alsc, const void *data)
{
	(void)data;

	if (!alsc) {
		dpu_pr_err("alsc invalid");
		return -EINVAL;
	}

	dpu_pr_debug("[ALSC]+");

	switch_alsc_status(alsc, ALSC_IDLE);
	alsc->data_tail = alsc->data_head;

	return 0;
}

static int32_t alsc_unblank(struct dpu_alsc *alsc, const void *data)
{
	(void)data;

	if (!alsc) {
		dpu_pr_err("alsc invalid");
		return -EINVAL;
	}

	dpu_pr_debug("[ALSC]+");

	switch_alsc_status(alsc, ALSC_UNINIT);
	alsc->action = BIT(ALSC_ENABLE);

	return 0;
}

static void alsc_free_list_node(struct dpu_alsc_data *node)
{
	struct dpu_alsc_data *p = node;
	struct dpu_alsc_data *q = node;

	while (p) {
		q = p->next;
		kfree(p);
		p = q;
	}
}

static void alsc_free_data_storage(struct dpu_alsc *alsc)
{
	if (!(alsc->data_head))
		return;

	/* alsc_data hasn't been a cycle */
	/* head-->node1...-->tail
	 * head<--node1...<--tail
	 *  |                 |
	 *  V                 V
	 * NULL              NULL
	 */
	if (!alsc->data_head->prev) {
		alsc_free_list_node(alsc->data_head);
		return;
	}

	/* alsc_data has been a cycle */
	/* head-->node1...-->tail
	 * head<--node1...<--tail
	 *  |^                |^
	 *  ||________________||
	 *  |__________________|
	 */
	alsc->data_head->prev->next = NULL;
	alsc_free_list_node(alsc->data_head);
}

static int32_t alsc_init_data_storage(struct dpu_alsc *alsc)
{
	struct dpu_alsc_data *p = NULL;
	struct dpu_alsc_data *head = NULL;
	int32_t i = 1;

	head = (struct dpu_alsc_data*)kzalloc(sizeof(struct dpu_alsc_data), GFP_ATOMIC);
	if (!head) {
		dpu_pr_err("[ALSC]alsc_init_data_storage failed");
		return -ENOMEM;
	}

	/* First node of alsc data cycle */
	alsc->data_head = head;
	alsc->data_tail = head;

	/* rest node of alsc data cycle except for last node */
	for (; i < ALSC_MAX_DATA_LEN - 1; ++i) {
		p = (struct dpu_alsc_data*)kzalloc(sizeof(struct dpu_alsc_data), GFP_ATOMIC);
		if (!p) {
			alsc_free_data_storage(alsc);
			dpu_pr_err("[ALSC]alsc_init_data_storage failed");
			return -ENOMEM;
		}
		head->next = p;
		p->prev = head;
		p->next = NULL;
		head = p;
	}

	/* last node of alsc data cycle */
	p = (struct dpu_alsc_data*)kzalloc(sizeof(struct dpu_alsc_data), GFP_ATOMIC);
	if (!p) {
		alsc_free_data_storage(alsc);
		dpu_pr_err("[ALSC]alsc_init_data_storage failed");
		return -ENOMEM;
	}
	head->next = p;
	p->prev = head;
	p->next = alsc->data_head;
	alsc->data_head->prev = p;
	dpu_pr_info("[ALSC]alsc_init_data_storage success");

	return 0;
}

static void dpu_alsc_send_data_work_func(struct work_struct *work)
{
	struct dpu_alsc *alsc = container_of(work, struct dpu_alsc, alsc_send_data_work);
	struct dpu_alsc_data *node = NULL;
	struct alsc_noise data_to_send;

	if (!alsc) {
		dpu_pr_err("[ALSC]alsc is NULL\n");
		return;
	}

	mutex_lock(&(alsc->alsc_lock));
	node = alsc->data_tail;
	if (!node) {
		dpu_pr_err("[ALSC]ALSC data node is NULL\n");
		mutex_unlock(&(alsc->alsc_lock));
		return;
	}

	data_to_send.status = ALSC_NOISE_VALID;
	data_to_send.noise1 = node->noise.noise1;
	data_to_send.noise2 = node->noise.noise2;
	data_to_send.noise3 = node->noise.noise3;
	data_to_send.noise4 = node->noise.noise4;
	data_to_send.timestamp = node->ktimestamp;

	/* data tail node move */
	alsc->data_tail = alsc->data_tail->next;
	mutex_unlock(&(alsc->alsc_lock));

	if (alsc->cb_func.send_data_func)
		alsc->cb_func.send_data_func(&data_to_send, alsc->panel_id);
}

static void alsc_bl_param_set_reg(const struct dpu_alsc *alsc)
{
	char __iomem *alsc_base = alsc->alsc_base;
	const struct alsc_bl_param *bl_param = &alsc->dkmd_alsc.bl_param;

	outp32(DPU_DPP_COEF_R2R_ADDR(alsc_base), bl_param->coef_r2r);
	outp32(DPU_DPP_COEF_R2G_ADDR(alsc_base), bl_param->coef_r2g);
	outp32(DPU_DPP_COEF_R2B_ADDR(alsc_base), bl_param->coef_r2b);
	outp32(DPU_DPP_COEF_R2C_ADDR(alsc_base), bl_param->coef_r2c);
	outp32(DPU_DPP_COEF_G2R_ADDR(alsc_base), bl_param->coef_g2r);
	outp32(DPU_DPP_COEF_G2G_ADDR(alsc_base), bl_param->coef_g2g);
	outp32(DPU_DPP_COEF_G2B_ADDR(alsc_base), bl_param->coef_g2b);
	outp32(DPU_DPP_COEF_G2C_ADDR(alsc_base), bl_param->coef_g2c);
	outp32(DPU_DPP_COEF_B2R_ADDR(alsc_base), bl_param->coef_b2r);
	outp32(DPU_DPP_COEF_B2G_ADDR(alsc_base), bl_param->coef_b2g);
	outp32(DPU_DPP_COEF_B2B_ADDR(alsc_base), bl_param->coef_b2b);
	outp32(DPU_DPP_COEF_B2C_ADDR(alsc_base), bl_param->coef_b2c);
}

static void alsc_block_param_set_reg(const struct dpu_alsc *alsc)
{
	int32_t block_cnt = 0;
	char __iomem *alsc_base = alsc->alsc_base;
	const struct dkmd_alsc *dkmd_alsc = &alsc->dkmd_alsc;

	for (; block_cnt < ALSC_BLOCK_LEN; ++block_cnt) {
		outp32(DPU_DPP_ADDR_BLOCK_ADDR(alsc_base, block_cnt), dkmd_alsc->addr_block[block_cnt]);
		outp32(DPU_DPP_AVE_BLOCK_ADDR(alsc_base, block_cnt), dkmd_alsc->ave_block[block_cnt]);
		outp32(DPU_DPP_COEF_BLOCK_R_ADDR(alsc_base, block_cnt), dkmd_alsc->coef_block_r[block_cnt]);
		outp32(DPU_DPP_COEF_BLOCK_G_ADDR(alsc_base, block_cnt), dkmd_alsc->coef_block_g[block_cnt]);
		outp32(DPU_DPP_COEF_BLOCK_B_ADDR(alsc_base, block_cnt), dkmd_alsc->coef_block_b[block_cnt]);
		outp32(DPU_DPP_COEF_BLOCK_C_ADDR(alsc_base, block_cnt), dkmd_alsc->coef_block_c[block_cnt]);
	}
}

static void alsc_degamma_coef_set_reg(const struct dpu_alsc *alsc)
{
	uint32_t block_cnt = 0;
	uint32_t val;
	char __iomem *alsc_base = alsc->alsc_base;
	const struct alsc_degamma_coef *degamma_coef = &alsc->dkmd_alsc.degamma_coef;

	for (; block_cnt < ALSC_DEGAMMA_COEF_LEN - 1; block_cnt += 0x2) {
		val = (degamma_coef->r_coef[block_cnt] & ALSC_DEGAMMA_VAL_MASK);
		val |= ((degamma_coef->r_coef[block_cnt + 1] & ALSC_DEGAMMA_VAL_MASK) << ALSC_DEGAMMA_ODD_COEF_START_BIT);
		outp32(DPU_DPP_U_ALSC_DEGAMMA_R_COEF_ADDR(alsc_base, (block_cnt >> 1)), val);

		val = (degamma_coef->g_coef[block_cnt] & ALSC_DEGAMMA_VAL_MASK);
		val |= ((degamma_coef->g_coef[block_cnt + 1] & ALSC_DEGAMMA_VAL_MASK) << ALSC_DEGAMMA_ODD_COEF_START_BIT);
		outp32(DPU_DPP_U_ALSC_DEGAMM_G_COEF_ADDR(alsc_base, (block_cnt >> 1)), val);

		val = (degamma_coef->b_coef[block_cnt] & ALSC_DEGAMMA_VAL_MASK);
		val |= ((degamma_coef->b_coef[block_cnt + 1] & ALSC_DEGAMMA_VAL_MASK) << ALSC_DEGAMMA_ODD_COEF_START_BIT);
		outp32(DPU_DPP_U_ALSC_DEGAMM_B_COEF_ADDR(alsc_base, (block_cnt >> 1)), val);
	}

	/* The 129th odd coef */
	outp32(DPU_DPP_U_ALSC_DEGAMMA_R_COEF_ADDR(alsc_base, (block_cnt >> 1)),
		degamma_coef->r_coef[block_cnt] & ALSC_DEGAMMA_VAL_MASK);
	outp32(DPU_DPP_U_ALSC_DEGAMM_G_COEF_ADDR(alsc_base, (block_cnt >> 1)),
		degamma_coef->g_coef[block_cnt] & ALSC_DEGAMMA_VAL_MASK);
	outp32(DPU_DPP_U_ALSC_DEGAMM_B_COEF_ADDR(alsc_base, (block_cnt >> 1)),
		degamma_coef->b_coef[block_cnt] & ALSC_DEGAMMA_VAL_MASK);
}

static void alsc_frame_set_reg(struct dpu_alsc *alsc)
{
	char __iomem *alsc_base = alsc->alsc_base;

	dpu_pr_debug("[ALSC]+");

	if ((is_bit_enable(alsc->action, ALSC_UPDATE_BL) != 0) || (is_bit_enable(alsc->action, ALSC_ENABLE) != 0))
		alsc_bl_param_set_reg(alsc);

	outp32(DPU_DPP_ALSC_DEGAMMA_EN_ADDR(alsc_base), alsc->dkmd_alsc.alsc_en);
	outp32(DPU_DPP_ALSC_EN_ADDR(alsc_base), alsc->dkmd_alsc.alsc_en);
}

static void alsc_init_set_reg(struct dpu_alsc *alsc)
{
	char __iomem *alsc_base = alsc->alsc_base;

	if (is_bit_enable(alsc->action, ALSC_ENABLE) == 0)
		return;

	dpu_pr_debug("[ALSC]+");

	/* CH0 ALSC DEGAMMA MEM */
	outp32(DPU_DPP_ALSC_DEGAMMA_MEM_CTRL_ADDR(alsc_base), 0x00000008);

	outp32(DPU_DPP_SENSOR_CHANNEL_ADDR(alsc_base), alsc->dkmd_alsc.sensor_channel);
	outp32(DPU_DPP_ALSC_SIZE_ADDR(alsc_base), alsc->dkmd_alsc.size);

	alsc_bl_param_set_reg(alsc);
	alsc_block_param_set_reg(alsc);
	alsc_degamma_coef_set_reg(alsc);

	outp32(DPU_DPP_ALSC_DEGAMMA_EN_ADDR(alsc_base), alsc->dkmd_alsc.alsc_en);
	outp32(DPU_DPP_ALSC_EN_ADDR(alsc_base), alsc->dkmd_alsc.alsc_en);

	switch_alsc_status(alsc, ALSC_WORKING);
}

static bool is_rect_included(const struct dkmd_rect * const alsc_rect, const struct dkmd_rect * const dirty_rect)
{
	return ((alsc_rect->x >= dirty_rect->x) &&
			(alsc_rect->y >= dirty_rect->y) &&
			((alsc_rect->x + alsc_rect->w) <= (dirty_rect->x + dirty_rect->w)) &&
			((alsc_rect->y + alsc_rect->h) <= (dirty_rect->y + dirty_rect->h)));
}

static void update_alsc_with_dirty_region(struct dpu_alsc *alsc, const struct dkmd_rect * const dirty_rect)
{
	struct dkmd_rect alsc_rect;

	alsc->alsc_en_by_dirty_region_limit = 1;

	if (dirty_rect->w <= 0 || dirty_rect->h <= 0) {
		alsc->alsc_en_by_dirty_region_limit = 0;
		dpu_pr_err("No dirty region\n");
		return;
	}

	alsc_rect.x = alsc->dkmd_alsc.addr & ALSC_ADDR_X_MASK;
	alsc_rect.y = (alsc->dkmd_alsc.addr >> ALSC_ADDR_Y_SHIFT) & ALSC_ADDR_X_MASK;
	alsc_rect.w = (alsc->dkmd_alsc.size & ALSC_SIZE_X_MASK) + 1;
	alsc_rect.h = ((alsc->dkmd_alsc.size >> ALSC_SIZE_Y_SHIFT) & ALSC_SIZE_X_MASK) + 1;

	/* 1. If dirty region doesn't overlap alsc region, bypass alsc module */
	if (!is_rect_included(&alsc_rect, dirty_rect)) {
		alsc->alsc_en_by_dirty_region_limit = 0;
		dpu_pr_debug("dirty region doesn't overlap alsc region, bypass alsc module\n");
		return;
	}

	dpu_pr_debug("[ALSC]ALSC dirty limit en=%u, addr=0x%x, size=0x%x,"
		" dirty region[%u %u %u %u]\n",
		alsc->alsc_en_by_dirty_region_limit, alsc->dkmd_alsc.addr, alsc->dkmd_alsc.size,
		dirty_rect->x, dirty_rect->y, dirty_rect->w, dirty_rect->h);
}

static int32_t alsc_set_reg(struct dpu_alsc *alsc, const void *data)
{
	const struct dkmd_alsc_frame *frame = (const struct dkmd_alsc_frame *)data;
	const struct dkmd_rect *disp_rect = frame->disp_rect;

	if ((is_bit_enable(alsc->action, ALSC_DISABLE) != 0) ||
		(frame->alsc_en == 0) ||
		(alsc->dkmd_alsc.alsc_en == 0) ||
		(g_enable_dpu_alsc == 0)) {
		outp32(DPU_DPP_ALSC_DEGAMMA_EN_ADDR(alsc->alsc_base), 0);
		outp32(DPU_DPP_ALSC_EN_ADDR(alsc->alsc_base), 0);

		if ((alsc->dkmd_alsc.alsc_en != 0) && (g_enable_dpu_alsc != 0)) {
			switch_alsc_status(alsc, ALSC_UNINIT);
			alsc->action = BIT(ALSC_ENABLE);
		}
		return 0;
	}

	update_alsc_with_dirty_region(alsc, disp_rect);
	if (alsc->alsc_en_by_dirty_region_limit == 0) {
		outp32(DPU_DPP_ALSC_DEGAMMA_EN_ADDR(alsc->alsc_base), 0);
		outp32(DPU_DPP_ALSC_EN_ADDR(alsc->alsc_base), 0);
		dpu_pr_debug("[ALSC]dirty limit, disable");
		return 0;
	}

	if (likely(alsc->status != ALSC_UNINIT))
		alsc_frame_set_reg(alsc);
	else
		alsc_init_set_reg(alsc);

	alsc_param_debug_print(alsc);

	alsc->action = ALSC_NO_ACTION;

	return 0;
}

static int32_t alsc_store_data(struct dpu_alsc *alsc, const void *data)
{
	static uint32_t control_status = 0;
	uint32_t isr_flag = *((const uint32_t *)data);
	struct dpu_alsc_data *p_data = alsc->data_head;
	char __iomem *alsc_base = alsc->alsc_base;
	struct timespec64 ts;

	dpu_assert(!p_data);

	if ((g_enable_dpu_alsc == 0) || (alsc->dkmd_alsc.alsc_en == 0)) {
		dpu_pr_debug("[ALSC]not enable %d %d", g_enable_dpu_alsc, alsc->dkmd_alsc.alsc_en);
		return 0;
	}

	dpu_pr_debug("isr_flag 0x%x", isr_flag);

	if (isr_flag & DSI_INT_VACT0_START) {
		p_data->noise.timestamp = inp32(DPU_DPP_TIMESTAMP_L_ADDR(alsc_base));
		p_data->noise.timestamp |= ((uint64_t)inp32(DPU_DPP_TIMESTAMP_H_ADDR(alsc_base)) << 32);
		control_status = (alsc->alsc_en_by_dirty_region_limit != 0) ? alsc->status : ALSC_STATUS_MAX;
		dpu_pr_debug("[ALSC]store timestamp [%llu], status %u", p_data->noise.timestamp, control_status);
	}

	if ((isr_flag & DSI_INT_VACT0_END)) {
		if (control_status != ALSC_WORKING) {
			dpu_pr_debug("[ALSC]status %u not working", control_status);
			return 0;
		}

		/* we need control_status here cause Noise should send or not must judge after
		 * VACTIVE0_START occurs
		 */
		p_data->noise.noise1 = inp32(DPU_DPP_NOISE1_ADDR(alsc_base));
		p_data->noise.noise2 = inp32(DPU_DPP_NOISE2_ADDR(alsc_base));
		p_data->noise.noise3 = inp32(DPU_DPP_NOISE3_ADDR(alsc_base));
		p_data->noise.noise4 = inp32(DPU_DPP_NOISE4_ADDR(alsc_base));
		ktime_get_boottime_ts64(&ts);
		p_data->ktimestamp = (uint64_t)ts.tv_sec * NSEC_PER_SEC + (uint64_t)ts.tv_nsec;
		p_data->noise.status = ALSC_NOISE_VALID;
		dpu_pr_debug("[ALSC]store noise [%u %u %u %u], ktimestamp=%llu", p_data->noise.noise1, p_data->noise.noise2,
			p_data->noise.noise3, p_data->noise.noise4, p_data->ktimestamp);

		/* Move to next node of the alsc data ring cycle */
		alsc->data_head = p_data->next;

		queue_work(alsc->alsc_send_data_wq, &alsc->alsc_send_data_work);
	}

	return 0;
}

static struct alsc_fsm g_alsc_fsm_tbl[ALSC_STATUS_MAX][ALSC_ACTION_MAX] = {
	// [ALSC_UNINIT]
	{
		// [ALSC_NO_ACTION] [ALSC_ENABLE] [ALSC_DISABLE]
		// [ALSC_UPDATE_BL] [ALSC_UPDATE_DEGAMMA] [ALSC_SET_REG]
		// [ALSC_BLANK] [ALSC_UNBLANK] [ALSC_HANDLE_ISR]
		{NULL}, {alsc_enable}, {NULL},
		{alsc_update_bl_param}, {alsc_update_degamma_coef}, {alsc_set_reg},
		{NULL}, {NULL}, {NULL}
	},

	// [ALSC_WORKING]
	{
		{NULL}, {alsc_enable}, {NULL},
		{alsc_update_bl_param}, {alsc_update_degamma_coef}, {alsc_set_reg},
		{alsc_blank}, {NULL}, {alsc_store_data}
	},

	// [ALSC_IDLE]
	{
		{NULL}, {alsc_enable}, {NULL},
		{alsc_update_bl_param}, {alsc_update_degamma_coef}, {NULL},
		{NULL}, {alsc_unblank}, {NULL}
	}
};

static int32_t dpu_alsc_fsm_handle(struct dpu_alsc *alsc, const void * const data, int32_t action)
{
	int32_t ret = 0;

	if (alsc == NULL) {
		dpu_pr_err("param alsc null");
		return -1;
	}

	if (!(alsc->is_inited))
		return -1;

	dpu_pr_debug("status=%d action=%d", alsc->status, action);
	if ((alsc->status >= ALSC_STATUS_MAX) || (action >= ALSC_ACTION_MAX)) {
		dpu_pr_err("status=%d or action=%d is invalid", alsc->status, action);
		return -1;
	}

	mutex_lock(&(alsc->alsc_lock));

	if (g_alsc_fsm_tbl[alsc->status][action].handle)
		ret = g_alsc_fsm_tbl[alsc->status][action].handle(alsc, data);

	mutex_unlock(&(alsc->alsc_lock));

	return ret;
}

static int32_t dpu_alsc_fsm_handle_unlock(struct dpu_alsc *alsc, const void * const data, int32_t action)
{
	int32_t ret = 0;

	if (alsc == NULL) {
		dpu_pr_err("param alsc null");
		return -1;
	}

	if (!(alsc->is_inited))
		return -1;

	dpu_pr_debug("status=%d action=%d", alsc->status, action);
	if ((alsc->status >= ALSC_STATUS_MAX) || (action >= ALSC_ACTION_MAX)) {
		dpu_pr_err("status=%d or action=%d is invalid", alsc->status, action);
		return -1;
	}

	/* ISR action should not use mutex, and alsc status is not changed */
	if (g_alsc_fsm_tbl[alsc->status][action].handle)
		ret = g_alsc_fsm_tbl[alsc->status][action].handle(alsc, data);

	return ret;
}

static int32_t dpu_effect_alsc_set_reg(struct dpu_composer *dpu_comp, struct dkmd_rect *disp_rect, uint32_t alsc_en)
{
	struct dkmd_alsc_frame frame;

	if ((!dpu_comp) || (!disp_rect)) {
		dpu_pr_err("dpu_comp invalid");
		return -1;
	}

	frame.disp_rect = disp_rect;
	frame.alsc_en = alsc_en;
	dpu_pr_debug("alsc_en is %d with the frame", alsc_en);

	return dpu_alsc_fsm_handle(dpu_comp->alsc, &frame, ALSC_SET_REG);
}

static int32_t dpu_effect_alsc_blank(struct dpu_alsc *alsc)
{
	if (!alsc) {
		dpu_pr_err("alsc invalid");
		return -EINVAL;
	}

	dpu_pr_debug("[ALSC]+");

	return dpu_alsc_fsm_handle(alsc, NULL, ALSC_BLANK);
}

static int32_t dpu_effect_alsc_unblank(struct dpu_alsc *alsc)
{
	if (!alsc) {
		dpu_pr_err("alsc invalid");
		return -EINVAL;
	}

	dpu_pr_debug("[ALSC]+");

	return dpu_alsc_fsm_handle(alsc, NULL, ALSC_UNBLANK);
}

static void dpu_effect_alsc_init(struct dpu_alsc *alsc)
{
	if (!alsc) {
		dpu_pr_err("alsc is NULL");
		return;
	}

	if (alsc->is_inited)
		return;

	if (alsc_init_data_storage(alsc))
		return;

	alsc->alsc_send_data_wq = alloc_workqueue("alsc_send_data_wq", WQ_HIGHPRI | WQ_UNBOUND, 0);
	if (alsc->alsc_send_data_wq == NULL) {
		dpu_pr_err("[ALSC] create alsc send data workqueue failed!\n");
		return;
	}
	INIT_WORK(&alsc->alsc_send_data_work, dpu_alsc_send_data_work_func);

	mutex_init(&(alsc->alsc_lock));
	alsc->comp_alsc_set_reg = dpu_effect_alsc_set_reg;
	alsc->comp_alsc_blank = dpu_effect_alsc_blank;
	alsc->comp_alsc_unblank = dpu_effect_alsc_unblank;
	switch_alsc_status(alsc, ALSC_UNINIT);
	alsc->is_inited = true;

	dpu_pr_debug("[ALSC]resources init success");
}

static void dpu_effect_alsc_deinit(struct dpu_alsc *alsc)
{
	if (!alsc) {
		dpu_pr_err("alsc is NULL");
		return;
	}

	if (!(alsc->is_inited))
		return;

	alsc_free_data_storage(alsc);
	mutex_destroy(&(alsc->alsc_lock));

	alsc->is_inited = false;
}

int32_t dpu_effect_alsc_store_data(struct dpu_alsc *alsc, uint32_t isr_flag)
{
	return dpu_alsc_fsm_handle_unlock(alsc, &isr_flag, ALSC_HANDLE_ISR);
}

int32_t dpu_effect_get_alsc_info(struct composer *comp, struct alsc_info *info)
{
	struct dpu_composer *dpu_comp = NULL;
	if ((!comp) || (!info)) {
		dpu_pr_err("param invalid");
		return -EINVAL;
	}

	dpu_comp = to_dpu_composer(comp);
	if (!dpu_comp->alsc)
		return 0;

	info->alsc_en = dpu_comp->alsc->dkmd_alsc.alsc_en & g_enable_dpu_alsc;
	info->alsc_addr = dpu_comp->alsc->dkmd_alsc.addr;
	info->alsc_size = dpu_comp->alsc->dkmd_alsc.size;
	info->bl_update = dpu_comp->alsc->bl_update_to_hwc;
	dpu_pr_debug("[ALSC]en addr size bl_update=[%u %u %u %u]\n",
		info->alsc_en, info->alsc_addr, info->alsc_size, info->bl_update);

	dpu_comp->alsc->bl_update_to_hwc = 0;

	return 0;
}

int32_t dpu_effect_register_alsc_composer(struct composer *comp, uint32_t index, uint32_t dpp_offset)
{
	uint32_t panel_id;

	struct dpu_composer *dpu_comp = to_dpu_composer(comp);
	if (!dpu_comp->comp_mgr) {
		dpu_pr_err("comp_mgr invalid");
		return -EINVAL;
	}

	if (!dpu_comp->comp_mgr->dpu_base) {
		dpu_pr_err("dpu_base invalid");
		return -EINVAL;
	}

	panel_id = (index == DEVICE_COMP_PRIMARY_ID) ? DPU_PANEL_ID_INNER : DPU_PANEL_ID_OUTER;
	dpu_comp->alsc = get_alsc_by_panel_id(panel_id);
	if (!dpu_comp->alsc) {
		dpu_pr_err("panel_id %d invalid", panel_id);
		return -EINVAL;
	}

	dpu_comp->alsc->comp = comp;
	dpu_comp->alsc->panel_id = panel_id;
	dpu_effect_alsc_init(dpu_comp->alsc);
	dpu_comp->alsc->alsc_base = dpu_comp->comp_mgr->dpu_base + dpp_offset;

	return 0;
}

int32_t dpu_effect_unregister_alsc_composer(struct composer *comp)
{
	struct dpu_composer *dpu_comp = to_dpu_composer(comp);
	if (dpu_comp->alsc)
		dpu_effect_alsc_deinit(dpu_comp->alsc);
	dpu_comp->alsc = NULL;

	return 0;
}

int dkmd_alsc_param_init(const struct dkmd_alsc* in_alsc, uint32_t panel_id)
{
	if (!in_alsc) {
		dpu_pr_err("[ALSC]in_alsc is NULL");
		return -1;
	}

	dpu_pr_info("[ALSC] param init, panel id %d", panel_id);

	return dpu_alsc_fsm_handle(get_alsc_by_panel_id(panel_id), in_alsc, ALSC_ENABLE);
}

void dkmd_alsc_refresh(uint32_t panel_id)
{
	struct dpu_alsc *alsc = get_alsc_by_panel_id(panel_id);

	if (!alsc || !alsc->is_inited || !alsc->comp)
		return;

	dpu_pr_debug("[ALSC] refresh, panel id %d", panel_id);

	device_mgr_primary_frame_refresh(alsc->comp, "ALSC");
}

int dkmd_alsc_update_bl_param(const struct alsc_bl_param* bl_param, uint32_t panel_id)
{
	if (!bl_param) {
		dpu_pr_err("[ALSC]bl_param is NULL");
		return -1;
	}

	dpu_pr_debug("[ALSC] update bl param, panel id %d", panel_id);

	return dpu_alsc_fsm_handle(get_alsc_by_panel_id(panel_id), bl_param, ALSC_UPDATE_BL);
}

int dkmd_alsc_update_degamma_coef(const struct alsc_degamma_coef* degamma_coef, uint32_t panel_id)
{
	if (!degamma_coef) {
		dpu_pr_err("[ALSC]degamma_coef is NULL");
		return -1;
	}

	dpu_pr_debug("[ALSC] update degamma, panel id %d", panel_id);

	return dpu_alsc_fsm_handle(get_alsc_by_panel_id(panel_id), degamma_coef, ALSC_UPDATE_DEGAMMA);
}

int32_t dkmd_alsc_register_cb_func(void (*func)(struct alsc_noise*, uint32_t),
								  void (*send_ddic_alpha)(uint32_t, uint32_t))
{
	struct dpu_alsc *alsc;
	uint32_t i;

	if (!func || !send_ddic_alpha) {
		dpu_pr_err("[ALSC]ALSC hasn't been initialized, wrong operation from input_hub");
		return -1;
	}

	dpu_pr_info("[ALSC]+");

	for (i = 0; i < DPU_PANEL_ID_MAX; i++) {
		alsc = get_alsc_by_panel_id(i);
		if (alsc == NULL) {
			dpu_pr_err("panel_id[%u] invalid", i);
			continue;
		}

		alsc->cb_func.send_data_func = func;
		alsc->cb_func.send_ddic_alpha = send_ddic_alpha;
	}

	return 0;
}
