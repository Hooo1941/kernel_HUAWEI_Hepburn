# Hisilicon IPP v500 Makefile
obj-$(CONFIG_HISPIPP_V500) += modules/ segment/ drivers/ dataflow/
obj-$(CONFIG_HISPIPP_V500) += memory.o

subdir-ccflags-$(CONFIG_HISPIPP_V500) += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow
ifeq ($(USE_BISHENG_CLANG),true)
subdir-ccflags-$(CONFIG_HISPIPP_V500) += -Wno-error=unused-but-set-variable
endif
subdir-ccflags-$(CONFIG_HISPIPP_V500) += -I$(srctree)/platform_source/isp/drivers/ipp/v500 \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v500/dataflow \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v500/segment \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v500/modules \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v500/platform \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v500/platform/reg_offset_field \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v500/include \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v500/include/config_table \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v500/include/dataflow \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v500/include/drivers \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v500/include/modules \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v500/include/segment \
                                    -I$(srctree)/platform_source/isp/drivers/ipp/v500/include/ippmessage_inc \
                                    -I$(srctree)/../../third_party/bounds_checking_function/include
ifneq ($(TARGET_BUILD_VARIANT),user)
subdir-ccflags-$(CONFIG_HISPIPP_V500) += -DCONFIG_IPP_DEBUG
endif
