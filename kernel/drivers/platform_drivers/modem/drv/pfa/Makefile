# pfa module
-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk
KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/dbg/
KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/hal/common/
KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/core/
KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/tft/
KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/entry/
KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/app_if/
KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/modem_reset/
KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/wan/
ifeq ($(strip $(CFG_CONFIG_PFA_V200)),YES)
KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/hal/v200/
else ifeq ($(strip $(CFG_CONFIG_PFA_V100)),YES)
KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/hal/v100/
else
KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/hal/v0/
endif
obj-$(CONFIG_PFA)              += pfa_objs.o

ifeq ($(strip $(CFG_CONFIG_PFA_V200)),YES)
pfa_objs-y						+= hal/v200/pfa_hal_version.o
else ifeq ($(strip $(CFG_CONFIG_PFA_V100)),YES)
pfa_objs-y						+= hal/v100/pfa_hal_version.o
else
pfa_objs-y						+= hal/v0/pfa_hal_version.o
endif
pfa_objs-y						+= hal/common/pfa_hal.o
pfa_objs-y						+= core/pfa_core.o
pfa_objs-y						+= core/pfa_desc.o
pfa_objs-y						+= core/pfa_interrupt.o
pfa_objs-y						+= core/pfa_platform.o
pfa_objs-y						+= core/pfa_port.o
pfa_objs-y						+= core/pfa_adp.o

pfa_objs-y						+= dbg/pfa_dbg.o

pfa_objs-y						+= entry/pfa_entry_hook.o
pfa_objs-y						+= entry/pfa_ip_entry.o
pfa_objs-y						+= entry/pfa_mac_entry.o
ifeq ($(strip $(CFG_CONFIG_PFA_DIRECT_FW)),YES)
pfa_objs-y						+= entry/pfa_direct_fw.o
endif

pfa_objs-y						+= app_if/pfa_sysfs.o
pfa_objs-y						+= app_if/pfa_sysfs_main.o
ifeq ($(strip $(CFG_CONFIG_PFA_V200)),YES)
pfa_objs-y						+= app_if/pfa_sysfs_pdu.o
pfa_objs-y						+= app_if/pfa_sysfs_ip_qos_stub.o
else ifeq ($(strip $(CFG_CONFIG_PFA_V100)),YES)
pfa_objs-y						+= app_if/pfa_sysfs_pdu.o
pfa_objs-y						+= app_if/pfa_sysfs_ip_qos.o
else
pfa_objs-y						+= app_if/pfa_sysfs_pdu_stub.o
endif

pfa_objs-y						+= modem_reset/pfa_modem_reset.o

ifeq ($(strip $(CFG_CONFIG_PFA_V200)),YES)
pfa_objs-y						+= tft/pfa_tft_ap_stub.o
pfa_objs-y						+= core/pfa_img.o
pfa_objs-y						+= core/pfa_load.o
else
pfa_objs-y						+= tft/pfa_tft_ap.o
pfa_objs-y						+= core/pfa_img_stub.o
endif

ifeq ($(strip $(CFG_CONFIG_PFA_FW)),YES)
pfa_objs-y						+= core/pfa_kernel_hook.o
endif

subdir-ccflags-y := -Werror
GCOV_PROFILE := y