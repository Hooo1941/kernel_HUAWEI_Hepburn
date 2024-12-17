/*
 * npu_chip_cfg.c
 *
 * about chip config
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
 */

#include <linux/io.h>
#include <npu_shm_config.h>
#include "npu_log.h"
#include "npu_common.h"
#include "npu_platform.h"
#include "npu_atf_subsys.h"
#include "npu_adapter.h"

/*
 * return value: 1 limited chip, 0 full specification chip
 */
static bool npu_plat_is_limited_chip(void)
{
	return false;
}

/*
 * return value : 1 support sc; 0 not support sc
 */
bool npu_plat_is_support_sc(void)
{
	return !npu_plat_is_limited_chip();
}

/*
 * return value : NPU_NON_BYPASS, NPU_BYPASS
 */
int npu_plat_bypass_status(void)
{
	int ret;
	u32 index;
	struct device_node *soc_spec_np = NULL;
	const char *soc_spec = NULL;
	static int bypass_status = NPU_INIT_BYPASS;
	int tmp_status = NPU_NON_BYPASS;
	static const char *npu_bypass_soc_spec[] = {
		"ap_lite_pad", "pc", "lite_pc", "unknown",
	};

	if (bypass_status != NPU_INIT_BYPASS) {
		if (bypass_status == NPU_BYPASS)
			npu_drv_warn("npu is bypass\n");
		return bypass_status;
	}

	soc_spec_np = of_find_compatible_node(NULL, NULL, "hisilicon, soc_spec");
	if (soc_spec_np == NULL) {
		npu_drv_err("find compatible node fail\n");
		return bypass_status;
	}

	ret = of_property_read_string(soc_spec_np, "soc_spec_set", &soc_spec);
	if (ret < 0) {
		npu_drv_err("read property string fail\n");
		return bypass_status;
	}

	npu_drv_warn("soc spec is %s\n", soc_spec);

	for (index = 0; index < ARRAY_SIZE(npu_bypass_soc_spec); index++) {
		if (strcmp(soc_spec, npu_bypass_soc_spec[index]) == 0) {
			npu_drv_warn("npu bypass, spec=%s\n", soc_spec);
			tmp_status = NPU_BYPASS;
			break;
		}
	}

	bypass_status = tmp_status;

	return bypass_status;
}

