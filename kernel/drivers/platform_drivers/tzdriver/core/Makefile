KERNEL_DIR :=$(srctree)

ifneq ($(TARGET_BUILD_VARIANT),user)
	ccflags-y += -DDEF_ENG
endif

ifeq ($(CONFIG_HIBERNATION),y)
ccflags-y += -DCONFIG_AP_CUST_PAD_MEM
ccflags-y += -DCONFIG_TZ_S4_SMC
ccflags-y += -DCONFIG_CMD_BUFFER_RESTORE
ccflags-y += -DCONFIG_TZ_S4_OPTIMIZE
endif

EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver/agent_rpmb/core
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver/auth
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver/tlogger
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver/tui
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver/ion
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver/core
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver/include
ifeq ($(CONFIG_TZDRIVER_INTERNAL), y)
	include $(KERNEL_DIR)/drivers/platform_drivers/tzdriver/tzdriver_internal/internal.mk
endif

obj-y += teek_client_api.o
obj-y += smc_smp.o tc_client_driver.o session_manager.o mailbox_mempool.o teek_app_load.o
obj-y += agent.o gp_ops.o mem.o cmdmonitor.o tzdebug.o tz_spi_notify.o tz_pm.o tee_compat_check.o
obj-y += reserved_mempool.o
obj-y += teek_client_ext.o
obj-y += shared_mem.o

ifdef CONFIG_FFA_SUPPORT
obj-y += ffa_abi.o
else
obj-y += smc_abi.o
endif
