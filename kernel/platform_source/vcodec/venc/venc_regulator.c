/*
 * venc_regulator.h
 *
 * This is venc drv.
 *
 * Copyright (c) 2009-2020 Huawei Technologies CO., Ltd.
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

#include "venc_regulator.h"
#include <linux/clk.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/regulator/consumer.h>
#include <linux/iommu/mm_iommu.h>
#include "drv_venc_dpm.h"
#include "hal_venc.h"
#ifdef VENC_MCORE_ENABLE
#include "drv_venc_mcore.h"
#include "drv_venc_ipc.h"
#endif
#ifdef VENC_AXI_CLK_ENABLE
#include "venc_aclk.h"
#endif

uint64_t g_smmu_page_base_addr;
struct venc_config g_venc_config;
static struct venc_regulator g_venc_regulator;
static struct venc_clock g_venc_clock_manager[MAX_SUPPORT_CORE_NUM];


static bool g_need_transient_freq_rate;
static bool g_support_power_control_per_frame = false;

struct venc_config *get_venc_config(void)
{
	return &g_venc_config;
}

struct venc_clock *get_venc_clock_manager(uint32_t core_id)
{
	if (core_id < MAX_SUPPORT_CORE_NUM)
		return &g_venc_clock_manager[core_id];
	else
		return NULL;
}

static void get_transient_freq_cfg(struct device_node *np)
{
	int32_t ret = of_property_read_u32(np, "transit_clk_rate", &g_venc_config.venc_conf_com.transit_clk_rate);
	if (ret) {
		VCODEC_INFO_VENC("read property of transit clk fail set default");
		g_venc_config.venc_conf_com.transit_clk_rate = g_venc_config.venc_conf_com.clk_rate[0];
		g_need_transient_freq_rate = false;
	} else {
		g_need_transient_freq_rate = true;
	}
}

static int32_t get_dpm_reg_address_range_cfg(struct device_node *np)
{

	return 0;
}

static int32_t get_core_cfg(struct device_node *np)
{
	int32_t ret;
	int32_t single_core_mode_value;

	ret = of_property_read_u32(np, "single_core_mode", &single_core_mode_value);
	if (ret) {
		VCODEC_INFO_VENC("can not get singel core mode value, default use false");
		g_venc_config.venc_conf_com.is_single_core = false;
	} else {
		g_venc_config.venc_conf_com.is_single_core = (bool)single_core_mode_value;
	}

	ret = of_property_read_u32(np, "valid_core_id", &g_venc_config.venc_conf_com.valid_core_id);
	if (ret) {
		VCODEC_INFO_VENC("can not get valid core id, default use core 0");
		g_venc_config.venc_conf_com.valid_core_id = VENC_CORE_0;
	} else if (g_venc_config.venc_conf_com.valid_core_id >= MAX_SUPPORT_CORE_NUM ||
		g_venc_config.venc_conf_com.valid_core_id < VENC_CORE_0) {
		VCODEC_FATAL_VENC("get valid core id [%d] is invalid", g_venc_config.venc_conf_com.valid_core_id);
		return VCODEC_FAILURE;
	}

	ret = of_property_read_u32(np, "core_num", &g_venc_config.venc_conf_com.core_num);
	if (ret) {
		VCODEC_INFO_VENC("read property of core num fail set default");
		g_venc_config.venc_conf_com.core_num = 1;
	} else if ((g_venc_config.venc_conf_com.core_num > MAX_SUPPORT_CORE_NUM) ||
		(g_venc_config.venc_conf_com.core_num <= 0)) {
		VCODEC_FATAL_VENC("read property of core num[%d] invalid", g_venc_config.venc_conf_com.core_num);
		return VCODEC_FAILURE;
	}

	return 0;
}

static int32_t get_irq_info(struct device_node *np)
{
	uint32_t i;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());
	uint32_t number;

	int32_t ret = of_property_read_u32(np, "interrupts_per_core", &number);
	if (ret) {
		VCODEC_INFO_VENC("can not get interrupt numbers, use default value");
		number = 3; // From v700, we have more irqs, old version only have 3 irqs.
	}

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		/* read IRQ num from dts */
		venc->ctx[i].irq_num.normal = irq_of_parse_and_map(np, (int32_t)(number * i));
		if (venc->ctx[i].irq_num.normal == 0) {
			VCODEC_FATAL_VENC("parse and map irq normal failed");
			return VCODEC_FAILURE;
		}

		venc->ctx[i].irq_num.protect = irq_of_parse_and_map(np, (int32_t)(1 + number * i));
		if (venc->ctx[i].irq_num.protect == 0) {
			VCODEC_FATAL_VENC("parse and map irq protect failed");
			return VCODEC_FAILURE;
		}

		venc->ctx[i].irq_num.safe = irq_of_parse_and_map(np, (int32_t)(2 + number * i));
		if (venc->ctx[i].irq_num.safe == 0) {
			VCODEC_FATAL_VENC("parse and map irq safe failed");
			return VCODEC_FAILURE;
		}
#ifdef VENC_MCORE_ENABLE
		venc->ctx[i].irq_num.mcore_normal = irq_of_parse_and_map(np, (int32_t)(3 + number * i));
		if (venc->ctx[i].irq_num.mcore_normal == 0) {
			VCODEC_FATAL_VENC("parse and map irq mcore normal failed");
			return VCODEC_FAILURE;
		}
#endif
	}
	return 0;
}

static int32_t get_core_clk_info(struct platform_device *pdev)
{
	uint32_t i;
	int32_t ret;
	char tmp_name[MAX_NAME_LEN] = {0};
	struct resource *res = NULL;
	struct device_node *np = NULL;
	struct device *dev = &pdev->dev;

	np = dev->of_node;
	if (!np) {
		VCODEC_FATAL_VENC("invalid argument np is NULL");
		return VCODEC_FAILURE;
	}

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		/* read venc register start address, range */
		res = platform_get_resource(pdev, IORESOURCE_MEM, i);
		if (IS_ERR_OR_NULL(res)) {
			VCODEC_FATAL_VENC("failed to get instruction resource!");
			return VCODEC_FAILURE;
		}

		g_venc_config.venc_conf_priv[i].reg_base_addr = (uint32_t)res->start; /*lint !e712*/
		g_venc_config.venc_conf_priv[i].reg_range    = (uint32_t)resource_size(res); /*lint !e712*/

		/* read venc clk rate [low, high], venc clock */
		if (i == 0) {
			ret = strcpy_s(tmp_name, MAX_NAME_LEN, VENC_CLOCK_NAME);
			if (ret) {
				VCODEC_FATAL_VENC("call strcpy_s failed");
				return VCODEC_FAILURE;
			}
		} else {
			ret = sprintf_s(tmp_name, sizeof(tmp_name), "%s%u", VENC_CLOCK_NAME, i);
			if (ret < 0) {
				VCODEC_FATAL_VENC("call sprintf_s failed");
				return VCODEC_FAILURE;
			}
		}

		g_venc_clock_manager[i].venc_clk  = devm_clk_get(dev, tmp_name);

		if (IS_ERR_OR_NULL(g_venc_clock_manager[i].venc_clk)) {
			VCODEC_FATAL_VENC("can not get core_id %d clock", i);
			return VCODEC_FAILURE;
		}
	}

	for (i = 0; i < VENC_CLK_BUTT; i++) {
		/* the clk rate is from high to low in dts */
		ret = of_property_read_u32_index(np, VENC_CLK_RATE, VENC_CLK_BUTT - i - 1,
				&g_venc_config.venc_conf_com.clk_rate[i]);
		if (ret) {
			VCODEC_FATAL_VENC("get venc rate type %d failed, ret is %d", i, ret);
			return VCODEC_FAILURE;
		}
#ifdef VCODECV700
		ret = of_property_read_u32_index(np, VENC_POWER_MODE_PROFILE, i,
				&g_venc_config.venc_conf_com.power_mode[i].loads);
		if (ret) {
			VCODEC_FATAL_VENC("get venc power profile %d failed, ret is %d", i, ret);
			return VCODEC_FAILURE;
		}
		g_venc_config.venc_conf_com.power_mode[i].power_level = i;
		g_venc_config.venc_conf_com.power_mode[i].core_num    = g_venc_config.venc_conf_com.core_num;
		VCODEC_INFO_VENC("venc clock type %d: clock rate is %d, loads is %d",
				i, g_venc_config.venc_conf_com.clk_rate[i], g_venc_config.venc_conf_com.power_mode[i].loads);
#endif
	}

#ifdef VCODECV700
	ret = of_property_read_u32(np, VENC_LOW_POWER_MODE_PROFILE_LEN,
				&g_venc_config.venc_conf_com.low_power_mode_len);
	if (ret) {
		VCODEC_WARN_VENC("get venc low power profile len failed, ret is %d", ret);
		return VCODEC_FAILURE;
	}

	for (i = 0; i < g_venc_config.venc_conf_com.low_power_mode_len; i++) {
		ret = of_property_read_u32_index(np, VENC_LOW_POWER_MODE_LOADS, i,
				&g_venc_config.venc_conf_com.low_power_mode[i].loads);
		if (ret) {
			VCODEC_WARN_VENC("get venc low power loads %d failed, ret is %d", i, ret);
			return VCODEC_FAILURE;
		}

		ret = of_property_read_u32_index(np, VENC_LOW_POWER_MODE_LEVEL, i,
				&g_venc_config.venc_conf_com.low_power_mode[i].power_level);
		if (ret) {
			VCODEC_WARN_VENC("get venc low power level %d failed, ret is %d", i, ret);
			return VCODEC_FAILURE;
		}

		g_venc_config.venc_conf_com.low_power_mode[i].core_num   = g_venc_config.venc_conf_com.core_num;
	}

#endif

	return 0;
}

static int32_t get_clk_info(struct platform_device *pdev)
{
	int ret;
	ret = get_core_clk_info(pdev);
	if (ret)
		return VCODEC_FAILURE;
#ifdef VENC_AXI_CLK_ENABLE
	ret = get_axi_clk_info(pdev);
	if (ret)
		return VCODEC_FAILURE;
#endif
	return 0;
}

static int32_t get_mcore_cfg(struct device_node *np)
{
#ifdef VENC_MCORE_ENABLE
	int32_t ret = of_property_read_u32(np, "mcore_code_base", &g_venc_config.venc_conf_com.mcore_code_base);
	if (ret) {
		VCODEC_INFO_VENC("can not get mcore code base ");
		return VCODEC_FAILURE;
	}
	ret = of_property_read_u32(np, "intr_hub_reg_base", &g_venc_config.venc_conf_com.intr_hub_reg_base);
	if (ret) {
		VCODEC_INFO_VENC("can not get intr hub base ");
		return VCODEC_FAILURE;
	}
#endif
	return 0;
}

int32_t VCODEC_ATTR_WEEK venc_get_dts_config_info(struct platform_device *pdev)
{
	int32_t ret;
	uint32_t power_control_flag = 0;
	struct device_node *np = NULL;
	struct device *dev = &pdev->dev;

	if (!dev) {
		VCODEC_FATAL_VENC("invalid argument, dev is NULL");
		return VCODEC_FAILURE;
	}

	np = dev->of_node;
	if (!np) {
		VCODEC_FATAL_VENC("invalid argument np is NULL");
		return VCODEC_FAILURE;
	}

	(void)memset_s(&g_venc_config, sizeof(g_venc_config), 0, sizeof(g_venc_config));
	(void)memset_s(g_venc_clock_manager, sizeof(g_venc_clock_manager), 0, sizeof(g_venc_clock_manager));

	ret = get_core_cfg(np);
	if (ret)
		return VCODEC_FAILURE;

	ret = get_irq_info(np);
	if (ret)
		return VCODEC_FAILURE;

	ret = get_clk_info(pdev);
	if (ret)
		return VCODEC_FAILURE;

	ret = get_dpm_reg_address_range_cfg(np);
	if (ret)
		return VCODEC_FAILURE;

	ret = of_property_read_u32(np, "power_off_clk_rate", &g_venc_config.venc_conf_com.power_off_clk_rate);
	if (ret) {
		VCODEC_INFO_VENC("read property of power off clk fail set default");
		g_venc_config.venc_conf_com.power_off_clk_rate = g_venc_config.venc_conf_com.clk_rate[0];
	}

	get_transient_freq_cfg(np);


	/* fpga platform */
	ret = of_property_read_u32(np, "venc_fpga", &g_venc_config.venc_conf_com.fpga_flag);
	if (ret)
		VCODEC_INFO_VENC("can not get the venc fpga flag, maybe not fpga");

	/* get venc qos mode */
	ret = of_property_read_u32(np, "venc_qos_mode", &g_venc_config.venc_conf_com.qos_mode);
	if (ret)
		VCODEC_ERR_VENC("can not get venc qos mode, use default value %d",
				g_venc_config.venc_conf_com.qos_mode);

	ret = of_property_read_u32(np, "support_power_control_per_frame", (uint32_t *)&power_control_flag);
	if (ret) {
		power_control_flag = 0;
		VCODEC_INFO_VENC("can not get venc power control flag, use default value 0");
	}
	g_support_power_control_per_frame = (bool)(power_control_flag);
	VCODEC_INFO_VENC("venc power control flag %d", g_support_power_control_per_frame);

	g_smmu_page_base_addr = (uint64_t)(kernel_domain_get_ttbr(&pdev->dev));
#ifndef SMMU_V3
	if (g_smmu_page_base_addr == 0) {
		VCODEC_ERR_VENC("get mmu addr failed");
		return VCODEC_FAILURE;
	}
#endif

	ret = get_mcore_cfg(np);
	if (ret)
		return VCODEC_FAILURE;

	ret = of_property_read_u32(np, "disable_cfg_tbu_max_tok_trans",
		(uint32_t *)&g_venc_config.venc_conf_com.disable_cfg_tbu_max_tok_trans);
	if (ret)
		VCODEC_INFO_VENC("can not get venc disable_cfg_tbu_max_tok_trans, use default value 0");

	ret = of_property_read_u32(np, "smmu_establish_link_robust",
		(uint32_t *)&g_venc_config.venc_conf_com.smmu_establish_link_robust);
	if (ret)
		VCODEC_INFO_VENC("can not get venc smmu_establish_link_robust, use default value 0");

	return 0;
}

int32_t venc_get_regulator_info(struct platform_device *pdev)
{
	uint32_t i;
	int32_t ret;
	char tmp_name[MAX_NAME_LEN] = {0};

	(void)memset_s(&g_venc_regulator, sizeof(g_venc_regulator), 0, sizeof(g_venc_regulator));

	g_venc_regulator.media_regulator = devm_regulator_get(&pdev->dev, MEDIA_REGULATOR_NAME);

	if (IS_ERR_OR_NULL(g_venc_regulator.media_regulator)) {
		VCODEC_FATAL_VENC("get media regulator failed, error no is %ld", PTR_ERR(g_venc_regulator.media_regulator));
		g_venc_regulator.media_regulator = VCODEC_NULL;
		return VCODEC_FAILURE;
	}

#ifdef SMMU_V3
	if (venc_smmu_get_tcu_regulator_info() != SMMU_OK)
		return VCODEC_FAILURE;
#endif

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		if (i == 0) {
			ret = strcpy_s(tmp_name, MAX_NAME_LEN, VENC_REGULATOR_NAME);
			if (ret) {
				VCODEC_FATAL_VENC("call strcpy_s failed!");
				return VCODEC_FAILURE;
			}
		} else {
			ret = sprintf_s(tmp_name, sizeof(tmp_name), "%s%u", VENC_REGULATOR_NAME, i);
			if (ret < 0) {
				VCODEC_FATAL_VENC("call sprintf_s failed! ");
				return VCODEC_FAILURE;
			}
		}

		g_venc_regulator.venc_regulator[i] = devm_regulator_get(&pdev->dev, tmp_name);

		if (IS_ERR_OR_NULL(g_venc_regulator.venc_regulator[i])) {
			VCODEC_FATAL_VENC("get regulator failed, error no is %ld", PTR_ERR(g_venc_regulator.venc_regulator[i]));
			g_venc_regulator.venc_regulator[i] = VCODEC_NULL;
			return VCODEC_FAILURE;
		}
	}

	return 0;
}


static bool is_hardware_busy(void)
{
	uint32_t i;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		if (venc->ctx[i].status == VENC_BUSY)
			return true;
	}

	return false;
}

static int32_t wait_core_idle(uint32_t core_id)
{
	int32_t ret;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	/*lint -e578 -e666*/
	ret = OSAL_WAIT_EVENT_TIMEOUT(&venc->event,
		venc->ctx[core_id].status != VENC_BUSY,
		WAIT_CORE_IDLE_TIMEOUT_MS);
	/*lint +e578 +e666*/
	if (ret != 0) {
		VCODEC_ERR_VENC("wait core idle timeout");
		return VCODEC_FAILURE;
	}

	return 0;
}

static int32_t check_param_valid(int32_t core_id)
{
	int32_t ret;

	ret = venc_check_coreid(core_id);
	if (ret != 0) {
		VCODEC_ERR_VENC("CORE_ERROR:invalid core ID is %d", core_id);
		return VCODEC_FAILURE;
	}

	if (IS_ERR_OR_NULL(g_venc_clock_manager[core_id].venc_clk) ||
		IS_ERR_OR_NULL(g_venc_regulator.venc_regulator[core_id]) ||
		IS_ERR_OR_NULL(g_venc_regulator.media_regulator)) {
		VCODEC_FATAL_VENC("core %d: regulator param error", core_id);
		return VCODEC_FAILURE;
	}
#ifdef VENC_AXI_CLK_ENABLE
	if (IS_ERR_OR_NULL(g_venc_clock_manager[core_id].venc_aclk)) {
		VCODEC_FATAL_VENC("core %d: regulator param error", core_id);
		return VCODEC_FAILURE;
	}
#endif

	return 0;
}

static void venc_init_encode_timer(struct venc_context *ctx, venc_entry_t *venc, int32_t core_id)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	ctx->core_id = core_id;
	osal_init_timer(&ctx->timer, venc->ops.encode_timeout);
#else
	osal_init_timer(&ctx->timer, venc->ops.encode_timeout, (unsigned long)core_id);//lint !e571
#endif
}

static void set_trans_clk_rate(int32_t core_id, uint32_t current_clk)
{
	int32_t ret;
	if (!g_need_transient_freq_rate)
		return;

	if (current_clk == g_venc_config.venc_conf_com.clk_rate[VENC_CLK_RATE_NORMAL] ||
		current_clk == g_venc_config.venc_conf_com.clk_rate[VENC_CLK_RATE_HIGH]) {
		ret = clk_set_rate(g_venc_clock_manager[core_id].venc_clk,
			g_venc_config.venc_conf_com.transit_clk_rate);
		if (ret)
			VCODEC_WARN_VENC("core_id is %d, failed set transit clk,fail code is %d", core_id, ret);
	}
}

static void power_off_disable_core_clk(int32_t core_id)
{
	int32_t ret;
	uint32_t low_rate;
	uint32_t current_clk;

	current_clk = (uint32_t)clk_get_rate(g_venc_clock_manager[core_id].venc_clk);
	set_trans_clk_rate(core_id, current_clk);

	/* we need set lowest clk rate before power off */
#if ((defined VCODECV600) || (defined(VCODECV700)) || (defined VCODECV520) || (defined VENC_SET_POWER_OFF_CLK))
	low_rate = g_venc_config.venc_conf_com.power_off_clk_rate;
	g_venc_clock_manager[core_id].curr_clk_type = VENC_CLK_RATE_LOWER;
#elif defined VCODECV500
	low_rate = g_venc_config.venc_conf_com.clk_rate[VENC_CLK_RATE_LOWER];
	g_venc_clock_manager[core_id].curr_clk_type = VENC_CLK_RATE_LOWER;
#else
	low_rate = g_venc_config.venc_conf_com.clk_rate[VENC_CLK_RATE_LOW];
	g_venc_clock_manager[core_id].curr_clk_type = VENC_CLK_RATE_LOW;
#endif

	ret = clk_set_rate(g_venc_clock_manager[core_id].venc_clk, low_rate);
	if (ret != 0)
		VCODEC_ERR_VENC("core_id is %d, set lowest clk rate failed before power off", core_id);

	clk_disable_unprepare(g_venc_clock_manager[core_id].venc_clk);
}

static void power_off_disable_clk(int32_t core_id)
{
	power_off_disable_core_clk(core_id);
#ifdef VENC_AXI_CLK_ENABLE
	power_off_disable_axi_clk(core_id);
#endif
}

static int32_t power_on_enable_core_clk(int32_t core_id)
{
	uint32_t low_rate;
	if (clk_prepare_enable(g_venc_clock_manager[core_id].venc_clk) != 0) {
		VCODEC_FATAL_VENC("core %d, prepare clk enable failed", core_id);
		return VCODEC_FAILURE;
	}

	/* we need set lowest clk rate before power on */
#ifdef VCODECV500
	low_rate = g_venc_config.venc_conf_com.clk_rate[VENC_CLK_RATE_LOWER];
	g_venc_clock_manager[core_id].curr_clk_type = VENC_CLK_RATE_LOWER;
#else
	low_rate = g_venc_config.venc_conf_com.clk_rate[VENC_CLK_RATE_LOW];
	g_venc_clock_manager[core_id].curr_clk_type = VENC_CLK_RATE_LOW;
#endif

	if (clk_set_rate(g_venc_clock_manager[core_id].venc_clk, low_rate) != 0) {
		VCODEC_FATAL_VENC("core %d, set clk low rate failed", core_id);
		goto on_error_prepare_clk;
	}
	return 0;

on_error_prepare_clk:
	clk_disable_unprepare(g_venc_clock_manager[core_id].venc_clk);
	return VCODEC_FAILURE;
}

static int32_t power_on_enable_clk(int32_t core_id)
{
	if (power_on_enable_core_clk(core_id) != 0)
		return VCODEC_FAILURE;
#ifdef VENC_AXI_CLK_ENABLE
	if (power_on_enable_axi_clk(core_id) != 0) {
		power_off_disable_core_clk(core_id);
		return VCODEC_FAILURE;
	}
#endif
	return 0;
}

static int32_t power_on_single_core(int32_t core_id)
{
	struct venc_context *ctx = NULL;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	if (check_param_valid(core_id) != 0)
		return VCODEC_FAILURE;

	ctx = &venc->ctx[core_id];
	if (ctx->status != VENC_POWER_OFF)
		return 0;

	if (regulator_enable(g_venc_regulator.media_regulator) != 0) {
		VCODEC_FATAL_VENC("core %d, enable media regulator failed", core_id);
		return VCODEC_FAILURE;
	}

	if (power_on_enable_clk(core_id) != 0)
		goto on_error_regulator;

	if (regulator_enable(g_venc_regulator.venc_regulator[core_id]) != 0) {
		VCODEC_FATAL_VENC("core %d, enable regulator failed", core_id);
		goto on_error_prepare_clk;
	}

	ctx->reg_base = (uint32_t *)vcodec_mmap(g_venc_config.venc_conf_priv[core_id].reg_base_addr,
					g_venc_config.venc_conf_priv[core_id].reg_range);
	if (!ctx->reg_base) {
		VCODEC_ERR_VENC("core %d, ioremap failed", core_id);
		goto on_error_set_base_addr;
	}

#if (!defined(VCODECV500) && !defined(VCODECV320))
	venc_hal_set_smmu_addr((S_HEVC_AVC_REGS_TYPE *)(ctx->reg_base));
#endif

	venc_hal_disable_all_int((S_HEVC_AVC_REGS_TYPE *)(ctx->reg_base));
	venc_hal_clr_all_int((S_HEVC_AVC_REGS_TYPE *)(ctx->reg_base));

#ifdef IRQ_EN
	if (venc_drv_osal_irq_init(ctx->irq_num.normal, venc_drv_encode_done) == VCODEC_FAILURE) {
		VCODEC_ERR_VENC("core_id is %d, venc_drv_osal_irq_init failed", core_id);
		goto on_error_set_irq;
	}
#endif
	venc_init_encode_timer(ctx, venc, core_id);

#ifdef SMMU_V3
	if (venc_smmu_tbu_init(core_id) != SMMU_OK)
		goto on_error_smmu_tbu;
#endif

#ifdef VENC_MCORE_ENABLE
	if (venc_mcore_open() != 0) {
		VCODEC_FATAL_VENC("open mcore failed %d");
		goto on_err_mcore_open;
	}
#endif


	ctx->status = VENC_IDLE;
	ctx->first_cfg_flag = true;
	pm_hardware_power_on_enter(&ctx->pm);

	VCODEC_DBG_VENC("core_id is %d, power on++", core_id);
	return 0;

#ifdef VENC_MCORE_ENABLE
on_err_mcore_open:
#ifdef SMMU_V3
	venc_smmu_tbu_deinit();
#endif
#endif
#ifdef SMMU_V3
on_error_smmu_tbu:
#endif
#ifdef IRQ_EN
	venc_drv_osal_irq_free(ctx->irq_num.normal);
#endif

on_error_set_irq:
	vcodec_munmap(ctx->reg_base);
on_error_set_base_addr:
	if (regulator_disable(g_venc_regulator.venc_regulator[core_id]))
		VCODEC_ERR_VENC("core_id is %d, disable media regulator failed", core_id);
on_error_prepare_clk:
	power_off_disable_clk(core_id);
on_error_regulator:
	if (regulator_disable(g_venc_regulator.media_regulator))
		VCODEC_ERR_VENC("core_id is %d, disable media regulator failed", core_id);

	return VCODEC_FAILURE;
}

static int32_t power_off_single_core(int32_t core_id)
{
	int32_t ret;
	struct venc_context *ctx = NULL;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	if (check_param_valid(core_id) != 0)
		return VCODEC_FAILURE;

	ctx = &venc->ctx[core_id];

	if (ctx->status == VENC_POWER_OFF)
		return 0;

	if (ctx->status == VENC_BUSY || ctx->status == VENC_TIME_OUT) {
		VCODEC_WARN_VENC("The current power-off core %d status is %d", core_id, ctx->status);
#ifdef VCODECV500
		vedu_hal_request_bus_idle(ctx->reg_base);
#endif
	}

#ifdef VENC_MCORE_ENABLE
	venc_mcore_close();
#endif

#ifdef SMMU_V3
	venc_smmu_tbu_deinit();
#endif
	venc_hal_disable_all_int((S_HEVC_AVC_REGS_TYPE *)(ctx->reg_base));
	venc_hal_clr_all_int((S_HEVC_AVC_REGS_TYPE *)(ctx->reg_base));

	/* there is not timer running nomally */
	if (osal_del_timer(&ctx->timer, true) == 0)
		VCODEC_WARN_VENC("core %d: timer is pending, when power off", core_id);

#ifdef IRQ_EN
	venc_drv_osal_irq_free(ctx->irq_num.normal);
#endif


	vcodec_munmap(ctx->reg_base);

	ret = regulator_disable(g_venc_regulator.venc_regulator[core_id]);
	if (ret != 0)
		VCODEC_ERR_VENC("core_id is %d, disable regulator failed", core_id);

	power_off_disable_clk(core_id);

	ret = regulator_disable(g_venc_regulator.media_regulator);
	if (ret != 0)
		VCODEC_ERR_VENC("disable media regulator failed");

	ctx->status = VENC_POWER_OFF;
	ctx->first_cfg_flag = true;

	pm_hardware_power_on_exit(&ctx->pm);

	return 0;
}

static uint32_t power_on_multi_core(uint32_t powr_on_core_num)
{
	int32_t ret;
	uint32_t i;
	uint32_t count = 0;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	for (i = 0; (i < g_venc_config.venc_conf_com.core_num) &&
			(count < powr_on_core_num); i++) {
		if (g_venc_config.venc_conf_com.is_single_core
			&& g_venc_config.venc_conf_com.valid_core_id != i)
			continue;

		if (venc->ctx[i].status == VENC_POWER_OFF) {
			ret = power_on_single_core((int32_t)i);
			if (ret != 0) {
				VCODEC_WARN_VENC("power on core %d failed", i);
				continue;
			}
			count++;
		}
	}

	if (count != powr_on_core_num)
		VCODEC_WARN_VENC("power on %d cores, but we need power on %d cores",
				count, powr_on_core_num);

	return count;
}

static uint32_t power_off_multi_core(uint32_t power_off_core_num)
{
	int32_t ret;
	int32_t i;
	uint32_t count = 0;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	for (i = (int32_t)g_venc_config.venc_conf_com.core_num - 1;
			(i >= 0) && (count < power_off_core_num); i--) {
		if (venc->ctx[i].status == VENC_POWER_OFF)
			continue;

		wait_core_idle((uint32_t)i);
		ret = power_off_single_core(i);
		if (ret != 0) {
			VCODEC_WARN_VENC("power off core %d failed", i);
			continue;
		}
		count++;
	}

	if (count != power_off_core_num)
		VCODEC_WARN_VENC("power off %d cores, but we need power off %d cores",
				count, power_off_core_num);

	return count;
}

static int32_t switch_core_num(uint32_t core_num)
{
	uint32_t ret;
	uint32_t i;
	uint32_t count;
	uint32_t cur_core_num = 0;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		if (venc->ctx[i].status != VENC_POWER_OFF)
			cur_core_num++;
	}

	if (cur_core_num == core_num)
		return 0;

	VCODEC_DBG_VENC("switch the working core from %d to %d", cur_core_num, core_num);

	if (cur_core_num < core_num) {
		count = core_num - cur_core_num;
		ret = power_on_multi_core(count);
	} else {
		count = cur_core_num - core_num;
		ret = power_off_multi_core(count);
	}

	return (ret == count) ? 0 : VCODEC_FAILURE;
}

static int32_t set_clk_rate_single_core(venc_clk_t clk_type, int32_t core_id)
{
	int32_t ret;
	uint32_t need_clk;
	uint32_t current_clk;
	venc_clk_t need_clk_type;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	ret = venc_check_coreid(core_id);
	if (ret != 0) {
		VCODEC_ERR_VENC("CORE_ERROR:invalid core ID is %d", core_id);
		return VCODEC_FAILURE;
	}

	if (venc->ctx[core_id].status == VENC_POWER_OFF)
		return 0;

#ifdef SUPPORT_VENC_FREQ_CHANGE
	mutex_lock(&g_venc_freq_mutex);
	if (clk_type < g_venc_freq)
		clk_type = g_venc_freq;
	mutex_unlock(&g_venc_freq_mutex);
#endif


	if (g_venc_clock_manager[core_id].curr_clk_type == clk_type)
		return 0;

	need_clk = g_venc_config.venc_conf_com.clk_rate[clk_type];
	current_clk = (uint32_t)clk_get_rate(g_venc_clock_manager[core_id].venc_clk);
	if (need_clk == current_clk) {
		g_venc_clock_manager[core_id].curr_clk_type = clk_type;
		return 0;
	}

	VCODEC_INFO_VENC("core %d: set clk type from %d to %d, clk rate from %u to %u",
			core_id, g_venc_clock_manager[core_id].curr_clk_type, clk_type, current_clk, need_clk);

	set_trans_clk_rate(core_id, current_clk);

	need_clk_type = clk_type;
	ret = clk_set_rate(g_venc_clock_manager[core_id].venc_clk, need_clk);

	while ((ret != 0) && (need_clk_type > 0)) {
		need_clk_type--;
		if (current_clk != g_venc_config.venc_conf_com.clk_rate[need_clk_type]) {
			VCODEC_WARN_VENC("core_id is %d, failed set clk to %u Hz,fail code is %d", core_id, need_clk, ret);
			need_clk = g_venc_config.venc_conf_com.clk_rate[need_clk_type];
			ret = clk_set_rate(g_venc_clock_manager[core_id].venc_clk, need_clk);
		} else {
			ret = 0;
		}
	}

#ifdef VENC_AXI_CLK_ENABLE
	ret = set_axi_clk_rate_single_core(clk_type, core_id);
#endif
	if (ret == 0) {
		g_venc_clock_manager[core_id].curr_clk_type = need_clk_type;
	} else {
		VCODEC_WARN_VENC("core_id is %d, failed set clk to %u Hz,fail code is %d", core_id, need_clk, ret);
	}
	return ret;
}

static int32_t set_clk_rate(venc_clk_t clk_type)
{
	uint32_t i;
	int32_t ret = 0;

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		if (set_clk_rate_single_core(clk_type, (int32_t)i) != 0) {
			VCODEC_ERR_VENC("set clock rate core %d failed", i);
			ret = VCODEC_FAILURE;
		}
	}

	return ret;
}

static int32_t VCODEC_ATTR_WEEK process_encode_timeout(void)
{
	uint32_t i;
	int32_t ret = 0;
	venc_clk_t venc_curr_clk;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		if (venc->ctx[i].status == VENC_TIME_OUT) {
#ifdef VENC_MCORE_ENABLE
			venc_mcore_log_dump();
#endif
			VCODEC_WARN_VENC("core_id: %d reset", i);
			venc_curr_clk = g_venc_clock_manager[i].curr_clk_type;

			if (power_off_single_core((int32_t)i) != 0) {
				VCODEC_ERR_VENC("power off core %d failed", i);
				ret = VCODEC_FAILURE;
				continue;
			}

			if (power_on_single_core((int32_t)i) != 0) {
				VCODEC_ERR_VENC("power on core %d failed", i);
				ret = VCODEC_FAILURE;
				continue;
			}

			if (set_clk_rate_single_core(venc_curr_clk, (int32_t)i) != 0) {
				VCODEC_ERR_VENC("set clock rate core %d failed", i);
				ret = VCODEC_FAILURE;
			}
		}
	}

	return ret;
}

static int32_t get_idle_core(void)
{
	uint32_t i;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	if (process_encode_timeout() != 0)
		VCODEC_WARN_VENC("time out reset reg fail");

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		if (venc->ctx[i].status == VENC_IDLE)
			break;
	}

	if (i == g_venc_config.venc_conf_com.core_num)
		return -1;

	return (int32_t)i;
}

int32_t VCODEC_ATTR_WEEK venc_regulator_enable(void)
{
	int32_t ret;
	ret = power_on_single_core((int32_t)g_venc_config.venc_conf_com.valid_core_id);
	if (ret != 0) {
		VCODEC_INFO_VENC("core %d: enable regulator failed", g_venc_config.venc_conf_com.valid_core_id);
		return VCODEC_FAILURE;
	}

	return 0;
}

int32_t VCODEC_ATTR_WEEK venc_regulator_disable(void)
{
	int32_t ret;
	uint32_t i;

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		ret = power_off_single_core((int32_t)i);
		if (ret != 0) {
			VCODEC_ERR_VENC("core %d: disable regulator failed", i);
			return VCODEC_FAILURE;
		}
	}

	return 0;
}

int32_t venc_regulator_disable_by_low_power(void)
{
	uint32_t i;
	int32_t ret = 0;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		if (!pm_if_need_power_off(&venc->ctx[i].pm))
			continue;

		if (power_off_single_core((int32_t)i) != 0) {
			VCODEC_ERR_VENC("core %d: disable regulator failed", i);
			ret = VCODEC_FAILURE;
		}
	}

	return ret;
}

int32_t venc_regulator_select_idle_core(vedu_osal_event_t *event)
{
	int32_t ret;
	int32_t core_id = -1;

	if (event == NULL) {
		VCODEC_ERR_VENC("event input is NULL");
		return -1;
	}

	ret = OSAL_WAIT_EVENT_TIMEOUT(event, (core_id = get_idle_core()) >= 0, ENCODE_DONE_TIMEOUT_MS); /*lint !e666 !e578*/
	if (ret != 0) {
		VCODEC_ERR_VENC("wait idle core timeout");
		return -1;
	}

	return core_id;
}

int32_t venc_regulator_wait_hardware_idle(vedu_osal_event_t *event)
{
	int32_t ret;

	if (event == NULL) {
		VCODEC_ERR_VENC("event input is NULL");
		return VCODEC_FAILURE;
	}

	/*lint -e578 -e666*/
	ret = OSAL_WAIT_EVENT_TIMEOUT(event, is_hardware_busy() == false,
			WAIT_CORE_IDLE_TIMEOUT_MS);
	/*lint +e578 +e666*/
	if (ret != 0) {
		VCODEC_WARN_VENC("wait hardware idle timeout");
		return VCODEC_FAILURE;
	}

	return 0;
}

int32_t VCODEC_ATTR_WEEK venc_regulator_update(struct clock_info *clock_info)
{
	int32_t ret;

	if (clock_info == NULL) {
		VCODEC_ERR_VENC("clock info input is NULL");
		return VCODEC_FAILURE;
	}

	if (clock_info->core_num > g_venc_config.venc_conf_com.core_num) {
		VCODEC_ERR_VENC("core num %d is more than the total %d",
				clock_info->core_num, g_venc_config.venc_conf_com.core_num);
		return VCODEC_FAILURE;
	}

	if (clock_info->clock_type >= VENC_CLK_BUTT) {
		VCODEC_ERR_VENC("clock type %d invalid", clock_info->clock_type);
		return VCODEC_FAILURE;
	}

	ret = switch_core_num(clock_info->core_num);
	if (ret != 0) {
		VCODEC_WARN_VENC("switch encode core num %d failed", clock_info->core_num);
		return VCODEC_FAILURE;
	}

	ret = set_clk_rate(clock_info->clock_type);
	if (ret != 0) {
		VCODEC_WARN_VENC("set venc clkrate failed, ret: %d", ret);
		return VCODEC_FAILURE;
	}

	return 0;
}

int32_t venc_regulator_reset(void)
{
	int32_t ret;
	uint32_t i;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) { //lint !e574
		if (venc->ctx[i].status == VENC_POWER_OFF)
			continue;

		wait_core_idle(i);

		ret = power_off_single_core((int32_t)i);
		if (ret != 0) {
			VCODEC_ERR_VENC("power off core %d failed", i);
			return VCODEC_FAILURE;
		}

		ret = power_on_single_core((int32_t)i);
		if (ret != 0) {
			VCODEC_ERR_VENC("power on core %d failed", i);
			return VCODEC_FAILURE;
		}
	}

	return 0;
}

bool venc_regulator_is_fpga(void)
{
	return (bool)g_venc_config.venc_conf_com.fpga_flag;
}

uint64_t venc_get_smmu_ttb(void)
{
	return g_smmu_page_base_addr;
}

bool is_support_power_control_per_frame(void)
{
	return g_support_power_control_per_frame;
}

uint32_t venc_get_reg_base_addr(int32_t core_id)
{
	if (venc_check_coreid(core_id) != 0)
		return 0;

	return g_venc_config.venc_conf_priv[core_id].reg_base_addr;
}

uint32_t venc_get_core_num(void)
{
	return g_venc_config.venc_conf_com.core_num;
}

#ifdef VCODECV700
uint32_t venc_get_power_profiles(uint32_t flag, struct venc_performance_info **performance_info)
{
	if (flag) {
		*performance_info = g_venc_config.venc_conf_com.low_power_mode;
		return g_venc_config.venc_conf_com.low_power_mode_len;
	} else {
		*performance_info = g_venc_config.venc_conf_com.power_mode;
		return VENC_CLK_BUTT;
	}
}
#endif

void venc_set_clock_info(struct clock_info *info)
{
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	mutex_lock(&venc->backup_info.lock);
	venc->backup_info.info = *info;
	mutex_unlock(&venc->backup_info.lock);
}

void venc_get_clock_info(struct clock_info *info)
{
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	mutex_lock(&venc->backup_info.lock);
	*info = venc->backup_info.info;
	mutex_unlock(&venc->backup_info.lock);
}

uint32_t venc_get_mcore_code_base(void)
{
	return g_venc_config.venc_conf_com.mcore_code_base;
}

uint32_t venc_get_intr_hub_reg_base(void)
{
	return g_venc_config.venc_conf_com.intr_hub_reg_base;
}

bool venc_get_disable_cfg_tbu_max_tok_trans(void)
{
	return (bool)g_venc_config.venc_conf_com.disable_cfg_tbu_max_tok_trans;
}

bool venc_get_smmu_estbl_link_robust_enable(void)
{
	return (bool)g_venc_config.venc_conf_com.smmu_establish_link_robust;
}
