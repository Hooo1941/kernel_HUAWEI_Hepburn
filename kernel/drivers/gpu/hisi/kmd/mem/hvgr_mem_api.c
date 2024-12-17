/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_mem_api.h"

#include <linux/mm.h>
#include <linux/dma-buf.h>
#ifdef HVGR_FEATURE_SYSTEM_CACHE
#include "hvgr_sc_api.h"
#endif

#include "hvgr_ioctl_mem.h"
#include "hvgr_mem_zone.h"
#include "hvgr_dm_api.h"
#include "hvgr_mem_alias.h"
#include "hvgr_mem_sparse.h"
#include "hvgr_mem_debugfs.h"
#include "hvgr_mem_import.h"
#include "hvgr_log_api.h"

static bool hvgr_mem_ioctl_alloc_para_is_valid(uint64_t property)
{
	/* Check the INVALID property from UMD */
	if ((property == HVGR_MEM_PROPERTY_JIT) ||
		(property == HVGR_MEM_PROPERTY_DRIVER) ||
		(property == HVGR_MEM_PROPERTY_ALIAS))
		return false;

	return true;
}

static bool hvgr_mem_ioctl_para_is_valid(uint64_t property)
{
	/* Check the INVALID property from UMD */
	if ((property == HVGR_MEM_PROPERTY_JIT) ||
		(property == HVGR_MEM_PROPERTY_DRIVER))
		return false;

	return true;
}

static long hvgr_mem_ioctl_alloc_memory(struct hvgr_ctx * const ctx,
	hvgr_ioctl_para_alloc_memory * const para)
{
	struct hvgr_mem_area *area = NULL;
	struct hvgr_mem_alloc_para info;
	struct hvgr_mem_area_va area_va = {0, 0, 0};

	if (!hvgr_mem_ioctl_alloc_para_is_valid(para->in.property))
		return -EPERM;

	info.property = para->in.property;
	info.attribute = para->in.attribute;
	info.pages = para->in.pages;
	info.max_pages = para->in.max_pages;
	info.extent = para->in.extent;
	info.va = para->in.va;
	info.phy_pages = para->in.phy_pages;
	area = hvgr_mem_allocate(ctx, &info, &area_va, true);
	if (area == NULL)
		return -ENOMEM;

	para->out.uva = area_va.uva;
	para->out.gva = area_va.gva;
	para->out.attr = info.attribute;

	return 0;
}

static long hvgr_mem_ioctl_free_memory(struct hvgr_ctx * const ctx,
	hvgr_ioctl_para_free_memory * const para)
{
	struct hvgr_mem_area *area = NULL;
	uint32_t area_prop;

	mutex_lock(&ctx->mem_ctx.area_mutex);
	area = hvgr_mem_zone_find_area_by_gva_range(&ctx->mem_ctx, para->in.gva);
	if (area == NULL) {
		mutex_unlock(&ctx->mem_ctx.area_mutex);
		return -EINVAL;
	}

	if (!area->user_buf) {
		mutex_unlock(&ctx->mem_ctx.area_mutex);
		hvgr_err(ctx->gdev, HVGR_MEM, "%s ctx_%u perm deny: area->prop 0x%lx attr 0x%lx",
			__func__, ctx->id, area->property, area->attribute);
		return -EPERM;
	}

	if (!hvgr_mem_ioctl_para_is_valid(area->zone->proterty)) {
		mutex_unlock(&ctx->mem_ctx.area_mutex);
		return -EPERM;
	}
	area_prop = (uint32_t)area->zone->proterty;
	if (hvgr_mem_free_area_nolock(ctx, area) != 0) {
		mutex_unlock(&ctx->mem_ctx.area_mutex);
		return -EINVAL;
	}
	mutex_unlock(&ctx->mem_ctx.area_mutex);
	if (area_prop == HVGR_MEM_PROPERTY_IMPORT)
		hvgr_del_bg_dmabuf_ctx_from_global_list(ctx);
	return 0;
}

static long hvgr_mem_ioctl_map_memory(struct hvgr_ctx * const ctx,
	hvgr_ioctl_para_map_memory * const para)
{
	struct hvgr_mem_map_para info;
	uint64_t uva = 0;

	info.attribute = para->in.attribute;
	info.gva = para->in.gva;
	info.pages = (uint32_t)para->in.pages;
	info.property = 0;
	if (hvgr_mem_map(ctx, &info, &uva) != 0)
		return -ENOMEM;

	para->out.uva = uva;

	return 0;
}

static inline long hvgr_mem_ioctl_unmap_memory(struct hvgr_ctx * const ctx,
	hvgr_ioctl_para_unmap_memory * const para)
{
	return hvgr_mem_unmap(ctx, para->in.uva);
}

static inline long hvgr_mem_ioctl_sync_memory(struct hvgr_ctx * const ctx,
	hvgr_ioctl_para_sync_memory * const para)
{
	return hvgr_mem_sync(ctx, para->in.uva, para->in.size, para->in.direct);
}

static long hvgr_mem_ioctl_import_memory(struct hvgr_ctx * const ctx,
	hvgr_ioctl_para_import_memory * const para)
{
	struct hvgr_mem_area *area = NULL;
	struct hvgr_mem_import_para info;

	info.fd = para->in.fd;
	info.attribute = para->in.attribute;
	info.gva = para->in.gva;
	info.pages = para->in.pages;
	area = hvgr_mem_import(ctx, &info, &para->out.uva, &para->out.gva);
	if (area == NULL)
		return -ENOMEM;

	return 0;
}

static long hvgr_mem_ioctl_query_memory(struct hvgr_ctx * const ctx,
	hvgr_ioctl_para_query_memory * const para)
{
	struct hvgr_mem_area *area = NULL;

	mutex_lock(&ctx->mem_ctx.area_mutex);
	/* Find area by gva range */
	area = hvgr_mem_zone_find_area_by_gva(&ctx->mem_ctx, para->in.gva);
	if (area == NULL) {
		mutex_unlock(&ctx->mem_ctx.area_mutex);
		return -EINVAL;
	}

	para->out.property = area->property;
	para->out.attribute = area->attribute;
	para->out.pages = area->pages;
	para->out.max_pages = area->max_pages;
	para->out.va = area->gva;
	para->out.phy_pages = area->phy_pages;
	mutex_unlock(&ctx->mem_ctx.area_mutex);
	return 0;
}

static inline long hvgr_mem_ioctl_resize_memory(struct hvgr_ctx * const ctx,
	hvgr_ioctl_para_resize_memory * const para)
{
	return hvgr_mem_resize(ctx, para->in.gva, para->in.pages);
}

static long hvgr_mem_ioctl_get_fd_size_memory(struct hvgr_ctx * const ctx,
	hvgr_ioctl_para_get_fd_size_memory * const para)
{
	struct dma_buf *dmabuf = NULL;

	dmabuf = dma_buf_get(para->in.fd);
	if (IS_ERR_OR_NULL(dmabuf)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u get fd (%d) dma buf fail",
			ctx->id, para->in.fd);
		return -EINVAL;
	}

	para->out.pages = (PAGE_ALIGN(dmabuf->size) >> PAGE_SHIFT);

	dma_buf_put(dmabuf);
	return 0;
}

static long hvgr_mem_ioctl_import_user_memory(struct hvgr_ctx * const ctx,
	hvgr_ioctl_para_import_user_memory * const para)
{
	hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u unsupport import user buf", ctx->id);
	return -ENOMEM;
}

static long hvgr_mem_ioctl_alias_memory(struct hvgr_ctx * const ctx,
	hvgr_ioctl_para_alias_memory * const para)
{
	struct hvgr_mem_area *area = NULL;
	struct hvgr_mem_alias_para info;

	info.attribute = para->in.attribute;
	info.stride = para->in.stride;
	info.count = para->in.count;
	info.alias_info_ptr = para->in.alias_info_ptr;
	area = hvgr_mem_alias(ctx, &info, &para->out.pages, &para->out.gva);
	if (area == NULL)
		return -ENOMEM;

	return 0;
}

static inline long hvgr_mem_ioctl_add_profile_memory(struct hvgr_ctx * const ctx,
	hvgr_ioctl_para_profile_memory * const para)
{
	return hvgr_mem_add_profile(ctx, para->in.user_buf, para->in.size);
}

#ifdef CONFIG_MEM_UVA_OFFSET
static long hvgr_mem_ioctl_uva_offset_memory(struct hvgr_ctx * const ctx,
	hvgr_ioctl_para_uva_offset_memory * const para)
{
	struct hvgr_mem_area *area = NULL;
	uint64_t uva = para->in.uva;
	uint64_t size = para->in.size;

	if (uva > (U64_MAX - size)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u offset params invalid 0x%lx:0x%lx.",
			ctx->id, uva, size);
		return -EINVAL;
	}

	mutex_lock(&ctx->mem_ctx.area_mutex);
	area = hvgr_mem_zone_find_area_by_gva_range(&ctx->mem_ctx, uva);
	if (area == NULL) {
		area = hvgr_mem_zone_find_area_by_uva_range(&ctx->mem_ctx, uva);
		if (area == NULL) {
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u offset no area with key 0x%lx.",
				ctx->id, uva);
			mutex_unlock(&ctx->mem_ctx.area_mutex);
			return -EINVAL;
		}
	}

	if ((uva + size) > (area->uva + (area->pages << PAGE_SHIFT)))
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u offset out of range 0x%lx:0x%lx.",
			ctx->id, uva, size);

	para->out.offset = uva - area->uva + (area->page_offset << PAGE_SHIFT);
	mutex_unlock(&ctx->mem_ctx.area_mutex);
	return 0;
}
#endif

static inline long hvgr_mem_ioctl_bind_sparse_memory(struct hvgr_ctx * const ctx,
	hvgr_ioctl_para_sparse_bind * const para)
{
	return hvgr_mem_sparse_bind(ctx, para->sparse_addr, para->mem_addr, para->pages);
}

static inline long hvgr_mem_ioctl_unbind_sparse_memory(struct hvgr_ctx * const ctx,
	hvgr_ioctl_para_sparse_unbind * const para)
{
	return hvgr_mem_sparse_unbind(ctx, para->sparse_addr, para->pages);
}

static inline long hvgr_mem_ioctl_change_memory_attr(struct hvgr_ctx * const ctx,
	hvgr_ioctl_para_change_memory_attr * const para)
{
	return hvgr_mem_change_mem_attr(ctx, para->gva, para->attribute);
}

#ifdef HVGR_FEATURE_SYSTEM_CACHE
static inline long hvgr_mem_ioctl_change_pgid_attr(struct hvgr_ctx * const ctx,
	hvgr_ioctl_para_change_pgid_attr * const para)
{
	return hvgr_sc_set_gid_policy(para->pgid, para->attribute);
}
#endif

long hvgr_ioctl_mem(struct hvgr_ctx * const ctx, unsigned int cmd, unsigned long arg)
{
	long ret = -EINVAL;

	switch (cmd) {
	case HVGR_IOCTL_ALLOC_MEM:
		hvgr_ioctl_handle_wr(ctx, HVGR_IOCTL_ALLOC_MEM,
			hvgr_mem_ioctl_alloc_memory, arg, hvgr_ioctl_para_alloc_memory);
		break;
	case HVGR_IOCTL_FREE_MEM:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_FREE_MEM,
			hvgr_mem_ioctl_free_memory, arg, hvgr_ioctl_para_free_memory);
		break;
	case HVGR_IOCTL_MAP_MEM:
		hvgr_ioctl_handle_wr(ctx, HVGR_IOCTL_MAP_MEM,
			hvgr_mem_ioctl_map_memory, arg, hvgr_ioctl_para_map_memory);
		break;
	case HVGR_IOCTL_UNMAP_MEM:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_UNMAP_MEM,
			hvgr_mem_ioctl_unmap_memory, arg, hvgr_ioctl_para_unmap_memory);
		break;
	case HVGR_IOCTL_SYNC_MEM:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_SYNC_MEM,
			hvgr_mem_ioctl_sync_memory, arg, hvgr_ioctl_para_sync_memory);
		break;
	case HVGR_IOCTL_IMPORT_MEMORY:
		hvgr_ioctl_handle_wr(ctx, HVGR_IOCTL_IMPORT_MEMORY,
			hvgr_mem_ioctl_import_memory, arg, hvgr_ioctl_para_import_memory);
		break;
	case HVGR_IOCTL_QUERY_MEM:
		hvgr_ioctl_handle_wr(ctx, HVGR_IOCTL_QUERY_MEM,
			hvgr_mem_ioctl_query_memory, arg, hvgr_ioctl_para_query_memory);
		break;
	case HVGR_IOCTL_RESIZE_MEM:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_RESIZE_MEM,
			hvgr_mem_ioctl_resize_memory, arg, hvgr_ioctl_para_resize_memory);
		break;
	case HVGR_IOCTL_GET_FD_SIZE_MEM:
		hvgr_ioctl_handle_wr(ctx, HVGR_IOCTL_GET_FD_SIZE_MEM,
			hvgr_mem_ioctl_get_fd_size_memory, arg, hvgr_ioctl_para_get_fd_size_memory);
		break;
	case HVGR_IOCTL_IMPORT_USER_MEMORY:
		hvgr_ioctl_handle_wr(ctx, HVGR_IOCTL_IMPORT_USER_MEMORY,
			hvgr_mem_ioctl_import_user_memory, arg, hvgr_ioctl_para_import_user_memory);
		break;
	case HVGR_IOCTL_ALIAS_MEMORY:
		hvgr_ioctl_handle_wr(ctx, HVGR_IOCTL_ALIAS_MEMORY,
			hvgr_mem_ioctl_alias_memory, arg, hvgr_ioctl_para_alias_memory);
		break;
	case HVGR_IOCTL_ADD_PROFILE_MEM:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_ADD_PROFILE_MEM,
			hvgr_mem_ioctl_add_profile_memory, arg, hvgr_ioctl_para_profile_memory);
		break;
#ifdef CONFIG_MEM_UVA_OFFSET
	case HVGR_IOCTL_UVA_OFFSET_MEMORY:
		hvgr_ioctl_handle_wr(ctx, HVGR_IOCTL_UVA_OFFSET_MEMORY,
			hvgr_mem_ioctl_uva_offset_memory, arg, hvgr_ioctl_para_uva_offset_memory);
		break;
#endif
	case HVGR_IOCTL_SPARSE_MEM_BIND:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_SPARSE_MEM_BIND,
			hvgr_mem_ioctl_bind_sparse_memory, arg, hvgr_ioctl_para_sparse_bind);
		break;
	case HVGR_IOCTL_SPARSE_MEM_UNBIND:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_SPARSE_MEM_UNBIND,
			hvgr_mem_ioctl_unbind_sparse_memory, arg, hvgr_ioctl_para_sparse_unbind);
		break;
	case HVGR_IOCTL_CHANGE_MEM_ATTR:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_CHANGE_MEM_ATTR,
			hvgr_mem_ioctl_change_memory_attr, arg, hvgr_ioctl_para_change_memory_attr);
		break;
#ifdef HVGR_FEATURE_SYSTEM_CACHE
	case HVGR_IOCTL_CHANGE_PGID_ATTR:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_CHANGE_PGID_ATTR,
			hvgr_mem_ioctl_change_pgid_attr, arg, hvgr_ioctl_para_change_pgid_attr);
		break;
#endif
	default:
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u ioctl cmd 0x%x is INVALID", ctx->id, cmd);
		break;
	}

exit:
	return ret;
}
