/* SPDX-License-Identifier: GPL-2.0 */
/*
 * mt5806.h
 *
 * mt5806 macro, addr etc.
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

#ifndef _MT5806_H_
#define _MT5806_H_

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
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/wireless_charge/wireless_firmware.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_power_supply.h>
#include <chipset_common/hwpower/common_module/power_bigdata.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_time.h>
#include <chipset_common/hwpower/common_module/power_wakeup.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>
#include <chipset_common/hwpower/protocol/wireless_protocol_qi.h>
#include <chipset_common/hwpower/common_module/power_pinctrl.h>

#include "mt5806_chip.h"

#define MT5806_Q_PINCTRL_LEN                 3
#define MT5806_PINCTRL_LEN                   1
#define MT5806_Q_LONG_PERIOD                 0
#define MT5806_Q_SHORT_PERIOD                1
#define MT5806_Q_FACTORY_WIDTH_LTH           1500
#define MT5806_Q_FACTORY_WIDTH_HTH           2500
#define MT5806_Q_SAMPLE_WIDTH_LTH            1800
#define MT5806_Q_SAMPLE_WIDTH_HTH            2100
#define MT5806_Q_SAMPLE_WIDTH_VAR_TH         10
#define MT5806_Q_FAC_WIDTH_DELTA_LTH         (-70)
#define MT5806_Q_FAC_WIDTH_DELTA_HTH         20
#define MT5806_Q_DYN_WIDTH_DELTA_LTH0        (-70)
#define MT5806_Q_DYN_WIDTH_DELTA_HTH0        (-25)
#define MT5806_Q_DYN_WIDTH_DELTA_LTH1        25
#define MT5806_Q_DYN_WIDTH_DELTA_HTH1        70
#define MT5806_Q_CALI_UNVALID                0
#define MT5806_Q_CALI_DYNAMIC                1
#define MT5806_Q_CALI_FACTORY                2

enum mt5806_q_cali_result {
	MT5806_Q_UNCALIBRATIN = 0,
	MT5806_Q_CALI_FAIL,
	MT5806_Q_CALI_SUCC,
	MT5806_Q_CALIBRATING,
	MT5806_Q_WRITE_MTP,
};

enum mt5806_default_psy_type {
	MT5806_DEFAULT_POWEROFF = 0,
	MT5806_DEFAULT_POWERON,
	MT5806_DEFAULT_LOWPOWER,
};

struct mt5806_chip_info {
	u16 chip_id;
	u8 cust_id;
	u8 hw_id;
	u16 minor_ver;
	u16 major_ver;
};

struct mt5806_global_val {
	bool mtp_chk_complete;
	bool mtp_latest;
	bool tx_calibrate_flag;
	struct hwqi_handle *qi_hdl;
};

struct mt5806_tx_init_para {
	u16 ping_interval;
	u16 ping_freq;
};

struct mt5806_tx_fod_para {
	u16 ploss_rpp_th0;
	u16 ploss_th0;
	u16 ploss_th1;
	u16 ploss_th2;
	u8 ploss_cnt;
};

struct mt5806_tx_fop_para {
	u16 tx_max_fop;
	u16 tx_min_fop;
};

struct mt5806_tx_q_factor_para {
	u16 tx_q_cnt;
	u16 tx_q_width;
	u16 tx_q_cnt_var;
	u16 tx_q_width_var;
	u16 tx_q_fac_width;
	u16 tx_q_dyn_width;
	u16 tx_q_spl_width;
	u16 tx_q_flag;
};

struct mt5806_dev_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	struct delayed_work mtp_check_work;
	struct mutex mutex_irq;
	struct wakeup_source *wakelock;
	struct mt5806_global_val g_val;
	struct mt5806_tx_init_para tx_init_para;
	struct mt5806_tx_fod_para tx_fod;
	struct mt5806_tx_fop_para tx_fop;
	struct mt5806_tx_q_factor_para tx_q_factor;
	unsigned int ic_type;
	unsigned int default_psy_type;
	unsigned int q_cali_dynamic_en;
	bool irq_active;
	int gpio_rx_online;
	int gpio_lowpower;
	int gpio_int;
	int irq_int;
	u32 irq_val;
	int irq_cnt;
	u32 ept_type;
	u16 tx_ocp_th;
	int tx_pocp_th;
	u16 tx_cep_timeout;
	u16 chip_id;
	u8 q_cali_result;
};

/* mt5806 common */
int mt5806_read_byte(struct mt5806_dev_info *di, u16 reg, u8 *data);
int mt5806_write_byte(struct mt5806_dev_info *di, u16 reg, u8 data);
int mt5806_read_word(struct mt5806_dev_info *di, u16 reg, u16 *data);
int mt5806_write_word(struct mt5806_dev_info *di, u16 reg, u16 data);
int mt5806_read_dword(struct mt5806_dev_info *di, u16 reg, u32 *data);
int mt5806_write_dword(struct mt5806_dev_info *di, u16 reg, u32 data);
int mt5806_write_dword_mask(struct mt5806_dev_info *di, u16 reg, u32 mask, u32 shift, u32 data);
int mt5806_read_block(struct mt5806_dev_info *di, u16 reg, u8 *data, u8 len);
int mt5806_write_block(struct mt5806_dev_info *di, u16 reg, u8 *data, u8 data_len);
void mt5806_chip_reset(void *dev_data);
void mt5806_chip_enable(bool enable, void *dev_data);
bool mt5806_is_chip_enable(void *dev_data);
void mt5806_enable_irq(struct mt5806_dev_info *di);
void mt5806_disable_irq_nosync(struct mt5806_dev_info *di);
struct device_node *mt5806_dts_dev_node(void *dev_data);
int mt5806_get_mode(struct mt5806_dev_info *di, u16 *mode);

/* mt5806 chip_info */
int mt5806_get_chip_id(struct mt5806_dev_info *di, u16 *chip_id);
int mt5806_get_chip_fw_version(u8 *data, int len, void *dev_data);

/* mt5806 tx */
unsigned int mt5806_tx_get_bnt_wltx_type(int ic_type);
int mt5806_tx_ops_register(struct wltrx_ic_ops *ops, struct mt5806_dev_info *di);
void mt5806_tx_mode_irq_handler(struct mt5806_dev_info *di);
int mt5806_tx_lowpower_enable(bool enable, void *dev_data);

/* mt5806 fw */
int mt5806_fw_ops_register(struct wltrx_ic_ops *ops, struct mt5806_dev_info *di);
void mt5806_fw_mtp_check_work(struct work_struct *work);
int mt5806_fw_sram_update(void *dev_data);
int mt5806_fw_program_q_data(struct mt5806_dev_info *di);

/* mt5806 qi */
int mt5806_qi_ops_register(struct wltrx_ic_ops *ops, struct mt5806_dev_info *di);

/* mt5806 dts */
int mt5806_parse_dts(struct device_node *np, struct mt5806_dev_info *di);

#endif /* _MT5806_H_ */
