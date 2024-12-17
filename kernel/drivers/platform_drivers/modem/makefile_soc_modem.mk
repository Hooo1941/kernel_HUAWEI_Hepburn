-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk
subdir-ccflags-y += -I$(srctree)/lib/libc_sec/securec_v2/include/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/tzdriver/libhwsecurec/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/product/$(OBB_PRODUCT_NAME)/include/platform/drv/acore/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/product/$(OBB_PRODUCT_NAME)/include/platform/drv/common/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/product/$(OBB_PRODUCT_NAME)/include/platform/pub/drv/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/product/$(OBB_PRODUCT_NAME)/include/platform/drv/ccore/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/adrv/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/drv/acore/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/include/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/product/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/acore/msp/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/common/sys/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/common/drv/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/common/msp/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/config/msg/public
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nva/comm/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/product_config/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/product/$(OBB_PRODUCT_NAME)/product/$(CUST_NAME)/define/acore/drv/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/product/$(OBB_PRODUCT_NAME)/product/$(CUST_NAME)/define/common/drv/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/product/$(OBB_PRODUCT_NAME)/product/$(CUST_NAME)/define/common/version_pub/

ifeq ($(strip $(OBB_LLT_MDRV)),y)
subdir-ccflags-y += -DDRV_BUILD_LLT
endif

ifeq ($(LLT_BUILD_GCOV), y)
subdir-ccflags-y += -DDRV_BUILD_GCOV
endif

# compile entry for phone
obj-$(CONFIG_MODEM_DRIVER) +=drv/
# compile entry for mbb
obj-$(CONFIG_MBB_MODEM) += drv/

ifneq ($(chip_type),es)
KBUILD_CFLAGS += -DCHIP_TYPE_CS=FEATURE_ON
endif

ifeq ($(secdog),rsa2048)
KBUILD_CFLAGS += -DSECDOG_SUPPORT_RSA_2048=FEATURE_ON
endif


ifeq ($(ota_netlock),true)
KBUILD_CFLAGS += -DFEATURE_OTA_NETLOCK=FEATURE_ON
endif

ifeq ($(simlock_cust),true)
KBUILD_CFLAGS += -DFEATURE_SIMLOCK_CUST=FEATURE_ON
endif

KBUILD_CFLAGS += -DMODULE_EXPORTED=

# compile entry for phone
obj-$(CONFIG_MODEM_DRIVER) +=ps/
# compile entry for mbb
obj-$(CONFIG_MBB_MODEM) += ps/

#add modem user
ifeq ($(choose_variant_modem),user)
KBUILD_CFLAGS += -DCHOOSE_MODEM_USER
endif

ifeq ($(cdma),false)
KBUILD_CFLAGS += -DFEATURE_UE_MODE_CDMA=FEATURE_OFF
KBUILD_CFLAGS += -DFEATURE_CSDR=FEATURE_OFF
endif


ifeq ($(strip $(CONFIG_MODEM_DRIVER)),m)

MODEM_DRIVER_MODULE := modem_driver
$(MODEM_DRIVER_MODULE)-y :=


# overwrite obj-m
obj-m	:= $(MODEM_DRIVER_MODULE).o

subdir-ccflags-y += -DCONFIG_MODEM_DRIVER -Wno-unused-function -Wno-undef

# drv
include $(srctree)/drivers/platform_drivers/modem/drv/Makefile
$(MODEM_DRIVER_MODULE)-y += $(addprefix drv/, $(drv-y))
obj-y	+= $(addprefix drv/, $(drv-builtin))

# ps
-include $(srctree)/drivers/platform_drivers/modem/ps/Makefile
$(MODEM_DRIVER_MODULE)-y += $(addprefix ps/, $(ps-objs))

endif
