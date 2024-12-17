obj-$(CONFIG_DKMD_DPU_DEVICE) += dpu_device.o
GCOV_PROFILE := $(CONFIG_GCOV_KERNEL)

dpu_device-objs := \
	gfxdev_mgr.o \
	gfxdev_pandisplay_utils.o \
	gfxdev_utils.o \
	fb/dpu_fb.o \
	fb/dpu_ion_mem.o \
	gfx/dpu_gfx.o \
	gfx/dpu_gfx_ion_mem.o

dpu_device-$(CONFIG_DRM) += \
	drm/dpu_drm.o

obj-$(CONFIG_VIRTIO_FB) += virtio/

dpu_device-$(CONFIG_VIRTIO_FB) += \
	vfb/dpu_vfb_fence.o \
	vfb/dpu_vfb_bufmgr.o \
	vfb/dpu_vfb.o

EXTRA_CFLAGS += \
	-I$(srctree)/include/platform_include/display/dkmd/ \
	-I$(srctree)/drivers/video/fbdev/dkmd/cmdlist \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/chrdev \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/fence \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/timeline \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/device \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/res_mgr \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/res_mgr/config \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/res_mgr/dvfs\
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/res_mgr/opr_mgr \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/device/fb \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/device/vfb \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/device/virtio \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/device/drm \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/device/gfx \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer \
	-I$(srctree)/arch/arm64/include/ \
	-I$(srctree)/include/

CFLAGS_MODULE += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow
ifeq ($(CONFIG_DKMD_DPU_DEVICE), m)
ifeq ($(chip_type),)
EXTRA_CFLAGS += \
	-I$(srctree)/drivers/platform_drivers/ap/platform/$(TARGET_PRODUCT)
else
EXTRA_CFLAGS += \
	-I$(srctree)/drivers/platform_drivers/ap/platform/$(TARGET_PRODUCT)_$(chip_type)
endif
endif

clean:
	rm -rf *.o .*.cmd *.ko *.mod *.mod.c .tmp_versions *.symvers *.order *.symversions signed .*.d
	find . -name "*.o" | xargs rm -f
	find . -name ".*.cmd" | xargs rm -f
	find . -name ".*.d" | xargs rm -f
	find . -name "*.gcno" | xargs rm -f
