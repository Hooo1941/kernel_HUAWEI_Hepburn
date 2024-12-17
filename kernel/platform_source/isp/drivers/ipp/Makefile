# IPP Makefile
ccflags-$(CONFIG_HISPIPP_V300) += -Idrivers/platform_drivers/ipp/v300
ccflags-$(CONFIG_HISPIPP_V320) += -Idrivers/platform_drivers/ipp/v320/base/
ccflags-$(CONFIG_RELEASE_IPP_V320) += -Idrivers/platform_drivers/ipp/v320/release/
ccflags-$(CONFIG_HISPIPP_V250) += -Idrivers/platform_drivers/ipp/v250
ccflags-$(CONFIG_HISPIPP_V230) += -Idrivers/platform_drivers/ipp/v230
ccflags-$(CONFIG_HISPIPP_V500) += -Idrivers/platform_drivers/ipp/v500
ifneq ($(chip_type),es)
	ccflags-$(CONFIG_HISPIPP_V350) += -Idrivers/platform_drivers/ipp/v350r02
else
	ccflags-$(CONFIG_HISPIPP_V350) += -Idrivers/platform_drivers/ipp/v350
endif

obj-$(CONFIG_HISPIPP_V300) += v300/
obj-$(CONFIG_HISPIPP_V320) += v320/base/
obj-$(CONFIG_RELEASE_IPP_V320) += v320/release/
obj-$(CONFIG_HISPIPP_V250) += v250/
obj-$(CONFIG_HISPIPP_V230) += v230/
obj-$(CONFIG_HISPIPP_V500) += v500/
ifneq ($(chip_type),es)
	obj-$(CONFIG_HISPIPP_V350) += v350r02/
else
	obj-$(CONFIG_HISPIPP_V350) += v350/
endif


