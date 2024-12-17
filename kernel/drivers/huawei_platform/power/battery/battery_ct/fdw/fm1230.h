/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * fm1230.h
 *
 * fm1230 driver head file for battery checker
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

#ifndef _FM1230_H_
#define _FM1230_H_

#include <linux/pm_qos.h>
#include <linux/types.h>
#include <chipset_common/hwpower/common_module/power_genl.h>
#include "../batt_aut_checker.h"

#define CYC_MAX                      0xffff
#define LOW_VOLTAGE                  0
#define HIGH_VOLTAGE                 1
#define TEST_ACT_GAP                 1
#define ORG_TYPE                     0
#define FM1230_ACT_LEN               60
#define FM1230_UID_LEN               7
#define FM1230_BATTTYP_LEN           2
#define FM1230_SN_ASC_LEN            16
#define FM1230_TE_PBK_LEN            48
#define FM1230_ODC_LEN               48
#define FM1230_IC_GROUP_SN_LENGTH    8
#define FM1230_CELVED_IND            1
#define FM1230_PKVED_IND             0
#define ACT_MAX_LEN                  64
#define FM1230_HEX_COUNT_PER_U64     16
#define FM1230_HEX_NUMBER_BASE       10
#define FM1230_BIT_COUNT_PER_HEX     4
#define FM1230_BYTE_COUNT_PER_U64    8
#define FM1230_BIT_COUNT_PER_BYTE    8
#define FM1230_SN_ASC_LEN            16
#define CRC16_INIT_VAL               0xffff
#define BIT_P_BYT                    8
#define ODD_MASK                     0x0001
#define FM1230_ACT_CRC_LEN           2
#define FM1230_ACT_CRC_BYT0          58
#define DEFAULT_PROTOCOL             0
#define LOCK_BUF_LEN                 100

enum fm1230_sysfs_type {
	FM1230_SYSFS_BEGIN = 0,
	/* read only */
	FM1230_SYSFS_IC_TYPE,
	FM1230_SYSFS_UID,
	FM1230_SYSFS_BATT_TYPE,
	FM1230_SYSFS_BATT_SN,
	/* write only */
	FM1230_SYSFS_CERT_STATUS,
	FM1230_SYSFS_ACT_SIG,
	FM1230_SYSFS_BATT_CYC,
	FM1230_SYSFS_SPAR_CK,
	FM1230_SYSFS_CYCLK,
	FM1230_SYSFS_GROUP_SN,
	FM1230_SYSFS_END,
};

struct fm1230_memory {
	uint8_t uid[FM1230_UID_LEN];
	uint8_t batt_type[FM1230_BATTTYP_LEN];
	uint8_t sn[FM1230_SN_ASC_LEN];
	uint8_t res_ct[FM1230_ODC_LEN];
	uint8_t act_sign[FM1230_ACT_LEN];
	enum batt_source source;
	bool ecce_pass;
	enum batt_ic_type ic_type;
	enum batt_cert_state cet_rdy;
	uint8_t group_sn[FM1230_SN_ASC_LEN + 1];
	bool group_sn_ready;
	bool uid_ready;
	bool sn_ready;
};

struct fm1230_swi_delay {
	uint32_t ow_reset_start_delay;
	uint32_t ow_reset_sample_delay;
	uint32_t ow_reset_end_delay;
	uint32_t ow_write_start_delay;
	uint32_t ow_write_low_delay;
	uint32_t ow_write_high_delay;
	uint32_t ow_write_end_delay;
	uint32_t ow_read_start_delay;
	uint32_t ow_read_sample_delay;
	uint32_t ow_read_end_delay;
};

struct fm1230_dev {
	struct device *dev;
	struct fm1230_memory mem;
	uint16_t product_id;
	uint16_t cyc_full;
	uint32_t cyc_num;
	int onewire_gpio;
	int mos_gpio;
	spinlock_t onewire_lock;
	bool lock_status;
	struct batt_ct_ops_list reg_node;
	uint32_t ic_index;
	struct pm_qos_request pm_qos;
	unsigned long irq_flags;
	struct fm1230_swi_delay fm1230_swi;
};

#endif /* _FM1230_H_ */
