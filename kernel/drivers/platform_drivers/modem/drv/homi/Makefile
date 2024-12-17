ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk

KBUILD_CPPFLAGS += -Idrivers/platform_drivers/modem/drv/homi

obj-y += homi_core.o
obj-y += homi_debug.o
obj-y += homi_interface.o
obj-y += homi_usb.o
obj-y += homi_socket.o
obj-y += homi_netlink.o
obj-y += homi_genl.o


GCOV_PROFILE := y
GCOV_PROFILE_homi_debug.o := n
GCOV_PROFILE_homi_netlink.o := n