obj-$(CONFIG_DKMD_DPU_COMPOSER) += dpu_composer.o
GCOV_PROFILE := $(CONFIG_GCOV_KERNEL)

dpu_composer-objs := \
	abnormal/dpu_comp_abnormal_handle.o \
	cmdlist/dpu_cmdlist.o \
	isr/dpu_isr_alsc.o \
	isr/dpu_isr_hiace.o \
	isr/dpu_isr_hdr.o \
	isr/dpu_isr_mdp.o \
	init/dpu_comp_init.o \
	init/dpu_lut_init.o \
	offline/dpu_comp_offline.o \
	online/dpu_comp_online_utils.o \
	online/dpu_comp_vactive.o \
	online/dpu_comp_vsync.o \
	online/dpu_comp_frame_isr.o \
	online/dpu_comp_maintain.o \
	online/dpu_comp_sysfs.o \
	online/dfr/dpu_comp_dfr.o \
	online/dfr/dpu_comp_dfr_constant.o \
	online/dfr/dpu_comp_dfr_ltps.o \
	online/dfr/dpu_comp_dfr_ltps_longh.o \
	online/ppc/dpu_comp_ppc.o \
	online/ppc/dpu_panel_partial_ctl.o \
	online/ppc/dpu_ppc_status_control.o \
	smmu/dpu_comp_smmu.o \
	secure/dpu_comp_secure.o \
	bl/dpu_comp_bl.o \
	ddr/ddr_dvfs.o \
	effect/alsc/dpu_effect_alsc_debug.o \
	effect/alsc/dpu_effect_alsc.o \
	effect/hiace/dpu_effect_hiace.o \
	effect/hiace/dpu_hiace_init.o \
	esd/dpu_comp_esd.o \
	dsi_clk_upt/dpu_comp_mipi_dsi_bit_clk_upt.o \
	lowtemp/dpu_comp_low_temp_handler.o

ifeq ($(CONFIG_TEE_TUI),y)
dpu_composer-objs += \
	secure/dpu_comp_tui.o
endif

ifeq ($(CONFIG_DKMD_DPU_VERSION),$(filter $(CONFIG_DKMD_DPU_VERSION), 800 820))
dpu_composer-objs += \
	abnormal/dpu_comp_abnormal_dump_lbuf_v8xx.o \
	abnormal/dpu_comp_abnormal_dump_dacc_v8xx.o \
	dump/dpu_debug_dump_v8xx.o \
	isr/dpu_isr_mdp_handler_v8xx.o \
	smmu/dpu_comp_smmu_plat_v8xx.o \
	isr/dpu_isr_dvfs_v8xx.o \
	dacc/dpu_dacc_v8xx.o
else
dpu_composer-objs += \
	abnormal/dpu_comp_abnormal_dump_lbuf_v7xx.o \
	abnormal/dpu_comp_abnormal_dump_dacc_v7xx.o \
	dump/dpu_debug_dump_v7xx.o \
	isr/dpu_isr_dvfs_v7xx.o \
	isr/dpu_isr_mdp_handler_v7xx.o \
	smmu/dpu_comp_smmu_plat_v7xx.o \
	dacc/dpu_dacc_v7xx.o
endif

ifeq ($(CONFIG_DKMD_DPU_VERSION),$(filter $(CONFIG_DKMD_DPU_VERSION), 700))
ifeq ($(chip_type),)
dpu_composer-objs += \
	config/v$(CONFIG_DKMD_DPU_VERSION)/dpu_comp_config.o
else
dpu_composer-objs += \
	config/v$(CONFIG_DKMD_DPU_VERSION)/dpu_comp_config_$(chip_type).o
endif
dpu_composer-objs += \
	online/dfr/dpu_comp_dfr_te_skip_v700.o \
	online/dfr/dpu_comp_dfr_te_skip_acpu.o \
	online/dfr/dpu_comp_dfr_longh_te_skip_v700.o \
	abnormal/dpu_comp_abnormal_dvfs_v700.o
else
dpu_composer-objs += \
	config/v$(CONFIG_DKMD_DPU_VERSION)/dpu_comp_config.o \
	online/dfr/dpu_comp_dfr_te_skip.o \
	abnormal/dpu_comp_abnormal_dvfs.o
endif

ifeq ($(CONFIG_DKMD_DPU_VERSION),$(filter $(CONFIG_DKMD_DPU_VERSION), 740))
dpu_composer-objs += \
	offline/config/v740/dpu_offline_config.o
else
dpu_composer-objs += \
	offline/config/common/dpu_offline_config.o
endif

ifeq ($(CONFIG_DKMD_DPU_VERSION),$(filter $(CONFIG_DKMD_DPU_VERSION), 740 741))
dpu_composer-objs += \
	dpu_comp_mgr_v$(CONFIG_DKMD_DPU_VERSION).o \
	power/dpu_comp_power_v$(CONFIG_DKMD_DPU_VERSION).o \
	online/dpu_comp_online_v740.o \
	present/dpu_comp_present_v740.o \
	scene/dpu_comp_scene_v740.o \
	isr/dpu_isr_sdp_v740.o \
	isr/dpu_isr_m1_qic.o \
	fusa/dpu_comp_fusa_handler.o
else
dpu_composer-objs += \
	dpu_comp_mgr.o \
	power/dpu_comp_power.o \
	present/dpu_comp_present.o \
	scene/dpu_comp_scene.o \
	isr/dpu_isr_sdp.o \
	online/dpu_comp_online.o
endif

ifeq ($(CONFIG_DKMD_DPU_VERSION),$(filter $(CONFIG_DKMD_DPU_VERSION), 740 741))
ifeq ($(chip_type),)
dpu_composer-objs += \
	dpu_comp_mgr_v74x_pg.o
else
dpu_composer-objs += \
	dpu_comp_mgr_v74x_pg_$(chip_type).o
endif
endif

ifeq ($(CONFIG_DKMD_DPU_VERSION),$(filter $(CONFIG_DKMD_DPU_VERSION), 720 740))
dpu_composer-objs += \
	init/localdimming/dpu_ld_init.o
EXTRA_CFLAGS += -DSURPORT_LOCAL_DIMMING
endif

ifeq ($(CONFIG_DKMD_DPU_AOD),y)
dpu_composer-objs += \
	aod/dpu_sh_aod.o
endif

ifeq ($(CONFIG_DPU_TBU_ECC),y)
dpu_composer-objs += \
	isr/dpu_isr_ecc.o
endif

EXTRA_CFLAGS += \
	-I$(srctree)/platform_source/cee/drivers/perf_stat/ \
	-I$(srctree)/include/ \
	-I$(srctree)/arch/arm64/include/ \
	-I$(srctree)/include/platform_include/display/dkmd/ \
	-I$(srctree)/drivers/video/fbdev/dkmd/cmdlist \
	-I$(srctree)/drivers/video/fbdev/dkmd/cmdlist/include \
	-I$(srctree)/drivers/video/fbdev/dkmd/backlight \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/timeline \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/buf_sync \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/fence \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/chrdev \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/peri \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/isr \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/wlt \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/mipi \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/dsc \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/spr \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/dp_ctrl \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/dp \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/offline \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/panel \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/res_mgr \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/res_mgr/dvfs \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/res_mgr/config \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/device \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/bl \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/ddr \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/abnormal \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/cmdlist \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/config \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/dacc \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/isr \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/init \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/offline \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/online \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/online/dfr \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/online/ppc \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/power \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/present \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/scene \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/smmu \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/aod \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/effect/alsc \
	-I$(srctree)/drivers/video/fbdev/dkmd/product \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/secure \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/esd \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/dsi_clk_upt \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/dump \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/lowtemp \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer/fusa

ifeq ($(CONFIG_TEE_TUI),y)
EXTRA_CFLAGS += \
	-I$(srctree)/drivers/platform_drivers/tzdriver
endif

CFLAGS_MODULE += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow

ifeq ($(CONFIG_DKMD_DPU_COMPOSER),m)
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
