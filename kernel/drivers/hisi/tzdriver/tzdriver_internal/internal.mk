ifeq ($(CONFIG_TEE_TUI_MTK), y)
EXTRA_CFLAGS += -Idrivers/platform_drivers/tzdriver/tzdriver_internal/livepatch
EXTRA_CFLAGS += -Idrivers/platform_drivers/tzdriver/tzdriver_internal/kthread_affinity
EXTRA_CFLAGS += -Idrivers/platform_drivers/tzdriver/tzdriver_internal/include
EXTRA_CFLAGS += -Idrivers/platform_drivers/tzdriver/tzdriver_internal/tee_trace_event
EXTRA_CFLAGS += -Idrivers/platform_drivers/tzdriver/tzdriver_internal/tlogger
else
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver/tzdriver_internal/livepatch
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver/tzdriver_internal/kthread_affinity
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver/tzdriver_internal/include
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver/tzdriver_internal/tee_trace_event
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver/tzdriver_internal/tlogger
endif
