ccflags-y += -Wextra -Wdate-time -Wfloat-equal -Wshadow -Wformat=2
obj-$(CONFIG_FFRT_RTG) += ffrt_rtg.o
obj-$(CONFIG_QOS_CTRL) += qos_ctrl.o
obj-$(CONFIG_FFTS_SCHED) += ffts_sched.o

ccflags-$(CONFIG_FFTS_SCHED) += -I$(srctree)/platform_source/ffrt
