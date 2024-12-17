ifeq ($(TARGET_VERSION_MODE),factory)
EXTRA_CFLAGS += -DLCD_FACTORY_MODE
endif

obj-y	+= lcd_kit_local_hbm.o

ifeq ($(CONFIG_DPU_FB_V346),y)
EXTRA_CFLAGS += -DLCD_KIT_ALPHA_LHBM
endif

EXTRA_CFLAGS += -I$(srctree)/drivers/devkit/lcdkit/lcdkit3.0 \
		-I$(srctree)/drivers/devkit/lcdkit/lcdkit3.0/kernel/common/include \
		-I$(srctree)/drivers/devkit/lcdkit/lcdkit3.0/kernel/hisi/include \
		-I$(srctree)/drivers/devkit/lcdkit/lcdkit3.0/kernel/hisi/ \
		-I$(srctree)/drivers/devkit/lcdkit/lcdkit3.0/kernel/hisi/displayengine \
		-I$(srctree)/drivers/devkit/lcdkit/lcdkit3.0/kernel/hisi/displayengine/include \
		-I$(srctree)/drivers/video/fbdev/dkmd/dpu/azalea \
		-I$(srctree)/drivers/video/dkmd/dpu/azalea

