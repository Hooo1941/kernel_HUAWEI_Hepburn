/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/delay.h>

#include "dkmd_log.h"
#include "hdmitx_ctrl_reg.h"
#include "hdmitx_aon_reg.h"
#include "hdmitx_ctrl.h"
#include "hdmitx_ddc_config.h"
#include "hdmitx_dbg.h"
#include "hdmitx_link_training_reg.h"

#define TRANSFER_START     0
#define TRANSFER_SUCCESS   1
#define WAIT_BUS_IDLE_FAILED 2
#define WAIT_SCL_HIGH_FAILED 3
#define WAIT_SDA_HIGH_FAILED 4
#define WAIT_RFIFO_FAILED    5
#define WAIT_WFIFO_FAILED    6
#define LOCK_HW_BUS_FAILED   7
#define UNLOCK_HW_BUS_FAILED 8
#define BUS_LOW_ERROR        9
#define SLAVE_NO_ACK         10
#define WAIT_PROG_TIMEOUT    11
#define TRANSFER_FAILED      12

#define FRL_REQ_DDC_STATUS_MASK   0x800
#define TRAINING_REG_SUB_OFFSET   0x38
#define TRAINING_REG_OFFSET       0x20000

static inline u32 ddc_fifo_count(const struct hdmitx_ddc *ddc)
{
	u32 value;
	if (ddc == NULL || ddc->hdmi_regs == NULL)
		return 0;

	value = hdmi_readl(ddc->hdmi_regs, REG_PWD_DATA_CNT);
	value = (value & REG_PWD_FIFO_DATA_CNT_M);

	return value;
}

static bool ddc_rfifo_is_empty(const struct hdmitx_ddc *ddc)
{
	u32 value;
	/*
	 * Check the fifo empty status bit first, and than check the fifo
	 * count as a double check.
	 */
	value = hdmi_readl(ddc->hdmi_regs, REG_PWD_MST_STATE);
	value = (value & REG_PWD_FIFO_EMPTY_M) >> 5; /* bit5: pwd_fifo_empty */
	if (value == 0x1)
		return true;

	value = ddc_fifo_count(ddc);
	if (value == 0)
		return true;

	return false;
}

static bool ddc_fifo_is_full(const struct hdmitx_ddc *ddc)
{
	u32 value;

	value = ddc_fifo_count(ddc);

	return value >= DDC_MAX_FIFO_SIZE;
}

static bool ddc_bus_is_err(const struct hdmitx_ddc *ddc)
{
	u32 value;

	/*
	 * If the slave response with no ack, or bus is busy,
	 * then we abort the transfer.
	 * for our usecase, when we are connected to a tv and the bus is busy
	 * indicates an abnornal case.
	 */
	value = hdmi_readl(ddc->hdmi_aon_regs, REG_DDC_MST_STATE);
	value &= REG_DDC_I2C_NO_ACK_M;
	if (value == 0x1) {
		dpu_pr_err("I2C_NO_ACK");
		goto err_clear;
	}

	value = hdmi_readl(ddc->hdmi_aon_regs, REG_DDC_MST_STATE);
	value = (value & REG_DDC_I2C_BUS_LOW_M) >> 1;
	if (value == 0x1) {
		dpu_pr_err("I2C_BUS_LOW");
		goto err_clear;
	}

	return false;
err_clear:
	hdmi_clrset(ddc->hdmi_regs, REG_PWD_MST_CMD,
				REG_PWD_MST_CMD_M, reg_pwd_mst_cmd_f(CMD_MASTER_ABORT));
	return true;
}

static inline u32 ddc_sda_level_get(const struct hdmitx_ddc *ddc)
{
	u32 value;
	if (ddc == NULL || ddc->hdmi_aon_regs == NULL)
		return 0;

	value = hdmi_readl(ddc->hdmi_aon_regs, REG_DDC_MAN_CTRL);
	value = (value & REG_DDC_SDA_ST_M) >> 1;
	return value;
}

static inline u32 ddc_scl_level_get(const struct hdmitx_ddc *ddc)
{
	u32 value;
	if (ddc == NULL || ddc->hdmi_aon_regs == NULL)
		return 0;

	value = hdmi_readl(ddc->hdmi_aon_regs, REG_DDC_MAN_CTRL);
	value = (value & REG_DDC_SCL_ST_M);

	return value;
}

/*
 * scl should not be high in general, it's very abnornal case,
 * we can only wait some timeout for scl high.
 * just return an error if scl is not high after a wait.
 */
static s32 ddc_scl_wait_high(const struct hdmitx_ddc *ddc)
{
	s32 ret = 0;
	u32 temp_time = 0;

	while (!ddc_scl_level_get(ddc)) {
		usleep_range(1000, 1100); /* need sleep 1ms. */
		temp_time += 1;
		if (temp_time > ddc->timeout.scl_timeout) {
			ret = -1;
			break;
		}
	}

	return ret;
}

/*
 * If the sda is low when we are expecting idle, it's mostly possible
 * deadlock due to master/slave reset during i2c transfer.
 * And we need to recover from this case(refer to i2c deadlock for more)
 * Simulate i2c transfer is used.
 */
static s32 ddc_try_resolve_deadlock(const struct hdmitx_ddc *ddc)
{
	s32 ret;
	u32 temp_time = 0;

	hdmi_clrset(ddc->hdmi_aon_regs, REG_DDC_MST_CTRL, REG_DCC_MAN_EN_M, reg_dcc_man_en(1));
	/* Generate scl clock util sda is high or timeout */
	while ((!ddc_sda_level_get(ddc)) && (temp_time++ < ddc->timeout.sda_timeout)) {
		/* pull scl high */
		hdmi_clrset(ddc->hdmi_aon_regs, REG_DDC_MAN_CTRL, REG_DDC_SCL_OEN_M, reg_ddc_scl_oen(1));
		udelay(8); /* delay 8 microsecond */
		/* pull scl low */
		hdmi_clrset(ddc->hdmi_aon_regs, REG_DDC_MAN_CTRL, REG_DDC_SCL_OEN_M, reg_ddc_scl_oen(0));
		udelay(8); /* delay 8 microsecond */
	}
	/* STOP contition */
	if (temp_time < ddc->timeout.sda_timeout && ddc_sda_level_get(ddc)) {
		/* pull sda low */
		hdmi_clrset(ddc->hdmi_aon_regs, REG_DDC_MAN_CTRL, REG_DDC_SDA_OEN_M, reg_ddc_sda_oen(0));
		udelay(8); /* delay 8 microsecond */
		/* pull scl high */
		hdmi_clrset(ddc->hdmi_aon_regs, REG_DDC_MAN_CTRL, REG_DDC_SCL_OEN_M, reg_ddc_scl_oen(1));
		udelay(8); /* delay 8 microsecond */
		/* pull sda high */
		hdmi_clrset(ddc->hdmi_aon_regs, REG_DDC_MAN_CTRL, REG_DDC_SDA_OEN_M, reg_ddc_sda_oen(1));
		udelay(8); /* delay 8 microsecond */

		dpu_pr_info("ddc deadlock clear success");
		ret = 0;
	} else {
		dpu_pr_err("ddc deadlock clear fail");
		ret = -1;
	}
	hdmi_clrset(ddc->hdmi_aon_regs, REG_DDC_MST_CTRL, REG_DCC_MAN_EN_M, reg_dcc_man_en(0));

	return ret;
}

static s32 ddc_sda_wait_high(const struct hdmitx_ddc *ddc)
{
	if (ddc_sda_level_get(ddc)) {
		dpu_pr_debug("sda get ok");
		return 0;
	}

	return ddc_try_resolve_deadlock(ddc);
}

static s32 ddc_wait_for_bus_idle(const struct hdmitx_ddc *ddc)
{
	s32 ret;

	ret = ddc_scl_wait_high(ddc);
	if (ret < 0) {
		dpu_pr_err("wait scl high fail");
		return ret;
	}

	ret = ddc_sda_wait_high(ddc);
	if (ret < 0)
		return ret;

	return ret;
}

static s32 ddc_wait_for_rfifo_ready(const struct hdmitx_ddc *ddc)
{
	u32 rd_tmo;
	u32 i;

	rd_tmo = ddc->timeout.access_timeout;

	for (i = 0; i < rd_tmo; i++) {
		if (ddc_rfifo_is_empty(ddc)) {
			usleep_range(1000, 1100); /* need sleep 1ms. */
			if (ddc_bus_is_err(ddc))
				return -1;
		} else {
			break;
		}
	}

	if (i >= rd_tmo) {
		dpu_pr_err("ddc read fifo timeout=%u", rd_tmo);
		return -1;
	}

	return 0;
}

static s32 ddc_wait_for_wfifo_ready(const struct hdmitx_ddc *ddc)
{
	u32 wr_tmo;
	u32 i;

	wr_tmo = ddc->timeout.access_timeout;

	for (i = 0; i < wr_tmo; i++) {
		if (ddc_fifo_is_full(ddc)) {
			usleep_range(1000, 1100); /* need sleep 1ms. */
			if (ddc_bus_is_err(ddc))
				return -1;
		} else {
			break;
		}
	}

	if (i >= wr_tmo) {
		dpu_pr_debug("ddc write fifo timeout = %u", wr_tmo);
		return -1;
	}

	return 0;
}

/*
 * Software can only see the read/write fifo of the ddc controller,
 * but sometimes it's nessary to make sure the bus transaction is finished.
 * especially when the ddc bus share by mutiple masters.
 * with ddc_wait_for_prog_complete, we can make sure that the bus transaction
 * is finished.
 */
static s32 ddc_wait_for_prog_complete(const struct hdmitx_ddc *ddc)
{
	u32 prog_tmo, value;
	u32 i = 0;

	prog_tmo = ddc->timeout.in_prog_timeout;

	while (i < prog_tmo) {
		value = hdmi_readl(ddc->hdmi_regs, REG_PWD_MST_STATE);
		if (!(value & REG_PWD_I2C_IN_PROG_M))
			break;

		i++;
		msleep(1); /* need sleep 1ms. */
	}

	if (i >= prog_tmo) {
		dpu_pr_err("wait prog finish timeout = %u", prog_tmo);
		return -1;
	}

	return 0;
}

static s32 ddc_lock_unlock_helper(const struct hdmitx_ddc *ddc, bool lock)
{
	u32 status;
	u32 arb_status;
	u32 frl_ddc_req;
	u32 arb_req_value;
	u32 aon_status;
	u32 pwd_status;
	u32 count = 0;
	u32 op = lock ? DDC_MASTER_ENABLE : DDC_MASTER_DISABLE;
	u32 tmo = ddc->timeout.access_timeout;

	// clear mcu req
	arb_status = hdmi_readl(ddc->hdmi_regs, REG_DDC_MST_ARB_STATE);
	frl_ddc_req = hdmi_readl(ddc->hdmi_regs + TRAINING_REG_OFFSET, REG_MCU_DDC_REQ - TRAINING_REG_SUB_OFFSET);
	if (arb_status & FRL_REQ_DDC_STATUS_MASK)
		dpu_pr_info("frl request ddc, arb_status = 0x%x, frl_ddc_req = 0x%x", arb_status, frl_ddc_req);

	if (frl_ddc_req & REG_MCU2DDC_REQ_M)
		hdmi_clrset(ddc->hdmi_regs+ TRAINING_REG_OFFSET, REG_MCU_DDC_REQ- TRAINING_REG_SUB_OFFSET,
					REG_MCU2DDC_REQ_M, reg_mcu2ddc_req(0));

	hdmi_clrset(ddc->hdmi_regs, REG_DDC_MST_ARB_REQ, REG_CPU_DDC_REQ_M, reg_cpu_ddc_req(op));

	while (1) {
		status = hdmi_readl(ddc->hdmi_regs, REG_DDC_MST_ARB_ACK);
		if (lock && (status & REG_CPU_DDC_REQ_ACK_M))
			break;
		else if (!lock && !(status & REG_CPU_DDC_REQ_ACK_M))
			break;

		msleep(1); /* need sleep 1ms. */
		count++;
		if (count > tmo)
			return -1;
	}

	return 0;
}

static s32 ddc_lock_internel_hw_bus(const struct hdmitx_ddc *ddc)
{
	return ddc_lock_unlock_helper(ddc, true);
}

static s32 ddc_unlock_internal_hw_bus(const struct hdmitx_ddc *ddc)
{
	return ddc_lock_unlock_helper(ddc, false);
}

/*
 * In some cases, there are something error happens.
 * we need to do cleanup to clear the error status.
 * currently, the following 3 cleanups are done:
 * 1.wait for i2c program finish.
 * 2.wait for the bus idle and resolve the i2c deadlock if it's needed
 * and possible.
 */
static void ddc_errs_cleanup(const struct hdmitx_ddc *ddc)
{
	s32 ret;
	dpu_pr_debug("+");
	/*
	 * No more error handling is needed since we are already done
	 * everything, we have to ignore.
	 */
	ret = ddc_wait_for_prog_complete(ddc);
	if (ret != 0)
		dpu_pr_info("wait prog finish timeout");

	ret = ddc_wait_for_bus_idle(ddc);
	if (ret < 0)
		dpu_pr_info("ddc bus not idle");
	dpu_pr_debug("-");
}

static void ddc_transfer_configure(const struct hdmitx_ddc *ddc, u16 addr, u16 len, bool read)
{
	u32 mode;

	if (ddc->hdmi_regs == NULL)
		return;

	if (read)
		mode = ddc->is_segment ? MODE_READ_SEGMENT_NO_ACK : MODE_READ_MUTIL_NO_ACK;
	else
		mode = MODE_WRITE_MUTIL_NO_ACK;

	/* Clear DDC FIFO, it's recommanded */
	hdmi_clrset(ddc->hdmi_regs, REG_PWD_MST_CMD, REG_PWD_MST_CMD_M, reg_pwd_mst_cmd_f(CMD_FIFO_CLR));
	/* Set Slave Address */
	hdmi_clrset(ddc->hdmi_regs, REG_PWD_SLAVE_CFG, REG_PWD_SLAVE_ADDR_M, reg_pwd_slave_addr(addr));
	/* Set Slave Offset */
	hdmi_clrset(ddc->hdmi_regs, REG_PWD_SLAVE_CFG, REG_PWD_SLAVE_OFFSET_M, reg_pwd_slave_offset(ddc->slave_reg));
	/* Set Slave Segment */
	hdmi_clrset(ddc->hdmi_regs, REG_PWD_SLAVE_CFG, REG_PWD_SLAVE_SEG_M, reg_pwd_slave_seg((u8)ddc->is_segment));
	/* Set DDC FIFO Data Cnt */
	hdmi_clrset(ddc->hdmi_regs, REG_PWD_DATA_CNT, REG_PWD_DATA_OUT_CNT_M, reg_pwd_data_out_cnt(len));
	/* Set DDC Master register read mode */
	hdmi_clrset(ddc->hdmi_regs, REG_PWD_MST_CMD, REG_PWD_MST_CMD_M, reg_pwd_mst_cmd_f((u32)mode));
}

static s32 ddc_data_read(struct hdmitx_ddc *ddc, const struct ddc_msg *msg)
{
	s32 i, ret;
	u16 length = msg->len;
	u8 *buf = msg->buf;
	u16 addr = msg->addr;

	/* Default regaddr is 0 */
	if (ddc->is_regaddr == false) {
		ddc->slave_reg = 0x00;
		ddc->is_regaddr = true;
	}

	ddc_transfer_configure(ddc, addr, length, true);
	/* DDC read fifo data */
	for (i = 0; i < length; i++, buf++) {
		ret = ddc_wait_for_rfifo_ready(ddc);
		if (ret)
			return ret;

		*buf = hdmi_readl(ddc->hdmi_regs, REG_PWD_FIFO_RDATA) & REG_PWD_FIFO_DATA_OUT_M;
		/*
		 * The hw fifo status is not updated in time after a fifo
		 * read/write, so we have to wait 5us to let the hw refresh
		 * for the next fifo status
		 */
		udelay(5); /* delay 5 microsecond */
	}

	ddc->is_segment = false;
	/*
	 * It's not that nessary to wait for a read complete, since there is
	 * no data on the way when the read fifo finished.
	 * but we still double check it for harden.
	 */
	ret = ddc_wait_for_prog_complete(ddc);

	return ret;
}

static s32 ddc_data_write(struct hdmitx_ddc *ddc, const struct ddc_msg *msgs)
{
	s32 i, ret;
	u16 length;
	u8 *buf = NULL;
	u16 addr;

	length = msgs->len;
	buf = msgs->buf;
	addr = msgs->addr;

	if (!ddc->is_regaddr) {
		/* Use the first write byte as offset */
		ddc->slave_reg = buf[0];
		length--;
		buf++;
		ddc->is_regaddr = true;
	}

	if (length == 0)
		return 0;

	ddc_transfer_configure(ddc, addr, length, false);

	/* DDC fifo write data */
	for (i = 0; i < length; i++, buf++) {
		ret = ddc_wait_for_wfifo_ready(ddc);
		if (ret)
			return ret;

		hdmi_clrset(ddc->hdmi_regs, REG_PWD_FIFO_WDATA, REG_PWD_FIFO_DATA_IN_M, reg_pwd_fifo_data_in(*buf));
		/*
		 * The hw fifo status is not updated in time after a fifo
		 * read/write, so we have to wait 5us to let the hw refresh
		 * for the next fifo status
		 */
		udelay(5); /* delay 5 microsecond */
	}

	/*
	 * It's mandantory to wait for a write complete, since there maybe
	 * still some data on the way when the write fifo is finished.
	 * if we return earlier, the fifo could be wrongly cleared by the next
	 * transfer.
	 */
	ret = ddc_wait_for_prog_complete(ddc);

	return ret;
}

s32 ddc_xfer(struct hdmitx_ddc *ddc, const struct ddc_msg *msgs, u8 num)
{
	s32 i;
	s32 ret;

	if (ddc == NULL || msgs == NULL) {
		dpu_pr_err("Null ptr err");
		return -1;
	}
	dpu_pr_debug("+");
	mutex_lock(&ddc->lock);

	/*
	 * Lock the DDC hardware bus
	 * The DDC hardware bus is shared by cpu/mcu/hdcp, so it's required to lock the
	 * bus before access
	 */
	ret = ddc_lock_internel_hw_bus(ddc);
	if (ret) {
		dpu_pr_err("ddc lock bus err");
		goto hw_lock_failed;
	}
	/* wait the bus idle and resolve the i2c deadlock if possible */
	ret = ddc_wait_for_bus_idle(ddc);
	if (ret) {
		dpu_pr_err("ddc bus not idle");
		goto out;
	}
	/* Set slave device register address on transfer */
	ddc->is_regaddr = false;
	/* Set segment pointer for I2C extended read mode operation */
	ddc->is_segment = false;

	for (i = 0; i < num; i++) {
		if (msgs[i].addr == DDC_SEGMENT_ADDR && msgs[i].len == 1) {
			ddc->is_segment = true;
			continue;
		}

		if (msgs[i].flags & DDC_M_RD)
			ret = ddc_data_read(ddc, &msgs[i]);
		else
			ret = ddc_data_write(ddc, &msgs[i]);

		if (ret < 0)
			break;
	}

	if (ret)
		 ddc_errs_cleanup(ddc);

out:
	/* Just print the error but no way to handle unlock error */
	if (ddc_unlock_internal_hw_bus(ddc) != 0)
		dpu_pr_err("ddc unlock bus err");
hw_lock_failed:
	mutex_unlock(&ddc->lock);
	dpu_pr_debug("-");
	return ret;
}
