ifeq ($(CONFIG_HI11XX_DRIVER_PATH),)
HI110X_DRIVER_BUILTIN_PATH ?= drivers/connectivity/hi11xx
else
HI110X_DRIVER_BUILTIN_PATH ?= $(subst ",,$(CONFIG_HI11XX_DRIVER_PATH))
endif

$(warning HI110X_DRIVER_BUILTIN_PATH $(HI110X_DRIVER_BUILTIN_PATH))
DRIVER_FLODER = wifi

ifeq ($(subst ",,$(CONFIG_CONNECTIVITY_HI116X)),)
export _PRE_PRODUCT_VERSION ?= 1105
else
export _PRE_PRODUCT_VERSION ?= gf61
endif
$(warning wifi _PRE_PRODUCT_VERSION $(_PRE_PRODUCT_VERSION))

export HI110X_BOARD_VERSION ?= default
export PLAT_DEFCONFIG_FILE  ?= wifi_$(_PRE_PRODUCT_VERSION)_$(HI110X_BOARD_VERSION)_defconfig

HI110X_COMM_DEFCONFIG := hi$(_PRE_PRODUCT_VERSION)_comm_defconfig

CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT ?= no

ifeq (Hi3751, $(findstring Hi3751,$(TARGET_DEVICE)))
PLAT_DEFCONFIG_FILE  := wifi_1105_$(HI110X_BOARD_VERSION)_defconfig
HI110X_COMM_DEFCONFIG := hi1105_comm_defconfig
else ifeq (v818, $(findstring v818,$(TARGET_DEVICE)))
PLAT_DEFCONFIG_FILE  := wifi_1105_$(HI110X_BOARD_VERSION)_defconfig
HI110X_COMM_DEFCONFIG := hi1105_comm_defconfig
BUILDIN_FLAG := $(HI110X_DRIVER_BUILTIN_PATH)
else ifeq (v900, $(findstring v900,$(TARGET_PRODUCT)))
PLAT_DEFCONFIG_FILE  := wifi_1105_$(HI110X_BOARD_VERSION)_defconfig
HI110X_COMM_DEFCONFIG := hi1105_comm_defconfig
BUILDIN_FLAG := $(HI110X_DRIVER_BUILTIN_PATH)
endif
ifeq ($(_PRE_PRODUCT_VERSION), gf61)
HI110X_COMM_DEFCONFIG := gf61_comm_defconfig
endif

BUILDIN_FLAG ?= $(KERNELRELEASE)

ifeq ($(CONFIG_PANGU_WIFI_DEVICE_HI110X), y)
_PRE_PRODUCT_VERSION := 1103
PLAT_DEFCONFIG_FILE  := wifi_1105_$(HI110X_BOARD_VERSION)_defconfig
HI110X_COMM_DEFCONFIG := hi1105_comm_defconfig
endif

ifneq ($(BUILDIN_FLAG),)
ifeq ($(HI1102_MAKE_FLAG),)
KERNEL_DIR := $(srctree)
# 在host上，共代码后，路径固定位hi1105，不再跟随芯片类型修改
HI110X_DRIVER_SRC_PATH:=$(KERNEL_DIR)/$(HI110X_DRIVER_BUILTIN_PATH)/hi1105
endif

DRIVER_PATH=$(HI110X_DRIVER_SRC_PATH)/$(DRIVER_FLODER)

include $(HI110X_DRIVER_SRC_PATH)/platform/$(HI110X_COMM_DEFCONFIG)
include $(DRIVER_PATH)/$(PLAT_DEFCONFIG_FILE)

$(warning defconfig: $(DRIVER_PATH)/$(PLAT_DEFCONFIG_FILE))

hmac-objs := hmac_main.o hmac_thread.o hmac_hcc_adapt.o hmac_auto_adjust_freq.o hmac_auto_cpufreq.o hmac_low_latency.o
hmac-objs += hmac_lp_miracast.o
hmac-objs := $(addprefix hmac/,$(hmac-objs))

connect-objs := hmac_dbac.o hmac_chan_mgmt.o hmac_fsm.o hmac_mgmt_bss_comm.o hmac_mgmt_classifier.o
connect-objs += hmac_scan.o hmac_scan_bss.o hmac_mgmt_join.o hmac_mgmt_ap.o hmac_mgmt_sta_up.o hmac_mgmt_sta.o hmac_sme_sta.o
connect-objs += hmac_blacklist.o hmac_arp_probe.o hmac_ht_self_cure.o hmac_wifi6_self_cure.o
connect-objs := $(addprefix hmac/connect/,$(connect-objs))
hmac-objs += $(connect-objs)

tx-objs := hmac_edca_opt.o hmac_frag.o hmac_host_al_tx.o hmac_host_tx_data.o hmac_multi_netbuf_amsdu.o hmac_pps_fsm.o hmac_soft_retry.o hmac_tcp_ack_buf.o hmac_tcp_opt.o
tx-objs += hmac_tid_ring_switch.o hmac_tid_sche.o hmac_tid_update.o hmac_traffic_classify.o hmac_tx_amsdu.o hmac_tx_complete.o hmac_tx_data.o hmac_tx_msdu_dscr.o
tx-objs += hmac_tx_ring_alloc.o hmac_tx_sche_switch_fsm.o hmac_tx_switch.o hmac_tx_switch_fsm.o
tx-objs := $(addprefix hmac/tx/,$(tx-objs))
hmac-objs += $(tx-objs)

mintp-objs := hmac_mintp.o hmac_mintp_test.o hmac_mintp_log.o
mintp-objs := $(addprefix hmac/tx/mintp/,$(mintp-objs))
hmac-objs += $(mintp-objs)

rx-objs := hmac_rx_thread.o hmac_napi.o
rx-objs += hmac_blockack.o hmac_host_rx.o hmac_rate_calc.o hmac_rx_data.o hmac_rx_data_misc.o hmac_rx_filter.o hmac_sniffer.o hmac_rx_complete.o
rx-objs := $(addprefix hmac/rx/,$(rx-objs))
hmac-objs += $(rx-objs)

measure-objs := hmac_chan_det.o hmac_config_ftm.o hmac_csi.o
measure-objs := $(addprefix hmac/measure/,$(measure-objs))
hmac-objs += $(measure-objs)

measure-ftm-objs := hmac_ftm.o mac_ftm.o hmac_ftm_frame.o
measure-ftm-objs := $(addprefix hmac/measure/ftm/,$(measure-ftm-objs))
hmac-objs += $(measure-ftm-objs)

chba-objs := hmac_chba_mgmt.o hmac_chba_common_function.o hmac_chba_sync.o hmac_chba_ps.o hmac_chba_chan_switch.o hmac_chba_coex.o hmac_chba_user.o hmac_chba_function.o hmac_chba_frame.o hmac_chba_interface.o hmac_chba_fsm.o hmac_chba_channel_sequence.o
chba-objs := $(addprefix hmac/chba/,$(chba-objs))
hmac-objs += $(chba-objs)

vsp-objs := hmac_vsp.o hmac_vsp_sink.o hmac_vsp_source.o hmac_vsp_buf.o hmac_vsp_flow_ctrl.o hmac_vsp_main.o hmac_vsp_test.o hmac_vsp_log.o
vsp-objs := $(addprefix hmac/vsp/,$(vsp-objs))
hmac-objs += $(vsp-objs)

cmd-objs := hmac_config.o hmac_package_params.o hmac_config_cmd.o hmac_config_hipriv.o hmac_config_d2h.o hmac_config_tlv.o hmac_config_atcmdsrv.o hmac_config_chba.o
cmd-objs := $(addprefix hmac/cmd/,$(cmd-objs))
hmac-objs += $(cmd-objs)

frame-objs := hmac_config_frame.o hmac_encap_frame_ap.o hmac_encap_frame_sta.o hmac_encap_frame.o
frame-objs += mac_data.o mac_frame.o mac_ie.o
frame-objs := $(addprefix hmac/frame/,$(frame-objs))
hmac-objs += $(frame-objs)

protocol-objs := hmac_11ax.o hmac_11k.o hmac_11v.o hmac_11r.o hmac_dfs.o hmac_mbo.o hmac_opmode.o hmac_p2p.o hmac_protection.o
protocol-objs += hmac_vowifi.o hmac_wmmac.o mac_regdomain.o mac_regdomain_bandwidth.o
protocol-objs := $(addprefix hmac/protocol/,$(protocol-objs))
hmac-objs += $(protocol-objs)

resource-objs := hmac_config_mib.o  hmac_config_vap.o hmac_config_user.o hmac_device.o hmac_resource.o hmac_resource_restriction.o hmac_user.o hmac_vap.o
resource-objs += mac_board.o mac_device.o mac_user.o mac_vap.o mac_vap_init.o mac_mib.o mac_resource.o
resource-objs := $(addprefix hmac/resource/,$(resource-objs))
hmac-objs += $(resource-objs)

dfx-objs := hmac_dfx.o hmac_config_query.o hmac_host_performance.o hmac_rr_performance.o hmac_ota_report.o hmac_reset.o hmac_stat.o
dfx-objs += hmac_wifi_delay.o hmac_sysfs_stat.o
dfx-objs := $(addprefix hmac/dfx/,$(dfx-objs))
hmac-objs += $(dfx-objs)

pm-objs := hmac_pm.o hmac_smps.o hmac_twt.o hmac_uapsd.o hmac_psm_ap.o
pm-objs += hmac_dyn_pcie.o hmac_arp_offload.o hmac_degradation.o
pm-objs := $(addprefix hmac/pm/,$(pm-objs))
hmac-objs += $(pm-objs)

ifeq ($(fe_new), y)
cali-objs := hmac_cali_arr_init.o hmac_cali_mgmt.o hmac_cali_mgmt_debug.o
else
cali-objs := hmac_cali_arr_init.o hmac_cali_mgmt.o hmac_cali_mgmt_debug.o
endif
cali-objs := $(addprefix hmac/cali/,$(cali-objs))
hmac-objs += $(cali-objs)

roaming-objs := hmac_roam_alg.o hmac_roam_connect.o hmac_roam_main.o hmac_roam_scan.o
roaming-objs := $(addprefix hmac/roaming/,$(roaming-objs))
hmac-objs += $(roaming-objs)

coex-objs := hmac_btcoex.o hmac_gnss_rsmc.o hmac_nrcoex.o
coex-objs := $(addprefix hmac/coex/,$(coex-objs))
hmac-objs += $(coex-objs)

crypto-objs := hmac_crypto_tkip.o hmac_sae.o hmac_wpi_mic.o hmac_wpi.o hmac_sms4.o hmac_wapi.o hmac_11i.o hmac_11w.o hmac_soft_encrypt.o
crypto-objs := $(addprefix hmac/crypto/,$(crypto-objs))
hmac-objs += $(crypto-objs)

enhanced-objs := hmac_hid2d.o hmac_hiex.o hmac_mcast_ampdu.o mac_hiex.o
enhanced-objs := $(addprefix hmac/enhanced/,$(enhanced-objs))
hmac-objs += $(enhanced-objs)

ifeq ($(PWL_ENABLE), y)
gcm-objs := hmac_gcm_frame.o hmac_gcm.o
gcm-objs := $(addprefix ../../drivers_pwl/wifi/hmac/05_gcm/,$(gcm-objs))
hmac-objs += $(gcm-objs)

pwl-objs := hmac_pwl.o hmac_roam_pwl.o
pwl-objs := $(addprefix ../../drivers_pwl/wifi/hmac/05_pwl/,$(pwl-objs))
hmac-objs += $(pwl-objs)
endif

wal-objs := wal_config.o wal_config_debug.o wal_linux_bridge.o  wal_main.o wal_linux_scan.o wal_linux_rx_rsp.o wal_linux_event.o wal_regdb.o  wal_linux_flowctl.o  wal_linux_ioctl.o wal_hipriv_chba.o wal_linux_vendor.o wal_linux_netdev_ops.o wal_linux_ioctl_debug.o wal_linux_atcmdsrv.o wal_config_base.o
wal-objs += wal_linux_cfg80211_band.o wal_linux_cfg80211.o wal_dfx.o wal_linux_cfgvendor.o wal_linux_cfgvendor_cmd.o wal_linux_cfgvendor_policy.o wal_cfg_ioctl.o wal_linux_customize.o wal_config_alg_cfg.o wal_cfg_release_ioctl.o wal_cfg_stat_ioctl.o wal_cfg_pm_ioctl.o wal_cfg_trx_ioctl.o wal_cfg_featuer_ioctl.o
wal-objs += wal_linux_ioctl_mcm.o wal_linux_ioctl_cali.o wal_cfg_vap_ioctl.o wal_cfg_dscr_ioctl.o wal_phy_debug_ioctl.o wal_cfg_ftm_ioctl.o wal_cfg_user_ioctl.o wal_linux_misc_ioctl.o
wal-objs += wal_linux_connect_cfg80211.o wal_linux_wep_key_cfg80211.o wal_linux_protocol_cfg80211.o wal_linux_vap_cfg80211.o wal_linux_chba_cfg80211.o wal_cfg80211.o wal_net.o

wal-objs := $(addprefix wal/,$(wal-objs))

ifeq ($(FUZZ_TEST), y)
wal-objs += ../../llt/wifi/secodefuzz/wal/wal_linux_cfg80211_fuzz.o ../../llt/wifi/secodefuzz/wal/wal_linux_wal_vendor_fuzz.o ../../llt/wifi/secodefuzz/wal/wal_linux_wal_iw_hanlder_fuzz.o
endif

oam_adapter-objs := oam_event_wifi.o oam_stat_wifi.o oam_log_wifi.o
oam_adapter-objs := $(addprefix oam_adapter/,$(oam_adapter-objs))

main-objs := main.o
main-objs := $(addprefix main/,$(main-objs))

wifi_customize-objs := hisi_customize_wifi.o
wifi_customize-objs := $(addprefix customize/,$(wifi_customize-objs))

#hal dir
hal-objs :=  host_hal_device.o host_hal_ring.o host_hal_main.o host_hal_dscr.o host_hal_access_mgmt.o
hal-objs := $(addprefix hal/,$(hal-objs))

hal-mp16-objs := host_dscr_mp16.o host_mac_mp16.o host_irq_mp16.o host_main_mp16.o host_csi_mp16.o host_ftm_mp16.o host_tx_mp16.o
hal-mp16-objs += host_sniffer_mp16.o
hal-mp16-objs := $(addprefix hal/product/mp16/,$(hal-mp16-objs))

hal-objs += $(hal-mp16-objs)

hal-mp16c-objs := host_dscr_mp16c.o host_mac_mp16c.o host_irq_mp16c.o host_main_mp16c.o host_csi_mp16c.o host_ftm_mp16c.o
hal-mp16c-objs += host_sniffer_mp16c.o host_tx_mp16c.o
hal-mp16c-objs := $(addprefix hal/product/mp16c_gf61/,$(hal-mp16c-objs))

hal-objs += $(hal-mp16c-objs)

ifeq ($(fe_new), y)
#fe dir
fe-objs := rf_customize/fe_customize_ini_parse.o
fe-objs += rf_customize/fe_customize_main.o
fe-objs += fe_extern_if/fe_extern_if.o
fe-objs += rf_customize/fe_customize_static_alloc_mem.o
fe-objs := $(addprefix fe/,$(fe-objs))
endif
#chip dir
chip-objs := wlan_chip.o hisi_customize_wifi_mp16.o hisi_conn_nve_interface.o hisi_customize_wifi_data.o hisi_customize_wifi_pow.o hisi_conn_nve_interface_gf61.o hisi_customize_wifi_gf61.o
chip-objs := $(addprefix chip/,$(chip-objs))

chip-mp13-objs := wlan_chip_mp13.o hisi_customize_wifi_mp13.o hisi_customize_config_dts_mp13.o hisi_customize_config_priv_mp13.o hisi_customize_dyn_pow_mp13.o
chip-mp13-objs += hisi_customize_config_cmd_mp13.o hisi_customize_nvram_config_mp13.o cali_array_init_mp13.o
chip-mp13-objs := $(addprefix chip/mp13/,$(chip-mp13-objs))
chip-objs += $(chip-mp13-objs)

chip-mp16-objs := wlan_chip_mp16.o
chip-mp16-objs := $(addprefix chip/mp16/,$(chip-mp16-objs))
chip-objs += $(chip-mp16-objs)

chip-mp16c-objs := wlan_chip_mp16c.o
chip-mp16c-objs := $(addprefix chip/mp16c/,$(chip-mp16c-objs))
chip-objs += $(chip-mp16c-objs)

chip-gf61-objs := wlan_chip_gf61.o
chip-gf61-objs := $(addprefix chip/gf61/,$(chip-gf61-objs))
chip-objs += $(chip-gf61-objs)

#wifi.ko

wifi_$(_PRE_PRODUCT_VERSION)-objs := $(wal-objs)

wifi_$(_PRE_PRODUCT_VERSION)-objs += $(main-objs)

wifi_$(_PRE_PRODUCT_VERSION)-objs += $(hmac-objs)

wifi_$(_PRE_PRODUCT_VERSION)-objs += $(hal-objs)

wifi_$(_PRE_PRODUCT_VERSION)-objs += $(chip-objs)

wifi_$(_PRE_PRODUCT_VERSION)-objs += $(wifi_customize-objs)

ifeq ($(fe_new), y)
wifi_$(_PRE_PRODUCT_VERSION)-objs += $(fe-objs)
endif

wifi_$(_PRE_PRODUCT_VERSION)-objs += $(oam_adapter-objs)
#main.ko
ifeq ($(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT), yes)
KBUILD_EXTRA_SYMBOLS += $(HI110X_DRIVER_SRC_PATH)/platform/Module.symvers
obj-m += wifi_$(_PRE_PRODUCT_VERSION).o
else
obj-y += wifi_$(_PRE_PRODUCT_VERSION).o
endif

HI110X_INCLUDES := -I$(DRIVER_PATH)/inc/hd_common
HI110X_INCLUDES += -I$(DRIVER_PATH)/inc
HI110X_INCLUDES += -I$(DRIVER_PATH)/hmac
HI110X_INCLUDES += -I$(DRIVER_PATH)/hmac/tx
HI110X_INCLUDES += -I$(DRIVER_PATH)/hmac/tx/mintp
HI110X_INCLUDES += -I$(DRIVER_PATH)/hmac/rx
HI110X_INCLUDES += -I$(DRIVER_PATH)/hmac/measure
HI110X_INCLUDES += -I$(DRIVER_PATH)/hmac/measure/ftm
HI110X_INCLUDES += -I$(DRIVER_PATH)/hmac/vsp
HI110X_INCLUDES += -I$(DRIVER_PATH)/hmac/chba/include
HI110X_INCLUDES += -I$(DRIVER_PATH)/hmac/resource
HI110X_INCLUDES += -I$(DRIVER_PATH)/hmac/frame
HI110X_INCLUDES += -I$(DRIVER_PATH)/hmac/connect
HI110X_INCLUDES += -I$(DRIVER_PATH)/hmac/cmd
HI110X_INCLUDES += -I$(DRIVER_PATH)/hmac/protocol
HI110X_INCLUDES += -I$(DRIVER_PATH)/hmac/dfx
HI110X_INCLUDES += -I$(DRIVER_PATH)/hmac/pm
HI110X_INCLUDES += -I$(DRIVER_PATH)/hmac/cali
HI110X_INCLUDES += -I$(DRIVER_PATH)/hmac/roaming
HI110X_INCLUDES += -I$(DRIVER_PATH)/hmac/coex
HI110X_INCLUDES += -I$(DRIVER_PATH)/hmac/crypto
HI110X_INCLUDES += -I$(DRIVER_PATH)/hmac/enhanced
HI110X_INCLUDES += -I$(DRIVER_PATH)/wal
HI110X_INCLUDES += -I$(DRIVER_PATH)/hal
HI110X_INCLUDES += -I$(DRIVER_PATH)/customize
HI110X_INCLUDES += -I$(DRIVER_PATH)/oam_adapter
ifeq ($(fe_new), y)
#fe dir
HI110X_INCLUDES += -I$(DRIVER_PATH)/fe/fe_depend_if/hi11xx/wifi
HI110X_INCLUDES += -I$(DRIVER_PATH)/fe/fe_extern_if
HI110X_INCLUDES += -I$(DRIVER_PATH)/fe/fe_pub_def
HI110X_INCLUDES += -I$(DRIVER_PATH)/fe/fe_spec/hi11xx/wifi
HI110X_INCLUDES += -I$(DRIVER_PATH)/fe/rf_customize
endif
#This is not good
ifneq ($(KERNEL_DIR),)
HI110X_INCLUDES += -I$(KERNEL_DIR)
endif
#HI110X_INCLUDES += -I$(SDK_INC_PATH)

HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/platform/inc
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/platform/board
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/platform/board/mp13
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/platform/board/mp16c
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/platform/board/mp16
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/platform/board/gf61
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/platform/driver
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/platform/inc/frw
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/platform/inc/oal
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/platform/inc/oal/linux
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/platform/inc/pm
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/platform/inc/oam
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/platform/oam
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/platform/pm
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/platform/excp
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/platform/firmware
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/platform/oal
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/platform/frw
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/platform/sdt
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/common/customize
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/wifi/chip
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/wifi/chip/mp13
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/common/oneimage
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/common/inc
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/common/chr_log
HI110X_INCLUDES += -I$(HI110X_DRIVER_SRC_PATH)/bfgx

# 大屏编译需要该目录下头文件
ifeq (Hi3751, $(findstring Hi3751,$(TARGET_DEVICE)))
KERNEL_DIR := $(srctree)
HI110X_INCLUDES += -I$(SDK_DIR)/source/common/include
HI110X_INCLUDES += -I$(SDK_DIR)/source/msp/drv/include
KBUILD_CFLAGS += -D_PRE_PRODUCT_HI3751V811
endif
ifeq (v818, $(findstring v818,$(TARGET_DEVICE)))
KERNEL_DIR := $(srctree)
HI110X_INCLUDES += -I$(SDK_DIR)/source/common/include
HI110X_INCLUDES += -I$(SDK_DIR)/source/msp/drv/include
KBUILD_CFLAGS += -D_PRE_PRODUCT_HI3751V811
KBUILD_CFLAGS += -D_PRE_PRODUCT_HI3751_PLATO
HI110X_INCLUDES += -I$(SDK_DIR)/source/msp/include
endif
ifeq (v900, $(findstring v900,$(TARGET_PRODUCT)))
KERNEL_DIR := $(srctree)
HI110X_INCLUDES += -I$(SDK_DIR)/source/common/include
HI110X_INCLUDES += -I$(SDK_DIR)/source/msp/drv/include
KBUILD_CFLAGS += -D_PRE_PRODUCT_HI3751V811
KBUILD_CFLAGS += -D_PRE_PRODUCT_HI3751_PLATO
endif

ifeq ($(PWL_ENABLE), y)
WIFI_HOST_DIR_PWL = $(HI110X_DRIVER_SRC_PATH)/../drivers_pwl/wifi/hmac
HI110X_INCLUDES += -I$(WIFI_HOST_DIR_PWL)/05_pwl
HI110X_INCLUDES += -I$(WIFI_HOST_DIR_PWL)/05_gcm
endif

#$(warning INCLUDE: $(HI110X_INCLUDES))

ifneq ($(TARGET_BUILD_VARIANT),user)
KBUILD_CFLAGS += -DPLATFORM_DEBUG_ENABLE
KBUILD_CFLAGS += -DPLATFORM_SSI_FULL_LOG
KBUILD_CFLAGS += -DWIFI_DEBUG_ENABLE
endif

EXTRA_CFLAGS = $(HI110X_INCLUDES) $(COPTS)
EXTRA_CFLAGS += $(COPTS)
EXTRA_CFLAGS += $(COMM_COPTS)
ifeq ($(CONFIG_FTRACE_ENABLE),yes)
EXTRA_CFLAGS += -finstrument-functions -DFTRACE_ENABLE
endif
ifeq ($(CONN_NON_STRICT_CHECK),y)
$(warning hisi connectivity driver compile strict check disable)
else
EXTRA_CFLAGS += -Werror
endif
EXTRA_CFLAGS +=  -fno-pic -fstack-protector-strong
MODFLAGS = -fno-pic
else
#make modules
export HI1102_MAKE_FLAG=MODULES

ifeq (Hi3751, $(findstring Hi3751,$(TARGET_DEVICE)))
# 大屏Hi3751平台编译使用的参数
export HI110X_DRIVER_SRC_PATH ?= $(shell pwd)/..
MODULE_PARAM ?= ARCH=arm64 CROSS_COMPILE=aarch64-hisiv610-linux- CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT=yes
KERNEL_DIR ?= $(LINUX_SRC)
else ifeq (v818, $(findstring v818,$(TARGET_DEVICE)))
MODULE_PARAM ?= ARCH=arm64 CROSS_COMPILE=aarch64-hi100-linux- CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT=no
KERNEL_DIR ?= $(LINUX_SRC)
else ifeq (v900, $(findstring v900,$(TARGET_PRODUCT)))
MODULE_PARAM ?= ARCH=arm64 CROSS_COMPILE=aarch64-hi100-linux- CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT=no
KERNEL_DIR ?= $(LINUX_SRC)
else
# 非Hi3751平台编译使用的参数
export HI110X_DRIVER_SRC_PATH ?= $(shell pwd)/..

ANDROID_PRODUCT=hi6210sft
ifeq ($(ARCH),arm64)
ANDROID_PATH ?= /home/zhouxinfeng/v8/v8r2-sft-bbit
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
endif
