/*
 *
 * hck cpufreq module
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#include <linux/init.h>
#include <platform_include/cee/linux/hck/lpcpu/cpufreq/hck_lpcpu_cpufreq.h>
#include <linux/cpufreq.h>
#include <linux/topology.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
static void lpcpu_cpufreq_policy_kobject_init(struct kobject *kobj, struct kobj_type *ktype,
					      struct kobject *parent, const char *fmt,
					      unsigned int cpu, int *ret, int *init_flag)
{
	*init_flag = 0;
#ifdef CONFIG_ARCH_PLATFORM
	*ret = kobject_init_and_add(kobj, ktype, parent, fmt, cpu);
	*init_flag = 1;
#endif
}

static int __init hck_lpcpu_cpufreq_register(void)
{
	REGISTER_HCK_VH(lpcpu_cpufreq_policy_kobject_init,
			lpcpu_cpufreq_policy_kobject_init);
	return 0;
}

early_initcall(hck_lpcpu_cpufreq_register);

#endif
