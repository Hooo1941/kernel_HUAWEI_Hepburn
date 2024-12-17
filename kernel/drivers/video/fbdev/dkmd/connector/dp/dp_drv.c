/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
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

#include "dp_ctrl.h"
#include "dpu_conn_mgr.h"
#include "dp_drv.h"
#include <linux/types.h>
#include <linux/of.h>
#include "dp_ctrl_config.h"
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <platform_include/display/linux/dpu_dss_dp.h>

#include <securec.h>
#include "drm_dp_helper_additions.h"

#define WORKQUEUE_NAME_SIZE 128
struct platform_device *g_dkmd_dp_devive[MAX_DPTX_DEV_INDEX] = {};

int dptx_init_dpc_num(struct dp_ctrl *dptx)
{
	dpu_check_and_return(!dptx, -EINVAL, err, "[DP] dptx is null pointer!");

	switch (dptx->port_id) { // usb/dp combophy：port 0~3, dp/edp combophy：port 4~5
	case DPTX_INVALID_PORT_ID: /* history platform don't need to select dpc */
	case DPTX_PORT_ID_0:
	case DPTX_PORT_ID_1:
		dptx->dpc_index_selected = DPTX_DPC_NUM_0;
		break;
	case DPTX_PORT_ID_2:
	case DPTX_PORT_ID_4:
		dptx->dpc_index_selected = DPTX_DPC_NUM_1;
		break;
	case DPTX_PORT_ID_3:
	case DPTX_PORT_ID_5:
		dptx->dpc_index_selected = DPTX_DPC_NUM_2;
		break;
	default:
		dpu_pr_err("[DP] input error port id, not support!");
		return -1;
	}
	dpu_pr_info("[DP] dptx->dpc_index_selected is %d", dptx->dpc_index_selected);
	return 0;
}

int get_dp_devive_index(int port_id, uint32_t *dev_index)
{
	switch (port_id) { // usb/dp combophy：port 0~3, dp/edp combophy：port 4~5
	case DPTX_INVALID_PORT_ID: /* history platform use gfx_dp */
	case DPTX_PORT_ID_0:
	case DPTX_PORT_ID_1:
		*dev_index = GFX_DP;
		break;
	case DPTX_PORT_ID_2:
	case DPTX_PORT_ID_4:
		*dev_index = GFX_DP1;
		break;
	case DPTX_PORT_ID_3:
	case DPTX_PORT_ID_5:
		*dev_index = GFX_DP2;
		break;
	default:
		dpu_pr_err("[DP] input error port id, not support!");
		return -1;
	}
	dpu_pr_info("[DP] get_dp_devive_index: port id is %d, dev_index is %d", port_id, *dev_index);

	return 0;
}

int32_t dp_set_backlight(struct dpu_connector *connector, const void *value)
{
	struct dp_private *dp_priv = NULL;
	struct dp_connect_ctrl_info *pinfo = NULL;
	uint32_t bl_level = 0;

	if (unlikely(!connector || !value))
		return -1;
	dp_priv = to_dp_private(connector->conn_info);
	if (unlikely(!dp_priv))
		return -1;

	pinfo = dp_priv->dp[0].dp_info;
	bl_level = *((uint32_t *)value);
	if (pinfo && pinfo->set_backlight)
		pinfo->set_backlight(pinfo->port_id, bl_level);

	return 0;
}

static void dp_panel_paser_rate_lane(struct device_node *np, struct dkmd_connector_info *pinfo) {
	int ret;

	ret = of_property_read_u32(np, "min_dptx_rate", &pinfo->min_dptx_rate);
	if (ret) {
		dpu_pr_info("[DP] get min_dptx_rate failed!\n");
		pinfo->min_dptx_rate = MIN_DPTX_RATE;
	}

	ret = of_property_read_u32(np, "max_dptx_rate", &pinfo->max_dptx_rate);
	if (ret) {
		dpu_pr_info("[DP] get max_dptx_rate failed!\n");
		pinfo->max_dptx_rate = DPTX_PHYIF_CTRL_MAX_RATE;
	}

	ret = of_property_read_u32(np, "min_dptx_lane", &pinfo->min_dptx_lane);
	if (ret) {
		dpu_pr_info("[DP] get min_dptx_lane failed!\n");
		pinfo->min_dptx_lane = MIN_DPTX_LANE;
	}

	ret = of_property_read_u32(np, "ssc_dptx_flag", &pinfo->ssc_dptx_flag);
	if (ret) {
		dpu_pr_info("[DP] get ssc_dptx_flag failed!\n");
		pinfo->ssc_dptx_flag = 1;
	}

	ret = of_property_read_u32(np, "fpga_flag", &pinfo->base.fpga_flag);
	if (ret) {
		dpu_pr_info("[DP] get fpga_flag failed!\n");
		pinfo->base.fpga_flag = 0;
	}
	dpu_pr_info("[DP] fpga_flag=%#x", pinfo->base.fpga_flag);
}

static int32_t dp_panel_parse_dt(struct dp_private *priv, struct device_node *np)
{
	int ret;
	uint32_t connector_count = 0;
	uint32_t value = 0;
	const __be32 *p = NULL;
	struct property *prop = NULL;
	struct dp_ctrl *dptx0 = &priv->dp[0];
	struct dp_ctrl *dptx1 = &priv->dp[1];
	struct dkmd_connector_info *pinfo = &priv->connector_info;

	dpu_check_and_return(init_dpctrl_pixel_clk(dptx0, NULL, np), -ENXIO, err, "[DP] init_dpctrl_pixel_clk failed");

	dptx0->hidptx_base = of_iomap(np, REG_HIDPTX_BASE);
	if (!dptx0->hidptx_base) {
		dpu_pr_err("[DP] failed to get hidptx_base!\n");
		return -ENXIO;
	}

	priv->hsdt1_crg_base = of_iomap(np, REG_HSDT1_CRG_BASE);
	if (!priv->hsdt1_crg_base) {
		dpu_pr_err("[DP] failed to get hsdt1_crg_base!\n");
		return -ENXIO;
	}

	dptx0->combo_phy_base = of_iomap(np, REG_COMBOPHY_BASE);
	if (!dptx0->combo_phy_base) {
		dpu_pr_err("[DP] failed to get combo_phy_base!\n");
		return -ENXIO;
	}

	/* v740 adjust difference add */
	dpu_check_and_return(init_dpctrl_16m_clk(dptx0, np), -ENXIO, err, "[DP] init_dpctrl_16m_clk failed");
	dpu_check_and_return(init_hsdt1_sysctrl_base(priv, np), -ENXIO, err, "[DP] init_hsdt1_sysctrl_base failed");
	dpu_check_and_return(init_sub_harden_crg_base(priv, np), -ENXIO, err, "[DP] init_sub_harden_crg_base failed");
	dpu_check_and_return(init_dp_sctrl(priv, np), -ENXIO, err, "[DP] init_dp_sctrl failed");

	priv->hpd_gpio = (uint32_t)of_get_named_gpio(np, "gpios", 0);
	dpu_pr_info("[DP] hpd_gpio= %d", priv->hpd_gpio);
	
	if (of_property_read_u32(np, "dp_hpd_plug", &priv->dp_hpd_plug)) {
		dpu_pr_info("[DP] get dp_hpd_plug failed! set to 0\n");
		priv->dp_hpd_plug = 0;
	}

	ret = of_property_read_string(np, "gfx_dp_dev_name", &pinfo->base.name);
	if (ret) {
		dpu_pr_info("[DP] get gfx_dp_dev_name failed!\n");
		pinfo->base.name = DTS_GFX_DP_NAME;
	}
	dpu_pr_info("[DP] pinfo->base.name=%s\n", pinfo->base.name);

	dp_panel_paser_rate_lane(np, pinfo);

	ret = of_property_read_u32(np, "pipe_sw_itfch_idx", &pinfo->base.pipe_sw_itfch_idx);
	if (ret) {
		dpu_pr_info("[DP] get pipe_sw_itfch_idx failed!\n");
		pinfo->base.pipe_sw_itfch_idx = PIPE_SW_PRE_ITFCH1;
	}

	ret = of_property_read_u32(np, "support_async_online", &pinfo->base.enable_async_online);
	if (ret) {
		dpu_pr_info("[DP] get enable_async_online failed!\n");
		pinfo->base.enable_async_online = 0;
	}
	dpu_pr_info("[DP] support_async_online=%#x", pinfo->base.enable_async_online);

	ret = of_property_read_u32(np, "fake_panel_flag", &pinfo->base.fake_panel_flag);
	if (ret)
		pinfo->base.fake_panel_flag = 0;

	ret = of_property_read_u32(np, "mode", &pinfo->base.mode);
	if (ret)
		pinfo->base.mode = 0;

	ret = of_property_read_u32(np, "channel_id", &pinfo->base.id);
	if (ret)
		pinfo->base.id = 0;

	dpu_pr_info("[DP] min_lane=%#x, min_rate=%#x, max_rate=%#x, ssc_flag=%#x, pipe_sw_pre_itfch_idx=%#x, \
		fake_panel_flag=%#x, dptx_mode=%#x, dptx_id=%#x, dp_hpd_plug=%d", pinfo->min_dptx_lane, pinfo->min_dptx_rate,
		pinfo->max_dptx_rate, pinfo->ssc_dptx_flag, pinfo->base.pipe_sw_itfch_idx, pinfo->base.fake_panel_flag,
		pinfo->base.mode, pinfo->base.id, priv->dp_hpd_plug);

	of_property_for_each_u32(np, "connector_id", prop, p, value) {
		if (connector_count >= MAX_CONNECT_CHN_NUM)
			break;
		++connector_count;
	}

	dptx0->is_dual_tx = false;
	dptx1->is_dual_tx = false;
	if (connector_count > EXTERNAL_CONNECT_CHN_IDX) {
		dpu_check_and_return(init_dpctrl_pixel_clk(NULL, dptx1, np), -ENXIO, err, "[DP] init_dpctrl_pixel_clk failed");

		dptx1->hidptx_base = of_iomap(np, REG_HIDPTX_EXTERNAL_BASE);
		if (!dptx1->hidptx_base) {
			dpu_pr_err("[DP] failed to get hidptx_base!\n");
			return -ENXIO;
		}

		dptx1->combo_phy_base = of_iomap(np, REG_COMBOPHY_EXTERNAL_BASE);
		if (!dptx1->combo_phy_base) {
			dpu_pr_err("[DP] failed to get combo_phy_base!\n");
			return -ENXIO;
		}

		dptx0->is_dual_tx = true;
		dptx1->is_dual_tx = true;
	}
	base_panel_connector_dts_parse(pinfo, np);

	return 0;
}

static const struct of_device_id dp_device_match_table[] = {
	{
		.compatible = DTS_COMP_DP,
		.data = NULL,
	},
	{
		.compatible = DTS_COMP_DP1,
		.data = NULL,
	},
	{
		.compatible = DTS_COMP_DP2,
		.data = NULL,
	},
	{},
};
MODULE_DEVICE_TABLE(of, dp_device_match_table);

static void dp_save_device(struct platform_device *pdev, struct dp_private *priv)
{
	int dev_index = 0;

	if (priv == NULL || pdev == NULL) {
		dpu_pr_err("[DP] pdev or priv is null ptr, error return!\n");
		return;
	}

	if (strcmp(priv->connector_info.base.name, DTS_GFX_DP_NAME) == 0) {
		dev_index = GFX_DP;
	} else if (strcmp(priv->connector_info.base.name, DTS_GFX_DP1_NAME) == 0) {
		dev_index = GFX_DP1;
	} else if (strcmp(priv->connector_info.base.name, DTS_GFX_DP2_NAME) == 0) {
		dev_index = GFX_DP2;
	} else {
		dpu_pr_err("[DP] gfx dp name is invalid!!!\n");
		return;
	}

	g_dkmd_dp_devive[dev_index] = pdev;

	return;
}

static void dptx_isr_pluggable_wq_handler(struct work_struct *work)
{
	struct dp_ctrl *dptx = NULL;
	uint32_t is_pluged = 0;
	struct dp_private *dp_priv = NULL;

	dptx = container_of(work, struct dp_ctrl, dptx_pluggable_work);
	dpu_check_and_no_retval((dptx == NULL), err, "[DPTX] dptx is NULL!");
	dpu_pr_info("[DP] workqueue irq handler start for dpport %d", dptx->port_id);

	dp_priv = to_dp_private(dptx->connector->conn_info);
	dpu_check_and_no_retval(!dp_priv, err, "DPTX is null!");
	is_pluged = (uint32_t)gpio_get_value(dp_priv->hpd_gpio);
	dpu_pr_info("[DP] is_pluged %d", is_pluged);
	if (is_pluged == 1) {
		mdelay(100);
		is_pluged = (uint32_t)gpio_get_value(dp_priv->hpd_gpio);
		dpu_pr_info("[DP] mdelay is_pluged %d", is_pluged);
		if (is_pluged != 1) {
			dpu_pr_info("[DP] ignore the irq is_pluged %d", is_pluged);
		} else {
			dpu_pr_info("[DP] port %d hotplug in!", dptx->port_id);
			dpu_multi_dptx_hpd_trigger(TCA_IRQ_HPD_IN, TCPC_DP, DP_PLUG_TYPE_NORMAL, MASTER_DPTX_IDX, dptx->port_id);
		}
	}
}

static int dptx_pluggable_wq_handler(struct dp_ctrl *dptx, bool setup)
{
	int ret = -1;
	char wq_name[WORKQUEUE_NAME_SIZE] = {0};
	if (setup) {
		ret = snprintf_s(wq_name, sizeof(wq_name), sizeof(wq_name) - 1, "dptx_edp_wq_%d", dptx->port_id);
		if (ret < 0) {
			dpu_pr_err("[DP] create wq_name err");
			return -EINVAL;
		}
		dpu_pr_info("[DP] create retraining workqueue : %s", wq_name);
		dptx->dptx_pluggable_wq = create_singlethread_workqueue(wq_name);
		if (dptx->dptx_pluggable_wq == NULL) {
			dpu_pr_err("[DP] create dptx_pluggable_wq failed");
			return -1;
		}
		INIT_WORK(&dptx->dptx_pluggable_work, dptx_isr_pluggable_wq_handler);
	} else {
		if (dptx->dptx_pluggable_wq != NULL) {
			destroy_workqueue(dptx->dptx_pluggable_wq);
			dptx->dptx_pluggable_wq = NULL;
		}
	}
	return 0;
}

static int32_t dp_probe(struct platform_device *pdev)
{
	struct dp_private *priv = NULL;
	struct dp_ctrl *dptx = NULL;

	if (unlikely(!pdev)) {
		dpu_pr_warn("pdev is null");
		return -1;
	}

	if (!is_connector_manager_available())
		return -EPROBE_DEFER;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		dpu_pr_err("[DP] alloc offline driver private data failed!\n");
		return -EINVAL;
	}

	priv->device = pdev;
	if (dp_panel_parse_dt(priv, pdev->dev.of_node)){
		dpu_pr_err("[DP] parse dts error!\n");
		devm_kfree(&pdev->dev, priv);
		return -EINVAL;
	}

	if (priv->connector_info.base.name == NULL) {
		dpu_pr_warn("[DP] get priv->connector_info.base.name from dtsi failed!");
		devm_kfree(&pdev->dev, priv);
		return -EINVAL;
	}

	/* this is the tail device */
	priv->connector_info.base.peri_device = pdev;
	priv->connector_info.base.type = PANEL_DP;
	priv->connector_info.base.pluggable_connect_mutex = devm_kzalloc(&pdev->dev, sizeof(struct mutex), GFP_KERNEL);
	if (!priv->connector_info.base.pluggable_connect_mutex) {
		dpu_pr_warn("[DP] alloc pluggable_connect_mutex failed!\n");
		devm_kfree(&pdev->dev, priv);
		return -EINVAL;
	}
	mutex_init(priv->connector_info.base.pluggable_connect_mutex);

	/* add private data to device */
	platform_set_drvdata(pdev, &priv->connector_info);

	/* will create chrdev: /dev/gfx_dpX  */
	if (register_connector(&priv->connector_info) != 0) {
		dpu_pr_warn("[DP] device register failed, need retry!\n");
		devm_kfree(&pdev->dev, priv->connector_info.base.pluggable_connect_mutex);
		devm_kfree(&pdev->dev, priv);
		return -EPROBE_DEFER;
	}

	dptx = &priv->dp[MASTER_DPTX_IDX];
	// register dp hpd devive
	if (switch_dev_register(&dptx->sdev) < 0)
		dpu_pr_err("[DP] dp switch register [%s] failed!", dptx->sdev.name);
	// register hdmi_audio hpd devive
	if (switch_dev_register(&dptx->dp_switch) < 0)
		dpu_pr_err("[DP] dp switch register [%s] failed!", dptx->dp_switch.name);

	/* save dp_private to list */
	dp_save_device(pdev, priv);

	/* set dp power saving mode */
	if (dptx->dptx_combophy_power_saving_handler)
		dptx->dptx_combophy_power_saving_handler(dptx);

	if (dptx->connector->conn_info->base.mode == EDP_MODE) {
		dpu_pr_info("[DP] enable_irq priv->hpd_irq_no: %d", priv->hpd_irq_no);
		enable_irq(priv->hpd_irq_no);
		if (dptx_pluggable_wq_handler(dptx, true)) {
			dpu_pr_err("[DP] dptx retraining workqueue failed!\n");
			return -EINVAL;
		}
		if (dptx->dptx_pluggable_wq != NULL)
			queue_work(dptx->dptx_pluggable_wq, &dptx->dptx_pluggable_work);
	}

	return 0;
}

/**
 * Clear resource when device removed but not for devicetree device
 */
static int32_t dp_remove(struct platform_device *pdev)
{
	struct dp_private *priv = platform_get_drvdata(pdev);
	struct dp_ctrl *dptx = NULL;

	if (!priv) {
		dpu_pr_err("[DP] get dsi private data failed!\n");
		return -EINVAL;
	}
	if (dptx->connector->conn_info->base.mode == EDP_MODE) {
		dpu_pr_info("[DP] dp_remove priv->hpd_gpio: %d, priv->hpd_irq_no: %d",
			priv->hpd_gpio, priv->hpd_irq_no);
		gpio_free(priv->hpd_gpio);
		free_irq(priv->hpd_irq_no, NULL);
		if (dptx_pluggable_wq_handler(dptx, false) != 0)
			dpu_pr_err("[DP] dptx rm wq failed!\n");
	}

	unregister_connector(&priv->connector_info);

	dptx = &priv->dp[MASTER_DPTX_IDX];
	// unregister dp hpd devive
	switch_dev_unregister(&dptx->sdev);
	// unregister hdmi_audio hpd devive
	switch_dev_unregister(&dptx->dp_switch);

	dpu_pr_info("[DP] %s remove complete!\n", priv->connector_info.base.name);

	return 0;
}

static const struct dev_pm_ops dp_dev_pm_ops = {
#ifdef CONFIG_PM_SLEEP
	.suspend = dp_pm_suspend,
	.resume = NULL,
#endif
};

static struct platform_driver dp_platform_driver = {
	.probe  = dp_probe,
	.remove = dp_remove,
	.driver = {
		.name  = DEV_NAME_DP,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(dp_device_match_table),
		.pm = &dp_dev_pm_ops,
	}
};

static int32_t __init dp_register(void)
{
	return platform_driver_register(&dp_platform_driver);
}

static void __exit dp_unregister(void)
{
	platform_driver_unregister(&dp_platform_driver);
}

late_initcall(dp_register);
module_exit(dp_unregister);

MODULE_AUTHOR("Graphics Display");
MODULE_DESCRIPTION("DP Module Driver");
MODULE_LICENSE("GPL");
