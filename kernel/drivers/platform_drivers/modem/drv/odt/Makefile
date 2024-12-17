
-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk
KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/odt/core/
KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/odt/debug/
KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/odt/hal/common/
KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/odt/hal/v300/

obj-y += hal/v300/v300_hal.o
obj-y += hal/common/hal_common.o
obj-y += core/odt_enc_src.o
obj-y += core/odt_enc_dst.o
obj-y += core/odt_init.o
obj-y += core/odt_irq_manager.o
obj-y += core/odt_mode_manager.o
obj-y += debug/odt_debug.o

subdir-ccflags-y := -Werror

GCOV_PROFILE := y