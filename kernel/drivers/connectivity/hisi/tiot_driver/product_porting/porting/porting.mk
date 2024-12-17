tiot-porting-objs += tiot_osal_port.o
tiot-porting-objs += tiot_board_uart_port.o
tiot-porting-objs += tiot_board_pin_irq_helper.o
tiot-porting-objs += tiot_board_pin_port.o
tiot-porting-objs += tiot_board_misc_port.o
tiot-porting-objs += bsxx_board_port.o
tiot-porting-objs += bsxx_misc_device.o
ifeq ($(CONFIG_FILE_BY_ARRAY), y)
ifneq (,$(wildcard $(TIOT_DRIVER_PATH)/product_porting/porting/firmware/bsxx_fw_file_array.c))
tiot-porting-objs += firmware/bsxx_fw_file_array.o
TIOT_HEADER_LIST += -I$(TIOT_DRIVER_PATH)/product_porting/porting/firmware
endif
endif