obj-$(CONFIG_HIPCIE_IDLE) += hi_pcie_idle.o
obj-$(CONFIG_HIPCIE_OCT) += hi_pcie_oct.o
obj-$(CONFIG_HIPCIE_OCT) += hi_pcie_oct_common.o
obj-$(CONFIG_HIPCIE_NSV) += hi_pcie_nsv.o

ifeq ($(CONFIG_HIPCIE_NSV), y)
EXTRA_CFLAGS += -I$(srctree)/drivers/hwspinlock
endif

ifeq ($(CONFIG_HIPCIE_IDLE), y)
EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/ap/platform/common/lowpm/
endif
