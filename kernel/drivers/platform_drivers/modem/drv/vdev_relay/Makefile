subdir-ccflags-y := -Werror
-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk

obj-y += relay_acm.o
ifeq ($(CONFIG_IOT_GNET), y)
obj-y += relay_net.o
endif
obj-y += relay_core.o
ifeq ($(findstring y, $(CONFIG_DIALUP_UART) $(CONFIG_DIALUP_PL011_UART)), y)
obj-y += relay_uart.o
endif
obj-y += bsp_relay.o

GCOV_PROFILE := y
