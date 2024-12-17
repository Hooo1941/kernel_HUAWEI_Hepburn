# SPDX-License-Identifier: GPL-2.0
ccflags-y += -I$(srctree)/drivers/i2c/busses
ccflags-y += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow
ifeq ($(CONFIG_CONTEXTHUB_IO_DIE_STS), y)
	ccflags-y += -I$(srctree)/platform_source/smart/drivers/channels/io_die/
endif
obj-$(CONFIG_I2C_V300_DESIGNWARE)	+= i2c_v300_designware.o
obj-$(CONFIG_I2C_V400_MASTER)	+= i2c_v400_master.o