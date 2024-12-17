ccflags-y += -Wextra -Wshadow

obj-$(CONFIG_DPU_COMPOSER) += hisi_dpu_composer.o

hisi_dpu_composer-objs := \
	composer_adaptor.o \
	composer_present.o \
	dacc/dpu_dacc.o \
	init/dpu_comp_init.o \
	ldi/dpu_ldi.o \
	irq/dpu_irq.o \
	vactive/dpu_comp_vactive.o \
	notifier/dpu_comp_notifier.o \
	osal/dpu_osal.o \
	smmu/dpu_comp_smmu.o \
	smmu/dpu_iommu.o \
	debug/dpu_debug.o \
	dbuf/dpu_dbuf.o \
	dvfs/dpu_dvfs.o \
	abnormal/dpu_comp_abnormal_handle.o \
	dpu_layerbuf_sync.o

# v700
hisi_dpu_composer-objs += \
	dpu_composer_drv.o \
	composer_power.o \
	plat/plat_get_dpu_index.o \
	config/dpu_comp_config_v700.o

EXTRA_CFLAGS += \
	-I$(srctree)/include \

EXTRA_CFLAGS += \
	-I$(srctree)/platform_source/display/drivers/dpu_drm \
	-I$(srctree)/platform_source/display/drivers/dpu_drm/cmdlist/pub \
	-I$(srctree)/platform_source/display/drivers/dpu_drm/composer \
	-I$(srctree)/platform_source/display/drivers/dpu_drm/opr_mgr \
	-I$(srctree)/platform_source/display/drivers/dpu_drm/opr_mgr/operators \
	-I$(srctree)/platform_source/display/drivers/dpu_drm/composer/dksm \
	-I$(srctree)/platform_source/display/drivers/dpu_drm/composer/plat \
	-I$(srctree)/platform_source/display/drivers/dpu_drm/mipi \
	-I$(srctree)/platform_source/display/drivers/dpu_drm/panel \
	-I$(srctree)/platform_source/display/drivers/dpu_drm/drm_gem \

ifeq ($(chip_type),)
EXTRA_CFLAGS += \
	-I$(srctree)/drivers/platform_drivers/ap/platform/$(TARGET_PRODUCT)
else
EXTRA_CFLAGS += \
	-I$(srctree)/drivers/platform_drivers/ap/platform/$(TARGET_PRODUCT)_$(chip_type)
endif

ifeq ($(CONFIG_DPU_COMPOSER),m)
KERNEL_DIR = $(ANDROID_BUILD_TOP)/kernel/linux-5.10
KERNEL_OUT = $(ANDROID_PRODUCT_OUT)/obj/KERNEL_OBJ
PWD := $(shell pwd)
ARCH := arm64
CROSS_COMPILE := aarch64-linux-android-
CFLAGS_MODULE += -fno-pic

KBUILD_EXTRA_SYMBOLS += $(PWD)/../cmdlist/Module.symvers
export KBUILD_EXTRA_SYMBOLS

default:
	$(MAKE) -C $(KERNEL_OUT) ARCH=$(ARCH) CFLAGS=$(EXTRA_CFLAGS) CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) modules
	$(MAKE) -C $(KERNEL_OUT) M=$(PWD) INSTALL_MOD_PATH=$(PWD)/signed INSTALL_MOD_STRIP=1 modules_install
endif

clean:
	rm -rf *.o .*.cmd *.ko *.mod *.mod.c .tmp_versions *.symvers *.order *.symversions signed .*.d
	find . -name "*.o" | xargs rm -f
	find . -name ".*.cmd" | xargs rm -f
	find . -name ".*.d" | xargs rm -f
	find . -name "*.gcno" | xargs rm -f