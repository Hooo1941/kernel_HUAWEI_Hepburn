# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the rest of the USB drivers
# (the ones that don't fit into any other categories)
#
ccflags-y += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow

obj-$(CONFIG_USB_DFU)			+= dfu.o
obj-$(CONFIG_USB_MODE_INIT)		+= chip_usb_mode_init.o
obj-$(CONFIG_USB_ARMPC)			+= chip_usb_armpc.o
obj-$(CONFIG_USB_SOURCESINK_EXT)        += sourcesink_ext_test.o sourcesink_ext_core.o
