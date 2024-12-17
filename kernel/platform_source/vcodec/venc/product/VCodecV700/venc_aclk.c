/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: set venc aclk
 * Create: 2022-10-28
 */
#include "venc_aclk.h"
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>

static void get_transient_axi_clk_cfg(struct device_node *np)

{
	int32_t ret;
	get_venc_config();
	ret = of_property_read_u32(np, "transit_aclk_rate", &get_venc_config()->venc_conf_com.transit_aclk_rate);
	if (ret) {
		VCODEC_INFO_VENC("read property of transit aclk fail set default");
		get_venc_config()->venc_conf_com.transit_aclk_rate = get_venc_config()->venc_conf_com.aclk_rate[0];
	}
}

int32_t get_axi_clk_info(struct platform_device *pdev)
{
	uint32_t i;
	int32_t ret;
	struct device_node *np = NULL;
	struct device *dev = &pdev->dev;

	np = dev->of_node;
	if (!np) {
		VCODEC_FATAL_VENC("invalid argument np is NULL");
		return VCODEC_FAILURE;
	}
	/* read venc aclk rate [low, high], venc axi clock */
	for (i = 0; i < get_venc_config()->venc_conf_com.core_num; i++) {
		get_venc_clock_manager(i)->venc_aclk  = devm_clk_get(dev, VENC_ACLOCK_NAME);

		if (IS_ERR_OR_NULL(get_venc_clock_manager(i)->venc_aclk)) {
			VCODEC_FATAL_VENC("can not get core_id %d clock", i);
			return VCODEC_FAILURE;
		}
	}

	for (i = 0; i < VENC_CLK_BUTT; i++) {
		/* the aclk rate is from high to low in dts */
		ret = of_property_read_u32_index(np, VENC_ACLK_RATE, VENC_CLK_BUTT - i - 1,
				&get_venc_config()->venc_conf_com.aclk_rate[i]);
		if (ret) {
			VCODEC_FATAL_VENC("get venc rate type %d failed, ret is %d", i, ret);
			return VCODEC_FAILURE;
		}

		VCODEC_INFO_VENC("venc clock type %d: clock rate is %d",
				i, get_venc_config()->venc_conf_com.aclk_rate[i]);
	}

	ret = of_property_read_u32(np, "power_off_aclk_rate", &get_venc_config()->venc_conf_com.power_off_aclk_rate);
	if (ret) {
		VCODEC_INFO_VENC("read property of power off aclk fail set default");
		get_venc_config()->venc_conf_com.power_off_aclk_rate = get_venc_config()->venc_conf_com.aclk_rate[0];
	}

	get_transient_axi_clk_cfg(np);

	return 0;
}

int32_t power_on_enable_axi_clk(int32_t core_id)
{
	uint32_t low_rate_clk;
	if (clk_prepare_enable(get_venc_clock_manager((uint32_t)core_id)->venc_aclk) != 0) {
		VCODEC_FATAL_VENC("core %d, prepare aclk enable failed", core_id);
		return VCODEC_FAILURE;
	}
	low_rate_clk = get_venc_config()->venc_conf_com.aclk_rate[VENC_CLK_RATE_LOWER];
	if (clk_set_rate(get_venc_clock_manager((uint32_t)core_id)->venc_aclk, low_rate_clk) != 0) {
		VCODEC_FATAL_VENC("core %d, set aclk low rate failed", core_id);
		clk_disable_unprepare(get_venc_clock_manager((uint32_t)core_id)->venc_aclk);
		return VCODEC_FAILURE;
	}
	return 0;
}

void power_off_disable_axi_clk(int32_t core_id)
{
	int32_t ret;
	uint32_t current_clk;
	uint32_t low_rate_clk;

	current_clk = (uint32_t)clk_get_rate(get_venc_clock_manager((uint32_t)core_id)->venc_aclk);
	low_rate_clk = get_venc_config()->venc_conf_com.power_off_aclk_rate;
	ret = clk_set_rate(get_venc_clock_manager((uint32_t)core_id)->venc_aclk, low_rate_clk);
	if (ret)
		VCODEC_ERR_VENC("core_id is %d, set lowest aclk rate failed before power off", core_id);

	clk_disable_unprepare(get_venc_clock_manager((uint32_t)core_id)->venc_aclk);
}

int32_t set_axi_clk_rate_single_core(venc_clk_t clk_type, int32_t core_id)
{
	int32_t ret;
	uint32_t need_clk;
	uint32_t current_clk;
	venc_clk_t need_clk_type;

	need_clk = get_venc_config()->venc_conf_com.aclk_rate[clk_type];
	current_clk = (uint32_t)clk_get_rate(get_venc_clock_manager((uint32_t)core_id)->venc_aclk);
	if (need_clk == current_clk) {
		get_venc_clock_manager((uint32_t)core_id)->curr_clk_type = clk_type;
		return 0;
	}

	VCODEC_DBG_VENC("core %d: set aclk type from %d to %d, aclk rate from %u to %u",
			core_id, get_venc_clock_manager((uint32_t)core_id)->curr_clk_type, clk_type, current_clk, need_clk);
	need_clk_type = clk_type;
	ret = clk_set_rate(get_venc_clock_manager((uint32_t)core_id)->venc_aclk, need_clk);

	while ((ret != 0) && (need_clk_type > 0)) {
		need_clk_type--;
		if (current_clk != get_venc_config()->venc_conf_com.aclk_rate[need_clk_type]) {
			VCODEC_WARN_VENC("core_id is %d, failed set aclk to %u Hz,fail code is %d", core_id, need_clk, ret);
			need_clk = get_venc_config()->venc_conf_com.aclk_rate[need_clk_type];
			ret = clk_set_rate(get_venc_clock_manager((uint32_t)core_id)->venc_aclk, need_clk);
		} else {
			return 0;
		}
	}
	return ret;
}
