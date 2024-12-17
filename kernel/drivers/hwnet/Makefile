#=============================================================================
#  wifi
#=============================================================================
obj-$(CONFIG_WIFI_DELAY_STATISTIC) += ipv4/wifi_delayst.o
obj-$(CONFIG_HW_WIFI) += ipv4/
obj-$(CONFIG_HW_BOOSTER) += booster/
ifeq (taro, $(findstring taro,$(TARGET_DEVICE)))
obj-y += hwril/
obj-y += hwril_sim_sd/
else
obj-$(CONFIG_VSIM_ADAPT) += hwril/
obj-$(CONFIG_SIM_SD_MUX) += hwril_sim_sd/
endif
obj-$(CONFIG_CHR_NETLINK_MODULE) += chr/
obj-$(CONFIG_HW_DC_MODULE) += hw_dual_connection/
obj-$(CONFIG_HW_RSMC) += rsmc/
obj-$(CONFIG_HW_NETWORK_DCP) += network_dcp/
obj-$(CONFIG_HW_DPIMARK_MODULE) += hw_dpi_mark/
ifeq (taro, $(findstring taro,$(TARGET_DEVICE)))
obj-$(CONFIG_HW_BOOSTER) += waudio/
else
obj-$(CONFIG_HW_WAUDIO_MODULE) += waudio/
endif
obj-$(CONFIG_HW_WIFI_EXT) += wifi_ext_adaptor/
obj-$(CONFIG_HW_SATE_COMM) += hw_sate_comm/
EXTRA_CFLAGS += -Idriver/huawei_platform
