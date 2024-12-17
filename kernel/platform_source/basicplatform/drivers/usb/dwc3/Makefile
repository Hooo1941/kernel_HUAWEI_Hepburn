# SPDX-License-Identifier: GPL-2.0
# define_trace.h needs to know how to find our header

ccflags-y += -I$(srctree)/drivers/usb/dwc3
ccflags-y += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow

ifneq ($(CONFIG_DEBUG_FS)$(CONFIG_USB_DWC3),)
obj-$(CONFIG_DEBUG_FS)	+= dwc3-debugfs-ext.o
endif

obj-$(CONFIG_USB_DWC3)		+= dwc3-usb-interface.o dwc3_hook.o chip/
obj-$(CONFIG_USB_HIUSB)       += dwc3-hiusb.o
