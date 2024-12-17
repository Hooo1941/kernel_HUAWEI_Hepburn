/*
 * npu_irq.c
 *
 * about npu irq
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "npu_irq.h"
#include "npu_log.h"
#include "npu_feature.h"
#ifdef CONFIG_NPU_HWTS
#include "npu_platform.h"
#include "npu_platform_register.h"
#include "soc_npu_hw_exp_irq_interface.h"
#include "npu_io.h"
#include "npd_interface.h"
#endif

int npu_plat_parse_irq(struct platform_device *pdev,
	struct npu_platform_info *plat_info)
{
#ifdef CONFIG_NPU_ECC
	int ecc_idx = 0;
	for (ecc_idx = 0; ecc_idx < NPU_IRQ_ECC_MAX; ++ecc_idx) {
		plat_info->dts_info.irq_ecc_err[ecc_idx] = platform_get_irq(pdev, NPU_IRQ_MECC_TBU_CORE0 + ecc_idx);
		cond_return_error(plat_info->dts_info.irq_ecc_err[ecc_idx] < 0,
			-EINVAL, "get irq_ecc_err[%d] irq fail\n", ecc_idx);
		npu_drv_info("irq_ecc_err[%d]=%d\n", ecc_idx, plat_info->dts_info.irq_ecc_err[ecc_idx]);
	}
#endif
#ifndef CONFIG_NPU_SWTS
	plat_info->dts_info.irq_cq_update = platform_get_irq(pdev,
		NPU_IRQ_CALC_CQ_UPDATE0);
	cond_return_error(plat_info->dts_info.irq_cq_update < 0,
		-EINVAL, "get cq update irq fail\n");

	plat_info->dts_info.irq_dfx_cq = platform_get_irq(pdev,
		NPU_IRQ_DFX_CQ_UPDATE);
	cond_return_error(plat_info->dts_info.irq_dfx_cq < 0,
		-EINVAL, "get dfx cq update irq fail\n");

	plat_info->dts_info.irq_mailbox_ack = platform_get_irq(pdev,
		NPU_IRQ_MAILBOX_ACK);
	cond_return_error(plat_info->dts_info.irq_mailbox_ack < 0,
		-EINVAL, "get mailbox irq fail\n");

#ifdef CONFIG_NPU_INTR_HUB
	plat_info->dts_info.irq_intr_hub = platform_get_irq(pdev,
		NPU_IRQ_INTR_HUB);
	cond_return_error(plat_info->dts_info.irq_intr_hub < 0,
		-EINVAL, "get intr hub irq fail\n");
#endif
	npu_drv_debug("calc_cq_update0=%d, dfx_cq_update=%d, mailbox_ack=%d\n",
		plat_info->dts_info.irq_cq_update,
		plat_info->dts_info.irq_dfx_cq,
		plat_info->dts_info.irq_mailbox_ack);
#else
	plat_info->dts_info.irq_swts_hwts_normal = platform_get_irq(pdev,
		NPU_IRQ_HWTS_NORMAL_NS);
	plat_info->dts_info.irq_swts_hwts_error = platform_get_irq(pdev,
		NPU_IRQ_HWTS_ERROR_NS);
	plat_info->dts_info.irq_dmmu = platform_get_irq(pdev,
		NPU_IRQ_DMMU);
	npu_drv_debug("hwts_normal_ns=%d, hwts_error_ns=%d, dmmu=%d\n",
		plat_info->dts_info.irq_swts_hwts_normal,
		plat_info->dts_info.irq_swts_hwts_error,
		plat_info->dts_info.irq_dmmu);
#endif
	if (plat_info->dts_info.feature_switch[NPU_FEATURE_EASC] == 1) {
		plat_info->dts_info.irq_easc = platform_get_irq(pdev, NPU_IRQ_EASC);
		cond_return_error(plat_info->dts_info.irq_easc < 0,
			-EINVAL, "get easc irq fail\n");
		npu_drv_debug("easc=%d\n", plat_info->dts_info.irq_easc);
	}
	return 0;
}

#if (defined CONFIG_NPU_HWTS) && (!defined CONFIG_NPU_SWTS)
// get hw irq offset according the receving irq num
#define npu_shift_left_val(irq_offset)   (0x1 << (irq_offset))

#define NPU_IRQ_GIC_MAP_COLUMN       2
u32 g_npu_irq_gic_map[][NPU_IRQ_GIC_MAP_COLUMN] = {
	// irq_type,                 gic_num
	{NPU_IRQ_CALC_CQ_UPDATE0, NPU_NPU2ACPU_HW_EXP_IRQ_NS_2},
	{NPU_IRQ_DFX_CQ_UPDATE,   NPU_NPU2ACPU_HW_EXP_IRQ_NS_1},
	{NPU_IRQ_MAILBOX_ACK,     NPU_NPU2ACPU_HW_EXP_IRQ_NS_0}
};

static int npu_parse_gic_irq(u32 gic_irq)
{
	cond_return_error((gic_irq < NPU_NPU2ACPU_HW_EXP_IRQ_0) ||
		(gic_irq > NPU_NPU2ACPU_HW_EXP_IRQ_NS_3),
		-1, "gic_irq = %u is invalid \n", gic_irq);
	return ((int)gic_irq >= NPU_NPU2ACPU_HW_EXP_IRQ_NS_0) ?
		(int)gic_irq - NPU_NPU2ACPU_HW_EXP_IRQ_NS_0 :
		(int)gic_irq - NPU_NPU2ACPU_HW_EXP_IRQ_0;
}

static void npu_clr_hw_exp_irq_int(u64 reg_base, u32 gic_irq)
{
	u32 setval;
	int irq_offset = npu_parse_gic_irq(gic_irq);
	if (irq_offset == -1) {
		npu_drv_err("invalide gic_irq:%d\n", gic_irq);
		return;
	}
	if (reg_base == 0) {
		npu_drv_err("invalide hwirq reg base\n");
		return;
	}

	setval = npu_shift_left_val((u32)irq_offset);
	update_reg32(SOC_NPU_HW_EXP_IRQ_CLR_ADDR(reg_base), setval, setval);
}
#endif // CONFIG_NPU_HWTS

int npu_plat_handle_irq_tophalf(u32 irq_index)
{
#if (defined NPU_ARCH_V100) || (defined CONFIG_NPU_SWTS)
	unused(irq_index);
	return 0;
#else
	u32 i;
	u32 gic_irq = 0;
	size_t map_len = sizeof(g_npu_irq_gic_map) /
		(NPU_IRQ_GIC_MAP_COLUMN * sizeof(u32));
	struct npu_platform_info *plat_info = npu_plat_get_info();
	cond_return_error(plat_info == NULL, -1, "get platform info is NULL\n");

	for (i = 0; i < map_len; i++) {
		if (g_npu_irq_gic_map[i][0] == irq_index) {
			gic_irq = g_npu_irq_gic_map[i][1];
			break;
		}
	}
	cond_return_error(gic_irq == 0, -1, "invalide irq_index:%d\n", irq_index);

	npu_clr_hw_exp_irq_int(
		(u64)(uintptr_t)plat_info->dts_info.reg_vaddr[NPU_REG_HW_EXP_IRQ_NS_BASE], gic_irq);
	return 0;
#endif
}
