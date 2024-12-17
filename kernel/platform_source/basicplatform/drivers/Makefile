obj-$(CONFIG_ARM_AMBA)		+= amba/
obj-$(CONFIG_SPMI_PLATFORM)	+=spmi_platform/
obj-$(CONFIG_BOOTDEVICE)   += bootdevice/

obj-y += mfd/
obj-y += nve/
obj-y += partition/
obj-y += block/

obj-$(CONFIG_PLAT_CLK)     += clk/
obj-y	+= regulator/
obj-$(CONFIG_CMDLINE_PARSE)	+= cmdline/
obj-$(CONFIG_PLATFORM_IPC_MAILBOX)        += mailbox/
obj-$(CONFIG_VENDOR_RPMB) += rpmb/
ifneq ($(strip $(CFG_HISI_OPENSOURCE)), true)
obj-$(CONFIG_DFX_WATCHPOINT_CB)+= watchpoint/
endif
obj-y += gpio/
obj-$(CONFIG_I2C) += i2c/
obj-$(CONFIG_USB_PHY_CHIP)			+= usb_phy_chip/
obj-y += phy/
obj-y += hid/
obj-y += hsdt_bus/
obj-y += usb/
obj-$(CONFIG_AB_PARTITION_TABLE)      += bootctrl/
obj-y += pinctrl/
obj-$(CONFIG_VENDOR_HWSPINLOCK)     += hwspinlock/
obj-y += pwm/
obj-y += rtc/
obj-$(CONFIG_I3C) += i3c/
obj-y += secmem/
obj-$(CONFIG_DMA_SHARED_BUFFER) += dma-buf/
obj-$(CONFIG_BG_DMABUF_RECLAIM) += bg_dmabuf_reclaim/
obj-$(CONFIG_DFX_KERNELDUMP) += memory_dump/
obj-$(CONFIG_DFX_NOC) += noc/
obj-$(CONFIG_MM_LB)                       += mm_lb/
obj-$(CONFIG_MM_SDCU) += sdcu/
obj-$(CONFIG_AP_CP_DECOUPL)           += modem_decoupl/
obj-$(CONFIG_DFX_PLATFORM_MAINTAIN) += mntn/
ifeq ($(TARGET_VERSION_MODE),factory)
obj-$(CONFIG_PLATFORM_DIEID) += dieid/
endif
obj-$(CONFIG_HISI_DUMMY_KO) += hisi_dummy_ko/
obj-$(CONFIG_DMADEVICES) += dma/
obj-y += tty/serial/
obj-$(CONFIG_SPI) += spi/
obj-$(CONFIG_DFX_SEC_QIC) += qic/
obj-y += watchdog/
obj-$(CONFIG_HISI_TIMEKEEPER) += timekeeper/
obj-y += hisi_cma/
obj-y += hisi_lmk/
obj-$(CONFIG_HCK) += hck/
obj-$(CONFIG_PCI) += pci/
obj-$(CONFIG_TASK_COPROCESSOR) += tcp/
obj-$(CONFIG_NEW_LEDS) += leds/
obj-$(CONFIG_SWITCH) += switch/
obj-y += input/keyboard/
obj-y += input/misc/
obj-$(CONFIG_POWER_PLATFORM) += power/
obj-y += scsi/
obj-$(CONFIG_VENDOR_UFS) += scsi/ufs/
obj-$(CONFIG_MMC) += mmc/
obj-$(CONFIG_HISILICON_PLATFORM_HITEST) += hitest/
obj-$(CONFIG_NET_VENDOR_HISI_HIXGEMAC) += net/ethernet/hisi/
ifeq ($(CONFIG_RTL9010_VA_SWITCH),)
obj-$(CONFIG_MOTORCOMM_PHY) += net/phy/
endif
obj-$(CONFIG_HYPERHOLD_INLINE_CRYPTO) += hp_ohos/
obj-$(CONFIG_HISI_PROC_HARDWARE) += hw_info/
obj-$(CONFIG_SOC_PG_FEATURE_DEVDRV) += soc_pg_feature_modem/
obj-$(CONFIG_VENDOR_SATA) += ata/
obj-$(CONFIG_SOFTWARE_BOARDID) += boardid/
obj-$(CONFIG_FAN_CTRL) += fan/
obj-$(CONFIG_SPI_NOR_AGENT) += spi_nor_agent/