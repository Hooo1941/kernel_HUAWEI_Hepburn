ccflags-y += -Wdate-time -Wfloat-equal -Wformat=2

obj-y += rdr_audio_adapter.o
obj-y += rdr_audio_dump_socdsp.o

ifeq ($(CONFIG_HIFI_DSP_ONE_TRACK),y)
obj-y += rdr_audio_soc.o
EXTRA_CFLAGS += -DRDR_SOCDSP
endif

obj-y += rdr_audio_codec.o
obj-y += rdr_audio_log.o
EXTRA_CFLAGS += -DRDR_CODECDSP

ifeq ($(CONFIG_HIFI_EXTRA_MODEM_MODE),y)
obj-y += rdr_audio_notify_modem.o
EXTRA_CFLAGS += -DEXTERNAL_MODEM
EXTRA_CFLAGS += -DHIFI_RESET_CDEV
endif

ifeq ($(CONFIG_HIFI_RESET_CDEV),y)
obj-y += rdr_audio_notify_modem.o
EXTRA_CFLAGS += -DHIFI_RESET_CDEV
endif

EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/platforms/common_image/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/hifi_dsp/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/hifi_dsp/om/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/da_combine_dsp/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/da_combine_dsp/om/
EXTRA_CFLAGS += -I$(srctree)/platform_source/basicplatform/drivers/mntn/blackbox/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/usb/hifi/
EXTRA_CFLAGS += -I$(srctree)/include/platform_include/audio/ipc
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/include
ccflags-y += -I$(srctree)/drivers/platform_drivers/tzdriver
ccflags-y += -I$(srctree)/drivers/devkit/audiokit/