# virtual voice proxy driver
obj-$(CONFIG_VIRTUAL_VOICE_PROXY)		+= virtual_voice_proxy.o
obj-$(CONFIG_VIRTUAL_VOICE_PROXY_EXTDEV)		+= virtual_voice_proxy_extdev.o

ifeq ($(CONFIG_HIFI_EXTRA_MODEM_MODE),y)
EXTRA_CFLAGS += -DEXTERNAL_MODEM
endif

EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/hifi_mailbox/mailbox \
		-I$(srctree)/platform_source/audio/drivers/hifi_mailbox/ipcm \
		-I$(srctree)/platform_source/audio/drivers/hifi_dsp \
		-I$(srctree)/platform_source/audio/drivers/hifi_dsp/om \
		-I$(srctree)/platform_source/audio/include \
		-I$(srctree)/include/platform_include/audio/ipc
