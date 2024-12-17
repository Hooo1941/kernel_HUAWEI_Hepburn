/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
 * Description: npu power management
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "npu_compile.h"
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/devfreq.h>
#include <linux/math64.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/pm.h>
#include <linux/pm_opp.h>
#include <linux/arm-smccc.h>
#ifdef CONFIG_NPUFREQ_PLATFORM_QOS
#include <linux/platform_drivers/platform_qos.h>
#else
#include <linux/pm_qos.h>
#endif

#if defined(CONFIG_PLATDRV_SUPPORT_FFA) && defined(CONFIG_NPU_PM_FFA_TRANSPORT)
#include <platform_include/see/ffa/ffa_plat_drv.h>
#endif
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/of.h>

#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <asm/compiler.h>
#include <platform_include/cee/linux/npu_pm.h>
#include "npu_pm_private.h"

#define NPU_PM_SMC_FID	0xc500dea0U /* smc service id */

enum {
	NPU_PM_SMC_PWRON_REQ,
	NPU_PM_SMC_PWROFF_REQ,
	NPU_PM_SMC_DVFS_REQ,
	NPU_PM_SMC_DBG_REQ,
	NPU_PM_SMC_SET_VOLT_REQ,
	NPU_PM_SMC_GET_VOLT_REQ,
	NPU_PM_SMC_INIT_REQ,
};

struct npu_pm_device *g_npu_pm_dev;
DEFINE_MUTEX(power_mutex);

#if defined(CONFIG_PLATDRV_SUPPORT_FFA) && defined(CONFIG_NPU_PM_FFA_TRANSPORT)
static int ffa_sync_send_receive_request(u64 _type, u64 _arg0, u64 _arg1, u64 _arg2)
{
	int ret;
	struct ffa_send_direct_data args = {
		.data0 = (unsigned long)NPU_PM_SMC_FID,
		.data1 = (unsigned long)_type,
		.data2 = (unsigned long)_arg0,
		.data3 = (unsigned long)_arg1,
		.data4 = (unsigned long)_arg2,
	};
	ret = ffa_platdrv_send_msg(&args);
	if (ret == 0)
		return (int)args.data1;

	return ret;
}
#endif

/*
 * brief smc caller for npu frequency scale
 *
 * param _arg0 param0 pass to atf
 * param _arg1 param1 pass to atf
 * param _arg2 param2 pass to atf
 * return atf execute result
 */
noinline int npu_pm_smc_request(u64 _type, u64 _arg0, u64 _arg1, u64 _arg2)
{
#if defined(CONFIG_PLATDRV_SUPPORT_FFA) && defined(CONFIG_NPU_PM_FFA_TRANSPORT)
	return ffa_sync_send_receive_request(_type, _arg0, _arg1, _arg2);
#else
	struct arm_smccc_res res;

#ifndef CONFIG_LIBLINUX
	arm_smccc_smc(NPU_PM_SMC_FID, _type, _arg0, _arg1, _arg2, 0, 0, 0, &res);
#else
	arm_smccc_1_1_smc(NPU_PM_SMC_FID, _type, _arg0, _arg1, _arg2, 0, 0, 0, &res);
#endif

	return (int)res.a0;
#endif
}

int npu_pm_pwron_smc_request(unsigned long freq, int temp)
{
	return npu_pm_smc_request(NPU_PM_SMC_PWRON_REQ,
				       freq,
				       (u64)temp,
				       0);
}

int npu_pm_pwroff_smc_request(int temp)
{
	return npu_pm_smc_request(NPU_PM_SMC_PWROFF_REQ,
				       (u64)temp,
				       0,
				       0);
}

int npu_pm_dvfs_request_smc_request(u64 tar_freq, int temp)
{
	return npu_pm_smc_request(NPU_PM_SMC_DVFS_REQ,
				       tar_freq,
				       (u64)temp,
				       0);
}

static int npu_pm_init_smc_request(void)
{
	return npu_pm_smc_request(NPU_PM_SMC_INIT_REQ,
				       0,
				       0,
				       0);
}

void npu_pm_dbg_smc_request(unsigned int enable)
{
#ifdef CONFIG_NPU_PM_DEBUG
	(void)npu_pm_smc_request(NPU_PM_SMC_DBG_REQ, enable, 0, 0);
#else
	(void)enable;
#endif
}

int npu_pm_set_voltage_smc_request(unsigned int voltage)
{
#ifdef CONFIG_NPU_PM_DEBUG
	return npu_pm_smc_request(NPU_PM_SMC_SET_VOLT_REQ,
				       voltage,
				       0,
				       0);
#else
	(void)voltage;

	return -ENODEV;
#endif
}

int npu_pm_get_voltage_smc_request(void)
{
#ifdef CONFIG_NPU_PM_DEBUG
	return npu_pm_smc_request(NPU_PM_SMC_GET_VOLT_REQ,
				       0,
				       0,
				       0);
#else
	return -ENODEV;
#endif
}

static inline void npu_pm_devfreq_lock(struct devfreq *df)
{
	if (!IS_ERR_OR_NULL(df))
		mutex_lock(&df->lock);
}

static inline void npu_pm_devfreq_unlock(struct devfreq *df)
{
	if (!IS_ERR_OR_NULL(df))
		mutex_unlock(&df->lock);
}

int npu_pm_power_on(void)
{
	struct devfreq *devfreq = NULL;
	ktime_t in_ktime;
	unsigned long delta_time;
	unsigned long last_freq;
	int ret;

	if (IS_ERR_OR_NULL(g_npu_pm_dev)) {
		pr_err("%s %d: npu pm device not exist!\n", __func__, __LINE__);
		return -ENODEV;
	}
	devfreq = g_npu_pm_dev->devfreq;
	if (IS_ERR_OR_NULL(devfreq)) {
		pr_err("%s %d: npu pm devfreq devices not exist!\n", __func__, __LINE__);
		return -ENODEV;
	}
	g_npu_pm_dev->power_on_count++;

	mutex_lock(&power_mutex);

	in_ktime = ktime_get();

	if (g_npu_pm_dev->power_on) {
		mutex_unlock(&power_mutex);
		return 0;
	}

	last_freq = devfreq->previous_freq;
	g_npu_pm_dev->last_freq = last_freq;
	g_npu_pm_dev->target_freq = last_freq;

	npu_pm_devfreq_lock(devfreq);

	ret = npu_pm_pwr_on(last_freq);
	if (ret != 0) {
		pr_err("%s %d: npu_pm_pwr_on failed!\n", __func__, __LINE__);
		goto err_power_on;
	}

	g_npu_pm_dev->power_on = true;

	if (!IS_ERR_OR_NULL(g_npu_pm_dev->dvfs_data))
		g_npu_pm_dev->dvfs_data->dvfs_enable = true;

	npu_pm_devfreq_unlock(devfreq);

	/* must out of devfreq lock */
	ret = npu_pm_devfreq_resume(devfreq);
	if (ret != 0)
		pr_err("%s %d: Resume device failed, ret=%d!\n", __func__, __LINE__, ret);

	delta_time = (unsigned long)ktime_to_ns(ktime_sub(ktime_get(), in_ktime));
	if (delta_time > g_npu_pm_dev->max_pwron_time) {
		g_npu_pm_dev->max_pwron_time = delta_time;
		if (delta_time >= MAX_TIME_NS)
			pr_err("%s %d: Take time more than 500ms!!!\n", __func__, __LINE__);
	}

	mutex_unlock(&power_mutex);

	return 0;

err_power_on:
	npu_pm_devfreq_unlock(devfreq);
	mutex_unlock(&power_mutex);

	return ret;
}

int npu_pm_power_off(void)
{
	struct devfreq *devfreq = NULL;
	ktime_t in_ktime;
	unsigned long delta_time;
	int ret;

	if (IS_ERR_OR_NULL(g_npu_pm_dev)) {
		pr_err("%s %d: npu pm device not exist!\n", __func__, __LINE__);
		return -ENODEV;
	}
	devfreq = g_npu_pm_dev->devfreq;
	if (IS_ERR_OR_NULL(devfreq)) {
		pr_err("%s %d: npu pm devfreq devices not exist!\n", __func__, __LINE__);
		return -ENODEV;
	}
	g_npu_pm_dev->power_off_count++;

	mutex_lock(&power_mutex);

	if (!g_npu_pm_dev->power_on) {
		mutex_unlock(&power_mutex);
		return 0;
	}

	in_ktime = ktime_get();

	/* out of devfreq lock */
	ret = npu_pm_devfreq_suspend(devfreq);
	if (ret != 0)
		pr_err("%s %d: Suspend device failed, ret=%d!\n", __func__, __LINE__, ret);

	npu_pm_devfreq_lock(devfreq);

	if (!IS_ERR_OR_NULL(g_npu_pm_dev->dvfs_data))
		g_npu_pm_dev->dvfs_data->dvfs_enable = false;

	ret = npu_pm_pwr_off();
	if (ret != 0) {
		pr_err("%s %d: npu_pm_pwr_off failed!\n", __func__, __LINE__);
		goto err_power_off;
	}

	g_npu_pm_dev->power_on = false;

err_power_off:
	npu_pm_devfreq_unlock(devfreq);

	delta_time = (unsigned long)ktime_to_ns(ktime_sub(ktime_get(), in_ktime));
	if (delta_time > g_npu_pm_dev->max_pwroff_time) {
		g_npu_pm_dev->max_pwroff_time = delta_time;
		if (delta_time >= MAX_TIME_NS)
			pr_err("%s %d: Take time more than 500ms!!!\n", __func__, __LINE__);
	}

	mutex_unlock(&power_mutex);

	return ret;
}

static int npu_pm_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	unsigned int init_freq = 0;
	int ret;

	g_npu_pm_dev = devm_kzalloc(dev, sizeof(*g_npu_pm_dev), GFP_KERNEL);
	if (g_npu_pm_dev == NULL) {
		ret = -ENOMEM;
		goto err_out;
	}

	mutex_init(&g_npu_pm_dev->mutex);

	ret = of_property_read_u32(dev->of_node, "initial_freq", &init_freq);
	if (ret != 0) {
		dev_err(dev, "parse npu initial frequency fail%d\n", ret);
		ret = -EINVAL;
		goto err_out;
	}
	g_npu_pm_dev->last_freq = (unsigned long)init_freq * KHZ;
	g_npu_pm_dev->target_freq = g_npu_pm_dev->last_freq;

	g_npu_pm_dev->dev = dev;
#ifdef CONFIG_NPUFREQ_PLATFORM_QOS
	g_npu_pm_dev->pm_qos_class = PLATFORM_QOS_NPU_FREQ_DNLIMIT;
#else
	g_npu_pm_dev->pm_qos_class = PM_QOS_NPU_FREQ_DNLIMIT;
#endif
	g_npu_pm_dev->power_on = false;

	ret = npu_pm_dvfs_init(g_npu_pm_dev);
	if (ret != 0) {
		dev_err(dev, "npu dvfs init fail%d\n", ret);
		ret = -EINVAL;
		goto err_out;
	}

	ret = npu_pm_init_smc_request();
	if (ret != 0) {
		dev_err(dev, "npu pm para init fail%d\n", ret);
		ret = -EINVAL;
		goto err_out;
	}

	ret = npu_pm_devfreq_init(g_npu_pm_dev);
	if (ret != 0)
		dev_err(dev, "npu devfreq init fail%d\n", ret);

	npu_pm_debugfs_init(g_npu_pm_dev);

err_out:
	of_node_put(dev->of_node);

	return ret;
}

#ifdef CONFIG_OF
static const struct of_device_id npu_pm_of_match[] = {
	{
		.compatible = "lpm,npu-pm",
	},
	{},
};

MODULE_DEVICE_TABLE(of, npu_pm_of_match);
#endif

static struct platform_driver npu_pm_driver = {
	.probe  = npu_pm_probe,
	.driver = {
			.name = "lpm-npu-pm",
			.owner = THIS_MODULE,
			.of_match_table = of_match_ptr(npu_pm_of_match),
		},
};

static int __init npu_pm_init(void)
{
	return platform_driver_register(&npu_pm_driver);
}

device_initcall(npu_pm_init);

static void __exit npu_pm_exit(void)
{
	npu_pm_debugfs_exit();

	npu_pm_devfreq_term(g_npu_pm_dev);

	platform_driver_unregister(&npu_pm_driver);
}

module_exit(npu_pm_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("npu power manager framework");
