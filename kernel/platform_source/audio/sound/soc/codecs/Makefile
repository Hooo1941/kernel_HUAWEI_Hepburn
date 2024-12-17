subdir-ccflags-y += -Wdate-time -Wfloat-equal -Wformat=2 -Wextra -Wshadow

EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/codecs/vendor/hs_auto_calib/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/da_combine_dsp/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/codec_bus/slimbus/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/codec_bus/soundwire/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/da_separate/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/platforms/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/codecs/vendor/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/codecs/vendor/legacy/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/codecs/vendor/adapter/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/platforms/common_image/

ifeq ($(CONFIG_AUDIO_DEBUG),y)
endif

snd-soc-hdmi-audio-objs := vendor/hdmi-audio.o
snd-soc-dp-audio-objs := vendor/dp_audio.o
obj-$(CONFIG_HUAWEI_AUDIO) += vendor/hs_auto_calib/

ccflags-y  += -I$(srctree)/drivers/devkit/audiokit/
ifneq ($(TARGET_BUILD_VARIANT),user)
	EXTRA_CFLAGS += -I$(srctree)/drivers/devkit/audiokit/headset_debug
	EXTRA_CFLAGS += -DCONFIG_HUAWEI_HEADSET_DEBUG
endif

ifneq ($(strip $(CONFIG_SWITCH)),)
ccflags-y += -DCONFIG_HUAWEI_HEADSET_DEBUG_SWITCH
endif

obj-$(CONFIG_SND_SOC_DA_COMBINE_V3) += vendor/legacy/da_combine_v3/
obj-$(CONFIG_SND_SOC_DA_COMBINE_V5) += vendor/legacy/da_combine_v5/
obj-$(CONFIG_SND_SOC_CODEC_DA_SEPARATE_V2) += vendor/legacy/da_separate/da_separate_v2/
obj-$(CONFIG_HDMI_K3) += snd-soc-hdmi-audio.o
obj-$(CONFIG_SND_SOC_DP_AUDIO)    += snd-soc-dp-audio.o

obj-$(CONFIG_SND_SOC_AU_PA) += vendor/au_pa/

obj-y += vendor/adapter/asoc_adapter.o
obj-$(CONFIG_SND_SOC_CODEC_DEBUG) += vendor/codec_debug.o

obj-$(CONFIG_SND_SOC_ASP_CODEC) += vendor/legacy/asp_codec/
obj-$(CONFIG_SND_SOC_ASP_CODEC_CODECLESS) += vendor/asp_codec_codecless/
obj-$(CONFIG_SND_SOC) += vendor/legacy/da_separate/

ifeq ($(product_type),cdc_ace)
endif
