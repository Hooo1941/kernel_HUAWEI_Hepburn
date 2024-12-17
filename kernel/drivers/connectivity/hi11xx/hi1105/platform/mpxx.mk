ifeq ($(CONFIG_HI11XX_DRIVER_PATH),)
HI110X_DRIVER_BUILTIN_PATH ?= drivers/connectivity/hi11xx
else
HI110X_DRIVER_BUILTIN_PATH ?= $(subst ",,$(CONFIG_HI11XX_DRIVER_PATH))
endif

DRIVER_FLODER = platform
export _PRE_PRODUCT_VERSION ?= 1105
export HI110X_BOARD_VERSION ?= default
export PLAT_DEFCONFIG_FILE  ?= plat_$(_PRE_PRODUCT_VERSION)_$(HI110X_BOARD_VERSION)_defconfig

HI110X_COMM_DEFCONFIG := hi$(_PRE_PRODUCT_VERSION)_comm_defconfig

CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT ?= no

ifeq (Hi3751, $(findstring Hi3751,$(TARGET_DEVICE)))
PLAT_DEFCONFIG_FILE  := plat_1105_$(HI110X_BOARD_VERSION)_defconfig
HI110X_COMM_DEFCONFIG := hi1105_comm_defconfig
else ifeq (v818, $(findstring v818,$(TARGET_DEVICE)))
PLAT_DEFCONFIG_FILE  := plat_1105_$(HI110X_BOARD_VERSION)_defconfig
HI110X_COMM_DEFCONFIG := hi1105_comm_defconfig
BUILDIN_FLAG := $(HI110X_DRIVER_BUILTIN_PATH)
else ifeq (v900, $(findstring v900,$(TARGET_DEVICE)))
PLAT_DEFCONFIG_FILE  := plat_1105_$(HI110X_BOARD_VERSION)_defconfig
HI110X_COMM_DEFCONFIG := hi1105_comm_defconfig
BUILDIN_FLAG := $(HI110X_DRIVER_BUILTIN_PATH)
endif

BUILDIN_FLAG ?= $(KERNELRELEASE)

ifeq ($(CONFIG_PANGU_WIFI_DEVICE_HI110X), y)
_PRE_PRODUCT_VERSION := 1103
PLAT_DEFCONFIG_FILE  := plat_1105_$(HI110X_BOARD_VERSION)_defconfig
HI110X_COMM_DEFCONFIG := hi1105_comm_defconfig
endif

ifneq ($(BUILDIN_FLAG),)
ifeq ($(HI1102_MAKE_FLAG),)
KERNEL_DIR := $(srctree)
HI110X_DRIVER_SRC_PATH=$(KERNEL_DIR)/$(HI110X_DRIVER_BUILTIN_PATH)/hi$(_PRE_PRODUCT_VERSION)
endif

DRIVER_PATH=$(HI110X_DRIVER_SRC_PATH)/$(DRIVER_FLODER)

include $(DRIVER_PATH)/$(HI110X_COMM_DEFCONFIG)
include $(DRIVER_PATH)/$(PLAT_DEFCONFIG_FILE)

$(warning defconfig: $(DRIVER_PATH)/$(PLAT_DEFCONFIG_FILE))

oal-objs := oal_bus_if.o oal_main.o oal_mem.o oal_hardware.o oal_schedule.o oal_kernel_file.o
oal-objs += oal_sdio_host.o
oal-objs += oal_hcc_host.o oal_hcc_bus.o oal_hcc_test.o
oal-objs += oal_softwdt.o oal_dft.o oal_workqueue.o oal_fsm.o ftrace.o

pcie-objs := pcie_dbg.o pcie_firmware.o pcie_host.o pcie_linux.o
pcie-objs := $(addprefix pcie/,$(pcie-objs))

pcie-edma-objs := pcie_edma_host.o pcie_edma_linux.o
pcie-edma-objs := $(addprefix pcie/edma/,$(pcie-edma-objs))

pcie-ete-objs := ete_linux.o ete_host.o ete_comm.o ete_debug.o
pcie-ete-objs := $(addprefix pcie/ete/,$(pcie-ete-objs))

pcie-chip-comm-objs := pcie_chip.o pcie_chiptest.o pcie_iatu.o pcie_pm.o
pcie-chip-comm-objs := $(addprefix pcie/chip/comm/,$(pcie-chip-comm-objs))

pcie-chip-mp13-objs := pcie_chip_mp13.o pcie_chiptest_mp13.o pcie_iatu_mp13.o pcie_pm_mp13.o
pcie-chip-mp13-objs := $(addprefix pcie/chip/mp13/,$(pcie-chip-mp13-objs))

pcie-chip-mp15-objs := pcie_chiptest_mp15.o pcie_iatu_mp15.o pcie_pm_mp15.o
pcie-chip-mp15-objs := $(addprefix pcie/chip/mp15/,$(pcie-chip-mp15-objs))

pcie-chip-mp16-objs := pcie_chip_mp16.o pcie_chiptest_mp16.o pcie_iatu_mp16.o pcie_pm_mp16.o
pcie-chip-mp16-objs := $(addprefix pcie/chip/mp16/,$(pcie-chip-mp16-objs))

pcie-chip-mp16c-objs := pcie_chip_mp16c.o pcie_chiptest_mp16c.o pcie_iatu_mp16c.o pcie_pm_mp16c.o
pcie-chip-mp16c-objs := $(addprefix pcie/chip/mp16c/,$(pcie-chip-mp16c-objs))

pcie-pcs-objs := hiphy/pcie_pcs_trace.o hiphy/pcie_pcs_trace_gpio_ssi.o hiphy/pcie_pcs_host.o hiphy/pcie_pcs.o
pcie-pcs-objs += hiphy/pcie_pcs_regs.o hiphy/pcie_pcs_deye_pattern.o hiphy/pcie_pcs_deye_pattern_host.o
pcie-pcs-objs := $(addprefix pcie/phy/,$(pcie-pcs-objs))

pcie-objs += $(pcie-edma-objs)
pcie-objs += $(pcie-ete-objs)
pcie-objs += $(pcie-chip-comm-objs)
pcie-objs += $(pcie-chip-mp13-objs)
pcie-objs += $(pcie-chip-mp15-objs)
pcie-objs += $(pcie-chip-mp16-objs)
pcie-objs += $(pcie-chip-mp16c-objs)
pcie-objs += $(pcie-pcs-objs)
oal-objs +=  $(pcie-objs)

oal-objs := $(addprefix oal/,$(oal-objs))

frw-objs := frw_event_deploy.o frw_event_main.o frw_event_sched.o frw_ipc_msgqueue.o frw_main.o frw_task.o frw_timer.o
frw-objs := $(addprefix frw/,$(frw-objs))

oam-objs := oam_main.o oam_event.o oam_log.o oam_statistics.o oam_config.o oam_rdr.o oam_dsm.o
oam-objs := $(addprefix oam/,$(oam-objs))

sdt-objs := sdt_drv.o
sdt-objs := $(addprefix sdt/,$(sdt-objs))

customize-objs := hisi_ini.o plat_parse_changid.o hisi_conn_nve.o plat_cust.o
customize-objs := $(addprefix ../common/customize/,$(customize-objs))

external-objs := lpcpu_feature.o
external-objs := $(addprefix oal/external/,$(external-objs))

chr_devs-objs := chr_devs.o
chr_devs-objs := $(addprefix ../common/chr_log/,$(chr_devs-objs))

main-objs := plat_main.o
main-objs := $(addprefix main/,$(main-objs))

pm-objs :=  plat_pm.o  plat_pm_gt.o plat_pm_wlan.o

pm-objs := $(addprefix pm/,$(pm-objs))

firmware-objs :=  plat_firmware.o plat_firmware_util.o plat_firmware_flash.o plat_firmware_download.o
ifeq ($(BFGX_UART_DOWNLOAD_SUPPORT),yes)
FEATURE_CFLAGS += -DBFGX_UART_DOWNLOAD_SUPPORT
$(warning *******BFGX_UART_DOWNLOAD_SUPPORT work*********)
firmware-objs += plat_firmware_uart.o
endif

firmware-objs := $(addprefix firmware/,$(firmware-objs))

factory-objs :=factory_mem.o factory_pcie.o factory_misc.o
factory-objs :=$(addprefix factory/,$(factory-objs))

driver-objs := plat_uart.o plat_gpio.o plat_cali.o plat_common_clk.o
driver-objs := $(addprefix driver/,$(driver-objs))

comm-excp-objs := bfgx_exception_rst.o wifi_exception_rst.o gt_exception_rst.o plat_exception_rst.o exception_common.o
comm-excp-objs := $(addprefix excp/,$(comm-excp-objs))
excp-objs := $(comm-excp-objs)

bfgx-objs := bfgx_data_parse.o bfgx_platform_msg_handle.o bfgx_dev.o bfgx_core.o bfgx_gnss.o bfgx_sle.o bfgx_user_ctrl.o  bfgx_excp_dbg.o\
             bfgx_cust.o bfgx_bt.o bfgx_fm.o bfgx_ir.o bfgx_debug.o
ifeq ($(CONFIG_PANGU_WIFI_DEVICE_HI110X), y)
bfgx-objs += bfgx_bluez.o
endif
ifeq ($(CONFIG_ARMPC_WIFI_DEVICE_HI110X), y)
bfgx-objs += bfgx_bluez.o
endif

ifeq ($(CONFIG_HI110X_GPS_REFCLK),y)
ifeq ($(CONFIG_HI110X_GPS_REFCLK_SRC_3),y)
bfgx-objs += gps_refclk_src_3.o
endif
endif
bfgx-objs := $(addprefix ../bfgx/,$(bfgx-objs))

ifeq ($(CONFIG_PANGU_WIFI_DEVICE_HI110X), y)
board-mp13-objs := board_mp13_kunpeng.o
else
board-mp13-objs := board_mp13.o
endif
board-mp13-objs += board_tv.o gpio_mpxx.o ssi_mp13.o bfgx_mp13.o mp13_exception_rst.o
board-mp15-objs := bfgx_mp15.o mp15_exception_rst.o
board-gf61-objs := board_gf61.o gpio_gf61.o ssi_gf61.o bfgx_gf61.o gf61_exception_rst.o
board-mp16c-objs := board_mp16c.o gpio_mp16c.o ssi_mp16c.o bfgx_mp16c.o mp16c_exception_rst.o
board-mp16-objs := gpio_mp16.o ssi_mp16.o bfgx_mp16.o mp16_exception_rst.o
board-common-objs := board_pm.o board.o ssi_common.o ssi_spmi.o ssi_dbg_cmd.o board_gpio.o board_dts.o board_bfgx.o
board-objs := $(addprefix board/,$(board-common-objs)) $(addprefix board/mp13/,$(board-mp13-objs)) $(addprefix board/mp15/,$(board-mp15-objs)) \
              $(addprefix board/mp16c/,$(board-mp16c-objs)) $(addprefix board/mp16/,$(board-mp16-objs)) $(addprefix board/gf61/,$(board-gf61-objs))

oneimage-objs := oneimage.o
oneimage-objs := $(addprefix ../common/oneimage/,$(oneimage-objs))

plat_$(_PRE_PRODUCT_VERSION)-objs += $(oal-objs)

plat_$(_PRE_PRODUCT_VERSION)-objs += $(frw-objs)

plat_$(_PRE_PRODUCT_VERSION)-objs += $(oam-objs)

plat_$(_PRE_PRODUCT_VERSION)-objs += $(sdt-objs)

plat_$(_PRE_PRODUCT_VERSION)-objs += $(pm-objs)

plat_$(_PRE_PRODUCT_VERSION)-objs += $(firmware-objs)

plat_$(_PRE_PRODUCT_VERSION)-objs += $(main-objs)

plat_$(_PRE_PRODUCT_VERSION)-objs += $(bfgx-objs)

plat_$(_PRE_PRODUCT_VERSION)-objs += $(factory-objs)

plat_$(_PRE_PRODUCT_VERSION)-objs += $(chr_devs-objs)

plat_$(_PRE_PRODUCT_VERSION)-objs += $(external-objs)

plat_$(_PRE_PRODUCT_VERSION)-objs += $(board-objs)

plat_$(_PRE_PRODUCT_VERSION)-objs += $(oneimage-objs)

plat_$(_PRE_PRODUCT_VERSION)-objs += $(customize-objs)

plat_$(_PRE_PRODUCT_VERSION)-objs += $(driver-objs)

plat_$(_PRE_PRODUCT_VERSION)-objs += $(excp-objs)

#plat ko
ifeq ($(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT), yes)
obj-m += plat_$(_PRE_PRODUCT_VERSION).o
else
obj-y += plat_$(_PRE_PRODUCT_VERSION).o
endif

HI110X_INCLUDES := -I$(DRIVER_PATH)/inc
HI110X_INCLUDES += -I$(DRIVER_PATH)/inc/frw
HI110X_INCLUDES += -I$(DRIVER_PATH)/inc/oal
HI110X_INCLUDES += -I$(DRIVER_PATH)/inc/oal/linux
HI110X_INCLUDES += -I$(DRIVER_PATH)/inc/oam
HI110X_INCLUDES += -I$(DRIVER_PATH)/inc/pm
HI110X_INCLUDES += -I$(DRIVER_PATH)/frw
HI110X_INCLUDES += -I$(DRIVER_PATH)/oal
HI110X_INCLUDES += -I$(DRIVER_PATH)/oal/pcie
HI110X_INCLUDES += -I$(DRIVER_PATH)/oal/pcie/phy/inc
HI110X_INCLUDES += -I$(DRIVER_PATH)/oal/pcie/chip/comm
HI110X_INCLUDES += -I$(DRIVER_PATH)/oal/external
HI110X_INCLUDES += -I$(DRIVER_PATH)/oam
HI110X_INCLUDES += -I$(DRIVER_PATH)/pm
HI110X_INCLUDES += -I$(DRIVER_PATH)/firmware
HI110X_INCLUDES += -I$(DRIVER_PATH)/../bfgx
HI110X_INCLUDES += -I$(DRIVER_PATH)/../gt
HI110X_INCLUDES += -I$(DRIVER_PATH)/sdt
HI110X_INCLUDES += -I$(DRIVER_PATH)/board
HI110X_INCLUDES += -I$(DRIVER_PATH)/board/mp13
HI110X_INCLUDES += -I$(DRIVER_PATH)/board/mp15
HI110X_INCLUDES += -I$(DRIVER_PATH)/board/mp16c
HI110X_INCLUDES += -I$(DRIVER_PATH)/board/mp16
HI110X_INCLUDES += -I$(DRIVER_PATH)/board/gf61
HI110X_INCLUDES += -I$(DRIVER_PATH)/driver
HI110X_INCLUDES += -I$(DRIVER_PATH)/excp
HI110X_INCLUDES += -I$(DRIVER_PATH)/../common/oneimage
HI110X_INCLUDES += -I$(DRIVER_PATH)/../common/customize
HI110X_INCLUDES += -I$(DRIVER_PATH)/../common/inc
HI110X_INCLUDES += -I$(DRIVER_PATH)/../common/chr_log
HI110X_INCLUDES += -I$(DRIVER_PATH)/factory
#This is not good
ifneq ($(KERNEL_DIR),)
HI110X_INCLUDES += -I$(KERNEL_DIR)
endif
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/wifi/inc/hd_common
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/wifi/inc/
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/wifi/hmac
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/wifi/customize

FEATURE_CFLAGS += -DPLATFORM_TYPE_FOR_HI110X=0
FEATURE_CFLAGS += -DPLATFORM_HI110X_k3v2oem1=1
FEATURE_CFLAGS += -DPLATFORM_HI110X_UEDGE=2

FEATURE_CFLAGS += -D_PRE_CONFIG_GPIO_TO_SSI_DEBUG

ifneq ($(TARGET_BUILD_VARIANT),user)
FEATURE_CFLAGS += -DPLATFORM_DEBUG_ENABLE
FEATURE_CFLAGS += -DPLATFORM_SSI_FULL_LOG
endif

FEATURE_CFLAGS += -DMPXX_OS_BUILD_VARIANT_ROOT=1
FEATURE_CFLAGS += -DMPXX_OS_BUILD_VARIANT_USER=2
ifneq ($(TARGET_BUILD_VARIANT),user)
FEATURE_CFLAGS += -DOS_MPXX_BUILD_VERSION=MPXX_OS_BUILD_VARIANT_ROOT
else
FEATURE_CFLAGS += -DOS_MPXX_BUILD_VERSION=MPXX_OS_BUILD_VARIANT_USER
endif

ifneq ($(HAVE_HISI_FEATURE_BT),false)
FEATURE_CFLAGS += -DHAVE_HISI_BT
endif

ifneq ($(CONFIG_PANGU_WIFI_DEVICE_HI110X), y)
ifneq (Hi3751, $(findstring Hi3751,$(TARGET_DEVICE)))
ifneq (v818, $(findstring v818,$(TARGET_DEVICE)))
ifneq ($(HAVE_HISI_FEATURE_FM),false)
FEATURE_CFLAGS += -DHAVE_HISI_FM
endif

ifneq ($(HAVE_HISI_FEATURE_GNSS),false)
FEATURE_CFLAGS += -DHAVE_HISI_GNSS
endif

ifneq ($(HAVE_HISI_FEATURE_IR),false)
FEATURE_CFLAGS += -DHAVE_HISI_IR
endif
endif
endif
endif

HI110X_VER_COMMIT_ID := $(shell cd $(HI110X_DRIVER_SRC_PATH);if test -d .git;then git rev-parse --verify --short HEAD 2>/dev/null;fi)
ifneq ($(HI110X_VER_COMMIT_ID),)
HI110X_VER_COMMIT_TIME := $(shell cd $(HI110X_DRIVER_SRC_PATH);git log -1 --pretty=format:%ci)

HI110X_VERSION="\"commitId:$(HI110X_VER_COMMIT_ID), commitTime:$(HI110X_VER_COMMIT_TIME)\""

#ifeq ($(HI110X_KERNEL_MODULES_BUILD_VERSION), y)
FEATURE_CFLAGS += -DMPXX_DRV_VERSION=$(HI110X_VERSION)
#endif
endif
$(warning HI110X_VER_COMMIT_ID: $(HI110X_VER_COMMIT_ID))
$(warning HI110X_VER_COMMIT_TIME: $(HI110X_VER_COMMIT_TIME))
$(warning HI110X_VERSION: $(HI110X_VERSION))
#$(warning INCLUDE: $(HI110X_INCLUDES))

EXTRA_CFLAGS = $(HI110X_INCLUDES)
EXTRA_CFLAGS += $(COMM_COPTS)
EXTRA_CFLAGS += $(COPTS) $(FEATURE_CFLAGS)
ifeq ($(CONN_NON_STRICT_CHECK),y)
$(warning hisi connectivity driver compile strict check disable)
else
EXTRA_CFLAGS += -Werror -Wuninitialized -Wempty-body
endif

ifeq ($(CONFIG_FTRACE_ENABLE),yes)
EXTRA_CFLAGS += -finstrument-functions -DFTRACE_ENABLE
endif

EXTRA_CFLAGS +=  -fno-pic
MODFLAGS = -fno-pic
else

#make modules
export HI1102_MAKE_FLAG=MODULES
export HI110X_DRIVER_SRC_PATH ?= $(shell pwd)/..

ifeq (Hi3751, $(findstring Hi3751,$(TARGET_DEVICE)))
# 大屏Hi3751平台编译使用的参数
MODULE_PARAM ?= ARCH=arm64 CROSS_COMPILE=aarch64-hisiv610-linux- CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT=yes
KERNEL_DIR ?= $(LINUX_SRC)
else ifeq (v818, $(findstring v818,$(TARGET_DEVICE)))
MODULE_PARAM ?= ARCH=arm64 CROSS_COMPILE=aarch64-hi100-linux- CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT=no
KERNEL_DIR ?= $(LINUX_SRC)
else ifeq (v900, $(findstring v900,$(TARGET_DEVICE)))
MODULE_PARAM ?= ARCH=arm64 CROSS_COMPILE=aarch64-hi100-linux- CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT=no
KERNEL_DIR ?= $(LINUX_SRC)
else
# 非Hi3751平台编译使用的参数
ANDROID_PRODUCT=hi6210sft
ifeq ($(ARCH),arm64)
ANDROID_PATH ?= /home/dengwenhua/zhouxinfeng/k3v5
MODULE_PARAM ?= ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-
else
ANDROID_PATH ?= /home/zhouxinfeng/1102/v8r2-checkin
MODULE_PARAM ?= ARCH=arm CROSS_COMPILE=arm-eabi-
endif

KERNEL_DIR ?= $(ANDROID_PATH)/out/target/product/$(ANDROID_PRODUCT)/obj/KERNEL_OBJ
CROSS_DIR ?= $(ANDROID_PATH)/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin:$(ANDROID_PATH)/prebuilts/gcc/linux-x86/arm/gcc-linaro-aarch64-linux-gnu-4.8/bin
PATH := $(CROSS_DIR):$(PATH)
CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT ?= yes
endif # ifeq Hi3751

default:
	$(MAKE) -C $(KERNEL_DIR) $(MODULE_PARAM)  M=$(HI110X_DRIVER_SRC_PATH)/$(DRIVER_FLODER) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(HI110X_DRIVER_SRC_PATH)/$(DRIVER_FLODER) clean
	-find ../bfgx/ -type f -name "*.o" -exec rm -f {} \;
	-find ../bfgx/ -type f -name "*.o.cmd" -exec rm -f {} \;
	-find ../bfgx/ -type f -name "*.o.d" -exec rm -f {} \;
	-find ../bfgx/ -type f -name "*.o.symversions" -exec rm -f {} \;
	-find ../common/ -type f -name "*.o" -exec rm -f {} \;
	-find ../common/ -type f -name "*.o.cmd" -exec rm -f {} \;
	-find ../common/ -type f -name "*.o.d" -exec rm -f {} \;
	-find ../common/ -type f -name "*.o.symversions" -exec rm -f {} \;

endif
