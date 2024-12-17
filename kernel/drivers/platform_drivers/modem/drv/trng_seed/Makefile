-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk
ifeq ($(strip $(CFG_CONFIG_TRNG_SEED)),YES)
obj-y += trng_seed.o
endif

GCOV_PROFILE := y