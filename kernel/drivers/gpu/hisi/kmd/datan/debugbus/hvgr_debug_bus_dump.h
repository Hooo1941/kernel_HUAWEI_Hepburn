/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_DEBUG_BUS_DUMP_H
#define HVGR_DEBUG_BUS_DUMP_H

#include <linux/types.h>

#include "hvgr_defs.h"

#include "hvgr_debug_bus_config.h"
#include "hvgr_regmap_debug_bus.h"
#include "hvgr_gpu_id.h"

struct bus_dump_resource_info {
	struct hvgr_device *gdev;
	/* write file offset */
	loff_t pos;
	/* debugbus dump buffer ptr */
	char *buffer;
	/* debugbus dump buffer size */
	u32 buffer_size;
};

#ifdef CONFIG_HVGR_DFX_DEBUG_BUS_DUMP

/*
 * @brief initialize debug bus module
 */
void kdatan_bus_dump_thread_init(struct hvgr_device * const gdev);

/*
 * @brief terminate debug bus module
 */
void kdatan_bus_dump_thread_term(struct hvgr_device * const gdev);

/*
 * @brief start dump debug bus data if debug bus is enable
 */
void kdatan_debug_bus_start_dump(struct hvgr_device * const gdev);

/*
 * @brief init cbit config while init or after gpu reset
 */
void kdatan_debug_bus_cbit_config(struct hvgr_device * const gdev);

/*
 * @brief dump job chain status
 */
void kdatan_dump_chain_state(struct hvgr_device * const gdev);

/*
 * @brief dump debugbus data with type
 */
void hvgr_kdatan_dump_debug_bus_data(struct hvgr_device * const gdev, u32 type);

/*
 * @brief dump debugbus by ecall hvgr_kdatan_dump_debug_bus_data_for_ecall
 */
void hvgr_kdatan_dump_debug_bus_data_for_ecall(void);

#endif /* CONFIG_HVGR_DFX_DEBUG_BUS_DUMP */

/*
 * @brief delay reset gpu for dump debugbus when page fault happen
 */
bool kdatan_pagefault_delay_reset(void);

/*
 * @brief dump debugbus of cq module.
 */
void kdatan_bus_dump_cq_mode(struct hvgr_device * const gdev);

/*
 * @brief get debugbus buffer dump size.
 */
u32 kdatan_debug_bus_get_buffer_size(struct hvgr_device * const gdev);

/*
 * @brief judge debugbus dump twice if needed.
 */
bool kdatan_need_to_dump_duplicately(void);

/*
 * @brief write debugbus buffer to file.
 */
void hvgr_debug_bus_write_to_file(char *buffer, char *file_name);

#endif /* HVGR_DEBUG_BUS_DUMP_H END */

