#
# Makefile for the vl53L0 drivers.
#

# Each configuration option enables a list of files.

# ccflags-y += -I$(srctree)/platform_source/camera/drivers/media/native/camera/laser/vl53Lx/vl53L0/inc
# ccflags-y += -I$(srctree)/platform_source/camera/drivers/media/native/camera/laser/vl53Lx/vl53L0
ccflags-y += -I$(srctree)/platform_source/camera/drivers/media/native/camera -DSTM_TEST
ccflags-y += -I$(srctree)/platform_source/camera/drivers/media/native/camera/laser/vl53Lx/vl53L1/inc
ccflags-y += -I$(srctree)/platform_source/camera/drivers/media/native/camera/laser/vl53Lx/vl53L1/ipp
ccflags-y += -I$(srctree)/platform_source/camera/drivers/media/native/camera/laser/vl53Lx/vl53L1
ccflags-y += -I$(srctree)/platform_source/camera/drivers/media/native/camera/laser/vl53Lx
ccflags-y += -I$(srctree)/platform_source/camera/drivers/media/native/camera/laser

ccflags-y += -I$(srctree)/platform_source/camera/drivers/media/native/camera -DSTM_TEST
ccflags-y += -I$(srctree)/platform_source/camera/drivers/media/native/camera/laser/vl53Lx/vl53L3/inc
ccflags-y += -I$(srctree)/platform_source/camera/drivers/media/native/camera/laser/vl53Lx/vl53L3/protected/inc
ccflags-y += -I$(srctree)/platform_source/camera/drivers/media/native/camera/laser/vl53Lx/vl53L3


obj-$(CONFIG_KERNEL_CAMERA_VL53L0_970)               += stmvl53l1.o
obj-$(CONFIG_KERNEL_CAMERA_VL53L3_970)               += stmvl53l3.o

# module
stmvl53l1-objs := laser_module.o
stmvl53l3-objs := vl53l3_module.o

# vl53L1
ifdef OLD_NETLINK_API
ccflags-y	+= -DOLD_NETLINK_API
endif

ifdef VL53L1_LOG_ENABLE
ccflags-y	+= -DVL53L1_LOG_ENABLE
endif

ifdef VL53L3_LOG_ENABLE
ccflags-y	+= -DVL53L3_LOG_ENABLE
endif

stmvl53l1-objs		+= vl53L1/stmvl53l1_module.o vl53L1/stmvl53l1_module-i2c.o
stmvl53l1-objs		+= vl53L1/stmvl53l1_i2c.o vl53L1/stmvl53l1_ipp_nl.o vl53L1/stmvl53l1_log.o
stmvl53l1-objs		+= vl53L1/src/vl53l1_api.o vl53L1/src/vl53l1_api_core.o
stmvl53l1-objs		+= vl53L1/src/vl53l1_api_strings.o vl53L1/src/vl53l1_error_strings.o
stmvl53l1-objs		+= vl53L1/src/vl53l1_core.o vl53L1/src/vl53l1_register_funcs.o
stmvl53l1-objs		+= vl53L1/src/vl53l1_api_preset_modes.o
stmvl53l1-objs		+= vl53L1/src/vl53l1_api_calibration.o
stmvl53l1-objs		+= vl53L1/src/vl53l1_fpga_core.o vl53L1/src/vl53l1_silicon_core.o
stmvl53l1-objs		+= vl53L1/src/vl53l1_zone_presets.o vl53L1/src/vl53l1_nvm.o
stmvl53l1-objs		+= vl53L1/src/vl53l1_api_debug.o vl53L1/src/vl53l1_core_support.o
stmvl53l1-objs		+= vl53L1/src/vl53l1_wait.o vl53L1/ipp/ipp_linux.o
stmvl53l1-objs		+= vl53L1/src/vl53l1_nvm_debug.o

stmvl53l3-objs		+= vl53L3/stmvl53l3_module.o vl53L3/stmvl53l3_module-i2c.o
stmvl53l3-objs		+= vl53L3/stmvl53l3_i2c.o vl53L3/stmvl53l3_ipp_nl.o vl53L3/stmvl53l3_log.o
stmvl53l3-objs		+= vl53L3/src/vl53l3_api.o vl53L3/src/vl53l3_api_core.o
stmvl53l3-objs		+= vl53L3/src/vl53l3_api_strings.o vl53L3/src/vl53l3_error_strings.o
stmvl53l3-objs		+= vl53L3/src/vl53l3_core.o vl53L3/src/vl53l3_register_funcs.o
stmvl53l3-objs		+= vl53L3/src/vl53l3_api_preset_modes.o
stmvl53l3-objs		+= vl53L3/src/vl53l3_api_calibration.o
stmvl53l3-objs		+= vl53L3/src/vl53l3_silicon_core.o
stmvl53l3-objs		+= vl53L3/src/vl53l3_zone_presets.o vl53L3/src/vl53l3_nvm.o
stmvl53l3-objs		+= vl53L3/src/vl53l3_api_debug.o vl53L3/src/vl53l3_core_support.o
stmvl53l3-objs		+= vl53L3/src/vl53l3_wait.o
stmvl53l3-objs		+= vl53L3/src/vl53l3_nvm_debug.o

stmvl53l3-objs		+= vl53L3/vl53l3_platform_ipp.o
stmvl53l3-objs		+= vl53L3/protected/src/vl53l3_dmax.o
stmvl53l3-objs		+= vl53L3/protected/src/vl53l3_hist_algos_gen3.o
stmvl53l3-objs		+= vl53L3/protected/src/vl53l3_hist_algos_gen4.o
stmvl53l3-objs		+= vl53L3/protected/src/vl53l3_hist_core.o
stmvl53l3-objs		+= vl53L3/protected/src/vl53l3_hist_funcs.o
stmvl53l3-objs		+= vl53L3/protected/src/vl53l3_sigma_estimate.o
stmvl53l3-objs		+= vl53L3/protected/src/vl53l3_xtalk.o
