// SPDX-License-Identifier: GPL-2.0
/*
 * cps2023_scp.c
 *
 * cps2023 scp driver
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

#include "cps2023_scp.h"
#include "cps2023_ufcs.h"
#include "cps2023_i2c.h"
#include <linux/delay.h>
#include <linux/mutex.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_device_id.h>
#include <chipset_common/hwpower/protocol/adapter_protocol.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_scp.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_fcp.h>

#define HWLOG_TAG cps2023_scp
HWLOG_REGIST();

static u32 g_cps2023_scp_error_flag;

static int cps2023_scp_wdt_reset_by_sw(struct cps2023_device_info *di)
{
	return cps2023_write_mask(di, CPS2023_UFCS_CTL1_REG,
		CPS2023_UFCS_CTL1_EN_PROTOCOL_MASK,
		CPS2023_UFCS_CTL1_EN_PROTOCOL_SHIFT, 0);
}

static int cps2023_scp_cmd_transfer_check(struct cps2023_device_info *di)
{
	u8 reg_val1 = 0;
	u8 reg_val2 = 0;
	int i = 0;
	int ret;

	do {
		(void)power_msleep(DT_MSLEEP_50MS, 0, NULL);
		ret = cps2023_read_byte(di, CPS2023_SCP_ISR1_REG, &reg_val1);
		ret += cps2023_read_byte(di, CPS2023_SCP_ISR2_REG, &reg_val2);
		if (ret) {
			hwlog_err("reg read fail\n");
			break;
		}
		hwlog_info("reg_val1=0x%x, reg_val2=0x%x, scp_isr_backup[0]=0x%x, scp_isr_backup[1]=0x%x\n",
			reg_val1, reg_val2, di->scp_isr_backup[0], di->scp_isr_backup[1]);
		/* interrupt work can hook the interrupt value first. so it is necessily to do backup via isr_backup */
		reg_val1 |= di->scp_isr_backup[0];
		reg_val2 |= di->scp_isr_backup[1];
		if (reg_val1 || reg_val2) {
			if (((reg_val2 & CPS2023_SCP_ISR2_ACK_MASK) &&
				(reg_val2 & CPS2023_SCP_ISR2_CMD_CPL_MASK)) &&
				!(reg_val1 & (CPS2023_SCP_ISR1_ACK_CRCRX_MASK |
				CPS2023_SCP_ISR1_ACK_PARRX_MASK |
				CPS2023_SCP_ISR1_ERR_ACK_L_MASK))) {
				return 0;
			} else if (reg_val1 & (CPS2023_SCP_ISR1_ACK_CRCRX_MASK |
				CPS2023_SCP_ISR1_ENABLE_HAND_NO_RESPOND_MASK)) {
				hwlog_err("scp transfer fail, slave status changed: ISR1=0x%x, ISR2=0x%x\n",
					reg_val1, reg_val2);
				return -EPERM;
			} else if (reg_val2 & CPS2023_SCP_ISR2_NACK_MASK) {
				hwlog_err("scp transfer fail, slave nack: ISR1=0x%x, ISR2=0x%x\n",
					reg_val1, reg_val2);
				return -EPERM;
			} else if (reg_val1 & (CPS2023_SCP_ISR1_ACK_CRCRX_MASK |
				CPS2023_SCP_ISR1_ACK_PARRX_MASK |
				CPS2023_SCP_ISR1_TRANS_HAND_NO_RESPOND_MASK)) {
				hwlog_err("scp transfer fail, CRCRX_PARRX_ERROR: ISR1=0x%x, ISR2=0x%x\n",
					reg_val1, reg_val2);
				return -EPERM;
			}
			hwlog_err("scp transfer fail, ISR1=0x%x, ISR2=0x%x, index=%d\n",
				reg_val1, reg_val2, i);
		}
		i++;
	} while (i < CPS2023_SCP_ACK_RETRY_CYCLE);

	hwlog_err("scp adapter transfer time out\n");
	return -EPERM;
}

static int cps2023_scp_adapter_reg_read(u8 *val, u8 reg, struct cps2023_device_info *di)
{
	int ret;
	int i;
	u8 reg_val1 = 0;
	u8 reg_val2 = 0;

	mutex_lock(&di->accp_adapter_reg_lock);
	hwlog_info("CMD=0x%x, REG=0x%x\n", CPS2023_SCP_CMD_SBRRD, reg);
	for (i = 0; i < CPS2023_SCP_RETRY_TIME; i++) {
		/* init */
		cps2023_write_mask(di, CPS2023_SCP_CTL_REG, CPS2023_SCP_CTL_SNDCMD_MASK,
			CPS2023_SCP_CTL_SNDCMD_SHIFT, CPS2023_SCP_CTL_SNDCMD_RESET);

		/* before send cmd, clear isr interrupt registers */
		ret = cps2023_read_byte(di, CPS2023_SCP_ISR1_REG, &reg_val1);
		ret += cps2023_read_byte(di, CPS2023_SCP_ISR2_REG, &reg_val2);
		ret += cps2023_write_byte(di, CPS2023_RT_BUFFER_0_REG, CPS2023_SCP_CMD_SBRRD);
		ret += cps2023_write_byte(di, CPS2023_RT_BUFFER_1_REG, reg);
		ret += cps2023_write_byte(di, CPS2023_RT_BUFFER_2_REG, 1);
		/* initial scp_isr_backup[0],[1] due to catching the missing isr by interrupt_work */
		di->scp_isr_backup[0] = 0;
		di->scp_isr_backup[1] = 0;
		ret += cps2023_write_mask(di, CPS2023_SCP_CTL_REG, CPS2023_SCP_CTL_SNDCMD_MASK,
			CPS2023_SCP_CTL_SNDCMD_SHIFT, CPS2023_SCP_CTL_SNDCMD_START);
		if (ret) {
			hwlog_err("write error, ret is %d\n", ret);
			/* manual init */
			cps2023_write_mask(di, CPS2023_SCP_CTL_REG, CPS2023_SCP_CTL_SNDCMD_MASK,
				CPS2023_SCP_CTL_SNDCMD_SHIFT, CPS2023_SCP_CTL_SNDCMD_RESET);
			mutex_unlock(&di->accp_adapter_reg_lock);
			return -EPERM;
		}

		/* check cmd transfer success or fail */
		if (cps2023_scp_cmd_transfer_check(di) == 0) {
			/* recived data from adapter */
			ret = cps2023_read_byte(di, CPS2023_RT_BUFFER_12_REG, val);
			break;
		}
	}
	if (i >= CPS2023_SCP_RETRY_TIME) {
		hwlog_err("ack error, retry %d times\n", i);
		ret = -1;
	}
	/* manual init */
	cps2023_write_mask(di, CPS2023_SCP_CTL_REG, CPS2023_SCP_CTL_SNDCMD_MASK,
		CPS2023_SCP_CTL_SNDCMD_SHIFT, CPS2023_SCP_CTL_SNDCMD_RESET);
	power_usleep(DT_USLEEP_10MS); /* wait 10ms for operate effective */

	mutex_unlock(&di->accp_adapter_reg_lock);
	return ret;
}

static int cps2023_scp_init_reg_read_block(struct cps2023_device_info *di, u8 reg, u8 len)
{
	u8 reg_val1 = 0;
	u8 reg_val2 = 0;
	int ret;

	cps2023_write_mask(di, CPS2023_SCP_CTL_REG, CPS2023_SCP_CTL_SNDCMD_MASK,
			CPS2023_SCP_CTL_SNDCMD_SHIFT, CPS2023_SCP_CTL_SNDCMD_RESET);

	/* before sending cmd, clear isr registers */
	ret = cps2023_read_byte(di, CPS2023_SCP_ISR1_REG, &reg_val1);
	ret += cps2023_read_byte(di, CPS2023_SCP_ISR2_REG, &reg_val2);
	ret += cps2023_write_byte(di, CPS2023_RT_BUFFER_0_REG, CPS2023_SCP_CMD_MBRRD);
	ret += cps2023_write_byte(di, CPS2023_RT_BUFFER_1_REG, reg);
	ret += cps2023_write_byte(di, CPS2023_RT_BUFFER_2_REG, len);
	/* initial scp_isr_backup[0],[1] */
	di->scp_isr_backup[0] = 0;
	di->scp_isr_backup[1] = 0;
	/* start transaction */
	ret += cps2023_write_mask(di, CPS2023_SCP_CTL_REG, CPS2023_SCP_CTL_SNDCMD_MASK,
			CPS2023_SCP_CTL_SNDCMD_SHIFT, CPS2023_SCP_CTL_SNDCMD_START);
	if (ret) {
		hwlog_err("read error ret is %d\n", ret);
			/* manual init */
		cps2023_write_mask(di, CPS2023_SCP_CTL_REG, CPS2023_SCP_CTL_SNDCMD_MASK,
				CPS2023_SCP_CTL_SNDCMD_SHIFT, CPS2023_SCP_CTL_SNDCMD_RESET);
	}
	return ret;
}
static int cps2023_scp_adapter_reg_read_block(u8 reg, u8 *val, u8 num,
	void *dev_data)
{
	int ret, i;
	u8 data_len;
	u8 *p = val;
	struct cps2023_device_info *di = dev_data;

	if (!di || !p)
		return -ENODEV;

	mutex_lock(&di->accp_adapter_reg_lock);

	data_len = (num < CPS2023_SCP_DATA_LEN) ? num : CPS2023_SCP_DATA_LEN;
	hwlog_info("CMD=0x%x, REG=0x%x, Num=0x%x\n", CPS2023_SCP_CMD_MBRRD, reg, data_len);

	for (i = 0; i < CPS2023_SCP_RETRY_TIME; i++) {
		/* init */
		ret = cps2023_scp_init_reg_read_block(di, reg, data_len);
		if (ret) {
			mutex_unlock(&di->accp_adapter_reg_lock);
			return -EPERM;
		}

		/* check cmd transfer success or fail */
		if (cps2023_scp_cmd_transfer_check(di) == 0) {
			/* recived data from adapter */
			ret = cps2023_read_block(di, CPS2023_RT_BUFFER_12_REG, p, data_len);
			break;
		}
	}
	if (i >= CPS2023_SCP_RETRY_TIME) {
		hwlog_err("ack error, retry %d times\n", i);
		ret = -1;
	}
	mutex_unlock(&di->accp_adapter_reg_lock);

	if (ret) {
		/* manual init */
		cps2023_write_mask(di, CPS2023_SCP_CTL_REG, CPS2023_SCP_CTL_SNDCMD_MASK,
			CPS2023_SCP_CTL_SNDCMD_SHIFT, CPS2023_SCP_CTL_SNDCMD_RESET);
		return ret;
	}

	num -= data_len;
	/* max is CPS2023_SCP_DATA_LEN. remaining data is read in below */
	if (num) {
		p += data_len;
		reg += data_len;
		ret = cps2023_scp_adapter_reg_read_block(reg, p, num, di);
		if (ret) {
			/* manual init */
			cps2023_write_mask(di, CPS2023_SCP_CTL_REG, CPS2023_SCP_CTL_SNDCMD_MASK,
				CPS2023_SCP_CTL_SNDCMD_SHIFT, CPS2023_SCP_CTL_SNDCMD_RESET);
			hwlog_err("read error, ret is %d\n", ret);
			return -EPERM;
		}
	}
	/* manual init */
	cps2023_write_mask(di, CPS2023_SCP_CTL_REG, CPS2023_SCP_CTL_SNDCMD_MASK,
		CPS2023_SCP_CTL_SNDCMD_SHIFT, CPS2023_SCP_CTL_SNDCMD_RESET);
	power_usleep(DT_USLEEP_10MS); /* wait 10ms for operate effective */

	return 0;
}

static int cps2023_scp_adapter_reg_write(u8 val, u8 reg, struct cps2023_device_info *di)
{
	int ret;
	int i;
	u8 reg_val1 = 0;
	u8 reg_val2 = 0;

	mutex_lock(&di->accp_adapter_reg_lock);
	hwlog_info("CMD=0x%x, REG=0x%x, val=0x%x\n", CPS2023_SCP_CMD_SBRWR, reg, val);
	for (i = 0; i < CPS2023_SCP_RETRY_TIME; i++) {
		/* init */
		cps2023_write_mask(di, CPS2023_SCP_CTL_REG, CPS2023_SCP_CTL_SNDCMD_MASK,
			CPS2023_SCP_CTL_SNDCMD_SHIFT, CPS2023_SCP_CTL_SNDCMD_RESET);

		/* before send cmd, clear accp interrupt registers */
		ret = cps2023_read_byte(di, CPS2023_SCP_ISR1_REG, &reg_val1);
		ret += cps2023_read_byte(di, CPS2023_SCP_ISR2_REG, &reg_val2);
		ret += cps2023_write_byte(di, CPS2023_RT_BUFFER_0_REG, CPS2023_SCP_CMD_SBRWR);
		ret += cps2023_write_byte(di, CPS2023_RT_BUFFER_1_REG, reg);
		ret += cps2023_write_byte(di, CPS2023_RT_BUFFER_2_REG, 1);
		ret += cps2023_write_byte(di, CPS2023_RT_BUFFER_3_REG, val);
		/* initial scp_isr_backup[0],[1] */
		di->scp_isr_backup[0] = 0;
		di->scp_isr_backup[1] = 0;
		ret += cps2023_write_mask(di, CPS2023_SCP_CTL_REG, CPS2023_SCP_CTL_SNDCMD_MASK,
			CPS2023_SCP_CTL_SNDCMD_SHIFT, CPS2023_SCP_CTL_SNDCMD_START);
		if (ret) {
			hwlog_err("write error, ret is %d\n", ret);
			/* manual init */
			cps2023_write_mask(di, CPS2023_SCP_CTL_REG, CPS2023_SCP_CTL_SNDCMD_MASK,
				CPS2023_SCP_CTL_SNDCMD_SHIFT, CPS2023_SCP_CTL_SNDCMD_RESET);
			mutex_unlock(&di->accp_adapter_reg_lock);
			return -EPERM;
		}

		/* check cmd transfer success or fail */
		if (cps2023_scp_cmd_transfer_check(di) == 0)
			break;
	}
	if (i >= CPS2023_SCP_RETRY_TIME) {
		hwlog_err("ack error, retry %d times\n", i);
		ret = -1;
	}
	/* manual init */
	cps2023_write_mask(di, CPS2023_SCP_CTL_REG, CPS2023_SCP_CTL_SNDCMD_MASK,
		CPS2023_SCP_CTL_SNDCMD_SHIFT, CPS2023_SCP_CTL_SNDCMD_RESET);
	power_usleep(DT_USLEEP_10MS); /* wait 10ms for operate effective */

	mutex_unlock(&di->accp_adapter_reg_lock);
	return ret;
}

static int cps2023_fcp_adapter_vol_check(int adapter_vol_mv,
	struct cps2023_device_info *di)
{
	int i;
	int ret;
	int adc_vol = 0;

	if ((adapter_vol_mv < CPS2023_FCP_ADAPTER_MIN_VOL) ||
		(adapter_vol_mv > CPS2023_FCP_ADAPTER_MAX_VOL)) {
		hwlog_err("check vol out of range, input vol=%dmV\n", adapter_vol_mv);
		return -EPERM;
	}

	for (i = 0; i < CPS2023_FCP_ADAPTER_VOL_CHECK_TIMEOUT; i++) {
		ret = cps2023_get_vbus_mv((unsigned int *)&adc_vol, di);
		if (ret)
			continue;
		if ((adc_vol > (adapter_vol_mv - CPS2023_FCP_ADAPTER_VOL_CHECK_ERROR)) &&
			(adc_vol < (adapter_vol_mv + CPS2023_FCP_ADAPTER_VOL_CHECK_ERROR)))
			break;
		msleep(CPS2023_FCP_ADAPTER_VOL_CHECK_POLLTIME);
	}

	if (i == CPS2023_FCP_ADAPTER_VOL_CHECK_TIMEOUT) {
		hwlog_err("check vol timeout, input vol=%dmV\n", adapter_vol_mv);
		return -EPERM;
	}
	hwlog_info("check vol success, input vol=%dmV, spend=%dms\n",
		adapter_vol_mv, i * CPS2023_FCP_ADAPTER_VOL_CHECK_POLLTIME);
	return 0;
}

static int cps2023_fcp_master_reset(void *dev_data)
{
	struct cps2023_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	return cps2023_scp_wdt_reset_by_sw(di);
}

static int cps2023_fcp_adapter_reset(void *dev_data)
{
	int ret;
	struct cps2023_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = cps2023_write_mask(di, CPS2023_SCP_CTL_REG, CPS2023_SCP_CTL_MSTR_RST_MASK,
		CPS2023_SCP_CTL_MSTR_RST_SHIFT, 1);
	power_usleep(DT_USLEEP_20MS); /* wait 20ms for operate effective */
	ret += cps2023_fcp_adapter_vol_check(CPS2023_FCP_ADAPTER_RST_VOL, di); /* set 5V */
	if (ret) {
		hwlog_err("fcp adapter reset fail\n");
		return cps2023_scp_wdt_reset_by_sw(di);
	}

	ret = cps2023_config_vusb_ovp_th_mv(di,  CPS2023_VUSB_VWPC_OVP_MIN);
	ret += cps2023_config_vbus_ovp_th_mv(di,  CPS2023_VBUS_OVP_F21SC_INIT, CPS2023_CHG_F21SC_MODE);

	return ret;
}

static int cps2023_fcp_read_switch_status(void *dev_data)
{
	return 0;
}

static bool cps2023_is_fcp_enable_hand_success(struct cps2023_device_info *di)
{
	int ret;
	u8 scp_hand_succ = 0;

	ret = cps2023_read_byte(di, CPS2023_SCP_STATUS_REG, &scp_hand_succ);
	if (ret == 0 && (scp_hand_succ & CPS2023_SCP_STATUS_ENABLE_HAND_SUCCESS_MASK))
		return true;

	return false;
}

static int cps2023_is_fcp_charger_type(void *dev_data)
{
	u8 reg_val = 0;
	int ret;
	struct cps2023_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (!di->dts_fcp_support)
		return 0;

	if (cps2023_is_fcp_enable_hand_success(di))
		return 1;

	return 0;
}

static void cps2023_scp_reg_reset(struct cps2023_device_info *di)
{
	cps2023_write_mask(di, CPS2023_UFCS_CTL1_REG, CPS2023_UFCS_CTL1_EN_PROTOCOL_MASK,
			CPS2023_UFCS_CTL1_EN_PROTOCOL_SHIFT, 0);
	cps2023_write_mask(di, CPS2023_SCP_CTL_REG, CPS2023_SCP_CTL_SCP_DET_EN_MASK,
		CPS2023_SCP_CTL_SCP_DET_EN_SHIFT, 0);
	/* reset scp registers when EN_SCP is changed to 0 */
	cps2023_scp_wdt_reset_by_sw(di);
}

static int cps2023_scp_enable_detect(struct cps2023_device_info *di)
{
	int ret;

	/* disable ufcs */
	ret = cps2023_write_mask(di, CPS2023_UFCS_CTL1_REG,
		CPS2023_UFCS_CTL1_EN_PROTOCOL_MASK,
		CPS2023_UFCS_CTL1_EN_PROTOCOL_SHIFT, 0);

	ret += cps2023_write_mask(di, CPS2023_UFCS_CTL1_REG, CPS2023_UFCS_CTL1_EN_PROTOCOL_MASK,
		CPS2023_UFCS_CTL1_EN_PROTOCOL_SHIFT, 1);
	ret += cps2023_write_mask(di, CPS2023_SCP_CTL_REG, CPS2023_SCP_CTL_SCP_DET_EN_MASK,
		CPS2023_SCP_CTL_SCP_DET_EN_SHIFT, CPS2023_SCP_ENABLE);
	if (ret) {
		hwlog_err("SCP enable detect fail\n");
		cps2023_scp_reg_reset(di);
	}
	return ret;
}

static int cps2023_fcp_adapter_detect(struct cps2023_device_info *di)
{
	u8 reg_val1 = 0;
	u8 reg_val2 = 0;
	int vbus_uvp;
	int i;
	int ret;

	mutex_lock(&di->scp_detect_lock);

	if (cps2023_is_fcp_enable_hand_success(di)) {
		hwlog_info("fcp adapter detect ok\n");
		mutex_unlock(&di->scp_detect_lock);
		return ADAPTER_DETECT_SUCC;
	}

	ret = cps2023_scp_enable_detect(di);
	if (ret) {
		mutex_unlock(&di->scp_detect_lock);
		return -EPERM;
	}
	/* waiting for scp set */
	for (i = 0; i < CPS2023_SCP_DETECT_MAX_COUT; i++) {
		vbus_uvp = cps2023_get_vbus_uvp_status(di);
		ret = cps2023_read_byte(di, CPS2023_SCP_STATUS_REG, &reg_val1);
		ret += cps2023_read_byte(di, CPS2023_SCP_ISR1_REG, &reg_val2);
		if (ret || vbus_uvp < 0) {
			hwlog_err("read det attach fail\n");
			continue;
		}
		hwlog_info("reg[%x]=0x%x reg[%x]=0x%x\n", CPS2023_SCP_STATUS_REG, reg_val1,
			CPS2023_SCP_ISR1_REG, reg_val2);
		if ((reg_val2 & CPS2023_SCP_ISR1_SCP_DET_FAIL_FLAG_MASK) || vbus_uvp) {
			hwlog_err("%s\n", vbus_uvp ? "vbus uvp happened" : "SCP SHAKE HAND FAILD");
			i = CPS2023_SCP_DETECT_MAX_COUT;
			break;
		}
		if (reg_val1 & CPS2023_SCP_STATUS_ENABLE_HAND_SUCCESS_MASK)
			break;
		msleep(CPS2023_SCP_POLL_TIME);
	}
	if (i == CPS2023_SCP_DETECT_MAX_COUT) {
		cps2023_scp_reg_reset(di);
		hwlog_err("CHG_SCP_ADAPTER_DETECT_OTHER return\n");
		mutex_unlock(&di->scp_detect_lock);
		return ADAPTER_DETECT_OTHER;
	}

	mutex_unlock(&di->scp_detect_lock);
	return ret;
}

static int cps2023_fcp_stop_charge_config(void *dev_data)
{
	struct cps2023_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	cps2023_fcp_master_reset(di);
	cps2023_write_mask(di, CPS2023_SCP_CTL_REG, CPS2023_SCP_CTL_SCP_DET_EN_MASK,
		CPS2023_SCP_CTL_SCP_DET_EN_SHIFT, 0);

	return 0;
}

static int cps2023_scp_reg_read(u8 *val, u8 reg,
	struct cps2023_device_info *di)
{
	int ret;

	if (g_cps2023_scp_error_flag) {
		hwlog_err("scp timeout happened, do not read reg=0x%x\n", reg);
		return -EPERM;
	}

	ret = cps2023_scp_adapter_reg_read(val, reg, di);
	if (ret) {
		hwlog_err("error reg=0x%x\n", reg);
		if (reg != HWSCP_ADP_TYPE0)
			g_cps2023_scp_error_flag = CPS2023_SCP_IS_ERR;

		return -EPERM;
	}

	return 0;
}

static int cps2023_scp_reg_write(u8 val, u8 reg,
	struct cps2023_device_info *di)
{
	int ret;

	if (g_cps2023_scp_error_flag) {
		hwlog_err("scp timeout happened, do not write reg=0x%x\n", reg);
		return -EPERM;
	}

	ret = cps2023_scp_adapter_reg_write(val, reg, di);
	if (ret) {
		hwlog_err("error reg=0x%x\n", reg);
		g_cps2023_scp_error_flag = CPS2023_SCP_IS_ERR;
		return -EPERM;
	}

	return 0;
}

static int cps2023_scp_chip_reset(void *dev_data)
{
	return cps2023_fcp_master_reset(dev_data);
}

static int cps2023_scp_reg_read_block(int reg, int *val, int num,
	void *dev_data)
{
	int ret;
	int i;
	u8 data = 0;
	struct cps2023_device_info *di = dev_data;

	if (!val || !di)
		return -ENODEV;

	g_cps2023_scp_error_flag = CPS2023_SCP_NO_ERR;

	for (i = 0; i < num; i++) {
		ret = cps2023_scp_reg_read(&data, reg + i, di);
		if (ret) {
			hwlog_err("scp read fail, reg=0x%x\n", reg + i);
			return -EPERM;
		}
		val[i] = data;
	}

	return 0;
}

static int cps2023_scp_reg_write_block(int reg, const int *val, int num,
	void *dev_data)
{
	int ret;
	int i;
	struct cps2023_device_info *di = dev_data;

	if (!val || !di)
		return -ENODEV;

	g_cps2023_scp_error_flag = CPS2023_SCP_NO_ERR;

	for (i = 0; i < num; i++) {
		ret = cps2023_scp_reg_write(val[i], reg + i, di);
		if (ret) {
			hwlog_err("scp write failed, reg=0x%x\n", reg + i);
			return -EPERM;
		}
	}

	return 0;
}

static int cps2023_scp_detect_adapter(void *dev_data)
{
	struct cps2023_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	return cps2023_fcp_adapter_detect(di);
}

static int cps2023_fcp_reg_read_block(int reg, int *val, int num,
	void *dev_data)
{
	int ret, i;
	u8 data = 0;
	struct cps2023_device_info *di = dev_data;

	if (!val || !di)
		return -ENODEV;

	g_cps2023_scp_error_flag = CPS2023_SCP_NO_ERR;

	for (i = 0; i < num; i++) {
		ret = cps2023_scp_reg_read(&data, reg + i, di);
		if (ret) {
			hwlog_err("fcp read fail, reg=0x%x\n", reg + i);
			return -EPERM;
		}
		val[i] = data;
	}
	return 0;
}

static int cps2023_fcp_reg_write_block(int reg, const int *val, int num,
	void *dev_data)
{
	int ret, i;
	struct cps2023_device_info *di = dev_data;

	if (!val || !di)
		return -ENODEV;

	g_cps2023_scp_error_flag = CPS2023_SCP_NO_ERR;

	for (i = 0; i < num; i++) {
		ret = cps2023_scp_reg_write(val[i], reg + i, di);
		if (ret) {
			hwlog_err("fcp write fail, reg=0x%x\n", reg + i);
			return -EPERM;
		}
	}

	return 0;
}

static int cps2023_fcp_detect_adapter(void *dev_data)
{
	struct cps2023_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	return cps2023_fcp_adapter_detect(di);
}

static int cps2023_scp_adapter_reset(void *dev_data)
{
	return cps2023_fcp_adapter_reset(dev_data);
}

static struct hwscp_ops cps2023_hwscp_ops = {
	.chip_name = "cps2023",
	.reg_read = cps2023_scp_reg_read_block,
	.reg_write = cps2023_scp_reg_write_block,
	.reg_multi_read = cps2023_scp_adapter_reg_read_block,
	.detect_adapter = cps2023_scp_detect_adapter,
	.soft_reset_master = cps2023_scp_chip_reset,
	.soft_reset_slave = cps2023_scp_adapter_reset,
};

static struct hwfcp_ops cps2023_hwfcp_ops = {
	.chip_name = "cps2023",
	.reg_read = cps2023_fcp_reg_read_block,
	.reg_write = cps2023_fcp_reg_write_block,
	.detect_adapter = cps2023_fcp_detect_adapter,
	.soft_reset_master = cps2023_fcp_master_reset,
	.soft_reset_slave = cps2023_fcp_adapter_reset,
	.get_master_status = cps2023_fcp_read_switch_status,
	.stop_charging_config = cps2023_fcp_stop_charge_config,
	.is_accp_charger_type = cps2023_is_fcp_charger_type,
};

int cps2023_protocol_ops_register(struct cps2023_device_info *di)
{
	int ret;

	if (di->dts_scp_support) {
		cps2023_hwscp_ops.dev_data = (void *)di;
		ret = hwscp_ops_register(&cps2023_hwscp_ops);
		if (ret)
			return -EINVAL;
	}

	if (di->dts_fcp_support) {
		cps2023_hwfcp_ops.dev_data = (void *)di;
		ret = hwfcp_ops_register(&cps2023_hwfcp_ops);
		if (ret)
			return -EINVAL;
	}

	return 0;
}