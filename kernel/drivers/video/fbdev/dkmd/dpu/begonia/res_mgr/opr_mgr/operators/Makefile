obj-$(CONFIG_DKMD_DPU_RES_MGR) += dpu_operators.o
GCOV_PROFILE := $(CONFIG_GCOV_KERNEL)

dpu_operators-objs := \
	opr_format.o \
	cmd_manager.o \
	cmd_manager_impl.o \
	opr_cmd_data.o \
	opr_cmd_data_interface.o \
	sdma/sdma_cmd_data.o \
	ov/ov_cmd_data.o \
	dpp/dpp_cmd_data.o \
	dsc/dsc_cmd_data.o \
	itfsw/itfsw_cmd_data.o \

ifeq ($(CONFIG_DKMD_DPU_VERSION),$(filter $(CONFIG_DKMD_DPU_VERSION), 800 820))
dpu_operators-objs += \
	cmd_manager_scene_info_v8xx.o \
	sdma/sdma_cmd_data_plat_v8xx.o \
	ov/ov_cmd_data_plat_v8xx.o \
	dpp/dpp_cmd_data_plat_v8xx.o \
	itfsw/itfsw_cmd_data_plat_v8xx.o
else
dpu_operators-objs += \
	cmd_manager_scene_info_v7xx.o \
	sdma/sdma_cmd_data_plat_v7xx.o \
	ov/ov_cmd_data_plat_v7xx.o \
	dpp/dpp_cmd_data_plat_v7xx.o \
	itfsw/itfsw_cmd_data_plat_v7xx.o
endif

EXTRA_CFLAGS += \
	-I$(srctree)/include/platform_include/display/dkmd/ \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm \
	-I$(srctree)/drivers/video/fbdev/dkmd/cmdlist \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/res_mgr/ \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/res_mgr/opr_mgr/operators \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/dkmd_interface \
	-I$(srctree)/drivers/video/fbdev/dkmd/dpu/begonia/composer \
	-I$(srctree)/arch/arm64/include/ \
	-I$(srctree)/include/

CFLAGS_MODULE += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow
ifeq ($(CONFIG_DKMD_DPU_ENABLE),m)
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
