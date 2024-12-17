/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#include "hvgr_platform_api.h"

#include <linux/of.h>
#include <linux/io.h>
#include <linux/errno.h>
#include "hvgr_mem_api.h"


int hvgr_platform_map_reg_addr(struct platform_device *pdev, struct hvgr_device *gdev)
{
	struct hvgr_crg_resource *crg_rst = &gdev->dm_dev.crg_rst;
#ifdef HVGR_FEATURE_SYSTEM_CACHE
	struct hvgr_ctrl_resource *ctrl_reg = &gdev->dm_dev.ctrl_reg;
#endif

	if (of_property_read_u32_array(gdev->dev->of_node, "crg-reset",
		(uint32_t *)crg_rst, HVGR_CRG_REG_PARA_NUM) != 0) {
		dev_err(gdev->dev, "get crg-reset property failed");
		return -EINVAL;
	}

	crg_rst->reg_base = devm_ioremap(&pdev->dev, crg_rst->crg_start, crg_rst->crg_size);
	if (crg_rst->reg_base == NULL) {
		dev_err(gdev->dev, "map crg-reset register failed");
		return -EINVAL;
	}

#ifdef HVGR_FEATURE_SYSTEM_CACHE
	if (of_property_read_u32_array(gdev->dev->of_node, "ctrl_reg",
		(uint32_t *)ctrl_reg, HVGR_CTRL_REG_PARA_NUM) != 0) {
		dev_err(gdev->dev, "Can't find gpu ctrl_reg register config");
		return 0;
	}

	ctrl_reg->reg_base = devm_ioremap(&pdev->dev, ctrl_reg->reg_start, ctrl_reg->reg_size);
	if (ctrl_reg->reg_base == NULL)
		dev_err(gdev->dev, "map ctrl_reg register failed");
	else
		hvgr_sc_config_remap_register_early(gdev);
#endif

	return 0;
}