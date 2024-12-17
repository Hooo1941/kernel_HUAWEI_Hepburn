// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 *
 * t91407_swi.h
 *
 * t91407_swi driver head file for one wire communication
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

#ifndef __T91407_SWI_H__
#define __T91407_SWI_H__

#include <uapi/linux/sched/types.h>
#include "t91407.h"

#define T91407_LOW_VOLTAGE                     0
#define T91407_HIGH_VOLTAGE                    1

#define T91407_SWI_LEN_OF_DATA_WORD            3
#define T91407_SWI_LEN_OF_EDGE_CAPTURE         13
#define T91407_SWI_OFFSET_OF_READ_DATA         22
#define T91407_SWI_LEN_OF_READ_DATA_LIMIT      140
#define T91407_CODE_BIT_BEGIN                  0
#define T91407_CODE_BIT_LEN                    4
#define T91407_DATA_BIT_BEGIN                  4
#define T91407_DATA_BIT_LEN                    8
#define T91407_DATA_READ_LEN                   100
#define T91407_DATA_STR_LEN                    5
#define T91407_LEN_OF_ONE_DATA                 13
#define T91407_CDOE_OF_WORD                    0
#define T91407_DATA_OF_WORD                    1
#define T91407_INVERT_OF_WORD                  2
#define T91407_INVERT_OF_BYT                   12

/* Delay */
#define T91407_SWI_DELAY_BIT_0_TIMES           1
#define T91407_SWI_DELAY_BIT_1_TIMES           3       /* TAU_BASE*3 */
#define T91407_SWI_DELAY_STOP_TIMES            5
#define T91407_SWI_DELAY_TIMEOUT_TIMES         200
#define T91407_SWI_DELAY_TIMEOUT_BIT_TIMES     5       /* TAU_BASE*5 */
#define T91407_SWI_TAU_BASE                    50
#define T91407_SWI_TXDLY                       4
#define T91407_SWI_WUDLY                       1
#define T91407_SWI_FLOATING_CONTROL            1       /* Minus floating delay */
#define T91407_SWI_CYCLES                      2
/* Select execution, INT8 */
#define T91407_SWI_INT_PUSH_PULL               T91407_SWI_TAU_BASE * (5 + T91407_SWI_TXDLY - 3)
/* Read execution + 16Tswi */
#define T91407_SWI_RRA_PUSH_PULL               T91407_SWI_TAU_BASE * (5 + T91407_SWI_TXDLY - 3)
/* <32Tswi, STOP/5 Tswi to be taken into account */
#define T91407_SWI_RA_PUSH_PULL                T91407_SWI_TAU_BASE * \
							   (5 + T91407_SWI_TXDLY + T91407_SWI_WUDLY - T91407_SWI_FLOATING_CONTROL)
#define T91407_SWI_DELAY_POWERUP_10MS          10000
#define T91407_SWI_DELAY_POWERUP_20MS          20000
#define T91407_SWI_DELAY_POWERUP_6MS           6000
#define T91407_SWI_DELAY_POWERUP_1MS           1100
#define T91407_SWI_DELAY_POWERUP_330US         330
#define T91407_SWI_DELAY_POWERDOWN             300
#define T91407_SWI_DELAY_RESET                 1000
#define T91407_SWI_DELAY_PACKAGE_RETRY_MS      20
#define T91407_SWI_DELAY_COMMAND_RETRY_MS      500

/* Error code */
#define T91407_SUCCESS                         0x00000000
#define T91407_ERR_CRC_VERIGY                  0x00000041
#define T91407_ERR_TIME_OUT                    0x00000042
#define T91407_ERR_READ_FAILED                 0x00000043
#define T91407_ERR_READ_CODE_ERROR             0x00000044
#define T91407_ERR_COPY_TO_USER                0x00000045
#define T91407_ERR_COPY_FROM_USER              0x00000046
#define T91407_ERR_WITE_SIZE                   0x00000047
#define T91407_ERR_WRONG_CMD                   0x00000048
#define T91407_ERR_GPIO_INVALID                0x00000049
#define T91407_ERR_GPIO_REQUEST_FAILED         0x0000004A
#define T91407_ERR_REGISTER_FAILED             0x0000004B
#define T91407_ERR_CREATE_CLASS_FAILED         0x0000004C
#define T91407_ERR_CREATE_DEVICE_FAILED        0x0000004D

#define T91407_SWI_RD_ACK                      0x06u    /* ACK and not End of transmission */
#define T91407_SWI_RD_ACK_EOT                  0x07u    /* ACK and End of transmission */

void t91407_print_data(unsigned char *send, unsigned char send_length, char *write, unsigned char write_length);
void t91407_udelay(uint32_t us_delay);
int t91407_read_data(struct t91407_dev *di, uint8_t *data_ret, uint8_t data_len);
void t91407_write_data(struct t91407_dev *di, uint32_t data);
void t91407_gpio_dir_input(int onewire_gpio);
void t91407_gpio_dir_output(int onewire_gpio, uint8_t value);
void t91407_gpio_set_value(int onewire_gpio, uint8_t value);
int t91407_gpio_get_value(int onewire_gpio);

#endif /* __T91407_SWI_H__ */