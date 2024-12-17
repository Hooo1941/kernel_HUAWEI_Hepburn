
-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk

subdir-ccflags-y := -Wall -Werror
ifeq ($(strip $(CFG_CONFIG_FIPC_V200)),YES)
obj-y += fipc_core.o
obj-y += fipc_device.o
obj-y += fipc_driver.o
obj-y += fipc_dump.o
obj-y += fipc_platform.o
obj-y += fipc_pmsr.o
obj-y += fipc_reset.o
obj-y += fipc_dts.o
obj-y += fipc_diff.o
ifeq ($(strip $(CFG_CONFIG_FIPC_SUPPORT_HIFI_RESET)),YES)
obj-y += fipc_reset_hifi.o
else
obj-y += fipc_reset_hifi_stub.o
endif
endif

GCOV_PROFILE := y
