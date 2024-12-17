/* SPDX-License-Identifier: GPL-2.0 */
/*
 * handshake.h
 *
 * the head file for file transfer handshake steps
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
#ifndef TRANSFER_HANDSHAKE_H
#define TRANSFER_HANDSHAKE_H

#include "transfer_work.h"

/*
 * send handshake to remote
 * @param worker the worker context
 * @return >=0 for success, otherwise for fail
 */
int send_handshake(struct transfer_worker *worker);

/*
 * wait for handshake ack from remote.
 * @param worker the worker context
 * @return >=0 for success, otherwise for fail
 */
int wait_handshake_ack(struct transfer_worker *worker);

/*
 * send handshake ack to remote
 * @param worker the worker context
 * @return >=0 for success, otherwise for fail
 */
int send_handshake_ack(struct transfer_worker *worker);

#endif
