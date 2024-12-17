/*
 * cpufreq_common.c
 *
 * for hisilicon efficinecy control algorithm cpufreq mips governor.
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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
#include "sched.h"
#include <linux/sched/cpufreq.h>

#if defined(CONFIG_CPU_FREQ_GOV_SCHEDUTIL_OPT) || defined(CONFIG_CPU_FREQ_GOV_MIPS)
BLOCKING_NOTIFIER_HEAD(cpugov_status_notifier_list);
int cpugov_register_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&cpugov_status_notifier_list, nb);
}

int cpugov_unregister_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&cpugov_status_notifier_list, nb);
}

void gov_sysfs_set_attr(unsigned int cpu, char *gov_name,
			struct governor_user_attr *attrs)
{
	char *buf = NULL;
	int i = 0;
	int gov_dir_len, gov_attr_len;
	long ret;
	mm_segment_t fs = 0;

	if (!capable(CAP_SYS_ADMIN))
		return;

	buf = kzalloc(PATH_MAX, GFP_KERNEL);
	if (!buf)
		return;

	gov_dir_len = scnprintf(buf, PATH_MAX,
				"/sys/devices/system/cpu/cpu%u/cpufreq/%s/",
				cpu, gov_name);

	fs = get_fs();
	set_fs(KERNEL_DS);

	while (attrs[i].name) {
		gov_attr_len = scnprintf(buf + gov_dir_len,
					 PATH_MAX - gov_dir_len, "%s", attrs[i].name);
		if (gov_dir_len + gov_attr_len >= PATH_MAX) {
			i++;
			continue;
		}
		buf[gov_dir_len + gov_attr_len] = '\0';

		ret = ksys_chown(buf, attrs[i].uid, attrs[i].gid);
		if (ret)
			pr_err("chown fail:%s ret=%ld\n", buf, ret);

		ret = ksys_chmod(buf, attrs[i].mode);
		if (ret)
			pr_err("chmod fail:%s ret=%ld\n", buf, ret);
		i++;
	}

	set_fs(fs);
	kfree(buf);
}
EXPORT_PER_CPU_SYMBOL_GPL(gov_sysfs_set_attr);
#endif

unsigned int *cpufreq_get_tokenized_data(const char *buf, int *num_tokens)
{
	const char *cp;
	int i;
	int ntokens = 1;
	unsigned int *tokenized_data;
	int err = -EINVAL;

	cp = buf;
	while ((cp = strpbrk(cp + 1, " :")))
		ntokens++;

	if (!(ntokens & 0x1))
		goto err;

	tokenized_data = kmalloc(ntokens * sizeof(unsigned int), GFP_KERNEL);
	if (!tokenized_data) {
		err = -ENOMEM;
		goto err;
	}

	cp = buf;
	i = 0;
	while (i < ntokens) {
		if (sscanf(cp, "%u", &tokenized_data[i++]) != 1)
			goto err_kfree;

		cp = strpbrk(cp, " :");
		if (!cp)
			break;
		cp++;
	}

	if (i != ntokens)
		goto err_kfree;

	*num_tokens = ntokens;
	return tokenized_data;

err_kfree:
	kfree(tokenized_data);
err:
	return ERR_PTR(err);
}
EXPORT_PER_CPU_SYMBOL_GPL(cpufreq_get_tokenized_data);
