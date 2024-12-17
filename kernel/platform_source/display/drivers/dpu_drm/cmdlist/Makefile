ccflags-y += -Wextra -Wshadow

obj-${CONFIG_DPU_CMDLIST} += dpu_drm_cmdlist.o

dpu_drm_cmdlist-objs := \
	./adaptor/cmdlist_module.o \
	./infra/cmdlist_dev_id_utils.o \
	./infra/default_memory_allocator.o \
	./infra/cmdlist_user_cmd.o \
	./domain/entity/cmdlist_device_repo.o \
	./domain/service/cmdlist_proc_service.o \
	./domain/service/cmdlist_device_service.o \
	./domain/service/cmdlist_mntn_service.o \
	./domain/factory/cmdlist_device_factory.o

ifneq ($(CONFIG_LOCAL_COMPILE), y)
obj-${CONFIG_DPU_CMDLIST} += cmdlist_7xx.o
cmdlist_7xx-objs := \
	./adaptor/cmdlist_impl/cmdlist_7xx/cmdlist_7xx_task.o \
	./adaptor/cmdlist_impl/cmdlist_7xx/cmdlist_7xx_dev.o \
	./adaptor/cmdlist_impl/cmdlist_7xx/cmdlist_7xx_channel.o \
	./adaptor/cmdlist_impl/cmdlist_7xx/cmdlist_7xx_node.o \
	./infra/cmdlist_mempool.o
endif

obj-${CONFIG_DPU_CMDLIST} += cmdlist_chr.o
cmdlist_chr-objs := \
	./adaptor/cmdlist_chr_dev.o

obj-${CONFIG_DPU_CMDLIST} += cmdlist_dbgfs.o
cmdlist_dbgfs-objs := \
	./adaptor/cmdlist_dbgfs.o

obj-${CONFIG_DPU_CMDLIST} += virt_cmdlist.o
virt_cmdlist-objs := \
	./adaptor/cmdlist_impl/virtual_cmdlist/virt_cmdlist_dev.o \
	./adaptor/cmdlist_impl/virtual_cmdlist/virt_memory_allocator.o \
	./infra/cmdlist_mempool.o

ifeq ($(chip_type),)
EXTRA_CFLAGS += \
	-I$(srctree)/drivers/platform_drivers/ap/platform/$(TARGET_PRODUCT)
else
EXTRA_CFLAGS += \
	-I$(srctree)/drivers/platform_drivers/ap/platform/$(TARGET_PRODUCT)_$(chip_type)
endif

EXTRA_CFLAGS += \
	-I$(srctree)/include \
	-I$(srctree)/platform_source/display/drivers/dpu_drm/cmdlist \
	-I$(srctree)/platform_source/display/drivers/dpu_drm/cmdlist/pub \
	-I$(srctree)/platform_source/display/drivers/dpu_drm/cmdlist/pub/adaptor \
	-I$(srctree)/platform_source/display/drivers/dpu_drm/cmdlist/domain/entity \
	-I$(srctree)/platform_source/display/drivers/dpu_drm/cmdlist/domain/service \
	-I$(srctree)/platform_source/display/drivers/dpu_drm/cmdlist/domain/factory \
	-I$(srctree)/platform_source/display/drivers/dpu_drm/cmdlist/infra

ccflags-y := ${EXTRA_CFLAGS}

ifeq ($(CONFIG_DPU_CMDLIST),m)
KERNEL_DIR = $(ANDROID_BUILD_TOP)/kernel/linux-5.10
KERNEL_OUT = $(ANDROID_PRODUCT_OUT)/obj/KERNEL_OBJ
PWD := $(shell pwd)
ARCH := arm64
CROSS_COMPILE := aarch64-linux-android-
CFLAGS_MODULE += -fno-pic

default:
	$(MAKE) -C $(KERNEL_OUT) ARCH=$(ARCH) CFLAGS="$(EXTRA_CFLAGS)" CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) modules
	$(MAKE) -C $(KERNEL_OUT) M=$(PWD) INSTALL_MOD_PATH=$(PWD)/signed INSTALL_MOD_STRIP=1 modules_install
endif

clean:
	rm -rf *.o .*.cmd *.ko *.mod *.mod.c .tmp_versions *.symvers *.order *.symversions signed .*.d
	find . -name "*.mod" | xargs rm -f
	find . -name "*.mod.c" | xargs rm -f
	find . -name "*.o" | xargs rm -f
	find . -name ".*.cmd" | xargs rm -f
	find . -name ".*.d" | xargs rm -f
	find . -name "*.gcno" | xargs rm -f

KDIR := /lib/modules/$(shell uname -r)/build
local_host:
	$(MAKE) -C $(KDIR) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" M=$(PWD) modules
