ccflags-y += -I$(srctree)/platform_source/camera/drivers/media/native/camera/idi2axi
ccflags-y += -I$(srctree)/platform_source/camera/drivers/media/native/camera/idi2axi/include
ccflags-y += -I$(srctree)/pplatform_source/camera/drivers/media/native/camera/idi2axi/platform/include
ccflags-y += -I$(srctree)/platform_source/camera/drivers/media/native/camera

subdir-ccflags-y += -I$(srctree)/platform_source/camera/drivers/media/native/camera/idi2axi
subdir-ccflags-y += -I$(srctree)/platform_source/camera/drivers/media/native/camera/idi2axi/include
subdir-ccflags-y += -I$(srctree)/platform_source/camera/drivers/media/native/camera/idi2axi/platform/include

cam_idi2axi-y := idi2axi_device.o

ifeq ($(CONFIG_CAM_IDI2AXI), m)
# IF USE MODULE MODE, PLEASE INCLUDE THE USED SENSOR DRIVER DIRECTLY, '+= directory' IS NOT OK!
cam_idi2axi-$(CONFIG_IDI2AXI_V100) += platform/v100/plat_csi2if.o platform/v100/plat_idi2axi.o
else
obj-$(CONFIG_IDI2AXI_V100) += platform/v100/
endif

cam_idi2axi-$(CONFIG_CAM_IDI2AXI_DEBUG) += i2a_debug.o
cam_idi2axi-y += idi2axi_channel.o
cam_idi2axi-y += i2a_csi2if.o
cam_idi2axi-y += i2a_atf.o
cam_idi2axi-y += i2a_power.o
cam_idi2axi-y += i2a_helper.o
cam_idi2axi-y += i2a_mgr.o
obj-$(CONFIG_CAM_IDI2AXI) += cam_idi2axi.o
