// SPDX-License-Identifier: GPL-2.0
/*
 * remote_transfer.c
 *
 * source file for remote transfer file processor
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

static struct transfer_processor processor = { .name = "remote_transfer" };

static int __init init_processor(void)
{
	init_transfer_processor(&processor);

	processor.add(&processor, transfer_inner_event_start);
	processor.add(&processor, preprocess_write_file);
	processor.add(&processor, send_handshake_ack);
	processor.add(&processor, receive_transfer_data);
	processor.add(&processor, transfer_inner_event_finish);

	register_transfer_processor(REMOTE_TRANSFER, &processor);
	return 0;
}

fs_initcall(init_processor);