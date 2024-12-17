# hwcxext codec module pc

ifdef CONFIG_HWCXEXT_CODEC_MODULE_PC
	ccflags-y += -Idrivers/huawei_platform/audio/hwcxext_codec_pc
	ccflags-y += -Idrivers/devkit/audiokit/
	ccflags-y += -I$(srctree)/drivers/devkit/audiokit/
	ccflags-y += -Idrivers/hisi/tzdriver
endif

ifneq ($(TARGET_BUILD_VARIANT),user)
	ccflags-y += -DHWCXEXT_CODEC_INFO_PERMISSION_ENABLE
endif

obj-$(CONFIG_HWCXEXT_CODEC_MODULE_PC) += hwcxext_log.o
obj-$(CONFIG_HWCXEXT_CODEC_INTERFACE_PC) += hwcxext_codec_interface.o
obj-$(CONFIG_HWCXEXT_MBHC_PC) += hwcxext_mbhc.o
obj-$(CONFIG_HWCXEXT_I2C_OPS_PC) += hwcxext_i2c_ops.o
obj-$(CONFIG_HWCXEXT_CODEC_V1_PC) += hwcxext_codec_v1.o
ifneq ($(TARGET_BUILD_VARIANT),user)
	obj-$(CONFIG_HWCXEXT_CODEC_MODULE_PC) += hwcxext_codec_info.o
endif