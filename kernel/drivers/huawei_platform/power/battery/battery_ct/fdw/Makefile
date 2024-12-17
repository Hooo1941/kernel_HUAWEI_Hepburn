# SPDX-License-Identifier: GPL-2.0+
ifeq ($(CONFIG_ONEWIRE_PHY_PL061_GPIO), y)
ccflags-y += -I$(srctree)/drivers/gpio
endif

ifneq ($(TARGET_BUILD_VARIANT),user)
ccflags-y += -DONEWIRE_STABILITY_DEBUG
endif

ifeq ($(TARGET_VERSION_MODE),factory)
ccflags-y += -DBATTBD_FORCE_MATCH
endif

obj-$(CONFIG_HUAWEI_BATTCT_FM1230) += core/
obj-$(CONFIG_HUAWEI_BATTCT_FM1230) += fm1230_api.o
obj-$(CONFIG_HUAWEI_BATTCT_FM1230) += fm1230_swi.o
obj-$(CONFIG_HUAWEI_BATTCT_FM1230) += fm1230.o

