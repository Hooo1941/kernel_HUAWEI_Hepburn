
/*
 * dpm_hwmon_v3.c
 *
 * dpm interface for component
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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

#include "dpm_hwmon_v3.h"
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <securec.h>
#include <linux/suspend.h>

LIST_HEAD(g_dpm_hwmon_ops_list);
DEFINE_MUTEX(g_dpm_hwmon_ops_list_lock);

static void dpm_sample_all(void);
static void dpm_set_sample_mode(struct dpm_hwmon_ops *pos, int mode);
#ifdef CONFIG_DPM_HWMON_PWRSTAT
#ifdef CONFIG_PM_DEL_PLATFORM_ADDR
#include <lpm_kernel_map.h>
#else
#include <soc_acpu_baseaddr_interface.h>
#include <soc_crgperiph_interface.h>
#endif

#include <soc_powerstat_interface.h>
#define PERI_CRG_SIZE	0xFFF
#define PWR_STAT_SIZE	0xFFF

#define BUFFER_SIZE	30
#define MAX_CHANNEL	31

static void __iomem *g_peri_crg_base;
static void __iomem *g_pwr_stat_base;
struct powerstat_work {
	bool is_on;
	unsigned int channel_max;
	unsigned int channel_cfg;
	unsigned long long *channel_power_table;
	unsigned int sample_interval;
};

DEFINE_MUTEX(g_powerstate_lock);
static struct powerstat_work g_powerstate_work = {false, 0, 0, NULL, 0};

static void set_powerstate_state(bool state)
{
	g_powerstate_work.is_on = state;
}

static bool get_powerstate_state(void)
{
	return g_powerstate_work.is_on;
}

void dpm_sample(struct dpm_hwmon_ops *pos)
{
	unsigned int channel;

	if (pos == NULL || g_powerstate_work.channel_power_table == NULL)
		return;
	mutex_lock(&g_powerstate_lock);
	if (!get_powerstate_state()) {
		mutex_unlock(&g_powerstate_lock);
		return;
	}
	for (channel = pos->power_start; channel <= pos->power_end; channel++)
		g_powerstate_work.channel_power_table[channel] +=
			readq(SOC_POWERSTAT_DPM_TOTAL_ENERGY_LOW_ADDR(g_pwr_stat_base, channel));
	mutex_unlock(&g_powerstate_lock);
}

static int map_powerstate_range(struct dpm_hwmon_ops *dpm_ops)
{
	char buffer[BUFFER_SIZE];
	int ret = 0;
	struct device_node *np = NULL;

	ret = snprintf_s(buffer, BUFFER_SIZE * sizeof(char), (BUFFER_SIZE - 1) * sizeof(char),
			"ipowerstate,%s", dpm_module_table[dpm_ops->dpm_module_id]);
	if (ret < 0) {
		pr_err("%s: snprintf_s buffer err.\n", __func__);
		return ret;
	}
	np = of_find_compatible_node(NULL, NULL, buffer);
	if (np == NULL) {
		pr_err("%s: %s not find.\n", __func__, buffer);
		return -ENODEV;
	}
	ret = of_property_read_u32(np, "power_start", &dpm_ops->power_start);
	if (ret < 0) {
		pr_err("%s no power start!\n", __func__);
		return -ENODEV;
	}
	ret = of_property_read_u32(np, "power_end", &dpm_ops->power_end);
	if (ret < 0) {
		pr_err("%s no power end!\n", __func__);
		return -ENODEV;
	}
	return 0;
}

static unsigned int convert_config_mode(enum powerstate_mode mode)
{
	switch (mode) {
	case WORK_TOTAL_ENERGY_MODE:
		return 0x000000f0;
	case WORK_DYN_ENERGY_MODE:
		return 0x000000f1;
	case FIT_TOTAL_ENERGY_MODE:
		return 0x18f03f4;
	case FIT_DYN_ENERGY_MODE:
		return 0x18f03f5;
	case FIT_ICG_CNT_MODE:
		return 0x10f07f6;
	case FIT_PMU_CNT_MODE:
		return 0x18f03f7;
	default:
		pr_err("%s,Wrong mode%d!", __func__, mode);
		return 0xf0;
	}
}

#define DPM_DATA_ADDR_H	0x1
#define DPM_DATA_ADDR_L	0x0
#define DPM_DATA_SIZE	0x80000000
/* mode:0-total_energy 1-total_enegy,dyn_energy 2-energy,icg_cnt 3-energy,pmu_cnt */
static void powerstat_enable(enum powerstate_mode mode, unsigned int channels,
			unsigned int interval, unsigned int sample_time)
{
	unsigned int powerstat_config;

	mutex_lock(&g_powerstate_lock);
	/* step 1,2 */
	writel(BIT(SOC_CRGPERIPH_PEREN0_gt_pclk_power_stat_START) |
		BIT(SOC_CRGPERIPH_PEREN0_gt_clk_power_stat_START),
		SOC_CRGPERIPH_PEREN0_ADDR(g_peri_crg_base));
	writel(BIT(SOC_CRGPERIPH_GENERAL_SEC_PEREN_gt_aclk_power_stat_START),
		SOC_CRGPERIPH_GENERAL_SEC_PEREN_ADDR(g_peri_crg_base));
	writel(BIT(SOC_CRGPERIPH_PERRSTDIS0_ip_rst_power_stat_START),
		SOC_CRGPERIPH_PERRSTDIS0_ADDR(g_peri_crg_base));

	/* step 3, init channel */
	writel(channels, SOC_POWERSTAT_CHNL_EN0_ADDR(g_pwr_stat_base));

	/* step 4,5 interval and sample time */
	writel(interval, SOC_POWERSTAT_INTERVAL_ADDR(g_pwr_stat_base));
	writel(sample_time, SOC_POWERSTAT_SAMPLE_TIME_ADDR(g_pwr_stat_base));

	powerstat_config = convert_config_mode(mode);
	writel(powerstat_config, SOC_POWERSTAT_WORK_MODE_CFG_ADDR(g_pwr_stat_base));
	if ((powerstat_config & BIT(SOC_POWERSTAT_WORK_MODE_CFG_sample_mode_START)) == 0) {
		writel(0xFFFF485E, SOC_POWERSTAT_SRAM_CTRL_ADDR(g_pwr_stat_base));
	} else {
		writel(0xFFFF4858, SOC_POWERSTAT_SRAM_CTRL_ADDR(g_pwr_stat_base));

		writel(0x20, SOC_POWERSTAT_SFIFO_CFG_ADDR(g_pwr_stat_base));
		writel(DPM_DATA_ADDR_L,
			SOC_POWERSTAT_SEQ_ADDR_CFG0_ADDR(g_pwr_stat_base));
		writel(DPM_DATA_ADDR_H, SOC_POWERSTAT_SEQ_ADDR_CFG1_ADDR(g_pwr_stat_base));
		writel(DPM_DATA_SIZE,
			SOC_POWERSTAT_SEQ_SPACE_CFG0_ADDR(g_pwr_stat_base));
		writel(0, SOC_POWERSTAT_SEQ_SPACE_CFG1_ADDR(g_pwr_stat_base));
	}

	/* step 9 en sample */
	writel(0, SOC_POWERSTAT_SAMPLE_EN_ADDR(g_pwr_stat_base));
	writel(1, SOC_POWERSTAT_SAMPLE_EN_ADDR(g_pwr_stat_base));

	set_powerstate_state(true);
	mutex_unlock(&g_powerstate_lock);
}

static void powerstat_disable(void)
{
	mutex_lock(&g_powerstate_lock);
	if (!get_powerstate_state()) {
		mutex_unlock(&g_powerstate_lock);
		return;
	}

	set_powerstate_state(false);
	/* step 1 disable sample */
	writel(0, SOC_POWERSTAT_SAMPLE_EN_ADDR(g_pwr_stat_base));

	mdelay(1);

	writel(0xFFFF485E, SOC_POWERSTAT_SRAM_CTRL_ADDR(g_pwr_stat_base));

	/* step 4,5 */
	writel(BIT(SOC_CRGPERIPH_PERRSTEN0_ip_rst_power_stat_START),
		SOC_CRGPERIPH_PERRSTEN0_ADDR(g_peri_crg_base));
	writel(BIT(SOC_CRGPERIPH_GENERAL_SEC_PERDIS_gt_aclk_power_stat_START),
		SOC_CRGPERIPH_GENERAL_SEC_PERDIS_ADDR(g_peri_crg_base));
	writel(BIT(SOC_CRGPERIPH_PERDIS0_gt_pclk_power_stat_START) |
		BIT(SOC_CRGPERIPH_PERDIS0_gt_clk_power_stat_START),
		SOC_CRGPERIPH_PERDIS0_ADDR(g_peri_crg_base));
	mutex_unlock(&g_powerstate_lock);
}

static unsigned int get_channel_config(unsigned int start, unsigned int end)
{
	unsigned int i;
	unsigned int channels = 0;

	if (start > end || end > MAX_CHANNEL)
		return channels;

	for (i = start; i <= end; i++)
		channels |= BIT(i);
	return channels;
}

static enum dpm_sample_mode convert_dpm_mode(enum powerstate_mode mode)
{
	switch (mode) {
	case FIT_ICG_CNT_MODE:
		return PWRSTAT_ICG_MODE;
	case FIT_PMU_CNT_MODE:
		return PWRSTAT_PMU_MODE;
	default:
		return SAMPLE_ERR_MODE;
	}
}

void powerstat_config(struct dpm_hwmon_ops *pos, int timer_span, int total_count, int mode)
{
	unsigned int channels;
	int sample_mode;

	if (pos == NULL)
		return;
	channels = get_channel_config(pos->power_start, pos->power_end);

	sample_mode = convert_dpm_mode((enum powerstate_mode)mode);
	if (sample_mode != SAMPLE_ERR_MODE)
		dpm_set_sample_mode(pos, sample_mode);
	else
		pr_err("invalid mode: %d\n", mode);

	powerstat_disable();
	powerstat_enable((enum powerstate_mode)mode, channels,
			(unsigned int)timer_span, (unsigned int)total_count);
}

static unsigned long long sum_total_energy(unsigned int start, unsigned int end)
{
	unsigned int channel;
	unsigned long long energy = 0;

	if ((end + 1) >= g_powerstate_work.channel_max ||
			g_powerstate_work.channel_power_table == NULL)
		return 0;
	for (channel = start; channel <= end; channel++)
		energy += g_powerstate_work.channel_power_table[channel];
	return energy;
}

#ifdef CONFIG_CPU_DPM
int get_single_cpu_power(struct dpm_hwmon_ops *pos)
{
	unsigned int channel;

	dpm_sample_all();
	for (channel = pos->power_start; channel <= pos->power_end; channel++)
		g_dpm_cpu_power_data.power[channel] = g_powerstate_work.channel_power_table[channel];

	return 0;
}
#endif

unsigned long long get_dpm_power(struct dpm_hwmon_ops *pos)
{
	unsigned long long total_power = 0;

	if (pos == NULL)
		return total_power;

	dpm_sample_all();
	total_power = sum_total_energy(pos->power_start, pos->power_end);
	return total_power;
}

void dpm_iounmap(void)
{
	if (g_peri_crg_base != NULL) {
		iounmap(g_peri_crg_base);
		g_peri_crg_base = NULL;
	}
	if (g_pwr_stat_base != NULL) {
		iounmap(g_pwr_stat_base);
		g_pwr_stat_base = NULL;
	}
}

int dpm_ioremap(void)
{
	g_peri_crg_base = ioremap(SOC_ACPU_PERI_CRG_BASE_ADDR, PERI_CRG_SIZE);
	g_pwr_stat_base = ioremap(SOC_ACPU_PWR_STAT_BASE_ADDR, PWR_STAT_SIZE);
	if (g_peri_crg_base == NULL || g_pwr_stat_base == NULL) {
		dpm_iounmap();
		return -EFAULT;
	}
	return 0;
}
#else
#include <linux/ktime.h>
#include <linux/workqueue.h>

#define DPM_SAMPLE_INTERVAL		1000

static struct delayed_work g_dpm_hwmon_work;

static inline void dpm_iounmap(void) {}
static inline int dpm_ioremap(void) {return 0; }

static void free_dpm_ops_mem(struct dpm_hwmon_ops *dpm_ops)
{
	if (dpm_ops->total_power_table != NULL) {
		kfree(dpm_ops->total_power_table);
		dpm_ops->total_power_table = NULL;
	}
#ifdef CONFIG_DPM_HWMON_DEBUG
	if (dpm_ops->dyn_power_table != NULL) {
		kfree(dpm_ops->dyn_power_table);
		dpm_ops->dyn_power_table = NULL;
	}
	if (dpm_ops->dpm_counter_table != NULL) {
		kfree(dpm_ops->dpm_counter_table);
		dpm_ops->dpm_counter_table = NULL;
	}
#endif
}
static int malloc_dpm_ops_mem(struct dpm_hwmon_ops *dpm_ops)
{
#ifdef CONFIG_DPM_HWMON_DEBUG
	if ((dpm_ops->dpm_cnt_len > 0 && dpm_ops->dpm_cnt_len < DPM_BUFFER_SIZE) &&
	    (dpm_ops->dpm_power_len > 0 && dpm_ops->dpm_power_len < DPM_BUFFER_SIZE)) {
		dpm_ops->total_power_table = kzalloc(sizeof(unsigned int) *
						     dpm_ops->dpm_power_len,
						     GFP_KERNEL);
		dpm_ops->dyn_power_table = kzalloc(sizeof(unsigned int) *
						   dpm_ops->dpm_power_len,
						   GFP_KERNEL);
		dpm_ops->dpm_counter_table = kzalloc(sizeof(unsigned int) *
						     dpm_ops->dpm_cnt_len,
						     GFP_KERNEL);
		if (dpm_ops->total_power_table == NULL ||
		    dpm_ops->dyn_power_table == NULL ||
		    dpm_ops->dpm_counter_table == NULL)
			return -ENOMEM;
	} else {
		return -EINVAL;
	}
#else
	if (dpm_ops->dpm_power_len > 0 && dpm_ops->dpm_power_len < DPM_BUFFER_SIZE) {
		dpm_ops->total_power_table = kzalloc(sizeof(unsigned long long) *
						     dpm_ops->dpm_power_len,
						     GFP_KERNEL);
		if (dpm_ops->total_power_table == NULL)
			return -ENOMEM;
	} else {
		return -EINVAL;
	}
#endif
	return 0;
}

unsigned long long get_dpm_power(struct dpm_hwmon_ops *pos)
{
	unsigned int i;
	unsigned long long total_power = 0;

	if (pos == NULL)
		return total_power;

	for (i = 0; i < pos->dpm_power_len; i++)
		total_power += pos->total_power_table[i];

	return total_power;
}

void dpm_sample(struct dpm_hwmon_ops *pos)
{
	if (pos == NULL)
		return;
	if (pos->hi_dpm_update_power() < 0)
		pr_err("DPM %d sample fail\n", pos->dpm_module_id);
}

static void dpm_hwmon_sample_func(struct work_struct *work)
{
	if (!g_dpm_report_enabled)
		goto restart_work;
	dpm_sample_all();
restart_work:
	queue_delayed_work(system_freezable_power_efficient_wq,
		&g_dpm_hwmon_work,
		msecs_to_jiffies(DPM_SAMPLE_INTERVAL));
}
#endif

int dpm_hwmon_register(struct dpm_hwmon_ops *dpm_ops)
{
	struct dpm_hwmon_ops *pos = NULL;
	int ret, err;

	if (dpm_ops == NULL || dpm_ops->dpm_module_id < 0 ||
	    dpm_ops->dpm_module_id >= DPM_MODULE_NUM) {
		pr_err("%s LINE %d dpm_ops is invalid\n", __func__, __LINE__);
		return -EINVAL;
	}

	mutex_lock(&g_dpm_hwmon_ops_list_lock);
	list_for_each_entry(pos, &g_dpm_hwmon_ops_list, ops_list) {
		if (dpm_ops->dpm_module_id == pos->dpm_module_id) {
			pr_err("%s LINE %d dpm module %d has registered\n",
				__func__, __LINE__, dpm_ops->dpm_module_id);
			mutex_unlock(&g_dpm_hwmon_ops_list_lock);
			return -EINVAL;
		}
	}
	list_add(&dpm_ops->ops_list, &g_dpm_hwmon_ops_list);
	mutex_unlock(&g_dpm_hwmon_ops_list_lock);
#ifdef CONFIG_DPM_HWMON_PWRSTAT
	ret = map_powerstate_range(dpm_ops);
	if (ret < 0) {
		pr_err("%s: map dtsi fail", __func__);
		goto err_handler;
	}
	dpm_ops->sample_mode = PWRSTAT_PMU_MODE;
#else
	ret = malloc_dpm_ops_mem(dpm_ops);
	if (ret < 0) {
		pr_err("%s: malloc mem fail", __func__);
		goto err_handler;
	}
	dpm_ops->sample_mode = APB_PMU_MODE;
#endif

	dpm_ops->module_enabled = true;
	pr_err("dpm hwmon %d register!\n", dpm_ops->dpm_module_id);
	return ret;

err_handler:
	err = dpm_hwmon_unregister(dpm_ops);
	if (err < 0)
		pr_err("%s LINE %d register fail", __func__, __LINE__);
	return ret;
}

int dpm_hwmon_unregister(struct dpm_hwmon_ops *dpm_ops)
{
	struct dpm_hwmon_ops *pos = NULL;
	struct dpm_hwmon_ops *tmp = NULL;

	if (dpm_ops == NULL) {
		pr_err("%s LINE %d dpm_ops is NULL\n", __func__, __LINE__);
		return -EINVAL;
	}

	mutex_lock(&g_dpm_hwmon_ops_list_lock);
	list_for_each_entry_safe(pos, tmp, &g_dpm_hwmon_ops_list, ops_list) {
		if (dpm_ops->dpm_module_id == pos->dpm_module_id) {
#ifndef CONFIG_DPM_HWMON_PWRSTAT
			free_dpm_ops_mem(dpm_ops);
#endif
			list_del_init(&pos->ops_list);
			break;
		}
	}
	mutex_unlock(&g_dpm_hwmon_ops_list_lock);
	return 0;
}

void dpm_enable_module(struct dpm_hwmon_ops *pos, bool dpm_switch)
{
	if (pos != NULL)
		pos->module_enabled = dpm_switch;
}

bool get_dpm_enabled(struct dpm_hwmon_ops *pos)
{
	if (pos != NULL)
		return pos->module_enabled;
	else
		return false;
}

static void dpm_sample_all(void)
{
	struct dpm_hwmon_ops *pos = NULL;

	list_for_each_entry(pos, &g_dpm_hwmon_ops_list, ops_list)
		dpm_sample(pos);
}

static void dpm_set_sample_mode(struct dpm_hwmon_ops *pos, int mode)
{
	if (pos == NULL)
		return;
	pos->sample_mode = mode;
}

static bool g_powerstate_on;
static int dpm_hwmon_pm_callback(struct notifier_block *nb,
	unsigned long action, void *ptr)
{
	if (nb == NULL || ptr == NULL)
		pr_info("dpm_hwmon only for sc!\n");

	switch (action) {
	case PM_SUSPEND_PREPARE:
		g_powerstate_on = get_powerstate_state();
		if (g_powerstate_work.is_on) {
			dpm_sample_all();
			powerstat_disable();
		}
		pr_info("dpm_hwmon suspend\n");
		break;

	case PM_POST_SUSPEND:
		pr_info("dpm_hwmon resume +\n");
		if (g_powerstate_on)
			powerstat_enable(WORK_TOTAL_ENERGY_MODE,
				g_powerstate_work.channel_cfg,
				g_powerstate_work.sample_interval, 0);
		pr_info("dpm_hwmon resume -\n");
		break;

	default:
		return NOTIFY_DONE;
	}

	return NOTIFY_OK;
}

static struct notifier_block dpm_hwmon_pm_notif_block = {
	.notifier_call = dpm_hwmon_pm_callback,
};

int init_powerstate_work(void)
{
	int ret;
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL, "ipowerstate,master");
	if (np == NULL) {
		pr_err("%s not find\n", __func__);
		return -ENODEV;
	}

	ret = of_property_read_u32(np, "sample_interval", &g_powerstate_work.sample_interval);
	if (ret < 0) {
		pr_err("%s:%d no sample interval!\n");
		return -ENODEV;
	}

	ret = of_property_read_u32(np, "channel_max", &g_powerstate_work.channel_max);
	if (ret < 0 || g_powerstate_work.channel_max > MAX_CHANNEL) {
		pr_err("%s:%d no max channel!\n",
			__func__, g_powerstate_work.channel_max);
		return -ENODEV;
	}
	g_powerstate_work.channel_cfg = get_channel_config(0, g_powerstate_work.channel_max);
	g_powerstate_work.channel_power_table = kzalloc(sizeof(unsigned long long) *
						g_powerstate_work.channel_max,
						GFP_KERNEL);
	if (g_powerstate_work.channel_power_table == NULL)
		return -ENOMEM;

	return 0;
}

int dpm_hwmon_prepare(void)
{
	if (dpm_ioremap() < 0)
		return -EFAULT;
#ifdef CONFIG_DPM_HWMON_PWRSTAT
	if (init_powerstate_work() < 0)
		return -EFAULT;

	powerstat_enable(WORK_TOTAL_ENERGY_MODE, g_powerstate_work.channel_cfg,
		g_powerstate_work.sample_interval, 0);
#else
	/* dpm workqueue initialize */
	INIT_DEFERRABLE_WORK(&g_dpm_hwmon_work, dpm_hwmon_sample_func);
	queue_delayed_work(system_freezable_power_efficient_wq,
			&g_dpm_hwmon_work,
			msecs_to_jiffies(DPM_SAMPLE_INTERVAL));
#endif
	register_pm_notifier(&dpm_hwmon_pm_notif_block);
	return 0;
}

void dpm_hwmon_end(void)
{
#ifdef CONFIG_DPM_HWMON_PWRSTAT
	powerstat_disable();
	if (g_powerstate_work.channel_power_table != NULL) {
		kfree(g_powerstate_work.channel_power_table);
		g_powerstate_work.channel_power_table = NULL;
	}
#else
	cancel_delayed_work(&g_dpm_hwmon_work);
#endif
	dpm_iounmap();
}
