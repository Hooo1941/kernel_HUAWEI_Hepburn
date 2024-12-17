GCOV_PROFILE := $(CONFIG_GCOV_KERNEL)
ifeq ($(CONFIG_RDA_AUDIO_DRIVERS),y)
EXTRA_CFLAGS += -I$(srctree)/vendor/drivers/audio/hifi_mailbox/mailbox/
EXTRA_CFLAGS += -I$(srctree)/vendor/drivers/audio/include/
else
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/hifi_mailbox/mailbox/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/platforms/common_image/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/include/
endif
obj-$(CONFIG_HIFI_IPC)                                 := bsp_ipc.o
