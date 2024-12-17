/*
 *  ISP driver, hisp_mempool.c
 *
 * Copyright (c) 2013 ISP Technologies CO., Ltd.
 *
 */

#include <platform_include/isp/linux/hisp_remoteproc.h>
#include <platform_include/isp/linux/hisp_mempool.h>
#include "hisp_internel.h"
#include <securec.h>
#include <linux/iommu/mm_iommu.h>
#include "isp_ddr_map.h"
#include <linux/dma-buf.h>
#ifdef CONFIG_ION
#include <linux/ion.h>
#include <linux/ion/mm_ion.h>
#elif defined CONFIG_DMABUF_HEAPS
#include <linux/dma-heap.h>
#include <linux/dmabuf/mm_dma_heap.h>
#endif
#include <linux/syscalls.h>

#define ISP_MAX_BYTE_BIT_NUM     (31)
#define HISP_DNYMEM_MAP_TYPE     (0)
#define HISP_ONLINE_RANMAP_TYPE  (1)
#define HISP_OFFLINE_RANMAP_TYPE (2)

#define HISP_DYM_ONLINE_TYPE     (0)
#define HISP_DYM_OFFLINE_TYPE    (1)
#define HISP_DYM_MAX_TYPE        (2)

#ifdef CONFIG_MM_LB
#define IOMMU_MASK (~(IOMMU_READ | IOMMU_WRITE | IOMMU_CACHE | IOMMU_NOEXEC \
		| IOMMU_MMIO | IOMMU_DEVICE | IOMMU_EXEC | IOMMU_PORT_MASK))
#else
#define IOMMU_MASK (~(IOMMU_READ | IOMMU_WRITE | IOMMU_CACHE | IOMMU_NOEXEC \
		| IOMMU_MMIO | IOMMU_DEVICE | IOMMU_EXEC))
#endif

struct hisp_mem_pool_info_list_s {
	unsigned int type;
	unsigned int iova;             /* for r8 genpool free */
	unsigned int iova_size;        /* for r8 genpool free */
	unsigned int addr;             /* for r8 mem unmap */
	unsigned int size;             /* for r8 mem unmap */
	unsigned long r8_addr;    /* for r8 mem unmap */
	unsigned long r8_size;    /* for r8 mem unmap */
	unsigned long r8_core_addr;    /* for r8 mem unmap */
	unsigned long r8_core_size;    /* for r8 mem unmap */
	unsigned long ispfw_addr;      /* for ispcore mem unmap */
	unsigned long ispfw_size;      /* for ispcore mem unmap */
	struct sg_table *ap_table;
	struct sg_table *fw_table;
	struct sg_table *ispfw_table;
	struct dma_buf *buf;
	struct dma_buf_attachment *attachment;
	struct list_head list;
};

struct hisp_mem_pool_info_s {
	unsigned int addr;
	unsigned int size;
	unsigned int prot;
	unsigned int enable;
	unsigned int mem_map_flag;
	unsigned int memmap_type;
	unsigned int iova_start;
	unsigned int iova_end;
	struct hisp_mem_pool_info_list_s *node;
	struct gen_pool *isp_mem_pool;
};

struct hisp_mem_pool_s {
	unsigned int count;
	struct hisp_mem_pool_info_s isp_pool_info[ISP_MEM_POOL_NUM];
	struct hisp_mem_pool_info_s iova_pool_info;
	struct mutex mem_pool_mutex;
	struct mutex hisp_map_mutex;
	struct mutex hisp_iova_mutex;
	struct mutex hisp_list_mutex;
	struct mutex hisp_mem_mutex;
};

struct hisp_separate_mem_info_s {
	size_t mem_iova;
	unsigned int cpu_size;
	struct dma_buf *dmabuf;
	struct dma_buf_attachment *attach;
	struct sg_table *sgt_cpu;
	unsigned int map_size;
};

struct hisp_separate_mem_s {
	struct hisp_separate_mem_info_s mem_info[HISP_DYM_MAX_TYPE];
};

static struct hisp_separate_mem_s g_separate_mem_info;

static struct hisp_mem_pool_s g_hisp_mem_pool_info;

static unsigned int hisp_memory_map(struct scatterlist *sgl, phys_addr_t paddr,
			size_t addr, size_t size, unsigned int prot)
{
	struct hisp_mem_pool_s *dev = &g_hisp_mem_pool_info;
	struct iommu_domain *domain = NULL;
	size_t phy_len = 0;
	ssize_t sphy_len = 0;
	struct device *subdev = NULL;
	unsigned long map_size;

	subdev = hisp_get_device();
	if (subdev == NULL) {
		pr_err("%s: platform dev is NULL!\n", __func__);
		return 0;
	}

	domain = iommu_get_domain_for_dev(subdev);
	if (domain == NULL) {
		pr_err("%s: domain is NULL!\n", __func__);
		return 0;
	}
	/* Now iommu_map_sg can't support more prot, only w+r */

#ifdef CONFIG_MM_LB
	prot = prot & (IOMMU_PORT_MASK | IOMMU_READ | IOMMU_WRITE);
#else
	prot = prot & (IOMMU_READ | IOMMU_WRITE);
#endif
#ifdef ISP_LINX_CPU_SUPPORT
	prot |= IOMMU_PRIV;
#endif
	pr_debug("%s : map addr.0x%lx, size.0x%lx, prot.0x%x\n",
		__func__, addr, size, prot);

	mutex_lock(&dev->hisp_map_mutex);
	if ((paddr == HISP_DNYMEM_MAP_TYPE) || (paddr == HISP_OFFLINE_RANMAP_TYPE)) {
		phy_len = iommu_map_sg(domain, addr, sgl, (unsigned int)sg_nents(sgl), (int)prot);
		if (phy_len != size) {
			pr_err("%s: iommu map sg failed! phy_len.0x%lx, size.0x%lx\n",
				__func__, phy_len, size);
			goto map_err;
		}
	} else if (paddr == HISP_ONLINE_RANMAP_TYPE) {
		addr = mm_iommu_map_sg(subdev, sgl, (int)prot, &map_size);
		if (addr == 0 || size != map_size) {
			pr_err("%s: mm iommu map sg failed! addr.0x%lx\n", __func__, addr);
			pr_err("%s: mm iommu map sg failed! size.0x%lx\n", __func__, size);
			pr_err("%s: mm iommu map sg failed! map_size.0x%lx\n", __func__, map_size);
			goto map_err;
		}
	} else {
		sphy_len = iommu_map(domain, addr, paddr, size, (int)prot);
		if (sphy_len != 0) {
			pr_err("%s: iommu map failed! phy_len.0x%lx, size.0x%lx\n",
				__func__, sphy_len, size);
			goto map_err;
		}
	}

	mutex_unlock(&dev->hisp_map_mutex);
	return (unsigned int)addr;
map_err:
	mutex_unlock(&dev->hisp_map_mutex);
	return 0;
}

static int hisp_memory_unmap(size_t addr, size_t size, unsigned int memmap_type)
{
	struct hisp_mem_pool_s *dev = &g_hisp_mem_pool_info;
	struct iommu_domain *domain = NULL;
	size_t phy_len = 0;
	struct device *subdev = NULL;
	int ret;

	subdev = hisp_get_device();
	if (subdev == NULL) {
		pr_err("%s: platform dev is NULL!\n", __func__);
		return 0;
	}

	domain = iommu_get_domain_for_dev(subdev);
	if (domain == NULL) {
		pr_err("%s: domain is NULL!\n", __func__);
		return 0;
	}

	pr_info("%s : unmap addr.0x%lx, size.0x%lx, type.%d\n", __func__, addr, size, memmap_type);

	mutex_lock(&dev->hisp_map_mutex);
	if (memmap_type == HISP_ONLINE_RANMAP_TYPE) {
		pr_debug("%s: online unmap addr.0x%lx, size.0x%lx\n", __func__,
			g_separate_mem_info.mem_info[HISP_DYM_ONLINE_TYPE].mem_iova,
			g_separate_mem_info.mem_info[HISP_DYM_ONLINE_TYPE].map_size);
		ret = mm_iommu_unmap(subdev,
			g_separate_mem_info.mem_info[HISP_DYM_ONLINE_TYPE].mem_iova,
			g_separate_mem_info.mem_info[HISP_DYM_ONLINE_TYPE].map_size);
		if (ret != 0)
			pr_err("%s: mm_iommu_unmap_sg failed\n", __func__);
		mutex_unlock(&dev->hisp_map_mutex);
		return ret;
	}

	if (hisp_smmuv3_mode()) {
		phy_len = mm_iommu_unmap_fast(subdev, addr, size);
	} else {
		if (memmap_type == HISP_OFFLINE_RANMAP_TYPE) {
			addr = g_separate_mem_info.mem_info[HISP_DYM_OFFLINE_TYPE].mem_iova;
			size = g_separate_mem_info.mem_info[HISP_DYM_OFFLINE_TYPE].map_size;
			pr_info("%s : offline unmap addr.0x%lx, size.0x%lx\n",
				__func__, addr, size);
		}
		phy_len = iommu_unmap(domain, addr, size);
	}

	if (phy_len != size) {
		pr_err("%s: iommu_unmap failed: phy_len 0x%lx size 0x%lx\n",
				__func__, phy_len, size);
		mutex_unlock(&dev->hisp_map_mutex);
		return -EINVAL;
	}

	mutex_unlock(&dev->hisp_map_mutex);
	return 0;
}

static void hisp_dump_mempool_info(struct hisp_mem_pool_info_s *pool_info)
{
	struct hisp_mem_pool_info_list_s *node = NULL;
	struct hisp_mem_pool_info_list_s *listnode = NULL;
	unsigned int use_size = 0;

	if (pool_info == NULL) {
		pr_err("%s: the pool_info is NULL!\n", __func__);
		return;
	}

	if (pool_info->enable == 0) {
		pr_err("%s: the pool not enable!\n", __func__);
		return;
	}

	listnode = pool_info->node;
	if (listnode == NULL) {
		pr_err("%s: the listnode is NULL!\n", __func__);
		return;
	}

	list_for_each_entry(node, &listnode->list, list) {/*lint !e64 !e826 */
#ifdef ISP_BUF_PRIVATIZE
		pr_debug("%s: va = 0x%x, size = 0x%x, ispfw va = 0x%x, size = 0x%x\n",
				__func__, node->addr, node->size, node->ispfw_addr, node->ispfw_size);
#else
		pr_debug("%s: va = 0x%x, size = 0x%x\n",
				__func__, node->addr, node->size);
#endif
		use_size += node->size;
	}
	pr_info("%s: use_size.0x%x, size.0x%x\n",
		__func__, use_size, pool_info->size);
}

static unsigned int hisp_find_first_bit(unsigned int num, unsigned int align)
{
	unsigned int index = 0;

	for (index = ISP_MAX_BYTE_BIT_NUM; (1 << index) >= align; index--) {
		if (num & (1 << index))
			return index;
		}
	return 0;
}

static unsigned int hisp_get_align_addr_from_iova(unsigned int iova,
		unsigned int size,
		unsigned int map_size,
		unsigned int align)
{
	unsigned int index = 0;
	unsigned int addr = 0;
	unsigned int end_addr = 0;

	if ((ISP_MAX_NUM_MAGIC - iova) < map_size) {
		pr_err("Invalid iova.0x%x, map_size.0x%x\n", iova, map_size);
		return 0;
	}
	end_addr = iova + map_size;

	index = hisp_find_first_bit(size, align);
	if (index == 0) {
		pr_err("Invalid argument addr.0x%x, size.0x%x, align.0x%x\n",
			addr, size, align);
		return 0;
	}

	addr = iova & (~(unsigned int)((unsigned int)(1 << index) - 1));
	if (iova > addr)
		addr += (1 << index);
	if ((ISP_MAX_NUM_MAGIC - addr) <= size) {
		pr_err("Invalid iova.0x%x, size.0x%x\n", addr, size);
		return 0;
	}
	if (addr > end_addr || ((addr + size) > end_addr)) {
		pr_err("Invalid iova.0x%x, size.0x%x\n", addr, size);
		return 0;
	}

	return addr;
}

static struct sg_table *hisp_get_fd_sg_table(
	int fd,
	struct dma_buf **dmabuf,
	struct dma_buf_attachment **attach)
{
	struct sg_table *sgt = NULL;
	struct device *dev = NULL;

	dev = hisp_get_device();
	if (dev == NULL) {
		pr_err("%s: platform dev.NULL\n", __func__);
		return NULL;
	}

	*dmabuf = dma_buf_get(fd);
	if (IS_ERR_OR_NULL(*dmabuf)) {
		pr_err("%s failed: get dmabuf\n", __func__);
		return NULL;
	}

	*attach = dma_buf_attach(*dmabuf, dev);
	if (IS_ERR_OR_NULL(*attach)) {
		pr_err("%s failed: dma_buf_attach\n", __func__);
		goto err_put;
	}

	sgt = dma_buf_map_attachment(*attach, DMA_BIDIRECTIONAL);
	if (IS_ERR_OR_NULL(sgt)) {
		pr_err("%s failed: dma_buf_map_attachment\n", __func__);
		goto err_detach;
	}

	return sgt;

err_detach:
	dma_buf_detach(*dmabuf, *attach);
err_put:
	dma_buf_put(*dmabuf);

	return NULL;
}

static void hisp_free_fd_sg_table(struct sg_table *sg_table,
	struct dma_buf_attachment *attach,
	struct dma_buf *dmabuf)
{
	if ((sg_table == NULL) || (attach == NULL) || (dmabuf == NULL)) {
		pr_err("%s failed: invalid pare\n", __func__);
		return;
	}

	dma_buf_unmap_attachment(attach, sg_table, DMA_BIDIRECTIONAL);
	dma_buf_detach(dmabuf, attach);
	dma_buf_put(dmabuf);
}

static struct sg_table *hisp_alloc_ion_sg_table(
	unsigned long length,
	struct dma_buf **dmabuf,
	struct dma_buf_attachment **attach)
{
	int fd;
	struct sg_table *sgt = NULL;

#ifdef CONFIG_DMABUF_MM
	fd = dma_heap_alloc("system_heap", length, 0x0);
#else
	fd = ion_alloc(length, 1 << ION_SYSTEM_HEAP_ID, 0x0);
#endif
	if (fd < 0) {
		pr_err("%s failed: alloc ion buffer\n", __func__);
		return NULL;
	}

	sgt = hisp_get_fd_sg_table(fd, dmabuf, attach);

	ksys_close(fd);

	return sgt;
}

#ifndef ISP_BUF_PRIVATIZE

static int hisp_record_memory_map(size_t addr, size_t size, unsigned int prot)
{
	unsigned int i;
	struct hisp_shared_para *share_para = hisp_share_get_para();

	hisp_lock_sharedbuf();
	if (share_para == NULL) {
		pr_err("%s:hisp_share_get_para failed.\n", __func__);
		hisp_unlock_sharedbuf();
		return -1;
	}

	for (i = 0; i < ISP_MEM_POOL_NUM; i++) {
		if (share_para->hisp_mem_record[i].state == HISP_MEM_UNUSED) {
			share_para->hisp_mem_record[i].state = HISP_MEM_MAP;
			share_para->hisp_mem_record[i].da = (unsigned int)addr;
			share_para->hisp_mem_record[i].size = (unsigned int)size;
			share_para->hisp_mem_record[i].prot = prot;
			hisp_unlock_sharedbuf();
			return i;
		}
	}
	pr_err("%s: record memory region failed: size.0x%x\n", __func__, size);
	hisp_unlock_sharedbuf();

	return -1;
}

static void hisp_record_memory_map_append(int index, unsigned int addr, unsigned int size)
{
	struct hisp_shared_para *share_para = hisp_share_get_para();

	if ((index < 0) || (index > ISP_MEM_POOL_NUM - 1)) {
		pr_err("%s failed :index.%d\n", __func__, index);
		return;
	}

	pr_err("%s: index.%d, addr.0x%x, size.0x%x\n", __func__, index, addr, size);

	hisp_lock_sharedbuf();
	if (share_para == NULL) {
		pr_err("%s failed: hisp_share_get_para\n", __func__);
		hisp_unlock_sharedbuf();
		return;
	}

	if (share_para->hisp_mem_record[index].state == HISP_MEM_MAP) {
#ifdef CONFIG_CAMERA_ISPV230
		share_para->hisp_mem_record[index].ispcpu_addr = addr;
		share_para->hisp_mem_record[index].ispcpu_size = size;
#endif
	} else {
		pr_err("%s failed\n", __func__);
	}
	hisp_unlock_sharedbuf();
}

static void hisp_unrecord_memory_map(size_t addr, size_t size)
{
	unsigned int i;
	struct hisp_shared_para *share_para = hisp_share_get_para();

	hisp_lock_sharedbuf();
	if (share_para == NULL) {
		pr_err("%s:hisp_share_get_para failed.\n", __func__);
		hisp_unlock_sharedbuf();
		return;
	}

	for (i = 0; i < ISP_MEM_POOL_NUM; i++) {
		if (share_para->hisp_mem_record[i].state == HISP_MEM_MAP &&
			share_para->hisp_mem_record[i].da == (unsigned int)addr &&
			share_para->hisp_mem_record[i].size == (unsigned int)size) {
			share_para->hisp_mem_record[i].state = HISP_MEM_UNUSED;
#ifdef CONFIG_CAMERA_ISPV230
			share_para->hisp_mem_record[i].ispcpu_addr = 0;
			share_para->hisp_mem_record[i].ispcpu_size = 0;
#endif
			hisp_unlock_sharedbuf();
			return;
		}
	}
	pr_err("%s: unrecord memory region failed: size.0x%x\n", __func__, size);
	hisp_unlock_sharedbuf();
}

static unsigned int hisp_dynamic_memory_map(struct scatterlist *sgl,
			size_t addr, size_t size, unsigned int prot, unsigned int map_type)
{
	if (sgl == NULL) {
		pr_err("%s: sgl is NULL!\n", __func__);
		return 0;
	}
	if ((IOMMU_MASK & prot) != 0) {
		pr_err("iommu attr error.0x%x\n", prot);
		return 0;
	}

	return hisp_memory_map(sgl, map_type, addr, size, prot);
}

static unsigned int hisp_get_size_align_mask(unsigned int align)
{
	unsigned int mask = ISP_MAX_NUM_MAGIC;
	unsigned int mask_num = 0;

	for (mask_num = 0; mask_num < 32; mask_num++) {
		if ((0x1 & (align >> mask_num)) == 1)
			return mask;

		mask &= ~(1 << mask_num);/*lint !e747 !e701 !e502 */
	}

	return 0;
}

static unsigned long hisp_alloc_iova(struct gen_pool *pool,
		unsigned int size, unsigned long align)
{
	unsigned long iova = 0;
	unsigned long tmp;

	if (pool == NULL)
		return 0;

	iova = gen_pool_alloc(pool, (unsigned long)size);
	if (iova == 0) {
		pr_err("gen_pool_alloc failed!\n");
		return 0;
	}

	tmp = 1 << (unsigned long)(pool->min_alloc_order);/*lint !e571 !e647 */
	if (align > tmp)
		pr_info("can't align to 0x%lx\n", align);
	return iova;
}

static void hisp_free_iova(struct gen_pool *pool,
		unsigned long iova, size_t size)
{
	struct hisp_mem_pool_s *dev = &g_hisp_mem_pool_info;
	(void)dev;
	gen_pool_free(pool, iova, size);
}

static struct sg_table *hisp_alloc_dynmem_sg_table_separate(
	unsigned int ispcpu_size, unsigned int type)
{
	struct hisp_separate_mem_s *dev = &g_separate_mem_info;
	struct sg_table *sgt_cpu_loc;
	unsigned int index;
	struct dma_buf *dmabuf = NULL;
	struct dma_buf_attachment *attach = NULL;

	index = (type == MAP_TYPE_DYNAMIC_ALLOC_BY_KERNEL) ? HISP_DYM_ONLINE_TYPE : HISP_DYM_OFFLINE_TYPE;

	if (dev->mem_info[index].sgt_cpu != NULL) {
		pr_err("%s failed: separate buffer type.%d exist\n", __func__, type);
		return NULL;
	}

	sgt_cpu_loc = hisp_alloc_ion_sg_table(ispcpu_size, &dmabuf, &attach);
	if (sgt_cpu_loc != NULL) {
		dev->mem_info[index].dmabuf = dmabuf;
		dev->mem_info[index].attach = attach;
		dev->mem_info[index].sgt_cpu = sgt_cpu_loc;
		dev->mem_info[index].cpu_size = ispcpu_size;
		pr_info("%s : separate buffer type.%d alloc succ\n", __func__, type);
	}

	return sgt_cpu_loc;
}

static void hisp_free_dynmem_sg_table_separate(unsigned int type)
{
	unsigned int index;
	struct hisp_separate_mem_s *dev = &g_separate_mem_info;

	index = (type == MAP_TYPE_DYNAMIC_ALLOC_BY_KERNEL) ? HISP_DYM_ONLINE_TYPE : HISP_DYM_OFFLINE_TYPE;

	if (dev->mem_info[index].sgt_cpu == NULL) {
		pr_err("%s failed: invalid para\n", __func__);
		return;
	}

	hisp_free_fd_sg_table(dev->mem_info[index].sgt_cpu, dev->mem_info[index].attach, dev->mem_info[index].dmabuf);

	dev->mem_info[index].sgt_cpu = NULL;
	dev->mem_info[index].attach = NULL;
	dev->mem_info[index].dmabuf = NULL;

	pr_info("%s : separate buffer type.%d free\n", __func__, type);
}

static unsigned int hisp_check_mempool_enable(unsigned int pool_num)
{
	struct hisp_mem_pool_s *dev = &g_hisp_mem_pool_info;
	struct hisp_mem_pool_info_s *pool_info = &dev->isp_pool_info[pool_num];

	if (pool_info != NULL)
		return pool_info->enable;
	else
		return 1;
}

static int hisp_free_addr_to_iova_mempool(unsigned int addr, unsigned int size)
{
	struct hisp_mem_pool_s *info = &g_hisp_mem_pool_info;
	struct hisp_mem_pool_info_s *dev = NULL;
	struct hisp_mem_pool_info_list_s *listnode = NULL;
	struct hisp_mem_pool_info_list_s *node = NULL;
	struct hisp_mem_pool_info_list_s *node_temp = NULL;

	dev = &info->iova_pool_info;
	listnode = dev->node;
	if ((dev->enable != 1) || (listnode == NULL)
		|| (dev->isp_mem_pool == NULL)) {
		pr_err("iova mem pool didn't creat\n");
		return -ENXIO;
	}
	if ((addr < dev->iova_start) || (addr >= dev->iova_end) ||
		(size > (dev->iova_end - addr))) {
		pr_err("Invalid argument addr.0x%x, size.0x%x\n", addr, size);
		return -EINVAL;
	}
	mutex_lock(&info->hisp_iova_mutex);
	list_for_each_entry_safe(node, node_temp, &listnode->list, list) {
		if (node->addr == addr) {
			gen_pool_free(dev->isp_mem_pool,
				node->iova, node->iova_size);
			list_del(&node->list);
			pr_info("[ISP Mem]: va = 0x%x, size = 0x%x\n",
				node->iova, node->iova_size);
			kfree(node);
			mutex_unlock(&info->hisp_iova_mutex);
			return 0;
		}
	}
	mutex_unlock(&info->hisp_iova_mutex);
	return -ENOMEM;
}

static unsigned int hisp_get_addr_form_iova_mempool(unsigned int size,
					unsigned int align)
{
	struct hisp_mem_pool_s *info = &g_hisp_mem_pool_info;
	struct hisp_mem_pool_info_s *dev = NULL;
	struct hisp_mem_pool_info_list_s *listnode = NULL;
	struct hisp_mem_pool_info_list_s *node = NULL;
	unsigned int map_size = 0;
	unsigned int iova = 0;
	unsigned int addr = 0;

	dev = &info->iova_pool_info;
	listnode = dev->node;

	if ((dev->enable != 1) || (listnode == NULL)
		|| (dev->isp_mem_pool == NULL)) {
		pr_err("iova mem pool didn't creat\n");
		return 0;
	}
	if ((size == 0) || (size > (dev->iova_end - dev->iova_start) / 2)) {
		pr_err("Invalid argument size.0x%x\n", size);
		return 0;
	}
	mutex_lock(&info->hisp_iova_mutex);
/* double size to find a align addr */
	map_size = PAGE_ALIGN(size * 2);/*lint !e50 */
	iova = gen_pool_alloc(dev->isp_mem_pool, (unsigned long)map_size);
	if (iova == 0) {
		pr_err("gen_pool_alloc failed, map_size.0x%x, size.0x%x\n",
			map_size, size);
		hisp_dump_mempool_info(dev);
		mutex_unlock(&info->hisp_iova_mutex);
		return 0;
	}

	addr = hisp_get_align_addr_from_iova(iova, size, map_size, align);
	if (addr == 0) {
		pr_err("get align addr failed, iova.0x%x, size.0x%x\n",
			iova, size);
		goto get_align_addr_fail;
	}

	node = kzalloc(sizeof(struct hisp_mem_pool_info_list_s), GFP_KERNEL);
	if (node == NULL)
		goto get_align_addr_fail;

	node->addr = addr;/*lint !e613 */
	node->size = size;/*lint !e613 */
	node->iova = iova;/*lint !e613 */
	node->iova_size = map_size;/*lint !e613 */
	list_add_tail(&node->list, &listnode->list);/*lint !e613 */
	mutex_unlock(&info->hisp_iova_mutex);

	return addr;

get_align_addr_fail:
	gen_pool_free(dev->isp_mem_pool, iova, map_size);
	mutex_unlock(&info->hisp_iova_mutex);
	return 0;
}

static void hisp_sec_unmap_mem(struct hisp_mem_pool_info_s *pool_info)
{
	struct isp_a7mapping_s *map_info = NULL;

	if (pool_info == NULL) {
		pr_err("%s: pool_info is NULL\n",
				__func__);
		return;
	}

	if (hisp_sec_boot_mode()) {
		if (!hisp_ca_boot_mode()) {
			map_info = hisp_get_ap_dyna_mapping();

			map_info->a7va = pool_info->addr;
			map_info->size = pool_info->size;

			hisp_smc_ispcpu_unmap();
		}
	}
}

static void hisp_mempool_list_clean(struct hisp_mem_pool_info_list_s *listnode,
			struct hisp_mem_pool_info_s *pool_info)
{
	struct hisp_mem_pool_info_list_s *node = NULL;
	struct hisp_mem_pool_info_list_s *node_temp = NULL;
	struct hisp_mem_pool_s *dev = &g_hisp_mem_pool_info;
	(void)dev;

	if ((pool_info == NULL) || (listnode == NULL)) {
		pr_err("%s: pool_info is %pK, pool_info is %pK\n",
			__func__, pool_info, listnode);
		return;
	}

	list_for_each_entry_safe(node, node_temp, &listnode->list, list) {
		hisp_free_iova(pool_info->isp_mem_pool,
			(unsigned long)node->addr, (unsigned long)node->size);
		list_del(&node->list);
		pr_err("%s: va = 0x%x, size = 0x%x\n",
				__func__, node->addr, node->size);
		kfree(node);
	}
}

static void hisp_mempool_destroy(unsigned int pool_num)
{
	struct hisp_mem_pool_s *dev = &g_hisp_mem_pool_info;
	struct hisp_mem_pool_info_s *pool_info = &dev->isp_pool_info[pool_num];
	struct hisp_mem_pool_info_list_s *listnode = pool_info->node;

	int ret = 0;

	mutex_lock(&dev->mem_pool_mutex);
	if (pool_info->enable == 0) {
		pr_err("%s: the pool_num %d not creat or had been disable\n",
				__func__, pool_num);
		mutex_unlock(&dev->mem_pool_mutex);
		return;
	}
	hisp_sec_unmap_mem(pool_info);

	if (hisp_smmuv3_mode()) {
		if (hisp_nsec_boot_mode()) {
			ret = hisp_free_cpu_map_addr(pool_info->addr,
						     pool_info->size,
						     pool_info->memmap_type);
			if (ret < 0) {
				pr_err("%s Fail: free ispcpu addr\n", __func__);
				mutex_unlock(&dev->mem_pool_mutex);
				return;
			}
		}
	} else {
		if (pool_info->mem_map_flag) {
			ret = hisp_memory_unmap(pool_info->addr,
						(size_t)pool_info->size,
						pool_info->memmap_type);
			if (ret < 0) {
				pr_err("%s failed: hisp_memory_unmap\n", __func__);
				mutex_unlock(&dev->mem_pool_mutex);
				return;
			}
			if (pool_info->memmap_type == HISP_ONLINE_RANMAP_TYPE)
				hisp_free_dynmem_sg_table_separate(MAP_TYPE_DYNAMIC_ALLOC_BY_KERNEL);
			else if (pool_info->memmap_type == HISP_OFFLINE_RANMAP_TYPE)
				hisp_free_dynmem_sg_table_separate(MAP_TYPE_OFFLINE_ALLOC_BY_KERNEL);

			pool_info->mem_map_flag = 0;
		}
	}
	hisp_mempool_list_clean(listnode, pool_info);
	kfree(listnode);

	if (dev->count > 0)
		dev->count--;
	else
		pr_err("[%s] mempool destroy pool num = %d, pool count.%d\n",
			__func__, pool_num, dev->count);

	if (pool_info->isp_mem_pool != NULL) {
		gen_pool_destroy(pool_info->isp_mem_pool);
		if ((!hisp_smmuv3_mode()) && hisp_nsec_boot_mode())
			hisp_unrecord_memory_map(pool_info->addr, pool_info->size);

		pool_info->isp_mem_pool = NULL;
	}
	pool_info->enable = 0;
	pr_info("[ISP Mem] mempool destroy pool num = %d, pool count.%d\n",
			pool_num, dev->count);

	mutex_unlock(&dev->mem_pool_mutex);
}

static int hisp_alloc_cpu_align_addr(struct scatterlist *sgl,
			unsigned int pool_num,
			unsigned int iova,
			unsigned int size,
			unsigned int prot,
			unsigned int align,
			unsigned int type)
{
	struct isp_a7mapping_s *map_info = NULL;
	void *addr = NULL;
	unsigned int pool_addr = 0;
	unsigned int align_mask = 0;
	int ret = 0;
	int index = 0;

	if (hisp_sec_boot_mode()) {
		map_info = hisp_get_ap_dyna_mapping();
		switch (pool_num) {
		case 0:
			pool_addr = 0xC8000000;
			break;
		case 1:
			pool_addr = 0xD0000000;
			break;
		case 2:
			pool_addr = 0xC4000000;
			break;
		default:
			pr_err("%s: wrong num.%d\n", __func__, pool_num);
			return 0;
		}

		addr = hisp_get_dyna_array();
		map_info->a7va = pool_addr;
		map_info->size = size;
		map_info->prot = prot;

		pr_debug("[%s] pool_num.%d, iova.0x%x, size.0x%x, prot.0x%x\n",
			__func__, pool_num, map_info->a7va, size, prot);

		hisp_sec_set_ispcpu_palist(addr, sgl, size);
		ret = hisp_smc_ispcpu_map();
		if (ret < 0) {
			pr_err("%s:hisp_smc_ispcpu_map fail.%d\n", __func__, ret);
			return 0;
		}
	} else {
		pool_addr = iova;

		align_mask = hisp_get_size_align_mask(align);
		if (pool_addr > (pool_addr & align_mask)) {
			pool_addr &= align_mask;
			pool_addr += align;
		}
		index = hisp_record_memory_map(pool_addr, size, prot);
		if (type == MAP_TYPE_DYNAMIC_ALLOC_BY_KERNEL)
			hisp_record_memory_map_append(index,
				size - g_separate_mem_info.mem_info[HISP_DYM_ONLINE_TYPE].cpu_size \
				+ g_separate_mem_info.mem_info[HISP_DYM_ONLINE_TYPE].mem_iova,
				g_separate_mem_info.mem_info[HISP_DYM_ONLINE_TYPE].cpu_size);
		else if (type == MAP_TYPE_OFFLINE_ALLOC_BY_KERNEL)
			hisp_record_memory_map_append(index,
				size - g_separate_mem_info.mem_info[HISP_DYM_OFFLINE_TYPE].cpu_size \
				+ g_separate_mem_info.mem_info[HISP_DYM_OFFLINE_TYPE].mem_iova,
				g_separate_mem_info.mem_info[HISP_DYM_OFFLINE_TYPE].cpu_size);
	}
	return pool_addr;
}

static unsigned int hisp_alloc_cpu_addr(struct scatterlist *sgl,
					 unsigned int iova,
					 unsigned int map_size,
					 unsigned int prot,
					 unsigned int pool_num,
					 unsigned int align,
					 unsigned int type)
{
	unsigned int pool_addr = 0;
	unsigned int ispcpu_vaddr = iova;

	if (!hisp_smmuv3_mode()) {
		if (type == MAP_TYPE_RAW2YUV)  {
			ispcpu_vaddr = hisp_dynamic_memory_map(sgl,
				MEM_RAW2YUV_DA, (size_t)map_size, prot, HISP_DNYMEM_MAP_TYPE);
			if (ispcpu_vaddr == 0) {
				pr_err("[%s] Failed : hisp_dynamic_memory_map!\n", __func__);
				return 0;
			}
		} else if (type == MAP_TYPE_DYNAMIC_ALLOC_BY_KERNEL) {
			ispcpu_vaddr = hisp_dynamic_memory_map(sgl,
				0, (size_t)map_size, prot, HISP_ONLINE_RANMAP_TYPE);
			if (ispcpu_vaddr == 0) {
				pr_err("[%s] Failed : hisp_dynamic_memory_map online\n", __func__);
				return 0;
			}
			g_separate_mem_info.mem_info[HISP_DYM_ONLINE_TYPE].mem_iova = ispcpu_vaddr;
			g_separate_mem_info.mem_info[HISP_DYM_ONLINE_TYPE].map_size = (unsigned int)map_size;

			pr_debug("%s: ispcpu_vaddr.0x%lx\n", __func__, ispcpu_vaddr);
		} else if (type == MAP_TYPE_OFFLINE_ALLOC_BY_KERNEL) {
			ispcpu_vaddr = hisp_dynamic_memory_map(sgl,
				MEM_RAW2YUV_DA, (size_t)map_size, prot, HISP_OFFLINE_RANMAP_TYPE);
			if (ispcpu_vaddr == 0) {
				pr_err("[%s] Failed : hisp_dynamic_memory_map offline\n", __func__);
				return 0;
			}
			g_separate_mem_info.mem_info[HISP_DYM_OFFLINE_TYPE].mem_iova = ispcpu_vaddr;
			g_separate_mem_info.mem_info[HISP_DYM_OFFLINE_TYPE].map_size = (unsigned int)map_size;

			pr_debug("%s: ispcpu_vaddr.0x%lx\n", __func__, ispcpu_vaddr);
		}
		pool_addr = hisp_alloc_cpu_align_addr(sgl, pool_num,
				ispcpu_vaddr, map_size, prot, align, type);
		pr_debug("%s: pool_addr.0x%lx\n", __func__, pool_addr);
	} else {
		if (hisp_nsec_boot_mode()) {
			pool_addr = hisp_alloc_cpu_map_addr(sgl, prot,
					map_size, align);
		} else {
			pool_addr = hisp_pool_mem_addr(pool_num);
		}
	}
	if (pool_addr == 0x0) {
		pr_err("[%s] alloc addr failed!.%d\n",
			__func__, hisp_smmuv3_mode());
		return 0;
	}

	return pool_addr;

}

static void hisp_free_cpu_addr(size_t iova,
			size_t size, unsigned int type)
{
	int ret = 0;

	if (!hisp_smmuv3_mode()) {
		if ((type == MAP_TYPE_RAW2YUV) || (type == MAP_TYPE_OFFLINE_ALLOC_BY_KERNEL)) {
			ret = hisp_memory_unmap(iova, size, HISP_DNYMEM_MAP_TYPE);
			if (ret < 0)
				pr_err("[%s] Failed : hisp_memory_unmap!\n", __func__);
		} else if (type == MAP_TYPE_DYNAMIC_ALLOC_BY_KERNEL) {
			ret = hisp_memory_unmap(iova, size, HISP_ONLINE_RANMAP_TYPE);
			if (ret < 0)
				pr_err("[%s] Failed : hisp_memory_unmap!\n", __func__);
		}

		hisp_unrecord_memory_map(iova, size);
	} else if (hisp_nsec_boot_mode()) {
		ret = hisp_free_cpu_map_addr(iova, size, HISP_DNYMEM_MAP_TYPE);
		if (ret < 0)
			pr_err("[%s] Failed : hisp_free_cpu_map_addr!\n", __func__);
	}
}
static int hisp_set_pool_info(struct hisp_mem_pool_info_s *pool_info,
			struct gen_pool *pool,
			unsigned int pool_addr,
			unsigned int map_size,
			unsigned int prot,
			unsigned int type)
{
	struct hisp_mem_pool_info_list_s *listnode = NULL;

	if ((pool_info == NULL) || (pool == NULL)) {
		pr_err("%s: alloc listnode fail\n", __func__);
		return -ENOMEM;
	}

	listnode = kzalloc(sizeof(struct hisp_mem_pool_info_list_s), GFP_KERNEL);
	if (listnode == NULL)
		return -ENOMEM;

	INIT_LIST_HEAD(&listnode->list);
	pool_info->node = listnode;
	pool_info->addr = pool_addr;
	pool_info->size = map_size;
	pool_info->prot = prot;
	pool_info->isp_mem_pool = pool;
	pool_info->enable = 1;
	if (!hisp_smmuv3_mode()) {
		if (type == MAP_TYPE_RAW2YUV) {
			pool_info->mem_map_flag = 1;
			pool_info->memmap_type = HISP_DNYMEM_MAP_TYPE;
		} else if (type == MAP_TYPE_DYNAMIC_ALLOC_BY_KERNEL) {
			pool_info->mem_map_flag = 1;
			pool_info->memmap_type = HISP_ONLINE_RANMAP_TYPE;
		} else if (type == MAP_TYPE_OFFLINE_ALLOC_BY_KERNEL) {
			pool_info->mem_map_flag = 1;
			pool_info->memmap_type = HISP_OFFLINE_RANMAP_TYPE;
		}
	}

	return 0;
}

static void *hisp_gen_pool_create(unsigned int pool_addr, unsigned int map_size)
{
	struct gen_pool *pool = NULL;
	int ret = 0;

	pool = gen_pool_create((int)(order_base_2(ISP_MEM_POOL_ALIGN)), -1);
	if (pool == NULL) {
		pr_err("Create isp gen pool failed!\n");
		return NULL;
	}

	ret = gen_pool_add(pool, (unsigned long)pool_addr,
			  (unsigned long)map_size, -1);
	if (ret) {
		pr_err("Gen pool add failed!\n");
		gen_pool_destroy(pool);
		return NULL;
	}

	return  pool;
}

static unsigned int hisp_mempool_setup(struct scatterlist *sgl,
					 unsigned int iova,
					 unsigned int size,
					 unsigned int prot,
					 unsigned int pool_num,
					 unsigned int align,
					 unsigned int type)
{
	struct hisp_mem_pool_s *dev = &g_hisp_mem_pool_info;
	struct hisp_mem_pool_info_s *pool_info = NULL;
	struct gen_pool *pool = NULL;
	unsigned int pool_addr = 0;
	unsigned int map_size = 0;
	unsigned int pool_size = 0;
	int ret = 0;

	mutex_lock(&dev->mem_pool_mutex);
	dev->count++;
	pool_info = &dev->isp_pool_info[pool_num];
	if ((dev->count > ISP_MEM_POOL_NUM) || (pool_info->enable == 1)) {
		pr_err("[%s] The %d mem pool had ben enabled, count.%d!\n",
			__func__, pool_num, dev->count);
		dev->count--;
		mutex_unlock(&dev->mem_pool_mutex);
		return 0;
	}

	ret = memset_s(pool_info, sizeof(struct hisp_mem_pool_info_s),
		 0, sizeof(struct hisp_mem_pool_info_s));/*lint !e838 */
	if (ret < 0)
		pr_err("[%s] hisp_mem_pool_info_s to 0 fail.%d\n", __func__, ret);

	map_size = PAGE_ALIGN(size);/*lint !e50 */

	pool_addr = hisp_alloc_cpu_addr(sgl, iova, map_size,
				prot, pool_num, align, type);
	if (pool_addr == 0x0) {
		pr_err("%s: alloc addr failed!\n", __func__);
		dev->count--;
		mutex_unlock(&dev->mem_pool_mutex);
		return 0;
	}

	if (type == MAP_TYPE_DYNAMIC_ALLOC_BY_KERNEL)
		pool_size = map_size - g_separate_mem_info.mem_info[HISP_DYM_ONLINE_TYPE].cpu_size \
			+ g_separate_mem_info.mem_info[HISP_DYM_ONLINE_TYPE].mem_iova - pool_addr;
	else if (type == MAP_TYPE_OFFLINE_ALLOC_BY_KERNEL)
		pool_size = map_size - g_separate_mem_info.mem_info[HISP_DYM_OFFLINE_TYPE].cpu_size \
			+ g_separate_mem_info.mem_info[HISP_DYM_OFFLINE_TYPE].mem_iova - pool_addr;
	else
		pool_size = map_size;

	pool = hisp_gen_pool_create(pool_addr, pool_size);
	if (pool == NULL) {
		pr_err("%s Failed: hisp_gen_pool_create!\n", __func__);
		goto gen_pool_creat_fail;
	}

	ret = hisp_set_pool_info(pool_info, pool, pool_addr,
			pool_size, prot, type);
	if (ret < 0) {
		pr_err("%s: hisp_set_pool_info failed!\n", __func__);
		goto set_pool_info_fail;
	}
	pr_debug("[%s]: mempool setup addr.0x%x, size.0x%x\n",  __func__, pool_addr, pool_size);
	pr_info("[%s]: mempool setup num.%d, count.%d\n", __func__, pool_num, dev->count);

	mutex_unlock(&dev->mem_pool_mutex);
	return pool_addr;

set_pool_info_fail:
	gen_pool_destroy(pool);
gen_pool_creat_fail:
	hisp_free_cpu_addr(pool_addr, map_size, type);
	dev->count--;
	mutex_unlock(&dev->mem_pool_mutex);
	return 0;
}

void hisp_dynamic_mem_pool_clean(void)
{
	struct hisp_mem_pool_s *dev = &g_hisp_mem_pool_info;
	struct hisp_mem_pool_info_s *pool_info = NULL;
	struct hisp_mem_pool_info_list_s *node = NULL;
	struct hisp_mem_pool_info_list_s *node_temp = NULL;
	struct hisp_mem_pool_info_list_s *listnode = NULL;
	int ret = -1;
	unsigned int index = 0;

	for (index = 0; index < ISP_MEM_POOL_NUM; index++) {
		pool_info = &dev->isp_pool_info[index];
		if (pool_info == NULL)
			break;

		if (pool_info->enable == 1)
			hisp_mempool_destroy(index);
	}
	pool_info = &dev->iova_pool_info;
	if (pool_info == NULL) {
		pr_err("%s: iova_pool_info is NULL\n", __func__);
		return;
	}
	listnode = pool_info->node;
	if (listnode == NULL) {
		pr_err("%s: listnode is NULL\n", __func__);
		return;
	}

	mutex_lock(&dev->hisp_iova_mutex);
	list_for_each_entry_safe(node, node_temp, &listnode->list, list) {
		ret = hisp_memory_unmap(node->addr, node->size, pool_info->memmap_type);
		if (ret < 0)
			pr_err("[%s]Fail: memory_unmap, size.0x%x\n",
				__func__, node->size);
		hisp_unrecord_memory_map(node->addr, node->size);
		gen_pool_free(pool_info->isp_mem_pool, node->iova, node->iova_size);
		list_del(&node->list);
		pr_info("[ISP Mem Clean]: size = 0x%x\n", node->iova_size);
		kfree(node);
	}
	mutex_unlock(&dev->hisp_iova_mutex);
}

unsigned int hisp_alloc_cpu_map_addr(struct scatterlist *sgl,
				unsigned int prot,
				unsigned int size,
				unsigned int align)
{
	unsigned int iova = 0;
	unsigned int da = 0;
	int ret = 0;

	da = hisp_get_addr_form_iova_mempool(size, align);
	if (da == 0) {
		pr_err("[ISP Mem] Fail: dyn get iova, size.0x%x, align.%d\n",
			size, align);
		return 0;
	}
	iova = hisp_dynamic_memory_map(sgl, da, (size_t)size, prot, HISP_DNYMEM_MAP_TYPE);
	if (iova == 0) {
		pr_err("[ISP Mem] Fail:hisp_dynamic_memory_map,da.0x%pK, size.0x%x\n",
				da, size);
		ret = hisp_free_addr_to_iova_mempool(da, size);
		if (ret < 0)
			pr_err("[ISP] Fail: free addr.0x%pK, size.0x%x\n",
				da, size);
	}

	hisp_record_memory_map(iova, size, prot);
	pr_debug("[ISP Mem] alloc cpu map mem from dynamic.0x%x\n", iova);
	return iova;
}
EXPORT_SYMBOL(hisp_alloc_cpu_map_addr);/*lint !e580 !e546 */

int hisp_free_cpu_map_addr(unsigned int iova,
				unsigned int size,
				unsigned int memmap_type)
{
	int ret = 0;

	pr_debug("[ISP Mem] free cpu map mem addr.0x%x,size.0x%x\n", iova, size);

	ret = hisp_free_addr_to_iova_mempool(iova, size);
	if (ret < 0) {
		pr_err("[%s]Fail:free iova.0x%x,ret.%d\n", __func__, iova, ret);
		return ret;
	}

	ret = hisp_memory_unmap(iova, size, memmap_type);
	if (ret < 0)
		pr_err("[%s]Fail: hisp_memory_unmap, iova.0x%x, size.0x%x\n",
			__func__, iova, size);

	hisp_unrecord_memory_map(iova, size);
	return ret;
}
EXPORT_SYMBOL(hisp_free_cpu_map_addr);/*lint !e580 !e546 */

unsigned long hisp_mem_pool_alloc_iova(
		unsigned int size, unsigned int pool_num)
{
	struct hisp_mem_pool_s *dev = &g_hisp_mem_pool_info;
	struct hisp_mem_pool_info_list_s *node = NULL;
	struct hisp_mem_pool_info_s *pool_info = NULL;
	struct hisp_mem_pool_info_list_s *listnode = NULL;

	unsigned int map_size = 0;
	unsigned long iova;

	if (size == 0) {
		pr_err("%s: invalid para, size == 0x%x.\n", __func__, size);
		return 0;
	}

	if (pool_num >= ISP_MEM_POOL_NUM) {
		pr_err("%s: wrong num, num : 0x%x\n", __func__, pool_num);
		return 0;
	}
	pr_debug("[ISP] alloc mem, pool_num.%d, size.0x%x\n", pool_num, size);
	pool_info = &dev->isp_pool_info[pool_num];
	mutex_lock(&dev->mem_pool_mutex);
	if (pool_info->enable == 0) {
		pr_err("%s: the pool_num %d not creat!\n", __func__, pool_num);
		mutex_unlock(&dev->mem_pool_mutex);
		return 0;
	}

	listnode = pool_info->node;
	map_size = PAGE_ALIGN(size);/*lint !e50 */

	iova = hisp_alloc_iova(pool_info->isp_mem_pool, map_size, 0);
	if (iova == 0) {
		pr_err("[%s]Failed:iova.0x%lx,num.%d,size.0x%x,map_size.0x%x\n",
				__func__, iova, pool_num, size, map_size);
		hisp_dump_mempool_info(pool_info);
		mutex_unlock(&dev->mem_pool_mutex);
		return 0;
	}

	node = kzalloc(sizeof(struct hisp_mem_pool_info_list_s), GFP_KERNEL);
	if (node == NULL) {
		hisp_free_iova(pool_info->isp_mem_pool,
				iova, (unsigned long)map_size);
		mutex_unlock(&dev->mem_pool_mutex);
		return 0;
	}

	node->addr = (unsigned int)iova;
	node->size = map_size;
	list_add_tail(&node->list, &listnode->list);
	pr_debug("[ISP Mem]:pool.num.%d, iova.0x%lx, size.0x%x, map_size.0x%x\n",
			pool_num, iova, size, map_size);
	mutex_unlock(&dev->mem_pool_mutex);
	return iova;
}
EXPORT_SYMBOL(hisp_mem_pool_alloc_iova);/*lint !e580 !e546 */

int hisp_mem_pool_free_iova(unsigned int pool_num,
					unsigned int va, unsigned int size)
{
	struct hisp_mem_pool_s *dev = &g_hisp_mem_pool_info;
	struct hisp_mem_pool_info_list_s *node = NULL;
	struct hisp_mem_pool_info_list_s *temp = NULL;
	struct hisp_mem_pool_info_s *pool_info = NULL;
	struct hisp_mem_pool_info_list_s *listnode = NULL;
	unsigned int map_size;

	if (size == 0) {
		pr_err("%s: invalid para, size == 0x%x.\n", __func__, size);
		return -EINVAL;
	}

	if (pool_num >= ISP_MEM_POOL_NUM) {
		pr_err("Mem pool num wrong!\n");
		return -EBADF;
	}

	pool_info = &dev->isp_pool_info[pool_num];
	mutex_lock(&dev->mem_pool_mutex);
	if (pool_info->enable == 0) {
		pr_err("%s: the pool_num %d not creat!\n", __func__, pool_num);
		mutex_unlock(&dev->mem_pool_mutex);
		return -EPERM;
	}

	pr_debug("[ISP Mem] free mem, pool_num.%d, va.0x%x, size.0x%x\n",
		pool_num, va, size);
	listnode = pool_info->node;
	map_size = PAGE_ALIGN(size);/*lint !e50 */
	if ((pool_info->addr <= va)
		&& (va < (pool_info->addr + pool_info->size))) {
		list_for_each_entry_safe(node, temp, &listnode->list, list) {
			if (node->addr == va) {
				pr_debug("[ISP Mem] free mem : va.0x%x, size.0x%x, map_size.0x%x\n",
					node->addr, node->size, map_size);
				hisp_free_iova(pool_info->isp_mem_pool,
						  (unsigned long)node->addr,
						  (unsigned long)node->size);
				list_del(&node->list);
				kfree(node);
				mutex_unlock(&dev->mem_pool_mutex);
				return 0;
			}
		}
	}
	mutex_unlock(&dev->mem_pool_mutex);
	return -ENXIO;
}
EXPORT_SYMBOL(hisp_mem_pool_free_iova);/*lint !e580 !e546 */

unsigned int hisp_mem_map_setup(struct scatterlist *sgl,
					unsigned int iova,
					unsigned int size,
					unsigned int prot,
					unsigned int pool_num,
					unsigned int flag,
					unsigned int align)
{
	unsigned int va = 0;

	if (pool_num >= ISP_MEM_POOL_NUM) {
		pr_err("%s: wrong num, num : 0x%x\n", __func__, pool_num);
		return 0;
	}

	if (hisp_check_mempool_enable(pool_num)) {
		pr_err("[%s] %d mem pool had ben enabled!\n",
				__func__, pool_num);
		return 0;
	}

	va = hisp_mempool_setup(sgl, iova, size, prot,
			pool_num, align, flag);
	if (va == 0) {
		pr_err("[%s] Failed : hisp_mempool_setup!\n",
				__func__);
		return 0;
	}

	return va;
}
EXPORT_SYMBOL(hisp_mem_map_setup);/*lint !e580 !e546 */

unsigned int hisp_mem_map_setup_separate(struct scatterlist *sgl,
					unsigned int *iova,
					unsigned int ispcpu_size,
					unsigned int total_size,
					unsigned int prot,
					unsigned int pool_num,
					unsigned int type,
					unsigned int align)
{
	unsigned int va = 0;
	struct sg_table *sgt_add;
	struct sg_table *sgt_cpu_loc;

	pr_info("%s: ispcpu_size.0x%x, total_size.0x%x, prot.0x%x\n",
		__func__, ispcpu_size, total_size, prot);
	pr_info("%s: pool_num.%d, flag.%d, align.0x%x\n",
		__func__, pool_num, type, align);

	if ((iova == NULL) || (sgl == NULL)) {
		pr_err("%s: iova or sgl NULL\n", __func__);
		return 0;
	}

	if (total_size <= ispcpu_size) {
		pr_err("[%s] Failed: ispcpu_size.0x%x, total_size.0x%x\n",
			__func__, ispcpu_size, total_size);
		return 0;
	}

	if (pool_num >= ISP_MEM_POOL_NUM) {
		pr_err("%s failed: wrong num.0x%x\n", __func__, pool_num);
		return 0;
	}

	if ((type != MAP_TYPE_DYNAMIC_ALLOC_BY_KERNEL) &&
	    (type != MAP_TYPE_OFFLINE_ALLOC_BY_KERNEL)) {
		pr_err("%s failed: invalid type.%d\n", __func__, type);
		return 0;
	}

	if (hisp_check_mempool_enable(pool_num)) {
		pr_err("[%s] %d mem pool had ben enabled!\n", __func__, pool_num);
		return 0;
	}

	sgt_cpu_loc = hisp_alloc_dynmem_sg_table_separate(ispcpu_size, type);
	if (sgt_cpu_loc == NULL) {
		pr_err("[%s] Failed: hisp_alloc_dynmem_sg_table\n", __func__);
		return 0;
	}

	sgt_add = hisp_alloc_page_add_sgl(sgt_cpu_loc, sgl);
	if (sgt_add == NULL) {
		pr_err("[%s] Failed: hisp_alloc_page_add_sgl\n", __func__);
		hisp_free_dynmem_sg_table_separate(type);
		return 0;
	}

	va = hisp_mempool_setup(sgt_add->sgl, 0, total_size, prot,
			pool_num, align, type);
	if (va == 0) {
		pr_err("[%s] Failed : hisp_mempool_setup!\n", __func__);
		kfree(sgt_add);
		hisp_free_dynmem_sg_table_separate(type);
		return 0;
	}

	if (type == MAP_TYPE_DYNAMIC_ALLOC_BY_KERNEL)
		*iova = g_separate_mem_info.mem_info[HISP_DYM_ONLINE_TYPE].mem_iova;
	else if (type == MAP_TYPE_OFFLINE_ALLOC_BY_KERNEL)
		*iova = g_separate_mem_info.mem_info[HISP_DYM_OFFLINE_TYPE].mem_iova;

	kfree(sgt_add);

	return va;
}
EXPORT_SYMBOL(hisp_mem_map_setup_separate);

void hisp_mem_pool_destroy(unsigned int pool_num)
{
	if (pool_num >= ISP_MEM_POOL_NUM) {
		pr_err("%s: wrong num, num : 0x%x\n", __func__, pool_num);
		return;
	}

	hisp_mempool_destroy(pool_num);
}
EXPORT_SYMBOL(hisp_mem_pool_destroy);/*lint !e580 !e546 */

unsigned int hisp_mem_pool_alloc_carveout(size_t size, unsigned int type)
{
	if ((type == MAP_TYPE_DYNAMIC_CARVEOUT) && (size == MEM_MDC_SIZE))
		return MEM_MDC_DA;

	return 0;
}
EXPORT_SYMBOL(hisp_mem_pool_alloc_carveout);/*lint !e580 !e546 */
#else
#define ISP_BUF_ALIGN 0x1000
enum MASTER_TYPE {
	AP_MASTER,
	R8_MASTER,
	ISPCORE_MASTER,
	MAX_MASTER
};

enum NOSEC_ISP_MEM_TYPE {
	NOSEC_UNKNOW,
	/* Types For Non-Secure Scene */
	NOSEC_ISP_MEM,
	NOSEC_ONLINE_MEM,
	NOSEC_OFFLINE_MEM,
	NOSEC_OFFLINE_FBD_MEM,
	NOSEC_IMU_MEM,
	NOSEC_ISPNN_ONLINE_MEM,
	NOSEC_ISPNN_OFFLINE_MEM,
	NOSEC_ISPNN_SC_MEM,
	NOSEC_ISPNN_WORKSPACE_MEM,
#ifdef HISP_FUZZ
	NOSEC_ISP_KCOV_MEM,
	NOSEC_ISP_SHADOW_MEM,
#endif

	/* Types For Secure Scene */
	NOSEC_DYNAMIC_MEM,        /* HISP_DYNAMIC_POOL */
	SEC_ISPFW_MEM,            /* HISP_SEC_POOL */
	SEC_ISP_MEM,              /* HISP_ISPSEC_POOL */
	ISP_MEM_MAX,
};

static void hisp_record_memory_map(unsigned int type, unsigned int prot,
	struct hisp_mem_pool_info_list_s *info)
{
	struct hisp_shared_para *share_para = hisp_share_get_para();
	struct hisp_mem_para *hisp_mem_record;

	hisp_lock_sharedbuf();
	if (share_para == NULL) {
		pr_err("%s:hisp_share_get_para failed.\n", __func__);
		hisp_unlock_sharedbuf();
		return;
	}

	hisp_mem_record = &(share_para->hisp_mem_record[type]);
	if (hisp_mem_record->state == HISP_MEM_UNUSED) {
		hisp_mem_record->state        = HISP_MEM_MAP;
		hisp_mem_record->type         = type;
		hisp_mem_record->da           = info->addr;
		hisp_mem_record->size         = info->size;
		hisp_mem_record->r8_addr      = (unsigned int)info->r8_addr;
		hisp_mem_record->r8_size      = (unsigned int)info->r8_size;
		hisp_mem_record->r8_core_addr = (unsigned int)info->r8_core_addr;
		hisp_mem_record->r8_core_size = (unsigned int)info->r8_core_size;
		hisp_mem_record->ispfw_addr   = (unsigned int)info->ispfw_addr;
		hisp_mem_record->ispfw_size   = (unsigned int)info->ispfw_size;
		hisp_mem_record->prot         = prot;

		hisp_unlock_sharedbuf();
		return;
	}

	pr_err("%s: record memory region failed, size=0x%x\n", __func__, info->size);
	hisp_unlock_sharedbuf();
}

static void hisp_unrecord_memory_map(unsigned int type,
	struct hisp_mem_pool_info_list_s *info)
{
	struct hisp_shared_para *share_para = hisp_share_get_para();
	struct hisp_mem_para *hisp_mem_record;

	hisp_lock_sharedbuf();
	if (share_para == NULL) {
		pr_err("%s:hisp_share_get_para failed.\n", __func__);
		hisp_unlock_sharedbuf();
		return;
	}

	hisp_mem_record = &(share_para->hisp_mem_record[type]);
	if (hisp_mem_record->state        == HISP_MEM_MAP &&
		hisp_mem_record->type         == type &&
		hisp_mem_record->da           == info->addr &&
		hisp_mem_record->size         == info->size &&
		hisp_mem_record->r8_addr      == (unsigned int)info->r8_addr &&
		hisp_mem_record->r8_size      == (unsigned int)info->r8_size &&
		hisp_mem_record->r8_core_addr == (unsigned int)info->r8_core_addr &&
		hisp_mem_record->r8_core_size == (unsigned int)info->r8_core_size &&
		hisp_mem_record->ispfw_addr   == (unsigned int)info->ispfw_addr &&
		hisp_mem_record->ispfw_size   == (unsigned int)info->ispfw_size) {
		hisp_mem_record->state       = HISP_MEM_UNUSED;

		hisp_unlock_sharedbuf();
		return;
	}

	pr_err("%s: unrecord memory region failed, size=0x%x\n", __func__, info->size);
	hisp_unlock_sharedbuf();
}

static unsigned int hisp_get_memory_state(unsigned int type)
{
	struct hisp_shared_para *share_para = hisp_share_get_para();
	unsigned int state = HISP_MEM_MAP;

	hisp_lock_sharedbuf();
	if (share_para == NULL) {
		pr_err("%s:hisp_share_get_para failed\n", __func__);
		hisp_unlock_sharedbuf();
		return HISP_MEM_MAP;
	}
	state = share_para->hisp_mem_record[type].state;
	hisp_unlock_sharedbuf();

	return state;
}

static void hisp_dump_mem_info(struct hisp_mem_info_s *mem)
{
	if (mem == NULL)
		return;

	pr_debug("[%s] cpu addr=0x%x, size=0x%x, fw_mem=0x%x, size=0x%x, ispfw_mem=0x%x, ispfw_mem_r8=0x%x, size=0x%x\n",
		__func__, mem->cpu_addr, mem->cpu_size, mem->fw_mem.cpu_addr, mem->fw_mem.size,
		mem->ispfw_mem.core_addr, mem->ispfw_mem.cpu_addr, mem->ispfw_mem.size);
}

static int hisp_alloc_sgt_from_fd(int fd,
	struct hisp_mem_pool_info_list_s *list_info)
{
	struct device *dev = NULL;
	struct dma_buf *buf = NULL;
	struct dma_buf_attachment *attach = NULL;
	struct sg_table *table = NULL;

	dev = hisp_get_device();
	if (IS_ERR_OR_NULL(dev)) {
		pr_err("%s: platform dev is NULL!\n", __func__);
		return -EINVAL;
	}

	buf = dma_buf_get(fd);
	if (IS_ERR_OR_NULL(buf)) {
		pr_err("%s: fail to get dma buf", __func__);
		return -EINVAL;
	}

	attach = dma_buf_attach(buf, dev);
	if (IS_ERR_OR_NULL(attach)) {
		pr_err("%s: fail to attach dma buf", __func__);
		goto err_attach_node_buf;
	}

	table = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
	if (IS_ERR_OR_NULL(table)) {
		pr_err("%s: fail to map attachment", __func__);
		goto err_map_node_buf;
	}

	list_info->ap_table = table;
	list_info->buf = buf;
	list_info->attachment = attach;

	return 0;

err_map_node_buf:
	dma_buf_detach(buf, attach);
err_attach_node_buf:
	dma_buf_put(buf);

	return -EINVAL;
}

static void hisp_free_sgt_from_fd(struct hisp_mem_pool_info_list_s *list_info)
{
	if (list_info->attachment == NULL ||
		list_info->ap_table == NULL ||
		list_info->buf == NULL) {
			pr_err("[%s] attachment or table or buf is NULL\n", __func__);
			return;
	}

	dma_buf_unmap_attachment(list_info->attachment, list_info->ap_table, DMA_BIDIRECTIONAL);
	dma_buf_detach(list_info->buf, list_info->attachment);
	dma_buf_put(list_info->buf);

	list_info->ap_table = NULL;
	list_info->buf = NULL;
	list_info->attachment = NULL;
}

static int hisp_alloc_sgt_from_size(unsigned int master, size_t size,
	struct hisp_mem_pool_info_list_s *list_info)
{
	struct sg_table *table = NULL;

	if (size == 0) {
		pr_err("%s: size equal 0, just return", __func__);
		return -EINVAL;
	}

	table = hisp_alloc_dynmem_sg_table(size);
	if (IS_ERR_OR_NULL(table)) {
		pr_err("%s: fail to map attachment", __func__);
		return -EINVAL;
	}

	if (master == R8_MASTER)
		list_info->fw_table = table;
	else if (master == ISPCORE_MASTER)
		list_info->ispfw_table = table;

	return 0;
}

static void hisp_free_sgt_from_size(unsigned int master,
	struct hisp_mem_pool_info_list_s *list_info)
{
	if (master == R8_MASTER && list_info->fw_table != NULL) {
		hisp_free_dynmem_sg_table(list_info->fw_table);
		list_info->fw_table = NULL;
	} else if (master == ISPCORE_MASTER && list_info->ispfw_table != NULL) {
		hisp_free_dynmem_sg_table(list_info->ispfw_table);
		list_info->ispfw_table = NULL;
	} else {
		pr_err("[%s]: invalid param master[%d]\n", __func__, master);
	}
}

static unsigned int hisp_fw_phy_memory_map(phys_addr_t paddr, unsigned int prot,
	struct hisp_mem_pool_info_list_s *list_info)
{
	unsigned int addr = list_info->addr;
	unsigned int size = list_info->size;

	if (paddr == 0) {
		pr_err("%s: paddr is invalid!\n", __func__);
		return 0;
	}

	if ((IOMMU_MASK & prot) != 0) {
		pr_err("iommu attr error.0x%x\n", prot);
		return 0;
	}

	return hisp_memory_map(NULL, paddr, addr, size, prot);
}

static int hisp_memory_map_from_sgl(struct scatterlist *sgl,
	unsigned int addr, unsigned int size, unsigned int prot)
{
	unsigned int iova;

	if (sgl == NULL) {
		pr_err("%s: sgt is NULL", __func__);
		return -EINVAL;
	}

	iova = hisp_memory_map(sgl, HISP_DNYMEM_MAP_TYPE, addr, size, prot);
	if (iova == 0) {
		pr_err("[%s] hisp_memory_map failed", __func__);
		return -EINVAL;
	}

	return 0;
}

static int hisp_fw_memory_map(unsigned int prot,
	struct hisp_map_info_s *map_info,
	struct hisp_mem_pool_info_list_s *list_info)
{
	unsigned int ap_addr;
	unsigned int ap_size;
	unsigned int r8_addr;
	unsigned int r8_size;
	unsigned int r8_core_addr;
	unsigned int r8_core_size;
	int ret;

	if ((IOMMU_MASK & prot) != 0) {
		pr_err("iommu attr error.0x%x\n", prot);
		return -EINVAL;
	}

	ap_addr = list_info->addr;
	ap_size = map_info->size;
	if (ap_size != 0 && list_info->ap_table != NULL) {
		ret = hisp_memory_map_from_sgl(list_info->ap_table->sgl, ap_addr, ap_size, prot);
		if (ret != 0) {
			pr_err("[%s] type[%d] hisp_memory_map ap failed", __func__, list_info->type);
			return -EINVAL;
		}
	}

	r8_addr = list_info->addr + map_info->size;
	r8_size = map_info->fw_mem_size;
	if (r8_size != 0 && list_info->fw_table != NULL) {
		ret = hisp_memory_map_from_sgl(list_info->fw_table->sgl, r8_addr, r8_size, prot);
		if (ret != 0) {
			pr_err("[%s] type[%d] hisp_memory_map r8 failed", __func__, list_info->type);
			goto err_map_r8_part;
		}
	}

	r8_core_addr = list_info->addr + map_info->size + map_info->fw_mem_size;
	r8_core_size = map_info->ispfw_mem_size;
	if (r8_core_size != 0 && list_info->ispfw_table != NULL) {
		ret = hisp_memory_map_from_sgl(list_info->ispfw_table->sgl, r8_core_addr, r8_core_size, prot);
		if (ret != 0) {
			pr_err("[%s] type[%d] hisp_memory_map r8 core failed", __func__, list_info->type);
			goto err_map_r8_core_part;
		}
	}

	return 0;

err_map_r8_core_part:
	if (r8_size != 0)
		hisp_memory_unmap(r8_addr, r8_size, HISP_DNYMEM_MAP_TYPE);
err_map_r8_part:
	if (ap_size != 0)
		hisp_memory_unmap(ap_addr, ap_size, HISP_DNYMEM_MAP_TYPE);

	return -EINVAL;
}

static void hisp_fw_memory_unmap(struct hisp_mem_pool_info_list_s *list_info)
{
	unsigned int ap_size = list_info->size - (unsigned int)list_info->r8_size - (unsigned int)list_info->r8_core_size;

	if (ap_size != 0)
		(void)hisp_memory_unmap(list_info->addr, ap_size, HISP_DNYMEM_MAP_TYPE);

	if (list_info->r8_size != 0)
		(void)hisp_memory_unmap(list_info->r8_addr, list_info->r8_size, HISP_DNYMEM_MAP_TYPE);

	if (list_info->r8_core_size != 0)
		(void)hisp_memory_unmap(list_info->r8_core_addr, list_info->r8_core_size, HISP_DNYMEM_MAP_TYPE);
}

static int hisp_ispfw_memory_map(unsigned int prot, unsigned int size,
			struct hisp_mem_pool_info_list_s *list_info)
{
	struct hisp_mem_pool_s *mem_pool = &g_hisp_mem_pool_info;
	struct scatterlist *sgl = NULL;
	struct device *dev = NULL;
	unsigned long map_size;
	unsigned long iova;

	if (size == 0) {
		pr_err("[%s] size equal 0, just return\n", __func__);
		return 0;
	}

	if (list_info->ispfw_table == NULL) {
		pr_err("[%s] ispfw_table invalid\n", __func__);
		return -EINVAL;
	}

	sgl = list_info->ispfw_table->sgl;
	if (sgl == NULL) {
		pr_err("%s: sgl is NULL!\n", __func__);
		return -EINVAL;
	}

	if ((IOMMU_MASK & prot) != 0) {
		pr_err("iommu attr error.0x%x\n", prot);
		return -EINVAL;
	}

	dev = hisp_get_ispfw_device();
	if (dev == NULL) {
		pr_err("%s: platform dev is NULL!\n", __func__);
		return -EINVAL;
	}

#ifdef CONFIG_MM_LB
	prot = prot & (IOMMU_PORT_MASK | IOMMU_READ | IOMMU_WRITE);
#else
	prot = prot & (IOMMU_READ | IOMMU_WRITE);
#endif

	mutex_lock(&mem_pool->hisp_map_mutex);
	iova = mm_iommu_map_sg(dev, sgl, (int)prot, &map_size);
	if (iova == 0 || size != map_size) {
		pr_err("[ISP Mem] Fail: dyn get ispfw iova 0x%x, size:0x%x, map_size:0x%x\n",
			iova, size, map_size);
		mutex_unlock(&mem_pool->hisp_map_mutex);
		return -EINVAL;
	}

	mutex_unlock(&mem_pool->hisp_map_mutex);

	list_info->ispfw_addr = iova;
	list_info->ispfw_size = size;

	return 0;
}

static void hisp_ispfw_memory_unmap(struct hisp_mem_pool_info_list_s *list_info)
{
	struct hisp_mem_pool_s *mem_pool = &g_hisp_mem_pool_info;
	struct scatterlist *sgl = NULL;
	struct device *dev = NULL;
	unsigned int iova;
	int ret;

	if (list_info->ispfw_table == NULL) {
		pr_err("[%s] ispfw_table invalid\n", __func__);
		return;
	}

	sgl = list_info->ispfw_table->sgl;
	if (sgl == NULL) {
		pr_err("%s: sgl is NULL!\n", __func__);
		return;
	}

	iova = (unsigned int)list_info->ispfw_addr;
	if (iova == 0) {
		pr_err("%s: iova is 0!\n", __func__);
		return;
	}

	dev = hisp_get_ispfw_device();
	if (dev == NULL) {
		pr_err("%s: platform dev is NULL!\n", __func__);
		return;
	}

	mutex_lock(&mem_pool->hisp_map_mutex);
	ret = mm_iommu_unmap_sg(dev, sgl, iova);
	if (ret != 0) {
		pr_err("%s: mm_iommu_unmap_sg failed: iova 0x%lx\n", __func__, iova);
		mutex_unlock(&mem_pool->hisp_map_mutex);
		return;
	}
	mutex_unlock(&mem_pool->hisp_map_mutex);
}

static int hisp_alloc_from_mempool(unsigned int ap_size, unsigned int r8_size, unsigned int core_size,
					struct hisp_mem_pool_info_list_s *list_info)
{
	struct hisp_mem_pool_s *mem_pool = &g_hisp_mem_pool_info;
	struct hisp_mem_pool_info_s *pool_info;
	unsigned int map_size;
	unsigned int iova;
	unsigned int addr;
	unsigned int size = ap_size + r8_size + core_size;

	pool_info = &mem_pool->iova_pool_info;
	if (pool_info == NULL) {
		pr_err("isp mem pool didn't creat\n");
		return -EINVAL;
	}

	if ((pool_info->enable != 1) || (pool_info->isp_mem_pool == NULL)) {
		pr_err("iova mem pool didn't creat\n");
		return -EINVAL;
	}

	if ((size == 0) || (size > (pool_info->iova_end - pool_info->iova_start) / 2)) {
		pr_err("Invalid argument size.0x%x\n", size);
		return -EINVAL;
	}

	mutex_lock(&mem_pool->hisp_iova_mutex);
	/* double size to find a align addr */
	map_size = PAGE_ALIGN(size * 2);
	iova = (unsigned int)gen_pool_alloc(pool_info->isp_mem_pool, (unsigned long)map_size);
	if (iova == 0) {
		pr_err("gen_pool_alloc failed, map_size.0x%x, size.0x%x\n", map_size, size);
		hisp_dump_mempool_info(pool_info);
		mutex_unlock(&mem_pool->hisp_iova_mutex);
		return -EINVAL;
	}

	addr = hisp_get_align_addr_from_iova(iova, size, map_size, ISP_BUF_ALIGN);
	if (addr == 0) {
		pr_err("get align addr failed, iova.0x%x, size.0x%x\n", iova, size);
		goto get_align_addr_fail;
	}

	list_info->addr = addr;
	list_info->size = size;
	list_info->iova = iova;
	list_info->iova_size = map_size;
	list_info->r8_addr = addr + ap_size;
	list_info->r8_size = r8_size;
	list_info->r8_core_addr = addr + ap_size + r8_size;
	list_info->r8_core_size = core_size;
	mutex_unlock(&mem_pool->hisp_iova_mutex);

	return 0;

get_align_addr_fail:
	gen_pool_free(pool_info->isp_mem_pool, iova, map_size);
	mutex_unlock(&mem_pool->hisp_iova_mutex);
	return -EINVAL;
}

static int hisp_free_to_mempool(struct hisp_mem_pool_info_list_s *list_info)
{
	struct hisp_mem_pool_s *mem_pool = &g_hisp_mem_pool_info;
	struct hisp_mem_pool_info_s *pool_info = NULL;

	pool_info = &mem_pool->iova_pool_info;
	if (pool_info == NULL) {
		pr_err("isp mem pool didn't creat\n");
		return -ENXIO;
	}

	if ((pool_info->enable != 1) ||
		(pool_info->isp_mem_pool == NULL)) {
		pr_err("iova mem pool didn't creat\n");
		return -ENXIO;
	}

	if ((list_info->addr < pool_info->iova_start) || (list_info->addr >= pool_info->iova_end) ||
		(list_info->size > (pool_info->iova_end - list_info->addr))) {
		pr_err("Invalid argument addr.0x%x, size.0x%x\n", list_info->addr, list_info->size);
		return -EINVAL;
	}

	mutex_lock(&mem_pool->hisp_iova_mutex);
	gen_pool_free(pool_info->isp_mem_pool, list_info->iova, list_info->iova_size);
	mutex_unlock(&mem_pool->hisp_iova_mutex);
	return -ENOMEM;
}

static int hisp_mem_map_r8_paddr(unsigned int type, struct hisp_map_info_s *map_info, struct hisp_mem_info_s *mem_info,
	struct hisp_mem_pool_info_list_s *list_info)
{
	unsigned int iova;
	int ret;

	if (map_info->fw_mem_prot != IOMMU_READ) {
		pr_err("%s: paddr", __func__);
		return -ENOMEM;
	}

	ret = hisp_alloc_from_mempool(map_info->size, map_info->fw_mem_size, map_info->ispfw_mem_size, list_info);
	if (ret != 0) {
		pr_err("[%s] type[%d] hisp_alloc_from_mempool failed", __func__, type);
		return -ENOMEM;
	}

	iova = hisp_fw_phy_memory_map(map_info->paddr, map_info->fw_mem_prot, list_info);
	if (iova == 0) {
		pr_err("[%s] type[%d] hisp_fw_phy_memory_map failed", __func__, type);
		goto err_map_r8_iova;
	}

	mem_info->cpu_addr = list_info->addr;
	mem_info->cpu_size = map_info->size;
	mem_info->fw_mem.cpu_addr = list_info->addr + map_info->size;
	mem_info->fw_mem.size = map_info->fw_mem_size;

	hisp_record_memory_map(type, map_info->fw_mem_prot, list_info);

	return 0;

err_map_r8_iova:
	hisp_free_to_mempool(list_info);

	return -EINVAL;
}

static void hisp_mem_unmap_r8_paddr(unsigned int type, struct hisp_mem_info_s *mem_info,
	struct hisp_mem_pool_info_list_s *list_node)
{
	struct hisp_mem_pool_s *mem_pool = &g_hisp_mem_pool_info;
	struct hisp_mem_pool_info_list_s *node = NULL;
	struct hisp_mem_pool_info_list_s *node_temp = NULL;

	(void)mem_info;

	mutex_lock(&mem_pool->hisp_list_mutex);
	list_for_each_entry_safe(node, node_temp, &list_node->list, list) {
		if (node->type == type) {
			pr_info("[%s] [ISP Mem] type[%d]\n", __func__, type);
			hisp_fw_memory_unmap(node);
			hisp_free_to_mempool(node);

			hisp_unrecord_memory_map(type, node);

			list_del(&node->list);
			kfree(node);
			mutex_unlock(&mem_pool->hisp_list_mutex);
			return;
		}
	}
	mutex_unlock(&mem_pool->hisp_list_mutex);
}

static int hisp_mem_map_r8_fd(unsigned int type, struct hisp_map_info_s *map_info, struct hisp_mem_info_s *mem_info,
	struct hisp_mem_pool_info_list_s *list_info)
{
	int ret;

	ret = hisp_alloc_sgt_from_fd(map_info->fd, list_info);
	if (ret != 0) {
		pr_err("[%s] type[%d] hisp_alloc_sgt_from_fd failed", __func__, type);
		return ret;
	}

	ret = hisp_alloc_from_mempool(map_info->size, map_info->fw_mem_size, map_info->ispfw_mem_size, list_info);
	if (ret != 0) {
		pr_err("[%s] type[%d] hisp_alloc_from_mempool failed", __func__, type);
		goto err_alloc_r8_iova;
	}

	ret = hisp_fw_memory_map(map_info->fw_mem_prot, map_info, list_info);
	if (ret != 0) {
		pr_err("[%s] type[%d] hisp_fw_memory_map failed", __func__, type);
		goto err_map_r8_iova;
	}

	mem_info->cpu_addr = list_info->addr;
	mem_info->cpu_size = map_info->size;
	mem_info->fw_mem.cpu_addr = list_info->addr + map_info->size;
	mem_info->fw_mem.size = map_info->fw_mem_size;

	hisp_record_memory_map(type, map_info->fw_mem_prot, list_info);

	return 0;

err_map_r8_iova:
	hisp_free_to_mempool(list_info);
err_alloc_r8_iova:
	hisp_free_sgt_from_fd(list_info);

	return -EINVAL;
}

static void hisp_mem_unmap_r8_fd(unsigned int type, struct hisp_mem_info_s *mem_info,
	struct hisp_mem_pool_info_list_s *list_node)
{
	struct hisp_mem_pool_s *mem_pool = &g_hisp_mem_pool_info;
	struct hisp_mem_pool_info_list_s *node = NULL;
	struct hisp_mem_pool_info_list_s *node_temp = NULL;

	(void)mem_info;

	mutex_lock(&mem_pool->hisp_list_mutex);
	list_for_each_entry_safe(node, node_temp, &list_node->list, list) {
		if (node->type == type) {
			pr_info("[%s] [ISP Mem] type[%d]\n", __func__, type);
			hisp_fw_memory_unmap(node);
			hisp_free_to_mempool(node);
			hisp_free_sgt_from_fd(node);

			hisp_unrecord_memory_map(type, node);

			list_del(&node->list);
			kfree(node);
			mutex_unlock(&mem_pool->hisp_list_mutex);
			return;
		}
	}
	mutex_unlock(&mem_pool->hisp_list_mutex);
}

static int hisp_mem_map_hybrid(unsigned int type, struct hisp_map_info_s *map_info, struct hisp_mem_info_s *mem_info,
	struct hisp_mem_pool_info_list_s *list_info)
{
	int ret;

	ret = hisp_alloc_sgt_from_fd(map_info->fd, list_info);
	if (ret != 0) {
		pr_err("[%s] type[%d] hisp_alloc_sgt_from_fd failed", __func__, type);
		return ret;
	}

	ret = hisp_alloc_sgt_from_size(R8_MASTER, map_info->fw_mem_size, list_info);
	if (ret != 0) {
		pr_err("[%s] type[%d] hisp_alloc_sgt_from_size failed", __func__, type);
		goto err_alloc_r8_sgt;
	}

	ret = hisp_alloc_sgt_from_size(ISPCORE_MASTER, map_info->ispfw_mem_size, list_info);
	if (ret != 0) {
		pr_err("[%s] type[%d] hisp_alloc_sgt_from_size failed", __func__, type);
		goto err_alloc_core_sgt;
	}

	ret = hisp_ispfw_memory_map(map_info->ispfw_mem_prot, map_info->ispfw_mem_size, list_info);
	if (ret != 0) {
		pr_err("[%s] type[%d] hisp_ispfw_memory_map failed", __func__, type);
		goto err_alloc_and_map_core_iova;
	}

	ret = hisp_alloc_from_mempool(map_info->size, map_info->fw_mem_size, map_info->ispfw_mem_size, list_info);
	if (ret != 0) {
		pr_err("[%s] type[%d] hisp_alloc_from_mempool failed", __func__, type);
		goto err_alloc_r8_iova;
	}

	ret = hisp_fw_memory_map(map_info->fw_mem_prot, map_info, list_info);
	if (ret != 0) {
		pr_err("[%s] type[%d] hisp_fw_memory_map failed", __func__, type);
		goto err_map_r8_iova;
	}

	mem_info->cpu_addr = list_info->addr;
	mem_info->cpu_size = map_info->size;
	mem_info->fw_mem.cpu_addr = list_info->addr + map_info->size;
	mem_info->fw_mem.size = map_info->fw_mem_size;
	mem_info->ispfw_mem.cpu_addr = list_info->addr + map_info->size + map_info->fw_mem_size;
	mem_info->ispfw_mem.core_addr = (unsigned int)list_info->ispfw_addr;
	mem_info->ispfw_mem.size = (unsigned int)list_info->ispfw_size;

	hisp_record_memory_map(type, map_info->fw_mem_prot, list_info);

	return 0;

err_map_r8_iova:
	hisp_free_to_mempool(list_info);
err_alloc_r8_iova:
	hisp_ispfw_memory_unmap(list_info);
err_alloc_and_map_core_iova:
	hisp_free_sgt_from_size(ISPCORE_MASTER, list_info);
err_alloc_core_sgt:
	hisp_free_sgt_from_size(R8_MASTER, list_info);
err_alloc_r8_sgt:
	hisp_free_sgt_from_fd(list_info);

	return -EINVAL;
}

static void hisp_mem_unmap_hybrid(unsigned int type, struct hisp_mem_info_s *mem_info,
	struct hisp_mem_pool_info_list_s *list_node)
{
	struct hisp_mem_pool_s *mem_pool = &g_hisp_mem_pool_info;
	struct hisp_mem_pool_info_list_s *node = NULL;
	struct hisp_mem_pool_info_list_s *node_temp = NULL;

	(void)mem_info;

	mutex_lock(&mem_pool->hisp_list_mutex);
	list_for_each_entry_safe(node, node_temp, &list_node->list, list) {
		if (node->type == type) {
			pr_info("[%s] [ISP Clean Mem] type[%d]\n", __func__, type);
			hisp_fw_memory_unmap(node);
			hisp_free_to_mempool(node);
			hisp_ispfw_memory_unmap(node);
			hisp_free_sgt_from_size(ISPCORE_MASTER, node);
			hisp_free_sgt_from_size(R8_MASTER, node);
			hisp_free_sgt_from_fd(node);

			hisp_unrecord_memory_map(type, node);

			list_del(&node->list);
			kfree(node);
			mutex_unlock(&mem_pool->hisp_list_mutex);
			return;
		}
	}
	mutex_unlock(&mem_pool->hisp_list_mutex);
}

static int hisp_mem_map_sec(unsigned int type, struct hisp_map_info_s *map_info, struct hisp_mem_info_s *mem_info,
	struct hisp_mem_pool_info_list_s *list_info)
{
	struct hisp_mem_pool_s *mem_pool = &g_hisp_mem_pool_info;

	(void)map_info;
	mutex_lock(&mem_pool->hisp_iova_mutex);
	list_info->addr = hisp_pool_mem_addr(type - NOSEC_DYNAMIC_MEM);
	if (list_info->addr == 0x0) {
		pr_err("[%s] alloc addr failed with type[%d]\n", __func__, type);
		goto err_get_r8_sec_iova;
	}
	mutex_unlock(&mem_pool->hisp_iova_mutex);

	hisp_record_memory_map(type, 0, list_info);

	mem_info->cpu_addr = list_info->addr;

	return 0;

err_get_r8_sec_iova:
	mutex_unlock(&mem_pool->hisp_iova_mutex);

	return -EINVAL;
}

static void hisp_mem_unmap_sec(unsigned int type, struct hisp_mem_info_s *mem_info,
	struct hisp_mem_pool_info_list_s *list_node)
{
	struct hisp_mem_pool_s *mem_pool = &g_hisp_mem_pool_info;
	struct hisp_mem_pool_info_list_s *node = NULL;
	struct hisp_mem_pool_info_list_s *node_temp = NULL;

	(void)mem_info;

	mutex_lock(&mem_pool->hisp_list_mutex);
	list_for_each_entry_safe(node, node_temp, &list_node->list, list) {
		if (node->type == type) {
			pr_info("[%s] [ISP Mem] type[%d]\n", __func__, type);

			hisp_unrecord_memory_map(type, node);

			list_del(&node->list);
			kfree(node);
			mutex_unlock(&mem_pool->hisp_list_mutex);
			return;
		}
	}
	mutex_unlock(&mem_pool->hisp_list_mutex);
}

#if HISP_FUZZ
static int g_init_done;
static struct scatterlist *isp_vmalloc_to_sg(void *virt, int nr_pages)
{
	struct scatterlist *sglist;
	struct page *pg;
	int i;

	sglist = vzalloc(nr_pages * sizeof(*sglist));
	if (sglist == NULL)
		return NULL;
		sg_init_table(sglist, nr_pages);
	for (i = 0; i < nr_pages; i++, virt += PAGE_SIZE) {
	pg = vmalloc_to_page(virt);
	if (pg == NULL)
	goto isp_vm_to_sg_err;
	BUG_ON(PageHighMem(pg));
	sg_set_page(&sglist[i], pg, PAGE_SIZE, 0);
	}
	return sglist;

isp_vm_to_sg_err:
	vfree(sglist);
	return NULL;
}

int hisp_vmalloc_map_ispaddr(void *vaddr, unsigned long size, unsigned int prot,
							 struct hisp_mem_pool_info_list_s *list_info)
{
	int ret;
	struct scatterlist *scatterlist = isp_vmalloc_to_sg(vaddr, size/PAGE_SIZE);

	if (scatterlist == NULL) {
		pr_err("[%s] isp_vmalloc_to_sg failed", __func__);
		return -EINVAL;
	}

	ret = hisp_alloc_from_mempool(0, size, 0, list_info);
	if (ret != 0) {
		pr_err("[%s] alloc failed", __func__);
		goto alloc_from_pool_fail;
	}

	ret = hisp_memory_map_from_sgl(scatterlist, list_info->addr,
								   list_info->size, prot);
	if (ret != 0) {
		pr_err("[%s] map failed", __func__);
		goto map_from_sgl_fail;
	}

	hisp_record_memory_map(list_info->type, prot, list_info);
	vfree(scatterlist);
	return 0;
map_from_sgl_fail:
	hisp_free_to_mempool(list_info);
alloc_from_pool_fail:
	vfree(scatterlist);
	return -EINVAL;
}

static int hisp_kcov_shadow_mem_map(
	unsigned int type, struct hisp_mem_pool_info_list_s *list_info)
{
	int ret;
	void *addr = NULL;
	unsigned long size = 0;

	list_info->type = type;
	if (type == NOSEC_ISP_KCOV_MEM) {
		ret = hisp_kcov_get_shmem(&addr, &size);
		pr_info("[%s] kcov mem: addr.0x%lx size.0x%lx\n", __func__, addr, size);
		if (ret != 0) {
			pr_err("[%s] kcov_get_shmem_fd failed", __func__);
			return -EINVAL;
		}
	} else if (type == NOSEC_ISP_SHADOW_MEM) {
		ret = hisp_fuzz_get_shmem(&addr, &size);
		pr_info("[%s] fuzz mem: addr.0x%lx size.0x%lx\n", __func__, addr, size);
		if (ret != 0) {
			pr_err("[%s] hisp_fuzz_get_shmem failed", __func__);
			return -EINVAL;
		}
	}
	ret = hisp_vmalloc_map_ispaddr(
		addr, size, IOMMU_READ | IOMMU_WRITE, list_info);
	if (ret != 0) {
		pr_err("[%s] vmalloc map ispaddr failed", __func__);
		return -EINVAL;
	}
	return 0;
}

void hisp_kcov_shadow_mem_deinit(void)
{
	struct hisp_mem_pool_s *mem_pool = &g_hisp_mem_pool_info;
	struct hisp_mem_pool_info_s *pool_info = NULL;
	struct hisp_mem_pool_info_list_s *list_node = NULL;
	struct hisp_mem_pool_info_list_s *node = NULL;
	struct hisp_mem_pool_info_list_s *node_temp = NULL;

	pr_info("[%s] +\n", __func__);
	if (!g_init_done) {
		pr_info("[%s] not init! will not deinit.\n", __func__);
		return;
	}

	pool_info = &mem_pool->iova_pool_info;
	if (pool_info == NULL) {
		pr_err("isp mem pool didn't creat\n");
		return;
	}

	list_node = pool_info->node;
	if (list_node == NULL) {
		pr_err("iova mem pool is NULL\n");
		return;
	}

	mutex_lock(&mem_pool->hisp_list_mutex);
	list_for_each_entry_safe(node, node_temp, &list_node->list, list) {
		if (node->type == NOSEC_ISP_KCOV_MEM || node->type == NOSEC_ISP_SHADOW_MEM) {
			pr_info("[%s] [Fuzz Mem] type[%d]\n", __func__, node->type);

			hisp_unrecord_memory_map(node->type, node);
			hisp_memory_unmap(node->addr, node->size, HISP_DNYMEM_MAP_TYPE);
			hisp_free_to_mempool(node);

			list_del(&node->list);
			kfree(node);
		}
	}
	g_init_done = 0;
	mutex_unlock(&mem_pool->hisp_list_mutex);
	pr_info("[%s] -\n", __func__);
	return;
}

int hisp_fuzz_kcov_shadow_mem_init(void)
{
	int ret;
	struct hisp_mem_pool_s *mem_pool = &g_hisp_mem_pool_info;
	struct hisp_mem_pool_info_s *pool_info;
	struct hisp_mem_pool_info_list_s *list_node;
	struct hisp_mem_pool_info_list_s *list_info;

	pr_info("[%s] +\n", __func__);
	if (g_init_done) {
		pr_info("[%s] already init done!\n", __func__);
		return 0;
	} else {
		g_init_done = 1;
	}

	pool_info = &mem_pool->iova_pool_info;
	if (pool_info == NULL) {
		pr_err("isp mem pool didn't creat\n");
		return -EINVAL;
	}

	list_node = pool_info->node;
	if (list_node == NULL) {
		pr_err("iova mem pool list_node is NULL\n");
		return -EINVAL;
	}

	list_info = kzalloc(sizeof(struct hisp_mem_pool_info_list_s), GFP_KERNEL);
	if (!list_info) {
		pr_err("%s: fail to alloc list_info", __func__);
		return -ENOMEM;
	}
	mutex_lock(&mem_pool->hisp_mem_mutex);

	// map kcov mem
	ret = hisp_kcov_shadow_mem_map(NOSEC_ISP_KCOV_MEM, list_info);
	if (ret != 0) {
		pr_err("%s: kcov mem map failed", __func__);
		kfree(list_info);
		mutex_unlock(&mem_pool->hisp_mem_mutex);
		return -EINVAL;
	}

	mutex_unlock(&mem_pool->hisp_mem_mutex);

	mutex_lock(&mem_pool->hisp_list_mutex);
	list_add_tail(&list_info->list, &list_node->list);
	mutex_unlock(&mem_pool->hisp_list_mutex);

	list_info = kzalloc(sizeof(struct hisp_mem_pool_info_list_s), GFP_KERNEL);
	if (!list_info) {
		pr_err("%s: fail to alloc list_info", __func__);
		goto shadow_list_alloc_fail;
	}
	mutex_lock(&mem_pool->hisp_mem_mutex);

	// map shadow mem
	ret = hisp_kcov_shadow_mem_map(NOSEC_ISP_SHADOW_MEM, list_info);
	if (ret != 0) {
		pr_err("%s: shadow mem map failed", __func__);
		goto shadow_smem_init_fail;
	}

	mutex_unlock(&mem_pool->hisp_mem_mutex);

	mutex_lock(&mem_pool->hisp_list_mutex);
	list_add_tail(&list_info->list, &list_node->list);
	mutex_unlock(&mem_pool->hisp_list_mutex);

	pr_info("[%s] -\n", __func__);
	return 0;

shadow_smem_init_fail:
	kfree(list_info);
	mutex_unlock(&mem_pool->hisp_mem_mutex);
shadow_list_alloc_fail:
	hisp_kcov_shadow_mem_deinit();
	return -EINVAL;
}
#endif

void hisp_mem_list_clean(void)
{
	struct hisp_mem_pool_s *mem_pool = &g_hisp_mem_pool_info;
	struct hisp_mem_pool_info_s *pool_info = NULL;
	struct hisp_mem_pool_info_list_s *list_node = NULL;
	struct hisp_mem_pool_info_list_s *node = NULL;
	struct hisp_mem_pool_info_list_s *node_temp = NULL;

	pool_info = &mem_pool->iova_pool_info;
	if (pool_info == NULL) {
		pr_err("isp mem pool didn't creat\n");
		return;
	}

	list_node = pool_info->node;
	if (list_node == NULL) {
		pr_err("iova mem pool list_node is NULL\n");
		return;
	}

	mutex_lock(&mem_pool->hisp_mem_mutex);
	list_for_each_entry_safe(node, node_temp, &list_node->list, list) {
		if (node->type == NOSEC_ISP_MEM ||
			node->type == NOSEC_OFFLINE_MEM ||
			node->type == NOSEC_OFFLINE_FBD_MEM) {
			pr_info("[ISP Clean Mem] type[%d]\n", node->type);
			hisp_fw_memory_unmap(node);
			hisp_free_to_mempool(node);
			hisp_ispfw_memory_unmap(node);
			hisp_free_sgt_from_size(ISPCORE_MASTER, node);
			hisp_free_sgt_from_size(R8_MASTER, node);
			hisp_free_sgt_from_fd(node);
		} else if (node->type == NOSEC_IMU_MEM) {
			pr_info("[ISP Clean Mem] type[%d]\n", node->type);
			hisp_fw_memory_unmap(node);
			hisp_free_to_mempool(node);
		} else if (node->type == NOSEC_ONLINE_MEM ||
				   node->type == NOSEC_ISPNN_SC_MEM ||
				   node->type == NOSEC_ISPNN_ONLINE_MEM ||
				   node->type == NOSEC_ISPNN_OFFLINE_MEM ||
				   node->type == NOSEC_ISPNN_WORKSPACE_MEM) {
			pr_info("[ISP Clean Mem] type[%d]\n", node->type);
			hisp_fw_memory_unmap(node);
			hisp_free_to_mempool(node);
			hisp_ispfw_memory_unmap(node);
			hisp_free_sgt_from_fd(node);
		} else if (node->type == NOSEC_DYNAMIC_MEM ||
				   node->type == SEC_ISPFW_MEM ||
				   node->type == SEC_ISP_MEM) {
			pr_debug("[ISP Clean Mem] type[%d] just unrecord map\n", node->type);
#ifdef HISP_FUZZ
		} else if (node->type == NOSEC_ISP_KCOV_MEM || node->type == NOSEC_ISP_SHADOW_MEM) {
			pr_info("[%s] [Fuzz Mem] type[%d]\n", __func__, node->type);
			hisp_memory_unmap(node->addr, node->size, HISP_DNYMEM_MAP_TYPE);
			hisp_free_to_mempool(node);
#endif
		}
		hisp_unrecord_memory_map(node->type, node);
		list_del(&node->list);
		kfree(node);
	}
#ifdef HISP_FUZZ
	if (g_init_done)
		g_init_done = 0;
#endif
	mutex_unlock(&mem_pool->hisp_mem_mutex);
}
EXPORT_SYMBOL(hisp_mem_list_clean);

static int hisp_mem_param_check(unsigned int type,
	struct hisp_map_info_s *info, struct hisp_mem_info_s *mem)
{
	unsigned int state = HISP_MEM_MAP;

	if (info == NULL || mem == NULL) {
		pr_err("[%s] invalid info or mem\n", __func__);
		return -EINVAL;
	}

	if (type >= ISP_MEM_MAX) {
		pr_err("[%s] invalid type %d\n", __func__, type);
		return -EINVAL;
	}

	state = hisp_get_memory_state(type);
	if (state == HISP_MEM_MAP) {
		pr_err("[%s] type[%d] memory mapped already\n", __func__, type);
		return -EINVAL;
	}

	if (info->size >= ISP_MAP_MAX_SIZE ||
		info->fw_mem_size >= ISP_MAP_MAX_SIZE ||
		info->ispfw_mem_size >= ISP_MAP_MAX_SIZE) {
		pr_err("[%s] type[%d] size[0x%x-0x%x-0x%x], map size over 0x%x\n",
			   __func__, type, info->size, info->fw_mem_size,
			   info->ispfw_mem_size, ISP_MAP_MAX_SIZE);
		return -EINVAL;
	}

	return 0;
}

int hisp_mem_map_addr(unsigned int type, struct hisp_map_info_s *info, struct hisp_mem_info_s *mem)
{
	struct hisp_mem_pool_s *mem_pool = &g_hisp_mem_pool_info;
	struct hisp_mem_pool_info_s *pool_info      = NULL;
	struct hisp_mem_pool_info_list_s *list_node = NULL;
	struct hisp_mem_pool_info_list_s *list_info = NULL;
	int ret;

	pr_debug("[%s] type[%d] map addr start\n", __func__, type);
	ret = hisp_mem_param_check(type, info, mem);
	if (ret != 0) {
		pr_err("[%s] invalid param\n", __func__);
		return -EINVAL;
	}

	pool_info = &mem_pool->iova_pool_info;
	if (pool_info == NULL) {
		pr_err("isp mem pool didn't creat\n");
		return -EINVAL;
	}

	list_node = pool_info->node;
	if (list_node == NULL) {
		pr_err("iova mem pool list_node is NULL\n");
		return -EINVAL;
	}

	list_info = kzalloc(sizeof(struct hisp_mem_pool_info_list_s), GFP_KERNEL);
	if (!list_info) {
		pr_err("%s: fail to alloc list_info", __func__);
		return -ENOMEM;
	}
	list_info->type = type;

	mutex_lock(&mem_pool->hisp_mem_mutex);
	switch (type) {
	case NOSEC_ISP_MEM:
	case NOSEC_OFFLINE_MEM:
	case NOSEC_OFFLINE_FBD_MEM:
		ret = hisp_mem_map_hybrid(type, info, mem, list_info);
		if (ret != 0) {
			pr_err("[%s] type[%d] hisp_mem_map_hybrid failed", __func__, type);
			goto MAP_ADDR_ERR;
		}

		break;
	case NOSEC_IMU_MEM:
		ret = hisp_mem_map_r8_paddr(type, info, mem, list_info);
		if (ret != 0) {
			pr_err("[%s] type[%d] hisp_mem_map_r8_paddr failed", __func__, type);
			goto MAP_ADDR_ERR;
		}

		break;
	case NOSEC_ONLINE_MEM:
	case NOSEC_ISPNN_SC_MEM:
	case NOSEC_ISPNN_ONLINE_MEM:
	case NOSEC_ISPNN_OFFLINE_MEM:
	case NOSEC_ISPNN_WORKSPACE_MEM:
		ret = hisp_mem_map_r8_fd(type, info, mem, list_info);
		if (ret != 0) {
			pr_err("[%s] type[%d] hisp_mem_map_r8_fd failed", __func__, type);
			goto MAP_ADDR_ERR;
		}

		break;
	case SEC_ISPFW_MEM:
	case SEC_ISP_MEM:
	case NOSEC_DYNAMIC_MEM:
		ret = hisp_mem_map_sec(type, info, mem, list_info);
		if (ret != 0) {
			pr_err("[%s] type[%d] hisp_mem_map_sec failed", __func__, type);
			goto MAP_ADDR_ERR;
		}

		break;
	default:
		pr_err("[%s] type[%d] unsupported", __func__, type);
		goto MAP_ADDR_ERR;
	}
	mutex_unlock(&mem_pool->hisp_mem_mutex);

	mutex_lock(&mem_pool->hisp_list_mutex);
	list_add_tail(&list_info->list, &list_node->list);
	mutex_unlock(&mem_pool->hisp_list_mutex);

	hisp_dump_mem_info(mem);
	pr_debug("[%s] type[%d] map addr end\n", __func__, type);

	return 0;

MAP_ADDR_ERR:
	mutex_unlock(&mem_pool->hisp_mem_mutex);
	kfree(list_info);
	return -EINVAL;
}
EXPORT_SYMBOL(hisp_mem_map_addr);

int hisp_mem_unmap_addr(unsigned int type, struct hisp_mem_info_s mem_info)
{
	struct hisp_mem_pool_s *mem_pool = &g_hisp_mem_pool_info;
	struct hisp_mem_pool_info_s *pool_info = NULL;
	struct hisp_mem_pool_info_list_s *list_node = NULL;

	if (type >= ISP_MEM_POOL_NUM) {
		pr_err("[%s] invalid type %d\n", __func__, type);
		return -EINVAL;
	}

	pr_info("[%s] type[%d] unmap addr start\n", __func__, type);
	pool_info = &mem_pool->iova_pool_info;
	if (pool_info == NULL) {
		pr_err("isp mem pool didn't creat\n");
		return -EINVAL;
	}

	list_node = pool_info->node;
	if (list_node == NULL) {
		pr_err("iova mem pool is NULL\n");
		return -EINVAL;
	}

	mutex_lock(&mem_pool->hisp_mem_mutex);
	switch (type) {
	case NOSEC_ISP_MEM:
	case NOSEC_OFFLINE_MEM:
	case NOSEC_OFFLINE_FBD_MEM:
		hisp_mem_unmap_hybrid(type, &mem_info, list_node);
		break;
	case NOSEC_IMU_MEM:
		hisp_mem_unmap_r8_paddr(type, &mem_info, list_node);
		break;
	case NOSEC_ONLINE_MEM:
	case NOSEC_ISPNN_SC_MEM:
	case NOSEC_ISPNN_ONLINE_MEM:
	case NOSEC_ISPNN_OFFLINE_MEM:
	case NOSEC_ISPNN_WORKSPACE_MEM:
		hisp_mem_unmap_r8_fd(type, &mem_info, list_node);
		break;
	case SEC_ISPFW_MEM:
	case SEC_ISP_MEM:
	case NOSEC_DYNAMIC_MEM:
		hisp_mem_unmap_sec(type, &mem_info, list_node);
		break;
	default:
		pr_err("[%s] type[%d] unsupported", __func__, type);
	}
	mutex_unlock(&mem_pool->hisp_mem_mutex);

	pr_info("[%s] type[%d] unmap addr end\n", __func__, type);
	return 0;
}
EXPORT_SYMBOL(hisp_mem_unmap_addr);
#endif

static int hisp_mempool_info_init(unsigned int use_mempool, unsigned int addr,
		unsigned int size)
{
	struct hisp_mem_pool_s *info = &g_hisp_mem_pool_info;
	int ret = 0;

	ret = memset_s(info, sizeof(g_hisp_mem_pool_info),
		       0x0, sizeof(g_hisp_mem_pool_info));/*lint !e838 */
	if (ret < 0) {
		pr_err("[%s] memset_s g_hisp_mem_pool_info to 0 fail.%d\n",
				__func__, ret);
		return ret;
	}

	if ((use_mempool == 0) || (addr == 0) || (size == 0)) {
		pr_err("[%s] Invalid args use_mempool.%d add.0x%x size.0x%x\n",
				__func__, use_mempool, addr, size);
		return -EINVAL;
	}

	if (addr >= ISP_MAX_IOVA_MAGIC ||
		size >= ISP_MAX_IOVA_MAGIC ||
		addr >= ISP_MAX_IOVA_MAGIC - size) {
		pr_err("[%s] argument out mem! addr.0x%x size.0x%x\n",
				__func__, addr, size);
		return -EINVAL;
	}

	return 0;
}

int hisp_mempool_init(unsigned int addr, unsigned int size)
{
	struct hisp_mem_pool_s *info = &g_hisp_mem_pool_info;
	struct hisp_mem_pool_info_s *dev = NULL;
	struct hisp_mem_pool_info_list_s *listnode = NULL;
	unsigned int mempool_size = 0;
	int ret = 0;
	unsigned int use_mempool = hisp_smmuv3_mode();

	ret = hisp_mempool_info_init(use_mempool, addr, size);
	if (ret < 0)
		pr_err("[%s] mem_pool_info_int fail.%d\n", __func__, ret);

	dev = &info->iova_pool_info;
	if (dev == NULL) {
		pr_err("isp mem pool didn't creat\n");
		return -ENXIO;
	}

	mutex_init(&info->mem_pool_mutex);
	mutex_init(&info->hisp_map_mutex);
	mutex_init(&info->hisp_iova_mutex);
	mutex_init(&info->hisp_list_mutex);
	mutex_init(&info->hisp_mem_mutex);

	dev->isp_mem_pool = gen_pool_create(
				(int)(order_base_2(ISP_MEM_POOL_ALIGN)), -1);
	if (dev->isp_mem_pool == NULL) {
		pr_err("Create isp iova mempool failed!\n");
		goto gen_pool_create_fail;
	}

	mempool_size = PAGE_ALIGN(size);/*lint !e50 */
	ret = gen_pool_add(dev->isp_mem_pool, (unsigned long)addr,
			  (unsigned long)mempool_size, -1);
	if (ret) {
		pr_err("iova mempool Gen pool add failed!\n");
		goto gen_pool_add_fail;
	}

	pr_info("[ISP] iova mempool creat : add.0x%x, size.0x%x\n",
			addr, mempool_size);

	listnode = kzalloc(sizeof(struct hisp_mem_pool_info_list_s), GFP_KERNEL);
	if (listnode == NULL)
		goto gen_pool_add_fail;

	INIT_LIST_HEAD(&listnode->list);/*lint !e613 */
	dev->node = listnode;
	dev->enable = 1;
	dev->iova_start = addr;
	dev->iova_end = addr + size;
	return 0;

gen_pool_add_fail:
	gen_pool_destroy(dev->isp_mem_pool);
gen_pool_create_fail:
	mutex_destroy(&info->mem_pool_mutex);
	mutex_destroy(&info->hisp_map_mutex);
	mutex_destroy(&info->hisp_iova_mutex);
	mutex_destroy(&info->hisp_list_mutex);
	mutex_destroy(&info->hisp_mem_mutex);

	return -ENOMEM;
}

void hisp_mempool_exit(void)
{
	struct hisp_mem_pool_s *info = &g_hisp_mem_pool_info;
	struct hisp_mem_pool_info_s *dev = NULL;
	struct hisp_mem_pool_info_list_s *listnode = NULL;
	struct hisp_mem_pool_info_list_s *node = NULL;
	struct hisp_mem_pool_info_list_s *temp = NULL;
	unsigned int use_mempool = hisp_smmuv3_mode();
	int ret = 0;

	if (use_mempool) {
		dev = &info->iova_pool_info;
		if (dev == NULL) {
			pr_err("isp mem pool didn't creat\n");
			return;
		}
		if (dev->enable == 0) {
			pr_err("iova mem pool didn't creat\n");
			return;
		}
		listnode = dev->node;
		if (listnode == NULL) {
			pr_err("isp mem pool didn't creat\n");
			return;
			}

		list_for_each_entry_safe(node, temp, &listnode->list, list) {
			pr_info("%s: va = 0x%x, size = 0x%x\n",
				__func__, node->iova, node->iova_size);
			gen_pool_free(dev->isp_mem_pool,
					node->iova, node->iova_size);
			list_del(&node->list);
			kfree(node);
		}
		gen_pool_destroy(dev->isp_mem_pool);
		kfree(listnode);
		dev->isp_mem_pool = NULL;
		dev->node = NULL;
		dev->enable = 0;
	}
	mutex_destroy(&info->mem_pool_mutex);
	mutex_destroy(&info->hisp_map_mutex);
	mutex_destroy(&info->hisp_iova_mutex);
	mutex_destroy(&info->hisp_list_mutex);
	mutex_destroy(&info->hisp_mem_mutex);
	ret = memset_s(info, sizeof(g_hisp_mem_pool_info),
		       0x0, sizeof(g_hisp_mem_pool_info));/*lint !e838 */
	if (ret < 0)
		pr_err("[%s] memset_s g_hisp_mem_pool_info to 0 fail.%d\n",
				__func__, ret);
}
