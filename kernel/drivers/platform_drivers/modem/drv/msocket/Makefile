ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk

KBUILD_CPPFLAGS += -Idrivers/platform_drivers/modem/drv/msocket

obj-y += msocket.o

GCOV_PROFILE := y