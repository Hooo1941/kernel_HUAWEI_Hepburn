ccflags-y += -I$(srctree)/drivers/huawei_platform/sensor/ext_inputhub/default/
ccflags-y += -I$(srctree)/drivers/huawei_platform/sensor/ext_inputhub/default/commu_handle/

ifneq ($(CONFIG_EXT_INPUTHUB_QCOM), y)
obj-$(CONFIG_EXT_INPUTHUB) += ext_sensorhub_sdio.o
obj-$(CONFIG_EXT_INPUTHUB) += ext_sensorhub_spi.o
obj-$(CONFIG_EXT_INPUTHUB) += ext_sensorhub_uart.o
else
obj-$(CONFIG_EXT_INPUTHUB) += ext_sensorhub_spi_slave.o
obj-$(CONFIG_EXT_INPUTHUB) += ext_sensorhub_uart.o
endif