# SPDX-License-Identifier: GPL-2.0
ccflags-y += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow
ccflags-y +=  -I$(srctree)/drivers/dma

obj-$(CONFIG_PERI_DMA) += peri_dma_64.o
obj-$(CONFIG_DMA_SMMU_ENABLE) += smmu_dma.o
