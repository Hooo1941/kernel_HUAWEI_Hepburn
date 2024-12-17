/*
 * hi116x_wltx.h
 *
 * interface for huawei sparklink driver
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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

#ifndef _HI116X_WLTX_H_
#define _HI116X_WLTX_H_
#include <linux/kernel.h>
#include <linux/types.h>

#define HI1162_CMD_REG_ADDR                   0x0011
#define HI1162_ACC_INFO_REG                   0x0049
#define HI1162_ACC_INFO_LEN                   15
#define HI1162_ACC_DEV_NO                     0
#define HI1162_ACC_DEV_STATE                  1
#define HI1162_ACC_DEV_MAC                    2
#define HI1162_ACC_DEV_MODEL_ID               8
#define HI1162_ACC_DEV_SUBMODEL_ID            11
#define HI1162_ACC_DEV_VERSION                12
#define HI1162_ACC_DEV_BUSINESS               13
#define HI1162_ACC_DEV_INFO_SLE               14
#define HI1162_RX_PRODUCT_TYPE_REG            0x0043
#define HI1162_RX_PRODUCT_TYPE_READ_LEN       4
#define HI1162_RX_PRODUCT_TYPE_BUFF_LEN       32
#define HI1162_TX_OPEN_REG                    0x0042
#define HI1162_TX_STATUS_REG                  0x0047
#define HI1162_TX_VIN_REG                     0x0058
#define HI1162_TX_IIN_REG                     0x005A
#define HI1162_TX_IIN_AVG_REG                 0x005C
#define HI1162_TX_CHARGE_INFO_LEN             2
#define HI1162_HALL_STATUS_REG                0x0048
#define HI1162_DEFAULT_HALL_STATE             0

#define ACC_CONNECTED_STR                     "CONNECTED"
#define ACC_DISCONNECTED_STR                  "DISCONNECTED"
#define ACC_UNKNOWN_STR                       "UNKNOWN"
#define ACC_PING_STR                          "PINGING"
#define ACC_PING_SUCC_STR                     "PING_SUCC"
#define ACC_PING_TIMEOUT_STR                  "PING_TIMEOUT"
#define ACC_PING_ERR_STR                      "PING_ERR"
#define ACC_CHARGE_DONE_STR                   "CHARGE_DONE"
#define ACC_CHARGE_ERROR_STR                  "CHARGE_ERR"

#define POWER_BASE_DEC                        10

enum wltx_acc_dev_state {
	WL_ACC_DEV_STATE_BEGIN = 0,
	WL_ACC_DEV_STATE_OFFLINE = WL_ACC_DEV_STATE_BEGIN,
	WL_ACC_DEV_STATE_ONLINE,
	WL_ACC_DEV_STATE_PINGING,
	WL_ACC_DEV_STATE_PING_SUCC,
	WL_ACC_DEV_STATE_PING_TIMEOUT,
	WL_ACC_DEV_STATE_PING_ERROR,
	WL_ACC_DEV_STATE_CHARGE_DONE,
	WL_ACC_DEV_STATE_CHARGE_ERROR,
	WL_ACC_DEV_STATE_END,
};

enum wltx_acc_info_index {
	WL_TX_ACC_INFO_BEGIN = 0,
	WL_TX_ACC_INFO_NO = WL_TX_ACC_INFO_BEGIN,
	WL_TX_ACC_INFO_STATE,
	WL_TX_ACC_INFO_MAC,
	WL_TX_ACC_INFO_MODEL_ID,
	WL_TX_ACC_INFO_SUBMODEL_ID,
	WL_TX_ACC_INFO_VERSION,
	WL_TX_ACC_INFO_BUSINESS,
	WL_TX_ACC_INFO_SLE,
	WL_TX_ACC_INFO_MAX,
};

#endif /* _HI116X_WLTX_H_ */
