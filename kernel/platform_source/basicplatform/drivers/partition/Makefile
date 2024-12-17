ifeq ($(CONFIG_LIBLINUX),y)
export CONFIG_VENDOR_CHIP_PREFIX=$(TARGET_BOARD_PLATFORM)
endif
ifneq ($(CONFIG_PARTITION_ARMPC),y)
CONFIG_PARTITION_PLATFORM := $(strip $(subst ", ,$(CONFIG_VENDOR_CHIP_PREFIX)))
subdir-ccflags-y += -I$(srctree)/include
subdir-ccflags-y += -I$(srctree)/include/platform_include/basicplatform/linux/partition
subdir-ccflags-y += -I$(srctree)/include/platform_include/basicplatform/linux/partition/plat/$(CONFIG_PARTITION_PLATFORM)
obj-$(CONFIG_PARTITION_TABLE) += partition_ap_kernel.o
else
obj-$(CONFIG_PARTITION_TABLE) += partition_ap_kernel_armpc.o
endif
