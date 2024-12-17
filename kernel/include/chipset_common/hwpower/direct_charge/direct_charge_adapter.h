/* SPDX-License-Identifier: GPL-2.0 */
/*
 * direct_charge_adapter.h
 *
 * adapter operate for direct charge
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef _DIRECT_CHARGE_ADAPTER_H_
#define _DIRECT_CHARGE_ADAPTER_H_

#include <linux/errno.h>
#include <linux/device.h>
#include <chipset_common/hwpower/protocol/adapter_protocol.h>

#define STEP_VOL_START                      6000

/* define protocol power supply operator for direct charge */
struct dc_pps_ops {
	int (*enable)(int enable);
};

/*
 * define operate type for retry with direct charge
 * DC is simplified identifier with direct-charge
 */
enum dc_adap_retry_operate_type {
	DC_RESET_ADAPTER = 0,
	DC_RESET_MASTER,
	DC_RESET_DPDM,
};

enum dc_adap_inherent_type {
	ADAP_MAX_CUR,
	IS_ABNORMAL_ADP,
	ADAP_ANTIFAKE_RESULT,
	ADAP_ANTIFAKE_FAILED_CNT,
};

enum dc_adap_mode_type {
	ADAP_MAX_TEMP,
	ADAP_MAX_ISET,
	ADAP_ANTIFAKE_EXE_EN,
	ADAP_ANTI_22P5W_STRATEGY,
};

#ifdef CONFIG_DIRECT_CHARGER
int dc_pps_ops_register(struct dc_pps_ops *ops);
void dc_adapter_protocol_power_supply(int enable);
void dc_adpt_clear_inherent_info(void);
int dc_adpt_get_inherent_info(unsigned int type);
int dc_adpt_get_mode_info(int mode, unsigned int type);
void dc_adpt_set_mode_info(int mode, unsigned int type, int val);
int dc_init_adapter(int mode, int init_vset);
int dc_reset_operate(int type);
int dc_get_adapter_type(void);
int dc_get_adapter_max_voltage(int *value);
int dc_get_adapter_min_voltage(int *value);
int dc_get_adapter_max_power(int *max_pwr);
int dc_get_adapter_current_set(int *value);
int dc_get_adapter_max_current(int mode, int value);
bool dc_adpt_cable_undetached(void);
int dc_get_adapter_cable_info(int *curr);
int dc_get_adapter_source_info(unsigned int flag, struct adapter_source_info *data);
int dc_get_adapter_source_id(int *source_id);
int dc_get_adapter_scpb_pwr(struct adp_scpb_pwr_data *scpb_pwr, int *size, int max_size);
bool dc_is_scp_superior(void);
int dc_get_protocol_register_state(void);
int dc_set_adapter_voltage(int mode, int *value);
int dc_set_adapter_voltage_step_by_step(int working_mode, int cur_mode, int start, int end);
int dc_set_adapter_current(int mode, int value);
int dc_set_adapter_output_capability(int vol, int cur, int wdt_time);
int dc_set_adapter_output_enable(int enable);
void dc_set_adapter_default_param(void);
int dc_set_adapter_default(void);
int dc_update_adapter_info(void);
unsigned int dc_adpt_get_adapter_support_mode(void);
int dc_adpt_detect_ping(void);
int dc_check_adapter_antifake(int mode, int ratio);
bool dc_adpt_skip_ileak_check(void);
bool dc_adpt_skip_security_check(int mode);
int dc_adpt_update_delta_err(int mode, int delta_err);
void dc_adpt_init_curr(int mode, int adap_vset, int ratio);
int dc_adpt_get_curr(int mode, int adap_vset);
int dc_adpt_22p5w_det(int mode, int ibat_th_high, int ratio, int cur_stage);
void dc_adpt_init(struct device *dev, int mode);
void dc_adpt_remove(struct device *dev, int mode);
#else
static inline int dc_pps_ops_register(struct dc_pps_ops *ops)
{
	return -EINVAL;
}

static inline void dc_adapter_protocol_power_supply(int enable)
{
}

static inline void dc_adpt_clear_inherent_info(void)
{
}

static inline int dc_adpt_get_inherent_info(unsigned int type)
{
	return 0;
}

static inline int dc_adpt_get_mode_info(int mode, unsigned int type)
{
	return 0;
}

static inline void dc_adpt_set_mode_info(int mode, unsigned int type, int val)
{
}

static inline int dc_init_adapter(int init_vset)
{
	return -EPERM;
}

static inline int dc_reset_operate(int type)
{
	return -EPERM;
}

static inline int dc_get_adapter_type(void)
{
	return 0;
}

static inline int dc_get_adapter_max_voltage(int *value)
{
	return -EPERM;
}

static inline int dc_get_adapter_min_voltage(int *value)
{
	return -EPERM;
}

static inline int dc_get_adapter_max_power(int *max_pwr)
{
	return -EPERM;
}

static inline int dc_get_adapter_current_set(int *value)
{
	return -EPERM;
}

static inline int dc_get_adapter_max_current(int value)
{
	return 0;
}

static inline bool dc_adpt_cable_undetached(void)
{
	return false;
}

static inline int dc_get_adapter_cable_info(int *curr)
{
	return -EPERM;
}

static inline int dc_get_adapter_source_info(unsigned int flag, struct adapter_source_info *data)
{
	return -EPERM;
}

static inline int dc_get_adapter_source_id(int *source_id)
{
	return -EPERM;
}

static inline int dc_get_adapter_scpb_pwr(struct adp_scpb_pwr_data *scpb_pwr, int *size, int max_size)
{
	return -EPERM;
}

static inline bool dc_is_scp_superior(void)
{
	return false;
}

static inline int dc_get_protocol_register_state(void)
{
	return -EPERM;
}

static inline int dc_set_adapter_voltage(int value)
{
	return -EPERM;
}

static inline int dc_set_adapter_voltage_step_by_step(int working_mode, int cur_mode, int start, int end)
{
	return -EPERM;
}

static inline int dc_set_adapter_current(int value)
{
	return -EPERM;
}

static inline int dc_set_adapter_output_capability(int vol, int cur, int wdt_time)
{
	return -EPERM;
}

static inline int dc_set_adapter_output_enable(int enable)
{
	return -EPERM;
}

static inline void dc_set_adapter_default_param(void)
{
}

static inline int dc_set_adapter_default(void)
{
	return -EPERM;
}

static inline int dc_update_adapter_info(void)
{
	return -EPERM;
}

static inline unsigned int dc_adpt_get_adapter_support_mode(void)
{
	return 0;
}

static inline int dc_adpt_detect_ping(void)
{
	return -EPERM;
}

static inline int dc_check_adapter_antifake(int mode, int ratio)
{
	return -EPERM;
}

static inline bool dc_adpt_skip_ileak_check(void)
{
	return false;
}

static inline bool dc_adpt_skip_security_check(int mode)
{
	return false;
}

static inline int dc_adpt_update_delta_err(int mode, int delta_err)
{
	return -EPERM;
}

static inline void dc_adpt_init_curr(int mode, int adap_vset, int ratio)
{
}

static inline int dc_adpt_get_curr(int mode, int adap_vset)
{
	return 0;
}

static inline int dc_adpt_22p5w_det(int mode, int ibat_th_high, int ratio, int cur_stage)
{
	return -EPERM;
}

static inline void dc_adpt_init(struct device *dev, int mode)
{
}

static inline void dc_adpt_remove(struct device *dev, int mode)
{
}

#endif /* CONFIG_DIRECT_CHARGER */

#endif /* _DIRECT_CHARGE_ADAPTER_H_ */
