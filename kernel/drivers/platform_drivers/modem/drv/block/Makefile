-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk

EXTRA_CFLAGS += -Idrivers/platform_drivers/tzdriver

obj-y += blk_base.o
ifeq ($(strip $(CFG_BSP_CONFIG_PHONE_TYPE)),YES)
obj-y += mmc/mmc_ops.o
else
obj-y += blk_logic.o
obj-y += fmc/fmc_ops.o
obj-y += fmc/fmc_blk.o
obj-y += mmc/mmc_ops.o
obj-y += mmc/mmc_blk.o
endif

GCOV_PROFILE := y