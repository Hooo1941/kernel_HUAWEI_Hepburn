/* SPDX-License-Identifier: GPL-2.0 */
/*
 * data_transfer.h
 *
 * the head file for file transfer data transfer steps
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
#ifndef DATA_TRANSFER_H
#define DATA_TRANSFER_H

#include "transfer_work.h"

/*
 * transfer file data to remote and wait data ack
 * @param worker the worker context
 * @return >=0 for success, otherwise for fail
 */
int transfer_file_data(struct transfer_worker *worker);

/*
 * receive the data from remote and send data ack if need
 * @param worker the worker context
 * @return >=0 for success, otherwise for fail
 */
int receive_transfer_data(struct transfer_worker *worker);

#endif
