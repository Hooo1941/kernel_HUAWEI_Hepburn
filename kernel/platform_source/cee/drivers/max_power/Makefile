LP_SOC_CHIP_NAME := $(strip $(subst ", ,$(CONFIG_VENDOR_CHIP_PREFIX)))

ccflags-$(CONFIG_LPCPU_MAX_POWER)  += -I$(srctree)/platform_source/cee/drivers/max_power
obj-$(CONFIG_LPCPU_MAX_POWER)		+= max_power_debug.o
obj-$(CONFIG_LPCPU_MAX_POWER)		+= max_power_stub.o

ccflags-$(CONFIG_LPCPU_MAX_POWER) += -I$(srctree)/platform_source/cee/drivers/max_power/$(shell echo $(LP_SOC_CHIP_NAME) | rev)
obj-$(CONFIG_LPCPU_MAX_POWER) += \
	$(shell echo $(LP_SOC_CHIP_NAME) | rev)/ananke_max_power.o \
	$(shell echo $(LP_SOC_CHIP_NAME) | rev)/ananke_max_power_wfe.o \
	$(shell echo $(LP_SOC_CHIP_NAME) | rev)/max_power.o \
	$(shell echo $(LP_SOC_CHIP_NAME) | rev)/hera_max_power.o \
	$(shell echo $(LP_SOC_CHIP_NAME) | rev)/hera_max_power_wfe.o \
	$(shell echo $(LP_SOC_CHIP_NAME) | rev)/hercules_max_power.o \
	$(shell echo $(LP_SOC_CHIP_NAME) | rev)/hercules_max_power_wfe.o \
	$(shell echo $(LP_SOC_CHIP_NAME) | rev)/deimos_max_power.o \
	$(shell echo $(LP_SOC_CHIP_NAME) | rev)/deimos_max_power_wfe.o
