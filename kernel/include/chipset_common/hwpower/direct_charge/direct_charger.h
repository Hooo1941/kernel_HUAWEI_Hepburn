/*
 * direct_charger.h
 *
 * direct charger driver
 *
 * Copyright (c) 2020-2023 Huawei Technologies Co., Ltd.
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

#ifndef _DIRECT_CHARGER_H_
#define _DIRECT_CHARGER_H_

#include <linux/module.h>
#include <linux/device.h>
#include <linux/notifier.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/hrtimer.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/pm_wakeup.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/log/hw_log.h>
#include <chipset_common/hwpower/common_module/power_ui_ne.h>
#ifdef CONFIG_SUPERSWITCH_FSC
#include <huawei_platform/usb/superswitch/fsc/core/hw_scp.h>
#endif
#ifdef CONFIG_WIRELESS_CHARGER
#include <huawei_platform/power/wireless/wireless_charger.h>
#endif
#include <chipset_common/hwpower/direct_charge/direct_charge_charging_info.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_control.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_power_supply.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_error_handle.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_device_id.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_para_parse.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_path_switch.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_adapter.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_uevent.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_check.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic_manager.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_cable.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_test.h>
#include <chipset_common/hwpower/direct_charge/multi_ic_check.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_mmi.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_comp.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_dts.h>

#define DC_DMDLOG_SIZE                2048
#define REVERSE_OCP_CNT               3
#define OTP_CNT                       3
#define ADP_OTP_CNT                   3
#define DOUBLE_SIZE                   2

/* sensor_id#scene_id#stage */
#define DC_THERMAL_REASON_SIZE        16

#define DC_ERROR_RESISTANCE           (-99999)
#define DC_MAX_RESISTANCE             10000
#define DC_CHARGE_TYPE_LVC            3
#define DC_CHARGE_TYPE_SC             2
#define DC_CHARGE_TYPE_FCP            4
#define DC_CHARGE_TYPE_SC4            5

#define DC_ERR_CNT_MAX                8
#define DC_MULTI_ERR_CNT_MAX          4
#define DC_OPEN_RETRY_CNT_MAX         3
#define DC_OPEN_CHECK_IC_CYCLE        5
#define DC_FULL_RES_ERR_MAX           6

#define INVALID                       (-1)
#define VALID                         0
#define WAIT_LS_DISCHARGE             200
#define MAX_TIMES_FOR_SET_ADAPTER_VOL 30
#define MIN_CURRENT_FOR_RES_DETECT    800
#define CURRENT_SET_FOR_RES_DETECT    1000

#define ENABLE                        1
#define DISABLE                       0

#define VBUS_ON_THRESHOLD             3000
#define VBAT_VBUS_DIFFERENCE          150
#define KICK_WATCHDOG_TIME            1000
#define WATCHDOG_TIMEOUT              2000
#define BATTERY_CAPACITY_HIGH_TH      95

#define DC_LS_RECOVERY_DELAY          500 /* ms */

#define DC_IN_CHARGING_STAGE          0
#define DC_NOT_IN_CHARGING_STAGE      (-1)

#define DC_IN_CHARGE_DONE_STAGE       0
#define DC_NOT_IN_CHARGE_DONE_STAGE   (-1)

#define DC_SET_DISABLE_FLAGS          1
#define DC_CLEAR_DISABLE_FLAGS        0

#define BASP_PARA_SCALE               100
#define BASP_RATIO_POLICY_ALL         0 /* apply ratio policy for all segments */
#define BASP_RATIO_POLICY_MAX         1 /* apply ratio policy for max current */
#define BASP_PARA_LEVEL 10

#define DC_SC_CUR_LEN                 64

#define SC_IN_OUT_VOLT_RATE           2

/* basp */
#define BASP_VTERM_DEC_MIN            0
#define BASP_VTERM_DEC_MAX            100 /* mv */
#define BASP_VTERM_DEC_DEFAULT        0
#define BASP_ICHG_RATIO_MIN           70
#define BASP_ICHG_RATIO_MAX           100
#define BASP_ICHG_RATIO_DEFAULT       100
#define BASP_VTERM_DEC_DEFAULT        0

#define DC_SINGLEIC_CURRENT_LIMIT     8000
#define DC_MULTI_IC_IBAT_TH           4000
#define DC_CURRENT_OFFSET             300
#define MIN_CURRENT_FOR_MULTI_IC      500
#define DC_MULTI_IC_INFO_IBAT_TH      7000

/* for charge temp hysteresis */
#define DC_LOW_TEMP_MAX               10
#define DC_HIGH_TEMP_MAX              45

enum direct_charge_mode_set {
	AND_SET = 0x0,
	OR_SET = 0x1,
};

/*
 * define stage type with direct charge
 * DC is simplified identifier with direct-charge
 */
enum direct_charge_stage_type {
	DC_STAGE_BEGIN = 0,
	DC_STAGE_DEFAULT = DC_STAGE_BEGIN,
	DC_STAGE_ADAPTER_DETECT,
	DC_STAGE_SWITCH_DETECT,
	DC_STAGE_CHARGE_INIT,
	DC_STAGE_SECURITY_CHECK,
	DC_STAGE_SUCCESS,
	DC_STAGE_CHARGING,
	DC_STAGE_CHARGE_DONE,
	DC_STAGE_END,
};

enum direct_charge_succ_flag {
	DC_SUCCESS,
	DC_ERROR,
};

/*
 * define disable type with direct charge
 * DC is simplified identifier with direct-charge
 */
enum direct_charge_disable_type {
	DC_DISABLE_BEGIN = 0,
	DC_DISABLE_SYS_NODE = DC_DISABLE_BEGIN,
	DC_DISABLE_FATAL_ISC_TYPE,
	DC_DISABLE_WIRELESS_TX,
	DC_DISABLE_BATT_CERTIFICATION_TYPE,
	DC_DISABLE_RSMC, /* rsmc:halifa */
	DC_DISABLE_END,
};

/*
 * define temprature threshold with maximum current
 * support up to 5 parameters list on dts
 */
#define DC_TEMP_LEVEL           5

enum direct_charge_temp_info {
	DC_TEMP_MIN = 0,
	DC_TEMP_MAX,
	DC_TEMP_CUR_MAX,
	DC_TEMP_TOTAL,
};

struct direct_charge_temp_para {
	int temp_min;
	int temp_max;
	int temp_cur_max;
};

/*
 * define multistage (cc)constant current and (cv)constant voltage
 * support up to 5 parameters list on dts
 */
#define DC_VOLT_LEVEL           8

enum direct_charge_volt_info {
	DC_PARA_VOL_TH = 0,
	DC_PARA_CUR_TH_HIGH,
	DC_PARA_CUR_TH_LOW,
	DC_PARA_VOLT_TOTAL,
};

struct direct_charge_volt_para {
	int vol_th;
	int cur_th_high;
	int cur_th_low;
};

/*
 * define voltage parameters of different batteries
 * at different temperature threshold
 * support up to 16 parameters list on dts
 */
#define DC_VOLT_GROUP_MAX       16
#define DC_BAT_BRAND_LEN_MAX    64
#define DC_VOLT_NODE_LEN_MAX    16

enum direct_charge_bat_info {
	DC_PARA_BAT_ID = 0,
	DC_PARA_TEMP_LOW,
	DC_PARA_TEMP_HIGH,
	DC_PARA_INDEX,
	DC_PARA_BAT_TOTAL,
};

struct direct_charge_bat_para {
	int temp_low;
	int temp_high;
	int parse_ok;
	char batid[DC_BAT_BRAND_LEN_MAX];
	char volt_para_index[DC_VOLT_NODE_LEN_MAX];
};

/*
 * define dc time threshold with maximum current
 * support up to 7 parameters list on dts
 */
#define DC_TIME_PARA_LEVEL               7

enum direct_charge_time_info {
	DC_TIME_INFO_TIME_TH,
	DC_TIME_INFO_IBAT_MAX,
	DC_TIME_INFO_MAX,
};

struct direct_charge_time_para {
	int time_th;
	int ibat_max;
};

struct direct_charge_volt_para_group {
	struct direct_charge_volt_para volt_info[DC_VOLT_LEVEL];
	struct direct_charge_bat_para bat_info;
	int stage_size;
};

enum dc_iin_thermal_channel_type {
	DC_CHANNEL_TYPE_BEGIN = 0,
	DC_SINGLE_CHANNEL = DC_CHANNEL_TYPE_BEGIN,
	DC_DUAL_CHANNEL,
	DC_CHANNEL_TYPE_END,
};

#define DC_VSTEP_PARA_LEVEL    4

enum direct_charge_vstep_info {
	DC_VSTEP_INFO_CURR_GAP,
	DC_VSTEP_INFO_VSTEP,
	DC_VSTEP_INFO_MAX,
};

struct direct_charge_vstep_para {
	int curr_gap;
	int vstep;
};

/* define lvc mos info */
struct dc_lvc_mos_para {
	bool enable;
	int is_support;
	int ibat_th;
	int curr_offset;
	int err_cnt_th;
	int err_th;
	int err_cnt;
};

struct direct_charge_device {
	struct device *dev;
	struct hrtimer calc_thld_timer;
	struct hrtimer anomaly_det_timer;
	struct hrtimer control_timer;
	struct hrtimer kick_wtd_timer;
	struct workqueue_struct *charging_wq;
	struct workqueue_struct *kick_wtd_wq;
	struct work_struct calc_thld_work;
	struct work_struct anomaly_det_work;
	struct work_struct control_work;
	struct work_struct fault_work;
	struct work_struct kick_wtd_work;
	struct notifier_block fault_nb;
	struct direct_charge_volt_para volt_para[DC_VOLT_LEVEL];
	struct direct_charge_volt_para orig_volt_para[DC_VOLT_LEVEL];
	struct direct_charge_volt_para_group orig_volt_para_p[DC_VOLT_GROUP_MAX];
	struct direct_charge_temp_para temp_para[DC_TEMP_LEVEL];
	struct direct_charge_time_para time_para[DC_TIME_PARA_LEVEL];
	struct direct_charge_vstep_para vstep_para[DC_VSTEP_PARA_LEVEL];
	struct multi_ic_check_para multi_ic_check_info;
	struct multi_ic_check_mode_para multi_ic_mode_para;
	struct dc_lvc_mos_para lvc_mos_dts;
	struct dc_comp_para comp_para;
	struct nty_data *fault_data;
	int time_para_parse_ok;
	int stage_need_to_jump[2 * DC_VOLT_LEVEL];
	int error_cnt;
	u32 multi_ic_error_cnt;
	int working_mode;
	int cur_mode;
	int local_mode;
	int curr_offset;
	int multi2single_ibat_delta;
	u32 use_5a;
	u32 use_8a;
	int sysfs_enable_charger;
	u32 sysfs_mainsc_enable_charger;
	u32 sysfs_auxsc_enable_charger;
	unsigned int sysfs_disable_charger[DC_DISABLE_END];
	u32 threshold_caculation_interval;
	u32 charge_control_interval;
	u32 force_disable;
	int cur_stage;
	int cur_vbat_th;
	int cur_ibat_th_high;
	int cur_ibat_th_low;
	int multi_ic_ibat_th;
	int vbat;
	int ibat;
	int ibat_abnormal_cnt;
	int ibat_abnormal_th;
	int vadapt;
	int iadapt;
	int tadapt;
	int ls_vbus;
	int ls_ibus;
	int tls;
	int stage_size;
	int stage_group_size;
	int volt_para_test_flag;
	int turbo_test_flag;
	int pre_stage;
	int adaptor_vset;
	int max_tls;
	int adaptor_iset;
	int init_delt_vset;
	int init_delt_iset;
	int init_adapter_vset;
	int delta_err;
	int vstep;
	int scp_stop_charging_flag_info;
	int stop_charging_flag_error;
	int max_dc_bat_vol;
	int min_dc_bat_vol;
	int min_ibat_th;
	int super_ico_current;
	int bst_ctrl;
	int scp_power_en;
	int compensate_r;
	int all_stage_compensate_r_en;
	int compensate_v;
	u32 cc_protect;
	int ls_id;
	const char *ls_name;
	int vol_err_th;
	int need_check_tx_iin;
	int adaptor_leakage_current_th;
	int dc_succ_flag;
	int scp_cable_detect_enable;
	u32 dc_multi_ic_start_time;
	unsigned int charge_fault;
	int adaptor_vendor_id;
	u32 scp_work_on_charger;
	int dc_err_report_flag;
	int dc_open_retry_cnt;
	int reverse_ocp_cnt;
	int otp_cnt;
	int adp_otp_cnt;
	int dc_volt_ratio;
	unsigned int dc_stage;
	int cutoff_normal_flag;
	char dsm_buff[DC_DMDLOG_SIZE];
	int err_type_cnt[DC_EH_END];
	int scp_stop_charging_complete_flag;
	struct wakeup_source *charging_lock;
	int sc_conv_ocp_count;
	int iin_thermal_default;
	int last_basp_level;
	u32 reset_adap_volt_enabled;
	unsigned int vterm_dec;
	unsigned int ichg_ratio;
	int bat_temp_before_charging;
	u32 low_temp_hysteresis;
	u32 orig_low_temp_hysteresis;
	u32 high_temp_hysteresis;
	u32 orig_high_temp_hysteresis;
	u32 startup_iin_limit;
	u32 hota_iin_limit;
	bool nonstd_cable_flag;
	u32 need_wired_sw_off;
	int max_pwr;
	int ui_max_pwr;
	int product_max_pwr;
	bool pri_inversion;
	int cur_inversion;
	u32 adapter_type;
	struct dc_test_info curr_info;
	u32 cc_unsafe_sc_enable;
	bool cc_safe;
	u32 use_higher_vbat;
	u32 tbat_id;
	u32 support_series_bat_mode;
	u32 bat_cell_num;
	u32 charge_time;
	u32 stop_charging_sleep_enable;
	u32 need_boost_5v;
	u32 prot_type;
	u32 recharge_vbat_th;
	u32 battery_connect_mode;
	u32 pmu_mode_set_flag;
	int regulation_by_ibat;
	int vadapt_diff_th;
	int dfg_disable;
	int sysfs_disable;
	u32 wtd_timeout;
	u32 kick_wtd_time;
};
typedef struct aging_safe_policy_tag {
	unsigned int level;       /* basp level */
	unsigned int dc_volt_dec;  /* direct_charge voltage decrease mV */
	unsigned int nondc_volt_dec;  /* non-direct_charge voltage decrease mV */
	unsigned int cur_ratio; /* current discount ratio:cur_ratio/100 */
	unsigned int cur_ratio_policy; /* 0:apply ratio policy for all segments */
					/* 1:apply ratio policy for max current */
	unsigned int learned_fcc; /* used as charging current 1C after fcc learned */
} aging_safe_policy_type;

#ifdef CONFIG_DIRECT_CHARGER
void direct_charge_set_di(struct direct_charge_device *di);
void direct_charge_set_di_by_mode(int mode);
struct direct_charge_device *direct_charge_get_di(void);
struct direct_charge_device *direct_charge_get_di_by_mode(int mode);
unsigned int direct_charge_get_stage_status(void);
unsigned int direct_charge_get_stage_status_by_mode(int mode);
const char *direct_charge_get_stage_status_string(unsigned int stage);
void direct_charge_set_stage_status(unsigned int stage);
void direct_charge_set_stage_status_default(void);
int direct_charge_in_charging_stage(void);
void direct_charge_set_disable_flags(int val, int type);
int direct_charge_get_product_max_current(int mode);
int direct_charge_battery_temp_handler(int value);
void direct_charge_force_disable_dc_path(void);
int direct_charge_init_adapter_and_device(void);
int direct_charge_switch_path_to_normal_charging(void);
int direct_charge_switch_path_to_dc_charging(void);
int direct_charge_open_charging_path(void);
bool direct_charge_get_stop_charging_complete_flag(void);
int direct_charge_detect_adapter_again(void);
void direct_charge_exit(void);
int direct_charge_get_cutoff_normal_flag(void);
void direct_charge_update_cutoff_normal_flag(void);
bool direct_charge_check_timeout(void);
void direct_charge_update_charge_info(void);
int direct_charge_fault_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data);
void direct_charge_stop_charging(void);
void direct_charge_select_charging_param(void);
void direct_charge_select_charging_stage(void);
void direct_charge_soh_policy(void);
int direct_charge_get_vstep(int mode);
bool direct_charge_check_priority_inversion(void);
bool direct_charge_is_priority_inversion(void);
int lvc_get_di(struct direct_charge_device **di);
int lvc_set_disable_flags(int val, int type);
int sc_get_di(struct direct_charge_device **di);
int sc_set_disable_flags(int val, int type);
int sc4_get_di(struct direct_charge_device **di);
int sc4_set_disable_flags(int val, int type);
int direct_charge_get_succ_flag(void);
#else
static inline void direct_charge_set_di(struct direct_charge_device *di)
{
}

static inline void direct_charge_set_di_by_mode(int mode)
{
}

static inline struct direct_charge_device *direct_charge_get_di(void)
{
	return NULL;
}

static inline struct direct_charge_device *direct_charge_get_di_by_mode(int mode)
{
	return NULL;
}

static inline unsigned int direct_charge_get_stage_status(void)
{
	return DC_STAGE_DEFAULT;
}

static inline unsigned int direct_charge_get_stage_status_by_mode(int mode)
{
	return DC_STAGE_DEFAULT;
}

static inline const char *direct_charge_get_stage_status_string(
	unsigned int stage)
{
	return "illegal stage status";
}

static inline void direct_charge_set_stage_status(unsigned int stage)
{
}

static inline void direct_charge_set_stage_status_default(void)
{
}

static inline int direct_charge_in_charging_stage(void)
{
	return -1;
}

static inline void direct_charge_set_disable_flags(int val, int type)
{
}

static inline int direct_charge_get_product_max_current(int mode)
{
	return -1;
}

static inline int direct_charge_battery_temp_handler(int value)
{
	return 0;
}

static inline int direct_charge_get_device_delta_err(int mode)
{
	return -EINVAL;
}

static inline void direct_charge_force_disable_dc_path(void)
{
}

static inline int direct_charge_init_adapter_and_device(void)
{
	return -1;
}

static inline int direct_charge_switch_path_to_normal_charging(void)
{
	return -1;
}

static inline int direct_charge_switch_path_to_dc_charging(void)
{
	return -1;
}

static inline int direct_charge_open_charging_path(void)
{
	return -1;
}

static inline bool direct_charge_get_stop_charging_complete_flag(void)
{
	return true;
}

static inline int direct_charge_detect_adapter_again(void)
{
	return -1;
}

static inline void direct_charge_exit(void)
{
}

static inline int direct_charge_get_cutoff_normal_flag(void)
{
	return -1;
}

static inline void direct_charge_update_cutoff_normal_flag(void)
{
}

static inline bool direct_charge_check_timeout(void)
{
	return false;
}

static inline void direct_charge_update_charge_info(void)
{
}

static inline int direct_charge_fault_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	return NOTIFY_OK;
}

static inline void direct_charge_stop_charging(void)
{
}

static inline void direct_charge_select_charging_param(void)
{
}

static inline void direct_charge_select_charging_stage(void)
{
}

static inline void direct_charge_soh_policy(void)
{
}

static inline int direct_charge_get_vstep(int mode)
{
	return -EINVAL;
}

static inline bool direct_charge_check_priority_inversion(void)
{
	return false;
}

static inline bool direct_charge_is_priority_inversion(void)
{
	return false;
}

static inline int lvc_get_di(struct direct_charge_device **di)
{
	return -1;
}

static inline int lvc_set_disable_flags(int val, int type)
{
	return -1;
}

static inline int sc_get_di(struct direct_charge_device **di)
{
	return -1;
}

static inline int sc_set_disable_flags(int val, int type)
{
	return -1;
}

static inline int sc4_get_di(struct direct_charge_device **di)
{
	return -1;
}

static inline int sc4_set_disable_flags(int val, int type)
{
	return -1;
}

static inline int direct_charge_get_succ_flag(void)
{
	return -1;
}
#endif /* CONFIG_DIRECT_CHARGER */

#endif /* _DIRECT_CHARGER_H_ */
