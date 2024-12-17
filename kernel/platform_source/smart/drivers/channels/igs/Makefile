SENSORHUB_SOC_CHIP_NAME := $(strip $(subst ", ,$(CONFIG_VENDOR_CHIP_PREFIX)))

ifeq ($(wildcard $(srctree)/drivers/huawei_platform/inputhub/$(SENSORHUB_SOC_CHIP_NAME)),)
ccflags-y  += -I$(srctree)/drivers/huawei_platform/inputhub/default/
else
ccflags-y  += -I$(srctree)/drivers/huawei_platform/inputhub/$(SENSORHUB_SOC_CHIP_NAME)/
endif
EXTRA_CFLAGS += -Idrivers/platform_drivers/tzdriver

ccflags-y  += -I$(srctree)/platform_source/smart/drivers/channels/

obj-$(CONFIG_CONTEXTHUB_IGS) += igs_dev.o
obj-$(CONFIG_CONTEXTHUB_IGS) += igs_ion.o
obj-$(CONFIG_CONTEXTHUB_IGS_DBG) += igs_dbg_dev.o
obj-$(CONFIG_CONTEXTHUB_IGS_TOF) += tof/igs_tof_dev.o
