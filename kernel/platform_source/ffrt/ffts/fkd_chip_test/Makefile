
EXTRA_CFLAGS += -I$(srctree)/platform_source/ffrt/ffts/dfx
EXTRA_CFLAGS += -I$(srctree)/platform_source/ffrt/ffts/fkd_chip_test
EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/ap/platform/$(FFTS_CHIP_PLATFORM_DIR)
EXTRA_CFLAGS += -DFFTS_PROC_BLOCK_HW
EXTRA_CFLAGS += -DFFTS_IO_COHERENCY

obj-y += fkd_chip_test.o

fkd_chip_test-objs := ffts_devinit.o ffts_ioctl_services.o ffts_map.o ffts_proc_ctx.o ffts_thread_ctx.o ffts_uid.o ffts_tid.o ffts_hw_config.o ffts_atf.o ffts_pm.o ffts_irq.o
