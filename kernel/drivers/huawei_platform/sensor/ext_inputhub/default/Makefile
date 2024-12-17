ccflags-y += -I$(srctree)/drivers/huawei_platform/sensor/ext_inputhub/default/bus_driver/
ccflags-y += -I$(srctree)/drivers/huawei_platform/sensor/ext_inputhub/default/commu_handle/
ccflags-y += -I$(srctree)/drivers/huawei_platform/sensor/ext_inputhub/default/inner_cmd/

#ext sensorhub Driver
obj-$(CONFIG_EXT_INPUTHUB) += ext_sensorhub_channel.o
obj-$(CONFIG_EXT_INPUTHUB) += ext_sensorhub_api.o
obj-$(CONFIG_EXT_INPUTHUB) += ext_sensorhub_route.o
obj-$(CONFIG_EXT_INPUTHUB) += ext_sensorhub_frame.o
obj-$(CONFIG_EXT_INPUTHUB) += crc16.o
obj-$(CONFIG_EXT_INPUTHUB) += ext_sensorhub_commu.o
obj-$(CONFIG_EXT_INPUTHUB) += ext_sensorhub_debug.o
obj-$(CONFIG_EXT_INPUTHUB) += bus_driver/
obj-$(CONFIG_EXT_INPUTHUB) += commu_handle/
obj-$(CONFIG_EXT_INPUTHUB) += inner_cmd/

ifneq ($(CONFIG_EXT_INPUTHUB_QCOM), y)
obj-$(CONFIG_EXT_INPUTHUB) += ext_motion_channel.o
endif