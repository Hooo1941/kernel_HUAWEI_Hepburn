subdir-ccflags-y := -Werror

obj-y += ppp_ndis_adp.o
ifeq ($(strip $(CONFIG_USB_GADGET)),y)
obj-y += ppp_ndis_net.o
obj-y += ppp_ndis_address.o
endif

GCOV_PROFILE := y