
-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk
ifeq ($(strip $(CFG_CONFIG_MODULE_IPC_FUSION)), YES)
obj-y += ipc_fusion.o
obj-y += ipc_fusion_test.o
else
ifneq ($(strip $(CFG_CONFIG_MODULE_IPCMSG)),YES)
obj-y += ipc_stub.o
endif
endif

subdir-ccflags-y := -Werror

GCOV_PROFILE := y
GCOV_PROFILE_ipc_fusion_test.o := n