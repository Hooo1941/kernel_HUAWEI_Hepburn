/*
 * hw_pd.c
 *
 * pd dpm driver
 *
 * Copyright (c) 2016-2020 Huawei Technologies Co., Ltd.
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
#include <linux/console.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fb.h>
#include <linux/hisi/usb/chip_usb.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeup.h>
#include <platform_include/basicplatform/linux/power/platform/coul/coul_drv.h>
#include <platform_include/basicplatform/linux/power/platform/bci_battery.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/usb/class-dual-role.h>
#include <linux/version.h>
#include <linux/err.h>
#include <linux/regulator/consumer.h>
#include <linux/pinctrl/consumer.h>
#include <linux/random.h>
#include <chipset_common/hwpower/hardware_channel/vbus_channel.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_icon.h>
#include <chipset_common/hwpower/charger/charger_common_interface.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_cable.h>
#include <chipset_common/hwpower/buck_charge/buck_charge_ic_manager.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <chipset_common/hwusb/hw_usb.h>
#include <chipset_common/hwpower/protocol/adapter_protocol.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_uvdm.h>

#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/usb/hw_pd_dev.h>
#include <huawei_platform/usb/hw_pogopin.h>
#include <huawei_platform/usb/pd/richtek/tcpm.h>
#include <huawei_platform/usb/switch/switch_fsa9685.h>
#include <huawei_platform/usb/usb_extra_modem.h>
#include "securec.h"

#ifdef CONFIG_BOOST_5V
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#endif
#ifdef CONFIG_CONTEXTHUB_PD
#include <linux/hisi/usb/tca.h>
#endif
#ifdef CONFIG_WIRELESS_CHARGER
#include <huawei_platform/power/wireless/wireless_charger.h>
#endif

#ifdef CONFIG_HUAWEI_HISHOW
#include <huawei_platform/usb/hw_hishow.h>
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#include <devkit/audiokit/ana_hs_kit/ana_hs.h>
#else
#include "ana_hs_kit/ana_hs.h"
#endif
#include "huawei_platform/audio/usb_analog_hs_interface.h"
#include "huawei_platform/audio/usb_audio_power.h"
#include "huawei_platform/audio/usb_audio_power_v600.h"
#include "huawei_platform/audio/dig_hs.h"
#include "huawei_platform/audio/ana_hs_extern_ops.h"
#include "huawei_platform/dp_aux_switch/dp_aux_switch.h"
#include "huawei_platform/usb/switch/usbswitch_common.h"


#define CC_SHORT_DEBOUNCE 50 /* ms */
#define TIME_T_MAX    (time64_t)((1UL << ((sizeof(time64_t) << 3) - 1)) - 1)

#ifdef CONFIG_CONTEXTHUB_PD
#define pd_dpm_wait_combphy_configdone() \
		wait_for_completion_timeout(&pd_dpm_combphy_configdone_completion, msecs_to_jiffies(11500)); \
		reinit_completion(&pd_dpm_combphy_configdone_completion)
#endif /* CONFIG_CONTEXTHUB_PD */
struct pd_dpm_info *g_pd_di;
static bool g_pd_cc_orientation;
static int g_pd_cc_orientation_factory;
#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
static bool g_pd_smart_holder;
int support_smart_holder;
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC2 */
static bool g_vbus_only_status;
static struct device *typec_dev;
static int pd_dpm_typec_state = PD_DPM_USB_TYPEC_DETACHED;
static int pd_dpm_analog_hs_state;
static struct pd_dpm_vbus_state g_vbus_state;
static unsigned long g_charger_type_event;
struct completion pd_get_typec_state_completion;
#ifdef CONFIG_CONTEXTHUB_PD
static int g_ack;
struct completion pd_dpm_combphy_configdone_completion;
#endif /* CONFIG_CONTEXTHUB_PD */
static bool g_pd_high_power_charging_status;
static bool g_pd_high_voltage_charging_status;
static bool g_pd_optional_max_power_status;
static bool g_pd_optional_wireless_cover_status;
struct cc_check_ops* g_cc_check_ops;
static bool ignore_vbus_on_event;
static bool ignore_vbus_off_event;
static bool ignore_bc12_event_when_vbuson;
static bool ignore_bc12_event_when_vbusoff;
static enum pd_dpm_cc_voltage_type remote_rp_level = PD_DPM_CC_VOLT_OPEN;
static struct pd_dpm_ops *g_ops;
#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
static struct cable_vdo_ops *g_cable_vdo_ops = NULL;
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC2 */
static void *g_client;
#ifdef CONFIG_CONTEXTHUB_PD
static bool g_last_hpd_status;
static TCPC_MUX_CTRL_TYPE g_combphy_mode = TCPC_NC;
bool g_pd_combophy_ready;
#endif /* CONFIG_CONTEXTHUB_PD */
static int switch_manual_enable = 1;
static unsigned int abnormal_cc_detection;
static unsigned int abnormal_cc_interval = PD_DPM_CC_CHANGE_INTERVAL;
int support_dp = 1;
static int g_otg_channel;
int moisture_detection_by_cc_enable;
static unsigned int g_cc_abnormal_dmd_report_enable;
int support_analog_audio = 1;
static int analog_audio_status = 0;
struct mutex typec_state_lock;
struct mutex typec_wait_lock;
static int g_pd_product_type = -1;
static bool g_pd_cc_moisture_status;
static bool g_pd_cc_moisture_happened;
static int cc_moisture_status_report;
static int external_pd_flag = 0;
static int emark_detect_enable;
static int emark_detect_finish_not_support;
static bool g_audio_power_no_hs_flag;
static int g_ibus_check;
static int g_vbus_only_min_voltage;
/* 0: disabled 1: only for SC; 2: for SC and LVC */
static unsigned int cc_dynamic_protect;
static struct delayed_work cc_short_work;
#ifdef CONFIG_UVDM_CHARGER
static struct work_struct g_uvdm_work;
static unsigned int g_uvdm_work_cnt;
#endif /* CONFIG_UVDM_CHARGER */
struct pd_dpm_typec_state *g_typec_state;
struct water_det_ops* g_water_det_ops;
/* flag that determines whether 9v2a can be requested after the SNK_FLOW_DELAY timer */
static bool g_pd_hv_en = true;

void reinit_typec_completion(void);
void typec_complete(enum pd_wait_typec_complete typec_completion);
void pd_dpm_set_cc_mode(int mode);
void pd_dpm_reinit_chip(void);

#define PD_DPM_MAX_OCP_COUNT            1000
#define OCP_DELAY_TIME                  5000
#define DISABLE_INTERVAL                50
#define GET_IBUS_INTERVAL               1000
#define MMI_PD_TIMES                    3
#define MMI_PD_IBUS_MIN                 300
#define POGOPIN_OTG_CHANNEL             1

#define PD_DPM_POWER_QUICK_CHG_THR      18000000
#define PD_DPM_POWER_WIRELESS_CHG_THR   9000000

/* define for cable certification */
#define EVENT_BUF_LEN                   128
#define UVDM_DATA_LEN_YC                7
#define UVDM_DATA_LEN_JHT               5
#define UVDM_DATA_RANDOM                4
#define UVDM_RETRY_TIME                 2
#define PD_DPM_DATA_RANDOM              5
#define PD_DPM_CABLE_HASH               6
#define PD_DPM_PRE_AUTH_DATA_LEN        7
#define UVDM_DATA_HEAD_JHT              0x12d10104    /* header for jht cable */
#define UVDM_DATA_HEAD                  0x12d12102    /* header for other cable */
#define RAND_LENGTH                     20
#define HASH_LENGTH_MAX                 24
#define PD_DPM_CABLE_JHT                0xef
#define PD_DPM_CABLE_NXT_JHT            0xca
#define PD_DPM_CABLE_MASK_DEC           0x1FFF        /* mask for dec data */
#define PD_DPM_CABLE_MASK_LAT           0xF           /* mask for latency data */

#define SRC_VBUS_DEFAULT_VOL            5000
#define SRC_VBUS_DEFAULT_CUR            1500
#define SRC_VBUS_HIGH_VOL               9000

struct pd_dpm_cable_state {
	int cable_fail;                                   /* cable authentication fail */
	int succ_flag;                                    /* cable authentication succ flag */
	int support_cable;                                /* cable authentication is supported */
	unsigned int cable_type;                          /* cable manufacturer */
	unsigned int value[PD_DPM_DATA_RANDOM];           /* random value */
	unsigned int cable_hash[PD_DPM_CABLE_HASH];       /* hash value */
	struct completion emark_detect_cable_data;
	struct completion emark_get_key_value;
	struct pd_dpm_cable_info cable_auth;
};
struct pd_dpm_cable_state g_cable_state;

struct pd_pdm_cable_6a_auth {
	unsigned int pdo_dec;                             /* product vdo dec data */
	unsigned int cdo_lat;                             /* cable vdo latency data */
};

struct pd_pdm_cable_6a_auth  g_cable_pre[PD_DPM_PRE_AUTH_DATA_LEN] = {
	{0x1194, 0x1},
	{0x0963, 0x1},
	{0x0022, 0x2},
	{0x09c8, 0x1},
	{0x1178, 0x2},
	{0x157B, 0x0},
	{0x09d9, 0x1},
};

#ifndef HWLOG_TAG
#define HWLOG_TAG huawei_pd
HWLOG_REGIST();
#endif /* HWLOG_TAG */

#ifdef CONFIG_CONTEXTHUB_PD
extern void dp_aux_uart_switch_disable(void);
#endif /* CONFIG_CONTEXTHUB_PD */
static bool g_ignore_vbus_only_event;
int g_cur_usb_event = PD_DPM_USB_TYPEC_NONE;
static enum power_supply_sinksource_type sink_source_type = POWER_SUPPLY_STOP_SINK;

static struct abnomal_change_info abnomal_change[] = {
	{PD_DPM_ABNORMAL_CC_CHANGE, true, 0, 0, {0}, {0}, {0}, {0}},
	{PD_DPM_UNATTACHED_VBUS_ONLY, true, 0, 0, {0}, {0}, {0}, {0}},
};

static inline unsigned int pd_dpm_swap_endpoint(unsigned int data)
{
	/* This function is used to adjust the Big-Endian and Little-Endian
	   8 and 24 is a bit required for left or right shift */
	data = (data >> 24) | ((data & 0x00FF0000) >> 8) |
		((data & 0x0000FF00) << 8) | (data << 24);
	return data;
}

void pd_dpm_set_emark_detect_enable(int flag)
{
	emark_detect_enable = flag;
}

int pd_dpm_get_emark_detect_enable(void)
{
	return emark_detect_enable;
}

static void pd_dpm_cc_moisture_flag_restore(struct work_struct *work)
{
	hwlog_err("%s %d,%d\n", __func__,
		  g_pd_cc_moisture_happened,
		  g_pd_cc_moisture_status);
	if (!g_pd_cc_moisture_happened)
		g_pd_cc_moisture_status = false;
}

static void fb_unblank_work(struct work_struct *work)
{
	unsigned int flag = WD_NON_STBY_DRY;

	hwlog_err("%s set pd to drp\n", __func__);
	pd_dpm_set_cc_mode(PD_DPM_CC_MODE_DRP);
	power_event_bnc_notify(POWER_BNT_WD, POWER_NE_WD_REPORT_UEVENT, &flag);

	if (cc_moisture_status_report == 0)
		return;

	g_pd_cc_moisture_happened = false;
	/* 120000: delay 2 minute to restore the flag */
	queue_delayed_work(g_pd_di->usb_wq,
		&g_pd_di->cc_moisture_flag_restore_work,
		msecs_to_jiffies(120000));
}

static int pd_dpm_handle_fb_event(struct notifier_block *self,
	unsigned long event, void *data)
{
	struct fb_event *fb_event = data;
	int *blank = NULL;

	if (!g_pd_di || !fb_event)
		return NOTIFY_DONE;

	blank = fb_event->data;
	if (!blank)
		return NOTIFY_DONE;

	if ((event == FB_EVENT_BLANK) && (*blank == FB_BLANK_UNBLANK))
		schedule_work(&g_pd_di->fb_work);

	return NOTIFY_DONE;
}

static struct notifier_block pd_dpm_handle_fb_notifier = {
	.notifier_call = pd_dpm_handle_fb_event,
};

static void init_fb_notification(void)
{
	fb_register_client(&pd_dpm_handle_fb_notifier);
}

void pd_dpm_set_source_sink_state(enum power_supply_sinksource_type type)
{
	hwlog_info("%s %d\n", __func__, (int)type);
	sink_source_type = type;
	power_supply_set_int_property_value("charger_detection",
		POWER_SUPPLY_PROP_CHG_PLUGIN, type);
}

enum power_supply_sinksource_type pd_dpm_get_source_sink_state(void)
{
	return sink_source_type;
}
int pd_dpm_ops_register(struct pd_dpm_ops *ops, void *client)
{
	int ret = 0;

	/*
	 * VXXX tcpc dirver lauchs earlier than rt1715 driver(external pd)
	 * and it shall call this function. Overwirte the callback handle
	 * while rt1715 driver calling this
	 */
	if (g_ops && !external_pd_flag) {
		hwlog_err("pd_dpm ops register fail! g_ops busy\n");
		return -EBUSY;
	}

	if (ops) {
		g_ops = ops;
		g_client = client;
	} else {
		hwlog_err("pd_dpm ops register fail\n");
		ret = -EPERM;
	}
	return ret;
}

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
int pd_dpm_cable_vdo_ops_register(struct cable_vdo_ops *ops)
{
	if (!ops) {
		hwlog_err("cable_vdo_ops ops register fail\n");
		return -EPERM;
	}

	g_cable_vdo_ops = ops;
	return 0;
}
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC2 */

void pd_dpm_start_data_role_swap(void)
{
	if (!g_ops || !g_ops->data_role_swap || !g_client)
		return;

	g_ops->data_role_swap(g_client);
}

void pd_dpm_detect_emark_cable(void)
{
	hwlog_err("%s\n", __func__);

	if (g_ops && g_ops->pd_dpm_detect_emark_cable)
		g_ops->pd_dpm_detect_emark_cable(g_client);
}

void pd_dpm_detect_emark_cable_finish(void)
{
	int typec_state = PD_DPM_USB_TYPEC_HOST_ATTACHED;

	if (emark_detect_finish_not_support)
		return;
	pd_dpm_get_typec_state(&typec_state);
	if (typec_state != PD_DPM_USB_TYPEC_HOST_ATTACHED)
		return;
	if (!g_ops || !g_ops->data_role_swap || !g_client || !g_ops->pd_dpm_inquire_usb_comm)
		return;
	if (g_ops->pd_dpm_inquire_usb_comm(g_client) && !g_ops->data_role_swap(g_client)) {
		hwlog_info("emark finish data_role_swap\n");
		msleep(50); /* wait for swap complete */
	}
}

int pd_dpm_disable_pd(bool disable)
{
	hwlog_info("%s\n", __func__);

	if (uem_check_online_status())
		return -EPERM;

	if (!g_ops) {
		hwlog_err("%s g_ops is NULL\n", __func__);
		return -EPERM;
	}

	if (!g_ops->pd_dpm_disable_pd) {
		hwlog_err("%s pd_dpm_disable_pd is NULL\n", __func__);
		return -EPERM;
	}

	return g_ops->pd_dpm_disable_pd(g_client, disable);
}

void pd_dpm_set_cc_mode(int mode)
{
	static int cur_mode = PD_DPM_CC_MODE_DRP;

	hwlog_info("%s cur_mode = %d, new mode = %d\n",
		__func__, cur_mode, mode);
	if (!g_ops || !g_ops->pd_dpm_set_cc_mode || (cur_mode == mode))
		return;

	g_ops->pd_dpm_set_cc_mode(mode);
	cur_mode = mode;
}

/*
 * bugfix: For Hi65xx
 * DRP lost Cable(without adapter) plugin during Wireless.
 */
void pd_pdm_enable_drp(void)
{
	if (g_ops && g_ops->pd_dpm_enable_drp)
		g_ops->pd_dpm_enable_drp(PD_PDM_RE_ENABLE_DRP);
}

void pd_dpm_reinit_chip(void)
{
	if (g_ops && g_ops->pd_dpm_reinit_chip)
		g_ops->pd_dpm_reinit_chip(g_client);
}

bool pd_dpm_get_hw_dock_svid_exist(void)
{
	if (g_ops && g_ops->pd_dpm_get_hw_dock_svid_exist)
		return g_ops->pd_dpm_get_hw_dock_svid_exist(g_client);

	return false;
}

int pd_dpm_notify_direct_charge_status(bool dc)
{
	hwlog_err("%s,%d", __func__, __LINE__);
	if (g_ops && g_ops->pd_dpm_notify_direct_charge_status) {
		hwlog_err("%s,%d", __func__, __LINE__);
		return g_ops->pd_dpm_notify_direct_charge_status(g_client, dc);
	}

	return false;
}

static bool pd_dpm_get_audio_power_no_hs_state(void)
{
	return g_audio_power_no_hs_flag;
}

void pd_dpm_set_audio_power_no_hs_state(bool flag)
{
	g_audio_power_no_hs_flag = flag;
}

void pd_dpm_set_cc_voltage(int type)
{
	remote_rp_level = type;
}

enum pd_dpm_cc_voltage_type pd_dpm_get_cc_voltage(void)
{
	return remote_rp_level;
}

#ifdef CONFIG_CONTEXTHUB_PD
void pd_dpm_set_combophy_ready_status(void)
{
	g_pd_combophy_ready = true;
}

bool pd_dpm_get_combophy_ready_status(void)
{
	return g_pd_combophy_ready;
}

void pd_dpm_set_combphy_status(TCPC_MUX_CTRL_TYPE mode)
{
	g_combphy_mode = mode;
}

TCPC_MUX_CTRL_TYPE pd_dpm_get_combphy_status(void)
{
	return g_combphy_mode;
}

void pd_dpm_set_last_hpd_status(bool hpd_status)
{
	g_last_hpd_status = hpd_status;
}

bool pd_dpm_get_last_hpd_status(void)
{
	return g_last_hpd_status;
}
#endif /* CONFIG_CONTEXTHUB_PD */

bool pd_dpm_ignore_vbuson_event(void)
{
	return ignore_vbus_on_event;
}

bool pd_dpm_ignore_vbusoff_event(void)
{
	return ignore_vbus_off_event;
}

void pd_dpm_set_ignore_vbuson_event(bool ignore)
{
	ignore_vbus_on_event = ignore;
}

void pd_dpm_set_ignore_vbusoff_event(bool ignore)
{
	ignore_vbus_off_event = ignore;
}

bool pd_dpm_ignore_bc12_event_when_vbuson(void)
{
	return ignore_bc12_event_when_vbuson;
}

bool pd_dpm_ignore_bc12_event_when_vbusoff(void)
{
	return ignore_bc12_event_when_vbusoff;
}

void pd_dpm_set_ignore_bc12_event_when_vbuson(bool _ignore_bc12_event)
{
	ignore_bc12_event_when_vbuson = _ignore_bc12_event;
}

void pd_dpm_set_ignore_bc12_event_when_vbusoff(bool _ignore_bc12_event)
{
	ignore_bc12_event_when_vbusoff = _ignore_bc12_event;
}

bool pd_dpm_get_high_power_charging_status()
{
	hwlog_info("%s status =%d\n", __func__, g_pd_high_power_charging_status);
	return g_pd_high_power_charging_status;
}

void pd_dpm_set_high_power_charging_status(bool status)
{
	hwlog_info("%s status =%d\n", __func__, status);
	g_pd_high_power_charging_status = status;
}

bool pd_dpm_get_high_voltage_charging_status()
{
	hwlog_info("%s status =%d\n", __func__, g_pd_high_voltage_charging_status);
	return g_pd_high_voltage_charging_status;
}

void pd_dpm_set_high_voltage_charging_status(bool status)
{
	hwlog_info("%s status =%d\n", __func__, status);
	g_pd_high_voltage_charging_status = status;
}

bool pd_dpm_get_optional_max_power_status()
{
	hwlog_info("%s status =%d\n", __func__, g_pd_optional_max_power_status);
	return g_pd_optional_max_power_status;
}

void pd_dpm_set_optional_max_power_status(bool status)
{
	hwlog_info("%s status =%d\n", __func__, status);
	g_pd_optional_max_power_status = status;
}

static bool pd_dpm_get_wireless_cover_power_status(void)
{
	hwlog_info("%s status =%d\n", __func__,
		g_pd_optional_wireless_cover_status);
	return g_pd_optional_wireless_cover_status;
}

static void pd_dpm_set_wireless_cover_power_status(bool status)
{
	hwlog_info("%s status =%d\n", __func__, status);
	g_pd_optional_wireless_cover_status = status;
}

void pd_dpm_get_charge_event(unsigned long *event, struct pd_dpm_vbus_state *state)
{
	hwlog_info("%s event =%ld\n", __func__, g_charger_type_event);

	*event = g_charger_type_event;
	memcpy(state, &g_vbus_state, sizeof(struct pd_dpm_vbus_state));
}

static void pd_dpm_set_charge_event(unsigned long event,
	struct pd_dpm_vbus_state *state)
{
	hwlog_info("%s event =%ld\n", __func__, event);

	if (state)
		memcpy(&g_vbus_state, state, sizeof(struct pd_dpm_vbus_state));
	g_charger_type_event = event;
}

int cc_check_ops_register(struct cc_check_ops* ops)
{
	int ret = 0;

	if (g_cc_check_ops) {
		hwlog_err("cc_check ops register fail! g_cc_check_ops busy\n");
		return -EBUSY;
	}

	if (ops) {
		g_cc_check_ops = ops;
	} else {
		hwlog_err("cc_check ops register fail\n");
		ret = -EPERM;
	}
	return ret;
}

static int direct_charge_cable_detect(void)
{
	int ret;

	if (!g_cc_check_ops)
		return -1;
	ret = g_cc_check_ops->is_cable_for_direct_charge();
	if (ret) {
		hwlog_info("%s:cc_check fail\n", __func__);
		return -1;
	}
	return 0;
}

int water_det_ops_register(struct water_det_ops* ops)
{
	int ret = 0;

	if (g_water_det_ops) {
		hwlog_err("water_det_ops register fail! g_water_det_ops busy\n");
		return -EBUSY;
	}

	if (ops) {
		g_water_det_ops = ops;
	} else {
		hwlog_err("water_det_ops register fail\n");
		ret = -EPERM;
	}
	return ret;
}

static int pd_dpm_cable_err(void)
{
	hwlog_info("%s:succ_flag is %d\n", __func__, g_cable_state.succ_flag);
	return 	g_cable_state.succ_flag;
}

static struct dc_cable_ops cable_detect_ops = {
	.detect = direct_charge_cable_detect,
	.cable_auth_det = pd_dpm_cable_err,
};

bool pd_dpm_get_cc_orientation(void)
{
	hwlog_info("%s cc_orientation =%d\n", __func__, g_pd_cc_orientation);
	return g_pd_cc_orientation;
}

static void pd_dpm_set_cc_orientation(bool cc_orientation)
{
	hwlog_info("%s cc_orientation =%d\n", __func__, cc_orientation);
	g_pd_cc_orientation = cc_orientation;
}

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
static bool pd_dpm_is_smart_holder(void)
{
	return g_pd_smart_holder;
}
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC2 */

static bool pd_dpm_vbus_only_status(void)
{
	return g_vbus_only_status;
}

void pd_dpm_get_typec_state(int *typec_state)
{
	hwlog_info("%s pd_dpm_get_typec_state = %d\n", __func__, pd_dpm_typec_state);

	*typec_state = pd_dpm_typec_state;

	return;
}
/* for analog audio drivers plooing */
int pd_dpm_get_analog_hs_state(void)
{
	hwlog_info("%s analog_hs_state  = %d\n", __func__, pd_dpm_analog_hs_state);

	return pd_dpm_analog_hs_state;
}

void pd_dpm_set_typec_state(int typec_state)
{
	hwlog_info("%s pd_dpm_set_typec_state  = %d\n", __func__, typec_state);
	blocking_notifier_call_chain(&g_pd_di->pd_port_status_nh, typec_state, NULL);
	pd_dpm_typec_state = typec_state;

	if ((pd_dpm_typec_state == PD_DPM_USB_TYPEC_NONE) ||
		(pd_dpm_typec_state == PD_DPM_USB_TYPEC_DETACHED) ||
		(pd_dpm_typec_state == PD_DPM_USB_TYPEC_AUDIO_DETACHED)) {
		hwlog_info("%s report detach, stop ovp & start res detect\n", __func__);
		ana_hs_fsa4480_stop_ovp_detect(ANA_HS_TYPEC_DEVICE_DETACHED);
		ana_hs_fsa4480_start_res_detect(ANA_HS_TYPEC_DEVICE_DETACHED);
	} else {
		hwlog_info("%s report attach, stop res & satrt ovp detect\n", __func__);
		ana_hs_fsa4480_stop_res_detect(ANA_HS_TYPEC_DEVICE_ATTACHED);
		ana_hs_fsa4480_start_ovp_detect(ANA_HS_TYPEC_DEVICE_ATTACHED);
	}

	if (pogopin_is_support())
		pogopin_set_typec_state(pd_dpm_typec_state);
}

int pd_get_product_type(void)
{
	hwlog_info("%s product type %d\n", __func__, g_pd_product_type);
	return g_pd_product_type;
}

void pd_set_product_type(int type)
{
	g_pd_product_type = type;
}

bool pd_judge_is_cover(void)
{
	if ((pd_get_product_type() == PD_PDT_WIRELESS_COVER) ||
		(pd_get_product_type() == PD_PDT_WIRELESS_COVER_TWO))
		return true;

	return false;
}

#ifdef CONFIG_UVDM_CHARGER
static void uvdm_send_work(struct work_struct *work)
{
	int type = -1;

	g_uvdm_work_cnt++;
	/* g_uvdm_work cycle not more than 30 times */
	if (g_uvdm_work_cnt > 30)
		return;

	adapter_get_adp_type(ADAPTER_PROTOCOL_UVDM, &type);
	switch (type) {
	case UVDM_CAHRGE_TYPE_5W:
	case UVDM_CAHRGE_TYPE_10W:
	case UVDM_CHARGE_TYPE_20W:
		uvdm_check(type);
		break;
	default:
		schedule_work(&g_uvdm_work);
		break;
	}
}

static inline void uvdm_init_work(void)
{
	INIT_WORK(&g_uvdm_work, uvdm_send_work);
}

static inline void uvdm_cancel_work(void)
{
	cancel_work_sync(&g_uvdm_work);
}

void uvdm_schedule_work(void)
{
	g_uvdm_work_cnt = 0;
	schedule_work(&g_uvdm_work);
}
#else
static inline void uvdm_send_work(struct work_struct *work)
{
}

static inline void uvdm_init_work(void)
{
}

static inline void uvdm_cancel_work(void)
{
}
#endif /* CONFIG_UVDM_CHARGER */

void pd_dpm_set_max_power(int max_power)
{
	int icon_type;

	if (pogopin_typec_chg_ana_audio_support() &&
		(pogopin_5pin_get_pogo_status() == POGO_CHARGER)) {
		hwlog_info("pogo charging, ignore max power notify\n");
		return;
	}

	hwlog_info("%s max_power=%d\n", __func__, max_power);
	/* wireless cover */
	if ((pd_get_product_type() == PD_PDT_WIRELESS_COVER) ||
		(pd_get_product_type() == PD_PDT_WIRELESS_COVER_TWO)) {
		if (max_power >= PD_DPM_POWER_WIRELESS_CHG_THR) {
			icon_type = ICON_TYPE_WIRELESS_QUICK;
			power_ui_event_notify(POWER_UI_NE_ICON_TYPE, &icon_type);
		}
		hwlog_info("%s wireless cover is on\n", __func__);
		return;
	}

	if (max_power >= PD_DPM_POWER_WIRELESS_CHG_THR)
		pd_dpm_set_wireless_cover_power_status(true);
	else
		pd_dpm_set_wireless_cover_power_status(false);

	/* wired */
	if (max_power >= PD_DPM_POWER_QUICK_CHG_THR) {
		pd_dpm_set_optional_max_power_status(true);
		if (power_icon_inquire() != ICON_TYPE_SUPER) {
			icon_type = ICON_TYPE_QUICK;
			power_ui_event_notify(POWER_UI_NE_ICON_TYPE, &icon_type);
		}
	} else {
		pd_dpm_set_optional_max_power_status(false);
	}
}

static void pd_send_wireless_cover_uevent(void)
{
	int cover_status = 1; /* wireless cover is ready */
	int icon_type;

	power_ui_event_notify(POWER_UI_NE_WL_COVER_STATUS, &cover_status);

	if (pd_dpm_get_wireless_cover_power_status())
		icon_type = ICON_TYPE_WIRELESS_QUICK;
	else
		icon_type = ICON_TYPE_WIRELESS_NORMAL;

	power_ui_event_notify(POWER_UI_NE_ICON_TYPE, &icon_type);
}

void pd_set_product_id_info(unsigned int vid,
			    unsigned int pid,
			    unsigned int bcd)
{
	int pd_product_type;

	uem_set_product_id_info(vid, pid);

	if (bcd == PD_PID_COVER_ONE)
		pd_product_type = PD_PDT_WIRELESS_COVER;
	else if (bcd == PD_PID_COVER_TWO)
		pd_product_type = PD_PDT_WIRELESS_COVER_TWO;
	else
		return;

	pd_set_product_type(pd_product_type);
	hwlog_info("%s vid = 0x%x, pid = 0x%x, type = 0x%x, bcd = 0x%x\n",
		__func__, vid, pid, pd_product_type, bcd);

	switch (pd_product_type) {
	case PD_PDT_WIRELESS_COVER:
		if ((vid == PD_DPM_HW_VID) && (pid == PD_DPM_HW_CHARGER_PID)) {
			coul_drv_charger_event_rcv(WIRELESS_COVER_DETECTED);
			pd_send_wireless_cover_uevent();
		}
		break;
	case PD_PDT_WIRELESS_COVER_TWO:
		if ((vid == PD_DPM_HW_VID) && (pid == PD_DPM_HW_CHARGER_PID)) {
			coul_drv_charger_event_rcv(WIRELESS_COVER_DETECTED);
			pd_send_wireless_cover_uevent();
		}
		uvdm_schedule_work();
		break;
	default:
		hwlog_err("undefined type %d\n", pd_product_type);
		break;
	}
}

/*
 * Function:       hw_pd_set_hv_en
 * Description:    enable/disable 9v2a rdo for 3rd-party pd
 *                 When a ufcs adapter is detected, the huawei_charger disables 9v2a rdo
 * Input:          hv_en (true:enable 9v2a rdo; false:disable 9v2a rdo)
 * Output:         none
 * Return:         none
*/
void hw_pd_set_hv_en(bool hv_en)
{
	g_pd_hv_en = hv_en;
}

bool pd_dpm_support_voltage_up(void)
{
	if (!g_pd_hv_en)
		return false;
	return true;
}

static ssize_t pd_dpm_cc_orientation_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	/* 3 means cc is abnormally grounded */
	if (g_pd_cc_orientation_factory == CC_ORIENTATION_FACTORY_SET)
		return scnprintf(buf, PAGE_SIZE, "%s\n", "3");

	return scnprintf(buf, PAGE_SIZE, "%s\n",
		pd_dpm_get_cc_orientation() ? "2" : "1");
}

static ssize_t pd_dpm_pd_state_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int retrys = 0;
	int cur;

	hwlog_info("%s = %d\n", __func__, pd_dpm_get_pd_finish_flag());
	if (pd_dpm_get_pd_finish_flag()) {
		if (!g_ibus_check)
			return scnprintf(buf, PAGE_SIZE, "%s\n",
				pd_dpm_get_pd_finish_flag() ? "0" : "1");
		while (retrys++ < MMI_PD_TIMES) {
			cur = charge_get_ibus();
			hwlog_info("%s, ibus = %d\n", __func__, cur);
			if (cur >= MMI_PD_IBUS_MIN)
				return scnprintf(buf, PAGE_SIZE, "%s\n", "0");

			msleep(GET_IBUS_INTERVAL);
		}
	}
	return scnprintf(buf, PAGE_SIZE, "%s\n", "1");
}

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
static ssize_t pd_dpm_smart_holder_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	hwlog_info("%s  = %d\n", __func__, pd_dpm_is_smart_holder());
	return scnprintf(buf, PAGE_SIZE, "%s\n",
		pd_dpm_is_smart_holder() ? "1" : "0");
}
#endif
static ssize_t pd_dpm_cc_state_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	unsigned int cc1 = 0;
	unsigned int cc2 = 0;
	unsigned int cc = 0;

	ret = pd_dpm_get_cc_state_type(&cc1, &cc2);
	if (ret == 0)
		cc = ((cc1 & PD_DPM_CC_STATUS_MASK) |
			(cc2 << PD_DPM_CC2_STATUS_OFFSET)) &
			PD_DPM_BOTH_CC_STATUS_MASK;

	return scnprintf(buf, PAGE_SIZE, "%d\n", cc);
}

static ssize_t pd_dpm_vbus_only_status_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	hwlog_info("%s  = %d\n", __func__, pd_dpm_vbus_only_status());
	return scnprintf(buf, PAGE_SIZE, "%s\n",
		pd_dpm_vbus_only_status() ? "1" : "0");
}

static ssize_t pd_dpm_succ_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	hwlog_info("%s usb_port store the key value", __func__);
	long val = 0;

	if (kstrtol(buf, UEM_BASE_DEC, &val) < 0) {
		hwlog_err("usb_power control val invalid\n");
		return -EINVAL;
	}

	g_cable_state.succ_flag = (int)val;
	complete(&g_cable_state.emark_get_key_value);
	return count;
}

static DEVICE_ATTR(cc_orientation, S_IRUGO, pd_dpm_cc_orientation_show, NULL);
static DEVICE_ATTR(pd_state, S_IRUGO, pd_dpm_pd_state_show, NULL);
#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
static DEVICE_ATTR(smart_holder, S_IRUGO, pd_dpm_smart_holder_show, NULL);
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC2 */
static DEVICE_ATTR(cc_state, S_IRUGO, pd_dpm_cc_state_show, NULL);
static DEVICE_ATTR(vbus_only_status, S_IRUGO, pd_dpm_vbus_only_status_show, NULL);
static DEVICE_ATTR(succ_value, S_IRWXU | S_IRWXG | S_IROTH, NULL, pd_dpm_succ_store);
static struct attribute *pd_dpm_ctrl_attributes[] = {
	&dev_attr_cc_orientation.attr,
	&dev_attr_pd_state.attr,
#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
	&dev_attr_smart_holder.attr,
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC2 */
	&dev_attr_cc_state.attr,
	&dev_attr_vbus_only_status.attr,
	&dev_attr_succ_value.attr,
	NULL,
};

static const struct attribute_group pd_dpm_attr_group = {
	.attrs = pd_dpm_ctrl_attributes,
};

int pd_dpm_wake_unlock_notifier_call(struct pd_dpm_info *di, unsigned long event, void *data)
{
	return atomic_notifier_call_chain(&di->pd_wake_unlock_evt_nh, event, data);
}

int pd_dpm_vbus_notifier_call(struct pd_dpm_info *di, unsigned long event, void *data)
{
	if (event == CHARGER_TYPE_NONE) {
		pd_dpm_set_high_power_charging_status(false);
		pd_dpm_set_optional_max_power_status(false);
		pd_dpm_set_wireless_cover_power_status(false);
		pd_dpm_set_high_voltage_charging_status(false);
	}
	if (event != PD_DPM_VBUS_TYPE_TYPEC)
		pd_dpm_set_charge_event(event, data);
	return blocking_notifier_call_chain(&di->pd_evt_nh, event, data);
}
static int charge_wake_unlock_notifier_call(struct notifier_block *chrg_wake_unlock_nb,
	unsigned long event, void *data)
{
	switch (event) {
	case POWER_NE_CHG_WAKE_UNLOCK:
		if (g_pd_di)
			pd_dpm_wake_unlock_notifier_call(g_pd_di, PD_WAKE_UNLOCK, NULL);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

bool pd_dpm_get_ctc_cable_flag(void)
{
	if (g_pd_di)
		return g_pd_di->ctc_cable_flag;

	return false;
}

bool pd_dpm_get_pd_finish_flag(void)
{
	if (g_pd_di)
		return g_pd_di->pd_finish_flag;
	else
		return false;
}

void pd_dpm_set_pd_finish_flag(bool flag)
{
	if (g_pd_di)
		g_pd_di->pd_finish_flag = flag;
}

bool pd_dpm_get_pd_source_vbus(void)
{
	if (g_pd_di)
		return g_pd_di->pd_source_vbus;
	else
		return false;
}

bool pd_dpm_check_cc_vbus_short(void)
{
	int ret;
	unsigned int cc1 = 0;
	unsigned int cc2 = 0;

	/*
	 * Get result from chip module directly,
	 * Only for HISI_PD at present.
	 */
	if (g_ops && g_ops->pd_dpm_check_cc_vbus_short) {
		hwlog_info("Hisi PD\n");
		return g_ops->pd_dpm_check_cc_vbus_short();
	}

	ret = pd_dpm_get_cc_state_type(&cc1, &cc2);
	if (ret)
		return false;

	hwlog_info("%s: cc1:%d, cc2:%d\n", __func__, cc1, cc2);

	if ((cc1 == PD_DPM_CC_3_0) && (cc2 > PD_DPM_CC_OPEN))
		return true;

	if ((cc2 == PD_DPM_CC_3_0) && (cc1 > PD_DPM_CC_OPEN))
		return true;

	return false;
}

/*
 * Function:       pd_dpm_get_cc_state_type
 * Description:   get cc1 and cc2 state
 *                             open    56k    22k    10k
 *                     cc1    00       01     10     11
 *                     cc2    00       01     10     11
 * Input:           cc1: value of cc1  cc2: value of cc2
 * Output:         cc1: value of cc1  cc2: value of cc2
 * Return:         success: 0   fail: -1
*/
int pd_dpm_get_cc_state_type(unsigned int *cc1, unsigned int *cc2)
{
	unsigned int cc;

	if (!g_ops || !g_ops->pd_dpm_get_cc_state)
		return -1;

	cc = g_ops->pd_dpm_get_cc_state();
	*cc1 = cc & PD_DPM_CC_STATUS_MASK;
	*cc2 = (cc >> PD_DPM_CC2_STATUS_OFFSET) & PD_DPM_CC_STATUS_MASK;

	return 0;
}

void pd_dpm_report_pd_source_vconn(const void *data)
{
	if (data)
#ifdef CONFIG_BOOST_5V
		boost_5v_enable(*(int *)data, BOOST_CTRL_PD_VCONN);
#endif /* CONFIG_BOOST_5V */
	hwlog_info("%s - \n", __func__);
}

static void pd_dpm_hw_set_src_psrdelay(int delay)
{
	if (!g_ops || !g_ops->pd_dpm_set_src_psrdelay)
		return;

	hwlog_info("%s: set delay %d\n", __func__, delay);
	g_ops->pd_dpm_set_src_psrdelay(g_client, delay);
}

static bool pd_dpm_hw_check_src_cap(void)
{
	int ret = -EINVAL;

	if (g_ops && g_ops->pd_dpm_get_src_extcap)
		ret = g_ops->pd_dpm_get_src_extcap(g_client);

	hwlog_info("%s: ret=%d\n", __func__, ret);
	return (ret > 0);
}

static int pd_dpm_hw_update_src_cap(struct otg_cap *ext, int send)
{
	if (!g_ops || !g_ops->pd_dpm_set_src_cap)
		return -EACCES;

	hwlog_info("%s: max_otg_vol=%d, max_otg_cur=%d, send=%d\n",
		__func__, ext->otg_vol, ext->otg_vol, send);
	g_ops->pd_dpm_set_src_cap(g_client, ext, (send ? 1 : 0));
	return 0;
}

int hw_pd_set_source_state(struct otg_cap *allow_ext)
{
	int ret;

	if (!pd_dpm_hw_check_src_cap()) {
		hwlog_err("%s not support\n", __func__);
		return -EACCES;
	}

	return pd_dpm_hw_update_src_cap(allow_ext, 1);
}

static int hw_pd_set_default_source_cap()
{
	struct otg_cap default_src_cap;

	default_src_cap.otg_vol = SRC_VBUS_DEFAULT_VOL;
	default_src_cap.otg_cur = SRC_VBUS_DEFAULT_CUR;

	return pd_dpm_hw_update_src_cap(&default_src_cap, 0);
}

static int wired_rechrg_event_notifier_call(struct notifier_block *nb, unsigned long event, void *cap)
{
	struct pd_dpm_info *di = g_pd_di;
	struct otg_cap *ext_src_cap = (struct otg_cap *)cap;

	if (!di || !ext_src_cap)
		return -ENODEV;

	if ((ext_src_cap->otg_vol != SRC_VBUS_DEFAULT_VOL) && (ext_src_cap->otg_vol != SRC_VBUS_HIGH_VOL)) {
		hwlog_info("%s: charger allow max otg_vol is illegal\n", __func__);
		return NOTIFY_DONE;
	}

	hwlog_info("%s: charger allow max otg_vol : %d, otg_cur : %d\n", __func__,
		ext_src_cap->otg_vol, ext_src_cap->otg_cur);

	if (ext_src_cap->otg_vol == SRC_VBUS_DEFAULT_VOL) {
		rchg_pd_9v_enable(false);
		hwlog_info("%s: close rchg 9V\n", __func__);
	}

	hw_pd_set_source_state(ext_src_cap);

	return NOTIFY_OK;
}

void pd_dpm_report_pd_source_vbus(struct pd_dpm_info *di, void *data)
{
	struct pd_dpm_vbus_state *vbus_state = data;

	mutex_lock(&di->sink_vbus_lock);
	if (external_pd_flag) {
		if (pmic_vbus_irq_is_enabled())
			ignore_bc12_event_when_vbuson = true;
	} else {
		ignore_bc12_event_when_vbuson = true;
	}

	if (vbus_state->vbus_type & TCP_VBUS_CTRL_PD_DETECT) {
		di->pd_finish_flag = true;
		di->ctc_cable_flag = true;
	}

	if (vbus_state->mv == 0) {
		hwlog_info("%s : Disable\n", __func__);
		rchg_pd_otg_enable(false);
		hwlog_info("%s:notify charger disable source vbus\n", __func__);

		if (g_otg_channel) {
			vbus_ch_close(VBUS_CH_USER_PD,
				VBUS_CH_TYPE_BOOST_GPIO, true, false);
#ifdef CONFIG_POGO_PIN
		} else if (pogopin_is_support() && (pogopin_otg_from_buckboost())) {
			vbus_ch_close(VBUS_CH_USER_PD, VBUS_CH_TYPE_POGOPIN_BOOST, true, false);
#endif /* CONFIG_POGO_PIN */
		} else {
			pd_dpm_vbus_notifier_call(g_pd_di, CHARGER_TYPE_NONE, data);
			pd_dpm_set_source_sink_state(POWER_SUPPLY_STOP_SOURCE);
		}
	} else {
		di->pd_source_vbus = true;
		hwlog_info("%s : Source %d mV, %d mA\n", __func__, vbus_state->mv, vbus_state->ma);

		if (vbus_state->mv == SRC_VBUS_HIGH_VOL) {
			pd_dpm_hw_set_src_psrdelay(1);
			rchg_pd_9v_enable(true);
			hwlog_info("%s: open rchg 9v\n", __func__);
		} else if (g_otg_channel) {
			vbus_ch_open(VBUS_CH_USER_PD, VBUS_CH_TYPE_BOOST_GPIO, true);
			rchg_pd_otg_enable(true);
			hwlog_info("%s:notify charger enable source vbus\n", __func__);
#ifdef CONFIG_POGO_PIN
		} else if (pogopin_is_support() && (pogopin_otg_from_buckboost())) {
			vbus_ch_open(VBUS_CH_USER_PD, VBUS_CH_TYPE_POGOPIN_BOOST, true);
#endif /* CONFIG_POGO_PIN */
		} else {
				pd_dpm_vbus_notifier_call(g_pd_di, PLEASE_PROVIDE_POWER, data);
				pd_dpm_set_source_sink_state(POWER_SUPPLY_START_SOURCE);
		}
	}
	mutex_unlock(&di->sink_vbus_lock);
}

#define VBUS_VOL_9000MV 9000
void pd_dpm_report_pd_sink_vbus(struct pd_dpm_info *di, void *data)
{
	bool high_power_charging = false;
	bool high_voltage_charging = false;
	unsigned long event;
	struct pd_dpm_vbus_state *vbus_state = data;

	mutex_lock(&di->sink_vbus_lock);
	pd_dpm_set_cc_voltage(vbus_state->remote_rp_level);

	if (vbus_state->vbus_type & TCP_VBUS_CTRL_PD_DETECT) {
		adapter_test_set_result(AT_TYPE_PD, AT_PROTOCOL_FINISH_SUCC);
		if (pmic_vbus_irq_is_enabled())
			ignore_bc12_event_when_vbuson = true;
		di->pd_finish_flag = true;
		di->ctc_cable_flag = true;
	}

	if (di->pd_finish_flag)
		event = PD_DPM_VBUS_TYPE_PD;
	else
		event = PD_DPM_VBUS_TYPE_TYPEC;

	vbus_state = data;

	if (vbus_state->mv == 0) {
		if (event == PD_DPM_VBUS_TYPE_PD) {
			hwlog_info("%s : Disable\n", __func__);
			pd_dpm_vbus_notifier_call(g_pd_di, CHARGER_TYPE_NONE, data);
			pd_dpm_set_source_sink_state(POWER_SUPPLY_STOP_SINK);
		}
	} else {
		di->pd_source_vbus = false;
		hwlog_info("%s : Sink %d mV, %d mA\n", __func__, vbus_state->mv, vbus_state->ma);
		if ((vbus_state->mv * vbus_state->ma) >= PD_DPM_POWER_QUICK_CHG_THR) {
			hwlog_info("%s : over 18w\n", __func__);
			high_power_charging = true;
		}
		if (vbus_state->mv >= VBUS_VOL_9000MV) {
			hwlog_info("%s : over 9V\n", __func__);
			high_voltage_charging = true;
		}
		hwlog_info("%s : %d\n", __func__, vbus_state->mv * vbus_state->ma);
		pd_dpm_set_high_power_charging_status(high_power_charging);
		pd_dpm_set_high_voltage_charging_status(high_voltage_charging);

		if (pd_dpm_typec_state != PD_DPM_USB_TYPEC_DETACHED)
			pd_dpm_set_source_sink_state(POWER_SUPPLY_START_SINK);
		pd_dpm_vbus_notifier_call(g_pd_di, event, data);
	}

	mutex_unlock(&di->sink_vbus_lock);
}

void pd_dpm_wakelock_ctrl(unsigned long event)
{
	if (g_pd_di) {
		if ((event == PD_WAKE_LOCK) || (event == PD_WAKE_UNLOCK))
			pd_dpm_wake_unlock_notifier_call(g_pd_di, event, NULL);
	}
}

#ifdef CONFIG_WIRELESS_CHARGER
static void pd_dpm_vbus_ch_open(struct pd_dpm_info *di)
{
	if (g_otg_channel) {
		pd_dpm_set_ignore_vbuson_event(false);
		vbus_ch_open(VBUS_CH_USER_PD, VBUS_CH_TYPE_BOOST_GPIO, false);
	} else {
		pd_dpm_set_ignore_vbuson_event(true);
		if (pogopin_is_support() && pogopin_otg_from_buckboost())
			vbus_ch_open(VBUS_CH_USER_PD, VBUS_CH_TYPE_POGOPIN_BOOST, true);
		else
			pd_dpm_set_source_sink_state(POWER_SUPPLY_START_SOURCE);
		pd_dpm_vbus_notifier_call(di, PLEASE_PROVIDE_POWER, NULL);
	}
}

static void pd_dpm_vbus_ch_close(struct pd_dpm_info *di)
{
	if (g_otg_channel) {
		pd_dpm_set_ignore_vbusoff_event(false);
		vbus_ch_close(VBUS_CH_USER_PD, VBUS_CH_TYPE_BOOST_GPIO, false, false);
	} else {
		pd_dpm_set_ignore_vbusoff_event(true);
		if (pogopin_is_support() && pogopin_otg_from_buckboost())
			vbus_ch_close(VBUS_CH_USER_PD,
				VBUS_CH_TYPE_POGOPIN_BOOST, true, false);
		else
			pd_dpm_set_source_sink_state(POWER_SUPPLY_STOP_SOURCE);
		pd_dpm_vbus_notifier_call(di, CHARGER_TYPE_NONE, NULL);
	}
}
#else
static void pd_dpm_vbus_ch_open(struct pd_dpm_info *di)
{
	if (pogopin_is_support() && pogopin_otg_from_buckboost()) {
		vbus_ch_open(VBUS_CH_USER_PD, VBUS_CH_TYPE_POGOPIN_BOOST, true);
	} else {
		pd_dpm_set_ignore_vbuson_event(true);
		pd_dpm_set_source_sink_state(POWER_SUPPLY_START_SOURCE);
	}
	pd_dpm_vbus_notifier_call(di, PLEASE_PROVIDE_POWER, NULL);
}

static void pd_dpm_vbus_ch_close(struct pd_dpm_info *di)
{
	if (pogopin_is_support() && pogopin_otg_from_buckboost()) {
		vbus_ch_close(VBUS_CH_USER_PD, VBUS_CH_TYPE_POGOPIN_BOOST, true, false);
	} else {
		pd_dpm_set_ignore_vbusoff_event(true);
		pd_dpm_set_source_sink_state(POWER_SUPPLY_STOP_SOURCE);
	}
	pd_dpm_vbus_notifier_call(di, CHARGER_TYPE_NONE, NULL);
}
#endif /* CONFIG_WIRELESS_CHARGER */

/* fix the problem of OTG output still valid when OTG is pluging out */
static void pd_dpm_vbus_ctrl_detached(struct pd_dpm_info *di,
	unsigned long event)
{
	int typec_state = PD_DPM_USB_TYPEC_DETACHED;

	if (!di)
		return;

	pd_dpm_get_typec_state(&typec_state);
	if ((event != PLEASE_PROVIDE_POWER) ||
		(typec_state != PD_DPM_USB_TYPEC_DETACHED) ||
		pd_dpm_get_audio_power_no_hs_state())
		return;

	hwlog_err("typec is detached, should cut off otg output\n");
	pd_dpm_vbus_ch_close(di);
}

/* event: CHARGER_TYPE_NONE, PLEASE_PROVIDE_POWER */
void pd_dpm_vbus_ctrl(unsigned long event)
{
	struct pd_dpm_info *di = g_pd_di;
	int typec_state = PD_DPM_USB_TYPEC_DETACHED;

	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	pd_dpm_get_typec_state(&typec_state);
	if ((typec_state == PD_DPM_USB_TYPEC_DETACHED) &&
		!pd_dpm_get_audio_power_no_hs_state()) {
		hwlog_info("%s typec unattached\n", __func__);
		return;
	}

	if (event == PLEASE_PROVIDE_POWER)
		pd_dpm_vbus_ch_open(di);
	else
		pd_dpm_vbus_ch_close(di);

	pd_dpm_vbus_ctrl_detached(di, event);
	hwlog_info("%s event = %ld\n", __func__, event);
}

int pd_dpm_ocp_nb_call(struct notifier_block *ocp_nb,
	unsigned long event, void *data)
{
	struct pd_dpm_info *di =
		container_of(ocp_nb, struct pd_dpm_info, ocp_nb);
	static unsigned int count;

	if (!di || !data || !di->pd_reinit_enable)
		return 0;

	if (strncmp(di->ldo_name, (char *)data, LDO_NAME_SIZE - 1))
		return 0;

	if (count > di->max_ocp_count)
		return 0;

	count++;

	if (!di->vdd_ocp_lock->active)
		__pm_stay_awake(di->vdd_ocp_lock);

	schedule_delayed_work(&di->reinit_pd_work,
		msecs_to_jiffies(di->ocp_delay_time));
	return 0;
}

int pd_dpm_report_bc12(struct notifier_block *usb_nb, unsigned long event, void *data)
{
	struct pd_dpm_info *di = container_of(usb_nb, struct pd_dpm_info, usb_nb);
	static unsigned long last_event = CHARGER_TYPE_NONE;

	hwlog_info("%s : received event = %lu\n", __func__, event);

	if (pmic_vbus_is_connected()) {
		pd_dpm_vbus_notifier_call(di, event, data);
		return NOTIFY_OK;
	}

	if ((event == CHARGER_TYPE_NONE) && !di->pd_finish_flag &&
		!pd_dpm_get_pd_source_vbus()) {
		hwlog_info("%s : PD_WAKE_UNLOCK \n", __func__);
		pd_dpm_wake_unlock_notifier_call(g_pd_di, PD_WAKE_UNLOCK, NULL);
	}

	hwlog_info("[sn]%s : bc12on %d,bc12off %d\n", __func__,
		ignore_bc12_event_when_vbuson, ignore_bc12_event_when_vbusoff);

	mutex_lock(&g_pd_di->sink_vbus_lock);
	di->bc12_event = event;

	if (event == PLEASE_PROVIDE_POWER) {
		mutex_unlock(&g_pd_di->sink_vbus_lock);
		return NOTIFY_OK;
	}

	if (pd_dpm_get_pd_source_vbus()) {
		hwlog_info("%s : line %d\n", __func__, __LINE__);
		goto End;
	}
	if (ignore_bc12_event_when_vbuson && (event == CHARGER_TYPE_NONE)) {
		hwlog_info("%s : bc1.2 event not match\n", __func__);
		goto End;
	}
	if (!pmic_vbus_irq_is_enabled() &&
		((sink_source_type == POWER_SUPPLY_STOP_SINK) && (event != CHARGER_TYPE_NONE))) {
		hwlog_info("%s : pd aready in STOP_SINK state,"
			"but bc1.2 event not CHARGER_TYPE_NONE, ignore\n", __func__);
		goto End;
	}
	if (!pmic_vbus_irq_is_enabled() && (event == CHARGER_TYPE_NONE) &&
		(!pogopin_is_support() || !pogopin_otg_from_buckboost())) {
		hwlog_info("%s : ignore CHARGER_TYPE_NONE\n", __func__);
		goto End;
	}

	if ((!ignore_bc12_event_when_vbusoff && (event == CHARGER_TYPE_NONE)) ||
		(!ignore_bc12_event_when_vbuson && (event != CHARGER_TYPE_NONE))) {
		if ((!di->pd_finish_flag && (last_event != event)) ||
			(di->pd_finish_flag && (event == CHARGER_TYPE_DCP))) {
			hwlog_info("%s : notify event = %lu\n", __func__, event);
			if (g_cur_usb_event == PD_DPM_TYPEC_ATTACHED_AUDIO) {
				event = CHARGER_TYPE_SDP;
				data = NULL;
			}
			last_event = event;
			pd_dpm_vbus_notifier_call(di, event, data);
		} else {
			hwlog_info("%s : ignore, current event=%lu, last event=%lu\n",
				__func__, event, last_event);
		}
	}

End:
	if (event == CHARGER_TYPE_NONE) {
		last_event = CHARGER_TYPE_NONE;
		ignore_bc12_event_when_vbusoff = false;
		ignore_bc12_event_when_vbuson = false;
	}
	mutex_unlock(&g_pd_di->sink_vbus_lock);

	return NOTIFY_OK;
}

int register_pd_wake_unlock_notifier(struct notifier_block *nb)
{
	if (!nb)
		return -EINVAL;

	if (!g_pd_di)
		return 0;

	return atomic_notifier_chain_register(&g_pd_di->pd_wake_unlock_evt_nh, nb);
}
EXPORT_SYMBOL(register_pd_wake_unlock_notifier);

int unregister_pd_wake_unlock_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_unregister(&g_pd_di->pd_wake_unlock_evt_nh, nb);
}
EXPORT_SYMBOL(unregister_pd_wake_unlock_notifier);

int register_pd_dpm_notifier(struct notifier_block *nb)
{
	int ret = 0;

	if (!nb)
		return -EINVAL;

	if (!g_pd_di)
		return ret;

	ret = blocking_notifier_chain_register(&g_pd_di->pd_evt_nh, nb);
	if (ret != 0)
		return ret;

	return ret;
}
EXPORT_SYMBOL(register_pd_dpm_notifier);

int unregister_pd_dpm_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&g_pd_di->pd_evt_nh, nb);
}
EXPORT_SYMBOL(unregister_pd_dpm_notifier);

int register_pd_dpm_portstatus_notifier(struct notifier_block *nb)
{
	int ret = -1;

	if (!nb)
		return -EINVAL;

	if (!g_pd_di)
		return ret;

	ret = blocking_notifier_chain_register(&g_pd_di->pd_port_status_nh, nb);
	if (ret != 0)
		return ret;

	return ret;
}
EXPORT_SYMBOL(register_pd_dpm_portstatus_notifier);

int unregister_pd_dpm_portstatus_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&g_pd_di->pd_port_status_nh, nb);
}
EXPORT_SYMBOL(unregister_pd_dpm_portstatus_notifier);

void pd_dpm_usb_host_on(void)
{
#ifdef CONFIG_CONTEXTHUB_PD
	struct pd_dpm_combphy_event event;

	event.dev_type = TCA_ID_FALL_EVENT;
	event.irq_type = TCA_IRQ_HPD_IN;
	event.mode_type = TCPC_USB31_CONNECTED;
	event.typec_orien = pd_dpm_get_cc_orientation();
	pd_dpm_handle_combphy_event(event);
#endif /* CONFIG_CONTEXTHUB_PD */
}

void pd_dpm_usb_host_off(void)
{
#ifdef CONFIG_CONTEXTHUB_PD
	struct pd_dpm_combphy_event event;

	event.typec_orien = pd_dpm_get_cc_orientation();
	event.dev_type = TCA_ID_RISE_EVENT;
	event.irq_type = TCA_IRQ_HPD_OUT;
	event.mode_type = TCPC_NC;
	pd_dpm_set_combphy_status(TCPC_NC);
	pd_dpm_handle_combphy_event(event);
#endif /* CONFIG_CONTEXTHUB_PD */
}

void pd_dpm_report_device_attach(void)
{
#ifdef CONFIG_CONTEXTHUB_PD
	struct pd_dpm_combphy_event event;
#endif

	hwlog_info("%s \r\n", __func__);

#ifdef CONFIG_CONTEXTHUB_PD
	event.dev_type = TCA_CHARGER_CONNECT_EVENT;
	event.irq_type = TCA_IRQ_HPD_IN;
	event.mode_type = TCPC_USB31_CONNECTED;
	event.typec_orien = pd_dpm_get_cc_orientation();
	pd_dpm_handle_combphy_event(event);
#else
	chip_usb_otg_event(CHARGER_CONNECT_EVENT);
#endif /* CONFIG_CONTEXTHUB_PD */

	water_detection_entrance();
}

void pd_dpm_report_host_attach(void)
{
#ifdef CONFIG_CONTEXTHUB_PD
	struct pd_dpm_combphy_event event;
#endif /* CONFIG_CONTEXTHUB_PD */

	hwlog_info("%s \r\n", __func__);

#ifdef CONFIG_SWITCH_FSA9685
	if (switch_manual_enable) {
		usbswitch_common_dcd_timeout_enable(true);
		usbswitch_common_manual_sw(FSA9685_USB1_ID_TO_IDBYPASS);
	}
#endif /* CONFIG_SWITCH_FSA9685 */

#ifdef CONFIG_CONTEXTHUB_PD
	event.dev_type = TCA_ID_FALL_EVENT;
	event.irq_type = TCA_IRQ_HPD_IN;
	event.mode_type = TCPC_USB31_CONNECTED;
	event.typec_orien = pd_dpm_get_cc_orientation();
	pd_dpm_handle_combphy_event(event);
#else
	chip_usb_otg_event(ID_FALL_EVENT);
#endif /* CONFIG_CONTEXTHUB_PD */

	uvdm_init_work();
}

void pd_dpm_report_device_detach(void)
{
#ifdef CONFIG_CONTEXTHUB_PD
	struct pd_dpm_combphy_event event;
#endif

	hwlog_info("%s \r\n", __func__);

#ifdef CONFIG_CONTEXTHUB_PD
	event.dev_type = TCA_CHARGER_DISCONNECT_EVENT;
	event.irq_type = TCA_IRQ_HPD_OUT;
	event.mode_type = TCPC_NC;
	event.typec_orien = pd_dpm_get_cc_orientation();
	pd_dpm_handle_combphy_event(event);
#else
	chip_usb_otg_event(CHARGER_DISCONNECT_EVENT);
#endif /* CONFIG_CONTEXTHUB_PD */
}

void pd_dpm_report_host_detach(void)
{
#ifdef CONFIG_CONTEXTHUB_PD
	struct pd_dpm_combphy_event event;
#endif /* CONFIG_CONTEXTHUB_PD */

	hwlog_info("%s \r\n", __func__);

#ifdef CONFIG_SWITCH_FSA9685
	usbswitch_common_dcd_timeout_enable(false);
#endif /* CONFIG_SWITCH_FSA9685 */

#ifdef CONFIG_CONTEXTHUB_PD
	event.typec_orien = pd_dpm_get_cc_orientation();
	event.mode_type = pd_dpm_get_combphy_status();
	hwlog_info("%s:mode_type=%d,typec_orien=%d\n", __func__, event.mode_type, event.typec_orien);
	if (pd_dpm_get_last_hpd_status()) {
		event.dev_type = TCA_DP_OUT;
		event.irq_type = TCA_IRQ_HPD_OUT;
		pd_dpm_handle_combphy_event(event);
		pd_dpm_set_last_hpd_status(false);
		hwlog_info("%s 1:mode_type=%d,typec_orien=%d\n", __func__, event.mode_type, event.typec_orien);
		hw_usb_send_event(DP_CABLE_OUT_EVENT);
	}
	hwlog_info("%s 2:mode_type=%d,typec_orien=%d\n", __func__, event.mode_type, event.typec_orien);
	event.dev_type = TCA_ID_RISE_EVENT;
	event.irq_type = TCA_IRQ_HPD_OUT;
	event.mode_type = TCPC_NC;
	pd_dpm_set_combphy_status(TCPC_NC);
	pd_dpm_handle_combphy_event(event);
	/* set aux uart switch low */
	if (support_dp)
		dp_aux_uart_switch_disable();
#else
	chip_usb_otg_event(ID_RISE_EVENT);
#endif /* CONFIG_CONTEXTHUB_PD */
	uvdm_cancel_work();
}

static void pd_dpm_report_attach(int new_state)
{
	switch (new_state) {
	case PD_DPM_USB_TYPEC_DEVICE_ATTACHED:
		pd_dpm_report_device_attach();
		break;
	case PD_DPM_USB_TYPEC_HOST_ATTACHED:
		pd_dpm_report_host_attach();
		break;
	default:
		break;
	}
}

static void pd_dpm_report_detach(int last_state)
{
	switch (last_state) {
	case PD_DPM_USB_TYPEC_DEVICE_ATTACHED:
		pd_dpm_report_device_detach();
		break;
	case PD_DPM_USB_TYPEC_HOST_ATTACHED:
		pd_dpm_report_host_detach();
		rchg_pd_otg_enable(false);
		hwlog_info("%s:notify charger disable source vbus\n", __func__);
		break;
	default:
		break;
	}
	hw_usb_host_abnormal_event_notify(USB_HOST_EVENT_NORMAL);
}

static void pd_dpm_usb_update_state(struct work_struct *work)
{
	int new_ev, last_ev;
	struct pd_dpm_info *usb_cb_data =
		container_of(to_delayed_work(work),
			struct pd_dpm_info, usb_state_update_work);

	mutex_lock(&usb_cb_data->usb_lock);
	new_ev = usb_cb_data->pending_usb_event;
	mutex_unlock(&usb_cb_data->usb_lock);

	last_ev = usb_cb_data->last_usb_event;

	if (last_ev == new_ev)
		return;

	switch (new_ev) {
	case PD_DPM_USB_TYPEC_DETACHED:
		pd_dpm_report_detach(last_ev);
		break;
	case PD_DPM_USB_TYPEC_DEVICE_ATTACHED:
	case PD_DPM_USB_TYPEC_HOST_ATTACHED:
		if (last_ev != PD_DPM_USB_TYPEC_DETACHED)
			pd_dpm_report_detach(last_ev);
		pd_dpm_report_attach(new_ev);
		break;
	default:
		return;
	}

	usb_cb_data->last_usb_event = new_ev;
}
int pd_dpm_get_cur_usb_event(void)
{
	if (g_pd_di)
		return g_pd_di->cur_usb_event;

	return 0;
}

bool pd_dpm_get_cc_moisture_status(void)
{
	return g_pd_cc_moisture_status;
}

void pd_dpm_handle_abnomal_change(int event)
{
	int i = 0;
	char dsm_buf[PD_DPM_CC_DMD_BUF_SIZE] = {0};
	int time_diff = 0;
	unsigned int time_diff_index = 0;
	unsigned int flag;
	struct timespec64 ts64_interval;
	struct timespec64 ts64_now = { 0 };
	struct timespec64 ts64_sum;

	struct timespec64 ts64_dmd_interval;
	struct timespec64 ts64_dmd_now = { 0 };
	struct timespec64 ts64_dmd_sum;

	int* change_counter = &abnomal_change[event].change_counter;
	int change_counter_threshold = PD_DPM_CC_CHANGE_COUNTER_THRESHOLD;
	int* dmd_counter = &abnomal_change[event].dmd_counter;
	int dmd_counter_threshold = PD_DPM_CC_DMD_COUNTER_THRESHOLD;
	ts64_interval.tv_sec = 0;
	ts64_interval.tv_nsec = abnormal_cc_interval * NSEC_PER_MSEC;
	ts64_dmd_interval.tv_sec = PD_DPM_CC_DMD_INTERVAL;
	ts64_dmd_interval.tv_nsec = 0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	ktime_get_coarse_real_ts64(&ts64_now);
#else
	ts64_now = current_kernel_time64();
#endif
	if (abnomal_change[event].first_enter) {
		abnomal_change[event].first_enter = false;
	} else {
		ts64_sum = timespec64_add_safe(abnomal_change[event].ts64_last,
			ts64_interval);
		if (ts64_sum.tv_sec == TIME_T_MAX) {
			hwlog_err("%s time overflow happend\n", __func__);
			*change_counter = 0;
		} else if (timespec64_compare(&ts64_sum, &ts64_now) >= 0) {
			++*change_counter;
			hwlog_info("%s change_counter = %d,\n", __func__, *change_counter);

			time_diff = (ts64_now.tv_sec -
				abnomal_change[event].ts64_last.tv_sec) *
				PD_DPM_CC_CHANGE_MSEC +
				(ts64_now.tv_nsec - abnomal_change[event].ts64_last.tv_nsec) /
				NSEC_PER_MSEC;
			time_diff_index = time_diff / (PD_DPM_CC_CHANGE_INTERVAL /
				PD_DPM_CC_CHANGE_BUF_SIZE);
			if (time_diff_index >= PD_DPM_CC_CHANGE_BUF_SIZE)
				time_diff_index = PD_DPM_CC_CHANGE_BUF_SIZE - 1;
			++abnomal_change[event].change_data[time_diff_index];
		} else {
			*change_counter = 0;
			memset(abnomal_change[event].change_data, 0,
				PD_DPM_CC_CHANGE_BUF_SIZE);
		}
	}

	if (*change_counter >= change_counter_threshold) {
		hwlog_err("%s change_counter hit\n", __func__);

		if (g_water_det_ops) {
			if (g_water_det_ops->is_water_detected() == WATER_DET_FALSE) {
				hwlog_err("%s is_water_detected FALSE\n", __func__);
				goto AFTER;
			} else {
				hwlog_err("%s is_water_detected TRUE\n", __func__);
			}
		}

		if (cc_moisture_status_report) {
			g_pd_cc_moisture_happened = true;
			g_pd_cc_moisture_status = true;
		}

		pd_dpm_set_cc_mode(PD_DPM_CC_MODE_UFP);

		for (i = 0; i < PD_DPM_CC_CHANGE_BUF_SIZE; i++)
			abnomal_change[event].dmd_data[i] += abnomal_change[event].change_data[i];
		*change_counter = 0;
		memset(abnomal_change[event].change_data, 0, PD_DPM_CC_CHANGE_BUF_SIZE);
		++*dmd_counter;

		if (moisture_detection_by_cc_enable) {
			hwlog_err("%s moisture_detected\n", __func__);
			flag = WD_NON_STBY_MOIST;
			power_event_bnc_notify(POWER_BNT_WD, POWER_NE_WD_REPORT_UEVENT, &flag);
		}
	}

AFTER:
	if ((*dmd_counter >= dmd_counter_threshold) && g_cc_abnormal_dmd_report_enable) {
		*dmd_counter = 0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
		ktime_get_coarse_real_ts64(&ts64_dmd_now);
#else
		ts64_dmd_now = current_kernel_time64();
#endif
		ts64_dmd_sum = timespec64_add_safe(abnomal_change[event].ts64_dmd_last,
			ts64_dmd_interval);
		if (ts64_dmd_sum.tv_sec == TIME_T_MAX) {
			hwlog_err("%s time overflow happend when add 24 hours\n", __func__);
		} else if (timespec64_compare(&ts64_dmd_sum, &ts64_dmd_now) < 0) {
			snprintf(dsm_buf, PD_DPM_CC_DMD_BUF_SIZE - 1,
				"cc abnormal is triggered:");
			for (i = 0; i < PD_DPM_CC_CHANGE_BUF_SIZE; i++)
				snprintf(dsm_buf + strlen(dsm_buf), PD_DPM_CC_DMD_BUF_SIZE - 1,
					" %d", abnomal_change[event].dmd_data[i]);
			snprintf(dsm_buf + strlen(dsm_buf), PD_DPM_CC_DMD_BUF_SIZE - 1, "\n");

			power_dsm_report_dmd(POWER_DSM_BATTERY,
				POWER_DSM_ERROR_NO_WATER_CHECK_IN_USB, dsm_buf);

			abnomal_change[event].ts64_dmd_last = ts64_dmd_now;
		} else {
			hwlog_info("error: cc abnormal happend within 24 hour, do not report\n");
		}
	}

	abnomal_change[event].ts64_last =  ts64_now;
}
void pd_dpm_ignore_vbus_only_event(bool flag)
{
	g_ignore_vbus_only_event = flag;
}

enum cur_cap pd_dpm_get_cvdo_cur_cap(void)
{
	enum cur_cap cap;

	cap = (g_pd_di->cable_vdo.cable_vdo & CABLE_CUR_CAP_MASK) >>
		CABLE_CUR_CAP_SHIFT;
	if (cap == PD_DPM_CURR_5A)
		cap += (g_pd_di->cable_vdo.cable_vdo_ext &
			CABLE_EXT_CUR_CAP_MASK) >> CABLE_EXT_CUR_CAP_SHIFT;

	hwlog_info("%s, cur_cap = %d\n", __func__, cap);
	return cap;
}

static bool pd_dpm_is_cc_protection(void)
{
	/* cc_dynamic_protect-0: disabled 1: only SC; 2: for SC and LVC */
	if (!cc_dynamic_protect)
		return false;

#ifdef CONFIG_DIRECT_CHARGER
	if (direct_charge_in_charging_stage() != DC_IN_CHARGING_STAGE)
		return false;
#endif /* CONFIG_DIRECT_CHARGER */

	if (!pd_dpm_check_cc_vbus_short())
		return false;

	hwlog_info("cc short\n");
	return true;
}

static void pd_dpm_cc_short_action(void)
{
#ifdef CONFIG_DIRECT_CHARGER
	unsigned int notifier_type = POWER_ANT_LVC_FAULT;
	int mode = direct_charge_get_working_mode();

	/* cc_dynamic_protect-0: disabled 1: only SC; 2: for SC and LVC */
	if (mode == SC4_MODE)
		notifier_type = POWER_ANT_SC4_FAULT;
	else if (mode == SC_MODE)
		notifier_type = POWER_ANT_SC_FAULT;
	else if ((mode == LVC_MODE) && (cc_dynamic_protect == 2))
		notifier_type = POWER_ANT_LVC_FAULT;

	power_event_anc_notify(notifier_type, POWER_NE_DC_FAULT_CC_SHORT, NULL);
#endif /* CONFIG_DIRECT_CHARGER */
	hwlog_info("cc_short_action\n");
}

static void pd_dpm_reinit_pd_work(struct work_struct *work)
{
	struct pd_dpm_info *di =
		container_of(work, struct pd_dpm_info, reinit_pd_work.work);

	if (!di)
		return;
	hwlog_info("pd_dpm_reinit_pd_work start\n");

	di->is_ocp = true;
	if (di->vdd_ocp_lock->active)
		__pm_relax(di->vdd_ocp_lock);

	(void)regulator_disable(di->pd_vdd_ldo);
	msleep(DISABLE_INTERVAL);
	(void)regulator_enable(di->pd_vdd_ldo);
	pd_dpm_reinit_chip();
	di->is_ocp = false;
	hwlog_info("pd_dpm_reinit_pd_work end\n");
}

static void pd_dpm_cc_short_work(struct work_struct *work)
{
	hwlog_info("cc_short_work\n");
	if (!pd_dpm_is_cc_protection())
		return;

	pd_dpm_cc_short_action();
}

static void pd_dpm_otg_restore_work(struct work_struct *work)
{
	struct pd_dpm_info *di = g_pd_di;

	if (!di)
		return;

	pd_dpm_vbus_ch_close(di);
}

void pd_dpm_cc_dynamic_protect(void)
{
	if (!pd_dpm_is_cc_protection())
		return;

	mod_delayed_work(system_wq, &cc_short_work,
		msecs_to_jiffies(CC_SHORT_DEBOUNCE));
}

bool pogo_charger_ignore_typec_event(unsigned long event)
{
	return ((pogopin_5pin_get_pogo_status() == POGO_CHARGER) &&
		((event != PD_DPM_PE_EVT_TYPEC_STATE) &&
		(event != PD_DPM_PE_ABNORMAL_CC_CHANGE_HANDLER))) ?
		true : false;
}

bool pogo_otg_ignore_typec_event(unsigned long event)
{
	return ((event == PD_DPM_PE_EVT_SOURCE_VBUS) ||
		(event == PD_DPM_PE_EVT_SOURCE_VCONN) ||
		(event == PD_DPM_PE_EVT_DR_SWAP) ||
		(event == PD_DPM_PE_EVT_PR_SWAP) ||
		(event == PD_DPM_PE_CABLE_VDO)) ? true : false;
}

bool ana_audio_event(struct pd_dpm_typec_state *typec_state)
{
	if (!typec_state)
		return false;

	return ((typec_state->new_state == PD_DPM_TYPEC_ATTACHED_AUDIO) ||
		((typec_state->new_state == PD_DPM_TYPEC_UNATTACHED) &&
		(g_pd_di->cur_usb_event == PD_DPM_TYPEC_ATTACHED_AUDIO))) ?
		true : false;
}

bool pogo_otg_typec_snk_event(struct pd_dpm_typec_state *typec_state)
{
	if (!typec_state)
		return false;

	return (((typec_state->new_state == PD_DPM_TYPEC_ATTACHED_SNK) ||
		((typec_state->new_state == PD_DPM_TYPEC_UNATTACHED) &&
		(g_pd_di->cur_usb_event == PD_DPM_TYPEC_ATTACHED_SNK))) &&
		(pogopin_5pin_get_pogo_status() == POGO_OTG)) ? true : false;
}

bool pogopin_ignore_typec_event(unsigned long event, void *data)
{
	struct pd_dpm_typec_state *typec_state = NULL;

	if (!pogopin_typec_chg_ana_audio_support() ||
		(pogopin_5pin_get_pogo_status() == POGO_NONE) ||
		!data)
		return false;

	if (pogo_charger_ignore_typec_event(event)) {
		return true;
	} else if ((pogopin_5pin_get_pogo_status() == POGO_OTG) &&
		(event != PD_DPM_PE_EVT_TYPEC_STATE)) {
		if (pogo_otg_ignore_typec_event(event))
			return true;
		else
			return false;
	}

	typec_state = data;

	return (ana_audio_event(typec_state) ||
		pogo_otg_typec_snk_event(typec_state)) ? false : true;
}

void pd_dpm_set_cable_vdo(void *data)
{
	if (!data || !g_pd_di)
		return;

	memcpy(&(g_pd_di->cable_vdo), data, sizeof(g_pd_di->cable_vdo));
	hwlog_info("%s cable_vdo=%x, add_vdo=%x\n", __func__,
		g_pd_di->cable_vdo.cable_vdo, g_pd_di->cable_vdo.cable_vdo_ext);
}

int pd_dpm_support_cable_auth(void)
{
	return g_cable_state.support_cable;
}

static void pd_dpm_start_cable_auth(void)
{
	int ret;
	struct pd_dpm_info *di = g_pd_di;
	uint32_t uvdm_data[UVDM_DATA_LEN_YC] = {0};
	unsigned int random[PD_DPM_DATA_RANDOM];
	uint8_t sop_type = 1;                    /* 1 is sop type flag */
	uint8_t len = 0;
	int i = 0;

	g_cable_state.succ_flag = PD_DPM_CABLE_INIT;
	for (i = 0; i < PD_DPM_DATA_RANDOM; i++) {
		get_random_bytes(&g_cable_state.value[i], 4);
		uvdm_data[i + 1] = g_cable_state.value[i];
		hwlog_info("%s random value is %x", __func__, g_cable_state.value[i]);
	}

	/* 24: The PID of the cable is stored in bits 24 to 31.
	 0xFF: The VID of the cable is stored in bits 0 to 7. */
	if ((g_cable_state.cable_auth.cable_vdo >> 24 == PD_DPM_CABLE_JHT) &&
		(g_cable_state.cable_auth.cable_vdo_ext & 0xFF) == PD_DPM_CABLE_NXT_JHT) {
		g_cable_state.cable_type = 1;        /* 1 is jht cable */
		uvdm_data[0] = UVDM_DATA_HEAD_JHT;
		uvdm_data[5] = 0;
		len = UVDM_DATA_LEN_JHT;
	} else  {
		/* other cable send uevent */
		g_cable_state.cable_type = 2;        /* 2 is yc cable */
		uvdm_data[0] = UVDM_DATA_HEAD;
		uvdm_data[6] = 0x00000001;           /* select key number */
		len = UVDM_DATA_LEN_YC;
	}

	reinit_completion(&g_cable_state.emark_detect_cable_data);
	schedule_delayed_work(&di->hw_pd_get_cable_data_work, msecs_to_jiffies(10));
	ret = adapter_uvdm_send_data(ADAPTER_PROTOCOL_UVDM, uvdm_data, len, true, sop_type);
	if (ret)
		hwlog_info("%s send random is fail");
}

static int pd_dpm_set_cable_vdo_auth(void *data)
{
	enum cur_cap cap;

	if (!data)
		return -1;

	if (memcpy_s(&(g_cable_state.cable_auth), sizeof(g_cable_state.cable_auth),
		data, sizeof(g_cable_state.cable_auth)) != EOK) {
		hwlog_err("%s: memcpy_s err\n", __func__);
		return 0;
	}

	hwlog_info("%s cable_vdo_auth=%x, add_vdo_auth=%x\n", __func__,
		g_cable_state.cable_auth.cable_vdo, g_cable_state.cable_auth.cable_vdo_ext);
	if (g_cable_state.cable_auth.cable_vdo_ext == 0)
			g_cable_state.succ_flag = PD_DPM_RESULT_THIRDPARTY;
	cap = (g_cable_state.cable_auth.cable_vdo & CABLE_CUR_CAP_MASK) >>
		CABLE_CUR_CAP_SHIFT;
	cap += (g_cable_state.cable_auth.cable_vdo_ext &
		CABLE_EXT_CUR_CAP_MASK) >> CABLE_EXT_CUR_CAP_SHIFT;
	if (cap <= PD_DPM_CURR_5A)
		return 0;

	return cap;
}

static void cal_cable_vdo_fail(void)
{
	/* bit 5 is set to 0. bits 6, 13, 14, and 15 are set to 0. */
	g_pd_di->cable_vdo.cable_vdo = g_pd_di->cable_vdo.cable_vdo | ((1 << 5));
	g_pd_di->cable_vdo.cable_vdo = g_pd_di->cable_vdo.cable_vdo & (~(1 << 6));

	g_pd_di->cable_vdo.cable_vdo_ext = g_pd_di->cable_vdo.cable_vdo_ext &
		(~(1 << 13)) & (~(1 << 14)) & (~(1 << 15));
	hwlog_info("%s add_vdo=%x\n", __func__, g_pd_di->cable_vdo.cable_vdo_ext);
}

static void pd_dpm_cable_pre_auth(void)
{
	unsigned int pdo_dec;
	unsigned int cdo_lat;
	int i;

	pdo_dec = g_cable_state.cable_auth.cable_vdo_ext & PD_DPM_CABLE_MASK_DEC;
	cdo_lat = (g_cable_state.cable_auth.cable_vdo >> 13) & PD_DPM_CABLE_MASK_LAT;
	hwlog_info("%s pdo_dec=%x, cdo_lat=%x\n", __func__, pdo_dec, cdo_lat);

	for (i = 0; i < PD_DPM_PRE_AUTH_DATA_LEN ; i++) {
		if ((pdo_dec == g_cable_pre[i].pdo_dec) &&
			(cdo_lat == g_cable_pre[i].cdo_lat)) {
			hwlog_info("%s 6A cable is succ", __func__);
			g_cable_state.succ_flag = PD_DPM_CABLE_INIT;
			return;
		}
	}
	hwlog_err("%s 6A cable is fail", __func__);
	cal_cable_vdo_fail();
	g_cable_state.succ_flag = PD_DPM_CABLE_FAIL;
}

static void pd_dpm_build_random(unsigned char *buf)
{
	int i = 0;
	unsigned int random[PD_DPM_DATA_RANDOM];

	for (i = 0; i < PD_DPM_DATA_RANDOM; i++)
		random[i] = pd_dpm_swap_endpoint(g_cable_state.value[i]);
	if (memcpy_s(buf, RAND_LENGTH, random, sizeof(random)) != EOK)
		hwlog_err("%s: memcpy_s err\n", __func__);
}

static inline void pd_dpm_htoa(unsigned char hb, char *buf)
{
	/* 16 means hexadecimal number, 10 is a decimal number */
	unsigned char c = hb / 16;
	buf[0] = c < 10 ? '0' + c : c - 10 + 'a';
	c = hb % 16;
	buf[1] = c < 10 ? '0' + c : c - 10 + 'a';
	buf[2] = 0;
}

static void pd_dpm_build_hash(unsigned char *buf)
{
	int i = 0;
	unsigned int cable_tmp[PD_DPM_CABLE_HASH];

	for (i = 0; i < PD_DPM_CABLE_HASH; i++)
		cable_tmp[i] = pd_dpm_swap_endpoint(g_cable_state.cable_hash[i]);
	if (memcpy_s(buf, HASH_LENGTH_MAX, cable_tmp, sizeof(cable_tmp)) != EOK)
		hwlog_err("%s: memcpy_s err\n", __func__);
}

static int pd_dpm_build_buf(unsigned int type, unsigned char rand[RAND_LENGTH], unsigned char hash[HASH_LENGTH_MAX], char *buf, int len)
{
	int i;
	int ret;

	ret = sprintf_s(buf, len, "HW_PD_EVENT_CABLE_CHECK=%02X", type);
	if (ret <= 0)
		return -1;

	for (i = 0; i < RAND_LENGTH; i++)
		pd_dpm_htoa(rand[i], buf + strlen(buf));

	for (i = 0; i < HASH_LENGTH_MAX; i++)
		pd_dpm_htoa(hash[i], buf + strlen(buf));
	return 0;
}

static bool pd_dpm_get_send_uevent(char *buf)
{
	int ret;
	char *envp[2] = { buf, NULL };
	struct pd_dpm_info *di = g_pd_di;

	ret = kobject_uevent_env(&di->dev->kobj, KOBJ_CHANGE, envp);
	if (ret == 0)
		hwlog_info("%s uevent sending ok", __func__);
	else
		hwlog_err("%s uevent sending failed", __func__);
	g_cable_state.succ_flag = PD_DPM_CABLE_INIT;

	/* wait 120ms for get key from usb_port */
	if (!wait_for_completion_timeout(&g_cable_state.emark_get_key_value,
		msecs_to_jiffies(120))) {
		hwlog_info("%s get key value is error", __func__);
		g_cable_state.succ_flag = PD_DPM_CABLE_FAIL;
		reinit_completion(&g_cable_state.emark_get_key_value);
		return false;
	}
	hwlog_info("%s  succ_flag is %d\n", __func__, g_cable_state.succ_flag);
	return true;
}

static void pd_dpm_set_cable_auth_cur(void)
{
	int cap;

	emark_detect_complete();
	if (g_cable_state.succ_flag) {
		g_pd_di->cable_vdo.cable_vdo = g_cable_state.cable_auth.cable_vdo;
		g_pd_di->cable_vdo.cable_vdo_ext = g_cable_state.cable_auth.cable_vdo_ext;
		cap =  (g_cable_state.cable_auth.cable_vdo_ext & CABLE_EXT_CUR_CAP_MASK) >> CABLE_EXT_CUR_CAP_SHIFT;
		hwlog_info("%s  cable_vdo=%x, add_vdo=%x\n, cap = %x", __func__,
			g_pd_di->cable_vdo.cable_vdo, g_pd_di->cable_vdo.cable_vdo_ext, cap);
		if ((g_cable_state.succ_flag == PD_DPM_CABLE_FAIL) && (cap == 1))
			pd_dpm_cable_pre_auth();
		if ((g_cable_state.succ_flag == PD_DPM_CABLE_FAIL) && (cap == 2))
			cal_cable_vdo_fail();
	}
	reinit_completion(&g_cable_state.emark_get_key_value);
	g_cable_state.cable_fail = PD_DPM_CABLE_INIT;
}

static void pd_dpm_get_key_work(struct work_struct *work)
{
	int ret;
	bool uevent;
	char buf[EVENT_BUF_LEN] = { 0 };
	unsigned char rand_port[RAND_LENGTH];
	unsigned char hash_port[HASH_LENGTH_MAX];

	hwlog_info("%s start work\n", __func__);
	pd_dpm_build_random(rand_port);
	pd_dpm_build_hash(hash_port);
	ret = pd_dpm_build_buf(g_cable_state.cable_type, rand_port, hash_port, buf, EVENT_BUF_LEN);
	if (ret == -1) {
		hwlog_err("%s build buf err, ret is %d", __func__, ret);
		g_cable_state.succ_flag == PD_DPM_CABLE_FAIL;
		pd_dpm_set_cable_auth_cur();
	}
	hwlog_info("%s pd_dpm_build_buf ret is %d", __func__, ret);
	/* send get key uevent */
	uevent = pd_dpm_get_send_uevent(buf);
	if (uevent && (g_cable_state.cable_fail < UVDM_RETRY_TIME) &&
		g_cable_state.succ_flag == PD_DPM_CABLE_FAIL) {
		g_cable_state.cable_fail++;
		pd_dpm_start_cable_auth();
		return;
	}
	pd_dpm_set_cable_auth_cur();
}

static void pd_dpm_get_cable_data_work(struct work_struct *work)
{
	int cur = 0;
	hwlog_info("%s start work\n", __func__);

	/* wait 150ms for get data from cable */
	if (wait_for_completion_timeout(
		&g_cable_state.emark_detect_cable_data,
		msecs_to_jiffies(150)))
		return;

	g_pd_di->cable_vdo.cable_vdo = g_cable_state.cable_auth.cable_vdo;
	g_pd_di->cable_vdo.cable_vdo_ext = g_cable_state.cable_auth.cable_vdo_ext;
	g_cable_state.succ_flag = PD_DPM_RESULT_6A_TIMEOUT;

	cur = (g_cable_state.cable_auth.cable_vdo & CABLE_CUR_CAP_MASK) >>
		CABLE_CUR_CAP_SHIFT;
	cur += (g_cable_state.cable_auth.cable_vdo_ext &
		CABLE_EXT_CUR_CAP_MASK) >> CABLE_EXT_CUR_CAP_SHIFT;

	if (cur == PD_DPM_CURR_7A) {
		cal_cable_vdo_fail();
		g_cable_state.succ_flag = PD_DPM_CABLE_FAIL;
	}
	if (cur == PD_DPM_CURR_6A)
		pd_dpm_cable_pre_auth();

	reinit_completion(&g_cable_state.emark_detect_cable_data);
	emark_detect_complete();
	hwlog_info("%s cable data timeout, cur is %d\n", __func__, cur);
}

static void pd_dpm_set_cable_data_handle(void *data)
{
	int i = 0;
	struct pd_dpm_info *di = g_pd_di;

	if (!data || !g_pd_di) {
		hwlog_info("%s  err", __func__);
		return;
	}
	if (memcpy_s(&(g_pd_di->cable_data), sizeof(g_pd_di->cable_data),
		data, sizeof(g_pd_di->cable_data)) != EOK) {
		hwlog_err("%s: memcpy_s err\n", __func__);
		return;
	}
	for (i = 1; i < g_pd_di->cable_data.cnt; i++) {
		hwlog_info("%s receive cnt is %d, cable data is %02x", __func__,
			g_pd_di->cable_data.cnt, g_pd_di->cable_data.msg[i]);
		g_cable_state.cable_hash[i - 1] = g_pd_di->cable_data.msg[i];
	}
	schedule_delayed_work(&di->hw_pd_get_key_work, msecs_to_jiffies(10));
	hwlog_info("%s  end", __func__);
}

int pd_dpm_handle_pe_event(unsigned long event, void *data)
{
	int usb_event = PD_DPM_USB_TYPEC_NONE;
	struct pd_dpm_typec_state *typec_state = NULL;
	bool notify_audio = false;
	int event_id = ANA_HS_PLUG_OUT;
	int vusb;
	int cable_cur;
	struct pd_dpm_info *di = g_pd_di;
	
	if (!g_pd_di) {
		hwlog_err("%s g_pd_di is null\n", __func__);
		return -1;
	}

	if (pogopin_ignore_typec_event(event, data)) {
		hwlog_info("pogo inset device ignore typec event:%d", event);
		return 0;
	}

	switch (event) {
	case PD_DPM_PE_ABNORMAL_CC_CHANGE_HANDLER:
		if (external_pd_flag)
			break;
		if (abnormal_cc_detection &&
			(direct_charge_get_stage_status() <
			DC_STAGE_CHARGE_INIT))
			pd_dpm_handle_abnomal_change(PD_DPM_ABNORMAL_CC_CHANGE);

		return 0;

	case PD_DPM_PE_EVT_TYPEC_STATE:
		{
			if (!data || !g_pd_di) {
				hwlog_info("%s data is null\r\n", __func__);
				return -1;
			}

			typec_state = data;
			if (!support_analog_audio && (typec_state->new_state ==
				PD_DPM_TYPEC_ATTACHED_AUDIO)) {
				analog_audio_status = 1;
				power_event_bnc_notify(POWER_BNT_HW_USB, POWER_NE_HW_USB_HEADPHONE, NULL);
				hwlog_err("%s does not support analog audio\n", __func__);
				return -1;
			}
			g_pd_di->cur_usb_event = typec_state->new_state;
			g_pd_cc_orientation_factory = CC_ORIENTATION_FACTORY_UNSET;
			switch (typec_state->new_state) {
			case PD_DPM_TYPEC_ATTACHED_SNK:
				pd_dpm_analog_hs_state = 0;
				usb_event = PD_DPM_USB_TYPEC_DEVICE_ATTACHED;
				hwlog_info("%s ATTACHED_SINK\r\n", __func__);
				pd_dpm_set_source_sink_state(POWER_SUPPLY_START_SINK);
				typec_complete(COMPLETE_FROM_TYPEC_CHANGE);
				break;

			case PD_DPM_TYPEC_ATTACHED_SRC:
				usb_event = PD_DPM_USB_TYPEC_HOST_ATTACHED;
				hwlog_info("%s ATTACHED_SOURCE\r\n", __func__);
				typec_complete(COMPLETE_FROM_TYPEC_CHANGE);
				break;

			case PD_DPM_TYPEC_UNATTACHED:
				g_pd_di->ctc_cable_flag = false;
				pd_dpm_set_optional_max_power_status(false);
				if (!support_analog_audio && analog_audio_status) {
					analog_audio_status = 0;
					power_event_bnc_notify(POWER_BNT_HW_USB, POWER_NE_HW_USB_HEADPHONE_OUT, NULL);
					hwlog_info("%s ANALOG AUDIO UNATTACHED\r\n", __func__);
				}
				/* the sequence can not change, would affect sink_vbus command */
				if (pd_dpm_analog_hs_state == 1) {
					usb_event = PD_DPM_USB_TYPEC_AUDIO_DETACHED;
					notify_audio = true;
					pd_dpm_analog_hs_state = 0;
					event_id = ANA_HS_PLUG_OUT;
					if (g_pd_di->pd_reinit_enable &&
						g_pd_di->is_ocp &&
						g_pd_di->pd_vdd_ldo)
						notify_audio = false;
					pogopin_5pin_set_ana_audio_status(false);
					hwlog_info("%s AUDIO UNATTACHED\r\n", __func__);
				} else {
					usb_event = PD_DPM_USB_TYPEC_DETACHED;

					if (sink_source_type == POWER_SUPPLY_START_SINK)
						pd_dpm_set_source_sink_state(POWER_SUPPLY_STOP_SINK);
					hw_pd_set_default_source_cap();
					hwlog_info("%s UNATTACHED\r\n", __func__);
				}
#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
				if (support_smart_holder) {
					if (g_pd_smart_holder)
						hishow_notify_android_uevent(HISHOW_DEVICE_OFFLINE,
							HISHOW_USB_DEVICE);
					g_pd_smart_holder = false;
				}
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC2 */
				reinit_typec_completion();
				g_pd_cc_orientation_factory = CC_ORIENTATION_FACTORY_SET;
				pd_set_product_type(PD_DPM_INVALID_VAL);
				g_pd_di->cable_vdo.cable_vdo = 0;
				g_pd_di->cable_vdo.cable_vdo_ext = 0;
				g_cable_state.succ_flag = PD_DPM_CABLE_INIT;
				uem_handle_detach_event();
				break;

			case PD_DPM_TYPEC_ATTACHED_AUDIO:
				notify_audio = true;
				pd_dpm_analog_hs_state = 1;
				event_id = ANA_HS_PLUG_IN;
				usb_event = PD_DPM_USB_TYPEC_AUDIO_ATTACHED;
				pogopin_5pin_set_ana_audio_status(true);
				hwlog_info("%s ATTACHED_AUDIO\r\n", __func__);
				typec_complete(COMPLETE_FROM_TYPEC_CHANGE);
				break;
			case PD_DPM_TYPEC_ATTACHED_DBGACC_SNK:
			case PD_DPM_TYPEC_ATTACHED_CUSTOM_SRC:
				usb_event = PD_DPM_USB_TYPEC_DEVICE_ATTACHED;
				pd_dpm_set_source_sink_state(POWER_SUPPLY_START_SINK);
				pd_dpm_set_cc_voltage(PD_DPM_CC_VOLT_SNK_DFT);
				hwlog_info("%s ATTACHED_CUSTOM_SRC\r\n", __func__);
				typec_complete(COMPLETE_FROM_TYPEC_CHANGE);
				break;
#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
			case PD_DPM_TYPEC_ATTACHED_CUSTOM_SRC2:
				if (support_smart_holder) {
					usb_event = PD_DPM_USB_TYPEC_DEVICE_ATTACHED;
					hwlog_info("%s ATTACHED_CUSTOM_SRC2\r\n", __func__);
					if (g_cable_vdo_ops && g_cable_vdo_ops->is_cust_src2_cable()) {
						if (!g_pd_smart_holder)
							hishow_notify_android_uevent(HISHOW_DEVICE_ONLINE,
								HISHOW_USB_DEVICE);
						g_pd_smart_holder = true;
					}
					typec_complete(COMPLETE_FROM_TYPEC_CHANGE);
				}
				break;
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC2 */
			case PD_DPM_TYPEC_ATTACHED_DEBUG:
				pd_dpm_set_cc_voltage(PD_DPM_CC_VOLT_SNK_DFT);
				break;
			case PD_DPM_TYPEC_ATTACHED_VBUS_ONLY:
				g_pd_cc_orientation_factory = CC_ORIENTATION_FACTORY_SET;
				g_vbus_only_status = true;
				if (g_ignore_vbus_only_event) {
					hwlog_err("%s: ignore ATTACHED_VBUS_ONLY\n", __func__);
					return 0;
				}
				if (g_vbus_only_min_voltage) {
					vusb = charge_get_vusb();
					hwlog_info("%s: charge_get_vusb: %d mV\n", __func__, vusb);
					if (vusb < g_vbus_only_min_voltage) {
						hwlog_err("%s: vusb less 3.9V, bypass\n", __func__);
						return 0;
					}
				}
				pd_dpm_set_source_sink_state(POWER_SUPPLY_START_SINK);
				usb_event = PD_DPM_USB_TYPEC_DEVICE_ATTACHED;
				hwlog_info("%s ATTACHED_VBUS_ONLY\r\n", __func__);
				break;
			case PD_DPM_TYPEC_UNATTACHED_VBUS_ONLY:
				g_pd_cc_orientation_factory = CC_ORIENTATION_FACTORY_SET;
				g_vbus_only_status = false;
				hwlog_info("%s UNATTACHED_VBUS_ONLY\r\n", __func__);
				pd_dpm_handle_abnomal_change(PD_DPM_UNATTACHED_VBUS_ONLY);
				usb_event = PD_DPM_USB_TYPEC_DETACHED;
				pd_dpm_set_source_sink_state(POWER_SUPPLY_STOP_SINK);
				break;
			default:
				hwlog_info("%s can not detect typec state\r\n", __func__);
				break;
			}
			pd_dpm_set_typec_state(usb_event);

			if (typec_state->polarity)
				pd_dpm_set_cc_orientation(true);
			else
				pd_dpm_set_cc_orientation(false);

			if (notify_audio) {
				usb_analog_hs_plug_in_out_handle(event_id);
				ana_hs_plug_handle(event_id);
			}
		}
		break;

	case PD_DPM_PE_EVT_PD_STATE:
		{
			struct pd_dpm_pd_state *pd_state = data;
			switch (pd_state->connected) {
			case PD_CONNECT_PE_READY_SNK:
			case PD_CONNECT_PE_READY_SRC:
				break;
			default:
				break;
			}
		}
		break;

	case PD_DPM_PE_EVT_DIS_VBUS_CTRL:
		hwlog_info("%s : Disable VBUS Control  first \n", __func__);

		if ((g_pd_di->pd_finish_flag == true) || pd_dpm_get_pd_source_vbus()) {
			struct pd_dpm_vbus_state vbus_state;
			hwlog_info("%s : Disable VBUS Control\n", __func__);
			vbus_state.mv = 0;
			vbus_state.ma = 0;
			if (g_otg_channel && g_pd_di->pd_source_vbus) {
				vbus_ch_close(VBUS_CH_USER_PD,
					VBUS_CH_TYPE_BOOST_GPIO,
					true, false);
#ifdef CONFIG_POGO_PIN
			} else if (pogopin_is_support() &&
				(pogopin_otg_from_buckboost() == true) &&
				g_pd_di->pd_source_vbus) {
				vbus_ch_close(VBUS_CH_USER_PD,
					VBUS_CH_TYPE_POGOPIN_BOOST,
					true, false);
#endif /* CONFIG_POGO_PIN */
			} else {
				pd_dpm_vbus_notifier_call(g_pd_di, CHARGER_TYPE_NONE, &vbus_state);
				if (g_pd_di->pd_source_vbus)
					pd_dpm_set_source_sink_state(POWER_SUPPLY_STOP_SOURCE);
				else
					pd_dpm_set_source_sink_state(POWER_SUPPLY_STOP_SINK);
			}

			mutex_lock(&g_pd_di->sink_vbus_lock);
			if (g_pd_di->bc12_event != CHARGER_TYPE_NONE)
				ignore_bc12_event_when_vbusoff = true;

			ignore_bc12_event_when_vbuson = false;
			g_pd_di->pd_source_vbus = false;
			g_pd_di->pd_finish_flag = false;
			usb_audio_power_scharger();
			usb_headset_plug_out();
			restore_headset_state();
			dig_hs_plug_out();
			mutex_unlock(&g_pd_di->sink_vbus_lock);
		}
		break;

	case PD_DPM_PE_EVT_SINK_VBUS:
		{
			pd_dpm_report_pd_sink_vbus(g_pd_di, data);
		}
		break;

	case PD_DPM_PE_EVT_SOURCE_VBUS:
		{
			pd_dpm_report_pd_source_vbus(g_pd_di, data);
		}
		break;

	case PD_DPM_PE_EVT_SOURCE_VCONN:
		{
			pd_dpm_report_pd_source_vconn(data);
			break;
		}
	case PD_DPM_PE_EVT_DR_SWAP:
		{
			struct pd_dpm_swap_state *swap_state = data;
			if (swap_state->new_role == PD_ROLE_DFP)
				usb_event = PD_DPM_USB_TYPEC_HOST_ATTACHED;
			else
				usb_event = PD_DPM_USB_TYPEC_DEVICE_ATTACHED;
		}
		pd_dpm_set_typec_state(usb_event);
		break;

	case PD_DPM_PE_EVT_PR_SWAP:
		uem_handle_pr_swap_end();
		break;
	case PD_DPM_PE_CABLE_VDO:
		if (g_cable_state.support_cable == 1) {
			g_cable_state.cable_fail = PD_DPM_CABLE_INIT;
			cable_cur = pd_dpm_set_cable_vdo_auth(data);
			hwlog_info("%s cable_cur is %d", __func__, cable_cur);
			if (cable_cur != 0) {
				/* The current  capacity of this cable is biger than 5A */
				pd_dpm_start_cable_auth();
				break;
			}
		}
		pd_dpm_set_cable_vdo(data);
		emark_detect_complete();
		break;
	case PD_DPM_PE_CABLE_DATA:
		hwlog_info("PD_DPM_PE_CABLE_DATA enter");
		complete(&g_cable_state.emark_detect_cable_data);
		pd_dpm_set_cable_data_handle(data);
		break;
	default:
		hwlog_info("%s  unkonw event \r\n", __func__);
		break;
	};

	if (usb_event != PD_DPM_USB_TYPEC_NONE) {
		mutex_lock(&g_pd_di->usb_lock);
		if (g_pd_di->pending_usb_event != usb_event) {
			cancel_delayed_work(&g_pd_di->usb_state_update_work);
			g_pd_di->pending_usb_event = usb_event;
			queue_delayed_work(g_pd_di->usb_wq,
				&g_pd_di->usb_state_update_work,
				msecs_to_jiffies(0));
		} else {
			pr_info("Pending event is same --> ignore this event %d\n", usb_event);
		}
		mutex_unlock(&g_pd_di->usb_lock);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(pd_dpm_handle_pe_event);

int pd_dpm_get_otg_channel(void)
{
	if (pogopin_is_support() && (pogopin_otg_from_buckboost() == true))
		return POGOPIN_OTG_CHANNEL;

	return g_otg_channel;
}

#ifdef CONFIG_CONTEXTHUB_PD
static int g_iput;
static int g_iget;
static int n;
struct pd_dpm_combphy_event combphy_notify_event_buffer[COMBPHY_MAX_PD_EVENT_COUNT];
static int addring (int i)
{
	return ((i + 1) == COMBPHY_MAX_PD_EVENT_COUNT) ? 0 : (i + 1);
}
static void pd_dpm_init_combphy_notify_event_buffer(void)
{
	int i = 0;
	for (i = 0; i < COMBPHY_MAX_PD_EVENT_COUNT; i++) {
		combphy_notify_event_buffer[i].irq_type = COMBPHY_PD_EVENT_INVALID_VAL;
		combphy_notify_event_buffer[i].mode_type = COMBPHY_PD_EVENT_INVALID_VAL;
		combphy_notify_event_buffer[i].dev_type = COMBPHY_PD_EVENT_INVALID_VAL;
		combphy_notify_event_buffer[i].typec_orien = COMBPHY_PD_EVENT_INVALID_VAL;
	}
}
static void pd_dpm_combphy_notify_event_copy(struct pd_dpm_combphy_event *dst_event,
	struct pd_dpm_combphy_event src_event)
{
	(*dst_event).dev_type = src_event.dev_type;
	(*dst_event).irq_type = src_event.irq_type;
	(*dst_event).mode_type = src_event.mode_type;
	(*dst_event).typec_orien = src_event.typec_orien;
}
static void pd_dpm_print_buffer(int idx)
{
#ifdef COMBPHY_NOTIFY_BUFFER_PRINT
	hwlog_info("\n+++++++++++++++++++++++++++++++++\n");
	hwlog_info("\nbuffer[%d].irq_type %d\n", idx,
		combphy_notify_event_buffer[idx].irq_type);
	hwlog_info("\nbuffer[%d].mode_type %d\n", idx,
		combphy_notify_event_buffer[idx].mode_type);
	hwlog_info("\nbuffer[%d].dev_type %d\n", idx,
		combphy_notify_event_buffer[idx].dev_type);
	hwlog_info("\nbuffer[%d].typec_orien %d\n", idx,
		combphy_notify_event_buffer[idx].typec_orien);
	hwlog_info("\n+++++++++++++++++++++++++++++++++\n");
#endif /* COMBPHY_NOTIFY_BUFFER_PRINT */
}
static int pd_dpm_put_combphy_pd_event(struct pd_dpm_combphy_event event)
{
	if (n < COMBPHY_MAX_PD_EVENT_COUNT) {
		pd_dpm_combphy_notify_event_copy(&(combphy_notify_event_buffer[g_iput]), event);
		pd_dpm_print_buffer(g_iput);
		g_iput = addring(g_iput);
		n++;
		hwlog_info("%s - input = %d, n = %d \n", __func__, g_iput, n);
		return 0;
	} else {
		hwlog_info("%s Buffer is full\n", __func__);
		return -1;
	}
}

int pd_dpm_get_pd_event_num(void)
{
	return n;
}

static int pd_dpm_get_combphy_pd_event(struct pd_dpm_combphy_event *event)
{
	int pos;
	if (n > 0) {
		pos = g_iget;
		g_iget = addring(g_iget);
		n--;
		pd_dpm_combphy_notify_event_copy(event, combphy_notify_event_buffer[pos]);
		pd_dpm_print_buffer(pos);
		hwlog_info("%s - g_iget = %d, n = %d \n", __func__, g_iget, n);
	} else {
		hwlog_info("%s Buffer is empty\n", __func__);
		return -1;
	}
	return n;
}
void dp_dfp_u_notify_dp_configuration_done(TCPC_MUX_CTRL_TYPE mode_type, int ack)
{
	g_ack = ack;
	hwlog_info("%s ret = %d \n", __func__, g_ack);
	complete(&pd_dpm_combphy_configdone_completion);
}

static void pd_dpm_handle_ldo_supply_ctrl(struct pd_dpm_combphy_event event, bool enable)
{
	if ((event.mode_type != TCPC_NC) && (enable == true))
		hw_usb_ldo_supply_enable(HW_USB_LDO_CTRL_COMBOPHY);

	if ((g_ack == 0) && (event.mode_type == TCPC_NC) && (enable == false))
		hw_usb_ldo_supply_disable(HW_USB_LDO_CTRL_COMBOPHY);
}


static void pd_dpm_combphy_event_notify(struct work_struct *work)
{
	int ret = 0;
	int event_count = 0;
	int busy_count = 10;
	struct pd_dpm_combphy_event event;
	hwlog_info("%s +\n", __func__);
	do {
		mutex_lock(&g_pd_di->pd_combphy_notify_lock);
		event_count = pd_dpm_get_combphy_pd_event(&event);
		mutex_unlock(&g_pd_di->pd_combphy_notify_lock);

		if (event_count < 0)
			break;

		if (!support_dp && ((event.dev_type == TCA_DP_OUT) ||
			(event.dev_type == TCA_DP_IN)))
			continue;

		if ((event.irq_type == COMBPHY_PD_EVENT_INVALID_VAL) ||
			(event.mode_type == COMBPHY_PD_EVENT_INVALID_VAL) ||
			(event.dev_type == COMBPHY_PD_EVENT_INVALID_VAL) ||
			(event.typec_orien == COMBPHY_PD_EVENT_INVALID_VAL)) {
			hwlog_err("%s invalid val\n", __func__);
		} else {
			pd_dpm_handle_ldo_supply_ctrl(event, true);

			ret = pd_event_notify(event.irq_type, event.mode_type,
				event.dev_type, event.typec_orien);
			pd_dpm_wait_combphy_configdone();

			if (g_ack == -EBUSY) {
				do {
					mdelay(100); /* 100: delay 100ms */
					busy_count--;
					ret = pd_event_notify(event.irq_type, event.mode_type,
						event.dev_type, event.typec_orien);
					pd_dpm_wait_combphy_configdone();
					if (g_ack != -EBUSY) {
						hwlog_info("%s %d exit busy succ\n", __func__, __LINE__);
						break;
					}
				} while (busy_count != 0);
				if (busy_count == 0)
					hwlog_err("%s %d BUSY!\n", __func__, __LINE__);
			}
			pd_dpm_handle_ldo_supply_ctrl(event, false);
		}
	} while (event_count);
	hwlog_info("%s -\n", __func__);
}
static bool pd_dpm_combphy_notify_event_compare(struct pd_dpm_combphy_event eventa,
	struct pd_dpm_combphy_event eventb)
{
	return ((eventa.dev_type == eventb.dev_type) &&
		(eventa.irq_type == eventb.irq_type) &&
		(eventa.mode_type == eventb.mode_type));
}
int pd_dpm_handle_combphy_event(struct pd_dpm_combphy_event event)
{
	int ret = 0;

	hwlog_info("%s +\n", __func__);
	mutex_lock(&g_pd_di->pd_combphy_notify_lock);
	if ((g_pd_di->last_combphy_notify_event.dev_type == TCA_DP_IN) ||
		(g_pd_di->last_combphy_notify_event.dev_type == TCA_DP_OUT) ||
		(g_pd_di->last_combphy_notify_event.dev_type == TCA_ID_FALL_EVENT)) {
		if ((event.dev_type == TCA_CHARGER_CONNECT_EVENT) ||
			(event.dev_type == TCA_CHARGER_DISCONNECT_EVENT)) {
			hwlog_info("%s invlid event sequence\n", __func__);
			mutex_unlock(&g_pd_di->pd_combphy_notify_lock);
			return -1;
		}
	}
	if ((g_pd_di->last_combphy_notify_event.mode_type ==
		TCPC_NC) && (event.mode_type == TCPC_NC)) {
		hwlog_info("%s repeated TCPC_NC event\n", __func__);
		mutex_unlock(&g_pd_di->pd_combphy_notify_lock);
		return -1;
	}

	if (!pd_dpm_combphy_notify_event_compare(g_pd_di->last_combphy_notify_event,
		event)) {
		pd_dpm_combphy_notify_event_copy(&(g_pd_di->last_combphy_notify_event), event);
		ret = pd_dpm_put_combphy_pd_event(event);

		if (ret < 0) {
			hwlog_err("%s pd_dpm_put_combphy_pd_event fail\n", __func__);
			mutex_unlock(&g_pd_di->pd_combphy_notify_lock);
			return -1;
		}

		queue_delayed_work(g_pd_di->pd_combphy_wq,
			&g_pd_di->pd_combphy_event_work,
			msecs_to_jiffies(0));
	} else {
		hwlog_info("%s Pending event is same --> ignore this event\n", __func__);
	}
	mutex_unlock(&g_pd_di->pd_combphy_notify_lock);
	hwlog_info("%s -\n", __func__);
	return 0;
}
#endif /* CONFIG_CONTEXTHUB_PD */

static void pd_cc_protection_dts_parser(struct pd_dpm_info *di,
	struct device_node *np)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"abnormal_cc_detection", &abnormal_cc_detection, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"cc_dynamic_protect", &cc_dynamic_protect, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"abnormal_cc_interval", &abnormal_cc_interval, PD_DPM_CC_CHANGE_INTERVAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"moisture_detection_by_cc_enable", &moisture_detection_by_cc_enable, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"cc_moisture_status_report", &cc_moisture_status_report, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"cc_abnormal_dmd_report_enable", &g_cc_abnormal_dmd_report_enable, 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"external_pd_flag", &external_pd_flag, 0);
}

static void pd_misc_dts_parser(struct pd_dpm_info *di, struct device_node *np)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"switch_manual_enable", &switch_manual_enable, 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_dp", &support_dp, 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"otg_channel", &g_otg_channel, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ibus_check", &g_ibus_check, 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vbus_only_min_vlotage", &g_vbus_only_min_voltage, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_cable", &g_cable_state.support_cable, 0);
}

static void pd_accessory_dts_parser(struct pd_dpm_info *di, struct device_node *np)
{
#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_smart_holder", &support_smart_holder, 0);
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC2 */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_analog_audio", &support_analog_audio, 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"emark_detect_enable", &emark_detect_enable, 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"emark_detect_finish_not_support", &emark_detect_finish_not_support, 0);
	if (power_dts_read_string(power_dts_tag(HWLOG_TAG), np,
		"tcp_name", &di->tcpc_name))
		di->tcpc_name = "type_c_port0";
}

static int pd_reinit_dts_parser(struct pd_dpm_info *di, struct device_node *np)
{
	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"pd_reinit_enable", &di->pd_reinit_enable, 0))
		return -EINVAL;

	if (!di->pd_reinit_enable) {
		hwlog_err("pd reinit not enable\n");
		return -EINVAL;
	}

	if (power_dts_read_string(power_dts_tag(HWLOG_TAG), np,
		"ldo_name", &di->ldo_name))
		return -EINVAL;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_ocp_count", &di->max_ocp_count, PD_DPM_MAX_OCP_COUNT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ocp_delay_time", &di->ocp_delay_time, OCP_DELAY_TIME);

	return 0;
}

static void pd_dpm_parse_dts(struct pd_dpm_info *di, struct device_node *np)
{
	if (!di || !np) {
		hwlog_err("di or np is null\n");
		return;
	}

	pd_cc_protection_dts_parser(di, np);
	pd_misc_dts_parser(di, np);
	pd_accessory_dts_parser(di, np);
	hwlog_info("parse_dts success\n");
}

static void pd_reinit_process(struct pd_dpm_info *di, struct device_node *np)
{
	if (pd_reinit_dts_parser(di, np))
		return;

	di->pd_vdd_ldo = devm_regulator_get(di->dev, "pd_vdd");
	if (IS_ERR(di->pd_vdd_ldo)) {
		hwlog_err("get pd vdd ldo failed\n");
		return;
	}

	(void)regulator_enable(di->pd_vdd_ldo);
	di->vdd_ocp_lock = wakeup_source_register(di->dev, "vdd_ocp_lock");
	if (!di->vdd_ocp_lock) {
		hwlog_err("%s wakeup source register failed\n", __func__);
		return;
	}

	INIT_DELAYED_WORK(&di->reinit_pd_work, pd_dpm_reinit_pd_work);
	di->ocp_nb.notifier_call = pd_dpm_ocp_nb_call;
	pmic_mntn_register_notifier(&di->ocp_nb);
}

static void pd_dpm_init_default_value(struct pd_dpm_info *di)
{
	g_charger_type_event = chip_get_charger_type();
	di->last_usb_event = PD_DPM_USB_TYPEC_NONE;
	di->pending_usb_event = PD_DPM_USB_TYPEC_NONE;
}

static void pd_dpm_init_resource(struct pd_dpm_info *di)
{
	int ret;

	mutex_init(&di->sink_vbus_lock);
	mutex_init(&di->usb_lock);
	mutex_init(&typec_state_lock);
	mutex_init(&typec_wait_lock);

	BLOCKING_INIT_NOTIFIER_HEAD(&di->pd_evt_nh);
	BLOCKING_INIT_NOTIFIER_HEAD(&di->pd_port_status_nh);
	ATOMIC_INIT_NOTIFIER_HEAD(&di->pd_wake_unlock_evt_nh);

	di->usb_nb.notifier_call = pd_dpm_report_bc12;
	ret = chip_charger_type_notifier_register(&di->usb_nb);
	if (ret < 0)
		hwlog_err("chip_charger_type_notifier_register failed\n");

	di->chrg_wake_unlock_nb.notifier_call = charge_wake_unlock_notifier_call;
#ifdef CONFIG_HUAWEI_CHARGER_AP
	(void)power_event_bnc_register(POWER_BNT_CHG, &di->chrg_wake_unlock_nb);
#endif
	typec_dev = power_sysfs_create_group("hw_typec", "typec", &pd_dpm_attr_group);

#ifdef CONFIG_CONTEXTHUB_PD
	mutex_init(&di->pd_combphy_notify_lock);
	pd_dpm_init_combphy_notify_event_buffer();
	di->pd_combphy_wq = create_workqueue("pd_combphy_event_notify_workque");
	INIT_DELAYED_WORK(&di->pd_combphy_event_work, pd_dpm_combphy_event_notify);
	init_completion(&pd_dpm_combphy_configdone_completion);
#endif /* CONFIG_CONTEXTHUB_PD */
	di->usb_wq = create_workqueue("pd_dpm_usb_wq");
	INIT_DELAYED_WORK(&di->usb_state_update_work, pd_dpm_usb_update_state);
	INIT_DELAYED_WORK(&di->cc_moisture_flag_restore_work,
		pd_dpm_cc_moisture_flag_restore);
	INIT_DELAYED_WORK(&cc_short_work, pd_dpm_cc_short_work);
	init_completion(&pd_get_typec_state_completion);
	/* use for board rt test */
	INIT_DELAYED_WORK(&di->otg_restore_work, pd_dpm_otg_restore_work);
	INIT_DELAYED_WORK(&di->hw_pd_get_key_work, pd_dpm_get_key_work);
	INIT_DELAYED_WORK(&di->hw_pd_get_cable_data_work, pd_dpm_get_cable_data_work);
	INIT_WORK(&g_pd_di->fb_work, fb_unblank_work);

	if (abnormal_cc_detection)
		init_fb_notification();
}

static int pd_set_rtb_success(unsigned int val)
{
	struct pd_dpm_info *di = g_pd_di;
	int typec_state = PD_DPM_USB_TYPEC_NONE;

	/* 1:set board running test result success */
	if (!di || (val <= 0))
		return -1;

	if (!power_cmdline_is_factory_mode())
		return 0;

	pd_dpm_get_typec_state(&typec_state);
	if ((typec_state == PD_DPM_USB_TYPEC_HOST_ATTACHED) ||
		(typec_state == PD_DPM_USB_TYPEC_DEVICE_ATTACHED) ||
		(typec_state == PD_DPM_USB_TYPEC_AUDIO_ATTACHED)) {
		hwlog_err("typec port is attached, can not open vbus\n");
		return -1;
	}

	pd_dpm_vbus_ch_open(di);
	cancel_delayed_work_sync(&di->otg_restore_work);
	/* 1000: delay val*1000 ms */
	schedule_delayed_work(&di->otg_restore_work,
		msecs_to_jiffies(val * 1000));
	return 0;
}

static struct power_if_ops pd_if_ops = {
	.set_rtb_success = pd_set_rtb_success,
	.type_name = "pd",
};

struct pd_dpm_info *get_pd_dpm_info()
{
	return g_pd_di;
}

void set_pd_dpm_info_flag(bool pd_finish_flag, bool pd_source_vbus)
{
	g_pd_di->pd_source_vbus = pd_source_vbus;
	g_pd_di->pd_finish_flag = pd_finish_flag;
}

unsigned int get_abnormal_cc_detection()
{
	return abnormal_cc_detection;
}

static int pd_dpm_probe(struct platform_device *pdev)
{
	struct pd_dpm_info *di = NULL;
	struct device_node *np = NULL;
	enum chip_charger_type type;
	int ret;

	hwlog_info("%s begin\n", __func__);
	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = di->dev->of_node;
	g_pd_di = di;

	init_completion(&g_cable_state.emark_detect_cable_data);
	init_completion(&g_cable_state.emark_get_key_value);
	platform_set_drvdata(pdev, di);
	pd_dpm_init_default_value(di);
	pd_dpm_parse_dts(di, np);
	pd_dpm_init_resource(di);
#ifdef CONFIG_RT_USB_PD
	// notify_tcp_dev_ready(di->tcpc_name);
#endif /* CONFIG_RT_USB_PD */

	type = chip_get_charger_type();
	di->bc12_event = type;
	dc_cable_ops_register(&cable_detect_ops);
	pd_reinit_process(di, np);
	power_if_ops_register(&pd_if_ops);

	di->wired_rechrg_nb.notifier_call = wired_rechrg_event_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_REVERSE_CHARGE, &di->wired_rechrg_nb);
	if (ret) {
		hwlog_err("%s register POWER_BNT_REVERSE_CHARGE failed\n", __func__);
		goto fail_free_mem;
	}

	hwlog_info("%s end\n", __func__);
	return 0;

fail_free_mem:
    power_event_bnc_unregister(POWER_BNT_REVERSE_CHARGE, &di->wired_rechrg_nb);
    kfree(di);
    return ret;
}

static const struct of_device_id pd_dpm_callback_match_table[] = {
	{
		.compatible = "huawei,pd_dpm",
		.data = NULL,
	},
	{},
};

static struct platform_driver pd_dpm_callback_driver = {
	.probe = pd_dpm_probe,
	.remove = NULL,
	.driver = {
		.name = "huawei,pd_dpm",
		.owner = THIS_MODULE,
		.of_match_table = pd_dpm_callback_match_table,
	}
};

static int __init pd_dpm_init(void)
{
	return platform_driver_register(&pd_dpm_callback_driver);
}

static void __exit pd_dpm_exit(void)
{
	platform_driver_unregister(&pd_dpm_callback_driver);
}

module_init(pd_dpm_init);
module_exit(pd_dpm_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("pd dpm logic driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
