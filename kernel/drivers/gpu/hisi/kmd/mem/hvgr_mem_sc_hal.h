/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_MEM_SC_HAL_H
#define HVGR_MEM_SC_HAL_H

#include "hvgr_defs.h"
#include "hvgr_sc_api.h"

void hvgr_sc_init(struct hvgr_device * const gdev);

void hvgr_sc_deinit(struct hvgr_device * const gdev);

uint32_t hvgr_sc_id_to_policy_id(uint32_t idx);

int hvgr_sc_vm_insert_pfn_prot(struct page **pages, uint32_t page_nr, pgprot_t *prot);

bool hvgr_sc_gid_remap_is_enable(void);

void hvgr_sc_config_remap(struct hvgr_device * const gdev);

void hvgr_sc_set_page_osh(struct page *page, uint32_t osh);

#endif
