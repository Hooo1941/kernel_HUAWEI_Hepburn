/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Description: async shrink slabd.
 * Author: misasa
 * Create: 2023-10-24
 */

#ifndef _KSHRINK_SLABD_H
#define _KSHRINK_SLABD_H

#include <linux/gfp.h>
#include <linux/types.h>

bool should_shrink_slab_async(gfp_t gfp_mask, int nid,
	struct mem_cgroup *memcg, int priority);
bool is_kshrink_slabd_enable(void);

extern unsigned long shrink_slab(gfp_t gfp_mask, int nid,
	struct mem_cgroup *memcg, int priority);

#endif
