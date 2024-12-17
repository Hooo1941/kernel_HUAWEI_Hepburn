mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
mkfile_dir := $(dir $(mkfile_path))

ifeq ($(subst ",,$(CONFIG_CONNECTIVITY_HI116X)),)
export _PRE_PRODUCT_VERSION ?= 1105
else
export _PRE_PRODUCT_VERSION ?= gf61
endif
$(warning plat _PRE_PRODUCT_VERSION $(_PRE_PRODUCT_VERSION))

ifeq ($(_PRE_PRODUCT_VERSION), gf61)
include $(mkfile_dir)/gfxx.mk
else
include $(mkfile_dir)/mpxx.mk
endif
