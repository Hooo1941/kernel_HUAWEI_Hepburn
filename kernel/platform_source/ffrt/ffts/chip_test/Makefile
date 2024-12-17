#EXTRA_CFLAGS += $(NPU_GLOBAL_CFLAGS)
EXTRA_CFLAGS += -I$(srctree)/platform_source/ffrt/ffts/dfx
EXTRA_CFLAGS += -I$(srctree)/platform_source/ffrt/ffts/fkd
EXTRA_CFLAGS += -I$(srctree)/platform_source/ffrt/ffts/chip_test
EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/ap/platform/$(FFTS_CHIP_PLATFORM_DIR)

obj-y += ffts_chip_test.o
ffts_chip_test-objs := ffts_chip_test_devinit.o ffts_chip_test_ioctl_services.o
