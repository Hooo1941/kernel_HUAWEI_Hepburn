/*
 * phy_chip_combophy_debugfs.h
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

#ifndef __PHY_CHIP_COMBOPHY_DEBUGFS_H__
#define __PHY_CHIP_COMBOPHY_DEBUGFS_H__

#include <linux/platform_drivers/usb/hiusb_phy.h>
#include <linux/dcache.h>
#include "phy_chip_combophy.h"

#ifdef CONFIG_DFX_DEBUG_FS

extern int chip_combophy_register_debugfs(struct hiusb_usb3phy *combophy, struct dentry *root);
extern void chip_combophy_regdump(struct hiusb_usb3phy *combophy);
extern void chip_combophy_create_regdump_debugfs(struct phy_priv *priv);
extern void chip_combophy_destroy_regdump_debugfs(struct phy_priv *priv);

#else

static inline int chip_combophy_register_debugfs(struct hiusb_usb3phy *combophy, struct dentry *root) {
	return 0;
}
static inline void chip_combophy_regdump(struct hiusb_usb3phy *combophy) { }
static inline void chip_combophy_create_regdump_debugfs(struct phy_priv *priv) { }
static inline void chip_combophy_destroy_regdump_debugfs(struct phy_priv *priv) { }

#endif /* CONFIG_DFX_DEBUG_FS */

#endif /* __PHY_CHIP_COMBOPHY_DEBUGFS_H__ */