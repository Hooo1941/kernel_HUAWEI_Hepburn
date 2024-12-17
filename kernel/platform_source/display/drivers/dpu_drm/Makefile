obj-$(CONFIG_DPU_OPR_MGR) += opr_mgr/
obj-$(CONFIG_DPU_MIPI_DSI) += mipi/
obj-$(CONFIG_DPU_PANEL) += panel/
obj-$(CONFIG_DPU_COMPOSER) += composer/
obj-$(CONFIG_DPU_CMDLIST) += cmdlist/
obj-$(CONFIG_DRM_GEM_HEAP) += heap/

obj-$(CONFIG_DPU_DRM) += dpu-drm.o

EXTRA_CFLAGS += \
		-I$(srctree)/include \
		-I$(srctree)/include/drm \
		-I$(srctree)/platform_source/display/drivers/dpu_drm/drm_gem \
		-I$(srctree)/platform_source/display/drivers/dpu_drm/heap \
		-I$(srctree)/../../vendor/hisi/ap/kernel/include

dpu-drm-objs := \
	dpu_drm_drv.o \
	dpu_kms.o \
	drm_gem/hisi_drm_gem.o \
	drm_gem/hisi_drm_gem_heap_helper.o \
	drm_gem/hisi_gem_dmabuf.o \
	drm_gem/hisi_drm_debug.o

hisi-dss-iommu-y := \
	fb_dev/hisi_fb.o \
	fb_dev/hisi_dss_ion.o \
	fb_dev/hisi_dss_iommu.o \
	fb_dev/hisi_fbdev.o

obj-$(CONFIG_DRM_DSS_IOMMU) += hisi-dss-iommu.o

ifeq ($(CONFIG_DPU_DRM),m)

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
	rm -rf *.o .*.cmd *.ko *.mod *.mod.c .tmp_versions *.symvers *.order *.symversions signed .*.d
	find . -name "*.o" | xargs rm -f
	find . -name ".*.cmd" | xargs rm -f
	find . -name ".*.d" | xargs rm -f
	find . -name "*.gcno" | xargs rm -f

