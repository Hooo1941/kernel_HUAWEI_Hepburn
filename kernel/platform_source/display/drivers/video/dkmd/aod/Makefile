ccflags-y  += -I$(srctree)/drivers/huawei_platform/inputhub/iom7/
#ifdef CONFIG_CONTEXTHUB_SHMEM
ccflags-y  += -I$(srctree)/platform_source/smart/drivers/channels
#endif


obj-y += dpu_aod_device.o

EXTRA_CFLAGS += -I$(srctree)/drivers/huawei_platform/inputhub/default\
		-I$(srctree)/drivers/staging/android

ifeq ($(CONFIG_DKMD_DPU_AOD),y)
EXTRA_CFLAGS += \
	-I$(srctree)/include/ \
	-I$(srctree)/arch/arm64/include/ \
	-I$(srctree)/include/platform_include/display/dkmd/ \
	-I$(srctree)/drivers/video/fbdev/dkmd/cmdlist \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/timeline \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/buf_sync \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/fence \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/chrdev \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/peri \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/isr \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/res_mgr \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/res_mgr/dvfs \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/res_mgr/config \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/device \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/abnormal \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/cmdlist \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/config \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/dacc \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/isr \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/init \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/offline \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/online \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/power \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/present \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/scene \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/smmu \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/aod
else
EXTRA_CFLAGS += -I$(srctree)/drivers/video/fbdev/dkmd/dpu/azalea \
		-I$(srctree)/drivers/video/dkmd/dpu/azalea
endif
