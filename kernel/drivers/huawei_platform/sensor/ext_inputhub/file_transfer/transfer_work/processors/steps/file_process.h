/* SPDX-License-Identifier: GPL-2.0 */
/*
 * file_process.h
 *
 * the head file for file transfer file process steps
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
#ifndef FILE_PROCESS_H
#define FILE_PROCESS_H

#include "transfer_work.h"

/*
 * open the file waitting for write
 * @param worker the worker context
 * @return >=0 for success, otherwise for fail
 */
int preprocess_write_file(struct transfer_worker *worker);

/*
 * open the file waitting for read
 * @param worker the worker context
 * @return >=0 for success, otherwise for fail
 */
int preprocess_read_file(struct transfer_worker *worker);

#endif
