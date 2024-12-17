/*
 * vcodec_vdec_refulator.c
 *
 * This is for vdec regulator
 *
 * Copyright (c) 2019-2023 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "vcodec_vdec_regulator.h"
#include "vcodec_vdec_plat.h"
#include "vcodec_vdec.h"
#include <linux/device.h>
#include "securec.h"
#include "vcodec_vdec_dpm.h"

vdec_plat g_vdec_plat_entry;
const char *g_regulator_name[MAX_REGULATOR] = { "ldo_media", "ldo_vdec", "ldo_smmu_tcu" };

vdec_plat *vdec_plat_get_entry(void)
{
	return &g_vdec_plat_entry;
}

static int32_t vdec_plat_init_smmu(struct device *dev)
{
	int32_t ret;
	vdec_plat *plt = vdec_plat_get_entry();
	(void)memset_s(&plt->smmu_info, sizeof(struct smmu_tbu_info), 0, sizeof(struct smmu_tbu_info));

	ret = of_property_read_u32(dev->of_node, "mmu_tbu_num", &plt->smmu_info.mmu_tbu_num);
	if (ret) {
		dprint(PRN_FATAL, "read mmu_tbu_num failed set to default\n");
		return VCODEC_FAILURE;
	}

	ret = of_property_read_u32(dev->of_node, "mmu_tbu_offset", &plt->smmu_info.mmu_tbu_offset);
	if (ret) {
		dprint(PRN_FATAL, "read mmu_tbu_offset failed set to default\n");
		return VCODEC_FAILURE;
	}

	ret = of_property_read_u32(dev->of_node, "mmu_sid_offset", &plt->smmu_info.mmu_sid_offset);
	if (ret) {
		dprint(PRN_FATAL, "read mmu_sid_offset failed set to default\n");
		return VCODEC_FAILURE;
	}

	return 0;
}

static int32_t vdec_plat_init_regulator(struct device *dev)
{
	uint32_t i;
	int32_t ret;
	struct vdec_regulator_pdt_ops *regulator_ops = get_vdec_regulator_plat_ops();
	vdec_plat *plt = vdec_plat_get_entry();
	(void)memset_s(&plt->regulator_info, sizeof(struct vdec_regulator), 0, sizeof(struct vdec_regulator));

	for (i = 0; i < MAX_REGULATOR; i++) {
		plt->regulator_info.regulators[i] = devm_regulator_get(dev, g_regulator_name[i]);
		if (IS_ERR_OR_NULL(plt->regulator_info.regulators[i])) {
			dprint(PRN_FATAL, "get regulator: %s failed\n", g_regulator_name[i]);
			return VCODEC_FAILURE;
		}
	}

	ret = regulator_ops->get_clk(dev, &plt->regulator_info);
	if (ret) {
		dprint(PRN_FATAL, "get regulator clk failed\n");
		return VCODEC_FAILURE;
	}

	for (i = VDEC_CLK_RATE_LOWER; i < VDEC_CLK_RATE_MAX; i++) {
		ret = of_property_read_u32_index(dev->of_node, VDEC_CLK_RATE, i,
			&plt->regulator_info.clk_values[i]);
		if (ret) {
			dprint(PRN_FATAL, "read %d clk failed\n", i);
			return VCODEC_FAILURE;
		}
	}

	/* transi_clk_rate & default_clk_rate are used for clk config constraint */
	ret = of_property_read_u32(dev->of_node, VDEC_TRANSITION_CLK_NAME,
		&plt->regulator_info.transi_clk_rate);
	if (ret) {
		dprint(PRN_FATAL, "failed to read transi_clk_rate\n");
		return VCODEC_FAILURE;
	}

	ret = of_property_read_u32(dev->of_node, MEDIA_DEFAULT_CLK_NAME,
		&plt->regulator_info.default_clk_rate);
	if (ret) {
		dprint(PRN_FATAL, "failed to read default_clk_rate\n");
		return VCODEC_FAILURE;
	}

	return 0;
}


static int32_t vdec_plat_init_dts(struct device *dev)
{
	int32_t ret;
	int32_t i;
	struct resource res;
	vdec_plat_ops *ops = get_vdec_plat_ops();
	vdec_plat *plt_info = vdec_plat_get_entry();
	(void)memset_s(&plt_info->dts_info, sizeof(vdec_dts), 0, sizeof(vdec_dts));
	(void)memset_s(&res, sizeof(res), 0, sizeof(res));

	plt_info->dts_info.irq_norm = irq_of_parse_and_map(dev->of_node, 0);
	plt_info->dts_info.irq_safe = irq_of_parse_and_map(dev->of_node, 1);
	if (plt_info->dts_info.irq_norm == 0 || plt_info->dts_info.irq_safe == 0) {
		dprint(PRN_ERROR, "irq_of_parse_and_map failed\n");
		return VCODEC_FAILURE;
	}

	if (ops->smmu_need_enable_tbu_ecc()) {
		plt_info->dts_info.irq_ecc_detect = irq_of_parse_and_map(dev->of_node, 2);
		plt_info->dts_info.irq_ecc_multpl = irq_of_parse_and_map(dev->of_node, 3);

		if (plt_info->dts_info.irq_ecc_detect == 0 || plt_info->dts_info.irq_ecc_multpl == 0)
			dprint(PRN_ERROR, "irq_of_parse_and_map ecc irq failed\n");
	}

	for (i = 0; i < MAX_INNER_MODULE; i++) {
		ret = of_address_to_resource(dev->of_node, i, &res);
		if (ret) {
			dprint(PRN_FATAL, "read dec inner module reg %d info failed\n", i);
			return VCODEC_FAILURE;
		}

		plt_info->dts_info.module_reg[i].reg_phy_addr = (UADDR)res.start;
		plt_info->dts_info.module_reg[i].reg_range = (uint32_t)resource_size(&res);
	}

	ret = of_property_read_u32(dev->of_node, "vdec_fpga", &plt_info->dts_info.is_fpga);
	if (ret) {
		dprint(PRN_WARN, "failed to get plat vdec_fpga flag\n");
		plt_info->dts_info.is_fpga = 0;
	}

	ret = of_property_read_u32(dev->of_node, "vdec_es", &plt_info->dts_info.is_es_plat);
	if (ret) {
		dprint(PRN_WARN, "failed to get plat vdec_es flag\n");
		plt_info->dts_info.is_es_plat = 0;
	}

	ret = of_property_read_u32(dev->of_node, "pcie_mmap_host_start_reg", &plt_info->dts_info.pcie_mmap_host_start_reg);
	if (ret) {
		dprint(PRN_WARN, "failed to get plat pcie_mmap_host_start_reg flag\n");
		plt_info->dts_info.pcie_mmap_host_start_reg = 0;
	}

	return 0;
}

static void vdec_plat_deinit_regulator(struct vdec_regulator *regulator_info)
{
	(void)memset_s(regulator_info, sizeof(struct vdec_regulator), 0, sizeof(struct vdec_regulator));
}

static void vdec_plat_deinit_dts(vdec_dts *dts_info)
{
	(void)memset_s(dts_info, sizeof(vdec_dts), 0, sizeof(vdec_dts));
}

static void vdec_plat_deinit_smmu(struct smmu_tbu_info *smm_info)
{
	(void)memset_s(smm_info, sizeof(struct smmu_tbu_info), 0, sizeof(struct smmu_tbu_info));
}

int32_t vdec_plat_init(struct platform_device *plt_dev)
{
	int32_t ret;
	vdec_plat *plt = vdec_plat_get_entry();

	vdec_init_mutex(&plt->vdec_plat_mutex);

	ret = vdec_plat_init_regulator(&plt_dev->dev);
	if (ret) {
		dprint(PRN_FATAL, "regulator init failed\n");
		return VCODEC_FAILURE;
	}

	ret = vdec_plat_init_dts(&plt_dev->dev);
	if (ret) {
		dprint(PRN_FATAL, "dts init failed\n");
		return VCODEC_FAILURE;
	}

	ret = vdec_plat_init_smmu(&plt_dev->dev);
	if (ret) {
		dprint(PRN_FATAL, "smmu info init failed\n");
		return VCODEC_FAILURE;
	}

	plt->dev = &plt_dev->dev;
	plt->dts_info.dev = plt->dev;

	plt->plt_init = 1;
	plt->power_flag = 0;
	plt->clk_ctrl.dynamic_clk = VDEC_CLK_RATE_LOWER;


	return 0;
}

int32_t vdec_plat_deinit(void)
{
	vdec_plat *plt = vdec_plat_get_entry();

	vdec_plat_deinit_regulator(&plt->regulator_info);

	vdec_plat_deinit_dts(&plt->dts_info);

	vdec_plat_deinit_smmu(&plt->smmu_info);

	plt->power_flag = 0;
	plt->plt_init = 0;

	return 0;
}

int32_t vdec_plat_regulator_map_reg(void)
{
	struct vdec_regulator_pdt_ops *regulator_ops = get_vdec_regulator_plat_ops();
	if (regulator_ops->map_reg_info != VCODEC_NULL)
		return regulator_ops->map_reg_info();
	return 0;
}

void vdec_plat_regulator_unmap_reg(void)
{
	struct vdec_regulator_pdt_ops *regulator_ops = get_vdec_regulator_plat_ops();
	if (regulator_ops->unmap_reg_info != VCODEC_NULL)
		regulator_ops->unmap_reg_info();
}

static bool vdec_plat_transition_clk_enable(void)
{
	struct vdec_regulator_pdt_ops *regulator_ops = get_vdec_regulator_plat_ops();
	return regulator_ops->transition_clk_enable();
}

int32_t vdec_plat_regulator_global_reset(void)
{
	struct vdec_regulator_pdt_ops *regulator_ops = get_vdec_regulator_plat_ops();
	if (regulator_ops->vdec_global_reset != VCODEC_NULL)
		return regulator_ops->vdec_global_reset();
	return 0;
}

static void vdec_plat_regulator_close_clk(void)
{
	struct vdec_regulator_pdt_ops *regulator_ops = get_vdec_regulator_plat_ops();
	if (regulator_ops->close_clk == VCODEC_NULL) {
		dprint(PRN_ERROR, "regulator close is null handle\n");
		return;
	}
	regulator_ops->close_clk();
}

static int32_t vdec_plat_regulator_open_clk(void)
{
	struct vdec_regulator_pdt_ops *regulator_ops = get_vdec_regulator_plat_ops();
	if (regulator_ops->open_clk == VCODEC_NULL) {
		dprint(PRN_ERROR, "regulator open is null handle\n");
		return VCODEC_FAILURE;
	}
	return regulator_ops->open_clk();
}

int32_t VCODEC_ATTR_WEEK vdec_plat_regulator_enable(void)
{
	int32_t ret;
	vdec_plat *plt = vdec_plat_get_entry();

	vdec_check_ret(plt->plt_init, VCODEC_FAILURE);

	vdec_mutex_lock(&plt->vdec_plat_mutex);
	if (plt->power_flag) {
		dprint(PRN_ALWS, "regulator already enable\n");
		vdec_mutex_unlock(&plt->vdec_plat_mutex);
		return 0;
	}

	ret = regulator_enable(plt->regulator_info.regulators[MEDIA_REGULATOR]);
	if (ret) {
		dprint(PRN_FATAL, "enable media regulator failed\n");
		goto exit;
	}

	ret = vdec_plat_regulator_open_clk();
	if (ret) {
		dprint(PRN_FATAL, "clk enable failed\n");
		goto disable_media;
	}

	ret = regulator_enable(plt->regulator_info.regulators[VDEC_REGULATOR]);
	if (ret) {
		dprint(PRN_FATAL, "enable vdec regulator failed\n");
		goto unprepare_clk;
	}

	plt->power_flag = 1;
	vdec_mutex_unlock(&plt->vdec_plat_mutex);

	dprint(PRN_CTRL, "success\n");
	return 0;

unprepare_clk:
	vdec_plat_regulator_close_clk();

disable_media:
	if (regulator_disable(plt->regulator_info.regulators[MEDIA_REGULATOR]))
		dprint(PRN_ALWS, "disable media regulator failed\n");
exit:
	vdec_mutex_unlock(&plt->vdec_plat_mutex);

	return VCODEC_FAILURE;
}

static void vdec_plat_set_transition_clk_by_type(vdec_plat *plt)
{
	clk_rate_e current_clk_type = plt->clk_ctrl.current_clk;
	if (!vdec_plat_transition_clk_enable())
		return;
	if (current_clk_type == VDEC_CLK_RATE_NORMAL ||
		current_clk_type == VDEC_CLK_RATE_HIGH ||
		current_clk_type == VDEC_CLK_RATE_MAX - 1) {
		(void)clk_set_rate(plt->regulator_info.clk_vdec, plt->regulator_info.transi_clk_rate);
		dprint(PRN_DBG, "current_clk is %d need set transi_clk_rate %u\n",
		current_clk_type, plt->regulator_info.transi_clk_rate);
	}
}

void VCODEC_ATTR_WEEK vdec_plat_regulator_disable(void)
{
	vdec_plat *plt = vdec_plat_get_entry();

	if (!(plt->plt_init)) {
		dprint(PRN_ERROR, "assert condition not match\n");
		return;
	}
	vdec_mutex_lock(&plt->vdec_plat_mutex);

	if (!plt->power_flag) {
		dprint(PRN_ALWS, "regulator already disable\n");
		vdec_mutex_unlock(&plt->vdec_plat_mutex);
		return;
	}

	(void)regulator_disable(plt->regulator_info.regulators[VDEC_REGULATOR]);
	// VDEC_CLK_RATE_MAX - 1 represents VDEC_CLK_RATE_HIGHER for 5 clk rates, else DEC_CLK_RATE_HIGH for 4 clk rates
	vdec_plat_set_transition_clk_by_type(plt);
	(void)clk_set_rate(plt->regulator_info.clk_vdec, plt->regulator_info.default_clk_rate);
	vdec_plat_regulator_close_clk();
	(void)regulator_disable(plt->regulator_info.regulators[MEDIA_REGULATOR]);

	plt->power_flag = 0;
	vdec_mutex_unlock(&plt->vdec_plat_mutex);

	dprint(PRN_CTRL, "success\n");
}

void vdec_plat_set_dynamic_clk_rate(clk_rate_e clk_rate)
{
	vdec_plat *plt = vdec_plat_get_entry();

	vdec_mutex_lock(&plt->vdec_plat_mutex);
	plt->clk_ctrl.dynamic_clk = clk_rate;
	vdec_mutex_unlock(&plt->vdec_plat_mutex);
}

void vdec_plat_get_dynamic_clk_rate(clk_rate_e *clk_rate)
{
	vdec_plat *plt = vdec_plat_get_entry();

	vdec_mutex_lock(&plt->vdec_plat_mutex);
	*clk_rate = plt->clk_ctrl.dynamic_clk;
	vdec_mutex_unlock(&plt->vdec_plat_mutex);
}

void vdec_plat_set_static_clk_rate(clk_rate_e clk_rate)
{
	vdec_plat *plt = vdec_plat_get_entry();

	vdec_mutex_lock(&plt->vdec_plat_mutex);
	plt->clk_ctrl.static_clk = clk_rate;
	vdec_mutex_unlock(&plt->vdec_plat_mutex);
}

void vdec_plat_get_static_clk_rate(clk_rate_e *clk_rate)
{
	vdec_plat *plt = vdec_plat_get_entry();

	vdec_mutex_lock(&plt->vdec_plat_mutex);
	*clk_rate = plt->clk_ctrl.static_clk;
	vdec_mutex_unlock(&plt->vdec_plat_mutex);
}

void vdec_plat_get_target_clk_rate(clk_rate_e *clk_rate)
{
	vdec_plat *plt = vdec_plat_get_entry();
	clk_rate_e target_clk_rate;

	vdec_mutex_lock(&plt->vdec_plat_mutex);
	if (*clk_rate >= VDEC_CLK_RATE_MAX)
		target_clk_rate = (plt->clk_ctrl.static_clk >= plt->clk_ctrl.dynamic_clk) ?
			plt->clk_ctrl.static_clk : plt->clk_ctrl.dynamic_clk;
	else
		target_clk_rate = (*clk_rate >= plt->clk_ctrl.dynamic_clk) ? *clk_rate : plt->clk_ctrl.dynamic_clk;
	*clk_rate = target_clk_rate;
	vdec_mutex_unlock(&plt->vdec_plat_mutex);
}

static void vdec_plat_set_transition_clk(uint32_t current_clk_rate)
{
	int32_t ret;
	vdec_plat *plt = vdec_plat_get_entry();

	if (!vdec_plat_transition_clk_enable())
		return;
	/* set a transit frequence to avoid high frequence low voltage problem */
	if (current_clk_rate == plt->regulator_info.clk_values[VDEC_CLK_RATE_HIGH] ||
		current_clk_rate == plt->regulator_info.clk_values[VDEC_CLK_RATE_NORMAL] ||
		current_clk_rate == plt->regulator_info.clk_values[VDEC_CLK_RATE_MAX - 1]) {
		dprint(PRN_ALWS, "current_clk_rate is %u need set transi_clk_rate %u\n",
			current_clk_rate, plt->regulator_info.transi_clk_rate);
		ret = clk_set_rate(plt->regulator_info.clk_vdec, plt->regulator_info.transi_clk_rate);
		if (ret)
			dprint(PRN_ERROR, "failed set transi_clk_rate to %u Hz,fail code is %d\n",
				plt->regulator_info.transi_clk_rate, ret);
	}
}

static int32_t vdec_plat_switch_clk(clk_rate_e dst_clk_type, uint32_t dst_clk_rate)
{
	int32_t ret;
	uint32_t current_clk_rate;
	clk_rate_e actual_clk_type;
	uint32_t actual_clk_rate;
	vdec_plat *plt = vdec_plat_get_entry();

	current_clk_rate = (uint32_t)clk_get_rate(plt->regulator_info.clk_vdec);
	if (dst_clk_rate == current_clk_rate)
		return 0;

	vdec_plat_set_transition_clk(current_clk_rate);

	ret = clk_set_rate(plt->regulator_info.clk_vdec, dst_clk_rate);
	actual_clk_type = dst_clk_type;
	// VDEC_CLK_RATE_MAX - 1 represents VDEC_CLK_RATE_HIGHER for 5 clk rates, else DEC_CLK_RATE_HIGH for 4 clk rates
	if (ret && (dst_clk_rate == plt->regulator_info.clk_values[VDEC_CLK_RATE_HIGH] ||
				dst_clk_rate == plt->regulator_info.clk_values[VDEC_CLK_RATE_MAX - 1])) {
		actual_clk_rate = plt->regulator_info.clk_values[VDEC_CLK_RATE_NORMAL];
		actual_clk_type = VDEC_CLK_RATE_NORMAL;
		ret = clk_set_rate(plt->regulator_info.clk_vdec, actual_clk_rate);
		dprint(PRN_ALWS, "low temperature state: %u\n", actual_clk_rate);
		if (ret) {
			dprint(PRN_ERROR, "failed set clk to %u Hz,fail code is %d\n",
				dst_clk_rate, ret);
			return VCODEC_FAILURE;
		}
	}

	plt->clk_ctrl.current_clk = actual_clk_type;

	return 0;
}

int32_t vdec_plat_regulator_set_clk_rate(clk_rate_e dst_clk_type)
{
	uint32_t dst_clk_rate;
	vdec_plat *plt = vdec_plat_get_entry();

	vdec_check_ret(plt->plt_init, VCODEC_FAILURE);
	vdec_check_ret((dst_clk_type < VDEC_CLK_RATE_MAX), VCODEC_FAILURE);

	vdec_mutex_lock(&plt->vdec_plat_mutex);

	if (plt->clk_ctrl.clk_flag && plt->clk_ctrl.current_clk == dst_clk_type) {
		vdec_mutex_unlock(&plt->vdec_plat_mutex);
		return 0;
	}

	dst_clk_rate = plt->regulator_info.clk_values[dst_clk_type];

	if (vdec_plat_switch_clk(dst_clk_type, dst_clk_rate) != 0)
		goto error_exit;

	vdec_mutex_unlock(&plt->vdec_plat_mutex);

	dprint(PRN_CTRL, "set clk_rate value: %u\n", dst_clk_rate);
	return 0;

error_exit:
	vdec_mutex_unlock(&plt->vdec_plat_mutex);

	return VCODEC_FAILURE;
}

