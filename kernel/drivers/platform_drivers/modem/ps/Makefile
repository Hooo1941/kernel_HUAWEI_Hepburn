MODEM_DIR := $(srctree)/drivers/platform_drivers/modem
include $(srctree)/drivers/platform_drivers/modem/product/${OBB_PRODUCT_NAME}/product/${GBB_CUST_NAME}/define/acore/ps/product_config_ps_ap.mk
include $(srctree)/drivers/platform_drivers/modem/product/${OBB_PRODUCT_NAME}/product/${GBB_CUST_NAME}/define/common/version_pub/product_config_version.mk
CC_USER_FLAGS   := -Werror

EXTRA_CFLAGS += -I$(MODEM_DIR)/include/adrv/
EXTRA_CFLAGS += -I$(MODEM_DIR)/include/bmi/acore/user/kernel/
EXTRA_CFLAGS += -I$(MODEM_DIR)/include/bmi/acore/kernel/
EXTRA_CFLAGS += -I$(MODEM_DIR)/ps/inc/
EXTRA_CFLAGS += -I$(MODEM_DIR)/config/msg/public/
EXTRA_CFLAGS += -I$(MODEM_DIR)/product/${OBB_PRODUCT_NAME}/product/${GBB_CUST_NAME}/define/acore/ps
EXTRA_CFLAGS += -I$(MODEM_DIR)/product/${OBB_PRODUCT_NAME}/product/${GBB_CUST_NAME}/define/common/version_pub

ifeq ($(CFG_CONFIG_RNIC), YES)
EXTRA_CFLAGS += -I$(MODEM_DIR)/ps/rnic_fusion/
EXTRA_CFLAGS += -I$(MODEM_DIR)/ps/rnic_fusion/infrastructure/
EXTRA_CFLAGS += -I$(MODEM_DIR)/ps/rnic_fusion/infrastructure/dump/
EXTRA_CFLAGS += -I$(MODEM_DIR)/ps/rnic_fusion/infrastructure/pkterr/
EXTRA_CFLAGS += -I$(MODEM_DIR)/ps/rnic_fusion/vnet/pktcap/
EXTRA_CFLAGS += -I$(MODEM_DIR)/ps/rnic_fusion/vnet/wakeup/
EXTRA_CFLAGS += -I$(MODEM_DIR)/ps/rnic_fusion/vnet/priority/
EXTRA_CFLAGS += -I$(MODEM_DIR)/ps/rnic_fusion/vnet/dpl/
endif

ifeq ($(CFG_CONFIG_RNIC_SRS), YES)
EXTRA_CFLAGS += -I$(MODEM_DIR)/ps/rnic_fusion/vnet/
endif

ifeq ($(CFG_CONFIG_RNIC_GRO), YES)
EXTRA_CFLAGS += -I$(MODEM_DIR)/ps/rnic_fusion/vnet/
endif

ifeq ($(CFG_CONFIG_RNIC_PRIORITY_MAP), YES)
EXTRA_CFLAGS += -I$(MODEM_DIR)/ps/rnic_fusion/vnet/
endif

ifeq ($(CFG_CONFIG_IPS), YES)
EXTRA_CFLAGS += -I$(MODEM_DIR)/ps/nfext/
endif

ifeq ($(CFG_FEATURE_BASTET), YES)
EXTRA_CFLAGS += -I$(MODEM_DIR)/ps/bastet/inc/
EXTRA_CFLAGS += -DCONFIG_BASTET_COMM
EXTRA_CFLAGS += -I$(srctree)/drivers/huawei_platform/inputhub/default/
EXTRA_CFLAGS += -DCONFIG_BASTET_IN_SENSORHUB
endif

ifeq ($(CFG_CONFIG_PCS), YES)
EXTRA_CFLAGS += -I$(MODEM_DIR)/ps/pcs/
EXTRA_CFLAGS += -I$(MODEM_DIR)/ps/pcs/bc_cap/
endif

ps-objs := init/ps_acore_init.o

ifeq ($(CFG_CONFIG_RNIC), YES)
ps-objs += rnic_fusion/rnic_init.o
ps-objs += rnic_fusion/rnic_register.o
ps-objs += rnic_fusion/rnic_wwan.o
ps-objs += rnic_fusion/infrastructure/rnic_log.o
ps-objs += rnic_fusion/infrastructure/rnic_fragment.o
ps-objs += rnic_fusion/infrastructure/pkterr/rnic_pkterr.o
ps-objs += rnic_fusion/vnet/rnic_vnet_cfg.o
ps-objs += rnic_fusion/vnet/rnic_vnet_enet.o
ps-objs += rnic_fusion/vnet/rnic_vnet_hdlr.o
ps-objs += rnic_fusion/vnet/rnic_vnet_ioctl.o
ps-objs += rnic_fusion/vnet/rnic_vnet_dbg.o
endif

ifeq ($(CFG_CONFIG_RNIC_MULTI_IF), YES)
ps-objs += rnic_fusion/rnic_multi_if.o
endif

ifeq ($(CFG_CONFIG_RNIC_WAKEUP), YES)
ps-objs += rnic_fusion/infrastructure/rnic_wakeup.o
endif

ifeq ($(CFG_CONFIG_RNIC_DUMP), YES)
ps-objs += rnic_fusion/infrastructure/dump/rnic_dump.o
endif

ifeq ($(CFG_CONFIG_RNIC_PKTCAP), YES)
ps-objs += rnic_fusion/vnet/pktcap/rnic_pktcap.o
endif

ifeq ($(CFG_CONFIG_RNIC_DPL), YES)
ps-objs += rnic_fusion/vnet/dpl/rnic_vnet_dpl.o
endif

ifeq ($(CFG_CONFIG_RNIC_DPL_LATENCY), YES)
ps-objs += rnic_fusion/vnet/dpl/rnic_vnet_dpl_latency.o
endif

ifeq ($(CFG_CONFIG_RNIC_PRIORITY_MAP), YES)
ps-objs += rnic_fusion/vnet/priority/rnic_priority_map.o
endif

ifeq ($(CFG_CONFIG_RNIC_SRS), YES)
ps-objs += rnic_fusion/vnet/srs/rnic_srs_boost.o
ps-objs += rnic_fusion/vnet/srs/rnic_srs_cc.o
ps-objs += rnic_fusion/vnet/srs/rnic_srs_lb.o
ps-objs += rnic_fusion/vnet/srs/rnic_srs_main.o
ps-objs += rnic_fusion/vnet/srs/rnic_srs_napi.o
ps-objs += rnic_fusion/vnet/srs/rnic_srs_dbg.o
endif

ifeq ($(CFG_CONFIG_RNIC_GRO), YES)
ps-objs += rnic_fusion/vnet/gro/rnic_gro_checksum.o
ps-objs += rnic_fusion/vnet/gro/rnic_gro_hash.o
ps-objs += rnic_fusion/vnet/gro/rnic_gro_main.o
ps-objs += rnic_fusion/vnet/gro/rnic_gro_ip.o
ps-objs += rnic_fusion/vnet/gro/rnic_gro_tcp.o
ps-objs += rnic_fusion/vnet/gro/rnic_gro_udp.o
endif

ifeq ($(CFG_CONFIG_RNIC_TUN), YES)
ps-objs += rnic_fusion/vnet/tun/rnic_tun.o
endif

ifeq ($(CFG_CONFIG_RNIC_LTEV), YES)
ps-objs += rnic_fusion/ltev/rnic_ltev.o
ps-objs += rnic_fusion/ltev/rnic_ltev_dpl.o
endif

ifeq ($(CFG_CONFIG_RNIC_LTEV), YES)
ifeq ($(CFG_CONFIG_PS_AP_ENG), YES)
ps-objs += ltev_test/ltev_datatest.o
endif
endif

ifeq ($(CFG_CONFIG_IPS), YES)
ps-objs += nfext/ips_mntn.o
ps-objs += nfext/ips_mntn_cfg.o
ps-objs += nfext/ips_traffic_statistic.o
ps-objs += nfext/ips_util.o
ps-objs += nfext/netfilter_buffer.o
ps-objs += nfext/netfilter_ex.o
ps-objs += nfext/netfilter_ex_ctrl.o
ps-objs += nfext/netfilter_os_adapter.o
endif

ifeq ($(CFG_FEATURE_BASTET), YES)
ps-objs += bastet/src/BastetComm.o
endif

ifeq ($(CFG_CONFIG_PCS), YES)
ps-objs += pcs/bc_cap/pcs_bc_cap_get_path.o
ps-objs += pcs/bc_cap/pcs_bc_cap.o
ps-objs += pcs/pcs_rrc_msg_proc.o
ps-objs += pcs/pcs_log.o
ps-objs += pcs/pcs_main.o

ifeq ($(CFG_CONFIG_PCS_BC_CAP_DIR), YES)
ps-objs += pcs/bc_cap/pcs_bc_cap_dir.o
endif
endif

ifneq ($(strip $(CONFIG_MODEM_DRIVER)),m)
obj-y  := ps.o
endif

