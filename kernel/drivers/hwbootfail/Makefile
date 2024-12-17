# Makefile for the Boot Detector modules.

ifeq ($(CONFIG_BOOT_DETECTOR_SINGLE_FRAME),y)
obj-$(CONFIG_BOOT_DETECTOR) += \
	core/boot_detector.o \
	core/boot_recovery.o \
	common/adapter_common.o \
	common/bootfail_timer.o \
	common/simple/bootfail_common.o \
	common/simple/bootfail_parsing.o
else
obj-$(CONFIG_BOOT_DETECTOR) += \
	core/boot_detector.o \
	core/boot_recovery.o \
	common/bootfail_common.o \
	common/adapter_common.o \
	common/bootfail_timer.o
endif

ifeq ($(CONFIG_BOOT_DETECTOR_HISI),y)
obj-$(CONFIG_BOOT_DETECTOR) += chipsets/hisi/bootfail_hisi.o
obj-$(CONFIG_BOOT_DETECTOR) += chipsets/hisi/adapter_hisi.o
obj-$(CONFIG_BOOT_DETECTOR) += chipsets/hisi/ocp_hisi.o
endif

ifeq ($(CONFIG_BOOT_DETECTOR_HONGHU),y)
obj-$(CONFIG_BOOT_DETECTOR) += chipsets/honghu/bootfail_honghu.o
obj-$(CONFIG_BOOT_DETECTOR) += chipsets/honghu/adapter_honghu.o
obj-$(CONFIG_BOOT_DETECTOR) += chipsets/honghu/bootfail_honghu_interface.o
endif

ifeq ($(CONFIG_BOOT_DETECTOR_MTK),y)
obj-$(CONFIG_BOOT_DETECTOR) += chipsets/mtk/bootfail_mtk.o
obj-$(CONFIG_BOOT_DETECTOR) += chipsets/mtk/adapter_mtk.o
endif

ifeq ($(CONFIG_BOOT_DETECTOR_MTK_4G),y)
obj-$(CONFIG_BOOT_DETECTOR) += chipsets/mtk/bootfail_mtk_4g.o
endif

ifeq ($(CONFIG_BOOT_DETECTOR_QCOM),y)
obj-$(CONFIG_BOOT_DETECTOR) += chipsets/qcom/bootfail_qcom.o
obj-$(CONFIG_BOOT_DETECTOR) += chipsets/qcom/adapter_qcom.o
endif

ifeq ($(CONFIG_BOOT_DETECTOR_NVT),y)
obj-$(CONFIG_BOOT_DETECTOR) += chipsets/nvt/bootfail_nvt.o
obj-$(CONFIG_BOOT_DETECTOR) += chipsets/nvt/adapter_nvt.o
endif
