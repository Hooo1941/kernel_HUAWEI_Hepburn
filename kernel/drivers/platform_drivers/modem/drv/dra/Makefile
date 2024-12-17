
subdir-ccflags-y := -Werror

-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk
ifeq ($(strip $(CFG_CONFIG_DRA)),YES)
obj-y           += dra_core.o bsp_dra.o dra_plat.o dra_debug.o
ifeq ($(strip $(CFG_CONFIG_DRA_V3)),YES)
obj-y           += dra_hal.o
endif
endif

GCOV_PROFILE := y
GCOV_PROFILE_dra_debug.o := n