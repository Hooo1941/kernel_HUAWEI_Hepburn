# SPDX-License-Identifier: GPL-2.0

INCLUDE += -I$(srctree)/drivers/ata
EXTRA_CFLAGS += -I$(srctree)/drivers/ata
ccflags-y +=  -I$(srctree)/drivers/ata

obj-$(CONFIG_VENDOR_SATA)	+= sata_phy_firmware.o sata_vendor.o sata_trap.o
obj-$(CONFIG_VENDOR_SATA_ANTLIA)	+= sata_antlia.o
obj-$(CONFIG_DFX_DEBUG_FS)	+= hi10cc_dfx_dump.o hi10cc_dfx_dump_cp.o hi10cc_dfx_dump_cs.o hi10cc_dfx_dump_tx.o hi10cc_dfx_dump_rx.o hi10cc_dfx.o hi10cc_dfx_atb.o hi10cc_debugfs.o
