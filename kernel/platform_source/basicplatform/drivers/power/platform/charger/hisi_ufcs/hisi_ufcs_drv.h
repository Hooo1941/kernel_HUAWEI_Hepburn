/*
 * hisi_ufcs_drv.h
 *
 * hisi ufcs driver
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

#ifndef _HISI_UFCS_DRV_
#define _HISI_UFCS_DRV_

#include "../schargerV600/hisi_scharger_v600.h"

#define UFCS_PAGE0_BASE_ADDR  (0x380)

struct reg_info {
	u16 irq_flag;            // 0x80+0x62=0xE2
	u16 sc_ufcs_en;          // 0x00
	u16 sc_hk_det_cnt_cfg;   // 0x02
	u16 sc_m0_baud_delta;    // 0x03
	u16 sc_ufcs_hw_rst;      // 0x11
	u16 sc_wd_clr;           // 0x12
	u16 sc_send_start;       // 0x14
	u16 sc_send_cable_rst;   // 0x15
	u16 sc_send_rst;         // 0x16
	u16 sc_msg_anck_tmpl_dat0; // 0x17
	u16 sc_msg_anck_tmpl_dat1; // 0x18
	u16 ufcs_ctrl_en_reg_1;    // 0x1B
	u16 sc_frame_idle_cnt_cfg; // 0x1E
	u16 sc_baud_cfg;           // 0x1f
	u16 sc_ufcs_soft_rst_n;    // 0x20
	u16 sc_read_rx_req;        // 0x21
	u16 ufcs_rx_status_ro;     // 0x26
	u16 ufcs_irq_flg_0;        // 0x27
	u16 ufcs_irq_flg_1;        // 0x28
	u16 ufcs_irq_mask_0;       // 0x29
	u16 ufcs_irq_mask_1;       // 0x2A
	u16 ufcs_pkg_type;         // 0x2E
	u16 ufcs_tx_msg_len;
	u16 tx_ram_data;
	u16 rx_ram_data;
	u16 ufcs_rx_msg_len;
	u16 irq_vusb_uv_flag;
	u16 fcp_detect_ctrl;
};

#define UFCS_DA_RESERVE_REG      (UFCS_PAGE0_BASE_ADDR + (0x35))

#define UFCS_RX_ANCK_WR_SHIFT    4
#define UFCS_RX_ANCK_WR_MASK     (1 << UFCS_RX_ANCK_WR_SHIFT)

#define HK_SUCC                   BIT(7)
#define HK_FAIL                   BIT(6)
#define BUFF_LEN_MAX              64
#define USLEEP_1MS                1000
#define USLEEP_100US              100

#define SC_SENDER_RESP_TMR_EN_SHIFT   1
#define SC_SENDER_RESP_TMR_EN_MSK     (1 << SC_SENDER_RESP_TMR_EN_SHIFT)

#define SC_HRESET_EN_SHIFT            0
#define SC_HRESET_EN_MSK              (1 << SC_HRESET_EN_SHIFT)

#define SC_HREST_CABLE_SHIFT          0
#define SC_HREST_CABLE_MSK            (1 << SC_HREST_CABLE_SHIFT)

#define SC_HW_CRC_NE_SHIFT            5
#define SC_HW_CRC_NE_MSK              (1 << SC_HW_CRC_NE_SHIFT)
#define RO_UFCS_RX_READ_ACK           (BIT(2))

#define WAIT_ACK_RETRY                120 // 120ms

#define IRQ_UFCS_NUM          2
#define IRQ_UFCS_WD_TMR       BIT(0)
#define IRQ_UFCS_HW_RST       BIT(1)
#define IRQ_UFCS_RX_ACK_ERR   BIT(2)
#define IRQ_UFCS_RX_ACK_OK    BIT(3)
#define IRQ_UFCS_RX_END       BIT(4)
#define IRQ_UCFS_BPS_DET      (BIT(0) << 8)
#define IRQ_UFCS_RCVR_TMR     (BIT(1) << 8)
#define IRQ_UFCS_SDR_TMR      (BIT(2) << 8)
#define IRQ_UFCS_ACK_TMR      (BIT(3) << 8)
#define IRQ_UFCS_TX_END       (BIT(4) << 8)
#define IRQ_UFCS_RER_OVER     (BIT(5) << 8)
#define IRQ_UFCS_HK_FAIL      (BIT(6) << 8)
#define IS_UFCS_IRQ           BIT(6)
#define HI6526V510_UFCS_BASE  0x380
#define MASK_ALL              0xFF
#define IRQ_MASK_ALL          0xFFFF
#define SINK_ADDR_MASK        0xE0
#define MIN_MSG_LEN           0x2


#define UFCS_DRAME_IDLE_CNT5        0x5

#define UFCS_BAUD_RATE_DELTA7       0x7

/* schargerv800 regs */
#define HI6526V800_UFCS_BASE        0x2000
#define CHG_IRQ_FLAG_REG            0x800
#define CHG_IRQ_FLAG_0_REG          0x802
#define FCP_FCP_CTRL_REG            0x11f

#define SCHARGER_VERSION_V600       0
#define SCHARGER_VERSION_V700       1
#define SCHARGER_VERSION_V800       2

struct ufcs_ram {
	u8 data[BUFF_LEN_MAX];
	u8 len;
	u8 state; /* 0: fifo is null，1:data ready, read end set state 0 */
};

struct ufcs_driver_info {
	int irq_gpio;
	int irq;
	bool communicating_flag;
	struct work_struct irq_work;
	struct device *dev;
	struct regmap *regmap;
	struct mutex ufcs_lock;
	struct mutex ufcs_worker_lock;
	struct reg_info regs;
	struct ufcs_ram rx_fifo;
	unsigned int chip_version;
};

#endif
