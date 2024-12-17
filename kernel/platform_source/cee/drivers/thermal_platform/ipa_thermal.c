/*
 * ipa_thermal.c
 *
 * thermal ipa framework
 *
 * Copyright (C) 2017-2020 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "ipa_thermal.h"
#include "hotplug_thermal.h"
#include <linux/cpu_cooling.h>
#include <linux/debugfs.h>
#include <linux/cpufreq.h>
#include <linux/cpumask.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/thermal.h>
#include <linux/topology.h>
#include <trace/events/thermal_power_allocator.h>
#include <trace/events/ipa_thermal_trace.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/cpu.h>
#include <linux/kthread.h>
#include <linux/sched/task.h>
#include <uapi/linux/sched/types.h>
#include <securec.h>
#include <linux/of_platform.h>
#ifdef CONFIG_THERMAL_SPM
#include <linux/string.h>
#endif
#include <platform_include/cee/linux/lpm_thermal.h>
#ifdef CONFIG_IPA_MNTN_INFO
#include <linux/io.h>
#include <thermal_common.h>
#include <m3_rdr_ddr_map.h>
#endif

#ifdef CONFIG_IPA_MNTN_INFO
void __iomem *g_ipa_lpmcu_freq_limit_base;
#endif

#if ((defined CONFIG_IPA_THERMAL) && (defined CONFIG_LIBLINUX))
#define MAX_TEMP 145000
#define MIN_TEMP -40000
s32 thermal_zone_temp_check(s32 temperature)
{
	if (temperature > MAX_TEMP)
		temperature = MAX_TEMP;
	else if (temperature < MIN_TEMP)
		temperature = MIN_TEMP;
	return temperature;
}
#endif

struct capacitances g_caps;
struct thermal g_ipa_thermal_info;
#ifdef CONFIG_THERMAL_HOTPLUG
struct thermal_hotplug g_thermal_hotplug;
EXPORT_SYMBOL(g_thermal_hotplug);
#endif
u32 g_cluster_num = 2;
u32 g_ipa_sensor_num = 3;
u32 g_ipa_cpufreq_table_index[CAPACITY_OF_ARRAY];
const char *g_ipa_actor_name[CAPACITY_OF_ARRAY];
u32 g_ipa_actor_index[CAPACITY_OF_ARRAY];

typedef int (*ipa_get_sensor_id_t)(const char *);
struct ipa_sensor_info {
	const char *ipa_sensor_name;
	ipa_get_sensor_id_t ipa_get_sensor_id;
};

unsigned int g_ipa_actor_num = 3;
unsigned int g_weights_profiles[2][CAPACITY_OF_ARRAY] = {{0}, {0}};

#ifndef CONFIG_LIBLINUX
struct ipa_sensor_info g_ipa_sensor_info[NUM_SENSORS] = {
	{
		.ipa_sensor_name = IPA_SENSOR,
		.ipa_get_sensor_id = (ipa_get_sensor_id_t)ipa_get_tsensor_id
	},
#ifdef CONFIG_THERMAL_SHELL
	{
		.ipa_sensor_name = IPA_SENSOR_SHELL,
		.ipa_get_sensor_id = (ipa_get_sensor_id_t)ipa_get_tsensor_id
	},
#endif
	{
		.ipa_sensor_name = IPA_SENSOR_SYSTEM_H,
		.ipa_get_sensor_id = (ipa_get_sensor_id_t)ipa_get_periph_id
	},
	{
		.ipa_sensor_name = IPA_SENSOR_VIRTUAL,
		.ipa_get_sensor_id = (ipa_get_sensor_id_t)ipa_get_tsensor_id
	}
};

get_power_t g_get_board_power[NUM_BOARD_CDEV] = {
	get_camera_power,
	get_backlight_power,
	get_charger_power
};
#endif

u32 get_camera_power(void)
{
	return 0;
}

u32 get_backlight_power(void)
{
	return 0;
}

u32 get_charger_power(void)
{
	return 0;
}

static int get_sensor_id_by_name(const char *name)
{
	int ret = -EINVAL;
#ifndef CONFIG_LIBLINUX
	unsigned long i;
#endif

	if (!strncmp(name, IPA_SENSOR, strlen(IPA_SENSOR))) {
		pr_info("%s: name %s match, use id:%d\n",
			__func__, name, IPA_SENSOR_MAXID);
		return IPA_SENSOR_MAXID;
	} else if (!strncmp(name, IPA_SENSOR_VIRTUAL,
						strlen(IPA_SENSOR_VIRTUAL))) {
		pr_info("%s: name %s match, use id:%d\n",
			__func__, name, IPA_SENSOR_VIRTUALID);
		return IPA_SENSOR_VIRTUALID;
#ifdef CONFIG_THERMAL_SHELL
	} else if (!strncmp(name, IPA_SENSOR_SHELL,
						strlen(IPA_SENSOR_SHELL))) {
		pr_info("%s: name %s match, use id:%d\n",
				__func__, name, IPA_SENSOR_SHELLID);
		return IPA_SENSOR_SHELLID;
#endif
	}
#ifndef CONFIG_LIBLINUX
	for (i = 0; i < ARRAY_SIZE(g_ipa_sensor_info); i++) {
		if (strncmp(name, g_ipa_sensor_info[i].ipa_sensor_name,
					strlen(g_ipa_sensor_info[i].ipa_sensor_name)) == 0)
			ret = g_ipa_sensor_info[i].ipa_get_sensor_id(name);
	}
#endif
	return ret;
}

#ifdef CONFIG_THERMAL_SPM
static bool setfreq_available(unsigned int idx, u32 freq)
{
#ifndef CONFIG_LIBLINUX
	struct cpufreq_frequency_table *pos = NULL;
	struct cpufreq_policy *policy = NULL;
	bool ret = false;

	if (idx < g_cluster_num) {
		/* cpu core */
		policy = cpufreq_cpu_get(g_ipa_cpufreq_table_index[idx]);
		if (policy == NULL || policy->freq_table == NULL) {
			pr_err("%s: Unable to find freq table\n", __func__);
			return false;
		}
		cpufreq_for_each_valid_entry(pos, policy->freq_table) { /*lint !e613*/
			if (pos != NULL && freq == pos->frequency) /*lint !e613*/
				ret = true;
		}
		if (ret != true)
			pr_err("idx %d : freq %d don't match any available freq.\n ",
			       idx, freq);
	} else {
		ret = true; /* GPU */
	}

	return ret;
#else
	return true;
#endif
}

unsigned int g_powerhal_actor_num = 3;
unsigned int g_powerhal_profiles[3][CAPACITY_OF_ARRAY] = {{0}, {0}, {0}};

/* cppcheck-suppress */
#define show_mode(mode_name)					\
static ssize_t show_##mode_name					\
(struct device *dev, struct device_attribute *attr, char *buf)	\
{								\
	if (dev == NULL || attr == NULL || buf == NULL)		\
		return 0;					\
								\
	return snprintf_s(buf, (unsigned long)MAX_SHOW_STR,	\
			  (unsigned long)(MAX_SHOW_STR - 1), "%d\n",	\
			  (int)g_ipa_thermal_info.spm.mode_name);	\
}

show_mode(vr_mode);
show_mode(spm_mode);

/* cppcheck-suppress */
#define store_mode(mode_name)					\
static ssize_t store_##mode_name				\
(struct device *dev, struct device_attribute *attr,		\
	const char *buf, size_t count)				\
{								\
	u32 mode_name = 0;					\
								\
	if (dev == NULL || attr == NULL || buf == NULL)		\
		return 0;					\
								\
	if (kstrtouint(buf, DECIMAL, &mode_name))		\
		return -EINVAL;					\
								\
	g_ipa_thermal_info.spm.mode_name = mode_name ? true : false;\
								\
	cpufreq_update_policies(true);				\
	gpufreq_update_policies(true);				\
	return (ssize_t)count;					\
}

store_mode(vr_mode);
store_mode(spm_mode);

/*lint -e84 -e846 -e514 -e778 -e866 -esym(84,846,514,778,866,*)*/
#define mode_attr_rw(mode_name)				\
static DEVICE_ATTR(mode_name, S_IWUSR | S_IRUGO,	\
		   show_##mode_name, store_##mode_name)

mode_attr_rw(vr_mode);
mode_attr_rw(spm_mode);
/*lint -e84 -e846 -e514 -e778 -e866 +esym(84,846,514,778,866,*)*/

static ssize_t vr_freq_store(struct device *dev, struct device_attribute *attr,
			     const char *buf, size_t count)
{
	u32 freq;
	int ret;
	unsigned int i = 0;
	char *token = NULL;
	char temp_buf[MAX_DEV_NUM] = {0};
	char *s = NULL;

	if (dev == NULL || attr == NULL || buf == NULL)
		return 0;

	ret = strncpy_s(temp_buf, sizeof(temp_buf), buf, sizeof(temp_buf) - 1);
	if (ret != EOK)
		return ret;
	s = temp_buf;

	for (token = strsep(&s, ","); (token != NULL) && (i < g_ipa_actor_num);
	     token = strsep(&s, ","), i++) {
		if (kstrtouint(token, DECIMAL, &freq))
			return -EINVAL;
		if (setfreq_available(i, freq))
			g_powerhal_profiles[1][i] = freq;
	}
	cpufreq_update_policies(true);
	gpufreq_update_policies(true);
	return (ssize_t)count;
}

static ssize_t get_freq_show(struct device *dev, struct device_attribute *devattr,
			     char *buf, int pwrhal_prf_idx)
{
	unsigned int i;
	ssize_t ret = 0;
	int rc;

	if (dev == NULL || devattr == NULL || buf == NULL)
		return 0;
	for (i = 0; i < g_ipa_actor_num; i++) {
		rc = snprintf_s(buf + ret, PAGE_SIZE - ret, FREQ_BUF_LEN, "%u%s",
				g_powerhal_profiles[pwrhal_prf_idx][i], ",");
		if (rc < 0) {
			pr_err("%s: snprintf_s error\n", __func__);
			return rc;
		}
		ret += rc;
	}

	rc = snprintf_s(buf + ret - 1, PAGE_SIZE - ret, 1UL, "\n");
	if (rc < 0) {
		pr_err("%s: snprintf_s error\n", __func__);
		return rc;
	}
	ret += rc;
	return ret;
}

static ssize_t vr_freq_show(struct device *dev,
			    struct device_attribute *devattr, char *buf)
{
	return get_freq_show(dev, devattr, buf, PROFILE_VR);
}
static DEVICE_ATTR(vr_freq, S_IWUSR | S_IRUGO, vr_freq_show, vr_freq_store);

static ssize_t spm_freq_store(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t count)
{
	u32 freq;
	unsigned int i = 0;
	char *token = NULL;
	char temp_buf[MAX_DEV_NUM] = {0};
	char *s = NULL;
	int ret;

	if (dev == NULL || attr == NULL || buf == NULL)
		return 0;

	ret = strncpy_s(temp_buf, sizeof(temp_buf), buf, sizeof(temp_buf) - 1);
	if (ret != EOK)
		return ret;
	s = temp_buf;

	for (token = strsep(&s, ","); (token != NULL) && (i < g_ipa_actor_num);
	     token = strsep(&s, ","), i++) {
		if (kstrtouint(token, DECIMAL, &freq))
			return -EINVAL;
		if (setfreq_available(i, freq))
			g_powerhal_profiles[0][i] = freq;
	}
	cpufreq_update_policies(true);
	gpufreq_update_policies(true);
	return (ssize_t)count;
}

static ssize_t spm_freq_show(struct device *dev, struct device_attribute *devattr, char *buf)
{
	return get_freq_show(dev, devattr, buf, PROFILE_SPM);
}
static DEVICE_ATTR(spm_freq, S_IWUSR | S_IRUGO, spm_freq_show, spm_freq_store);

static ssize_t min_freq_store(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t count)
{
	u32 freq;
	unsigned int i = 0;
	char *token = NULL;
	char temp_buf[MAX_DEV_NUM] = {0};
	char *s = NULL;
	int ret;

	if (dev == NULL || attr == NULL || buf == NULL)
		return 0;

	ret = strncpy_s(temp_buf, sizeof(temp_buf), buf, sizeof(temp_buf) - 1);
	if (ret != EOK)
		return ret;
	s = temp_buf;

	for (token = strsep(&s, ","); (token != NULL) && (i < g_ipa_actor_num);
	     token = strsep(&s, ","), i++) {
		if (kstrtouint(token, DECIMAL, &freq))
			return -EINVAL;
		if (setfreq_available(i, freq))
			g_powerhal_profiles[2][i] = freq;
	}
	cpufreq_update_policies(true);
	gpufreq_update_policies(true);
	return (ssize_t)count;
}

static ssize_t min_freq_show(struct device *dev, struct device_attribute *devattr, char *buf)
{
	return get_freq_show(dev, devattr, buf, PROFILE_MIN);
}
static DEVICE_ATTR(min_freq, S_IWUSR | S_IRUGO, min_freq_show, min_freq_store);

bool is_spm_mode_enabled(void)
{
	return g_ipa_thermal_info.spm.spm_mode || g_ipa_thermal_info.spm.vr_mode;
}
EXPORT_SYMBOL(is_spm_mode_enabled);

int ipa_get_actor_id(const char *name)
{
	int ret = -1;
	u32 id;

	for (id = 0; id < g_ipa_sensor_num; id++) {
		if (strncmp(name, g_ipa_actor_name[id], strlen(name)) == 0) {
			ret = id;
			return ret;
		}
	}

	return ret;
}
EXPORT_SYMBOL_GPL(ipa_get_actor_id);

unsigned int get_powerhal_profile(int actor)
{
	unsigned int freq = 0;
	int gpu_id;

	gpu_id = ipa_get_actor_id("gpu");
	if (gpu_id < 0 || actor < 0)
		return freq;

	if (actor <= gpu_id) {
		if (g_ipa_thermal_info.spm.vr_mode)
			freq = g_powerhal_profiles[1][actor];

		else if (g_ipa_thermal_info.spm.spm_mode)
			freq = g_powerhal_profiles[0][actor];
	}
	return freq;
}
EXPORT_SYMBOL(get_powerhal_profile);

unsigned int get_minfreq_profile(int actor)
{
	unsigned int freq = 0;
	int gpu_id;

	gpu_id = ipa_get_actor_id("gpu");
	if (gpu_id < 0 || actor < 0)
		return freq;

	if (actor <= gpu_id)
		freq = g_powerhal_profiles[2][actor];

	return freq;
}
EXPORT_SYMBOL(get_minfreq_profile);
#endif

int get_soc_temp(void)
{
	if (g_ipa_thermal_info.ipa_thermal[SOC].init_flag == IPA_INIT_OK &&
	    g_ipa_thermal_info.ipa_thermal[SOC].tzd != NULL) {
		if (g_ipa_thermal_info.ipa_thermal[SOC].tzd->temperature < 0)
			return 0;
		return g_ipa_thermal_info.ipa_thermal[SOC].tzd->temperature; /*lint !e571*/
	}

	return  IPA_SOC_INIT_TEMP;
}
EXPORT_SYMBOL(get_soc_temp);


static u32 get_dyn_power_coeff(int cluster, struct ipa_thermal *ipa_dev)
{
	if (ipa_dev == NULL || ipa_dev->caps == NULL) {
		pr_err("%s parm null\n", __func__);
		return 0;
	}

	return ipa_dev->caps->cluster_dyn_capacitance[cluster];
}

static u32 get_cpu_static_power_coeff(int cluster)
{
	struct capacitances *caps = &g_caps;

	return caps->cluster_static_capacitance[cluster];
}

static u32 get_cache_static_power_coeff(int cluster)
{
	struct capacitances *caps = &g_caps;

	return caps->cache_capacitance[cluster];
}

#define MAX_TEMPERATURE 145
#define MIN_TEMPERATURE (-40)
#define MAX_CAPS        0xFFFFF
static unsigned long get_temperature_scale(int temp)
{
	int i, ret;
	struct temp_para_t temp_para;
	long long t_scale = 0;
	struct capacitances *caps = &g_caps;
	int capacitance[NUM_CAPACITANCES] = {0};

	temp_para.t_exp = 1;
	temp_para.temperature = clamp_val(temp, MIN_TEMPERATURE, MAX_TEMPERATURE);
	for (i = 0; i < NUM_CAPACITANCES - 1; i++) {
		ret = kstrtoint(caps->temperature_scale_capacitance[i],
			DECIMAL, &temp_para.cap);
		if (ret != 0) {
			pr_err("%s kstortoint is failed\n", __func__);
			return 0;
		}
		capacitance[i] = clamp_val(temp_para.cap, -MAX_CAPS, MAX_CAPS);
		t_scale += (long long)capacitance[i] * temp_para.t_exp;
		temp_para.t_exp *= temp_para.temperature;
	}

	temp_para.t_scale = clamp_val(t_scale, 0, UINT_MAX);
	ret = kstrtoint(caps->temperature_scale_capacitance[NUM_CAPACITANCES - 1],
			DECIMAL, &capacitance[NUM_CAPACITANCES - 1]);
	if (ret != 0) {
		pr_err("%s kstortoint is failed\n", __func__);
		return 0;
	}
	if (capacitance[NUM_CAPACITANCES - 1] <= 0)
		return 0;

	return (unsigned long)(temp_para.t_scale /
			       capacitance[NUM_CAPACITANCES - 1]);
}

static unsigned long get_voltage_scale(unsigned long u_volt)
{
	unsigned long m_volt = u_volt / 1000;
	unsigned long v_scale;

	v_scale = m_volt * m_volt * m_volt; /* = (m_V^3) / (900 ^ 3) =  */
	return v_scale / 1000000; /* the value returned needs to be /(1E3) */
}

/* voltage in uV and temperature in mC */
/*lint -e715 -esym(715,*)*/
int cluster_get_static_power(cpumask_t *cpumask, unsigned long u_volt, u32 *static_power)
{
	int temperature;
	unsigned long t_scale, v_scale;
	u32 cpu_coeff;
	u32 nr_cpus = cpumask_weight(cpumask);
	int cluster = topology_physical_package_id(cpumask_any(cpumask));

	temperature = get_soc_temp();
	temperature /= 1000;

	cpu_coeff = get_cpu_static_power_coeff(cluster);

	t_scale = get_temperature_scale(temperature);
	v_scale = get_voltage_scale(u_volt);
	*static_power = (u32)(nr_cpus * (cpu_coeff * t_scale * v_scale) / 1000000UL);

	if (nr_cpus != 0) {
		u32 cache_coeff = get_cache_static_power_coeff(cluster);
		/* cache leakage */
		*static_power += (u32)((cache_coeff * v_scale * t_scale) / 1000000UL);
	}
	return 0;
}

#define FRAC_BITS	8
#define int_to_frac(x)	((x) << FRAC_BITS)

/*
 * mul_frac() - multiply two fixed-point numbers
 * @x: first multiplicand
 * @y: second multiplicand
 *
 * Return: the result of multiplying two fixed-point numbers.  The
 * result is also a fixed-point number.
 */
/*lint -e702*/
static inline s32 mul_frac(s32 x, s32 y)
{
	return (s32)((u32)(x * y) >> FRAC_BITS);
}

#define MAX_ALPHA	256
static u32 thermal_zone_alpha_check(u32 alpha)
{
	if (alpha > MAX_ALPHA)
		alpha = MAX_ALPHA;
	return alpha;
}

static int get_temp_value(void *data, int *temp)
{
	int sensor_val[CAPACITY_OF_ARRAY] = {0};
	struct ipa_sensor *sensor = (struct ipa_sensor *)data;
	int val = 0;
	int val_max, id;
	int ret = -EINVAL;
#ifdef CONFIG_THERMAL_SHELL
	struct thermal_zone_device *shell_tz = NULL;
#endif

	if (sensor->sensor_id == IPA_SENSOR_MAXID) {
		/* read all sensor */
		val_max = val;
		for (id = 0; id < (int)g_ipa_sensor_num; id++) {
			ret = ipa_get_sensor_value((u32)id, &val);
			sensor_val[id] = val;
			if(sensor_val[id] > val_max)
				val_max = sensor_val[id];
			if (ret != 0)
				return ret;
		}

		val = val_max;
#ifndef CONFIG_LIBLINUX
		trace_perf(IPA_get_tsens_value, g_ipa_sensor_num, sensor_val, val_max);
#endif
#ifdef CONFIG_THERMAL_SHELL
	} else if (sensor->sensor_id == IPA_SENSOR_SHELLID) {
		shell_tz = thermal_zone_get_zone_by_name(IPA_SENSOR_SHELL);
		ret = get_shell_temp(shell_tz, temp);
#ifndef CONFIG_LIBLINUX
		trace_perf(IPA_get_tsens_value, g_ipa_sensor_num, sensor_val, *temp);
#endif
		if (ret != 0)
			return ret;
		else
			return 0;
#endif
	} else if (sensor->sensor_id == IPA_SENSOR_VIRTUALID) {
		*temp = IPA_SENSOR_VIRTUALTEMP;
#ifndef CONFIG_LIBLINUX
		trace_perf(IPA_get_tsens_value, g_ipa_sensor_num, sensor_val,
					  IPA_SENSOR_VIRTUALTEMP);
#endif
		return 0;
#ifndef CONFIG_LIBLINUX
	} else if (sensor->sensor_id < IPA_PERIPH_NUM) {
		ret = ipa_get_periph_value(sensor->sensor_id, &val);
		trace_perf(IPA_get_tsens_value, g_ipa_sensor_num, sensor_val, val);
		if (ret != 0)
			return ret;
		val = val < 0 ? 0 : val;
#endif
	} else {
		return ret;
	}

	if (sensor->prev_temp == 0)
		sensor->prev_temp = val;
	 /*lint -e776*/
#ifndef CONFIG_LIBLINUX
	val = thermal_zone_temp_check(val);
	sensor->prev_temp = thermal_zone_temp_check(sensor->prev_temp);
	sensor->alpha = thermal_zone_alpha_check(sensor->alpha);
	sensor->prev_temp = mul_frac(sensor->alpha, val) +
		mul_frac(int_to_frac(1) - sensor->alpha, sensor->prev_temp);
#endif
	 /*lint +e776*/
	*temp = sensor->prev_temp;

	return 0;
}

static const struct thermal_zone_of_device_ops ipa_thermal_ops = {
	.get_temp = get_temp_value,
};

#ifndef CONFIG_LIBLINUX
int of_parse_ipa_sensor_index_table(void)
{
	int ret, i;
	struct device_node *np = NULL;
#ifdef CONFIG_THERMAL_SUPPORT_LITE
	int j = 0;
	int oncpucluster[MAX_THERMAL_CLUSTER_NUM] = {0};
	int cpuclustercnt, gpuclustercnt, npuclustercnt;

	get_clustermask_cnt(oncpucluster, MAX_THERMAL_CLUSTER_NUM, &cpuclustercnt,
			    &gpuclustercnt, &npuclustercnt);
#endif
	np = of_find_node_by_name(NULL, "ipa_sensors_info");
	if (np == NULL) {
		pr_err("ipa_sensors_info node not found\n");
		return -ENODEV;
	}
	ret = of_property_read_u32(np, "ithermal,cluster_num", &g_cluster_num);
	if (ret != 0) {
		pr_err("%s cluster_num read err\n", __func__);
		goto node_put;
	}
	ret = of_property_read_u32(np, "ithermal,ipa_sensor_num",
				   &g_ipa_sensor_num);
	if (ret != 0) {
		pr_err("%s ipa_sensor_num read err\n", __func__);
		goto node_put;
	}
	for (i = 0; i < (int)g_ipa_sensor_num; i++) {
#ifdef CONFIG_THERMAL_SUPPORT_LITE
		if (i < g_cluster_num && oncpucluster[i] == 0)
			continue;
		ret = of_property_read_string_index(np, "ithermal,ipa_actor_name",
						    i, &g_ipa_actor_name[j++]);
		if (g_ipa_actor_name[j - 1] != NULL) {
			if (strncmp(g_ipa_actor_name[j - 1], "gpu", 3) == 0 &&
				    (gpuclustercnt == 0)) {
				j--;
				continue;
			}
		}
#else
		ret = of_property_read_string_index(np, "ithermal,ipa_actor_name",
						    i, &g_ipa_actor_name[i]);
#endif
		if (ret != 0) {
			pr_err("%s g_ipa_actor_name %d read err\n", __func__, i);
			goto node_put;
		}
	}
#ifdef CONFIG_THERMAL_SUPPORT_LITE
	g_ipa_sensor_num = cpuclustercnt + gpuclustercnt;
#endif
	ret = of_property_read_u32_array(np, "ithermal,ipa_actor_index",
					 g_ipa_actor_index, g_ipa_sensor_num);
	if (ret != 0) {
		pr_err("%s g_ipa_actor_index read err\n", __func__);
		goto node_put;
	}

	ret = of_property_read_u32_array(np, "ithermal,ipa_cpufreq_table_index",
					 g_ipa_cpufreq_table_index, g_cluster_num);
	if (ret != 0) {
		pr_err("%s g_ipa_cpufreq_table_index read err\n", __func__);
		goto node_put;
	}
	for (i = 0; i < (int)g_cluster_num; i++)
		pr_err("%s, cluster%d, g_ipa_cpufreq_table_index:%d \n", __func__,
			i, g_ipa_cpufreq_table_index[i]);

	of_node_put(np);
	return 0;
node_put:
	of_node_put(np);
	return ret;
}
EXPORT_SYMBOL(of_parse_ipa_sensor_index_table);

static int of_parse_thermal_zone_caps(void)
{
	int ret, i;
	struct device_node *np = NULL;

	if (g_caps.initialized)
		return 0;

	np = of_find_node_by_name(NULL, "capacitances");
	if (np == NULL) {
		pr_err("Capacitances node not found\n");
		return -ENODEV;
	}

	ret = of_property_read_u32_array(np, "ithermal,cluster_dyn_capacitance",
					 g_caps.cluster_dyn_capacitance,
					 g_cluster_num);
	if (ret != 0) {
		pr_err("%s actor_dyn_capacitance read err\n", __func__);
		goto node_put;
	}

	ret = of_property_read_u32_array(np, "ithermal,cluster_static_capacitance",
					 g_caps.cluster_static_capacitance,
					 g_cluster_num);
	if (ret != 0) {
		pr_err("%s actor_dyn_capacitance read err\n", __func__);
		goto node_put;
	}

	ret = of_property_read_u32_array(np, "ithermal,cache_capacitance",
					 g_caps.cache_capacitance,
					 g_cluster_num);
	if (ret != 0) {
		pr_err("%s actor_dyn_capacitance read err\n", __func__);
		goto node_put;
	}

	for (i = 0; i < NUM_TEMP_SCALE_CAPS; i++) {
		ret = of_property_read_string_index(np, "ithermal,temperature_scale_capacitance",
						    i, &g_caps.temperature_scale_capacitance[i]);
		if (ret != 0) {
			pr_err("%s temperature_scale_capacitance %d "
			       "read err\n", __func__, i);
			goto node_put;
		}
	}
	g_caps.initialized = true;
	of_node_put(np);

	return 0;

node_put:
	of_node_put(np);

	return ret;
}

void ipa_get_clustermask(int *clustermask, int len,
			 int *oncpucluster, int *cpuclustercnt)
{
	int cpu;
	int clustercnt[MAX_THERMAL_CLUSTER_NUM] = {0};
	int clusternum = 0;

	(void)len;
	for_each_possible_cpu(cpu) { /*lint !e713 !e574*/
		int cluster_idx = topology_physical_package_id(cpu);
		if (clustercnt[cluster_idx] == 0) {
			clustercnt[cluster_idx]++;
			oncpucluster[cluster_idx]++;
			clusternum++;
		}
	}
	*cpuclustercnt = clusternum;
	if (clusternum == 1) {
		clustermask[0] = 0;
		clustermask[1] = 0;
		clustermask[2] = 0;
	} else if (clusternum == 2) {
		// cluster id is 0 or 1 / 0 or 2 /1 or 2
		if (clustercnt[0] == 1 && clustercnt[1] == 1) {
			clustermask[0] = 0;
			clustermask[1] = 1;
		} else if (clustercnt[0] == 1 && clustercnt[2] == 1) {
			clustermask[0] = 0;
			clustermask[2] = 1;
		} else if (clustercnt[1] == 1 && clustercnt[2] == 1) {
			clustermask[1] = 0;
			clustermask[2] = 1;
		}
	} else {
		clustermask[0] = 0;
		clustermask[1] = 1;
		clustermask[2] = 2;
	}
}

static int ipa_register_soc_cdev(struct ipa_thermal *thermal_data,
				 struct platform_device *pdev)
{
	int ret, i;
	struct cpufreq_policy *policy = NULL;
	struct mask_id_t mask_id;
	int cpuclustercnt;
	int oncpucluster[MAX_THERMAL_CLUSTER_NUM] = {0};
	u32 caps;

	ret = memset_s(mask_id.cpu_masks, sizeof(struct cpumask) * CAPACITY_OF_ARRAY,
		       0, sizeof(struct cpumask) * CAPACITY_OF_ARRAY);
	if (ret != EOK) {
		pr_err("memset_s fail");
		return -ENOMEM;
	}

	ipa_get_clustermask(mask_id.clustermask, MAX_THERMAL_CLUSTER_NUM, oncpucluster, &cpuclustercnt);
	for_each_online_cpu(mask_id.cpuid) {
		int cluster_id = topology_physical_package_id(mask_id.cpuid);
		if (mask_id.clustermask[cluster_id] >= (int)g_cluster_num) {
			pr_warn("IPA:Cluster id: %d >= max id:%d\n",
				cluster_id, g_cluster_num);
			return -ENODEV;
		}
		cpumask_set_cpu((u32)mask_id.cpuid, &mask_id.cpu_masks[mask_id.clustermask[cluster_id]]);
	}

	thermal_data->cdevs =
		kcalloc((size_t)g_cluster_num,
			sizeof(struct thermal_cooling_device *), GFP_KERNEL);
	if (thermal_data->cdevs == NULL) {
		ret = -ENOMEM;
		goto end;
	}

	for (i = 0; i < (int)g_cluster_num; i++) {
		mask_id.cpuid = (int)cpumask_any(&mask_id.cpu_masks[i]);
		if ((u32)mask_id.cpuid >= nr_cpu_ids)
			continue;
		mask_id.cluster = topology_physical_package_id(mask_id.cpuid);
		policy = cpufreq_cpu_get(mask_id.cpuid);
		thermal_data->cdevs[i] = of_cpufreq_cooling_register(policy);
		if (IS_ERR_OR_NULL(thermal_data->cdevs[i])) {
			ret = -EFAULT;
			if (IS_ERR(thermal_data->cdevs[i]))
				ret = (int)PTR_ERR(thermal_data->cdevs[i]);
			if (policy != NULL)
				cpufreq_cpu_put(policy);
			dev_err(&pdev->dev,
				"IPA:Error registering cpu power actor: "
				"cluster %d ERROR_ID %d\n",
				i, ret);
			goto cdevs_unregister;
		}
		caps = get_dyn_power_coeff(mask_id.cluster, thermal_data);
		ret = update_freq_table((struct cpufreq_cooling_device *)thermal_data->cdevs[i]->devdata, caps);
		if (ret != 0)
			goto cdevs_unregister;

		thermal_data->cdevs_num++;
		if (policy != NULL)
			cpufreq_cpu_put(policy);
	}

	return 0;

cdevs_unregister:
	for (i = 0; i < thermal_data->cdevs_num; i++)
		thermal_data->cdevs[i] = NULL;
	thermal_data->cdevs_num = 0;
	kfree(thermal_data->cdevs);
	thermal_data->cdevs = NULL;
end:
	return ret;
}

static int ipa_register_board_cdev(struct ipa_thermal *thermal_data,
				   struct platform_device *pdev)
{
	struct device_node *board_np = NULL;
	int ret = 0;
	int i;
	struct device_node *child = NULL;

	board_np = of_find_node_by_name(NULL, "board-map");
	if (board_np == NULL) {
		dev_err(&pdev->dev, "Board map node not found\n");
		goto end;
	}

	ret = of_get_child_count(board_np);
	if (ret <= 0) {
		ret = -EINVAL;
		of_node_put(board_np);
		pr_err("IPA: board map child count err\n");
		goto end;
	}

	thermal_data->cdevs = kzalloc(sizeof(struct thermal_cooling_device *) *
				      (unsigned long)ret, /*lint !e571*/
				      GFP_KERNEL);
	if (thermal_data->cdevs == NULL) {
		ret = -ENOMEM;
		of_node_put(board_np);
		goto end;
	}

	for_each_child_of_node(board_np, child) {
		thermal_data->cdevs[thermal_data->cdevs_num] =
			board_power_cooling_register(child,
						     g_get_board_power[thermal_data->cdevs_num]);
		if (IS_ERR(thermal_data->cdevs[thermal_data->cdevs_num])) {
			of_node_put(board_np);
			ret = PTR_ERR(thermal_data->cdevs[thermal_data->cdevs_num]); /*lint !e712*/
			dev_err(&pdev->dev,
				"IPA:Error registering board power actor: "
				"ERROR_ID %d\n", ret);
			goto cdevs_unregister;
		}
		thermal_data->cdevs_num++;
	}
	of_node_put(board_np);

	return 0;

cdevs_unregister:
	for (i = 0; i < thermal_data->cdevs_num; i++)
		board_cooling_unregister(thermal_data->cdevs[i]);
	thermal_data->cdevs_num = 0;
	kfree(thermal_data->cdevs);
	thermal_data->cdevs = NULL;
end:
	return ret;
}

static inline void cooling_device_unregister(struct ipa_thermal *thermal_data)
{
	int i;

	if (thermal_data->cdevs == NULL)
		return;

	for (i = 0; i < thermal_data->cdevs_num; i++) {
		if (strstr(thermal_data->cdevs[i]->type, "thermal-board"))
			board_cooling_unregister(thermal_data->cdevs[i]);
		else
			cpufreq_cooling_unregister(thermal_data->cdevs[i]);
	}

	kfree(thermal_data->cdevs);
}

#ifdef CONFIG_IPA_MNTN_INFO
static char *g_runtime_rdr_base;

inline unsigned int lpmcu_runtime_read(void)
{
	if (g_runtime_rdr_base != NULL)
		return readl((void __iomem *)(g_runtime_rdr_base + M3_RDR_THERMAL_FREQ_LIMIT_OFFSET));

	return 0;
}

int init_lpmcu_runtime_base(void)
{
	if (g_runtime_rdr_base != NULL)
		return 0;

	if (M3_RDR_SYS_CONTEXT_BASE_ADDR == 0) {
		pr_err("M3_RDR_SYS_CONTEXT_BASE_ADDR is NULL");
		return -EINVAL;
	}

	g_runtime_rdr_base = M3_RDR_SYS_CONTEXT_BASE_ADDR;
	return 0;
}

inline int is_lpmcu_runtime_base_inited(void)
{
	if (g_runtime_rdr_base != NULL)
		return 0;

	return -EINVAL;
}
#endif

static int ipa_thermal_probe(struct platform_device *pdev)
{
	struct ipa_thermal *thermal_data = &g_ipa_thermal_info.ipa_thermal[SOC];
	int sensor, ret;
	struct device *dev = &pdev->dev;
	struct device_node *dev_node = dev->of_node;
	const char *ch = NULL;
	struct cpufreq_policy *policy = NULL;

	if (!of_device_is_available(dev_node)) {
		dev_err(&pdev->dev, "IPA thermal dev not found\n");
		return -ENODEV;
	}

	policy = cpufreq_cpu_get(0);
	if (policy == NULL)
		return -EPROBE_DEFER;

	if (!policy->freq_table) {
		dev_info(&pdev->dev, "IPA:Frequency table not initialized. "
			 "Deferring probe...\n");
		cpufreq_cpu_put(policy);
		return -EPROBE_DEFER;
	}

	ret = of_parse_thermal_zone_caps();
	if (ret != 0) {
		pr_err("thermal zone caps parse error\n");
		goto end;
	}

	if (strncmp(pdev->name, "ipa-sensor@0", sizeof("ipa-sensor@0") - 1) == 0) {
		thermal_data = &g_ipa_thermal_info.ipa_thermal[SOC];
		thermal_data->caps = &g_caps;
		ret = ipa_register_soc_cdev(thermal_data, pdev);
	} else if (strncmp(pdev->name, "ipa-sensor@1",
			    sizeof("ipa-sensor@1") - 1) == 0) {
		thermal_data = &g_ipa_thermal_info.ipa_thermal[BOARD];
		thermal_data->caps = &g_caps;
		ret = ipa_register_board_cdev(thermal_data, pdev);
	}
	if (ret != 0)
		goto end;

	ret = of_property_read_string(dev_node, "type", &ch);
	if (ret != 0) {
		dev_err(dev, "%s sensor name read err\n", __func__);
		goto cdevs_unregister;
	}

	sensor = get_sensor_id_by_name(ch);
	if (sensor < 0) {
		ret = sensor;
		goto cdevs_unregister;
	}

	thermal_data->ipa_sensor.sensor_id = (u32)sensor;
	dev_info(&pdev->dev, "IPA:Probed %s sensor. Id = %d\n", ch, sensor);

	/*
	 * alpha ~= 2 / (N + 1) with N the window of a rolling mean We
	 * use 8-bit fixed point arithmetic.  For a rolling average of
	 * window 20, alpha = 2 / (20 + 1) ~= 0.09523809523809523 .
	 * In 8-bit fixed point arigthmetic, 0.09523809523809523 * 256
	 * ~= 24
	 */
	thermal_data->ipa_sensor.alpha = 24;
	thermal_data->tzd =
		thermal_zone_of_sensor_register(&pdev->dev,
						(int)thermal_data->ipa_sensor.sensor_id,
						&thermal_data->ipa_sensor,
						&ipa_thermal_ops);

	if (IS_ERR(thermal_data->tzd)) {
		dev_err(&pdev->dev, "IPA ERR:registering sensor tzd error.\n");
		ret = PTR_ERR(thermal_data->tzd); /*lint !e712*/
		goto cdevs_unregister;
	}

	update_debugfs(&thermal_data->ipa_sensor);
	thermal_zone_device_update(thermal_data->tzd, THERMAL_EVENT_UNSPECIFIED);

	platform_set_drvdata(pdev, thermal_data);
	thermal_data->init_flag = IPA_INIT_OK;
	cpufreq_cpu_put(policy);
#ifdef CONFIG_IPA_MNTN_INFO
	if (is_lpmcu_runtime_base_inited() != 0)
		init_lpmcu_runtime_base();
#endif
	return 0;

cdevs_unregister:
	cooling_device_unregister(thermal_data);
end:
	cpufreq_cpu_put(policy);
	return ret;
}

static int ipa_thermal_remove(struct platform_device *pdev)
{
	struct ipa_thermal *thermal_data = platform_get_drvdata(pdev);

	if (thermal_data == NULL) {
		dev_warn(&pdev->dev, "%s sensor is null!\n", __func__);
		return -1;
	}

	thermal_zone_of_sensor_unregister(&pdev->dev, thermal_data->tzd);
	cooling_device_unregister(thermal_data);

	platform_set_drvdata(pdev, NULL);

#ifdef CONFIG_IPA_MNTN_INFO
	if (g_runtime_rdr_base != NULL)
		g_runtime_rdr_base = NULL;
#endif
	return 0;
}

/*lint -e785 -esym(785,*)*/
static struct of_device_id ipa_thermal_of_match[] = {
	{ .compatible = "arm,ipa-thermal0" },
	{ .compatible = "arm,ipa-thermal1" },
	{},
};

/*lint -e785 +esym(785,*)*/

MODULE_DEVICE_TABLE(of, ipa_thermal_of_match);

static struct platform_driver ipa_thermal_platdrv = {
	.driver = {
		.name = "ipa-thermal",
		.owner = THIS_MODULE,
		.of_match_table = ipa_thermal_of_match
	},
	.probe = ipa_thermal_probe,
	.remove = ipa_thermal_remove,
};

/*lint -e64 -e528 -esym(64,528,*)*/
module_platform_driver(ipa_thermal_platdrv);
/*lint -e64 -e528 +esym(64,528,*)*/

#ifdef CONFIG_THERMAL_SPM
static int powerhal_cfg_init(void)
{
	struct device_node *node = NULL;
	int ret;
	unsigned int data[CAPACITY_OF_ARRAY] = {0};
#ifdef CONFIG_THERMAL_SUPPORT_LITE
	int oncpucluster[MAX_THERMAL_CLUSTER_NUM] = {0};
	int cpuclustercnt, gpuclustercnt, npuclustercnt;

	get_clustermask_cnt(oncpucluster, MAX_THERMAL_CLUSTER_NUM, &cpuclustercnt, &gpuclustercnt,
			    &npuclustercnt);
#endif
	node = of_find_compatible_node(NULL, NULL, "ithermal,powerhal");
	if (node == NULL) {
		pr_err("%s cannot find powerhal dts.\n", __func__);
		return -ENODEV;
	}

	ret = of_property_count_u32_elems(node, "ithermal,powerhal-spm-cfg");
	if (ret < 0) {
		pr_err("%s cannot find ithermal,powerhal-spm-cfg.\n", __func__);
		goto node_put;
	}
	g_powerhal_actor_num = (unsigned int)ret;
	ret = of_property_read_u32_array(node, "ithermal,powerhal-spm-cfg",
					 data, g_powerhal_actor_num);
	if (ret < 0) {
		pr_err("%s cannot find ithermal,powerhal-spm-cfg.\n", __func__);
		goto node_put;
	}
	ret = memcpy_s(g_powerhal_profiles[0], sizeof(g_powerhal_profiles[0]),
		       data, sizeof(g_powerhal_profiles[0]));
	if (ret != EOK) {
		pr_err("%s:%d:memcpy_s copy failed.\n", __func__, __LINE__);
		goto node_put;
	}

	ret = of_property_read_u32_array(node, "ithermal,powerhal-vr-cfg",
					 data, g_powerhal_actor_num);
	if (ret < 0) {
		pr_err("%s cannot find ithermal,powerhal-spm-cfg.\n", __func__);
		goto node_put;
	}
	ret = memcpy_s(g_powerhal_profiles[1], sizeof(g_powerhal_profiles[1]),
		       data, sizeof(g_powerhal_profiles[1]));
	if (ret != EOK) {
		pr_err("%s:%d:memcpy_s copy failed.\n", __func__, __LINE__);
		goto node_put;
	}

	ret = of_property_read_u32_array(node, "ithermal,powerhal-min-cfg",
					 data, g_powerhal_actor_num);
	if (ret < 0) {
		pr_err("%s cannot find ithermal,powerhal-spm-cfg.\n", __func__);
		goto node_put;
	}

	ret = memcpy_s(g_powerhal_profiles[2], sizeof(g_powerhal_profiles[2]),
		       data, sizeof(g_powerhal_profiles[2]));
	if (ret != EOK) {
		pr_err("%s:%d:memcpy_s copy failed.\n", __func__, __LINE__);
		goto node_put;
	}
#ifdef CONFIG_THERMAL_SUPPORT_LITE
	lite_powerhal_proc(oncpucluster, MAX_THERMAL_CLUSTER_NUM, cpuclustercnt,
			   gpuclustercnt);
#endif
	return 0;
node_put:
	of_node_put(node);
	return ret;
}
#endif

int ipa_weights_cfg_init(void)
{
	struct device_node *node = NULL;
	int ret;
	unsigned int data[2][CAPACITY_OF_ARRAY] = {{0}, {0}};
#ifdef CONFIG_THERMAL_SUPPORT_LITE
	int oncpucluster[MAX_THERMAL_CLUSTER_NUM] = {0};
	int cpuclustercnt, gpuclustercnt, npuclustercnt;
#endif

#if CONFIG_OF
#ifdef CONFIG_THERMAL_SUPPORT_LITE
	get_clustermask_cnt(oncpucluster, MAX_THERMAL_CLUSTER_NUM, &cpuclustercnt, &gpuclustercnt,
			    &npuclustercnt);
#endif
	node = of_find_compatible_node(NULL, NULL, "ithermal,weights");
	if (node == NULL) {
		pr_err("%s cannot find weights dts.\n", __func__);
		return -ENODEV;
	}

	ret = of_property_count_u32_elems(node, "ithermal,weights-default-cfg");
	if (ret < 0) {
		pr_err("%s cannot find ithermal,weights-default-cfg.\n", __func__);
		goto node_put;
	}
	g_ipa_actor_num = (unsigned int)ret;

	ret = of_property_read_u32_array(node, "ithermal,weights-default-cfg",
					 data[0], g_ipa_actor_num);
	if (ret < 0) {
		pr_err("%s cannot find ithermal,weights-default-cfg.\n", __func__);
		goto node_put;
	}

	ret = memcpy_s(g_weights_profiles[0], sizeof(g_weights_profiles[0]),
		       data[0], sizeof(g_weights_profiles[0]));
	if (ret != EOK) {
		pr_err("%s:%d:memcpy_s copy failed.\n", __func__, __LINE__);
		goto node_put;
	}
	ret = of_property_read_u32_array(node, "ithermal,weights-boost-cfg",
					 data[1], g_ipa_actor_num);
	if (ret < 0) {
		pr_err("%s cannot find ithermal,weights-boost-cfg.\n", __func__);
		goto node_put;
	} /*lint !e419*/

	ret = memcpy_s(g_weights_profiles[1], sizeof(g_weights_profiles[1]),
		       data[1], sizeof(g_weights_profiles[1]));
	if (ret != EOK) {
		pr_err("%s:%d:memcpy_s copy failed.\n", __func__, __LINE__);
		goto node_put;
	}
#ifdef CONFIG_THERMAL_SUPPORT_LITE
	lite_weight_proc(oncpucluster, MAX_THERMAL_CLUSTER_NUM, cpuclustercnt, gpuclustercnt);
#endif
#endif
	return 0;
node_put:
	of_node_put(node);
	return ret;
}
EXPORT_SYMBOL(ipa_weights_cfg_init);

static void dynipa_get_weights_cfg(unsigned int *weight0, unsigned int *weight1)
{
	int ret;

	ret = memcpy_s(weight0, sizeof(*weight0) * g_ipa_actor_num,
		       g_weights_profiles[0], sizeof(*weight0) * g_ipa_actor_num);
	if (ret != EOK)
		pr_err("%s:%d:memcpy_s copy failed.\n", __func__, __LINE__);
	ret = memcpy_s(weight1, sizeof(*weight1) * g_ipa_actor_num,
		       g_weights_profiles[1], sizeof(*weight1) * g_ipa_actor_num);
	if (ret != EOK)
		pr_err("%s:%d:memcpy_s copy failed.\n", __func__, __LINE__);
}
#endif

#ifdef CONFIG_THERMAL_SPM
static int thermal_spm_node_create(void)
{
	int ret;

	ret = device_create_file(g_ipa_thermal_info.spm.device, &dev_attr_spm_mode);
	if (ret != 0) {
		pr_err("Spm mode create error\n");
		return ret;
	}

	ret = device_create_file(g_ipa_thermal_info.spm.device, &dev_attr_spm_freq);
	if (ret != 0) {
		pr_err("Spm freq create error\n");
		return ret;
	}

	ret = device_create_file(g_ipa_thermal_info.spm.device, &dev_attr_vr_mode);
	if (ret != 0) {
		pr_err("VR mode create error\n");
		return ret;
	}

	ret = device_create_file(g_ipa_thermal_info.spm.device, &dev_attr_vr_freq);
	if (ret != 0) {
		pr_err("VR freq create error\n");
		return ret;
	}

	ret = device_create_file(g_ipa_thermal_info.spm.device, &dev_attr_min_freq);
	if (ret != 0) {
		pr_err("Min freq create error\n");
		return ret;
	}

	return 0;
}
#endif

#ifdef CONFIG_IPA_MNTN_INFO
ssize_t read_limit_freq(char *buf)
{
	unsigned int i, lpmcu_v;
	ssize_t ret = 0;
	int rc;

	if (buf == NULL)
		return 0;
	lpmcu_v = lpmcu_runtime_read();
	for (i = 0; i < g_ipa_actor_num; i++) {
		rc = snprintf_s(buf + ret, PAGE_SIZE - ret, MAX_IPA_MNTN_INF_LEN, "%u%s",
				(char)(lpmcu_v >> (8 * i)), ",");
		if (rc < 0) {
			pr_err("%s: snprintf_s error\n", __func__);
			return rc;
		}
		ret += rc;
	}

	rc = snprintf_s(buf + ret - 1, PAGE_SIZE - ret, 1UL, "\n");
	if (rc < 0) {
		pr_err("%s: snprintf_s error\n", __func__);
		return rc;
	}
	ret += rc;
	return ret;
}
EXPORT_SYMBOL(read_limit_freq);

ssize_t lpm_limit_freq(struct device *dev, struct device_attribute *devattr, char *buf)
{
	if (dev == NULL || devattr == NULL)
		return 0;

	return read_limit_freq(buf);
}
EXPORT_SYMBOL(lpm_limit_freq);
static ssize_t lpm_freq_show(struct device *dev, struct device_attribute *devattr, char *buf)
{
	return lpm_limit_freq(dev, devattr, buf);
}
static DEVICE_ATTR(lpm_freq, S_IRUGO, lpm_freq_show, NULL);
#endif

#ifdef CONFIG_THERMAL_SUPPORT_LITE
static void update_lite_cluster_num()
{
	int oncpucluster[MAX_THERMAL_CLUSTER_NUM] = {0};
	int cpuclustercnt, gpuclustercnt, npuclustercnt;

	get_clustermask_cnt(oncpucluster, MAX_THERMAL_CLUSTER_NUM, &cpuclustercnt, &gpuclustercnt,
			    &npuclustercnt);
	g_cluster_num = cpuclustercnt;
}
#endif

static int thermal_init(void)
{
	int ret;

#ifndef CONFIG_LIBLINUX
	dynipa_get_weights_cfg(g_ipa_normal_weights, g_ipa_gpu_boost_weights);
#endif
#if defined(CONFIG_THERMAL_SPM) || defined(CONFIG_THERMAL_HOTPLUG)
	g_ipa_thermal_info.thermal_class =
		class_create(THIS_MODULE, "ithermal"); /*lint !e64*/
	if (IS_ERR(g_ipa_thermal_info.thermal_class)) {
		pr_err("thermal class create error\n");
		return (int)PTR_ERR(g_ipa_thermal_info.thermal_class);
	}
	g_thermal_hotplug.thermal_class = g_ipa_thermal_info.thermal_class;
#endif

#ifdef CONFIG_THERMAL_SPM
	if (powerhal_cfg_init()) {
		pr_err("%s: powerhal_init error\n", __func__);
		ret = -ENODEV;
		goto class_destroy;
	}

	g_ipa_thermal_info.spm.device =
	    device_create(g_ipa_thermal_info.thermal_class, NULL, 0, NULL, "spm");
	if (IS_ERR(g_ipa_thermal_info.spm.device)) {
		pr_err("Spm device create error\n");
		ret = (int)PTR_ERR(g_ipa_thermal_info.spm.device);
		goto class_destroy;
	}

	ret = thermal_spm_node_create();
	if (ret != 0) {
		goto device_destroy;
	}
#endif

#ifdef CONFIG_IPA_MNTN_INFO
	g_ipa_thermal_info.lpm.device =
	    device_create(g_ipa_thermal_info.thermal_class, NULL, 0, NULL, "lpm");
	if (IS_ERR(g_ipa_thermal_info.lpm.device)) {
		pr_err("lpm device create error\n");
		ret = (int)PTR_ERR(g_ipa_thermal_info.lpm.device);
		goto class_destroy;
	}

	ret = device_create_file(g_ipa_thermal_info.lpm.device, &dev_attr_lpm_freq);
	if (ret != 0) {
		pr_err("lpm freq create error\n");
		goto device_destroy;
	}
#endif

#ifdef CONFIG_THERMAL_SUPPORT_LITE
	update_lite_cluster_num();
#endif

#ifdef CONFIG_THERMAL_HOTPLUG
	thermal_hotplugs_init();
#endif
	return 0;

#if defined(CONFIG_THERMAL_SPM) || defined(CONFIG_IPA_MNTN_INFO)
device_destroy:
	device_destroy(g_ipa_thermal_info.thermal_class, 0);
class_destroy:
	class_destroy(g_ipa_thermal_info.thermal_class);
	g_ipa_thermal_info.thermal_class = NULL;

	return ret;
#endif
}

static void thermal_exit(void)
{
#if defined(CONFIG_THERMAL_SPM) || defined(CONFIG_IPA_MNTN_INFO)
	if (g_ipa_thermal_info.thermal_class) {
		device_destroy(g_ipa_thermal_info.thermal_class, 0);
		class_destroy(g_ipa_thermal_info.thermal_class);
	}
#endif
#ifdef CONFIG_THERMAL_HOTPLUG
	thermal_hotplugs_exit();
#endif
}

/*lint -e528 -esym(528,*)*/
late_initcall(thermal_init);
module_exit(thermal_exit);
/*lint -e528 +esym(528,*)*/

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("thermal ipa module driver");
