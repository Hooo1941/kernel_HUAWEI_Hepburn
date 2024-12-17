#
# Makefile for the modem drivers.
#
-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk


ifeq ($(strip $(CONFIG_MODEM_DRIVER)),m)
drv-y :=
drv-builtin :=

subdir-ccflags-y += -I$(srctree)/lib/libc_sec/securec_v2/include/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/tzdriver/libhwsecurec/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/platform/ccore/$(CFG_PLATFORM)/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/platform/dsp/$(CFG_PLATFORM)/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/platform/acore/$(CFG_PLATFORM)/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/platform/acore/$(CFG_PLATFORM)/drv/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/platform/common/$(CFG_PLATFORM)/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/platform/common/$(CFG_PLATFORM)/soc/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/adrv/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/drv/acore/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/drv/common/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/common/include/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/tl/drv/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/product/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/tl/oam/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/tl/lps/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/acore/sys/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/acore/drv/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/acore/msp/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/acore/pam/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/acore/guc_as/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/common/sys/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/common/drv/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/common/msp/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/common/pam/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/common/guc_as/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/common/guc_nas/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/common/tl_as
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/phy/lphy/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/config/nvim/include/gu/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nva/comm/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/taf/common/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/taf/acore/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/platform/common/

subdir-ccflags-y+= -Idrivers/platform_drivers/modem/drv/rtc
subdir-ccflags-y+= -Idrivers/platform_drivers/modem/drv/mem
subdir-ccflags-y+= -Idrivers/platform_drivers/modem/drv/om \
                   -Idrivers/platform_drivers/modem/drv/om/common \
                   -Idrivers/platform_drivers/modem/drv/om/dump \
                   -Idrivers/platform_drivers/modem/drv/om/log
subdir-ccflags-y+= -Idrivers/platform_drivers/modem/drv/udi
subdir-ccflags-y+= -Idrivers/platform_drivers/modem/drv/timer
subdir-ccflags-y+= -Idrivers/usb/gadget
subdir-ccflags-y+= -Idrivers/platform_drivers/modem/drv/diag/scm \
                   -Idrivers/platform_drivers/modem/drv/diag/cpm \
                   -Idrivers/platform_drivers/modem/drv/diag/debug \
                   -Idrivers/platform_drivers/modem/drv/diag/soft_decode
subdir-ccflags-y+= -Idrivers/platform_drivers/modem/include/tools
subdir-ccflags-y+= -Idrivers/platform_drivers/modem/drv/modem_boot
subdir-ccflags-y+= -Idrivers/platform_drivers/modem/drv/ddr
subdir-ccflags-y += -DMODULE_EXTRA_MODEM

drv-y += dt/device_tree_native.o

#sysboot
drv-y += sysboot/sysboot_main.o
drv-y += sysboot/sysboot_main_extra.o

ifeq ($(strip $(CFG_CONFIG_RFS_SERVER)),YES)
drv-y += rfs_server/rfs_server.o
endif

ifeq ($(strip $(CFG_CONFIG_IOT_PCIE_DEV)),YES)
drv-y += pcie_iot_dev/pcie_iot_dev.o
drv-y += pcie_boot_dev/pcie_boot_dev.o
drv-y += pcie_iot_dev/pcie_iot_debug.o
endif

ifeq ($(strip $(CFG_CONFIG_IOT_PCIE_DEV_TEST)),YES)
drv-y += pcie_iot_dev/pcie_iot_dev_test.o
endif

ifeq ($(strip $(CFG_CONFIG_IOT_PCIE_DEV_PM)),YES)
drv-y += pcie_iot_dev/pcie_iot_dev_pm.o
drv-y += pcie_iot_dev/pcie_iot_dev_pm_api.o
endif

ifeq ($(strip $(CFG_CONFIG_IOT_PCIE_DEV_PM_TEST)),YES)
drv-y += pcie_iot_dev/pcie_iot_dev_pm_test.o
endif

ifeq ($(strip $(CFG_CONFIG_MODEM_BOOT)),YES)
drv-y += modem_boot/modem_load.o
drv-y += modem_boot/modem_boot.o
endif

ifeq ($(strip $(CFG_CONFIG_MLOADER_HOT_PATCH)),YES)
drv-y += modem_boot/modem_load_patch.o
endif

drv-y += memory_layout/memory_layout.o
drv-y += memory/memory_driver.o

ifeq ($(strip $(CFG_CONFIG_PCIE_CDEV_RC)),YES)
drv-y += pfunc/pcie_cdev.o
drv-y += pfunc/pcie_cdev_rc.o
drv-y += pfunc/pcie_cdev_desc.o
drv-y += pfunc/pcie_cdev_dbg.o
drv-y += pfunc/pcie_cdev_test.o
drv-y += pfunc/hisi_adp_pcdev_rc.o
drv-y += pfunc/pcie_usb_relay.o
ifeq ($(strip $(CFG_CONFIG_PCIE_CDEV_ENG)),YES)
ifeq ($(strip $(CFG_CONFIG_PCIE_MODEM)),YES)
drv-y += pfunc/pcie_modem.o
endif
endif
endif

drv-y           += adp/adp_usb.o

ifeq ($(strip $(CFG_CONFIG_EXTRA_TIMERSLICE)),YES)
drv-y += timer/ab_slice.o
endif

ifeq ($(strip $(CFG_CONFIG_EXTRA_SLICE_SYNC)),YES)
drv-y += timer/slice_sync.o
endif

ifeq ($(strip $(CFG_CONFIG_WETH)),YES)
drv-y += wan_eth/wan_eth_rc.o
drv-y += wan_eth/wan_eth_table_rc.o
drv-y += wan_eth/wan_eth_event_buffer.o
drv-y += wan_eth/wan_eth_debug_rc.o
endif

ifeq ($(strip $(CFG_CONFIG_WETH)),YES)
drv-y += net_om/net_om_rc.o
endif

ifeq ($(strip $(CFG_CONFIG_MODEM_GLOBAL_RESET)),YES)
drv-y += reset/reset_modem_global.o
endif
ifeq ($(strip $(CFG_CONFIG_DIAG_SYSTEM)),YES)
subdir-ccflags-y+= -Idrivers/platform_drivers/modem/drv/diag/scm \
                   -Idrivers/platform_drivers/modem/drv/diag/cpm \
                   -Idrivers/platform_drivers/modem/drv/diag/debug \
                   -Idrivers/platform_drivers/modem/drv/diag/comm \
                   -Idrivers/platform_drivers/modem/drv/diag/serivce \
                   -Idrivers/platform_drivers/modem/drv/diag/report \
                   -Idrivers/platform_drivers/modem/drv/odt/soft_decode
subdir-ccflags-y+= -Idrivers/platform_drivers/modem/include/tools

drv-y           += diag/cpm/diag_port_manager.o
drv-y           += diag/debug/diag_system_debug.o
drv-y           += diag/comm/diag_comm.o
drv-y           += odt/soft_decode/ring_buffer.o
endif

drv-y           += chr/chr_stub.o

ifeq ($(strip $(CFG_CONFIG_DIAG_NETLINK)),YES)
drv-y           += diag_vcom/diag_vcom_main.o
drv-y           += diag_vcom/diag_vcom_handler.o
endif

ifeq ($(strip $(CFG_CONFIG_EFUSE)),YES)
drv-y           += efuse/efuse_extra_agent.o
endif

drv-y += version/version_emdm.o

drv-y+= log/bsp_om_log.o
drv-y+= adp/adp_print.o
drv-y+= block/blk_base.o
drv-y+= block/blk_ops_mmc.o


ifeq ($(strip $(CFG_CONFIG_NVIM)),YES)
subdir-ccflags-y+= -Idrivers/platform_drivers/modem/drv/sec_nvim/extra
drv-y += sec_nvim/extra/nv_reset.o
endif


# module makefile end
else

obj-y           += dt/device_tree_native.o

#sysboot
obj-y           += sysboot/sysboot_main_extra.o

obj-y           += adp/adp_usb.o

ifeq ($(strip $(CFG_CONFIG_DIAG_SYSTEM)),YES)
subdir-ccflags-y+= -Idrivers/platform_drivers/modem/drv/diag/scm \
                   -Idrivers/platform_drivers/modem/drv/diag/cpm \
                   -Idrivers/platform_drivers/modem/drv/diag/debug \
                   -Idrivers/platform_drivers/modem/drv/diag/comm \
                   -Idrivers/platform_drivers/modem/drv/diag/serivce \
                   -Idrivers/platform_drivers/modem/drv/diag/report \
                   -Idrivers/platform_drivers/modem/drv/odt/soft_decode
subdir-ccflags-y+= -Idrivers/platform_drivers/modem/include/tools

obj-y           += diag/cpm/diag_port_manager.o
obj-y           += diag/debug/diag_system_debug.o
obj-y           += diag/comm/diag_comm.o
obj-y           += odt/soft_decode/ring_buffer.o
endif

##ifeq ($(strip $(CFG_CONFIG_MODULE_IPC)),YES)
##obj-y               += ipc/
##endif

ifeq ($(strip $(CFG_CONFIG_MODEM_GLOBAL_RESET)),YES)
obj-y += reset/reset_modem_global.o
endif

ifeq ($(strip $(CFG_CONFIG_MODEM_BOOT)),YES)
obj-y += modem_boot/modem_load.o
obj-y += modem_boot/modem_boot.o
endif


obj-y += memory_layout/memory_layout.o
obj-y += memory/memory_driver.o
subdir-ccflags-y+= -Idrivers/platform_drivers/modem/drv/memory
subdir-ccflags-y+= -Idrivers/platform_drivers/modem/drv/memory_layout

ifeq ($(strip $(CFG_CONFIG_RFS_SERVER)),YES)
obj-y += rfs_server/
endif

ifeq ($(strip $(CFG_CONFIG_IOT_PCIE_DEV)),YES)
obj-y += pcie_iot_dev/pcie_iot_dev.o
obj-y += pcie_boot_dev/pcie_boot_dev.o
obj-y += pcie_iot_dev/pcie_iot_debug.o
endif

ifeq ($(strip $(CFG_CONFIG_IOT_PCIE_DEV_TEST)),YES)
obj-y += pcie_iot_dev/pcie_iot_dev_test.o
endif

ifeq ($(strip $(CFG_CONFIG_IOT_PCIE_DEV_PM)),YES)
obj-y += pcie_iot_dev/pcie_iot_dev_pm.o
obj-y += pcie_iot_dev/pcie_iot_dev_pm_api.o
endif
ifeq ($(strip $(CFG_CONFIG_IOT_PCIE_DEV_PM_TEST)),YES)
obj-y += pcie_iot_dev/pcie_iot_dev_pm_test.o
endif
ifeq ($(strip $(CFG_CONFIG_PCIE_CDEV_RC)),YES)
obj-y += pfunc/
endif

obj-y += adp/adp_usb.o
ifeq ($(strip $(CFG_CONFIG_DIAG_SYSTEM)),YES)
subdir-ccflags-y+= -Idrivers/platform_drivers/modem/drv/diag/scm \
                   -Idrivers/platform_drivers/modem/drv/diag/cpm \
                   -Idrivers/platform_drivers/modem/drv/diag/debug \
                   -Idrivers/platform_drivers/modem/drv/diag/comm \
                   -Idrivers/platform_drivers/modem/drv/diag/serivce \
                   -Idrivers/platform_drivers/modem/drv/diag/report \
                   -Idrivers/platform_drivers/modem/drv/odt/soft_decode
subdir-ccflags-y+= -Idrivers/platform_drivers/modem/include/tools
subdir-ccflags-y+= -Idrivers/platform_drivers/modem/include/nv/acore/drv
subdir-ccflags-y+= -Idrivers/platform_drivers/modem/include/nv/common/drv
obj-y           += diag/cpm/diag_port_manager.o
obj-y           += diag/debug/diag_system_debug.o
obj-y           += diag/comm/diag_comm.o
obj-y           += odt/soft_decode/ring_buffer.o
endif

drv-y           += chr/chr_stub.o

ifeq ($(strip $(CFG_CONFIG_DIAG_NETLINK)),YES)
obj-y           += diag_vcom/diag_vcom_main.o
obj-y           += diag_vcom/diag_vcom_handler.o
endif

ifeq ($(strip $(CFG_CONFIG_EFUSE)),YES)
obj-y           += efuse/efuse_extra_agent.o
endif

obj-y += version/version_emdm.o
obj-y += block/blk_base.o
obj-y += block/blk_ops_mmc.o

ifeq ($(strip $(CFG_CONFIG_NVIM)),YES)
subdir-ccflags-y+= -Idrivers/platform_drivers/modem/drv/sec_nvim/extra
obj-y += sec_nvim/extra/nv_reset.o
endif

obj-y+= log/bsp_om_log.o
obj-y+= adp/adp_print.o

endif
