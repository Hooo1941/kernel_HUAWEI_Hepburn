/*
 * light_spot_res.h
 *
 * Manager of light spot flash resource header.
 *
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd.
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
#ifndef LIGHT_SPOT_RES_H
#define LIGHT_SPOT_RES_H

#define FLASH_DATA_SIZE 3600 /* 30 * 30 * 4 bytes */
#define FLASH_FRAME_SIZE 30 /* 30x30 pixel */
#define FLASH_FRAME_NUM 22 /* 22 frames */

void get_flash_circle_data(int index, unsigned char *data_buf);

#endif // LIGHT_SPOT_RES_H
