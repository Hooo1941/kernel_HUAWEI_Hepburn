

#ifndef __LCD_KIT_ELVSS_CONTROL__
#define __LCD_KIT_ELVSS_CONTROL__
#include <linux/kernel.h>
#include "dkmd_lcd_interface.h"
#if defined(CONFIG_LOG_EXCEPTION) || defined(CONFIG_LOG_USERTYPE)
#include <log/log_usertype.h>
#endif
#define TIME_SUM_MAX 0xFFFFFFFF // ms
#define DUBAI_TIME_LIMIT 30000 // ms
#define EL_CTRL_OEM_DEFAULT 256
struct bl_elvss_vint2 {
	uint32_t bl_level;
	uint32_t elvss_level;
	uint32_t vint2_level;
};

struct vint2_coef {
	uint32_t vint2_level;
	uint32_t coef;
};

struct voltage_register_bias {
	int register_offset;
	int voltage_offset;
};

enum elvss_event {
	SEND_EL_CTRL_NONE,
	SEND_OPEN_CMD,
	SEND_CLOSE_CMD,
	SEND_EL_CTRL_CMD,
};

struct dpu_elvss_state {
	enum elvss_event event;
	uint32_t bl;
	uint32_t als;
	uint32_t ps;
	uint32_t temp;
	bool scene_ctrl;
	bool cur_ddic_enable; // 实时开关状态
	uint32_t cur_elvss_state; // 实时 elvss 状态
	uint32_t cur_vint2_state; // 实时 vint2 状态
	bool exp_ddic_enable; // 实时开关状态
	uint32_t exp_elvss_state; // 预期 elvss 状态
	uint32_t exp_vint2_state; // 预期 vint2 状态
	struct bl_elvss_vint2 def_info; // diming 默认配置
	struct bl_elvss_vint2 tar_info; // diming offset配置
	bool force_once_send;
	bool cmds_retry;
	uint64_t first_into_time;
	uint64_t into_effect_time;
	uint64_t outo_effect_time;
	uint64_t total_effect_time;
};

struct dpu_elvss_config {
	bool support;
	uint32_t bl_threshold;
	uint32_t als_threshold;
	uint32_t temp_threshold;
	uint32_t elvss_min_step;
	uint32_t diming_fast_mode;
	uint32_t diming_med_mode;
	uint32_t diming_slow_mode;
	uint32_t min_offset_brightness;
	uint32_t max_offset_brightness;
	uint32_t restore_bias_brightness;
	uint32_t min_offset_value;
	uint32_t max_offset_value;
	uint32_t panel_id;
	bool first_beta_valid;
	struct bl_elvss_vint2 *elvss_defalut_table;
	int coef_table_len;
	struct vint2_coef *vint2_coef_table;
	struct work_struct beta_elvss_work;
};

void reset_elvss_state(int panel_id);
int lcd_kit_set_lux(int panel_id, uint32_t lux);
int set_elvss_scene_ctrl(int panel_id, uint32_t ctrl_info);
int dpu_elvss_info_init(int panel_id);
int dpu_offset_table_init(int panel_id);
int dpu_elvss_state_init(int panel_id);
struct dpu_elvss_state * get_elvss_state(int panel_id);
struct dpu_elvss_state *get_elvss_info(int panel_id, uint32_t bl_level);
void sensorhub_report_ps_info(int tag, int res, int dis_data);
static inline bool elvss_is_beta_user(void)
{
#if defined(CONFIG_LOG_EXCEPTION) || defined(CONFIG_LOG_USERTYPE)
	return (get_logusertype_flag() == BETA_USER || get_logusertype_flag() == OVERSEA_USER);
#else
	return false;
#endif
}

#endif