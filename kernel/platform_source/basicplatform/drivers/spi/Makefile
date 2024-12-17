# SPDX-License-Identifier: GPL-2.0
ccflags-y += -I$(srctree)/drivers/spi

obj-$(CONFIG_SPI_V500_MASTER)           += spi_v500.o

ifeq ($(CONFIG_CONTEXTHUB_IO_DIE_STS), y)
	ccflags-y += -I$(srctree)/platform_source/smart/drivers/channels/io_die/
endif
