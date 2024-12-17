/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: usb misc ctrl ops
 * Create: 2019-09-30
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */

#include "usb_misc_ctrl.h"
#include "usb_smmu.h"
#include <linux/platform_drivers/usb/usb_reg_cfg.h>
#include <linux/platform_drivers/usb/chip_usb_helper.h>
#include <linux/platform_drivers/usb/usb_misc_ctrl.h>
#include <linux/clk.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/slab.h>

static DEFINE_SPINLOCK(g_sysctrl_lock);
static LIST_HEAD(g_sysctrl_list);
static void put_reg_cfgs(struct chip_usb_misc_ctrl *misc_ctrl);
static int get_clks(struct chip_usb_misc_ctrl *misc_ctrl);
static int get_reg_cfgs(struct chip_usb_misc_ctrl *misc_ctrl);

struct chip_usb_misc_ctrl *sysctrl_create(struct device *dev, struct fwnode_handle *fwnode)
{
	int ret;
	struct chip_usb_misc_ctrl *sysctrl = NULL;
	misc_ctrl_info("+\n");

	if (!fwnode||!dev)
		return ERR_PTR(-EINVAL);

	sysctrl = devm_kzalloc(dev, sizeof(*sysctrl), GFP_KERNEL);
	if (!sysctrl)
		return ERR_PTR(-ENOMEM);

	sysctrl->fwnode = fwnode;
	sysctrl->dev = dev;

	ret = get_clks(sysctrl);
	if (ret) {
		misc_ctrl_err("get clks failed\n");
		return ERR_PTR(ret);
	}

	ret = get_reg_cfgs(sysctrl);
	if (ret) {
		put_reg_cfgs(sysctrl);
		misc_ctrl_err("get reg cfgs failed\n");
		return ERR_PTR(ret);
	}

	ret = get_smmu_info(sysctrl);
	if (ret)
		misc_ctrl_info("not support smmu\n");

	misc_ctrl_info("-\n");

	return sysctrl;
}

static struct chip_usb_misc_ctrl *misc_ctrl_get(struct fwnode_handle *fwnode)
{
	struct chip_usb_misc_ctrl *iter = NULL;
	unsigned long flags;

	misc_ctrl_info("+");
	spin_lock_irqsave(&g_sysctrl_lock, flags);
	list_for_each_entry(iter, &g_sysctrl_list, list) {
		if (iter->fwnode == fwnode) {
			iter->refcnt++;
			spin_unlock_irqrestore(&g_sysctrl_lock, flags);
			return iter;
		}
	}
	spin_unlock_irqrestore(&g_sysctrl_lock, flags);
	misc_ctrl_info("-");
	return NULL;
}

static void misc_ctrl_put(struct chip_usb_misc_ctrl *misc_ctrl)
{
	misc_ctrl_info("+");
	misc_ctrl->refcnt--;
	misc_ctrl_info("-");
}

static struct chip_usb_misc_ctrl *misc_ctrl_find(struct fwnode_handle *fwnode)
{
	struct chip_usb_misc_ctrl *usb_misc_ctrl = NULL;

	usb_misc_ctrl = misc_ctrl_get(fwnode);
	if(!usb_misc_ctrl)
		return NULL;
	misc_ctrl_put(usb_misc_ctrl);

	return usb_misc_ctrl;
}

static int enter_noc_power_idle(struct chip_usb_misc_ctrl *misc_ctrl);

struct fwnode_handle *g_msic_ctrl_fwnode;

static int reset_misc_ctrl(struct chip_usb_misc_ctrl *misc_ctrl)
{
	int ret;

	ret = chip_usb_reg_write(misc_ctrl->misc_ctrl_reset);
	if (ret)
		misc_ctrl_err("config failed\n");

	return ret;
}

static int unreset_misc_ctrl(struct chip_usb_misc_ctrl *misc_ctrl)
{
	int ret;

	misc_ctrl_err("unreset_misc_ctrl+\n");

	ret = chip_usb_reg_write(misc_ctrl->misc_ctrl_unreset);
	if (ret)
		misc_ctrl_err("config failed\n");

	misc_ctrl_err("unreset_misc_ctrl-\n");

	return ret;
}

bool multi_usb_misc_ctrl_is_ready(struct fwnode_handle *fwnode)
{
	struct chip_usb_misc_ctrl *usb_misc_ctrl = NULL;
	if(!fwnode)
		return false;

	usb_misc_ctrl = misc_ctrl_get(fwnode);
	if(!usb_misc_ctrl)
		return false;
	misc_ctrl_put(usb_misc_ctrl);
	return true;
}
EXPORT_SYMBOL_GPL(multi_usb_misc_ctrl_is_ready);

bool misc_ctrl_is_ready(void)
{
#ifdef CONFIG_USB_HIUSB
	return true;
#endif
	if(g_msic_ctrl_fwnode == NULL)
		return false;
	else
		return multi_usb_misc_ctrl_is_ready(g_msic_ctrl_fwnode);
}
EXPORT_SYMBOL_GPL(misc_ctrl_is_ready);

bool misc_ctrl_is_unreset(struct chip_usb_misc_ctrl *usb_misc_ctrl)
{
	int ret;

	mutex_lock(&usb_misc_ctrl->usb_misc_ctrl_lock);

	ret = chip_usb_reg_test_cfg(usb_misc_ctrl->misc_ctrl_is_unreset);
	if (ret < 0)
		misc_ctrl_err("config failed\n");

	mutex_unlock(&usb_misc_ctrl->usb_misc_ctrl_lock);

	return ret < 0 ? false : ret;
}
EXPORT_SYMBOL_GPL(misc_ctrl_is_unreset);

static bool test_exit_noc_power_idle(struct chip_usb_misc_ctrl *misc_ctrl)
{
	bool ret = false;

	ret = chip_usb_reg_test_cfg(misc_ctrl->exit_noc_power_idleack) == 1;
	if (misc_ctrl->exit_noc_power_idlestate)
		return (ret && (chip_usb_reg_test_cfg(
			misc_ctrl->exit_noc_power_idlestate) == 1));

	return ret;
}

static int exit_noc_power_idle(struct chip_usb_misc_ctrl *misc_ctrl)
{
	if (misc_ctrl->exit_noc_power_idle) {
		int ret;
		int retrys = 10;

		ret =  chip_usb_reg_write(misc_ctrl->exit_noc_power_idle);
		if (ret < 0) {
			misc_ctrl_err("config failed\n");
			return ret;
		}

		while (retrys--) {
			if (test_exit_noc_power_idle(misc_ctrl))
				break;
			/* according to noc pw idle exit process */
			udelay(10);
		}

		if (retrys <= 0) {
			misc_ctrl_err("wait noc power idle state timeout\n");
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static bool test_usbc_mem_poweron_state(struct chip_usb_misc_ctrl *misc_ctrl)
{
	return chip_usb_reg_test_cfg(misc_ctrl->usbc_mem_poweron_state) == 1;
}

static int usbc_mem_power_on(struct chip_usb_misc_ctrl *misc_ctrl)
{
	if (misc_ctrl->usbc_mem_power_on) {
		int ret;
		int retrys = 100;

		ret =  chip_usb_reg_write(misc_ctrl->usbc_mem_power_on);
		if (ret < 0) {
			misc_ctrl_err("config failed\n");
			return ret;
		}

		while (retrys--) {
			if (test_usbc_mem_poweron_state(misc_ctrl))
				break;
			/* according to noc pw idle exit process */
			udelay(1);
		}

		if (retrys <= 0) {
			misc_ctrl_err("wait usbc mem poweron timeout\n");
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static bool test_usbc_mem_powerdown_state(struct chip_usb_misc_ctrl *misc_ctrl)
{
	return chip_usb_reg_test_cfg(misc_ctrl->usbc_mem_powerdown_state) == 1;
}

static int usbc_mem_power_down(struct chip_usb_misc_ctrl *misc_ctrl)
{
	if (misc_ctrl->usbc_mem_power_down) {
		int ret;
		int retrys = 100;

		ret =  chip_usb_reg_write(misc_ctrl->usbc_mem_power_down);
		if (ret < 0) {
			misc_ctrl_err("config failed\n");
			return ret;
		}

		while (retrys--) {
			if (test_usbc_mem_powerdown_state(misc_ctrl))
				break;
			/* according to noc pw idle exit process */
			udelay(1);
		}

		if (retrys <= 0) {
			misc_ctrl_err("wait usbc mem powerdown timeout\n");
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int usb_misc_ctrl_init_cfg(struct chip_usb_misc_ctrl *misc_ctrl)
{
	if (misc_ctrl->init_regcfgs)
		return chip_usb_reg_write_array(misc_ctrl->init_regcfgs,
					       misc_ctrl->num_init_regcfg);

	return 0;
}

static bool usb_sp_memory_need_config(struct chip_usb_misc_ctrl *usb_misc_ctrl)
{
	bool ret = false;

	ret = chip_usb_reg_test_cfg(usb_misc_ctrl->sp_memory_need_config) == 1;

	return ret;
}

static int usb_sp_memory_config(struct chip_usb_misc_ctrl *misc_ctrl)
{
	if (usb_sp_memory_need_config(misc_ctrl) && misc_ctrl->sp_memory_config)
		return chip_usb_reg_write_array(misc_ctrl->sp_memory_config,
					       misc_ctrl->num_sp_memory_config);

	return 0;
}

static bool usb_tp_memory_need_config(struct chip_usb_misc_ctrl *usb_misc_ctrl)
{
	bool ret = false;

	ret = chip_usb_reg_test_cfg(usb_misc_ctrl->tp_memory_need_config) == 1;

	return ret;
}

static int usb_tp_memory_config(struct chip_usb_misc_ctrl *misc_ctrl)
{
	if (usb_tp_memory_need_config(misc_ctrl) && misc_ctrl->tp_memory_config)
		return chip_usb_reg_write_array(misc_ctrl->tp_memory_config,
					       misc_ctrl->num_tp_memory_config);

	return 0;
}

static int usb2_phy_interface_cfg(struct chip_usb_misc_ctrl *misc_ctrl)
{
	int ret = 0;

	if (misc_ctrl->config_phy_interface) {
		ret = chip_usb_reg_write(misc_ctrl->config_phy_interface);
		if (ret) {
			misc_ctrl_err("config phy interface_cfg failed\n");
			return ret;
		}
	}

	return ret;
}

static int unreset_usb_apb_ctrl(struct chip_usb_misc_ctrl *misc_ctrl)
{
	int ret = 0;

	if (misc_ctrl->apb_bridge_unreset) {
		misc_ctrl_err("config apb_bridge_unreset\n");
		ret = chip_usb_reg_write(misc_ctrl->apb_bridge_unreset);
		if (ret)
			misc_ctrl_err("config apb_bridge_unreset failed\n");
	}

	return ret;
}

static int unreset_usb_sysctrl(struct chip_usb_misc_ctrl *misc_ctrl)
{
	int ret = 0;

	if (misc_ctrl->usb_sysctrl_unreset) {
		misc_ctrl_err("config usb_sysctrl_unreset\n");
		ret = chip_usb_reg_write(misc_ctrl->usb_sysctrl_unreset);
		if (ret)
			misc_ctrl_err("config usb_sysctrl_unreset failed\n");
	}

	return ret;
}

static int reset_usb_apb_ctrl(struct chip_usb_misc_ctrl *misc_ctrl)
{
	int ret = 0;

	if (misc_ctrl->apb_bridge_reset) {
		misc_ctrl_err("config apb_bridge_reset\n");
		ret = chip_usb_reg_write(misc_ctrl->apb_bridge_reset);
		if (ret)
			misc_ctrl_err("config apb_bridge_reset failed\n");
	}

	return ret;
}

static int reset_usb_sysctrl(struct chip_usb_misc_ctrl *misc_ctrl)
{
	int ret = 0;

	if (misc_ctrl->usb_sysctrl_reset) {
		misc_ctrl_err("config usb_sysctrl_unreset\n");
		ret = chip_usb_reg_write(misc_ctrl->usb_sysctrl_reset);
		if (ret)
			misc_ctrl_err("config usb_sysctrl_reset failed\n");
	}

	return ret;
}

int multi_usb_misc_ctrl_init(struct fwnode_handle *fwnode)
{
	int ret;
	struct chip_usb_misc_ctrl *usb_misc_ctrl = NULL;
	misc_ctrl_info("+\n");

	if(!fwnode)
		return -EINVAL;
	usb_misc_ctrl = misc_ctrl_get(fwnode);
	if(!usb_misc_ctrl)
		return -ENODEV;

	mutex_lock(&usb_misc_ctrl->usb_misc_ctrl_lock);

	misc_ctrl_info("init_count %d\n", usb_misc_ctrl->init_count);
	if (usb_misc_ctrl->init_count > 0) {
		usb_misc_ctrl->init_count++;
		ret = 0;
		goto out;
	}

	ret = unreset_usb_apb_ctrl(usb_misc_ctrl);
	if (ret)
		goto out;

	ret = unreset_usb_sysctrl(usb_misc_ctrl);
	if (ret)
		goto usb_apb_reset;

	ret = chip_usb_init_clks(usb_misc_ctrl->clks,
				 usb_misc_ctrl->num_clocks);
	if (ret)
		goto usb_sysctrl_reset;

	ret = usbc_mem_power_on(usb_misc_ctrl);
	if (ret) {
		misc_ctrl_err("usbc mem power on failed ret %d\n", ret);
		goto shutdown_clks;
	}
	ret = usb_smmu_init(usb_misc_ctrl);
	if (ret)
		misc_ctrl_err("usb_smmu_init failed ret %d\n", ret);

	ret = exit_noc_power_idle(usb_misc_ctrl);
	if (ret) {
		misc_ctrl_err("exit_noc_power_idle failed ret %d\n", ret);
		goto usbc_power_down;
	}

	/* select hiusbc */
	if (usb_misc_ctrl->controller_select_cfg) {
		ret = chip_usb_reg_write(usb_misc_ctrl->controller_select_cfg);
		if (ret) {
			misc_ctrl_err("select controller  failed ret %d\n", ret);
			goto enter_power_idle;
		}
	}

	ret = usb2_phy_interface_cfg(usb_misc_ctrl);
	if (ret) {
		misc_ctrl_err("config usb2_phy_interface_cfg failed ret %d\n", ret);
		goto enter_power_idle;
	}

	ret = usb_misc_ctrl_init_cfg(usb_misc_ctrl);
	if (ret) {
		misc_ctrl_err("config init_regcfgs failed\n");
		goto enter_power_idle;
	}

	ret = usb_sp_memory_config(usb_misc_ctrl);
	if (ret) {
		misc_ctrl_err("config sp_memory_config failed\n");
		goto enter_power_idle;
	}

	ret = usb_tp_memory_config(usb_misc_ctrl);
	if (ret) {
		misc_ctrl_err("config tp_memory_config failed\n");
		goto enter_power_idle;
	}

	usb_misc_ctrl->init_count = 1;
	misc_ctrl_info("-\n");
	mutex_unlock(&usb_misc_ctrl->usb_misc_ctrl_lock);

	return 0;

enter_power_idle:
	if (enter_noc_power_idle(usb_misc_ctrl))
		misc_ctrl_err("enter_noc_power_idle failed\n");
usbc_power_down:
	usbc_mem_power_down(usb_misc_ctrl);
shutdown_clks:
	chip_usb_shutdown_clks(usb_misc_ctrl->clks, usb_misc_ctrl->num_clocks);
usb_sysctrl_reset:
	reset_usb_sysctrl(usb_misc_ctrl);
usb_apb_reset:
	reset_usb_apb_ctrl(usb_misc_ctrl);
out:
	mutex_unlock(&usb_misc_ctrl->usb_misc_ctrl_lock);
	misc_ctrl_put(usb_misc_ctrl);

	return ret;
}
EXPORT_SYMBOL_GPL(multi_usb_misc_ctrl_init);

int misc_ctrl_init(void)
{
#ifdef CONFIG_USB_HIUSB
		return 0;
#endif
		if(g_msic_ctrl_fwnode == NULL)
			return -ENODEV;
		else
			return multi_usb_misc_ctrl_init(g_msic_ctrl_fwnode);
}
EXPORT_SYMBOL_GPL(misc_ctrl_init);


static bool test_enter_noc_power_idle(struct chip_usb_misc_ctrl *misc_ctrl)
{
	bool ret = false;

	ret = chip_usb_reg_test_cfg(misc_ctrl->enter_noc_power_idleack) == 1;
	if (misc_ctrl->enter_noc_power_idlestate)
		return (ret && (chip_usb_reg_test_cfg(
			misc_ctrl->enter_noc_power_idlestate) == 1));

	return ret;
}

static int enter_noc_power_idle(struct chip_usb_misc_ctrl *misc_ctrl)
{
	if (misc_ctrl->exit_noc_power_idle) {
		int ret;
		int retrys = 10;

		ret =  chip_usb_reg_write(misc_ctrl->enter_noc_power_idle);
		if (ret < 0) {
			misc_ctrl_err("config failed\n");
			return ret;
		}

		while (retrys--) {
			if (test_enter_noc_power_idle(misc_ctrl))
				break;
			/* according to noc pw idle enter process */
			udelay(10);
		}

		if (retrys <= 0) {
			misc_ctrl_err("wait noc power idle state timeout\n");
			return -ETIMEDOUT;
		}
	}

	return 0;
}

void multi_usb_misc_ctrl_exit(struct fwnode_handle *fwnode)
{
	int ret;
	struct chip_usb_misc_ctrl *usb_misc_ctrl = NULL;
	misc_ctrl_info("+\n");
	if(!fwnode)
		return;
	usb_misc_ctrl = misc_ctrl_find(fwnode);
	if(!usb_misc_ctrl)
		return;

	mutex_lock(&usb_misc_ctrl->usb_misc_ctrl_lock);
	misc_ctrl_info("init_count %d \n", usb_misc_ctrl->init_count );

	if (usb_misc_ctrl->init_count <= 0) {
		misc_ctrl_err("call not balance\n");
		goto out;
	}

	if (--usb_misc_ctrl->init_count > 0)
		goto out;

	ret = usb_disable_smmu(usb_misc_ctrl);
	if (ret)
		misc_ctrl_err("usb_disable_smmu %d\n", ret);

	ret = enter_noc_power_idle(usb_misc_ctrl);
	if (ret) {
		misc_ctrl_err("enter_noc_power_idle failed ret %d\n", ret);
		goto out;
	}

	chip_usb_shutdown_clks(usb_misc_ctrl->clks, usb_misc_ctrl->num_clocks);
	usbc_mem_power_down(usb_misc_ctrl);
	/* reset usb ctrl config */
	if (reset_misc_ctrl(usb_misc_ctrl))
		misc_ctrl_err("reset_misc_ctrl failed\n");
	/* keep usb ctrl unreset */
	if (unreset_misc_ctrl(usb_misc_ctrl))
		misc_ctrl_err("unreset_misc_ctrl failed\n");

	if (reset_usb_sysctrl(usb_misc_ctrl))
		misc_ctrl_err("reset_usb_sysctrl failed\n");
	if (reset_usb_apb_ctrl(usb_misc_ctrl))
		misc_ctrl_err("reset_usb_apb_ctrl failed\n");

	misc_ctrl_put(usb_misc_ctrl);
	misc_ctrl_info("-\n");
out:
	mutex_unlock(&usb_misc_ctrl->usb_misc_ctrl_lock);
}
EXPORT_SYMBOL_GPL(multi_usb_misc_ctrl_exit);

void misc_ctrl_exit(void)
{
		if(g_msic_ctrl_fwnode == NULL)
			return;
		else
			return multi_usb_misc_ctrl_exit(g_msic_ctrl_fwnode);
}
EXPORT_SYMBOL_GPL(misc_ctrl_exit);

static int get_clks(struct chip_usb_misc_ctrl *misc_ctrl)
{
	struct device *dev = misc_ctrl->dev;

	return devm_chip_usb_get_clks(dev, &misc_ctrl->clks, &misc_ctrl->num_clocks);
}

#define get_reg_cfg(name) (misc_ctrl->name = \
		of_get_chip_usb_reg_cfg(np, #name))

static int get_reg_cfgs(struct chip_usb_misc_ctrl *misc_ctrl)
{
	struct device *dev = misc_ctrl->dev;
	struct device_node *np = dev->of_node;

	get_reg_cfg(misc_ctrl_reset);
	if (!misc_ctrl->misc_ctrl_reset)
		return -EINVAL;

	get_reg_cfg(misc_ctrl_unreset);
	if (!misc_ctrl->misc_ctrl_unreset)
		return -EINVAL;

	get_reg_cfg(misc_ctrl_is_unreset);
	if (!misc_ctrl->misc_ctrl_is_unreset)
		return -EINVAL;

	/* optional configs */
	get_reg_cfg(exit_noc_power_idle);
	get_reg_cfg(exit_noc_power_idleack);
	get_reg_cfg(exit_noc_power_idlestate);
	get_reg_cfg(enter_noc_power_idle);
	get_reg_cfg(enter_noc_power_idleack);
	get_reg_cfg(enter_noc_power_idlestate);
	get_reg_cfg(config_phy_interface);
	get_reg_cfg(apb_bridge_reset);
	get_reg_cfg(apb_bridge_unreset);
	get_reg_cfg(usb_sysctrl_reset);
	get_reg_cfg(usb_sysctrl_unreset);
	get_reg_cfg(sp_memory_need_config);
	get_reg_cfg(tp_memory_need_config);
	get_reg_cfg(controller_select_cfg);
	get_reg_cfg(usbc_mem_power_on);
	get_reg_cfg(usbc_mem_poweron_state);
	get_reg_cfg(usbc_mem_power_down);
	get_reg_cfg(usbc_mem_powerdown_state);

	if (misc_ctrl->exit_noc_power_idle &&
			(!misc_ctrl->exit_noc_power_idleack ||
			 !misc_ctrl->enter_noc_power_idle ||
			 !misc_ctrl->enter_noc_power_idleack))
		return -EINVAL;

	if (get_chip_usb_reg_cfg_array(dev, "init_regcfgs",
				       &misc_ctrl->init_regcfgs,
				       &misc_ctrl->num_init_regcfg)) {
		misc_ctrl_info("no init_regcfgs\n");
		misc_ctrl->init_regcfgs = NULL;
		misc_ctrl->num_init_regcfg = 0;
	}

	if (get_chip_usb_reg_cfg_array(dev, "sp_memory_config",
				       &misc_ctrl->sp_memory_config,
				       &misc_ctrl->num_sp_memory_config)) {
		misc_ctrl_info("no sp_memory_config\n");
		misc_ctrl->sp_memory_config = NULL;
		misc_ctrl->num_sp_memory_config = 0;
	}

	if (get_chip_usb_reg_cfg_array(dev, "tp_memory_config",
				       &misc_ctrl->tp_memory_config,
				       &misc_ctrl->num_tp_memory_config)) {
		misc_ctrl_info("no tp_memory_config\n");
		misc_ctrl->tp_memory_config = NULL;
		misc_ctrl->num_tp_memory_config = 0;
	}

	return 0;
}

static void put_reg_cfgs(struct chip_usb_misc_ctrl *misc_ctrl)
{
	of_remove_chip_usb_reg_cfg(misc_ctrl->misc_ctrl_reset);
	of_remove_chip_usb_reg_cfg(misc_ctrl->misc_ctrl_unreset);
	of_remove_chip_usb_reg_cfg(misc_ctrl->misc_ctrl_is_unreset);
	of_remove_chip_usb_reg_cfg(misc_ctrl->exit_noc_power_idle);
	of_remove_chip_usb_reg_cfg(misc_ctrl->exit_noc_power_idleack);
	of_remove_chip_usb_reg_cfg(misc_ctrl->enter_noc_power_idle);
	of_remove_chip_usb_reg_cfg(misc_ctrl->enter_noc_power_idleack);

	if (misc_ctrl->exit_noc_power_idlestate)
		of_remove_chip_usb_reg_cfg(misc_ctrl->exit_noc_power_idlestate);
	if (misc_ctrl->enter_noc_power_idlestate)
		of_remove_chip_usb_reg_cfg(misc_ctrl->enter_noc_power_idlestate);
	if (misc_ctrl->init_regcfgs)
		free_chip_usb_reg_cfg_array(misc_ctrl->init_regcfgs,
					    misc_ctrl->num_init_regcfg);

	if (misc_ctrl->config_phy_interface)
		of_remove_chip_usb_reg_cfg(misc_ctrl->config_phy_interface);
	if (misc_ctrl->apb_bridge_reset)
		of_remove_chip_usb_reg_cfg(misc_ctrl->apb_bridge_reset);
	if (misc_ctrl->apb_bridge_unreset)
		of_remove_chip_usb_reg_cfg(misc_ctrl->apb_bridge_unreset);
	if (misc_ctrl->usb_sysctrl_reset)
		of_remove_chip_usb_reg_cfg(misc_ctrl->usb_sysctrl_reset);
	if (misc_ctrl->usb_sysctrl_unreset)
		of_remove_chip_usb_reg_cfg(misc_ctrl->usb_sysctrl_unreset);
	if (misc_ctrl->sp_memory_need_config)
		of_remove_chip_usb_reg_cfg(misc_ctrl->sp_memory_need_config);
	if (misc_ctrl->tp_memory_need_config)
		of_remove_chip_usb_reg_cfg(misc_ctrl->tp_memory_need_config);
	if (misc_ctrl->sp_memory_config)
		free_chip_usb_reg_cfg_array(misc_ctrl->sp_memory_config,
					    misc_ctrl->num_sp_memory_config);
	if (misc_ctrl->tp_memory_config)
		free_chip_usb_reg_cfg_array(misc_ctrl->tp_memory_config,
					    misc_ctrl->num_tp_memory_config);
	if (misc_ctrl->controller_select_cfg)
		of_remove_chip_usb_reg_cfg(misc_ctrl->controller_select_cfg);
	if (misc_ctrl->usbc_mem_power_on)
		of_remove_chip_usb_reg_cfg(misc_ctrl->usbc_mem_power_on);
	if (misc_ctrl->usbc_mem_poweron_state)
		of_remove_chip_usb_reg_cfg(misc_ctrl->usbc_mem_poweron_state);
	if (misc_ctrl->usbc_mem_power_down)
		of_remove_chip_usb_reg_cfg(misc_ctrl->usbc_mem_power_down);
	if (misc_ctrl->usbc_mem_powerdown_state)
		of_remove_chip_usb_reg_cfg(misc_ctrl->usbc_mem_powerdown_state);
}

static int chip_usb_misc_ctrl_probe(struct platform_device *pdev)
{
	struct chip_usb_misc_ctrl *misc_ctrl = NULL;
	struct device *dev = &pdev->dev;
	unsigned long flags;
	int ret;

	misc_ctrl_info("+\n");
	misc_ctrl = sysctrl_create(dev, dev_fwnode(dev));
	if (IS_ERR_OR_NULL(misc_ctrl)) {
		pr_err("create misc_ctrl failed %ld\n", PTR_ERR_OR_ZERO(misc_ctrl));
		return PTR_ERR_OR_ZERO(misc_ctrl);
	}

	misc_ctrl_info("misc_ctrl %pK\n",misc_ctrl);

	platform_set_drvdata(pdev, misc_ctrl);

	ret = unreset_misc_ctrl(misc_ctrl);
	if (ret) {
		put_reg_cfgs(misc_ctrl);
		misc_ctrl_err("unreset_misc_ctrl failed\n");
		return ret;
	}

	mutex_init(&misc_ctrl->usb_misc_ctrl_lock);
	INIT_LIST_HEAD(&misc_ctrl->list);

	spin_lock_irqsave(&g_sysctrl_lock, flags);
	list_add(&misc_ctrl->list, &g_sysctrl_list);
#ifndef CONFIG_USB_HIUSB
	g_msic_ctrl_fwnode = misc_ctrl->fwnode;
#endif
	spin_unlock_irqrestore(&g_sysctrl_lock, flags);

	misc_ctrl_info("-\n");

	return 0;
}

static int chip_usb_misc_ctrl_remove(struct platform_device *pdev)
{
	struct chip_usb_misc_ctrl *misc_ctrl = platform_get_drvdata(pdev);
	unsigned long flags;

	spin_lock_irqsave(&g_sysctrl_lock, flags);
	list_del(&misc_ctrl->list);
	spin_unlock_irqrestore(&g_sysctrl_lock, flags);

	if(misc_ctrl->refcnt > 0)
		return -EBUSY;

	mutex_lock(&misc_ctrl->usb_misc_ctrl_lock);
	if (misc_ctrl->init_count) {
		mutex_unlock(&misc_ctrl->usb_misc_ctrl_lock);
		return -EBUSY;
	}

	if (reset_misc_ctrl(misc_ctrl))
		misc_ctrl_err("reset_misc_ctrl failed\n");

	put_reg_cfgs(misc_ctrl);
	mutex_unlock(&misc_ctrl->usb_misc_ctrl_lock);

#ifndef  CONFIG_USB_HIUSB
	spin_lock_irqsave(&g_sysctrl_lock, flags);
	g_msic_ctrl_fwnode = NULL;
	spin_unlock_irqrestore(&g_sysctrl_lock, flags);
#endif

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int chip_usb_misc_ctrl_resume_early(struct device *dev)
{
	struct chip_usb_misc_ctrl *misc_ctrl = dev_get_drvdata(dev);
	int ret;

	misc_ctrl_info("+\n");
	if (misc_ctrl) {
		ret = unreset_misc_ctrl(misc_ctrl);
		if (ret) {
			misc_ctrl_err("unreset_misc_ctrl failed\n");
			return ret;
		}
	}
	misc_ctrl_info("-\n");

	return 0;
}
#endif

static const struct dev_pm_ops chip_usb_misc_ctrl_pm_ops = {
	SET_LATE_SYSTEM_SLEEP_PM_OPS(NULL, chip_usb_misc_ctrl_resume_early)
};

static const struct of_device_id chip_usb_misc_ctrl_of_match[] = {
	{ .compatible = "hisilicon,usb-misc-ctrl", },
	{ }
};
MODULE_DEVICE_TABLE(of, chip_usb_misc_ctrl_of_match);

static struct platform_driver chip_usb_misc_ctrl_driver = {
	.probe = chip_usb_misc_ctrl_probe,
	.remove = chip_usb_misc_ctrl_remove,
	.driver = {
		.name = "usb-misc-ctrl",
		.of_match_table = chip_usb_misc_ctrl_of_match,
		.pm = &chip_usb_misc_ctrl_pm_ops,
	}
};

static int __init chip_usb_misc_ctrl_init(void)
{
	return platform_driver_register(&chip_usb_misc_ctrl_driver);
}
subsys_initcall(chip_usb_misc_ctrl_init);

static void __exit chip_usb_misc_ctrl_exit(void)
{
	platform_driver_unregister(&chip_usb_misc_ctrl_driver);
}
module_exit(chip_usb_misc_ctrl_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Hilisicon USB Misc Ctrl Driver");
