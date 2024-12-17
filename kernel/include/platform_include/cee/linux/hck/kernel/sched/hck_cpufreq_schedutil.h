#ifndef __HCK_CPUFREQ_SCHEDUTIL_H__
#define __HCK_CPUFREQ_SCHEDUTIL_H__

#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))

#include <linux/types.h>
#include <linux/cpufreq.h>
#include <linux/irq_work.h>
#include <linux/kthread.h>

#ifdef CONFIG_CPU_FREQ_GOV_SCHEDUTIL_OPT
#include <linux/uidgid.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#endif

#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>

struct gov_attr_set;
struct irq_work;
struct kthread_work;
struct kthread_worker;
struct sugov_policy;

#ifdef CONFIG_CPU_FREQ_GOV_SCHEDUTIL_OPT
#define IOWAIT_BOOST_INC_STEP 200000 /* 200MHz */
#define IOWAIT_BOOST_CLEAR_NS 8000000 /* 8ms */

struct sugov_tunables {
	struct gov_attr_set	attr_set;
	unsigned int		rate_limit_us;

#define DEFAULT_OVERLOAD_DURATION_MS	250
	/* boost freq to max when running without idle over this duration time */
	unsigned int overload_duration;
	/* Hi speed to bump to from lo speed when load burst (default max) */
	unsigned int hispeed_freq;

	/* Go to hi speed when CPU load at or above this value. */
#define DEFAULT_GO_HISPEED_LOAD 100
	unsigned long go_hispeed_load;

	/* Target load. Lower values result in higher CPU speeds. */
	spinlock_t target_loads_lock;
	unsigned int *target_loads;
	int ntarget_loads;

	/*
	 * Wait this long before raising speed above hispeed, by default a
	 * single timer interval.
	*/
	spinlock_t above_hispeed_delay_lock;
	unsigned int *above_hispeed_delay;
	int nabove_hispeed_delay;

	/*
	 * The minimum amount of time to spend at a frequency before we can ramp
	 * down.
	 */
	spinlock_t min_sample_time_lock;
	unsigned int *min_sample_time;
	int nmin_sample_time;

	/* Duration of a boot pulse in usecs */
#define DEFAULT_BOOSTPULSE_DURATION	(80 * USEC_PER_MSEC)
	int boostpulse_duration;
	/* End time of boost pulse in ktime converted to usecs */
	u64 boostpulse_endtime;
	bool boosted;
	/* Minimun boostpulse interval */
#define DEFAULT_MIN_BOOSTPULSE_INTERVAL (500 * USEC_PER_MSEC)
	int boostpulse_min_interval;

	/*
	 * Max additional time to wait in idle, beyond timer_rate, at speeds
	 * above minimum before wakeup to reduce speed, or -1 if unnecessary.
	 */
#define DEFAULT_TIMER_SLACK (80 * USEC_PER_MSEC)
	int timer_slack;

	bool io_is_busy;
	unsigned int iowait_boost_step;
#ifdef CONFIG_FREQ_IO_LIMIT
	unsigned int iowait_upper_limit;
#endif

	bool fast_ramp_up;
	bool fast_ramp_down;
#define DEFAULT_FREQ_REPORTING_POLICY \
	(FREQ_STAT_CPU_LOAD | FREQ_STAT_TOP_TASK)
	unsigned int freq_reporting_policy;

#ifdef CONFIG_SCHED_TOP_TASK
	unsigned int top_task_hist_size;
	unsigned int top_task_stats_policy;
	bool top_task_stats_empty_window;
#endif

#ifdef CONFIG_ED_TASK
	unsigned int ed_task_running_duration;
	unsigned int ed_task_waiting_duration;
	unsigned int ed_new_task_running_duration;
#endif

#ifdef CONFIG_MIGRATION_NOTIFY
	unsigned int freq_inc_notify;
	unsigned int freq_dec_notify;
#endif
};

struct sugov_policy {
	struct cpufreq_policy	*policy;

	struct sugov_tunables	*tunables;
	struct list_head	tunables_hook;

	raw_spinlock_t		update_lock;	/* For shared policies */
	u64			last_freq_update_time;
	s64			freq_update_delay_ns;
	unsigned int		next_freq;
	unsigned int		cached_raw_freq;

	/* The next fields are only needed if fast switch cannot be used: */
	struct			irq_work irq_work;
	struct			kthread_work work;
#ifdef CONFIG_CPU_FREQ_GOV_MIPS
	struct			kthread_work sysfs_work;
#endif
	struct			mutex work_lock;
	struct			kthread_worker worker;
	struct task_struct	*thread;
	bool			work_in_progress;

	bool			limits_changed;
	bool			need_freq_update;

	u64 overload_duration_ns;
	u64 floor_validate_time;
	u64 hispeed_validate_time;
	u64 update_time;
	/* protect slack timer */
	raw_spinlock_t timer_lock;
	/* policy slack timer */
	struct timer_list pol_slack_timer;
	unsigned int min_freq; /* in kHz */
	/* remember last active cpu and set slack timer on it */
	unsigned int trigger_cpu;
	/* max util cpu */
	unsigned int max_cpu;
	unsigned int iowait_boost;
	u64 last_iowait;
#ifdef CONFIG_SCHED_RTG
	unsigned long rtg_util;
	unsigned int rtg_freq;
#endif
	atomic_t skip_min_sample_time;
	atomic_t skip_hispeed_logic;
	bool util_changed;
	bool governor_enabled;
#ifdef CONFIG_SCHED_COLOC_GROUP
	unsigned int coloc_grp_util;
#endif
};

struct sugov_cpu {
	struct update_util_data	update_util;
	struct sugov_policy	*sg_policy;
	unsigned int		cpu;

	bool			iowait_boost_pending;
	unsigned int		iowait_boost;
	u64			last_update;

	unsigned long		bw_dl;
	unsigned long		max;

	/* The field below is for single-CPU policies only: */
#ifdef CONFIG_NO_HZ_COMMON
	unsigned long		saved_idle_calls;
#endif

	unsigned long util;
	unsigned int flags;

	u64 idle_update_ts;
	u64 last_idle_time;

	unsigned int iowait_boost_max;
	unsigned int iowait_boost_min;
	unsigned int iowait_boost_step;

	bool use_max_freq;
	bool enabled;
};

#define SYSTEM_UID (uid_t)1000
#define SYSTEM_GID (uid_t)1000

#define INVALID_ATTR \
	{.name = NULL, .uid = (uid_t)(-1), .gid = (uid_t)(-1), .mode = 0000}

extern __read_mostly unsigned int sched_io_is_busy;
extern struct governor_attr overload_duration;
extern struct governor_attr go_hispeed_load;
extern struct governor_attr hispeed_freq;
extern struct governor_attr target_loads;
extern struct governor_attr above_hispeed_delay;
extern struct governor_attr min_sample_time;
extern struct governor_attr boostpulse;
extern struct governor_attr boostpulse_duration;
extern struct governor_attr io_is_busy;
extern struct governor_attr timer_slack;
extern struct governor_attr fast_ramp_down;
extern struct governor_attr fast_ramp_up;
extern struct governor_attr freq_reporting_policy;
extern struct governor_attr iowait_boost_step;
#ifdef CONFIG_SCHED_TOP_TASK
extern struct governor_attr top_task_hist_size;
extern struct governor_attr top_task_stats_policy;
extern struct governor_attr top_task_stats_empty_window;
#endif
#ifdef CONFIG_ED_TASK
extern struct governor_attr ed_task_running_duration;
extern struct governor_attr ed_task_waiting_duration;
extern struct governor_attr ed_new_task_running_duration;
#endif
#ifdef CONFIG_MIGRATION_NOTIFY
extern struct governor_attr freq_inc_notify;
extern struct governor_attr freq_dec_notify;
#endif
#ifdef CONFIG_FREQ_IO_LIMIT
extern struct governor_attr iowait_upper_limit;
#endif

extern struct governor_user_attr schedutil_user_attrs[];
extern struct blocking_notifier_head cpugov_status_notifier_list;
DECLARE_PER_CPU(struct sugov_cpu, sugov_cpu);

void walt_sugov_work(struct kthread_work *work);
inline struct sugov_tunables *to_sugov_tunables(struct gov_attr_set *attr_set);
void sugov_deferred_update(struct sugov_policy *sg_policy, u64 time, unsigned int next_freq);
void sugov_tunables_init(struct cpufreq_policy *policy, struct sugov_tunables *tunables);
void sugov_tunables_restore(struct cpufreq_policy *policy);
void gov_sysfs_set_attr(unsigned int cpu, char *gov_name, struct governor_user_attr *attrs);
void sugov_tunables_save(struct cpufreq_policy *policy, struct sugov_tunables *tunables);
void sugov_nop_timer(struct timer_list *unused);

unsigned long sugov_get_util(struct sugov_cpu *sg_cpu);
unsigned long sugov_iowait_apply(struct sugov_cpu *sg_cpu, u64 time,
					unsigned long util, unsigned long max);
unsigned int get_next_freq(struct sugov_policy *sg_policy,
				  unsigned long util, unsigned long max);
#endif

DECLARE_HCK_VH(map_util_freq_hook,
	TP_PROTO(struct sugov_policy *sg_policy, unsigned long util, unsigned long max,
		unsigned long *next_freq),
	TP_ARGS(sg_policy, util, max, next_freq));
#endif
#endif /* __HCK_CPUFREQ_SCHEDUTIL_H__ */
