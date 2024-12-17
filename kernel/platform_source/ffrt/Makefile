
obj-y += ffts/
obj-y += hts/
obj-y += uniform_id/

ifeq ($(chip_type),)
FFTS_CHIP_PLATFORM_DIR := $(TARGET_BOARD_PLATFORM)
else
FFTS_CHIP_PLATFORM_DIR := $(TARGET_BOARD_PLATFORM)_$(chip_type)
endif