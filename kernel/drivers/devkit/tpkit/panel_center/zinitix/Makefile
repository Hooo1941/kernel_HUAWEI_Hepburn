#drivers/input/touchscreen
#
#Makefile for the touch driver of Zinitix
ifeq ($(CONFIG_HUAWEI_DEVKIT_HISI),y)
obj-$(CONFIG_ZINITIX_TS_KIT) += zinitix_ts.o
zinitix_ts-objs += zinitix_core.o zinitix_dts.o #zinitix_test.o zinitix_flash.o
EXTRA_CFLAGS += -I$(srctree)/drivers/devkit/tpkit
EXTRA_CFLAGS += -I$(srctree)/drivers/huawei_platform/sensor/ext_inputhub/default/
else
obj-$(CONFIG_ZINITIX_TS_KIT) += zinitix_ts.o
zinitix_ts-objs += zinitix_core.o zinitix_dts.o #zinitix_test.o zinitix_flash.o
EXTRA_CFLAGS += -Idrivers/devkit/tpkit
endif
