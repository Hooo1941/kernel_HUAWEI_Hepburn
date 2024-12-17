
#include <linux/cpumask.h>
#include <linux/psi_types.h>
#include <linux/psi.h>
#include <linux/sched/loadavg.h>
#include <linux/cpufreq.h>
#include <linux/sched.h>
#include <../../../kernel/sched/sched.h>
#include <linux/kernel_stat.h>
#include <linux/tick.h>

#include "utils/perf_buffered_log_sender.h"
#include "perf_ioctl.h"
#include "utils/perf_utils.h"
#include "perf_system_info.h"

/* 0->avg10, 1->avg60, 2->avg300 */
#define PERF_SYSINFO_INDEX_OF_PSI_AVG10 0

struct perf_cpu_psi_avg {
	unsigned long avg_int;
	unsigned long avg_dec;
};

struct perf_cpu_status_info {
	unsigned int max_freq;
	unsigned int cur_freq;
	unsigned int cfs_nr_running;
	unsigned int rt_nr_running;
	struct perf_cpu_time_info cpu_time_info;
};

struct perf_system_status_info {
	unsigned int ddr_freq;
	unsigned int gpu_freq;
	int temp;
	unsigned int cpu_activemask;
	struct perf_cpu_psi_avg cpu_psi_avg_10;
	struct perf_cpu_status_info cpu_info[0];
};

/* stub now */
static int perf_sysinfo_get_ddr_freq(void)
{
	return 0;
}

/* stub now */
static int perf_sysinfo_get_gpu_freq(void)
{
	return 0;
}

static int perf_sysinfo_get_temp(void)
{
	/*
	* The hosp branch does not provide the 'hw_battery_temperature' interface
	* for obtaining the temperature.
	* Therefore, 0 is returned temporarily.
	*/
	return 0;
}

void perf_sysinfo_get_psi_avg_10(struct perf_cpu_psi_avg *avg10)
{
	struct psi_group *group;
	unsigned long avg;

	group = &psi_system;
	avg = group->avg[PSI_CPU * 2][PERF_SYSINFO_INDEX_OF_PSI_AVG10];

	/* "avg10=%lu.%02lu", LOAD_INT(avg), LOAD_FRAC(avg) */
	avg10->avg_int = LOAD_INT(avg);
	avg10->avg_dec = LOAD_FRAC(avg);

	return;
}

static void perf_sysinfo_get_common_info(
	struct perf_system_status_info *sys_info, int core_num)
{
	sys_info->ddr_freq = perf_sysinfo_get_ddr_freq();
	sys_info->gpu_freq = perf_sysinfo_get_gpu_freq();
	sys_info->temp = perf_sysinfo_get_temp();
	sys_info->cpu_activemask = cpu_active_mask->bits[0];
	perf_sysinfo_get_psi_avg_10(&sys_info->cpu_psi_avg_10);

	return;
}

static int perf_sysinfo_get_per_core_freq(unsigned int *max_freq,
	unsigned int *cur_freq, int cpu)
{
	struct cpufreq_policy *cpu_policy;

	cpu_policy = cpufreq_cpu_get(cpu);
	if (!cpu_policy)
		return -EINVAL;

	*max_freq = cpu_policy->max;
	*cur_freq = cpu_policy->cur;

	return 0;
}

static void perf_sysinfo_get_per_core_rq_nr(unsigned int *cfs_rq_nr,
	unsigned int *rt_rq_nr, int cpu)
{
	struct rq *rq;
	struct cfs_rq *cfs;
	struct rt_rq *rt;

	rq = cpu_rq(cpu);

	cfs = &rq->cfs;
	rt = &rq->rt;

	*cfs_rq_nr = cfs->nr_running;
	*rt_rq_nr = rt->rt_nr_running;
}

static int perf_sysinfo_get_per_core_cpu_info(
	struct perf_cpu_status_info *per_cpu_info, int cpu)
{
	int ret;

	perf_sysinfo_get_per_core_rq_nr(&per_cpu_info->cfs_nr_running,
		&per_cpu_info->rt_nr_running, cpu);
	perf_sysinfo_get_per_core_cpu_time(&per_cpu_info->cpu_time_info, cpu);
	ret = perf_sysinfo_get_per_core_freq(&per_cpu_info->max_freq,
		&per_cpu_info->cur_freq, cpu);

	return ret;
}

static int perf_sysinfo_get_cpu_info(struct perf_cpu_status_info *cpu_info)
{
	int cpu = 0;
	int ret;

	for_each_online_cpu(cpu) {
		ret = perf_sysinfo_get_per_core_cpu_info(&cpu_info[cpu], cpu);
		if (ret < 0) {
			pr_err("PerfD %s: get system core %d info err, ret %d.\n",
				__func__, cpu, ret);
			break;
		}
	}

	return ret;
}

static int perf_system_info_get_data(struct perf_system_status_info *sys_info,
	int core_num)
{
	int ret;

	perf_sysinfo_get_common_info(sys_info, core_num);
	ret = perf_sysinfo_get_cpu_info(sys_info->cpu_info);

	return ret;
}

long perf_ioctl_system_info(unsigned int cmd, void __user *argp)
{
	int ret = -EINVAL;
	int core_num;
	unsigned int entry_size;
	struct perf_buffered_log_entry *entry = NULL;

	core_num = perf_get_cpu_num();

	entry_size = sizeof(struct perf_buffered_log_entry) +
		sizeof(struct perf_system_status_info) +
		core_num * sizeof(struct perf_cpu_status_info);

	entry = perf_create_log_entry(entry_size, IOC_SYSTEM_INFO, 1);
	if (entry == NULL) {
		pr_err("PerfD %s: failed to create log entry\n", __func__);
		goto error;
	}

	ret = perf_system_info_get_data(
		(struct perf_system_status_info *)(&entry->data), core_num);
	if (ret < 0) {
		pr_err("PerfD %s: failed to get system info data, ret %d\n",
			__func__, ret);
		goto error;
	}

	ret = send_perf_buffered_log_entry(entry);
	if (ret < 0) {
		pr_err("PerfD %s: failed to send log entry ret %d\n", __func__, ret);
		goto error;
	}

error:
	free_perf_buffered_log_entry(entry);
	return ret;
}