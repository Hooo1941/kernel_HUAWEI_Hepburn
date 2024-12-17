-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk

ifeq ($(strip $(CFG_CONFIG_MODEM_PINCTRL)),YES)
obj-y += pinctrl_driver.o
endif

GCOV_PROFILE := y