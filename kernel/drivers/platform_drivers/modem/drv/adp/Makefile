-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk
KBUILD_CPPFLAGS += -Idrivers/platform_drivers/modem/drv/icc
KBUILD_CPPFLAGS += -Idrivers/platform_drivers/modem/drv/rtc
KBUILD_CPPFLAGS += -Idrivers/platform_drivers/modem/drv/nvim
KBUILD_CPPFLAGS += -Idrivers/platform_drivers/modem/drv/mem
KBUILD_CPPFLAGS += -Idrivers/platform_drivers/modem/drv/om \
                   -Idrivers/platform_drivers/modem/drv/om/common \
                   -Idrivers/platform_drivers/modem/drv/om/dump \
                   -Idrivers/platform_drivers/modem/drv/om/log
KBUILD_CPPFLAGS += -Idrivers/platform_drivers/modem/drv/udi
KBUILD_CPPFLAGS += -Idrivers/platform_drivers/modem/drv/timer
KBUILD_CPPFLAGS += -Idrivers/usb/gadget
KBUILD_CPPFLAGS += -Idrivers/platform_drivers/modem/drv/diag/scm \
                   -Idrivers/platform_drivers/modem/drv/diag/cpm \
                   -Idrivers/platform_drivers/modem/drv/diag/debug \

ifneq ($(strip $(CFG_ATE_VECTOR)),YES)
obj-y           += adp_version.o
obj-y			+= adp_om.o
obj-$(CONFIG_USB_GADGET)     += adp_usb.o
#obj-y           += adp_nvim.o
obj-y           += adp_timer.o
obj-y           += adp_wifi.o
ifeq ($(strip $(CFG_CONFIG_MODEM_CORESIGHT)),YES)
ifeq ($(strip $(CFG_CONFIG_OCD_V200)),YES)
obj-y           += adp_coresight_v1.o
else
obj-y           += adp_coresight.o
endif
endif
obj-y           += adp_charger.o
#obj-y           += adp_misc.o
else
obj-y           += adp_reset.o
obj-y           += adp_timer.o
endif

obj-y           += adp_socket.o


obj-y           += adp_print.o
obj-y           += adp_reset.o

GCOV_PROFILE := y
GCOV_PROFILE_adp_socket.o := n
GCOV_PROFILE_adp_coresight.o := n
GCOV_PROFILE_adp_wifi.o := n
GCOV_PROFILE_adp_reset.o := n
GCOV_PROFILE_adp_charger.o := n