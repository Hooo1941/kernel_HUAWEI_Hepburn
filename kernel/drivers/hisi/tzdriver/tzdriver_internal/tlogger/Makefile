KERNEL_DIR := $(srctree)

ifneq ($(TARGET_BUILD_VARIANT),user)
	ccflags-y += -DDEF_ENG
endif

include $(KERNEL_DIR)/drivers/platform_drivers/tzdriver/tzdriver_internal/internal.mk
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver/include
EXTRA_CFLAGS += -include internal_functions.h

EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver/core
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/platform_drivers/tzdriver/tlogger

include $(KERNEL_DIR)/drivers/platform_drivers/tzdriver/tzdriver_internal/internal.mk

obj-$(CONFIG_BBOX_MEM) += log_bbox_cfg.o
obj-$(CONFIG_RDR_MEM) += log_rdr_cfg.o
