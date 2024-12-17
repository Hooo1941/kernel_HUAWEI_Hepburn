ifeq ($(CONFIG_LIBLINUX),y)
ccflags-y += -Wall
else
ccflags-y += -Wall -Werror=
endif
EXTRA_CFLAGS += -I$(srctree)/platform_source/basicplatform/drivers/mntn/blackbox/
EXTRA_CFLAGS += -I$(srctree)/platform_source/see/drivers/load_image/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/hifi_mailbox/mailbox/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/hifi_dsp/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/rdr/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/platforms/common_image/
EXTRA_CFLAGS += -I$(srctree)/include/platform_include/audio/ipc
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/include
obj-$(CONFIG_AP_CP_DECOUPL) += modem_symbol.o

