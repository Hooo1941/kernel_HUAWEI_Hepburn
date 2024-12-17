# SPDX-License-Identifier: GPL-2.0

ccflags-y += -I$(srctree)/fs

ifneq ($(strip $(product_type)),armpc)
obj-y += crypto/
obj-y += f2fs/
endif
obj-y += filemap_manager/
obj-$(CONFIG_HISI_PROC) += proc/

obj-$(CONFIG_VENDOR_FS) += vendor_fs_interface.o
obj-$(CONFIG_HYPERFRAME) += hyperframe.o
