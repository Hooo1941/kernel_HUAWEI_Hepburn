ccflags-y += -Idrivers/devkit/audiokit/

ifdef CONFIG_HW_PD_COMMON
obj-y += hw_pd_platform/
else
obj-y += hw_pd.o
endif
obj-y += hw_usb_vbus.o

ifdef CONFIG_TCPC_RT1715
obj-$(CONFIG_TCPC_RT1715) += rt1715/
else
obj-$(CONFIG_RT_USB_PD) += richtek/
endif
obj-$(CONFIG_FSC_USB_PD) += fsc/
