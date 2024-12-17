obj-$(CONFIG_DKMD_DKSM) += dkmd_dksm.o
GCOV_PROFILE := $(CONFIG_GCOV_KERNEL)

dkmd_dksm-objs = \
	dksm_debug.o \
	buf_sync/dkmd_buf_sync.o \
	chrdev/dkmd_sysfs.o \
	chrdev/dkmd_chrdev.o \
	fence/dkmd_fence_utils.o \
	fence/dkmd_release_fence.o \
	fence/dkmd_acquire_fence.o \
	timeline/dkmd_timeline.o \
	peri/dkmd_peri.o \
	isr/dkmd_isr.o \
	notify/dkmd_notify.o

ifeq ($(CONFIG_HUAWEI_OHOS_DSM),y)
dkmd_dksm-objs += \
	dksm_dmd_ohos.o
else
dkmd_dksm-objs += \
	dksm_dmd.o
endif

ifeq ($(CONFIG_WLT_DISPLAY),y)
dkmd_dksm-objs += \
	wlt/dpu_wlt.o
endif

EXTRA_CFLAGS += \
	-I$(srctree)/include/platform_include/display/dkmd/ \
	-I$(srctree)/drivers/video/fbdev/dkmd/cmdlist \
	-I$(srctree)/drivers/video/fbdev/dkmd/cmdlist/include \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm\wlt \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/buf_sync \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/chrdev \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/fence \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/timeline \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/isr

CFLAGS_MODULE += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow
# If you want to compile into modular drive, will need to decide
# according to the following configuration to compile
DS_SOC_CHIP_NAME := $(strip $(subst ", , $(CONFIG_VENDOR_CHIP_PREFIX)))
ifeq ($(CONFIG_DKMD_DKSM),m)
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
	rm -rf *.o .*.cmd *.ko *.mod *.mod.c .tmp_versions *.symvers *.order *.symversions signed .*.d
	find . -name "*.o" | xargs rm -f
	find . -name ".*.cmd" | xargs rm -f
	find . -name ".*.d" | xargs rm -f
	find . -name "*.gcno" | xargs rm -f
