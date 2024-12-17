#
# Makefile for bootdevice
#
obj-$(CONFIG_BOOTDEVICE) += bootdevice.o proc_boot.o

ifeq ($(TARGET_VERSION_MODE),factory)
EXTRA_CFLAGS += -DUFS_BOOTROM_FACTORY_MODE
EXTRA_CFLAGS += -DUFS_ANONYMOUS_FACTORY_MODE
endif
