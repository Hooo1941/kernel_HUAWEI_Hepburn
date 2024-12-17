/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * Description:
 * smmu test for smmu driver implementations based on ARM architected
 * SMMU.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Create: 2019-12-6
 */

#include <linux/atomic.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/dma-iommu.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/iommu.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_iommu.h>
#include <linux/platform_device.h>
#include <linux/sizes.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/syscalls.h>
#include <linux/iommu/mm_iommu.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/highmem.h>

#include "mm_smmu.h"
#include "ion.h"
#ifdef CONFIG_DMABUF_MM
#include <linux/dmabuf/mm_dma_heap.h>
#else
#include "mm_ion_priv.h"
#endif
#include "mm_virtio_iommu.h"

static struct platform_device *test_pdev, *test_pdev2;
#define US_PER_MS     1000
#define SG_NENTS      6
#define SMMU_TEST_PA  0x10000000
#define SG_CHECK      64
#define HEAP_ID_MASK  (1 << 0)
#define PAD_SIZE_IN_TEST  0x10000
#define PAD_SIZE_OUT_TEST  0x20000

#define SG_VNENTS     (512)
#define E_SIZE		  (48)
#define VIOMMU_TEST_PGSIZE		(0x20000)
#define PAGE_NUM_64MB	(0x4000000 / PAGE_SIZE)
#define LEAK_NENTS      (SG_VNENTS * PAGE_SIZE / sizeof(unsigned long))


/* Maximum size allowed in a single DMA_BUF_TE_ALLOC call */
#define DMA_BUF_TE_ALLOC_MAX_SIZE (((SZ_4G) * 2) >> PAGE_SHIFT) /* 8 GB */
#define SMMU_TEST_TYPE_SIZE 100

enum smmu_test_func_type {
	TEST_IOMMU_MAP_DMABUF = 0,
	TEST_IOMMU_UNMAP_DMABUF,
	TEST_IOMMU_MAP_PADDING_DMABUF,
	TEST_IOMMU_UNMAP_PADDING_DMABUF,
	TEST_FUNC_TYPE_COUNT,
};

enum dma_buffer_type {
	ION_BUFFER = 0,
	NON_ION_BUFFER,
	BUFFER_TYPE_COUNT,
};

char *g_test_func_name[TEST_FUNC_TYPE_COUNT] = {
	"kernel_iommu_map_dmabuf",
	"kernel_iommu_unmap_dmabuf",
	"kernel_iommu_map_padding_dmabuf",
	"kernel_iommu_unmap_padding_dmabuf"
};

struct dma_buf_te_ioctl_alloc {
	__u64 size; /* size of buffer to allocate, in pages */
};

struct dma_buf_te_alloc {
	/* the real alloc */
	size_t nr_pages;
	struct page **pages;
};

struct dma_buf_te_attachment {
	struct sg_table *sg;
	bool attachment_mapped;
};

static long mm_smmu_time_cost(ktime_t before_tv,
				ktime_t after_tv)
{
	long usdelta;
	long tms, tus;

	usdelta = ktime_us_delta(after_tv, before_tv);

	tms = usdelta / US_PER_MS;
	tus = usdelta % US_PER_MS;
	pr_err("this operation cost %ldms %ldus\n", tms, tus);

	return  usdelta;
}

static void __close_dmabuf_fd(int fd)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	ksys_close(fd);
#else
	sys_close(fd);
#endif
}

static phys_addr_t mm_smmu_iova_to_phys(struct device *dev, dma_addr_t iova)
{
	struct iommu_domain *domain;

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		pr_err("Dev(%s) has no iommu domain!\n", dev_name(dev));
		return 0;
	}

	return iommu_iova_to_phys(domain, iova);
}

static unsigned long mm_smmu_map_sg_time(struct device *dev,
				struct scatterlist *sgl, int prot,
				unsigned long *out_size)
{
	unsigned long iova;
	ktime_t before_tv, after_tv;

	before_tv = ktime_get();
	iova = mm_iommu_map_sg(dev, sgl, prot, out_size);
	after_tv = ktime_get();
	mm_smmu_time_cost(before_tv, after_tv);
	pr_err("%s: map iova 0x%lx, size:0x%lx\n", __func__, iova, *out_size);

	return iova;
}

static int mm_iommu_unmap_sg_time(struct device *dev,
				struct scatterlist *sgl, unsigned long iova)
{
	ktime_t before_tv, after_tv;

	before_tv = ktime_get();
	mm_iommu_unmap_sg(dev, sgl, iova);
	after_tv = ktime_get();
	mm_smmu_time_cost(before_tv, after_tv);
	pr_err("%s: unmap iova 0x%lx\n", __func__, iova);

	return 0;
}

void test_smmu_map_sg(void)
{
	struct sg_table *table = NULL;
	struct scatterlist *sg = NULL;
	struct page *page = NULL;
	unsigned long iova = 0;
	unsigned long size;
	int i;
	int nents = 0;

	pr_err("into %s %d\n", __func__, __LINE__);
	table = kmalloc(sizeof(*table), GFP_KERNEL);
	if (!table)
		return;

	if (sg_alloc_table(table, SG_NENTS, GFP_KERNEL))
		goto free_table;

	sg = table->sgl;
	for (i = 0; i < SG_NENTS; i++) {
		page = alloc_page(GFP_KERNEL);
		if (!page)
			goto free_page;
		sg_set_page(sg, page, PAGE_SIZE, 0);
		sg = sg_next(sg);
	}
	sg = table->sgl;
	iova = mm_smmu_map_sg_time(&test_pdev->dev,
		sg, IOMMU_READ | IOMMU_WRITE, &size);
	if (!iova)
		goto free_page;

	mm_iommu_unmap_sg_time(&test_pdev->dev, sg, iova);
	pr_err("%s succ\n", __func__);

free_page:
	nents = i;
	for_each_sg(table->sgl, sg, nents, i) {
		page = sg_page(sg);
		__free_page(page);
	}
	sg_free_table(table);
free_table:
	kfree(table);
}

static unsigned long map_dmabuf_with_time(struct platform_device *pdev,
				struct dma_buf *dmabuf)
{
	long tus;
	unsigned long size = 0;
	unsigned long iova;
	ktime_t before_tv, after_tv;
	int prot = IOMMU_READ | IOMMU_WRITE;

	before_tv = ktime_get();
	iova = kernel_iommu_map_dmabuf(&pdev->dev, dmabuf, prot, &size);
	after_tv = ktime_get();
	tus = mm_smmu_time_cost(before_tv, after_tv);
	pr_err("map size 0x%lx used %ldus\n", size, tus);
	if (!iova)
		pr_err("%s: map dmabuf fail\n", dev_name(&pdev->dev));

	return iova;
}

static void map_dmabuf_error_print(unsigned long *iov1, unsigned long *iov2,
			 struct dma_buf *dbuf)
{
	phys_addr_t pa;

	*iov1 = map_dmabuf_with_time(test_pdev, dbuf);
	pr_err("%s: iova 0x%lx\n", dev_name(&test_pdev->dev), *iov1);
	pa = mm_smmu_iova_to_phys(&test_pdev->dev, *iov1);
	pr_err("%s: pa 0x%lx\n", dev_name(&test_pdev->dev), pa);

	*iov2 = map_dmabuf_with_time(test_pdev2, dbuf);
	pr_err("%s: iova2 0x%lx\n", dev_name(&test_pdev2->dev), *iov2);
	pa = mm_smmu_iova_to_phys(&test_pdev2->dev, *iov2);
	pr_err("%s: pa 0x%lx\n", dev_name(&test_pdev2->dev), pa);

	if (*iov1 != *iov2)
		pr_err("test iova refs fail\n");
}

static void unmap_dmabuf_error_print(unsigned long iova, unsigned long iova2,
			 struct dma_buf *dbuf)
{
	int ret;
	long tus;
	ktime_t before_tv, after_tv;

	before_tv = ktime_get();
	ret = kernel_iommu_unmap_dmabuf(&test_pdev->dev, dbuf, iova);
	after_tv = ktime_get();
	if (ret)
		pr_err("%s: map undmabuf fail\n", dev_name(&test_pdev->dev));

	ret = kernel_iommu_unmap_dmabuf(&test_pdev2->dev, dbuf, iova2);
	if (ret)
		pr_err("%s: map undmabuf fail\n", dev_name(&test_pdev2->dev));

	tus = mm_smmu_time_cost(before_tv, after_tv);
	pr_err("unmap dmabuf used %ldus\n", tus);
}

void test_smmu_map_dmabuf(size_t len)
{
	int fd;
	unsigned long iova = 0;
	unsigned long iova2 = 0;
	struct dma_buf *dmabuf = NULL;
	struct mm_domain *mm_domain = NULL;
	struct iommu_domain *domain = NULL;

	pr_err("into %s %d\n", __func__, __LINE__);
#ifdef CONFIG_DMABUF_MM
	fd = dma_heap_alloc("system_heap", len, ION_FLAG_CACHED);
#else
	fd = ion_alloc(len, HEAP_ID_MASK, ION_FLAG_CACHED);
#endif
	if (fd < 0) {
		pr_err("%s %d: alloc ion buffer fail\n", __func__, __LINE__);
		return;
	}

	dmabuf = dma_buf_get(fd);
	if (IS_ERR(dmabuf)) {
		pr_err("%s %d: get dmabuf fail\n", __func__, __LINE__);
		goto free_fd;
	}

	domain = iommu_get_domain_for_dev(&test_pdev->dev);
	if (!domain) {
		pr_err("dev has no iommu domain!\n");
		goto free_dmabuf;
	}
	mm_domain = to_mm_domain(domain);

	map_dmabuf_error_print(&iova, &iova2, dmabuf);
	mm_iova_dom_info(&test_pdev->dev);
	unmap_dmabuf_error_print(iova, iova2, dmabuf);
	pr_err("%s succ\n", __func__);

free_dmabuf:
	dma_buf_put(dmabuf);
free_fd:
	__close_dmabuf_fd(fd);
}

void test_smmu_unmap_sg_check(size_t len)
{
	unsigned long iova[SG_CHECK];
	unsigned long size = 0;
	int fd[SG_CHECK];
	int prot = IOMMU_READ | IOMMU_WRITE;
	struct dma_buf *dmabuf[SG_CHECK];
	struct scatterlist *sgl = NULL;
	int i;
	int ret = 0;

	pr_err("into %s %d\n", __func__, __LINE__);
	for (i = 0; i < SG_CHECK; i++) {
		fd[i] = -1;
		dmabuf[i] = NULL;
	}

	for (i = 0; i < SG_CHECK; i++) {
#ifdef CONFIG_DMABUF_MM
		fd[i] = dma_heap_alloc("system_heap", len, ION_FLAG_CACHED);
#else
		fd[i] = ion_alloc(len, HEAP_ID_MASK, ION_FLAG_CACHED);
#endif
		if (fd[i] < 0) {
			pr_err("%s %d: alloc ion buffer fail, i = %d\n",
				__func__, __LINE__, i);
			goto free;
		}

		dmabuf[i] = dma_buf_get(fd[i]);
		if (IS_ERR_OR_NULL(dmabuf[i])) {
			pr_err("%s %d: get dmabuf fail, i = %d\n",
				__func__, __LINE__, i);
			goto free;
		}

		iova[i] = kernel_iommu_map_dmabuf(&test_pdev->dev, dmabuf[i],
			prot, &size);
	}

	sgl = ((struct ion_buffer *)dmabuf[SG_CHECK - 1]->priv)->sg_table->sgl;
	ret = mm_iommu_unmap_sg(&test_pdev->dev, sgl, iova[SG_CHECK - 1]);
	if (ret)
		pr_err("%s %d: mm_iommu_unmap_sg ret = %d\n",
			__func__, __LINE__, ret);

	for (i = 0; i < SG_CHECK; i++) {
		ret = kernel_iommu_unmap_dmabuf(&test_pdev->dev, dmabuf[i],
			iova[i]);
		if (ret)
			pr_err("%s %d: map undmabuf fail, i = %d\n",
				__func__, __LINE__, i);
	}
	pr_err("%s test end\n", __func__);

free:
	for (i = 0; i < SG_CHECK; i++) {
		if (!IS_ERR_OR_NULL(dmabuf[i]))
			dma_buf_put(dmabuf[i]);
		if (fd[i] > 0)
			__close_dmabuf_fd(fd[i]);
	}
}

void test_smmu_padding_size_check(size_t len)
{
	struct dma_buf *dmabuf = NULL;
	unsigned long iova;
	unsigned long size = 0;
	int prot = IOMMU_READ | IOMMU_WRITE;
	int fd   = -1;
	int ret  = 0;

#ifdef CONFIG_DMABUF_MM
	fd = dma_heap_alloc("system_heap", len, ION_FLAG_CACHED);
#else
	fd = ion_alloc(len, HEAP_ID_MASK, ION_FLAG_CACHED);
#endif
	if (fd < 0) {
		pr_err("%s %d: alloc ion buffer fail!\n",
				__func__, __LINE__);
		return;
	}

	dmabuf = dma_buf_get(fd);
	if (IS_ERR_OR_NULL(dmabuf)) {
		pr_err("%s: get dmabuf fail!\n", __func__);
		goto free_fd;
	}

	iova = kernel_iommu_map_padding_dmabuf(&test_pdev->dev,
		dmabuf, PAD_SIZE_IN_TEST, prot, &size);
	if (!iova) {
		pr_err("%s: map padding size dmabuf fail!\n", __func__);
		goto free_dbuf;
	}

	ret = kernel_iommu_unmap_padding_dmabuf(&test_pdev->dev, dmabuf,
				PAD_SIZE_OUT_TEST, iova);
	if (ret) {
		pr_err("%s: unmap padding_size_out dmabuf fail,ret = %d!\n",
			__func__, ret);

		ret = kernel_iommu_unmap_padding_dmabuf(&test_pdev->dev, dmabuf,
				PAD_SIZE_IN_TEST, iova);
		if (!ret)
			pr_err("%s: unmap padding_size_in dmabuf succ,ret = %d!\n",
			__func__, ret);
	}
	pr_err("%s: test end\n", __func__);

free_dbuf:
	dma_buf_put(dmabuf);
free_fd:
	__close_dmabuf_fd(fd);
}

static void map_dmabuf_print_dom_pte(unsigned long iov1, unsigned long iov2,
				struct dma_buf *dbuf1, struct dma_buf *dbuf2)
{
	int ret;
	unsigned long size;
	int prot = IOMMU_READ | IOMMU_WRITE;

	iov1 = kernel_iommu_map_dmabuf(&test_pdev->dev, dbuf1, prot, &size);
	if (!iov1) {
		pr_err("%s: map dmabuf fail\n", dev_name(&test_pdev->dev));
		return;
	}
	pr_err("%s:iova 0x%lx\n", dev_name(&test_pdev->dev), iov1);

	iov2 = kernel_iommu_map_dmabuf(&test_pdev->dev, dbuf2, prot, &size);
	if (!iov2) {
		pr_err("%s: map dmabuf2 fail\n", dev_name(&test_pdev->dev));
		return;
	}
	pr_err("%s:iova2 0x%lx\n", dev_name(&test_pdev->dev), iov2);

#if !defined(CONFIG_ARM_SMMU_V3) && !defined(CONFIG_MM_SMMU_V3)
	mm_print_iova_dom(&test_pdev->dev);
#endif
	ret = kernel_iommu_unmap_dmabuf(&test_pdev->dev, dbuf1, iov1);
	pr_err("return %d after unmap dmabuf1.\n", ret);
	if (ret)
		pr_err("%s: map dmabuf fail\n", dev_name(&test_pdev->dev));

	ret = kernel_iommu_unmap_dmabuf(&test_pdev->dev, dbuf2, iov2);
	pr_err("return %d after unmap dmabuf2.\n", ret);
	if (ret)
		pr_err("%s: map dmabuf2 fail\n", dev_name(&test_pdev->dev));
}

static void test_smmu_print_dom_pte(void)
{
	int fd;
	int fd2;
	struct dma_buf *dmabuf = NULL;
	struct dma_buf *dmabuf2 = NULL;
	unsigned long iova = 0;
	unsigned long iova2 = 0;
	struct iommu_domain *domain = NULL;

	pr_err("into %s %d\n", __func__, __LINE__);
#ifdef CONFIG_DMABUF_MM
	fd = dma_heap_alloc("system_heap", SZ_64K, ION_FLAG_CACHED);
#else
	fd = ion_alloc(SZ_64K, HEAP_ID_MASK, ION_FLAG_CACHED);
#endif
	if (fd < 0) {
		pr_err("alloc ion buffer fail\n");
		return;
	}

	dmabuf = dma_buf_get(fd);
	if (IS_ERR(dmabuf)) {
		pr_err("get dmabuf fail\n");
		return;
	}

#ifdef CONFIG_DMABUF_MM
	fd2 = dma_heap_alloc("system_heap", SZ_64K, ION_FLAG_CACHED);
#else
	fd2 = ion_alloc(SZ_64K, HEAP_ID_MASK, ION_FLAG_CACHED);
#endif
	if (fd2 < 0) {
		pr_err("alloc ion buffer 2 fail\n");
		return;
	}

	dmabuf2 = dma_buf_get(fd2);
	if (IS_ERR(dmabuf)) {
		pr_err("get dmabuf2 fail\n");
		return;
	}

	domain = iommu_get_domain_for_dev(&test_pdev->dev);
	if (!domain) {
		pr_err("dev has no iommu domain!\n");
		return;
	}

	map_dmabuf_print_dom_pte(iova, iova2, dmabuf, dmabuf2);

	dma_buf_put(dmabuf);
	dma_buf_put(dmabuf2);
	__close_dmabuf_fd(fd);
	__close_dmabuf_fd(fd2);
	pr_err("%s succ!!\n", __func__);
}

void test_smmu_map(size_t size)
{
	struct iommu_domain *domain =
		iommu_get_domain_for_dev(&test_pdev->dev);
	unsigned long iova;
	ktime_t before_tv, after_tv;
	int prot = IOMMU_READ | IOMMU_WRITE;

	pr_err("into %s %d\n", __func__, __LINE__);
	if (!domain) {
		pr_err("%s:domian is NULL\n", __func__);
		return;
	}

	before_tv = ktime_get();
	iova = mm_iommu_map(&test_pdev->dev, SMMU_TEST_PA, size, prot);
	after_tv = ktime_get();
	mm_smmu_time_cost(before_tv, after_tv);
	if (!iova) {
		pr_err("iommu_map fail\n");
		return;
	}
	pr_err("%s:map ova is 0x%lx, size 0x%lx\n", __func__, iova, size);

	before_tv = ktime_get();
	if (mm_iommu_unmap(&test_pdev->dev, iova, size))
		pr_err("iommu_unmap_phy fail\n");

	after_tv = ktime_get();
	mm_smmu_time_cost(before_tv, after_tv);
	pr_err("%s:map ova is 0x%lx\n", __func__, iova);
	pr_err("%s succ\n", __func__);
}

static void test_smmu_print_pte(void)
{
	struct iommu_domain *domain =
		iommu_get_domain_for_dev(&test_pdev->dev);
	unsigned long iova;
	size_t size = SZ_64K;
	int prot = IOMMU_READ | IOMMU_WRITE;
	int ret;

	if (!domain) {
		pr_err("%s:domian is NULL\n", __func__);
		return;
	}

	iova = mm_iommu_map(&test_pdev->dev, SMMU_TEST_PA, size, prot);
	if (!iova) {
		pr_err("iommu_map fail\n");
		return;
	}
	pr_err("%s: iova is 0x%lx\n", __func__, iova);
#if !defined(CONFIG_ARM_SMMU_V3) && !defined(CONFIG_MM_SMMU_V3)
	mm_smmu_show_pte(&test_pdev->dev, iova, size);
#endif
	ret = mm_iommu_unmap(&test_pdev->dev, iova, size);
	if (ret) {
		pr_err("iommu_unmap_phy fail\n");
		return;
	}

	pr_err("print_pte succ\n");
}

void test_smmu_dma_release(size_t len)
{
	int fd;
	unsigned long iova, iova2;
	ktime_t before_tv, after_tv;
	struct dma_buf *dmabuf = NULL;

	pr_err("into %s %d\n", __func__, __LINE__);
#ifdef CONFIG_DMABUF_MM
	fd = dma_heap_alloc("system_heap", len, ION_FLAG_CACHED);
#else
	fd = ion_alloc(len, HEAP_ID_MASK, ION_FLAG_CACHED);
#endif
	if (fd < 0) {
		pr_err("%s %d: alloc ion buffer fail\n", __func__, __LINE__);
		return;
	}

	dmabuf = dma_buf_get(fd);
	if (IS_ERR(dmabuf)) {
		pr_err("%s %d: get dmabuf fail\n", __func__, __LINE__);
		__close_dmabuf_fd(fd);
		return;
	}

	map_dmabuf_error_print(&iova, &iova2, dmabuf);
	before_tv = ktime_get();
	dmabuf_release_iommu(dmabuf);
	after_tv = ktime_get();
	mm_smmu_time_cost(before_tv, after_tv);
	dma_buf_put(dmabuf);
	__close_dmabuf_fd(fd);
	pr_err("%s succ\n", __func__);
}

void test_smmu(void)
{
	struct iommu_group *dev_group = NULL;
	struct iommu_domain *domain = NULL;

	if (!test_pdev || !test_pdev2) {
		pr_err("test fail! device NULL! dts is not suitable\n");
		return;
	}

	dev_group = iommu_group_get(&test_pdev->dev);
	domain = iommu_get_domain_for_dev(&test_pdev->dev);

	if (!dev_group) {
		pr_err("test fail! dev_group NULL\n");
		return;
	}

	if (!domain) {
		pr_err("test fail! domian is NULL\n");
		return;
	}

	pr_err("%s %d:start\n", __func__, __LINE__);
	test_smmu_map_sg();
	test_smmu_map(SZ_32M);
	test_smmu_map_dmabuf(SZ_32M);
	test_smmu_print_pte();
	test_smmu_print_dom_pte();
	pr_err("%s %d:end\n", __func__, __LINE__);
}

static int mm_smmu_test_probe(struct platform_device *pdev)
{
	if (!test_pdev) {
		test_pdev = pdev;
		pr_err("%s %d:test dev1\n", __func__, __LINE__);
	} else {
		test_pdev2 = pdev;
		pr_err("%s %d:test dev2\n", __func__, __LINE__);
	}
	dma_set_mask_and_coherent(&pdev->dev, ~0ULL);
	pr_err("dev %pK (%s)\n", &pdev->dev, dev_name(&pdev->dev));

	return 0;
}

static int mm_smmu_test_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id mm_smmu_test_of_match[] = {
	{.compatible = "mm,smmu_test" },
	{},
};
MODULE_DEVICE_TABLE(of, mm_smmu_test_of_match);

static struct platform_driver mm_smmu_test_driver = {
	.driver = {
			.owner = THIS_MODULE,
			.name = "smmu-test",
			.of_match_table = of_match_ptr(mm_smmu_test_of_match),
		},
	.probe = mm_smmu_test_probe,
	.remove = mm_smmu_test_remove,
};

static int dma_buf_te_attach(struct dma_buf *buf, struct dma_buf_attachment *attachment)
{
	attachment->priv = kzalloc(sizeof(struct dma_buf_te_attachment), GFP_KERNEL);
	if (!attachment->priv)
		return -ENOMEM;

	return 0;
}

/**
 * dma_buf_te_detach - The detach callback function to release &attachment
 *
 * @buf: buffer for the &attachment
 * @attachment: attachment data to be released
 */
static void dma_buf_te_detach(struct dma_buf *buf, struct dma_buf_attachment *attachment)
{
	struct dma_buf_te_attachment *pa = attachment->priv;

	if (pa->attachment_mapped)
		pr_err("%s: WARNING: detach with mapped attachment.\n", __func__);

	kfree(pa);
}

static struct sg_table *dma_buf_te_map(struct dma_buf_attachment *attachment,
		enum dma_data_direction direction)
{
	struct sg_table *sg = NULL;
	struct scatterlist *iter = NULL;
	struct dma_buf_te_alloc *alloc = NULL;
	struct dma_buf_te_attachment *pa = attachment->priv;
	size_t i;
	int ret;

	alloc = attachment->dmabuf->priv;

	if (pa->attachment_mapped) {
		pr_err("%s: Attempted to map already mapped attachment.\n", __func__);
		return ERR_PTR(-EBUSY);
	}

	sg = kmalloc(sizeof(struct sg_table), GFP_KERNEL);
	if (!sg)
		return ERR_PTR(-ENOMEM);

	/* from here we access the allocation object, so lock the dmabuf pointing to it */
	mutex_lock(&attachment->dmabuf->lock);

	ret = sg_alloc_table(sg, alloc->nr_pages, GFP_KERNEL);
	if (ret) {
		mutex_unlock(&attachment->dmabuf->lock);
		kfree(sg);
		return ERR_PTR(ret);
	}

	for_each_sg(sg->sgl, iter, alloc->nr_pages, i)
		sg_set_page(iter, alloc->pages[i], PAGE_SIZE, 0);

	if (!dma_map_sg(attachment->dev, sg->sgl, sg->nents, direction)) {
		mutex_unlock(&attachment->dmabuf->lock);
		sg_free_table(sg);
		kfree(sg);
		return ERR_PTR(-ENOMEM);
	}

	pa->attachment_mapped = true;
	pa->sg = sg;
	mutex_unlock(&attachment->dmabuf->lock);
	return sg;
}

static void dma_buf_te_unmap(struct dma_buf_attachment *attachment,
		struct sg_table *sg, enum dma_data_direction direction)
{
	struct dma_buf_te_attachment *pa = attachment->priv;

	mutex_lock(&attachment->dmabuf->lock);

	if (!pa->attachment_mapped)
		pr_err("%s: WARNING: Unmatched unmap of attachment.\n", __func__);

	pa->attachment_mapped = false;
	pa->sg = NULL;
	mutex_unlock(&attachment->dmabuf->lock);

	dma_unmap_sg(attachment->dev, sg->sgl, sg->nents, direction);
	sg_free_table(sg);
	kfree(sg);
}

static void dma_buf_te_release(struct dma_buf *buf)
{
	size_t i;
	struct dma_buf_te_alloc *alloc = NULL;

	alloc = buf->priv;
	/* no need for locking */

	for (i = 0; i < alloc->nr_pages; i++)
		__free_page(alloc->pages[i]);

	kvfree(alloc->pages);

	kfree(alloc);
}

static struct dma_buf_ops dma_buf_te_ops = {
	.attach = dma_buf_te_attach,
	.detach = dma_buf_te_detach,
	.map_dma_buf = dma_buf_te_map,
	.unmap_dma_buf = dma_buf_te_unmap,
	.release = dma_buf_te_release,
};

static int do_test_dma_buf_alloc(struct dma_buf_te_ioctl_alloc *buf)
{
	struct dma_buf_te_ioctl_alloc *alloc_req = NULL;
	struct dma_buf_te_alloc *alloc = NULL;
	struct dma_buf *dma_buf = NULL;
	size_t i = 0;
	size_t max_nr_pages = DMA_BUF_TE_ALLOC_MAX_SIZE;
	int fd;
	struct dma_buf_export_info export_info = {
		.exp_name = "dma_buf_te",
		.owner = THIS_MODULE,
		.ops = &dma_buf_te_ops,
		.flags = O_CLOEXEC | O_RDWR,
	};

	if (!buf) {
		pr_err("%s: buf is NULL\n", __func__);
		goto no_input;
	}

	alloc_req = buf;
	if (!alloc_req->size || alloc_req->size > max_nr_pages) {
		pr_err("%s: invalid size of %llu, the mapping limit is %zu pages\n",
			__func__, alloc_req->size, max_nr_pages);
		goto invalid_size;
	}

	alloc = kzalloc(sizeof(struct dma_buf_te_alloc), GFP_KERNEL);
	if (alloc == NULL) {
		pr_err("%s: couldn't alloc object\n", __func__);
		goto no_alloc_object;
	}

	alloc->nr_pages = alloc_req->size;
	alloc->pages = kvzalloc(sizeof(struct page *) * alloc->nr_pages, GFP_KERNEL);

	if (!alloc->pages) {
		pr_err("%s: couldn't alloc %zu page structures", __func__, alloc->nr_pages);
		goto free_alloc_object;
	}

	for (i = 0; i < alloc->nr_pages; i++) {
		alloc->pages[i] = alloc_page(GFP_KERNEL | __GFP_ZERO);
		if (alloc->pages[i] == NULL) {
			pr_err("%s: couldn't alloc page\n", __func__);
			goto no_page;
		}
	}

	/* alloc ready, export it */
	export_info.size = alloc->nr_pages << PAGE_SHIFT;
	export_info.priv = alloc;
	dma_buf = dma_buf_export(&export_info);
	if (IS_ERR_OR_NULL(dma_buf)) {
		pr_err("%s: couldn't export dma_buf\n", __func__);
		goto no_export;
	}

	/* get fd for buf */
	fd = dma_buf_fd(dma_buf, O_CLOEXEC);
	if (fd < 0) {
		pr_err("%s: couldn't get fd from dma_buf\n", __func__);
		goto no_fd;
	} else {
		pr_err("smmu_test info into %s, alloc NON_ION_BUFFER dmabuf_type succ\n", __func__);
	}

	return fd;

no_fd:
	dma_buf_put(dma_buf);
no_export:
	/* i still valid */
no_page:
	while (i-- > 0)
		__free_page(alloc->pages[i]);
	kvfree(alloc->pages);
free_alloc_object:
	kfree(alloc);
no_alloc_object:
invalid_size:
no_input:
	return -EFAULT;
}

static int smmu_test_set_debugfs_show(struct seq_file *m, void *v)
{
	int prot = IOMMU_READ | IOMMU_WRITE;
	unsigned long padding_len = PAD_SIZE_IN_TEST;

	seq_puts(m, "smmu_test param_input is:\n");
	seq_puts(m, "[func_type0-4],[dmabuf_type0-1],param1,param2....\n");
	seq_puts(m, "for exmple: want to test kernel_iommu_map_dmabuf, param:dma_buf,prot\n");
	seq_printf(m, "echo \"0,0,%d\" > smmu_test_set_param\n", prot);
	seq_printf(m, "show normal prot is %d\n", prot);
	seq_printf(m, "show normal padding_len is %lu\n", padding_len);

	return 0;
}

static int smmu_test_set_debugfs_open(struct inode *inode, struct file *file)
{
	pr_err("%s\n", __func__);
	single_open(file, smmu_test_set_debugfs_show, NULL);
	return 0;
}

static int do_alloc_dma_buffer(int dma_buffer_type, struct dma_buf_te_ioctl_alloc **alloc_req)
{
	int fd   = -1;
	size_t len = SZ_16M;
	size_t pages = (len) >> PAGE_SHIFT;

	pr_err("smmu_test info into %s\n", __func__);

	if (dma_buffer_type == ION_BUFFER) {
#ifdef CONFIG_DMABUF_MM
		fd = dma_heap_alloc("system_heap", len, ION_FLAG_CACHED);
#else
		fd = ion_alloc(len, HEAP_ID_MASK, ION_FLAG_CACHED);
#endif
		if (fd < 0) {
			pr_err("%s %d: alloc ion buffer fail!\n", __func__, __LINE__);
			return -1;
		}
	} else if (dma_buffer_type == NON_ION_BUFFER){
		pr_err("smmu_test info into %s, dma_buffer_type is NON_ION_BUFFER\n", __func__);
		*alloc_req = kzalloc(sizeof(struct dma_buf_te_ioctl_alloc), GFP_KERNEL);
		(*alloc_req)->size = pages;
		fd = do_test_dma_buf_alloc(*alloc_req);
		if (fd < 0) {
			pr_err("%s %d: do_test_dma_buf_alloc fail!\n", __func__, __LINE__);
			kfree(*alloc_req);
			return -1;
		}
	} else {
		pr_err("%s %d: invailed dma_buffer_type!\n", __func__, __LINE__);
		return -1;
	}

	pr_err("smmu_test info out %s\n", __func__);
	return fd;
}

static unsigned long __map_func(int type, struct device *dev, struct dma_buf *dmabuf,
		unsigned long padding_len, int prot, unsigned long *out_size)
{
	unsigned long ret = 0;

	if (type == TEST_IOMMU_MAP_DMABUF)
		ret = kernel_iommu_map_dmabuf(dev, dmabuf, prot, out_size);
	else if (type == TEST_IOMMU_MAP_PADDING_DMABUF)
		ret = kernel_iommu_map_padding_dmabuf(dev, dmabuf, padding_len, prot, out_size);

	return ret;
}

static int __unmap_func(int type, struct device *dev, struct dma_buf *dmabuf,
		unsigned long padding_len, unsigned long iova)
{
	int ret = -1;

	if (type == TEST_IOMMU_MAP_DMABUF || type == TEST_IOMMU_UNMAP_DMABUF)
		ret = kernel_iommu_unmap_dmabuf(dev, dmabuf, iova);
	else if (type == TEST_IOMMU_MAP_PADDING_DMABUF || type == TEST_IOMMU_UNMAP_PADDING_DMABUF)
		ret = kernel_iommu_unmap_padding_dmabuf(dev, dmabuf, padding_len, iova);

	return ret;
}

#ifdef CONFIG_BG_DMABUF_TEST
static inline struct device *_get_smmu_test_dev(void)
{
	if (test_pdev)
		return (&test_pdev->dev);
	else if (test_pdev2)
		return (&test_pdev2->dev);
	else
		return NULL;
}

unsigned long bg_test_smmu_map_func(struct dma_buf *dmabuf,
	int prot, unsigned long *out_size)
{
	struct device *dev = _get_smmu_test_dev();

	if (!dev) {
		pr_err("%s test_pdev is null\n", __func__);
		return 0;
	}

	return __map_func(TEST_IOMMU_MAP_DMABUF, dev, dmabuf, 0, prot, out_size);
}

int bg_test_smmu_unmap_func(struct dma_buf *dmabuf, unsigned long iova)
{
	struct device *dev = _get_smmu_test_dev();

	if (!dev) {
		pr_err("%s test_pdev is null\n", __func__);
		return -EINVAL;
	}

	return __unmap_func(TEST_IOMMU_MAP_DMABUF, &test_pdev->dev, dmabuf, 0, iova);
}
#endif /* CONFIG_BG_DMABUF_TEST */

static int test_smmu_map_unmap(int dma_buffer_type, int prot_set,
		unsigned long iova_set, unsigned long padding_len_set, int test_func_set)
{
	struct dma_buf *dmabuf = NULL;
	struct dma_buf_te_ioctl_alloc *alloc_req = NULL;
	unsigned long iova;
	unsigned long padding_len = PAD_SIZE_IN_TEST;
	unsigned long size = 0;
	int prot = IOMMU_READ | IOMMU_WRITE;
	int fd   = -1;
	int ret  = 0;

	pr_err("smmu_test info into %s\n", __func__);

	fd = do_alloc_dma_buffer(dma_buffer_type, &alloc_req);
	if (fd < 0) {
		pr_err("%s %d: do_alloc_dma_buffer fail!\n", __func__, __LINE__);
		return -1;
	}

	dmabuf = dma_buf_get(fd);
	if (IS_ERR_OR_NULL(dmabuf)) {
		pr_err("%s: get dmabuf fail!\n", __func__);
		goto free_fd;
	}
	if (test_func_set == TEST_IOMMU_MAP_DMABUF ||
		test_func_set == TEST_IOMMU_MAP_PADDING_DMABUF) {
		prot = prot_set;
		padding_len = padding_len_set;
		iova = __map_func(test_func_set, &test_pdev->dev,dmabuf, padding_len, prot, &size);
		if (!iova) {
			pr_err("%s:test %s fail! padding_len = %lu, prot = %d!\n",
				__func__, g_test_func_name[test_func_set], padding_len, prot);
			goto free_dbuf;
		} else {
			pr_err("%s: func %s succ!\n", __func__, g_test_func_name[test_func_set]);
			ret = __unmap_func(test_func_set, &test_pdev->dev, dmabuf, padding_len, iova);
			if (ret)
				pr_err("%s: func %s fail, ret = %d!\n",
					 __func__, g_test_func_name[test_func_set], ret);
			else
				pr_err("%s: func %s succ!\n", __func__, g_test_func_name[test_func_set]);
		}
	} else if (test_func_set == TEST_IOMMU_UNMAP_DMABUF ||
		test_func_set == TEST_IOMMU_UNMAP_PADDING_DMABUF) {
		iova = iova_set;
		padding_len = padding_len_set;
		ret = __unmap_func(test_func_set, &test_pdev->dev, dmabuf, padding_len, iova);
		if (ret)
			pr_err("%s: func %s, ret = %d!\n",__func__, g_test_func_name[test_func_set], ret);
		else
			pr_err("%s: func %s succ!\n",__func__, g_test_func_name[test_func_set]);
	}

	pr_err("%s: test %s end\n", __func__, g_test_func_name[test_func_set]);
	if (!ret)
		pr_err("%s: test succ\n", __func__);
free_dbuf:
	dma_buf_put(dmabuf);
free_fd:
	__close_dmabuf_fd(fd);
	if (alloc_req)
		kfree(alloc_req);

	pr_err("smmu_test info out %s\n", __func__);
	return ret;
}

static int get_type_param_check_range(char **input_param_buf, char *input_param,
		int type_range_left_value, int type_range_right_value)
{
	int ret = 0;
	int type_input;

	input_param = strsep(input_param_buf, ",");
	if (!input_param) {
		pr_err("%s: err input_param buffer :%s\n", __func__);
		return -EFAULT;
	}

	ret = kstrtoint(input_param, 0, &type_input);
	if (ret != 0) {
		pr_err("%s : type_input kstrtoint failed\n", __func__);
		return -EFAULT;
	}
	if (type_input < type_range_left_value ||
		type_input >= type_range_right_value) {
		pr_err("%s: err type_input :%d\n", __func__, type_input);
		return -EFAULT;
	}

	return type_input;
}

static void do_test_smmu_map_unmap(int func_type_input, int dmabuf_type_input,
		unsigned long param_3, unsigned long param_4)
{
	int prot_input;
	unsigned long iova_input;
	unsigned long padding_len_input;

	switch (func_type_input) {
	case TEST_IOMMU_MAP_DMABUF:
		prot_input = (int)param_3;
		test_smmu_map_unmap(dmabuf_type_input, prot_input, 0, 0, func_type_input);
		break;
	case TEST_IOMMU_UNMAP_DMABUF:
		iova_input = param_3;
		test_smmu_map_unmap(dmabuf_type_input, 0, iova_input, 0, func_type_input);
		break;
	case TEST_IOMMU_MAP_PADDING_DMABUF:
		padding_len_input = param_3;
		prot_input = (int)param_4;
		test_smmu_map_unmap(dmabuf_type_input, prot_input, 0, padding_len_input, func_type_input);
		break;
	case TEST_IOMMU_UNMAP_PADDING_DMABUF:
		padding_len_input = param_3;
		iova_input = param_4;
		test_smmu_map_unmap(dmabuf_type_input, 0, iova_input, padding_len_input, func_type_input);
		break;
	default:
		pr_err("%s: invalid func_type_input %d\n", __func__, func_type_input);
		break;
	}

	return;
}

static ssize_t smmu_test_set_debugfs_write(struct file *file,
		const char __user *user_buffer, size_t cnt, loff_t *ppos)
{
	char buffer[SMMU_TEST_TYPE_SIZE] = {0};
	char *input_param_buf = NULL;
	char *input_param = NULL;
	int func_type_input, dmabuf_type_input, type_input;
	unsigned long param_3;
	unsigned long param_4 = 0;
	ssize_t ret = 0;

	pr_err("smmu_test info into %s\n", __func__);

	if (cnt > sizeof(buffer) - 1) {
		pr_err("%s: err oversize input_param buffer\n", __func__);
		return -EFAULT;
	}

	if (copy_from_user(buffer, user_buffer, cnt))
		return -EFAULT;

	buffer[cnt] = '\0';
	input_param_buf = strstrip(buffer);

	/* param1 : func_type */
	type_input = get_type_param_check_range(&input_param_buf,
		input_param, 0, TEST_FUNC_TYPE_COUNT);
	if (type_input < 0) {
		pr_err("%s : func_type_input invalid, err input buffer:%s\n", __func__, buffer);
		return -EFAULT;
	}
	func_type_input = type_input;

	/* param2 : dmabuf_type */
	type_input = get_type_param_check_range(&input_param_buf,
		input_param, 0, BUFFER_TYPE_COUNT);
	if (type_input < 0) {
		pr_err("%s : dmabuf_type_input invalid, err input buffer:%s\n", __func__, buffer);
		return -EFAULT;
	}
	dmabuf_type_input = type_input;

	/* param3 */
	input_param = strsep(&input_param_buf, ",");
	if (!input_param) {
		pr_err("%s: err input_param buffer :%s\n", __func__, buffer);
		return -EFAULT;
	}
	ret = kstrtoul(input_param, 0, &param_3);
	if (ret != 0) {
		pr_err("%s : param_3 kstrtoul failed\n", __func__);
		return -EFAULT;
	}

	/* param4 */
	if (func_type_input == TEST_IOMMU_MAP_PADDING_DMABUF ||
		func_type_input == TEST_IOMMU_UNMAP_PADDING_DMABUF) {
		input_param = strsep(&input_param_buf, ",");
		if (!input_param) {
			pr_err("%s: err input_param buffer :%s\n", __func__, buffer);
			return -EFAULT;
		}
		ret = kstrtoul(input_param, 0, &param_4);
		if (ret != 0) {
			pr_err("%s : param_4 kstrtoul failed\n", __func__);
			return -EFAULT;
		}
	}

	do_test_smmu_map_unmap(func_type_input, dmabuf_type_input, param_3, param_4);
	pr_err("smmu_test info out %s\n", __func__);
	return cnt;
}

void test_vsmmu_map_dmabuf(size_t page_num)
{
	int fd;
	unsigned long iova1 = 0;
	unsigned long iova2 = 0;
	struct dma_buf *dmabuf = NULL;
	ktime_t before_tv1, after_tv1;
	ktime_t before_tv2, after_tv2;

	pr_err("%s start, page_num = %d\n", __func__, page_num);
#ifdef CONFIG_DMABUF_MM
	fd = dma_heap_alloc("system_heap", PAGE_SIZE * page_num, ION_FLAG_CACHED);
#else
	fd = ion_alloc(PAGE_SIZE * page_num, HEAP_ID_MASK, ION_FLAG_CACHED);
#endif
	if (fd < 0) {
		pr_err("%s %d: alloc ion buffer fail\n", __func__, __LINE__);
		return;
	}

	dmabuf = dma_buf_get(fd);
	if (IS_ERR(dmabuf)) {
		pr_err("%s %d: get dmabuf fail\n", __func__, __LINE__);
		goto free_fd;
	}

	before_tv1 = ktime_get();
	iova1 = viommu_map_dmabuf(0, dmabuf);
	after_tv1 = ktime_get();

	before_tv2 = ktime_get();
	iova2 = viommu_map_dmabuf(0, dmabuf);
	after_tv2 = ktime_get();

	mm_smmu_time_cost(before_tv1, after_tv1);
	mm_smmu_time_cost(before_tv2, after_tv2);

	pr_err("%s succ, iova1 = 0x%lx, iova2 = 0x%lx\n", __func__, iova1, iova2);

	dma_buf_put(dmabuf);
free_fd:
	__close_dmabuf_fd(fd);
}

void test_vsmmu_map_sg(void)
{
	struct sg_table *table = NULL;
	struct scatterlist *sg = NULL;
	struct page *page = NULL;
	unsigned long iova = 0;
	int nents;
	int i;

	pr_err("%s start\n", __func__);
	table = kmalloc(sizeof(*table), GFP_KERNEL);
	if (!table)
		return;

	if (sg_alloc_table(table, SG_VNENTS, GFP_KERNEL))
		goto free_table;

	sg = table->sgl;
	for (i = 0; i < SG_VNENTS; i++) {
		page = alloc_page(GFP_KERNEL);
		if (!page)
			goto free_page;
		sg_set_page(sg, page, PAGE_SIZE, 0);
		sg = sg_next(sg);
	}
	sg = table->sgl;
	iova = viommu_map_sg(SMMUID_TEST, sg);
	if (!iova)
		goto free_page;

	pr_err("%s succ, iova = 0x%lx\n", __func__, iova);

	viommu_unmap_sg(SMMUID_TEST, sg, iova);
free_page:
	nents = i;
	for_each_sg(table->sgl, sg, nents, i) {
		page = sg_page(sg);
		__free_page(page);
	}
	sg_free_table(table);
free_table:
	kfree(table);
}

/* iova leak test: map more than 3G iova */
void test_vsmmu_map_iova_leak(void)
{
	struct sg_table *table = NULL;
	struct scatterlist *sg = NULL;
	struct page *page = NULL;
	unsigned long iova[E_SIZE] = {0};
	int nents = PAGE_NUM_64MB;
	unsigned long size;
	int ret;
	int i, j;

	pr_err("%s start\n", __func__);
	table = kmalloc(sizeof(*table), GFP_KERNEL);
	if (!table)
		return;

	if (sg_alloc_table(table, nents, GFP_KERNEL))
		goto free_table;
	/* alloc 64MB sgl */
	sg = table->sgl;
	for (i = 0; i < nents; i++) {
		page = alloc_page(GFP_KERNEL);
		if (!page)
			goto free_page;
		sg_set_page(sg, page, PAGE_SIZE, 0);
		sg = sg_next(sg);
	}
	sg = table->sgl;

	size = nents * PAGE_SIZE;
	/* 48 * 64MB = 3072MB(3GB) */
	for (j = 0; j < E_SIZE; j++) {
		iova[j] = viommu_map_sg(SMMUID_TEST, sg);
		if (!iova[j]) {
			pr_err("%s, map fail, iova[%d]:0x%lx, size:0x%lx\n",
						__func__, j, iova[j], size);
			break;
		}
		pr_err("%s, map succ, iova[%d]:0x%lx, size:0x%lx\n",
						__func__, j, iova[j], size);
	}

	for (--j; j >= 0; j--) {
		ret = viommu_unmap_sg(SMMUID_TEST, sg, iova[j]);
		if (ret) {
			pr_err("%s, unmap fail, iova[%d]:0x%lx, size:0x%lx\n",
						__func__, j, iova[j], size);
			break;
		}
		pr_err("%s, unmap succ, iova[%d]:0x%lx, size:0x%lx\n",
						__func__, j, iova[j], size);
	}

	pr_err("%s end\n", __func__);

free_page:
	nents = i;
	for_each_sg(table->sgl, sg, nents, i) {
		page = sg_page(sg);
		__free_page(page);
	}
	sg_free_table(table);
free_table:
	kfree(table);
}

/* virtio fail test: add virtio queue entry more than 512 */
void test_vsmmu_virtio_fail(void)
{
	struct sg_table *table = NULL;
	struct scatterlist *sg = NULL;
	struct page *page = NULL;
	unsigned long iova = 0;
	int nents = LEAK_NENTS;
	int i;
	int ret = 0;

	pr_err("%s start\n", __func__);
	table = kmalloc(sizeof(*table), GFP_KERNEL);
	if (!table)
		return;

	if (sg_alloc_table(table, nents, GFP_KERNEL)) {
		pr_err("%s, sg_alloc_table\n", __func__);
		goto free_table;
	}

	sg = table->sgl;
	/* nents * 4KB = 512MB */
	for (i = 0; i < nents; i++) {
		page = alloc_page(GFP_KERNEL);
		if (!page)
			goto free_page;
		sg_set_page(sg, page, PAGE_SIZE, 0);
		sg = sg_next(sg);
	}
	sg = table->sgl;

	iova = viommu_map_sg(SMMUID_TEST, sg);
	if (!iova) {
		pr_err("%s, map_sg fail\n", __func__);
		goto free_page;
	}

	ret = viommu_unmap_sg(SMMUID_TEST, sg, iova);
	if (ret)
		pr_err("%s, unmap_sg fail\n", __func__);

free_page:
	for_each_sg(table->sgl, sg, nents, i) {
		page = sg_page(sg);
		__free_page(page);
	}
	sg_free_table(table);
free_table:
	kfree(table);
	pr_err("%s end\n", __func__);
}

int test_vsmmu_iova_to_phys(unsigned long iova, size_t iova_size)
{
	return 0;
}

static const struct file_operations smmu_test_set_debugfs_fops = {
	.open = smmu_test_set_debugfs_open,
	.write = smmu_test_set_debugfs_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static void __init debugfs_mm_smmu_test_operation_init(void)
{
	struct dentry *debug_file = NULL;
	struct dentry *debug_dir = NULL;

	debug_dir = debugfs_create_dir("mm_smmu_test", NULL);
	if (IS_ERR_OR_NULL(debug_dir)) {
		pr_err("failed to create the mm_smmu_test debug dir\n");
		return;
	}

	debug_file = debugfs_create_file("smmu_test_set_param",
		0660, debug_dir, NULL, &smmu_test_set_debugfs_fops);
	if (IS_ERR_OR_NULL(debug_file)) {
		pr_err("failed to create smmu_test_set_param debug file in dir\n");
		return;
	}
}

static int __init mm_smmu_test_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&mm_smmu_test_driver);
	debugfs_mm_smmu_test_operation_init();
	return ret;
}

static void __exit mm_smmu_test_exit(void)
{
	platform_driver_unregister(&mm_smmu_test_driver);
}

module_init(mm_smmu_test_init);
module_exit(mm_smmu_test_exit);
