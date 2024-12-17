/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_MEM_API_H
#define HVGR_MEM_API_H

#include "hvgr_defs.h"
#include "mem/hvgr_mem_opts.h"
#ifdef HVGR_FEATURE_SYSTEM_CACHE
#include "mem/hvgr_mem_sc_hal.h"
#include "mem/hvgr_mem_sc.h"
#endif

#define WITHOUT_FLUSH_L2    0
#define WITH_FLUSH_L2       1

#define HVGR_4GVA_SIZE_4G   (4UL * 1024 * 1024 * 1024)

struct hvgr_mem_alloc_para {
	uint64_t property;
	uint64_t attribute;
	uint32_t pages;
	uint32_t max_pages;
	uint32_t extent;
	/* replay mem alloc may use the specific gva */
	uint64_t va;

	uint32_t phy_pages;
};

struct hvgr_mem_map_para {
	uint64_t property;
	uint64_t attribute;
	uint64_t gva;
	uint32_t pages;
};

struct hvgr_mem_view_para {
	uint64_t property;
	uint64_t attribute;
	uint64_t src_nums;
};

struct hvgr_mem_import_para {
	int fd;
	uint64_t attribute;
	uint64_t gva;
	uint64_t pages;
};

struct hvgr_mem_alias_para {
	uint64_t attribute;
	uint64_t stride;
	uint64_t count;
	uint64_t alias_info_ptr;
};

struct hvgr_mem_cfg_para {
	uint64_t mem_replay_size;
	uint32_t jit_large_size;
	uint32_t jit_large_cnt;
	uint32_t jit_medium_size;
	uint32_t jit_medium_cnt;
	uint32_t jit_small_size;
	uint32_t jit_small_cnt;
	uint32_t jit_default_size;
};

struct hvgr_mem_sc_policy_info {
	uint64_t gva;
	uint64_t size;
	uint32_t sc_id;
	uint32_t pading;
};

/*
 * hvgr_ioctl_mem() - support mem alloc/free/map .eg operations.
 * @ctx: Pointer to context.
 * @cmd: Key value for operation.
 * @arg: Pointer to user params.
 * Return: 0 for success and other for err.
 */
long hvgr_ioctl_mem(struct hvgr_ctx * const ctx, unsigned int cmd, unsigned long arg);

/*
 * hvgr_mem_init() - init mem for gdev.
 * @gdev: Pointer to gdev.
 * Return: 0 for success and other for err.
 */
int hvgr_mem_init(struct hvgr_device * const gdev);

/*
 * hvgr_mem_term() - deinit mem for gdev, release resource.
 * @gdev: Pointer to gdev.
 */
void hvgr_mem_term(struct hvgr_device * const gdev);

/*
 * hvgr_mem_init_proc() - init mem for ctx's kv/mmu/zone, called when ctx create.
 * @ctx: Pointer to ctx.
 * Return: 0 for success and other for err.
 */
long hvgr_mem_init_proc(struct hvgr_ctx * const ctx);

/*
 * hvgr_mem_config_proc() - configure mem for ctx, register same_va/4g_va/driver zone .eg
 * @ctx: Pointer to ctx.
 * @para: Pointer to mem cfg para.
 * Return: 0 for success and other for err.
 */
long hvgr_mem_config_proc(struct hvgr_ctx * const ctx, struct hvgr_mem_cfg_para *para);

/*
 * hvgr_mem_term_proc() - deinit mem for ctx's kv/mmu/zone, called when ctx release
 * @ctx: Pointer to ctx.
 */
void hvgr_mem_term_proc(struct hvgr_ctx * const ctx);

/*
 * hvgr_mem_allocate() - alloc pages, alloc cpu va then map gpu mem, may map cpu mem or kernel mem
 * decided by the para->property
 * @ctx: Pointer to ctx.
 * @para: Pointer to mem para.
 * @area_va: Pointer to area_va
 * @user_buf: true only if invoked by hvgr_mem_ioctl_alloc_memory
 * Return: Area for success and NULL for err.
 */
struct hvgr_mem_area *hvgr_mem_allocate(struct hvgr_ctx * const ctx,
	struct hvgr_mem_alloc_para *para, struct hvgr_mem_area_va *area_va, bool user_buf);

/*
 * hvgr_mem_free() - find the area from ctx->mem_ctx.area_rec by gva, then free the area now or
 * wait for ummmap done.
 * @ctx: Pointer to ctx.
 * @gva: Key to free.
 * Return: 0 for success and other for err.
 */
long hvgr_mem_free(struct hvgr_ctx * const ctx, uint64_t gva);

/*
 * hvgr_jit_mem_free() - find the area from ctx->mem_ctx.area_rec by gva, then free the area now or
 * wait for ummmap done.
 * @ctx: Pointer to ctx.
 * @gva: Key to free.
 * Return: 0 for success and other for err.
 */
long hvgr_jit_mem_free(struct hvgr_ctx * const ctx, uint64_t gva);

/*
 * hvgr_mem_free_area_nolock() - free the area, unmap gva/uva/kva, free uva, free pages, del area
 * now or wait for unmmap done.
 * @ctx: Pointer to ctx.
 * @area: Area to free.
 * Return: 0 for success and other for err.
 */
long hvgr_mem_free_area_nolock(struct hvgr_ctx * const ctx, struct hvgr_mem_area * const area);

/*
 * hvgr_mem_map() - map the gpu va's pages to cpu mem. Find the gpu va area, create a new map area,
 * alloc cpu va, map the pages to cpu va, gpu va area ref++.
 * @ctx: Pointer to ctx.
 * @para: Pointer to mem para.
 * @uva: Pointer to uva, out param.
 * Return: 0 for success and other for err.
 */
long hvgr_mem_map(struct hvgr_ctx * const ctx, struct hvgr_mem_map_para * const para,
	uint64_t *uva);

/*
 * hvgr_mem_unmap() - find map_area and invoke hvgr_mem_unmap_area_nolock to unmap the cpu mem.
 * and pages
 * @ctx: Pointer to ctx.
 * @uva: Use it find the map area form map_rec.
 * Return: 0 for success and other for err.
 */
long hvgr_mem_unmap(struct hvgr_ctx * const ctx, uint64_t uva);

/*
 * hvgr_mem_unmap_area_nolock() - unmap the cpu mem. Gpu area ref--, if gpu area ref == 0,
 * free the gpu area and pages
 * @ctx: Pointer to ctx.
 * @map_area: The map area to be unmapped.
 * Return: 0 for success and other for err.
 */
long hvgr_mem_unmap_area_nolock(struct hvgr_ctx * const ctx, struct hvgr_mem_area *map_area,
	bool fd_close);

/*
 * hvgr_mem_mmap() - hvgr file ops.mmap interface. Called when we alloc cpu va by vm_mmap().
 * Just register null vma->vm_ops.
 * @ctx: Pointer to ctx.
 * @vma: The vma alloced by the kernel.
 * Return: 0 for success and other for err.
 */
int hvgr_mem_mmap(struct hvgr_ctx * const ctx, struct vm_area_struct *vma);

/*
 * hvgr_mem_get_msb() - get the 4g va high 32bit addr.
 * @ctx: Pointer to ctx.
 * Return: 4g va high 32bit addr.
 */
uint32_t hvgr_mem_get_msb(struct hvgr_ctx * const ctx);

/*
 * hvgr_mem_sync() - sync the mem page start from uva with size to cpu or dev.
 * @ctx: Pointer to ctx.
 * @gva: The start va.
 * @size: The sync size in Byte.
 * @direct: Cpu or dev.
 * Return: 0 for success and other for err.
 */
long hvgr_mem_sync(struct hvgr_ctx * const ctx, uint64_t gva, uint64_t size, uint64_t direct);

/*
 * hvgr_mem_new_area() - alloc a new area.
 * @pmem: Pointer to mem ctx.
 * Return: Area for success and NULL for err.
 */
struct hvgr_mem_area *hvgr_mem_new_area(struct hvgr_mem_ctx * const pmem);

/*
 * hvgr_mem_delete_area() - free the area.
 * @area: Pointer to area.
 */
void hvgr_mem_delete_area(struct hvgr_mem_area * const area);

/*
 * hvgr_mem_new_area_with_va() - alloc a new area, alloc area page array,
 * alloc va, add area into area_rec. The real physical page is not allocated.
 * @ctx: Pointer to ctx.
 * @prot: area's property.
 * @attribute: area's attribute.
 * @max_pages: area's max_pages.
 * Return: Area for success and NULL for err.
 */
struct hvgr_mem_area *hvgr_mem_new_area_with_va(struct hvgr_ctx * const ctx,
	uint64_t prot, uint64_t attribute, uint64_t max_pages);

/*
 * hvgr_mem_rw() - read or write by gpu va.
 * @ctx: Pointer to ctx.
 * @gva: The start va.
 * @data: write or data value.
 * @write: true is write, false is read.
 */
long hvgr_mem_rw(struct hvgr_ctx * const ctx, uint64_t gva, uint64_t *data, bool write);

/*
 * hvgr_mem_import() - import mem by fd. One mem may need to be imported times partly.
 * @ctx: Pointer to ctx.
 * @para: Pointer to mem para.
 * @uva:  Pointer to user va.
 * @gva:  Pointer to gpu va.
 * Return: Area for success and NULL for err.
 */
struct hvgr_mem_area *hvgr_mem_import(struct hvgr_ctx * const ctx,
	struct hvgr_mem_import_para * const para, uint64_t *uva, uint64_t *gva);

/*
 * hvgr_mem_deconfig_proc() - deconfigure mem for ctx, unregister 4g/jit/replay rsv .eg
 * @ctx: Pointer to ctx.
 * Return: 0 for success and other for err.
 */
long hvgr_mem_deconfig_proc(struct hvgr_ctx * const ctx);

/*
 * hvgr_mem_resize() - resize the mem area started by gva to new pages.
 * @ctx: Pointer to ctx.
 * @gva: The start va.
 * @pages: The new size in page.
 * Return: 0 for success and other for err.
 */
long hvgr_mem_resize(struct hvgr_ctx * const ctx, uint64_t gva, uint64_t pages);

/*
 * hvgr_mem_change_mem_attr() - change mem's attribute. Until now, only sparse mem
 * can change attribute and can only convert HVGR_MEM_ATTR_GPU_CACHEABLE
 * @ctx: Pointer to ctx.
 * @gva: The start va.
 * @attr: The new attribute.
 * Return: 0 for success and other for err.
 */
long hvgr_mem_change_mem_attr(struct hvgr_ctx * const ctx, uint64_t gva, uint64_t attr);

/*
 * hvgr_mem_get_page_dma_addr() - get page dma map addr from page.private.
 * @page: Pointer to page.
 * Return: Page dma map addr.
 */
static inline dma_addr_t hvgr_mem_get_page_dma_addr(struct page *page)
{
	if (sizeof(dma_addr_t) > sizeof(page_private(page)))
		return ((dma_addr_t)page_to_phys(page)) << PAGE_SHIFT;
	else
		return (dma_addr_t)page_to_phys(page);
}

/*
 * hvgr_mem_free_area_when_gpu_fault() - free mem when gpu fault..
 * @ctx: Pointer to ctx.
 * @prot: area's property.
 */
void hvgr_mem_free_area_when_gpu_fault(struct hvgr_ctx * const ctx, uint64_t prot);

/*
 * hvgr_mem_unbind_all_when_gpu_fault() - unbind all sparse mem when gpu fault..
 * @ctx: Pointer to ctx.
 * @prot: area's property.
 */
void hvgr_mem_unbind_all_when_gpu_fault(struct hvgr_ctx * const ctx, uint64_t prot);

/*
 * hvgr_mem_is_cpu_coherent() - if the mem between cpu and gpu is coherent.
 * @gdev: Pointer to hvgr_device.
 * Return: True is coherent.
 */
bool hvgr_mem_is_cpu_coherent(struct hvgr_device * const gdev);

/*
 * hvgr_mem_sc_get_policy_info() - get policy info from user and check the info
 * @ctx: Pointer to ctx.
 * @policy_info: user info buf point.
 * @info_num: info nums
 * @infos: out para for infos
 * Return: 0 for success and other for err..
 */
long hvgr_mem_sc_get_policy_info(struct hvgr_ctx * const ctx, uint64_t policy_info,
	uint32_t info_num, void **infos);

/*
 * hvgr_mem_sc_update_policy() - update the sc mem policy by policy_info
 * @ctx: Pointer to ctx.
 * @policy_info: info buf point.
 * @info_num: info nums
 */
void hvgr_mem_sc_update_policy(struct hvgr_ctx * const ctx, void *policy_info,
	uint32_t info_num);

/*
 * hvgr_flush_mmu_faults() - Flush mmu bus/page fault workqueue.
 * @gdev: Pointer to gdev.
 */
void hvgr_flush_mmu_faults(struct hvgr_device * const gdev);

/*
 * hvgr_mem_flush_pt() - flush pt after mmu set up or mmu take down
 * @ctx: Pointer to ctx.
 * @gva: gpu va.
 * @pages: page nums
 */
void hvgr_mem_flush_pt(struct hvgr_ctx * const ctx, uint64_t gva, uint64_t pages, uint32_t flag);

/*
 * hvgr_mem_get_vm_opts() - get hvgr file vm_opts
 * Return: &hvgr_vm_opts, never be null
 */
const struct vm_operations_struct *hvgr_mem_get_vm_opts(void);

/*
 * hvgr_mem_vmap_user_addr() - get user addr
 * Return:user addr && offset
 */
long hvgr_mem_vmap_user_addr(struct hvgr_ctx * const ctx, uint64_t gpu_addr,
	void **target_addr, uint64_t *offset);

void hvgr_mem_vunmap_user_addr(const void *gpu_addr);

/*
 * hvgr_mem_vm_munmap() - vm munmap the mem range
 * Return:0 for success and other for err..
 */
int hvgr_mem_vm_munmap(unsigned long addr, size_t len);

/*
 * hvgr_mem_resize_expand() - resize area pages to large
 * Return:0 for success and other for err..
 */
long hvgr_mem_resize_expand(struct hvgr_ctx * const ctx, struct hvgr_mem_area *area,
	uint64_t pages);

/*
 * hvgr_mem_free_area_by_property() - free all area of property
 * @ctx: Pointer to ctx.
 * @area_rec: ctx area map.
 * @prot: property to be freed
 */
void hvgr_mem_free_area_by_property(struct hvgr_ctx * const ctx,
	struct hvgr_kv_map *area_rec, uint64_t prot);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
int hvgr_mem_vm_fault(struct vm_fault *vmf);
#else
vm_fault_t hvgr_mem_vm_fault(struct vm_fault *vmf);
#endif

#ifdef CONFIG_HVGR_SAMEVA_CACHE
struct hvgr_mem_area *hvgr_mem_get_cache(struct hvgr_ctx * const ctx, struct hvgr_mem_alloc_para * const para,
					 struct hvgr_mem_area_va *area_va,  bool user_buf);
int hvgr_mem_set_cache(struct hvgr_ctx * const ctx,
		       struct hvgr_mem_area *const area);

void hvgr_mem_disable_cache(struct hvgr_ctx * const ctx);
void hvgr_mem_enable_cache(struct hvgr_ctx * ctx);
#endif

#endif
