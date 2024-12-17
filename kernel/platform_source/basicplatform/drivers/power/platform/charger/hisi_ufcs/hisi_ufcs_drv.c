/*
 * hisi_ufcs_drv.c
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

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/regmap.h>
#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>

#include <chipset_common/hwpower/protocol/adapter_protocol.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_ufcs.h>
#include "hisi_ufcs_drv.h"

#define ufcs_debug(fmt, args...) pr_info("[hi_ufcs] %s " fmt, __func__, ## args)
#define ufcs_info(fmt, args...) pr_info("[hi_ufcs] %s " fmt, __func__, ## args)
#define ufcs_warn(fmt, args...) pr_warn("[hi_ufcs] %s " fmt, __func__, ## args)
#define ufcs_err(fmt, args...) pr_err("[hi_ufcs] %s " fmt, __func__, ## args)

static void __hisi_ufcs_interrupt_work(struct ufcs_driver_info *ufcs);

static void dump_rx_ram(struct ufcs_driver_info *ufcs)
{
	size_t i;
	unsigned int val = 0;

	if (!ufcs) {
		ufcs_err("%s ufcs is null\n",__func__);
		return;
	}

	for(i = 0; i < 10; i++) {
		regmap_read(ufcs->regmap, ufcs->regs.rx_ram_data + i, &val);
		ufcs_info("rx data[0x%x] = 0x%x\n", ufcs->regs.rx_ram_data + i, val);
	}
}

static void hisi_ufcs_en(struct ufcs_driver_info *ufcs, int en)
{
	unsigned int irq_clear_all = IRQ_MASK_ALL;

	if (!ufcs) {
		ufcs_err("%s ufcs is null\n",__func__);
		return;
	}

	if(en == 0) {
		regmap_write(ufcs->regmap, ufcs->regs.sc_send_start, 0);
		regmap_bulk_write(ufcs->regmap, ufcs->regs.ufcs_irq_flg_0, &irq_clear_all, IRQ_UFCS_NUM);
	}

	regmap_write(ufcs->regmap, ufcs->regs.sc_ufcs_en, en);
}

static unsigned int is_ufcs_enabled(struct ufcs_driver_info *ufcs)
{
	unsigned int val = 0;

	if (!ufcs) {
		ufcs_err("%s ufcs is null\n",__func__);
		return EPERM;
	}

	regmap_read(ufcs->regmap, ufcs->regs.sc_ufcs_en, &val);

	if(val == 0)
		ufcs_err("ufcs is disabled\n");

	return val;
}


static void hw_crc_enable(struct ufcs_driver_info *ufcs)
{
	regmap_update_bits(ufcs->regmap, ufcs->regs.ufcs_ctrl_en_reg_1,
		SC_HW_CRC_NE_MSK, 1 << SC_HW_CRC_NE_SHIFT);
}

static void rx_anck_wr_enable(struct ufcs_driver_info *ufcs)
{
	regmap_update_bits(ufcs->regmap, ufcs->regs.ufcs_ctrl_en_reg_1,
		UFCS_RX_ANCK_WR_MASK, 1 << UFCS_RX_ANCK_WR_SHIFT);
}

/* avoid protocol conflicts with ufcs and fcp */
static inline void disable_fcp_protocol_detect(struct ufcs_driver_info *ufcs)
{
	regmap_update_bits(ufcs->regmap,ufcs->regs.fcp_detect_ctrl, CHG_FCP_DET_EN_MSK, 0 << CHG_FCP_DET_EN_SHIFT);
}

static void hisi_ufcs_init(struct ufcs_driver_info *ufcs)
{
	disable_fcp_protocol_detect(ufcs);

	/* irq mask */
	regmap_write(ufcs->regmap, ufcs->regs.ufcs_irq_mask_0, 0xEC);
	regmap_write(ufcs->regmap, ufcs->regs.ufcs_irq_mask_1, MASK_ALL);
	hw_crc_enable(ufcs);
	rx_anck_wr_enable(ufcs);

	/* ACK head */
	regmap_write(ufcs->regmap, ufcs->regs.sc_msg_anck_tmpl_dat0, 0x20);
	regmap_write(ufcs->regmap, ufcs->regs.sc_msg_anck_tmpl_dat1, 0x08);

	regmap_write(ufcs->regmap, UFCS_DA_RESERVE_REG, 0x80);
	regmap_write(ufcs->regmap, ufcs->regs.sc_frame_idle_cnt_cfg, UFCS_DRAME_IDLE_CNT5);
	regmap_write(ufcs->regmap, ufcs->regs.sc_m0_baud_delta, UFCS_BAUD_RATE_DELTA7);
}

static void rx_fifo_init(struct ufcs_driver_info *ufcs)
{
	ufcs->rx_fifo.len = 0;
	ufcs->rx_fifo.state = 0;
	(void)memset_s(ufcs->rx_fifo.data, BUFF_LEN_MAX, 0 , BUFF_LEN_MAX);
}

static int hisi_ufcs_detect_adapter(void *dev_data)
{
	struct ufcs_driver_info *ufcs = dev_data;
	struct reg_info *regs = NULL;
	int wait_cnt = 10;
	unsigned int reg_val = 0;
	int ret = HWUFCS_DETECT_OTHER;
	u16 irq_flag_val = 0;

	if (!ufcs) {
		ufcs_err("%s ufcs is null\n",__func__);
		return HWUFCS_DETECT_OTHER;
	}

	regs = &ufcs->regs;

	mutex_lock(&ufcs->ufcs_lock);

	hisi_ufcs_init(ufcs);

	rx_fifo_init(ufcs);

	regmap_bulk_read(ufcs->regmap, ufcs->regs.ufcs_irq_flg_0,
		(unsigned int *)&irq_flag_val, IRQ_UFCS_NUM);
	regmap_bulk_write(ufcs->regmap, ufcs->regs.ufcs_irq_flg_0, &irq_flag_val, IRQ_UFCS_NUM);

	ufcs_info("ufcs_irq_flg = 0x%x\n", irq_flag_val);

	/* ufcs en, start handshake */
	hisi_ufcs_en(ufcs, 1);

	/* waiting for handshake */
	while (wait_cnt) {
		mdelay(10);
		wait_cnt--;
		regmap_read(ufcs->regmap, regs->ufcs_irq_flg_1, &reg_val);
		if (reg_val) {
			regmap_write(ufcs->regmap, regs->ufcs_irq_flg_1, reg_val);
			ufcs_info("[%d] flag1 0x%x = 0x%x\n", wait_cnt, regs->ufcs_irq_flg_1, reg_val);
		}
		if (reg_val & HK_SUCC) {
			ufcs_info("Handshake SUCC\n");
			ret = HWUFCS_DETECT_SUCC;
			break;
		} else if (reg_val & HK_FAIL) {
			ufcs_err("Handshake FAIL\n");
			hisi_ufcs_en(ufcs, 0);
			ret = HWUFCS_DETECT_FAIL;
			break;
		}
	}
	mutex_unlock(&ufcs->ufcs_lock);

	ufcs_info("handshake ret %d, wait_cnt %d\n", ret, wait_cnt);
	return ret;
}

static int hisi_ufcs_rx_read_request(struct ufcs_driver_info *ufcs, u8 req)
{
	ufcs_debug(" ------- %d\n", req);
	return regmap_write(ufcs->regmap, ufcs->regs.sc_read_rx_req, req);
}

static int hisi_ufcs_vusb_uv(struct ufcs_driver_info *ufcs)
{
	unsigned int vusb_uv_flag = 0;

	if (!ufcs) {
		ufcs_err("%s ufcs is null\n",__func__);
		return -EPERM;
	}

	regmap_read(ufcs->regmap, ufcs->regs.irq_vusb_uv_flag, &vusb_uv_flag);

	if (vusb_uv_flag & BIT(2)) {
		ufcs_err("adapter pluggout\n");
		return 1;
	}

	return 0;
}

static int hisi_ufcs_wait(struct ufcs_driver_info *ufcs, u8 flag)
{
	unsigned int irq_flag_val = 0;
	unsigned int irq_flag_val_temp = 0;
	int ret, i;
	ufcs_info("+ flag 0x%x\n", flag);

	for (i = 0; i < WAIT_ACK_RETRY; i++) {
		mdelay(1);
		irq_flag_val = 0;

		if (hisi_ufcs_vusb_uv(ufcs))
			return -EINVAL;

		if (!is_ufcs_enabled(ufcs))
			return -ENODEV;

		ret = regmap_bulk_read(ufcs->regmap,
			ufcs->regs.ufcs_irq_flg_0, (unsigned int *)&irq_flag_val, 2);
		irq_flag_val_temp = irq_flag_val & (~IRQ_UFCS_RX_END);
		if (irq_flag_val)
			regmap_bulk_write(ufcs->regmap, ufcs->regs.ufcs_irq_flg_0, &irq_flag_val_temp, 2);

		ufcs_info("ufcs_irq_flg_0 = 0x%x\n", irq_flag_val);

		if (irq_flag_val & IRQ_UFCS_RX_ACK_ERR)
			ufcs_err("ack error\n");

		if ((flag & HWUFCS_WAIT_SEND_PACKET_COMPLETE) &&
			(irq_flag_val & IRQ_UFCS_TX_END)) {
			flag = (flag & (~HWUFCS_WAIT_SEND_PACKET_COMPLETE));
			ufcs_info("IRQ_UFCS_TX_END, flag 0x%x\n", flag);
		}

		if (flag & HWUFCS_WAIT_CRC_ERROR) {
			if (irq_flag_val & IRQ_UFCS_RX_ACK_ERR) {
				ufcs_err("crc error\n");
				return -EINVAL;
			}

			if (irq_flag_val & IRQ_UFCS_RX_ACK_OK) {
				flag = (flag & (~HWUFCS_WAIT_CRC_ERROR));
				ufcs_err("rx ack ok, flag 0x%x\n", flag);
			}

			if (irq_flag_val & IRQ_UFCS_RX_END) {
				flag = (flag & (~HWUFCS_WAIT_CRC_ERROR));
				ufcs_err("ufcs rx end,no crc err flag 0x%x\n", flag);
			}
		}

		if (flag & HWUFCS_WAIT_DATA_READY) {
			if (irq_flag_val & IRQ_UFCS_RX_END) {
				flag = (flag & (~HWUFCS_WAIT_DATA_READY));
				ufcs_err("ufcs rx end, flag 0x%x\n", flag);
			}
			if (irq_flag_val & IRQ_UFCS_SDR_TMR) {
				ufcs_err("ufcs sender response time out\n");
				return -EINVAL;
			}
		}

		if (flag & HWUFCS_ACK_RECEIVE_TIMEOUT) {
			if (irq_flag_val & IRQ_UFCS_ACK_TMR)
				ufcs_info("ACK receive time out\n");
			if (irq_flag_val & IRQ_UFCS_RX_ACK_OK) {
				flag = (flag & (~HWUFCS_ACK_RECEIVE_TIMEOUT));
				ufcs_err("rc ack ok, flag 0x%x\n", flag);
			}
			if (irq_flag_val & IRQ_UFCS_RER_OVER) {
				flag = (flag & (~HWUFCS_ACK_RECEIVE_TIMEOUT));
				ufcs_err("rc retry over, flag 0x%x\n", flag);
				return -EINVAL;
			}
		}

		if (flag == 0) {
			ufcs_info("wait succ\n");
			ufcs_err("-\n");
			return 0;
		}
	}

	ufcs_err("wait fail -\n");
	return -EINVAL;
}

static int hisi_ufcs_config_baud_rate(void *dev_data, int baud_rate)
{
	struct ufcs_driver_info *ufcs = dev_data;

	if (!ufcs) {
		ufcs_err("ufcs is null\n");
		return -ENODEV;
	}

	return regmap_write(ufcs->regmap, ufcs->regs.sc_baud_cfg, (unsigned int)baud_rate);
}

static int hisi_ufcs_write_msg(void *dev_data, u8 *data, u8 len, u8 flag)
{
	struct ufcs_driver_info *ufcs = dev_data;
	struct reg_info *regs = NULL;
	int ret, i;
	int lenth = len + 1; /* add 1 byte CRC */
	unsigned int irq_clear_all = IRQ_MASK_ALL;

	if (!ufcs || !data) {
		ufcs_err("ufcs or data is null\n");
		return -ENODEV;
	}

	if (!is_ufcs_enabled(ufcs))
		return -ENODEV;

	regs = &ufcs->regs;

	if (len > BUFF_LEN_MAX) {
		ufcs_err("invalid length=%u\n", len);
		return -EINVAL;
	}

	/* clear ufcs irq before send msg. */
	regmap_bulk_write(ufcs->regmap, ufcs->regs.ufcs_irq_flg_0, &irq_clear_all, IRQ_UFCS_NUM);

	regmap_write(ufcs->regmap, regs->ufcs_tx_msg_len, lenth);
	regmap_bulk_write(ufcs->regmap, regs->tx_ram_data, data, len);
	for(i = 0; i < len; i++)
		ufcs_debug("tx ram data[%d] = 0x%x\n", i, data[i]);

	regmap_write(ufcs->regmap, regs->sc_send_start, 1);
	ret = hisi_ufcs_wait(ufcs, flag);
	return ret;
}

// tSenderResponse Max is 50ms
static int hisi_ufcs_wait_msg_ready(void *dev_data, u8 flag)
{
	struct ufcs_driver_info *ufcs = dev_data;
	int retry_cnt = 50;

	if (!ufcs) {
		ufcs_err("ufcs is null\n");
		return -ENODEV;
	}

	ufcs_err("%s +\n", __func__);
	if (!is_ufcs_enabled(ufcs))
		return -ENODEV;

	while(retry_cnt--) {
		if(ufcs->rx_fifo.state) {
			ufcs_info("rx fifo ready\n");
			return 0;
		} else if (retry_cnt == 10) {
			__hisi_ufcs_interrupt_work(ufcs);
		}
		mdelay(1);
	}

	ufcs_err("rx fifo is NULL\n");
	return -1;
}

static int read_rx_ram(void *dev_data, u8 *data, u8 len)
{
	struct ufcs_driver_info *ufcs = dev_data;
	int ret;
	int i;

	if (!ufcs) {
		ufcs_err("ufcs is null\n");
		return -ENODEV;
	}

	if (len > BUFF_LEN_MAX) {
		ufcs_err("invalid length=%u\n", len);
		return -EINVAL;
	}

	ret = regmap_bulk_read(ufcs->regmap, ufcs->regs.rx_ram_data, data, len);
	for (i = 0; i < len; i++)
		ufcs_info(" 0x%x = 0x%x\n", ufcs->regs.rx_ram_data + i, data[i]);

	return ret;
}

static int hisi_ufcs_end_read_msg(void *dev_data)
{
	struct ufcs_driver_info *ufcs = dev_data;

	if (!ufcs) {
		ufcs_err("ufcs is null\n");
		return -ENODEV;
	}

	return 0;
}

static int hisi_ufcs_soft_reset_master(void *dev_data)
{
	struct ufcs_driver_info *ufcs = dev_data;

	if (!ufcs) {
		ufcs_err("ufcs is null\n");
		return -ENODEV;
	}

	hisi_ufcs_en(ufcs, 0);
	return 0;
}

static int _hisi_ufcs_get_rx_len(void *dev_data, u8 *len)
{
	struct ufcs_driver_info *ufcs = dev_data;
	unsigned int reg_val = 0;
	int ret;

	if (!ufcs) {
		ufcs_err("ufcs is null\n");
		return -ENODEV;
	}

	ret = regmap_bulk_read(ufcs->regmap, ufcs->regs.ufcs_rx_msg_len, &reg_val, 1);
	if (ret) {
		*len = 0;
		return -EINVAL;
	}

	ufcs_debug("reg_val 0x%x\n", reg_val);

	/* Only use 1byte register */
	*len = (reg_val);
	return ret;
}

static int hisi_ufcs_set_communicating_flag(void *dev_data, bool flag)
{
	struct ufcs_driver_info *ufcs = dev_data;

	if (!ufcs) {
		ufcs_err("ufcs is null\n");
		return -ENODEV;
	}

	if (ufcs->communicating_flag && flag) {
		ufcs_info("is communicating, wait\n");
		return -EINVAL;
	}

	ufcs->communicating_flag = flag;
	return 0;
}

static bool hisi_ufcs_need_check_ack(void *dev_data)
{
	return false;
}

static int hisi_ufcs_read_msg(void *dev_data, u8 *data, u8 len)
{
	struct ufcs_driver_info *ufcs = dev_data;

	if (!ufcs) {
		ufcs_err("ufcs is null\n");
		return -ENODEV;
	}
	if (!is_ufcs_enabled(ufcs))
		return -ENODEV;

	if(ufcs->rx_fifo.state) {
		(void)memcpy_s(data, BUFF_LEN_MAX, ufcs->rx_fifo.data, len);
		ufcs->rx_fifo.state = 0;
		ufcs_debug("rx fifo read end\n");
		return 0;
	}

	ufcs_err("rx fifo is null\n");
	return -ENODEV;
}

static int hisi_ufcs_get_rx_len(void *dev_data, u8 *len)
{
	struct ufcs_driver_info *ufcs = dev_data;
	int i = 0;

	if (!ufcs) {
		ufcs_err("ufcs is null\n");
		return -ENODEV;
	}

	if (!is_ufcs_enabled(ufcs))
		return -ENODEV;

	if(ufcs->rx_fifo.state) {
		ufcs_debug("rx fifo [%d] len %d\n", i, ufcs->rx_fifo.len);
		*len = ufcs->rx_fifo.len;
		return 0;
	}

	return -1;
}

static int hisi_ufcs_clear_rx_buff(void *dev_data)
{
	struct ufcs_driver_info *ufcs = dev_data;

	if (!ufcs) {
		ufcs_err("ufcs is null\n");
		return -ENODEV;
	}

	if (!is_ufcs_enabled(ufcs))
		return -ENODEV;

	rx_fifo_init(ufcs);

	return 0;
}

static int hisi_ufcs_hreset_mask(void *dev_data, u8 mask)
{
	struct ufcs_driver_info *ufcs = dev_data;

	if (!ufcs) {
		ufcs_err("ufcs is null\n");
		return -ENODEV;
	}

	if (!is_ufcs_enabled(ufcs))
		return -ENODEV;

	return regmap_update_bits(ufcs->regmap, ufcs->regs.ufcs_ctrl_en_reg_1,
		SC_HRESET_EN_MSK, (!mask) << SC_HRESET_EN_SHIFT);
}

static int hisi_ufcs_hreset_cable(void *dev_data)
{
	struct ufcs_driver_info *ufcs = dev_data;

	if (!ufcs) {
		ufcs_err("ufcs is null\n");
		return -ENODEV;
	}

	if (!is_ufcs_enabled(ufcs))
		return -ENODEV;

	return regmap_update_bits(ufcs->regmap, ufcs->regs.sc_send_cable_rst,
		SC_HREST_CABLE_MSK, 1 << SC_HREST_CABLE_SHIFT);
}

static struct hwufcs_ops ufcs_drv_ops = {
	.chip_name = "hisi_ufcs",
	.detect_adapter = hisi_ufcs_detect_adapter,
	.config_baud_rate = hisi_ufcs_config_baud_rate,
	.write_msg = hisi_ufcs_write_msg,
	.wait_msg_ready = hisi_ufcs_wait_msg_ready,
	.read_msg = hisi_ufcs_read_msg,
	.end_read_msg = hisi_ufcs_end_read_msg,
	.clear_rx_buff = hisi_ufcs_clear_rx_buff,
	.soft_reset_master = hisi_ufcs_soft_reset_master,
	.get_rx_len = hisi_ufcs_get_rx_len,
	.set_communicating_flag = hisi_ufcs_set_communicating_flag,
	.need_check_ack = hisi_ufcs_need_check_ack,
	.hard_reset_mask = hisi_ufcs_hreset_mask,
	.hard_reset_cable = hisi_ufcs_hreset_cable,
};

static int ufcs_get_chip_version(struct ufcs_driver_info *ufcs)
{
	struct device_node *parent_node = NULL;
	int ret;

	parent_node = of_get_parent(ufcs->dev->of_node);
	if (parent_node == NULL) {
		ufcs_err("get parent compatible fail\n");
		return -1;
	}

	ret = of_property_read_u32(parent_node, "chip_version", &(ufcs->chip_version));
	if (ret != 0) {
		ufcs_err("get chip_version fail\n");
		ret = -1;
	}
	of_node_put(parent_node);

	return ret;
}

static int ufcs_regs_init(struct ufcs_driver_info *ufcs)
{
	struct reg_info *regs = &(ufcs->regs);
	unsigned int reg_base;
	int ret;

	ret = ufcs_get_chip_version(ufcs);
	if (ret != 0)
		return -1;

	if (ufcs->chip_version == SCHARGER_VERSION_V600) {
		reg_base = HI6526V510_UFCS_BASE;
		regs->ufcs_tx_msg_len =       reg_base + 0x3F;
		regs->tx_ram_data =           reg_base + 0x40;
		regs->ufcs_rx_msg_len =       reg_base + 0x33F;
		regs->rx_ram_data =           reg_base + 0x340;
		regs->irq_flag =              CHG_IRQ_ADDR;
		regs->irq_vusb_uv_flag =      V510_CHG_IRQ_STATUS6;
		regs->fcp_detect_ctrl =       CHG_FCP_DET_CTRL_REG;
	} else if (ufcs->chip_version == SCHARGER_VERSION_V800) {
		reg_base = HI6526V800_UFCS_BASE;
		regs->ufcs_tx_msg_len =       reg_base + 0x3A;
		regs->tx_ram_data =           reg_base + 0x3D;
		regs->ufcs_rx_msg_len =       reg_base + 0x33A;
		regs->rx_ram_data =           reg_base + 0x33D;
		regs->irq_flag =              CHG_IRQ_FLAG_REG;
		regs->irq_vusb_uv_flag =      CHG_IRQ_FLAG_0_REG;
		regs->fcp_detect_ctrl =       FCP_FCP_CTRL_REG;
	} else {
		ufcs_err("%s invaild chip version\n",__func__);
		return -1;
	}

	regs->sc_ufcs_en =            reg_base + 0x00;
	regs->sc_hk_det_cnt_cfg =     reg_base + 0x02;
	regs->sc_m0_baud_delta =      reg_base + 0x03;
	regs->sc_ufcs_hw_rst =        reg_base + 0x11;
	regs->sc_wd_clr =             reg_base + 0x12;
	regs->sc_send_start =         reg_base + 0x14;
	regs->sc_send_cable_rst =     reg_base + 0x15;
	regs->sc_send_rst =           reg_base + 0x16;
	regs->sc_msg_anck_tmpl_dat0 = reg_base + 0x17;
	regs->sc_msg_anck_tmpl_dat1 = reg_base + 0x18;
	regs->ufcs_ctrl_en_reg_1 =    reg_base + 0x1B;
	regs->sc_frame_idle_cnt_cfg = reg_base + 0x1E;
	regs->sc_baud_cfg =           reg_base + 0x1F;
	regs->sc_ufcs_soft_rst_n =    reg_base + 0x20;
	regs->sc_read_rx_req =        reg_base + 0x21;
	regs->ufcs_rx_status_ro =     reg_base + 0x26;
	regs->ufcs_irq_flg_0 =        reg_base + 0x27;
	regs->ufcs_irq_flg_1 =        reg_base + 0x28;
	regs->ufcs_irq_mask_0 =       reg_base + 0x29;
	regs->ufcs_irq_mask_1 =       reg_base + 0x2A;
	regs->ufcs_pkg_type =         reg_base + 0x2E;

	return 0;
}

static void irq_ufcs_hw_rst_handle(struct ufcs_driver_info *ufcs)
{
	hisi_ufcs_en(ufcs, 0);
}

static void send_hw_rst(struct ufcs_driver_info *ufcs)
{
	regmap_write(ufcs->regmap, ufcs->regs.sc_send_rst, 1);
}

static void irq_ufcs_wd_tmr_handle(struct ufcs_driver_info *ufcs)
{
	send_hw_rst(ufcs);
	hisi_ufcs_wait(ufcs, HWUFCS_WAIT_SEND_PACKET_COMPLETE);
	hisi_ufcs_en(ufcs, 0);
}

static bool is_pkg_crc_err(unsigned int pkg_stats)
{
	if (pkg_stats & BIT(6))
		return true;
	return false;
}

static bool hisi_ufcs_check_received_msg_vaild(u8 *data,u8 len,unsigned int pkg_stats)
{
	if (len < MIN_MSG_LEN) {
		ufcs_err("%s err len, len = %d \n",__func__,len);
		return false;
	}

	if (is_pkg_crc_err(pkg_stats)) {
		ufcs_err("%s err crc ,pkg_stats = 0x%x\n",__func__,pkg_stats);
		return false;
	}

	ufcs_debug("sink addr = 0x%x\n",data[0]);
	/* only message recipient is a sink device, return true */
	if ((data[0] & SINK_ADDR_MASK) == BIT(6))
		return true;

	return false;
}

static void __hisi_ufcs_interrupt_work(struct ufcs_driver_info *ufcs)
{
	unsigned int val = 0;
	unsigned int cache_val = 0;
	unsigned int pkg_stats = 0;
	u8 len = 0;
	int retry_cnt = 10;

	mutex_lock(&ufcs->ufcs_worker_lock);

	regmap_read(ufcs->regmap, ufcs->regs.irq_flag, &val);
	if((val & IS_UFCS_IRQ) == 0) {
		mutex_unlock(&ufcs->ufcs_worker_lock);
		return;
	}

	regmap_read(ufcs->regmap, ufcs->regs.ufcs_irq_flg_0, &val);
	ufcs_info("ufcs_irq_flg_0 0x%x\n", val);

	if(val & IRQ_UFCS_RX_END) {
		hisi_ufcs_rx_read_request(ufcs, 1);

		while(retry_cnt--) {
			regmap_read(ufcs->regmap, ufcs->regs.ufcs_rx_status_ro, &cache_val);
			ufcs_err("UFCS_RX_STATUS_RO 0x%x\n", cache_val);
			if(cache_val & RO_UFCS_RX_READ_ACK) {
				break;
			}
		}

		_hisi_ufcs_get_rx_len(ufcs, &len);
		read_rx_ram(ufcs, ufcs->rx_fifo.data, len);
		regmap_read(ufcs->regmap, ufcs->regs.ufcs_pkg_type, &pkg_stats);

		ufcs->rx_fifo.len = len;
		hisi_ufcs_rx_read_request(ufcs, 0);
	}

	if (val & IRQ_UFCS_HW_RST) {
		irq_ufcs_hw_rst_handle(ufcs);
		ufcs_err("IRQ_UFCS_HW_RST\n");
	}

	if (val & IRQ_UFCS_WD_TMR) {
		irq_ufcs_wd_tmr_handle(ufcs);
		ufcs_err("IRQ_UFCS_WD_TMR\n");
	}

	/* clear irq */
	regmap_write(ufcs->regmap, ufcs->regs.ufcs_irq_flg_0,
		(IRQ_UFCS_RX_END | IRQ_UFCS_HW_RST | IRQ_UFCS_WD_TMR));

	if (!hisi_ufcs_check_received_msg_vaild(ufcs->rx_fifo.data, len, pkg_stats)) {
		mutex_unlock(&ufcs->ufcs_worker_lock);
		return;
	}

	ufcs->rx_fifo.state = 1;

	if (!ufcs->communicating_flag)
		power_event_bnc_notify(POWER_BNT_UFCS,
			POWER_NE_UFCS_REC_UNSOLICITED_DATA, NULL);
	mutex_unlock(&ufcs->ufcs_worker_lock);
}

static void hisi_ufcs_interrupt_work(struct work_struct *work)
{
	struct ufcs_driver_info *ufcs = NULL;

	ufcs = container_of(work, struct ufcs_driver_info, irq_work);
	if (!ufcs ) {
		ufcs_err("di is null\n");
		return;
	}

	__hisi_ufcs_interrupt_work(ufcs);
}

static irqreturn_t hisi_ufcs_intr_handler(int irq, void *data)
{
	struct ufcs_driver_info *ufcs = data;
	queue_work(system_highpri_unbound_wq, &ufcs->irq_work);
	return IRQ_NONE; // IRQ_HANDLED;
}

unsigned int ufcs_test_read(unsigned int reg)
{
	struct ufcs_driver_info *ufcs = ufcs_drv_ops.dev_data;
	unsigned int val = 0;

	if (!ufcs ) {
		ufcs_err("%s di is null\n",__func__);
		return EPERM;
	}

	regmap_read(ufcs->regmap, reg, &val);
	return val;
}

unsigned int ufcs_test_write(unsigned int reg, unsigned int val)
{
	struct ufcs_driver_info *ufcs = ufcs_drv_ops.dev_data;
	if (!ufcs ) {
		ufcs_err("%s di is null\n",__func__);
		return EPERM;
	}

	regmap_write(ufcs->regmap, reg, val);
	regmap_read(ufcs->regmap, reg, &val);
	return val;
}

static int ufcs_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct ufcs_driver_info *ufcs = NULL;
	int ret;
	enum of_gpio_flags flags;
	u32 irq_flag = IRQF_SHARED | IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND | IRQF_NO_THREAD;

	ufcs = devm_kzalloc(dev, sizeof(*ufcs), GFP_KERNEL);
	if (!ufcs) {
		ufcs_err("[%s]ufcs is null.\n", __func__);
		return -ENOMEM;
	}

	mutex_init(&ufcs->ufcs_lock);
	mutex_init(&ufcs->ufcs_worker_lock);
	ufcs->dev = dev;
	ufcs->regmap = dev_get_regmap(dev->parent, NULL);
	if (!ufcs->regmap) {
		ufcs_err("Parent regmap unavailable.\n");
		return -ENXIO;
	}
	ret = ufcs_regs_init(ufcs);
	if (ret != 0) {
		ufcs_err("ufcs regs init fail\n");
		return -EINVAL;
	}

	INIT_WORK(&ufcs->irq_work, hisi_ufcs_interrupt_work);
	ufcs->irq_gpio = of_get_gpio_flags(dev->of_node, 0, &flags);
	if (ufcs->irq_gpio < 0) {
		ufcs_err("failed to get_gpio_flags:%d\n", ufcs->irq_gpio);
		return -EIO;
	}
	ufcs->irq = gpio_to_irq(ufcs->irq_gpio);
	ret = request_irq(ufcs->irq, hisi_ufcs_intr_handler, irq_flag,
		"hisi_ufcs", ufcs);

	ufcs_err("[%s]gpio %d, irq %d\n", __func__, ufcs->irq_gpio, ufcs->irq);

	ufcs_drv_ops.dev_data = (void *)ufcs;
	ret = hwufcs_ops_register(&ufcs_drv_ops);

	ufcs_info("[%s] ret %d -\n", __func__, ret);
	return ret;
}

static int ufcs_remove(struct platform_device *pdev)
{
	return 0;
}

static struct of_device_id ufcs_match_table[] = {
	{
		.compatible = "hisilicon,ufcs-driver",
		.data = NULL,
	},
	{},
};

static struct platform_driver ufcs_driver = {
	.probe = ufcs_probe,
	.remove = ufcs_remove,
	.driver =
		{
			.name = "huawei,ufcs",
			.owner = THIS_MODULE,
			.of_match_table = of_match_ptr(ufcs_match_table),
		},
};

static int __init ufcs_init(void)
{
	return platform_driver_register(&ufcs_driver);
}

static void __exit ufcs_exit(void)
{
	platform_driver_unregister(&ufcs_driver);
}

device_initcall_sync(ufcs_init);
module_exit(ufcs_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ufcs module driver");
