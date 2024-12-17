# SPDX-License-Identifier: GPL-2.0
obj-$(CONFIG_MPAM)  += mpam_ctrl.o
obj-$(CONFIG_MPAM)  += mpam_device.o
obj-$(CONFIG_MPAM_DEBUG) += mpam_debug.o
obj-y  += hck_mpam.o
obj-$(CONFIG_MPAM_DEBUG)  += mpam_cpbm_cfg.o
obj-$(CONFIG_MPAM_DEBUG)  += mpam_mon_sys.o
obj-$(CONFIG_MPAM_DEBUG)  += mpam_qos_cfg.o
obj-$(CONFIG_MPAM_DEBUG)  += mpam_mon_common.o

ccflags-y += -Wextra -Wdate-time -Wfloat-equal -Wshadow
