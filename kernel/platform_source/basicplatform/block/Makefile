# SPDX-License-Identifier: GPL-2.0
ccflags-y += -I$(srctree)/include/platform_include/basicplatform/linux/mas
ccflags-y += -I$(srctree)/include/platform_include/basicplatform/linux/ufs
ccflags-y += -I$(srctree)/block

obj-$(CONFIG_MAS_BLK) += mas_blk_core.o mas_blk_dump.o mas_blk_latency.o mas_blk_busy_idle_notify.o \
		mas_blk_flush.o mas_blk_iosched_ufs_mq.o mas_blk_iosched_interface.o \
		mas_blk_busy_idle_interface.o mas_blk_core_interface.o mas_blk_dump_interface.o \
		mas_blk_flush_interface.o mas_blk_latency_interface.o mas_blk_mq_tag.o mas_blk_tz.o \
		mas_blk_unistore_interface.o dsm_block.o mas_blk_sysfs.o
obj-$(CONFIG_MAS_MCQ) += mas_mcq_tag.o mas_mcq_iosched.o mas_mcq_sysfs.o
obj-$(CONFIG_MAS_UFS_GEAR_CTRL) += mas_blk_gear_ctrl.o mas_blk_gear_ctrl_flowctrl.o mas_blk_gear_ctrl_sysfs.o
 obj-$(CONFIG_MAS_UNISTORE_PRESERVE) += mas_blk_unistore.o mas_blk_unistore_register.o mas_blk_unistore_sysfs.o
 obj-$(CONFIG_MAS_BKOPS) += mas_bkops_core.o mas_bkops_sysfs.o mas_bkops_core_interface.o mas_bkops_sysfs_interface.o
 obj-$(CONFIG_MAS_HEALTH) += mas_blk_health.o
 obj-$(CONFIG_RPMB_UFS) += blk_scsi_kern.o
 obj-$(CONFIG_SCSI_UFS_INLINE_CRYPTO) += mas_blk_inline_crypt.o
 obj-$(CONFIG_VIRTIO_BLK_COOKIE_BE) += virtio_blk_cookie.o
 obj-$(CONFIG_VIRTIO_BLK_COOKIE_FE) += virtio_blk_cookie.o
 obj-$(CONFIG_MAS_VIRTIO_BLK_FE) += mas_virtio_blk.o
 obj-$(CONFIG_MAS_VIRTIO_BLK_BE) += mas_virtio_blk.o
 obj-$(CONFIG_MAS_BLK_HM) += mas_blk_hm.o
EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/ap/platform/common/lowpm