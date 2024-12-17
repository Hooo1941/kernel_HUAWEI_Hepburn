/*
 * QIC V100 err probe functions head file.
 *
 * Copyright (c) 2021-2023 Huawei Technologies Co., Ltd.
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
#ifndef __DFX_SEC_QIC_ERR_PROBE_H
#define __DFX_SEC_QIC_ERR_PROBE_H
#include "../dfx_sec_qic.h"

#define DFX_SEC_QIC_INT_NORMAL_TYPE          1
#define DFX_SEC_QIC_INT_SAFETY_TYPE          2
#define DFX_SEC_QIC_INT_STATUS_MASK          3

#define DFX_SEC_QIC_RW_NORMAL_OFFSET         15
#define DFX_SEC_QIC_RW_SAFETY_OFFSET         7
#define DFX_SEC_QIC_RW_MASK                  1

#define DFX_SEC_QIC_QTP_RSP_OFFSET           54
#define DFX_SEC_QIC_QTP_RSP_MASK             7

#define DFX_SEC_QIC_QTP_SF_OFFSET            42
#define DFX_SEC_QIC_QTP_SF_MASK              1

#define DFX_SEC_QIC_MID_MASK                 0xFF

#define DFX_SEC_QIC_READ_OPERATION           0
#define DFX_SEC_QIC_WRITE_OPERATION          1
#define DFX_SEC_QIC_RW_ERROR                 2

#define DFX_SEC_QIC_FCM_PERI_PORT            0x5A
#define DFX_SEC_QIC_ACPU_COREID_OFFSET       13
#define DFX_SEC_QIC_ACPU_COREID_MASK         0xF
#define DFX_SEC_ERR_ADDR_ERROR               0

#define dfx_sec_qic_parse_addr(high, low)    ((((high) & 0x1FFFFF) << 17) + (((low) >> 47) & 0x1FFFF))

int dfx_sec_qic_get_errinfo(const struct dfx_sec_qic_device *qic_dev, bool from_irq);
void dfx_sec_qic_reset_handler(void);
#endif

