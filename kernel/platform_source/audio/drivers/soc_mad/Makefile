GCOV_PROFILE := $(CONFIG_GCOV_KERNEL)
ccflags-y += -Wdate-time -Wfloat-equal -Wformat=2

EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/include/

soc-mad-objs := soc_mad.o

obj-$(CONFIG_SOC_MAD) += soc-mad.o
