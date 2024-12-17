obj-$(CONFIG_DKMD_DPU_CONNECTOR) += mipi.o
GCOV_PROFILE := $(CONFIG_GCOV_KERNEL)

mipi-objs := \
	mipi_dsi_dev.o \
	mipi_dsi_host.o \
	dual_mipi_dsi_host.o \
	mipi_dsi_ulps.o \
	mipi_dsi_partial_update.o \
	mipi_dsi_lcd.o \
	mipi_dsi_dfr.o \
	mipi_dsi_bit_clk_upt_helper.o

ifeq ($(CONFIG_DKMD_DPU_VERSION),$(filter $(CONFIG_DKMD_DPU_VERSION), 740 741))
mipi-objs += \
	config/v740/mipi_config.o
else
mipi-objs += \
	config/v$(CONFIG_DKMD_DPU_VERSION)/mipi_config.o
endif

ifeq ($(CONFIG_DKMD_DPU_VERSION),$(filter $(CONFIG_DKMD_DPU_VERSION), 740 741))
ifeq ($(chip_type),)
mipi-objs += \
	config/v740/mipi_config_pg.o
else
mipi-objs += \
	config/v740/mipi_config_pg_$(chip_type).o
endif
endif

ifeq ($(CONFIG_DKMD_DPU_VERSION),$(filter $(CONFIG_DKMD_DPU_VERSION), 720))
mipi-objs += \
	mipi_cdphy_utils_v720.o
else
ifeq ($(CONFIG_DKMD_DPU_VERSION),$(filter $(CONFIG_DKMD_DPU_VERSION), 800))
mipi-objs += \
	mipi_cdphy_utils_v8xx.o
else
mipi-objs += \
	mipi_cdphy_utils.o
endif
endif

EXTRA_CFLAGS += \
	-I$(srctree)/include/ \
	-I$(srctree)/arch/arm64/include/ \
	-I$(srctree)/include/platform_include/display/dkmd/ \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/peri \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/isr \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/mipi \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/mipi/config \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/dsc \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/spr \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/offline \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/dp_ctrl \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/panel \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/res_mgr/config \
	-I$(srctree)/drivers/platform_drivers/ap/platform/common \

DS_SOC_CHIP_NAME := $(strip $(subst ", , $(CONFIG_VENDOR_CHIP_PREFIX)))
CFLAGS_MODULE += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow
# If you want to compile into modular drive, will need to decide
# according to the following configuration to compile
ifeq ($(CONFIG_DKMD_DPU_CONNECTOR),m)
EXTRA_CFLAGS += \
	-I$(srctree)/drivers/platform_drivers/ap/platform/$(DS_SOC_CHIP_NAME)_es

KERNEL_DIR = $(ANDROID_BUILD_TOP)/kernel/linux-5.10
KERNEL_OUT = $(ANDROID_PRODUCT_OUT)/obj/KERNEL_OBJ
PWD := $(shell pwd)
ARCH := arm64
CROSS_COMPILE := aarch64-linux-android-
CFLAGS_MODULE += -fno-pic

default:
	$(MAKE) -C $(KERNEL_OUT) ARCH=$(ARCH) CFLAGS=$(EXTRA_CFLAGS) CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) modules
	$(MAKE) -C $(KERNEL_OUT) M=$(PWD) INSTALL_MOD_PATH=$(PWD)/signed INSTALL_MOD_STRIP=1 modules_install
endif

clean:
	rm -rf *.o .*.cmd *.ko *.mod.c .tmp_versions *.symvers *.order *.symversions signed .*.d
	find . -name "*.o" | xargs rm -f
	find . -name ".*.cmd" | xargs rm -f
	find . -name ".*.d" | xargs rm -f
	find . -name "*.gcno" | xargs rm -f
