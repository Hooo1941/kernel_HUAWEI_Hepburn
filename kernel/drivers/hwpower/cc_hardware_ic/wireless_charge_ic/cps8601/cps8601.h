/* SPDX-License-Identifier: GPL-2.0 */
/*
 * cps8601.h
 *
 * cps8601 macro, addr etc.
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#ifndef _CPS8601_H_
#define _CPS8601_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/bitops.h>
#include <linux/jiffies.h>
#include <chipset_common/hwpower/protocol/wireless_protocol_qi.h>
#include <chipset_common/hwpower/wireless_charge/wireless_firmware.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_power_supply.h>
#include <chipset_common/hwpower/common_module/power_bigdata.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_pinctrl.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_wakeup.h>
#include "cps8601_chip.h"

#define CPS8601_Q_PINCTRL_LEN                 3
#define CPS8601_PINCTRL_LEN                   1
#define CPS8601_FW_REG_CFG_SIZE               9
#define CPS8601_Q_LONG_PERIOD                 0
#define CPS8601_Q_SHORT_PERIOD                1
#define CPS8601_Q_FACTORY_WIDTH_LTH           1500
#define CPS8601_Q_FACTORY_WIDTH_HTH           2500
#define CPS8601_Q_SAMPLE_WIDTH_LTH            1800
#define CPS8601_Q_SAMPLE_WIDTH_HTH            2100
#define CPS8601_Q_SAMPLE_WIDTH_VAR_TH         10
#define CPS8601_Q_FAC_WIDTH_DELTA_LTH         (-70)
#define CPS8601_Q_FAC_WIDTH_DELTA_HTH         20
#define CPS8601_Q_DYN_WIDTH_DELTA_LTH0        (-70)
#define CPS8601_Q_DYN_WIDTH_DELTA_HTH0        (-25)
#define CPS8601_Q_DYN_WIDTH_DELTA_LTH1        25
#define CPS8601_Q_DYN_WIDTH_DELTA_HTH1        70
#define CPS8601_Q_CALI_UNVALID                0
#define CPS8601_Q_CALI_DYNAMIC                1
#define CPS8601_Q_CALI_FACTORY                2

enum cps8601_q_cali_result {
	CPS8601_Q_UNCALIBRATIN = 0,
	CPS8601_Q_CALI_FAIL,
	CPS8601_Q_CALI_SUCC,
	CPS8601_Q_CALIBRATING,
	CPS8601_Q_WRITE_MTP,
};

enum cps8601_default_psy_type {
	CPS8601_DEFAULT_POWEROFF = 0,
	CPS8601_DEFAULT_POWERON,
	CPS8601_DEFAULT_LOWPOWER,
};

struct cps8601_chip_info {
	u16 chip_id;
	u16 mtp_ver;
};

struct cps8601_global_val {
	bool mtp_chk_complete;
	bool mtp_latest;
	bool tx_stop_chrg_flag;
	bool tx_calibrate_flag;
	struct hwqi_handle *qi_hdl;
};

struct cps8601_tx_init_para {
	u16 ping_interval;
	u16 ping_freq;
};

struct cps8601_tx_fod_para {
	u16 ploss_rp_th0;
	u16 ploss_th0;
	u16 ploss_th1;
	u16 ploss_th2;
	u8 ploss_cnt;
};

struct cps8601_tx_fop_para {
	u16 tx_max_fop;
	u16 tx_min_fop;
};

struct cps8601_tx_q_factor_para {
	u16 tx_q_cnt;
	u16 tx_q_width;
	u8 tx_q_cnt_var;
	u8 tx_q_width_var;
	u16 tx_q_fac_width;
	u16 tx_q_dyn_width;
	u16 tx_q_spl_width;
	u16 tx_q_flag;
};

struct cps8601_fw_reg_para {
	u32 addr;
	u32 data;
	u32 interval;
};

struct cps8601_dev_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	struct delayed_work mtp_check_work;
	struct mutex mutex_irq;
	struct wakeup_source *wakelock;
	struct cps8601_global_val g_val;
	struct cps8601_tx_init_para tx_init_para;
	struct cps8601_tx_fod_para tx_fod;
	struct cps8601_tx_fop_para tx_fop;
	struct cps8601_tx_q_factor_para tx_q_factor;
	unsigned int ic_type;
	unsigned int default_psy_type;
	unsigned int q_cali_dynamic_en;
	bool irq_active;
	int gpio_rx_online;
	int gpio_lowpower;
	int gpio_int;
	int irq_int;
	u16 irq_val;
	int irq_cnt;
	u16 ept_type;
	u16 tx_ocp_th;
	u16 tx_pocp_th;
	u16 tx_cep_timeout;
	u16 chip_id;
	u8 q_cali_result;
};

/* cps8601 i2c */
int cps8601_read_byte(struct cps8601_dev_info *di, u16 reg, u8 *data);
int cps8601_read_word(struct cps8601_dev_info *di, u16 reg, u16 *data);
int cps8601_write_byte(struct cps8601_dev_info *di, u16 reg, u8 data);
int cps8601_write_word(struct cps8601_dev_info *di, u16 reg, u16 data);
int cps8601_write_word_mask(struct cps8601_dev_info *di, u16 reg, u16 mask, u16 shift, u16 data);
int cps8601_read_block(struct cps8601_dev_info *di, u16 reg, u8 *data, u8 len);
int cps8601_write_block(struct cps8601_dev_info *di, u16 reg, u8 *data, u8 data_len);
int cps8601_hw_read_block(struct cps8601_dev_info *di, u32 addr, u8 *data, u8 len);
int cps8601_hw_write_block(struct cps8601_dev_info *di, u32 addr, u8 *data, u8 data_len);
int cps8601_hw_read_dword(struct cps8601_dev_info *di, u32 addr, u32 *data);
int cps8601_hw_write_dword(struct cps8601_dev_info *di, u32 addr, u32 data);

/* cps8601 common */
void cps8601_chip_enable(bool enable, void *dev_data);
void cps8601_enable_irq(struct cps8601_dev_info *di);
void cps8601_disable_irq_nosync(struct cps8601_dev_info *di);
struct device_node *cps8601_dts_dev_node(void *dev_data);
int cps8601_get_mode(struct cps8601_dev_info *di, u8 *mode);

/* cps8601 chip_info */
int cps8601_get_chip_id(struct cps8601_dev_info *di, u16 *chip_id);
int cps8601_get_chip_info(struct cps8601_dev_info *di, struct cps8601_chip_info *info);
int cps8601_get_chip_info_str(struct cps8601_dev_info *di, char *info_str, int len);

/* cps8601 tx */
unsigned int cps8601_tx_get_bnt_wltx_type(int ic_type);
void cps8601_tx_mode_irq_handler(struct cps8601_dev_info *di);
int cps8601_tx_ops_register(struct wltrx_ic_ops *ops, struct cps8601_dev_info *di);
int cps8601_tx_lowpower_enable(bool enable, void *dev_data);

/* cps8601 fw */
void cps8601_fw_mtp_check_work(struct work_struct *work);
int cps8601_fw_sram_update(void *dev_data);
int cps8601_fw_ops_register(struct wltrx_ic_ops *ops, struct cps8601_dev_info *di);
int cps8601_fw_program_q_data(struct cps8601_dev_info *di);

/* cps8601 qi */
int cps8601_qi_ops_register(struct wltrx_ic_ops *ops, struct cps8601_dev_info *di);

/* cps8601 dts */
int cps8601_parse_dts(struct device_node *np, struct cps8601_dev_info *di);

#endif /* _CPS8601_H_ */
