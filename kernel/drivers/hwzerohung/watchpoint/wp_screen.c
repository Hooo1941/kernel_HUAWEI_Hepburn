/*
 * wp_screen.c
 *
 * Reporting the frozen screen alarm event when the screen is abnormal
 *
 * Copyright (c) 2017-2021 Huawei Technologies Co., Ltd.
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

#include "wp_screen.h"

#include <linux/version.h>
/* no sched/clock.h before kernel4.14 in which sched_show_task is declared */
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/clock.h>
#include <linux/sched/debug.h>
#endif

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/time.h>
#include <linux/input.h>
#include <linux/jiffies.h>
#include <linux/sched/debug.h>
#include <securec.h>
#include <platform/linux/zrhung.h>
#include <platform/linux/hungtask.h>
#include <platform/trace/events/rainbow.h>
#include <platform/linux/rainbow.h>

#ifdef CONFIG_POWERKEY_SPMI
#include <platform_include/basicplatform/linux/powerkey_event.h>
#endif
#if defined(CONFIG_KEYBOARD_GPIO_VOLUME_KEY) || defined(CONFIG_KEYBOARD_PMIC_VOLUME_KEY)
#include <platform_include/basicplatform/linux/keypad.h>
#endif
#ifdef CONFIG_KEYBOARD_PMIC_VOLUME_KEY
#include <platform_include/basicplatform/linux/keypad_pmic_event.h>
#else
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#define OF_GPIO_ACTIVE_LOW 0x1
#define PON_KPDPWR 0
#define PON_RESIN 1
#define PON_KEYS 2
#if (KERNEL_VERSION(4, 15, 0) <= LINUX_VERSION_CODE)
static int *check_off_point;
#endif
#endif

#define TIME_CONVERT_UNIT 1000
#define DEFAULT_TIMEOUT 10
#define CONFIG_LEN_MAX 64
#define POWER_OFF_MENU_APPEAR_TIMEOUT 0.5

#define SCREEN_ON_PANIC_COUNT 8
#ifdef CONFIG_DFX_OHOS
#define WP_SCREEN_ENABLE 1
#define POWERKEYEVENT_ENABLE 1
#define POWERKEYEVENT_LIMIT_MS 500
#endif
#define POWERKEYEVENT_MAX_COUNT 10
#define POWERKEYEVENT_DEFAULT_COUNT 3
#define POWERKEYEVENT_MAX_TIMEWINDOW 20
#define POWERKEYEVENT_DEFAULT_TIMEWINDOW 5
#define POWERKEYEVENT_MAX_LIMIT_MS 2000
#define POWERKEYEVENT_DEFAULT_LIMIT_MS 300
#define POWERKEYEVENT_MAX_REPORT_MIN 3600
#define POWERKEYEVENT_DEFAULT_REPORT_MIN 2
#define POWERKEYEVENT_TIME_LEN (POWERKEYEVENT_MAX_COUNT + 2)

struct hung_wp_screen_config {
	int enable;
	uint32_t timeout;
};
struct zrhung_powerkeyevent_config {
	int enable;
	unsigned int timewindow;
	unsigned int count;
	unsigned int ignore_limit;
	unsigned int report_limit;
};

struct hung_wp_screen_data {
	struct hung_wp_screen_config *config;
	struct timer_list timer;
	struct timer_list power_off_menu_timer;
#if (!defined(CONFIG_KEYBOARD_GPIO_VOLUME_KEY)) && (!defined(CONFIG_KEYBOARD_PMIC_VOLUME_KEY))
	struct timer_list long_press_timer;
#endif
	struct workqueue_struct *workq;
	struct work_struct config_work;
	struct work_struct send_work;
	spinlock_t lock;
	int bl_level;
	int check_id;
	int tag_id;
	int screen_on_count;
};

static bool config_done;
static bool init_done;
static struct hung_wp_screen_data g_data;
#ifdef CONFIG_MTK_ZRHUNG_FEATURE
static unsigned int vkeys_temp;
#endif
static unsigned int vkeys_pressed;

static int check_off;

static struct hung_wp_screen_config on_config;
static struct hung_wp_screen_config off_config;

static bool prkyevt_config_done;
static struct zrhung_powerkeyevent_config event_config = {0};

static unsigned int lastreport_time;
static unsigned int lastprkyevt_time;
static unsigned int powerkeyevent_time[POWERKEYEVENT_TIME_LEN] = {0};

static unsigned int newevt;
static unsigned int headevt;
static int g_screen_bl_level = 100;  /* initial screen backlight, non-zero value */
struct work_struct powerkeyevent_config;
struct work_struct powerkeyevent_sendwork;
#ifdef CONFIG_DFX_OHOS
struct work_struct lpressevent_sendwork;
#endif

#ifdef CONFIG_POWERKEY_SPMI
static struct notifier_block wp_screen_powerkey_notify;
#endif
#ifdef CONFIG_KEYBOARD_PMIC_VOLUME_KEY
static struct notifier_block wp_screen_volumekey_notify;
#endif

/* Get configuration from zerohung_config.xml */
static int zrhung_powerkeyevent_get_config(struct zrhung_powerkeyevent_config *cfg)
{
	int ret;
#ifdef CONFIG_DFX_OHOS
	cfg->enable = POWERKEYEVENT_ENABLE;
	cfg->timewindow = POWERKEYEVENT_DEFAULT_TIMEWINDOW;
	cfg->count = POWERKEYEVENT_DEFAULT_COUNT;
	cfg->ignore_limit = POWERKEYEVENT_LIMIT_MS;
	cfg->report_limit = POWERKEYEVENT_DEFAULT_REPORT_MIN;

	pr_info("%s: ZRHUNG_EVENT_POWERKEY %d, %d, %d, %d, %d\n",
	       __func__, cfg->enable, cfg->timewindow, cfg->count,
	       cfg->ignore_limit, cfg->report_limit);
	return ret;
#endif
	char config_str[CONFIG_LEN_MAX] = {0};

	ret = zrhung_get_config(ZRHUNG_EVENT_POWERKEY, config_str,
		sizeof(config_str));
	if (ret != 0) {
		pr_err("%s: powerkeyevent read config fail: %d\n",
		       __func__, ret);
		cfg->enable = 0;
		cfg->timewindow = POWERKEYEVENT_DEFAULT_TIMEWINDOW;
		cfg->count = POWERKEYEVENT_DEFAULT_COUNT;
		cfg->ignore_limit = POWERKEYEVENT_DEFAULT_LIMIT_MS;
		cfg->report_limit = POWERKEYEVENT_DEFAULT_REPORT_MIN;
		return ret;
	}
	/* 5: the number of successful values returned */
	if (sscanf_s(config_str, "%d,%d,%d,%d,%d", &cfg->enable,
		&cfg->timewindow, &cfg->count, &cfg->ignore_limit,
		&cfg->report_limit) != 5)
		pr_err("%s: get args from config_str not 5\n", __func__);

	pr_err("%s: ZRHUNG_EVENT_POWERKEY %d, %d, %d, %d, %d\n",
	       __func__, cfg->enable, cfg->timewindow, cfg->count,
	       cfg->ignore_limit, cfg->report_limit);

	if ((cfg->timewindow <= 0) ||
		(cfg->timewindow > POWERKEYEVENT_MAX_TIMEWINDOW))
		cfg->timewindow = POWERKEYEVENT_DEFAULT_TIMEWINDOW;

	if ((cfg->count <= 0) || (cfg->count > POWERKEYEVENT_MAX_COUNT))
		cfg->count = POWERKEYEVENT_DEFAULT_COUNT;

	if ((cfg->ignore_limit <= 0) ||
		(cfg->ignore_limit > POWERKEYEVENT_MAX_LIMIT_MS))
		cfg->ignore_limit = POWERKEYEVENT_DEFAULT_LIMIT_MS;

	if ((cfg->report_limit <= 0) ||
		(cfg->report_limit > POWERKEYEVENT_MAX_REPORT_MIN))
		cfg->report_limit = POWERKEYEVENT_DEFAULT_REPORT_MIN;

	return ret;
}

/* Get configuration from zerohung_config.xml */
static void zrhung_powerkeyevent_config_work(struct work_struct *work)
{
	int ret;

	ret = zrhung_powerkeyevent_get_config(&event_config);
	if (ret == 0) {
		prkyevt_config_done = true;
		pr_err("%s: read config success\n", __func__);
	}
}

#ifdef CONFIG_DFX_OHOS
/* send event to zrhung when powerkey longpress  */
static void zrhung_lpressevent_send_work(struct work_struct *work)
{
	pr_info("%s, LONGPRESS_EVENT send to zerohung\n", __func__);
	zrhung_send_event(ZRHUNG_EVENT_LONGPRESS, NULL, "powerkey long press");
}
#endif

/* send event to zrhung when match out requirements model */
static void zrhung_powerkeyevent_send_work(struct work_struct *work)
{
	pr_err("%s: ZRHUNG_EVENT_POWERKEY send to zerohung\n", __func__);
	zrhung_send_event(ZRHUNG_EVENT_POWERKEY, NULL, "power key event");
}

/* send event to zrhung when match out requirements model */
static void zrhung_powerkeyevent_report(unsigned int dur, unsigned int end)
{
	unsigned int send_interval;

	send_interval = end > lastreport_time ?
		((end - lastreport_time) / TIME_CONVERT_UNIT) :
		POWERKEYEVENT_DEFAULT_REPORT_MIN;
	if (unlikely(lastreport_time == 0)) {
		lastreport_time = end;
	} else if (send_interval < event_config.report_limit) {
		pr_err("%s: powerkeyevent too fast to report: %d\n",
			__func__, end);
		return;
	}
	lastreport_time = end;
	queue_work(g_data.workq, &powerkeyevent_sendwork);
}

/* just to build a loop in the array */
static unsigned int refresh_prkyevt_index(unsigned int event)
{
	unsigned int evt = event;

	if (evt < POWERKEYEVENT_MAX_COUNT)
		evt++;
	else
		evt = 0;
	return evt;
}

/* handle each powerkey event and check if match our model */
static void zrhung_new_powerkeyevent(unsigned int tmescs)
{
	unsigned int prkyevt_interval;
	unsigned int evt_index;
	int diff;

	powerkeyevent_time[newevt] = tmescs;
	evt_index = (newevt >= headevt) ?
		(newevt - headevt) :
		(newevt + POWERKEYEVENT_MAX_COUNT + 1 - headevt);
	if (evt_index < (event_config.count - 1)) {
		pr_err("%s: powerkeyevent not enough-%d\n",
			__func__, event_config.count);
	} else {
		diff = powerkeyevent_time[newevt] - powerkeyevent_time[headevt];
		if (diff < 0) {
			pr_err("%s: powerkeyevent sth wrong in record time\n",
				__func__);
			return;
		}

		prkyevt_interval = (unsigned int)(diff / TIME_CONVERT_UNIT);
		if (prkyevt_interval <= event_config.timewindow) {
			pr_err("%s: powerkeyevent check send to zerohung %d\n",
				__func__, tmescs);
			zrhung_powerkeyevent_report(prkyevt_interval, tmescs);
		}

		headevt = refresh_prkyevt_index(headevt);
	}
	newevt = refresh_prkyevt_index(newevt);
}

/* handle each powerkey event and check if match our model */
static void zrhung_powerkeyevent_handler(void)
{
	unsigned int curtime;
	unsigned long curjiff;

	if (!prkyevt_config_done) {
		pr_err("%s: powerkeyevent prkyevt_config_done=%d, read it\n",
			__func__, prkyevt_config_done);
		queue_work(g_data.workq, &powerkeyevent_config);
		return;
	}
	if (event_config.enable == 0) {
		pr_err("%s: powerkeyevent check disable\n", __func__);
		return;
	}
	pr_err("%s: powerkeyevent check start\n", __func__);
	curjiff = jiffies;
	curtime = jiffies_to_msecs(curjiff);
	if (unlikely(lastprkyevt_time > curtime)) {
		pr_err("%s: powerkeyevent check but time overflow\n", __func__);
		lastprkyevt_time = curtime;
		return;
	} else if ((curtime - lastprkyevt_time) < event_config.ignore_limit) {
		pr_err("%s: powerkeyevent user press powerkey too fast-time:%d\n",
		     __func__, curtime);
		return;
	}
	lastprkyevt_time = curtime;
	zrhung_new_powerkeyevent(curtime);
}

/*
 * if there is a volumn key press  between powerkey press and release,
 * the event that screen being set to off would be stopped so screen-off
 * watchpoint should not report this normal situation.
 * -function call back from volume keys driver and get state to vkeys_pressed
 *
 * NOTE: 'value' must be in {0,1}
 */
void hung_wp_screen_vkeys_cb(unsigned int knum, unsigned int value)
{
	if ((knum == WP_SCREEN_VUP_KEY) || (knum == WP_SCREEN_VDOWN_KEY)) {
#ifdef CONFIG_MTK_ZRHUNG_FEATURE
		vkeys_pressed = knum;
		// flag: bit 1 for VOLUME UP, bit 0 for VOLUME DOWN
		// value: set 1 for PRESS, set 0 for RELEASE
		if (knum == WP_SCREEN_VUP_KEY)
			vkeys_temp = (value << 1) + (vkeys_temp & 1);
		else
			vkeys_temp = (vkeys_temp & 2) + value;
#else
		vkeys_pressed |= knum;
#endif
	} else {
		pr_err("%s: unknown volume keynum:%u\n", __func__, knum);
	}
}

/* get a value which indicates if any vkeys has pressed */
static unsigned int hung_wp_screen_has_vkeys_pressed(void)
{
	return vkeys_pressed;
}

/* clear the pressed vkeys value */
static void hung_wp_screen_clear_vkeys_pressed(void)
{
	vkeys_pressed = 0;
}

/* Get configuration from zerohung_config.xml */
static int hung_wp_screen_get_config(int id, struct hung_wp_screen_config *cfg)
{
	int ret;
#ifdef CONFIG_DFX_OHOS
	cfg->enable = WP_SCREEN_ENABLE;
	cfg->timeout = DEFAULT_TIMEOUT;
	return ret;
#endif
	char config[CONFIG_LEN_MAX] = {0};

	ret = zrhung_get_config(id, config, CONFIG_LEN_MAX);
	if (ret != 0) {
		pr_err("%s: read config:%d fail: %d\n", __func__, id, ret);
		cfg->enable = 0;
		cfg->timeout = DEFAULT_TIMEOUT;
		return ret;
	}

	/* 2: the number of successful values returned */
	if (sscanf_s(config, "%d,%d", &cfg->enable, &cfg->timeout) != 2)
		pr_err("%s: get enable or timeout failed\n", __func__);

	pr_err("%s: id=%d, enable=%d, timeout=%d\n",
	       __func__, id, cfg->enable, cfg->timeout);
	if (cfg->timeout <= 0)
		cfg->timeout = DEFAULT_TIMEOUT;

	return ret;
}

/* Get configuration from zerohung_config.xml */
static void hung_wp_screen_config_work(struct work_struct *work)
{
	int ret_on;
	int ret_off;

	ret_on = hung_wp_screen_get_config(ZRHUNG_WP_SCREENON, &on_config);
	ret_off = hung_wp_screen_get_config(ZRHUNG_WP_SCREENOFF, &off_config);
	if ((ret_on <= 0) && (ret_off <= 0)) {
		config_done = true;
		pr_err("%s: init done\n", __func__);
	}
}

/* AOD mode, lcd fake power off, close SCREEN_OFF */
void hung_wp_screen_fake_power_off(void)
{
	unsigned long flags;

	spin_lock_irqsave(&(g_data.lock), flags);
	if (!config_done) {
		spin_unlock_irqrestore(&(g_data.lock), flags);
		return;
	}
	del_timer(&g_data.timer);
	pr_info("%s: AOD mode is fake power off, close screen_off check\n", __func__);
	g_data.check_id = ZRHUNG_WP_NONE;
	g_data.screen_on_count = 0;
	spin_unlock_irqrestore(&(g_data.lock), flags);
}

/* Get lcd screen backlight */
void hung_wp_screen_getbl(int *screen_bl_level)
{
	*screen_bl_level = g_screen_bl_level;
}

/*
 * Called from lcd driver when setting backlight for mark:
 *   For hisi platform, it's "hisifb_set_backlight" in hisi_fb_bl.c
 */
void hung_wp_screen_setbl(int level)
{
	unsigned long flags;

	if (!init_done)
		return;

	spin_lock_irqsave(&(g_data.lock), flags);
	g_data.bl_level = level;
	g_screen_bl_level = level;
	if (!config_done) {
		spin_unlock_irqrestore(&(g_data.lock), flags);
		return;
	}
	if (((g_data.check_id == ZRHUNG_WP_SCREENON) && (level != 0)) ||
	    ((g_data.check_id == ZRHUNG_WP_SCREENOFF))) {
		pr_err("%s: check_id=%d, level=%d\n", __func__,
		       g_data.check_id, g_data.bl_level);
		del_timer(&g_data.timer);
		g_data.check_id = ZRHUNG_WP_NONE;
		g_data.screen_on_count = 0;
	}
	spin_unlock_irqrestore(&(g_data.lock), flags);
}

/* Send envent through zrhung_send_event */
void hung_wp_screen_send_work(struct work_struct *work)
{
	unsigned long flags = 0;
	char *msg = NULL;

	hungtask_show_state_filter(TASK_UNINTERRUPTIBLE);
	htmmap_post_process_check(get_system_server_pid());

	msg = g_data.check_id == ZRHUNG_WP_SCREENON ? "screen on block" : "screen off block";
	zrhung_send_event(g_data.check_id, NULL, msg);
	pr_err("%s: send event: %d\n", __func__, g_data.check_id);
	spin_lock_irqsave(&(g_data.lock), flags);
	if (g_data.check_id == ZRHUNG_WP_SCREENON)
		g_data.screen_on_count++;
	else
		g_data.screen_on_count = 0;
	g_data.check_id = ZRHUNG_WP_NONE;
	spin_unlock_irqrestore(&(g_data.lock), flags);

	if (g_data.screen_on_count >= SCREEN_ON_PANIC_COUNT) {
		pr_err("screen_on failed more than %d times!", SCREEN_ON_PANIC_COUNT);
#ifdef CONFIG_QCOM_ARCH
		rb_sreason_set("screen_on_block");
#endif
		panic("screen_on_block");
	}
}

/* Send event when timeout */
#if (KERNEL_VERSION(4, 15, 0) <= LINUX_VERSION_CODE)
static void hung_wp_screen_send(struct timer_list *t)
#else
static void hung_wp_screen_send(uintptr_t pdata)
#endif
{
	del_timer(&g_data.timer);
	pr_err("%s: hung_wp_screen_%d end\n", __func__, g_data.tag_id);
	queue_work(g_data.workq, &g_data.send_work);
}

/* Close screen_off check */
#if (KERNEL_VERSION(4, 15, 0) <= LINUX_VERSION_CODE)
static void hung_wp_screen_close_screen_off_check(struct timer_list *t)
#else
static void hung_wp_screen_close_screen_off_check(uintptr_t pdata)
#endif
{
	del_timer(&g_data.power_off_menu_timer);
	pr_info("%s: power off menu appear, close screen_off check\n", __func__);
	check_off = 0;
}

/* Start up the timer work for screen on delay check */
void hung_wp_screen_start(int check_id)
{
	if (g_data.check_id != ZRHUNG_WP_NONE) {
		pr_err("%s: already in check_id: %d\n", __func__,
			g_data.check_id);
		return;
	}
	g_data.config = (check_id == ZRHUNG_WP_SCREENON) ?
		&on_config : &off_config;
	if (!g_data.config->enable) {
		pr_err("%s: id=%d, enable=%d\n", __func__,
		       check_id, g_data.config->enable);
		return;
	}

	g_data.check_id = check_id;
	if (timer_pending(&g_data.timer))
		del_timer(&g_data.timer);

	g_data.timer.expires = jiffies +
		msecs_to_jiffies(g_data.config->timeout * TIME_CONVERT_UNIT);
	add_timer(&g_data.timer);
	pr_err("%s: going to check ID=%d timeout=%d screen_on_count=%d\n",
	       __func__, check_id, g_data.config->timeout, g_data.screen_on_count);
}

#if (!defined(CONFIG_KEYBOARD_GPIO_VOLUME_KEY)) && (!defined(CONFIG_KEYBOARD_PMIC_VOLUME_KEY))
/* Store the powerkey and volume down key state for key state check */
void *hung_wp_screen_qcom_pkey_press(int type, int state)
{
	static int keys[PON_KEYS] = {0};

	if (type == WP_SCREEN_VDOWN_KEY) {
		vkeys_pressed = PON_RESIN;
		keys[PON_RESIN] = state;
	}
	if (type == WP_SCREEN_POWER_KEY)
		keys[PON_KPDPWR] = state;

	return (void *)keys;
}

static int hung_wp_screen_get_qcom_volup_gpio(void)
{
	static int volup_gpio;
	struct device_node *node = NULL;
	struct device_node *snode = NULL;

	if (volup_gpio > 0)
		return volup_gpio;
	node = of_find_compatible_node(NULL, NULL, "gpio-keys");
	if (node == NULL) {
		volup_gpio = -1;
		pr_err("%s:gpio-keys not found\n", __func__);
		return -ENXIO;
	}
	snode = of_find_node_by_name(node, "vol_up");
	if (snode == NULL) {
		volup_gpio = -1;
		pr_err("%s:vol_up not found\n", __func__);
		return -ENXIO;
	}
	volup_gpio = of_get_named_gpio(snode, "gpios", 0);
	if (!gpio_is_valid(volup_gpio)) {
		volup_gpio = -1;
		pr_err("%s:invalid gpio: %d\n", __func__, volup_gpio);
		return -ENXIO;
	}
	pr_err("%s:volup gpio: %d\n", __func__, volup_gpio);
	return volup_gpio;
}

static int hung_wp_screen_qcom_vkey_get(void)
{
	int volup_gpio = hung_wp_screen_get_qcom_volup_gpio();
	int vol_up_state = (gpio_get_value(volup_gpio) ? 1 : 0) ^
		OF_GPIO_ACTIVE_LOW;
	int *state = (int *)hung_wp_screen_qcom_pkey_press(PON_KEYS, 0);
	int vol_state = *(state + PON_RESIN) | vol_up_state;

	return vol_state;
}

#if (KERNEL_VERSION(4, 15, 0) <= LINUX_VERSION_CODE)
static void hung_wp_screen_qcom_lpress(struct timer_list *t)
{
	int *check_off = check_off_point;
#else
static void hung_wp_screen_qcom_lpress(uintptr_t pdata)
{
	int *check_off = (int *)pdata;
#endif
	unsigned long flags = 0;
#ifndef CONFIG_MTK_ZRHUNG_FEATURE
	int *state = (int *)hung_wp_screen_qcom_pkey_press(PON_KEYS, 0);

	pr_err("%s: get powerkey state:%d\n", __func__, *state);
#endif
	spin_lock_irqsave(&(g_data.lock), flags);
#ifndef CONFIG_MTK_ZRHUNG_FEATURE
	if (*state) {
#endif
		pr_err("%s: check_id=%d, level=%d\n", __func__,
		       g_data.check_id, g_data.bl_level);
		/* don't check screen off when powerkey long pressed */
		*check_off = 0;
#ifndef CONFIG_MTK_ZRHUNG_FEATURE
	}
#endif
	spin_unlock_irqrestore(&(g_data.lock), flags);

	del_timer(&g_data.long_press_timer);
}
#endif

/* Call back from powerkey driver */
void hung_wp_screen_powerkey_ncb(unsigned long event)
{
	int volkeys;
	unsigned int vkeys_pressed_tmp;
	unsigned long flags = 0;

	if (!init_done)
		return;

	if (!config_done) {
		pr_err("%s: config_done=%d, read config first\n",
		       __func__, config_done);
		queue_work(g_data.workq, &g_data.config_work);
		return;
	}
#ifdef CONFIG_KEYBOARD_PMIC_VOLUME_KEY
	volkeys = pmic_gpio_key_vol_updown_press_get();
#elif defined CONFIG_KEYBOARD_GPIO_VOLUME_KEY
	volkeys = gpio_key_vol_updown_press_get();
#elif defined CONFIG_MTK_ZRHUNG_FEATURE
	volkeys = vkeys_temp > 0 ? true : false;
#else
	volkeys = hung_wp_screen_qcom_vkey_get();
#endif

	spin_lock_irqsave(&(g_data.lock), flags);
	if (event == WP_SCREEN_PWK_PRESS ||
		event == WP_SCREEN_PWK_PRESS_QCOM_LAHAINA) {
		pr_err("%s: hung_wp_screen_%d start! bllevel=%d, volkeys=%d\n",
		       __func__, ++g_data.tag_id, g_data.bl_level, volkeys);
		check_off = 0;
		if (g_data.bl_level == 0) {
			hung_wp_screen_start(ZRHUNG_WP_SCREENON);
		} else if (volkeys == 0) {
			hung_wp_screen_clear_vkeys_pressed();
			check_off = 1;
			g_data.power_off_menu_timer.expires = jiffies +
				msecs_to_jiffies(POWER_OFF_MENU_APPEAR_TIMEOUT * TIME_CONVERT_UNIT);
			if (!timer_pending(&g_data.power_off_menu_timer))
				add_timer(&g_data.power_off_menu_timer);
#if (!defined(CONFIG_KEYBOARD_GPIO_VOLUME_KEY)) && (!defined(CONFIG_KEYBOARD_PMIC_VOLUME_KEY))
			pr_info("%s: start longpress test timer\n", __func__);
#if (KERNEL_VERSION(4, 15, 0) <= LINUX_VERSION_CODE)
			check_off_point = &check_off;
#else
			g_data.long_press_timer.data = (unsigned long)&check_off;
#endif
			g_data.long_press_timer.expires = jiffies +
				msecs_to_jiffies(TIME_CONVERT_UNIT);
			if (!timer_pending(&g_data.long_press_timer))
				add_timer(&g_data.long_press_timer);
#endif
		}
		zrhung_powerkeyevent_handler();
	} else if (check_off) {
#if (!defined(CONFIG_KEYBOARD_GPIO_VOLUME_KEY)) && (!defined(CONFIG_KEYBOARD_PMIC_VOLUME_KEY))
		del_timer(&g_data.long_press_timer);
#endif
#ifndef CONFIG_DFX_OHOS
		if (event == WP_SCREEN_PWK_LONGPRESS)
			check_off = 0;
#endif
		if ((event == WP_SCREEN_PWK_RELEASE) && (volkeys == 0)) {
			del_timer(&g_data.power_off_menu_timer);
			vkeys_pressed_tmp = hung_wp_screen_has_vkeys_pressed();
			if (vkeys_pressed_tmp) {
				pr_err("%s: vkeys_pressed_tmp:0x%x\n",
				       __func__, vkeys_pressed_tmp);
				spin_unlock_irqrestore(&(g_data.lock), flags);
				return;
			}
			if (g_data.bl_level != 0)
				hung_wp_screen_start(ZRHUNG_WP_SCREENOFF);
		}
	}
	spin_unlock_irqrestore(&(g_data.lock), flags);
}

#ifdef CONFIG_POWERKEY_SPMI
static int wp_screen_powerkey_notifier(struct notifier_block *powerkey_nb,
				       unsigned long event, void *data)
{
	switch (event) {
	case PRESS_KEY_DOWN:
		hung_wp_screen_powerkey_ncb(WP_SCREEN_PWK_PRESS);
		break;
	case PRESS_KEY_UP:
		hung_wp_screen_powerkey_ncb(WP_SCREEN_PWK_RELEASE);
		break;
#ifndef CONFIG_DFX_OHOS
	case PRESS_KEY_1S:
		hung_wp_screen_powerkey_ncb(WP_SCREEN_PWK_LONGPRESS);
		break;
#endif
#ifdef CONFIG_DFX_OHOS
	case PRESS_KEY_6S:
		queue_work(g_data.workq, &lpressevent_sendwork);
		break;
#endif
	default:
		break;
	}
	return 0;
}
#endif

#ifdef CONFIG_KEYBOARD_PMIC_VOLUME_KEY
static int wp_screen_volumekey_notifier(struct notifier_block *volumekey_nb,
					unsigned long event, void *data)
{
	switch (event) {
	case KEYPAD_PMIC_DOWN_PRESS:
		hung_wp_screen_vkeys_cb(WP_SCREEN_VDOWN_KEY, 1);
		break;
	case KEYPAD_PMIC_DOWN_RELEASE:
		hung_wp_screen_vkeys_cb(WP_SCREEN_VDOWN_KEY, 0);
		break;
	case KEYPAD_PMIC_UP_PRESS:
		hung_wp_screen_vkeys_cb(WP_SCREEN_VUP_KEY, 1);
		break;
	case KEYPAD_PMIC_UP_RELEASE:
		hung_wp_screen_vkeys_cb(WP_SCREEN_VUP_KEY, 0);
		break;
	default:
		break;
	}
	return 0;
}
#endif
/* Configuration init */
static int __init hung_wp_screen_init(void)
{
	init_done = false;
	config_done = false;

	pr_err("%s: +\n", __func__);
	g_data.bl_level = 1;
	g_data.tag_id = 0;
	g_data.screen_on_count = 0;
	g_data.check_id = ZRHUNG_WP_NONE;
	spin_lock_init(&(g_data.lock));
#if (KERNEL_VERSION(4, 15, 0) <= LINUX_VERSION_CODE)
	timer_setup(&g_data.timer, hung_wp_screen_send, 0);
	timer_setup(&g_data.power_off_menu_timer, hung_wp_screen_close_screen_off_check, 0);
#else
	init_timer(&g_data.timer);
	g_data.timer.function = hung_wp_screen_send;
	g_data.timer.data = 0;
	init_timer(&g_data.power_off_menu_timer)
	g_data.power_off_menu_timer.function = hung_wp_screen_close_screen_off_check;
	g_data.power_off_menu_timer.data = 0;
#endif
#if (!defined(CONFIG_KEYBOARD_GPIO_VOLUME_KEY)) && (!defined(CONFIG_KEYBOARD_PMIC_VOLUME_KEY))
#if (KERNEL_VERSION(4, 15, 0) <= LINUX_VERSION_CODE)
	timer_setup(&g_data.long_press_timer, hung_wp_screen_qcom_lpress, 0);
#else
	init_timer(&g_data.long_press_timer);
	g_data.long_press_timer.function = hung_wp_screen_qcom_lpress;
	g_data.long_press_timer.data = 0;
#endif
	hung_wp_screen_get_qcom_volup_gpio();
#endif
	g_data.workq = create_workqueue("hung_wp_screen_workq");
	if (g_data.workq == NULL) {
		pr_err("%s: create workq failed\n", __func__);
		return -ENOMEM;
	}
	INIT_WORK(&g_data.config_work, hung_wp_screen_config_work);
	INIT_WORK(&g_data.send_work, hung_wp_screen_send_work);
	INIT_WORK(&powerkeyevent_config, zrhung_powerkeyevent_config_work);
	INIT_WORK(&powerkeyevent_sendwork, zrhung_powerkeyevent_send_work);
#ifdef CONFIG_DFX_OHOS
	INIT_WORK(&lpressevent_sendwork, zrhung_lpressevent_send_work);
#endif
#ifdef CONFIG_POWERKEY_SPMI
	wp_screen_powerkey_notify.notifier_call = wp_screen_powerkey_notifier;
	powerkey_register_notifier(&wp_screen_powerkey_notify);
#endif
#ifdef CONFIG_KEYBOARD_PMIC_VOLUME_KEY
	wp_screen_volumekey_notify.notifier_call = wp_screen_volumekey_notifier;
	keypad_pmic_register_notifier(&wp_screen_volumekey_notify);
#endif
	init_done = true;
	pr_err("%s: -\n", __func__);
	return 0;
}

module_init(hung_wp_screen_init);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Reporting the frozen screen alarm event");
MODULE_AUTHOR("Huawei Technologies Co., Ltd");
