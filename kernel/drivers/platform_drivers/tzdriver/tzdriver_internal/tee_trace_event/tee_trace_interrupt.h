/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: exported functions for tracing interrupts in teeos
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

#ifndef TEE_TRACE_INTERRUPT_H
#define TEE_TRACE_INTERRUPT_H

#define LOGGER_INTERVAL     		2000
#define LOGGER_RUNNING      		1
#define LOGGER_NOT_RUNNING  		0

#ifndef CONFIG_INT_TRACE_LOG_PATH
#define CONFIG_INT_TRACE_LOG_PATH       "/data/log/tee/int_trace"
#endif

#ifndef CONFIG_EVENT_TRACE_LOG_PATH
#define CONFIG_EVENT_TRACE_LOG_PATH 	"/data/log/tee/event_trace"
#endif

#define TRACE_LOG_OPEN_FILE_MODE	0640U
#define LOG_STR_BUFFER_LEN 			256

void interrupt_trace_start(void);
void interrupt_trace_stop(void);
void free_interrupt_trace(void);
#endif
