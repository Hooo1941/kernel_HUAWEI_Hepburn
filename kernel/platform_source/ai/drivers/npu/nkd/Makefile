EXTRA_CFLAGS += $(NPU_GLOBAL_CFLAGS)
EXTRA_CFLAGS += -I$(NKD_DIR)/inc
EXTRA_CFLAGS += -I$(NKD_DIR)/inc/comm
EXTRA_CFLAGS += -I$(NKD_DIR)/core
EXTRA_CFLAGS += -I$(NKD_DIR)/format_convertor
EXTRA_CFLAGS += -I$(NKD_DIR)/heartbeat
EXTRA_CFLAGS += -I$(NKD_DIR)/lowpower
EXTRA_CFLAGS += -I$(NKD_DIR)/stream_manager
EXTRA_CFLAGS += -I$(NKD_DIR)/inc/model
EXTRA_CFLAGS += -I$(NKD_DIR)/memory
EXTRA_CFLAGS += -I$(NKD_DIR)/atf
EXTRA_CFLAGS += -I$(NKD_DIR)/communication
EXTRA_CFLAGS += -I$(NKD_DIR)/id_allocator
EXTRA_CFLAGS += -I$(NKD_DIR)/dfx
EXTRA_CFLAGS += -I$(NKD_DIR)/dfx/profiling
EXTRA_CFLAGS += -I$(NKD_DIR)/dfx/counter
ifneq ($(CONFIG_NPU_SWTS),y)
	EXTRA_CFLAGS += -I$(NKD_DIR)/dfx/log
endif
EXTRA_CFLAGS += -I$(NKD_DIR)/dfx/bbox
EXTRA_CFLAGS += -I$(NPD_DIR)/
EXTRA_CFLAGS += -I$(NKD_DIR)/dts
EXTRA_CFLAGS += -I$(NKD_DIR)/manager
EXTRA_CFLAGS += -I$(NPD_DIR)/$(NPU_ARCH_VERSION)
EXTRA_CFLAGS += -I$(NPD_DIR)/$(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)
EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/ap/platform/common/
EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/ap/kernel/drivers/tzdriver/libhwsecurec/
EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/ap/kernel/include/
EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/ap/kernel/
EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/ap/kernel/include/linux/platform_drivers/
EXTRA_CFLAGS += -I$(srctree)/drivers/iommu/smmu/
EXTRA_CFLAGS += -I$(srctree)/drivers/iommu/arm/smmu/
EXTRA_CFLAGS += -I$(NPU_DIR)/bbit_debugfs
EXTRA_CFLAGS += -I$(NPU_DIR)/load_ts
ifeq ($(CONFIG_ARM_FFA_TRANSPORT),y)
ifeq ($(CONFIG_NPU_FFA_SUPPORT),y)
EXTRA_CFLAGS += -I$(srctree)/include/platform_include/see/ffa/
endif
endif

ifeq ($(CONFIG_FFRT_UNIFORM_ID),y)
EXTRA_CFLAGS += -I$(srctree)/platform_source/ffrt/uniform_id
EXTRA_CFLAGS += -DCONFIG_FFRT_UNIFORM_ID
endif

obj-$(CONFIG_NPU_KERNEL_DRIVER) += nkd.o

nkd-objs := npu_common.o npu_devinit.o core/npu_ioctl_services.o core/npu_proc_ctx.o core/npu_recycle.o
nkd-objs += format_convertor/npu_comm_sqe_fmt.o format_convertor/npu_comm_task_verify.o
nkd-objs += stream_manager/npu_calc_channel.o stream_manager/npu_hwts.o stream_manager/npu_sink_stream.o stream_manager/npu_stream.o
nkd-objs += core/npu_easc.o manager/npu_manager.o manager/npu_manager_ioctl_services.o dfx/bbox/npu_dfx_black_box.o
nkd-objs += dfx/profiling/npu_dfx_profiling.o dfx/counter/npu_counter.o atf/npu_atf_subsys.o
nkd-objs += memory/npu_iova.o memory/npu_pool.o memory/npu_shm.o memory/npu_ion.o memory/npu_svm.o
nkd-objs += id_allocator/npu_calc_cq.o id_allocator/npu_calc_sq.o id_allocator/npu_event.o \
            id_allocator/npu_hwts_cq.o id_allocator/npu_hwts_sq.o id_allocator/npu_model.o \
            id_allocator/npu_task.o id_allocator/npu_id_allocator.o id_allocator/npu_hwts_event.o \
            id_allocator/npu_notify.o
nkd-objs += communication/npu_intr_hub_intc.o communication/npu_task_message.o
nkd-objs += dts/npu_dfx.o dts/npu_feature.o dts/npu_irq.o dts/npu_reg.o
nkd-objs += npu_platform.o npu_iova_dev.o npu_except_mgr.o

ifeq ($(CONFIG_NPU_DIRECT),y)
	EXTRA_CFLAGS += -I$(NKD_DIR)/direct_device
	EXTRA_CFLAGS += -I$(NKD_DIR)/direct_device/core
	nkd-objs += direct_device/npu_direct_devinit.o direct_device/core/npu_direct_ioctl_services.o direct_device/core/npu_direct_message.o
endif

ifeq ($(CONFIG_NPU_SWTS),y)
	EXTRA_CFLAGS += -I$(NKD_DIR)/schedule
	nkd-objs += schedule/hwts_channel_manager.o schedule/hwts_config.o schedule/schedule_stream_manager.o \
		schedule/schedule_stream.o schedule/task_pool.o schedule/schedule_interface.o \
		schedule/event_manager.o schedule/event.o schedule/hwts_sq_cq_manager.o schedule/hwts_event_manager.o schedule/hwts_sq_cq.o \
		schedule/schedule_model_manager.o schedule/schedule_model.o schedule/schedule_stream_engine.o schedule/schedule_task_process.o \
		schedule/hwts_interrupt_handler.o schedule/hwts_interrupt.o schedule/irq_bottom_half.o schedule/placeholder.o schedule/response.o schedule/schedule_model_engine.o \
		schedule/mutex_ops.o
else
	nkd-objs += core/npu_ts_report.o
	nkd-objs += heartbeat/npu_heart_beat.o
	nkd-objs += id_allocator/npu_mailbox.o id_allocator/npu_dfx_cq.o id_allocator/npu_dfx_sq.o
	nkd-objs += communication/npu_doorbell.o communication/npu_mailbox_msg.o communication/npu_message.o
	nkd-objs += dfx/log/npu_dfx_log.o
endif

ifeq ($(NPU_ARCH_VERSION),npu_v100)
	EXTRA_CFLAGS += -I$(NKD_DIR)/power_manager/v100
	nkd-objs += power_manager/v100/npu_pm_framework.o
else
	EXTRA_CFLAGS += -I$(NKD_DIR)/power_manager
	nkd-objs += power_manager/npu_pm_framework.o power_manager/npu_pm_interframe.o
	ifeq ($(CONFIG_NPU_PCR_ENABLED), y)
		nkd-objs += lowpower/npu_powercapping.o
	endif
	ifeq ($(CONFIG_NPU_AUTOFS), y)
		nkd-objs += lowpower/npu_autofs.o
	endif
endif

ifeq ($(NPU_ARCH_VERSION),npu_v100)
	nkd-objs += dfx/profiling/npu_dfx_profiling_bs_adapter.o
else
	nkd-objs += dfx/profiling/npu_dfx_profiling_hwts_adapter.o
endif

ifeq ($(CONFIG_NPU_DPM_ENABLED), y)
	EXTRA_CFLAGS += -DCONFIG_NPU_DPM_ENABLED
	ifeq ($(CONFIG_DPM_HWMON_V1),y)
		nkd-objs += lowpower/npu_dpm_v1.o
	endif
	ifeq ($(CONFIG_DPM_HWMON_V2),y)
		nkd-objs += lowpower/npu_dpm_v2.o
	endif
	ifeq ($(CONFIG_DPM_HWMON_V3),y)
		nkd-objs += lowpower/npu_dpm_v3.o
	endif
endif

ifeq ($(CONFIG_NPU_SYSCACHE), y)
	nkd-objs += core/npu_syscache.o
endif


ifeq ($(CONFIG_NPU_DEBUG),y)
	EXTRA_CFLAGS += -DCONFIG_NPU_DEBUG
	nkd-objs += dfx/dbg/npu_debug_resource_count.o
endif

ifeq ($(CONFIG_NPU_VIRTUAL),y)
	EXTRA_CFLAGS += -DNPU_VIRTUAL
	nkd-objs += communication/npu_event_channel.o
endif

ifeq ($(CONFIG_NPU_ECC),y)
	EXTRA_CFLAGS += -I$(NKD_DIR)/dfx/ecc
	nkd-objs += dfx/ecc/npu_ecc.o
endif

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
