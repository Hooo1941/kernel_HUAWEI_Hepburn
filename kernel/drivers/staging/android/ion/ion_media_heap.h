#ifndef __ION_MEDIA_HEAP_H
#define __ION_MEDIA_HEAP_H

#include "heaps/ion_page_pool.h"
#include "ion.h"

#define MEDIA_MODE1 (0x9)
#define MEDIA_MODE2 (0x8)

bool check_alloc_path(struct ion_page_pool **cma_pools, unsigned long flag);
int ion_media_cma_pool_count(struct ion_page_pool **cma_pools);
void fill_media_pool_watermark(struct ion_page_pool **pools, struct ion_page_pool **cma_pools,
			unsigned long pool_watermark, unsigned long watermark);
void ion_media_cma_pool_page_empty(struct ion_page_pool **cma_pools, atomic_t *mode_switch);
void set_media_heap_mode(size_t mode);
void free_media_page_common(struct ion_page_pool **pools, struct page *page, pgprot_t pgprot);
void free_media_page_pool(struct ion_page_pool **pools, struct ion_page_pool **cma_pools,
			struct page *page, pgprot_t pgprot);
struct page *alloc_media_largest_available(struct ion_page_pool **cma_pools, struct ion_buffer *buffer,
				unsigned long size, unsigned int max_order);
bool check_alloc_path(struct ion_page_pool **cma_pools, unsigned long flag);
#endif
