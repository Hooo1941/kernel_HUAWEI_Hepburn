#gcov
ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nva/comm
ifeq ($(strip $(CFG_FEATURE_NVA_ON)),YES)
    obj-y    += nva_modem.o
    obj-y    += nva_partition.o
endif

GCOV_PROFILE := y