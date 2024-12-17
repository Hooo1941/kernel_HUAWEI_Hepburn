/*
 * ISP driver, isp_mempool.h
 *
 * Copyright (c) 2013 ISP Technologies CO., Ltd.
 *
 */

#ifndef _PLAT_MEMPOOL_HISP_H
#define _PLAT_MEMPOOL_HISP_H

#include <linux/version.h>
#include <linux/scatterlist.h>
#include <linux/printk.h>
#include <linux/genalloc.h>
#include <linux/list.h>
#include <linux/mutex.h>

#ifdef __cplusplus
extern "C"
{
#endif

enum maptype {
	MAP_TYPE_DYNAMIC                 = 0,
	MAP_TYPE_RAW2YUV                 = 1,
	MAP_TYPE_STATIC                  = 2,
	MAP_TYPE_STATIC_SEC              = 3,
	MAP_TYPE_DYNAMIC_CARVEOUT        = 4,
	MAP_TYPE_STATIC_ISP_SEC          = 5,
	MAP_TYPE_DYNAMIC_SEC             = 6,
	MAP_TYPE_DYNAMIC_PREVIEW         = 7,
	MAP_TYPE_SMARTISP_ONLINE         = 8,
	MAP_TYPE_SMARTISP_OFFLINE        = 9,
	MAP_TYPE_RAW2YUV_FBD             = 10,
	MAP_TYPE_DYNAMIC_ALLOC_BY_KERNEL = 11,
	MAP_TYPE_OFFLINE_ALLOC_BY_KERNEL = 12,
	MAP_TYPE_MAX,
};

struct hisp_map_info_s {
	int fd;
	phys_addr_t paddr;
	unsigned int size;
#ifdef ISP_LINX_CPU_SUPPORT
	unsigned int prot;
#else
	unsigned int fw_mem_size;
	unsigned int ispfw_mem_size;
	unsigned int fw_mem_prot;
	unsigned int ispfw_mem_prot;
#endif
};

struct hisp_isp_mem_info_s {
#ifdef ISP_LINX_CPU_SUPPORT
	unsigned long long cpu_addr;    // 63 - 32: index; 31-0 : addr
	unsigned int core_addr;         // Index same with cpu_addr' index
	unsigned int oisp_addr;         // 0x40000000=0x60000000, only one index
#else
	unsigned int cpu_addr;
	unsigned int core_addr;
	unsigned int size;
#endif
};

struct hisp_mem_info_s {
#ifdef ISP_LINX_CPU_SUPPORT
	unsigned long long cpu_addr; // 63-32 : index ; 31-0 : offset
	unsigned int cpu_size;
#else
	unsigned int cpu_addr;
	unsigned int cpu_size;
	struct hisp_isp_mem_info_s fw_mem;
	struct hisp_isp_mem_info_s ispfw_mem;
#endif
};

#ifdef ISP_LINX_CPU_SUPPORT
typedef enum {
	ISP_CPU =  1 << 0,
	OISP_CPU = 1 << 1,
	ISP_CORE = 1 << 2,
	MASTER_MAX = 1 << 3,
} isp_master_e;

typedef enum {
	ISP_UNKNOW_MEM = 0,
	ISP_SHARE_MEM,
	ISP_PRIVATE_MEM,
	ISP_PHY_MEM,
	ISP_MEM_TYPE_MAX,
} isp_memtype_e;

typedef enum {
	SCENE_UNKNOW = 0,
	SCENE_PREV,
	SCENE_CAP,
	SCENE_FBD,
	SCENE_MAX,
} isp_scenetype_e;

struct hisp_mem_type {
	unsigned int master   : 16;
	unsigned int mem_type : 16;
};
#endif


#ifdef CONFIG_DMABUF_MM
extern int dma_heap_alloc(const char *name, size_t len, unsigned int heap_flags);
#else
extern int ion_alloc(size_t len, unsigned int heap_id_mask, unsigned int flags);
#endif

#if defined(ISP_LINX_CPU_SUPPORT)
int hisp_mem_map_addr(struct hisp_mem_type type, struct hisp_map_info_s *info, struct hisp_mem_info_s *mem);
int hisp_mem_unmap_addr(struct hisp_mem_type type, struct hisp_mem_info_s mem_info);
int hisp_mem_map_addr_sec(struct hisp_mem_type type, struct hisp_map_info_s *info, struct hisp_mem_info_s *mem);
int hisp_mem_unmap_addr_sec(struct hisp_mem_type type, struct hisp_mem_info_s mem_info);
void hisp_mem_list_clean(void);
#elif defined(CONFIG_ISP_BUF_PRIVATIZE)
int hisp_mem_map_addr(unsigned int type, struct hisp_map_info_s *info, struct hisp_mem_info_s *mem);
int hisp_mem_unmap_addr(unsigned int type, struct hisp_mem_info_s mem_info);
void hisp_mem_list_clean(void);
#else
/* hisp map addr for ispcpu */
unsigned int hisp_alloc_cpu_map_addr(struct scatterlist *sgl,
		unsigned int prot, unsigned int size, unsigned int align);
int hisp_free_cpu_map_addr(unsigned int iova, unsigned int size, unsigned int memmap_type);

/* hisp mempool api */
unsigned long hisp_mem_pool_alloc_iova(unsigned int size,
			unsigned int pool_num);
int hisp_mem_pool_free_iova(unsigned int pool_num,
				unsigned int va, unsigned int size);
unsigned int hisp_mem_map_setup(struct scatterlist *sgl,
				unsigned int iova, unsigned int size,
				unsigned int prot, unsigned int pool_num,
				unsigned int flag, unsigned int align);
unsigned int hisp_mem_map_setup_separate(struct scatterlist *sgl,
	unsigned int *iova,
	unsigned int ispcpu_size,
	unsigned int total_size,
	unsigned int prot,
	unsigned int pool_num,
	unsigned int type,
	unsigned int align);
void hisp_mem_pool_destroy(unsigned int pool_num);
void hisp_dynamic_mem_pool_clean(void);
unsigned int hisp_mem_pool_alloc_carveout(size_t size, unsigned int type);
/* MDC reserved memory */
static inline int hisp_mem_pool_free_carveout(unsigned int iova,
	size_t size) {
	(void)iova;
	(void)size;
	return 0;
}

#endif

#ifdef __cplusplus
}
#endif

#endif

