/*
 * pcie-kport-common.h
 *
 * PCIe common functions
 *
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _PCIE_KPORT_COMMON_H
#define _PCIE_KPORT_COMMON_H

#include <securec.h>
#include "pcie-kport.h"

#define INBOUNT_OFFSET 0x100

#define PER_ATU_SIZE 0x200

/* noc power domain */
#define NOC_POWER_IDLEREQ_1		0x38c
#define NOC_POWER_IDLE_1		0x394
#define NOC_PW_MASK			0x10000
#define NOC_PW_SET_BIT			0x1

#define PCIEPHY_RESET_BIT		(0x1 << 17)
#define PCIEPHY_PIPE_LINE0_RESET_BIT	(0x1 << 19)
#define PCIEPHY_PIPE_REFCLK_MODE_BIT	(0x1 << 6)

#define PCIE_REF2PERST1_DELAY_MIN	20000
#define PCIE_REF2PERST2_DELAY_MAX	21000
#define PCIE_PERST2ACCESS1_DELAY_MIN	7000
#define PCIE_PERST2ACCESS2_DELAY_MAX	8000
#define PCIE_PERST2RST1_DELAY_MIN	10000
#define PCIE_PERST2RST2_DELAY_MAX	11000

#define PCI_PROGRAM_INTERFACE		8
#define PCI_CLASS_CODE_MASK		0xFFFF0000
#define PCI_VENDOR_ID_MASK		0x0000FFFF

/* ECO Function for PHY Debug */
#define ECO_BYPASS_ADDR			SOC_PCIEPHY_CTRL40_ADDR

#define LTSSM_STATUE_MASK		0x3F
#define LTSSM_L1SS_MASK			0xC000

#define PCIE_CTRL_BUS_TIMEOUT_MASK	(0x3F << 1)
#define PCIE_CTRL_BUS_TIMEOUT_VALUE	(0x36 << 1)

/* pcie ep device type */
#define pcie_ep_is_modem(pcie)  ((pcie)->dtsinfo.ep_device_type == EP_DEVICE_MODEM)
#define pcie_ep_is_fpga(pcie)  ((pcie)->dtsinfo.ep_device_type == EP_DEVICE_FPGA)

/* bit operations */
#define dword_low(addr)  ((addr) & 0xffffffff)
#define dword_high(addr)  (((addr) >> 32) & 0xffffffff)

/* get a specific bit */
#define get_bit(val, bit) (((val) & (1 << (bit))) >> (bit))

/*  EP start  */
#define PCIE_PERST_IN_INTR_MASK		(0x1 << 4)
#define PCIE_PERST_IN_MASK_BIT		25
#define PCIE_PAD_PERST_IN_MASK		(0x1 << 27)
#define PCIE_PAD_PERST_IN_BIT		27

#define PCIE_FUNCTION0_OFFSET		(0x1 << 16)
#define PCIE_FUNCTION1_OFFSET		(0x2 << 16)
#define PCIE_FUNCTION2_OFFSET		(0x3 << 16)
#define PCIE_FUNCTION3_OFFSET		(0x4 << 16)
#define PCIE_FUNCTION4_OFFSET		(0x5 << 16)
#define PCIE_FUNCTION5_OFFSET		(0x6 << 16)
#define PCIE_FUNCTION6_OFFSET		(0x7 << 16)
#define PCIE_FUNCTION7_OFFSET		(0x8 << 16)

/* DMA related */
#define PCIE_DMA_LOCAL_OFFSET		((0x7) << 19)
#define DMA_CHANNEL_CONTROL1_VAL	0x4000008
#define DMA_WR_ENGINE_ENABLE		0x00C
#define DMA_LL_PT_LOW				0x01C
#define DMA_LL_PT_HIGH				0x020
#define DMA_RD_ENGINE_ENBALE		0x02C
#define DMA_WR_INT_MASK				0x054
#define DMA_RD_INT_MASK				0x0A8
#define DMA_TRANS_SIZE				0x008
#define DMA_SAR_LOW					0x00C
#define DMA_SAR_HIGH				0x010
#define DMA_DAR_LOW					0x014
#define DMA_DAR_HIGH				0x018
#define DMA_RD_DOORBELL				0x030
#define DMA_WR_DOORBELL				0x010
#define DMA_WR_LL_ERROR_EN			0x090
#define DMA_WR_PWR_EN_CH0			0x128
#define DMA_LL_TRANS_START			0x1000
#define DMA_LL_NUM					3

/* EP end  */

/* all plat with the default device id, for product safety */
#define PCIE_DEFAULT_DEVICE_ID 0x36A0
#define PCIE_DENVER_DEVICE_ID 0x6290

int config_enable_dbi(u32 rc_id, int flag);
int ltssm_enable(u32 rc_id, int yes);
int set_bme(u32 rc_id, int flag);
int set_mse(u32 rc_id, int flag);
void pcie_kport_set_devid(struct pcie_kport *pcie);
void pcie_config_axi_timeout(struct pcie_kport *pcie);
int pcie_noc_power(struct pcie_kport *pcie, int enable);
int pcie_port_phy_init(struct pcie_kport *pcie);
void pcie_natural_cfg(struct pcie_kport *pcie);
int pcie_phy_fw_update(struct pcie_kport *pcie, u16 *fw_data, u32 fw_size);
void pcie_config_l0sl1(u32 rc_id, enum link_aspm_state aspm_state);
void pcie_config_l1ss(u32 rc_id, enum l1ss_ctrl_state enable);
void pcie_kport_inbound_atu(u32 rc_id, int index,
		int type, u64 cpu_addr, u64 pci_addr, u32 size);
void pcie_io_adjust(struct pcie_kport *pcie);
int pcie_generate_msg(u32 rc_id, int index, u32 iatu_offset,
		int msg_type, u32 msg_code);
void enable_req_clk(struct pcie_kport *pcie, u32 enable_flag);
void pcie_axi_timeout_mask(struct pcie_kport *pcie);
void pcie_set_clkreq_high(struct pcie_kport *pcie);
void pcie_ep_check_perst_in(struct pcie_kport *pcie);
void pcie_ep_clear_intr(struct pcie_kport *pcie);

u32 show_link_state(u32 rc_id);

#ifdef CONFIG_PCIE_KPORT_TEST
int set_link_speed(u32 rc_id, enum link_speed gen);
int pcie_ep_mac_init(u32 pcie_id, u32 func_num);
int pcie_set_ep_mode(u32 rc_id);
#else
static inline int set_link_speed(u32 rc_id, enum link_speed gen)
{
	return 0;
}

static inline int pcie_ep_mac_init(u32 pcie_id, u32 func_num)
{
	return 0;
}

static inline int pcie_set_ep_mode(u32 rc_id)
{
	return 0;
}
#endif

#ifdef CONFIG_FMEA_FAULT_INJECTION
int pcie_fault_injection(void);
#endif

#endif
