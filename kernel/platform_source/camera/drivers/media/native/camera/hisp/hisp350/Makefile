
ccflags-y += -Wextra -Wdate-time -Wfloat-equal -Wshadow
ccflags-y += -I$(srctree)/platform_source/camera/drivers/media/native/camera/hisp
ccflags-y += -I$(srctree)/platform_source/camera/drivers/media/native/camera
ccflags-y += -I$(srctree)/drivers/platform_drivers/ap/platform/common

ifeq ($(strip $(subst ", , $(CONFIG_VENDOR_CHIP_SUFFIX))),es)
EXTRA_CFLAGS += -DCAM_CHIP_TYPE_ES
endif
EXTRA_CFLAGS += -DHISP350_CAMERA
obj-y += hisp350.o
