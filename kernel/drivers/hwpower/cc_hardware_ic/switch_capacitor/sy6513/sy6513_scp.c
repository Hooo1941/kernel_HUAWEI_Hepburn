// SPDX-License-Identifier: GPL-2.0
/*
 * SY6513_scp.c
 *
 * SY6513 SCP_driver
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

#include "sy6513_scp.h"
#include "sy6513_i2c.h"
#include "sy6513.h"
#include <linux/mutex.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/protocol/adapter_protocol.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_scp.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_fcp.h>

#define HWLOG_TAG sy6513_scp
HWLOG_REGIST();

static bool sy6513_scp_data_valid(struct sy6513_device_info *di, u8 scp_int)
{
	int i;
	int ret;
	u8 scp_stat;
	ret = sy6513_read_mask(di, SY6513_SCP_STATUS_REG, SY6513_SLV_R_CPL_MASK, SY6513_SLV_R_CPL_SHIFT, &scp_stat);
	if (ret)
		return false;
	/* ACK, NACK, LENGTH, CRC, PAPR CHECK */
	if (((scp_int & SY6513_ACK_MASK) >> SY6513_ACK_SHIFT == 1) &&
		((scp_int & SY6513_NACK_MASK) >> SY6513_NACK_SHIFT == 0) &&
		((scp_int & SY6513_ERR_ACK_L_MASK) >> SY6513_ERR_ACK_L_SHIFT == 0) &&
		((scp_int & SY6513_CRCRX_MASK) >> SY6513_CRCRX_SHIFT == 0) &&
		((scp_int & SY6513_PAPRX_MASK) >> SY6513_PAPRX_SHIFT == 0) &&
		(scp_stat == 1)) {
		/* save rxdata */
		if (di->scp_op == SY6513_SCP_READ_OP) {
			for (i = 0; i < di->scp_op_num; i++) {
				(void)sy6513_read_byte(di, SY6513_SCP_RDATA1_REG + i, &di->scp_data[i]);
				hwlog_info("read scp_data=0x%x\n", di->scp_data[i]);
			}
		}
		hwlog_info("scp_op_num=%d, scp_op=%d\n", di->scp_op_num, di->scp_op);
		return true;
	}
	return false;
}

static int sy6513_scp_cmd_transfer_check(struct sy6513_device_info *di)
{
	int cnt = 0;
	u8 scp_int = 0;
	int ret;

	do {
		(void)power_msleep(DT_MSLEEP_50MS, 0, NULL);
		ret = sy6513_read_byte(di, SY6513_SCP_INT1_REG, &scp_int);
		if (ret)
			return false;
		di->scp_trans_done = (scp_int & SY6513_CMDCPL_MASK) >> SY6513_CMDCPL_SHIFT;
		hwlog_info("%s ic_%u reg[0x%x]=0x%x \n", __func__, di->ic_role, SY6513_SCP_INT1_REG, scp_int);

		if (di->scp_trans_done) {
			if (sy6513_scp_data_valid(di, scp_int)) {
				hwlog_info("scp_trans success\n");
				return 0;
			}
			hwlog_err("scp_trans_done, but data err\n");
			return -EINVAL;
		}
		cnt++;
	} while (cnt < SY6513_SCP_ACK_RETRY_TIME);

	hwlog_err("SCP_adapter trans time out\n");
	return -EINVAL;
}

static int sy6513_scp_adapter_reg_read(u8 *val, u8 reg, struct sy6513_device_info *di)
{
	int ret;
	int i;

	mutex_lock(&di->accp_adapter_reg_lock);

	/* clear SCP_data */
	memset(di->scp_data, 0, sizeof(di->scp_data));

	di->scp_op = SY6513_SCP_READ_OP;
	di->scp_op_num = SY6513_SCP_SBRWR_NUM;
	di->scp_trans_done = false;
	for (i = 0; i < SY6513_SCP_RETRY_TIME; i++) {
		/* write data */
		ret = sy6513_write_byte(di, SY6513_SCP_CMD_REG, SY6513_SCP_CMD_SBRRD);
		ret += sy6513_write_byte(di, SY6513_SCP_ADDR_REG, reg);
		/* start trans */
		ret += sy6513_write_mask(di, SY6513_SCP_CTRL_REG, SY6513_SNDCMD_MASK,
			SY6513_SNDCMD_SHIFT, 1);
		if (ret) {
			mutex_unlock(&di->accp_adapter_reg_lock);
			return -EIO;
		}
		/* check cmd transfer success or fail */
		if (sy6513_scp_cmd_transfer_check(di) == 0) {
			memcpy(val, &di->scp_data[0], di->scp_op_num);
			break;
		}
	}
	if (i >= SY6513_SCP_RETRY_TIME) {
		hwlog_err("ack error, retry\n");
		ret = -EINVAL;
	}

	mutex_unlock(&di->accp_adapter_reg_lock);
	return ret;
}

static int sy6513_scp_adapter_multi_reg_read(u8 reg, u8 *val, u8 num, struct sy6513_device_info *di)
{
	int ret;
	int i;

	mutex_lock(&di->accp_adapter_reg_lock);

	/* clear SCP_data */
	memset(di->scp_data, 0, sizeof(di->scp_data));

	di->scp_op = SY6513_SCP_READ_OP;
	di->scp_op_num = num;
	di->scp_trans_done = false;
	for (i = 0; i < SY6513_SCP_RETRY_TIME; i++) {
		/* write cmd, reg, num */
		ret = sy6513_write_byte(di, SY6513_SCP_CMD_REG, SY6513_SCP_CMD_MBRRD);
		ret += sy6513_write_byte(di, SY6513_SCP_ADDR_REG, reg);
		ret += sy6513_write_byte(di, SY6513_SCP_NB_REG, num);

		/* start trans */
		ret += sy6513_write_mask(di, SY6513_SCP_CTRL_REG,
			SY6513_SNDCMD_MASK, SY6513_SNDCMD_SHIFT, 1);
		if (ret) {
			mutex_unlock(&di->accp_adapter_reg_lock);
			return -EIO;
		}
		/* check cmd transfer success or fail, ignore ack data */
		if (sy6513_scp_cmd_transfer_check(di) == 0) {
			memcpy(val, &di->scp_data[0], num);
			break;
		}
	}
	if (i >= SY6513_SCP_RETRY_TIME) {
		hwlog_err("ack error, retry\n");
		ret = -EINVAL;
	}

	mutex_unlock(&di->accp_adapter_reg_lock);
	return ret;
}

static int sy6513_scp_reg_read(u8 *val, u8 reg, struct sy6513_device_info *di)
{
	int ret;

	if (di->scp_error_flag) {
		hwlog_err("SCP_timeout happened, do not read reg=0x%x\n", reg);
		return -EINVAL;
	}

	ret = sy6513_scp_adapter_reg_read(val, reg, di);
	if (ret) {
		if (reg != HWSCP_ADP_TYPE0)
			di->scp_error_flag = SY6513_SCP_IS_ERR;

		return -EINVAL;
	}

	return 0;
}

static int sy6513_scp_adapter_reg_write(u8 val, u8 reg, struct sy6513_device_info *di)
{
	int ret;
	int i;

	mutex_lock(&di->accp_adapter_reg_lock);

	/* clear SCP_data */
	memset(di->scp_data, 0, sizeof(di->scp_data));

	di->scp_op = SY6513_SCP_WRITE_OP;
	di->scp_op_num = SY6513_SCP_SBRWR_NUM;
	di->scp_trans_done = false;
	for (i = 0; i < SY6513_SCP_RETRY_TIME; i++) {
		/* write data */
		ret = sy6513_write_byte(di, SY6513_SCP_CMD_REG, SY6513_SCP_CMD_SBRWR);
		ret += sy6513_write_byte(di, SY6513_SCP_ADDR_REG, reg);
		ret += sy6513_write_byte(di, SY6513_SCP_WDATA1_REG, val); // write DATA in WDATA1_REG
		/* start trans */
		ret += sy6513_write_mask(di, SY6513_SCP_CTRL_REG,
			SY6513_SNDCMD_MASK, SY6513_SNDCMD_SHIFT, 1);
		if (ret) {
			mutex_unlock(&di->accp_adapter_reg_lock);
			return -EIO;
		}
		/* check cmd transfer success or fail */
		if (sy6513_scp_cmd_transfer_check(di) == 0)
			break;
	}
	if (i >= SY6513_SCP_RETRY_TIME) {
		hwlog_err("ack error, retry\n");
		ret = -EINVAL;
	}

	mutex_unlock(&di->accp_adapter_reg_lock);
	return ret;
}

static int sy6513_scp_adapter_multi_reg_write(u8 reg, int *val, u8 num, struct sy6513_device_info *di)
{
	int ret;
	int i;
	int j;

	mutex_lock(&di->accp_adapter_reg_lock);

	/* clear SCP_data */
	memset(di->scp_data, 0, sizeof(di->scp_data));

	di->scp_op = SY6513_SCP_WRITE_OP;
	di->scp_op_num = num;
	di->scp_trans_done = false;
	for (i = 0; i < SY6513_SCP_RETRY_TIME; i++) {
		/* write cmd, reg, num */
		ret = sy6513_write_byte(di, SY6513_SCP_CMD_REG, SY6513_SCP_CMD_MBRWR);
		hwlog_info("write reg[%x]=0x%x\n", SY6513_SCP_CMD_REG, SY6513_SCP_CMD_MBRWR);
		ret += sy6513_write_byte(di, SY6513_SCP_ADDR_REG, reg);
		hwlog_info("write reg[%x]=0x%x\n", SY6513_SCP_ADDR_REG, reg);
		ret += sy6513_write_byte(di, SY6513_SCP_NB_REG, num);
		hwlog_info("write reg[%x]=0x%x\n", SY6513_SCP_NB_REG, num);

		for (j = 0; j < num; j++) {
			ret += sy6513_write_byte(di, SY6513_SCP_WDATA1_REG + j, (u8)val[j]);
			hwlog_info("write reg[%x]=0x%x\n", SY6513_SCP_WDATA1_REG, val[j]);
		}
		/* start trans */
		ret += sy6513_write_mask(di, SY6513_SCP_CTRL_REG,
			SY6513_SNDCMD_MASK, SY6513_SNDCMD_SHIFT, 1);
		if (ret) {
			mutex_unlock(&di->accp_adapter_reg_lock);
			return -EIO;
		}
		/* check cmd transfer success or fail, ignore ack data */
		if (sy6513_scp_cmd_transfer_check(di) == 0) {
			break;
		}
	}
	if (i >= SY6513_SCP_RETRY_TIME) {
		hwlog_err("ack error, retry\n");
		ret = -EINVAL;
	}

	mutex_unlock(&di->accp_adapter_reg_lock);
	return ret;
}

static int sy6513_scp_reg_write(u8 val, u8 reg, struct sy6513_device_info *di)
{
	int ret;

	if (di->scp_error_flag) {
		hwlog_err("SCP_timeout happened, do not write reg=0x%x\n", reg);
		return -EINVAL;
	}

	ret = sy6513_scp_adapter_reg_write(val, reg, di);
	if (ret) {
		di->scp_error_flag = SY6513_SCP_IS_ERR;
		return -EINVAL;
	}

	return 0;
}

static int sy6513_fcp_adapter_vol_check(int adapter_vol, struct sy6513_device_info *di)
{
	int ret;
	int i;
	int adc_vol = 0;

	if ((adapter_vol < SY6513_FCP_ADAPTER_MIN_VOL) ||
		(adapter_vol > SY6513_FCP_ADAPTER_MAX_VOL)) {
		hwlog_err("check vol out of range, input vol = %dmV\n", adapter_vol);
		return -EINVAL;
	}

	for (i = 0; i < SY6513_FCP_ADAPTER_VOL_CHECK_TIME; i++) {
		ret = sy6513_get_vbus_mv((unsigned int *)&adc_vol, di);
		if (ret)
			continue;
		if ((adc_vol > (adapter_vol - SY6513_FCP_ADAPTER_ERR_VOL)) &&
			(adc_vol < (adapter_vol + SY6513_FCP_ADAPTER_ERR_VOL)))
			break;
		(void)power_msleep(DT_MSLEEP_100MS, 0, NULL);
	}
	if (i == SY6513_FCP_ADAPTER_VOL_CHECK_TIME) {
		hwlog_err("check vol timeout, input vol = %dmV\n", adapter_vol);
		return -EINVAL;
	}

	hwlog_info("check vol success, input vol = %dmV, spend %dms\n",
		adapter_vol, i * DT_MSLEEP_100MS);
	return 0;
}

static int sy6513_fcp_master_reset(void *dev_data)
{
	int ret;
	struct sy6513_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sy6513_write_mask(di, SY6513_SCP_STATUS_REG,
		SY6513_RESET_SCP_MASK, SY6513_RESET_SCP_SHIFT, 1); // SCP_reset
	power_usleep(DT_USLEEP_10MS);

	return ret;
}

static int sy6513_fcp_adapter_reset(void *dev_data)
{
	int ret;
	struct sy6513_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sy6513_write_mask(di, SY6513_SCP_CTRL_REG,
		SY6513_MSTR_RST_MASK, SY6513_MSTR_RST_SHIFT, 1);
	power_usleep(DT_USLEEP_20MS);
	ret += sy6513_fcp_adapter_vol_check(SY6513_FCP_ADAPTER_RST_VOL, di);
	if (ret)
		return -EINVAL;

	ret = sy6513_config_vusb_ovp_th_mv(di, SY6513_VUSB_OVP_7P5V);
	ret += sy6513_config_vbus_ovp_th_mv(di, SY6513_BPS_VBUS_OVP_5P6V, SY6513_CHG_FBYPASS_MODE);

	return ret;
}

static int sy6513_fcp_read_switch_status(void *dev_data)
{
	return 0;
}

static int sy6513_is_fcp_charger_type(void *dev_data)
{
	struct sy6513_device_info *di = dev_data;

	if (!di)
		return 0;

	if (di->dts_fcp_support == 0) {
		hwlog_err("not support fcp\n");
		return 0;
	}

	if (di->fcp_support)
		return 1;

	return 0;
}

static void sy6513_fcp_adapter_detect_reset(struct sy6513_device_info *di)
{
	(void)sy6513_write_mask(di, SY6513_DETECT_CONTROL_REG,
		SY6513_DPDM_DETC_EN_MASK, SY6513_DPDM_DETC_EN_SHIFT, false); // DPDM detect disable
	(void)sy6513_write_mask(di, SY6513_SCP_CONTROL_REG,
		SY6513_SET_DPDM_EN_MASK, SY6513_SET_DPDM_EN_SHIFT, false); // SET_DPDM disable
	(void)sy6513_write_mask(di, SY6513_SCP_CONTROL_REG, SY6513_SCP_EN_MASK,
		SY6513_SCP_EN_SHIFT, false); // SCP_disable
	(void)sy6513_fcp_adapter_reset(di);
}

static int sy6513_fcp_adapter_detect_enable(struct sy6513_device_info *di)
{
	int ret;
	int cnt;
	u8 usb_stat;
	u8 val;

	ret = sy6513_write_byte(di, SY6513_SCP_BC12_REG, SY6513_SCP_SET_BC12);
	(void)power_msleep(DT_MSLEEP_150MS, 0, NULL);
	do {
		(void)power_msleep(DT_MSLEEP_10MS, 0, NULL);
		ret += sy6513_read_byte(di, SY6513_USB_STATUS2_REG, &val);
		hwlog_debug("reg[0x%x], usb_stat=0x%x\n", SY6513_USB_STATUS2_REG, val);
		if (val == 0x40) {
			hwlog_debug("usb_stat=0x%x\n", val);
			break;
		}
		cnt++;
	} while (cnt < 10);

	ret += sy6513_write_mask(di, SY6513_SCP_CONTROL_REG, SY6513_SCP_EN_MASK,
		SY6513_SCP_EN_SHIFT, true); // SCP_enable
	ret += sy6513_write_mask(di, SY6513_SCP_CONTROL_REG, SY6513_SCP_DET_EN_MASK,
		SY6513_SCP_DET_EN_SHIFT, true); // SCP_DET_enable
	ret += sy6513_read_mask(di, SY6513_USB_STATUS2_REG, SY6513_USB_STATUS_MASK,
		SY6513_USB_STATUS_SHIFT, &usb_stat);
	if (!ret && (usb_stat == 0x4)) // usb_stat is DCP
		return false;

	return true;
}

static int sy6513_fcp_adapter_detect_dpdm_stat(struct sy6513_device_info *di)
{
	int cnt;
	int ret;
	u8 en = 0;
	u8 stat = 0;

	for (cnt = 0; cnt < SY6513_SCP_DETECT_MAX_CNT; cnt++) {
		ret = sy6513_write_mask(di, SY6513_DETECT_CONTROL_REG, SY6513_DPDM_DETC_EN_MASK,
			SY6513_DPDM_DETC_EN_SHIFT, true);
		ret += sy6513_read_mask(di, SY6513_DETECT_CONTROL_REG, SY6513_DPDM_DETC_EN_MASK,
			SY6513_DPDM_DETC_EN_SHIFT, &en);
		hwlog_debug("dpdm_detect_control=0x%x", en);
		ret += sy6513_read_byte(di, SY6513_DETECT_CONTROL_REG, &stat);
		hwlog_info("scp_dm_stat=0x%x\n", stat);
		if (ret) {
			hwlog_err("read dm_stat_reg fail\n");
			continue;
		}

		/* 0: DM voltage < 0.325v, 2: 0.425v < DP voltage < 1v */
		if (((((stat & SY6513_DETECT_DM_MASK) >> SY6513_DETECT_DM_SHIFT) == 0) ||
			(((stat & SY6513_DETECT_DM_MASK) >> SY6513_DETECT_DM_SHIFT) == 1)) &&
			((((stat & SY6513_DETECT_DP_MASK) >> SY6513_DETECT_DP_SHIFT) >= 2)))
			break;
		(void)power_msleep(DT_MSLEEP_100MS, 0, NULL);
	}

	if (cnt == SY6513_SCP_DETECT_MAX_CNT) {
		hwlog_err("detect_dpdm fail, CHG_SCP_ADAPTER_DETECT_OTHER\n");
		return -EINVAL;
	}

	return 0;
}

static int sy6513_fcp_adapter_detect_ping_stat(struct sy6513_device_info *di)
{
	int cnt;
	u8 scp_stat = 0;

	for (cnt = 0; cnt < SY6513_SCP_PING_DETECT_MAX_CNT; cnt++) {
		power_msleep(DT_MSLEEP_10MS, 0, NULL);
		sy6513_read_byte(di, SY6513_INT1_REG, &scp_stat);
		hwlog_debug("scp_ping_stat=0x%x\n", scp_stat);
		if (((scp_stat & SY6513_ENABLE_HAND_NO_RESPOND_MASK) >> SY6513_ENABLE_HAND_NO_RESPOND_SHIFT) == 0) {
			hwlog_info("SCP_adapter ping Detect_ok\n");
			di->fcp_support = true;
			break;
		}
	}
	if (cnt == SY6513_SCP_PING_DETECT_MAX_CNT) {
		hwlog_err("detect_ping_fail, CHG_SCP_ADAPTER_DETECT_OTHER\n");

		return -EINVAL;
	}

	return 0;
}

static bool sy6513_fcp_adapter_detect_check(struct sy6513_device_info *di)
{
	u8 dpdm_ctrl = 0;
	u8 scp_ctrl = 0;
	u8 dpdm_val = 0;
	int ret;

	ret = sy6513_read_byte(di, SY6513_INT1_REG, &dpdm_ctrl);    // SCP_DM_DET_FLAG speed
	ret += sy6513_read_byte(di, SY6513_USB_STATUS1_REG, &scp_ctrl);    // DVC SCP detected_FLAG
	ret += sy6513_read_byte(di, SY6513_DETECT_CONTROL_REG, &dpdm_val); // DPDM
	if (!ret && ((dpdm_ctrl & SY6513_SCP_DM_DET_FLAG_MASK) >> SY6513_SCP_DM_DET_FLAG_SHIFT == 1) &&
		((scp_ctrl & SY6513_DVC_MASK) >> SY6513_DVC_SHIFT == 1) &&
		((dpdm_val & SY6513_DETECT_DP_MASK) >> SY6513_DETECT_DP_SHIFT == SY6513_DETECT_DP_IS_0P6V) &&
		((dpdm_val & SY6513_DETECT_DM_MASK) >> SY6513_DETECT_DM_SHIFT == 0))
		return true;

	return false;
}

static int sy6513_fcp_adapter_detect(struct sy6513_device_info *di)
{
	int ret;
	u8 enable = 0;

	mutex_lock(&di->scp_detect_lock);
	di->init_finish_flag = true;

	if (di->fcp_support && sy6513_fcp_adapter_detect_check(di)) {
		mutex_unlock(&di->scp_detect_lock);

		return ADAPTER_DETECT_SUCC;
	}

	/* transmit vdp_src_BC12_signal */
	ret = sy6513_fcp_adapter_detect_enable(di);
	if (ret) {
		sy6513_fcp_adapter_detect_reset(di);
		mutex_unlock(&di->scp_detect_lock);

		return ADAPTER_DETECT_OTHER;
	}

	/* waiting for hvdcp */
	(void)power_msleep(DT_MSLEEP_1S, 0, NULL);

	ret = sy6513_read_byte(di, SY6513_SCP_CONTROL_REG, &enable); // SCP EN, SCE DET EN
	if (ret || (((((enable & SY6513_SCP_EN_MASK) >> SY6513_SCP_EN_SHIFT) == 0)) &&
		((enable & SY6513_SCP_DET_EN_MASK) >> SY6513_SCP_DET_EN_SHIFT) == 0)) {
		hwlog_info("SCP_en detect,scp_en:0x%x\n", enable);
		sy6513_fcp_adapter_detect_reset(di);
		mutex_unlock(&di->scp_detect_lock);

		return ADAPTER_DETECT_OTHER;
	}
	/* Detect_dpdm stat */
	ret = sy6513_fcp_adapter_detect_dpdm_stat(di);
	if (ret) {
		sy6513_fcp_adapter_detect_reset(di);
		mutex_unlock(&di->scp_detect_lock);

		return ADAPTER_DETECT_OTHER;
	}
	/* Detect_ping stat */
	ret = sy6513_fcp_adapter_detect_ping_stat(di);
	if (ret) {
		sy6513_fcp_adapter_detect_reset(di);
		mutex_unlock(&di->scp_detect_lock);

		return ADAPTER_DETECT_OTHER;
	}

	mutex_unlock(&di->scp_detect_lock);
	return ADAPTER_DETECT_SUCC;
}

static int sy6513_fcp_stop_charge_config(void *dev_data)
{
	int ret;
	struct sy6513_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sy6513_fcp_master_reset(di);
	ret += sy6513_write_mask(di, SY6513_SCP_CONTROL_REG, SY6513_SCP_EN_MASK,
		SY6513_SCP_EN_SHIFT, 0); // SCP disable
	ret += sy6513_write_mask(di, SY6513_SCP_CONTROL_REG, SY6513_SCP_DET_EN_MASK,
		SY6513_SCP_DET_EN_SHIFT, 0); // SCP det disable
	if (ret)
		return -EINVAL;

	di->fcp_support = false;

	return ret;
}

static int sy6513_self_check(void *dev_data)
{
	return 0;
}

static int sy6513_scp_chip_reset(void *dev_data)
{
	return sy6513_fcp_master_reset(dev_data);
}

static int sy6513_scp_dpdm_reset(void *dev_data)
{
	struct sy6513_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	return sy6513_write_mask(di, SY6513_SCP_CONTROL_REG,
		SY6513_SET_DPDM_EN_MASK, SY6513_SET_DPDM_EN_SHIFT, 0); // set DPDM disable
}

static int sy6513_scp_reg_read_block(int reg, int *val, int num, void *dev_data)
{
	int ret;
	int i;
	u8 data = 0;
	struct sy6513_device_info *di = dev_data;

	if (!di || !val)
		return -ENODEV;

	di->scp_error_flag = SY6513_SCP_NO_ERR;

	for (i = 0; i < num; i++) {
		ret = sy6513_scp_reg_read(&data, reg + i, di);
		if (ret) {
			hwlog_err("SCP_read failed, reg=0x%x\n", reg + i);
			return -EINVAL;
		}
		val[i] = data;
	}

	return 0;
}

static int sy6513_scp_multi_reg_read_block(u8 reg, u8 *val, u8 num, void *dev_data)
{
	int ret;
	u8 *p = val;
	u8 data_len = (num < SY6513_SCP_MULTI_READ_LEN) ? num : SY6513_SCP_MULTI_READ_LEN;

	struct sy6513_device_info *di = dev_data;

	if (!di || !val)
		return -ENODEV;

	di->scp_error_flag = SY6513_SCP_NO_ERR;

	ret = sy6513_scp_adapter_multi_reg_read(reg, p, data_len, di);
	if (ret)
		return -EINVAL;

	num -= data_len;
	if (num) {
		p += data_len;
		reg += data_len;
		ret = sy6513_scp_multi_reg_read_block(reg, p, num, di);
		if (ret)
			return -EINVAL;
	}

	return ret;
}

static int sy6513_scp_multi_reg_write_block(u8 reg, int *val, u8 num, void *dev_data)
{
	int ret;
	int *p = val;
	u8 data_len = (num < SY6513_SCP_MULTI_WRITE_LEN) ? num : SY6513_SCP_MULTI_WRITE_LEN;

	struct sy6513_device_info *di = dev_data;

	if (!di || !val)
		return -ENODEV;

	di->scp_error_flag = SY6513_SCP_NO_ERR;

	ret = sy6513_scp_adapter_multi_reg_write(reg, p, data_len, di);
	if (ret)
		return -EINVAL;

	num -= data_len;
	if (num) {
		p += data_len;
		reg += data_len;
		ret = sy6513_scp_multi_reg_write_block(reg, p, num, di);
		if (ret)
			return -EINVAL;
	}

	return ret;
}

static int sy6513_scp_reg_write_block(int reg, const int *val, int num, void *dev_data)
{
	int ret;
	int i;
	struct sy6513_device_info *di = dev_data;

	if (!di || !val)
		return -ENODEV;

	di->scp_error_flag = SY6513_SCP_NO_ERR;

	for (i = 0; i < num; i++) {
		ret = sy6513_scp_reg_write(val[i], reg + i, di);
		if (ret) {
			hwlog_err("SCP_write failed, reg=0x%x\n", reg + i);
			return -EINVAL;
		}
	}

	return 0;
}

static int sy6513_scp_detect_adapter(void *dev_data)
{
	struct sy6513_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	return sy6513_fcp_adapter_detect(di);
}

static int sy6513_fcp_reg_read_block(int reg, int *val, int num, void *dev_data)
{
	int ret;
	int i;
	u8 data = 0;
	struct sy6513_device_info *di = dev_data;

	if (!di || !val)
		return -ENODEV;

	di->scp_error_flag = SY6513_SCP_NO_ERR;

	for (i = 0; i < num; i++) {
		ret = sy6513_scp_reg_read(&data, reg + i, di);
		if (ret) {
			hwlog_err("fcp read failed, reg=0x%x\n", reg + i);
			return -EINVAL;
		}
		val[i] = data;
	}
	return 0;
}

static int sy6513_fcp_reg_write_block(int reg, const int *val, int num, void *dev_data)
{
	int ret;
	int i;
	struct sy6513_device_info *di = dev_data;

	if (!di || !val)
		return -ENODEV;

	di->scp_error_flag = SY6513_SCP_NO_ERR;

	for (i = 0; i < num; i++) {
		ret = sy6513_scp_reg_write(val[i], reg + i, di);
		if (ret) {
			hwlog_err("fcp write failed, reg=0x%x\n", reg + i);
			return -EINVAL;
		}
	}

	return 0;
}

static int sy6513_fcp_detect_adapter(void *dev_data)
{
	struct sy6513_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	return sy6513_fcp_adapter_detect(di);
}

static int sy6513_pre_init(void *dev_data)
{
	struct sy6513_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	return sy6513_self_check(di);
}

static int sy6513_scp_adapter_rest(void *dev_data)
{
	return sy6513_fcp_adapter_reset(dev_data);
}

static struct hwscp_ops sy6513_hwscp_ops = {
	.chip_name = "sy6513",
	.reg_read = sy6513_scp_reg_read_block,
	.reg_write = sy6513_scp_reg_write_block,
	.reg_multi_read = sy6513_scp_multi_reg_read_block,
	.reg_multi_write = sy6513_scp_multi_reg_write_block,
	.detect_adapter = sy6513_scp_detect_adapter,
	.soft_reset_master = sy6513_scp_chip_reset,
	.soft_reset_slave = sy6513_scp_adapter_rest,
	.soft_reset_dpdm = sy6513_scp_dpdm_reset,
	.pre_init = sy6513_pre_init,
};

static struct hwfcp_ops sy6513_hwfcp_ops = {
	.chip_name = "sy6513",
	.reg_read = sy6513_fcp_reg_read_block,
	.reg_write = sy6513_fcp_reg_write_block,
	.detect_adapter = sy6513_fcp_detect_adapter,
	.soft_reset_master = sy6513_fcp_master_reset,
	.soft_reset_slave = sy6513_fcp_adapter_reset,
	.get_master_status = sy6513_fcp_read_switch_status,
	.stop_charging_config = sy6513_fcp_stop_charge_config,
	.is_accp_charger_type = sy6513_is_fcp_charger_type,
};

int sy6513_protocol_ops_register(struct sy6513_device_info *di)
{
	int ret;

	if (di->dts_scp_support) {
		sy6513_hwscp_ops.dev_data = (void *)di;
		ret = hwscp_ops_register(&sy6513_hwscp_ops);
		if (ret)
			return -EINVAL;
	}
	if (di->dts_fcp_support) {
		sy6513_hwfcp_ops.dev_data = (void *)di;
		ret = hwfcp_ops_register(&sy6513_hwfcp_ops);
		if (ret)
			return -EINVAL;
	}

	return 0;
}

void sy6513_scp_adapter_default_vset(void *dev_data)
{
	int value[BYTE_ONE];

	 /* Set_adap default vSet_5000 mv */
	value[0] = (5000 - HWSCP_VSSET_OFFSET) / HWSCP_VSSET_STEP;
	sy6513_scp_reg_write_block(HWSCP_VSSET, value, BYTE_ONE, dev_data);
}
