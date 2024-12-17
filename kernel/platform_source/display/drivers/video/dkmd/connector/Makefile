obj-$(CONFIG_DKMD_DPU_CONNECTOR) += dsc/
obj-$(CONFIG_DKMD_DPU_CONNECTOR) += spr/
obj-$(CONFIG_DKMD_DPU_CONNECTOR) += mipi/
obj-$(CONFIG_DKMD_DPU_CONNECTOR) += panel/
obj-$(CONFIG_DKMD_DPU_CONNECTOR) += offline/
obj-$(CONFIG_DKMD_DPU_DP) += dp_ctrl/
obj-$(CONFIG_DKMD_DPU_DP) += dp/
obj-$(CONFIG_DKMD_DPU_DP) += hdcptx/

obj-$(CONFIG_DKMD_DPU_HDMI_RX) += hdmirx/
obj-$(CONFIG_DKMD_DPU_DP_RX) += dprx/

obj-$(CONFIG_DKMD_DPU_HDMI_TX) += hdmitx/

ifneq ($(CONFIG_NORDA_DISPLAY_DRIVER_CPRO),y)
ifeq ($(CONFIG_DKMD_DPU_VERSION),$(filter 740 741,$(CONFIG_DKMD_DPU_VERSION)))
obj-$(CONFIG_DKMD_DPU_CONNECTOR) += serdes/
obj-$(CONFIG_DKMD_DPU_CONNECTOR) += serdes/common/
obj-$(CONFIG_DKMD_DPU_CONNECTOR) += serdes/mcu/
obj-$(CONFIG_DKMD_DPU_CONNECTOR) += serdes/upgrade/
endif
endif

obj-$(CONFIG_DKMD_DPU_CONNECTOR) += dkmd_connector.o
GCOV_PROFILE := $(CONFIG_GCOV_KERNEL)

dkmd_connector-objs := \
	dpu_connector.o

ifeq ($(CONFIG_DKMD_DPU_VERSION),$(filter 740 741,$(CONFIG_DKMD_DPU_VERSION)))
dkmd_connector-objs += \
	dpu_conn_mgr_v$(CONFIG_DKMD_DPU_VERSION).o
else
dkmd_connector-objs += \
	dpu_conn_mgr.o
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
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/offline

ifeq ($(CONFIG_DKMD_DPU_DP),y)
EXTRA_CFLAGS += \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/dp_ctrl \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/hdcptx
endif

ifeq ($(CONFIG_DKMD_DPU_HDMI_TX),y)
EXTRA_CFLAGS += \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/hdmitx/hdmitx_ctrl \
	-I$(srctree)/drivers/video/fbdev/dkmd/connector/hdmitx/hdmitx_ctrl/controller
endif

CFLAGS_MODULE += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow
# If you want to compile into modular drive, will need to decide
# according to the following configuration to compile
ifeq ($(CONFIG_DKMD_DPU_CONNECTOR),m)
ifeq ($(chip_type),)
EXTRA_CFLAGS += \
	-I$(srctree)/drivers/platform_drivers/ap/platform/$(TARGET_PRODUCT)
else
EXTRA_CFLAGS += \
	-I$(srctree)/drivers/platform_drivers/ap/platform/$(TARGET_PRODUCT)_$(chip_type)
endif

KERNEL_DIR = $(ANDROID_BUILD_TOP)/kernel/linux-5.10
KERNEL_OUT = $(ANDROID_PRODUCT_OUT)/obj/KERNEL_OBJ
PWD := $(shell pwd)
ARCH := arm64
CROSS_COMPILE := aarch64-linux-android-
CFLAGS_MODULE += -fno-pic -DCONFIG_DKMD_DPU_DP

default:
	$(MAKE) -C $(KERNEL_OUT) ARCH=$(ARCH) CFLAGS=$(EXTRA_CFLAGS) CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) modules
	$(MAKE) -C $(KERNEL_OUT) M=$(PWD) INSTALL_MOD_PATH=$(PWD)/signed INSTALL_MOD_STRIP=1 modules_install
endif

clean:
	$(MAKE) -C panel clean
	$(MAKE) -C offline clean
	$(MAKE) -C dsc clean
	$(MAKE) -C spr clean
	$(MAKE) -C mipi clean
ifeq ($(CONFIG_DKMD_DPU_DP),y)
	$(MAKE) -C dp clean
	$(MAKE) -C dp_ctrl clean
	$(MAKE) -C hdcptx clean
endif

ifeq ($(CONFIG_DKMD_DPU_HDMI_RX),y)
	$(MAKE) -C hdmirx clean
endif

ifeq ($(CONFIG_DKMD_DPU_DP_RX),y)
	$(MAKE) -C dprx clean
endif

	rm -rf *.o .*.cmd *.ko *.mod.c .tmp_versions *.symvers *.order *.symversions signed .*.d
	find . -name "*.o" | xargs rm -f
	find . -name ".*.cmd" | xargs rm -f
	find . -name ".*.d" | xargs rm -f
	find . -name "*.gcno" | xargs rm -f
	find . -name "*.mod" | xargs rm -f
