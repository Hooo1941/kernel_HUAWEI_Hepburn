#ifndef DUBAI_SENSORHUB_STATS_H
#define DUBAI_SENSORHUB_STATS_H

#include <linux/types.h>

#define PHYSICAL_SENSOR_TYPE_MAX			40

enum {
	TINY_CORE_MODEL_FD = 0,
	TINY_CORE_MODEL_SD,
	TINY_CORE_MODEL_FR,
	TINY_CORE_MODEL_AD,
	TINY_CORE_MODEL_FP,
	TINY_CORE_MODEL_HD,
	TINY_CORE_MODEL_WD,
	TINY_CORE_MODEL_WE,
	TINY_CORE_MODEL_WC,
	TINY_CORE_MODEL_MAX,
};

enum {
	AOD_NORMAL_STATE,
	AOD_LOW_POWER_STATE,
	AOD_STATE_END,
};

enum {
	AOD_SCREEN_1,
	AOD_SCREEN_2,
	AOD_SCREEN_3,
	AOD_SCREEN_MAX,
};

struct swing_data {
	uint64_t front_active_time;
	uint64_t front_software_standby_time;
	uint64_t front_continue_time;
	uint64_t front_eod_als_time;
	uint64_t front_cam_als_time;
	uint64_t front_cam_als_capture_time;
	uint64_t front_eod_darklight_time;
	uint64_t rear_active_time;
	uint64_t rear_swstandby_time;
	uint64_t rear_sleep_time;
	uint64_t rear_actine_time;
	uint32_t rear_move_cnt;
} __packed;

struct cpld_data {
	uint64_t audio_time;
	uint64_t tp_time;
	uint64_t camera_time;
	uint64_t others_time;
	uint32_t ap_time;
	uint32_t modem_time;
	uint32_t m7_time;
	uint32_t coreothers_time;
	uint32_t coretotal;
} __packed;

struct tp_duration {
	uint64_t active_time;
	uint64_t idle_time;
} __packed;

struct sensor_time {
	uint32_t type;
	uint32_t time;
} __packed;

struct sensor_time_transmit {
	int32_t count;
	struct sensor_time data[PHYSICAL_SENSOR_TYPE_MAX];
} __packed;

struct aod_info {
	uint32_t touch_num;
	uint32_t state_time[AOD_STATE_END];
	uint32_t screen_show_time[AOD_SCREEN_MAX];
	uint32_t swing_focus_time;
	uint32_t switch_swing_num;
	uint32_t switch_finger_num;
	uint32_t tof_num;
	uint32_t dss_on_time;
	uint32_t bl_level[10];
} __packed;

struct aod_dd_info {
	uint32_t ddr_dss_lv[25];
} __packed;

struct aod_working_state_monitor {
	struct aod_info info_data;
	struct aod_dd_info dd_info;
} __packed;

struct lcd_state_duration {
	uint32_t normal_on;
	uint32_t low_power;
} __packed;

#ifdef CONFIG_INPUTHUB_20
long dubai_ioctl_sensorhub(unsigned int cmd, void __user *argp);
#else /* !CONFIG_INPUTHUB_20 */
static inline long dubai_ioctl_sensorhub(unsigned int cmd, void __user *argp) { return -EOPNOTSUPP; }
#endif /* !CONFIG_INPUTHUB_20 */

#endif // DUBAI_SENSORHUB_STATS_H
