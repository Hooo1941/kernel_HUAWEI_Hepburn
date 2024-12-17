/*
 * Copyright (c) 2022-2023 Huawei Technologies Co., Ltd.
 * Description : a new idle governor named aligned governor.
 *
 * In smp and big-little cpu system, different types of cpu has different power statics.
 * Besides, for mobile device, comparing with benchmark, Dou is the major using scenes.
 * So this governor is designed dedicatedly for big and little core and mainly serves for Dou.
 *
 * In menu and teo governor, timer/tick is the most significant factor to predict sleep time.
 * But in hisilicon mobile system, excluding timer, thread waking and irq is far more frequent
 * to wake cpu up. And comparing timer, these waking source's prediction is not so easy.
 * So align governor will include information from schedule and irq module, to predict
 * as accurately as possible.
 *
 * Another innovative point is making all cpus enter deep idle as synchronously as possible.
 * First of all, have an introduction of why to do this.
 * For current hisilicon system, little cpu's single-core power is tiny but cluster idle state's
 * power saving is much more significant. So, to get best power status, governor should control
 * little cpu enter deepest idle as aligned and fast as possible.
 * But the point is different in big cores, most of the big cores has mtcmos. So its power down
 * earns a lot of power saving. Therefore, single big core should enter core idle power down more
 * frequent than little ones.
 * Secondly, how to achieve above target?
 * In hisi menu governor, hrtimer is used to wakeup shallow idle cpu to enter deeper idle.
 * But this way can hardly work to make cpu aligned idle.
 * Firstly, timer's wakeup and handling flow is pretty long; in addition, timer must be set one by
 * one for cpu to be waken, but when enter aligned clusteridle, wakeup source is supposed multiples.
 * In conclusion, percpu timer doesn't work well for aligned idle governor.
 * As an alternative, we choose one ipi (nr num is 11) as wake source.
 *
 * In detail, for little core, when there are running task in scheduler, clusteridle is impossible,
 * simply choosing wfi is enough for system. Besides, timer should also be in consider
 * when select idle index satisfying residency and latency request.
 * So when running thread is clear and predictable waking source is late enough, send ipi to enter
 * clusteridle. For big cores, governor behaves not the same. Threads running on big core are
 * mostly scheduled in random.Therefore, its thread load can hardly be predicted.
 * In this condition, history sleep time is used to predict.
 *
 * After above introduction, if you still want more details, codes and comments perhaps help a lot.
 * Welcome to read and comment on this governor.
 *
 */
#include <asm/smp.h>
#include <linux/kernel.h>
#include <linux/cpuidle.h>
#include <linux/ktime.h>
#include <linux/tick.h>
#include <linux/sched.h>
#include <linux/sched/stat.h>
#include <linux/module.h>
#include <linux/cpumask.h>
#include <linux/hrtimer.h>
#include <linux/slab.h>
#include <linux/sched/smt.h>
#include <securec.h>
#include <platform_include/cee/linux/lpcpu_idle_drv.h>

#define HIST_INTERVAL_CNT       20
#define LATEST_IDX_CNT          3
#define UNINIT                  (~0U)
#define IPI_MAX_NUM             16
#define PPI_MAX_NUM             32
#define IDLE_PATTERN_THREHOLD   5
#define FRAME_INTERVAL_NS       16700000
#define HIST_HWIRQ_INTER_CNT    4

/*
 * fcm idle data: only one entity in system.
 * @schedidle: record if or not enter fcm idle.
 * @shallow_idle_cpumask: record cpus which is not in deepest state
 * @next_timer_global: nearest timer between all cpus.
 * @next_timer_percpu: percpu next timer.
 * @residency_ns: fcm idle target residency.
 * @lock: protect shareable in this struc.
 * @basecpumask: the cpus handling background thread,
 *   using thread info to predict idle index instead history.
 * @ppi_timer: timer ppi hw irq num.
 * @spi_timer: timer spi hw irq num.
 */
struct align_fcmidle_data {
	bool schedidle;
	struct cpumask shallow_idle_cpumask;
	ktime_t next_timer_global;
	ktime_t next_timer_percpu[NR_CPUS];
	u64 target_residency_ns;
	u64 base_residency_ns;

	spinlock_t lock;

	struct cpumask basecpumask;

	int ppi_timer;
	int spi_timer;
	ktime_t hwirq_wakeup_time[HIST_HWIRQ_INTER_CNT];
};
struct align_fcmidle_data *g_fcm_data;

/*
 * align_idle_pattern: some flags indicate current situation.
 * @heavy_load: current cpu is busy or high-loading such as Gaming.
 * @shortidle_continuous: short idle is mainly continuous.
 */
struct align_idle_pattern {
	bool heavy_load;
	bool shortidle_continuous;
};

/*
 * Concepts and ideas behind this idle governor:
 * @last_state_idx: last idle index.
 * @history_vote: the index selected by history prediction.
 * @wakeup_ipi: waken up by wakeup ipi or timer.
 * @next_timer_ns: next timer in current cpu.
 * @wakeup_timer_ns: timer to wake shallow idle.
 * @cpu: cpuid.
 * @wakeup_hwirq: wakeup irq number, only update once each wakeup.
 * @irq_wakeup_time: irq wakeup time stamp.
 * @history_event_idx: next update data index.
 * @history_resi_ns: history sleep length.
 * @latest_idx: most recent idle index.
 * @enter_deepest_idle: if direct enter deepest idle or not.
 * @enter_wfi: whether enter wfi. Most for basecpu,
 *   because running thread is not zero
 */
struct align_cpu_data {
	int last_state_idx;
	int history_vote;
	bool wakeup_ipi;
	u64 next_timer_ns;
	u64 wakeup_timer_ns;
	int cpu;
	u64 longidle_threshold_ns;

	unsigned int wakeup_hwirq;
	ktime_t irq_wakeup_time;

	u8 history_event_idx;
	u64 history_resi_ns[HIST_INTERVAL_CNT];
	u64 last_residency_ns;
	u8 latest_idx[LATEST_IDX_CNT];

	bool enter_deepest_idle;
	bool enter_wfi;

	struct align_idle_pattern idle_pattern;
};
static DEFINE_PER_CPU(struct align_cpu_data, align_devices);
static DEFINE_PER_CPU(struct hrtimer, idle_hrtimer);

static bool is_base_cpu(int cpu)
{
	if (cpumask_test_cpu(cpu, &g_fcm_data->basecpumask))
		return true;
	return false;
}

static void operate_shallowidle_mask(int cpu, bool value)
{
	spin_lock(&(g_fcm_data->lock));
	if (value)
		cpumask_set_cpu(cpu, &(g_fcm_data->shallow_idle_cpumask));
	else
		cpumask_clear_cpu(cpu, &(g_fcm_data->shallow_idle_cpumask));
	spin_unlock(&(g_fcm_data->lock));
}

static u64 idle_ktime_to_ns(ktime_t timer)
{
	return (u64)ktime_to_ns(timer);
}

static bool irq_is_timer(struct align_cpu_data *data)
{
	if (data->wakeup_hwirq == g_fcm_data->spi_timer ||
	    data->wakeup_hwirq == g_fcm_data->ppi_timer)
		return true;

	return false;
}

static void set_timer_irqnum(struct align_cpu_data *data)
{
	struct hrtimer *hrtmr;

	if (is_base_cpu(data->cpu))
		return;

	if (likely(g_fcm_data->ppi_timer != UNINIT &&
	    g_fcm_data->spi_timer != UNINIT))
		return;

	if (data->wakeup_timer_ns != 0)
		return;

	/* ppi or spi timer irq num has not been inited. */
	if ((g_fcm_data->ppi_timer == UNINIT && data->last_state_idx == 0) ||
	    (g_fcm_data->spi_timer == UNINIT && data->last_state_idx > 0)) {
		hrtmr = &per_cpu(idle_hrtimer, data->cpu);
		hrtimer_start(hrtmr, ns_to_ktime(NSEC_PER_MSEC), HRTIMER_MODE_REL_PINNED);
	}
}

static void read_timer_irqnum(struct align_cpu_data *data)
{
	if (likely(g_fcm_data->ppi_timer != UNINIT &&
	    g_fcm_data->spi_timer != UNINIT))
		return;

	if (data->wakeup_hwirq < IPI_MAX_NUM)
		return;

	spin_lock(&(g_fcm_data->lock));
	if (data->wakeup_hwirq < PPI_MAX_NUM)
		g_fcm_data->ppi_timer = data->wakeup_hwirq;
	else
		g_fcm_data->spi_timer = data->wakeup_hwirq;
	spin_unlock(&(g_fcm_data->lock));
}

/*
 * hwirq_update_fcmidle_residency - update fcmidle residency when continous hwirq is coming
 * @data: the struct store idle information.
 *
 * Note:
 * Within system, fcm idle.is mostly waken up by timer, which can be predicted very precisely.
 * However, in some situation, playing vedio or camera working for example,
 * fcmidle could be waken up repeatedly by hwirq, which could not be predicted.
 * And therefore, energy wasting happens.
 * So in this scene, we set fcmidle residency much longer to prevent bad prediction.
 * And when hwirq is missing(no hwirq waking fcmidle between two frame intervals),
 * the residency statics restores.
 */
static void hwirq_update_fcmidle_residency(struct align_cpu_data *data)
{
	int i;
	u64 interval;
	ktime_t curr_time;

	/* Soft irq is not in consider. */
	if (data->wakeup_hwirq < IPI_MAX_NUM)
		return;

	/* Timer wakeup, consider recover residency. */
	if (likely(irq_is_timer(data))) {
		if (g_fcm_data->target_residency_ns < FRAME_INTERVAL_NS)
			return;
		curr_time = ktime_get();
		interval = idle_ktime_to_ns(curr_time -
		    g_fcm_data->hwirq_wakeup_time[HIST_HWIRQ_INTER_CNT - 1]);
		if (interval < FRAME_INTERVAL_NS * 2)
			return;
		spin_lock(&(g_fcm_data->lock));
		g_fcm_data->target_residency_ns = g_fcm_data->base_residency_ns;
		spin_unlock(&(g_fcm_data->lock));
		return;
	}

	/* hwirq wakeup. choose residency changing. */
	curr_time = ktime_get();
	spin_lock(&(g_fcm_data->lock));
	for (i = 0; i < HIST_HWIRQ_INTER_CNT - 1; i++)
		g_fcm_data->hwirq_wakeup_time[i] = g_fcm_data->hwirq_wakeup_time[i + 1];
	g_fcm_data->hwirq_wakeup_time[i] = curr_time;

	for (i = HIST_HWIRQ_INTER_CNT - 1; i > 0; i--) {
		if (idle_ktime_to_ns(g_fcm_data->hwirq_wakeup_time[i] -
		    g_fcm_data->hwirq_wakeup_time[i - 1]) > FRAME_INTERVAL_NS) {
			spin_unlock(&(g_fcm_data->lock));
			return;
		}
	}

	g_fcm_data->target_residency_ns = g_fcm_data->base_residency_ns + FRAME_INTERVAL_NS;
	spin_unlock(&(g_fcm_data->lock));
}

static void idle_hrtimer_cancel(void)
{
	unsigned int cpu = smp_processor_id();
	struct hrtimer *hrtmr = &per_cpu(idle_hrtimer, cpu);
	struct align_cpu_data *data = &per_cpu(align_devices, cpu);

	if (data->wakeup_timer_ns == 0)
		return;

	/* wakeup timer is timeout. */
	if (ns_to_ktime(data->wakeup_timer_ns) < ktime_get())
		return;

	hrtimer_cancel(hrtmr);
	data->wakeup_timer_ns = 0;
}

static enum hrtimer_restart idle_hrtimer_notify(struct hrtimer *phrtimer)
{
	unsigned int cpu = smp_processor_id();
	struct align_cpu_data *data = &per_cpu(align_devices, cpu);

	if (!phrtimer)
		return HRTIMER_NORESTART;

	data->wakeup_ipi = true;
	data->wakeup_timer_ns = 0;
	read_timer_irqnum(data);

	return HRTIMER_NORESTART;
}

static void cpuidle_hrtimer_init(struct cpuidle_device *dev)
{
	struct hrtimer *t = &per_cpu(idle_hrtimer, dev->cpu);

	hrtimer_init(t, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	t->function = idle_hrtimer_notify;
}

void idle_irq_info_update(unsigned int hwirq)
{
	struct align_cpu_data *data = this_cpu_ptr(&align_devices);

	/* hwirq has been inited, return. */
	if (data->wakeup_hwirq != UNINIT)
		return;
	operate_shallowidle_mask(data->cpu, false);
	data->wakeup_hwirq = hwirq;
	data->irq_wakeup_time = ktime_get();
	idle_hrtimer_cancel();
	hwirq_update_fcmidle_residency(data);
}

void set_deepest_idle_flag(void)
{
	int cpu = smp_processor_id();
	struct align_cpu_data *data = &per_cpu(align_devices, cpu);

	data->enter_deepest_idle = true;
	data->wakeup_ipi = true;
}

static int interval2idx(u64 last_residency, struct cpuidle_driver *drv)
{
	int i;
	struct cpuidle_state *target;

	for (i = 1; i < drv->state_count; i++) {
		target = &drv->states[i];
		if (last_residency <= target->target_residency_ns)
			return i - 1;
	}
	return drv->state_count - 1;
}

static bool direct_enter_condition(struct align_cpu_data *data, int index)
{
	if (is_base_cpu(data->cpu))
		return true;

	if (index <= 0)
		return true;

	if (data->wakeup_ipi == true &&
	    index <= data->last_state_idx + 1)
		return true;

	if (irq_is_timer(data) &&
	    data->last_residency_ns > data->longidle_threshold_ns)
		return true;

	return false;
}

/*
 * train_idle_pattern - use many flags to indicate current load.
 * @data: the struct store idle information.
 *
 * Note: in Dou, using fastcpu only for high loading, as Gaming/Video or swiping.
 * In gaming: Arena of Valo as an example, most of idle is very short,
 * in this occation, just selecting wfi is enough. We set busy flag this time.
 * In video, such as tiktok/wechat video.
 * This situation, fastcpu's load is pretty light, but maybe accept continous work
 * in some frame. So, most of the time, we tend to select deepidle, but should be
 * sensitive to continuous short idle. We set continous shortidle flag this time.
 * Record short idle is continous or not.
 * Other situation, use default idle selection.
 */
static void train_idle_pattern(struct align_cpu_data *data)
{
	int long_idle = 0;
	int continuous_short = 0;
	int uncontinuous_short = 0;
	int i, lastidx, nextidx;
	struct align_idle_pattern *pattern = &data->idle_pattern;

	for (i = 0; i < HIST_INTERVAL_CNT; i++)
		if (data->history_resi_ns[i] >= data->longidle_threshold_ns)
			long_idle++;

	pattern->heavy_load = long_idle < (HIST_INTERVAL_CNT / IDLE_PATTERN_THREHOLD);

	/* record short idle continous */
	for (i = 0; i < HIST_INTERVAL_CNT; i++) {
		lastidx = (i - 1 + HIST_INTERVAL_CNT) % HIST_INTERVAL_CNT;
		nextidx = (i + 1) % HIST_INTERVAL_CNT;
		if (data->history_resi_ns[i] >= data->longidle_threshold_ns)
			continue;
		if (data->history_resi_ns[lastidx] < data->longidle_threshold_ns ||
		    data->history_resi_ns[nextidx] < data->longidle_threshold_ns)
			continuous_short++;
		else
			uncontinuous_short++;
	}
	pattern->shortidle_continuous =
	    (continuous_short > IDLE_PATTERN_THREHOLD * uncontinuous_short);
}

/*
 * history_residency_vote_result - use history predict this idle index is acceptable or not
 * @data: the struct store idle information.
 * @drv: cpuidle driver
 * @index: which index we are choosing enter or not.
 *
 * Note:
 * For fastcpu, history sleep length is the most important information to predict
 * current idle index, so we must handle them as cautious as possible.
 * So in this function, we transfer history data into recent idx and recent idx sum.
 * For input index, if index transfered by history sleep length is larger, we record it 1
 * in recent idx or idx sum, else record 0. And then, we use these tow recent data indicate
 * idle condition nearby.
 */
static bool history_residency_vote_result(struct align_cpu_data *data,
	struct cpuidle_driver *drv, int index)
{
	int i, recent_idx;
	int recent_idx_sum = 0;
	struct align_idle_pattern *pattern;

	data->history_vote++;
	if (direct_enter_condition(data, index))
		return true;

	recent_idx = (data->latest_idx[LATEST_IDX_CNT - 1] >= index ? 1 : 0);
	for (i = 0; i < LATEST_IDX_CNT; i++)
		if (data->latest_idx[i] >= index)
			recent_idx_sum += 1;

	train_idle_pattern(data);
	pattern = &data->idle_pattern;

	/* heavy load and recent is still busy. */
	if (pattern->heavy_load && recent_idx_sum < 2)
		return recent_idx_sum >= 2;

	if (pattern->shortidle_continuous)
		return (recent_idx != 0 && recent_idx_sum > 1);

	return (recent_idx != 0 || recent_idx_sum > 1);
}

static void update_next_timer(struct align_cpu_data *data)
{
	int i;
	ktime_t now = ktime_get();
	ktime_t delta_next;
	ktime_t timer_event;

	/* update single core next timer event in data and next_timer_percpu */
	timer_event = tick_nohz_get_sleep_length(&delta_next);
	data->next_timer_ns = idle_ktime_to_ns(timer_event);
	timer_event = tick_nohz_get_next_timer(timer_event);
	g_fcm_data->next_timer_percpu[data->cpu] = timer_event;

	/* update next timer including all present cpus. */
	for_each_present_cpu(i) {
		/* cpu's timer has expired: go on. */
		if (g_fcm_data->next_timer_percpu[i] < now)
			continue;
		if (g_fcm_data->next_timer_percpu[i] < timer_event)
			timer_event = g_fcm_data->next_timer_percpu[i];
	}

	if (g_fcm_data->next_timer_global != timer_event) {
		spin_lock(&(g_fcm_data->lock));
		g_fcm_data->next_timer_global = timer_event;
		spin_unlock(&(g_fcm_data->lock));
	}
}

static void send_align_idle_ipi(struct align_cpu_data *data, bool need_ipi_wakeup)
{
	/* if nr > 0 or wakenup by irq/ipi(wakeup-ipi is not included), schedidle should be clear. */
	if (get_total_nr_running() > 0 || !data->wakeup_ipi) {
		if (g_fcm_data->schedidle) {
			spin_lock(&(g_fcm_data->lock));
			g_fcm_data->schedidle = false;
			spin_unlock(&(g_fcm_data->lock));
		}
	}

	if (get_total_nr_running() == 0) {
		if (!need_ipi_wakeup)
			return;
		/*
		 * when schedule idle flag is set:
		 * that's to say system has sent align idle signal to all cpus,
		 * duplicate ipi should be avoided.
		 */
		if (g_fcm_data->schedidle)
			return;
		spin_lock(&(g_fcm_data->lock));
		g_fcm_data->schedidle = true;
		if (!cpumask_empty(&(g_fcm_data->shallow_idle_cpumask)))
			arch_send_wakeup_ipi_mask(&(g_fcm_data->shallow_idle_cpumask));
		spin_unlock(&(g_fcm_data->lock));
	}
}

static void set_percpu_wakeup_timer(struct align_cpu_data *data)
{
	struct hrtimer *hrtmr = &per_cpu(idle_hrtimer, data->cpu);

	if (data->wakeup_timer_ns == 0)
		return;

	/* Not wfi or next timer is close to wakeup timer. */
	if (data->last_state_idx > 0 ||
	    data->next_timer_ns < data->wakeup_timer_ns + data->longidle_threshold_ns) {
		data->wakeup_timer_ns = 0;
		return;
	}

	hrtimer_start(hrtmr, ns_to_ktime(data->wakeup_timer_ns), HRTIMER_MODE_REL_PINNED);
	data->wakeup_timer_ns += idle_ktime_to_ns(ktime_get());
}

static bool enter_deeper_idle(struct align_cpu_data *data,
	struct cpuidle_state *s, s64 latency_req)
{
	/* receive wakeup-ipi: percpu timer don't need consider. */
	if (data->enter_deepest_idle)
		return true;
	if (s->target_residency_ns > data->next_timer_ns)
		return false;
	/* many threads are running */
	if (data->enter_wfi)
		return false;
	return true;
}

static bool can_stop_tick(struct align_cpu_data *data, struct cpuidle_driver *drv)
{
	int target_idx = data->last_state_idx;
	struct cpuidle_state *s = &drv->states[target_idx + 1];

	/*
	 * Many conditions, wakeup timer is not in need.
	 * Here we list them and return TRUE to vote stopping sched tick:
	 * I: Base Cpus, they rely on wakeup ipi;
	 * II: Enter deepest idle;
	 * III: idle selection forbidden by other condition
	 *      eg: latency request/node disablility/timer nearby.
	 * IV: wakeup timer has been set.
	 */
	if (is_base_cpu(data->cpu) ||
	    target_idx >= drv->state_count - 1 ||
	    data->history_vote <= target_idx ||
	    data->wakeup_timer_ns != 0)
		return true;

	/* Do not rely on sched tick, instead set timer to wake up forbidding shallow idle too long. */
	data->wakeup_timer_ns = s->target_residency_ns * 2;
	return true;
}

/*
 * align_update - before select this index, update last idle info
 * @drv: cpuidle driver
 * @dev: cpuidle device
 * @data: the struct store idle information.
 */
static void align_update(struct cpuidle_driver *drv, struct cpuidle_device *dev,
	struct align_cpu_data *data)
{
	int j;

	/* get nrrunning info to decide whether enter deepest idle or not. */
	if (get_total_nr_running() == 0) {
		data->enter_deepest_idle = true;
	} else if (is_base_cpu(data->cpu)) {
		data->enter_wfi = true;
		data->enter_deepest_idle = false;
	}

	/*
	 * corner condition:
	 * 1: after idle selection is done, idle flow is broken by somethread.
	 * shallow idle cpumask occurs error and therefore, add following
	 * judgement to fix it.
	 * 2: in this condition, timer may be not canceled, cancel it.
	 */
	if (cpumask_test_cpu(data->cpu, &(g_fcm_data->shallow_idle_cpumask)))
		operate_shallowidle_mask(data->cpu, false);
	idle_hrtimer_cancel();
	data->wakeup_timer_ns = 0;

	if (is_base_cpu(data->cpu))
		return;

	data->last_residency_ns += dev->last_residency_ns;
	/*
	 * Timer or wakeup ipi doesn't update history sleep length.
	 * Because these could be predicted, don't need to analyze.
	 */
	if (data->wakeup_ipi || irq_is_timer(data))
		return;

	/* update history residency data. */
	data->history_resi_ns[data->history_event_idx] = data->last_residency_ns;
	data->history_event_idx = (data->history_event_idx + 1) % HIST_INTERVAL_CNT;

	/* update latest residency to idx infomation. */
	for (j = 0; j < LATEST_IDX_CNT - 1; j++)
		data->latest_idx[j] = data->latest_idx[j + 1];
	data->latest_idx[j] = interval2idx(data->last_residency_ns, drv);
	data->last_residency_ns = 0;
}

/*
 * align_select - selects the next idle state to enter
 * @drv: cpuidle driver containing state data
 * @dev: the CPU
 * @stop_tick: indication on whether or not to stop the tick
 */
static int align_select(struct cpuidle_driver *drv, struct cpuidle_device *dev,
		       bool *stop_tick)
{
	struct align_cpu_data *data = this_cpu_ptr(&align_devices);
	s64 latency_req = cpuidle_governor_latency_req(dev->cpu);
	int i, idx;
	bool history_info_vote;
	bool need_ipi_wakeup = false;
	ktime_t func_start_time = ktime_get();

	align_update(drv, dev, data);
	update_next_timer(data);

	if (cpu_should_thread_off(data->cpu)) {
		data->last_state_idx = drv->state_count - 1;
		*stop_tick = true;
		data->enter_deepest_idle = false;
		data->wakeup_ipi = false;
		data->wakeup_hwirq = UNINIT;
		return data->last_state_idx;
	}

	/* Find deepest idle state satisfying our constraint. */
	idx = -1;
	data->history_vote = 0;
	for (i = 0; i < drv->state_count; i++) {
		struct cpuidle_state *s = &drv->states[i];
		if (dev->states_usage[i].disable)
			continue;
		if (idx == -1)
			idx = i;
		if (!enter_deeper_idle(data, s, latency_req))
			break;
		data->longidle_threshold_ns = s->target_residency_ns;
		history_info_vote = history_residency_vote_result(data, drv, i);
		if (s->exit_latency_ns > (u64)latency_req)
			break;
		if (data->enter_deepest_idle) {
			/* there is a timer very near in system, wakeup-ipi shouldn't be set. */
			if (g_fcm_data->target_residency_ns >
			    idle_ktime_to_ns(g_fcm_data->next_timer_global - func_start_time)) {
				if (is_base_cpu(data->cpu) || !history_info_vote)
					break;
				idx = i;
				continue;
			}
			idx = i;
			/* only clusteridle need ipi wakeup. */
			need_ipi_wakeup = (idx == drv->state_count - 1);
			continue;
		}
		if (!history_info_vote)
			break;
		idx = i;
	}

	/* No states enabled. Must use 0. */
	if (idx == -1)
		idx = 0;

	data->last_state_idx = idx;
	*stop_tick = can_stop_tick(data, drv);
	set_percpu_wakeup_timer(data);
	set_timer_irqnum(data);

	send_align_idle_ipi(data, need_ipi_wakeup);
	data->enter_deepest_idle = false;
	data->wakeup_ipi = false;
	data->wakeup_hwirq = UNINIT;

	if (data->last_state_idx < drv->state_count - 1)
		operate_shallowidle_mask(data->cpu, true);

	return data->last_state_idx;
}

/**
 * align_enable_device - scans a CPU's states and does setup
 * @drv: cpuidle driver
 * @dev: the CPU
 */
static int align_enable_device(struct cpuidle_driver *drv,
				struct cpuidle_device *dev)
{
	struct align_cpu_data *data = &per_cpu(align_devices, dev->cpu);

	memset_s(data, sizeof(data), 0, sizeof(struct align_cpu_data));
	data->cpu = (int)dev->cpu;
	cpuidle_hrtimer_init(dev);
	return EOK;
}

/**
 * align_reflect - records that data structures need update
 * @dev: the CPU
 * @index: the index of actual entered state
 *
 * NOTE: it's important to be fast here because this operation will add to
 *       the overall exit latency.
 */
static void align_reflect(struct cpuidle_device *dev, int index)
{
	struct align_cpu_data *data = this_cpu_ptr(&align_devices);
	data->last_state_idx = index;
}

static struct cpuidle_governor align_governor = {
	.name =		"align",
	.rating =	25,
	.enable =	align_enable_device,
	.select =	align_select,
	.reflect =	align_reflect,
};

static int fcmidle_dtsinfo_init(void)
{
	int ret, cpu;
	unsigned int temp;
	struct device_node *np;

	np = of_find_compatible_node(NULL, NULL, "fcm-idle");
	if (!np) {
		pr_err("fcm-idle node not found.\n");
		return -ENODEV;
	}

	ret = of_property_read_u32(np, "residency-us", &temp);
	if (ret != 0) {
		pr_err("residency-us node not found.\n");
		return -ENODEV;
	}
	g_fcm_data->base_residency_ns = temp * NSEC_PER_USEC;
	g_fcm_data->target_residency_ns = temp * NSEC_PER_USEC;

	g_fcm_data->ppi_timer = UNINIT;
	g_fcm_data->spi_timer = UNINIT;

	for_each_online_cpu(cpu)
		if (test_slow_cpu(cpu))
			cpumask_set_cpu(cpu, &(g_fcm_data->basecpumask));

	return EOK;
}

static int fcmdata_init(void)
{
	g_fcm_data = (struct align_fcmidle_data *)kzalloc(sizeof(struct align_fcmidle_data), GFP_KERNEL);
	if (g_fcm_data == NULL) {
		pr_err("fcm data init fail.\n");
		return -ENOMEM;
	}

	if (fcmidle_dtsinfo_init() != EOK) {
		kfree(g_fcm_data);
		g_fcm_data = NULL;
		return -EINVAL;
	}

	spin_lock_init(&(g_fcm_data->lock));
	return EOK;
}

static int __init init_align(void)
{
	if (fcmdata_init() != EOK)
		return -ENODEV;
	return cpuidle_register_governor(&align_governor);
}

postcore_initcall(init_align);
