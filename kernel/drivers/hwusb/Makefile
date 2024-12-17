obj-$(CONFIG_HW_GADGET) += hw_gadget/
obj-$(CONFIG_HUAWEI_USB) += hw_usb/
ifeq ($(CONFIG_HUAWEI_USB_VENDOR),y)
obj-y += hw_dp/
else
obj-y += hw_pd/
endif
obj-$(CONFIG_HW_PD_COMMON) += hw_pd_common/