/*
 * lcd_kit_displayengine.c
 *
 * display engine dkmd function in lcd
 *
 * Copyright (c) 2023-2024 Huawei Technologies Co., Ltd.
 *
 */

#include <linux/kernel.h>
#include <linux/workqueue.h>

#include "lcd_kit_adapt.h"
#include "lcd_kit_common.h"
#include "lcd_kit_utils.h"
#include "lcd_kit_disp.h"
#include "display_engine_interface.h"
#include "display_engine_kernel.h"
#include "lcd_kit_local_hbm.h"
#include "lcd_kit_panel_irc.h"
#include "lcd_kit_panel_pfm.h"
#include "lcd_kit_displayengine.h"
#include "lcd_kit_panel_od.h"
#include "lcd_kit_elvss_control.h"
#include "lcd_kit_panel_temper.h"
#include "lcd_kit_safe_frm_rate.h"

#define SN_TEXT_SIZE 128
/* DDIC deburn-in need 12 RGB weight */
#define DDIC_COMPENSATION_LUT_COUNT 12

struct display_engine_vsync {
	const char *name;
	bool is_inited;
	struct kthread_worker handle_worker;
	struct task_struct *handle_thread;
	struct kthread_work handle_work;
};

static struct display_engine_vsync g_de_vsync_ctrl = {
	.name = "display_engine_vsync",
	.is_inited = false,
	.handle_thread = NULL,
};

struct display_engine_compensation_aging {
	unsigned int support;
	bool is_inited;
	struct mutex lock;
	unsigned int min_hbm_dbv;
	unsigned int orig_level;
	unsigned int mipi_level;
	unsigned int fps_index;
	int last_dbi_set_ret;
	bool is_display_region_enable[DE_REGION_NUM];
	unsigned int data[DE_STATISTICS_TYPE_MAX][DE_REGION_NUM][DE_FPS_MAX];
	struct timeval last_time[DE_STATISTICS_TYPE_MAX][DE_REGION_NUM][DE_FPS_MAX];
};

/* Parameters for compensation AB */
struct display_engine_compensation_ddic_data {
	struct display_engine_roi_param roi_param;
	uint16_t dbi_weight[DE_COLOR_INDEX_COUNT][DDIC_COMPENSATION_LUT_COUNT];
};
 
/* Context for all display engine features, which not care about Panel ID . */
struct display_engine_common_context {
	struct display_engine_compensation_aging aging_data;
	struct display_engine_compensation_ddic_data compensation_data;
};
 
static struct display_engine_common_context g_de_common_context = {
/* {0, 0, 1008, 2232} is GL primary rectangle */
	.compensation_data.roi_param.top = 0,
	.compensation_data.roi_param.bottom = 1008,
	.compensation_data.roi_param.left = 0,
	.compensation_data.roi_param.right = 2232,
};

/* Common function define */
static int copy_buf(char *dst, uint32_t dst_size, char *src, uint32_t src_size);
static int copy_str(char *dst, uint32_t dst_size, char *src, uint32_t src_size);
static uint32_t display_engine_panel_id_to_lcdkit(uint32_t de_id);
static void print_sn_code(unsigned char sn_code[SN_CODE_LEN_MAX], unsigned int sn_code_length);

/* Feature function define */
static int display_engine_panel_info_get(uint32_t panel_id, struct display_engine_panel_info *info);
static int display_engine_foldable_info_get(uint32_t panel_id, struct display_engine_foldable_info *param);
static int display_engine_foldable_info_set(uint32_t panel_id, struct display_engine_foldable_info *param);
static void display_engine_workqueue_init(void);
static bool display_engine_workqueue_init_support(void);
static void display_engine_vsync_workqueue_handler(struct kthread_work *work);
static int display_engine_set_ddic_irc(struct display_engine_param* de_param);
static int display_engine_set_ddic_od(struct display_engine_param* de_param);
static int display_engine_set_ddic_local_hbm_alpha_map(struct display_engine_param* de_param);
static int display_engine_set_ddic_sre_lux(struct display_engine_param* de_param);
static int display_engine_set_ddic_el_switch(struct display_engine_param* de_param);
static int display_engine_set_ddic_foldable_info(struct display_engine_param* de_param);
static int display_engine_set_ddic_temper(struct display_engine_param* de_param);

static int display_engine_set_ddic_irc(struct display_engine_param* de_param)
{
	if (de_param->modules & DISPLAY_ENGINE_DDIC_IRC) {
		if (display_engine_set_param_irc(de_param->panel_id, de_param->ddic_irc_enable)) {
			LCD_KIT_WARNING("display_engine_set_param_irc() failed!\n");
			return -1;
		}
	}
	return 0;
}

static int display_engine_set_ddic_od(struct display_engine_param* de_param)
{
	if (de_param->modules & DISPLAY_ENGINE_DDIC_OD) {
		if (display_engine_set_param_panel_od(de_param->panel_id, de_param->ddic_od_enable)) {
			LCD_KIT_WARNING("display_engine_set_param_od() failed!\n");
			return -1;
		}
	}
	return 0;
}

static int display_engine_set_ddic_local_hbm_alpha_map(struct display_engine_param* de_param)
{
	if (de_param->modules & DISPLAY_ENGINE_LOCAL_HBM_ALPHA_MAP) {
		if (display_engine_set_param_aplha_map(de_param->panel_id, &de_param->alpha_map)) {
			LCD_KIT_WARNING("display_engine_set_param_aplha_map() failed!\n");
			return -1;
		}
	}
	return 0;
}

static int display_engine_set_ddic_sre_lux(struct display_engine_param* de_param)
{
	if (de_param->modules & DISPLAY_ENGINE_SRE_LUX) {
		if (lcd_kit_set_lux(de_param->panel_id, de_param->sre_lux)) {
			LCD_KIT_WARNING("lcd_kit_set_lux() failed!\n");
			return -1;
		}
	}
	return 0;
}

static int display_engine_set_ddic_el_switch(struct display_engine_param* de_param)
{
	if (de_param->modules & DISPLAY_ENGINE_EL_SWITCH) {
		LCD_KIT_INFO("elvss_switch: %d", de_param->elvss_switch);
		if (set_elvss_scene_ctrl(de_param->panel_id, de_param->elvss_switch)) {
			LCD_KIT_WARNING("set_elvss_scene_ctrl() failed!\n");
			return -1;
		}
	}
	return 0;
}

static int display_engine_set_ddic_foldable_info(struct display_engine_param* de_param)
{
	if (de_param->modules & DISPLAY_ENGINE_FOLDABLE_INFO) {
		if (display_engine_foldable_info_set(de_param->panel_id, &de_param->foldable_info)) {
			LCD_KIT_WARNING("display_engine_foldable_info_set() failed!\n");
			return -1;
		}
	}
	return 0;
}

static int display_engine_set_ddic_temper(struct display_engine_param* de_param)
{
	if (de_param->modules & DISPLAY_ENGINE_DDIC_TEMPER) {
		if (display_engine_set_param_panel_temper(de_param->panel_id, de_param->ddic_temper_enable)) {
			LCD_KIT_WARNING("display_engine_set_param_temper() failed!\n");
			return -1;
		}
	}
	return 0;
}

typedef int (*display_engine_set_func)(struct display_engine_param* de_param);

static const display_engine_set_func set_func_list[] = {
	display_engine_set_ddic_irc,
	display_engine_set_ddic_od,
	display_engine_set_ddic_local_hbm_alpha_map,
	display_engine_set_ddic_sre_lux,
	display_engine_set_ddic_el_switch,
	display_engine_set_ddic_foldable_info,
	display_engine_set_ddic_temper,
};

static uint32_t display_engine_panel_id_to_lcdkit(uint32_t de_id)
{
	switch (de_id) {
	case DISPLAY_ENGINE_PANEL_INNER:
		return PRIMARY_PANEL;
	case DISPLAY_ENGINE_PANEL_OUTER:
		return SECONDARY_PANEL;
	default:
		LCD_KIT_WARNING("unknown display engine panel id [%d]!\n", de_id);
		break;
	}
	return PRIMARY_PANEL;
}

int display_engine_get_param(void __user *argp)
{
	size_t size = sizeof(struct display_engine_param);
	struct display_engine_param de_param;
	int ret = 0;
	if (!argp) {
		LCD_KIT_ERR("argp is nullptr\n");
		return -1;
	}
	ret = (int)copy_from_user(&de_param, argp, size);
	if (ret) {
		LCD_KIT_ERR("copy_from_user failed, ret:%d!\n", ret);
		return -1;
	}
	if (de_param.modules & DISPLAY_ENGINE_PANEL_INFO) {
		if (display_engine_panel_info_get(de_param.panel_id, &de_param.panel_info)) {
			LCD_KIT_WARNING("display_engine_panel_info_get() failed!\n");
			return -1;
		}
	}
	if (de_param.modules & DISPLAY_ENGINE_FOLDABLE_INFO) {
		if (display_engine_foldable_info_get(de_param.panel_id, &de_param.foldable_info)) {
			LCD_KIT_WARNING("display_engine_foldable_info_get() failed!\n");
			return -1;
		}
	}

	ret = (int)copy_to_user(argp, &de_param, size);
	if (ret) {
		LCD_KIT_ERR("copy_to_user failed, ret:%d!\n", ret);
		return -1;
	}
	return 0;
}

int display_engine_set_param(void __user *argp)
{
	size_t size = sizeof(struct display_engine_param);
	struct display_engine_param de_param;
	int ret = 0;
	if (!argp) {
		LCD_KIT_ERR("argp is nullptr\n");
		return -1;
	}
	ret = (int)copy_from_user(&de_param, argp, size);
	if (ret) {
		LCD_KIT_ERR("copy_from_user failed, ret:%d!\n", ret);
		return -1;
	}

	int setFuncSize = sizeof(set_func_list) / sizeof(set_func_list[0]);
	for (int i = 0; i < setFuncSize; i++) {
		if (set_func_list[i](&de_param)) {
			return -1;
		}
	}
	return 0;
}

static void print_sn_code(unsigned char sn_code[SN_CODE_LEN_MAX], unsigned int sn_code_length)
{
	char sn_text[SN_TEXT_SIZE];
	uint32_t i;

	if (sn_code_length > SN_CODE_LEN_MAX || sn_code_length >= SN_TEXT_SIZE / 2) {
		LCD_KIT_INFO("sn_code_length:%d is not right, max{SN_CODE_LEN_MAX:%d, SN_TEXT_SIZE/2:%d}\n",
			sn_code_length, SN_CODE_LEN_MAX, SN_TEXT_SIZE / 2);
		return;
	}

	for (i = 0; i < sn_code_length; i++)
		sprintf(sn_text + (i << 1), "%02x", sn_code[i]);

	i <<= 1;
	sn_text[i] = '\0';
	LCD_KIT_DEBUG("sn[%u]=\'%s\'\n", sn_code_length, sn_text);
}

static int copy_buf(char *dst, uint32_t dst_size, char *src, uint32_t src_size)
{
	uint32_t len = (dst_size > src_size) ? src_size : dst_size;

	if (!src || !dst) {
		LCD_KIT_ERR("point is null \n");
		return -1;
	}

	if (dst_size == 0 || src_size == 0) {
		LCD_KIT_WARNING("dst_size:%u, src_size:%u\n", dst_size, src_size);
		return -1;
	}

	memcpy(dst, src, len);
	return (int)len;
}

static int copy_str(char *dst, uint32_t dst_size, char *src, uint32_t src_size)
{
	uint32_t len = (dst_size > src_size) ? (src_size - 1) : (dst_size - 1);

	if (!src || !dst) {
		LCD_KIT_ERR("point is null\n");
		return -1;
	}

	if (dst_size == 0 || src_size == 0) {
		LCD_KIT_WARNING("dst_size:%u, src_size:%u\n", dst_size, src_size);
		return -1;
	}

	strncpy(dst, src, len);
	dst[len] = '\0';
	return (int)len;
}

/* Panel information: */
static int display_engine_panel_info_get(uint32_t panel_id, struct display_engine_panel_info *info)
{
	uint32_t lcd_panel_id = display_engine_panel_id_to_lcdkit(panel_id);
	struct dpu_panel_info *panel_info = lcd_kit_get_dpu_pinfo(lcd_panel_id);
	struct lcd_kit_disp_info *disp_info_p = lcd_kit_get_disp_info(lcd_panel_id);
	int ret = 0;

	if (!panel_info || !info) {
		LCD_KIT_ERR("NULL point: panel_info=%p\n", panel_info);
		return LCD_KIT_FAIL;
	}

	info->width = panel_info->xres;
	info->height = panel_info->yres;
	info->max_backlight = panel_info->bl_info.bl_max;
	info->min_backlight = panel_info->bl_info.bl_min;
	info->max_luminance = disp_info_p->max_luminance;
	info->min_luminance = disp_info_p->min_luminance;
	/* get panel name */
	ret = copy_str(info->panel_name, PANEL_NAME_LEN, common_info->panel_name, DISPLAY_PANEL_NAME_MAX_LEN);
	if (ret < 0)
		LCD_KIT_WARNING("panel_name copy fail, ret:%d", ret);

	/* get panel version */
	ret = copy_str(info->panel_version, PANEL_VERSION_LEN, disp_info_p->panel_version.lcd_panel_version,
		LCD_PANEL_VERSION_SIZE);
	if (ret < 0)
		LCD_KIT_WARNING("panel_version copy fail, ret:%d", ret);

	/* get sn code */
	info->sn_code_length = (disp_info_p->sn_code_length > SN_CODE_LENGTH_MAX) ?
		SN_CODE_LENGTH_MAX : disp_info_p->sn_code_length;
	ret = copy_buf(info->sn_code, SN_CODE_LEN_MAX, disp_info_p->sn_code, info->sn_code_length);
	if (ret < 0) {
		memset(info->sn_code, 0, sizeof(info->sn_code));
		LCD_KIT_INFO("sn is empty, ret:%d\n", ret);
	} else {
		print_sn_code(info->sn_code, info->sn_code_length);
	}

#ifdef LCD_FACTORY_MODE
	info->is_factory = true;
#else
	info->is_factory = false;
#endif
	info->local_hbm_support = display_engine_local_hbm_get_support();
	info->oled_type = common_info->oled_type;
	info->elvss_support = common_info->el_ctrl_info.support;

	LCD_KIT_INFO("panel_id=%u res(w=%u,h=%u) bl(min=%u,max=%u) name=%s local_hbm_support=%d oled_type=%d elvss_support=%d\n",
		panel_id, info->width, info->height, info->min_backlight, info->max_backlight, info->panel_name,
		info->local_hbm_support, info->oled_type, info->elvss_support);
	return LCD_KIT_OK;
}

static inline void do_gettimeofday(struct timeval *tv)
{
	struct timespec64 now;
 
	ktime_get_real_ts64(&now);
	tv->tv_sec = now.tv_sec;
	tv->tv_usec = now.tv_nsec/1000;
}
 
 
static uint32_t calculate_acc_value(uint32_t type, uint32_t region, struct timeval cur_time)
{
	uint64_t delta = 0;
	uint64_t level = g_de_common_context.aging_data.mipi_level;
	uint32_t fps = g_de_common_context.aging_data.fps_index;
	struct timeval last_time = g_de_common_context.aging_data.last_time[type][region][fps];
 
	do_gettimeofday(&cur_time);
	switch (type) {
	case DE_MIPI_DBV_ACC:
		level = (level > 0 && level < g_de_common_context.aging_data.min_hbm_dbv) ? level : 0;
		break;
	case DE_ORI_DBV_ACC:
		level = g_de_common_context.aging_data.orig_level;
		break;
	case DE_SCREEN_ON_TIME:
		level = (level > 0 && level < g_de_common_context.aging_data.min_hbm_dbv) ? 1 : 0;
		break;
	case DE_HBM_ACC:
		level = (level >= g_de_common_context.aging_data.min_hbm_dbv) ? level : 0;
		break;
	case DE_HBM_ON_TIME:
		level = (level > g_de_common_context.aging_data.min_hbm_dbv) ? 1 : 0;
		break;
	default:
		return 0;
	}
	delta = level * ((cur_time.tv_sec - last_time.tv_sec) * 1000 +
		(cur_time.tv_usec - last_time.tv_usec) / 1000);
	LCD_KIT_DEBUG("DE_FC acc_value delta %u, type %u, level %u, fps %u, region %u\n",
		delta, type, level, fps, region);
	return (uint32_t)delta;
}
 
static void update_acc_value(void)
{
	struct timeval cur_time;
	unsigned int type;
	unsigned int region;
	unsigned int fps = g_de_common_context.aging_data.fps_index;
	do_gettimeofday(&cur_time);
	for (region = 0; region < DE_REGION_NUM; region++) {
		if (!g_de_common_context.aging_data.is_display_region_enable[region]) {
			continue;
		}
		for (type = 0; type < DE_STATISTICS_TYPE_MAX; type++) {
			g_de_common_context.aging_data.data[type][region][fps] +=
				calculate_acc_value(type, region, cur_time);
			g_de_common_context.aging_data.last_time[type][region][fps] = cur_time;
		}
	}
}
 
static void update_last_time_value(void)
{
	unsigned int type = 0;
	unsigned int region = 0;
	unsigned int fps = 0;
	struct timeval cur_time;
 
	do_gettimeofday(&cur_time);
	for (type = 0; type < DE_STATISTICS_TYPE_MAX; type++) {
		for (region = 0; region < DE_REGION_NUM; region++) {
			for (fps = 0; fps < DE_FPS_MAX; fps++)
				g_de_common_context.aging_data.last_time[type][region][fps] =
					cur_time;
		}
	}
}
 
static void display_engine_foldable_info_print(uint32_t fps_index, uint32_t region,
	struct display_engine_foldable_info *param)
{
	LCD_KIT_INFO("DE_FC: fps_index:%u,region:%u,mipi %u,ori %u,screen_on %u,hbm %u,hbm_on %u",
		fps_index, region, param->data[DE_MIPI_DBV_ACC][region][fps_index],
		param->data[DE_ORI_DBV_ACC][region][fps_index],
		param->data[DE_SCREEN_ON_TIME][region][fps_index],
		param->data[DE_HBM_ACC][region][fps_index],
		param->data[DE_HBM_ON_TIME][region][fps_index]);
}
 
static int display_engine_foldable_info_get(uint32_t panel_id,
	struct display_engine_foldable_info *param)
{
	uint32_t fps_index;
	uint32_t region;
 
	if (!g_de_common_context.aging_data.support)
		return LCD_KIT_OK;
	if (!g_de_common_context.aging_data.is_inited) {
		LCD_KIT_INFO("not init yet");
		return LCD_KIT_OK;
	}
	mutex_lock(&g_de_common_context.aging_data.lock);
	update_acc_value();
	memcpy(param->data, g_de_common_context.aging_data.data,
		sizeof(param->data));
	memset(g_de_common_context.aging_data.data, 0,
		sizeof(g_de_common_context.aging_data.data));
	update_last_time_value();
	for (fps_index = 0; fps_index < DE_FPS_MAX; fps_index++) {
		for (region = 0; region < DE_REGION_NUM; region++)
			display_engine_foldable_info_print(fps_index, region, param);
	}
 
	mutex_unlock(&g_de_common_context.aging_data.lock);
	return LCD_KIT_OK;
}
 
void display_engine_compensation_set_dbv(uint32_t oir_level, uint32_t mipi_level, uint32_t panel_id)
{
	if (!g_de_common_context.aging_data.support)
		return;
	if (!g_de_common_context.aging_data.is_inited) {
		LCD_KIT_INFO("DE_FC not init yet");
		return;
	}
	/* aging_data.lock is just lock for aging_data updating */
	mutex_lock(&g_de_common_context.aging_data.lock);
	if (g_de_common_context.aging_data.mipi_level == 0) {
		update_last_time_value();
		g_de_common_context.aging_data.mipi_level = mipi_level;
		g_de_common_context.aging_data.orig_level = oir_level;
		mutex_unlock(&g_de_common_context.aging_data.lock);
		return;
	}
	update_acc_value();
	update_last_time_value();
	g_de_common_context.aging_data.mipi_level = mipi_level;
	g_de_common_context.aging_data.orig_level = oir_level;
	LCD_KIT_INFO("DE_FC: set dbv: mipi_level=%d, oir_level=%d", g_de_common_context.aging_data.mipi_level,
		g_de_common_context.aging_data.orig_level);
	mutex_unlock(&g_de_common_context.aging_data.lock);
}
 
static int display_engine_foldable_info_set(uint32_t panel_id,
	struct display_engine_foldable_info *param)
{
	int ret = LCD_KIT_OK;
 
	if (!g_de_common_context.aging_data.support)
		return ret;
	if (!g_de_common_context.aging_data.is_inited) {
		LCD_KIT_INFO("DE_FC not init yet");
		return ret;
	}
	mutex_lock(&g_de_common_context.aging_data.lock);
	update_acc_value();
	update_last_time_value();
	LCD_KIT_INFO("DE_FC region_enable primary:%u, slave:%u, folding:%u\n",
		param->is_region_enable[DE_REGION_PRIMARY],
		param->is_region_enable[DE_REGION_SLAVE],
		param->is_region_enable[DE_REGION_FOLDING]);
	memcpy(g_de_common_context.aging_data.is_display_region_enable,
		param->is_region_enable, sizeof(g_de_common_context.aging_data.is_display_region_enable));
	mutex_unlock(&g_de_common_context.aging_data.lock);
	return ret;
}

static void display_engine_vsync_workqueue_handler(struct kthread_work *work)
{
	display_engine_local_hbm_workqueue_handler(work);
}

static bool display_engine_workqueue_init_support(void)
{
	if (display_engine_local_hbm_get_support())
		return true;
	return false;
}

void display_engine_workqueue_init(void)
{
	if (g_de_vsync_ctrl.is_inited) {
		LCD_KIT_INFO("vsync_ctrl is inited\n");
		return;
	}

	if (!display_engine_workqueue_init_support()) {
		LCD_KIT_WARNING("workqueue init not support\n");
		return;
	}

	kthread_init_worker(&g_de_vsync_ctrl.handle_worker);
	g_de_vsync_ctrl.handle_thread = kthread_create(kthread_worker_fn,
		&g_de_vsync_ctrl.handle_worker, g_de_vsync_ctrl.name);
	if (!g_de_vsync_ctrl.handle_thread) {
		LCD_KIT_ERR("%s failed to create handle_thread!\n",
			g_de_vsync_ctrl.name);
		return;
	}

	kthread_init_work(&g_de_vsync_ctrl.handle_work,
		display_engine_vsync_workqueue_handler);
	wake_up_process(g_de_vsync_ctrl.handle_thread);
	g_de_vsync_ctrl.is_inited = true;
	LCD_KIT_INFO("workqueue inited\n");
}

/* queue work function called in vsync interrrupt and cannot be blocked */
void display_engine_vsync_queue_work(void)
{
	if (!g_de_vsync_ctrl.is_inited) {
		LCD_KIT_DEBUG("vsync_ctrl is not inited\n");
		return;
	}

	kthread_queue_work(&g_de_vsync_ctrl.handle_worker,
		&g_de_vsync_ctrl.handle_work);
}

static int display_engine_common_context_init(int panel_id)
{
	g_de_common_context.aging_data.support = common_info->dbv_stat_support;
	g_de_common_context.aging_data.min_hbm_dbv = common_info->min_hbm_dbv;
	LCD_KIT_INFO("DE_FC: compensation support= %d, min hbm dbv = %d\n",
		g_de_common_context.aging_data.support,
		g_de_common_context.aging_data.min_hbm_dbv);
	if (g_de_common_context.aging_data.support) {
		mutex_init(&g_de_common_context.aging_data.lock);
		g_de_common_context.aging_data.is_inited = true;
		update_last_time_value();
	}
	return LCD_KIT_OK;
}

void lcd_kit_display_engine_param_init(int panel_id, struct device_node *np)
{
	/* parse panel irc */
	lcd_kit_parse_ddic_irc(panel_id, np);
	/* parse dynamic pfm switch */
	lcd_kit_parse_panel_pfm(panel_id, np);
	display_engine_common_context_init(panel_id);
	/* parse panel od param */
	lcd_kit_parse_panel_od(panel_id, np);
	/* parse safe frame decision info */
	lcd_kit_parse_safe_frm_decision_info(panel_id, np);
}

void display_engine_init(void)
{
	display_engine_workqueue_init();
	display_engine_local_hbm_init();
}
