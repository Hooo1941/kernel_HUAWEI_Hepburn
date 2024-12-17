EXTRA_CFLAGS += $(NPU_GLOBAL_CFLAGS)
EXTRA_CFLAGS += -I$(NKD_DIR)/inc/comm
EXTRA_CFLAGS += -I$(NKD_DIR)/inc
EXTRA_CFLAGS += -I$(NKD_DIR)/inc/model
EXTRA_CFLAGS += -I$(NKD_DIR)/core
EXTRA_CFLAGS += -I$(NKD_DIR)/config
EXTRA_CFLAGS += -I$(NKD_DIR)/format_convertor
EXTRA_CFLAGS += -I$(NKD_DIR)/heartbeat
EXTRA_CFLAGS += -I$(NKD_DIR)/device/stream_manager
EXTRA_CFLAGS += -I$(NKD_DIR)/atf
EXTRA_CFLAGS += -I$(NKD_DIR)/memory
EXTRA_CFLAGS += -I$(NKD_DIR)/communication
EXTRA_CFLAGS += -I$(NKD_DIR)/manager
EXTRA_CFLAGS += -I$(NKD_DIR)/dfx
EXTRA_CFLAGS += -I$(NKD_DIR)/dfx/bbox
EXTRA_CFLAGS += -I$(NKD_DIR)/dts
EXTRA_CFLAGS += -I$(NKD_DIR)/id_allocator
EXTRA_CFLAGS += -I$(NKD_DIR)/
EXTRA_CFLAGS += -I$(NPD_DIR)/$(NPU_ARCH_VERSION)
EXTRA_CFLAGS += -I$(NPD_DIR)/$(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)
EXTRA_CFLAGS += -I$(NPD_DIR)/
EXTRA_CFLAGS += -I$(NKD_DIR)/lowpower
EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/ap/platform/$(NPU_CHIP_VERSION)/
EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/ap/platform/common/
EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/ap/kernel/drivers/tzdriver/libhwsecurec/
EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/ap/kernel/include/
EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/ap/kernel/
EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/ap/kernel/include/linux/platform_drivers/
EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/ap/kernel/platform_source/basicplatform/drivers/iommu/smmu/
EXTRA_CFLAGS += -I$(srctree)/platform_source/ffrt/uniform_id
EXTRA_CFLAGS += -I$(NPU_DIR)/bbit_debugfs
ifeq ($(CONFIG_ARM_FFA_TRANSPORT),y)
ifeq ($(CONFIG_NPU_FFA_SUPPORT),y)
EXTRA_CFLAGS += -I$(srctree)/include/platform_include/see/ffa/
endif
endif

ifeq ($(CONFIG_NPU_SWTS),y)
	EXTRA_CFLAGS += -I$(NKD_DIR)/schedule/
	EXTRA_CFLAGS += -I$(NKD_DIR)/dfx/profiling
endif

ifeq ($(NPU_ARCH_VERSION),npu_v100)
	EXTRA_CFLAGS += -I$(NKD_DIR)/power_manager/v100
else
	EXTRA_CFLAGS += -I$(NKD_DIR)/power_manager
endif

obj-$(CONFIG_NPU_PLATFORM_DRIVER) += npd.o

npd-objs := npd_main.o npu_pm_internal.o
npd-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_platform_pm.o
npd-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_adapter.o
npd-objs += $(NPU_ARCH_VERSION)/npu_arch_exception.o
npd-objs += $(NPU_ARCH_VERSION)/npu_resmem.o
npd-objs += $(NPU_ARCH_VERSION)/npu_mntn.o
npd-objs += $(NPU_ARCH_VERSION)/npu_sink_sqe_fmt.o
npd-objs += $(NPU_ARCH_VERSION)/npu_sink_task_verify.o

ifneq ($(NPU_ARCH_VERSION),npu_v100)
	npd-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_hwts_plat.o
	npd-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_aicore_plat.o
	npd-$(CONFIG_NPU_SDMA) += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_sdma_plat.o
	npd-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_platform_exception.o
	npd-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_chip_cfg.o
	ifneq ($(CONFIG_NPU_PM_FUSION),y)
		npd-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_pm_config_aicore.o
		npd-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_pm_config_npusubsys.o
		npd-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_pm_config_nputop.o
		npd-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_pm_config_smmutbu.o
		npd-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_pm_config_tscpu.o
	endif
endif

ifeq ($(CONFIG_NPU_AUTOFS),y)
	npd-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_autofs_plat.o
endif

ifeq ($(CONFIG_NPU_SWTS),y)
	npd-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_hwts_defines.o
	npd-objs += $(NPU_ARCH_VERSION)/hwts_driver.o
	npd-objs += $(NPU_ARCH_VERSION)/profiling_driver.o
endif

ifeq ($(NPU_CHIP_CFG),y)
	npd-objs +=  $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_chip_cfg.o
endif
ifeq ($(CONFIG_NPU_PM_FUSION),y)
	npd-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_pm_top_common.o
	npd-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_pm_top_specify.o
	npd-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_pm_non_top_common.o
	npd-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_pm_non_top_specify.o
endif

all:
	$(info  NKD_DIR is  $(value NKD_DIR) S(NKD_DIR) PATH is $(value PATH))
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
 