// SPDX-License-Identifier: GPL-2.0
/*
 * inner_event.c
 *
 * the source file for file transfer inner event steps
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
#include "inner_event.h"

#include "transfer_inner_commu.h"

int transfer_inner_event_status(struct transfer_worker *worker, enum transfer_status status)
{
	struct transfer_resp resp = {0};

	/* sub_type default 1 */
	resp.sub_type = 0x01;
	resp.rfd = worker->record->rfd;
	resp.topic_id = worker->record->trans_info->topic_id;
	resp.transfer_type = worker->transfer_type;
	resp.event.offset = worker->record->trans_info->file.src_offset;
	resp.event.data_len = worker->record->trans_info->file.data_len;
	resp.event.status = status;

	if (worker->transfer_type == TRANSFER_BUFFER) {
		/* enum type for transfer buffer */
		resp.transfer_type = 0;
		resp.sub_type = TRANSFER_BUFFER_TYPE;
	}
	if (worker->transfer_type == REMOTE_TRANSFER_BUFFER) {
		/* enum type for remote transfer buffer */
		resp.transfer_type = 1;
		resp.sub_type = TRANSFER_BUFFER_TYPE;
	}

	return send_transfer_inner_resp(&resp);
}

int transfer_inner_event_start(struct transfer_worker *worker)
{
	return transfer_inner_event_status(worker, TRANS_STATUS_START);
}

int transfer_inner_event_finish(struct transfer_worker *worker)
{
	return transfer_inner_event_status(worker, TRANS_STATUS_FINISH);
}
