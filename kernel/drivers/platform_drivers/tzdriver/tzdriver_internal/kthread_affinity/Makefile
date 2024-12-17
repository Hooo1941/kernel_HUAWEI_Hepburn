KERNEL_DIR := $(srctree)

ifneq ($(TARGET_BUILD_VARIANT),user)
    ccflags-y += -DDEF_ENG
endif

include $(KERNEL_DIR)/drivers/platform_drivers/tzdriver/tzdriver_internal/internal.mk
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver/include
EXTRA_CFLAGS += -include internal_functions.h

EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver/core

obj-$(CONFIG_KTHREAD_AFFINITY) += tz_kthread_affinity.o tz_kthread_cpumask_dts.o
