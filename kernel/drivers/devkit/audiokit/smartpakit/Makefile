# Makefile for the smartpa devkit drivers.
#

# Each configuration option enables a list of files.
ifeq ($(TARGET_BUILD_VARIANT),eng)
	ccflags-y += -DSMARTPAKIT_MISC_RW_PERMISSION_ENABLE
endif
ifdef CONFIG_HUAWEI_SMARTPAKIT_AUDIO
	ccflags-y += -Idrivers/devkit/audiokit/
	ccflags-y += -I$(srctree)/drivers/devkit/audiokit/
endif

ifdef CONFIG_CONTEXTHUB_IO_DIE_STS
	ccflags-y += -I$(srctree)/platform_source/smart/drivers/channels/io_die/
endif

SMARTPAKIT_OBJS += smartpakit.o
SMARTPAKIT_OBJS += smartpakit_i2c_ops.o
SMARTPAKIT_OBJS += high_power_smartpa/high_power_smartpa.o
ifeq ($(CONFIG_HIBERNATION), y)
    SMARTPAKIT_OBJS += smartpakit_i2c_pm.o
else ifeq ($(CONFIG_HUAWEI_ARMPC_PLATFORM), y)
    SMARTPAKIT_OBJS += smartpakit_i2c_pm.o
endif
ifneq ($(TARGET_BUILD_VARIANT),user)
       SMARTPAKIT_OBJS += smartpakit_info.o
endif
obj-$(CONFIG_HUAWEI_SMARTPAKIT_AUDIO) += smartpa.o
smartpa-y := $(SMARTPAKIT_OBJS)

obj-$(CONFIG_SND_SOC_AU_PA) += external_requirement.o
obj-y += high_power_smartpa/
