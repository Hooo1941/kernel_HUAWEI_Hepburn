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
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/include/
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

subdir-ccflags-y+= -I$(srctree)/drivers/platform_drivers/modem/drv/rtc
subdir-ccflags-y+= -I$(srctree)/drivers/platform_drivers/modem/drv/mem \
                   -I$(srctree)/drivers/platform_drivers/modem/drv/memory
subdir-ccflags-y+= -I$(srctree)/drivers/platform_drivers/modem/drv/memory_layout
subdir-ccflags-y+= -I$(srctree)/drivers/platform_drivers/modem/drv/om \
                   -I$(srctree)/drivers/platform_drivers/modem/drv/om/common \
                   -I$(srctree)/drivers/platform_drivers/modem/drv/om/dump \
                   -I$(srctree)/drivers/platform_drivers/modem/drv/om/log
subdir-ccflags-y+= -I$(srctree)/drivers/platform_drivers/modem/drv/udi
subdir-ccflags-y+= -I$(srctree)/drivers/platform_drivers/modem/drv/timer
subdir-ccflags-y+= -I$(srctree)/drivers/platform_drivers/modem/drv/nmi
subdir-ccflags-y+= -I$(srctree)/drivers/usb/gadget
subdir-ccflags-y+= -I$(srctree)/drivers/platform_drivers/modem/include/tools
subdir-ccflags-y+= -I$(srctree)/drivers/platform_drivers/modem/include/bmi/acore/kernel
subdir-ccflags-y+= -I$(srctree)/drivers/platform_drivers/modem/include/bmi/acore/user/kernel

ifeq ($(strip $(CFG_CONFIG_KERNEL_DTS_DECOUPLED)),YES)
subdir-ccflags-y+= -I$(srctree)/drivers/platform_drivers/modem/drv/dt/
subdir-ccflags-y+= -I$(srctree)/drivers/platform_drivers/modem/drv/dt/fdt/
drv-y           += dt/device_tree_fdt.o
drv-y           += dt/device_tree_load_acore_dtb.o

drv-y           += dt/fdt/fdt.o
drv-y           += dt/fdt/fdt_ro.o
drv-y           += dt/fdt/fdt_overlay.o
drv-y           += dt/fdt/fdt_rw.o
drv-y           += dt/fdt/fdt_strerror.o
drv-y           += dt/fdt/fdt_wip.o
else
drv-y           += dt/device_tree_native.o
endif

drv-y           += dt/device_tree_load_untils.o
drv-y           += dt/device_tree_load_fw_dtb.o
drv-y           += dt/device_tree_load_ccore_dt.o
drv-y           += dt/device_tree_driver.o

ifneq ($(strip $(CFG_ATE_VECTOR)),YES)
drv-y           += adp/adp_version.o
drv-y           += onoff/adp_onoff.o
drv-y           += adp/adp_om.o
drv-$(CONFIG_USB_GADGET)     += adp/adp_usb.o

ifeq ($(strip $(CFG_CONFIG_RFILE_ON)),YES)
drv-y += rfile/rfile_server.o
drv-y += rfile/rfile_server_dump.o
drv-y += rfile/rfile_server_load_mode.o
drv-y += rfile/rfile_server_fipc.o
endif

drv-y           += adp/adp_timer.o
drv-y           += adp/adp_wifi.o
ifeq ($(strip $(CFG_CONFIG_MODEM_CORESIGHT)),YES)
ifeq ($(strip $(CFG_CONFIG_OCD_V200)),YES)
drv-y           += adp/adp_coresight_v1.o
else
drv-y           += adp/adp_coresight.o
endif
endif
drv-y           += adp/adp_charger.o
else
drv-y           += adp/adp_timer.o
endif

drv-y           += block/blk_base.o
drv-y           += block/mmc/mmc_ops.o

ifeq ($(strip $(CFG_CONFIG_CHR_FS)),YES)
drv-y           += chr/chr_v200/chr_report.o
drv-y           += chr/chr_v200/chr_core.o
drv-y           += chr/chr_v200/chr_cfg.o
drv-y           += chr/chr_v200/chr_debug.o
else
drv-y           += chr/chr_v200/chr_stub.o
endif

drv-y           += adp/adp_socket.o

ifeq ($(strip $(CFG_CONFIG_OM_SOCKET)),YES)
drv-y           += msocket/msocket.o
endif

ifeq ($(strip $(CFG_FEATURE_SVLSOCKET)),YES)
drv-y           += svlan_socket/
drv-y           += bvp_vlan/
endif

ifeq ($(strip $(CFG_FEATURE_VLAN)),YES)
drv-y           += bvp_vlan/
endif


ifeq ($(strip $(CFG_CONFIG_MODULE_TIMER)),YES)
drv-y   += timer/timer_slice.o
drv-y   += timer/hrtimer.o
drv-y   += timer/timer.o
drv-y   += timer/timer_device.o
drv-y   += timer/timer_dpm.o
drv-y   += timer/timer_hal.o
drv-y   += timer/timer_driver_arm.o
drv-y   += timer/timer_driver_modem.o
endif

ifeq ($(strip $(CFG_CONFIG_CSHELL)),YES)
drv-y += console/virtshell.o
drv-y += console/cshell_port.o
endif

ifeq ($(strip $(CFG_CONFIG_MODEM_MSG)),YES)
subdir-ccflags-y+= -I$(srctree)/drivers/platform_drivers/modem/drv/msg
drv-y += msg/msg_alias.o
drv-y += msg/msg_base.o
drv-y += msg/msg_tskque.o
drv-y += msg/msg_event.o
drv-y += msg/msg_mem.o
drv-y += msg/msg_core.o
drv-y += msg/msg_lite.o
drv-y += msg/msg_cmsg.o
drv-y += msg/msg_mntn.o
drv-y += msg/msg_reset.o
drv-y += msg/fipc_ops.o
endif

ifeq ($(strip $(CFG_CONFIG_FIPC_V200)),YES)
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/fipc
drv-y += fipc/fipc_core.o
drv-y += fipc/fipc_device.o
drv-y += fipc/fipc_driver.o
drv-y += fipc/fipc_dump.o
drv-y += fipc/fipc_platform.o
drv-y += fipc/fipc_pmsr.o
drv-y += fipc/fipc_reset.o
drv-y += fipc/fipc_dts.o
drv-y += fipc/fipc_diff.o
ifeq ($(strip $(CFG_CONFIG_FIPC_SUPPORT_HIFI_RESET)),YES)
drv-y += fipc/fipc_reset_hifi.o
else
drv-y += fipc/fipc_reset_hifi_stub.o
endif
endif

ifeq ($(strip $(CFG_CONFIG_MODEM_PINCTRL)),YES)
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/pinctrl
drv-y += pinctrl/pinctrl_driver.o
endif

ifneq ($(strip $(CFG_CONFIG_HOMI_DISABLE)),YES)
drv-y += homi/homi_core.o
drv-y += homi/homi_debug.o
drv-y += homi/homi_interface.o
drv-y += homi/homi_usb.o
drv-y += homi/homi_socket.o
drv-y += homi/homi_netlink.o
drv-y += homi/homi_genl.o
endif

ifeq ($(strip $(CFG_CONFIG_DIAG_SYSTEM)),YES)
subdir-ccflags-y+= -I$(srctree)/drivers/platform_drivers/modem/drv/diag/ \
                   -I$(srctree)/drivers/platform_drivers/modem/drv/diag/msgserver \
                   -I$(srctree)/drivers/platform_drivers/modem/drv/diag/srvchan

drv-y           += diag/diag_init.o
drv-y           += diag/diagmsg.o
drv-y           += diag/encoder.o
drv-y           += diag/decoder.o
drv-y           += diag/logreport.o
drv-y           += diag/transfertask.o
drv-y           += diag/diagnve.o
drv-y           += diag/diag_api.o
drv-y           += diag/diag_debug.o
drv-y           += diag/msgserver/configsrv.o
drv-y           += diag/msgserver/connectsrv.o
drv-y           += diag/msgserver/msgsrv.o
drv-y           += diag/srvchan/nlsrvport.o
drv-y           += diag/srvchan/srvchan.o
drv-y           += memory_layout/memory_report.o

subdir-ccflags-y+= -I$(srctree)/drivers/platform_drivers/modem/drv/omserver/omp \
                   -I$(srctree)/drivers/platform_drivers/modem/drv/omserver/hdlc

drv-y           += omserver/omp/omp_service.o
drv-y           += omserver/omp/hdlc/algorithm.o
drv-y           += omserver/omp/hdlc/decode.o
drv-y           += omserver/omp/hdlc/ring_buffer.o
endif

ifeq ($(strip $(CFG_CONFIG_MPROBE)),YES)
subdir-ccflags-y+= -I$(srctree)/drivers/platform_drivers/modem/drv/omserver/mprobe

drv-y           += omserver/mprobe/mprobe_init.o
drv-y           += omserver/mprobe/mprobe_channel_manager.o
drv-y           += omserver/mprobe/mprobe_message.o
drv-y           += omserver/mprobe/mprobe_port.o
drv-y           += omserver/mprobe/mprobe_service.o
drv-y           += omserver/mprobe/mprobe_debug.o
endif

#gen uuid
subdir-ccflags-y+= -DCURRENT_MODEM_IMAGE_ID=\"$(GBB_MODEM_IMAGEID)\"
drv-y           += uuid/uuid_modem.o

ifeq ($(strip $(CFG_ENABLE_BUILD_OM)),YES)
drv-y           += dump/dump_core.o
drv-y           += dump/dump_frame.o
drv-y           += dump/dump_boot_check.o
drv-y           += dump/dump_config.o
drv-y           += dump/dump_sharemem.o
drv-y           += dump/logs/dump_log_strategy.o
drv-y           += dump/logs/dump_log_parser.o
drv-y           += dump/logs/dump_logs.o
drv-y           += dump/logs/dump_log_uploader.o
drv-y           += dump/logs/dump_extra_logs.o
drv-y           += dump/logs/dump_genl.o
drv-y           += dump/logs/dump_reset_log.o
drv-y           += dump/logs/dump_avs_escape.o
drv-y           += dump/logs/dump_sec_log.o
drv-y           += dump/logs/dump_teeos_service.o
drv-y           += dump/mdmap/dump_area.o
ifeq ($(strip $(CFG_BSP_CONFIG_PHONE_TYPE)),YES)
drv-y           += dump/mdmap/dump_baseinfo.o
endif
drv-y           += dump/mdmap/dump_notifer.o
drv-y           += dump/mdmap/dump_ko.o
drv-y           += dump/mdmap/dump_eproc.o
drv-y           += dump/mdmap/dump_field_strategy.o
ifeq ($(strip $(CFG_BSP_CONFIG_PHONE_TYPE)),YES)
drv-y           += dump/mdmap/dump_phone_eproc.o
drv-y           += dump/mdmap/mdmap_subsys_exc_stub.o
else
drv-y           += dump/mdmap/dump_iot_eproc.o
endif
drv-y           += dump/mdmap/dump_lastkmsg.o
drv-y           += dump/cp_agent/dump_agent.o
ifeq ($(strip $(CFG_BSP_CONFIG_PHONE_TYPE)),YES)
drv-y           += dump/cp_agent/dump_phone_agent.o
else
drv-y           += dump/cp_agent/dump_iot_agent.o
endif
drv-y           += dump/cp_agent/dump_eproc.o
drv-y           += dump/cp_agent/dump_wdt.o
drv-y           += dump/cp_agent/dump_log.o
ifeq ($(strip $(CFG_ENABLE_BUILD_DUMP_MDM_LPM3)),YES)
drv-y           += dump/m3_agent/dump_agent.o
drv-y           += dump/m3_agent/dump_eproc.o
endif
ifeq ($(strip $(CFG_ENABLE_BUILD_NRRDR)),YES)
drv-y           += dump/nr_agent/dump_agent.o
ifeq ($(strip $(CFG_BSP_CONFIG_PHONE_TYPE)),YES)
drv-y           += dump/nr_agent/dump_phone_agent.o
else
drv-y           += dump/nr_agent/dump_iot_agent.o
endif
drv-y           += dump/nr_agent/dump_eproc.o
drv-y           += dump/nr_agent/dump_wdt.o
drv-y           += dump/nr_agent/dump_log.o
endif
ifeq ($(strip $(CFG_CONFIG_DUMP_PLATFORM_V100)),YES)
drv-y           += dump/hal/v100/dump_exc_table.o
endif
ifeq ($(strip $(CFG_CONFIG_DUMP_PLATFORM_V200)),YES)
drv-y           += dump/hal/v200/dump_exc_table.o
endif
subdir-ccflags-y+= -I$(srctree)/drivers/platform_drivers/modem/drv/dump\
                   -I$(srctree)/drivers/platform_drivers/modem/drv/dump/logs\
                   -I$(srctree)/drivers/platform_drivers/modem/drv/dump/mdmap
ifeq ($(strip $(CFG_CONFIG_DUMP_PLATFORM_V100)),YES)
subdir-ccflags-y+= -I$(srctree)/drivers/platform_drivers/modem/drv/dump/hal/v100
endif
ifeq ($(strip $(CFG_CONFIG_DUMP_PLATFORM_V200)),YES)
subdir-ccflags-y+= -I$(srctree)/drivers/platform_drivers/modem/drv/dump/hal/v200
endif
endif

ifeq ($(strip $(CFG_BSP_CONFIG_PHONE_TYPE)),YES)
drv-y           += wdt/watchdog_mdm_intr.o
endif

ifeq ($(strip $(CFG_CONFIG_AP_QIC)),YES)
drv-y   += busprobe/qic_dump_register.o
endif

ifeq ($(strip $(CFG_CONFIG_AP_NOC)),YES)
drv-y   += busprobe/noc_dump_register.o
endif

ifeq ($(strip $(CFG_CONFIG_EFUSE)),YES)
drv-$(CONFIG_EFUSE_IOT)      += efuse/efuse_driver.o
drv-$(CONFIG_EFUSE_MODEM_AGENT)    += efuse/efuse_agent.o
endif
drv-y               += efuse/efuse_ioctl.o
drv-y               += efuse/efuse_dump.o

# vsim
drv-y += vsim/vsim_driver.o

subdir-ccflags-y 	+= -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/core\
                       -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/hal/common\
                       -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/entry\
                       -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/tft\
                       -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/wan\
                       -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/dbg\
                       -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/modem_reset\
                       -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/app_if
ifeq ($(strip $(CFG_CONFIG_PFA_V100)),YES)
subdir-ccflags-y 	+= -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/hal/v100
else
subdir-ccflags-y 	+= -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/hal/v0
endif

ifeq ($(strip $(CONFIG_PFA)),y)
drv-y               += pfa/core/pfa_core.o pfa/core/pfa_platform.o pfa/core/pfa_interrupt.o\
                       pfa/core/pfa_desc.o pfa/entry/pfa_ip_entry.o pfa/entry/pfa_mac_entry.o\
                       pfa/core/pfa_port.o pfa/dbg/pfa_dbg.o pfa/modem_reset/pfa_modem_reset.o
drv-y               += pfa/app_if/pfa_sysfs.o
drv-y               += pfa/app_if/pfa_sysfs_main.o
drv-y               += pfa/hal/common/pfa_hal.o
ifeq ($(strip $(CFG_CONFIG_PFA_V100)),YES)
drv-y               += pfa/hal/v100/pfa_hal_version.o
drv-y               += pfa/app_if/pfa_sysfs_pdu.o
drv-y               += pfa/app_if/pfa_sysfs_ip_qos.o
else
drv-y               += pfa/hal/v0/pfa_hal_version.o
drv-y               += pfa/app_if/pfa_sysfs_pdu_stub.o
endif
ifeq ($(strip $(CFG_CONFIG_PFA_DIRECT_FW)),YES)
drv-y               += pfa/entry/pfa_direct_fw.o
endif
endif

ifeq ($(strip $(CFG_CONFIG_PFA_V200)),YES)
drv-y       += pfa/core/pfa_img.o
else
drv-y       += pfa/core/pfa_img_stub.o
endif

ifeq ($(strip $(CFG_CONFIG_WAN)),YES)
drv-y       += pfa/tft/pfa_tft_ap.o pfa/wan/wan.o pfa/wan/lan.o
else
drv-y       += pfa/wan/wan_stub.o
endif

ifeq ($(strip $(CFG_CONFIG_DRV_LTEV)),YES)
drv-y       += pcv_adaptor/pcv_adaptor.o
endif

ifeq ($(strip $(CONFIG_HEATING_CALIBRATION)),y)
drv-y               += heating_calibration.o
endif

ifeq ($(strip $(CFG_CONFIG_NMI)),YES)
drv-y   += nmi/nmi.o
endif

subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/tzdriver

ifeq ($(strip $(CFG_CONFIG_MODULE_IPC_FUSION)),YES)
drv-y += ipc/ipc_fusion.o
drv-y += ipc/ipc_fusion_test.o
else
ifneq ($(strip $(CFG_CONFIG_MODULE_IPCMSG)),YES)
drv-y += ipc/ipc_stub.o
endif
endif

ifeq ($(strip $(CFG_CONFIG_MODULE_IPCMSG)),YES)
drv-y += ipcmsg/ipcmsg_core.o
drv-y += ipcmsg/ipcmsg_device.o
drv-y += ipcmsg/ipcmsg_driver.o
ifneq ($(strip $(CFG_CONFIG_MODULE_IPC_FUSION)),YES)
drv-y += ipcmsg/ipc_resource.o
drv-y += ipcmsg/ipc_interrupt.o
endif
else
drv-y += ipcmsg/ipcmsg_stub.o
endif

ifeq ($(strip $(CFG_CONFIG_LLT_MDRV)),YES)
ifneq ($(sort $(wildcard $(GBB_TOPDIR)/llt/mdrv/blt/common/llt_tools/acore)),)
subdir-ccflags-y += -DDRV_BUILD_LLT
include $(GBB_TOPDIR)/llt/mdrv/blt/common/llt_tools/acore/makefile_acore_llt_tools.mk
endif
endif

ifeq ($(bbit_type),nr)
subdir-ccflags-y += -DBBIT_TYPE_NR=FEATURE_ON
endif

drv-y               += syslog/syslog_print.o
drv-y               += adp/adp_print.o
drv-y               += syslog/syslog_dmesg.o
drv-y               += syslog/syslog_logmem.o
drv-y               += syslog/syslog_logser.o
drv-y               += syslog/syslog_msg.o

drv-y              += memory_layout/memory_layout.o
drv-y              += memory/memory_driver.o
ifeq ($(strip $(CFG_CONFIG_DRA)),YES)
drv-y           += dra/dra_core.o dra/bsp_dra.o dra/dra_plat.o dra/dra_debug.o
ifeq ($(strip $(CFG_CONFIG_DRA_V3)),YES)
drv-y           += dra/dra_hal.o
else
drv-y           += dra/dra_hal_legacy.o
endif
endif

ifeq ($(strip $(CFG_CONFIG_SC_GID_CTRL)),YES)
drv-y           += syscache/sc_gid_ctrl.o syscache/sc_gid_ctrl_hal.o
endif

ifeq ($(strip $(CFG_CONFIG_SYSCACHE)),YES)
drv-y           += syscache/sc_hal.o syscache/sc_main.o
endif

ifeq ($(strip $(CFG_CONFIG_CCPUDEBUG)),YES)
drv-y           += ccpudebug/ccpudebug.o
endif
ifeq ($(strip $(CFG_CONFIG_MODEM_CORESIGHT)),YES)
drv-y           += etrace/coresight_funnel.o etrace/coresight_replicator.o etrace/coresight_tmc.o etrace/coresight_trace_mgr.o etrace/coresight_mprobe.o
endif
ifeq ($(strip $(CFG_CONFIG_SYSVIEW)),YES)
drv-y           += sysview/sysview.o sysview/sysview_comm.o sysview/sysview_hids.o sysview/sysview_mprobe.o sysview/sysview_msg.o
endif

ifeq ($(strip $(CFG_CONFIG_MLOADER)),YES)
drv-y           += mloader/mloader_util.o
drv-y           += mloader/mloader_sec_call.o
drv-y           += mloader/mloader_load_image.o
drv-y           += mloader/mloader_load_lpmcu.o
drv-y           += mloader/mloader_load_modem.o
drv-y           += mloader/mloader_load_patch.o
drv-y           += mloader/mloader_debug.o
drv-y           += mloader/mloader_msg.o
drv-y           += mloader/mloader_main.o
endif

ifeq ($(strip $(CFG_CONFIG_NET_OM)),YES)
drv-y           += net_om/net_om_pkt.o
drv-y           += net_om/net_om_cnt.o
endif

EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/modem/include/nv/product/

drv-$(CONFIG_PMU_OCP)      += pmu/pmu_ocp.o

drv-$(CONFIG_MODEM_ONOFF_PARA)  += onoff/power_para.o
drv-$(CONFIG_MODEM_ONOFF)    += onoff/power_exchange.o
drv-$(CONFIG_MODEM_ONOFF)    += onoff/power_on.o
drv-$(CONFIG_MODEM_ONOFF)    += onoff/power_off.o
drv-$(CONFIG_MODEM_ONOFF)    += onoff/bsp_modem_boot.o
drv-$(CONFIG_MODEM_ONOFF)    += onoff/onoff_msg.o

drv-$(CONFIG_C_SR_STRESS_TEST) += c_pm_stress/c_sr_stress_test.o

drv-$(CONFIG_MDMPM_DEBUG) += mdmpm_debug/mdmpm_debug.o

# reset
drv-y           += adp/adp_reset.o
drv-y       += reset/reset_dev.o
ifeq ($(strip $(CFG_CONFIG_MODEM_RESET)), YES)
drv-y       += reset/reset_frame.o
drv-y       += reset/reset_debug.o
else
drv-y       += reset/reset_stub.o
endif

drv-$(CONFIG_SEC_CALL)          += sec_call/sec_call.o
ifeq ($(strip $(CFG_FEATURE_NVA_ON)),YES)
drv-y                           += nva/nva_modem.o
drv-y                           += nva/nva_partition.o
endif

ifeq ($(strip $(CFG_CONFIG_NV_FUSION)),YES)
drv-$(CONFIG_NVIM)              += sec_nvim/fusion/nv_base.o \
                                   sec_nvim/fusion/nv_debug.o \
                                   sec_nvim/fusion/nv_msg.o \
                                   sec_nvim/fusion/nv_msg_req.o \
                                   sec_nvim/fusion/nv_msg_fipc.o \
                                   sec_nvim/fusion/nv_partition.o \
                                   sec_nvim/fusion/nv_partition_image.o \
                                   sec_nvim/fusion/nv_partition_backup.o \
                                   sec_nvim/fusion/nv_partition_factory.o \
                                   sec_nvim/fusion/nv_upgrade.o
else
drv-$(CONFIG_NVIM)              += sec_nvim/nv_ctrl.o \
                                   sec_nvim/nv_comm.o \
                                   sec_nvim/nv_base.o \
                                   sec_nvim/nv_emmc.o \
                                   sec_nvim/nv_debug.o \
                                   sec_nvim/nv_index.o \
                                   sec_nvim/nv_partition_upgrade.o \
                                   sec_nvim/nv_partition_img.o \
                                   sec_nvim/nv_partition_bakup.o \
                                   sec_nvim/nv_partition_factory.o \
                                   sec_nvim/nv_factory_check.o \
                                   sec_nvim/nv_msg.o \
                                   sec_nvim/nv_proc.o \
                                   sec_nvim/nv_verify.o
ifeq ($(strip $(CFG_CONFIG_NV_FUSION_MSG)),YES)
drv-$(CONFIG_NVIM)              += sec_nvim/nv_msg_task.o
else
drv-$(CONFIG_NVIM)              += sec_nvim/nv_msg_icc.o
endif
endif #CFG_CONFIG_NV_FUSION

drv-$(CONFIG_SIM_HOTPLUG_SPMI)     += sim_hotplug/sim_hotplug.o
drv-$(CONFIG_SIM_HOTPLUG_SPMI)     += sim_hotplug/sim_hw_service.o
drv-$(CONFIG_SIM_HOTPLUG_SPMI)     += sim_hotplug/sim_hw_mgr.o

subdir-ccflags-y 	+= -I$(srctree)/drivers/platform_drivers/modem/drv/odt/core/
subdir-ccflags-y 	+= -I$(srctree)/drivers/platform_drivers/modem/drv/odt/debug/
subdir-ccflags-y 	+= -I$(srctree)/drivers/platform_drivers/modem/drv/odt/hal/common/
subdir-ccflags-y 	+= -I$(srctree)/drivers/platform_drivers/modem/drv/odt/hal/v300/

ifneq ($(strip $(CFG_CONFIG_ODT_DISABLE)),YES)
drv-y += odt/hal/v300/v300_hal.o
drv-y += odt/hal/common/hal_common.o
drv-y += odt/core/odt_enc_src.o
drv-y += odt/core/odt_enc_dst.o
drv-y += odt/core/odt_init.o
drv-y += odt/core/odt_irq_manager.o
drv-y += odt/core/odt_mode_manager.o
drv-y += odt/debug/odt_debug.o
endif

ifeq ($(strip $(CFG_CONFIG_BBPDS)),YES)
drv-y      += bbpds/bbpds.o
endif

subdir-ccflags-y    += -I$(srctree)/drivers/platform_drivers/modem/drv/sys_bus/intf
subdir-ccflags-y    += -I$(srctree)/drivers/platform_drivers/modem/drv/sys_bus/test

ifeq ($(strip $(CFG_CONFIG_SYSBUS)),YES)
drv-y += sys_bus/frame/sys_bus_core.o
drv-y += sys_bus/frame/sys_bus_cmd.o
drv-y += sys_bus/test/sys_sample.o
drv-y += sys_bus/test/sys_odt.o
drv-y += sys_bus/test/sys_acore.o
drv-y += sys_bus/test/sys_hdlc.o
endif

drv-y += sysboot/sysboot_main.o
drv-y += sysboot/sysboot_para.o
drv-y += sysctrl/sysctrl.o
drv-y += sysboot/sysboot_load_modem_teeos.o
drv-y += sysboot/sysboot_sec_call.o

drv-$(CONFIG_UDI_SUPPORT)   += udi/udi_core.o udi/adp_udi.o

drv-$(CONFIG_MODEM_VBAT)    += vbat/vbat.o

drv-y       += version/version_driver.o

ifeq ($(strip $(CFG_CONFIG_VDEV)),YES)
drv-y        += vdev/vdev_core.o vdev/vdev_app.o vdev/vdev_relay.o
drv-y        += vdev/vdev_msg.o
drv-y        += vdev/vdev_socket.o vdev/vdev_om.o
ifeq ($(strip $(CFG_CONFIG_VDEV_PHONE)),YES)
drv-y        += vdev/vdev_proc.o
endif
ifeq ($(strip $(CFG_CONFIG_MCI_ATP)),YES)
drv-y        += vdev_atp.o
endif

endif

ifeq ($(strip $(CFG_CONFIG_USB_RELAY)),YES)
drv-y += vdev_relay/bsp_relay.o vdev_relay/relay_core.o vdev_relay/relay_acm.o
else
drv-y += vdev_relay/bsp_relay_stub.o
endif

#trng_seed
ifeq ($(strip $(CFG_CONFIG_TRNG_SEED)),YES)
drv-y += trng_seed/trng_seed.o
endif


ifeq ($(strip $(CFG_CONFIG_AVS_TEST)),YES)
drv-y         += avs/avs_test.o
endif

drv-y += file_ops/bsp_file_ops.o
ifeq ($(strip $(CFG_CONFIG_DPA_OM)),YES)
drv-y           += dpa_om/diag_agent.o
else
drv-y           += dpa_om/diag_agent_stub.o
endif
# module makefile end

else
#for mbb dt and phone dt(without ko)
obj-y               += dt/

ifneq ($(strip $(CFG_ATE_VECTOR)),YES)
obj-y               += sysctrl/
ifeq ($(strip $(CFG_CONFIG_MODULE_TIMER)),YES)
obj-y               += timer/
endif
ifeq ($(strip $(CFG_CONFIG_NMI)),YES)
obj-y               += nmi/
endif

ifeq ($(strip $(CFG_CONFIG_MODULE_IPC_FUSION)),YES)
obj-y               += ipc/
endif

obj-y               += ipcmsg/

ifeq ($(strip $(CFG_CONFIG_MODEM_MSG)),YES)
obj-y               += msg/
endif

ifeq ($(strip $(CFG_CONFIG_FIPC_V200)),YES)
obj-y               += fipc/
endif

obj-y               += c_pm_stress/
obj-y               += mdmpm_debug/
obj-y               += reset/
obj-y               += sec_call/
obj-y               += rfile/
ifeq ($(strip $(CFG_FEATURE_TDS_WCDMA_DYNAMIC_LOAD)),FEATURE_ON)
obj-y               += load_ps/
endif
ifneq ($(strip $(CFG_CONFIG_ODT_DISABLE)),YES)
obj-y           += odt/
endif

ifeq ($(strip $(CFG_CONFIG_BBPDS)),YES)
obj-y           += bbpds/
endif

obj-y           += uuid/
ifeq ($(strip $(CFG_ENABLE_BUILD_OM)),YES)
obj-y               += dump/
endif
ifeq ($(strip $(CFG_BSP_CONFIG_PHONE_TYPE)),YES)
obj-y               += wdt/
endif

obj-y   += onoff/

ifeq ($(strip $(CFG_CONFIG_CCPUDEBUG)),YES)
obj-y += ccpudebug/
endif
ifeq ($(strip $(CFG_CONFIG_MODEM_CORESIGHT)),YES)
obj-y += etrace/
endif

obj-$(CONFIG_SYSVIEW) += sysview/

obj-y += dpa_om/

ifeq ($(strip $(CFG_CONFIG_MLOADER)),YES)
obj-y += mloader/
endif

obj-y += sysboot/
obj-y               += sec_nvim/
ifeq ($(strip $(CFG_FEATURE_NVA_ON)),YES)
obj-y               += nva/
endif

obj-y           += adp/
obj-y           += block/

obj-y           += version/

#trng_seed
ifeq ($(strip $(CFG_CONFIG_TRNG_SEED)),YES)
obj-y += trng_seed/
endif
#syslog
ifeq ($(strip $(CFG_ENABLE_BUILD_PRINT)),YES)
obj-y           += syslog/
endif
ifneq ($(strip $(CFG_CONFIG_HOMI_DISABLE)),YES)
obj-y               += homi/
endif

obj-$(CONFIG_OM_SOCKET)          += msocket/

ifeq ($(strip $(CFG_FEATURE_SVLSOCKET)),YES)
obj-y += svlan_socket/
obj-y += bvp_vlan/
endif

ifeq ($(strip $(CFG_FEATURE_VLAN)),YES)
obj-y += bvp_vlan/
endif

obj-y               += omserver/

ifeq ($(strip $(CFG_CONFIG_DIAG_SYSTEM)),YES)
obj-y += diag/
endif

obj-y           += chr/


ifeq ($(strip $(CFG_CONFIG_SYSBUS)),YES)
obj-y           += sys_bus/
endif

obj-y           += memory_layout/
obj-y           += memory/

ifeq ($(strip $(CFG_CONFIG_DRA)),YES)
obj-y           += dra/
endif

ifeq ($(strip $(CFG_CONFIG_SC_GID_CTRL)),YES)
obj-y           += syscache/
endif

obj-$(CONFIG_PFA)   += pfa/
ifeq ($(strip $(CFG_CONFIG_WAN)),YES)
obj-y               += pfa/wan/
else
obj-y               += pfa/wan/wan_stub.o
endif

obj-$(CONFIG_HEATING_CALIBRATION)   += heating_cal/

obj-y           += udi/

obj-y   += efuse/

obj-y   += vsim/

ifeq ($(strip $(CFG_CONFIG_CSHELL)),YES)
obj-y += console/
endif


obj-$(CONFIG_PMU_OCP) += pmu/

ifeq ($(strip $(CFG_CONFIG_LLT_MDRV)),YES)
ifneq ($(sort $(wildcard $(GBB_TOPDIR)/llt/mdrv/blt/common/llt_tools/acore)),)
subdir-ccflags-y += -DDRV_BUILD_LLT
include $(GBB_TOPDIR)/llt/mdrv/blt/common/llt_tools/acore/makefile_acore_llt_tools.mk
endif
endif

ifeq ($(strip $(CFG_CONFIG_MODULE_BUSSTRESS)),YES)
obj-y                   += busstress/$(OBB_PRODUCT_NAME)/
endif

obj-$(CONFIG_SIM_HOTPLUG_SPMI)     += sim_hotplug/

else
obj-y               += sysctrl/

ifeq ($(strip $(CFG_CONFIG_MODULE_TIMER)),YES)
obj-y               += timer/
endif

obj-y               += reset/
obj-y               += adp/
ifeq ($(strip $(CFG_ENABLE_BUILD_OM)),YES)
obj-y               += om/
endif
endif
ifeq ($(strip $(CFG_CONFIG_DLOCK)),YES)
obj-y               += dlock/
endif
obj-$(CONFIG_BBP_ACORE)             += bbp/

ifeq ($(strip $(CFG_CONFIG_DRV_LTEV)),YES)
obj-y       += pcv_adaptor/pcv_adaptor.o
endif

ifeq ($(strip $(CFG_CONFIG_VDEV)),YES)
obj-y        += vdev/
endif

obj-$(CONFIG_PCIE_MODEM_DEV)          += pcie_modem_dev/

ifeq ($(strip $(CFG_CONFIG_USB_RELAY)),YES)
obj-y += vdev_relay/
else
obj-y += vdev_relay/bsp_relay_stub.o
endif

ifeq ($(strip $(CFG_CONFIG_USB_PPP_NDIS)),YES)
obj-y += ppp_ndis_adp/
endif

ifeq ($(strip $(CFG_CONFIG_MODEM_BOOT)),YES)
obj-y += modem_boot/modem_boot_mbb.o
endif

ifeq ($(strip $(CFG_CONFIG_PCIE_NORMALIZE)),YES)
obj-y += modem_boot/modem_boot_mbb.o
endif

ifeq ($(strip $(CFG_CONFIG_PCIE_DEV_NORMALIZE)),YES)
obj-y += pcie_iot_dev/pcie_iot_dev_normalize.o
endif

ifeq ($(strip $(CFG_CONFIG_NET_OM)),YES)
obj-y += net_om/
endif

ifeq ($(strip $(CFG_CONFIG_AVS_TEST)),YES)
obj-y         += avs/
endif

ifeq ($(strip $(CFG_CONFIG_MODEM_PINCTRL)),YES)
obj-y += pinctrl/
endif

obj-y += file_ops/

obj-y += wan_eth/

obj-y += pfunc/

ifeq ($(strip $(CFG_CONFIG_AP_QIC)),YES)
drv-y   += busprobe/
endif

endif

ifeq ($(strip $(CFG_CONFIG_ECDC)),YES)
ifneq ($(sort $(wildcard $(GBB_TOPDIR)/llt/mdrv/blt/common/ecdc/acore)),)
include $(GBB_TOPDIR)/llt/mdrv/blt/common/ecdc/acore/makefile_acore_ecdc.mk
subdir-ccflags-y += -DECDC_INIT_ENABLE
endif
endif

ifeq ($(strip $(CFG_CONFIG_FUZZ_MDRV)),YES)
include $(srctree)/../../$(GBB_TOPDIR)/llt/mdrv/api_fuzz/build/makefile_acore.mk
endif
