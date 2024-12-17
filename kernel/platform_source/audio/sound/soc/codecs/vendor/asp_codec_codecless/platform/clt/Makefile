GCOV_PROFILE := $(CONFIG_GCOV_KERNEL)
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/codecs/vendor/
EXTRA_CFLAGS += -I$(srctree)/include/linux/platform_drivers/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/platforms/common_image/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/include/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/soc_mad/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/codecs/vendor/asp_codec_codecless/platform/clt/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/codecs/vendor/asp_codec_codecless/base/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/codecs/vendor/asp_codec_codecless/store/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/platforms/pcm_codec/

obj-y += asp_codec_driver.o
ifeq ($(CONFIG_SND_SOC_ARMPC_CUSTOM),y)
obj-y += asp_codec_path_pc.o
obj-y += ../../store/asp_codec_hdmi_adapter.o
else
obj-y += asp_codec_path.o

obj-y += ../../store/asp_codec_play_path.o
obj-y += ../../store/asp_codec_btfm_path.o
obj-y += ../../store/asp_codec_uplink_path.o
obj-y += ../../store/asp_codec_sidetone_path.o
obj-y += ../../store/asp_codec_axi2tdm_path.o
obj-y += ../../store/asp_codec_mad_path.o
endif