# SPDX-License-Identifier: GPL-2.0
ifeq ($(CONFIG_HUAWEI_VIBRATOR), y)
EXTRA_CFLAGS += -I$(srctree)/drivers/huawei_platform/vibrator/vibrator_event
endif

obj-$(CONFIG_FLAT_VIBRATOR)   += flat_vibrator.o
obj-$(CONFIG_LINEAR_VIBRATOR) += linear_vibrator.o
obj-$(CONFIG_POWERKEY_SPMI)   += powerkey_spmi.o
obj-$(CONFIG_POWERKEY_SPMI)   += powerkey_event.o
