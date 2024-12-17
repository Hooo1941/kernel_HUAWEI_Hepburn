GCOV_PROFILE := $(CONFIG_GCOV_KERNEL)
subdir-ccflags-y += -Wall -Werror

ccflags-y += -I$(srctree)/drivers/devkit/audiokit/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/codec_bus/
EXTRA_CFLAGS += -I$(srctree)/include/platform_include/audio/platform_drivers/codec_bus/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/hifi_dsp/
EXTRA_CFLAGS += -I$(srctree)/include/platform_include/audio/platform_drivers/da_combine_dsp/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/rdr/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/soundtrigger/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/codecs/vendor/fpga_test/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/codecs/vendor/adapter/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/da_combine_dsp/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/da_combine_dsp/om/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/da_combine_dsp/comm/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/da_combine_dsp/config/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/da_combine_dsp/download/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/platforms/common_image/
EXTRA_CFLAGS += -I$(srctree)/include/platform_include/audio/ipc
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/include/

ifeq ($(CONFIG_AUDIO_DEBUG),y)
ifeq ($(CONFIG_KCOV),y)
EXTRA_CFLAGS += -DENABLE_AUDIO_KCOV
else
EXTRA_CFLAGS += -DENABLE_DA_COMBINE_HIFI_DEBUG
endif
endif

obj-$(CONFIG_CODEC_DSP_MISC) += da_combine_dsp_misc.o
obj-$(CONFIG_CODEC_DSP_MISC) += om/
obj-$(CONFIG_CODEC_DSP_MISC) += comm/
obj-$(CONFIG_CODEC_DSP_MISC) += config/
obj-$(CONFIG_CODEC_DSP_MISC) += download/
