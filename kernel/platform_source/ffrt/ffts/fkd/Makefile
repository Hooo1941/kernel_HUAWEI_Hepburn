EXTRA_CFLAGS += -I$(FKD_DIR)/
EXTRA_CFLAGS += -I$(FKD_DIR)/atf
EXTRA_CFLAGS += -I$(FKD_DIR)/communication
EXTRA_CFLAGS += -I$(FKD_DIR)/core
EXTRA_CFLAGS += -I$(FKD_DIR)/dfx
EXTRA_CFLAGS += -I$(FKD_DIR)/dts
EXTRA_CFLAGS += -I$(FKD_DIR)/inc
EXTRA_CFLAGS += -I$(FKD_DIR)/memory
EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/ap/platform/$(FFTS_CHIP_PLATFORM_DIR)
EXTRA_CFLAGS += -I$(UNIFORM_ID_DIR)/
EXTRA_CFLAGS += -I$(FPD_DIR)/

obj-y += fkd.o

fkd-objs := ffts_common.o ffts_devinit.o ffts_platform.o atf/ffts_atf.o communication/ffts_interrupt.o core/ffts_ioctl_services.o \
	core/ffts_tid.o core/ffts_proc_ctx.o dts/ffts_irq.o dts/ffts_reg.o memory/ffts_map.o