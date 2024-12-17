ifeq ($(TARGET_VERSION_MODE),factory)
	ccflags-y += -DDP_FACTORY_MODE_ENABLE
endif
ifneq ($(TARGET_BUILD_VARIANT),user)
	ccflags-y += -DDP_DEBUG_ENABLE
endif

ifdef CONFIG_HUAWEI_DP_LAHAINA
	ccflags-y := -I$(srctree)/techpack/display/msm
	ccflags-y += -I$(srctree)/techpack/display/msm/sde
	ccflags-y += -I$(srctree)/techpack/display/msm/dp
	ccflags-y += -I$(srctree)/techpack/display/include
	ccflags-y += -I$(srctree)/techpack/display/include/uapi/display
	ccflags-y += -I$(srctree)/include/drm
endif

ifdef CONFIG_HUAWEI_DP_TARO
	ccflags-y += -DDP_DSM_SHIELD
endif

obj-$(CONFIG_HUAWEI_DP_SOURCE) += dp_extra_module.o
dp_extra_module-y += dp_source_switch.o
dp_extra_module-y += dp_interface.o

dp_extra_module-$(CONFIG_HUAWEI_DP_DSM) += dp_dsm.o
dp_extra_module-$(CONFIG_HUAWEI_DP_DSM) += dp_dsm_debug.o
dp_extra_module-$(CONFIG_HUAWEI_DP_DSM) += dp_factory.o
dp_extra_module-$(CONFIG_HUAWEI_DP_DSM) += dp_hw_debug.o
dp_extra_module-$(CONFIG_HUAWEI_DP_LAHAINA) += lahaina/platform_interface.o
dp_extra_module-$(CONFIG_HUAWEI_DP_TARO) += taro/platform_interface.o
