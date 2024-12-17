# SPDX-License-Identifier: GPL-2.0-only

ccflags-y += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow
obj-$(CONFIG_PHY_HISI_USB2_ULPI)	+= chip-usb2-phy-ulpi.o
obj-$(CONFIG_PHY_HISI_COMBOPHY)     += phy-chip-combophy.o chip-combophy-mcu.o
ifeq ($(CONFIG_PHY_HISI_COMBOPHY), y)
	obj-$(CONFIG_DFX_DEBUG_FS)	+= phy_chip_combophy_debugfs.o
endif
obj-$(CONFIG_PHY_GEN2_COMBOPHY)     += phy_gen2_combophy.o
ifeq ($(CONFIG_PHY_GEN2_COMBOPHY), y)
	obj-$(CONFIG_DFX_DEBUG_FS)      += phy_gen2_combophy_debugfs.o
endif
