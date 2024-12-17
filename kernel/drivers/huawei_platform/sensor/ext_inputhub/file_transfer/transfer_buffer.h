/* SPDX-License-Identifier: GPL-2.0 */
/*
 * transfer_buffer.h
 *
 * head file for buffer transfer
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
#ifndef TRANSFER_BUFFER_H
#define TRANSFER_BUFFER_H

/*
 * queue a transfer buffer handshake to process
 * @param rfd the remote fd get from handshake
 * @param data data buffer
 * @param data_len data buffer length
 * @return >= 0 for success, otherwise for fail
 */
int queue_buffer_handshake(int rfd, void *data, u32 data_len);

#endif
