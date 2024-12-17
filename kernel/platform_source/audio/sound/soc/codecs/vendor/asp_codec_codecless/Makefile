GCOV_PROFILE := $(CONFIG_GCOV_KERNEL)
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/codecs/vendor/
EXTRA_CFLAGS += -I$(srctree)/include/linux/platform_drivers/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/platforms/common_image/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/include/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/soc_mad/
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/codecs/vendor/asp_codec_codecless/base/

ifeq ($(CONFIG_BALT_IP_PLATFORM),y)
endif

ifeq ($(CONFIG_CHAR_IP_PLATFORM),y)
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/codecs/vendor/asp_codec_codecless/platform/clt/
obj-y += platform/clt/
endif

ifeq ($(CONFIG_CHARPRO_IP_PLATFORM),y)
endif

ifeq ($(CONFIG_NA_IP_PLATFORM),y)
endif

ifeq ($(CONFIG_NSV_IP_PLATFORM),y)
endif

ifeq ($(CONFIG_XA_IP_PLATFORM),y)
endif

EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/sound/soc/codecs/vendor/asp_codec_codecless/store/

snd-soc-asp-codec-base-objs := base/asp_codec_utils.o base/asp_codec_control.o store/asp_codec_store.o

obj-y += snd-soc-asp-codec-base.o