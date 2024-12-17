FBE_SOC_CHIP_NAME := $(strip $(subst ", ,$(CONFIG_VENDOR_CHIP_PREFIX)))
FBE_CHIP_TYPE := $(strip $(subst ", , $(CONFIG_VENDOR_CHIP_SUFFIX)))
FBE_CHIP_VERSION := $(FBE_SOC_CHIP_NAME)_$(FBE_CHIP_TYPE)

ifeq ($(FBE_CHIP_VERSION), charlotte_csv2)
ccflags-$(CONFIG_FILE_BASED_ENCRYPTO) += -DCONFIG_FBE3_1
ifeq ($(CONFIG_HIBERNATION),y)
ccflags-$(CONFIG_FILE_BASED_ENCRYPTO) += -DCONFIG_FBE_S4
endif
endif

ifeq ($(FBE_CHIP_VERSION), charlotte_cs)
ccflags-$(CONFIG_FILE_BASED_ENCRYPTO) += -DCONFIG_FBE_MSPC_ABSENT
endif


ifeq ($(CONFIG_FBE3_1), y)
ccflags-y += -I$(srctree)/drivers/platform_drivers/tzdriver \
	     -I$(srctree)/platform_source/basicplatform/drivers/rpmb
else ifeq ($(FBE_CHIP_VERSION), charlotte_csv2)
ccflags-y += -I$(srctree)/drivers/platform_drivers/tzdriver \
	     -I$(srctree)/platform_source/basicplatform/drivers/rpmb
else
ccflags-y += -I$(srctree)/drivers/platform_drivers/tzdriver \
	     -I$(srctree)/platform_source/see/drivers/crypto_core
endif

ccflags-y += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow

obj-$(CONFIG_FILE_BASED_ENCRYPTO) += fbex_dev.o \
	fbex_driver.o fbex_screen_handle.o fbex_preload.o
obj-$(CONFIG_FILE_BASED_ENCRYPTO_FOR_HM_FSCRYPTO) += fbex_fscrypto.o
obj-$(CONFIG_FILE_BASED_ENCRYPTO_DBG) += fbex_debug.o
