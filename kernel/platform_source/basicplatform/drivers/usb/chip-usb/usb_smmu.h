/*
 * Copyright (C) 2023 Hisilicon
 * Author: Hisillicon <>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _LOCAL_USB_SMMU_H_
#define _LOCAL_USB_SMMU_H_
#include <linux/platform_drivers/usb/usb_reg_cfg.h>
#include "usb_misc_ctrl.h"

#define smmu_info(format, arg...) \
	pr_info("[USB_SMMU][I][%s]"format, __func__, ##arg)

#define smmu_err(format, arg...) \
	pr_err("[USB_SMMU][E][%s]"format, __func__, ##arg)


struct usb_smmu {
	u32 smmu_id;
	u32 sid;
	u32 tbu_num;
	bool support_smmu;
	bool bypass_smmu;
	void __iomem *tbu_base;
	void __iomem *tbu_sid_base;
	u32 mmusid_off[2];
	u32 sec_bit_offset;
	u32 port_id;
	struct chip_usb_reg_cfg *usb_smmu_hsdt1_unreset;
	struct chip_usb_reg_cfg *usb_smmu_tbu_unreset;
	struct chip_usb_reg_cfg *usb_smmu_hsdt1_reset;
	struct chip_usb_reg_cfg *usb_smmu_tbu_reset;
};

#define TBU_CONFIG_TIMEOUT      100
#define TBU_CR                  0x0
#define TBU_CRACK               0x4
#define TBU_EN_REQ_BIT          BIT(0)
#define TBU_EN_ACK_BIT          BIT(0)
#define TBU_CONNCT_STATUS_BIT   BIT(1)
#define TBU_TOK_TRANS_MSK       (0xFF << 8)
#define TBU_TOK_TRANS           0x8
#define TBU_DEFALUT_TOK_TRANS   0x3
#define MAX_TBU_INDEX 2
#define MAX_TBU_NUM 3
#define ENABLE 1
#define DISABLE 0
#define SEC_ENABLE 0x3
#define SEC_DISABLE 0x0
#define SMMU_TBU_IRPT_CLR_NS_OFFSET 0x001C
#define SMMU_TBU_IRPT_CLR_NS_ECC_1BIT_MSK         (BIT(4))
#define SMMU_TBU_IRPT_CLR_NS_ECC_MULTI_BITS_MSK   (BIT(5))

#define SMMU_TBU_MEM_CTRL_S_DR_OFFSET		0x0008
#define SMMU_TBU_MEM_CTRL_S_SR_OFFSET		0x002C
#define SMMU_TBU_MEM_CTRL_S_ECC_ENABLE		BIT(25)
#define SMMU_TBU_MEM_CTRL_T_DR_OFFSET		0x0028
#define SMMU_TBU_MEM_CTRL_T_SR_OFFSET		0x0034
#define SMMU_TBU_MEM_CTRL_T_ECC_ENABLE		BIT(15)
#define SMMU_TBU_MEM_CTRL_B_DR_OFFSET		0x000C
#define SMMU_TBU_MEM_CTRL_B_SR_OFFSET		0x0030
#define SMMU_TBU_MEM_CTRL_B_ECC_ENABLE		BIT(13)
#define SMMU_TBU_IRPT_MASK_NS_OFFSET		0x0010
#define SMMU_TBU_IRPT_MASK_NS_ECC_MSK		~(BIT(4) | BIT(5))
#define SMMU_TBU_MEM_CTRL_TRA_OFFSET        0x34
#define SMMU_TBU_MEM_CTRL_TRA_VALUE         BIT(11)
#define SMMU_TBU_MEM_CTRL_TRA_MSK           (BIT(11) | BIT(12))

int usb_smmu_init(struct chip_usb_misc_ctrl *misc_ctrl);
int usb_disable_smmu(struct chip_usb_misc_ctrl *misc_ctrl);
int get_smmu_info(struct chip_usb_misc_ctrl *misc_ctrl);

#endif
