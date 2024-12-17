/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * This file is the dvfs implementation of the PM module.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_LOG_API_H
#define HVGR_LOG_API_H
#include <linux/device.h>
#include <linux/types.h>
#include "hvgr_defs.h"
#include "utils/log/hvgr_log.h"

enum HVGR_LOG_MODULE_BIT {
	HVGR_PM_LOG_BIT = 0,
	HVGR_DM_LOG_BIT,
	HVGR_MEM_LOG_BIT,
	HVGR_CQ_LOG_BIT,
	HVGR_DATAN_LOG_BIT,
	HVGR_SCH_LOG_BIT,
	HVGR_SYNC_LOG_BIT,
#if defined(HVGR_SUPPORT_SPEC_LOW) && HVGR_SUPPORT_SPEC_LOW != 0
	HVGR_SPEC_LOW_LOG_BIT,
#endif
	HVGR_FENCE_LOG_BIT,
};

#define HVGR_PM       (uint32_t)(1U << (uint32_t)HVGR_PM_LOG_BIT)
#define HVGR_DM       (uint32_t)(1U << (uint32_t)HVGR_DM_LOG_BIT)
#define HVGR_MEM      (uint32_t)(1U << (uint32_t)HVGR_MEM_LOG_BIT)
#define HVGR_CQ       (uint32_t)(1U << (uint32_t)HVGR_CQ_LOG_BIT)
#define HVGR_DATAN    (uint32_t)(1U << (uint32_t)HVGR_DATAN_LOG_BIT)
#define HVGR_SCH      (uint32_t)(1U << (uint32_t)HVGR_SCH_LOG_BIT)
#define HVGR_SYNC     (uint32_t)(1U << (uint32_t)HVGR_SYNC_LOG_BIT)
#if defined(HVGR_SUPPORT_SPEC_LOW) && HVGR_SUPPORT_SPEC_LOW != 0
#define HVGR_SPEC_LOW (uint32_t)(1U << (uint32_t)HVGR_SPEC_LOW_LOG_BIT)
#endif
#define HVGR_FENCE    (uint32_t)(1U << (uint32_t)HVGR_FENCE_LOG_BIT)

#if defined(HVGR_SUPPORT_SPEC_LOW) && HVGR_SUPPORT_SPEC_LOW != 0
#define HVGR_LOG_OPEN_ALL \
	(HVGR_PM | HVGR_DM | HVGR_MEM | HVGR_CQ | HVGR_DATAN | HVGR_SCH | \
	HVGR_SYNC | HVGR_SPEC_LOW | HVGR_FENCE)
#else
#define HVGR_LOG_OPEN_ALL \
	(HVGR_PM | HVGR_DM | HVGR_MEM | HVGR_CQ | HVGR_DATAN | HVGR_SCH | \
	HVGR_SYNC | HVGR_FENCE)
#endif

#define HVGR_LOG_CLOSE_ALL  0

#define hvgr_info(gdev, module, fmt, ...) \
	hvgr_log(gdev, module, HVGR_LOG_INFO, fmt, ##__VA_ARGS__)

#define hvgr_debug(gdev, module, fmt, ...) \
	hvgr_log(gdev, module, HVGR_LOG_DEBUG, fmt, ##__VA_ARGS__)

#define hvgr_err(gdev, module, fmt, ...) \
	hvgr_log(gdev, module, HVGR_LOG_ERROR, fmt, ##__VA_ARGS__)

/*
 * hvgr_log_init - Log module init
 *
 * @param gdev - The gpu devie object pointer.
 *
 * @date - 2022-01-28
 *
 * @return: void.
 */
void hvgr_log_init(struct hvgr_device * const gdev);

/*
 * hvgr_start_timing - Get the start timestamp.
 *
 * @date - 2022-01-28
 *
 * @return: Start timestamp, in ns
 */
uint64_t hvgr_start_timing(void);

/*
 * hvgr_end_timing_and_log - Get the end timestamp, calculates the consumed time,
 * and records logs.
 * This interface is used together with hvgr_start_timing. In the function entry,
 * call hvgr_start_timing to get the start timestamp, then call hvgr_end_timing_and_log
 * at the function exit to calculate the consumed time and record logs.
 *
 * @param gdev - The gpu devie object pointer.
 * @param module - Module, such as HVGR_PM, HVGR_CQ, etc.
 * @param start_time - start timesatmp, obtained by hvgr_start_timing.
 * @param msg - msg written to the log.
 *
 * @date - 2022-01-28
 */
void hvgr_end_timing_and_log(struct hvgr_device *gdev, uint32_t module, uint64_t start_time,
	const char *msg);

/*
 * hvgr_dump_stack - dump_stack.
 *
 * @date - 2022-01-28
 *
 */
void hvgr_dump_stack(void);

/*
 * hvgr_set_log_switch - Change log switch status.
 * This interface is only called by debugfs
 *
 * @param data - Private data passed by debugfs frame.
 *               The private data is saved when the debugfs node is created.
 * @param buf - Input data buffer.
 * @param count - buf size.
 *
 * @return: @count if the function succeeded. An error code on failure.
 *
 * @date - 2022-02-23
 */
ssize_t hvgr_set_log_switch(void *data, const char *buf, size_t count);

/*
 * hvgr_show_log_switch - Show log switch status.
 * This interface is only called by debugfs
 *
 * @param data - Private data passed by debugfs frame.
 *               The private data is saved when the debugfs node is created.
 * @param buf - Output data buffer.
 *
 * @return: The number of bytes output to @buf.
 *
 * @date - 2022-02-23
 */
ssize_t hvgr_show_log_switch(void *data, char * const buf);
#endif

