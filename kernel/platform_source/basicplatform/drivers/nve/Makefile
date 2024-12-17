ifeq ($(CONFIG_ARM64_PTR_AUTH_EXT),y)
ccflags-y			+= -march=armv8.3-a+crypto+crc
else
ccflags-y			+= -march=armv8-a+crypto+crc
endif
obj-$(CONFIG_NVE_AP_KERNEL)		+= nve_ap_kernel.o
