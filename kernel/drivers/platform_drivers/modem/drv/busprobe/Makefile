
-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk

subdir-ccflags-y := -Werror
obj-y           += qic_dump_register.o

ifeq ($(strip $(CFG_CONFIG_AP_NOC)), YES)
obj-y           += noc_dump_register.o
endif
