/* SPDX-License-Identifier: GPL-2.0 */
/*
 * transfer_processor.h
 *
 * the head file for file transfer processor
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
#ifndef TRANSFER_PROCESSOR_H
#define TRANSFER_PROCESSOR_H

#include "transfer_work.h"

typedef int (*step_process)(struct transfer_worker *worker);

struct transfer_processor {
	char *name;
	struct list_head step_list;
	/* add the process steps */
	void (*add)(struct transfer_processor *processor, step_process process);
	/* trigger to process the processor's steps */
	int (*process)(struct transfer_processor *processor, struct transfer_worker *worker);
};

struct transfer_step {
	step_process process;
	struct list_head list;
};

/*
 * init the input processor
 * @param processor the input processor to init
 * @return >=0 for success, otherwise for fail
 */
int init_transfer_processor(struct transfer_processor *processor);

/*
 * register the proccessor for certain transfer type
 * @param type the transfer type, will use the type as array index
 * @param proccessor the proccessor to register
 * @return >=0 for success, otherwise for fail
 */
void register_transfer_processor(enum transfer_type type, struct transfer_processor *proccessor);

/*
 * get the all registered proccessors
 * @return the map of processors, get NULL when failed
 */
struct transfer_processor **get_transfer_processors(void);

#endif
