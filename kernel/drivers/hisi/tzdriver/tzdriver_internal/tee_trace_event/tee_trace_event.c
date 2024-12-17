/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: functions for tee trace event attach point
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

#include "tee_trace_event.h"

#include <linux/smp.h>
#include <linux/preempt.h>
#include <asm/arch_timer.h>
#include <linux/version.h>
#include <securec.h>

#include <teek_ns_client.h>
#include <tc_ns_log.h>
#include <mailbox_mempool.h>
#include <teek_client_constants.h>
#include <smc_smp.h>
#include <ko_adapt.h>
#include "livepatch_cmd.h"
#include "tee_trace_interrupt.h"

#define TEE_TASK_NAME_LEN 16 /* same as tcb_prop->tcb_name */

#define trace_event_name(event_id, enable)  \
	[event_id] = { #event_id, enable }

#define compile_time_assert(cond, msg) typedef char g_assert_##msg[(cond) ? 1 : -1]

struct tee_view_state_t {
	const char *name;
	bool enable;
};

/* Make sure it has the same order as 'enum tee_event_id' */
static struct tee_view_state_t view_state[TEE_EVENT_MAX] = {
	trace_event_name(INVOKE_CMD_START, true),
	trace_event_name(INVOKE_CMD_END, true),
	trace_event_name(SMC_SEND, true),
	trace_event_name(SMC_DONE, true),
	trace_event_name(SMC_IN, true),
	trace_event_name(SMC_OUT, true),
	trace_event_name(SMC_SLEEP, true),
	trace_event_name(SMC_PREEMPT, true),
	trace_event_name(GTASK_GET_CMD, false),
	trace_event_name(GTASK_PUT_CMD, false),
	trace_event_name(GTASK_REQ_TA, false),
	trace_event_name(GTASK_RESP_TA, false),
	trace_event_name(SPI_WAKEUP, true),
	trace_event_name(SCHED_IN, true),
	trace_event_name(SCHED_OUT, true),
	trace_event_name(INTERRUPT_HANDLE_SPI_START, true),
	trace_event_name(INTERRUPT_HANDLE_SPI_REE_RESPONSE, true),
	trace_event_name(INTERRUPT_HANDLE_SPI_REE_MISS, true),
	trace_event_name(INTERRUPT_HANDLE_SPI_REE_SCHEDULED, true),
	trace_event_name(INTERRUPT_HANDLE_SPI_END, true),
	trace_event_name(INTERRUPT_HANDLE_START, true),
	trace_event_name(INTERRUPT_HANDLE_END, true),
};

static const char* trace_task[] = {
	/* add ta name like this "echo_task", */
};
compile_time_assert(ARRAY_SIZE(trace_task) <= TEE_TRACE_TASK_MAX,
	trace_task_too_large);

struct tee_trace_event_t {
	enum tee_event_id id;
	uint32_t ca_pid;
	uint64_t time;
	uint64_t add_info;
};

struct tee_trace_stream_t {
	uint32_t total;
	uint32_t cur;
	uint32_t overflowed;
	struct tee_trace_event_t events[TEE_TRACE_EVENT_NUM];
};

struct tee_trace_mem_t {
	bool start;
	uint32_t freq;
	uint32_t loop_enable;
	bool enable[TEE_EVENT_MAX];
	uint32_t trace_task;
	char trace_task_name[TEE_TRACE_TASK_MAX][TEE_TASK_NAME_LEN];
	struct tee_trace_stream_t *streams[NR_CPUS];
};

#define TRACE_STREAM_SIZE ALIGN(sizeof(struct tee_trace_stream_t), PAGE_SIZE)
#define TRACE_MEM_SIZE ALIGN(sizeof(struct tee_trace_mem_t), PAGE_SIZE)

/* pages associate with g_event_mem */
static struct page *g_event_pages;
/* pages associate with g_event_mem->streams[NR_CPUS] */
static struct page *g_stream_pages[NR_CPUS];
static struct tee_trace_mem_t *g_event_mem;
static uint64_t g_trace_start_time;
void tee_trace_add_event(enum tee_event_id id, uint64_t add_info)
{
	int32_t cpu_id = raw_smp_processor_id();
	struct tee_trace_stream_t *stream = NULL;
	struct tee_trace_event_t *event = NULL;

	if (id < INVOKE_CMD_START || id >= TEE_EVENT_MAX)
		return;

	if (g_event_mem == NULL || !g_event_mem->start || !g_event_mem->enable[id])
		return;

	preempt_disable();
	stream = g_event_mem->streams[cpu_id];
	event = &stream->events[stream->cur];

	if (g_event_mem->loop_enable == TRACE_LOG_LOOP_DISABLED &&
		unlikely((stream->cur + 1) >= stream->total)) {
		tloge("events buffer too small\n");
		preempt_enable();
		return;
	}

	event->id = id;
	event->ca_pid = current->pid;
#if (KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE)
	event->time = arch_timer_read_counter();
#else
	event->time = arch_counter_get_cntvct();
#endif
	event->add_info = (add_info == 0) ? current->pid : add_info;
	stream->cur++;
	if (unlikely(stream->cur >= stream->total)) {
		stream->cur = 0;
		stream->overflowed = 1;
	}
	preempt_enable();
}

struct trace_mem_cmd_t {
	uint64_t trace_mem;
	uint64_t trace_mem_size;
	uint64_t trace_streams[NR_CPUS];
};

static void *init_cmd_of_trace_mem(void)
{
	uint32_t i;
	struct trace_mem_cmd_t *p =
		mailbox_alloc(sizeof(struct trace_mem_cmd_t), MB_FLAG_ZERO);

	if (p == NULL)
		return NULL;

	p->trace_mem = virt_to_phys(g_event_mem);
	p->trace_mem_size = TRACE_MEM_SIZE;
	for (i = 0; i < NR_CPUS; i++)
		p->trace_streams[i] = virt_to_phys(g_event_mem->streams[i]);

	return p;
}

static int tee_trace_event_send_cmd(uint32_t cmd)
{
	int ret = 0;
	struct tc_ns_smc_cmd smc_cmd = { { 0 }, 0 };
	struct mb_cmd_pack *mb_pack = NULL;
	void *cmd_buffer = NULL;

	mb_pack = mailbox_alloc_cmd_pack();
	if (mb_pack == NULL)
		return -ENOMEM;

	switch (cmd) {
	case GLOBAL_CMD_ID_TRACE_ENABLE:
		mb_pack->operation.paramtypes = TEE_PARAM_TYPE_MEMREF_INPUT;
		cmd_buffer = init_cmd_of_trace_mem();
		if (cmd_buffer == NULL) {
			mailbox_free(mb_pack);
			return -ENOMEM;
		}
		mb_pack->operation.params[0].memref.buffer = mailbox_virt_to_phys((uintptr_t)cmd_buffer);
		mb_pack->operation.buffer_h_addr[0] =
			mailbox_virt_to_phys((uintptr_t)cmd_buffer) >> ADDR_TRANS_NUM;
		mb_pack->operation.params[0].memref.size =
			ALIGN(sizeof(struct trace_mem_cmd_t), PAGE_SIZE);
		break;
	default:
		mailbox_free(mb_pack);
		return -EINVAL;
	}
	smc_cmd.cmd_type = CMD_TYPE_GLOBAL;
	smc_cmd.cmd_id = cmd;
	smc_cmd.operation_phys = mailbox_virt_to_phys((uintptr_t)&mb_pack->operation);
	smc_cmd.operation_h_phys =
		(uint64_t)mailbox_virt_to_phys((uintptr_t)&mb_pack->operation) >> ADDR_TRANS_NUM;

	livepatch_down_read_sem();
	if (tc_ns_smc(&smc_cmd)) {
		ret = -EIO;
		tloge("trace cmd 0x%x failed\n", cmd);
	}
	livepatch_up_read_sem();

	if (cmd_buffer != NULL)
		mailbox_free(cmd_buffer);
	mailbox_free(mb_pack);

	return ret;
}

void free_event_mem(void)
{
	uint32_t i;

	(void)memset_s(g_event_mem, TRACE_MEM_SIZE, 0, TRACE_MEM_SIZE);

	for (i = 0; i < NR_CPUS; i++) {
		if (g_stream_pages[i] != NULL) {
			__free_pages(g_stream_pages[i], get_order(TRACE_STREAM_SIZE));
			g_stream_pages[i] = NULL;
		}
	}

	if (g_event_pages != NULL) {
		__free_pages(g_event_pages, get_order(TRACE_MEM_SIZE));
		g_event_mem = NULL;
	}
}

static int init_event_mem(void)
{
	uint32_t i;
	int ret;

	g_event_pages = koadpt_alloc_pages(GFP_KERNEL, get_order(TRACE_MEM_SIZE));
	if (g_event_pages == NULL) {
		tloge("alloc event mem (size 0x%lx) failed\n", TRACE_MEM_SIZE);
		return -ENOMEM;
	}
	g_event_mem = page_address(g_event_pages);
	(void)memset_s(g_event_mem, TRACE_MEM_SIZE, 0, TRACE_MEM_SIZE);

	for (i = 0; i < NR_CPUS; i++) {
		g_stream_pages[i] =
			koadpt_alloc_pages(GFP_KERNEL, get_order(TRACE_STREAM_SIZE));
		if (!g_stream_pages[i]) {
			tloge("alloc stream mem (size 0x%lx) failed\n", TRACE_STREAM_SIZE);
			ret = -ENOMEM;
			goto clean;
		}
		g_event_mem->streams[i] = page_address(g_stream_pages[i]);
	}

	for (i = 0; i < TEE_EVENT_MAX; i++)
		g_event_mem->enable[i] = view_state[i].enable;

	g_event_mem->trace_task = ARRAY_SIZE(trace_task);
	for (i = 0; i < ARRAY_SIZE(trace_task); i++)
		if (strcpy_s(g_event_mem->trace_task_name[i], TEE_TASK_NAME_LEN,
			trace_task[i]) != EOK) {
			tloge("task name %s too long\n", trace_task[i]);
			ret = -EINVAL;
			goto clean;
		}

	g_event_mem->freq = arch_timer_get_cntfrq();
	for (i = 0; i < NR_CPUS; i++)
		g_event_mem->streams[i]->total = TEE_TRACE_EVENT_NUM;

	return 0;

clean:
	free_event_mem();
	return ret;
}

int tee_trace_event_enable(void)
{
	int ret;

	if (g_event_mem != NULL)
		return 0;

	ret = init_event_mem();
	if (ret != 0)
		return ret;

	ret = tee_trace_event_send_cmd(GLOBAL_CMD_ID_TRACE_ENABLE);
	if (ret != 0) {
		free_event_mem();
		tloge("register tee trace mem failed\n");
		return ret;
	}

	return 0;
}

static int tee_trace_event_start_common(uint32_t loop_enable)
{
	uint32_t cpu;

	if (g_event_mem == NULL)
		return 0;

	interrupt_trace_stop();

	g_event_mem->loop_enable = loop_enable;

	for (cpu = 0; cpu < NR_CPUS; cpu++) {
		g_event_mem->streams[cpu]->cur = 0;
		g_event_mem->streams[cpu]->overflowed = 0;
		(void)memset_s(g_event_mem->streams[cpu]->events,
			sizeof(struct tee_trace_event_t) * TEE_TRACE_EVENT_NUM, 0,
			sizeof(struct tee_trace_event_t) * TEE_TRACE_EVENT_NUM);
	}
	g_event_mem->start = true;

#if (KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE)
	g_trace_start_time = arch_timer_read_counter();
#else
	g_trace_start_time = arch_counter_get_cntvct();
#endif

	smp_mb();

	interrupt_trace_start();

	return 0;
}
int tee_trace_event_start_loop_record(void)
{
	return tee_trace_event_start_common(TRACE_LOG_LOOP_ENABLED);
}

int tee_trace_event_start(void)
{
	return tee_trace_event_start_common(TRACE_LOG_LOOP_DISABLED);
}

int tee_trace_event_stop(void)
{
	if (g_event_mem == NULL || !g_event_mem->start)
		return 0;
	g_event_mem->start = false;
	smp_mb();
	interrupt_trace_stop();
	return 0;
}

void get_tee_trace_start(struct tee_trace_view_t *view)
{
	uint32_t i;

	if (g_event_mem == NULL || view == NULL)
		return;

	for (i = 0; i < NR_CPUS; i++) {
		uint64_t time = g_event_mem->streams[i]->events[0].time;
		if (g_event_mem->loop_enable != TRACE_LOG_LOOP_DISABLED)
			time = g_trace_start_time;
		if (time != 0 && (view->start == 0 || view->start > time))
			view->start = time;
		if (g_event_mem->loop_enable != TRACE_LOG_LOOP_DISABLED && 
			g_event_mem->streams[i]->overflowed == 1) {
			view->total += g_event_mem->streams[i]->total;
			view->end[i] = g_event_mem->streams[i]->total;
			view->buffer_is_full = 1;
		} else {
			view->total += g_event_mem->streams[i]->cur;
			view->end[i] = g_event_mem->streams[i]->cur;
		}

		view->at[i] = 0;
	}

	view->freq = g_event_mem->freq;
}

static int32_t get_read_idx(struct tee_trace_view_t *view, int32_t index,
	bool cyclically)
{
	if (cyclically) {
		if (view->at[index] == g_event_mem->streams[index]->cur)
			return -1;
		return view->at[index];
	}
	if (view->at[index] >= view->end[index])
		return -1;
	return view->at[index];
}

static void get_next_log_cpu_idx(struct tee_trace_view_t *view,
	int32_t *index, bool cyclically)
{
	uint32_t i;
	uint64_t first = 0;
	*index = -1;
	for (i = 0; i < NR_CPUS; i++) {
		int32_t read_idx = get_read_idx(view, i, cyclically);
		if (read_idx != -1) {
			if (*index == -1 ||
				first > g_event_mem->streams[i]->events[read_idx].time) {
				first = g_event_mem->streams[i]->events[read_idx].time;
				*index = i;
			}
		}
	}
}

int get_tee_trace_next(struct tee_trace_view_t *view, struct trace_log_info *log_info,
	bool cyclically)
{
	int32_t index = -1;
	struct tee_trace_event_t *event = NULL;

	if (log_info == NULL)
		return -1;

	if (g_event_mem == NULL || !g_event_mem->start || view == NULL)
		return -1;

	if (g_event_mem->freq == 0)
		return -1;

	get_next_log_cpu_idx(view, &index, cyclically);

	if (index == -1)
		return -1;

	event = &g_event_mem->streams[index]->events[view->at[index]];
	log_info->event_id = event->id;
	log_info->event_name = view_state[event->id].name;
	log_info->cpu = (uint32_t)index;
	log_info->ca_pid = event->ca_pid;
	log_info->time = (event->time - view->start) * USEC_PER_SEC / g_event_mem->freq;
	log_info->add_info = event->add_info;
	view->at[index]++;

	if (g_event_mem->loop_enable != TRACE_LOG_LOOP_DISABLED && cyclically &&
		view->at[index] == g_event_mem->streams[index]->total)
		view->at[index] = 0;

	return 0;
}

const char *get_tee_trace_task_name(uint32_t task_idx)
{
	if (task_idx >= ARRAY_SIZE(trace_task))
		return NULL;
	return trace_task[task_idx];
}
