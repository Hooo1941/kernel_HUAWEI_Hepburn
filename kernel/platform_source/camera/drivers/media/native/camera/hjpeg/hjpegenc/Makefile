
ccflags-y += -Wextra -Wdate-time -Wfloat-equal -Wshadow -Wno-implicit-int
ifeq ($(es_low_freq),true)
EXTRA_CFLAGS += -DCONFIG_ES_LOW_FREQ
endif

ccflags-y += -I$(srctree)/platform_source/camera/drivers/media/native/camera/hjpeg
ccflags-y += -I$(srctree)/platform_source/camera/drivers/media/native/camera
obj-y += hjpeg_process.o smmu_cfg.o jpegenc_cfg.o cvdr_cfg.o default_config.o
ifeq ($(pcie_fpga_verify),true)
ccflags-$(CONFIG_KERNEL_CAMERA_ISPV350) += -DJPEG_USE_PCIE_VERIFY=1
endif
ifneq ($(CONFIG_KERNEL_CAMERA_ISPV300), y)
obj-y += hjpeg_debug.o
endif

ifeq ($(CONFIG_KERNEL_CAMERA_ISPV350), y)
obj-y += platform_v350/
else ifeq ($(CONFIG_KERNEL_CAMERA_ISPV500), y)
obj-y += platform_v500/
endif

ifeq ($(CONFIG_CAMERA_ISPV230), y)
EXTRA_CFLAGS += -DHISP230_CAMERA
endif
