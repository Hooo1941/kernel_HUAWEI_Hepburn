/*
 * usb2-phy-t5.c
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 * Create:2019-09-24
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

#include <linux/atomic.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/workqueue.h>
#include <linux/version.h>
#include <linux/platform_drivers/usb/chip_usb.h>
#include <linux/platform_drivers/usb/hiusb_phy.h>
#include <linux/platform_drivers/usb/usb_misc_ctrl.h>
#include <linux/platform_drivers/usb/chip_usb_helper.h>
#include <linux/platform_drivers/usb/usb_reg_cfg.h>

#define USB2_T5_INFO(format, arg...) \
	pr_err("[USB2_T5][I][%s]"format, __func__, ##arg)

#define USB2_T5_ERR(format, arg...) \
	pr_err("[USB2_T5][E][%s]"format, __func__, ##arg)

#define chip_usb2_phy_to_phy_priv(pointer) \
	container_of(pointer, struct phy_priv, usb_phy)

struct bc_detect_cfg {
	struct chip_usb_reg_cfg **bc12_detect_dcd;
	int bc12_detect_dcd_num;
	struct chip_usb_reg_cfg **bc12_check_fsvplus;
	int bc12_check_fsvplus_num;
	struct chip_usb_reg_cfg **bc12_detect_dcd_end;
	int bc12_detect_dcd_end_num;
	struct chip_usb_reg_cfg **bc12_primary_detect;
	int bc12_primary_detect_num;
	struct chip_usb_reg_cfg **bc12_check_chrdet;
	int bc12_check_chrdet_num;
	struct chip_usb_reg_cfg **bc12_detect_sdp_stop;
	int bc12_detect_sdp_stop_num;
	struct chip_usb_reg_cfg **bc12_secondary_detect;
	int bc12_secondary_detect_num;
	struct chip_usb_reg_cfg **bc12_secondary_detect_done;
	int bc12_secondary_detect_done_num;
	struct chip_usb_reg_cfg **bc12_detect_cdp_done;
	int bc12_detect_cdp_done_num;
	struct chip_usb_reg_cfg **bc12_detect_dcd_fail;
	int bc12_detect_dcd_fail_num;
	struct chip_usb_reg_cfg **bc12_primary_detect_fail;
	int bc12_primary_detect_fail_num;
	struct chip_usb_reg_cfg **bc12_detect_close;
	int bc12_detect_close_num;
};

struct phy_priv {
	struct device *dev;
	struct hiusb_usb2phy usb_phy;

	enum phy_mode mode;
	bool disable_bc;
	bool is_fpga;
	struct clk **clks;
	int num_clks;
	struct chip_usb_reg_cfg **reset;
	int reset_num;
	struct chip_usb_reg_cfg **unreset;
	int unreset_num;
	struct chip_usb_reg_cfg **vbus_valid;
	int vbus_valid_num;
	struct chip_usb_reg_cfg **vbus_invalid;
	int vbus_invalid_num;
	bool set_eye_param_enable;
	struct chip_usb_reg_cfg **host_eye_param;
	int host_eye_param_num;
	struct chip_usb_reg_cfg **device_eye_param;
	int device_eye_param_num;
	bool calibrate_enable;
	struct chip_usb_reg_cfg *calibrate_param;
	bool bc12_detect_enable;
	struct bc_detect_cfg bc_detect_cfg;

	struct chip_usb_reg_cfg **dpdm_pullup;
	int dpdm_pullup_num;
	struct chip_usb_reg_cfg **dpdm_pulldown;
	int dpdm_pulldown_num;
	struct chip_usb_reg_cfg **enable_vdp_src;
	int enable_vdp_src_num;
	struct chip_usb_reg_cfg **disable_vdp_src;
	int disable_vdp_src_num;
	struct chip_usb_reg_cfg *phy_iddq;
	bool is_vdp_src_enabled;

	struct work_struct vdp_src_work;
	atomic_t desired_vdp_src;
};

static int usb2_phy_open_clk(struct phy_priv *priv)
{
	int ret;

	USB2_T5_INFO("+\n");

	if (priv->is_fpga)
		return 0;

	ret = chip_usb_init_clks(priv->clks, priv->num_clks);
	if (ret)
		USB2_T5_ERR("open clk failed %d\n", ret);

	USB2_T5_INFO("-\n");
	return ret;
}

static void set_usb2_eye_diagram_param(struct phy_priv *priv)
{
	struct chip_usb_reg_cfg **reg_cfgs = NULL;
	struct chip_usb_reg_cfg *eye_cfgs = NULL;
	int check_num = 0;
	int ret = 0;
	int i = 0;

	if (priv->is_fpga)
		return;

	if (!(priv->set_eye_param_enable)) {
		USB2_T5_INFO("No need to configure eye diagram\n");
		return;
	}

	if (priv->mode == PHY_MODE_USB_HOST) {
		chip_usb_reg_write_array(priv->host_eye_param, priv->host_eye_param_num);
		reg_cfgs = priv->host_eye_param;
		check_num = priv->host_eye_param_num;
		ret = chip_usb_reg_test_cfg(*(priv->host_eye_param));
	} else if (priv->mode == PHY_MODE_USB_DEVICE) {
		chip_usb_reg_write_array(priv->device_eye_param, priv->device_eye_param_num);
		reg_cfgs = priv->device_eye_param;
		check_num = priv->device_eye_param_num;
		ret = chip_usb_reg_test_cfg(*(priv->device_eye_param));
	}

	if (reg_cfgs && ret) {
		for (i = 0; i < check_num; i++) {
			eye_cfgs = reg_cfgs[i];
			USB2_T5_INFO("set phy diagram param: 0x%x\n", eye_cfgs->value);
		}
	} else {
		USB2_T5_ERR("usb parameters set failed\n");
	}

	return;
}

static int _usb2_phy_init(struct phy_priv *priv)
{
	int ret;

	USB2_T5_INFO("+\n");

	ret = misc_ctrl_init();
	if (ret)
		return ret;

	ret = usb2_phy_open_clk(priv);
	if (ret)
		goto exit_misc_ctrl;

	ret = chip_usb_reg_write_array(priv->unreset, priv->unreset_num);
	if (ret) {
		USB2_T5_ERR("phy unreset failed %d\n", ret);
		goto exit_misc_ctrl;
	}

	ret = chip_usb_reg_write_array(priv->vbus_valid, priv->vbus_valid_num);
	if (ret) {
		USB2_T5_ERR("set vbus valid failed %d\n", ret);
		goto exit_misc_ctrl;
	}

	USB2_T5_INFO("[PHY_MODE_USB_%s]: set phy-eye-diagram-param\n",
			(priv->mode == PHY_MODE_USB_HOST) ? "HOST" : "DEVICE");
	/* set eye param */
	set_usb2_eye_diagram_param(priv);

	USB2_T5_INFO("-\n");
	return ret;

exit_misc_ctrl:
	misc_ctrl_exit();

	USB2_T5_ERR("failed to init phy ret: %d\n", ret);
	return ret;
}

static int usb2_phy_init(struct phy *phy)
{
	struct hiusb_usb2phy *usb2_phy = NULL;
	struct phy_priv *priv = NULL;

	if (!phy)
		return -ENODEV;

	usb2_phy = phy_get_drvdata(phy);
	if (!usb2_phy)
		return -ENODEV;

	priv = chip_usb2_phy_to_phy_priv(usb2_phy);

	return _usb2_phy_init(priv);
}

static int usb2_phy_close_clk(struct phy_priv *priv)
{
	USB2_T5_INFO("+\n");

	if (priv->is_fpga)
		return 0;

	chip_usb_shutdown_clks(priv->clks, priv->num_clks);

	USB2_T5_INFO("-\n");
	return 0;
}

static void usb2_phy_config_iddq(struct phy_priv *priv)
{
	int ret;

	if (priv->phy_iddq) {
		USB2_T5_INFO("config phy iddq\n");
		ret = chip_usb_reg_write(priv->phy_iddq);
		if (ret) {
			USB2_T5_ERR("config phy iddq failed\n");
			return;
		}
	}
}

static int _usb2_phy_exit(struct phy_priv *priv)
{
	int ret;

	USB2_T5_INFO("+\n");

	ret = chip_usb_reg_write_array(priv->vbus_invalid,
			priv->vbus_invalid_num);
	if (ret)
		USB2_T5_ERR("set vbus invalid failed %d\n", ret);

	usb2_phy_config_iddq(priv);
	ret = chip_usb_reg_write_array(priv->reset,
			priv->reset_num);
	if (ret)
		USB2_T5_ERR("phy reset failed %d\n", ret);

	ret = usb2_phy_close_clk(priv);
	if (ret)
		USB2_T5_ERR("close clk failed %d\n", ret);

	misc_ctrl_exit();

	USB2_T5_INFO("-\n");
	return ret;
}

static int usb2_phy_exit(struct phy *phy)
{
	struct hiusb_usb2phy *usb2_phy = NULL;
	struct phy_priv *priv = NULL;

	if (!phy)
		return -ENODEV;

	usb2_phy = phy_get_drvdata(phy);
	if (!usb2_phy)
		return -ENODEV;

	priv = chip_usb2_phy_to_phy_priv(usb2_phy);

	return _usb2_phy_exit(priv);
}

static int usb2_phy_set_mode(struct phy *phy, enum phy_mode mode)
{
	struct hiusb_usb2phy *usb2_phy = NULL;
	struct phy_priv *priv = NULL;

	if (!phy)
		return -ENODEV;

	usb2_phy = (struct hiusb_usb2phy *)phy_get_drvdata(phy);
	if (!usb2_phy)
		return -ENODEV;

	priv = chip_usb2_phy_to_phy_priv(usb2_phy);

	priv->mode = mode;

	return 0;
}

static int usb2_phy_set_mode_ext(struct phy *phy, enum phy_mode mode,
				 int submode)
{
	return usb2_phy_set_mode(phy, mode);
}

static int _usb2_phy_calibrate(struct phy_priv *priv)
{
	int ret;

	USB2_T5_INFO("+\n");

	ret = chip_usb_reg_write(priv->calibrate_param);
	if (ret)
		USB2_T5_ERR("phy_calibrate failed %d\n", ret);

	USB2_T5_INFO("-\n");

	return ret;
}

static int usb2_phy_calibrate(struct phy *phy)
{
	struct hiusb_usb2phy *usb2_phy = NULL;
	struct phy_priv *priv = NULL;

	if (!phy)
		return -ENODEV;

	usb2_phy = phy_get_drvdata(phy);
	if (!usb2_phy)
		return -ENODEV;

	priv = chip_usb2_phy_to_phy_priv(usb2_phy);
	if (priv->calibrate_enable)
		return _usb2_phy_calibrate(priv);

	return 0;
}

static struct phy_ops usb2_phy_ops = {
	.init		= usb2_phy_init,
	.exit		= usb2_phy_exit,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
	.set_mode	= usb2_phy_set_mode_ext,
#else
	.set_mode	= usb2_phy_set_mode,
#endif
	.calibrate	= usb2_phy_calibrate,
	.owner		= THIS_MODULE,
};

static int is_dcd_timeout(struct phy_priv *priv)
{
	unsigned long jiffies_expire;
	int ret = 0;
	int i = 0;

	jiffies_expire = jiffies + msecs_to_jiffies(900);
	msleep(50);
	while (i < 10) {
		ret = chip_usb_reg_test_cfg(*(priv->bc_detect_cfg.bc12_check_fsvplus));
		if (ret)
			i++;
		else
			i = 0;

		msleep(10);

		if (time_after(jiffies, jiffies_expire)) {
			USB2_T5_ERR("DCD timeout!\n");
			ret = -1;
			break;
		}
	}

	return ret;
}

static enum chip_charger_type bc12_detect_charger_type(struct phy_priv *priv)
{
	enum chip_charger_type charger_type = CHARGER_TYPE_UNKNOWN;
	int ret;

	chip_usb_reg_write_array(priv->bc_detect_cfg.bc12_detect_dcd,
						priv->bc_detect_cfg.bc12_detect_dcd_num);

	ret = is_dcd_timeout(priv);
	if (ret <= 0) {
		USB2_T5_ERR("bc12_check_fsvplus fail\n");
		chip_usb_reg_write_array(priv->bc_detect_cfg.bc12_detect_dcd_fail,
							priv->bc_detect_cfg.bc12_detect_dcd_fail_num);
		goto close_bc12_detect;
	}

	chip_usb_reg_write_array(priv->bc_detect_cfg.bc12_detect_dcd_end,
						priv->bc_detect_cfg.bc12_detect_dcd_end_num);
	msleep(2);
	chip_usb_reg_write_array(priv->bc_detect_cfg.bc12_primary_detect,
						priv->bc_detect_cfg.bc12_primary_detect_num);
	msleep(30);

	ret = chip_usb_reg_test_cfg(*(priv->bc_detect_cfg.bc12_check_chrdet));
	if (ret <= 0) {
		USB2_T5_ERR("charger type = SDP\n");
		chip_usb_reg_write_array(priv->bc_detect_cfg.bc12_primary_detect_fail,
							priv->bc_detect_cfg.bc12_primary_detect_fail_num);
		charger_type = CHARGER_TYPE_SDP;
		goto close_bc12_detect;
	}

	chip_usb_reg_write_array(priv->bc_detect_cfg.bc12_detect_sdp_stop,
						priv->bc_detect_cfg.bc12_detect_sdp_stop_num);
	msleep(2);
	chip_usb_reg_write_array(priv->bc_detect_cfg.bc12_secondary_detect,
						priv->bc_detect_cfg.bc12_secondary_detect_num);
	msleep(30);

	ret = chip_usb_reg_test_cfg(*(priv->bc_detect_cfg.bc12_check_chrdet));
	if (ret <= 0) {
		USB2_T5_ERR("charger type = CDP\n");
		chip_usb_reg_write_array(priv->bc_detect_cfg.bc12_secondary_detect_done,
							priv->bc_detect_cfg.bc12_secondary_detect_done_num);
		chip_usb_reg_write_array(priv->bc_detect_cfg.bc12_detect_cdp_done,
							priv->bc_detect_cfg.bc12_detect_cdp_done_num);
		charger_type = CHARGER_TYPE_CDP;
	} else {
		USB2_T5_ERR("charger type = DCP\n");
		chip_usb_reg_write_array(priv->bc_detect_cfg.bc12_secondary_detect_done,
							priv->bc_detect_cfg.bc12_secondary_detect_done_num);
		charger_type = CHARGER_TYPE_DCP;
	}

close_bc12_detect:
	chip_usb_reg_write_array(priv->bc_detect_cfg.bc12_detect_close,
						priv->bc_detect_cfg.bc12_detect_close_num);
	return charger_type;
}

static enum chip_charger_type detect_charger_type(
		struct hiusb_usb2phy *usb2_phy)
{
	struct phy_priv *priv = NULL;
	struct phy *phy = NULL;
	enum chip_charger_type charger_type = CHARGER_TYPE_SDP;

	if (!usb2_phy)
		return CHARGER_TYPE_NONE;

	priv = chip_usb2_phy_to_phy_priv(usb2_phy);

	if (priv->disable_bc)
		return CHARGER_TYPE_SDP;

	phy = priv->usb_phy.phy;

	mutex_lock(&phy->mutex);
	if (phy->init_count == 0) {
		mutex_unlock(&phy->mutex);
		USB2_T5_ERR("phy has not been initialized\n");
		return CHARGER_TYPE_NONE;
	}

	if (priv->bc12_detect_enable)
		charger_type = bc12_detect_charger_type(priv);

	mutex_unlock(&phy->mutex);

	return charger_type;
}

static int set_dpdm_pulldown(struct hiusb_usb2phy *usb2_phy, bool pulldown)
{
	struct phy_priv *priv = NULL;
	struct phy *phy = NULL;
	int ret;

	if (!usb2_phy)
		return -EINVAL;

	priv = chip_usb2_phy_to_phy_priv(usb2_phy);
	phy = priv->usb_phy.phy;

	if (priv->is_fpga || priv->disable_bc ||
		!priv->dpdm_pullup || !priv->dpdm_pulldown)
		return 0;

	USB2_T5_INFO("+\n");
	mutex_lock(&phy->mutex);
	if (phy->init_count == 0) {
		mutex_unlock(&phy->mutex);
		USB2_T5_ERR("phy has not been initialized\n");
		return -ENODEV;
	}

	if (pulldown)
		ret = chip_usb_reg_write_array(priv->dpdm_pulldown, priv->dpdm_pulldown_num);
	else
		ret = chip_usb_reg_write_array(priv->dpdm_pullup, priv->dpdm_pullup_num);

	if (ret)
		USB2_T5_ERR("%s failed %d\n", pulldown ? "pulldown" : "pullup", ret);

	mutex_unlock(&phy->mutex);
	USB2_T5_INFO("-\n");
	return ret;
}

static int _set_vdp_src(struct phy_priv *priv, bool enable)
{
	struct phy *phy = priv->usb_phy.phy;
	int ret;

	if (priv->is_fpga || priv->disable_bc ||
		!priv->enable_vdp_src || !priv->disable_vdp_src)
		return 0;

	if ((enable && priv->is_vdp_src_enabled) ||
		(!enable && !priv->is_vdp_src_enabled))
		return 0;

	USB2_T5_INFO("+\n");
	mutex_lock(&phy->mutex);
	if (phy->init_count == 0) {
		mutex_unlock(&phy->mutex);
		USB2_T5_ERR("phy has not been initialized\n");
		return -ENODEV;
	}

	if (enable)
		ret = chip_usb_reg_write_array(priv->enable_vdp_src, priv->enable_vdp_src_num);
	else
		ret = chip_usb_reg_write_array(priv->disable_vdp_src, priv->disable_vdp_src_num);

	if (ret)
		USB2_T5_ERR("%s vdp_src failed %d\n", enable ? "enable" : "disable", ret);
	else
		priv->is_vdp_src_enabled = enable ? true : false;

	mutex_unlock(&phy->mutex);
	USB2_T5_INFO("-\n");
	return ret;
}

static int set_vdp_src(struct hiusb_usb2phy *usb2_phy, bool enable)
{
	struct phy_priv *priv = NULL;

	if (!usb2_phy)
		return -EINVAL;

	priv = chip_usb2_phy_to_phy_priv(usb2_phy);

	if (priv->is_fpga)
		return 0;

	if (in_atomic() || irqs_disabled()) {
		if (enable)
			atomic_set(&priv->desired_vdp_src, 1);
		else
			atomic_set(&priv->desired_vdp_src, 0);

		if (!queue_work(system_power_efficient_wq,
					&priv->vdp_src_work)) {
			USB2_T5_INFO("vdp src work already in queue\n");
			return -EAGAIN;
		}
		return 0;
	} else {
		return _set_vdp_src(priv, enable);
	}
}

static void vdp_src_work_fn(struct work_struct *work)
{
	int ret;
	struct phy_priv *priv = container_of(work, struct phy_priv, vdp_src_work);

	ret = _set_vdp_src(priv, atomic_read(&priv->desired_vdp_src) == 1);
	if (ret)
		USB2_T5_ERR("set vdp src failed %d\n", ret);
}

static int get_bc12_ops_resource(struct phy_priv *priv)
{
	struct device *dev = priv->dev;
	int ret;

	ret = get_chip_usb_reg_cfg_array(dev, "bc12-detect-dcd",
			&(priv->bc_detect_cfg.bc12_detect_dcd), &(priv->bc_detect_cfg.bc12_detect_dcd_num));
	if (ret)
		goto exit_bc12_detect_dcd;

	ret = get_chip_usb_reg_cfg_array(dev, "bc12-check-fsvplus",
			&(priv->bc_detect_cfg.bc12_check_fsvplus), &(priv->bc_detect_cfg.bc12_check_fsvplus_num));
	if (ret)
		goto exit_bc12_check_fsvplus;

	ret = get_chip_usb_reg_cfg_array(dev, "bc12-detect-dcd-end",
			&(priv->bc_detect_cfg.bc12_detect_dcd_end), &(priv->bc_detect_cfg.bc12_detect_dcd_end_num));
	if (ret)
		goto exit_bc12_detect_dcd_end;

	ret = get_chip_usb_reg_cfg_array(dev, "bc12-primary-detect",
			&(priv->bc_detect_cfg.bc12_primary_detect), &(priv->bc_detect_cfg.bc12_primary_detect_num));
	if (ret)
		goto exit_bc12_primary_detect;

	ret = get_chip_usb_reg_cfg_array(dev, "bc12-check-chrdet",
			&(priv->bc_detect_cfg.bc12_check_chrdet), &(priv->bc_detect_cfg.bc12_check_chrdet_num));
	if (ret)
		goto exit_bc12_check_chrdet;

	ret = get_chip_usb_reg_cfg_array(dev, "bc12-detect-sdp-stop",
			&(priv->bc_detect_cfg.bc12_detect_sdp_stop), &(priv->bc_detect_cfg.bc12_detect_sdp_stop_num));
	if (ret)
		goto exit_bc12_detect_sdp_stop;

	ret = get_chip_usb_reg_cfg_array(dev, "bc12-secondary-detect",
			&(priv->bc_detect_cfg.bc12_secondary_detect), &(priv->bc_detect_cfg.bc12_secondary_detect_num));
	if (ret)
		goto exit_bc12_secondary_detect;

	ret = get_chip_usb_reg_cfg_array(dev, "bc12-secondary-detect-done",
			&(priv->bc_detect_cfg.bc12_secondary_detect_done), &(priv->bc_detect_cfg.bc12_secondary_detect_done_num));
	if (ret)
		goto exit_bc12_secondary_detect_done;

	ret = get_chip_usb_reg_cfg_array(dev, "bc12-detect-cdp-done",
			&(priv->bc_detect_cfg.bc12_detect_cdp_done), &(priv->bc_detect_cfg.bc12_detect_cdp_done_num));
	if (ret)
		goto exit_bc12_detect_cdp_done;

	ret = get_chip_usb_reg_cfg_array(dev, "bc12-detect-dcd-fail",
			&(priv->bc_detect_cfg.bc12_detect_dcd_fail), &(priv->bc_detect_cfg.bc12_detect_dcd_fail_num));
	if (ret)
		goto exit_bc12_detect_dcd_fail;

	ret = get_chip_usb_reg_cfg_array(dev, "bc12-primary-detect-fail",
			&(priv->bc_detect_cfg.bc12_primary_detect_fail), &(priv->bc_detect_cfg.bc12_primary_detect_fail_num));
	if (ret)
		goto exit_bc12_primary_detect_fail;

	ret = get_chip_usb_reg_cfg_array(dev, "bc12-detect-close",
			&(priv->bc_detect_cfg.bc12_detect_close), &(priv->bc_detect_cfg.bc12_detect_close_num));
	if (ret)
		goto exit_bc12_detect_close;

	return ret;

exit_bc12_detect_close:
	free_chip_usb_reg_cfg_array(priv->bc_detect_cfg.bc12_primary_detect_fail,
			priv->bc_detect_cfg.bc12_primary_detect_fail_num);
exit_bc12_primary_detect_fail:
	free_chip_usb_reg_cfg_array(priv->bc_detect_cfg.bc12_detect_dcd_fail, priv->bc_detect_cfg.bc12_detect_dcd_fail_num);
exit_bc12_detect_dcd_fail:
	free_chip_usb_reg_cfg_array(priv->bc_detect_cfg.bc12_detect_cdp_done, priv->bc_detect_cfg.bc12_detect_cdp_done_num);
exit_bc12_detect_cdp_done:
	free_chip_usb_reg_cfg_array(priv->bc_detect_cfg.bc12_secondary_detect_done,
			priv->bc_detect_cfg.bc12_secondary_detect_done_num);
exit_bc12_secondary_detect_done:
	free_chip_usb_reg_cfg_array(priv->bc_detect_cfg.bc12_secondary_detect, priv->bc_detect_cfg.bc12_secondary_detect_num);
exit_bc12_secondary_detect:
	free_chip_usb_reg_cfg_array(priv->bc_detect_cfg.bc12_detect_sdp_stop, priv->bc_detect_cfg.bc12_detect_sdp_stop_num);
exit_bc12_detect_sdp_stop:
	free_chip_usb_reg_cfg_array(priv->bc_detect_cfg.bc12_check_chrdet, priv->bc_detect_cfg.bc12_check_chrdet_num);
exit_bc12_check_chrdet:
	free_chip_usb_reg_cfg_array(priv->bc_detect_cfg.bc12_primary_detect, priv->bc_detect_cfg.bc12_primary_detect_num);
exit_bc12_primary_detect:
	free_chip_usb_reg_cfg_array(priv->bc_detect_cfg.bc12_detect_dcd_end, priv->bc_detect_cfg.bc12_detect_dcd_end_num);
exit_bc12_detect_dcd_end:
	free_chip_usb_reg_cfg_array(priv->bc_detect_cfg.bc12_check_fsvplus, priv->bc_detect_cfg.bc12_check_fsvplus_num);
exit_bc12_check_fsvplus:
	free_chip_usb_reg_cfg_array(priv->bc_detect_cfg.bc12_detect_dcd, priv->bc_detect_cfg.bc12_detect_dcd_num);
exit_bc12_detect_dcd:
	USB2_T5_ERR("get bc detect resouce failed %d\n", ret);
	return ret;
}

static int get_ops_resource(struct phy_priv *priv)
{
	struct device *dev = priv->dev;
	int ret;

	USB2_T5_INFO("+\n");

	ret = get_chip_usb_reg_cfg_array(dev, "phy-reset",
			&priv->reset, &priv->reset_num);
	if (ret) {
		USB2_T5_ERR("get phy reset failed\n");
		return -ENOENT;
	}

	ret = get_chip_usb_reg_cfg_array(dev, "phy-unreset",
			&priv->unreset, &priv->unreset_num);
	if (ret) {
		USB2_T5_ERR("get phy unrest failed %d\n", ret);
		goto exit_unreset;
	}

	ret = get_chip_usb_reg_cfg_array(dev, "vbus-valid",
				&priv->vbus_valid, &priv->vbus_valid_num);
	if (ret) {
		USB2_T5_ERR("get vbus valid failed %d\n", ret);
		goto exit_vbus_valid;
	}

	ret = get_chip_usb_reg_cfg_array(dev, "vbus-invalid",
				&priv->vbus_invalid, &priv->vbus_invalid_num);
	if (ret) {
		USB2_T5_ERR("get vbus invalid failed %d\n", ret);
		goto exit_vbus_invalid;
	}

	priv->set_eye_param_enable = of_property_read_bool(dev->of_node, "set-eye-param-enable");
	if (priv->set_eye_param_enable) {
		ret = get_chip_usb_reg_cfg_array(dev, "host-eye-param",
				&priv->host_eye_param, &priv->host_eye_param_num);
		if (ret) {
			USB2_T5_ERR("get host eye param failed %d\n", ret);
			goto exit_host_eye_param;
		}
		ret = get_chip_usb_reg_cfg_array(dev, "device-eye-param",
				&priv->device_eye_param, &priv->device_eye_param_num);
		if (ret) {
			USB2_T5_ERR("get  device eye param failed %d\n", ret);
			goto exit_device_eye_param;
		}
	}

	priv->calibrate_enable = of_property_read_bool(dev->of_node, "calibrate-enable");
	if (priv->calibrate_enable) {
		priv->calibrate_param = of_get_chip_usb_reg_cfg(dev->of_node, "calibrate-para");
		if (!priv->calibrate_param) {
			ret = -EINVAL;
			USB2_T5_ERR("get phy_calibrate_param failed %d\n", ret);
			goto exit_calibrate_param;
		}
	}

	priv->bc12_detect_enable = of_property_read_bool(dev->of_node, "bc12-detect-enable");
	if (priv->bc12_detect_enable) {
		ret = get_bc12_ops_resource(priv);
		if (ret) {
			USB2_T5_ERR("get bc1.2 ops resource failed %d\n", ret);
			goto exit_bc12_resource;
		}
	}

	ret = get_chip_usb_reg_cfg_array(dev, "dpdm-pullup",
				&priv->dpdm_pullup, &priv->dpdm_pullup_num);
	if (ret)
		USB2_T5_ERR("get dpdm-pullup failed %d\n", ret);

	ret = get_chip_usb_reg_cfg_array(dev, "dpdm-pulldown",
				&priv->dpdm_pulldown, &priv->dpdm_pulldown_num);
	if (ret)
		USB2_T5_ERR("get dpdm-pulldown failed %d\n", ret);

	ret = get_chip_usb_reg_cfg_array(dev, "enable-vdp-src",
				&priv->enable_vdp_src, &priv->enable_vdp_src_num);
	if (ret)
		USB2_T5_ERR("get enable-vdp-src failed %d\n", ret);

	ret = get_chip_usb_reg_cfg_array(dev, "disable-vdp-src",
				&priv->disable_vdp_src, &priv->disable_vdp_src_num);
	if (ret)
		USB2_T5_ERR("get disable-vdp-src failed %d\n", ret);

	priv->phy_iddq = of_get_chip_usb_reg_cfg(dev->of_node, "phy_iddq");
	if (!priv->phy_iddq)
		USB2_T5_INFO("phy_iddq not found\n");

	USB2_T5_INFO("-\n");
	return 0;

exit_bc12_resource:
	if (priv->calibrate_enable)
		of_remove_chip_usb_reg_cfg(priv->calibrate_param);
exit_calibrate_param:
	if (priv->set_eye_param_enable)
		free_chip_usb_reg_cfg_array(priv->device_eye_param, priv->device_eye_param_num);
exit_device_eye_param:
	if (priv->set_eye_param_enable)
		free_chip_usb_reg_cfg_array(priv->host_eye_param, priv->host_eye_param_num);
exit_host_eye_param:
	free_chip_usb_reg_cfg_array(priv->vbus_invalid, priv->vbus_invalid_num);
exit_vbus_invalid:
	free_chip_usb_reg_cfg_array(priv->vbus_valid, priv->vbus_valid_num);
exit_vbus_valid:
	free_chip_usb_reg_cfg_array(priv->unreset, priv->unreset_num);
exit_unreset:
	free_chip_usb_reg_cfg_array(priv->reset, priv->reset_num);
	USB2_T5_ERR("- %d\n", ret);
	return ret;
}

static void put_bc12_ops_resource(struct phy_priv *priv)
{
	if (!(priv->bc12_detect_enable))
		return;

	free_chip_usb_reg_cfg_array(priv->bc_detect_cfg.bc12_detect_close,
					priv->bc_detect_cfg.bc12_detect_close_num);
	free_chip_usb_reg_cfg_array(priv->bc_detect_cfg.bc12_primary_detect_fail,
					priv->bc_detect_cfg.bc12_primary_detect_fail_num);
	free_chip_usb_reg_cfg_array(priv->bc_detect_cfg.bc12_detect_dcd_fail,
					priv->bc_detect_cfg.bc12_detect_dcd_fail_num);
	free_chip_usb_reg_cfg_array(priv->bc_detect_cfg.bc12_detect_cdp_done,
					priv->bc_detect_cfg.bc12_detect_cdp_done_num);
	free_chip_usb_reg_cfg_array(priv->bc_detect_cfg.bc12_secondary_detect_done,
					priv->bc_detect_cfg.bc12_secondary_detect_done_num);
	free_chip_usb_reg_cfg_array(priv->bc_detect_cfg.bc12_secondary_detect,
					priv->bc_detect_cfg.bc12_secondary_detect_num);
	free_chip_usb_reg_cfg_array(priv->bc_detect_cfg.bc12_detect_sdp_stop,
					priv->bc_detect_cfg.bc12_detect_sdp_stop_num);
	free_chip_usb_reg_cfg_array(priv->bc_detect_cfg.bc12_check_chrdet,
					priv->bc_detect_cfg.bc12_check_chrdet_num);
	free_chip_usb_reg_cfg_array(priv->bc_detect_cfg.bc12_primary_detect,
					priv->bc_detect_cfg.bc12_primary_detect_num);
	free_chip_usb_reg_cfg_array(priv->bc_detect_cfg.bc12_detect_dcd_end,
					priv->bc_detect_cfg.bc12_detect_dcd_end_num);
	free_chip_usb_reg_cfg_array(priv->bc_detect_cfg.bc12_check_fsvplus,
					priv->bc_detect_cfg.bc12_check_fsvplus_num);
	free_chip_usb_reg_cfg_array(priv->bc_detect_cfg.bc12_detect_dcd,
					priv->bc_detect_cfg.bc12_detect_dcd_num);
	return;
}
static void put_ops_resource(struct phy_priv *priv)
{
	USB2_T5_INFO("+\n");

	free_chip_usb_reg_cfg_array(priv->reset, priv->reset_num);

	free_chip_usb_reg_cfg_array(priv->unreset, priv->unreset_num);

	free_chip_usb_reg_cfg_array(priv->vbus_valid,
					priv->vbus_valid_num);
	free_chip_usb_reg_cfg_array(priv->vbus_invalid,
					priv->vbus_invalid_num);
	if (priv->set_eye_param_enable) {
		free_chip_usb_reg_cfg_array(priv->host_eye_param,
						priv->host_eye_param_num);
		free_chip_usb_reg_cfg_array(priv->device_eye_param,
						priv->device_eye_param_num);
	}

	if (priv->calibrate_enable)
		of_remove_chip_usb_reg_cfg(priv->calibrate_param);

	put_bc12_ops_resource(priv);

	if (priv->dpdm_pullup)
		free_chip_usb_reg_cfg_array(priv->dpdm_pullup, priv->dpdm_pullup_num);
	if (priv->dpdm_pulldown)
		free_chip_usb_reg_cfg_array(priv->dpdm_pulldown, priv->dpdm_pulldown_num);
	if (priv->enable_vdp_src)
		free_chip_usb_reg_cfg_array(priv->enable_vdp_src, priv->enable_vdp_src_num);
	if (priv->disable_vdp_src)
		free_chip_usb_reg_cfg_array(priv->disable_vdp_src, priv->disable_vdp_src_num);
	if (priv->phy_iddq)
		of_remove_chip_usb_reg_cfg(priv->phy_iddq);
	USB2_T5_INFO("-\n");
}

static int get_clk_resource(struct phy_priv *priv)
{
	struct device *dev = priv->dev;
	int ret;

	if (priv->is_fpga)
		return 0;

	ret = chip_usb_get_clks(dev->of_node, &priv->clks, &priv->num_clks);
	if (ret)
		USB2_T5_ERR("get clk fail %d\n", ret);

	return ret;
}

static void put_clk_resource(struct phy_priv *priv)
{
	int ret;
	if (priv->is_fpga)
		return;

	ret = chip_usb_put_clks(priv->clks, priv->num_clks);
	if (ret)
		USB2_T5_ERR("put clk fail %d\n", ret);
}

static int get_dts_resource(struct phy_priv *priv)
{
	priv->is_fpga = of_property_read_bool(priv->dev->of_node, "is-fpga");
	priv->disable_bc = of_property_read_bool(priv->dev->of_node,
					"disable-bc");

	USB2_T5_INFO("is_fpga %d, disable_bc %d!\n",
		priv->is_fpga, priv->disable_bc);

	if (get_clk_resource(priv)) {
		USB2_T5_ERR("get clk failed\n");
		return -EINVAL;
	}

	if (get_ops_resource(priv)) {
		put_clk_resource(priv);
		USB2_T5_ERR("get ops failed\n");
		return -EINVAL;
	}

	return 0;
}

static void put_dts_resource(struct phy_priv *priv)
{
	put_clk_resource(priv);

	put_ops_resource(priv);
}

static int usb2_phy_suspend(struct hiusb_usb2phy *usb2_phy)
{
	struct phy_priv *priv = NULL;
	int ret;

	if (!usb2_phy)
		return -EINVAL;

	priv = chip_usb2_phy_to_phy_priv(usb2_phy);
	ret = usb2_phy_close_clk(priv);
	if (ret)
		USB2_T5_ERR("close clk failed %d\n", ret);

	return ret;
}

static int usb2_phy_resume(struct hiusb_usb2phy *usb2_phy)
{
	struct phy_priv *priv = NULL;
	int ret;

	if (!usb2_phy)
		return -EINVAL;

	priv = chip_usb2_phy_to_phy_priv(usb2_phy);
	ret = usb2_phy_open_clk(priv);
	if (ret)
		USB2_T5_ERR("open clk failed %d\n", ret);

	return ret;
}

static int usb2_phy_t5_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct phy_provider *phy_provider = NULL;
	struct phy *phy = NULL;
	struct phy_priv *priv = NULL;
	int ret;

	USB2_T5_INFO("+\n");
	if (!misc_ctrl_is_ready()) {
		USB2_T5_ERR("misc ctrl is not ready\n");
		return -EPROBE_DEFER;
	}

	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);
	pm_runtime_no_callbacks(dev);
	ret = pm_runtime_get_sync(dev);
	if (ret < 0) {
		USB2_T5_ERR("pm_runtime_get_sync failed\n");
		goto err_pm_put;
	}

	pm_runtime_forbid(dev);

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		ret = -ENOMEM;
		goto err_pm_allow;
	}

	priv->dev = dev;

	ret = get_dts_resource(priv);
	if (ret) {
		USB2_T5_ERR("get_dts_resource failed\n");
		goto err_pm_allow;
	}

	phy = devm_phy_create(dev, NULL, &usb2_phy_ops);
	if (IS_ERR(phy)) {
		ret = PTR_ERR(phy);
		goto err_get_dts;
	}

	priv->usb_phy.phy = phy;
	priv->usb_phy.detect_charger_type = detect_charger_type;
	priv->usb_phy.set_dpdm_pulldown = set_dpdm_pulldown;
	priv->usb_phy.set_vdp_src = set_vdp_src;
	priv->usb_phy.phy_suspend = usb2_phy_suspend;
	priv->usb_phy.phy_resume = usb2_phy_resume;
	priv->mode = PHY_MODE_INVALID;
	priv->is_vdp_src_enabled = false;
	phy_set_drvdata(phy, &priv->usb_phy);
	INIT_WORK(&priv->vdp_src_work, vdp_src_work_fn);
	atomic_set(&priv->desired_vdp_src, 0);

	phy_provider = devm_of_phy_provider_register(dev, of_phy_simple_xlate);
	if (IS_ERR(phy_provider)) {
		ret = PTR_ERR(phy_provider);
		goto err_get_dts;
	}

	platform_set_drvdata(pdev, priv);

	USB2_T5_ERR("-\n");
	return 0;

err_get_dts:
	put_dts_resource(priv);
err_pm_allow:
	pm_runtime_allow(dev);
err_pm_put:
	pm_runtime_put_sync(dev);
	pm_runtime_disable(dev);

	USB2_T5_INFO("ret %d\n", ret);
	return ret;
}

static int usb2_phy_t5_remove(struct platform_device *pdev)
{
	struct phy_priv *priv = (struct phy_priv *)platform_get_drvdata(pdev);

	if (cancel_work_sync(&priv->vdp_src_work))
		USB2_T5_INFO("vdp src work canceled\n");

	put_dts_resource(priv);
	pm_runtime_allow(&pdev->dev);
	pm_runtime_put_sync(&pdev->dev);
	pm_runtime_disable(&pdev->dev);

	return 0;
}

static const struct of_device_id usb2_phy_t5_of_match[] = {
	{ .compatible = "hisilicon,usb2-phy-t5", },
	{ }
};
MODULE_DEVICE_TABLE(of, usb2_phy_t5_of_match);

static struct platform_driver usb2_phy_t5_driver = {
	.probe = usb2_phy_t5_probe,
	.remove = usb2_phy_t5_remove,
	.driver = {
		.name = "usb2-phy-t5",
		.of_match_table = usb2_phy_t5_of_match,
	}
};
module_platform_driver(usb2_phy_t5_driver);

MODULE_AUTHOR("Hu Wang");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("USB2 PHY T5 Driver");
