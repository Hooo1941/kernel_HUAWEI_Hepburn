
-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk

subdir-ccflags-y := -Werror
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/tzdriver
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/tzdriver_hm
obj-$(CONFIG_SEC_CALL) += sec_call.o

GCOV_PROFILE := y