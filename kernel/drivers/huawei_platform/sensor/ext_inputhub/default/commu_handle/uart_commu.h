/*
 * uart_commu.h
 *
 * head file for communication with external sensorhub
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

#ifndef UART_COMMU_H
#define UART_COMMU_H
#include "ext_sensorhub_commu.h"

/* set uart mode */
void set_uart_commu_mode(int mode);

/* get uart mode */
int get_uart_commu_mode(void);

int ext_sensorhub_commu_uart_close(void);

int ext_sensorhub_commu_uart_open(void);

#endif
