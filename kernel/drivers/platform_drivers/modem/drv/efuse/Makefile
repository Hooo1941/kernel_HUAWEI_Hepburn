
-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk

ifeq ($(strip $(CFG_CONFIG_EFUSE)),YES)
obj-$(CONFIG_EFUSE_IOT)		+= efuse_driver.o
obj-$(CONFIG_EFUSE_MODEM_AGENT)	+= efuse_agent.o
obj-$(CONFIG_EFUSE_MODEM_AGENT)	+= efuse_layout.o
endif
obj-y	+= efuse_ioctl.o
obj-y	+= efuse_dump.o

GCOV_PROFILE := y