# SPDX-License-Identifier: GPL-2.0

ccflags-y += -I$(srctree)/drivers/mmc/host

obj-$(CONFIG_MMC_SDHCI)         += sdhci_zodiac.o
obj-$(CONFIG_MMC_SDHCI_MSHC_LIBRA)          += sdhci_mshc_libra.o sdhci_mshc.o
obj-$(CONFIG_MMC_SDHCI_ZODIAC_AQUILA)      += sdhci_zodiac_mmc.o sdhci_zodiac_mmc_aquila.o
obj-$(CONFIG_MMC_SDHCI_ZODIAC_ANTLIA)      += sdhci_zodiac_mmc.o sdhci_zodiac_mmc_antlia.o
obj-$(CONFIG_MMC_SDHCI_ZODIAC_SCORPIO)      += sdhci_zodiac_mmc.o sdhci_zodiac_mmc_scorpio.o
obj-$(CONFIG_MMC_SDHCI_ZODIAC_ANDROMEDA)    += sdhci_zodiac_mmc.o sdhci_zodiac_mmc_andromeda.o
obj-$(CONFIG_MMC_SDHCI_ZODIAC_APUS)    += sdhci_zodiac_mmc.o sdhci_zodiac_mmc_apus.o
obj-$(CONFIG_MMC_SDHCI_MUX_SDSIM)      += sdhci_zodiac_mux_sdsim.o

obj-$(CONFIG_MMC_DW_HI3XXX)	+= dw_mmc_hi3xxx.o dw_mmc_hi3xxx_helper.o
obj-$(CONFIG_MMC_DW_MUX_SDSIM)  += dw_mmc_mux_sdsim.o
obj-$(CONFIG_DFX_DEBUG_FS)     += dw_mmc_test.o
ifeq ($(CONFIG_HIFI_EXTRA_MODEM_MODE),y)
obj-$(CONFIG_MMC_DW_HI3XXX)     += dw_mmc_clk_for_calibration.o
endif

obj-$(CONFIG_EMMC_FAULT_INJECT) += emmc_fault_inject.o
