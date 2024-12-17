# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the HID driver
#
ccflags-y += -I$(srctree)/drivers/hid
ccflags-y += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow

obj-$(CONFIG_HID_VR)		+= hid-vr.o
obj-$(CONFIG_HID_HUAWEI)	+= hid-huawei.o
