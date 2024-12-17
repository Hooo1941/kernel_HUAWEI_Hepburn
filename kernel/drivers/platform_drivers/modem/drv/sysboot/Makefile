-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk

subdir-ccflags-y := -Werror
subdir-ccflags-y += 	-Idrivers/platform_drivers/tzdriver
subdir-ccflags-y += 	-Idrivers/platform_drivers/tzdriver_hm

obj-y			+= sysboot_para.o

ifeq ($(strip $(BSP_CONFIG_PHONE_TYPE)), YES)
obj-y			+= sysboot_main.o
obj-y			+= sysboot_load_modem_teeos.o
obj-y			+= sysboot_sec_call.o
else
obj-y			+= oeminfo/
endif

GCOV_PROFILE := y