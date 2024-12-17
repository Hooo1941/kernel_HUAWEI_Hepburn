-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk
subdir-ccflags-y += -I$(srctree)/lib/libc_sec/securec_v2/include/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/tzdriver/libhwsecurec/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/product/$(OBB_PRODUCT_NAME)/include/platform/drv/acore/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/product/$(OBB_PRODUCT_NAME)/include/platform/drv/common/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/product/$(OBB_PRODUCT_NAME)/include/platform/pub/drv/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/product/$(OBB_PRODUCT_NAME)/include/platform/drv/ccore/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/adrv/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/drv/acore/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/drv/common/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/include/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/common/include/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/include/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nva/comm/

ifeq ($(strip $(OBB_LLT_MDRV)),y)
subdir-ccflags-y += -DDRV_BUILD_LLT
endif
ifeq ($(LLT_BUILD_GCOV), y)
subdir-ccflags-y += -DDRV_BUILD_GCOV
endif

obj-$(CONFIG_MODEM_DRIVER) +=drv/

ifneq ($(chip_type),es)
KBUILD_CFLAGS += -DCHIP_TYPE_CS=FEATURE_ON
endif
KBUILD_CFLAGS += -DMODULE_EXPORTED=

ifeq ($(choose_variant_modem),user)
KBUILD_CFLAGS += -DCHOOSE_MODEM_USER
endif


ifeq ($(strip $(CONFIG_MODEM_DRIVER)),m)

MODEM_DRIVER_MODULE := modem_driver
$(MODEM_DRIVER_MODULE)-y :=


# overwrite obj-m
obj-m	:= $(MODEM_DRIVER_MODULE).o

subdir-ccflags-y += -DCONFIG_MODEM_DRIVER -Wno-unused-function -Wno-undef

# drv
include $(srctree)/drivers/platform_drivers/modem/drv/Makefile
$(MODEM_DRIVER_MODULE)-y += $(addprefix drv/, $(drv-y))
obj-y	+= $(addprefix drv/, $(drv-builtin))
endif
