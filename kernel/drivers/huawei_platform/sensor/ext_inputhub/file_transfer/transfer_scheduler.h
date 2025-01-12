/* SPDX-License-Identifier: GPL-2.0 */
/*
 * transfer_topic.h
 *
 * topic manager head file for file transfer
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
#ifndef TRANSFER_SCHEDULER_H
#define TRANSFER_SCHEDULER_H

#include <linux/types.h>
#include "transfer_record.h"

/*
 * queue the remote file transfer to scheduler
 * @param rfd the fd generated by remote
 * @param type the request transfer type
 * @param file_req file info for transfer request
 * @return >= 0 for success, otherwise for fail
 */
int queue_remote_transfer(int rfd, enum transfer_type type, struct file_transfer *file_req);

/*
 * queue the local file transfer to scheduler
 * @param tid the id for transfer direction
 * @param trans_type transfer type
 * @param file_req file info for transfer request
 * @return >= 0 for success, otherwise for fail
 */
int queue_file_work(s8 tid, int trans_type, struct file_transfer *file_req);

#endif
