/*
 * npu_hwts_plat.c
 *
 * about npu hwts plat
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
#include "npu_hwts_plat.h"
#include <linux/io.h>
#include "npu_log.h"

u64 npu_hwts_get_base_addr(void)
{
	u64 hwts_base = 0ULL;
	struct npu_platform_info *plat_info = npu_plat_get_info();

	cond_return_error(plat_info == NULL, 0ULL, "get platform info failed\n");
	hwts_base = (u64)(uintptr_t)plat_info->dts_info.reg_vaddr[NPU_REG_HWTS_BASE];
	npu_drv_debug("hwts_base = 0x%pK\n", hwts_base);
	return hwts_base;
}

static u16 hwts_get_acsq_id_by_rtsq_id(u16 channel_id)
{
	SOC_NPU_HWTS_SQ_TO_ACTIVE_SQ_MAP0_UNION map0 = {0};
	SOC_NPU_HWTS_SQ_TO_ACTIVE_SQ_MAP1_UNION map1 = {0};
	u64 hwts_base = npu_hwts_get_base_addr();

	cond_return_error(hwts_base == 0ULL, MAX_UINT16_NUM, "hwts_base is NULL\n");

	map0.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_HWTS_SQ_TO_ACTIVE_SQ_MAP0_ADDR(hwts_base, channel_id));
	map1.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_HWTS_SQ_TO_ACTIVE_SQ_MAP1_ADDR(hwts_base, channel_id));

	if (map1.reg.mapped_in_active_set == 0) {
		npu_drv_info("rtsq id %u is not activated", channel_id);
		return MAX_UINT16_NUM;
	}

	return map0.reg.mapped_in_active_sqid;
}

int npu_hwts_query_aicore_pool_status(
	u8 index, u8 *reg_val)
{
	SOC_NPU_HWTS_HWTS_AICORE_POOL_STATUS0_NS_UNION aicore_pool_status0_ns = {0};
	SOC_NPU_HWTS_HWTS_AICORE_POOL_STATUS1_NS_UNION aicore_pool_status1_ns = {0};
	SOC_NPU_HWTS_HWTS_AICORE_POOL_STATUS2_NS_UNION aicore_pool_status2_ns = {0};
	SOC_NPU_HWTS_HWTS_AICORE_POOL_STATUS3_NS_UNION aicore_pool_status3_ns = {0};

	u64 hwts_base = npu_hwts_get_base_addr();

	cond_return_error(hwts_base == 0ULL, -EINVAL, "hwts_base is NULL\n");
	cond_return_error(reg_val == NULL, -EINVAL, "reg_val is NULL\n");

	if (index == 0) {
		aicore_pool_status0_ns.value = readl(
			(const volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_AICORE_POOL_STATUS0_NS_ADDR(hwts_base));
		*reg_val = aicore_pool_status0_ns.reg.aic_enabled_status0_ns;
	} else if (index == 1) {
		aicore_pool_status1_ns.value = readl(
			(const volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_AICORE_POOL_STATUS1_NS_ADDR(hwts_base));
		*reg_val = aicore_pool_status1_ns.reg.aic_enabled_status1_ns;
	} else if (index == 2) {
		aicore_pool_status2_ns.value = readl(
			(const volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_AICORE_POOL_STATUS2_NS_ADDR(hwts_base));
		*reg_val = aicore_pool_status2_ns.reg.aic_enabled_status2_ns;
	} else {
		aicore_pool_status3_ns.value = readl(
			(const volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_AICORE_POOL_STATUS3_NS_ADDR(hwts_base));
		*reg_val = aicore_pool_status3_ns.reg.aic_enabled_status3_ns;
	}

	npu_drv_warn(
		"HWTS_AICORE_POOL_STATUS%u_NS = 0x%x\n", index, *reg_val);
	return 0;
}

int npu_hwts_query_aiv_pool_status(
	u8 index, u8 *reg_val)
{
	SOC_NPU_HWTS_HWTS_VEC_POOL_STATUS0_NS_UNION vec_pool_status0_ns = {0};
	SOC_NPU_HWTS_HWTS_VEC_POOL_STATUS1_NS_UNION vec_pool_status1_ns = {0};
	SOC_NPU_HWTS_HWTS_VEC_POOL_STATUS2_NS_UNION vec_pool_status2_ns = {0};
	SOC_NPU_HWTS_HWTS_VEC_POOL_STATUS3_NS_UNION vec_pool_status3_ns = {0};

	u64 hwts_base = npu_hwts_get_base_addr();

	cond_return_error(hwts_base == 0ULL, -EINVAL, "hwts_base is NULL\n");
	cond_return_error(reg_val == NULL, -EINVAL, "reg_val is NULL\n");

	if (index == 0) {
		vec_pool_status0_ns.value = readl(
			(const volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_VEC_POOL_STATUS0_NS_ADDR(hwts_base));
		*reg_val = vec_pool_status0_ns.reg.vec_enabled_status0_ns;
	} else if (index == 1) {
		vec_pool_status1_ns.value = readl(
			(const volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_VEC_POOL_STATUS1_NS_ADDR(hwts_base));
		*reg_val = vec_pool_status1_ns.reg.vec_enabled_status1_ns;
	} else if (index == 2) {
		vec_pool_status2_ns.value = readl(
			(const volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_VEC_POOL_STATUS2_NS_ADDR(hwts_base));
		*reg_val = vec_pool_status2_ns.reg.vec_enabled_status2_ns;
	} else {
		vec_pool_status3_ns.value = readl(
			(const volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_VEC_POOL_STATUS3_NS_ADDR(hwts_base));
		*reg_val = vec_pool_status3_ns.reg.vec_enabled_status3_ns;
	}
	npu_drv_warn(
			"HWTS_VEC_POOL_STATUS%u_NS = 0x%x\n", index, *reg_val);
	return 0;
}

int npu_hwts_query_bus_error(void)
{
	SOC_NPU_HWTS_HWTS_BUS_ERR_INFO_UNION bus_err_info = {0};
	u64 hwts_base = npu_hwts_get_base_addr();

	cond_return_error(hwts_base == 0ULL, -EINVAL, "hwts_base is NULL\n");
	bus_err_info.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_BUS_ERR_INFO_ADDR(hwts_base));

	npu_drv_warn("HWTS_BUS_ERR_INFO.bus_err_type=0x%x, bus_err_id=0x%x\n",
			bus_err_info.reg.bus_err_type, bus_err_info.reg.bus_err_id);
	return 0;
}

int npu_hwts_query_sw_status(u16 channel_id, u32 *reg_val)
{
	SOC_NPU_HWTS_HWTS_SQ_SW_STATUS_UNION sq_sw_status = {0};
	u64 hwts_base = npu_hwts_get_base_addr();

	cond_return_error(hwts_base == 0ULL, -EINVAL, "hwts_base is NULL\n");
	cond_return_error(reg_val == NULL, -EINVAL, "reg_val is NULL\n");

	sq_sw_status.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_SQ_SW_STATUS_ADDR(hwts_base, channel_id));
	*reg_val = sq_sw_status.reg.sq_sw_status;
	npu_drv_warn("HWTS_BUS_ERR_INFO.sq_sw_status = 0x%08x\n",
			sq_sw_status.reg.sq_sw_status);
	return 0;
}

int npu_hwts_query_sq_head(u16 channel_id, u16 *reg_val)
{
	SOC_NPU_HWTS_HWTS_SQ_CFG4_UNION sq_cfg4 = {0};
	u64 hwts_base = npu_hwts_get_base_addr();

	cond_return_error(hwts_base == 0ULL, -EINVAL, "hwts_base is NULL\n");
	cond_return_error(reg_val == NULL, -EINVAL, "reg_val is NULL\n");

	sq_cfg4.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_SQ_CFG4_ADDR(hwts_base, channel_id));
	*reg_val = sq_cfg4.reg.sq_head;
	npu_drv_warn("HWTS_SQ_CFG4.sq_head = 0x%04x\n", sq_cfg4.reg.sq_head);
	return 0;
}

int npu_hwts_query_sqe_type(u16 channel_id, u8 *reg_val)
{
	u16 acsq_id;
	SOC_NPU_HWTS_SQCQ_FSM_MISC_STATE0_UNION sqcq_fsm_misc_state0 = {0};
	u64 hwts_base = npu_hwts_get_base_addr();

	cond_return_error(hwts_base == 0ULL, -EINVAL, "hwts_base is NULL\n");
	cond_return_error(reg_val == NULL, -EINVAL, "reg_val is NULL\n");

	acsq_id = hwts_get_acsq_id_by_rtsq_id(channel_id);
	cond_return_warn(acsq_id == MAX_UINT16_NUM, 0, "invalid acsq id\n");
	sqcq_fsm_misc_state0.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_HWTS_SQCQ_FSM_MISC_STATE0_ADDR(hwts_base, acsq_id));
	*reg_val = sqcq_fsm_misc_state0.reg.sqe_type;
	npu_drv_warn("SQCQ_FSM_MISC_STATE.sqe_type = 0x%x\n",
			sqcq_fsm_misc_state0.reg.sqe_type);
	return 0;
}

static int npu_hwts_query_ip_own_bitmap(u16 channel_id, u8 *reg_val)
{
	u16 acsq_id;
	SOC_NPU_HWTS_SQCQ_FSM_IP_OWN_STATE_UNION sqcq_fsm_state = {0};
	u64 hwts_base = npu_hwts_get_base_addr();

	cond_return_error(hwts_base == 0, -EINVAL, "hwts_base is NULL\n");
	cond_return_error(reg_val == NULL, -EINVAL, "reg_val is NULL\n");

	acsq_id = hwts_get_acsq_id_by_rtsq_id(channel_id);
	cond_return_warn(acsq_id == MAX_UINT16_NUM, 0, "invalid acsq id\n");
	sqcq_fsm_state.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_HWTS_SQCQ_FSM_IP_OWN_STATE_ADDR(hwts_base, acsq_id));
	*reg_val = sqcq_fsm_state.reg.ip_own_bitmap;
	npu_drv_warn("SQCQ_FSM_IP_OWN_STATE.ip_own_bitmap = 0x%x\n",
			sqcq_fsm_state.reg.ip_own_bitmap);
	return 0;
}

static int npu_hwts_query_ip_exception_bitmap(u16 channel_id, u8 *reg_val)
{
	u16 acsq_id;
	SOC_NPU_HWTS_SQCQ_FSM_IP_EXCEPTION_STATE_UNION sqcq_fsm_state = {0};
	u64 hwts_base = npu_hwts_get_base_addr();

	cond_return_error(hwts_base == 0, -EINVAL, "hwts_base is NULL\n");
	cond_return_error(reg_val == NULL, -EINVAL, "reg_val is NULL\n");

	acsq_id = hwts_get_acsq_id_by_rtsq_id(channel_id);
	cond_return_warn(acsq_id == MAX_UINT16_NUM, 0, "invalid acsq id\n");
	sqcq_fsm_state.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_HWTS_SQCQ_FSM_IP_EXCEPTION_STATE_ADDR(hwts_base, acsq_id));
	*reg_val = sqcq_fsm_state.reg.ip_exception_bitmap;
	npu_drv_warn("SQCQ_FSM_IP_EXCEPTION_STATE.ip_exception_bitmap = 0x%x\n",
			sqcq_fsm_state.reg.ip_exception_bitmap);
	return 0;
}

static int npu_hwts_query_ip_trap_bitmap(u16 channel_id, u8 *reg_val)
{
	u16 acsq_id;
	SOC_NPU_HWTS_SQCQ_FSM_IP_TRAP_STATE_UNION sqcq_fsm_state = {0};
	u64 hwts_base = npu_hwts_get_base_addr();

	cond_return_error(hwts_base == 0, -EINVAL, "hwts_base is NULL\n");
	cond_return_error(reg_val == NULL, -EINVAL, "reg_val is NULL\n");

	acsq_id = hwts_get_acsq_id_by_rtsq_id(channel_id);
	cond_return_warn(acsq_id == MAX_UINT16_NUM, 0, "invalid acsq id\n");
	sqcq_fsm_state.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_HWTS_SQCQ_FSM_IP_TRAP_STATE_ADDR(hwts_base, acsq_id));
	*reg_val = sqcq_fsm_state.reg.ip_trap_bitmap;
	npu_drv_warn("SQCQ_FSM_IP_TRAP_STATE.aic_trap_bitmap = 0x%x\n",
			sqcq_fsm_state.reg.ip_trap_bitmap);
	return 0;
}

int npu_hwts_query_aic_own_bitmap(u16 channel_id, u8 *reg_val)
{
	return npu_hwts_query_ip_own_bitmap(channel_id, reg_val);
}

int npu_hwts_query_aic_exception_bitmap(u16 channel_id, u8 *reg_val)
{
	return npu_hwts_query_ip_exception_bitmap(channel_id, reg_val);
}

int npu_hwts_query_aic_trap_bitmap(u16 channel_id, u8 *reg_val)
{
	return npu_hwts_query_ip_trap_bitmap(channel_id, reg_val);
}

int npu_hwts_query_sdma_own_state(u16 channel_id, u8 *reg_val)
{
	return npu_hwts_query_ip_own_bitmap(channel_id, reg_val);
}

int npu_hwts_query_aic_task_config(void)
{
	u64 value;
	u64 hwts_base = npu_hwts_get_base_addr();

	cond_return_error(hwts_base == 0, -EINVAL, "hwts_base is NULL\n");

	value = readl((const volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_FREE_AI_CORE_BITMAP_ADDR(hwts_base));
	npu_drv_warn("HWTS_FREE_AI_CORE_BITMAP = 0x%llx\n", value);
	value = readl((const volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_AIC_BLK_FSM_SEL_ADDR(hwts_base));
	npu_drv_warn("HWTS_AIC_BLK_FSM_SEL = 0x%llx\n", value);
	value = readl((const volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_AIC_BLK_FSM_STATE_ADDR(hwts_base));
	npu_drv_warn("HWTS_AIC_BLK_FSM_STATE = 0x%llx\n", value);
	value = readl((const volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_L2_OWNER_ADDR(hwts_base));
	npu_drv_warn("HWTS_L2_OWNER = 0x%llx\n", value);

	return 0;
}

int npu_hwts_query_interrupt_info(
	struct hwts_interrupt_info *interrupt_info)
{
	SOC_NPU_HWTS_HWTS_GLB_CTRL2_UNION glb_ctrl2 = {0};
	SOC_NPU_HWTS_HWTS_GLB_CTRL3_UNION glb_ctrl3 = {0};
	SOC_NPU_HWTS_HWTS_L1_NORMAL_NS_INTERRUPT_UNION l1_normal_ns_interrupt = {0};
	SOC_NPU_HWTS_HWTS_L1_DEBUG_NS_INTERRUPT_UNION l1_debug_ns_interrupt = {0};
	SOC_NPU_HWTS_HWTS_L1_ERROR_NS_INTERRUPT_UNION l1_error_ns_interrupt = {0};
	SOC_NPU_HWTS_HWTS_L1_DFX_INTERRUPT_UNION l1_dfx_interrupt = {0};
	u64 hwts_base = npu_hwts_get_base_addr();

	cond_return_error(hwts_base == 0ULL, -EINVAL, "hwts_base is NULL\n");
	cond_return_error(interrupt_info == NULL,
		-EINVAL, "interrupt_info is NULL\n");

	glb_ctrl2.value = readl((const volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_GLB_CTRL2_ADDR(hwts_base));
	glb_ctrl3.value = readl((const volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_GLB_CTRL3_ADDR(hwts_base));
	interrupt_info->aic_batch_mode_timeout_man =
		glb_ctrl2.reg.aic_batch_mode_timeout_man;
	interrupt_info->aic_task_runtime_limit_exp =
		glb_ctrl3.reg.aic_task_runtime_limit_exp;
	interrupt_info->vec_batch_mode_timeout_man =
		glb_ctrl2.reg.vec_batch_mode_timeout_man;
	interrupt_info->vec_task_runtime_limit_exp =
		glb_ctrl3.reg.vec_task_runtime_limit_exp;
	interrupt_info->wait_task_limit_exp = glb_ctrl3.reg.aic_task_runtime_limit_exp;
	npu_drv_warn("HWTS_GLB_CTRL2.aic_batch_mode_timeout_man = 0x%x\n",
		glb_ctrl2.reg.aic_batch_mode_timeout_man);
	npu_drv_warn("HWTS_GLB_CTRL3.aic_task_runtime_limit_exp = 0x%x\n",
		glb_ctrl3.reg.aic_task_runtime_limit_exp);
	npu_drv_warn("HWTS_GLB_CTRL2.vec_batch_mode_timeout_man = 0x%x\n",
		glb_ctrl2.reg.vec_batch_mode_timeout_man);
	npu_drv_warn("HWTS_GLB_CTRL3.vec_task_runtime_limit_exp = 0x%x\n",
		glb_ctrl3.reg.vec_task_runtime_limit_exp);
	npu_drv_warn("HWTS_GLB_CTRL3.wait_task_limit_exp = 0x%x\n", glb_ctrl3.reg.aic_task_runtime_limit_exp);

	l1_normal_ns_interrupt.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_L1_NORMAL_NS_INTERRUPT_ADDR(hwts_base));
	interrupt_info->l1_normal_ns_interrupt = l1_normal_ns_interrupt.value;
	npu_drv_warn("HWTS_L1_NORMAL_NS_INTERRUPT.value = 0x%x\n",
			l1_normal_ns_interrupt.value);

	l1_debug_ns_interrupt.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_L1_DEBUG_NS_INTERRUPT_ADDR(hwts_base));
	interrupt_info->l1_debug_ns_interrupt = l1_debug_ns_interrupt.value;
	npu_drv_warn("HWTS_L1_DEBUG_NS_INTERRUPT.value = 0x%x\n",
			l1_debug_ns_interrupt.value);

	l1_error_ns_interrupt.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_L1_ERROR_NS_INTERRUPT_ADDR(hwts_base));
	interrupt_info->l1_error_ns_interrupt = l1_error_ns_interrupt.value;
	npu_drv_warn("HWTS_L1_ERROR_NS_INTERRUPT.value = 0x%x\n",
			l1_error_ns_interrupt.value);

	l1_dfx_interrupt.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_L1_DFX_INTERRUPT_ADDR(hwts_base));
	interrupt_info->l1_dfx_interrupt = l1_dfx_interrupt.value;
	npu_drv_warn("HWTS_L1_DFX_INTERRUPT.value = 0x%x\n",
			l1_dfx_interrupt.value);

	return 0;
}

static void npu_hwts_query_acsq_info(u64 hwts_base, u16 acsq_id, struct sq_exception_info *sq_info)
{
	SOC_NPU_HWTS_HWTS_SQ_CFG3_UNION sq_cfg3 = {0};
	SOC_NPU_HWTS_SQCQ_FSM_STATE0_UNION sqcq_fsm_state0 = {0};
	SOC_NPU_HWTS_SQCQ_FSM_STATE1_UNION sqcq_fsm_state1 = {0};
	SOC_NPU_HWTS_SQCQ_FSM_MISC_STATE0_UNION sqcq_misc_state0 = {0};
	SOC_NPU_HWTS_SQCQ_FSM_MISC_STATE1_UNION sqcq_misc_state1 = {0};

	sq_cfg3.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_SQ_CFG3_ADDR(hwts_base, acsq_id));
	sq_info->sq_length = sq_cfg3.reg.sq_length;
	sqcq_fsm_state0.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_HWTS_SQCQ_FSM_STATE0_ADDR(hwts_base, acsq_id));
	sq_info->sqcq_fsm_state = sqcq_fsm_state0.reg.dfx_sqcq_fsm_state;
	sqcq_fsm_state1.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_HWTS_SQCQ_FSM_STATE1_ADDR(hwts_base, acsq_id));
	sq_info->sq_prefetch_busy = sqcq_fsm_state1.reg.sq_prefetch_busy;
	sqcq_misc_state0.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_HWTS_SQCQ_FSM_MISC_STATE0_ADDR(hwts_base, acsq_id));
	sq_info->sqe_type = sqcq_misc_state0.reg.sqe_type;
	sqcq_misc_state1.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_HWTS_SQCQ_FSM_MISC_STATE1_ADDR(hwts_base, acsq_id));
	sq_info->blk_dim = sqcq_misc_state1.reg.blk_dim;
	sq_info->kernel_credit = sqcq_misc_state1.reg.kernel_credit;

	npu_drv_warn("HWTS_SQ_CFG3.sq_length = 0x%04x\n", sq_cfg3.reg.sq_length);
	npu_drv_warn("HWTS_SQCQ_FSM_STATE0.reg.dfx_sqcq_fsm_state = 0x%04x\n", sqcq_fsm_state0.reg.dfx_sqcq_fsm_state);
	npu_drv_warn("HWTS_SQCQ_FSM_STATE1.reg.sq_prefetch_busy = 0x%04x\n", sqcq_fsm_state1.reg.sq_prefetch_busy);
	npu_drv_warn("HWTS_SQCQ_FSM_MISC_STATE0.reg.sqe_type = 0x%04x\n", sqcq_misc_state0.reg.sqe_type);
	npu_drv_warn("HWTS_SQCQ_FSM_MISC_STATE1.reg.blk_dim = 0x%04x\n", sqcq_misc_state1.reg.blk_dim);
	npu_drv_warn("HWTS_SQCQ_FSM_MISC_STATE1.reg.kernel_credit = 0x%04x\n", sqcq_misc_state1.reg.kernel_credit);
}

static void npu_hwts_dump_acsq_prefetch_head(u64 hwts_base, u16 acsq_id)
{
	u32 sqe[HWTS_SQE_SIZE_IN_U32];
	u64 base_addr = hwts_base + HWTS_PREFETCH_HEAD_OFFSET + acsq_id * HWTS_PREFETCH_HEAD_PER_SIZE;
	u32 i;

	for (i = 0; i < HWTS_SQE_SIZE_IN_U32; i++)
		sqe[i] = readl((const volatile void *)(uintptr_t)(base_addr + i * 4));

	npu_drv_warn("present sqe:0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x "
		"0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x",
		sqe[0], sqe[1], sqe[2], sqe[3], sqe[4], sqe[5], sqe[6], sqe[7],
		sqe[8], sqe[9], sqe[10], sqe[11], sqe[12], sqe[13], sqe[14], sqe[15]);
}

int npu_hwts_query_sq_info(u16 channel_id, struct sq_exception_info *sq_info)
{
	u16 acsq_id;
	SOC_NPU_HWTS_HWTS_SQ_DB_UNION sq_db = {0};
	SOC_NPU_HWTS_HWTS_SQ_CFG4_UNION sq_cfg4 = {0};
	u64 hwts_base = npu_hwts_get_base_addr();

	cond_return_error(hwts_base == 0ULL, -EINVAL, "hwts_base is NULL\n");
	cond_return_error(sq_info == NULL, -EINVAL, "interrupt_info is NULL\n");

	sq_db.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_SQ_DB_ADDR(hwts_base, channel_id));
	sq_info->sq_tail = sq_db.reg.sq_tail;
	npu_drv_warn("HWTS_SQ_DB.sq_tail = 0x%04x\n", sq_db.reg.sq_tail);

	sq_cfg4.value = readl(
		(const volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_SQ_CFG4_ADDR(hwts_base, channel_id));
	sq_info->sq_head = sq_cfg4.reg.sq_head;
	npu_drv_warn("HWTS_SQ_CFG4.sq_head = 0x%04x\n", sq_cfg4.reg.sq_head);

	acsq_id = hwts_get_acsq_id_by_rtsq_id(channel_id);
	if (acsq_id != MAX_UINT16_NUM) {
		npu_drv_warn("channel id %u is mapped to %u", channel_id, acsq_id);
		npu_hwts_query_acsq_info(hwts_base, acsq_id, sq_info);
		npu_hwts_dump_acsq_prefetch_head(hwts_base, acsq_id);
	} else {
		npu_drv_warn("channel id %u is not activated", channel_id);
	}
	return 0;
}

int npu_hwts_query_ispnn_info(void)
{
	u32 value;
	u64 hwts_base = npu_hwts_get_base_addr();

	cond_return_error(hwts_base == 0ULL, -EINVAL, "hwts_base is NULL\n");

	/* HWTS_HW_HS */
	value = readl((const volatile void *)(uintptr_t)SOC_NPU_HWTS_HW_HS_CHANNEL_CFG0_ADDR(hwts_base, 0));
	npu_drv_warn("HW_HS_CHANNEL_CFG0 = 0x%x\n", value);
	value = readl((const volatile void *)(uintptr_t)SOC_NPU_HWTS_HW_HS_CHANNEL_CFG1_ADDR(hwts_base, 0));
	npu_drv_warn("HW_HS_CHANNEL_CFG1 = 0x%llx\n", value);
	value = readl((const volatile void *)(uintptr_t)SOC_NPU_HWTS_HW_HS_CHANNEL_CFG2_ADDR(hwts_base, 0));
	npu_drv_warn("HW_HS_CHANNEL_CFG2 = 0x%llx\n", value);
	value = readl((const volatile void *)(uintptr_t)SOC_NPU_HWTS_HW_HS_SHADOW0_ADDR(hwts_base, 0));
	npu_drv_warn("HW_HS_SHADOW0 = 0x%llx\n", value);
	value = readl((const volatile void *)(uintptr_t)SOC_NPU_HWTS_HW_HS_SHADOW1_ADDR(hwts_base, 0));
	npu_drv_warn("HW_HS_SHADOW1 = 0x%llx\n", value);
	value = readl((const volatile void *)(uintptr_t)SOC_NPU_HWTS_HW_HS_SHADOW2_ADDR(hwts_base, 0));
	npu_drv_warn("HW_HS_SHADOW2 = 0x%llx\n", value);
	value = readl((const volatile void *)(uintptr_t)SOC_NPU_HWTS_HW_HS_DFX0_ADDR(hwts_base, 0));
	npu_drv_warn("HW_HS_DFX0 = 0x%llx\n", value);
	value = readl((const volatile void *)(uintptr_t)SOC_NPU_HWTS_HW_HS_DFX1_ADDR(hwts_base, 0));
	npu_drv_warn("HW_HS_DFX1 = 0x%llx\n", value);

	return 0;
}

