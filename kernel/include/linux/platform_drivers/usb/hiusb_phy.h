/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */

#ifndef _HIUSB_PHY_H_
#define _HIUSB_PHY_H_

#ifndef CONFIG_USB_HIUSB
#include <linux/platform_drivers/usb/tca.h>
#endif

#include <linux/platform_drivers/usb/chip_usb.h>
#include <linux/phy/phy.h>
#include <linux/types.h>
#include <uapi/linux/usb/charger.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>

struct hiusb_usb2phy {
	struct phy *phy;
	enum chip_charger_type (*detect_charger_type)(struct hiusb_usb2phy *usb2_phy);
	int (*set_dpdm_pulldown)(struct hiusb_usb2phy *usb2_phy, bool pulldown);
	int (*set_vdp_src)(struct hiusb_usb2phy *usb2_phy, bool enable);
	int (*phy_suspend)(struct hiusb_usb2phy *usb2_phy);
	int (*phy_resume)(struct hiusb_usb2phy *usb2_phy);
};

enum hiusb_phy_orien {
	HIUSB3_PHY_ORIEN_NORMAL,
	HIUSB3_PHY_ORIEN_FLIP,
};

enum hiusb_phy_dp_mode {
	HIUSB3_PHY_NC,
	HIUSB3_PHY_DP_0LANE,
	HIUSB3_PHY_DP_2LANE,
	HIUSB3_PHY_DP_4LANE,
};

struct hiusb_usb3phy {
	struct phy *phy;
	struct mutex phy_mutex;
#ifdef CONFIG_USB_HIUSB
	int (*set_orien)(struct hiusb_usb3phy *usb3_phy, enum hiusb_phy_orien orien);
	int (*set_dpmode)(struct hiusb_usb3phy *usb3_phy, enum hiusb_phy_dp_mode dpmode);
#else
	int (*set_mode)(struct hiusb_usb3phy *usb3_phy,
		enum tcpc_mux_ctrl_type mode_type,
		enum typec_plug_orien_e typec_orien);
#endif
	int (*register_debugfs)(struct hiusb_usb3phy *usb3_phy,
				struct dentry *root);
	void (*regdump)(struct hiusb_usb3phy *usb3_phy);
};
int hiusb_usb3phy_init(struct phy *phy);
int hiusb_usb3phy_exit(struct phy *phy);
int hiusb_usb3phy_set_orien(struct phy *phy, enum hiusb_phy_orien orien);
int hiusb_usb3phy_set_dpmode(struct phy *phy, enum hiusb_phy_dp_mode dpmode);
int hiusb_usb2phy_init(struct phy *phy);
int hiusb_usb2phy_poweron(struct phy *phy);
int hiusb_usb2phy_poweroff(struct phy *phy);
int hiusb_usb2phy_setmode(struct phy *phy, enum phy_mode mode);
int hiusb_usb2phy_exit(struct phy *phy);
int hiusb_usb2phy_set_dpdm_pulldown(struct phy *phy, bool pulldown);
enum chip_charger_type hiusb_usb2phy_detect_charger_type(struct phy *phy);
int hiusb_usb2phy_set_vdp_src(struct phy *phy, bool enable);
int hiusb_usb2phy_suspend(struct phy *phy);
int hiusb_usb2phy_resume(struct phy *phy);
int hiusb_usb2phy_calibrate(struct phy *phy);

#endif /* _HIUSB_PHY_H_ */
