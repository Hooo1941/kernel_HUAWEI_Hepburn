# Hisilicon IPP v350r02 Makefile
obj-$(CONFIG_HISPIPP_V350) += modules/ segment/ drivers/ dataflow/
obj-$(CONFIG_HISPIPP_V350) += memory.o

subdir-ccflags-$(CONFIG_HISPIPP_V350) += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow
ifeq ($(USE_BISHENG_CLANG),true)
subdir-ccflags-$(CONFIG_HISPIPP_V350) += -Wno-error=unused-but-set-variable
endif
subdir-ccflags-$(CONFIG_HISPIPP_V350) += -I$(srctree)/platform_source/isp/drivers/ipp/v350r02 \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v350r02/dataflow \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v350r02/segment \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v350r02/modules \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v350r02/platform \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v350r02/platform/reg_offset_field \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v350r02/include \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v350r02/include/config_table \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v350r02/include/dataflow \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v350r02/include/drivers \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v350r02/include/modules \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v350r02/include/segment \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v350r02/include/ippmessage_inc \
                                    -I$(srctree)/../../third_party/bounds_checking_function/include
ifneq ($(TARGET_BUILD_VARIANT),user)
subdir-ccflags-$(CONFIG_HISPIPP_V350) += -DCONFIG_IPP_DEBUG
endif
