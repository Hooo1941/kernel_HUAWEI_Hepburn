/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_REGMAP_FCP_H
#define HVGR_REGMAP_FCP_H

/* Begin Register Offsets */
#define FCP_BASE                   0x10000
#define FCP_START_CODE             0
#define FCP_DTCM_START_ADDR        0x5000
#define FCP_ITCM_CQ_START_ADDR     0x8000
#define FCP_DTCM0_CQ_START_ADDR    0x9000
#define FCP_DTCM1_CQ_START_ADDR    0xB000
#define FCP_MB_MCU_KEY             0xD000

#define FCP_MB_MCUVECTOR_ADDR               0x0
#define FCP_MB_MCU_CPUWAIT                  0x4
#define FCP_MB_MCUBOOT_STATUS               0x8
#define FCP_MB_MCUSFTRST                    0xC
#define FCP_MB_MCUSLEEP_STATUS              0x10
#define FCP_MB_MCU_ERROR                    0x14

#define FCP_MB_MCU_DDR_MAP_BASE             0x20

#define FCP_MB_MCU_VAADDR                   0x40
#define FCP_MB_MCU_ACCESS_MODE              0x44
#define FCP_MB_MCU_JASID_SEL                0x48
#define FCP_MB_MCU_REG_ADDR                 0x50
#define FCP_MB_MCU_REG_WDATA                0x54
#define FCP_MB_MCU_REG_RDATA                0x58
#define FCP_MB_MCU_REG_CTRL                 0x5c
#define FCP_MB_MCU_REG_STATUS               0x60

#define FCP_MB_MCUVECTOR_ADDR_CQ            0x100
#define FCP_MB_MCU_CPUWAIT_CQ               0x104
#define FCP_MB_MCUBOOT_STATUS_CQ            0x108
#define FCP_MB_MCUSFTRST_CQ                 0x10C
#define FCP_MB_MCUSLEEP_STATUS_CQ           0x110
#define FCP_MB_MCU_ERROR_CQ                 0x114

#define FCP_MB_MCU_DDR_MAP_CQ_BASE          0x120

#define FCP_MB_MCU_VAADDR_CQ                0x140
#define FCP_MB_MCU_ACCESS_MODE_CQ           0x144
#define FCP_MB_MCU_JASID_SEL_CQ             0x148
#define FCP_MB_MCU_REG_ADDR_CQ              0x150
#define FCP_MB_MCU_REG_WDATA_CQ             0x154
#define FCP_MB_MCU_REG_RDATA_CQ             0x158
#define FCP_MB_MCU_REG_CTRL_CQ              0x15C
#define FCP_MB_MCU_REG_STATUS_CQ            0x160
#define FCP_MB_EDMA2MCU_CF_MASKINT          0x210
#define FCP_MB_EDMA2MCU_CQ_MASKINT          0x214
#define FCP_MB_EDMA2JM_MASKINT              0x218
#define FCP_MB_EDMA2HOST_MASKINT            0x21C

#define FCP_MB_CF_MCU_IRQ                   0x220
#define FCP_MB_CF_JOB_INFO0                 0x224
#define FCP_MB_CF_JOB_INFO1                 0x228
#define FCP_MB_CF_MCU_IRQ_CLR               0x22C
#define FCP_MB_CF_MCU_DONE                  0x230
#define FCP_MB_CQ_MCU_IRQ                   0x240
#define FCP_MB_CQ_MCU_ADDR                  0x244
#define FCP_MB_CQ_MCU_INFO                  0x248
#define FCP_MB_CQ_MCU_IRQ_CLR               0x24C
#define FCP_MB_CQ_MCU_DONE                  0x250

#define FCP_MB_HOST2FCP_MSG_0              0x280
#define FCP_MB_HOST2FCP_MSG_1              0x284
#define FCP_MB_HOST2FCP_MSG_2              0x288
#define FCP_MB_HOST2FCP_MSG_3              0x28C
#define FCP_MB_HOST2FCP_MSG_VLD            0x290
#define FCP_MB_HOST2FCP_MSG_VLD_CLR        0x294

#define FCP_MB_FCP2HOST_MSG_0              0x2A0
#define FCP_MB_FCP2HOST_MSG_1              0x2A4
#define FCP_MB_FCP2HOST_MSG_VLD            0x2A8
#define FCP_MB_FCP2HOST_MSG_VLD_CLR        0x2AC

#define FCP_DMA_CHANEL_BASE                 0xD800

#define FCP_DMA_CX_CNT                      0x20
#define FCP_DMA_CX_SRC_ADDR_L               0x24
#define FCP_DMA_CX_SRC_ADDR_H               0x28
#define FCP_DMA_CX_DES_ADDR_L               0x2C
#define FCP_DMA_CX_DES_ADDR_H               0x30
#define FCP_DMA_CX_CONFIG                   0x34
#define FCP_DMA_BUFDATA                     0x38

#define FCP_DMA_CH_INT_TCRAW                0x0

#define FCP_TIMER_LOAD_BASE                 0xE000

#define FCP_TIMER_CLK_CONTROL             0x0
#define FCP_TIMER_INIT_COUNTER            0x4
#define FCP_TIMER_CONTROL                 0x8
#define FCP_TIMER_INT_STATUS              0xC
#define FCP_TIMER_INT_CLEAR               0x10
#define FCP_TIMER_COUNTER                 0x14
#define FCP_TIMER_VERSION                 0xF0
#define FCP_INT_MASK                      0xFFFFFFFFu
#define FCP_INT_UNMASK                    0x0u

/* (RW) Read action of FCP control configuration */
#define GPU_FCP_CFG_READ  1
#define GPU_FCP_CFG_WRITE 0   /* (RW) Write action of control configuration */

/*
 * Maximum number of loops for completed in reading/writing FCP registers
 * before we assume it must have completed
 */
#define GPU_FCP_CFG_TRY_COUNT  100000
/* Set when a cache clean operation has completed. */
#define GPU_FCP_CFG_COMPLETED  ((u32)1 << 0)

#define fcp_dma_cfg_val(jasid, enable) ((jasid) << 4 | (enable))

#define fcp_active_bits(val) (((val) >> 3) & 0x3)

#define fcp_reg_base(p) ((p)->dm_dev.reg_base.fcp_base.fcp_reg_base)

/* ITCM data start address */
#define fcp_itcm_offset(p) \
	((p)->dm_dev.reg_base.fcp_base.fcp_itcm_offset)

#define fcp_itcm_reg(p, r) (fcp_reg_base(p) + fcp_itcm_offset(p) + (r))

/* DTCM data start address */
#define fcp_dtcm_offset(p) \
	((p)->dm_dev.reg_base.fcp_base.fcp_dtcm_offset)

#define fcp_dtcm_reg(p, r) (fcp_reg_base(p) + fcp_dtcm_offset(p) + (r))

/* ITCM_CQ data start address */
#define fcp_cq_itcm_offset(p) \
	((p)->dm_dev.reg_base.fcp_base.fcp_itcm_cq_offset)

#define fcp_cq_itcm_reg(p, r) (fcp_reg_base(p) + fcp_cq_itcm_offset(p) + (r))

/* DTCM0_CQ data start address */
#define fcp_cq_dtcm0_offset(p) \
	((p)->dm_dev.reg_base.fcp_base.fcp_dtcm0_cq_offset)

#define fcp_cq_dtcm0_reg(p, r) (fcp_reg_base(p) + fcp_cq_dtcm0_offset(p) + \
	(r))

/* DTCM1_CQ data start address */
#define fcp_cq_dtcm1_offset(p) \
	((p)->dm_dev.reg_base.fcp_base.fcp_dtcm1_cq_offset)

#define fcp_cq_dtcm1_reg(p, r) (fcp_reg_base(p) + fcp_cq_dtcm1_offset(p) + \
	(r))

#define fcp_reg_mb_offset(p) \
	((p)->dm_dev.reg_base.fcp_base.fcp_reg_mb_offset)

#define fcp_mb_reg(p, r) (fcp_reg_base(p) + fcp_reg_mb_offset(p) + (r))

#define fcp_mb_mcu_ddr_map_reg(p, n) (fcp_reg_base(p) + \
	fcp_reg_mb_offset(p) + FCP_MB_MCU_DDR_MAP_BASE + ((u32)(n) << 2))

#define fcp_mb_mcu_ddr_map_wind(p, n) (fcp_reg_base(p) + \
	fcp_reg_mb_offset(p) + FCP_MB_MCU_DDR_MAP_BASE + 0x10 + \
	((u32)(n) << 2))

#define fcp_mb_mcu_ddr_map_cq_reg(p, n) (fcp_reg_base(p) + \
	fcp_reg_mb_offset(p) + FCP_MB_MCU_DDR_MAP_CQ_BASE + ((u32)(n) << 2))

#define fcp_mb_mcu_ddr_map_cq_wind(p, n) (fcp_reg_base(p) + \
	fcp_reg_mb_offset(p) + FCP_MB_MCU_DDR_MAP_CQ_BASE + 0x10 + \
	((u32)(n) << 2))

#define fcp_reg_dma_offset(p) \
	((p)->dm_dev.reg_base.fcp_base.fcp_reg_dma_offset)

#define fcp_dma_reg(p, r) (fcp_reg_base(p) + fcp_reg_dma_offset(p) + (r))

#define fcp_dma_chanel_reg(p, n, r) ((fcp_reg_base(p) + \
	fcp_reg_dma_offset(p) + ((u32)(n) << 5)) + (r))

#define fcp_reg_timer_offset(p) \
	((p)->dm_dev.reg_base.fcp_base.fcp_reg_timer_offset)

#define fcp_timer_load_reg(p, n, r) ((fcp_reg_base(p) + \
	fcp_reg_timer_offset(p) + ((u32)(n) << 10)) + (r))


#endif /* HVGR_REGMAP_FCP_H */
