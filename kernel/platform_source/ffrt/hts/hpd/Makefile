
EXTRA_CFLAGS += -I$(srctree)/platform_source/ffrt/hts/dfx
EXTRA_CFLAGS += -I$(srctree)/platform_source/ffrt/hts/hkd
EXTRA_CFLAGS += -I$(srctree)/platform_source/ffrt/hts/hkd/atf
EXTRA_CFLAGS += -I$(srctree)/platform_source/ffrt/hts/hkd/communication
EXTRA_CFLAGS += -I$(srctree)/platform_source/ffrt/hts/hkd/core
EXTRA_CFLAGS += -I$(srctree)/platform_source/ffrt/hts/hkd/dts
EXTRA_CFLAGS += -I$(srctree)/platform_source/ffrt/hts/hkd/inc
EXTRA_CFLAGS += -I$(srctree)/platform_source/ffrt/hts/hkd/interfaces
EXTRA_CFLAGS += -I$(srctree)/platform_source/ffrt/hts/hkd/power_manager
EXTRA_CFLAGS += -I$(srctree)/platform_source/ffrt/hts/hpd
EXTRA_CFLAGS += -I$(srctree)/platform_source/ffrt/hts/hpd/$(ARCH_VERSION)/
EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/ap/platform/$(FFTS_CHIP_PLATFORM_DIR)
EXTRA_CFLAGS += -I$(srctree)/platform_source/ffrt/uniform_id
obj-y += hpd.o

hpd-objs := hpd_main.o
hpd-objs +=$(ARCH_VERSION)/hts_rule.o $(ARCH_VERSION)/hts_cfg.o $(ARCH_VERSION)/hts_arch_exception.o $(ARCH_VERSION)/hts_resmem.o
