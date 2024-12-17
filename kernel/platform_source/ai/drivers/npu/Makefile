NPU_GLOBAL_CFLAGS := -Wextra -Wdate-time -Wfloat-equal -Wshadow -Wformat=2

NPU_CHIP_VERSION := $(strip $(subst ", ,$(CONFIG_VENDOR_CHIP_PREFIX)))
NPU_DIR :=$(srctree)/platform_source/ai/drivers/npu
NKD_DIR :=$(srctree)/platform_source/ai/drivers/npu/nkd
NPD_DIR :=$(srctree)/platform_source/ai/drivers/npu/npd

export NPU_CHIP_VERSION NPU_GLOBAL_CFLAGS NPU_DIR NKD_DIR NPD_DIR
ifneq ($(NPU_CHIP_VERSION),)
	include $(srctree)/platform_source/ai/drivers/npu/npd/$(NPU_CHIP_VERSION).mk
endif

ifeq ($(NPU_ARCH_VERSION),npu_v100)
	NPU_GLOBAL_CFLAGS += -DNPU_ARCH_V100
else
	NPU_GLOBAL_CFLAGS += -DCONFIG_NPU_HWTS
	ifeq ($(NPU_ARCH_VERSION),npu_v200)
		NPU_GLOBAL_CFLAGS += -DNPU_ARCH_V200
	else ifeq ($(NPU_ARCH_VERSION),npu_v210)
		NPU_GLOBAL_CFLAGS += -DNPU_ARCH_V210
	else ifeq ($(NPU_ARCH_VERSION),npu_v300)
		NPU_GLOBAL_CFLAGS += -DNPU_ARCH_V300
	endif
endif

ifneq ($(NPU_CUT_MAKEFILE), yes)
obj-$(CONFIG_NPU_KERNEL_DRIVER) += nkd/
obj-$(CONFIG_NPU_PLATFORM_DRIVER) += npd/
obj-$(CONFIG_NPU_BBIT_DEBUGFS) += bbit_debugfs/
obj-$(CONFIG_NPU_PCIE) += pcie/
obj-$(CONFIG_NPU_LOAD_TS) += load_ts/
endif

