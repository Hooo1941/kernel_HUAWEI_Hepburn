# SPDX-License-Identifier: GPL-2.0
#
# Makefile for ethernet
#
ccflags-y += -Wextra -Wdate-time -Wfloat-equal -Wformat=2 -Wshadow
ifeq ($(CONFIG_VM_PRESET),y)
obj-$(CONFIG_ETHERNET_HIXGEMAC)	+= xge_plat_helper.o xge_reset.o
endif
