/*
 * vdec_regulator_diff.c
 *
 * This is for vdec platform
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
#include <linux/delay.h>
#include "vcodec_vdec_regulator.h"
#include "dbg.h"

#define VDH_SRST_REQ_REG_OFS 0xc
#define VDH_SRST_OK_REG_OFS  0x14
#define ALL_RESET_OK_BIT     0
#define ALL_RESET_OK_MASK    (1U << ALL_RESET_OK_BIT)
struct vcrg_vdh_srst_req {
	uint32_t vdh_all_srst_req : 1;
	uint32_t vdh_mfde_srst_req : 1;
	uint32_t vdh_scd_srst_req : 1;
	uint32_t vdh_bpd_srst_req : 1;
	uint32_t reserved : 28;
};

struct vcrg_vdh_srst_ok {
	uint32_t vdh_all_srst_ok : 1;
	uint32_t vdh_mfde_srst_ok : 1;
	uint32_t vdh_scd_srst_ok : 1;
	uint32_t vdh_bpd_srst_ok : 1;
	uint32_t reserved : 28;
};

struct vdec_regulater_reg_info {
	UADDR crg_reg_phy_addr;
	uint8_t *crg_reg_vaddr;
	uint32_t crg_reg_size;
};

static struct vdec_regulater_reg_info *vdec_pdt_get_glb_info(void)
{
	static struct vdec_regulater_reg_info g_vdec_crg_reg;
	return &g_vdec_crg_reg;
}

static int32_t vdec_map_reg_info_info(void)
{
	vdec_plat *plt = vdec_plat_get_entry();
	vdec_dts *dts_info = &plt->dts_info;
	vfmw_module_reg_info *crg_reg_info = &dts_info->module_reg[CRG_MOUDLE];
	struct vdec_regulater_reg_info *glb_info = vdec_pdt_get_glb_info();

	glb_info->crg_reg_vaddr = (uint8_t *)IO_REMAP_NO_CACHE(crg_reg_info->reg_phy_addr, crg_reg_info->reg_range);
	if (!glb_info->crg_reg_vaddr) {
		dprint(PRN_ERROR, "map crg reg failed\n");
		return VCODEC_FAILURE;
	}
	glb_info->crg_reg_phy_addr = crg_reg_info->reg_phy_addr;
	glb_info->crg_reg_size = crg_reg_info->reg_range;
	dprint(PRN_DBG, "map crg reg success\n");
	return VCODEC_SUCCESS;
}

static void vdec_unmap_reg_info_info(void)
{
	struct vdec_regulater_reg_info *glb_info = vdec_pdt_get_glb_info();
	if (glb_info->crg_reg_vaddr) {
		iounmap(glb_info->crg_reg_vaddr);
		glb_info->crg_reg_vaddr = VCODEC_NULL;
	}
	glb_info->crg_reg_phy_addr = 0;
	glb_info->crg_reg_size = 0;
	dprint(PRN_DBG, "unmap crg reg success\n");
}

static int32_t vdec_pdt_get_regulator_clk(struct device *dev, struct vdec_regulator *regulator_info)
{
	regulator_info->clk_vdec = devm_clk_get(dev, VDEC_CLOCK_NAME);
	if (IS_ERR_OR_NULL(regulator_info->clk_vdec)) {
		dprint(PRN_FATAL, "get clk failed\n");
		return VCODEC_FAILURE;
	}

	regulator_info->aclk_vdec = devm_clk_get(dev, VDEC_ACLOCK_NAME);
	if (IS_ERR_OR_NULL(regulator_info->aclk_vdec)) {
		dprint(PRN_FATAL, "get axi clk failed\n");
		return VCODEC_FAILURE;
	}
	return VCODEC_SUCCESS;
}

static int32_t vdec_pdt_fix_reset_clock(vdec_plat *plt)
{
	int32_t ret = clk_prepare_enable(plt->regulator_info.aclk_vdec);
	if (ret) {
		dprint(PRN_FATAL, "axi clk enable failed\n");
		return VCODEC_FAILURE;
	}
	clk_disable_unprepare(plt->regulator_info.aclk_vdec);

	ret = clk_prepare_enable(plt->regulator_info.clk_vdec);
	if (ret) {
		dprint(PRN_FATAL, "vdh clk enable failed\n");
		return VCODEC_FAILURE;
	}
	clk_disable_unprepare(plt->regulator_info.clk_vdec);

	ret = clk_prepare_enable(plt->regulator_info.aclk_vdec);
	if (ret) {
		dprint(PRN_FATAL, "axi clk enable failed\n");
		return VCODEC_FAILURE;
	}

	ret = clk_prepare_enable(plt->regulator_info.clk_vdec);
	if (ret) {
		clk_disable_unprepare(plt->regulator_info.aclk_vdec);
		dprint(PRN_FATAL, "vdh clk enable failed\n");
		return VCODEC_FAILURE;
	}
	dprint(PRN_ALWS, "fix clk success\n");
	return VCODEC_SUCCESS;
}

static int32_t vdec_regulator_open_clk(void)
{
	vdec_plat *plt = vdec_plat_get_entry();
	int32_t ret = clk_prepare_enable(plt->regulator_info.clk_vdec);
	vfmw_assert_ret_prnt(ret == 0, ret, "vdec clk enable failed\n");
	ret = clk_prepare_enable(plt->regulator_info.aclk_vdec);
	vfmw_assert_goto_prnt(ret == 0, open_aclk_failed, "vdec axi enable failed\n");
	return VCODEC_SUCCESS;
open_aclk_failed:
	clk_disable_unprepare(plt->regulator_info.clk_vdec);
	return VCODEC_FAILURE;
}

static void vdec_regulator_close_clk(void)
{
	vdec_plat *plt = vdec_plat_get_entry();
	clk_disable_unprepare(plt->regulator_info.clk_vdec);
	clk_disable_unprepare(plt->regulator_info.aclk_vdec);
}

static int32_t vdec_pdt_global_reset(void)
{
	int32_t i;
	uint32_t rst_req, rst_ok;
	vdec_plat *plt = vdec_plat_get_entry();
	struct vdec_regulater_reg_info *glb_info = vdec_pdt_get_glb_info();
	uint8_t *crg_reg_addr = glb_info->crg_reg_vaddr;
	vfmw_assert_ret_prnt(crg_reg_addr != VCODEC_NULL, VCODEC_FAILURE, "crg reg addr is null\n");

	vdec_rd_vreg(crg_reg_addr, VDH_SRST_REQ_REG_OFS, rst_req);
	((struct vcrg_vdh_srst_req *)(&rst_req))->vdh_all_srst_req = 1;
	vdec_wr_vreg(crg_reg_addr, VDH_SRST_REQ_REG_OFS, rst_req);
	mb();

	for (i = 0; i < 1000; i++) { /* 1000us */
		udelay(1);
		vdec_rd_vreg(crg_reg_addr, VDH_SRST_OK_REG_OFS, rst_ok);
		if ((rst_ok & ALL_RESET_OK_MASK) == 1) {
			dprint(PRN_ALWS, "dec glb reset success\n");
			break;
		}
	}
	if (i == 1000) { /* over 1000 times */
		dprint(PRN_ERROR, "dec glb reset failed\n");
		return VCODEC_FAILURE;
	}
	clk_disable_unprepare(plt->regulator_info.aclk_vdec);
	vdec_rd_vreg(crg_reg_addr, VDH_SRST_REQ_REG_OFS, rst_req);
	((struct vcrg_vdh_srst_req *)(&rst_req))->vdh_all_srst_req = 0;
	vdec_wr_vreg(crg_reg_addr, VDH_SRST_REQ_REG_OFS, rst_req);
	clk_disable_unprepare(plt->regulator_info.clk_vdec);
	return vdec_pdt_fix_reset_clock(plt);
}

static bool vdec_pdt_transition_clk_enable(void)
{
	return false;
}

static struct vdec_regulator_pdt_ops g_vdec_regulator_ops = {
	.get_clk = vdec_pdt_get_regulator_clk,
	.map_reg_info = vdec_map_reg_info_info,
	.unmap_reg_info = vdec_unmap_reg_info_info,
	.open_clk = vdec_regulator_open_clk,
	.close_clk = vdec_regulator_close_clk,
	.vdec_global_reset = vdec_pdt_global_reset,
	.transition_clk_enable = vdec_pdt_transition_clk_enable,
};

struct vdec_regulator_pdt_ops *get_vdec_regulator_plat_ops(void)
{
	return &g_vdec_regulator_ops;
}

