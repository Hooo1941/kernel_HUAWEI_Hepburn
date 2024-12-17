/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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

#include <linux/interrupt.h>
#include <linux/gpio.h>
#include "dpu_conn_mgr.h"
#include <platform_include/see/efuse_driver.h>
#include "hdmitx_drv.h"
#include "peri/dkmd_peri.h"

struct platform_device *g_dkmd_hdmitx_devive = NULL;
static struct pinctrl_data g_hpd_pctrl;

static struct pinctrl_cmd_desc hpd_pinctrl_init_cmds[] = {
	{DTYPE_PINCTRL_GET, &g_hpd_pctrl, 0},
	{DTYPE_PINCTRL_STATE_GET, &g_hpd_pctrl, DTYPE_PINCTRL_STATE_DEFAULT},
	{DTYPE_PINCTRL_STATE_GET, &g_hpd_pctrl, DTYPE_PINCTRL_STATE_IDLE},
	{DTYPE_PINCTRL_SET, &g_hpd_pctrl, DTYPE_PINCTRL_STATE_IDLE},
	{DTYPE_PINCTRL_SET, &g_hpd_pctrl, DTYPE_PINCTRL_STATE_DEFAULT},
};

static struct pinctrl_cmd_desc hpd_pinctrl_finit_cmds[] = {
	{DTYPE_PINCTRL_PUT, &g_hpd_pctrl, 0},
};

#define SDEV_NAME_HDMITX "gfx_hdmitx"

static int32_t hdmitx_panel_parse_dt(struct hdmitx_private *priv, struct device_node *np)
{
	int ret;
	struct dkmd_connector_info *pinfo = &priv->connector_info;

	priv->hdmitx_base = of_iomap(np, 0);
	if (!priv->hdmitx_base) {
		dpu_pr_err("failed to get hidptx_base!");
		return -ENXIO;
	}

	priv->hsdt1_crg_base = of_iomap(np, 1);
	if (!priv->hsdt1_crg_base) {
		dpu_pr_err("failed to get hsdt1_crg_base!");
		return -ENXIO;
	}

	priv->hsdt1_sub_harden_base = of_iomap(np, 2);
	if (!priv->hsdt1_sub_harden_base) {
		dpu_pr_err("failed to get hsdt1_sub_harden_base!");
		return -ENXIO;
	}

	priv->hsdt1_sys_ctrl_base = of_iomap(np, 3);
	if (!priv->hsdt1_sys_ctrl_base) {
		dpu_pr_err("failed to get hsdt1_sys_ctrl_base!");
		return -ENXIO;
	}

	priv->hsdt1_xctrl_base = of_iomap(np, 4);
	if (!priv->hsdt1_xctrl_base) {
		dpu_pr_err("failed to get hsdt1_xctrl_base!");
		return -ENXIO;
	}

	priv->hpd_irq_no = (uint32_t)of_irq_get(np, 0);
	dpu_pr_info("hpd_irq_no=%#x", priv->hpd_irq_no);

	priv->hpd_gpio = (uint32_t)of_get_named_gpio(np, "gpios", 0);
	dpu_pr_info("hpd_gpio=%#x", priv->hpd_gpio);

	ret = of_property_read_u32(np, "fpga_flag", &pinfo->base.fpga_flag);
	if (ret) {
		dpu_pr_info("get fpga_flag failed!");
		pinfo->base.fpga_flag = 0;
	}
	dpu_pr_info("fpga_flag=%#x", pinfo->base.fpga_flag);

	ret = of_property_read_u32(np, "pipe_sw_itfch_idx", &pinfo->base.pipe_sw_itfch_idx);
	if (ret) {
		dpu_pr_info("get pipe_sw_itfch_idx failed!");
		pinfo->base.pipe_sw_itfch_idx = PIPE_SW_PRE_ITFCH1;
	}
	dpu_pr_info("pipe_sw_pre_itfch_idx=%#x", pinfo->base.pipe_sw_itfch_idx);

	ret = of_property_read_u32(np, "fake_panel_flag", &pinfo->base.fake_panel_flag);
	if (ret)
		pinfo->base.fake_panel_flag = 0;
	dpu_pr_info("fake_panel_flag=%#x", pinfo->base.fake_panel_flag);

	base_panel_connector_dts_parse(pinfo, np);

	return 0;
}

static const struct of_device_id hdmitx_device_match_table[] = {
	{
		.compatible = DTS_COMP_HDMI,
		.data = NULL,
	},
	{},
};
MODULE_DEVICE_TABLE(of, hdmitx_device_match_table);

static bool hdmitx_is_bypass_in_pg(void)
{
	const char *soc_spec = NULL;
	struct device_node *np = of_find_compatible_node(NULL, NULL, "hisilicon, soc_spec");
	if (!np) {
		dpu_pr_err("no soc_spec dts node");
		return false;
	}

	if (of_property_read_string(np, "soc_spec_set", &soc_spec)) {
		dpu_pr_err("no soc_spec_set dts info");
		return false;
	}

	if (soc_spec == NULL) {
		dpu_pr_err("get pg type fail");
		return false;
	}

	dpu_pr_debug("hdmitx_is_bypass_in_pg soc spec is %s:", soc_spec);
	if (strncmp(soc_spec, "level1", strlen("level1")) == 0 ||
		strncmp(soc_spec, "pc", strlen("pc")) == 0 ||
		strncmp(soc_spec, "desktop_pc", strlen("desktop_pc")) == 0) {
		dpu_pr_debug("soc spec is valid, so hdmi do not need bypass");
		return false;
	}

	return true;
}

static int32_t hdmitx_probe(struct platform_device *pdev)
{
	int ret;
	struct hdmitx_private *priv = NULL;

	dpu_pr_info("+");

	if (unlikely(!pdev)) {
		dpu_pr_warn("pdev is null");
		return -1;
	}

	if (hdmitx_is_bypass_in_pg()) {
		dpu_pr_err("HDMI bypass by pg type");
		return -EINVAL;
	}

	if (!is_connector_manager_available())
		return -EPROBE_DEFER;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		dpu_pr_err("alloc offline driver private data failed");
		return -EINVAL;
	}

	priv->device = pdev;
	priv->dev = &pdev->dev;
	if (hdmitx_panel_parse_dt(priv, pdev->dev.of_node)){
		dpu_pr_err("parse dts error");
		devm_kfree(&pdev->dev, priv);
		return -EINVAL;
	}

	/* this is the tail device */
	priv->connector_info.base.peri_device = pdev;
	priv->connector_info.base.name = DEV_NAME_HDMI;
	priv->connector_info.base.type = PANEL_HDMI;
	priv->connector_info.base.pluggable_connect_mutex = devm_kzalloc(&pdev->dev, sizeof(struct mutex), GFP_KERNEL);
	if (!priv->connector_info.base.pluggable_connect_mutex) {
		dpu_pr_warn("[HDMI] alloc hdmitx_connect_mutex failed!\n");
		return -EINVAL;
	}
	mutex_init(priv->connector_info.base.pluggable_connect_mutex);

	/* add private data to device */
	platform_set_drvdata(pdev, &priv->connector_info);

	/* will create chrdev: /dev/gfx_hdmi  */
	if (register_connector(&priv->connector_info) != 0) {
		dpu_pr_warn("device register failed, need retry!");
		devm_kfree(&pdev->dev, priv);
		return -EPROBE_DEFER;
	}

	/* create sdev for hdmi */
	priv->hdmitx.sdev.name = SDEV_NAME_HDMITX;
	dpu_pr_info("register hdmitx sdev");
	if (switch_dev_register(&priv->hdmitx.sdev) < 0)
		dpu_pr_err("[HDMITX]hdmitx sdev registration failed");

	ret = peri_pinctrl_cmds_tx(priv->device, hpd_pinctrl_init_cmds, ARRAY_SIZE(hpd_pinctrl_init_cmds));
	if (ret != 0)
		dpu_pr_err("Init hpd pinctrl failed! ret=%d", ret);

	g_dkmd_hdmitx_devive = pdev;
	enable_irq(priv->hpd_irq_no);
	dpu_pr_info("-");

	return 0;
}

/**
 * Clear resource when device removed but not for devicetree device
 */
static int32_t hdmitx_remove(struct platform_device *pdev)
{
	struct hdmitx_private *priv = platform_get_drvdata(pdev);

	if (!priv) {
		dpu_pr_err("get dsi private data failed!");
		return -EINVAL;
	}

	peri_pinctrl_cmds_tx(priv->device, hpd_pinctrl_finit_cmds, ARRAY_SIZE(hpd_pinctrl_finit_cmds));
	gpio_free(priv->hpd_gpio);
	free_irq(priv->hpd_irq_no, NULL);
	unregister_connector(&priv->connector_info);
	// unregister hdmi sdev devive
	switch_dev_unregister(&priv->hdmitx.sdev);

	dpu_pr_info("%s remove complete!", priv->connector_info.base.name);

	return 0;
}

static struct platform_driver hdmitx_platform_driver = {
	.probe  = hdmitx_probe,
	.remove = hdmitx_remove,
	.driver = {
		.name  = DEV_NAME_HDMI,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hdmitx_device_match_table),
	}
};

static int32_t __init hdmitx_register(void)
{
	return platform_driver_register(&hdmitx_platform_driver);
}

static void __exit hdmitx_unregister(void)
{
	platform_driver_unregister(&hdmitx_platform_driver);
}

late_initcall(hdmitx_register);
module_exit(hdmitx_unregister);

MODULE_AUTHOR("Graphics Display");
MODULE_DESCRIPTION("HDMITX Module Driver");
MODULE_LICENSE("GPL");

