KERNEL_DIR := $(srctree)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/kernel/sched

obj-y += hck_cpufreq_schedutil.o
obj-y += plat_sched.o
obj-$(CONFIG_SCHED_COLOC_GROUP) += walt_coloc_grp.o
obj-y += misc.o
