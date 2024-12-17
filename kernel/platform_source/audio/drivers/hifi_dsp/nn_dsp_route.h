/*
 * nn_dsp_route.h
 *
 * nn dsp route misc driver.
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

#ifndef _NN_DSP_ROUTE_H_
#define _NN_DSP_ROUTE_H_

#include <linux/types.h>
#include "dsp_om.h"

void rcv_nn_dsp_route_msg(enum socdsp_om_work_id work_id, const uint8_t *data, uint32_t len);
void init_nn_dsp_route(void);
void deinit_nn_dsp_route(void);

#endif
