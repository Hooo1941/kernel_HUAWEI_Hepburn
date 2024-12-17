/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: exported functions for tee_trace_event
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

#ifndef TEE_TRACE_EVENT_H
#define TEE_TRACE_EVENT_H

#include <linux/types.h>
#include <linux/smp.h>
#include "teek_client_constants.h"

/* maximum trace event per cpu stream */
#define TEE_TRACE_EVENT_NUM		20000
/* maxium trace task of 'sched_in/out' event */
#define TEE_TRACE_TASK_MAX		8
#define MAX_UINT64_TIME			(uint64_t)(~((uint64_t)0))

#define TRACE_LOG_LOOP_ENABLED	1
#define TRACE_LOG_LOOP_DISABLED	0

#ifdef CONFIG_TEE_TRACE
void tee_trace_add_event(enum tee_event_id id, uint64_t add_info);

int tee_trace_event_enable(void);
int tee_trace_event_start(void);
int tee_trace_event_start_loop_record(void);
int tee_trace_event_stop(void);

struct tee_trace_view_t {
	uint64_t start;
	uint32_t total;
	uint32_t freq;
	uint32_t buffer_is_full;
	uint32_t end[NR_CPUS];
	uint32_t at[NR_CPUS];
};

struct trace_log_info {
	const char *event_name;
	uint32_t event_id;
	uint32_t ca_pid;
	uint32_t cpu;
	uint64_t time;
	uint64_t add_info;
};

void get_tee_trace_start(struct tee_trace_view_t *view);
int get_tee_trace_next(struct tee_trace_view_t *view, struct trace_log_info *log_info,
	bool cyclically);
const char *get_tee_trace_task_name(uint32_t task_idx);
void free_event_mem(void);
#endif

#endif
