/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ext_sensorhub_inner_cmd.h
 *
 * head file for communication with inner command
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
#ifndef EXT_SENSORHUB_INNER_CMD_H
#define EXT_SENSORHUB_INNER_CMD_H
#include "ext_sensorhub_api.h"

typedef int (*inner_cmd_callback)(unsigned char service_id,
	unsigned char command_id, unsigned char *data, int data_len);

/*
 * Register inner command
 * one channel can only hold a callback, only hold the last regitered info.
 * @param service_ids register for the service_id list
 * @param command_ids register for the command_id list
 * @param cmd_cnt the cmds count for service_id&command_id
 * @param callback when get the aim sid and cid, the func will be called
 * @return ret >= 0 on success, < 0 on failure.
 */
int register_inner_cmd(enum ext_channel_id channel_id,
		       unsigned char *service_ids, unsigned char *command_ids,
		       unsigned short cmd_cnt, inner_cmd_callback callback);

/*
 * Unregister inner command for the channel id
 * @param channel_id the commu channel id for unregister
 * @return ret >= 0 on success, < 0 on failure.
 */
int unregister_inner_cmd(enum ext_channel_id channel_id);

/*
 * Send a inner cmd to response list
 * @param service_id the commu service_id
 * @param command_id the commu command_id
 * @param data the cmd data to send
 * @param data_len the cmd data length
 * @return ret >= 0 on success, < 0 on failure.
 */
int send_inner_command(unsigned char service_id, unsigned char command_id,
		       unsigned char *data, int data_len);

/*
 * Hook for process inner cmd. do not use for client. This
 * @param service_id the commu service_id
 * @param command_id the commu command_id
 * @param data the cmd data
 * @param data_len the cmd data length
 * @param ret the hook process result
 * @return ret true for inner cmd, false for normal commu cmd.
 */
bool process_inner_cmd_hook(unsigned char service_id, unsigned char command_id,
			    unsigned char *data, int data_len, int *ret);

#endif /* EXT_SENSORHUB_INNER_CMD_H */
