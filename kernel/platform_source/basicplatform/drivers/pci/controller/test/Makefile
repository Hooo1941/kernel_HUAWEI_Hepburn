#
# Makefile for kernel vendor pcie.
#

obj-$(CONFIG_HIPCIE_TEST) += hi_pcie_test.o
obj-$(CONFIG_HIPCIE_SLT)  += hi_pcie_slt.o
obj-$(CONFIG_HIPCIE_TEST) += hi_pcie_ep_test.o

ifeq ($(CONFIG_FMEA_FAULT_INJECTION), y)
EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/ap/platform/common/
endif
