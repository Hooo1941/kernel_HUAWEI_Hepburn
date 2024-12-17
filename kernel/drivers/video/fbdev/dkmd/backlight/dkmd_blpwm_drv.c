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
#include "dkmd_blpwm_drv.h"
#include <soc_blpwm_interface.h>
#include "dkmd_blpwm.h"
#include "dkmd_blpwm_api.h"
#include "peri/dkmd_peri.h"

#define DEV_NAME_BLPWM "dpu_blpwm"
#define DTS_COMP_BLPWM_NAME "hisilicon,dpublpwm"

static struct blpwm_drv_private *g_blpwm_drv = NULL;

static struct pinctrl_data blpwmpctrl;

static struct pinctrl_cmd_desc blpwm_pinctrl_init_cmds[] = {
	{DTYPE_PINCTRL_GET, &blpwmpctrl, 0},
	{DTYPE_PINCTRL_STATE_GET, &blpwmpctrl, DTYPE_PINCTRL_STATE_DEFAULT},
	{DTYPE_PINCTRL_STATE_GET, &blpwmpctrl, DTYPE_PINCTRL_STATE_IDLE},
};

static struct pinctrl_cmd_desc blpwm_pinctrl_normal_cmds[] = {
	{DTYPE_PINCTRL_SET, &blpwmpctrl, DTYPE_PINCTRL_STATE_DEFAULT},
};

static struct pinctrl_cmd_desc blpwm_pinctrl_lowpower_cmds[] = {
	{DTYPE_PINCTRL_SET, &blpwmpctrl, DTYPE_PINCTRL_STATE_IDLE},
};

static struct pinctrl_cmd_desc blpwm_pinctrl_finit_cmds[] = {
	{DTYPE_PINCTRL_PUT, &blpwmpctrl, 0},
};

#define BLPWM_OUT_PRECISION_DEFAULT 800
#define BLPWM_OUT_PRECISION 10000
#define BLPWM_OUT_PRECISION_2048 2047
#define BLPWM_OUT_PRECISION_4096 4095

struct blpwm_out {
	uint32_t precision;
	uint32_t div;
};
static struct blpwm_out g_blpwm_out_info[BLPWM_PRECISION_TYPE_MAX] = {
	{BLPWM_OUT_PRECISION, 0x2},
	{BLPWM_OUT_PRECISION, 0x1},
	{BLPWM_OUT_PRECISION_2048, 0x5},
	{BLPWM_OUT_PRECISION_4096, 0x5}
};

int blpwm_drv_on(struct panel_drv_private *priv)
{
	int ret;
	char __iomem *blpwm_base = NULL;

	dpu_check_and_return(!priv, -EINVAL, err, "priv info is NULL!\n");

	if (g_blpwm_drv == NULL) {
		dpu_pr_err("blpwm driver is NULL!\n");
		return 0;
	}

	if (priv->blpwm_precision_type >= BLPWM_PRECISION_TYPE_MAX) {
		dpu_pr_err("blpwm_precision_type is out of range!\n");
		return 0;
	}

	if (g_blpwm_drv->blpwm_on == 1)
		return 0;
	blpwm_base = g_blpwm_drv->blpwm_base;

	if (g_blpwm_drv->blpwm_clk) {
		ret = clk_prepare(g_blpwm_drv->blpwm_clk);
		if (ret) {
			dpu_pr_err("dss_blpwm_clk clk_prepare failed, error=%d!\n", ret);
			return -EINVAL;
		}

		ret = clk_enable(g_blpwm_drv->blpwm_clk);
		if (ret) {
			dpu_pr_err("dss_blpwm_clk clk_enable failed, error=%d!\n", ret);
			return -EINVAL;
		}
	}

	ret = peri_pinctrl_cmds_tx(g_blpwm_drv->pdev, blpwm_pinctrl_normal_cmds, ARRAY_SIZE(blpwm_pinctrl_normal_cmds));

	outp32(SOC_BLPWM_OUT_CTRL_ADDR(blpwm_base), 0x1);
	outp32(SOC_BLPWM_OUT_DIV_ADDR(blpwm_base), g_blpwm_out_info[priv->blpwm_precision_type].div);

	if (priv->blpwm_input_ena) {
		/* config blpwm in */
		outp32(SOC_BLPWM_IN_CTRL_ADDR(blpwm_base), 0x1);
		outp32(SOC_BLPWM_IN_DIV_ADDR(blpwm_base), 0x1);
	}

	g_blpwm_drv->blpwm_on = 1;
	return 0;
}

int blpwm_drv_off(struct panel_drv_private *priv)
{
	int ret;

	dpu_check_and_return(!priv, -EINVAL, err, "info is NULL!\n");

	if (g_blpwm_drv == NULL) {
		dpu_pr_err("blpwm driver is NULL!\n");
		return 0;
	}

	if (g_blpwm_drv->blpwm_on == 0)
		return 0;

	outp32(SOC_BLPWM_OUT_CTRL_ADDR(g_blpwm_drv->blpwm_base), 0x0);

	ret = peri_pinctrl_cmds_tx(g_blpwm_drv->pdev, blpwm_pinctrl_lowpower_cmds,
		ARRAY_SIZE(blpwm_pinctrl_lowpower_cmds));

	if (g_blpwm_drv->blpwm_clk) {
		clk_disable(g_blpwm_drv->blpwm_clk);
		clk_unprepare(g_blpwm_drv->blpwm_clk);
	}

	if (priv->blpwm_input_ena) {
		if (unlikely(!(g_blpwm_drv->blpwm_base)))
			dpu_pr_warn("blpwm_base is NULL!\n");
		else
			outp32(SOC_BLPWM_IN_CTRL_ADDR(g_blpwm_drv->blpwm_base), 0x0);
	}

	g_blpwm_drv->blpwm_on = 0;

	return ret;
}

int blpwm_drv_set_backlight(struct panel_drv_private *priv, uint32_t bkl_lvl)
{
	uint32_t brightness;
	uint32_t blpwm_out_precision;

	dpu_check_and_return(!priv, -EINVAL, err, "info is NULL!\n");

	if (g_blpwm_drv == NULL) {
		dpu_pr_err("blpwm driver is NULL!\n");
		return 0;
	}

	if (priv->blpwm_precision_type >= BLPWM_PRECISION_TYPE_MAX) {
		dpu_pr_err("blpwm_precision_type is out of range!\n");
		return 0;
	}

	blpwm_out_precision = g_blpwm_out_info[priv->blpwm_precision_type].precision;
	brightness = (bkl_lvl << 16) | (blpwm_out_precision - bkl_lvl);
	outp32(SOC_BLPWM_OUT_CFG_ADDR(g_blpwm_drv->blpwm_base), brightness);

	dpu_pr_debug("blpwm_out_precision=%u, bl_level=%u, brightness=%u\n", blpwm_out_precision, bkl_lvl, brightness);
	return 0;
}

static int blpwm_drv_probe(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	struct blpwm_drv_private *priv = NULL;
	int ret;

	dpu_pr_debug("+.\n");

	dpu_check_and_return(!pdev, -EINVAL, err, "pdev is null!\n");

	if (g_blpwm_drv != NULL) {
		dpu_pr_info("blpwm driver is ready!\n");
		return 0;
	}

	priv = (struct blpwm_drv_private *)devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		dpu_pr_err("alloc blpwm_drv_private failed!\n");
		return -ENOMEM;
	}

	priv->pdev = pdev;

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_BLPWM_NAME);
	if (!np) {
		dpu_pr_err("NOT FOUND device node %s!\n", DTS_COMP_BLPWM_NAME);
		goto err_return;
	}

	/* get blpwm reg base */
	priv->blpwm_base = of_iomap(np, 0);
	if (!priv->blpwm_base) {
		dpu_pr_err("failed to get blpwm_base resource.\n");
		goto err_return;
	}

	ret = of_property_read_u32(np, "fpga_flag", &priv->fpga_flag);
	if (ret) {
		dpu_pr_err("failed to get fpga_flag resource.\n");
		goto err_return;
	}

	if (priv->fpga_flag == 0) {
		/* blpwm pinctrl init */
		ret = peri_pinctrl_cmds_tx(priv->pdev, blpwm_pinctrl_init_cmds, ARRAY_SIZE(blpwm_pinctrl_init_cmds));
		if (ret != 0) {
			dpu_pr_err("Init blpwm pinctrl failed! ret=%d.\n", ret);
			goto err_return;
		}

		/* get blpwm clk resource */
		priv->blpwm_clk = of_clk_get(np, 0);
		if (IS_ERR(priv->blpwm_clk)) {
			dpu_pr_err("%s clock not found: %d!\n", np->name, (int)PTR_ERR(priv->blpwm_clk));
			goto err_return;
		}

		dpu_pr_info("blpwm_clk:[%lu].\n", clk_get_rate(priv->blpwm_clk));
	}

	g_blpwm_drv = priv;

	dpu_pr_debug("-.\n");

	return 0;

err_return:
	devm_kfree(&pdev->dev, priv);
	ret = -ENXIO;
	return ret;
}

static int blpwm_drv_remove(struct platform_device *pdev)
{
	int ret;

	dpu_check_and_return(!pdev, -EINVAL, err, "pdev is null!\n");

	ret = peri_pinctrl_cmds_tx(pdev, blpwm_pinctrl_finit_cmds, ARRAY_SIZE(blpwm_pinctrl_finit_cmds));

	if (!g_blpwm_drv) {
		dpu_pr_err("blpwm driver is NULL!\n");
		return 0;
	}

	if (g_blpwm_drv->blpwm_clk) {
		clk_put(g_blpwm_drv->blpwm_clk);
		g_blpwm_drv->blpwm_clk = NULL;
	}

	return ret;
}

static const struct of_device_id blpwm_drv_match_table[] = {
	{
		.compatible = DTS_COMP_BLPWM_NAME,
		.data = NULL,
	},
	{},
};
MODULE_DEVICE_TABLE(of, blpwm_drv_match_table);

static struct platform_driver this_driver = {
	.probe = blpwm_drv_probe,
	.remove = blpwm_drv_remove,
	.suspend = NULL,
	.resume = NULL,
	.shutdown = NULL,
	.driver = {
		.name = DEV_NAME_BLPWM,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(blpwm_drv_match_table),
	},
};

static int __init blpwm_drv_register(void)
{
	int ret;

	ret = platform_driver_register(&this_driver);
	if (ret)
		dpu_pr_err("platform_driver_register failed, error=%d!\n", ret);

	return ret;
}

module_init(blpwm_drv_register);