EXTRA_CFLAGS += $(NPU_GLOBAL_CFLAGS)
EXTRA_CFLAGS += -I$(NKD_DIR)/inc
EXTRA_CFLAGS += -I$(NKD_DIR)/inc/comm
EXTRA_CFLAGS += -I$(NKD_DIR)/inc/model
EXTRA_CFLAGS += -I$(NKD_DIR)/
EXTRA_CFLAGS += -I$(NKD_DIR)/core
EXTRA_CFLAGS += -I$(NKD_DIR)/dts
EXTRA_CFLAGS += -I$(NKD_DIR)/communication
EXTRA_CFLAGS += -I$(NKD_DIR)/memory
EXTRA_CFLAGS += -I$(NPD_DIR)/$(NPU_ARCH_VERSION)
EXTRA_CFLAGS += -I$(NPD_DIR)/$(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)
EXTRA_CFLAGS += -I$(NPD_DIR)/
EXTRA_CFLAGS += -I$(NPU_DIR)/bbit_debugfs
EXTRA_CFLAGS += -I$(NPU_DIR)/bbit_debugfs/core
EXTRA_CFLAGS += -I$(NPU_DIR)/bbit_debugfs/kprobe

LOCAL_PATH := $(call my-dir)

obj-$(CONFIG_NPU_BBIT_DEBUGFS) += npu_bbit_debugfs_module.o
npu_bbit_debugfs_module-objs :=npu_bbit_debugfs.o core/npu_bbit_core.o core/npu_kprobe_core.o kprobe/npu_kprobe_debugfs.o kprobe/npu_kprobe.o

ifeq ($(NPU_ARCH_VERSION),npu_v100)
  EXTRA_CFLAGS += -I$(NPU_DIR)/bbit_debugfs/hiail/npu_v100
	EXTRA_CFLAGS += -I$(NKD_DIR)/power_manager/v100
	npu_bbit_debugfs_module-objs += hiail/npu_bbit_hwts_stub.o
else
	EXTRA_CFLAGS += -I$(NKD_DIR)/power_manager
	ifneq ($(CONFIG_NPU_BBIT_DEBUGFS),y)
		npu_bbit_debugfs_module-objs += hiail/npu_bbit_hwts_stub.o
	else
		ifeq ($(NPU_ARCH_VERSION),npu_v210)
			EXTRA_CFLAGS += -I$(NPU_DIR)/bbit_debugfs/hiail/npu_v210
			npu_bbit_debugfs_module-objs += hiail/npu_bbit_ipc_send.o
			npu_bbit_debugfs_module-objs += hiail/npu_bbit_hwts_log_210.o
			npu_bbit_debugfs_module-objs += hiail/npu_bbit_hwts_config_210.o
		else
			EXTRA_CFLAGS += -I$(NPU_DIR)/bbit_debugfs/hiail/npu_v200
			npu_bbit_debugfs_module-objs += hiail/npu_bbit_hwts_log.o
			npu_bbit_debugfs_module-objs += hiail/npu_bbit_hwts_config.o
		endif
	endif
endif
