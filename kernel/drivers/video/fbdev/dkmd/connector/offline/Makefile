obj-$(CONFIG_DKMD_DPU_OFFLINE) += dpu_offline.o

GCOV_PROFILE := $(CONFIG_GCOV_KERNEL)

dpu_offline-objs := \
	dpu_offline_drv.o

ifeq ($(CONFIG_DKMD_DPU_VERSION),740)
dpu_offline-objs += \
	dpu_offline_dev_v$(CONFIG_DKMD_DPU_VERSION).o
else
dpu_offline-objs += \
	dpu_offline_dev.o
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
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/dsc \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/spr \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/dp_ctrl \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/offline

CFLAGS_MODULE += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow
DS_SOC_CHIP_NAME := $(strip $(subst ", , $(CONFIG_VENDOR_CHIP_PREFIX)))
ifeq ($(CONFIG_DKMD_DPU_OFFLINE),m)
EXTRA_CFLAGS += \
	-I$(srctree)/drivers/platform_drivers/ap/platform/$(DS_SOC_CHIP_NAME)_es
endif

clean:
	rm -rf *.o .*.cmd *.ko *.mod.c .tmp_versions *.symvers *.order *.symversions signed .*.d
	find . -name "*.o" | xargs rm -f
	find . -name ".*.cmd" | xargs rm -f
	find . -name ".*.d" | xargs rm -f
	find . -name "*.gcno" | xargs rm -f
