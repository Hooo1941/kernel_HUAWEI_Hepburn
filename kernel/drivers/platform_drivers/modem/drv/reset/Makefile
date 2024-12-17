-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk

ifeq ($(strip $(CFG_CONFIG_MODEM_RESET)), YES)
obj-y       += reset_frame.o
obj-y       += reset_debug.o
else
obj-y       += reset_stub.o
endif
obj-y       += reset_dev.o

EXTRA_CFLAGS += -Idrivers/platform_drivers/tzdriver \
                -Idrivers/tzdriver

subdir-ccflags-y := -Werror

GCOV_PROFILE := y