#ifndef _ION_BUFFER_REUSE_H
#define _ION_BUFFER_REUSE_H

#include "ion.h"

#ifdef CONFIG_ION_BUFFER_REUSE
size_t ion_heap_reuselist_size(struct ion_heap *heap);
bool ion_heap_try_reuselist_add(struct ion_heap *heap, struct ion_buffer *buffer);
size_t ion_heap_drain_from_reuselist(struct ion_heap *heap, size_t size,
				     bool skip_pools);
size_t ion_heap_reuselist_drain(struct ion_heap *heap, size_t size);
struct ion_buffer *ion_heap_alloc_from_reuselist(struct ion_heap *heap, size_t size);
int ion_heap_init_buf_reuse(struct ion_heap *heap);
void ion_buffer_before_reuse_list(struct ion_buffer *buffer);
#else
static inline size_t ion_heap_reuselist_size(struct ion_heap *heap)
{
	(void) heap;

	return 0;
}

static inline bool ion_heap_try_reuselist_add(struct ion_heap *heap, struct ion_buffer *buffer)
{
	(void) heap;
	(void) buffer;

	return false;
}

static inline size_t ion_heap_drain_from_reuselist(struct ion_heap *heap, size_t size,
						   bool skip_pools)
{
	(void) heap;
	(void) size;
	(void) skip_pools;

	return 0;
}

static inline size_t ion_heap_reuselist_drain(struct ion_heap *heap, size_t size)
{
	(void) heap;
	(void) size;

	return 0;
}

static inline struct ion_buffer *ion_heap_alloc_from_reuselist(struct ion_heap *heap, size_t size)
{
	(void) heap;
	(void) size;

	return NULL;
}

static inline int ion_heap_init_buf_reuse(struct ion_heap *heap)
{
	(void) heap;

	return 0;
}

static inline void ion_buffer_before_reuse_list(struct ion_buffer *buffer)
{
	(void) buffer;
}
#endif
#endif
