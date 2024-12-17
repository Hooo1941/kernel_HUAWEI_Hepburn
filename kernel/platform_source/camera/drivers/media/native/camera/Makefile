
ccflags-y += -Wextra -Wdate-time -Wfloat-equal -Wshadow
ifneq ($(TARGET_BUILD_VARIANT),user)
EXTRA_CFLAGS += -DDEBUG_KERNEL_CAMERA
endif

obj-y += cam_usrdev.o cam_cfgdev.o

CFLAGS_cam_cfgdev.o = -I$(src)


obj-y += sensor/
obj-y += hisp/
obj-y += pmic/
obj-y += platform/
obj-y += laser/
obj-y += hjpeg/
obj-y += clt/
obj-y += fd/
obj-y += cam_buf/
obj-y += cam_buf_v3/
obj-$(CONFIG_KERNEL_CAMERA_BUCK) += buck/
obj-$(CONFIG_KERNEL_CAMERA_FPGA) += fpga/
obj-$(CONFIG_KERNEL_CAMERA_DRIVER_IC) += driver_ic/
obj-$(CONFIG_KERNEL_CAMERA_DOT_PROJECTOR) += dot/
obj-$(CONFIG_CAM_IDI2AXI) += idi2axi/
obj-$(CONFIG_CAM_IDI2AXI_SENSOR) += idi2axi_sensor/
obj-$(CONFIG_KERNEL_CAMERA_SENSORMOTOR) += cam_star_module/
obj-$(CONFIG_KERNEL_CAMERA_CONTAINER) += cam_sync.o

