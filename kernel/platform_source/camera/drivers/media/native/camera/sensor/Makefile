CAMERA_SOC_CHIP_NAME := $(strip $(subst ", ,$(CONFIG_VENDOR_CHIP_PREFIX)))

ifneq ($(TARGET_BUILD_VARIANT),user)
EXTRA_CFLAGS += -DDEBUG_KERNEL_CAMERA
endif
ccflags-y += -I$(srctree)/platform_source/camera/drivers/media/native/camera
ccflags-y += -I$(srctree)/platform_source/camera/drivers/media/native/camera/platform

ifeq (,$(filter $(CAMERA_SOC_CHIP_NAME), napa))
obj-y += camerafs.o
endif
obj-y += hwsensor.o flash/ vcm/ ois/ cam_misc/
