// SPDX-License-Identifier: GPL-2.0
/*
 * transfer_processor.c
 *
 * source file for transfer processor
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include "transfer_processor.h"

#include <linux/slab.h>
#include <linux/types.h>
#include <huawei_platform/log/hw_log.h>
#include "inner_event.h"

#define HWLOG_TAG file_trans
HWLOG_REGIST();

static struct transfer_processor *processors[MAX_TRANSFER_TYPE];

/* append a step to the processor */
static void add_transfer_process(struct transfer_processor *processor, step_process process)
{
	struct transfer_step *step = NULL;

	if (!processor || !process)
		return;
	step = kmalloc(sizeof(*step), GFP_KERNEL);
	if (!step)
		return;

	step->process = process;
	INIT_LIST_HEAD(&step->list);
	list_add_tail(&step->list, &processor->step_list);
}

/* process the steps added in proccessor */
static int do_transfer_process(struct transfer_processor *processor, struct transfer_worker *worker)
{
	int ret = -EFAULT;
	struct transfer_step *buf_node = NULL;

	if (!processor)
		return -EINVAL;

	hwlog_info("%s get in", processor->name);
	/* process each step by order */
	list_for_each_entry(buf_node, &processor->step_list, list) {
		if (!buf_node)
			continue;

		hwlog_info("%s to process:%ps", __func__, buf_node->process);
		ret = buf_node->process(worker);
		if (ret < 0) {
			hwlog_err("%s failed to process:%ps", __func__, buf_node->process);
			break;
		}
	}
	/* when failure break, send a transfer response event */
	if (ret < 0)
		transfer_inner_event_status(worker, TRANS_STATUS_UNKNOWN_ERR);

	return ret;
}

void register_transfer_processor(enum transfer_type type, struct transfer_processor *proccessor)
{
	if (type >= MAX_TRANSFER_TYPE || !proccessor || !proccessor->process)
		return;

	processors[type] = proccessor;
}

int init_transfer_processor(struct transfer_processor *processor)
{
	if (!processor)
		return -EINVAL;

	processor->add = add_transfer_process;
	processor->process = do_transfer_process;
	INIT_LIST_HEAD(&processor->step_list);
	return 0;
}

struct transfer_processor **get_transfer_processors(void)
{
	return processors;
}
