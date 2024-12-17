/*
 * ipa_thermal.h
 *
 * header of thermal ipa framework
 *
 * Copyright (C) 2020-2020 Huawei Technologies Co., Ltd.
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
#ifndef __IPA_THERMAL_H
#define __IPA_THERMAL_H
#include <linux/thermal.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/types.h>

#define IPA_SENSOR	"tsens_max"
#define IPA_SENSOR_SYSTEM_H	"system_h"
#define IPA_SENSOR_MAXID	255
/* 254 used by IPA_SENSOR_SHELLID */
#define IPA_SENSOR_VIRTUAL	"virtual_sensor"
#define IPA_SENSOR_VIRTUALID	253
#define IPA_SENSOR_VIRTUALTEMP	45000
#define IPA_INIT_OK	0x05a5a5a5b
#define IPA_SOC_INIT_TEMP	85000
#ifdef CONFIG_THERMAL_SPM
#define MAX_SHOW_STR	5
#endif
#ifdef CONFIG_THERMAL_SHELL
#define NUM_SENSORS	4
#else
#define NUM_SENSORS	3
#endif
#define NUM_TEMP_SCALE_CAPS	5
#define NUM_TZD	2
#define NUM_BOARD_CDEV	3
#define FREQ_BUF_LEN	11UL
#define DECIMAL	10
#define NUM_CAPACITANCES	5
#define MAX_DEV_NUM	40

#ifdef CONFIG_THERMAL_SPM
struct spm_t {
	struct device *device;
	bool spm_mode;
	bool vr_mode;
};
#endif

#ifdef CONFIG_IPA_MNTN_INFO
struct lpmcu_mntn_t {
	struct device *device;
};
#endif

enum {
	SOC = 0,
	BOARD
};

enum {
	PROFILE_SPM = 0,
	PROFILE_VR,
	PROFILE_MIN
};
#ifdef CONFIG_IPA_MNTN_INFO
extern void __iomem *g_ipa_lpmcu_freq_limit_base;
#endif
/*
 * the num of ipa cpufreq table equals cluster num , but
 * cluster num is a variable. So define a larger arrays in advance.
 */
#define CAPACITY_OF_ARRAY	10
struct capacitances {
	u32 cluster_dyn_capacitance[CAPACITY_OF_ARRAY];
	u32 cluster_static_capacitance[CAPACITY_OF_ARRAY];
	u32 cache_capacitance[CAPACITY_OF_ARRAY];
	const char *temperature_scale_capacitance[NUM_TEMP_SCALE_CAPS];
	bool initialized;
};

struct ipa_sensor {
	u32 sensor_id;
	s32 prev_temp;
	int alpha;
};

struct ipa_thermal {
	struct ipa_sensor ipa_sensor;
	struct thermal_zone_device *tzd;
	int cdevs_num;
	struct thermal_cooling_device **cdevs;
	struct capacitances *caps;
	int init_flag;
};

#define MAX_CLUSTER_NUM		3
struct mask_id_t {
	int cpuid;
	int cluster;
	struct cpumask cpu_masks[CAPACITY_OF_ARRAY];
	int clustermask[MAX_CLUSTER_NUM];
};

struct temp_para_t
{
	int temperature;
	int cap;
	int t_exp;
	long long t_scale;
};

struct thermal {
#if defined(CONFIG_THERMAL_SPM) || defined(CONFIG_THERMAL_HOTPLUG)
	struct class *thermal_class;
#endif
	struct ipa_thermal ipa_thermal[NUM_TZD];
#ifdef CONFIG_THERMAL_SPM
	struct spm_t spm;
#endif
#ifdef CONFIG_IPA_MNTN_INFO
	struct lpmcu_mntn_t lpm;
#endif
};

void update_debugfs(struct ipa_sensor *sensor_data);
#ifdef CONFIG_IPA_MNTN_INFO
#define MAX_IPA_MNTN_INF_LEN (M3_RDR_THERMAL_FREQ_LIMIT_ADDR_SIZE + 5)
ssize_t lpm_limit_freq(struct device *dev, struct device_attribute *devattr, char *buf);
#endif
#endif

