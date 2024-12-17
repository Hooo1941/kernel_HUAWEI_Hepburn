
/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <securec.h>
#include "dpu_comp_mipi_dsi_bit_clk_upt.h"
#include "dpu_comp_mgr.h"
#include "dkmd_log.h"
#include "dkmd_peri.h"

#include "dpu_connector.h"
#include "gfxdev_mgr.h"
#include "dpu_conn_mgr.h"
#include "chrdev/dkmd_sysfs.h"

static ssize_t mipi_dsi_bit_clk_upt_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret;
	struct dpu_composer *dpu_comp = NULL;
	struct dpu_connector *connector = NULL;
	struct mipi_panel_info *mipi = NULL;

	dpu_comp = to_dpu_composer(get_comp_from_device(dev));
	dpu_check_and_return(!dpu_comp, -EINVAL, err, "dpu_comp is NULL\n");
	dpu_pr_debug("comp index = %u!\n", dpu_comp->comp.index);
	connector = get_primary_connector(dpu_comp->conn_info);
	dpu_check_and_return(!connector, -EINVAL, err, "connector is NULL\n");
	mipi = &connector->mipi;
	ret = snprintf_s(buf, PAGE_SIZE + 1, PAGE_SIZE, "%u", mipi->dsi_bit_clk);
	return ret;
}

static ssize_t mipi_dsi_bit_clk_upt_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	uint32_t dsi_bit_clk_upt = 0;
	size_t n_str = 0;
	int i = 0;
	int wait_count = 0;
	struct dpu_composer *dpu_comp = NULL;
	struct dpu_connector *connector = NULL;
	struct mipi_panel_info *mipi = NULL;

	dpu_comp = to_dpu_composer(get_comp_from_device(dev));
	dpu_check_and_return(!dpu_comp, -EINVAL, err, "dpu_comp is NULL\n");
	connector = get_primary_connector(dpu_comp->conn_info);
	dpu_check_and_return(!connector, -EINVAL, err, "connector is NULL\n");
	mipi = &connector->mipi;

	if (mipi->dsi_bit_clk_upt_support == 0) {
		dpu_pr_info("connector %d mipi clk upt is not supported", connector->connector_id);
		return (ssize_t)count;
	}

	for (i = 0; buf[i] != '\0' && buf[i] != '\n'; i++) {
		n_str++;
		if (n_str >= 6) {
			dpu_pr_err("invalid parameter: n_str = %lu, count = %lu\n",
				n_str, count);
			break;
		}
	}

	if (n_str != 5) {
		dpu_pr_err("invalid parameter: n_str = %d, count = %ld\n",
			n_str, count);
		return (ssize_t)count;
	}

	dpu_pr_debug("comp index = %u!\n", dpu_comp->comp.index);

	if (strncmp(buf, MIPI_DSI_BIT_CLK_STR1, n_str) == 0)
		dsi_bit_clk_upt = mipi->dsi_bit_clk_val1;
	else if (strncmp(buf, MIPI_DSI_BIT_CLK_STR2, n_str) == 0)
		dsi_bit_clk_upt = mipi->dsi_bit_clk_val2;
	else if (strncmp(buf, MIPI_DSI_BIT_CLK_STR3, n_str) == 0)
		dsi_bit_clk_upt = mipi->dsi_bit_clk_val3;
	else if (strncmp(buf, MIPI_DSI_BIT_CLK_STR4, n_str) == 0)
		dsi_bit_clk_upt = mipi->dsi_bit_clk_val4;
	else if (strncmp(buf, MIPI_DSI_BIT_CLK_STR5, n_str) == 0)
		dsi_bit_clk_upt = mipi->dsi_bit_clk_val5;
	else
		dpu_pr_debug("comp index = %u, unknown dsi_bit_clk_index!\n",
			dpu_comp->comp.index);

	if (dsi_bit_clk_upt == 0)
		return (ssize_t)count;

	/* wait when dsi_bit_clk_upt_flag flag is true */
	while ((atomic_read(&connector->mipi.dsi_bit_clk_upt_flag) == 1) &&
			(wait_count < MIPI_CLK_UPDT_TIMEOUT)) {
		wait_count++;
		msleep(20); // msleep < 20ms can sleep for up to 20ms
	}

	dpu_check_and_return(wait_count >= MIPI_CLK_UPDT_TIMEOUT, -EINVAL, info,
		"dsi clk %u store timeout\n", dsi_bit_clk_upt);

	mipi->dsi_bit_clk_upt = dsi_bit_clk_upt;
	return (ssize_t)count;
}

static DEVICE_ATTR(mipi_dsi_bit_clk_upt, 0640, mipi_dsi_bit_clk_upt_show, mipi_dsi_bit_clk_upt_store);
void dpu_comp_mipi_dsi_bit_clk_upt_init(struct dpu_composer *dpu_comp)
{
	dkmd_sysfs_attrs_append(&dpu_comp->attrs, &dev_attr_mipi_dsi_bit_clk_upt.attr);
}
