obj-y	+= lcd_kit_adapt.o
obj-y	+= lcd_kit_sysfs_dkmd.o

ifeq ($(TARGET_VERSION_MODE),factory)
obj-y   += lcd_kit_sysfs_fac.o
obj-y   += lcd_kit_fac.o
EXTRA_CFLAGS += -DLCD_FACTORY_MODE
endif

EXTRA_CFLAGS += -I$(srctree)/include/platform_include/display/dkmd \
		-I$(srctree)/drivers/devkit/lcdkit/lcdkit3.0/kernel/common/include \
		-I$(srctree)/drivers/devkit/lcdkit/lcdkit3.0/kernel/dkmd/include
