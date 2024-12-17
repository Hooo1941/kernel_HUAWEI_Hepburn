KERNEL_DIR := $(srctree)

ifneq ($(TARGET_BUILD_VARIANT),user)
    ccflags-y += -DDEF_ENG
endif

EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver/include
EXTRA_CFLAGS += -include internal_functions.h

EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver/core

obj-y += agent_allowed_ca.o
