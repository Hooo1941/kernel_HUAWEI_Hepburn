obj-$(CONFIG_DKMD_DPU_HDMI_RX) += libhdmirx.o

GCOV_PROFILE := $(CONFIG_GCOV_KERNEL)

#obj-y += libhdmirx.o

libhdmirx-objs := hdmirx_cmd.o \
					hdmirx_common.o \
					hdmirx_init.o \
					audio/hdmirx_audio.o \
					control/hdmirx_ctrl.o \
					control/hdmirx_ctrl_irq.o \
					hdcp/hdmirx_hdcp.o \
					hdcp/hdmirx_hdcp_tee.o \
					link/hdmirx_link.o \
					link/hdmirx_link_cec.o \
					link/hdmirx_link_ddc.o \
					link/hdmirx_link_frl.o \
					link/hdmirx_link_hpd.o \
					link/hdmirx_link_tmds.o \
					link/hdmirx_link_edid.o \
					packet/hdmirx_packet.o \
					phy/hdmirx_phy.o \
					video/hdmirx_video.o \
					video/hdmirx_video_dsc.o \
					video/hdmirx_video_hdr.o \
					hdmirx_log.o


#ifeq ($(CONFIG_HISI_DISP_TEST_ENABLE),y)
libhdmirx-objs += \
	test/hdmirx_test.o
#endif

EXTRA_CFLAGS += \
	-I$(srctree)/include/ \
	-I$(srctree)/arch/arm64/include/ \
	-I$(srctree)/include/platform_include/display/dkmd/ \
	-I$(srctree)/drivers/video/fbdev/dkmd/cmdlist \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/chrdev \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/peri \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm/isr \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/mipi \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/dsc \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/spr \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/offline \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/dp_ctrl \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/hdmirx \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/hdmirx/audio \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/hdmirx/control \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/hdmirx/hdcp \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/hdmirx/link \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/hdmirx/packet \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/hdmirx/phy \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/hdmirx/test \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/hdmirx/video



ifeq ($(CONFIG_DKMD_DPU_VERSION),$(filter 740 741,$(CONFIG_DKMD_DPU_VERSION)))
EXTRA_CFLAGS += \
	-I$(srctree)/drivers/platform_drivers/ap/platform/$(TARGET_PRODUCT)/dpu_v$(CONFIG_DKMD_DPU_VERSION)
endif

# If you want to compile into modular drive, will need to decide
# according to the following configuration to compile
ifeq ($(CONFIG_DKMD_DPU_CONNECTOR),m)
EXTRA_CFLAGS += \
	-I$(srctree)/drivers/platform_drivers/ap/platform/$(TARGET_PRODUCT)_es

KERNEL_DIR = $(ANDROID_BUILD_TOP)/kernel/linux-5.10
KERNEL_OUT = $(ANDROID_PRODUCT_OUT)/obj/KERNEL_OBJ
PWD := $(shell pwd)
ARCH := arm64
CROSS_COMPILE := aarch64-linux-android-
CFLAGS_MODULE := -fno-pic

default:
	$(MAKE) -C $(KERNEL_OUT) ARCH=$(ARCH) CFLAGS=$(EXTRA_CFLAGS) CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) modules
	$(MAKE) -C $(KERNEL_OUT) M=$(PWD) INSTALL_MOD_PATH=$(PWD)/signed INSTALL_MOD_STRIP=1 modules_install
endif

clean:
	rm -rf *.o .*.cmd *.ko *.mod.c .tmp_versions *.symvers *.order *.symversions signed .*.d
