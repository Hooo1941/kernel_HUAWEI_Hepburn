#
# Makefile for the memory technology device drivers.
#

# Core functionality.
-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk


ifeq ($(strip $(CFG_ENABLE_BUILD_OM)),YES)
obj-y               += logs/
obj-y               += mdmap/
obj-y               += cp_agent/
ifeq ($(strip $(CFG_ENABLE_BUILD_DUMP_MDM_LPM3)),YES)
obj-y               += m3_agent/
endif
ifeq ($(strip $(CFG_CONFIG_DUMP_PLATFORM_V100)),YES)
obj-y               += hal/v100/
KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/dump/hal/v100
endif
ifeq ($(strip $(CFG_CONFIG_DUMP_PLATFORM_V200)),YES)
obj-y               += hal/v200/
KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/dump/hal/v200
endif
obj-y += dump_core.o dump_frame.o dump_config.o dump_sharemem.o
ifeq ($(strip $(CFG_BSP_CONFIG_PHONE_TYPE)),YES)
#only conpile in phone vmlinux, not ko
obj-y += dump_boot_check.o
endif
endif

KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/dump\
                   -I$(srctree)/drivers/platform_drivers/modem/drv/dump/mdmap\
                   -I$(srctree)/drivers/platform_drivers/modem/drv/dump/logs\
                   -I$(srctree)/drivers/platform_drivers/modem/include/adrv

GCOV_PROFILE := y
GCOV_PROFILE_dump_frame.o := n
GCOV_PROFILE_dump_config.o := n
GCOV_PROFILE_dump_boot_check.o := n
GCOV_PROFILE_dump_core.o := n
GCOV_PROFILE_dump_sharemem.o := n
GCOV_PROFILE_dump_ko.o := n
GCOV_PROFILE_dump_iot_eproc.o := n
GCOV_PROFILE_dump_phone_eproc.o := n
GCOV_PROFILE_dump_field_strategy.o := n
GCOV_PROFILE_dump_lastkmsg.o := n
GCOV_PROFILE_dump_notifer.o := n
GCOV_PROFILE_dump_area.o := n
GCOV_PROFILE_dump_baseinfo.o := n
GCOV_PROFILE_dump_eproc.o := n
GCOV_PROFILE_dump_exc_table.o := n
GCOV_PROFILE_dump_agent.o := n
GCOV_PROFILE_dump_eproc.o := n
GCOV_PROFILE_dump_phone_agent.o := n
GCOV_PROFILE_dump_log.o := n
GCOV_PROFILE_dump_agent.o := n
GCOV_PROFILE_dump_wdt.o := n
GCOV_PROFILE_dump_iot_agent.o := n
GCOV_PROFILE_dump_eproc.o := n
GCOV_PROFILE_dump_log_uploader.o := n
GCOV_PROFILE_dump_teeos_service.o := n
GCOV_PROFILE_dump_log_strategy.o := n
GCOV_PROFILE_dump_reset_log.o := n
GCOV_PROFILE_dump_genl.o := n
GCOV_PROFILE_dump_logs.o := n
GCOV_PROFILE_dump_sec_log.o := n
GCOV_PROFILE_dump_log_parser.o := n