obj-$(CONFIG_DPU_PANEL) += hisi_dpu_panel.o

# v700
hisi_dpu_panel-objs := \
	dpu_panel.o \
	panel_boe_nt37700p_fake.o

EXTRA_CFLAGS += \
	-I$(srctree)/platform_source/display/drivers/dpu_drm/mipi \
	-I$(srctree)/platform_source/display/drivers/dpu_drm/panel

EXTRA_CFLAGS += \
	-I$(srctree)/include

ifeq ($(CONFIG_DPU_PANEL),m)
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