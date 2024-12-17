/*
 * huawei_charger.h
 *
 * huawei charger driver
 *
 * Copyright (C) 2012-2015 Huawei Technologies Co., Ltd.
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

#ifndef _HUAWEI_CHARGER
#define _HUAWEI_CHARGER

#include <linux/device.h>
#include <linux/notifier.h>
#include <linux/workqueue.h>
#include <linux/power_supply.h>
#include <linux/bitops.h>
#include <linux/time.h>
#include <huawei_platform/usb/hw_typec_dev.h>
#include <huawei_platform/usb/hw_typec_platform.h>
#include <huawei_platform/usb/hw_pd_dev.h>

#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_interface.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>

#include <huawei_platform/power/uvdm_charger/uvdm_charger.h>
#include <huawei_platform/power/huawei_charger_common.h>
#include <huawei_platform/power/huawei_charger_uevent.h>
#include <chipset_common/hwpower/charger/charger_common_interface.h>
#include <chipset_common/hwpower/direct_charge/direct_charger.h>
#include <chipset_common/hwpower/hardware_channel/vbus_channel.h>
#include <chipset_common/hwpower/hardware_channel/vbus_channel_charger.h>
#include <chipset_common/hwpower/protocol/adapter_protocol.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_scp.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_fcp.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_pd.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_uvdm.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>
#include <chipset_common/hwpower/common_module/power_nv.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_ui_ne.h>
#include <chipset_common/hwpower/hardware_monitor/water_detect.h>
#include <chipset_common/hwpower/common_module/power_temp.h>
#include <chipset_common/hwpower/common_module/power_log.h>
#include <chipset_common/hwpower/hardware_monitor/ffc_control.h>
#include <chipset_common/hwpower/adapter/adapter_test.h>
#include <chipset_common/hwpower/buck_charge/buck_charge_ic_manager.h>

/* marco define area */
#ifndef TRUE
#define TRUE                                1
#endif
#ifndef FALSE
#define FALSE                               0
#endif
#define get_index(x)                        ((x) - ERROR_FCP_VOL_OVER_HIGH)
#define ERR_NO_STRING_SIZE                  256
#define CHARGE_DMDLOG_SIZE                  2048
#define CHIP_RESP_TIME                      200

#define TERM_ERR_CNT                        120

/* sensor_id#scene_id#stage */
#define THERMAL_REASON_SIZE                 16

/* charge threshold */
#define NO_CHG_TEMP_LOW                     0
#define NO_CHG_TEMP_HIGH                    50
#define BATT_EXIST_TEMP_LOW                 (-40)
#define DEFAULT_NORMAL_TEMP                 25

/* options of NTC battery temperature */
#define BATTERY_TEMPERATURE_MIN             (-32767)
#define BATTERY_TEMPERATURE_MAX             32767
#define BATTERY_TEMPERATURE_0_C             0
#define BATTERY_TEMPERATURE_5_C             5

#define PD_EVENT_NUM_TH1                    3
#define PD_EVENT_NUM_TH2                    10

#define CHARGING_WORK_TIMEOUT               30000
#define CHARGING_WORK_TIMEOUT1              20000
#define CHARGING_WORK_PDTOSCP_TIMEOUT       1000
#define CHARGING_WORK_WAITPD_TIMEOUT        2000
#define MIN_CHARGING_CURRENT_OFFSET         (-10)
#define BATTERY_FULL_CHECK_TIMIES           2
#define IIN_AVG_SAMPLES                     10
#define IMPOSSIBLE_IIN                      999999
#define CURRENT_FULL_CHECK_TIMES            3
#define DELA_ICHG_FOR_CURRENT_FULL          30
#define CURRENT_FULL_VOL_OFFSET             50
#define CURRENT_FULL_VALID_PERCENT          80
#define CAP_TH_FOR_CURRENT_FULL             80
#define CHARGING_WORK_FAST_TIMES            2
#define SECONDS_TO_MILLISECONDS             1000

/* options of vbus voltage */
#define VBUS_VOL_READ_CNT                   3
#define COOL_LIMIT                          11
#define WARM_LIMIT                          44
#define WARM_CUR_RATIO                      35
#define RATIO_BASE                          100
#define IBIS_RATIO                          120

#define VBUS_VOLTAGE_ABNORMAL_MAX_COUNT     2

/* Type C mode current */
#define TYPE_C_HIGH_MODE_CURR               3000
#define TYPE_C_MID_MODE_CURR                1500
#define TYPE_C_DEFAULT_MODE_CURR            500

#define MAX_EVENT_COUNT                     16
#define EVENT_QUEUE_UNIT                    MAX_EVENT_COUNT

#define CHARGER_SET_DISABLE_FLAGS           1
#define CHARGER_CLEAR_DISABLE_FLAGS         0

#define PD_VOLTAGE_CHANGE_WORK_TIMEOUT      2000

#define IIN_REGL_INTERVAL_DEFAULT           500
#define IIN_REGL_STAGE_MAX                  20

#define NO_EVENT                            (-1)

#define RT_TEST_TEMP_TH                     45

#define BASP_PARA_SCALE                     100

#define SDP_IIN_USB                         475

#define strict_strtol                       kstrtol

enum {
	CHARGE_FULL_CHECK_DEFAULT = 0,
	CHARGE_FULL_CHECK_MULTI_BAT,
	CHARGE_FULL_CHECK_TYPE_END,
};

struct charge_full_check_info {
	int ichg;
	int ichg_avg;
	int bat_exist;
};

enum disable_charger_type {
	CHARGER_SYS_NODE = 0,
	CHARGER_FATAL_ISC_TYPE,
	CHARGER_WIRELESS_TX,
	BATT_CERTIFICATION_TYPE,
	MAX_DISABLE_CHAGER,
};

enum soft_recharge_para {
	RECHARGE_SOC_TH = 0,
	RECHARGE_VOL_TH,
	RECHARGE_CC_TH,
	RECHARGE_TIMES_TH,
	RECHARGE_DMD_SWITCH,
	RECHARGE_NT_SWITCH, /* night time switch */
	RECHARGE_PARA_TOTAL,
};

struct charge_core_data {
	unsigned int iin;
	unsigned int ichg;
	unsigned int vterm;
	unsigned int iin_ac;
	unsigned int ichg_ac;
	unsigned int iin_usb;
	unsigned int ichg_usb;
	unsigned int iin_nonstd;
	unsigned int ichg_nonstd;
	unsigned int iin_bc_usb;
	unsigned int ichg_bc_usb;
	unsigned int iin_vr;
	unsigned int ichg_vr;
	unsigned int iin_pd;
	unsigned int ichg_pd;
	unsigned int iin_fcp;
	unsigned int ichg_fcp;
	unsigned int iin_nor_scp;
	unsigned int ichg_nor_scp;
	unsigned int iin_weaksource;
	unsigned int iterm;
	unsigned int vdpm;
	unsigned int vdpm_control_type;
	unsigned int vdpm_buf_limit;
	unsigned int iin_max;
	unsigned int ichg_max;
	unsigned int otg_curr;
	unsigned int typec_support;
	unsigned int segment_type;
	unsigned int segment_level;
	unsigned int temp_level;
	unsigned int high_temp_limit;
	bool warm_triggered;
#ifdef CONFIG_WIRELESS_CHARGER
	unsigned int iin_wireless;
	unsigned int ichg_wireless;
#endif
	unsigned int vterm_bsoh;
	unsigned int ichg_bsoh;
	unsigned int battery_cell_num;
	unsigned int vterm_low_th;
	unsigned int vterm_high_th;
};

struct charge_iin_regl_lut {
	int total_stage;
	int *iin_regl_para;
};

#ifdef CONFIG_TCPC_CLASS
struct tcpc_device;
#endif
struct charge_device_info {
	struct device *dev;
	struct notifier_block usb_nb;
	struct notifier_block typec_nb;
	struct delayed_work charge_work;
	struct delayed_work plugout_uscp_work;
	struct delayed_work pd_voltage_change_work;
	struct work_struct usb_work;
	struct charge_core_data *core_data;
	struct hrtimer timer;
	struct mutex mutex_hiz;
#ifdef CONFIG_TCPC_CLASS
	struct notifier_block tcpc_nb;
	struct tcpc_device *tcpc;
	unsigned int tcpc_otg;
	struct mutex tcpc_otg_lock;
	struct pd_dpm_vbus_state *vbus_state;
#endif
	unsigned int recharge_para[RECHARGE_PARA_TOTAL];
	unsigned int pd_input_current;
	unsigned int pd_charge_current;
	enum typec_input_current typec_current_mode;
	unsigned int charge_enable;
	unsigned int input_current;
	unsigned int charge_current;
	unsigned int input_typec_current;
	unsigned int charge_typec_current;
	unsigned int enable_current_full;
	unsigned int check_current_full_count;
	unsigned int check_full_count;
	unsigned int start_attemp_ico;
	unsigned int support_standard_ico;
	unsigned int ico_current_mode;
	unsigned int ico_all_the_way;
	unsigned int fcp_vindpm;
	unsigned int hiz_ref;
	unsigned int check_ibias_sleep_time;
	unsigned int need_filter_pd_event;
	unsigned int boost_5v_enable;
	u32 force_disable_dc_path;
	u32 scp_adp_normal_chg;
	u32 startup_iin_limit;
	u32 hota_iin_limit;
#ifdef CONFIG_DIRECT_CHARGER
	int support_scp_power;
#endif
#ifdef CONFIG_WIRELESS_CHARGER
	struct notifier_block wireless_nb;
	int wireless_vbus;
	int otg_channel;
	int gpio_otg_switch;
	unsigned int iin_limit;
#endif
	struct mutex event_type_lock;
	unsigned int charge_done_maintain_fcp;
	unsigned int term_exceed_time;
	unsigned int clear_water_intrused_flag_after_read;
	unsigned int disable_charger[MAX_DISABLE_CHAGER];
	char thermal_reason[THERMAL_REASON_SIZE];
	int avg_iin_ma;
	int max_iin_ma;
	int current_full_status;
#ifdef CONFIG_HUAWEI_YCABLE
	struct notifier_block ycable_nb;
#endif
#ifdef CONFIG_POGO_PIN
	struct notifier_block pogopin_nb;
	struct notifier_block pogopin_otg_typec_chg_nb;
#endif
	int iin_regulation_enabled;
	int iin_regl_interval;
	int iin_now;
	int iin_target;
	struct mutex iin_regl_lock;
	struct charge_iin_regl_lut iin_regl_lut;
	struct delayed_work iin_regl_work;
	u32 increase_term_volt_en;
	bool is_dc_enable_hiz;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	struct timespec64 charge_start_time;
	struct timespec64 charge_done_time;
#else
	struct timeval charge_start_time;
	struct timeval charge_done_time;
#endif
	u32 smart_charge_support;
	u32 term_err_vterm_dec;
	u32 charge_full_check_type;
	u32 monitor_interval;
	bool charge_term_disable;
	bool smart_battery_chargedone;
	bool vterm_dec_flag;
};

#define WEAKSOURCE_TRUE                     1
#define WEAKSOURCE_FALSE                    0
#define FCP_DETECT_DELAY_IN_POWEROFF_CHARGE 2000
#define ASW_PROTECT_IIN_LIMIT               100
/* variable and function declarationn area */
#ifdef CONFIG_HISI_ASW
extern int asw_get_iin_limit(void);
#else
static inline int asw_get_iin_limit(void)
{
	return 0;
}
#endif /* CONFIG_HISI_ASW */

#ifdef CONFIG_HUAWEI_CHARGER_AP
void charge_type_dcp_detected_notify(void);
int charge_set_input_current(int iset);
int huawei_charger_get_dieid(char *dieid, unsigned int len);
#else
static inline void charge_type_dcp_detected_notify(void) { }

static inline int charge_set_input_current(int iset)
{
	return 0;
}
static inline int huawei_charger_get_dieid(char *dieid, unsigned int len)
{
	return 0;
}
#endif /* CONFIG_HUAWEI_CHARGER_AP */

#ifdef CONFIG_TCPC_CLASS
#ifdef CONFIG_HUAWEI_CHARGER_AP
int is_pd_supported(void);
#else
static inline int is_pd_supported(void)
{
	return 0;
}
#endif /* CONFIG_HUAWEI_CHARGER_AP */
#endif /* CONFIG_TCPC_CLASS */

#ifdef CONFIG_HUAWEI_CHARGER_AP
void huawei_charger_input_current_handler(void);
int huawei_charger_get_iin_thermal_charge_type(void);
void huawei_charger_set_input_current(unsigned int input_current);
unsigned int huawei_charger_get_input_current(void);
unsigned int huawei_charger_get_charge_current(void);
bool huawei_charger_get_dc_enable_hiz_status(void);
unsigned int huawei_charger_get_hota_iin_limit(void);
unsigned int huawei_charger_get_startup_iin_limit(void);
void huawei_charger_process_vr_charge_event(void);
bool get_term_err_chrg_en_flag(void);
void charger_dsm_report(int err_no);
int set_charger_disable_flags(int, int);

void water_detection_entrance(void);
int charge_otg_mode_enable(int enable, unsigned int user);
void charge_set_adapter_voltage(int val, unsigned int type,
	unsigned int delay_time);
int charge_set_input_current_max(void);
int charge_set_hiz_enable_by_direct_charge(int enable);
void charge_reset_hiz_state(void);
void charge_request_charge_monitor(void);
struct charge_device_info *huawei_charge_get_di(void);
void wireless_charge_wired_vbus_handler(void);
void emark_detect_complete(void);
unsigned int charge_get_bsoh_vterm(void);
void huawei_charge_into_sleep(void);
void huawei_charge_set_init_crit(void);
void huawei_charge_recharge_get_charge_time(void);
void huawei_charge_vbat_ocp_handler(unsigned int *charge_fault);
void huawei_charge_update_volt_term_data(void);
void huawei_charge_update_iterm(int iterm);
#else
static inline void huawei_charger_input_current_handler(void)
{
}

static inline int huawei_charger_get_iin_thermal_charge_type(void)
{
	return 0;
}

static inline void huawei_charger_set_input_current(unsigned int input_current)
{
}

static inline unsigned int huawei_charger_get_input_current(void)
{
	return 0;
}

static inline unsigned int huawei_charger_get_charge_current(void)
{
	return 0;
}

static inline bool huawei_charger_get_dc_enable_hiz_status(void)
{
	return false;
}

static inline unsigned int huawei_charger_get_hota_iin_limit(void)
{
	return 0;
}

static inline unsigned int huawei_charger_get_startup_iin_limit(void)
{
	return 0;
}

static inline void huawei_charger_process_vr_charge_event(void)
{
}

static inline bool get_term_err_chrg_en_flag(void)
{
	return true;
}

static inline void charger_dsm_report(int err_no)
{
}

static inline int set_charger_disable_flags(int val, int type)
{
	return 0;
}

static inline void water_detection_entrance(void) { }
static inline int charge_otg_mode_enable(int enable, unsigned int user)
{
	return 0;
}
static inline void charge_set_adapter_voltage(int val, unsigned int type,
	unsigned int delay_time) { }
static inline int charge_set_input_current_max(void)
{
	return 0;
}

static inline int charge_set_hiz_enable_by_direct_charge(int enable)
{
	return 0;
}

static inline void charge_reset_hiz_state(void) { }
static inline void charge_request_charge_monitor(void) { }
static inline struct charge_device_info *huawei_charge_get_di(void)
{
	return NULL;
}
static inline void wireless_charge_wired_vbus_handler(void) { }
static inline void emark_detect_complete(void) { }

static inline void huawei_charge_into_sleep(void)
{
}

static inline void huawei_charge_set_init_crit(void)
{
}

static inline void huawei_charge_recharge_get_charge_time(void)
{
}

static inline void huawei_charge_vbat_ocp_handler(unsigned int *charge_fault)
{
}

static inline void huawei_charge_update_volt_term_data(void)
{
}

static inline void huawei_charge_update_iterm(int iterm)
{
}
#endif /* CONFIG_HUAWEI_CHARGER_AP */

#endif /* _HUAWEI_CHARGER */
