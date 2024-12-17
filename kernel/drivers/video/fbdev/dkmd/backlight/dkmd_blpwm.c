/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
#include "dkmd_blpwm.h"

#define DKMD_BLPWM_DEVICE_MAX 8
static struct blpwm_device {
	const char *dev_name;
	struct blpwm_dev_ops *dev_ops;
} g_blpwm_device[DKMD_BLPWM_DEVICE_MAX];

static uint32_t g_blpwm_dev_count = 0;

static struct blpwm_dev_ops *get_blpwm_dev_ops(const char *dev_name)
{
	uint32_t i;
	struct blpwm_device *dev = NULL;

	if (dev_name == NULL)
		return NULL;

	for (i = 0; i < g_blpwm_dev_count; i++) {
		dev = &g_blpwm_device[i];
		if (dev->dev_name == NULL)
			continue;

		if (strcmp(dev->dev_name, dev_name) == 0)
			return dev->dev_ops;
	}

	return NULL;
}

int dkmd_blpwm_on(struct panel_drv_private *priv)
{
	struct blpwm_dev_ops *dev_ops = NULL;
	int ret = 0;

	if (unlikely(!priv)) {
		dpu_pr_warn("priv is null");
		return -1;
	}

	if (priv->bl_ic_ctrl_mode == BLPWM_MODE) {
		ret = blpwm_drv_on(priv);
		if (ret != 0)
			return ret;
	}

	dev_ops = get_blpwm_dev_ops(priv->bl_dev_name);
	if (unlikely(!dev_ops)) {
		dpu_pr_err("bl_dev_name %s dev_ops is null", priv->bl_dev_name);
		return -1;
	}

	if (likely(dev_ops->on))
		return dev_ops->on(NULL);

	return 0;
}

int dkmd_blpwm_off(struct panel_drv_private *priv)
{
	struct blpwm_dev_ops *dev_ops = NULL;
	int ret = 0;

	if (unlikely(!priv)) {
		dpu_pr_warn("priv is null");
		return -1;
	}

	if (priv->bl_ic_ctrl_mode == BLPWM_MODE) {
		ret = blpwm_drv_off(priv);
		if (ret != 0)
			return ret;
	}

	dev_ops = get_blpwm_dev_ops(priv->bl_dev_name);
	if (unlikely(!dev_ops)) {
		dpu_pr_err("bl_dev_name %s dev_ops is null", priv->bl_dev_name);
		return -1;
	}

	if (likely(dev_ops->off))
		return dev_ops->off(NULL);

	return 0;
}

int dkmd_blpwm_set_backlight(struct panel_drv_private *priv, uint32_t bkl_lvl)
{
	struct blpwm_dev_ops *dev_ops = NULL;

	if (unlikely(!priv)) {
		dpu_pr_warn("priv is null");
		return -1;
	}

	if (priv->bl_ic_ctrl_mode == BLPWM_MODE) {
		blpwm_drv_set_backlight(priv, bkl_lvl);
		return 0;
	}

	dev_ops = get_blpwm_dev_ops(priv->bl_dev_name);
	if (unlikely(!dev_ops)) {
		dpu_pr_err("bl_dev_name %s dev_ops is null", priv->bl_dev_name);
		return -1;
	}

	if (likely(dev_ops->set_backlight))
		return dev_ops->set_backlight(NULL, bkl_lvl);

	return 0;
}

void dkmd_blpwm_register_bl_device(const char *dev_name, struct blpwm_dev_ops *ops)
{
	if (ops == NULL || dev_name == NULL) {
		dpu_pr_err("ops or dev_name is null");
		return;
	}

	if (g_blpwm_dev_count >= DKMD_BLPWM_DEVICE_MAX) {
		dpu_pr_err("register bl device %s fail, index is overflow", dev_name);
		return;
	}

	g_blpwm_device[g_blpwm_dev_count].dev_name = dev_name;
	g_blpwm_device[g_blpwm_dev_count].dev_ops = ops;
	++g_blpwm_dev_count;
}