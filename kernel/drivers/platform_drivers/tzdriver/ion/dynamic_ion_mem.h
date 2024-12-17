/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: dynamic Ion memory function declaration.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef DYNAMIC_MMEM_H
#define DYNAMIC_MMEM_H

#include <linux/version.h>
#include <securec.h>
#include "teek_ns_client.h"

#ifdef CONFIG_DYNAMIC_ION
#ifdef CONFIG_DMABUF_MM
#include <linux/dmabuf/mm_dma_heap.h>
#else
#include <linux/ion/mm_ion.h>
#endif
#endif

struct sg_memory {
	int dyn_shared_fd;
	struct sg_table *dyn_sg_table;
	struct dma_buf *dyn_dma_buf;
	phys_addr_t ion_phys_addr;
	size_t len;
	void *ion_virt_addr;
};
struct dynamic_mem_item {
	struct list_head head;
	uint32_t configid;
	uint32_t size;
	struct sg_memory memory;
	uint32_t cafd;
	struct tc_uuid uuid;
	uint32_t ddr_sec_region;
};

struct dynamic_mem_config {
	struct tc_uuid uuid;
	uint32_t ddr_sec_region;
};

#define MAX_ION_NENTS      1024
typedef struct ion_page_info {
	phys_addr_t phys_addr;
	uint32_t npages;
} tz_page_info;

typedef struct sglist {
	uint64_t sglist_size;
	uint64_t ion_size;
	uint64_t ion_id;
	uint64_t info_length; /* page_info number */
	struct ion_page_info page_info[0];
} tz_sg_list;

#ifdef CONFIG_DYNAMIC_ION

bool is_ion_param(uint32_t param_type);
int init_dynamic_mem(void);
int load_app_use_configid(uint32_t configid, uint32_t cafd,
	const struct tc_uuid *uuid, uint32_t size, int32_t *ret_origin);
void kill_ion_by_cafd(unsigned int cafd);
void kill_ion_by_uuid(const struct tc_uuid *uuid);
int load_image_for_ion(const struct load_img_params *params, int32_t *ret_origin);
int alloc_for_ion_sglist(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, uint8_t kernel_params,
	uint32_t param_type, unsigned int index);
int alloc_for_ion(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, uint8_t kernel_params,
	uint32_t param_type, unsigned int index);
#else
static inline bool is_ion_param(uint32_t param_type)
{
	(void)param_type;
	return false;
}

static inline int load_image_for_ion(const struct load_img_params *params, int32_t *ret_origin)
{
	(void)params;
	(void)ret_origin;
	return 0;
}

static inline int init_dynamic_mem(void)
{
	return 0;
}

static inline int load_app_use_configid(uint32_t configid, uint32_t cafd,
	const struct tc_uuid *uuid, uint32_t size)
{
	(void)configid;
	(void)cafd;
	(void)uuid;
	(void)size;
	return 0;
}

static inline void kill_ion_by_cafd(unsigned int cafd)
{
	(void)cafd;
	return;
}

static inline void kill_ion_by_uuid(const struct tc_uuid *uuid)
{
	(void)uuid;
	return;
}

static inline int alloc_for_ion_sglist(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, uint8_t kernel_params,
	uint32_t param_type, unsigned int index)
{
	(void)call_params;
	(void)op_params;
	(void)kernel_params;
	(void)param_type;
	(void)index;
	tloge("not support seg and related feature!\n");
	return -1;
}

static inline int alloc_for_ion(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, uint8_t kernel_params,
	uint32_t param_type, unsigned int index)
{
	(void)call_params;
	(void)op_params;
	(void)kernel_params;
	(void)param_type;
	(void)index;
	tloge("not support ion and related feature!\n");
	return -1;
}
#endif
#endif
