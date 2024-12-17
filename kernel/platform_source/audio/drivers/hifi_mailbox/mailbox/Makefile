GCOV_PROFILE := $(CONFIG_GCOV_KERNEL)

EXTRA_CFLAGS += -DBSP_CORE_APP
ifdef CONFIG_HIFI_RESET
EXTRA_CFLAGS += -D_HIFI_WD_DEBUG
endif

ifeq ($(CONFIG_AUDIO_DEBUG),y)
EXTRA_CFLAGS += -DENABLE_MAILBOX_DEBUG
endif
ifeq ($(CONFIG_RDA_AUDIO_DRIVERS),y)
EXTRA_CFLAGS += -I$(srctree)/vendor/drivers/audio/hifi_mailbox/ipcm
EXTRA_CFLAGS += -I$(srctree)/vendor/drivers/audio/hifi_mailbox/mailbox
EXTRA_CFLAGS += -I$(srctree)/vendor/include/platform_include/audio/ipc
EXTRA_CFLAGS += -I$(srctree)/vendor/drivers/audio/include
else
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/hifi_mailbox/ipcm
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/hifi_mailbox/mailbox
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/platforms/common_image/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/hifi_dsp
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/hifi_dsp/om
EXTRA_CFLAGS += -I$(srctree)/include/platform_include/audio/ipc
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/include
endif

obj-$(CONFIG_HIFI_MAILBOX)	+= drv_mailbox_gut.o
obj-$(CONFIG_HIFI_MAILBOX)	+= drv_mailbox_msg.o
obj-$(CONFIG_HIFI_MAILBOX)	+= drv_mailbox_debug.o
obj-$(CONFIG_HIFI_MAILBOX)	+= drv_mailbox_port_linux.o
obj-$(CONFIG_HIFI_MAILBOX)	+= drv_mailbox.o