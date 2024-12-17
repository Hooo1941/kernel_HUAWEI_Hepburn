GCOV_PROFILE := $(CONFIG_GCOV_KERNEL)
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/da_separate/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/codecs/vendor/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/codecs/vendor/legacy/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/codecs/vendor/legacy/da_separate/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/codecs/vendor/adapter/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/platforms/common_image/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/include/
EXTRA_CFLAGS += -I$(srctree)/include/platform_include/audio/

snd-soc-da_separate_v3-objs :=da_separate_v3.o
obj-$(CONFIG_SND_SOC_CODEC_DA_SEPARATE_V3) += snd-soc-da_separate_v3.o
