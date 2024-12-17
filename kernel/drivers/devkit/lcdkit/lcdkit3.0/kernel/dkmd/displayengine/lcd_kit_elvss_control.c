#include "lcd_kit_elvss_control.h"

#include "lcd_kit_utils.h"
#include "lcd_kit_disp.h"
#include <securec.h>
#ifdef CONFIG_POWER_DUBAI
#include <huawei_platform/log/hwlog_kernel.h>
#endif
#define MAX_ELVSS_STEPS 70
static struct dpu_elvss_state g_elvss_state[MAX_PANEL];
static struct dpu_elvss_config g_elvss_config[MAX_PANEL];
static struct bl_elvss_vint2 g_elvss_offset_policy[MAX_PANEL][MAX_ELVSS_STEPS];
static struct bl_elvss_vint2 g_elvss_default_policy[MAX_PANEL][MAX_ELVSS_STEPS];

// XHS_DDIC vint2 table
static struct vint2_coef xhs_vint2_table[] = {
		{0xd5, 62500},
		{0x1f, 15625},
		{0x2, 62500},
		{0x1, 200000},
		{0x0, 0},
};

// RD_DDIC vint2 table
static struct vint2_coef rd_vint2_table[] = {
		{0x4, 10000},
		{0x3, 170000},
		{0x0, 0},
};

int find_elvss_with_bl(struct bl_elvss_vint2 *element_info, struct bl_elvss_vint2 *table)
{
	int i;

	if (!table) {
		LCD_KIT_ERR("elvss1_table is null\n");
		return LCD_KIT_FAIL;
	}

	for (i = 0; i < MAX_ELVSS_STEPS; i++) {
		if (element_info->bl_level >= table[i].bl_level) {
			element_info->elvss_level = table[i].elvss_level;
			element_info->vint2_level = table[i].vint2_level;
			break;
		}
	}

	if (i >= MAX_ELVSS_STEPS) {
		element_info->elvss_level = 0;
		element_info->vint2_level = 0;
	}
	return LCD_KIT_OK;
}

static int cal_vint2_sum(struct dpu_elvss_config *config, uint32_t vint2_start, uint32_t vint2_end)
{
	int i;
	int len;
	uint32_t vint2_tmp;
	uint32_t vint2_sum;
	struct vint2_coef *vint2_table;

	len = config->coef_table_len;
	vint2_table = config->vint2_coef_table;
	vint2_sum = 0;

	if (!vint2_table || len == 0 || vint2_start <= vint2_end) {
		LCD_KIT_INFO("input param is invalid");
		return 0;
	}

	vint2_tmp = vint2_start;
	for (i = 0; i < len; i++) {
		if (vint2_table[i].vint2_level <= vint2_tmp && vint2_table[i].vint2_level <= vint2_end) {
			vint2_sum += (vint2_tmp - vint2_end) * vint2_table[i].coef;
			break;
		}

		if (vint2_table[i].vint2_level <= vint2_tmp && vint2_table[i].vint2_level > vint2_end) {
			vint2_sum += (vint2_tmp - vint2_table[i].vint2_level) * vint2_table[i].coef;
			vint2_tmp = vint2_table[i].vint2_level;
			continue;
		}
	}

	return vint2_sum;
}

static int calculate_vint2(struct dpu_elvss_config *config, int vint2_start, int bias)
{
	int i, j;
	int len;
	int tmp_value = 0;
	int start_index = 0;
	int end_index = 0;
	int dest_value = bias; // 1000 is mv change to uv
	struct vint2_coef *vint2_table;

	len = config->coef_table_len;
	vint2_table = config->vint2_coef_table;
	if (bias == 0 || !vint2_table || len == 0)
		return vint2_start;

	for (i = 0; i < len; i++) {
		if (vint2_table[i].vint2_level <= vint2_start)
			break;
	}

	start_index = vint2_start;
	for (j = i; j < len; j++) {
		tmp_value = (start_index - vint2_table[j].vint2_level) * vint2_table[j].coef;
		if (tmp_value < dest_value) {
			start_index = vint2_table[j].vint2_level;
			dest_value -= tmp_value;
			continue;
		}
		if (vint2_table[j].coef != 0)
			end_index = start_index - (dest_value / vint2_table[j].coef);
		break;
	}
	return end_index;
}

static int cal_bias_data(struct dpu_elvss_config *config, struct bl_elvss_vint2 *max_info, struct bl_elvss_vint2 *min_info)
{
	uint32_t min_offset, max_offset;

	if (!config || !min_info || !max_info) {
		LCD_KIT_ERR("config or data is null\n");
		return LCD_KIT_FAIL;
	}

	if (max_info->bl_level < min_info->bl_level ||
		max_info->elvss_level < min_info->elvss_level || max_info->vint2_level < min_info->vint2_level) {
		LCD_KIT_ERR("max_info less than min_info, invalid input data!\n");
		return LCD_KIT_FAIL;
	}

	min_offset = config->min_offset_value / config->elvss_min_step;
	max_offset = config->max_offset_value / config->elvss_min_step;

	if (max_info->elvss_level <= max_offset || min_info->elvss_level <= min_offset) {
		LCD_KIT_ERR("elvss_level is less than offset!\n");
		return LCD_KIT_FAIL;
	}

	max_info->elvss_level = max_info->elvss_level - max_offset;
	min_info->elvss_level = min_info->elvss_level - min_offset;
	if (max_info->elvss_level < min_info->elvss_level) {
		LCD_KIT_ERR("offset exceeds the predetermined range\n");
		return LCD_KIT_FAIL;
	}

	max_info->vint2_level = calculate_vint2(config, max_info->vint2_level, config->max_offset_value * 1000);
	min_info->vint2_level = calculate_vint2(config, min_info->vint2_level, config->min_offset_value * 1000);
	return LCD_KIT_OK;
}

static int linear_interpolation_calculation_elvss(struct bl_elvss_vint2 *offset_table, struct dpu_elvss_config *config,
	struct bl_elvss_vint2 *max_info, struct bl_elvss_vint2 *min_info, int* index)
{
	int j;
	int i;
	uint32_t el_bias;
	uint32_t dbv_step;
	uint32_t vin_step;
	uint32_t vint2_tmp;
	uint32_t dbv_tmp;

	i = *index;
	if (i >= MAX_ELVSS_STEPS) {
		LCD_KIT_ERR("interpolations exceeds the limit!\n");
		return LCD_KIT_FAIL;
	}

	if (!max_info || !min_info) {
		LCD_KIT_ERR("ptr is null\n");
		return LCD_KIT_FAIL;
	}

	if (max_info->bl_level < min_info->bl_level || max_info->elvss_level < min_info->elvss_level ||
		max_info->vint2_level < min_info->vint2_level) {
		LCD_KIT_ERR("invalid input param!\n");
		return LCD_KIT_FAIL;
	}

	el_bias = max_info->elvss_level - min_info->elvss_level;
	if (el_bias == 0)
		return LCD_KIT_OK;

	dbv_step = (max_info->bl_level - min_info->bl_level) * 10 / el_bias;
	vint2_tmp = cal_vint2_sum(config, max_info->vint2_level, min_info->vint2_level);
	vin_step = vint2_tmp * 10 / el_bias;
	LCD_KIT_DEBUG("dbv_step:%d,vint2_step:%d, vint2_tmp:%d uv", dbv_step, vin_step, vint2_tmp);

	for (j = el_bias - 1; j >= 0; j--) {
		dbv_tmp = dbv_step * j / 10;
		vint2_tmp = vin_step * (el_bias - j) / 10;
		offset_table[i].bl_level = dbv_tmp + min_info->bl_level;
		offset_table[i].elvss_level = j + min_info->elvss_level;
		offset_table[i].vint2_level = calculate_vint2(config, max_info->vint2_level, vint2_tmp);
		if (j == 0)
			offset_table[i].vint2_level = min_info->vint2_level;
		LCD_KIT_INFO("init_offset_table[%d]:{%d, 0x%x, 0x%x}", i, offset_table[i].bl_level,
			offset_table[i].elvss_level, offset_table[i].vint2_level);
		i++;
		if (i >= MAX_ELVSS_STEPS) {
			LCD_KIT_ERR("interpolations exceeds the limit!\n");
			return LCD_KIT_FAIL;
		}
	}
	*index = i;
	return LCD_KIT_OK;
}

static int gen_policy_table(struct bl_elvss_vint2 *offset_table, struct dpu_elvss_config *config,
	struct bl_elvss_vint2 *max_info, struct bl_elvss_vint2 *min_info)
{
	int ret = LCD_KIT_OK;
	int i;
	struct bl_elvss_vint2 *de_table = NULL;
	struct bl_elvss_vint2 restore_bias_data;

	de_table = config->elvss_defalut_table;
	if (!offset_table || !de_table || !max_info || !min_info) {
		LCD_KIT_ERR("table or info is null\n");
		return LCD_KIT_FAIL;
	}

	if (max_info->bl_level < min_info->bl_level || max_info->elvss_level < min_info->elvss_level ||
		max_info->vint2_level < min_info->vint2_level) {
		LCD_KIT_ERR("max_info less than min_info, invalid input data!\n");
		return LCD_KIT_FAIL;
	}

	for (i = 0; i < MAX_ELVSS_STEPS; i++) {
		if (de_table[i].bl_level > config->restore_bias_brightness) {
			offset_table[i].bl_level = de_table[i].bl_level;
			offset_table[i].elvss_level = de_table[i].elvss_level;
			offset_table[i].vint2_level = de_table[i].vint2_level;
			LCD_KIT_INFO("init_offset_table[%d]:{%d, 0x%x, 0x%x}", i, offset_table[i].bl_level,
				offset_table[i].elvss_level, offset_table[i].vint2_level);
		}
		if (de_table[i].bl_level <= config->restore_bias_brightness) {
			offset_table[i].bl_level = config->restore_bias_brightness;
			offset_table[i].elvss_level = de_table[i].elvss_level;
			offset_table[i].vint2_level = de_table[i].vint2_level;
			LCD_KIT_INFO("init_offset_table[%d]:{%d, 0x%x, 0x%x}", i, offset_table[i].bl_level,
				offset_table[i].elvss_level, offset_table[i].vint2_level);
			break;
		}
	}

	restore_bias_data.bl_level = offset_table[i].bl_level;
	restore_bias_data.elvss_level = offset_table[i].elvss_level;
	restore_bias_data.vint2_level = offset_table[i].vint2_level;
	// 1200nit-->1600nit
	i++;
	ret = linear_interpolation_calculation_elvss(offset_table, config, &restore_bias_data, max_info, &i);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("interpolation failed\n");
		return LCD_KIT_FAIL;
	}
	// 500nit-->1200nit
	ret = linear_interpolation_calculation_elvss(offset_table, config, max_info, min_info, &i);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("interpolation failed\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

struct bl_elvss_vint2 *get_elvss_defualt_policy(int panel_id)
{
	if (panel_id >= MAX_PANEL || panel_id < 0) {
		LCD_KIT_ERR("not support panel_id %d \n", panel_id);
		return NULL;
	}

	return g_elvss_default_policy[panel_id];
}

struct bl_elvss_vint2 *get_elvss_offset_policy(int panel_id)
{
	if (panel_id >= MAX_PANEL || panel_id < 0) {
		LCD_KIT_ERR("not support panel_id %d \n", panel_id);
		return NULL;
	}

	return g_elvss_offset_policy[panel_id];
}

struct dpu_elvss_config *get_elvss_config(int panel_id)
{
	if (panel_id >= MAX_PANEL || panel_id < 0) {
		LCD_KIT_ERR("not support panel_id %d \n", panel_id);
		return NULL;
	}

	return &g_elvss_config[panel_id];
}

struct dpu_elvss_state *get_elvss_state(int panel_id)
{
	if (panel_id >= MAX_PANEL || panel_id < 0) {
		LCD_KIT_ERR("not support panel_id %d \n", panel_id);
		return NULL;
	}

	return &g_elvss_state[panel_id];
}

int validate_panel_config(struct dpu_elvss_config *config, int panel_id)
{
	if (!config) {
		LCD_KIT_ERR("elvss_config is null\n");
		return LCD_KIT_FAIL;
	}
	// todo data validate
	config->support = common_info->el_ctrl_info.support;
	config->bl_threshold = common_info->el_ctrl_info.bl_threshold;
	config->als_threshold = common_info->el_ctrl_info.als_threshold; // lux
	config->temp_threshold = common_info->el_ctrl_info.temp_threshold;
	config->elvss_min_step = common_info->el_ctrl_info.elvss_min_step; // mv
	config->diming_fast_mode = common_info->el_ctrl_info.diming_fast_mode;
	config->diming_med_mode = common_info->el_ctrl_info.diming_med_mode;
	config->diming_slow_mode = common_info->el_ctrl_info.diming_slow_mode;
	config->min_offset_brightness = common_info->el_ctrl_info.min_offset_brightness; // dbv
	config->max_offset_brightness = common_info->el_ctrl_info.max_offset_brightness; // dbv
	config->min_offset_value = common_info->el_ctrl_info.min_offset_value; // mv
	config->max_offset_value = common_info->el_ctrl_info.max_offset_value; // mv
	config->restore_bias_brightness = common_info->el_ctrl_info.restore_bias_brightness;

	LCD_KIT_INFO("bl_th:%d bl_max:%d als_th:%d tmp_th:%d step:%d diming_mode:%d-%d-%d off_bl:%d-%d-%d off_val:%d-%d\n",
		config->bl_threshold, common_info->bl_level_max, config->als_threshold, config->temp_threshold,
		config->elvss_min_step, config->diming_fast_mode, config->diming_med_mode, config->diming_slow_mode,
		config->min_offset_brightness, config->max_offset_brightness, config->restore_bias_brightness,
		config->min_offset_value, config->max_offset_value);

	if (config->max_offset_brightness > common_info->bl_level_max)
		config->max_offset_brightness = common_info->bl_level_max;

	if (config->diming_fast_mode == 0 || config->diming_med_mode == 0 ||
		config->diming_slow_mode == 0) {
		LCD_KIT_ERR("diming_mode is not valid\n");
		return LCD_KIT_FAIL;
	}

	if (!config->support || config->min_offset_brightness > config->max_offset_brightness ||
		config->min_offset_value > config->max_offset_value) {
		LCD_KIT_ERR("config data is not valid\n");
		return LCD_KIT_FAIL;
	}

	if (common_info->el_ctrl_info.ddic_type == DDIC_TYPE_CXX) {
		config->vint2_coef_table = rd_vint2_table;
		config->coef_table_len = sizeof(rd_vint2_table) / sizeof(struct vint2_coef);
	} else if (common_info->el_ctrl_info.ddic_type == DDIC_TYPE_HXX) {
		config->vint2_coef_table = xhs_vint2_table;
		config->coef_table_len = sizeof(xhs_vint2_table) / sizeof(struct vint2_coef);
	}

	if (config->coef_table_len == 0 || !config->vint2_coef_table) {
		LCD_KIT_ERR("coef_data is not inited\n");
		return LCD_KIT_FAIL;
	}

	return LCD_KIT_OK;
}

int validate_ddic_elvss_data(struct dpu_elvss_config *config, struct bl_elvss_vint2 *next_data,
	struct bl_elvss_vint2 *pre_data, int panel_id, int index)
{
	if (!config || !next_data || !pre_data) {
		LCD_KIT_ERR("ptr is null\n");
		return LCD_KIT_FAIL;
	}

	if (next_data->elvss_level > common_info->el_ctrl_info.elvss_max ||
		next_data->elvss_level < common_info->el_ctrl_info.elvss_min) {
		LCD_KIT_ERR("elvss data is invalid\n");
		return LCD_KIT_FAIL;
	}

	if (next_data->vint2_level > common_info->el_ctrl_info.vint2_max ||
		next_data->vint2_level < common_info->el_ctrl_info.vint2_min) {
		LCD_KIT_ERR("vint2 data is invalid\n");
		return LCD_KIT_FAIL;
	}

	if (next_data->bl_level > common_info->bl_level_max) {
		LCD_KIT_ERR("bl data is invalid\n");
		return LCD_KIT_FAIL;
	}

	if (index != 0 && next_data->bl_level != 0) {
		if (pre_data->bl_level <= next_data->bl_level || pre_data->elvss_level < next_data->elvss_level ||
			pre_data->vint2_level < next_data->vint2_level) {
			LCD_KIT_ERR("data is invalid\n");
			return LCD_KIT_FAIL;
		}
	}
	LCD_KIT_DEBUG("valid ddic_data[%d]:0x%x-0x%x-0x%x el_vint2_range(0x%x-0x%x 0x%x-0x%x) success\n", index,
		next_data->bl_level, next_data->elvss_level, next_data->vint2_level, common_info->el_ctrl_info.elvss_max,
		common_info->el_ctrl_info.elvss_min, common_info->el_ctrl_info.vint2_max, common_info->el_ctrl_info.vint2_min);

	pre_data->bl_level = next_data->bl_level;
	pre_data->elvss_level = next_data->elvss_level;
	pre_data->vint2_level = next_data->vint2_level;
	return LCD_KIT_OK;
}

static void beta_elvss_init_work(struct work_struct *work)
{
	int ret = LCD_KIT_OK;
	int panel_id = lcd_kit_get_active_panel_id();
	struct dpu_elvss_config *config = container_of(work,
		struct dpu_elvss_config, beta_elvss_work);

	if (!config || config->max_offset_value == 0 ||
		config->panel_id != panel_id) {
		LCD_KIT_ERR("elvss_config or value is null\n");
		common_info->el_ctrl_info.el_init_done = 0;
		return;
	}

	ret = dpu_offset_table_init(panel_id);
	if (ret) {
		LCD_KIT_ERR("dpu_offset_table_init error and return\n");
		common_info->el_ctrl_info.el_init_done = 0;
		return;
	}

	ret = dpu_elvss_state_init(panel_id);
	if (ret) {
		LCD_KIT_ERR("dpu_elvss_state_init error and return\n");
		common_info->el_ctrl_info.el_init_done = 0;
		return;
	}

	common_info->el_ctrl_info.el_init_done = 1;
	LCD_KIT_INFO("repeat elvss_init success!\n");
}

int dpu_elvss_config_init(int panel_id)
{
	int ret = LCD_KIT_OK;
	int i, len;
	struct dpu_elvss_config *config = NULL;
	struct bl_elvss_vint2 next_data = {0};
	struct bl_elvss_vint2 pre_data = {0};

	config = get_elvss_config(panel_id);
	if (!config) {
		LCD_KIT_ERR("elvss_config is null\n");
		return LCD_KIT_FAIL;
	}

	(void)memset_s(config, sizeof(struct dpu_elvss_config), 0, sizeof(struct dpu_elvss_config));
	ret = validate_panel_config(config, panel_id);
	if (ret != 0)
		return LCD_KIT_FAIL;

	if (config->restore_bias_brightness > common_info->bl_level_max)
		config->restore_bias_brightness = common_info->bl_level_max;

	if (config->restore_bias_brightness < config->max_offset_brightness) {
		LCD_KIT_ERR("res_bl is not valid\n");
		return LCD_KIT_FAIL;
	}

	config->elvss_defalut_table = get_elvss_defualt_policy(panel_id);
	if (!config->elvss_defalut_table) {
		LCD_KIT_ERR("elvss_defalut_table is null\n");
		return LCD_KIT_FAIL;
	}

	(void)memset_s(config->elvss_defalut_table, MAX_ELVSS_STEPS * sizeof(struct bl_elvss_vint2), 0,
		MAX_ELVSS_STEPS * sizeof(struct bl_elvss_vint2));

	len = common_info->el_ctrl_info.el_ctrl_forms.el_ctrl_form_len;
	if (len > MAX_ELVSS_STEPS) {
		LCD_KIT_ERR("len exceeds limit\n");
		return LCD_KIT_FAIL;
	}

	// init table
	for (i = 0; i < len; i++) {
		next_data.bl_level = common_info->el_ctrl_info.el_ctrl_forms.lut_form[len - 1 - i];
		next_data.elvss_level = common_info->el_ctrl_info.el_ctrl_forms.elvss_input_form[len - 1 - i];
		next_data.vint2_level = common_info->el_ctrl_info.el_ctrl_forms.vint2_input_form[len - 1 - i];
		config->elvss_defalut_table[i].bl_level = next_data.bl_level;
		config->elvss_defalut_table[i].elvss_level = next_data.elvss_level;
		config->elvss_defalut_table[i].vint2_level = next_data.vint2_level;
		LCD_KIT_DEBUG("de_offset_table[%d]:{%d, 0x%x, 0x%x}", i, config->elvss_defalut_table[i].bl_level,
			config->elvss_defalut_table[i].elvss_level, config->elvss_defalut_table[i].vint2_level);
		ret = validate_ddic_elvss_data(config, &next_data, &pre_data, panel_id, i);
		if (ret != LCD_KIT_OK)
			return LCD_KIT_FAIL;
	}
	INIT_WORK(&config->beta_elvss_work, beta_elvss_init_work);
	return LCD_KIT_OK;
}

int dpu_offset_table_init(int panel_id)
{
	int ret;
	struct bl_elvss_vint2 min_info;
	struct bl_elvss_vint2 max_info;
	struct bl_elvss_vint2 *offset = NULL;
	struct dpu_elvss_config *config = NULL;

	offset = get_elvss_offset_policy(panel_id);
	if (!offset) {
		LCD_KIT_ERR("elvss_offset is null\n");
		return LCD_KIT_FAIL;
	}

	(void)memset_s(offset, MAX_ELVSS_STEPS * sizeof(struct bl_elvss_vint2), 0,
		MAX_ELVSS_STEPS * sizeof(struct bl_elvss_vint2));

	config = get_elvss_config(panel_id);
	if (!config || !config->elvss_defalut_table) {
		LCD_KIT_ERR("elvss_config or elvss_defalut_table is null\n");
		return LCD_KIT_FAIL;
	}

	if (config->elvss_min_step == 0 || config->min_offset_brightness > config->max_offset_brightness) {
		LCD_KIT_ERR("config_data is not valid!\n");
		return LCD_KIT_FAIL;
	}

	min_info.bl_level = config->min_offset_brightness;
	find_elvss_with_bl(&min_info, config->elvss_defalut_table);
	if (min_info.elvss_level == 0 || min_info.vint2_level == 0) {
		LCD_KIT_ERR("find elvss or vint2 is not valid!\n");
		return LCD_KIT_FAIL;
	}

	max_info.bl_level = config->max_offset_brightness;
	find_elvss_with_bl(&max_info, config->elvss_defalut_table);
	if (max_info.elvss_level == 0 || max_info.vint2_level == 0) {
		LCD_KIT_ERR("find elvss or vint2 is not valid!\n");
		return LCD_KIT_FAIL;
	}

	ret = cal_bias_data(config, &max_info, &min_info);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("calculate bias_data error!\n");
		return LCD_KIT_FAIL;
	}

	ret = gen_policy_table(offset, config, &max_info, &min_info);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("generate elvss policy error!\n");
		return LCD_KIT_FAIL;
	}

	return LCD_KIT_OK;
}

int dpu_elvss_state_init(int panel_id)
{
	struct dpu_elvss_state *state = NULL;

	state = get_elvss_state(panel_id);
	if (!state) {
		LCD_KIT_ERR("elvss_state is null\n");
		return LCD_KIT_FAIL;
	}

	(void)memset_s(state, sizeof(struct dpu_elvss_state), 0, sizeof(struct dpu_elvss_state));
	return LCD_KIT_OK;
}

int dpu_elvss_info_init(int panel_id)
{
	int ret = LCD_KIT_OK;

	if (common_info->el_ctrl_info.support != 1 ||
		common_info->el_ctrl_info.el_init_done != 1 ||
		common_info->el_ctrl_info.max_offset_value == 0) {
		LCD_KIT_INFO("El ctrl not support\n");
		common_info->el_ctrl_info.el_init_done = 0;
		return LCD_KIT_FAIL;
	}

	ret = dpu_elvss_config_init(panel_id);
	if (ret) {
		LCD_KIT_ERR("dpu_elvss_config init error and return\n");
		common_info->el_ctrl_info.el_init_done = 0;
		return ret;
	}
	common_info->el_ctrl_info.validate_done = 1;

	if (PUBLIC_INFO->calibrate_level == EL_CTRL_OEM_DEFAULT) {
		LCD_KIT_ERR("invalid oeminfo\n");
		common_info->el_ctrl_info.el_init_done = 0;
		return LCD_KIT_FAIL;
	}

	ret = dpu_offset_table_init(panel_id);
	if (ret) {
		LCD_KIT_ERR("dpu_offset_table_init error and return\n");
		common_info->el_ctrl_info.el_init_done = 0;
		return ret;
	}

	ret = dpu_elvss_state_init(panel_id);
	if (ret) {
		LCD_KIT_ERR("dpu_elvss_state_init error and return\n");
		common_info->el_ctrl_info.el_init_done = 0;
		return ret;
	}

	return ret;
}

void wake_up_dkmd_vsync(int panel_id)
{
	int ret = LCD_KIT_OK;
	unsigned int dsi0_index;
	unsigned int connector_id;

	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is not power on\n");
		return;
	}

	connector_id = DPU_PINFO->connector_id;
	ret = lcd_kit_get_dsi_index(&dsi0_index, connector_id);
	if (ret) {
		LCD_KIT_ERR("get dsi0 index error\n");
		return;
	}

	dkmd_exit_current_vsync_idle_period(dsi0_index, DPU_PINFO->type);
}

bool validate_trigger_beta_condition(int panel_id)
{
	if (common_info->el_ctrl_info.support != 1 ||
		common_info->el_ctrl_info.el_init_done != 0 ||
		common_info->el_ctrl_info.validate_done == 0 ||
		PUBLIC_INFO->calibrate_level != EL_CTRL_OEM_DEFAULT) {
		LCD_KIT_ERR("repeat cond is not satisfied");
		return false;
	}
	return true;
}

void report_dubai_support_info(int panel_id)
{
	uint32_t unspport_flag = 0;
	uint64_t report_time = (uint64_t)ktime_to_ms(ktime_get_real());
	if (common_info->el_ctrl_info.support == 1 && (PUBLIC_INFO->calibrate_level == EL_CTRL_OEM_DEFAULT ||
		PUBLIC_INFO->fake_compatible_panel == 1 || common_info->el_ctrl_info.max_offset_value == 0)) {
#ifdef CONFIG_POWER_DUBAI
		HWDUBAI_LOGE("DUBAI_TAG_EPS_LCD_EL", "into_effect_time=%llu outo_effect_time=%llu total_time=%llu",
			report_time, report_time, unspport_flag);
#endif
		LCD_KIT_INFO("dubai report unsupport flag!");
	}
}

int set_elvss_scene_ctrl(int panel_id, uint32_t ctrl_info)
{
	bool cond = false;
	struct dpu_elvss_state *el_state = NULL;
	struct dpu_elvss_config *config = NULL;

	if (panel_id != lcd_kit_get_active_panel_id()) {
		LCD_KIT_ERR("get panel:%d not match current panel!", panel_id);
		return LCD_KIT_FAIL;
	}

	config = get_elvss_config(panel_id);
	if (!config) {
		LCD_KIT_ERR("elvss_config is null\n");
		return LCD_KIT_FAIL;
	}

	if (!config->first_beta_valid) {
		config->first_beta_valid = true;
#ifdef CONFIG_POWER_DUBAI
		report_dubai_support_info(panel_id);
#endif
		cond = validate_trigger_beta_condition(panel_id);
		if (cond && elvss_is_beta_user()) {
			config->panel_id = panel_id;
			schedule_work(&(config->beta_elvss_work));
			return LCD_KIT_OK;
		}
	}

	if (common_info->el_ctrl_info.support != 1 ||
		common_info->el_ctrl_info.el_init_done != 1) {
		LCD_KIT_DEBUG("El ctrl not support\n");
		return LCD_KIT_OK;
	}

	el_state = get_elvss_state(panel_id);
	if (!el_state) {
		LCD_KIT_ERR("panel:%d elvss_state is null", panel_id);
		return LCD_KIT_FAIL;
	}

	if (ctrl_info == 0)
		el_state->scene_ctrl = true;
	else
		el_state->scene_ctrl = false;

	return LCD_KIT_OK;
}

int lcd_kit_set_lux(int panel_id, uint32_t lux)
{
	uint32_t lux_tmp;
	struct dpu_elvss_state *el_state = NULL;
	struct dpu_elvss_config *config = NULL;

	if (common_info->el_ctrl_info.support != 1 ||
		common_info->el_ctrl_info.el_init_done != 1) {
		LCD_KIT_DEBUG("El ctrl not support\n");
		return LCD_KIT_OK;
	}

	if (panel_id != lcd_kit_get_active_panel_id()) {
		LCD_KIT_ERR("get panel:%d hal_sre_lux not match current panel!", panel_id);
		return LCD_KIT_FAIL;
	}

	el_state = get_elvss_state(panel_id);
	if (!el_state) {
		LCD_KIT_ERR("panel:%d elvss_state is null", panel_id);
		return LCD_KIT_FAIL;
	}

	lux_tmp = el_state->als;
	el_state->als = lux;
	LCD_KIT_INFO("get panel:%d hal_sre_lux:%d", panel_id, lux);

	config = get_elvss_config(panel_id);
	if (!config) {
		LCD_KIT_ERR("elvss_config is null\n");
		return LCD_KIT_FAIL;
	}

	if (config->als_threshold >= lux_tmp && config->als_threshold >= lux) {
		LCD_KIT_DEBUG("lux not change in als_th\n");
		return LCD_KIT_OK;
	}

	if (lux_tmp != el_state->als)
		wake_up_dkmd_vsync(panel_id);

	return LCD_KIT_OK;
}

void diming_update_elvss(struct dpu_elvss_state *el_state, struct bl_elvss_vint2 * tar,
	struct dpu_elvss_config *config)
{
	uint32_t value_diff;
	bool inc_flag; // flag inc or dec
	uint32_t diming_speed_mode = 0;
	uint32_t volatge_offset;
	if (el_state->cur_elvss_state > tar->elvss_level) {
		value_diff = el_state->cur_elvss_state - tar->elvss_level;
		inc_flag = false;
	} else {
		value_diff = tar->elvss_level - el_state->cur_elvss_state;
		inc_flag = true;
	}

	if (value_diff > 15) // "15" represents 750mv
		diming_speed_mode = config->diming_fast_mode;
	else if (value_diff <= 15 && value_diff > 8) // "8" represents 400mv
		diming_speed_mode = config->diming_med_mode;
	else if (value_diff <= 8 && value_diff > 0)
		diming_speed_mode = config->diming_slow_mode;

	if (diming_speed_mode == 0) {
		LCD_KIT_WARNING("dimging speed_mode is 0!\n");
		return;
	}

	if (value_diff <= diming_speed_mode) {
		el_state->exp_elvss_state = tar->elvss_level;
		el_state->exp_vint2_state = tar->vint2_level;
		LCD_KIT_DEBUG("last current:0x%x_0x%x dimming target:0x%x_0x%x\n", el_state->cur_elvss_state,
			el_state->cur_vint2_state, el_state->exp_elvss_state, el_state->exp_vint2_state);
		return;
	}

	if (inc_flag) {
		el_state->exp_elvss_state = diming_speed_mode + el_state->cur_elvss_state;
		volatge_offset = (value_diff - diming_speed_mode) * config->elvss_min_step;
		el_state->exp_vint2_state = calculate_vint2(config, tar->vint2_level, volatge_offset * 1000);
	} else {
		el_state->exp_elvss_state = el_state->cur_elvss_state - diming_speed_mode;
		volatge_offset = diming_speed_mode * config->elvss_min_step;
		el_state->exp_vint2_state = calculate_vint2(config, el_state->cur_vint2_state, volatge_offset * 1000);
	}
	LCD_KIT_DEBUG("current:0x%x_0x%x dimming target:0x%x_0x%x\n", el_state->cur_elvss_state,
		el_state->cur_vint2_state, el_state->exp_elvss_state, el_state->exp_vint2_state);
}

bool validate_condition(struct dpu_elvss_state *el_state, struct dpu_elvss_config *config, uint32_t bl_level)
{
	bool res;

	if (bl_level <= config->bl_threshold || config->als_threshold >= el_state->als || el_state->scene_ctrl) {
		res = false;
		el_state->exp_ddic_enable = false;
	} else {
		res = true;
		el_state->exp_ddic_enable = true;
	}

	LCD_KIT_DEBUG("conditon result:%d,bl:%d-%d,als:%d-%d\n", res, bl_level, config->bl_threshold,
		el_state->als, config->als_threshold);
	return res;
}

void update_elvss_state(struct dpu_elvss_state *el_state, struct dpu_elvss_config *config,
	struct bl_elvss_vint2 *offset, uint32_t bl_level)
{
	bool condition_validate_res;
	struct bl_elvss_vint2 *def_info;
	struct bl_elvss_vint2 *tar_info;

	condition_validate_res = validate_condition(el_state, config, bl_level);
	tar_info = &el_state->tar_info;
	def_info = &el_state->def_info;
	tar_info->bl_level = bl_level;
	def_info->bl_level = bl_level;

	if (condition_validate_res) {
		uint32_t last_elvss_level = tar_info->elvss_level;
		if (bl_level != el_state->bl) {
			find_elvss_with_bl(tar_info, offset);
			find_elvss_with_bl(def_info, config->elvss_defalut_table);
			el_state->exp_elvss_state = tar_info->elvss_level;
			el_state->exp_vint2_state = tar_info->vint2_level;
		}

		if (!el_state->cur_ddic_enable) {
			el_state->exp_elvss_state = def_info->elvss_level;
			el_state->exp_vint2_state = def_info->vint2_level;
			return;
		}
		// '50' is DBV Fluctuation range, '1' is number of voltage steps
		if (el_state->cmds_retry || (abs(bl_level - el_state->bl) > 50 &&
			abs(last_elvss_level - tar_info->elvss_level) > 1)) {
			el_state->force_once_send = true;
			return;
		}

		if (el_state->cur_elvss_state != tar_info->elvss_level || el_state->cur_vint2_state != tar_info->vint2_level)
			diming_update_elvss(el_state, tar_info, config);
	} else {
		if (el_state->cur_ddic_enable) {
			uint32_t last_elvss_level = def_info->elvss_level;
			if (bl_level != el_state->bl) {
				find_elvss_with_bl(tar_info, offset);
				find_elvss_with_bl(def_info, config->elvss_defalut_table);
				el_state->exp_elvss_state = def_info->elvss_level;
				el_state->exp_vint2_state = def_info->vint2_level;
			}
			// '50' is DBV Fluctuation range, '1' is number of voltage steps
			if (el_state->cmds_retry ||
				(abs(bl_level - el_state->bl) > 50 && abs(last_elvss_level - def_info->elvss_level) > 1)) {
				el_state->force_once_send = true;
				return;
			}
				
			if (el_state->cur_elvss_state != def_info->elvss_level ||
				el_state->cur_vint2_state != def_info->vint2_level)
				diming_update_elvss(el_state, def_info, config);
		}
	}
}

struct dpu_elvss_state *get_elvss_info(int panel_id, uint32_t bl_level)
{
	struct dpu_elvss_state *el_state = NULL;
	struct dpu_elvss_config *config = NULL;
	struct bl_elvss_vint2 *offset = NULL;

	el_state = get_elvss_state(panel_id);
	if (!el_state) {
		LCD_KIT_ERR("panel:%d elvss_state is null\n", panel_id);
		return NULL;
	}

	config = get_elvss_config(panel_id);
	if (!config || !config->elvss_defalut_table) {
		LCD_KIT_ERR("elvss_config is null\n");
		return NULL;
	}

	offset = get_elvss_offset_policy(panel_id);
	if (!offset) {
		LCD_KIT_ERR("elvss_offset is null\n");
		return NULL;
	}

	if (bl_level > common_info->bl_level_max)
		bl_level = common_info->bl_level_max;

	if ((el_state->scene_ctrl || bl_level < config->bl_threshold || config->als_threshold >= el_state->als)
		&& !el_state->cur_ddic_enable) {
		el_state->event = SEND_EL_CTRL_NONE;
		el_state->exp_ddic_enable = false;
		return el_state;
	}

	update_elvss_state(el_state, config, offset, bl_level);

	el_state->event = SEND_EL_CTRL_NONE;
	if (el_state->cur_elvss_state != el_state->exp_elvss_state ||
		el_state->cur_vint2_state != el_state->exp_vint2_state) {
		el_state->event = SEND_EL_CTRL_CMD;
		wake_up_dkmd_vsync(panel_id);
		return el_state;
	}

	if (el_state->cur_ddic_enable != el_state->exp_ddic_enable) {
		if (el_state->exp_ddic_enable)
			el_state->event = SEND_OPEN_CMD;
		else
			el_state->event = SEND_CLOSE_CMD;
		wake_up_dkmd_vsync(panel_id);
	}
	return el_state;
}

void reset_elvss_state(int panel_id)
{
	int ret;
	struct dpu_elvss_state *el_state = NULL;
	uint64_t time_diff = 0;
	uint64_t first_into_time = 0;

	el_state = get_elvss_state(panel_id);
	if (!el_state) {
		LCD_KIT_ERR("panel:%d elvss_state is null\n", panel_id);
		return;
	}

	if (el_state->into_effect_time != 0 && el_state->into_effect_time > el_state->outo_effect_time) {
		el_state->outo_effect_time = (uint64_t)ktime_to_ms(ktime_get_real());
		if (el_state->outo_effect_time > el_state->into_effect_time)
			time_diff = el_state->outo_effect_time - el_state->into_effect_time;
		if (time_diff < TIME_SUM_MAX && (el_state->total_effect_time + time_diff < TIME_SUM_MAX))
			el_state->total_effect_time += time_diff;
	}
	time_diff = el_state->total_effect_time;
	first_into_time = el_state->first_into_time;
	LCD_KIT_DEBUG("save total_time:%lld", time_diff);

	ret = memset_s(el_state, sizeof(struct dpu_elvss_state), 0, sizeof(struct dpu_elvss_state));
	if (ret < 0) {
		LCD_KIT_ERR("%s memset elvss_state fail\n", __func__);
		el_state->event = SEND_EL_CTRL_NONE;
		el_state->cur_ddic_enable = false;
		el_state->exp_ddic_enable = false;
		el_state->force_once_send = false;
		el_state->cmds_retry = false;
		el_state->cur_elvss_state = 0;
		el_state->cur_vint2_state = 0;
		el_state->exp_elvss_state = 0;
		el_state->exp_vint2_state = 0;
	}

	el_state->total_effect_time = time_diff;
	el_state->first_into_time = first_into_time;
	LCD_KIT_INFO("reset_elvss_state DDIC:%d-%d\n", el_state->cur_ddic_enable, el_state->exp_ddic_enable);
}

void sensorhub_report_ps_info(int tag, int res, int dis_data)
{
	struct dpu_elvss_state *el_state = NULL;
	int panel_id = 0;
	int primary_tag = 5; // PS_TAG

	if (tag == primary_tag)
		panel_id = 0;
	else
		panel_id = 1; // PS_TAG1

	el_state = get_elvss_state(panel_id);
	if (!el_state) {
		LCD_KIT_ERR("panel elvss_state is null\n");
		return;
	}

	el_state->ps = res;
	LCD_KIT_INFO("get_sensor:%d_ps:%d-%d\n", tag, res, dis_data);
}