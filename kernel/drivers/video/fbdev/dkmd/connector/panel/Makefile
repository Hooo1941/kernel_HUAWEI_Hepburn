obj-$(CONFIG_DKMD_DPU_PANEL) += dpu_panel.o

GCOV_PROFILE := $(CONFIG_GCOV_KERNEL)

dpu_panel-objs := \
	panel_drv.o \
	panel_dev.o \
	panel_mgr.o \
	panel_utils.o \
	panel_lg_td4322.o \
	panel_edo_rm69091.o \
	panel_boe_hx5293.o \
	panel_boe_nt37290.o \
	panel_visionox_6p39_ltpo.o \
	panel_boe_6p39_ltpo.o \
	panel_visionox310.o \
	panel_boe_7p847.o \
	panel_cmi_nt35521.o \
	panel_boe_nt37701a.o

ifeq ($(CONFIG_LCD_KIT_DRIVER),y)
dpu_panel-objs += \
	panel_lcdkit.o
endif

ifeq ($(CONFIG_DKMD_DPU_VERSION),$(filter $(CONFIG_DKMD_DPU_VERSION), 740 741))
dpu_panel-objs += \
	panel_cdc.o \
	v740/panel_boe_nt37700p.o \
	v740/panel_boe_nt37800a.o \
	v740/panel_boe_nt37701_brq.o \
	v740/panel_sharp_nt36870.o \
	v740/panel_tcl_hx83121.o

else
dpu_panel-objs += \
	panel_boe_nt37700p.o \
	panel_boe_nt37800a.o \
	panel_boe_nt37701_brq.o \
	panel_sharp_nt36870.o \
	panel_tcl_hx83121.o
endif

ifneq ($(CONFIG_NORDA_DISPLAY_DRIVER_CPRO),y)
ifeq ($(CONFIG_DKMD_DPU_VERSION),$(filter $(CONFIG_DKMD_DPU_VERSION), 740 741))
dpu_panel-objs += \
	v740/dsi_panel.o
endif
endif

EXTRA_CFLAGS += \
	-I$(srctree)/include/ \
	-I$(srctree)/arch/arm64/include/ \
	-I$(srctree)/include/platform_include/display/dkmd/ \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm \
	-I$(srctree)/drivers/video/fbdev/dkmd/backlight \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/peri \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/isr \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/mipi \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/dsc \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/spr \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/offline \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/dp_ctrl \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/panel \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/serdes/common \

CFLAGS_MODULE += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow
DS_SOC_CHIP_NAME := $(strip $(subst ", , $(CONFIG_VENDOR_CHIP_PREFIX)))
ifeq ($(CONFIG_DKMD_DPU_PANEL),m)
EXTRA_CFLAGS += \
	-I$(srctree)/drivers/platform_drivers/ap/platform/$(DS_SOC_CHIP_NAME)_es
endif

clean:
	rm -rf *.o .*.cmd *.ko *.mod.c .tmp_versions *.symvers *.order *.symversions signed .*.d
	find . -name "*.o" | xargs rm -f
	find . -name ".*.cmd" | xargs rm -f
	find . -name ".*.d" | xargs rm -f
	find . -name "*.gcno" | xargs rm -f
