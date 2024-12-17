obj-$(CONFIG_USB_HIUSBC)			+= hiusbc_core.o hiusbc_debug.o  hiusbc_drd.o hiusbc_setup.o hiusbc_transfer.o hiusbc_event.o hiusbc_gadget.o hiusbc_host.o
obj-$(CONFIG_USB_HIUSB)       += hiusbc_hiusb.o
obj-$(CONFIG_DFX_DEBUG_FS)       += hiusbc_debugfs.o

ifeq ($(CONFIG_FMEA_FAULT_INJECTION), y)
EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/ap/platform/common/
endif