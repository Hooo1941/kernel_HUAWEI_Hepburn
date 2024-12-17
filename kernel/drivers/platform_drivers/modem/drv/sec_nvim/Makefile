
-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk

KBUILD_CPPFLAGS += -Idrivers/platform_drivers/modem/drv/om/dump \
                   -Idrivers/platform_drivers/modem/include/adrv

subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/product/

ifeq ($(strip $(CFG_CONFIG_NV_FUSION)), YES)
obj-y += fusion/nv_base.o \
         fusion/nv_debug.o \
         fusion/nv_msg.o \
         fusion/nv_msg_req.o \
         fusion/nv_msg_fipc.o \
         fusion/nv_partition.o \
         fusion/nv_partition_image.o \
         fusion/nv_partition_backup.o \
         fusion/nv_partition_factory.o \
         fusion/nv_upgrade.o
endif

subdir-ccflags-y := -Werror


GCOV_PROFILE := y