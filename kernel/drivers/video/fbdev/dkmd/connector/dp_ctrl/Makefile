obj-$(CONFIG_DKMD_DPU_DP) += dp_ctrl.o

GCOV_PROFILE := $(CONFIG_GCOV_KERNEL)

dp_ctrl-objs := \
	controller/dp_avgen_base.o \
	controller/hidptx/hidptx_dp_aux.o \
	controller/hidptx/hidptx_dp_irq.o \
	controller/hidptx/hidptx_dp_avgen.o \
	controller/hidptx/hidptx_dp_core.o \
	controller/dsc/hidptx_dsc_config.o \
	controller/dsc/dsc_config_base.o \
	controller/psr/psr_config_base.o \
	link/dp_irq.o \
	link/dp_aux.o \
	link/dp_edid.o \
	link/dp_link_training.o \
	link/dp_mst_topology.o \
	link/dp_dsc_algorithm.o \
	link/drm_dp_helper.o \
	dp_ctrl_dev.o

ifeq ($(CONFIG_DKMD_DPU_VERSION),$(filter $(CONFIG_DKMD_DPU_VERSION), 720))
dp_ctrl-objs += \
	phy/hidptx_phy_config_v720.o \
	config/v720/dp_ctrl_config.o \
	dpu_dp_dbg.o
else ifeq ($(CONFIG_DKMD_DPU_VERSION),$(filter $(CONFIG_DKMD_DPU_VERSION), 740))
dp_ctrl-objs += \
	phy/hidptx_phy_config_v740.o \
	config/v740/dp_ctrl_config.o \
	dpu_dp_dbg_v740.o
else
dp_ctrl-objs += \
	phy/hidptx_phy_config.o \
	config/default/dp_ctrl_config.o \
	dpu_dp_dbg.o
endif

EXTRA_CFLAGS += \
	-I$(srctree)/include/ \
	-I$(srctree)/arch/arm64/include/ \
	-I$(srctree)/include/platform_include/display/dkmd/ \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/peri \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/isr \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/res_mgr/dvfs \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/mipi \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/dsc \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/spr \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/offline \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/dp \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/dp_ctrl \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/dp_ctrl/controller \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/dp_ctrl/controller/hidptx \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/dp_ctrl/controller/psr \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/dp_ctrl/link \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/dp_ctrl/phy \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/hdcptx \
	-I$(srctree)/drivers/platform_drivers/ap/platform/common \

ifeq ($(CONFIG_DKMD_DPU_VERSION),$(filter $(CONFIG_DKMD_DPU_VERSION), 720))
EXTRA_CFLAGS += \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/dp_ctrl/config/v720
else ifeq ($(CONFIG_DKMD_DPU_VERSION),$(filter $(CONFIG_DKMD_DPU_VERSION), 740))
EXTRA_CFLAGS += \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/dp_ctrl/config/v740
else
EXTRA_CFLAGS += \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/dp_ctrl/config/default
endif

ifeq ($(chip_type), csv2)
EXTRA_CFLAGS += -DNOT_SURPORT_PG
endif

DS_CHIP_TYPE := $(strip $(subst ", , $(CONFIG_VENDOR_CHIP_SUFFIX)))
DS_SOC_CHIP_NAME := $(strip $(subst ", , $(CONFIG_VENDOR_CHIP_PREFIX)))
CFLAGS_MODULE += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow
# If you want to compile into modular drive, will need to decide
# according to the following configuration to compile
ifeq ($(CONFIG_DKMD_DPU_CONNECTOR),m)
ifeq ($(DS_CHIP_TYPE),)
EXTRA_CFLAGS += \
	-I$(srctree)/drivers/platform_drivers/ap/platform/$(DS_SOC_CHIP_NAME)
else
EXTRA_CFLAGS += \
	-I$(srctree)/drivers/platform_drivers/ap/platform/$(DS_SOC_CHIP_NAME)_$(DS_CHIP_TYPE)
endif

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
