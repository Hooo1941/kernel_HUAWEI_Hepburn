#
# Makefile for hisilicon camera fpga driver
#
EXTRA_CFLAGS += -I$(srctree)/platform_source/camera/drivers/media/native/camera/
EXTRA_CFLAGS += -I$(srctree)/platform_source/camera/drivers/media/native/camera/cam_star_module/
EXTRA_CFLAGS += -I$(srctree)/platform_source/camera/drivers/media/native/camera/cam_star_module/stm32g031/
EXTRA_CFLAGS += -I$(srctree)/platform_source/camera/drivers/media/native/camera/cam_star_module/hc32l110/
EXTRA_CFLAGS += -fno-pic

obj-$(CONFIG_KERNEL_CAMERA_SENSORMOTOR) += cam_star_module_adapter.o
obj-$(CONFIG_KERNEL_CAMERA_SENSORMOTOR) += cam_sensormotor_i2c.o
obj-$(CONFIG_KERNEL_CAMERA_SENSORMOTOR) += cam_sensormotor_dmd_report.o
obj-$(CONFIG_KERNEL_CAMERA_SENSORMOTOR) += stm32g031/
obj-$(CONFIG_KERNEL_CAMERA_SENSORMOTOR) += hc32l110/