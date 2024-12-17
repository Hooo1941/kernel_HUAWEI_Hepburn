/*
 * bg_dmabuf_test.h
 *
 * Copyright (C) 2022 Hisilicon Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef _BG_DMABUF_TEST_H
#define _BG_DMABUF_TEST_H
#include <linux/sysfs.h>
#include <linux/dma-buf.h>

#ifdef CONFIG_BG_DMABUF_TEST
void register_bg_buffer_test(struct kobject *kobj);

#ifdef CONFIG_MM_IOMMU_TEST
extern unsigned long bg_test_smmu_map_func(struct dma_buf *dmabuf,
	int prot, unsigned long *out_size);
extern int bg_test_smmu_unmap_func(struct dma_buf *dmabuf, unsigned long iova);
#endif /* CONFIG_MM_IOMMU_TEST */

#else
static inline void register_bg_buffer_test(struct kobject *kobj)
{
}
#endif /* CONFIG_BG_DMABUF_TEST */

#endif /* _BG_DMABUF_TEST_H */
