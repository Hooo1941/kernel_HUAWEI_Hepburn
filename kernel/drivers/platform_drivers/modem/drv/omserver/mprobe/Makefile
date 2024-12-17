-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk

KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/omserver/mprobe

obj-y += mprobe_init.o
obj-y += mprobe_channel_manager.o
obj-y += mprobe_message.o
obj-y += mprobe_port.o
obj-y += mprobe_service.o
obj-y += mprobe_debug.o
GCOV_PROFILE := y
GCOV_PROFILE_mprobe_channel_manager.o := n
GCOV_PROFILE_mprobe_service.o := n
GCOV_PROFILE_mprobe_debug.o := n
GCOV_PROFILE_mprobe_port.o := n
GCOV_PROFILE_mprobe_init.o := n
GCOV_PROFILE_mprobe_message.o := n