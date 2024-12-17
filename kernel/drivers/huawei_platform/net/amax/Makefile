obj-$(CONFIG_HW_WIFI_CLOCK)  += wifi_clock/
obj-y += \
	amax_multilink_interface.o \
	amax_multilink_dev.o \

obj-$(CONFIG_HW_WIFI_AMAX) += amax.o

ifeq ($(CONFIG_HW_WIFI_AMAX), m)
CONFIG_HW_WIFI_INSMOD_AMAX := y
else
CONFIG_HW_WIFI_INSMOD_AMAX :=
endif

amax-objs := amax_multilink_main.o amax_multilink_stat.o amax_multilink_tool.o dps_data_txrx.o