# SPDX-License-Identifier: GPL-2.0
UFS_SOC_CHIP_NAME := $(strip $(subst ", ,$(CONFIG_VENDOR_CHIP_PREFIX)))
UFS_CHIP_TYPE := $(strip $(subst ", , $(CONFIG_VENDOR_CHIP_SUFFIX)))
UFS_CHIP_VERSION := $(UFS_SOC_CHIP_NAME)_$(UFS_CHIP_TYPE)

INCLUDE += -I$(srctree)/platform_include/basicplatform/linux/ufs
INCLUDE += -I$(srctree)/include/platform_include/basicplatform/linux/ufs
INCLUDE += -I$(srctree)/vendor/include/platform_include/basicplatform/linux/ufs
INCLUDE += -I$(srctree)/drivers
INCLUDE += -I$(srctree)/drivers/scsi/ufs
EXTRA_CFLAGS += -I$(srctree)/platform_include/basicplatform/linux/ufs
EXTRA_CFLAGS += -I$(srctree)/include/platform_include/basicplatform/linux/ufs
EXTRA_CFLAGS += -I$(srctree)/vendor/include/platform_include/basicplatform/linux/ufs
EXTRA_CFLAGS += -I$(srctree)/drivers
EXTRA_CFLAGS += -I$(srctree)/drivers/scsi/ufs
ccflags-y += -I$(srctree)/platform_include/basicplatform/linux/ufs
ccflags-y += -I$(srctree)/include/platform_include/basicplatform/linux/ufs
ccflags-y += -I$(srctree)/vendor/include/platform_include/basicplatform/linux/ufs
ccflags-y += -I$(srctree)/drivers
ccflags-y += -I$(srctree)/drivers/scsi/ufs

ccflags-y += -I$(srctree)/drivers/platform_drivers/tzdriver

obj-$(CONFIG_SCSI_HUFS) += ufshcd_extend.o ufshcd_perf.o ufshcd_debug.o ufshcd_protocol.o
obj-$(CONFIG_SCSI_HUFS) += hufs_plat.o
obj-$(CONFIG_SCSI_HUFS) += ufshcd_hufs_extend.o
obj-$(CONFIG_SCSI_HUFS) += hufs_mphy.o
obj-$(CONFIG_SCSI_HUFS) += ufs_mphy_firmware.o
obj-$(CONFIG_SCSI_UFS_TAURUS) += ufs-taurus.o
obj-$(CONFIG_SCSI_UFS_PISCES) += ufs-pisces.o
obj-$(CONFIG_SCSI_UFS_SCORPIO) += ufs_scorpio.o
obj-$(CONFIG_SCSI_UFS_LEO) += ufs-leo.o
obj-$(CONFIG_SCSI_UFS_SGR) += ufs-sgr.o
obj-$(CONFIG_SCSI_UFS_ANTLIA) += ufs_antlia.o ufs_trap.o
obj-$(CONFIG_SCSI_UFS_AQUILA) += ufs_aquila.o
obj-$(CONFIG_SCSI_UFS_ARA) += ufs_ara.o
obj-$(CONFIG_UFS_4_0_HC) += ufs_mcq.o
obj-$(CONFIG_DFA_MCQ) += dfa_mcq.o dfa_debugfs.o dfa_error.o
obj-$(CONFIG_SCSI_HUFS) += ufs-hpb.o
obj-$(CONFIG_HUFS_HC) += hufs_hcd.o
obj-$(CONFIG_HUFS_BOOTROM_MODE) += ufs_config_efuse.o ufs_trap.o
obj-$(CONFIG_SCSI_UFS_LIBRA) += ufs-libra.o
obj-$(CONFIG_HUFS_HC) += hufs-dump.o
obj-$(CONFIG_RPMB_UFS) += ufs_rpmb.o
obj-$(CONFIG_HUAWEI_UFS_VENDOR_MODE) += ufs_vendor_mode.o
obj-$(CONFIG_SCSI_UFS_FAULT_INJECT) += ufs_fault_inject.o
obj-$(CONFIG_DFX_DEBUG_FS) += ufs_debugfs.o
obj-$(CONFIG_HUAWEI_UFS_DSM) += dsm_ufs.o
obj-$(CONFIG_AB_PARTITION_TABLE) += hufs-lib.o
obj-$(CONFIG_HUFS_MANUAL_BKOPS) += hufs-bkops.o hufs_bkops_interface.o
obj-$(CONFIG_SCSI_UFS_HI1861_VCMD) += ufs_vendor_cmd.o
obj-$(CONFIG_SCSI_UFS_HI1861_VCMD) += ufs_vcmd_proc.o
obj-$(CONFIG_SCSI_UFS_HI1861_VCMD) += ufstt.o
obj-$(CONFIG_SCSI_UFS_UNISTORE) += ufs_unistore_core.o
obj-$(CONFIG_SCSI_UFS_UNISTORE) += ufs_unistore_read.o
obj-$(CONFIG_SCSI_UFS_UNISTORE) += ufs_unistore_write.o
obj-$(CONFIG_SCSI_UFS_GEAR_CTRL) += ufs_gear_ctrl.o
obj-$(CONFIG_SCSI_UFS_VIRTUALIZ_AGENT) += ufs_virtualiz_agent.o
obj-$(CONFIG_SCSI_UFS_VIRTUALIZ_SERVICE) += ufs_virtualiz_service.o
ifneq ($(CONFIG_RPMB_UFS),y)
obj-$(CONFIG_SCSI_UFS_VIRTUALIZ_SERVICE) += ufs_rpmb.o
endif

ifeq ($(CONFIG_DFX_DEBUG_FS),y)
obj-$(CONFIG_HUFS_HC) += hufs_test.o
endif

ifeq ($(TARGET_VERSION_MODE), factory)
	obj-$(CONFIG_HUAWEI_UFS_TRACE) += ufs_trace.o
	EXTRA_CFLAGS += -DUFS_TRACE_FLAG
	EXTRA_CFLAGS += -DHEALTH_INFO_FACTORY_MODE
	EXTRA_CFLAGS += -DDSM_FOR_FACTORY_MODE
	EXTRA_CFLAGS += -DUFS_HIGH_TEMP_DISABLE
endif

ifeq ($(UFS_CHIP_VERSION), charlotte_csv2)
ccflags-$(CONFIG_FILE_BASED_ENCRYPTO) += -DCONFIG_FBE3_1
ccflags-$(CONFIG_FILE_BASED_ENCRYPTO) += -DCONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_KM
endif
