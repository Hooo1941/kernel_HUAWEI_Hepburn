# SPDX-License-Identifier: GPL-2.0

obj-$(CONFIG_USB_HIUSB)		+= hiusb.o
hiusb-y		+= core.o port.o event.o client.o phy.o test_pd.o stub.o hifi.o bc12.o notify.o wrapper.o class.o
hiusb-$(CONFIG_DFX_DEBUG_FS)		+= debugfs.o

EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/ap/platform/common/lowpm/
ifeq ($(CONFIG_FMEA_FAULT_INJECTION), y)
EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/ap/platform/common/
endif