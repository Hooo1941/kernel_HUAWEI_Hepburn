/*
 * npu_pm_non_top_specify.c
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

#include "npu_pm_non_top_specify.h"

#include <linux/io.h>
#include <linux/random.h>
#include <platform_include/basicplatform/linux/rdr_pub.h>

#include "npu_platform_register.h"
#include "npu_platform.h"
#include "npu_atf_subsys.h"
#include "npu_pm_internal.h"
#include "npu_log.h"
#include "bbox/npu_dfx_black_box.h"

#ifdef CONFIG_NPU_SDMA
static void npu_plat_enable_sdma_channel(u64 sdma_ch_base_addr)
{
	SOC_NPU_sysDMA_SQ_SID_UNION sdma_sq_sid_union = {0};
	SOC_NPU_sysDMA_CQ_SID_UNION sdma_cq_sid_union = {0};
	SOC_NPU_sysDMA_CH_CTRL_UNION sdma_ch_ctrl_union = {0};

	/* config sq/cq sid to bypass smmu */
	sdma_sq_sid_union.reg.sq_sid = SMMU_BYPASS_SID;
	sdma_cq_sid_union.reg.cq_sid = SMMU_BYPASS_SID;
	sdma_sq_sid_union.reg.sq_sub_sid = 0;
	sdma_cq_sid_union.reg.cq_sub_sid = 0;
	writel(sdma_sq_sid_union.value,
		(volatile void *)(uintptr_t)SOC_NPU_sysDMA_SQ_SID_ADDR(sdma_ch_base_addr, 0));
	writel(sdma_cq_sid_union.value,
		(volatile void *)(uintptr_t)SOC_NPU_sysDMA_CQ_SID_ADDR(sdma_ch_base_addr, 0));

	/* enable sdma channel */
	sdma_ch_ctrl_union.reg.cqe_fmt_sel = 0; /* hwts just adapt cqe 16B mode */
	sdma_ch_ctrl_union.reg.cq_size = SDMA_CQ_SIZE;
	sdma_ch_ctrl_union.reg.sq_size = SDMA_SQ_SIZE;
	sdma_ch_ctrl_union.reg.qos = SDMA_CHN_QOS;
	sdma_ch_ctrl_union.reg.ch_en = 1; /* enable sdma channel */
	writel(sdma_ch_ctrl_union.value,
		(volatile void *)(uintptr_t)SOC_NPU_sysDMA_CH_CTRL_ADDR(sdma_ch_base_addr, 0));
}

static int npu_plat_init_hwts_sdma_channel(void)
{
	struct npu_mem_desc *sdma_sq_desc = NULL;
	struct npu_platform_info *plat_info = NULL;
	u64 hwts_cfg_base = 0;
	u64 sdma_cfg_base = 0;
	u64 sdma_ch_base_addr = 0;
	u32 sq_base_addr_low = 0;
	u32 cq_base_addr_low = 0;
	u8 channel_id = 0;
	SOC_NPU_HWTS_HWTS_SDMA_NS_SQ_BASE_ADDR_CFG0_UNION
		sdma_ns_sq_base_addr_cfg0 = {0};
	SOC_NPU_HWTS_HWTS_SDMA_NS_SQ_BASE_ADDR_CFG1_UNION
		sdma_ns_sq_base_addr_cfg1 = {0};

	npu_drv_warn("enter init hwts sdma channel");

	plat_info = npu_plat_get_info();
	cond_return_error(plat_info == NULL, -1, "get platform info failed");

	/* config hwts/sdma cfg base address */
	hwts_cfg_base = (u64) (uintptr_t) plat_info->dts_info.reg_vaddr[NPU_REG_HWTS_BASE];
	sdma_cfg_base = (u64) (uintptr_t) plat_info->dts_info.reg_vaddr[NPU_REG_SDMA_BASE];
	sdma_sq_desc = plat_info->resmem_info.sdma_sq_buf; /* reserved sdma sq memory */
	cond_return_error(sdma_sq_desc == NULL, -1, "ptr sdma sq desc is null");

	npu_drv_debug("hwts_cfg_base 0x%pK, sdma_cfg_base 0x%pK, sdma_sq_base 0x%pK",
		hwts_cfg_base, sdma_cfg_base, sdma_sq_desc->base);

	/* config hwts for sdma channel base address */
	sdma_ns_sq_base_addr_cfg0.reg.sdma_ns_sq_base_addr_l =
		(sdma_sq_desc->base) & UINT32_MAX; // sdma channel0 base addr
	writel(sdma_ns_sq_base_addr_cfg0.value,
		(volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_SDMA_NS_SQ_BASE_ADDR_CFG0_ADDR(hwts_cfg_base));
	sdma_ns_sq_base_addr_cfg1.reg.sdma_ns_sq_base_addr_h =
		((u64)sdma_sq_desc->base) >> 32;
	sdma_ns_sq_base_addr_cfg1.reg.sdma_ns_sq_shift = 0xF; /* 1<<sdma_ns_sq_shift: SDMA_SQ_ENTRIES * SDMA_SQE_SIZE */
	sdma_ns_sq_base_addr_cfg1.reg.sdma_ns_sq_base_is_virtual = 0; /* 0: physic	1:virtual */
	writel(sdma_ns_sq_base_addr_cfg1.value,
		(volatile void *)(uintptr_t)SOC_NPU_HWTS_HWTS_SDMA_NS_SQ_BASE_ADDR_CFG1_ADDR(hwts_cfg_base));

	for (channel_id = 0; channel_id < NPU_HWTS_SDMA_CHANNEL_NUM; channel_id++) {
		sdma_ch_base_addr = sdma_cfg_base +
			(channel_id * (0x1 << NPU_SDMA_CHANNEL_SHIFT));

		/* configure sdma sq base addr_reg */
		sq_base_addr_low = sdma_sq_desc->base +
			(channel_id * SDMA_SQ_ENTRIES * SDMA_SQE_SIZE);
		writel(sq_base_addr_low,
			(volatile void *)(uintptr_t)SOC_NPU_sysDMA_SQ_BASE_L_ADDR(sdma_ch_base_addr, 0));
		/* using 32 bit physic addr, p = 1 ssidv = 0 */
		writel((u32)(1 << 31),
			(volatile void *)(uintptr_t)SOC_NPU_sysDMA_SQ_BASE_H_ADDR(sdma_ch_base_addr, 0));

		/* configure sdma cq base addr_reg */
		cq_base_addr_low = NPU_HWTS_CFG_BASE + NPU_HWTS_SDMA_CQ_CFG_OFFSET +
			(channel_id * NPU_HWTS_SDMA_CQ_OFFSET);
		writel(cq_base_addr_low,
			(volatile void *)(uintptr_t)SOC_NPU_sysDMA_CQ_BASE_L_ADDR(sdma_ch_base_addr, 0));
		/* using 32 bit physic addr, p = 1 ssidv = 0 */
		writel((u32)(1 << 31),
			(volatile void *)(uintptr_t)SOC_NPU_sysDMA_CQ_BASE_H_ADDR(sdma_ch_base_addr, 0));

		npu_plat_enable_sdma_channel(sdma_ch_base_addr);
	}

	asm volatile("dsb st" : : : "memory");
	return 0;
}

static int npu_plat_init_sdma(u64 work_mode)
{
	int ret;
	unused(work_mode);

	ret = npu_plat_init_hwts_sdma_channel();
	cond_return_error(ret != 0, ret, "init sdma channel failed, ret 0x%x\n",
		ret);

	return 0;
}
#endif /* CONFIG_NPU_SDMA */

int npu_non_top_specify_powerup(u32 work_mode, u32 subip_set)
{
	int ret;
	unused(subip_set);

	npu_drv_warn("non top spec power up start\n");

	ret = atfd_service_npu_smc(NPU_POWER_UP_NON_TOP_SPECIFY, 0, work_mode, 0);
	if (ret != 0)
		npu_drv_err("npu non top spec power up  failed, ret:%d\n", ret);
	else
		npu_drv_warn("non top spec power up succ\n");

	return ret;
}

int npu_non_top_specify_powerdown(u32 work_mode, u32 subip_set)
{
	int ret;
	unused(subip_set);

	npu_drv_warn("non top spec power down start\n");

	ret = atfd_service_npu_smc(NPU_POWER_DOWN_NON_TOP_SPECIFY, 0, work_mode, 0);
	if (ret != 0)
		npu_drv_err("npu non top spec power down failed, ret:%d\n", ret);
	else
		npu_drv_warn("non top spec power down succ\n");

	return ret;
}

int npu_non_top_specify_nosec_powerup(u32 work_mode, u32 subip_set)
{
	int ret;
	u32 canary = 0;
	unused(subip_set);

	npu_drv_warn("non top spec nosec power up start\n");

	get_random_bytes(&canary, sizeof(canary));
	if (canary == 0)
		get_random_bytes(&canary, sizeof(canary));

	ret = atfd_service_npu_smc(NPU_POWER_UP_NON_TOP_SPECIFY_NOSEC, 0, work_mode, (u64)canary);
	if (ret != 0) {
		npu_drv_err("npu non top spec nosec power up  failed, ret:%d\n", ret);
		npu_rdr_exception_report(RDR_EXC_TYPE_TS_INIT_EXCEPTION);
	} else {
		npu_drv_debug("non top spec nosec power up succ\n");
	}

#ifdef CONFIG_NPU_SDMA
	/* sdma init */
	ret = npu_plat_init_sdma(work_mode);
#endif

	if (npu_sync_ts_time() != 0)
		npu_drv_warn("npu_sync_ts_time fail.\n");

	npu_drv_warn("non top spec nosec power up succ\n");
	return ret;
}

int npu_non_top_specify_nosec_powerdown(u32 work_mode, u32 subip_set)
{
	int ts_ret;
	int ret;
	unused(subip_set);

	npu_drv_warn("non top spec nosec power down start\n");

	ts_ret = npu_plat_powerdown_ts(0, work_mode);
	if (ts_ret != 0)
		npu_drv_err("powerdown ts failed, ts_ret:%d\n", ts_ret);

	ret = atfd_service_npu_smc(NPU_POWER_DOWN_NON_TOP_SPECIFY_NOSEC, 0, work_mode, 0);
	if (ret != 0)
		npu_drv_err("non top spec nosec power down failed, ret:%d\n", ret);
	else
		npu_drv_warn("non top spec nosec power down succ\n");

	ret += ts_ret;
	return ret;
}
