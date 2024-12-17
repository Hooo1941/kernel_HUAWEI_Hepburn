# SPDX-License-Identifier: GPL-2.0

ccflags-y += -I$(srctree)/drivers/mmc/core

obj-$(CONFIG_MMC) += cache_enable.o
obj-$(CONFIG_MMC_PASSWORDS) += lock.o sysfs.o
obj-$(CONFIG_BOOTDEVICE) 	+= emmc_rpmb.o
obj-$(CONFIG_MMC_CQ_HCI) 	+= zodiac_cmdq_core.o
obj-$(CONFIG_ZODIAC_MMC) 	+= zodiac_core_mmc.o
obj-$(CONFIG_ZODIAC_MMC_MANUAL_BKOPS) += zodiac_mmc_bkops.o
obj-$(CONFIG_MMC_FFU)		+= ffu.o
obj-$(CONFIG_HW_SD_HEALTH_DETECT)	+= mmc_health_diag.o
obj-$(CONFIG_HW_MMC_TEST_MODULE)	+= hw_mmc_test.o
obj-$(CONFIG_HW_SYSTEM_HW_WR_PROTECT) += emmc_system_wp.o
obj-$(CONFIG_HUAWEI_EMMC_DSM) += dsm_emmc.o
obj-$(CONFIG_HUAWEI_SDCARD_DSM) += dsm_sdcard.o
obj-$(CONFIG_MMC_CQ_HCI) += zodiac_cmdq_card.o
obj-$(CONFIG_MMC_MQ_CQ_HCI) += zodiac_cmdq_mq_card.o
obj-$(CONFIG_RPMB_MMC) += zodiac_rpmb_ioctl.o