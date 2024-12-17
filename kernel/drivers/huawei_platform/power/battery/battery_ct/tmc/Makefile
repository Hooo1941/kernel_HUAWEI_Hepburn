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

obj-$(CONFIG_HUAWEI_BATTCT_T91407) += core/
obj-$(CONFIG_HUAWEI_BATTCT_T91407) += t91407_swi.o
obj-$(CONFIG_HUAWEI_BATTCT_T91407) += t91407_api.o
obj-$(CONFIG_HUAWEI_BATTCT_T91407) += t91407.o