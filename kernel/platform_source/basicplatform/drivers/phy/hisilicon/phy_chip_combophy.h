/*
 * phy-chip-combophy.h
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
 * Create:2022-10-15
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

#ifndef __PHY_CHIP_COMBOPHY_H__
#define __PHY_CHIP_COMBOPHY_H__

#include <linux/clk.h>
#include <linux/dcache.h>
#include <linux/device.h>
#include <linux/platform_drivers/usb/hiusb_phy.h>

struct phy_priv {
	struct device *dev;
	struct hiusb_usb3phy combophy;
	/* phy for dwc3 driver */
	struct phy *phy;
	TCPC_MUX_CTRL_TYPE phy_mode;
	TYPEC_PLUG_ORIEN_E typec_orien;
	struct clk *apb_clk;
	struct clk *aux_clk;
	struct clk *mcubus_clk;
	void __iomem *usb_tca;
	void __iomem *usb_misc_ctrl;
	void __iomem *usb_dp_ctrl;
	struct dentry *debug_dir;
	/* phy parameter */
	uint32_t txdrvctrl;
	uint32_t txeqcoeff;
	uint32_t rxctrl0;
	uint32_t rxctrl;
	uint32_t rxeqactrl;
	uint32_t rxgsactrl;
	uint32_t rxlosctrl;
	uint32_t pll0ckgctrlr0;
	uint32_t termctrl;
	uint32_t pll0sscgcntr0;
	uint32_t txvcmctrl;

	bool phy_running;
	bool start_mcu;
	bool debug;
	bool test_stub;
	bool config_ssc;
	bool config_quirk_device;
	bool is_combophy_v2;
	bool is_hiusbc;

	struct notifier_block usb_nb;
	struct dentry *regdump_file;
	struct dentry *eye_diagram_symbol_file;
	struct dentry *eye_diagram_number_file;
	struct dentry *eye_diagram_data_file;

	struct chip_usb_reg_cfg *pre_unreset;
	struct chip_usb_reg_cfg *post_unreset;
	struct chip_usb_reg_cfg *pre_reset;
	struct chip_usb_reg_cfg *post_reset;
};

#endif /* __PHY_CHIP_COMBOPHY_H__ */
