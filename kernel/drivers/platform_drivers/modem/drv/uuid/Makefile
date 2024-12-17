ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk

#if not exist GBB_MODEM_IMAGEID, gen random uuid
#CUR_MODEM_UUID := $(shell cat /proc/sys/kernel/random/uuid)

KBUILD_CFLAGS += -DCURRENT_MODEM_IMAGE_ID=\"$(GBB_MODEM_IMAGEID)\"
obj-y += uuid_modem.o

GCOV_PROFILE := y
