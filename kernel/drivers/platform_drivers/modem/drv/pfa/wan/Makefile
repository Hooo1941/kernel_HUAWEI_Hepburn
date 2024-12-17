
-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk
KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/tft/
KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/core/
KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/modem_reset/

ifeq ($(strip $(CFG_CONFIG_PFA_V200)),YES)
KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/hal/v200/
else ifeq ($(strip $(CFG_CONFIG_PFA_V100)),YES)
KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/hal/v100/
else
KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/hal/v0/
endif


obj-y	+= wan.o
ifeq ($(strip $(CFG_CONFIG_PFA_V200)),YES)
obj-y	+= lan_stub.o
else ifeq ($(strip $(CFG_CONFIG_PFA_V100)),YES)
obj-y	+= lan_stub.o
else
obj-y	+= lan.o
endif
subdir-ccflags-y := -Werror

GCOV_PROFILE := y