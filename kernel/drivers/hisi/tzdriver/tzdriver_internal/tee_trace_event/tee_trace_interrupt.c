/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: functions for tracing interrupts in teeos
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

#include "tee_trace_interrupt.h"

#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <securec.h>
#include "cmdmonitor.h"
#include "tee_trace_event.h"

static struct delayed_work g_interrupt_trace_log_work;
static struct tee_trace_view_t g_interrupt_log_view = { 0, 0, 0, 0, { 0 }, { 0 } };
volatile static int g_interrupt_trace_log_work_state = LOGGER_NOT_RUNNING;
volatile static bool g_trace_log_work_inited = false;
static struct file *g_int_log_fp = NULL;
static struct file *g_event_log_fp = NULL;
static loff_t g_int_log_fp_pos = 0;
static loff_t g_event_log_fp_pos = 0;
static bool is_interrupt_event(uint32_t event_id)
{
	return (event_id == INTERRUPT_HANDLE_SPI_START || event_id == INTERRUPT_HANDLE_SPI_END ||
		event_id == INTERRUPT_HANDLE_START || event_id == INTERRUPT_HANDLE_END ||
		event_id == INTERRUPT_HANDLE_SPI_REE_RESPONSE || event_id == INTERRUPT_HANDLE_SPI_REE_MISS ||
		event_id == INTERRUPT_HANDLE_SPI_REE_SCHEDULED);
}

static void close_trace_log(void)
{
	if (g_int_log_fp != NULL) {
		vfs_fsync(g_int_log_fp, 0);
		filp_close(g_int_log_fp, 0);
		g_int_log_fp = NULL;
	}

	if (g_event_log_fp != NULL) {
		vfs_fsync(g_event_log_fp, 0);
		filp_close(g_event_log_fp, 0);
		g_event_log_fp = NULL;
	}
}

static void refresh_int_trace_log(void)
{
	g_int_log_fp_pos = 0;
	g_event_log_fp_pos = 0;

	g_int_log_fp = filp_open(CONFIG_INT_TRACE_LOG_PATH, O_CREAT | O_RDWR | O_TRUNC, TRACE_LOG_OPEN_FILE_MODE);
	if (!g_int_log_fp || IS_ERR(g_int_log_fp)) {
		tloge("create interrupt trace log file err %ld\n", PTR_ERR(g_int_log_fp));
		g_int_log_fp = NULL;
		return;
	}

	g_event_log_fp = filp_open(CONFIG_EVENT_TRACE_LOG_PATH, O_CREAT | O_RDWR | O_TRUNC, TRACE_LOG_OPEN_FILE_MODE);
	if (!g_event_log_fp || IS_ERR(g_event_log_fp)) {
		tloge("create event trace log file err %ld\n", PTR_ERR(g_event_log_fp));
		g_event_log_fp = NULL;
		return;
	}
}

static void init_tee_trace_view(void)
{
	memset_s(&g_interrupt_log_view, sizeof(struct tee_trace_view_t),
		0, sizeof(struct tee_trace_view_t));
	get_tee_trace_start(&g_interrupt_log_view);
	smp_mb();
}

static void write_interrupt_trace_log(char *log_str_buffer)
{
	struct trace_log_info log_info;
	ssize_t write_len;

	if (log_str_buffer == NULL || g_interrupt_log_view.freq == 0)
		return;
	while (g_interrupt_trace_log_work_state == LOGGER_RUNNING &&
			get_tee_trace_next(&g_interrupt_log_view, &log_info, true) != -1) {
		log_info.time = log_info.time + g_interrupt_log_view.start * USEC_PER_SEC /
			g_interrupt_log_view.freq;
		int len = snprintf_s(log_str_buffer, LOG_STR_BUFFER_LEN, LOG_STR_BUFFER_LEN - 1,
			"[%016lluus]cpu=%u, info=%llu, pid=%u, event=%s\n", log_info.time, log_info.cpu, log_info.add_info,
			log_info.ca_pid, log_info.event_name);
		if (len < 0) {
			tloge("failed to write interrupt trace str to buffer\n");
			return;
		}
		if (is_interrupt_event(log_info.event_id) && g_int_log_fp != NULL) {
			write_len = kernel_write(g_int_log_fp, log_str_buffer, len, &g_int_log_fp_pos);
			if (write_len < 0) {
				tloge("Failed to write interrupt trace log %zd\n", write_len);
				return;
			}
		}
		if (g_event_log_fp == NULL)
			return;
		write_len = kernel_write(g_event_log_fp, log_str_buffer, len, &g_event_log_fp_pos);
		if (write_len < 0) {
			tloge("Failed to write event trace log %zd\n", write_len);
			return;
		}
	}
}

static void interrupt_trace_logger(struct work_struct *work)
{
	(void)(work);
	char *log_str_buffer = kzalloc(LOG_STR_BUFFER_LEN, GFP_KERNEL);
	if (!log_str_buffer) {
		tloge("failed to alloc trace log str buffer\n");
		return;
	}

	do {
		write_interrupt_trace_log(log_str_buffer);
		msleep(LOGGER_INTERVAL);
	} while(g_interrupt_trace_log_work_state == LOGGER_RUNNING);
	if (log_str_buffer != NULL)
		kfree(log_str_buffer);
}

void interrupt_trace_start(void)
{
	if (!g_trace_log_work_inited) {
		INIT_DEFERRABLE_WORK((struct delayed_work *)
			(uintptr_t)&g_interrupt_trace_log_work, interrupt_trace_logger);
		g_trace_log_work_inited = true;
	}

	init_tee_trace_view();
	refresh_int_trace_log();

	if (g_interrupt_trace_log_work_state == LOGGER_NOT_RUNNING) {
		g_interrupt_trace_log_work_state = LOGGER_RUNNING;
		schedule_delayed_work(&g_interrupt_trace_log_work,
			usecs_to_jiffies(S_TO_US));
	}
}

void interrupt_trace_stop(void)
{
	g_interrupt_trace_log_work_state = LOGGER_NOT_RUNNING;
	close_trace_log();
}

void free_interrupt_trace(void)
{
	if (g_trace_log_work_inited) {
		cancel_delayed_work(&g_interrupt_trace_log_work);
		g_trace_log_work_inited = false;
	}
}
