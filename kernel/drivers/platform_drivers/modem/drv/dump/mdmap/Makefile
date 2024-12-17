#
# Makefile for the memory technology device drivers.
#
# Core functionality.
-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk
ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif
ifeq ($(strip $(CFG_BSP_CONFIG_PHONE_TYPE)),YES)
obj-y				+= dump_baseinfo.o
endif
obj-y				+= dump_eproc.o
ifeq ($(strip $(CFG_BSP_CONFIG_PHONE_TYPE)),YES)
obj-y				+= dump_phone_eproc.o
else
obj-y				+= dump_iot_eproc.o
endif
obj-y				+= dump_lastkmsg.o
obj-y				+= dump_area.o 
obj-y				+= dump_notifer.o
obj-y                           += dump_ko.o
obj-y                           += dump_field_strategy.o

GCOV_PROFILE := y
GCOV_PROFILE_dump_ko.o := n
GCOV_PROFILE_dump_iot_eproc.o := n
GCOV_PROFILE_dump_phone_eproc.o := n
GCOV_PROFILE_dump_field_strategy.o := n
GCOV_PROFILE_dump_lastkmsg.o := n
GCOV_PROFILE_dump_notifer.o := n
GCOV_PROFILE_dump_area.o := n
GCOV_PROFILE_dump_baseinfo.o := n
GCOV_PROFILE_dump_eproc.o := n