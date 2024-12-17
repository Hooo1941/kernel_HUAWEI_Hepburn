KERNEL_DIR := $(srctree)

ifneq ($(TARGET_BUILD_VARIANT),user)
    ccflags-y += -DDEF_ENG
endif

include $(KERNEL_DIR)/drivers/platform_drivers/tzdriver/tzdriver_internal/internal.mk
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver/include
EXTRA_CFLAGS += -include internal_functions.h

EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver/core
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver/tzdriver_internal/livepatch

obj-$(CONFIG_LIVEPATCH_ENABLE) += livepatch_cmd.o
