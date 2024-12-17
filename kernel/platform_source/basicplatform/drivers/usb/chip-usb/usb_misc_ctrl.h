/*
 * Copyright (C) 2023 Hisilicon
 * Author: Hisillicon <>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _LOCAL_USB_MISC_CTRL_H_
#define _LOCAL_USB_MISC_CTRL_H_
#include <linux/mutex.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/platform_drivers/usb/usb_reg_cfg.h>

#define MAX_CLKS	5

#define misc_ctrl_info(format, arg...) \
	pr_info("[USB_MISC_CTRL][I][%s]"format, __func__, ##arg)

#define misc_ctrl_err(format, arg...) \
	pr_err("[USB_MISC_CTRL][E][%s]"format, __func__, ##arg)

struct chip_usb_misc_ctrl {
	struct device *dev;
	struct list_head list;
	struct fwnode_handle *fwnode;
	struct clk **clks;
	int num_clocks;
	int init_count;
	int refcnt;
	struct mutex usb_misc_ctrl_lock;
	struct chip_usb_reg_cfg *misc_ctrl_reset;
	struct chip_usb_reg_cfg *misc_ctrl_unreset;
	struct chip_usb_reg_cfg *misc_ctrl_is_unreset;
	struct chip_usb_reg_cfg *exit_noc_power_idle;
	struct chip_usb_reg_cfg *exit_noc_power_idleack;
	struct chip_usb_reg_cfg *exit_noc_power_idlestate;
	struct chip_usb_reg_cfg *enter_noc_power_idle;
	struct chip_usb_reg_cfg *enter_noc_power_idleack;
	struct chip_usb_reg_cfg *enter_noc_power_idlestate;
	struct chip_usb_reg_cfg **init_regcfgs;
	struct chip_usb_reg_cfg *usbc_mem_power_on;
	struct chip_usb_reg_cfg *usbc_mem_poweron_state;
	struct chip_usb_reg_cfg *usbc_mem_power_down;
	struct chip_usb_reg_cfg *usbc_mem_powerdown_state;
	int num_init_regcfg;
	struct chip_usb_reg_cfg *config_phy_interface;
	struct chip_usb_reg_cfg *apb_bridge_reset;
	struct chip_usb_reg_cfg *apb_bridge_unreset;
	struct chip_usb_reg_cfg *usbdp_clk_open;
	struct chip_usb_reg_cfg *usb_sysctrl_reset;
	struct chip_usb_reg_cfg *usb_sysctrl_unreset;
	struct chip_usb_reg_cfg *sp_memory_need_config;
	struct chip_usb_reg_cfg **sp_memory_config;
	int num_sp_memory_config;
	struct chip_usb_reg_cfg *tp_memory_need_config;
	struct chip_usb_reg_cfg **tp_memory_config;
	int num_tp_memory_config;
	struct chip_usb_reg_cfg *controller_select_cfg;
	struct usb_smmu *smmu;
};

#endif
