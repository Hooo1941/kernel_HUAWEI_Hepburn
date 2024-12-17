
ccflags-$(CONFIG_SECURITY_INFO) += -I$(srctree)/drivers/platform_drivers/tzdriver
ccflags-$(CONFIG_SECURITY_INFO) += -I$(srctree)/platform_source/see/drivers/dev_secinfo/include

obj-$(CONFIG_SECURITY_INFO)     += src/dev_secinfo.o
obj-$(CONFIG_SECURITY_INFO)     += src/dev_socid.o
obj-$(CONFIG_SECURITY_INFO)     += src/dev_secinfo_by_tee.o
obj-$(CONFIG_SECURITY_INFO)     += src/ioctl/security_info_main.o
obj-$(CONFIG_SECURITY_INFO)     += src/ioctl/bypass_net_cert.o
obj-$(CONFIG_SECURITY_INFO)     += src/ioctl/uapp_ioctl.o

ccflags-y += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow