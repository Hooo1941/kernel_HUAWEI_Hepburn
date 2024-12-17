/* SPDX-License-Identifier: GPL-2.0 */
/*
 * inner_event.h
 *
 * the head file for file transfer inner event steps
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
#ifndef TRANSFER_INNER_EVENT_H
#define TRANSFER_INNER_EVENT_H

#include "transfer_work.h"

/*
 * transfer start event
 * @param worker the worker context
 * @return >=0 for success, otherwise for fail
 */
int transfer_inner_event_start(struct transfer_worker *worker);

/*
 * transfer finish event
 * @param worker the worker context
 * @return >=0 for success, otherwise for fail
 */
int transfer_inner_event_finish(struct transfer_worker *worker);

/*
 * transfer the given status event
 * @param worker the worker context
 * @param status the status to transfer
 * @return >=0 for success, otherwise for fail
 */
int transfer_inner_event_status(struct transfer_worker *worker, enum transfer_status status);

#endif
