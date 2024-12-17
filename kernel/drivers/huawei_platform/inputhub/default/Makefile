#ifdef CONFIG_CONTEXTHUB_SHMEM
ccflags-y  += -I$(srctree)/platform_source/smart/drivers/channels/shmem
subdir-ccflags-y += -I$(srctree)/platform_source/smart/drivers/channels/shmem
#endif

#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
ccflags-y  += -I$(srctree)/platform_source/smart/drivers
subdir-ccflags-y += -I$(srctree)/platform_source/smart/drivers
#endif

#ifdef CONFIG_HUAWEI_HOST_TS_KIT
ccflags-y  += -I$(srctree)/drivers/devkit/tpkit/hostprocessing/
subdir-ccflags-y += -I$(srctree)/drivers/devkit/tpkit/hostprocessing/
#endif

ifeq ($(TARGET_BOARD_PLATFORM),baltimore)
EXTRA_CFLAGS += -I$(srctree)/drivers/video/fbdev/dkmd/dpu/azalea/effect/
EXTRA_CFLAGS += -I$(srctree)/drivers/video/fbdev/dkmd/dpu/azalea/
EXTRA_CFLAGS += -I$(srctree)/drivers/huawei_platform/blpwm/
EXTRA_CFLAGS += -DDSS_ALSC_NOISE
EXTRA_CFLAGS += -DCONFIG_DPU_FB_ALSC
subdir-ccflags-y += -I$(srctree)/drivers/video/fbdev/dkmd/dpu/azalea/effect/
subdir-ccflags-y += -I$(srctree)/drivers/video/fbdev/dkmd/dpu/azalea/
subdir-ccflags-y += -DDSS_ALSC_NOISE
subdir-ccflags-y += -DCONFIG_DPU_FB_ALSC
endif

ifdef CONFIG_INPUTHUB_PLAT_C
EXTRA_CFLAGS += -I$(srctree)/include/platform_include/display/dkmd/
EXTRA_CFLAGS += -DCONFIG_DPU_FB_ALSC
EXTRA_CFLAGS += -DDKMD_ALSC_NOISE
EXTRA_CFLAGS += -DCHA_MAG_DETACH
EXTRA_CFLAGS += -DALS_AFTER_SALE_STD
EXTRA_CFLAGS += -DCONFIG_PROX_LCDTYPE
subdir-ccflags-y += -I$(srctree)/include/platform_include/display/dkmd/
subdir-ccflags-y += -DCONFIG_DPU_FB_ALSC
subdir-ccflags-y += -DDKMD_ALSC_NOISE
subdir-ccflags-y += -DCHA_MAG_DETACH
subdir-ccflags-y += -DALS_AFTER_SALE_STD
subdir-ccflags-y += -DCONFIG_PROX_LCDTYPE
subdir-ccflags-y += -I$(srctree)/platform_source/smart/drivers/channels/io_die
obj-$(CONFIG_INPUTHUB_20)  += carcrash_channel.o
ifdef CONFIG_LCD_KIT_DRIVER
ifdef CONFIG_LCD_KIT_HISI
EXTRA_CFLAGS += -DALS_PANEL_VERSION
subdir-ccflags-y += -DALS_PANEL_VERSION
EXTRA_CFLAGS += -I$(srctree)/drivers/devkit/lcdkit/lcdkit3.0/kernel/dkmd/include/
EXTRA_CFLAGS += -I$(srctree)/drivers/devkit/lcdkit/lcdkit3.0/kernel/common/include/
subdir-ccflags-y += -I$(srctree)/drivers/devkit/lcdkit/lcdkit3.0/kernel/dkmd/include/
subdir-ccflags-y += -I$(srctree)/drivers/devkit/lcdkit/lcdkit3.0/kernel/common/include/
endif
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM),burbank)
EXTRA_CFLAGS += -I$(srctree)/drivers/video/fbdev/dkmd/dpu/azalea/effect/
EXTRA_CFLAGS += -I$(srctree)/drivers/video/fbdev/dkmd/dpu/azalea/
EXTRA_CFLAGS += -I$(srctree)/drivers/huawei_platform/blpwm/
EXTRA_CFLAGS += -DDSS_ALSC_NOISE
EXTRA_CFLAGS += -DCONFIG_DPU_FB_ALSC
EXTRA_CFLAGS += -DALS_AFTER_SALE_STD
EXTRA_CFLAGS += -DBBK_MAG_DETACH
subdir-ccflags-y += -I$(srctree)/drivers/video/fbdev/dkmd/dpu/azalea/effect/
subdir-ccflags-y += -I$(srctree)/drivers/video/fbdev/dkmd/dpu/azalea/
subdir-ccflags-y += -DDSS_ALSC_NOISE
subdir-ccflags-y += -DCONFIG_DPU_FB_ALSC
subdir-ccflags-y += -DALS_AFTER_SALE_STD
subdir-ccflags-y += -DBBK_MAG_DETACH
endif

EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/tzdriver

subdir-ccflags-y += -I$(srctree)/$(src)/
subdir-ccflags-y += -I$(srctree)/$(src)/acc/
subdir-ccflags-y += -I$(srctree)/$(src)/airpress/
subdir-ccflags-y += -I$(srctree)/$(src)/als/
subdir-ccflags-y += -I$(srctree)/$(src)/cap_prox/
subdir-ccflags-y += -I$(srctree)/$(src)/gyro/
subdir-ccflags-y += -I$(srctree)/$(src)/handpress/
subdir-ccflags-y += -I$(srctree)/$(src)/mag/
subdir-ccflags-y += -I$(srctree)/$(src)/motion/
subdir-ccflags-y += -I$(srctree)/$(src)/ps/
subdir-ccflags-y += -I$(srctree)/$(src)/therm/
subdir-ccflags-y += -I$(srctree)/$(src)/vibrator/
subdir-ccflags-y += -I$(srctree)/$(src)/tof/

# contexthub Driver
obj-$(CONFIG_INPUTHUB_20)		+= contexthub_boot.o
obj-$(CONFIG_INPUTHUB_20)		+= contexthub_route.o
obj-$(CONFIG_INPUTHUB_20)		+= contexthub_route_plu.o
obj-$(CONFIG_INPUTHUB_20)		+= sensor_config.o
obj-$(CONFIG_INPUTHUB_20)       += contexthub_logbuff.o
obj-$(CONFIG_INPUTHUB_20)       += contexthub_debug.o
obj-$(CONFIG_INPUTHUB_20)	+= sensor_channel.o
obj-$(CONFIG_INPUTHUB_20)  += motion_channel.o
obj-$(CONFIG_INPUTHUB_20)  += sensor_detect.o
obj-$(CONFIG_INPUTHUB_20)  += sensor_sysfs.o
obj-$(CONFIG_INPUTHUB_20)  += sensor_info.o
obj-$(CONFIG_INPUTHUB_20)  += sensor_feima.o
obj-$(CONFIG_INPUTHUB_20)  += contexthub_recovery.o
obj-$(CONFIG_INPUTHUB_20)  += contexthub_pm.o
obj-$(CONFIG_INPUTHUB_20)  += fingerprint_channel.o
obj-$(CONFIG_INPUTHUB_20)  += contexthub_ext_log.o
obj-$(CONFIG_INPUTHUB_20)  += fingerprint_ud_channel.o
obj-$(CONFIG_INPUTHUB_20)  += vibrator_channel.o
obj-$(CONFIG_INPUTHUB_20)  += big_data_channel.o
obj-$(CONFIG_INPUTHUB_20)  += kbhub_channel.o
obj-$(CONFIG_INPUTHUB_20)  += modem_route.o
obj-$(CONFIG_INPUTHUB_20)  += thp_channel.o
obj-$(CONFIG_INPUTHUB_20)  += mlps_channel.o
obj-$(CONFIG_INPUTHUB_20)  += display_engine_channel.o
obj-$(CONFIG_INPUTHUB_20) += acc/
obj-$(CONFIG_INPUTHUB_20) += airpress/
obj-$(CONFIG_INPUTHUB_20) += als/
obj-$(CONFIG_INPUTHUB_20) += cap_prox/
obj-$(CONFIG_INPUTHUB_20) += gyro/
obj-$(CONFIG_INPUTHUB_20) += handpress/
obj-$(CONFIG_INPUTHUB_20) += mag/
obj-$(CONFIG_INPUTHUB_20) += motion/
obj-$(CONFIG_INPUTHUB_20) += ps/
obj-$(CONFIG_INPUTHUB_20) += therm/
obj-$(CONFIG_INPUTHUB_20) += vibrator/
obj-$(CONFIG_INPUTHUB_20) += tof/
