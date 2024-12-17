-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk

obj-$(CONFIG_MODEM_ONOFF_PARA)	+= power_para.o
obj-$(CONFIG_MODEM_ONOFF)	+= power_exchange.o
obj-$(CONFIG_MODEM_ONOFF)	+= power_on.o
obj-$(CONFIG_MODEM_ONOFF)	+= power_off.o
obj-$(CONFIG_MODEM_ONOFF)	+= bsp_modem_boot.o
obj-$(CONFIG_MODEM_ONOFF)    += adp_onoff.o
obj-$(CONFIG_MODEM_ONOFF)    += onoff_msg.o

ifeq ($(strip $(CFG_CONFIG_SYSBOOT_V1)),YES)
obj-y += power_config_v1.o
else ifeq ($(strip $(CFG_CONFIG_SYSBOOT_V2)),YES)
obj-y += power_config_v2.o
else ifeq ($(strip $(CFG_CONFIG_SYSBOOT_V3)),YES)
obj-y += power_config_v3.o
else
obj-y += power_config_stub.o
endif

GCOV_PROFILE := y