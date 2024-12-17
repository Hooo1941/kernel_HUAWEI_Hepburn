// SPDX-License-Identifier: GPL-2.0
/* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022.
 * All rights reserved.
 * Description: Noise Calculation and Denoising Algorithm Task
 * Author: yangxiaopeng@huawei.com
 * Create: 2022-11-1
 */
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/rtc.h>
#include <linux/time.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/pvclock_gtod.h>
#include <linux/rtc.h>
#include <linux/suspend.h>
#include <linux/semaphore.h>
#include <vdso/time64.h>
#include <securec.h>
#include <stdint.h>
#include "ext_sensorhub_api.h"
#include "als_matting_algo.h"
#include "image_compositing_algo.h"
#include "matting_algo_task.h"
#include "lcd_kit_drm_panel.h"
#include "lcd_kit_hybrid_core.h"
#include "sde_drm.h"
#include "huawei_ts_kit_hybrid_core.h"
#include <linux/timekeeping32.h>
#include "file_mgr.h"
#include <huawei_platform/log/hw_log.h>
#include "math_calculation.h"
#include "matting_algo_fs_interface.h"
#include "lcd_kit_drm_panel.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG matting_algo_task
HWLOG_REGIST();

#define FILE_PATH_MAX_LEN  (255)
#define MAX_PRINT_BUFFER_LEN (512)
#define MAX_INTERVAL_SEND_MSG_TO_MCU (5)
#define MATTING_ALGO_SUB_CMD_LEN (1)

#define CREAT_FILE_DEFAULT_MODE (0644)

struct semaphore semaphore_frame_crop;
struct semaphore semaphore_als_report;

struct matting_algo_lock_st matting_algo_lock_s;

struct alg_matting_data_param_info_st g_alg_matting_data_param_info = {0};

struct task_struct *g_frame_crop_thread = NULL;
struct task_struct *g_als_value_calc_thread = NULL;

#define MATTINGALG_SERVICE_ID (0x01)
#define CMD_ID_RECEIVE_ALG_PARAM (0x8D)

static struct sid_cid g_mattingalg_mcu_sid_cid[] = {
	{ MATTINGALG_SERVICE_ID, CMD_ID_RECEIVE_ALG_PARAM },
};
static struct subscribe_cmds g_mattingalg_mcu_cmds = {
	g_mattingalg_mcu_sid_cid, sizeof(g_mattingalg_mcu_sid_cid) / sizeof(g_mattingalg_mcu_sid_cid[0]),
};

/* Average time interval 600 milliseconds */
static uint32_t g_max_interval_send_msg_to_mcu = 0;

void matting_algo_lock_st_init(void)
{
	mutex_init(&matting_algo_lock_s.set_frame_crop_state_lock);
	mutex_init(&matting_algo_lock_s.set_start_time_lock);
	mutex_init(&matting_algo_lock_s.set_cut_out_index_lock);
	mutex_init(&matting_algo_lock_s.matting_algo_lux_value_lock);
	mutex_init(&matting_algo_lock_s.matting_algo_brightness_lock);
}

struct matting_algo_lock_st *matting_algo_get_lock_st(void)
{
	return &matting_algo_lock_s;
}

struct semaphore *matting_algo_get_semaphore_frame_crop(void)
{
	return &semaphore_frame_crop;
}

struct semaphore *matting_algo_get_semaphore_als_report(void)
{
	return &semaphore_als_report;
}

struct alg_matting_data_param_info_st *matting_algo_get_data_param_info(void)
{
	return &g_alg_matting_data_param_info;
}

uint64_t get_millisec_timestamp(void)
{
	struct timespec uptime;

	get_monotonic_boottime(&uptime);
	return uptime.tv_sec * MSEC_PER_SEC + uptime.tv_nsec / NSEC_PER_MSEC;
}

bool g_frame_crop_state = true;
bool get_frame_crop_state(void)
{
	return g_frame_crop_state;
}
EXPORT_SYMBOL(get_frame_crop_state);

void set_frame_crop_state(bool state)
{
	mutex_lock(&matting_algo_get_lock_st()->set_frame_crop_state_lock);
	g_frame_crop_state = state;
	mutex_unlock(&matting_algo_get_lock_st()->set_frame_crop_state_lock);
}
EXPORT_SYMBOL(set_frame_crop_state);

static uint64_t g_start_time = 0;
uint64_t matting_algo_get_start_time(void)
{
	return g_start_time;
}

void matting_algo_set_start_time(uint64_t start_time)
{
	mutex_lock(&matting_algo_get_lock_st()->set_start_time_lock);
	g_start_time = start_time;
	mutex_unlock(&matting_algo_get_lock_st()->set_start_time_lock);
}

int32_t g_cut_out_index = ALS_CUTOUT_STOP_STATE_INDEX;
int32_t matting_algo_get_cut_out_index(void)
{
	return g_cut_out_index;
}

void matting_algo_set_cut_out_index(int32_t cut_out_index)
{
	mutex_lock(&matting_algo_get_lock_st()->set_cut_out_index_lock);
	g_cut_out_index = cut_out_index;
	mutex_unlock(&matting_algo_get_lock_st()->set_cut_out_index_lock);
}

static enum matting_algo_lcd_state g_matting_algo_ap_lcd_state = MATTING_ALGO_AP_LCD_INVALID;

static void matting_algo_lcd_state_process(bool ap_lcd_open)
{
	if (ap_lcd_open) {
		// lcd open
		if (g_matting_algo_ap_lcd_state != MATTING_ALGO_AP_LCD_OPEN) {
			matting_algo_set_lux_value(UINT_MAX);
			g_matting_algo_ap_lcd_state = MATTING_ALGO_AP_LCD_OPEN;
			matting_algo_set_start_time(get_millisec_timestamp());
			matting_algo_set_cut_out_index(0);
			set_frame_crop_state(true);
			if (matting_algo_debug_enable())
				hwlog_info("[%s] matting_algo_ap_lcd_state is open\n", __func__);
		}

		return;
	}

	// lcd close
	g_matting_algo_ap_lcd_state = MATTING_ALGO_AP_LCD_CLOSE;
	matting_algo_set_start_time(0);
	matting_algo_set_cut_out_index(ALS_CUTOUT_STOP_STATE_INDEX);
	set_frame_crop_state(false);
	matting_algo_set_lux_value(UINT_MAX);
	matting_algo_set_current_brightness(0);
	matting_algo_clear_calc_cut_out_info();
	matting_algo_clear_last_print_brightness();
	if (matting_algo_debug_enable())
		hwlog_info("[%s] matting_algo_ap_lcd_state is close\n", __func__);
}

int notify_lcd_state_to_matting_algo_task(int sw, uint32_t power_mode)
{
	if (matting_algo_debug_enable())
		hwlog_info("[%s] sw=%d, power_mode=%u\n", __func__, sw, power_mode);

	// sw: 0 not ap; 1 ap;
	// power_mode: SDE_MODE_DPMS_ON lcd open; SDE_MODE_DPMS_OFF lcd close; otherwise in aod
	if ((sw) && (power_mode != SDE_MODE_DPMS_LP1) && (power_mode != SDE_MODE_DPMS_LP2)) {
		// AP and no AOD and lcdopen: The LCD is in AP and in the normal state and not in AOD
		matting_algo_lcd_state_process(true);
		return sw;
	}

	// 1. The LCD is not in AP
	// 2. The LCD is in AP and in AOD (AOD open)
	// 3. The LCD is in AP and not in AOD and close
	matting_algo_lcd_state_process(false);

	return sw;
}

int read_file_to_buffer(uint8_t *data, int data_len, uint8_t *path)
{
	struct file *fp;
	mm_segment_t oldfs;
	loff_t pos;
	int count = 0;

	if (!data || data_len <= 0 || !path) {
		hwlog_err("[%s] inputparam error!\n", __func__);
		return EC_FAILURE;
	}

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	fp = filp_open(path, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		hwlog_err("[%s] open file=%s error no %d!\n", __func__, path, PTR_ERR(fp));
		set_fs(oldfs);
		return EC_FAILURE;
	}

	pos = 0;
	count = vfs_read(fp, data, data_len, &pos);
	// print log, no need return
	if (count < 0)
		hwlog_err("[%s] read error, count=%d!\n", __func__, count);

	filp_close(fp, NULL);
	set_fs(oldfs);

	return 0;
}

void write_file_to_device(uint8_t *data, int data_len, uint8_t *path)
{
	struct file *fp;
	mm_segment_t oldfs;
	loff_t pos;
	int count = 0;

	if (!data || data_len <= 0 || !path) {
		hwlog_err("[%s] inputparam error!\n", __func__);
		return;
	}

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	fp = filp_open(path, O_RDWR|O_CREAT, CREAT_FILE_DEFAULT_MODE);
	if (IS_ERR(fp)) {
		hwlog_err("[%s] open file=%s error no %d!\n", __func__, path, PTR_ERR(fp));
		set_fs(oldfs);
		return;
	}

	pos = 0;
	count = vfs_write(fp, data, data_len, &pos);
	// print log, no need return
	if (count != data_len)
		hwlog_err("[%s] write len error, count=%d!\n", __func__, count);

	filp_close(fp, NULL);
	set_fs(oldfs);
}

static int g_frame_num = 0;
static void write_frame_to_device(uint8_t *image_buffer, int image_len)
{
	struct timeval tv;
	unsigned long local_time;
	struct rtc_time tm;
	const char *dir = "/data/data";
	uint8_t path[FILE_PATH_MAX_LEN] = {0};
	int i;
	int ret;
	struct display_framebuffer_info_st *display_frame_buf_info = matting_algo_get_display_frame_buf_info();
	const struct frame_lyer_priority_st *const_frame_priority = matting_algo_get_const_frame_priority();

	if (!matting_algo_crop_debug_enable()) {
		g_frame_num = 0;
		hwlog_debug("[%s] matting_algo_debug_enable return disable.\n", __func__);
		return;
	}

	g_frame_num++;
	for (i = 0; i < display_frame_buf_info->fb_num; i++) {
		do_gettimeofday(&tv);
		// 60 sec per minute
		local_time = (unsigned int)(tv.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		memset_s(path, sizeof(path), '\0', sizeof(path));
		ret = snprintf_s(path, FILE_PATH_MAX_LEN, FILE_PATH_MAX_LEN - 1,
			"%s/frame-%d-plane-%d_%02d-%02d-%02d-%ld.bin", dir,
			g_frame_num, const_frame_priority->layerinfo[i].plane_id,
			tm.tm_hour, tm.tm_min, tm.tm_sec,
			tv.tv_usec / MSEC_PER_SEC);
		if (ret < 0)
			hwlog_info("[%s] (original image) call snprintf_s return: %d\n", __func__, ret);

		write_file_to_device(display_frame_buf_info->fb_start_addr[i], display_frame_buf_info->fb_length, path);
	}

	if (image_buffer != NULL) {
		do_gettimeofday(&tv);
		// 60 sec per minute
		local_time = (unsigned int)(tv.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		memset_s(path, sizeof(path), '\0', sizeof(path));
		ret = snprintf_s(path, FILE_PATH_MAX_LEN, FILE_PATH_MAX_LEN - 1,
			"%s/frame-%d-image-composited_%002d-%02d-%02d-%ld.bin",
			dir, g_frame_num, tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec / MSEC_PER_SEC);
		if (ret < 0)
			hwlog_info("[%s] (image_buffer) call snprintf_s return: %d\n", __func__, ret);

		write_file_to_device(image_buffer, image_len, path);
	}
}

static int frame_crop_thread(void *arg)
{
	uint8_t *buffer = NULL;
	uint64_t crop_start_time_ms;
	uint64_t crop_end_time_ms;

	do {
		down(&semaphore_frame_crop);

		// Performance statistics, recording start time.
		if (matting_algo_debug_enable())
			crop_start_time_ms = get_millisec_timestamp();

		buffer = image_compositing();

		// when need write frame do it
		write_frame_to_device(buffer, COMPOSITED_FRAME_RGB_BUF_LEN);

		// calc noise for one image
		lcd_calc_screen_noise(buffer);

		// enable crop
		set_frame_crop_state(true);

		if (matting_algo_debug_enable()) {
			// Performance statistics, recording the duration time.
			crop_end_time_ms = get_millisec_timestamp();
			hwlog_info("[%s] Performance statistics [frame_crop], time duration(ms): %d\n", __func__,
				crop_end_time_ms - crop_start_time_ms);
		}
	} while (!kthread_should_stop());

	return 0;
}

static struct qcom_panel_info *get_current_panel_info(void)
{
	return lcm_get_panel_info(lcd_get_active_panel_id());
}

static void get_display_als_data_info(struct qcom_panel_info *panel_info_ptr,
	struct display_als_data_info_st *als_data_ptr)
{
	als_data_ptr->als_lux = panel_info_ptr->matting_algo_als_param.als_lux;
	memcpy_s(als_data_ptr->als_gain, sizeof(als_data_ptr->als_gain),
		panel_info_ptr->matting_algo_als_param.als_again,
		sizeof(panel_info_ptr->matting_algo_als_param.als_again));

	als_data_ptr->als_atime = panel_info_ptr->matting_algo_als_param.als_atime;

	memcpy_s(als_data_ptr->als_channel, sizeof(als_data_ptr->als_channel),
		panel_info_ptr->matting_algo_als_param.als_channel,
		sizeof(panel_info_ptr->matting_algo_als_param.als_channel));
	als_data_ptr->timestamp_start = matting_algo_get_start_time();
	als_data_ptr->timestamp_end = get_millisec_timestamp();
}

static int get_alg_matting_data_param_from_mcu(void)
{
	int ret;
	unsigned char buff[MATTING_ALGO_SUB_CMD_LEN] = { GET_ALG_MAT_PARAM_CMD };
	struct command cmd;

	cmd.service_id = MATTINGALG_SERVICE_ID;
	cmd.command_id = CMD_ID_RECEIVE_ALG_PARAM;
	cmd.send_buffer = buff;
	cmd.send_buffer_len = sizeof(buff);

	ret = send_command(MATTINGALG_CHANNEL, &cmd, false, NULL);
	if (ret < 0)
		hwlog_err("[%s] ap send message to mcu failed\n", __func__);

	return ret;
}

static bool is_bypass_als_value_calc(void)
{
	if (matting_algo_get_cut_out_index() < 0) {
		hwlog_debug("[%s] cut_out_index < 0, so no need process\n", __func__);
		return true;
	}

	if (!get_alg_matting_data_param_valid()) {
		g_max_interval_send_msg_to_mcu++;
		if (g_max_interval_send_msg_to_mcu >= MAX_INTERVAL_SEND_MSG_TO_MCU) {
			g_max_interval_send_msg_to_mcu = 0;
			get_alg_matting_data_param_from_mcu();
			hwlog_info("[%s] send msg to get data param from mcu!\n", __func__);
		}
		return true;
	}

	return false;
}

static int als_value_calc_thread(void *arg)
{
	uint32_t lux_value;
	struct qcom_panel_info *panel_info;
	struct display_als_data_info_st als_data;
	uint64_t crop_start_time_ms;
	uint64_t crop_end_time_ms;

	do {
		down(&semaphore_als_report);

		if (is_bypass_als_value_calc()) {
			hwlog_debug("[%s] is_bypass_als_value_calc is true, so no need process\n", __func__);
			continue;
		}

		g_max_interval_send_msg_to_mcu = 0;

		// Performance statistics, recording start time.
		if (matting_algo_debug_enable()) {
			crop_start_time_ms = get_millisec_timestamp();
			crop_end_time_ms = 0;
		}

		panel_info = get_current_panel_info();
		if (panel_info == NULL) {
			hwlog_err("[%s] panel_info is NULL\n", __func__);
			continue;
		}

		get_display_als_data_info(panel_info, &als_data);
		if (als_data.als_lux == 0) {
			hwlog_debug("[%s] src lux is 0, so no need process\n", __func__);
			continue;
		}

		lux_value = lcd_dest_als_value_calc(&als_data);
		matting_algo_set_start_time(als_data.timestamp_end);
		matting_algo_set_cut_out_index(0);

		matting_algo_set_lux_value(lux_value);

		hwlog_info("[%s] lux_value: %d -> %d\n", __func__, als_data.als_lux, lux_value);
		if (matting_algo_debug_enable()) {
			// Performance statistics, recording the duration time.
			crop_end_time_ms = get_millisec_timestamp();
			hwlog_info("[%s] Performance statistics [als_value_calc], time duration(ms): %d\n",
				__func__, crop_end_time_ms - crop_start_time_ms);
		}
	} while (!kthread_should_stop());

	return 0;
}

bool matting_algo_debug_enable(void)
{
	bool debug_enable;
	struct qcom_panel_info *panel_info = get_current_panel_info();

	if (panel_info == NULL) {
		hwlog_err("[%s] panel_info is NULL\n", __func__);
		return false;
	}

	debug_enable = panel_info->matting_algo_debug == 0 ? false : true;
	hwlog_debug("[%s] is %s\n", __func__, debug_enable ? "true" : "false");

	return debug_enable;
}

bool matting_algo_crop_debug_enable(void)
{
	bool crop_debug_enable;

	struct qcom_panel_info *panel_info = get_current_panel_info();

	if (panel_info == NULL) {
		hwlog_err("[%s] panel_info is NULL\n", __func__);
		return false;
	}

	crop_debug_enable = panel_info->matting_algo_crop_debug == 0 ? false : true;
	hwlog_debug("[%s] is %s\n", __func__, crop_debug_enable ? "true" : "false");

	return crop_debug_enable;
}

static bool alg_matting_data_param_valid = false;

void set_alg_matting_data_param_valid(bool valid)
{
	alg_matting_data_param_valid = valid;
}

bool get_alg_matting_data_param_valid(void)
{
	return alg_matting_data_param_valid;
}

static void show_alg_matting_data_param_info(void)
{
	int i;
	int j;
	struct alg_matting_data_param_info_st *data_param_info = matting_algo_get_data_param_info();

	for (i = 0; i < SENSORMGR_ALS_CHANNEL_MAX; i++) {
		for (j = 0; j < RGB_CURVE; j++)
			hwlog_info("[%s] dbv_coef channel=%d color=%d coef=%d,%d,%d\n",
				__func__, i, j, data_param_info->dbv_coef[i][j][INDEX_ZERO],
				data_param_info->dbv_coef[i][j][INDEX_ONE],
				data_param_info->dbv_coef[i][j][INDEX_TWO]);
	}
	hwlog_info("[%s] m=%d, x=%d, y=%d, w=%d, h=%d\n", __func__,
		data_param_info->multiples, data_param_info->offset_x,
		data_param_info->offset_y, data_param_info->fov_width,
		data_param_info->fov_height);
	hwlog_info("[%s] d_g=%d,%d,%d,%d,%d\n", __func__,
		data_param_info->default_gain[INDEX_ZERO], data_param_info->default_gain[INDEX_ONE],
		data_param_info->default_gain[INDEX_TWO], data_param_info->default_gain[INDEX_THREE],
		data_param_info->default_gain[INDEX_FOUR]);
	hwlog_info("[%s] als_type=%u, als_offset=%u\n", __func__,
		data_param_info->als_type_e, data_param_info->als_offset_s.als_offset);
}

void set_alg_matting_data_param_info(void *param)
{
	g_alg_matting_data_param_info = *((struct alg_matting_data_param_info_st *)param);

	show_alg_matting_data_param_info();
}

struct alg_matting_data_param_info_st *get_alg_matting_data_param_info(void)
{
	return &g_alg_matting_data_param_info;
}

static int matting_algo_parse_mcu_subcmd(uint8_t *data, int32_t data_len)
{
	int ret;
	enum matting_algo_ctr_cmd_type sub_cmd_type;

	if (data_len <= 1) {
		hwlog_err("[%s] data_len=%d error\n", __func__, data_len);
		return -EINVAL;
	}
	sub_cmd_type = (enum matting_algo_ctr_cmd_type)data[ALG_MAT_SUBCMD];

	switch (sub_cmd_type) {
	case SEND_ALG_MAT_PARAM_CMD:
		set_alg_matting_data_param_info(&data[ALG_MAT_CMD_VALUES]);
		set_alg_matting_data_param_valid(true);
		ret = 0;
		break;
	default:
		ret = EC_FAILURE;
		break;
	}

	hwlog_info("[%s] data_len=%d, sub_cmd_type=%d, ret=%d\n", __func__, data_len, sub_cmd_type, ret);
	return ret;
}

int32_t matting_algo_task_recv_param_from_mcu(uint8_t service_id,
	uint8_t command_id, uint8_t *data, int32_t data_len)
{
	int ret;
	struct alg_matting_data_param_info_st *data_param_info = matting_algo_get_data_param_info();

	if ((service_id != MATTINGALG_SERVICE_ID) && (command_id != CMD_ID_RECEIVE_ALG_PARAM)) {
		hwlog_err("[%s] service_id=%d or command_id=%d err\n", __func__, service_id, command_id);
		return -EINVAL;
	}

	// prase params, and check subcommond
	ret = matting_algo_parse_mcu_subcmd(data, data_len);
	if (ret < 0) {
		hwlog_err("[%s] matting_algo_parse_subcmd return ret=%d\n", __func__, ret);
		return ret;
	}

	data_param_info->fov_width = data_param_info->fov_width > DIS_ALS_CUTOUT_VALUE ? \
		DIS_ALS_CUTOUT_VALUE : data_param_info->fov_width;
	data_param_info->fov_height = data_param_info->fov_height > DIS_ALS_CUTOUT_VALUE ? \
		DIS_ALS_CUTOUT_VALUE : data_param_info->fov_height;

	return 0;
}

uint32_t g_matting_algo_current_brightnesss = 0;
uint32_t matting_algo_get_current_brightness(void)
{
	return g_matting_algo_current_brightnesss;
}

void matting_algo_set_current_brightness(uint32_t brightness)
{
	mutex_lock(&matting_algo_get_lock_st()->matting_algo_brightness_lock);
	g_matting_algo_current_brightnesss = brightness;
	mutex_unlock(&matting_algo_get_lock_st()->matting_algo_brightness_lock);
}

uint32_t matting_algo_get_current_brightness_callback(uint32_t brightness)
{
	matting_algo_set_current_brightness(brightness);
	return brightness;
}

static int __init matting_algo_task_init(void)
{
	int ret;

	sema_init(&semaphore_frame_crop, 0);
	sema_init(&semaphore_als_report, 0);

	matting_algo_lock_st_init();

	// register listening the message from MCU
	// MATTINGALG_CHANNEL
	ret = register_data_callback(MATTINGALG_CHANNEL, &g_mattingalg_mcu_cmds,
		matting_algo_task_recv_param_from_mcu);
	if (ret < 0) {
		hwlog_err("[%s] regist MATTINGALG_CHANNEL failed\n", __func__);
		return ret;
	}

	g_frame_crop_thread = kthread_run(frame_crop_thread, NULL, "frame_crop_thread");
	if (!g_frame_crop_thread) {
		hwlog_err("[%s] create thread frame_crop_thread faild.\n", __func__);
		return EC_FAILURE;
	}

	g_als_value_calc_thread = kthread_run(als_value_calc_thread, NULL, "als_value_calc_thread");
	if (!g_als_value_calc_thread) {
		hwlog_err("[%s] create thread als_value_calc_thread faild.\n", __func__);
		return EC_FAILURE;
	}
	hwlog_info("[%s] create frame_crop_thread and als_value_calc_thread sucess.", __func__);

	ret = lcd_kit_hybrid_mipi_state_register(notify_lcd_state_to_matting_algo_task);
	if (ret)
		hwlog_err("[%s] lcd_kit_hybrid_mipi_state_register return %d\n", __func__, ret);

	ret = lcd_kit_matting_algo_get_brightness_register(matting_algo_get_current_brightness_callback);
	if (ret)
		hwlog_err("[%s] lcd_kit_matting_algo_get_brightness_register return %d\n", __func__, ret);

	return ret;
}

static void __exit matting_algo_task_exit(void)
{
	if (g_frame_crop_thread)
		kthread_stop(g_frame_crop_thread);

	if (g_als_value_calc_thread)
		kthread_stop(g_als_value_calc_thread);

	unregister_data_callback(MATTINGALG_CHANNEL);
}

late_initcall(matting_algo_task_init);
module_exit(matting_algo_task_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei matting_algo_task driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
