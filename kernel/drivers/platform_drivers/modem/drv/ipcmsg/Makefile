-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk

subdir-ccflags-y := -Wall -Werror
ifeq ($(strip $(CFG_CONFIG_MODULE_IPCMSG)),YES)
obj-y += ipcmsg_core.o
obj-y += ipcmsg_device.o
obj-y += ipcmsg_driver.o

ifneq ($(strip $(CFG_CONFIG_MODULE_IPC_FUSION)),YES)
obj-y += ipc_resource.o
obj-y += ipc_interrupt.o
endif
else
obj-y += ipcmsg_stub.o
endif

GCOV_PROFILE := y
GCOV_PROFILE_ipcmsg_stub.o := n