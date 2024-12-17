/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2023. All rights reserved.
 * Description: hisi_eis_core_freq.c
 *
 * eis core freq driver header
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

#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/reboot.h>
#include <linux/slab.h>
#include <linux/sort.h>
#include <linux/time.h>
#include <linux/workqueue.h>
#include <linux/power_supply.h>
#include <stdbool.h>
#include <securec.h>
#include <linux/platform_drivers/usb/chip_usb.h>
#include <platform_include/basicplatform/linux/power/platform/coul/coul_drv.h>
#include <platform_include/basicplatform/linux/power/platform/coul/coul_event.h>
#include <platform_include/basicplatform/linux/power/platform/bci_battery.h>
#include "hisi_eis_core_sysfs.h"
#include "hisi_eis_data_archive.h"
#ifdef CONFIG_BATT_SOH
#include "../soh/hisi_soh_core.h"
#endif
#include "hisi_eis_core_freq.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
typedef struct timespec64 timespec_64;
static void get_real_time_of_day(timespec_64 *tv)
{
	ktime_get_real_ts64(tv);
}
#else
typedef struct timespec timespec_64;
static void get_real_time_of_day(timespec_64 *tv)
{
	do_gettimeofday(tv);
}
#endif

static inline u64 sum_of_squares(int a, int b)
{
	return (u64)((s64)a * (s64)a + (s64)b * (s64)b);
}

/* n: lower bit, m: higher bit, n, and m itself is included */
#define eis_reg_getbits(x, n, m) (((x) & ~(~0U << ((m) - (n) + 1)) << (n)) >> (n))

static unsigned int g_samp_start_prd;
/* global variable for adb shell read */
static atomic_t g_is_screen_on = ATOMIC_INIT(SCREEN_IS_ON);
static atomic_t g_usb_state_flag = ATOMIC_INIT(USB_IS_OUT);

static struct hisi_eis_device *g_eis_di[BATT_INDEX_END];
static struct eis_freq_device_ops *g_eis_freq_core_ops[BATT_INDEX_END];
static struct eis_device_ops *g_eis_core_ops[BATT_INDEX_END];

/* unit:mS, do not change the array order */
static unsigned int t_freq[] = {
	1, 2, 4, 5, 10, 16, 20, 50,
	100, 200, 500, 1000, 2000, 5000, 10000, 20000
};

struct hisi_eis_device *get_g_eis_di(unsigned int batt)
{
	if (batt >= BATT_INDEX_END)
		return NULL;
	return g_eis_di[batt];
}

/* flag 1: work */
int get_freq_work_flag(void)
{
	int i;
	int ret = 0;

	for (i = 0; i < BATT_INDEX_END; i++) {
		if (g_eis_di[i] != NULL) {
			ret = g_eis_di[i]->eis_freq_dev.freq_ops->read_work_flag();
			if (ret > 0)
				break;
		}
	}
	hisi_eis_inf("work flag = %d\n", ret);
	return ret;
}

static void set_freq_work_flag(struct hisi_eis_device *di, int flag)
{
#ifdef CONFIG_BATT_SOH
	if (get_soh_core_device() == NULL) {
		di->eis_freq_dev.freq_ops->set_work_flag(flag);
		hisi_eis_inf("soh core device is null, flag = %d!\n", flag);
		return;
	}
	mutex_lock(&get_soh_core_device()->soh_mutex);
	di->eis_freq_dev.freq_ops->set_work_flag(flag);
	hisi_eis_inf("eis_freq is set to %d!\n", flag);
	mutex_unlock(&get_soh_core_device()->soh_mutex);
#else
	di->eis_freq_dev.freq_ops->set_work_flag(flag);
#endif
}

static void eis_wake_lock(struct hisi_eis_device *di)
{
	if (di == NULL)
		return;
	if (di->eis_wake_lock->active != 1) {
		__pm_stay_awake(di->eis_wake_lock);
		hisi_eis_inf("eis core wake lock!\n");
	}
}

static void eis_wake_unlock(struct hisi_eis_device *di)
{
	if (di == NULL)
		return;
	if (di->eis_wake_lock->active == 1) {
		__pm_relax(di->eis_wake_lock);
		hisi_eis_inf("eis core wake unlock!\n");
	}
}

static void set_g_is_screen_on(int val)
{
	atomic_set(&g_is_screen_on, val);
}

static void set_g_usb_state_flag(int val)
{
	atomic_set(&g_usb_state_flag, val);
}

static int get_g_is_screen_on(void)
{
	return atomic_read(&g_is_screen_on);
}

static int get_g_usb_state_flag(void)
{
	return atomic_read(&g_usb_state_flag);
}

static const char *get_battery_name(unsigned int batt_index)
{
	const char *batt_name;

	switch (batt_index) {
	case BATT_INDEX_0:
		batt_name = BATT_0_NAME;
		break;
	case BATT_INDEX_1:
		batt_name = BATT_1_NAME;
		break;
	default:
		batt_name = BATT_0_NAME;
	}
	return batt_name;
}

int eis_core_drv_ops_register(
	void *ops, enum eis_drv_ops_type ops_type, unsigned int batt_index)
{
	if (ops == NULL) {
		hisi_eis_err("ops is null!\n");
		return _ERROR;
	}
	if (batt_index >= BATT_INDEX_END) {
		hisi_eis_err("batt_index=%d is null!\n", batt_index);
		return _ERROR;
	}

	switch (ops_type) {
	case FREQ_DRV_OPS:
		g_eis_freq_core_ops[batt_index] = ops;
		break;
	case EIS_OPS:
		g_eis_core_ops[batt_index] = ops;
		break;
	default:
		break;
	}
	hisi_eis_inf("batt %d ops register success!\n", batt_index);
	return _SUCC;
}

static int eis_drv_freq_ops_check(struct hisi_eis_device *di)
{
	if (di->eis_freq_dev.freq_support == 0) {
		hisi_eis_err("not support eis freq!\n");
		return _ERROR;
	}

	di->eis_freq_dev.freq_ops = g_eis_freq_core_ops[di->batt_index];

	if (di->eis_freq_dev.freq_ops == NULL) {
		hisi_eis_err("g_eis_freq_core_ops is null!\n");
		return _ERROR;
	}

	check_eis_freq_ops(di, locate_in_ary);
	check_eis_freq_ops(di, get_v_bat_gain);
	check_eis_freq_ops(di, chk_adjust_freq_i_gain);
	check_eis_freq_ops(di, chk_adjust_freq_v_gain);
	check_eis_freq_ops(di, set_cur_thres);
	check_eis_freq_ops(di, eis_sampling_init);
	check_eis_freq_ops(di, set_v_gain);
	check_eis_freq_ops(di, set_i_gain);
	check_eis_freq_ops(di, set_work_flag);
	check_eis_freq_ops(di, read_work_flag);
	check_eis_freq_ops(di, get_raw_eis_ri);
	if (di->eacr_support)
		check_eis_freq_ops(di, get_raw_eacr_real);

	return _SUCC;
}

static int eis_core_ops_check(struct hisi_eis_device *di)
{
	if (g_eis_core_ops[di->batt_index] == NULL) {
		hisi_eis_err("core ops num %d not registered!\n",
			di->batt_index);
		return _ERROR;
	}
	di->eis_ops = g_eis_core_ops[di->batt_index];

	check_eis_ops(di, get_fifo_period_now);
	check_eis_ops(di, get_fifo_order_now);
	check_eis_ops(di, get_v_fifo_data);
	check_eis_ops(di, get_i_fifo_data);
	check_eis_ops(di, read_and_clear_irq);
	check_eis_ops(di, irq_seq_clear);
	check_eis_ops(di, irq_seq_read);
	check_eis_ops(di, eis_detect_flag);
	check_eis_ops(di, set_t_n_m);
	check_eis_ops(di, get_t_n_m);
	check_eis_ops(di, eis_enable);
	check_eis_ops(di, cnt_overflow_num);
	check_eis_ops(di, cur_offset);

	return _SUCC;
}

static int eis_drv_ops_check(
	struct hisi_eis_device *di, enum eis_drv_ops_type type)
{
	int ret = _ERROR;

	if (di == NULL) {
		hisi_eis_err("di is null!\n");
		return ret;
	}
	switch (type) {
	case FREQ_DRV_OPS:
		ret = eis_drv_freq_ops_check(di);
		break;
	case EIS_OPS:
		ret = eis_core_ops_check(di);
		break;
	default:
		hisi_eis_err("ops register failed!\n");
		break;
	}
	return ret;
}

static int parse_eis_freq_dts(struct hisi_eis_device *di)
{
	struct device_node *np = NULL;

	if (di == NULL) {
		hisi_eis_err("di is null\n");
		return _ERROR;
	}

	np = di->dev->of_node;
	if (np == NULL) {
		hisi_eis_err("np is null\n");
		return _ERROR;
	}

	if (of_property_read_u32(np, "freq_support",
		(u32 *)&di->eis_freq_dev.freq_support)) {
		/* 0: not support */
		di->eis_freq_dev.freq_support = 0;
		hisi_eis_err("get freq support fail\n");
	}
	hisi_eis_inf("eis freq support = %d\n",
			 di->eis_freq_dev.freq_support);

	if (of_property_read_u32(np, "avg_freq_check",
		(u32 *)&di->eis_freq_dev.avg_freq_check)) {
		/* 0: not support */
		di->eis_freq_dev.avg_freq_check = 0;
		hisi_eis_err("get freq support fail\n");
	}
	hisi_eis_inf("eis avg_freq_check = %d\n",
			 di->eis_freq_dev.avg_freq_check);

	if (of_property_read_u32(np, "batt_index", (u32 *)&di->batt_index))
		di->batt_index = 0;

	if (di->batt_index >= BATT_INDEX_END) {
		hisi_eis_err("batt_index=%d, err!", di->batt_index);
		return _ERROR;
	}
	hisi_eis_inf("eis batt_index = %d\n", di->batt_index);

	if (of_property_read_u32(np, "sample_mod", (u32 *)&di->sample_mod))
		di->sample_mod = FIFO_MOD;
	hisi_eis_inf("sample_mod = %d\n", di->sample_mod);

	if (of_property_read_u32(np, "eacr_support", (u32 *)&di->eacr_support)) {
		di->eacr_support = INVALID; /* 0: not support */
		hisi_eis_inf("eacr_support invalid\n");
	}

	return _SUCC;
}

static int hisi_eis_is_screen_on_callback(
	struct notifier_block *nb, unsigned long event, void *data)
{
	int *blank = NULL;
	struct fb_event *fb_event = data;

	if (fb_event == NULL || fb_event->data == NULL)
		return NOTIFY_DONE;

	blank = fb_event->data;
	switch (event) {
	case FB_EVENT_BLANK:
		if (*blank == FB_BLANK_UNBLANK) {
			hisi_eis_inf("screen is on\n");
			set_g_is_screen_on(SCREEN_IS_ON);
		} else if (*blank == FB_BLANK_POWERDOWN) {
			hisi_eis_inf("screen is off\n");
			set_g_is_screen_on(SCREEN_IS_OFF);
		}
		break;
	default:
		hisi_eis_inf("event = 0x%lx!\n", event);
		return NOTIFY_DONE;
	}
	return NOTIFY_OK;
}

static int hisi_eis_usb_plugout_callback(
	struct notifier_block *nb, unsigned long event, void *data)
{
	hisi_eis_inf(" event:%d\n", event);
	switch (event) {
	case CHARGER_TYPE_NONE:
		set_g_usb_state_flag(USB_IS_OUT);
		break;
	default:
		set_g_usb_state_flag(USB_IS_PLUG);
	}
	return NOTIFY_OK;
}

static void hisi_eis_register_fb_usb_list(struct hisi_eis_device *di)
{
	int ret;

	if (di == NULL) {
		hisi_eis_err("di is null\n");
		return;
	}
	di->screen_status_nb.notifier_call = hisi_eis_is_screen_on_callback;
	ret = fb_register_client(&di->screen_status_nb);
	if (ret) {
		hisi_eis_err("ret of fb_register = %d, failed!\n", ret);
		return;
	}

	di->usb_plugout_nb.notifier_call = hisi_eis_usb_plugout_callback;
	ret = chip_charger_type_notifier_register(&di->usb_plugout_nb);
	if (ret) {
		hisi_eis_err("ret of chip_charger_type_notifier_register = %d, failed!\n", ret);
		return;
	}
	hisi_eis_inf("succeed!\n");
}

static void hisi_eis_unregister_fb_usb_list(struct hisi_eis_device *di)
{
	int ret;

	if (di == NULL) {
		hisi_eis_err("di is null\n");
		return;
	}
	ret = chip_charger_type_notifier_unregister(&di->usb_plugout_nb);
	hisi_eis_inf("ret of fb_unregister = %d, !\n", ret);
	ret = fb_unregister_client(&di->screen_status_nb);
	hisi_eis_inf("ret of usb_unregister = %d, !\n", ret);
}

static bool hisi_eis_cur_temp_soc_check(struct hisi_eis_device *di)
{
	int bat_temp, i_bat, soc;
	int ret = 0;
	union power_supply_propval val = { 0 };
#ifdef CONFIG_DFX_DEBUG_FS
	int temper_test;
#endif

	if (di->psy) {
		ret += power_supply_get_property(di->psy, POWER_SUPPLY_PROP_TEMP, &val);
		bat_temp = val.intval / TENTH;
		ret += power_supply_get_property(di->psy, POWER_SUPPLY_PROP_CURRENT_NOW, &val);
		i_bat = val.intval;
		ret += power_supply_get_property(di->psy, POWER_SUPPLY_PROP_CAPACITY, &val);
		soc = val.intval;
		if (ret) {
			hisi_eis_err("get psy val err!");
			return false;
		}
	} else {
		bat_temp = coul_drv_battery_temperature();
		i_bat = -coul_drv_battery_current();
		soc = coul_drv_battery_capacity();
	}

	if (abs(i_bat) > EIS_FREQ_ENTER_CUR) {
		hisi_eis_err("i_bat = %d, over |%u| mA\n",
			i_bat, EIS_FREQ_ENTER_CUR);
		return false;
	}
#ifdef CONFIG_DFX_DEBUG_FS
	temper_test = get_test_bat_temp();
	if (temper_test < EIS_FREQ_TEMP_LOW || temper_test > EIS_FREQ_TEMP_HIGH) {
		hisi_eis_err("batt %d temp_test = %d, over [%u,%u]\n",
			di->batt_index, temper_test, EIS_FREQ_TEMP_LOW, EIS_FREQ_TEMP_HIGH);
		return false;
	}
#endif
	if (bat_temp < EIS_FREQ_TEMP_LOW || bat_temp > EIS_FREQ_TEMP_HIGH) {
		hisi_eis_err("batt %d temp = %d, over [%u,%u]\n",
			di->batt_index, bat_temp, EIS_FREQ_TEMP_LOW, EIS_FREQ_TEMP_HIGH);
		return false;
	}

	if (soc < EIS_FREQ_SOC) {
		hisi_eis_err("batt %d soc = %d, less than %u\n", di->batt_index, soc, EIS_FREQ_SOC);
		return false;
	}

	return true;
}

static bool hisi_eis_is_sampling_allowed(struct hisi_eis_device *di, enum eis_mode mode)
{
	union power_supply_propval val = { 0 };
	int bat_cycle_now;
	int latest_history_cycle;
	int full_freq_rolling, freq_rolling_now;
	struct eis_freq_info freq_his_inf[LEN_T_FREQ] = {};
	int last_rolling;
	bool ret = false;

	if (di->psy) {
		bat_cycle_now = power_supply_get_property(di->psy,
			POWER_SUPPLY_PROP_CYCLE_COUNT, &val);
		if (bat_cycle_now) {
			hisi_eis_err("get psy val err!");
			return ret;
		}
		bat_cycle_now = val.intval;
	} else {
		bat_cycle_now = coul_drv_battery_cycle_count();
	}

	full_freq_rolling = eis_get_g_full_freq_rolling(di->batt_index);
	freq_rolling_now = eis_get_g_freq_rolling_now(di->batt_index);
	if (full_freq_rolling > 0 || freq_rolling_now > 0) {
		last_rolling = freq_rolling_now - 1;
		if (last_rolling < 0)
			last_rolling = EIS_FREQ_ROLLING - 1;
	} else {
		last_rolling = freq_rolling_now;
	}

	switch (mode) {
	case EIS_FREQ:
		if (hisi_eis_cur_temp_soc_check(di) == false)
			return false;
		/* 0: newly run eis freq */
		if (full_freq_rolling == 0 && freq_rolling_now == 0) {
			latest_history_cycle = 0;
			hisi_eis_inf("cycle_now = %d, cycle_history = %d, first run eis freq\n",
				 bat_cycle_now, latest_history_cycle);
			ret = true;
		} else {
			hisi_eis_freq_read_single_info_from_flash(freq_his_inf,
				last_rolling, di->batt_index);
			/* 15: the last record */
			latest_history_cycle = freq_his_inf[LEN_T_FREQ - 1].cycle;
			hisi_eis_inf("latest_history_cycle = %d\n",
				latest_history_cycle);
			if (bat_cycle_now == latest_history_cycle) {
				hisi_eis_err("cycle_now = %d, cycle_history = %d, in the same cycle\n",
					bat_cycle_now, latest_history_cycle);
				ret = false;
			} else {
				hisi_eis_inf("cycle_now = %d, cycle_history = %d, not in the same cycle\n",
					bat_cycle_now, latest_history_cycle);
				ret = true;
			}
		}
		return ret;
	default:
		hisi_eis_err("mode = %u, input invalid!\n", mode);
		return false;
	}
}

static void collect_para_prepare(struct hisi_eis_device *di,
	struct collect_para *para, enum eis_mode mode, int idx)
{
	long long timeout_ms;
	errno_t err;

	switch (mode) {
	case EIS_FREQ:
		para->ptr_v = di->eis_freq_dev.freq_sample_info[idx].v_bat;
		para->ptr_i = di->eis_freq_dev.freq_sample_info[idx].i_bat;
		para->key_prds = -1; /* not use this flag */
		timeout_ms = (long long)t_freq[idx];
		break;
	default:
		hisi_eis_err("invalid mode = %u, sampling stopped\n", mode);
		return;
	}
	para->timeout_ms = timeout_ms;

	if (idx < T_W_COLLECT_FLAG || idx > F_COLLECT_FLAG) {
		err = memset_s(para->ptr_v, SIZEOF_VBAT_SAMPLE, 0,
			SIZEOF_VBAT_SAMPLE);
		if (err != EOK) {
			hisi_eis_err("v_bat memset_s failed\n");
			return;
		}
		err = memset_s(para->ptr_i, SIZEOF_IBAT_SAMPLE, 0,
			SIZEOF_IBAT_SAMPLE);
		if (err != EOK) {
			hisi_eis_err("i_bat memset_s failed\n");
			return;
		}
	}
}

static void hisi_eis_traverse_cur_fifo(struct hisi_eis_device *di, int irq_idx,
	unsigned int (*ptr_i)[I_BAT_FIFO_DEPTH], unsigned int p_now, int shift)
{
	int q, r;

	/*
	 * when i equals to 3, 5; r in the range of [0, 1], [2, 3]
	 * q: intermediate variable
	 * 1: intermediate variable
	 * 2: half of I_BAT_FIFO_DEPTH
	 * r: cur fifo order, r + 1: func param starts from 1
	 */
	q = irq_idx - 1;
	r = q - 2;
	hisi_eis_debug("g_samp_start_prd = %u!\n", g_samp_start_prd);
	while (r < q) {
		ptr_i[p_now - g_samp_start_prd + (unsigned int)shift][r] =
			di->eis_ops->get_i_fifo_data(r + 1);
		r++;
	}
}

static void hisi_eis_traverse_vol_fifo(struct hisi_eis_device *di, int irq_idx,
	unsigned int (*ptr_v)[V_BAT_FIFO_DEPTH], unsigned int p_now, int shift)
{
	int q, r;

	/*
	 * when i equals to 3, 5; r in the range of [0, 7], [8, 15]
	 * q: intermediate variable
	 * 4: intermediate variable
	 * 8: half of V_BAT_FIFO_DEPTH
	 * r: vol fifo order, r + 1: func param starts from 1
	 */
	q = 4 * (irq_idx - 1);
	r = q - 8;
	hisi_eis_debug("g_samp_start_prd = %u!\n", g_samp_start_prd);
	while (r < q) {
		ptr_v[p_now - g_samp_start_prd + (unsigned int)shift][r] =
			di->eis_ops->get_v_fifo_data(r + 1);
		r++;
	}
}

static int clear_all_irq_before_collect_and_enable(
	struct hisi_eis_device *di, int idx, unsigned int n, long long prd_ms, int periods)
{
	int eis_irq;
	timespec_64 tv = {0};
	long long time_b, time_a, time_span;
	int irq_en;
	long long wait_time;

	hisi_eis_inf("idx=%d, n=%d, prd_ms=%d\n", idx, n, prd_ms);
	if (di->sample_mod == FIFO_MOD) {
		irq_en = EIS_IRQ_FIFO_NULL_DEPLETED;
		wait_time = prd_ms * n;
	} else {
		irq_en = EIS_IRQ_NOFIFO_NULL_DEPLETED;
		wait_time = prd_ms * (periods + 3) * 2;
	}

	di->eis_ops->irq_seq_clear(IRQ_ALL);
	di->eis_ops->eis_enable(true);
	eis_irq = di->eis_ops->irq_seq_read(IRQ_ALL);
	/* wait for null periods to be depleted if n isn't zero */
	get_real_time_of_day(&tv);
	time_b = (long long)(tv.tv_sec * S_TO_MS) + tv.tv_nsec / US_TO_NS / MS_TO_US;
	if (n == 0 && di->sample_mod == FIFO_MOD) {
		di->eis_ops->irq_seq_clear(IRQ_FIFO);
		return _SUCC;
	}

	while (eis_irq != irq_en) {
		udelay(1); /* 1: 1uS to avoid busy */
		if (get_g_is_screen_on() == SCREEN_IS_ON ||
			get_g_usb_state_flag() == USB_IS_OUT) {
			hisi_eis_err("screen on or usb out, stop eis\n");
			return _STOP;
		}
		eis_irq = di->eis_ops->irq_seq_read(IRQ_ALL);
		get_real_time_of_day(&tv);
		time_a = (long long)(tv.tv_sec * S_TO_MS) + tv.tv_nsec /
			US_TO_NS / MS_TO_US;
		time_span = time_a - time_b;
		if (time_span > wait_time) {
			hisi_eis_err("time span = %lld, time out = %lld, eis_irq = %d, abort!\n",
				time_span, wait_time, eis_irq);
			return _ERROR;
		}
	}
	/* before collecting data, clear irq4~irq16 irq status */
	di->eis_ops->irq_seq_clear(IRQ_FIFO);
	return _SUCC;
}

static int check_if_stop_in_advance(
	struct hisi_eis_device *di, int idx, int p)
{
	int eis_end_irq;

	eis_end_irq = di->eis_ops->read_and_clear_irq(EIS_FIFO_IRQ_END, _READ);
	/* 1: eis work ended, 4: idx of freq > 10mS, idx < 0, t-w mode */
	if (eis_end_irq == 1 && (idx > F_COLLECT_FLAG || idx < T_W_COLLECT_FLAG)) {
		hisi_eis_err("period_now=%d, EIS_IRQ=:%x, idx = %d end irq caused sample_lock stop\n",
			 p, eis_end_irq, idx);
		/* stopped in advance */
		return _ERROR;
	}
	/* didn't stop in advance */
	return _SUCC;
}

static int wait_until_fifo_is_filled_up(
	struct hisi_eis_device *di, int irq_num, int idx, long long timeout)
{
	unsigned int eis_irq_tmp;
	long long t_b, t_a;
	timespec_64 tv_1 = {0};

	eis_irq_tmp = (unsigned int)di->eis_ops->read_and_clear_irq(irq_num, _READ);
	hisi_eis_debug("eis_irq_tmp = %u!\n", eis_irq_tmp);
	/* idx = 4, freq mode when freq > 10mS, idx < 0, t-w mode */
	if (idx > F_COLLECT_FLAG || idx < T_W_COLLECT_FLAG) {
		hisi_eis_inf("idx = %d!\n", idx);
		get_real_time_of_day(&tv_1);
		t_b = (long long)(tv_1.tv_sec * S_TO_MS) + tv_1.tv_nsec / US_TO_NS / MS_TO_US;
		hisi_eis_debug("idx = %d, t_b = %ld!\n", idx, t_b);
		while (eis_irq_tmp != EIS_FIFO_DATA_FILLED) {
			eis_irq_tmp = (unsigned int)di->eis_ops->read_and_clear_irq(
				irq_num, _READ);
			get_real_time_of_day(&tv_1);
			t_a = (long long)(tv_1.tv_sec * S_TO_MS) + tv_1.tv_nsec / US_TO_NS / MS_TO_US;
			if (t_a - t_b > timeout) {
				hisi_eis_err("time span = %lld, time out = %lld, abort!\n",
					t_a - t_b, timeout);
				/* _ERROR: time out */
				return _ERROR;
			}
		}
	}
	/* _SUCC: not time out */
	return _SUCC;
}

static int if_collect_time_out(
	int idx, long long time_before, long long tm_out)
{
	timespec_64 tv = {0};
	long long time_val_a1, time_span;

	get_real_time_of_day(&tv);
	time_val_a1 = (long long)(tv.tv_sec * S_TO_MS) + tv.tv_nsec / US_TO_NS / MS_TO_US;
	time_span = time_val_a1 - time_before;
	if (time_span > tm_out) {
		hisi_eis_err("idx = %d, time_span = %lld, timeout = %lld, time out!\n",
			idx, time_span, tm_out);
		/* _ERROR: time out */
		return _ERROR;
	}
	/* _SUCC: not time out */
	return _SUCC;
}

/* idx is only valid for freq mode, it is assigned to minus num for t-w mode */
static int hisi_eis_sampling_data_collect(
	enum eis_mode mode, int periods,
	struct hisi_eis_device *di, int idx, int shift)
{
	unsigned int p_now;
	int i;
	int ret = _SUCC;
	unsigned int t_n_m[T_N_M_PARA_LEN] = {0};
	timespec_64 tv_0 = {0};
	long long t_ms[TIMEOUT_PARA_NUM] = {0};
	struct collect_para para = {};

	collect_para_prepare(di, &para, mode, idx);
	t_ms[1] = para.timeout_ms;
	/* 0, 1, 2: the 1st, 2nd, 3rd term of array */
	di->eis_ops->get_t_n_m(&t_n_m[0], &t_n_m[1], &t_n_m[2]);

	/* clear all irq status */
	/* idx > 4 for freq >=10mS in freq mode, idx < 0 for t-w mode */
	if (idx > F_COLLECT_FLAG || idx < T_W_COLLECT_FLAG) {
		ret = clear_all_irq_before_collect_and_enable(di, idx, t_n_m[1], t_ms[1], periods);
		if (ret == _STOP)
			return _STOP;
	}

	p_now = di->eis_ops->get_fifo_period_now();
	g_samp_start_prd = p_now;
	t_ms[2] = t_ms[1] * (periods + 1);
	get_real_time_of_day(&tv_0);
	t_ms[0] = (long long)(tv_0.tv_sec * S_TO_MS) + tv_0.tv_nsec / US_TO_NS /MS_TO_US;
	while (p_now - g_samp_start_prd < (unsigned int)periods) {
		if (get_g_is_screen_on() == SCREEN_IS_ON ||
			get_g_usb_state_flag() == USB_IS_OUT) {
			hisi_eis_err("screen on or usb out, stop eis\n");
			return _STOP;
		}
		ret = check_if_stop_in_advance(di, idx, p_now);
		if (ret != _SUCC)
			return _ERROR;
		for (i = EIS_FIFO_IRQ_8; i <= EIS_FIFO_IRQ_16; i++) {
			if (i == EIS_FIFO_IRQ_12)
				continue;
			ret = wait_until_fifo_is_filled_up(di, i, idx, t_ms[1]);
			if (ret != _SUCC)
				return ret;
			hisi_eis_traverse_cur_fifo(di, i, para.ptr_i,
				p_now, shift);
			hisi_eis_traverse_vol_fifo(di, i, para.ptr_v,
				p_now, shift);
			/* clear irq4, irq8; irq12, irq16 correspondingly */
			di->eis_ops->read_and_clear_irq(i - 1, _CLEAR);
			di->eis_ops->read_and_clear_irq(i, _CLEAR);
		}
		/* update p_now */
		p_now = (idx > 4 || idx < 0) ?
			di->eis_ops->get_fifo_period_now() : (p_now + 1);
		ret = if_collect_time_out(idx, t_ms[0], t_ms[2]);
		if (ret != _SUCC)
			return _ERROR;
	}
	return ret;
}

static int hisi_eis_sampling_data_collect_ram(
	enum eis_mode mode, int periods,
	struct hisi_eis_device *di, int idx, int shift)
{
	int ret, i, r;
	int v_n, i_n;
	int offset;
	unsigned int t_n_m[T_N_M_PARA_LEN] = {0};
	long long t_ms[TIMEOUT_PARA_NUM] = {0};
	struct collect_para para = {};

	hisi_eis_inf("idx=%d, periods=%d\n", idx, periods);

	collect_para_prepare(di, &para, mode, idx);
	t_ms[1] = para.timeout_ms;
	/* 0, 1, 2: the 1st, 2nd, 3rd term of array */
	di->eis_ops->get_t_n_m(&t_n_m[0], &t_n_m[1], &t_n_m[2]);

	/* clear all irq status */
	ret = clear_all_irq_before_collect_and_enable(di, idx, t_n_m[1], t_ms[1], periods);
	if (ret == _STOP)
		return _STOP;

	for (i = 0; i < periods; i++) {
		v_n = 0;
		i_n = 0;
		for (r = 0; r < V_BAT_FIFO_DEPTH + I_BAT_FIFO_DEPTH; r++) {
			offset = i * (V_BAT_FIFO_DEPTH + I_BAT_FIFO_DEPTH) + r;
			if ((r + 1) % V_I_BAT_GROUP_SIZE > 0) {
				para.ptr_v[i][v_n] = di->eis_ops->get_v_fifo_data(offset);
				v_n++;
			} else {
				para.ptr_i[i][i_n] = di->eis_ops->get_i_fifo_data(offset);
				i_n++;
			}
		}
	}
	return _SUCC;
}

/* idx is only valid for freq mode, it is redundant for t-w mode */
static void hisi_eis_sampling_data_print(enum eis_mode mode,
	struct hisi_eis_device *di, int idx)
{
	unsigned int (*ptr_v)[V_BAT_FIFO_DEPTH];
	unsigned int (*ptr_i)[I_BAT_FIFO_DEPTH];
	unsigned int freq;
	int i;
	unsigned int periods;
	unsigned int cur[I_BAT_FIFO_DEPTH], vol[V_BAT_FIFO_DEPTH];

	switch (mode) {
	case EIS_FREQ:
		ptr_v = di->eis_freq_dev.freq_sample_info[idx].v_bat;
		ptr_i = di->eis_freq_dev.freq_sample_info[idx].i_bat;
		freq = t_freq[idx];
		periods = di->eis_freq_dev.freq_sample_info[idx].sample_prds;
		break;
	default:
		return;
	}
	for (i = 0; i < periods; i++) {
		cur[0] = ptr_i[i][0];
		cur[1] = ptr_i[i][1];
		cur[2] = ptr_i[i][2];
		cur[3] = ptr_i[i][3];
		hisi_eis_inf("%s:sample_freq = 1000/%u, %dth cur = %d, %d, %d, %d\n",
			"EIS_FREQ", freq, i, cur[0], cur[1], cur[2], cur[3]);
		vol[0] = ptr_v[i][0];
		vol[1] = ptr_v[i][1];
		vol[2] = ptr_v[i][2];
		vol[3] = ptr_v[i][3];
		vol[4] = ptr_v[i][4];
		vol[5] = ptr_v[i][5];
		vol[6] = ptr_v[i][6];
		vol[7] = ptr_v[i][7];
		vol[8] = ptr_v[i][8];
		vol[9] = ptr_v[i][9];
		vol[10] = ptr_v[i][10];
		vol[11] = ptr_v[i][11];
		vol[12] = ptr_v[i][12];
		vol[13] = ptr_v[i][13];
		vol[14] = ptr_v[i][14];
		vol[15] = ptr_v[i][15];
		hisi_eis_inf("%s:sample_freq = 1000/%u, %dth vol = %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u\n",
			"EIS_FREQ", freq, i, vol[0], vol[1], vol[2], vol[3],
			vol[4], vol[5], vol[6], vol[7], vol[8], vol[9], vol[10],
			vol[11], vol[12], vol[13], vol[14], vol[15]);
	}
}

static int wait_eis_working_stop(
	struct hisi_eis_device *di, timespec_64 tv, long timeout_ms)
{
	int eis_detect_flag;
	timespec_64 tv_tmp = {0};
	long long time_span = 0;
	long long time_b, time_a;

	if (di == NULL) {
		hisi_eis_err("null eis_di!\n");
		return _ERROR;
	}
	time_b = (long long)(tv.tv_sec * S_TO_MS) + tv.tv_nsec / US_TO_NS / MS_TO_US;
	eis_detect_flag = di->eis_ops->eis_detect_flag();
	while (eis_detect_flag == EIS_IS_WORKING) {
		eis_detect_flag = di->eis_ops->eis_detect_flag();
		get_real_time_of_day(&tv_tmp);
		time_a = (long long)(tv_tmp.tv_sec * S_TO_MS) +
			tv_tmp.tv_nsec / US_TO_NS / MS_TO_US;
		time_span = time_a - time_b;
		if (time_span > timeout_ms) {
			hisi_eis_err("time span = %lld, timeout!\n", time_span);
			return _ERROR;
		}
	}
	return _SUCC;
}

static void eis_freq_info_temp_cur_soc_update(
	struct hisi_eis_device *di, int idx)
{
	int bat_temp, i_bat, soc;
	union power_supply_propval val = { 0 };
	int ret = 0;

	if (di->psy) {
		ret += power_supply_get_property(di->psy, POWER_SUPPLY_PROP_TEMP, &val);
		bat_temp = val.intval / TENTH;
		ret += power_supply_get_property(di->psy, POWER_SUPPLY_PROP_CURRENT_NOW, &val);
		i_bat = val.intval;
		ret += power_supply_get_property(di->psy, POWER_SUPPLY_PROP_CAPACITY, &val);
		soc = val.intval;
		if (ret)
			hisi_eis_err("get psy val err!");
	} else {
		bat_temp = coul_drv_battery_temperature();
		i_bat = -coul_drv_battery_current();
		soc = coul_drv_battery_capacity();
	}

	di->eis_freq_dev.freq_info[idx].temp = bat_temp;
	di->eis_freq_dev.freq_info[idx].cur = i_bat;
	di->eis_freq_dev.freq_info[idx].soc = soc;
}

static int sampling_for_freq_less_than_ten_ms(
	struct eis_freq_device *di, int freq, int idx)
{
	struct hisi_eis_device *eis_di;
	int p, eis_irq;
	timespec_64 tv = {0};
	int cur_offset;
	int ret;

	eis_di = container_of(di, struct hisi_eis_device, eis_freq_dev);
	if (eis_di == NULL) {
		hisi_eis_err("null eis_di!\n");
		return _ERROR;
	}
	eis_freq_info_temp_cur_soc_update(eis_di, idx);
	if (eis_di->sample_mod == FIFO_MOD) {
		for (p = 0; p < FREQ_SAMP_PRDS_10; p++) {
			/* clear all irq status */
			eis_di->eis_ops->irq_seq_clear(IRQ_ALL);
			/*
			 * for the first sampling, null period is set to 4,
			 * sampling period is set to 2; for the rest periods,
			 * null period is set to 0, sampling period is set to 2
			 */
			get_real_time_of_day(&tv);

			if (wait_eis_working_stop(eis_di, tv, WAIT_EIS_END) == _ERROR) {
				hisi_eis_err("wait working stop time out!\n");
				return _ERROR;
			}

			if (p == 0)
				/* 4: null periods, 1: sampling periods */
				eis_di->eis_ops->set_t_n_m(freq, 4, 1);
			else
				/* 1: null periods, 1: sampling periods */
				eis_di->eis_ops->set_t_n_m(freq, 1, 1);
			eis_di->eis_ops->eis_enable(true);
			eis_irq = eis_di->eis_ops->irq_seq_read(IRQ_ALL);
			hisi_eis_debug("p = %d, eis_irq = %d!\n", p, eis_irq);
			get_real_time_of_day(&tv);
			if (wait_eis_working_stop(eis_di, tv, WAIT_EIS_END) == _ERROR) {
				hisi_eis_err("wait working stop time out!\n");
				return _ERROR;
			}
			ret = hisi_eis_sampling_data_collect(EIS_FREQ, 1, eis_di, idx, p);
			if (ret == _STOP) {
				eis_di->eis_ops->eis_enable(false);
				return _STOP;
			}

			eis_di->eis_ops->eis_enable(false);
			cur_offset = eis_di->eis_ops->cur_offset();
			hisi_eis_inf("idx = %d, p = %d, cur_offset = %d!\n", idx, p, cur_offset);
		}
	} else {
		eis_di->eis_ops->set_t_n_m(freq, 4, 10);
		ret = hisi_eis_sampling_data_collect_ram(
			EIS_FREQ, FREQ_SAMP_PRDS_10, eis_di, idx, 0);
		if (ret == _STOP) {
			eis_di->eis_ops->eis_enable(false);
			return _STOP;
		}

		cur_offset = eis_di->eis_ops->cur_offset();
		di->freq_sample_info[idx].sample_prds = FREQ_SAMP_PRDS_10;
		eis_di->eis_ops->eis_enable(false);
		hisi_eis_inf("idx = %d, cur_offset = %d!\n", idx, cur_offset);
	}
	return _SUCC;
}

static int sampling_for_freq_less_than_thousand_ms(
	struct eis_freq_device *di, int freq, int idx)
{
	struct hisi_eis_device *eis_di = NULL;
	int cur_offset;
	int ret;

	eis_di = container_of(di, struct hisi_eis_device, eis_freq_dev);
	if (eis_di == NULL) {
		hisi_eis_err("null eis_di!\n");
		return _ERROR;
	}

	eis_freq_info_temp_cur_soc_update(eis_di, idx);
	/* 15: set one more for the depletion of intermediate process */
	eis_di->eis_ops->set_t_n_m(freq, 1, 15); /* F_SAMP_NULL_PRDS */
	/* 0: shift set to zero for cases when freq greater than 10mS */
	if (eis_di->sample_mod == FIFO_MOD)
		ret = hisi_eis_sampling_data_collect(
			EIS_FREQ, FREQ_SAMP_PRDS_10, eis_di, idx, 0);
	else
		ret = hisi_eis_sampling_data_collect_ram(
			EIS_FREQ, FREQ_SAMP_PRDS_10, eis_di, idx, 0);
	if (ret == _STOP) {
		eis_di->eis_ops->eis_enable(false);
		return ret;
	}
	cur_offset = eis_di->eis_ops->cur_offset();
	hisi_eis_inf("idx = %d, cur_offset = %d!\n", idx, cur_offset);
	hisi_eis_debug("freq < 1000!\n");
	di->freq_sample_info[idx].sample_prds = FREQ_SAMP_PRDS_10;
	eis_di->eis_ops->eis_enable(false);
	return ret;
}

static int sampling_for_freq_greater_than_thousand_ms(
	struct eis_freq_device *di, int freq, int idx)
{
	struct hisi_eis_device *eis_di = NULL;
	int cur_offset;
	int ret;

	eis_di = container_of(di, struct hisi_eis_device, eis_freq_dev);
	if (eis_di == NULL) {
		hisi_eis_err("null eis_di!\n");
		return _ERROR;
	}

	eis_freq_info_temp_cur_soc_update(eis_di, idx);
	/* 0: null periods, 3: extra 3 periods for preparing */
	eis_di->eis_ops->set_t_n_m(freq, 0, FREQ_SAMP_PRDS_5 + 3);
	hisi_eis_debug("freq = %d!\n", freq);
	/* 0: shift set to zero for cases when freq greater than 10mS */
	if (eis_di->sample_mod == FIFO_MOD)
		ret = hisi_eis_sampling_data_collect(
			EIS_FREQ, FREQ_SAMP_PRDS_5, eis_di, idx, 0);
	else
		ret = hisi_eis_sampling_data_collect_ram(
			EIS_FREQ, FREQ_SAMP_PRDS_5, eis_di, idx, 0);
	if (ret == _STOP) {
		eis_di->eis_ops->eis_enable(false);
		return ret;
	}

	cur_offset = eis_di->eis_ops->cur_offset();
	hisi_eis_inf("idx = %d, cur_offset = %d!\n", idx, cur_offset);
	hisi_eis_debug("freq < 20000!\n");
	di->freq_sample_info[idx].sample_prds = FREQ_SAMP_PRDS_5;
	eis_di->eis_ops->eis_enable(false);
	return ret;
}

static int hisi_eis_freq_start_sampling_func(
	struct eis_freq_device *di, int idx)
{
	int freq;
	int ret;
	timespec_64 tv = {0};
	struct hisi_eis_device *eis_di = NULL;

	if (di == NULL || idx < 0 || idx > LEN_T_FREQ - 1) {
		hisi_eis_err("input invalid!\n");
		return _ERROR;
	}
	/* 0: just a intermediate variable */
	freq = (int)t_freq[idx];
	hisi_eis_inf("freq = %d, idx = %d!\n", freq, idx);
	di->freq_info[idx].freq = freq;
	eis_di = container_of(di, struct hisi_eis_device, eis_freq_dev);
	di->freq_sample_info[idx].sample_freq = (unsigned int)freq;
	eis_di->eis_freq_dev.freq_ops->eis_sampling_init(eis_di->batt_index);
	/* if eis is working, wait until it's not working */
	get_real_time_of_day(&tv);
	if (wait_eis_working_stop(eis_di, tv, WAIT_EIS_END) == _ERROR) {
		hisi_eis_err("wait working stop time out!\n");
		return _ERROR;
	}

	/* 10: freq eauals 10mS */
	if (freq <= 10) {
		hisi_eis_debug("freq = %d!\n", freq);
		ret = sampling_for_freq_less_than_ten_ms(di, freq, idx);
		hisi_eis_debug("freq < 10!\n");
		di->freq_sample_info[idx].sample_prds = FREQ_SAMP_PRDS_10;
	/* 1000: freq eauals 1000mS */
	} else if (freq <= 1000) {
		ret = sampling_for_freq_less_than_thousand_ms(di, freq, idx);
	} else {
		ret = sampling_for_freq_greater_than_thousand_ms(di, freq, idx);
	}
	hisi_eis_inf("completed!, ret = %d\n", ret);
	return ret;
}


static void hisi_eis_set_cur_threshold(struct hisi_eis_device *di, enum eis_mode mode)
{
	int eis_detect_flag;
	int curr_offset;
	timespec_64 tv = {0};

	/* get offset current and set current threshold */
	di->eis_freq_dev.freq_ops->eis_sampling_init(di->batt_index);
	di->eis_ops->irq_seq_clear(IRQ_ALL);
	/* 4, 0,1: period, null periods, sampling periods */
	di->eis_ops->set_t_n_m(4, 0, 1);
	eis_detect_flag = di->eis_ops->eis_detect_flag();
	hisi_eis_debug("eis_detect_flag = %d!\n", eis_detect_flag);
	get_real_time_of_day(&tv);
	if (wait_eis_working_stop(di, tv, WAIT_EIS_END) == _ERROR) {
		di->eis_ops->eis_enable(false);
		hisi_eis_err("wait working stop time out!\n");
		return;
	}

	di->eis_ops->eis_enable(true);
	hisi_eis_debug("eis_enabled!\n");
	get_real_time_of_day(&tv);
	if (wait_eis_working_stop(di, tv, WAIT_EIS_END) == _ERROR) {
		hisi_eis_err("wait working stop time out!\n");
		di->eis_ops->eis_enable(false);
		return;
	}
	hisi_eis_debug("eis_detect_flag = %d!\n", eis_detect_flag);
	di->eis_ops->eis_enable(false);
	curr_offset = di->eis_ops->cur_offset();
	hisi_eis_inf("cur_offset = %d!\n", curr_offset);
	di->eis_freq_dev.freq_ops->set_cur_thres(di->batt_index, curr_offset);
	hisi_eis_inf("mode = %u, set threshold finished!\n", mode);
}

static int check_and_adjust_v_i_gain_for_freq(
	struct eis_freq_device *di, int idx)
{
	int ret_v, ret_i;

	ret_v = di->freq_ops->chk_adjust_freq_v_gain(
		&di->freq_sample_info[idx].v_gain,
		di->freq_sample_info[idx].sample_prds,
		(int (*)[V_BAT_FIFO_DEPTH])di->freq_sample_info[idx].v_bat);
	/* if i_gain gear is improper, resample */
	ret_i = di->freq_ops->chk_adjust_freq_i_gain(
		&di->freq_sample_info[idx].i_gain,
		di->freq_sample_info[idx].sample_prds,
		(int (*)[I_BAT_FIFO_DEPTH])di->freq_sample_info[idx].i_bat);
	if (ret_v == V_GAIN_ADJUSTED || ret_i == V_GAIN_ADJUSTED)
		return V_I_GAIN_ADJUSTED;
	return V_I_GAIN_UNADJUSTED;
}

static int hisi_eis_freq_sampling_work_exe(struct eis_freq_device *di)
{
	int err_cnt = 0;
	int i, ret, err_idx, bat_cycle_now;
	int gain_err_freq[LEN_T_FREQ] = {0};
	struct hisi_eis_device *eis_di = NULL;
	union power_supply_propval val = { 0 };

	if (di == NULL) {
		hisi_eis_err("di is null!\n");
		return _ERROR;
	}
	eis_di = container_of(di, struct hisi_eis_device, eis_freq_dev);

	for (i = 0; i < LEN_T_FREQ; i++) {
		hisi_eis_inf("freq loop idx=%d\n", i);
		hisi_eis_set_cur_threshold(eis_di, EIS_FREQ);
		/* 0: just an intermediate variable, could be [0, 15] */
		if (eis_di->psy) {
			bat_cycle_now = power_supply_get_property(eis_di->psy,
				POWER_SUPPLY_PROP_CYCLE_COUNT, &val);
			if (bat_cycle_now) {
				hisi_eis_err("get psy val err!");
				return _ERROR;
			}
			bat_cycle_now = val.intval;
		} else {
			bat_cycle_now = coul_drv_battery_cycle_count();
		}

		di->freq_info[i].cycle = bat_cycle_now;
		ret = hisi_eis_freq_start_sampling_func(di, i);
		if (ret == _STOP)
			return ret;

		/* if v_gain gear is improper, resample */
		hisi_eis_debug("%dth sample_prds = %d!\n", i,
			di->freq_sample_info[i].sample_prds);
		ret = check_and_adjust_v_i_gain_for_freq(di, i);
		if (ret == V_I_GAIN_ADJUSTED) {
			gain_err_freq[err_cnt] = (int)t_freq[i];
			err_cnt++;
		}
		msleep(50); /* 50: delay 50mS for next sampling */
		hisi_eis_inf("%dth, freq = %d,slept 50mS!\n", i, t_freq[i]);
	}

	/* for those improper gain gear freq cases, re-samplig later */
	for (i = 0; i < err_cnt; i++) {
		hisi_eis_err("gain_gear_err happened!\n");
		err_idx = di->freq_ops->locate_in_ary(
			t_freq, LEN_T_FREQ, gain_err_freq[i]);
		di->freq_ops->set_i_gain(di->freq_sample_info[err_idx].i_gain);
		di->freq_ops->set_v_gain(di->freq_sample_info[err_idx].v_gain);
		ret = hisi_eis_freq_start_sampling_func(di, err_idx);
		if (ret == _STOP)
			return ret;
		msleep(50); /* 50: delay 50mS for next sampling */
		hisi_eis_err("%dth, freq = %d resample ended,slept 50mS!\n",
			i, gain_err_freq[i]);
	}

	return _SUCC;
}


static int is_all_element_zero_in_array(const int *array, int len)
{
	int i;
	int cnt = 0;

	if (array == NULL || len < 1) {
		hisi_eis_err("len = %d, or null array, invalid\n", len);
		return _ERROR;
	}
	for (i = 0; i < len; i++) {
		if (array[i] == 0)
			cnt += 1;
	}
	return cnt;
}

static int hisi_eis_is_freq_sampling_valid(
	int frq, struct eis_freq_device *di)
{
	unsigned int i, j;
	int ret = _ERROR;
	unsigned int invalid_cnt = 0;
	int cur_over_flag, idx, act_v;

	idx = di->freq_ops->locate_in_ary(t_freq, LEN_T_FREQ, frq);
	if (idx == -1) {
		hisi_eis_err("freq = %d,invalid input\n", frq);
		return ret;
	}

	for (i = 0; i < di->freq_sample_info[idx].sample_prds; i++) {
		if (is_all_element_zero_in_array(
			di->freq_sample_info[idx].v_bat[i],
			V_BAT_FIFO_DEPTH) == V_BAT_FIFO_DEPTH) {
			invalid_cnt += 1;
			di->freq_sample_info[idx].valid_prd_idx[i] = false;
			continue;
		}
		for (j = 0; j < V_BAT_FIFO_DEPTH; j++) {
			/* 0, 8: no need to judge D1 and D9 */
			if (j == 0 || j == 8)
				continue;
			/*
			 * 12: the <12> bit
			 * 0,11: <11:0> bit
			 */
			cur_over_flag = eis_reg_getbits(
				di->freq_sample_info[idx].v_bat[i][j], 12, 12);
			act_v = eis_reg_getbits(
				di->freq_sample_info[idx].v_bat[i][j], 0, 11);
			if (cur_over_flag == CUR_OVERFLOW_FLAG || act_v == 0) {
				invalid_cnt += 1;
				di->freq_sample_info[idx].valid_prd_idx[i] = false;
				hisi_eis_inf("%dth period is invalid, cur_over_flag=%d, act_v=%d\n",
					i, cur_over_flag, act_v);
				break;
			}
		}
		if (j == V_BAT_FIFO_DEPTH) {
			di->freq_sample_info[idx].valid_sample_cnt += 1;
			di->freq_sample_info[idx].valid_prd_idx[i] = true;
		}
	}

	/* 2: invalid counts less than half of total periods is valid */
	if (di->freq_sample_info[idx].sample_prds > 0 &&
			2 * invalid_cnt <= di->freq_sample_info[idx].sample_prds) {
		di->freq_info[idx].sample_valid = true;
		hisi_eis_inf("freq = %d, EIS sample valid\n", t_freq[idx]);
		ret = _SUCC;
	} else {
		di->freq_info[idx].sample_valid = false;
		hisi_eis_err("freq = %d,EIS sample invalid\n", t_freq[idx]);
		ret = _ERROR;
	}
	return ret;
}

static int _compare(const void *_a, const void *_b)
{
	long long *a = (long long *)_a;
	long long *b = (long long *)_b;

	if (*a > *b)
		return 1;
	if (*a < *b)
		return -1;
	return 0;
}

static bool is_all_element_non_negative(
	const long long *sorted_array, int len)
{
	int i;

	if (sorted_array == NULL || len < 1) {
		hisi_eis_err("len = %d, or null ptr, invalid\n", len);
		return false;
	}

	for (i = 0; i < len; i++) {
		if (sorted_array[i] < 0)
			return false;
	}

	return true;
}

/*
 * for valid eis impedance over 5 cases, sort them and get the avg of
 * intermediate 5, otherwise, get the avged eis directly
 */
static long long get_averaged_freq_eis_real(
	const long long *sorted_array, int len)
{
	int i, start, end;
	int len_non_zero = 0;
	long long ret = 0;
	long long mean_thres;
	long long non_zero_ary[FREQ_SAMP_PRDS_10];

	if (sorted_array == NULL || len < 1) {
		hisi_eis_err("len = %d, or null ptr, invalid\n", len);
		return _ERROR;
	}

	for (i = 0; i < len; i++) {
		if (sorted_array[i] != 0) {
			non_zero_ary[len_non_zero] = sorted_array[i];
			len_non_zero++;
		}
	}

	if (len_non_zero == 0) {
		hisi_eis_inf("all real tmp is 0, retutn 0\n");
		return 0;
	}

	if (len_non_zero <= FREQ_EIS_AVGED_LEN) {
		for (i = 0; i < len_non_zero; i++)
			ret += non_zero_ary[i];
		ret /= len_non_zero;
		mean_thres = ret / 10; /* 10% */
		for (i = 0; i < len_non_zero; i++) {
			if (abs(non_zero_ary[i] - ret) > mean_thres) {
				hisi_eis_err("mean diff over 10 percent, eis_real set -1\n");
				ret = -1;
				break;
			}
		}
	} else {
		start = (len_non_zero - FREQ_EIS_AVGED_LEN) / 2;
		end = start + FREQ_EIS_AVGED_LEN;
		for (i = start; i < end; i++)
			ret += non_zero_ary[i];
		ret /= FREQ_EIS_AVGED_LEN;
		mean_thres = ret / 10; /* 10% */
		for (i = start; i < end; i++) {
			if (abs(non_zero_ary[i] - ret) > mean_thres) {
				hisi_eis_err("mean diff over 10 percent, eis_real set -1\n");
				ret = -1;
				break;
			}
		}
	}

	return ret;
}

static long long get_averaged_freq_eis_imag(
	const long long *sorted_array, int len)
{
	int i;
	int len_non_zero = 0;
	int start;
	int end;
	long long ret = 0;
	long long non_zero_ary[FREQ_SAMP_PRDS_10];

	if (sorted_array == NULL || len < 1) {
		hisi_eis_err("len = %d, or null ptr, invalid\n", len);
		return _ERROR;
	}
	for (i = 0; i < len; i++) {
		if (sorted_array[i] != 0) {
			non_zero_ary[len_non_zero] = sorted_array[i];
			len_non_zero++;
		}
	}

	if (len_non_zero == 0) {
		hisi_eis_inf("all imag tmp is 0, retutn 0\n");
		return 0;
	}

	if (len_non_zero <= FREQ_EIS_AVGED_LEN) {
		for (i = 0; i < len_non_zero; i++)
			ret += non_zero_ary[i];
		ret /= len_non_zero;
		hisi_eis_debug("ret = %lld\n", ret);
	} else {
		start = (len_non_zero - FREQ_EIS_AVGED_LEN) / 2;
		end = start + FREQ_EIS_AVGED_LEN;
		for (i = start; i < end; i++)
			ret += non_zero_ary[i];
		ret /= FREQ_EIS_AVGED_LEN;
		hisi_eis_debug("ret = %lld\n", ret);
	}

	return ret;
}

static int hisi_eis_init_freq_raw_to_zero(
	struct eis_freq_device *di, int idx)
{
	int ret;

	ret = memset_s(di->freq_sample_info[idx].raw_eis_r, SIZE_OF_RAW_EIS, 0,
		SIZE_OF_RAW_EIS);
	if (ret != EOK) {
		hisi_eis_err("memset_s failed, ret = %d!\n", ret);
		return _ERROR;
	}
	ret = memset_s(di->freq_sample_info[idx].raw_eis_i, SIZE_OF_RAW_EIS, 0,
		SIZE_OF_RAW_EIS);
	if (ret != EOK) {
		hisi_eis_err("memset_s failed, ret = %d!\n", ret);
		return _ERROR;
	}
	return _SUCC;
}

static bool hisi_eis_cal_freq_intermediate_data_check(
	struct eis_freq_device *di, int idx)
{
	bool data_judge = false;

	data_judge = is_all_element_non_negative(
		di->freq_sample_info[idx].raw_eis_r, FREQ_SAMP_PRDS_10);
	if (data_judge == false) {
		di->freq_info[idx].real = CAL_ERR;
		hisi_eis_err("freq = 1000/%d, negative err, eis_real set to -1!\n",
			t_freq[idx]);
		return false;
	}
	return true;
}

static void hisi_eis_cal_freq_avg_val(struct eis_freq_device *di, int idx)
{
	sort(di->freq_sample_info[idx].raw_eis_r, FREQ_SAMP_PRDS_10,
		sizeof(long long), &_compare, NULL);
	sort(di->freq_sample_info[idx].raw_eis_i, FREQ_SAMP_PRDS_10,
		sizeof(long long), &_compare, NULL);
	/* for over 5 cases, sort them and get the avg of intermediate 5 */
	di->freq_info[idx].real = get_averaged_freq_eis_real(
		di->freq_sample_info[idx].raw_eis_r, FREQ_SAMP_PRDS_10);
	hisi_eis_inf("freq = 1000/%d, eis_real = %lld!\n",
		 t_freq[idx], di->freq_info[idx].real);
	di->freq_info[idx].imag = get_averaged_freq_eis_imag(
		di->freq_sample_info[idx].raw_eis_i, FREQ_SAMP_PRDS_10);
	hisi_eis_inf("freq = 1000/%d, eis_imag = %lld!\n",
		 t_freq[idx], di->freq_info[idx].imag);
}

static void hisi_eis_calculate_freq_func(struct eis_freq_device *di, int idx)
{
	unsigned int i;
	int v_gain;
	int ret;

	v_gain = di->freq_sample_info[idx].v_gain;
	if (v_gain <= 0) {
		hisi_eis_err("v_gain = %d, invalid!\n", v_gain);
		return;
	}
	hisi_eis_inf("v_gain = %d!\n", v_gain);

	ret = hisi_eis_init_freq_raw_to_zero(di, idx);
	if (ret == _ERROR) {
		hisi_eis_err("init freq raw to zero failed!\n");
		return;
	}
	for (i = 0; i < di->freq_sample_info[idx].sample_prds; i++) {
		if (di->freq_sample_info[idx].valid_prd_idx[i] == true) {
			di->freq_ops->get_raw_eis_ri(&di->freq_sample_info[idx].raw_eis_r[i],
				&di->freq_sample_info[idx].raw_eis_i[i], v_gain,
				di->freq_sample_info[idx].v_bat[i], V_BAT_FIFO_DEPTH);
		} else {
			hisi_eis_err("idx%d invalid, set to 0\n", idx);
			di->freq_sample_info[idx].raw_eis_r[i] = 0;
			di->freq_sample_info[idx].raw_eis_i[i] = 0;
		}
	}
	if (hisi_eis_cal_freq_intermediate_data_check(di, idx) == false) {
		di->freq_info[idx].imag = CAL_ERR;
		hisi_eis_err("freq = 1000/%d, negative in eis_real, eis_imag set to -1!\n",
			t_freq[idx]);
		return;
	}
}

static int hisi_eis_is_freq_mod_valid(struct eis_freq_device *di, int idx)
{
	int i;
	u64 freq_mod2;
	u32 freq_mod[FREQ_SAMP_PRDS_10];
	u32 avg = 0;
	int valid = 0;
	int freq_cnt = 0;

	for (i = 0; i < di->freq_sample_info[idx].sample_prds; i++) {
		freq_mod2 = sum_of_squares(di->freq_sample_info[idx].raw_eis_r[i],
			di->freq_sample_info[idx].raw_eis_i[i]);
		if (freq_mod2 > 0) {
			freq_mod[freq_cnt] = int_sqrt64(freq_mod2);
			avg += freq_mod[freq_cnt];
			freq_cnt++;
		}
	}

	if (freq_cnt == 0)
		return _ERROR;
	avg = avg / freq_cnt;

	/* each freq valid condition:diff below 2.5% avg freq */
	for (i = 0; i < freq_cnt; i++) {
		if (abs(freq_mod[i] - avg) < (avg * 1 / 40))
			valid++;
		hisi_eis_inf("freq = %d, freq_mod=%d, abs(dif)=%d\n",
			t_freq[idx], freq_mod[i], abs(freq_mod[i] - avg));
	}
	hisi_eis_inf("valid_num=%d, total_valid=%d, avg=%d, avg/40 = %d\n", valid, freq_cnt, avg, avg * 1 / 40);
	/* 8: if over 80% of freq valid, succ */
	if (valid * 10 >= freq_cnt * 8)
		return _SUCC;
	else
		return _ERROR;
}

static int eis_check_freq_valid(struct hisi_eis_device *eis_di, struct eis_freq_device *di)
{
	int i, ret;
	int fail_cnt = 0;
	int ret2 = _SUCC;

	for (i = 0; i < LEN_T_FREQ; i++) {
		hisi_eis_sampling_data_print(EIS_FREQ, eis_di, i);
		ret = hisi_eis_is_freq_sampling_valid(t_freq[i], di);
		if (ret == _SUCC) {
			hisi_eis_calculate_freq_func(di, i);
			if (di->avg_freq_check)
				ret2 = hisi_eis_is_freq_mod_valid(di, i);
		}
		if (ret == _SUCC && ret2 == _SUCC) {
			hisi_eis_cal_freq_avg_val(di, i);
		} else {
			hisi_eis_err("freq = %d, invalid sampling, eis_imag and eis_real set to -1!\n",
				 t_freq[i]);
			di->freq_info[i].imag = CAL_ERR;
			di->freq_info[i].real = CAL_ERR;
			fail_cnt++;
		}
	}
	if (fail_cnt >= LEN_T_FREQ) {
		hisi_eis_err("all cal failed, not notify and save to flash!\n");
		eis_wake_unlock(eis_di);
		set_freq_work_flag(eis_di, 0);
		return _ERROR;
	}
	return _SUCC;
}

static int cal_eacr_val(struct eis_freq_device *di)
{
	int i;
	int ret;
	long long raw_eacr_r[FREQ_SAMP_PRDS_10];
	int idx = EACR_FREQ_INDEX;
	long long batt_eacr;

	if (di->freq_info[idx].real == CAL_ERR) {
		hisi_eis_err("invalid %dms eis_data, do not cal eACR", t_freq[idx]);
		return CAL_ERR;
	}

	ret = memset_s(raw_eacr_r, SIZE_OF_RAW_EIS, 0, SIZE_OF_RAW_EIS);
	if (ret != EOK) {
		hisi_eis_err("memset_s failed, ret = %d!\n", ret);
		return CAL_ERR;
	}

	for (i = 0; i < di->freq_sample_info[idx].sample_prds; i++)
		raw_eacr_r[i] = di->freq_ops->get_raw_eacr_real(
			di->freq_sample_info[idx].v_gain, di->freq_sample_info[idx].v_bat[i],
			V_BAT_FIFO_DEPTH);

	batt_eacr = get_averaged_freq_eis_real(raw_eacr_r, FREQ_SAMP_PRDS_10);
	return (int)(batt_eacr * UOHM_PER_MOHM / COEF_MAG_TIMES);
}


static void hisi_eis_freq_cal_impedance_work(struct work_struct *work)
{
	int ret;
	errno_t err;
	struct eis_freq_infos infos;
	const char *name;
	struct hisi_eis_device *eis_di = NULL;
	struct eis_freq_device *di =
		container_of(work, struct eis_freq_device, freq_cal_work.work);
	if (di == NULL) {
		hisi_eis_err("eis_freq_device is null\n");
		return;
	}

	eis_di = container_of(di, struct hisi_eis_device, eis_freq_dev);
	if (eis_di == NULL) {
		hisi_eis_err("hisi_eis_device is null\n");
		return;
	}

	/* prevent system sleep when calculating eis_freq */
	hisi_eis_inf("start eis batt_%d!\n", eis_di->batt_index);
	eis_wake_lock(eis_di);

	name = get_battery_name(eis_di->batt_index);
	eis_di->psy = power_supply_get_by_name(name);
	if (eis_di->psy == NULL)
		hisi_eis_err("power supply named %s not exist, use coul_drv api!\n", name);

	if (eis_get_g_freq_rolling_now(eis_di->batt_index) == 0 &&
		eis_get_g_full_freq_rolling(eis_di->batt_index) == 0)
		hisi_eis_freq_read_index_from_flash(eis_di->batt_index);
	if (hisi_eis_is_sampling_allowed(eis_di, EIS_FREQ) == false) {
		hisi_eis_err("freq sampling condition not satisfied!\n");
		goto EXIT_CAL;
	}
	ret = hisi_eis_freq_sampling_work_exe(di); /* start sampling */
	if (ret != _SUCC) {
		hisi_eis_err("freq sampling execution failed!\n");
		goto EXIT_CAL;
	}
	if (eis_check_freq_valid(eis_di, di) == _ERROR)
		goto EXIT_CAL;

	if (eis_di->eacr_support) {
		di->eacr_info.batt_acr = cal_eacr_val(di);
		if (di->eacr_info.batt_acr != CAL_ERR) {
			di->eacr_info.batt_cycle = di->freq_info[EACR_FREQ_INDEX].cycle;
			di->eacr_info.batt_temp = di->freq_info[EACR_FREQ_INDEX].temp;
			di->eacr_info.chip_temp[0] = di->eacr_info.batt_temp;
			di->eacr_info.chip_temp[1] = di->eacr_info.batt_temp;
			di->eacr_info.batt_vol = coul_drv_battery_voltage();
			di->eacr_info.batt_index = eis_di->batt_index;
			hisi_eis_inf("eACR cal succ, real=%d\n", di->eacr_info.batt_acr);

			hisi_call_eis_blocking_notifiers(EVT_EIS_EACR_UPDATE, (void *)&di->eacr_info);
			sysfs_notify(&eis_di->dev->kobj, NULL, "eacr_raw_data");
		}
	}

	sysfs_notify(&eis_di->dev->kobj, NULL, "freq_raw_data");
	infos.batt_index = eis_di->batt_index;
	infos.num_of_info = LEN_T_FREQ;
	err = memcpy_s(infos.freq_infos, sizeof(struct eis_freq_info) * infos.num_of_info,
		di->freq_info, sizeof(di->freq_info));
	if (err != EOK)
		hisi_eis_err("eis freq memcpy_s failed!\n");
	hisi_call_eis_blocking_notifiers(EVT_EIS_FREQ_UPDATE, (void *)&infos);
	hisi_eis_inf("write to freq flash!\n");
	hisi_eis_freq_write_info_to_flash(di->freq_info, eis_di->batt_index);
EXIT_CAL:
	if (eis_di->psy)
		power_supply_put(eis_di->psy);
	eis_wake_unlock(eis_di);
	set_freq_work_flag(eis_di, 0);
}

static int eis_freq_calculate_notifier_callback(
	struct notifier_block *freq_nb, unsigned long event, void *data)
{
	struct eis_freq_device *di = container_of(freq_nb,
		struct eis_freq_device, eis_freq_notify);
	struct hisi_eis_device *eis_di;
	int ret;

	if (di == NULL) {
		hisi_eis_err("di is null, invalid!\n");
		return NOTIFY_DONE;
	}

	if (event == BATT_EIS_FREQ) {
		eis_di = container_of(di, struct hisi_eis_device, eis_freq_dev);
		if (eis_di == NULL || data == NULL) {
			hisi_eis_err("eis_di or data is null, invalid!\n");
			return NOTIFY_DONE;
		}

		if (eis_di->batt_index != *(int *)data) {
			hisi_eis_err("batt %d, input index=%d, return!",
				eis_di->batt_index, *(int *)data);
			return NOTIFY_DONE;
		}

		if (di->freq_ops->read_work_flag() == 1) {
			hisi_eis_inf("other eis is running, return\n");
			return NOTIFY_DONE;
		}

#ifdef CONFIG_BATT_SOH
		if (get_soh_core_device() != NULL) {
			mutex_lock(&get_soh_core_device()->soh_mutex);
			if (get_soh_core_device()->soh_acr_dev.acr_work_flag == 1 ||
				get_soh_core_device()->soh_dcr_dev.dcr_work_flag == 1) {
				mutex_unlock(&get_soh_core_device()->soh_mutex);
				hisi_eis_inf("acr or dcr is working!\n");
				return NOTIFY_DONE;
			}
			mutex_unlock(&get_soh_core_device()->soh_mutex);
		}
#endif
		di->freq_ops->set_work_flag(1);
		ret = queue_delayed_work(system_power_efficient_wq,
			&di->freq_cal_work, msecs_to_jiffies(0));
		hisi_eis_inf("batt_%d, ret of freq cal work = %d!\n",
			eis_di->batt_index, ret);
	}
	return NOTIFY_OK;
}

static int eis_freq_init(struct hisi_eis_device *di)
{
	int ret;

	if (di == NULL) {
		hisi_eis_err("di is null!\n");
		return _ERROR;
	}
	ret = parse_eis_freq_dts(di);
	if (ret == _ERROR) {
		hisi_eis_err("parse freq dts fail!\n");
		return _ERROR;
	}
	/* already parsed, but not support */
	if (di->eis_freq_dev.freq_support == 0) {
		hisi_eis_err("not support eis freq!\n");
		return _ERROR;
	}

	ret = eis_drv_ops_check(di, FREQ_DRV_OPS);
	if (ret != _SUCC) {
		hisi_eis_err("freq_drv_ops check fail!\n");
		return _ERROR;
	}
	ret = eis_drv_ops_check(di, EIS_OPS);
	if (ret != _SUCC) {
		hisi_eis_err("eis_core_ops check fail!\n");
		return _ERROR;
	}
	di->eis_freq_dev.eis_freq_notify.notifier_call =
		eis_freq_calculate_notifier_callback;
	ret = coul_register_blocking_notifier(
		&di->eis_freq_dev.eis_freq_notify);
	if (ret < 0) {
		hisi_eis_err("eis_freq_register_notifier failed!\n");
		return _ERROR;
	}
	/* init interruptable notifier work */
	INIT_DELAYED_WORK(&di->eis_freq_dev.freq_cal_work,
		hisi_eis_freq_cal_impedance_work);
	hisi_eis_inf("eis_freq init success\n");
	return _SUCC;
}

static void eis_freq_uninit(struct hisi_eis_device *di)
{
	(void)coul_unregister_blocking_notifier(
		&di->eis_freq_dev.eis_freq_notify);
	hisi_eis_inf("eis_freq uninit succ\n");
}

static int eis_probe(struct platform_device *pdev)
{
	int ret;
	struct hisi_eis_device *di = NULL;

	di = (struct hisi_eis_device *)devm_kzalloc(&pdev->dev, sizeof(*di),
		GFP_KERNEL);
	if (di == NULL)
		return -ENOMEM;

	di->dev = &pdev->dev;
	platform_set_drvdata(pdev, di);
	/* init wakeup lock */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	di->eis_wake_lock = wakeup_source_register(&pdev->dev, "eis_wakelock");
#else
	di->eis_wake_lock = wakeup_source_register("eis_wakelock");
#endif

	hisi_eis_register_fb_usb_list(di);
	ret = eis_freq_init(di);
	if (ret == _ERROR) {
		hisi_eis_err("eis freq init fail!\n");
		goto eis_freq_fail;
	}

	ret = eis_create_sysfs_file(di);
	if (ret != 0) {
		hisi_eis_err("create sysfs fail!\n");
		goto sysfs_fail;
	}
	g_eis_di[di->batt_index] = di;
	hisi_eis_inf("probe eis batt_%d ok!\n", di->batt_index);
	return 0;

sysfs_fail:
	eis_freq_uninit(di);
eis_freq_fail:
	hisi_eis_unregister_fb_usb_list(di);
	wakeup_source_unregister(di->eis_wake_lock);
	platform_set_drvdata(pdev, NULL);
	return ret;
}

static int eis_remove(struct platform_device *pdev)
{
	struct hisi_eis_device *di = platform_get_drvdata(pdev);
	unsigned int batt;

	if (di == NULL) {
		hisi_eis_err(" null ptr di!\n");
		return -ENODEV;
	}

	batt = di->batt_index;
	eis_freq_uninit(di);
	hisi_eis_unregister_fb_usb_list(di);
	wakeup_source_unregister(di->eis_wake_lock);
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	g_eis_di[batt] = NULL;
	return 0;
}

static const struct of_device_id hisi_eis_core_match_table[] = {
	{
		.compatible = "hisi,eis_core",
	},
	{
	},
};

static void eis_shutdown(struct platform_device *pdev)
{
	struct hisi_eis_device *di = platform_get_drvdata(pdev);

	if (di == NULL)
		return;
	hisi_eis_err("ok!\n");
}

static struct platform_driver hisi_eis_core_driver = {
	.probe          = eis_probe,
	.remove         = eis_remove,
	.shutdown       = eis_shutdown,
	.driver         = {
		.name           = "hisi_eis_core",
		.owner          = THIS_MODULE,
		.of_match_table = hisi_eis_core_match_table,
	},
};

static int __init eis_core_init(void)
{
	return platform_driver_register(&hisi_eis_core_driver);
}

static void __exit eis_core_exit(void)
{
	platform_driver_unregister(&hisi_eis_core_driver);
}

late_initcall(eis_core_init);
module_exit(eis_core_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("eis core driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
