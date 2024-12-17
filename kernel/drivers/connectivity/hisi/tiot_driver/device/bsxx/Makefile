# BSXX Makefile
ifeq ($(CONFIG_DEV_BSXX), y)
tiot-bsxx-objs += bsxx_device_info.o bsxx_service.o

ifeq ($(CONFIG_FILE_BY_ARRAY), y)
ifneq (,$(wildcard $(TIOT_DRIVER_PATH)/device/bsxx/firmware/bsxx_fw_file_array.c))
tiot-bsxx-objs += firmware/bsxx_fw_file_array.o
TIOT_HEADER_LIST += -I$(TIOT_DRIVER_PATH)/device/bsxx/firmware
endif
endif

tiot-bsxx-objs := $(addprefix device/bsxx/,$(tiot-bsxx-objs))
tiot_driver-objs += $(tiot-bsxx-objs)

TIOT_HEADER_LIST += -I$(TIOT_DRIVER_PATH)/device/bsxx
endif