LP_SOC_CHIP_NAME := $(strip $(subst ", ,$(CONFIG_VENDOR_CHIP_PREFIX)))

ccflags-y += -Wall -Werror
ccfags-y += -I$(srctree)/drivers/platform_drivers/ap/platform/$(LP_SOC_CHIP_NAME)

EXTRA_CFLAGS += -Idrivers/platform_drivers/tzdriver

obj-$(CONFIG_LPMCU_BB) += rdr_lpm3.o


