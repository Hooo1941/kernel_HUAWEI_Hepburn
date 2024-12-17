LP_SOC_CHIP_NAME := $(strip $(subst ", ,$(CONFIG_VENDOR_CHIP_PREFIX)))

SOC_PG_FEATURE_DIR :=$(srctree)/platform_source/basicplatform/drivers/soc_pg_feature_modem/include
ccflags-y += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow

ifeq ($(chip_type), )
subdir-ccflags-$(CONFIG_SOC_PG_FEATURE_DEVDRV_DEVPLATFORM) += -I$(SOC_PG_FEATURE_DIR)/$(LP_SOC_CHIP_NAME)
else
subdir-ccflags-$(CONFIG_SOC_PG_FEATURE_DEVDRV_DEVPLATFORM) += -I$(SOC_PG_FEATURE_DIR)/$(LP_SOC_CHIP_NAME)_$(chip_type)
endif

ifneq ($(LP_SOC_CHIP_NAME),)
  obj-$(CONFIG_SOC_PG_FEATURE_DEVDRV_DEVPLATFORM) += $(LP_SOC_CHIP_NAME)/soc_pg_feature_adapt.o
endif
