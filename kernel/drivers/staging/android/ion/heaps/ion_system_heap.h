#ifndef _ION_SYSTEM_HEAP_H
#define _ION_SYSTEM_HEAP_H

#include <linux/mm_types.h>
#include "ion_page_pool.h"
#include "../ion.h"


atomic_t *get_ion_sys_heap_mode_switch(void);
struct ion_page_pool **get_ion_sys_heap_cam_pools(void);
struct page *media_alloc_buffer_page(struct ion_buffer *buffer,
				unsigned long order);

#endif
