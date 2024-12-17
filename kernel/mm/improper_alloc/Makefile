# SPDX-License-Identifier: GPL-2.0

ifeq ($(BUILD_TC_CHIPSET_COMPONENT), true)
ccflags-$(CONFIG_IMPROPER_ALLOC) += -DIMPROPER_ALLOC_ENABLE_DEFAULT=1
else ifeq ($(CONFIG_IMPROPER_ALLOC_SINGLE_FRAME), y)
ccflags-$(CONFIG_IMPROPER_ALLOC) += -DIMPROPER_ALLOC_ENABLE_DEFAULT=1
else
ccflags-$(CONFIG_IMPROPER_ALLOC) += -DIMPROPER_ALLOC_ENABLE_DEFAULT=0
endif

obj-$(CONFIG_IMPROPER_ALLOC) += improper_alloc.o improper_alloc_core.o improper_alloc_sysctl.o
obj-$(CONFIG_IMPROPER_ALLOC_FAULT) += improper_alloc_fault.o
