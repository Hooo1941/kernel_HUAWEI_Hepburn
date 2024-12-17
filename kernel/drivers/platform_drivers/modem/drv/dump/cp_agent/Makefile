
#
# Makefile for the memory technology device drivers.
#
# Core functionality.
-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk
ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

obj-y				+= dump_agent.o 
ifeq ($(strip $(CFG_BSP_CONFIG_PHONE_TYPE)),YES)
obj-y				+= dump_phone_agent.o
else
obj-y				+= dump_iot_agent.o
endif
obj-y				+= dump_eproc.o 
obj-y				+= dump_wdt.o 
obj-y				+= dump_log.o 


GCOV_PROFILE := y
GCOV_PROFILE_dump_phone_agent.o := n
GCOV_PROFILE_dump_log.o := n
GCOV_PROFILE_dump_agent.o := n
GCOV_PROFILE_dump_wdt.o := n
GCOV_PROFILE_dump_iot_agent.o := n
GCOV_PROFILE_dump_eproc.o := n