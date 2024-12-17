#
# Makefile for PCI NET PORT Driver
#

subdir-ccflags-y := -Werror
#Build PCI NET if needed
#-I $(GBB_TOPDIR)/system/kernel/drivers/platform_drivers/spe/spe_drv
-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk

ifeq ($(strip $(CFG_CONFIG_WETH)),YES)
obj-y	+= wan_eth_rc.o wan_eth_table_rc.o wan_eth_event_buffer.o wan_eth_debug_rc.o
endif

ifeq ($(strip $(CFG_CONFIG_WETH_DEV)),YES)
obj-y	+=  wan_eth_ep.o wan_eth_table_ep.o wan_eth_dma.o wan_eth_event_buffer.o wan_eth_spe.o wan_eth_debug_ep.o wan_eth_stick.o
endif

GCOV_PROFILE := y