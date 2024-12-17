/*
 * npu_aicore_plat.c
 *
 * about npu aicore plat
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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
#include "npu_aicore_plat.h"

#include <linux/io.h>

#include "npu_platform.h"
#include "npu_log.h"

static u64 npu_aicore_get_base_addr(u8 aic_id)
{
	u64 aicore_base = 0ULL;
	u64 base = 0ULL;
	struct npu_platform_info *plat_info = npu_plat_get_info();

	cond_return_error(plat_info == NULL, 0ULL, "npu_plat_get_info failed\n");
	base = (u64)(uintptr_t)plat_info->dts_info.reg_vaddr[NPU_REG_AIC0_BASE];
	if (aic_id == NPU_AICORE_0)
		aicore_base = base;
	else if (aic_id == NPU_AIVECTOR_0)
		aicore_base = base + NPU_AIV_ADDR_OFFSET;
	else
		npu_drv_err("aic_id=%d is invalid\n", aic_id);

	npu_drv_debug("aicore%u_base = 0x%016llx\n", aic_id, aicore_base);
	return aicore_base;
}

static int npu_aicore_dbg_busy_wait(u64 addr, u32 *val)
{
	u32 busy_wait_loop = 0;

	while (busy_wait_loop < NPU_AICORE_DBG_WAIT_LOOP_MAX) {
		*val = readl((const volatile void *)(uintptr_t)addr);
		if (*val != 0)
			break;
		busy_wait_loop++;
	}

	if (*val == 0 && busy_wait_loop >= NPU_AICORE_DBG_WAIT_LOOP_MAX) {
		npu_drv_warn("busy wait timed out\n");
		return -1;
	}

	npu_drv_debug("busy wait successfully\n");
	return 0;
}

static int npu_aicore_read_dbg_register(u8 aic_id, u64 debug_addr_id, u64 *read_buf)
{
	int ret;
	SOC_NPU_AICORE_CLK_GATE_MASK_1_UNION clk_gate_mask_1 = {0};
	SOC_NPU_AICORE_DBG_ADDR_0_UNION dbg_addr_0 = {0};
	SOC_NPU_AICORE_DBG_ADDR_1_UNION dbg_addr_1 = {0};
	SOC_NPU_AICORE_DBG_RW_CTRL_UNION dbg_rw_ctrl = {0};
	SOC_NPU_AICORE_DBG_STATUS_UNION dbg_status = {0};
	SOC_NPU_AICORE_DBG_DATA0_0_UNION dbg_data00 = {0};
	SOC_NPU_AICORE_DBG_DATA0_1_UNION dbg_data01 = {0};
	u64 base = npu_aicore_get_base_addr(aic_id);

	cond_return_error(base == 0ULL, -1, "aicore base is NULL\n");
	npu_drv_debug("base = 0x%pK\n", base);

	/* step 1 */
	clk_gate_mask_1.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_AICORE_CLK_GATE_MASK_1_ADDR(base));
	clk_gate_mask_1.reg.debug_mode_en = 0x1;
	writel(clk_gate_mask_1.value, (volatile void *)(uintptr_t)SOC_NPU_AICORE_CLK_GATE_MASK_1_ADDR(base));
	npu_drv_debug("clk_gate_mask_1.value = 0x%016lx\n", clk_gate_mask_1.value);

	/* step 2 */
	dbg_addr_0.reg.dbg_addr_0 = debug_addr_id & UINT32_MAX;
	writel(dbg_addr_0.value, (volatile void *)(uintptr_t)SOC_NPU_AICORE_DBG_ADDR_0_ADDR(base));
	npu_drv_debug("dbg_addr_0.value = 0x%016lx\n", dbg_addr_0.value);
	dbg_addr_1.reg.dfx_en = 0x0;
	dbg_addr_1.reg.dbg_sel = 0x0;
	dbg_addr_1.reg.dbg_addr_1 = (unsigned int)(debug_addr_id >> 32);
	writel(dbg_addr_1.value, (volatile void *)(uintptr_t)SOC_NPU_AICORE_DBG_ADDR_1_ADDR(base));
	npu_drv_debug("dbg_addr_1.value = 0x%016lx\n", dbg_addr_1.value);

	/* step 3 */
	dbg_rw_ctrl.reg.dbg_ctrl_rd = 0x1;
	writel(dbg_rw_ctrl.value, (volatile void *)(uintptr_t)SOC_NPU_AICORE_DBG_RW_CTRL_ADDR(base));
	npu_drv_debug("dbg_rw_ctrl.value = 0x%016lx\n", dbg_rw_ctrl.value);

	/* step 4 */
	ret = npu_aicore_dbg_busy_wait(
		SOC_NPU_AICORE_DBG_RW_CTRL_ADDR(base), &dbg_rw_ctrl.value);

	/* step 5 */
	clk_gate_mask_1.reg.debug_mode_en = 0x0;
	writel(clk_gate_mask_1.value, (volatile void *)(uintptr_t)SOC_NPU_AICORE_CLK_GATE_MASK_1_ADDR(base));
	npu_drv_debug("clk_gate_mask.value = 0x%016lx\n", clk_gate_mask_1.value);

	/* step 6 */
	if (ret != 0) {
		dbg_status.value = readl((const volatile void *)(uintptr_t)SOC_NPU_AICORE_DBG_STATUS_ADDR(base));
		if (dbg_status.reg.dbg_read_err == 0x1)
			npu_drv_warn("register read error\n");
		else
			npu_drv_warn("read register timed out\n");
		return -1;
	}

	/* step 7 */
	dbg_data00.value = readl((const volatile void *)(uintptr_t)SOC_NPU_AICORE_DBG_DATA0_0_ADDR(base));
	dbg_data01.value = readl((const volatile void *)(uintptr_t)SOC_NPU_AICORE_DBG_DATA0_1_ADDR(base));
	*read_buf = ((u64)dbg_data01.reg.dbg_data0_1 << 32) | dbg_data00.reg.dbg_data0_0;
	npu_drv_debug("*read_buf = 0x%016llx\n", *read_buf);

	return 0;
}

int npu_aicore_query_exception_dbg_info(
	u8 aic_id, struct aicore_exception_dbg_info *aic_info)
{
	u64 read_buf = 0;
	int ret;
	u32 reg_id;
	const u32 x_reg_base = 64;

	cond_return_error(aic_info == NULL, -EINVAL, "aic_info is NULL\n");

	/* ifu current */
	ret = npu_aicore_read_dbg_register(aic_id, 0x0, &read_buf);
	if (ret == 0) {
		aic_info->ifu_current = read_buf;
		npu_drv_warn("ifu current is at 0x%016llx\n", read_buf);
	}

	/* x0-x31 */
	for (reg_id = 0; reg_id < 32; reg_id++) {
		ret = npu_aicore_read_dbg_register(
			aic_id, x_reg_base + reg_id, &read_buf);
		if (ret == 0) {
			aic_info->general_register[reg_id] = read_buf;
			npu_drv_warn("general purpose register x%u = 0x%016llx\n",
					reg_id, read_buf);
		}
	}

	return 0;
}

static int npu_aiv_query_exception_info(u64 base)
{
	SOC_NPU_AICORE_AIC_VEC_210_ERR_INFO_0_UNION vec_210_err_info_0 = {0};
	SOC_NPU_AICORE_AIC_VEC_210_ERR_INFO_1_UNION vec_210_err_info_1 = {0};
	SOC_NPU_AICORE_AIC_VEC_210_ERR_INFO_2_UNION vec_210_err_info_2 = {0};
	SOC_NPU_AICORE_AIC_VEC_210_ERR_INFO_3_UNION vec_210_err_info_3 = {0};
	SOC_NPU_AICORE_AIC_VEC_210_ERR_INFO_4_UNION vec_210_err_info_4 = {0};
	SOC_NPU_AICORE_AIC_VEC_210_ERR_INFO_5_UNION vec_210_err_info_5 = {0};
	SOC_NPU_AICORE_AIC_VEC_210_ERR_INFO_6_UNION vec_210_err_info_6 = {0};
	SOC_NPU_AICORE_AIC_VEC_210_ERR_INFO_7_UNION vec_210_err_info_7 = {0};

	cond_return_error(base == 0ULL, -EINVAL, "aicore base is NULL\n");

	vec_210_err_info_0.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_AICORE_AIC_VEC_210_ERR_INFO_0_ADDR(base));
	vec_210_err_info_1.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_AICORE_AIC_VEC_210_ERR_INFO_1_ADDR(base));
	vec_210_err_info_2.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_AICORE_AIC_VEC_210_ERR_INFO_2_ADDR(base));
	vec_210_err_info_3.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_AICORE_AIC_VEC_210_ERR_INFO_3_ADDR(base));
	vec_210_err_info_4.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_AICORE_AIC_VEC_210_ERR_INFO_4_ADDR(base));
	vec_210_err_info_5.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_AICORE_AIC_VEC_210_ERR_INFO_5_ADDR(base));
	vec_210_err_info_6.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_AICORE_AIC_VEC_210_ERR_INFO_6_ADDR(base));
	vec_210_err_info_7.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_AICORE_AIC_VEC_210_ERR_INFO_7_ADDR(base));
	npu_drv_warn("AIC_VEC_210_ERR_INFO_0.value = 0x%016llx\n", vec_210_err_info_0.value);
	npu_drv_warn("AIC_VEC_210_ERR_INFO_1.value = 0x%016llx\n", vec_210_err_info_1.value);
	npu_drv_warn("AIC_VEC_210_ERR_INFO_2.value = 0x%016llx\n", vec_210_err_info_2.value);
	npu_drv_warn("AIC_VEC_210_ERR_INFO_3.value = 0x%016llx\n", vec_210_err_info_3.value);
	npu_drv_warn("AIC_VEC_210_ERR_INFO_4.value = 0x%016llx\n", vec_210_err_info_4.value);
	npu_drv_warn("AIC_VEC_210_ERR_INFO_5.value = 0x%016llx\n", vec_210_err_info_5.value);
	npu_drv_warn("AIC_VEC_210_ERR_INFO_6.value = 0x%016llx\n", vec_210_err_info_6.value);
	npu_drv_warn("AIC_VEC_210_ERR_INFO_7.value = 0x%016llx\n", vec_210_err_info_7.value);
	return 0;
}

int npu_aicore_query_exception_info(
	u8 aic_id, struct aicore_exception_info *aic_info)
{
	SOC_NPU_AICORE_PC_START_0_UNION ifu_start_0 = {0};
	SOC_NPU_AICORE_PC_START_1_UNION ifu_start_1 = {0};
	SOC_NPU_AICORE_AIC_ERROR_0_UNION aic_error_0 = {0};
	SOC_NPU_AICORE_AIC_ERROR_1_UNION aic_error_1 = {0};
	SOC_NPU_AICORE_AIC_ERROR_2_UNION aic_error_2 = {0};
	SOC_NPU_AICORE_AIC_ERROR_3_UNION aic_error_3 = {0};
	SOC_NPU_AICORE_AIC_ERROR_4_UNION aic_error_4 = {0};
	SOC_NPU_AICORE_AIC_ERROR_5_UNION aic_error_5 = {0}; // aiv error
	SOC_NPU_AICORE_AIC_ERROR_MASK_0_UNION aic_error_mask_0 = {0};
	SOC_NPU_AICORE_AIC_ERROR_MASK_1_UNION aic_error_mask_1 = {0};
	SOC_NPU_AICORE_AI_CORE_INT_UNION aic_core_int = {0};
	u64 base = npu_aicore_get_base_addr(aic_id);

	cond_return_error(base == 0ULL, -EINVAL, "aicore base is NULL\n");
	cond_return_error(aic_info == NULL, -EINVAL, "aic_info is NULL\n");

	ifu_start_0.value = readl((const volatile void *)(uintptr_t)SOC_NPU_AICORE_PC_START_0_ADDR(base));
	ifu_start_1.value = readl((const volatile void *)(uintptr_t)SOC_NPU_AICORE_PC_START_1_ADDR(base));
	aic_info->ifu_start = ((u64)ifu_start_1.reg.va_1 << 32) | ifu_start_0.reg.va_0;
	npu_drv_warn("ifu start = %016llx\n", aic_info->ifu_start);

	aic_error_0.value = readl((const volatile void *)(uintptr_t)SOC_NPU_AICORE_AIC_ERROR_0_ADDR(base));
	aic_error_1.value = readl((const volatile void *)(uintptr_t)SOC_NPU_AICORE_AIC_ERROR_1_ADDR(base));
	aic_info->aic_error = ((u64)aic_error_1.value << 32) | aic_error_0.value;
	npu_drv_warn("AIC_ERROR.value = 0x%016llx\n", aic_info->aic_error);

	aic_error_2.value = readl((const volatile void *)(uintptr_t)SOC_NPU_AICORE_AIC_ERROR_2_ADDR(base));
	aic_error_3.value = readl((const volatile void *)(uintptr_t)SOC_NPU_AICORE_AIC_ERROR_3_ADDR(base));
	aic_info->aic_error1 = ((u64)aic_error_3.value << 32) | aic_error_2.value;
	npu_drv_warn("AIC_ERROR1.value = 0x%016llx\n", aic_info->aic_error1);

	aic_error_4.value = readl((const volatile void *)(uintptr_t)SOC_NPU_AICORE_AIC_ERROR_4_ADDR(base));
	aic_error_5.value = readl((const volatile void *)(uintptr_t)SOC_NPU_AICORE_AIC_ERROR_5_ADDR(base));
	aic_info->aic_error2 = ((u64)aic_error_5.value << 32) | aic_error_4.value;
	npu_drv_warn("AIC_ERROR2.value = 0x%016llx\n", aic_info->aic_error2);

	aic_error_mask_0.value = readl((const volatile void *)(uintptr_t)SOC_NPU_AICORE_AIC_ERROR_MASK_0_ADDR(base));
	aic_error_mask_1.value = readl((const volatile void *)(uintptr_t)SOC_NPU_AICORE_AIC_ERROR_MASK_1_ADDR(base));
	aic_info->aic_error_mask = ((u64)aic_error_mask_1.value << 32) | aic_error_mask_0.value;
	npu_drv_warn(
			"AIC_ERROR_MASK.value = 0x%016llx\n", aic_info->aic_error_mask);

	aic_core_int.value = readl((const volatile void *)(uintptr_t)SOC_NPU_AICORE_AI_CORE_INT_ADDR(base));
	aic_info->aic_core_int = aic_core_int.value;
	npu_drv_warn("AI_CORE_INT.value = 0x%x\n", aic_core_int.value);

	if (aic_id == NPU_AIVECTOR_0)
		return npu_aiv_query_exception_info(base);

	return 0;
}

int npu_aicore_query_exception_error_info(
	u8 aic_id, struct aicore_exception_error_info *aic_info)
{
	SOC_NPU_AICORE_BIU_ERR_INFO_0_UNION biu_err_info_0 = {0};
	SOC_NPU_AICORE_BIU_ERR_INFO_1_UNION biu_err_info_1 = {0};
	SOC_NPU_AICORE_CCU_ERR_INFO_0_UNION ccu_err_info_0 = {0};
	SOC_NPU_AICORE_CCU_ERR_INFO_1_UNION ccu_err_info_1 = {0};
	SOC_NPU_AICORE_CUBE_ERR_INFO_UNION cube_err_info = {0};
	SOC_NPU_AICORE_MTE_ERR_INFO_UNION mte_err_info = {0};
	SOC_NPU_AICORE_VEC_ERR_INFO_UNION vec_err_info = {0}; // aiv
	SOC_NPU_AICORE_MTE_SU_ECC_1BIT_ERR_0_UNION mte_ecc_1bit_err_0 = {0};
	SOC_NPU_AICORE_MTE_SU_ECC_1BIT_ERR_1_UNION mte_ecc_1bit_err_1 = {0};
	SOC_NPU_AICORE_VEC_CUBE_ECC_1BIT_ERR_0_UNION vec_cube_ecc_1bit_err_0 = {0}; // aiv cube
	SOC_NPU_AICORE_VEC_CUBE_ECC_1BIT_ERR_1_UNION vec_cube_ecc_1bit_err_1 = {0}; // aiv cube
	u64 base = npu_aicore_get_base_addr(aic_id);

	cond_return_error(base == 0ULL, -EINVAL, "aicore base is NULL\n");
	cond_return_error(aic_info == NULL, -EINVAL, "aic_info is NULL\n");

	biu_err_info_0.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_AICORE_BIU_ERR_INFO_0_ADDR(base));
	biu_err_info_1.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_AICORE_BIU_ERR_INFO_1_ADDR(base));
	aic_info->biu_err_info = ((u64)biu_err_info_1.value << 32) | biu_err_info_0.value;
	npu_drv_warn("BIU_ERR_INFO.value = 0x%016llx\n", aic_info->biu_err_info);

	ccu_err_info_0.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_AICORE_CCU_ERR_INFO_0_ADDR(base));
	ccu_err_info_1.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_AICORE_CCU_ERR_INFO_1_ADDR(base));
	aic_info->ccu_err_info = ((u64)ccu_err_info_1.value << 32) | ccu_err_info_0.value;
	npu_drv_warn("CCU_ERR_INFO.value = 0x%016llx\n", aic_info->ccu_err_info);

	cube_err_info.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_AICORE_CUBE_ERR_INFO_ADDR(base));
	aic_info->cube_err_info = cube_err_info.value;
	npu_drv_warn("CUBE_ERR_INFO.value = 0x%x\n", cube_err_info.value);

	mte_err_info.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_AICORE_MTE_ERR_INFO_ADDR(base));
	aic_info->mte_err_info = mte_err_info.value;
	npu_drv_warn("MTE_ERR_INFO.value = 0x%x\n", mte_err_info.value);

	vec_err_info.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_AICORE_VEC_ERR_INFO_ADDR(base));
	aic_info->vec_err_info = vec_err_info.value;
	npu_drv_warn("VEC_ERR_INFO.value = 0x%x\n", vec_err_info.value);

	mte_ecc_1bit_err_0.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_AICORE_MTE_SU_ECC_1BIT_ERR_0_ADDR(base));
	mte_ecc_1bit_err_1.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_AICORE_MTE_SU_ECC_1BIT_ERR_1_ADDR(base));
	aic_info->mte_ecc_1bit_err = ((u64)mte_ecc_1bit_err_1.value << 32) | mte_ecc_1bit_err_0.value;
	npu_drv_warn(
			"MTE_ECC_1BIT_ERR.value = 0x%016llx\n", aic_info->mte_ecc_1bit_err);

	vec_cube_ecc_1bit_err_0.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_AICORE_VEC_CUBE_ECC_1BIT_ERR_0_ADDR(base));
	vec_cube_ecc_1bit_err_1.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_AICORE_VEC_CUBE_ECC_1BIT_ERR_1_ADDR(base));
	aic_info->vec_cube_ecc_1bit_err = ((u64)vec_cube_ecc_1bit_err_1.value << 32) | vec_cube_ecc_1bit_err_0.value;
	npu_drv_warn("VEC_CUBE_ECC_1BIT_ERR.value = 0x%016llx\n",
			aic_info->vec_cube_ecc_1bit_err);

	return 0;
}
