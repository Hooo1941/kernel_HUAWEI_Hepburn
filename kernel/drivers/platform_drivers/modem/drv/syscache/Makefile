
subdir-ccflags-y := -Werror

-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk
ifeq ($(strip $(CFG_CONFIG_SC_GID_CTRL)),YES)
obj-y           += sc_gid_ctrl.o sc_gid_ctrl_hal.o
endif

ifeq ($(strip $(CFG_CONFIG_SYSCACHE)),YES)
obj-y           += sc_main.o sc_hal.o
endif

GCOV_PROFILE := y
GCOV_PROFILE_dra_debug.o := n
