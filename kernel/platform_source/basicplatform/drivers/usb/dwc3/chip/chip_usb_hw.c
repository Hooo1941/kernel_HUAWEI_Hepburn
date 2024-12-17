/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Hardware config for USB.
 * Create: 2019-6-16
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */

#include "chip_usb_hw.h"
#include <securec.h>
#include <linux/clk.h>
#include <linux/platform_drivers/usb/chip_usb_helper.h>
#include <linux/platform_drivers/usb/usb_reg_cfg.h>
#include <linux/platform_drivers/usb/tca.h>
#include <linux/platform_drivers/usb/dwc3_usb_interface.h>
#include <linux/of_platform.h>
#include <linux/arm-smccc.h>
#include <linux/gpio.h>
#include <platform_include/see/bl31_smc.h>
#ifdef CONFIG_PLATDRV_SUPPORT_FFA
#include <platform_include/see/ffa/ffa_plat_drv.h>
#endif
#include "chip_usb_bc12.h"
#include "combophy_core.h"

#define USBC_NUM_MAX 20 /* usb controller num limit */

void chip_usb3_phy_dump_info(struct chip_dwc3_device *chip_dwc)
{
	usb_info("+\n");
	if (!chip_dwc || !chip_dwc->usb3_phy)
		return;

	combophy_regdump(chip_dwc->usb3_phy);
	usb_info("-\n");
}

int chip_usb3_phy_init(struct phy *phy,
		       enum tcpc_mux_ctrl_type mode_type,
		       enum typec_plug_orien_e typec_orien)
{
	int ret;

	/* usb3 phy is optional */
	if (!phy)
		return 0;

	usb_info("+\n");
	ret = combophy_set_mode(phy, mode_type, typec_orien);
	if (ret) {
		usb_err("combophy set mode failed\n");
		return ret;
	}

	ret = combophy_init(phy);
	if (ret) {
		usb_err("combophy init failed\n");
		return ret;
	}
	usb_info("-\n");

	return 0;
}

int chip_usb3_phy_exit(struct phy *phy)
{
	int ret;

	/* usb3 phy is optional */
	if (!phy)
		return 0;

	usb_info("+\n");
	ret = combophy_exit(phy);
	if (ret) {
		usb_err("combophy exit failed\n");
		return ret;
	}

	ret = combophy_set_mode(phy, TCPC_NC, TYPEC_ORIEN_POSITIVE);
	if (ret) {
		usb_err("combophy set mode failed\n");
		return ret;
	}

	usb_info("-\n");
	return 0;
}

int chip_usb2_phy_init(struct phy *usb2_phy, bool host_mode)
{
	int ret;

	if (!usb2_phy)
		return -EINVAL;

	usb_info("+\n");
	if (usb2_phy->power_count == 0) {
		ret = phy_power_on(usb2_phy);
		if (ret) {
			usb_err("usb2 phy poweron failed\n");
			return ret;
		}
	}

	if (host_mode)
		ret = phy_set_mode(usb2_phy, PHY_MODE_USB_HOST);
	else
		ret = phy_set_mode(usb2_phy, PHY_MODE_USB_DEVICE);
	if (ret) {
		usb_err("usb2 phy set mode failed\n");
		if (phy_power_off(usb2_phy))
			usb_err("usb2 phy poweroff failed\n");
		return ret;
	}

	ret = phy_init(usb2_phy);
	if (ret) {
		usb_err("usb2 phy init failed\n");
		if (phy_power_off(usb2_phy))
			usb_err("usb2 phy poweroff failed\n");
	}

	usb_info("-\n");
	return ret;
}

int chip_usb2_phy_exit(struct phy *usb2_phy)
{
	int ret;

	if (!usb2_phy)
		return -EINVAL;

	usb_info("+\n");
	ret = phy_exit(usb2_phy);
	if (ret) {
		usb_err("usb2 phy exit failed\n");
		return ret;
	}

	ret = phy_power_off(usb2_phy);
	if (ret) {
		usb_err("usb2 phy poweroff failed\n");
		if (phy_init(usb2_phy))
			usb_err("usb2 phy init failed\n");
	}

	usb_info("-\n");
	return ret;
}

static int chip_usb3_lite_phy_init(struct phy *phy)
{
	int ret;

	/* usb3 lite phy is optional */
	if (!phy)
		return 0;

	usb_info("+\n");
	ret = phy_init(phy);
	if (ret) {
		usb_err("usb3 lite phy init failed\n");
		return ret;
	}

	usb_info("-\n");
	return 0;
}

static int chip_usb3_lite_phy_exit(struct phy *phy)
{
	int ret;

	/* usb3 lite phy is optional */
	if (!phy)
		return 0;

	usb_info("+\n");
	ret = phy_exit(phy);
	if (ret) {
		usb_err("usb3 lite phy exit failed\n");
		return ret;
	}

	usb_info("-\n");
	return 0;
}

static int chip_usb3_tzpc_config(void)
{
#ifdef CONFIG_PLATDRV_SUPPORT_FFA
	struct ffa_send_direct_data args = {
		.data0 = FID_USB_TZPC,
		.data1 = 0,
		.data2 = 0,
		.data3 = 0,
		.data4 = 0,
	};
	return ffa_platdrv_send_msg(&args);
#else
	struct arm_smccc_res res;

	usb_info("tzpc_config start\n");
	arm_smccc_smc((u64)FID_USB_TZPC, 0, 0, 0, 0, 0, 0, 0, &res);
	return res.a0;
#endif
}

static int chip_usb_get_controller_clks(struct device_node *node,
			struct usb_controller *c)
{
	int ret;

	if (c->stub_clk)
		return 0;

	ret = chip_usb_get_clks(node, &c->clks, &c->num_clks);
	if (ret)
		return ret;

	ret = chip_usb_get_clk_freq(node, &c->clk_freq_cfg, c->num_clks);
	if (ret)
		return ret;

	return 0;
}

static void chip_usb_put_controller_clks(struct usb_controller *c)
{
	if (c->stub_clk)
		return;

	chip_usb_put_clk_freq(c->clk_freq_cfg, c->num_clks);
	chip_usb_put_clks(c->clks, c->num_clks);
}

static int chip_usb_get_controller_ops(struct device_node *node,
			struct usb_controller *c)
{
	c->reset = of_get_chip_usb_reg_cfg(node, "reset");
	if (!c->reset) {
		usb_err("controller reset fail\n");
		goto exit;
	}

	c->unreset = of_get_chip_usb_reg_cfg(node, "unreset");
	if (!c->unreset) {
		usb_err("controller unreset fail\n");
		goto unreset_err;
	}

	c->sel_self = of_get_chip_usb_reg_cfg(node, "sel-self");
	if (!c->sel_self)
		usb_info("dont support sel-self\n");

	c->irq_enable = of_get_chip_usb_reg_cfg(node, "irq_enable");
	if (!c->irq_enable)
		usb_info("dont support irq_enable\n");

	c->irq_clr = of_get_chip_usb_reg_cfg(node, "irq_clr");
	if (!c->irq_clr)
		usb_info("dont support irq_clr\n");

	return 0;

unreset_err:
	of_remove_chip_usb_reg_cfg(c->reset);
exit:
	return -ENOENT;
}

static void chip_usb_put_controller_ops(struct usb_controller *c)
{
	of_remove_chip_usb_reg_cfg(c->reset);

	of_remove_chip_usb_reg_cfg(c->unreset);

	of_remove_chip_usb_reg_cfg(c->sel_self);
}

static int chip_usb_get_controller_property(
	struct chip_dwc3_device *chip_usb,
	struct device *dev)
{
	int ret = 0;
	u32 i;
	errno_t m_ret;
	struct property *prop = NULL;
	const char *name = NULL;
	struct device_node *child = NULL;
	struct of_device_id _match[2];
	struct of_device_id *match = &_match[0];

	m_ret = memset_s(_match, sizeof(_match), 0, sizeof(_match));
	if (m_ret != EOK)
		return -ENOMEM;

	i = 0;
	of_property_for_each_string(dev->of_node, "controller-list", prop, name) {
		chip_usb->usbc_sets[i] = devm_kmalloc(dev,
			sizeof(*chip_usb->usbc_sets[i]), GFP_KERNEL);
		if (!chip_usb->usbc_sets[i]) {
			ret = -ENOMEM;
			goto malloc_err;
		}

		m_ret = strncpy_s(match->compatible, sizeof(match->compatible),
			name, strlen(name));
		if (m_ret != EOK)
			goto get_clk_err;

		for_each_child_of_node(dev->of_node, child) {
			if (!of_match_node(match, child))
				continue;

			usb_info("get %s prop\n", match->compatible);
			chip_usb->usbc_sets[i]->node = child;
			chip_usb->usbc_sets[i]->stub_clk =
				of_property_read_bool(child, "stub-clk");

			ret = chip_usb_get_controller_clks(child,
				chip_usb->usbc_sets[i]);
			if (ret) {
				usb_err("get controller clks failed %d\n", ret);
				goto get_clk_err;
			}

			ret = chip_usb_get_controller_ops(child,
				chip_usb->usbc_sets[i]);
			if (ret) {
				usb_err("get controller ops failed %d\n", ret);
				goto get_ops_err;
			}
		}
		i++;
	}
	return ret;

get_ops_err:
	chip_usb_put_controller_clks(chip_usb->usbc_sets[i]);
get_clk_err:
	devm_kfree(dev, chip_usb->usbc_sets[i]);
	chip_usb->usbc_sets[i] = NULL;
malloc_err:
	if (i == 0)
		return ret;
	chip_usb->usbc_nums = i;
	return 0;
}

static int chip_usb_get_controller_res(struct chip_dwc3_device *chip_usb,
			struct device *dev)
{
	int ret;

	if (of_property_read_u32(dev->of_node, "usbc_idx", &chip_usb->usbc_idx))
		chip_usb->usbc_idx = 0;

	chip_usb->usbc_working_idx = chip_usb->usbc_idx;
	ret = of_property_count_strings(dev->of_node, "controller-list");
	if (ret <= 0) {
		usb_err("one usb controller at least\n");
		return -ENODEV;
	}
	chip_usb->usbc_nums = (u32)ret;
	if (chip_usb->usbc_nums >= USBC_NUM_MAX) {
		usb_err("usbc index err\n");
		chip_usb->usbc_nums = 0;
		return -EINVAL;
	}

	chip_usb->usbc_sets = devm_kmalloc(dev, sizeof(*chip_usb->usbc_sets) * chip_usb->usbc_nums,
		GFP_KERNEL);

	if (!chip_usb->usbc_sets) {
		usb_err("fail to malloc usbc_sets\n");
		return -ENOMEM;
	}

	ret =  chip_usb_get_controller_property(chip_usb, dev);
	if (ret) {
		usb_err("fail to get property\n");
		devm_kfree(dev, chip_usb->usbc_sets);
		chip_usb->usbc_sets = NULL;
		chip_usb->usbc_nums = 0;
	}
	return ret;
}

static int chip_usb_get_phy_res(struct chip_dwc3_device *chip_usb,
			struct device *dev)
{
	struct device_node *np = NULL;

	chip_usb->usb2_phy = devm_phy_get(dev, "usb2-phy");
	if (IS_ERR(chip_usb->usb2_phy)) {
		usb_err("no usb2 phy configured\n");
		return -EPROBE_DEFER;
	}

	np = of_get_child_by_name(dev->of_node, "combophy_func");
	if (np && of_device_is_available(np)) {
		chip_usb->usb3_phy = devm_phy_get(dev, "usb3-phy");
		if (IS_ERR(chip_usb->usb3_phy)) {
			usb_err("no usb3 phy configured\n");
			return -EPROBE_DEFER;
		}
	} else {
		chip_usb->usb3_phy = NULL;
	}
	if (np)
		of_node_put(np);

	chip_usb->usb3_lite_phy = devm_phy_get(dev, "usb3-lite-phy");
	if (IS_ERR(chip_usb->usb3_lite_phy))
		chip_usb->usb3_lite_phy = NULL;

	return 0;
}

int chip_usb_get_hw_res(struct chip_dwc3_device *chip_usb, struct device *dev)
{
	int ret;

	if (!chip_usb || !dev)
		return -EINVAL;

	ret = chip_usb_get_phy_res(chip_usb, dev);
	if (ret)
		return ret;

	return  chip_usb_get_controller_res(chip_usb, dev);
}

static void chip_usb_put_controller_res(struct chip_dwc3_device *chip_usb)
{
	u32 i;

	for (i = 0; i < chip_usb->usbc_nums; i++) {
		if (!chip_usb->usbc_sets[i])
			continue;

		chip_usb_put_controller_ops(chip_usb->usbc_sets[i]);

		chip_usb_put_controller_clks(chip_usb->usbc_sets[i]);

		devm_kfree(&chip_usb->pdev->dev, chip_usb->usbc_sets[i]);
		chip_usb->usbc_sets[i] = NULL;
	}
	devm_kfree(&chip_usb->pdev->dev, chip_usb->usbc_sets);
	chip_usb->usbc_sets = NULL;
}

void chip_usb_put_hw_res(struct chip_dwc3_device *chip_usb)
{
	chip_usb_put_controller_res(chip_usb);
}

int chip_usb_register_hw_debugfs(struct chip_dwc3_device *chip_usb)
{
	if (!chip_usb)
		return -EINVAL;

	if (!chip_usb->debug_root)
		return 0;

	if (chip_usb->usb3_phy)
		return combophy_register_debugfs(chip_usb->usb3_phy,
			chip_usb->debug_root);

	return 0;
}

enum chip_charger_type chip_usb_detect_charger_type(
		struct chip_dwc3_device *chip_dwc3)
{
	struct hiusb_usb2phy *hiusb_usb2phy = NULL;

	if (!chip_dwc3)
		return CHARGER_TYPE_NONE;

	if (chip_dwc3->fpga_flag) {
		usb_dbg("this is fpga platform, charger is SDP\n");
		return CHARGER_TYPE_SDP;
	}

	if (chip_dwc3->fake_charger_type != CHARGER_TYPE_NONE) {
		usb_dbg("fake type: %d\n", chip_dwc3->fake_charger_type);
		return chip_dwc3->fake_charger_type;
	}

	/* In PC or CDC, usb connect to the standard downstream port,
	 * bc detect is not required, just return CHARGER_TYPE_SDP */
	if (chip_dwc3->disable_bc_detect) {
		usb_info("disable bc12 detect\n");
		return CHARGER_TYPE_SDP;
	}

	if (chip_dwc3->use_new_frame) {
		if (!chip_dwc3->usb2_phy)
			return CHARGER_TYPE_NONE;

		hiusb_usb2phy = (struct hiusb_usb2phy *)
			phy_get_drvdata(chip_dwc3->usb2_phy);
		if (!hiusb_usb2phy || !hiusb_usb2phy->detect_charger_type)
			return CHARGER_TYPE_NONE;

		return hiusb_usb2phy->detect_charger_type(hiusb_usb2phy);
	} else {
		return detect_charger_type(chip_dwc3);
	}
}

void chip_usb_dpdm_pulldown(struct chip_dwc3_device *chip_dwc3)
{
	struct hiusb_usb2phy *hiusb_usb2phy = NULL;

	usb_dbg("+\n");
	if (!chip_dwc3)
		return;

	if (chip_dwc3->use_new_frame) {
		if (!chip_dwc3->usb2_phy)
			return;

		hiusb_usb2phy = (struct hiusb_usb2phy *)
			phy_get_drvdata(chip_dwc3->usb2_phy);
		if (!hiusb_usb2phy || !hiusb_usb2phy->set_dpdm_pulldown)
			return;

		hiusb_usb2phy->set_dpdm_pulldown(hiusb_usb2phy, true);
	} else {
		chip_bc_dplus_pulldown(chip_dwc3);
	}
	usb_dbg("-\n");
}

void chip_usb_dpdm_pullup(struct chip_dwc3_device *chip_dwc3)
{
	struct hiusb_usb2phy *hiusb_usb2phy = NULL;

	usb_dbg("+\n");
	if (!chip_dwc3)
		return;

	if (chip_dwc3->use_new_frame) {
		if (!chip_dwc3->usb2_phy)
			return;

		hiusb_usb2phy = (struct hiusb_usb2phy *)
			phy_get_drvdata(chip_dwc3->usb2_phy);
		if (!hiusb_usb2phy || !hiusb_usb2phy->set_dpdm_pulldown)
			return;

		hiusb_usb2phy->set_dpdm_pulldown(hiusb_usb2phy, false);
	} else {
		chip_bc_dplus_pullup(chip_dwc3);
	}
	usb_dbg("-\n");
}

void chip_usb_disable_vdp_src(struct chip_dwc3_device *chip_dwc3)
{
	usb_dbg("+\n");

	if (!chip_dwc3)
		return;

	if (chip_dwc3->use_new_frame) {
		struct hiusb_usb2phy *hiusb_usb2phy = NULL;

		if (!chip_dwc3->usb2_phy)
			return;

		hiusb_usb2phy = (struct hiusb_usb2phy *)
			phy_get_drvdata(chip_dwc3->usb2_phy);
		if (!hiusb_usb2phy || !hiusb_usb2phy->set_vdp_src)
			return;

		hiusb_usb2phy->set_vdp_src(hiusb_usb2phy, false);

		usb_dbg("-\n");
	} else {
		unsigned long flags;

		spin_lock_irqsave(&chip_dwc3->bc_again_lock, flags);
		chip_bc_disable_vdp_src(chip_dwc3);
		spin_unlock_irqrestore(&chip_dwc3->bc_again_lock, flags);
	}
}

void chip_usb_enable_vdp_src(struct chip_dwc3_device *chip_dwc3)
{
	usb_dbg("+\n");

	if (!chip_dwc3)
		return;

	if (chip_dwc3->use_new_frame) {
		struct hiusb_usb2phy *hiusb_usb2phy = NULL;

		if (!chip_dwc3->usb2_phy)
			return;

		hiusb_usb2phy = (struct hiusb_usb2phy *)
			phy_get_drvdata(chip_dwc3->usb2_phy);
		if (!hiusb_usb2phy || !hiusb_usb2phy->set_vdp_src)
			return;

		hiusb_usb2phy->set_vdp_src(hiusb_usb2phy, true);

		usb_dbg("-\n");
	} else {
		unsigned long flags;

		spin_lock_irqsave(&chip_dwc3->bc_again_lock, flags);
		chip_bc_enable_vdp_src(chip_dwc3);
		spin_unlock_irqrestore(&chip_dwc3->bc_again_lock, flags);
	}
}

static int usb_controller_init(struct usb_controller *usbc)
{
	int ret;

	if (usbc->sel_self) {
		ret = chip_usb_reg_write(usbc->sel_self);
		if (ret) {
			usb_err("sel self failed %d\n", ret);
			return ret;
		}
	}

	if (!usbc->stub_clk) {
		ret = chip_usb_init_clks(usbc->clks, usbc->num_clks);
		if (ret) {
			usb_err("clks init failed %d\n", ret);
			return ret;
		}
	}

	if (usbc->irq_enable) {
		ret = chip_usb_reg_write(usbc->irq_enable);
		if (ret) {
			usb_err("irq_enable failed %d\n", ret);
			return ret;
		}
	}

	if (usbc->unreset) {
		ret = chip_usb_reg_write(usbc->unreset);
		if (ret) {
			usb_err("unreset failed %d\n", ret);
			return ret;
		}
	}
	return 0;
}

static int usb_controller_exit(struct usb_controller *usbc)
{
	int ret;

	if (usbc->reset) {
		ret = chip_usb_reg_write(usbc->reset);
		if (ret) {
			usb_err("reset fail %d\n", ret);
			return ret;
		}
	}

	if (!usbc->stub_clk)
		chip_usb_shutdown_clks(usbc->clks, usbc->num_clks);

	return 0;
}

int chip_usb_controller_init(struct chip_dwc3_device *chip_dwc3)
{
	int ret;

	if (!chip_dwc3)
		return -EINVAL;

	if (!chip_dwc3->usbc_sets) {
		usb_err("usbc sets null\n");
		return -EINVAL;
	}

	if (chip_dwc3->usbc_nums <= chip_dwc3->usbc_idx) {
		usb_err("usbc index err\n");
		return -EINVAL;
	}

	chip_dwc3->usbc_working_idx = chip_dwc3->usbc_idx;
	ret = usb_controller_init(chip_dwc3->usbc_sets[chip_dwc3->usbc_idx]);
	if (ret) {
		usb_err("controller init fail %d\n", ret);
		return ret;
	}
	/* require to delay 10ms */
	mdelay(10);

	return 0;
}

int chip_usb_controller_exit(struct chip_dwc3_device *chip_dwc3)
{
	if (!chip_dwc3)
		return -EINVAL;

	if (!chip_dwc3->usbc_sets) {
		usb_err("usbc sets null\n");
		return -EINVAL;
	}

	if (chip_dwc3->usbc_nums <= chip_dwc3->usbc_working_idx) {
		usb_err("usbc index err\n");
		return -EINVAL;
	}

	return usb_controller_exit(chip_dwc3->usbc_sets[chip_dwc3->usbc_working_idx]);
}

void usb_controller_irq_clr(void)
{
	if (chip_dwc3_dev
		&& (chip_dwc3_dev->usbc_sets)
		&& (chip_dwc3_dev->usbc_idx < chip_dwc3_dev->usbc_nums)
		&& (chip_dwc3_dev->usbc_sets[chip_dwc3_dev->usbc_idx])
		&& (chip_dwc3_dev->usbc_sets[chip_dwc3_dev->usbc_idx]->irq_clr))
		(void)chip_usb_reg_write(chip_dwc3_dev->usbc_sets[chip_dwc3_dev->usbc_idx]->irq_clr);
}
EXPORT_SYMBOL_GPL(usb_controller_irq_clr);

void usb_controller_force_unsec(void)
{
	if (chip_dwc3_dev
		&& (!chip_dwc3_dev->use_new_frame)
		&& chip_dwc3_dev->usb_tzpc_config)
		if (chip_usb3_tzpc_config())
			usb_info("tzpc_config failed\n");
}
EXPORT_SYMBOL_GPL(usb_controller_force_unsec);

/* multi controller */
int chip_usb_controller_probe(struct chip_dwc3_device *chip_dwc3)
{
	struct platform_device *pdev = NULL;
	struct device *parent = NULL;
	struct device_node *child = NULL;

	if (!chip_dwc3)
		return -ENAVAIL;

	if (!chip_dwc3->usbc_sets) {
		usb_err("usbc sets null\n");
		return -EINVAL;
	}

	if (chip_dwc3->usbc_nums <= chip_dwc3->usbc_idx) {
		usb_err("usbc index err\n");
		return -EINVAL;
	}

	parent = &chip_dwc3->pdev->dev;
	child = chip_dwc3->usbc_sets[chip_dwc3->usbc_idx]->node;
	if (!child) {
		usb_err("usbc node err\n");
		return -EINVAL;
	}

	pdev = of_platform_device_create(child, NULL, parent);
	if (!pdev) {
		usb_err("create usbc dev failed!\n");
		return -ENODEV;
	}
	chip_dwc3->usbc_sets[chip_dwc3->usbc_idx]->pdev = pdev;
	return 0;
}

int chip_usb_controller_destroy(struct chip_dwc3_device *chip_dwc3)
{
	int ret;
	struct device *parent = NULL;
	struct platform_device *child = NULL;

	if (!chip_dwc3)
		return -ENAVAIL;

	if (!chip_dwc3->usbc_sets) {
		usb_err("usbc sets null\n");
		return -EINVAL;
	}

	if (chip_dwc3->usbc_nums <= chip_dwc3->usbc_working_idx) {
		usb_err("usbc index err\n");
		return -EINVAL;
	}

	parent = &chip_dwc3->pdev->dev;
	child = chip_dwc3->usbc_sets[chip_dwc3->usbc_working_idx]->pdev;
	if (!child) {
		usb_err("usbc index err\n");
		return -EINVAL;
	}

	ret = of_platform_device_destroy(&child->dev, NULL);
	if (ret)
		return ret;

	chip_dwc3->usbc_sets[chip_dwc3->usbc_working_idx]->pdev = NULL;
	return 0;
}

void chip_usb_unreset_phy_if_fpga(void)
{
	unsigned int gpio;

	if (!chip_dwc3_dev || chip_dwc3_dev->fpga_phy_reset_gpio < 0)
		return;

	gpio = (unsigned int)chip_dwc3_dev->fpga_phy_reset_gpio;

	gpio_direction_output(gpio, 1);
	mdelay(10); /* mdelay time */

	gpio_direction_output(gpio, 0);
	mdelay(10); /* mdelay time */
}

static int usb_controller_clk_freq_cfg(struct chip_dwc3_device *chip_dwc3, enum usb_clk_freq_select select)
{
	struct usb_controller *usbc = NULL;
	unsigned int freq[2] = {0};
	int ret, i;

	if (!chip_dwc3)
		return -EINVAL;

	if (!chip_dwc3->usbc_sets) {
		usb_err("usbc sets null\n");
		return -EINVAL;
	}

	if (chip_dwc3->usbc_nums <= chip_dwc3->usbc_idx) {
		usb_err("usbc index err\n");
		return -EINVAL;
	}

	usbc = chip_dwc3->usbc_sets[chip_dwc3->usbc_idx];
	if (usbc->stub_clk)
		return 0;

	for (i = 0; i < usbc->num_clks; i++) {
		if (usbc->clk_freq_cfg && usbc->clk_freq_cfg[i].clk) {
			freq[0] = usbc->clk_freq_cfg[i].high_freq;
			freq[1] = usbc->clk_freq_cfg[i].low_freq;
			ret = clk_set_rate(usbc->clk_freq_cfg[i].clk, freq[select]);
			if (ret) {
				usb_err("[clk-index:%d]Failed to set clk rate\n", i);
				return ret;
			}
		}
	}

	return 0;
}

int dwc3_phy_init(struct chip_dwc3_device *chip_dwc,
		bool host_mode)
{
	unsigned int eye_diagram_param;
	int ret = -ENODEV;

	if (chip_dwc->use_new_frame) {
		chip_usb_unreset_phy_if_fpga();

		if (chip_dwc->usb_tzpc_config)
			if (chip_usb3_tzpc_config()) {
				usb_info("tzpc_config failed\n");
				return -ESRCH;
			}

		ret = chip_usb2_phy_init(chip_dwc->usb2_phy, host_mode);
		if (ret) {
			usb_err("usb2_phy_init failed\n");
			return ret;
		}

		ret = chip_usb3_phy_init(chip_dwc->usb3_phy,
					 chip_dwc->mode_type,
					 chip_dwc->plug_orien);
		if (ret) {
			usb_err("usb3_phy_init failed\n");
			if (chip_usb2_phy_exit(chip_dwc->usb2_phy))
				usb_err("usb2_phy_exit failed\n");
			return ret;
		}

		ret = chip_usb3_lite_phy_init(chip_dwc->usb3_lite_phy);
		if (ret) {
			usb_err("usb3_lite_phy_init failed\n");
			return ret;
		}

		ret = usb_controller_clk_freq_cfg(chip_dwc, HIGH_FREQUENCY);
		if (ret) {
			usb_err("usb_controller_clk_freq_cfg failed\n");
			if (chip_usb2_phy_exit(chip_dwc->usb2_phy))
				usb_err("usb2_phy_exit failed\n");
			if (chip_usb3_phy_exit(chip_dwc->usb3_phy))
				usb_err("usb3_phy_exit failed\n");
			return ret;
		}

		ret = chip_usb_controller_init(chip_dwc);
		if (ret) {
			usb_err("usb_controller_init failed\n");
			if (chip_usb2_phy_exit(chip_dwc->usb2_phy))
				usb_err("usb2_phy_exit failed\n");
			if (chip_usb3_phy_exit(chip_dwc->usb3_phy))
				usb_err("usb3_phy_exit failed\n");
			return ret;
		}

		set_chip_dwc3_power_flag(USB_POWER_ON);

		/* dwc3 core_ops should be called after power flag set */
		if (chip_dwc->usb3_phy && chip_dwc->mode_type == TCPC_DP)
			dwc3_core_disable_pipe_clock();
	} else if (chip_dwc->usb_phy->init) {
		if (host_mode)
			eye_diagram_param = chip_dwc->eye_diagram_host_param;
		else
			eye_diagram_param = chip_dwc->eye_diagram_param;

		return chip_dwc->usb_phy->init(chip_dwc->support_dp,
				eye_diagram_param,
				chip_dwc->usb3_phy_tx_vboost_lvl);
	}

	return ret;
}

int dwc3_phy_shutdown(struct chip_dwc3_device *chip_dwc)
{
	int ret = -ENODEV;

	if (chip_dwc->use_new_frame) {
		set_chip_dwc3_power_flag(USB_POWER_HOLD);
		ret = chip_usb_controller_exit(chip_dwc);
		if (ret)
			usb_err("usb_controller_exit failed\n");

		ret = chip_usb2_phy_exit(chip_dwc->usb2_phy);
		if (ret)
			usb_err("usb2_phy_exit failed\n");

		ret = chip_usb3_phy_exit(chip_dwc->usb3_phy);
		if (ret)
			usb_err("usb3_phy_exit failed\n");

		ret = chip_usb3_lite_phy_exit(chip_dwc->usb3_lite_phy);
		if (ret)
			usb_err("usb3_lite_phy_exit failed\n");

		set_chip_dwc3_power_flag(USB_POWER_OFF);
	} else if (chip_dwc->usb_phy->shutdown) {
		return chip_dwc->usb_phy->shutdown(chip_dwc->support_dp);
	}
	return ret;
}

/* Currently this function only called in hifi usb mode */
int dwc3_usb20_phy_init(struct chip_dwc3_device *chip_dwc,
		unsigned int combophy_flag)
{
	int ret = 0;

	usb_dbg("+\n");
	if (chip_dwc->use_new_frame) {
		chip_usb_unreset_phy_if_fpga();

		ret = chip_usb2_phy_init(chip_dwc->usb2_phy, true);
		if (ret) {
			usb_err("usb2_phy_init failed\n");
			return ret;
		}

		ret = usb_controller_clk_freq_cfg(chip_dwc, LOW_FREQUENCY);
		if (ret) {
			usb_err("usb_controller_clk_freq_cfg failed\n");
			if (chip_usb2_phy_exit(chip_dwc->usb2_phy))
				usb_err("usb2_phy_exit failed\n");
			return ret;
		}

		ret = chip_usb_controller_init(chip_dwc);
		if (ret) {
			usb_err("usb_controller_init failed\n");
			if (chip_usb2_phy_exit(chip_dwc->usb2_phy))
				usb_err("usb2_phy_exit failed\n");
		}
	} else if (chip_dwc->usb_phy->shared_phy_init) {
		return chip_dwc->usb_phy->shared_phy_init(chip_dwc->support_dp,
				chip_dwc->eye_diagram_host_param,
				combophy_flag);
	} else {
		WARN_ON(1);
	}
	usb_dbg("-\n");
	return ret;
}

int dwc3_usb20_phy_shutdown(struct chip_dwc3_device *chip_dwc,
		unsigned int combophy_flag, unsigned int keep_power)
{
	int ret = 0;

	usb_dbg("+\n");
	if (chip_dwc->use_new_frame) {
		ret = chip_usb_controller_exit(chip_dwc);
		if (ret) {
			usb_err("usb_controller_exit failed\n");
			return ret;
		}
		if (keep_power) {
			ret = phy_power_on(chip_dwc->usb2_phy);
			if (ret)
				usb_err("usb2 phy poweron failed\n");
		}
		ret = chip_usb2_phy_exit(chip_dwc->usb2_phy);
		if (ret) {
			usb_err("usb2_phy_exit failed\n");
			if (chip_usb_controller_init(chip_dwc))
				usb_err("usb_controller_init failed\n");
		}
	} else if (chip_dwc->usb_phy->shared_phy_shutdown) {
		return chip_dwc->usb_phy->shared_phy_shutdown(chip_dwc->support_dp,
				combophy_flag, keep_power);
	} else {
		WARN_ON(1);
	}
	usb_dbg("-\n");
	return ret;
}

static int chip_usb2_phy_calibrate(struct phy *usb2_phy)
{
	int ret;

	if (!usb2_phy)
		return -EINVAL;

	ret = phy_calibrate(usb2_phy);
	if (ret)
		usb_err("usb2 phy calibrate failed\n");

	return ret;
}


/* Currently this function only called in hauwei_fs_earphone mode */
int dwc3_usb20_phy_calibrate(struct chip_dwc3_device *chip_dwc)
{
	int ret = 0;

	if (chip_dwc->use_new_frame) {
		ret = chip_usb2_phy_calibrate(chip_dwc->usb2_phy);
		if (ret)
			usb_err("usb2_phy_calibrate failed\n");
	}

	return ret;
}
