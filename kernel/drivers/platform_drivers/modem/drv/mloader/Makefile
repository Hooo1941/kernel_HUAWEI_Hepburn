
-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk

subdir-ccflags-y := -Werror
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/tzdriver_hm

ifeq ($(strip $(CFG_CONFIG_MLOADER)),YES)
obj-y	+= mloader_util.o
obj-y	+= mloader_sec_call.o
obj-y	+= mloader_load_image.o
ifeq ($(strip $(CFG_BSP_CONFIG_PHONE_TYPE)),YES)
obj-y	+= mloader_load_lpmcu.o
endif
obj-y	+= mloader_load_modem.o
obj-y	+= mloader_load_patch.o
obj-y	+= mloader_debug.o
obj-y	+= mloader_msg.o
obj-y	+= mloader_main.o
endif

GCOV_PROFILE := y