// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 *
 * t91407.h
 *
 * t91407 driver head file for battery checker
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

#ifndef _T91407_H_
#define _T91407_H_

#include <linux/pm_qos.h>
#include <linux/types.h>
#include <chipset_common/hwpower/common_module/power_genl.h>
#include "../batt_aut_checker.h"

#define T91407_UID_LEN               8
#define T91407_UID_HEX_LEN           16
#define T91407_BATTTYP_LEN           2
#define T91407_SN_ASC_LEN            16
#define T91407_ACT_LEN               60
#define T91407_ACT_HEX_LEN           120
#define T91407_BATTTYPE_LEN          8
#define T91407_ODC_LEN               136
#define T91407_LOCK_BUF_LEN          100
#define T91407_IC_GROUP_SN_LENGTH    16
#define T91407_HEX_COUNT_PER_U64     16
#define T91407_HEX_NUMBER_BASE       10
#define T91407_BIT_COUNT_PER_HEX     4
#define T91407_BYTE_COUNT_PER_U64    8
#define T91407_BIT_COUNT_PER_BYTE    8
#define T91407_ADDR_OFFSET_BIT4      16

#define T91407_CRC16_INIT_VAL        0xffff
#define T91407_BIT_P_BYT             8
#define T91407_ODD_MASK              0x0001
#define T91407_ACT_CRC_LEN           2
#define T91407_ACT_CRC_BYT0          58

#define T91407_ASCLL_OF_NUMBER       57
#define T91407_OFFSET_LETTER         7

#define T91407_CELVED_IND            1
#define T91407_PKVED_IND             0

struct t91407_memory {
	uint8_t uid[T91407_UID_LEN];
	uint8_t batt_type[T91407_BATTTYP_LEN];
	uint8_t sn[T91407_SN_ASC_LEN];
	uint8_t res_ct[T91407_ODC_LEN];
	uint8_t act_sign[T91407_ACT_LEN];
	enum batt_source source;
	bool ecce_pass;
	enum batt_ic_type ic_type;
	enum batt_cert_state cet_rdy;
	uint8_t group_sn[T91407_SN_ASC_LEN + 1];
	bool group_sn_ready;
	bool uid_ready;
	bool sn_ready;
	bool batt_type_ready;
	bool act_sign_ready;
};

struct t91407_swi_delay {
	uint32_t ow_tau_delay;
	uint32_t ow_bit0_delay;
	uint32_t ow_bit1_delay;
	uint32_t ow_stop_delay;
	uint32_t ow_timeout_delay;
	uint32_t ow_timeout_bit_delay;
	uint32_t ow_execution_delay;
};

struct t91407_dev {
	struct device *dev;
	struct t91407_memory mem;
	uint16_t product_id;
	uint32_t cyc_num;
	int onewire_gpio;
	int mos_gpio;
	spinlock_t onewire_lock;
	bool lock_status;
	struct batt_ct_ops_list reg_node;
	uint32_t ic_index;
	struct pm_qos_request pm_qos;
	unsigned long irq_flags;
	struct t91407_swi_delay t91407_swi;
	uint8_t device_inited;
};

#endif /* _T91407_H_ */
