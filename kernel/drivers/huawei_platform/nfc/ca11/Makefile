#
# Makefile for nfc devices
#
CONFIG_PRIV_NFC_KERNEL_BUILD_TYPE ?= y
CONFIG_PRIV_NFC_ADAPTER_PLAT ?= q
CONFIG_PRIV_NFC_DEBUG_ENABLE ?= disable

ifneq ($(TARGET_BUILD_VARIANT),user)
CONFIG_PRIV_NFC_DEBUG_ENABLE = enable
endif

MK_FILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
PRIV_NFC_DRIVER_BUILTIN_DIR := $(dir $(MK_FILE_PATH))

ifeq ($(CONFIG_PRIV_NFC_ADAPTER_PLAT), kbd)
COMM_DRIVER_PATH=comm_driver
PRODUCT_PATH=adapter/product/keyboard
OAL_PATH=adapter/oal/liteos_kbd
else
COMM_DRIVER_PATH=comm_driver
PRODUCT_PATH=adapter/product/phone
OAL_PATH=adapter/oal/linux
endif

PRIV_NFC_INCLUDES := -I$(PRIV_NFC_DRIVER_BUILTIN_DIR)$(COMM_DRIVER_PATH)
PRIV_NFC_INCLUDES += -I$(PRIV_NFC_DRIVER_BUILTIN_DIR)$(PRODUCT_PATH)
PRIV_NFC_INCLUDES += -I$(PRIV_NFC_DRIVER_BUILTIN_DIR)$(OAL_PATH)

ifeq ($(CONFIG_PRIV_NFC_DEBUG_ENABLE),enable)
KBUILD_CFLAGS += -DPLATFORM_DEBUG_ENABLE
endif

comm_driver-objs := priv_nfc_driver.o
ifeq ($(CONFIG_PRIV_NFC_DEBUG_ENABLE),enable)
comm_driver-objs += priv_nfc_test.o priv_nfc_ssi.o
endif
comm_driver-objs := $(addprefix $(COMM_DRIVER_PATH)/,$(comm_driver-objs))

ifeq ($(CONFIG_PRIV_NFC_ADAPTER_PLAT), udp)
KBUILD_CFLAGS += -DNFC_PLATFORM_HISI
product-objs := nfc_dev.o board_hisi.o
ifeq ($(CONFIG_PRIV_NFC_DEBUG_ENABLE),enable)
product-objs += nfc_dev_test.o
endif
else ifeq ($(CONFIG_PRIV_NFC_ADAPTER_PLAT), q)
KBUILD_CFLAGS += -DNFC_PLATFORM_Q
product-objs := nfc_dev_q.o board_q.o nfc_dev_attr_q.o
ifeq ($(CONFIG_PRIV_NFC_DEBUG_ENABLE),enable)
product-objs += nfc_dev_test_q.o
endif
else ifeq ($(CONFIG_PRIV_NFC_ADAPTER_PLAT), hisi)
KBUILD_CFLAGS += -DNFC_PLATFORM_HISI
ifeq ($(CONFIG_CA11_NFC), y)
PRIV_NFC_INCLUDES += -I$(srctree)/drivers/platform_drivers/tzdriver
product-objs := nfc_dev_q.o board_ca11.o nfc_dev_attr_q.o
else
product-objs := nfc_dev_q.o board_q.o nfc_dev_attr_q.o
endif
ifeq ($(CONFIG_PRIV_NFC_DEBUG_ENABLE),enable)
product-objs += nfc_dev_test_q.o
endif
else ifeq ($(CONFIG_PRIV_NFC_ADAPTER_PLAT), kbd)
product-objs := nfc_dev.o board_kbd.o fw_cfg.o fw_ram.o fw_rom_data.o
else
$(error CONFIG_PRIV_NFC_ADAPTER_PLAT = $(CONFIG_PRIV_NFC_ADAPTER_PLAT))
endif
product-objs := $(addprefix $(PRODUCT_PATH)/,$(product-objs))

priv_nfc-objs := $(comm_driver-objs)
priv_nfc-objs += $(product-objs)

obj-$(CONFIG_PRIV_NFC_KERNEL_BUILD_TYPE)		+= $(priv_nfc-objs)
EXTRA_CFLAGS = $(PRIV_NFC_INCLUDES)
EXTRA_CFLAGS += $(KBUILD_CFLAGS)

ifeq ($(CONFIG_PRIV_NFC_ADAPTER_PLAT), kbd)
PRIV_NFC_SRC := $(addprefix $(PRIV_NFC_DRIVER_BUILTIN_DIR),$(patsubst %.o,%.c,$(priv_nfc-objs)))
NFC_SRC += $(PRIV_NFC_SRC)
C_INCLUDES += $(PRIV_NFC_INCLUDES)
C_DEFS += $(KBUILD_CFLAGS)
endif