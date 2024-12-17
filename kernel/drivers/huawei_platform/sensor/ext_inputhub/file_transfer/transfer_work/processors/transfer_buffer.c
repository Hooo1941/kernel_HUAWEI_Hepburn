// SPDX-License-Identifier: GPL-2.0
/*
 * transfer_buffer.c
 *
 * source file for transfer buffer to remote
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
#include "handshake.h"
#include "file_process.h"
#include "data_transfer.h"
#include "inner_event.h"
#include "transfer_processor.h"

static struct transfer_processor processor = { .name = "transfer_buffer" };

static int __init init_processor(void)
{
	init_transfer_processor(&processor);

	processor.add(&processor, preprocess_read_file);
	processor.add(&processor, send_handshake);
	processor.add(&processor, wait_handshake_ack);
	processor.add(&processor, transfer_inner_event_start);
	processor.add(&processor, transfer_file_data);
	processor.add(&processor, transfer_inner_event_finish);

	register_transfer_processor(TRANSFER_BUFFER, &processor);
	return 0;
}

fs_initcall(init_processor);
