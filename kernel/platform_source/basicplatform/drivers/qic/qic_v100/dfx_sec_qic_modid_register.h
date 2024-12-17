/*
 * QIC v100 modid register functions head file.
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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
#ifndef __DFX_SEC_QIC_MODID_REGISTER_H
#define __DFX_SEC_QIC_MODID_REGISTER_H
#include "../dfx_sec_qic.h"
#include "../common/sec_qic_share_info.h"
#include "../common/qic_interface.h"

#define QIC_REGISTER_LIST_MAX_LENGTH 512
#define QIC_MODID_NOT_EXIST 0x0
#define QIC_MODID_EXIST 0x1
#define QIC_MODID_NEGATIVE 0xFFFFFFFF
#define QIC_MODID_NUM_MAX 0x6
#define QIC_SOFTWARE_IRQ_NUMBER 442
#define QIC_HAVE_SYS_BUS_INTR 0x400
#define QIC_HAVE_TB0_TIMEOUT_INTR 0x30000
/* xxx_qic_cs.dtsi qic_bus_info.bus_key */
#define QIC_SYS_BUS_KEY 0xa
#define QIC_GICD_QIC_BIT_MASK 0x4000000

/* NPU NOC exception code 0xc0000700-0xc00007ff */
#define RDR_EXC_TYPE_NOC_NPU2   0xc0000702
#define RDR_EXC_TYPE_NOC_NPU5   0xc0000705
#define RDR_EXC_TYPE_NOC_NPU6   0xc0000706

struct qic_coreid_modid_trans_s {
	struct list_head s_list;
	unsigned int qic_coreid;
	unsigned int modid;
};

unsigned int dfx_qic_modid_find(unsigned int qic_coreid, unsigned int mid);
void dfx_qic_set_errlog_info(struct dfx_errlog_info *errlog_info);
#endif