/* SPDX-License-Identifier: GPL-2.0 */
/*
 * cbm8580_interface.h
 *
 * cbm8580 function interface
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

#ifndef _CBM8580_INTERFACE_H_
#define _CBM8580_INTERFACE_H_

#include "../bq40z50/bq40z50.h"

#define CBM8580_RET_FAILED					 (-1)
#define CBM8580_RET_SUCCESS					 0
#define CBM8580_FW_RETRY_COUNT               3
#define CBM8580_FW_SINGLE_BIT_LEN			 8
#define CBM8580_FW_SINGLE_BIT_MASK			 0xFF

/* reg address and sub cmd */
#define CBM8580_FW_REG						 0x44
#define CBM8580_OP_STAT_H_REG				 0x88
#define CBM8580_FW_SUB_CMD_READ_RTC			 0x9000
#define CBM8580_FW_SUB_CMD_WRITE_RTC		 0x9001
#define CBM8580_FW_SUB_CMD_SHUTDOWN_FLAG	 0x0012

/* sub cmd index */
#define CBM8580_FW_SUB_CMD_INDEX			 1
#define CBM8580_FW_RTC_DATA_INDEX		 	 3
#define CBM8580_FW_SHUTDOWN_LEN			 	 2
#define CBM8580_FW_READ_RTC_1_LEN			 3
#define CBM8580_FW_READ_RTC_2_LEN			 8
#define CBM8580_FW_WRITE_RTC_LEN			 8
#define CBM8580_UPDATE_SHUTDOWN_FLAG_LEN     4
#define CBM8580_OP_STAT_H_SHUTDOWN_FLAG_BIT  13

/* rtc */
#define CBM8580_FW_READ_RTC_FORMAT_HEX		 0x00
#define CBM8580_FW_READ_RTC_FORMAT_BCD		 0x01
#define CBM8580_CUR_YEAR					 2000
#define CBM8580_BASE_YEAR					 1900

/* shutdown flag */
#define CBM8580_SHUTDOWN_FLAG_DIEABLE 0
#define CBM8580_SHUTDOWN_FLAG_ENABLE 1

/* power saving */
void cbm8580_power_saving_restore(struct bq40z50_device_info *di);
void cbm8580_power_saving_shutdown(struct bq40z50_device_info *di);
void cbm8580_power_saving_free_resource(struct bq40z50_device_info *di);

#endif /* _CBM8580_INTERFACE_H_ */