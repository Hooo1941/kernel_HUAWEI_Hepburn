/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef __DP_CTRL_COMMON_H__
#define __DP_CTRL_COMMON_H__

#define DPTX_INVALID_PORT_ID    (-1)

enum dptx_port_id_e {
	DPTX_PORT_ID_0          = 0,
	DPTX_PORT_ID_1          = 1,
	DPTX_PORT_ID_2          = 2,
	DPTX_PORT_ID_3          = 3,
	DPTX_PORT_ID_4          = 4,
	DPTX_PORT_ID_5          = 5,
	DPTX_PORT_ID_MAX        = 6,
};

/* only v740 need select dpc number now */
enum dptx_dpc_num_e {
	DPTX_DPC_NUM_0          = 0,
	DPTX_DPC_NUM_1          = 1,
	DPTX_DPC_NUM_2          = 2,
	DPTX_DPC_NUM_MAX        = 3,
};

#endif