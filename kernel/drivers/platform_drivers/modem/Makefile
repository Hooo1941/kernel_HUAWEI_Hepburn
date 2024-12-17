# for ohos
ifeq ($(strip $(CONFIG_HISI_MODEM_OHOS)),y)
OBB_PRODUCT_NAME ?= $(platform)
endif

-include $(srctree)/drivers/platform_drivers/modem/modem_env.mak
include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk
ifeq ($(strip $(MODEM_FULL_DUMP)),true)
subdir-ccflags-y += -DMODEM_FULL_DUMP_INUSER
endif

ifeq ($(strip $(MODEM_DDR_MINI_DUMP)),true)
subdir-ccflags-y += -DMODEM_DDR_MINI_DUMP_INUSER
endif

ifeq ($(strip $(MODEM_LOGCAT_INUSER)),true)
subdir-ccflags-y += -DMODEM_LOGCAT_INUSER
endif

ifeq ($(strip $(modem_log_filter_nv_control)),true)
subdir-ccflags-y += -DMODEM_LOG_FILTER_INUSER
endif

ifeq ($(strip $(MODEM_CHR_INUSER)),true)
subdir-ccflags-y += -DMODEM_CHR_INUSER
endif

ifeq ($(strip $(MODEM_PM_OM_INUSER)),true)
subdir-ccflags-y += -DMODEM_PM_OM_INUSER
endif

ifeq ($(strip $(CONFIG_EXTRA_MODEM_DRIVER)),y)
include $(srctree)/drivers/platform_drivers/modem/makefile_extra_modem.mk

else
include $(srctree)/drivers/platform_drivers/modem/makefile_soc_modem.mk

endif


