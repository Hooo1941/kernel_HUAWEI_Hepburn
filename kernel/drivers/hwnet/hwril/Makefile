ifeq ($(CONFIG_TZDRIVER), y)
ccflags-y += -I$(srctree)/drivers/platform_drivers/tzdriver
endif

ifeq (taro, $(findstring taro,$(TARGET_DEVICE)))
obj-y += hwril_netlink.o vsim.o
else
obj-$(CONFIG_VSIM_ADAPT) += hwril_netlink.o vsim.o
endif