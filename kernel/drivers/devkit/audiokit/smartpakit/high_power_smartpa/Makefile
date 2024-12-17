# SPDX-License-Identifier: GPL-2.0
#
# smartpa high voltage converter
#
ifdef CONFIG_HUAWEI_SMARTPA_HIGH_VOLTAGE_CONVERTER
	ccflags-y += -Idrivers/devkit/audiokit/
	ccflags-y += -I$(srctree)/drivers/devkit/audiokit/
endif

obj-$(CONFIG_HUAWEI_SMARTPA_HIGH_VOLTAGE_CONVERTER) += smartpa_high_voltage_converter.o
