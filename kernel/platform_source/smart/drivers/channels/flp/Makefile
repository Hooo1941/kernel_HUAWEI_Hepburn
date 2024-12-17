#
# FLP PDR configuration
#
SENSORHUB_SOC_CHIP_NAME := $(strip $(subst ", ,$(CONFIG_VENDOR_CHIP_PREFIX)))

ccflags-$(CONFIG_EXT_INPUTHUB) += -I$(srctree)/drivers/huawei_platform/sensor/ext_inputhub/default
ifeq ($(wildcard $(srctree)/drivers/huawei_platform/inputhub/$(SENSORHUB_SOC_CHIP_NAME)),)
ccflags-y  += -I$(srctree)/drivers/huawei_platform/inputhub/default/
else
ccflags-y  += -I$(srctree)/drivers/huawei_platform/inputhub/$(SENSORHUB_SOC_CHIP_NAME)/
endif

ccflags-y  += -I$(srctree)/platform_source/smart/drivers/channels/

EXTRA_CFLAGS += -Idrivers/platform_drivers/tzdriver
obj-$(CONFIG_SMART_SOFTTIMER) += smart_softtimer.o
obj-$(CONFIG_SMART_AR) += smart_ar.o
obj-$(CONFIG_SMART_FLP) += smart_flp.o
obj-$(CONFIG_SMART_FLP) += smart_flp_dt.o
obj-$(CONFIG_EXT_INPUTHUB) += smart_1135_ar.o
