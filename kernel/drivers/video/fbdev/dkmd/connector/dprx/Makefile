obj-$(CONFIG_DKMD_DPU_DP) += libdprx.o

libdprx-objs := hidprx.o \
				hidprx_edid.o \
				hidprx_sdp.o \
				hidprx_core.o \
				hidprx_irq.o \
				hidprx_ctrl.o \
				hidprx_dbg.o

EXTRA_CFLAGS += \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/dsc \
	-I$(srctree)/include/platform_include/display/dkmd/ \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/chrdev \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/peri \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/isr \
	-I$(srctree)/arch/arm64/include/ \
	-I$(srctree)/include/

KERNEL_DIR = $(ANDROID_BUILD_TOP)/kernel/linux-5.10
KERNEL_OUT = $(ANDROID_PRODUCT_OUT)/obj/KERNEL_OBJ
PWD := $(shell pwd)
ARCH := arm64
CROSS_COMPILE := aarch64-linux-android-
CFLAGS_MODULE := -fno-pic

default:
	$(MAKE) -C $(KERNEL_OUT) ARCH=$(ARCH) CFLAGS=$(EXTRA_CFLAGS) CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) modules
	$(MAKE) -C $(KERNEL_OUT) M=$(PWD) INSTALL_MOD_PATH=$(PWD)/signed INSTALL_MOD_STRIP=1 modules_install

clean:
	rm -rf *.o .*.cmd *.ko *.mod.c .tmp_versions *.symvers *.order *.symversions signed .*.d
