# Hisilicon IPP Makefile

obj-$(CONFIG_HISPIPP_V230) += adapter_common.o
obj-$(CONFIG_HISPIPP_V230) += adapter_slam.o
obj-$(CONFIG_HISPIPP_V230) += adapter_slamtest.o
obj-$(CONFIG_HISPIPP_V230) += ipp_core.o
obj-$(CONFIG_HISPIPP_V230) += ipp_com.o
obj-$(CONFIG_HISPIPP_V230) += smc.o
obj-$(CONFIG_HISPIPP_V230) += memory.o

obj-$(CONFIG_HISPIPP_V230) += v230/
obj-$(CONFIG_HISPIPP_V230) += device/

subdir-ccflags-$(CONFIG_HISPIPP_V230) += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow
subdir-ccflags-$(CONFIG_HISPIPP_V230) += -I$(srctree)/platform_source/isp/drivers/ipp/v230 \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v230/device \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v230/v230 \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v230/include \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v230/v230/include \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v230/device/include \
                                    -I$(srctree)/../../third_party/bounds_checking_function/include
ifneq ($(TARGET_BUILD_VARIANT),user)
subdir-ccflags-$(CONFIG_HISPIPP_V230) += -DCONFIG_IPP_DEBUG
endif

