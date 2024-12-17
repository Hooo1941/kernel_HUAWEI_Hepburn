ccflags-y += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow

obj-$(CONFIG_CHIP_USB_REG_CFG)		   += usb_reg_cfg.o
obj-$(CONFIG_CHIP_USB_HELPER)		   += chip_usb_helper.o
obj-$(CONFIG_CHIP_USB_MISC_CTRL)	   += usb_misc_ctrl.o usb_smmu.o
ifeq ($(CONFIG_USB_DWC3_CHIP),y)
obj-$(CONFIG_DFX_DEBUG_FS)      += chip_usb_debugfs_common.o
else ifeq ($(CONFIG_USB_HIUSB),y)
obj-$(CONFIG_DFX_DEBUG_FS)      += chip_usb_debugfs_common.o
endif
