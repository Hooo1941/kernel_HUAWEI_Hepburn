/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: gear ctrl  header file
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __SCSI_GEAR_CTRL_H__
#define __SCSI_GEAR_CTRL_H__
void scsi_dev_gear_ctrl_register(struct Scsi_Host *shost);
void hck_scsi_dev_gear_ctrl_register(struct Scsi_Host *shost);
void track_mas_gear_ctrl_init(void *data, struct request_queue *q);
void mas_gear_ctrl_init(struct request_queue *q);
#endif
