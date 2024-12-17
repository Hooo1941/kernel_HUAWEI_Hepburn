SENSORHUB_SOC_CHIP_NAME := $(strip $(subst ", ,$(CONFIG_VENDOR_CHIP_PREFIX)))

ifeq ($(wildcard $(srctree)/drivers/huawei_platform/inputhub/$(SENSORHUB_SOC_CHIP_NAME)),)
ccflags-y  += -I$(srctree)/drivers/huawei_platform/inputhub/default/
else
ccflags-y  += -I$(srctree)/drivers/huawei_platform/inputhub/$(SENSORHUB_SOC_CHIP_NAME)/
endif

obj-$(CONFIG_CONTEXTHUB) += inputhub_api/inputhub_api.o

ifneq ($(CONFIG_INPUTHUB_20_970),y)
obj-$(CONFIG_CONTEXTHUB) += timestamp/timestamp.o
endif

ifneq ($(CONFIG_INPUTHUB_30),y)
obj-$(CONFIG_CONTEXTHUB_SHMEM) += shmem/shmem.o
endif
obj-$(CONFIG_CONTEXTHUB_SHELL) += shell_dbg/shell_dbg.o
obj-$(CONFIG_CONTEXTHUB_LOADMONITOR) += loadmonitor/loadmonitor.o
obj-$(CONFIG_CONTEXTHUB_PROFILE) += profile/profile_channel.o
obj-$(CONFIG_CONTEXTHUB_PLAYBACK) += dbg/playback.o
obj-$(CONFIG_CONTEXTHUB_CHRE) += chre/chre.o
obj-$(CONFIG_CONTEXTHUB)   += flp/
obj-$(CONFIG_CONTEXTHUB_IGS)   += igs/
obj-$(CONFIG_CONTEXTHUB_UDI) += dump_inject/dump_inject.o
obj-$(CONFIG_CONTEXTHUB_BLE) += ble/ble_channel.o
obj-$(CONFIG_CONTEXTHUB_IO_DIE_STS) += io_die/sts.o
ifneq ($(CONFIG_IO_DIE_NO_EXTERN),y)
obj-y += io_die/sts_extern.o
endif
ifeq ($(TARGET_VERSION_MODE),factory)
obj-$(CONFIG_IO_DIE_STS_DEBUG) += io_die/iodie_dbg.o
endif
obj-$(CONFIG_SENSOR_DIRECT_CHANNEL) += sdc/sdc.o
obj-$(CONFIG_CONTEXTHUB_IO_DIE_PCIE) += io_die/pcie/int_parse.o
obj-$(CONFIG_CONTEXTHUB_IO_DIE_PCIE) += io_die/pcie/iodie_pcie.o
ifeq ($(CONFIG_DFX_DEBUG_FS),y)
obj-$(CONFIG_CONTEXTHUB_IO_DIE_PCIE) += io_die/pcie/iodie_pcie_test.o
endif

