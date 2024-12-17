#
# Makefile for the memory technology device drivers.
#

# Core functionality.
-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk

ifeq ($(strip $(CFG_CONFIG_CHR_FS)),YES)
obj-y      += chr_report.o
obj-y      += chr_core.o
obj-y      += chr_cfg.o
obj-y      += chr_debug.o
else
obj-y      += chr_stub.o
endif

GCOV_PROFILE := y