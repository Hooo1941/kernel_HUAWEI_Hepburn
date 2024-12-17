/*
 * vfmw_sys.h
 *
 * This is for vfmw_sys interface.
 *
 * Copyright (c) 2017-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef VFMW_SYS_H
#define VFMW_SYS_H

#include "vcodec_types.h"

#define SCD_NUM 2
#define VDH_NUM 2

#define MDMA_NUM 0
#define VDH_REG_NUM 1

#define VDH_SRST_REQ_REG_OFS 0xc
#define VDH_SRST_OK_REG_OFS  0x14

#define ALL_RESET_OK_BIT          0
#define MFDE_RESET_OK_BIT         1U
#define SCD_RESET_OK_BIT          2U
#define BSP_STATE_BIT             30U
#define PXP_STATE_BIT             29U

#define ALL_RESET_OK_MASK         (1U << ALL_RESET_OK_BIT)
#define MFDE_RESET_OK_MASK        (1U << MFDE_RESET_OK_BIT)
#define SCD_RESET_OK_MASK         (1U << SCD_RESET_OK_BIT)
#define BSP_CANCEL_TASK_OK_MASK   (1U << BSP_STATE_BIT)
#define PXP_CANCEL_TASK_OK_MASK   (1U << PXP_STATE_BIT)

struct vcrg_vdh_clksel {
	uint32_t vdh_all_clken : 2;
	uint32_t vdh_mfde_clken : 2;
	uint32_t vdh_pxp_clken : 2;
	uint32_t vdh_scd_clken : 2;
	uint32_t vdh_bpd_clken : 2;
	uint32_t vdh_bsp_clken : 2;
	uint32_t vdh_ppfc_cken : 2;
	uint32_t vdh_crc_cken : 2;
	uint32_t reserved : 16;
};

struct vcrg_vdh_srst_req {
	uint32_t vdh_all_srst_req : 1;
	uint32_t vdh_mfde_srst_req : 1;
	uint32_t vdh_scd_srst_req : 1;
	uint32_t vdh_bpd_srst_req : 1;
	uint32_t reserved : 28;
};

struct vcrg_vdh_srst_ok {
	uint32_t vdh_all_srst_ok : 1;
	uint32_t vdh_mfde_srst_ok : 1;
	uint32_t vdh_scd_srst_ok : 1;
	uint32_t vdh_bpd_srst_ok : 1;
	uint32_t reserved : 28;
};

struct vfmw_global_info {
	UADDR crg_reg_phy_addr;
	uint8_t *crg_reg_vaddr;
	uint32_t crg_reg_size;
	uint32_t is_fpga;
	void *dev;
};

#endif
