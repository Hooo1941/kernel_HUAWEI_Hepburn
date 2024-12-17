/*
 * Huawei Touchscreen Driver
 *
 * Copyright (c) 2012-2050 Huawei Technologies Co., Ltd.
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/spi/spi.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/notifier.h>
#include <linux/fb.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#ifdef CONFIG_HISYSEVENT
#include <dfx/hiview_hisysevent.h>
#endif
#ifdef CONFIG_HUAWEI_THP_MTK
#include <linux/of_irq.h>
#endif
#include <linux/regulator/consumer.h>
#include <linux/hwspinlock.h>
#include <linux/kthread.h>
#include <hwspinlock_internal.h>
#include <linux/kconfig.h>
#include "huawei_pen_clk.h"

#if ((defined CONFIG_HUAWEI_THP_QCOM) && (!defined CONFIG_LCD_KIT_DRIVER))
#include "dsi_panel.h"
#endif

#if ((!defined CONFIG_LCD_KIT_DRIVER) && (!defined CONFIG_HUAWEI_THP_UDP))
#include "../../lcdkit/lcdkit1.0/core/common/lcdkit_tp.h"
#endif
#if (!defined CONFIG_HUAWEI_THP_QCOM) && (!defined CONFIG_HUAWEI_THP_MTK)
#include "../../lcdkit/lcdkit1.0/include/lcdkit_ext.h"
#endif

#include "huawei_thp.h"
#include "huawei_thp_mt_wrapper.h"
#include "huawei_thp_attr.h"
#ifdef CONFIG_HUAWEI_SHB_THP
#include "huawei_thp_log.h"
#endif

#ifdef CONFIG_LCD_KIT_DRIVER
#ifdef CONFIG_MODULE_KO_TP
#include "huawei_thp_api.h"
#else
#ifdef CONFIG_HUAWEI_THP_QCOM
#include "../../lcdkit/lcdkit3.0/kernel/qcom/common/include/lcd_kit_core.h"
#else
#include "../../lcdkit/lcdkit3.0/kernel/common/include/lcd_kit_core.h"
#endif
#endif
#endif

#ifdef CONFIG_INPUTHUB_20
#include "contexthub_recovery.h"
#endif

#ifdef CONFIG_HUAWEI_PS_SENSOR
#include "ps_sensor.h"
#endif

#ifdef CONFIG_HUAWEI_SENSORS_2_0
#include "sensor_scp.h"
#endif

#if (!defined CONFIG_HUAWEI_THP_QCOM) && (!defined CONFIG_HUAWEI_THP_MTK)
#include <huawei_platform/sensor/hw_comm_pmic.h>
#endif

#ifdef CONFIG_BCI_BATTERY
#if (defined(CONFIG_HUAWEI_THP_MTK) || defined(CONFIG_HUAWEI_THP_QCOM))
#include <linux/power/platform/bci_battery.h>
#else
#include <platform_include/basicplatform/linux/power/platform/bci_battery.h>
#endif
#endif

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#if (defined(CONFIG_HUAWEI_THP_MTK) || defined(CONFIG_HUAWEI_THP_QCOM))
#include <hwmanufac/dev_detect/dev_detect.h>
#else
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#endif

#if defined(CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
#endif

#if defined(CONFIG_TEE_TUI)
#include "tui.h"
#endif
#if ((defined CONFIG_HUAWEI_THP_MTK) || (defined CONFIG_HUAWEI_THP_QCOM))
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#endif
#if defined(CONFIG_HUAWEI_TS_KIT_3_0)
#include "../3_0/trace-events-touch.h"
#else
#define trace_touch(x...)
#endif

#if (defined(CONFIG_HUAWEI_THP_QCOM) || defined (CONFIG_HUAWEI_THP_UDP))
int g_tskit_pt_station_flag;
#endif

struct mutex thp_power_mutex;
struct mutex polymeric_chip_mutex;
struct mutex tui_mutex;
struct mutex penclk_mutex;
struct thp_core_data *g_thp_core[TOTAL_PANEL_NUM];
static u8 is_fw_update;
static int spi_sync_count_in_suspend;
static int get_frame_count_in_suspend;
static atomic_t count_vote_for_polymeric_chip;

static int thp_unregister_ic_num;

static u8 *g_queue_data_buf;
static unsigned int g_thp_queue_node_len = THP_QUEUE_NODE_BUFF_MAX_LEN;

static bool g_thp_prox_enable;
static bool onetime_poweroff_done;

static int g_daemon_reset_count = 0;
static bool g_is_boot_proc = true;
#define DAEMON_RESET_MAX_COUNT 3

#if (!defined CONFIG_HUAWEI_THP_QCOM) && (!defined CONFIG_HUAWEI_THP_MTK)
static struct hw_comm_pmic_cfg_t tp_pmic_ldo_set;
#endif

#define TEMP_OPEN_DEBUG_LOG 2
#define DISABLE_DEBUG_LOG 0
#define MAX_SUPPORT_DEBUG_IRQ_NUM 5
#define HIGH_PERFORMANCE_STATE 1

#define TAS_EVENT_TO_UDFP_MODE_ONE 1
#define TAS_EVENT_TO_UDFP_MODE_TWO 2
#define SWITCH_ENABLE_LENGTH 1

static int irq_count_for_debug;
extern unsigned int get_runmode_type(void);

extern int power_event_bnc_register(unsigned int type, struct notifier_block *nb);
extern int power_event_bnc_unregister(unsigned int type, struct notifier_block *nb);

static unsigned int lcd_panel_index;
static unsigned int lcd_product_type;

static int touch_driver_power_on_for_seq0(struct thp_device *tdev);
static int touch_driver_power_off_for_seq0(struct thp_device *tdev);

struct common_power_control g_thp_common_power[] = {
	{
		.power_on_flag = 0,
		.power_on = touch_driver_power_on_for_seq0,
		.power_off = touch_driver_power_off_for_seq0,
	},
};

struct common_power_control *get_common_power_control(u8 power_sequence)
{
	if (power_sequence >= ARRAY_SIZE(g_thp_common_power)) {
		thp_log_err("%s: invalid power_sequence, use default 0\n");
		return &g_thp_common_power[0];
	}
	return &g_thp_common_power[power_sequence];
}

static char *misc_device_name[TOTAL_PANEL_NUM] = {
	"thp0",
	"thp1"
};

#ifdef CONFIG_HISYSEVENT
struct hisysevent_info g_thp_hisysevent_info_list[] = {
	{ "THP_DEV_STATUS_ERROR", THP_STATISTIC },
	{ "THP_DEV_PROBE", THP_STATISTIC }
};
#endif

#if defined(CONFIG_TEE_TUI)
struct thp_tui_data thp_tui_info;
EXPORT_SYMBOL(thp_tui_info);

struct multi_tp_tui_data tp_tui_data[MAX_SUPPORT_TP_TUI_NUM];
#endif
#if defined(CONFIG_HUAWEI_DSM)
#define THP_CHIP_DMD_REPORT_SIZE 1024
#define THP_DSM_BUFF_SIZE 4096

static struct dsm_dev dsm_thp = {
	.name = "dsm_tphostprocessing",
	.device_name = "TPHOSTPROCESSING",
	.ic_name = "syn",
	.module_name = "NNN",
	.fops = NULL,
	.buff_size = THP_DSM_BUFF_SIZE,
};

struct dsm_client *dsm_thp_dclient;

#define PRINTF_ERROR_CODE "printf error\n"
void thp_dmd_report(int dmd_num, const char *psz_format, ...)
{
	va_list args;
	char *input_buf = NULL;
	int ret;
	int buf_length;
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);

	if ((!cd) || (!dsm_thp_dclient)) {
		thp_log_err("cd or dsm_thp_dclient is NULL\n");
		return;
	}
	if (!psz_format) {
		thp_log_err("psz_format is NULL\n");
		return;
	}

	input_buf = kzalloc(THP_CHIP_DMD_REPORT_SIZE, GFP_KERNEL);
	if (!input_buf) {
		thp_log_err("%s: kzalloc failed!\n", __func__);
		return;
	}

	va_start(args, psz_format);
	ret = vsnprintf(input_buf, THP_CHIP_DMD_REPORT_SIZE, psz_format, args);
	if (ret < 0) {
		thp_log_err("vsnprintf failed, ret: %d\n", ret);
		strcpy(input_buf, PRINTF_ERROR_CODE);
	}
	va_end(args);
	buf_length = strlen(input_buf);
	ret = snprintf((input_buf + buf_length),
		(THP_CHIP_DMD_REPORT_SIZE - buf_length),
		"irq_gpio:%d\tvalue:%d\nreset_gpio:%d\tvalue:%d\nTHP_status:%u\nprojectid:%s\n",
		cd->gpios.irq_gpio,
		gpio_get_value(cd->gpios.irq_gpio),
		cd->gpios.rst_gpio,
		gpio_get_value(cd->gpios.rst_gpio),
		thp_get_status_all(cd->panel_id),
		cd->project_id);
	if (ret < 0)
		thp_log_err("snprintf failed, ret: %d\n", ret);

	if (!dsm_client_ocuppy(dsm_thp_dclient)) {
		dsm_client_record(dsm_thp_dclient, input_buf);
		dsm_client_notify(dsm_thp_dclient, dmd_num);
		thp_log_info("%s %s\n", __func__, input_buf);
	}

	kfree(input_buf);
}
EXPORT_SYMBOL(thp_dmd_report);
#endif

#ifdef CONFIG_HISYSEVENT
/*
 * name:esd_hiview_hisysevent
 * function:big data dotting by hisysevent
 * @domain:event domain
 * @casename:event name
 * @status:event status
 */
void esd_hiview_hisysevent(const char *domain, const char *casename, int status)
{
	struct hiview_hisysevent *event = NULL;
	int ret;

	event = hisysevent_create(domain, casename, STATISTIC);
	if (!event) {
		thp_log_err("failed to create event\n");
		return;
	}

	ret = hisysevent_put_integer(event, "STATUS", status);
	if (ret != 0) {
		thp_log_err("failed to put value to event, ret=%d\n", ret);
		goto hisysevent_end;
	}

	ret = hisysevent_write(event);
	if (ret != 0)
		thp_log_err("lcd_kit_hiview_hisysevent fail, %s\n", casename);

hisysevent_end:
	hisysevent_destroy(&event);
}

#define THP_HISYSEVENT_REPORT_SIZE 1024
static enum case_type get_hisysevent_type(const char *casename)
{
	int len;
	int i;

	if (!casename) {
		thp_log_err("casename is null!\n");
		return THP_INVALID_TYPE;
	}

	len = sizeof(g_thp_hisysevent_info_list) / sizeof(struct hisysevent_info);
	for (i = 0; i < len; i++) {
		if (!strcmp(casename, g_thp_hisysevent_info_list[i].casename))
			return g_thp_hisysevent_info_list[i].type;
	}
	thp_log_err("%s not in g_thp_hisysevent_info_list\n", casename);
	return THP_INVALID_TYPE;
}

static void hiview_hisysevent_report(const char *casename,
	enum case_type type, char *message)
{
	const char *domain = "THPDRIVER";
	struct hiview_hisysevent *event;
	int ret;

	event = hisysevent_create(domain, casename, type);
	if (!event) {
		thp_log_err("failed to create event\n");
		return;
	}

	ret = hisysevent_put_string(event, "MSG", message);
	if (ret < 0) {
		thp_log_err("failed to put value to event, ret=%d\n", ret);
		goto hisysevent_end;
	}

	ret = hisysevent_write(event);
	if (ret < 0)
		thp_log_err("thp_hiview_hisysevent report fail, %s\n", casename);

hisysevent_end:
	hisysevent_destroy(&event);
}

void thp_hiview_hisysevent(const char *casename, const char *psz_format, ...)
{
	va_list args;
	int ret;
	enum case_type type;
	char *input_buf = NULL;
	int len;
	struct thp_core_data *cd = thp_get_core_data(get_active_panel());

	if (!cd) {
		thp_log_err("%s: cd is null!\n", __func__);
		return;
	}

	type = get_hisysevent_type(casename);
	if (type == THP_INVALID_TYPE)
		return;

	input_buf = vzalloc(THP_HISYSEVENT_REPORT_SIZE);
	if (!input_buf) {
		thp_log_err("%s: vzalloc failed!\n", __func__);
		return;
	}

	va_start(args, psz_format);
	ret = vsnprintf(input_buf, THP_HISYSEVENT_REPORT_SIZE, psz_format, args);
	if (ret < 0) {
		thp_log_err("vsnprintf failed, ret: %d\n", ret);
		va_end(args);
		vfree(input_buf);
		return;
	}
	va_end(args);

	len = strlen(input_buf);
	snprintf((input_buf + len), (THP_HISYSEVENT_REPORT_SIZE - len),
		"irq_gpio:%d value:%d reset_gpio:%d value:%d THP_status:%u projectid:%s ",
		cd->gpios.irq_gpio,
		gpio_get_value(cd->gpios.irq_gpio),
		cd->gpios.rst_gpio,
		gpio_get_value(cd->gpios.rst_gpio),
		thp_get_status_all(cd->panel_id),
		cd->project_id);

	hiview_hisysevent_report(casename, type, input_buf);
	vfree(input_buf);
}
#endif

#define PINCTRL_INIT_ENABLE "pinctrl_init_enable"

#define THP_DEVICE_NAME	"huawei_thp"
#define THP_MISC_DEVICE_NAME "thp"
#define IC_THP_BOE_SYNA_DW_ELS "P150921300"
#define IC_THP_BOE_SYNA_RM_ELS "P150921350"
#define MULTI_PANLE 1
#define SINGNAL_PANLE 0

/*
 * get app display status
 * @app_info
 * app_info contains NEED_CLOSE_STYLUS3, enter 144HZ frame rate.
 * app_info contains NEED_OPEN_STYLUS3, exit 144HZ frame rate.
 * otherwise, app_info is irrelevant to the 144 Hz frame rate.
 * @app_dsi_status
 * the app_dsi_sattus default value is 0.
 * app_dsi_status = 0, open stylus3 uplink
 * app_dsi_status = 1, close stylus3 uplink
 */
#define NEED_OPEN_STYLUS3 "#996#"
#define NEED_CLOSE_STYLUS3 "#997#"
static int get_app_dsi_status(struct thp_core_data *cd)
{
	int ret = -EINVAL;

	if (strstr(cd->app_info, NEED_OPEN_STYLUS3)) {
		atomic_set(&cd->app_dsi_status, 0);
		ret = NO_ERR;
	} else if (strstr(cd->app_info, NEED_CLOSE_STYLUS3)) {
		atomic_set(&cd->app_dsi_status, 1);
		ret = NO_ERR;
	}
	return ret;
}

/*
 * send open/close pen uplink command base on display frame rate
 * @ret
 * ret < 0, app info do not contains NEED_CLOSE_STYLUS3 or NEED_OPEN_STYLUS3.
 * ret = 0, app info contains NEED_CLOSE_STYLUS3 or NEED_OPEN_STYLUS3.
 * @status_check
 * if status_check = current app_dsi_status, frame rate do not change,
 * do not need send command.
 */
int app_send_stylus3_status_to_fw(struct thp_core_data *cd)
{
	int ret;
	static int status_check;

	ret = get_app_dsi_status(cd);
	if (ret) {
		thp_log_info("%s: app dsi status is invaled\n", __func__);
		return ret;
	}
	if (status_check == atomic_read(&cd->app_dsi_status)) {
		thp_log_info("%s app dsi status is not change\n", __func__);
		return ret;
	}
	status_check = atomic_read(&cd->app_dsi_status);
	if ((cd->send_bt_status_to_fw) &&
		(cd->thp_dev->ops->bt_handler))
		cd->thp_dev->ops->bt_handler(cd->thp_dev, false);
	return ret;
}

static void thp_get_lcd_product_type()
{
	struct device_node *lcd_panel_type_node = NULL;
	int rc;

	lcd_product_type = MULTI_PANLE;
	lcd_panel_index = MAIN_TOUCH_PANEL;

	lcd_panel_type_node = of_find_compatible_node(lcd_panel_type_node,
		NULL, "huawei,lcd_panel_type");
	if (!lcd_panel_type_node) {
		thp_log_info("%s:can't find lcd node\n", __func__);
		return;
	}

	rc = of_property_read_u32(lcd_panel_type_node, "product_type", &lcd_product_type);
	if (rc < 0) {
		thp_log_info("%s:get product_type failed\n", __func__);
		return;
	}

	rc = of_property_read_u32(lcd_panel_type_node, "panel_index", &lcd_panel_index);
	if (rc < 0) {
		thp_log_info("%s:get panel_index failed\n", __func__);
		return;
	}
	thp_log_info("%s:lcd_product_type = %u, lcd_panel_index = %u\n",
		__func__, lcd_product_type, lcd_panel_index);
	return;
}

#if (defined(CONFIG_HUAWEI_THP_MTK) || defined(CONFIG_HUAWEI_THP_QCOM))
struct ud_fp_ops *g_ud_fp_operations;
static void send_event_to_fingerprint_ud(struct thp_udfp_data udfp_data);
void fp_ops_register(struct ud_fp_ops *ops)
{
	if (g_ud_fp_operations == NULL) {
		g_ud_fp_operations = ops;
		thp_log_info("%s:g_ud_fp_ops register success\n", __func__);
	} else {
		thp_log_info("%s:g_ud_fp_ops has been registered\n", __func__);
	}
}
EXPORT_SYMBOL_GPL(fp_ops_register);


void fp_ops_unregister(struct ud_fp_ops *ops)
{
	if (g_ud_fp_operations != NULL) {
		g_ud_fp_operations = NULL;
		thp_log_info("%s:g_ud_fp_ops_unregister success\n", __func__);
	} else {
		thp_log_info("%s:g_ud_fp_ops has been unregister\n", __func__);
	}
}
EXPORT_SYMBOL_GPL(fp_ops_unregister);

struct ud_fp_ops *fp_get_ops(void)
{
	return g_ud_fp_operations;
}
#endif

void do_timetransfer(struct timespec64 *tv)
{
	ktime_get_real_ts64(tv);
	tv->tv_nsec = tv->tv_nsec / 1000;
}

#if defined(CONFIG_LCD_KIT_DRIVER)
int thp_power_control_notify(enum lcd_kit_ts_pm_type pm_type, int timeout);
static int thp_get_status_by_type(int type, int *status);
int thp_multi_power_control_notify(enum lcd_kit_ts_pm_type pm_type, int timeout,
	int panel_index);
static bool thp_get_afe_download_status(struct timespec64 *record_tv);
static int thp_send_esd_event(unsigned int status);
static int thp_provide_sn_to_lcd(char *sn_code, unsigned int len);
static void thp_multi_suspend(u32 panel_id);
static void thp_multi_resume(u32 panel_id);
static void tskit_multi_resume(void);
static void tskit_multi_suspend(void);
static void thp_suspend_thread(u32 panel_id);
static void thp_resume_thread(u32 panel_id);
static int thp_power_control(struct thp_core_data *cd,
	enum lcd_kit_ts_pm_type pm_type, int timeout);

struct ts_kit_ops thp_ops = {
	.ts_power_notify = thp_power_control_notify,
	.get_tp_proxmity = thp_get_prox_switch_status,
	.get_tp_status_by_type = thp_get_status_by_type,
	.ts_multi_power_notify = thp_multi_power_control_notify,
	.get_afe_status = thp_get_afe_download_status,
	.send_esd_event = thp_send_esd_event,
	.get_sn_code = thp_provide_sn_to_lcd,
};

static int thp_send_esd_event(unsigned int status)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);

	thp_log_info("%s: enter\n", __func__);

	if (cd == NULL) {
		thp_log_err("%s: cd is null\n", __func__);
		return -EINVAL;
	}
	if (cd->lcd_esd_event_upload == 0) {
		thp_log_info("%s: not support esd\n", __func__);
		return -EINVAL;
	}
	if (thp_mt_wrapper_esd_event(status, cd->panel_id)) {
		thp_log_err("%s: gpio detect fail\n", __func__);
		return -EINVAL;
	}
#if defined(CONFIG_HUAWEI_DSM)
	thp_dmd_report(DSM_LCD_ESD_STATUS_ERROR_NO,
		"%s, status value is %d\n",
		__func__, status);
#endif
#ifdef CONFIG_HISYSEVENT
	esd_hiview_hisysevent("ESD", "RECORD", status);
#endif

	return 0;
}

static bool thp_get_afe_download_status(struct timespec64 *record_tv)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);

	if ((record_tv == NULL) || (cd == NULL)) {
		thp_log_err("%s: record_tv or cd is null\n", __func__);
		return false;
	}

	if (cd->lcd_need_get_afe_status == 0) {
		thp_log_err("%s: lcd no need get afe status\n", __func__);
		return false;
	}

	if (cd->afe_download_status) {
		record_tv->tv_sec = cd->afe_status_record_tv.tv_sec;
		record_tv->tv_nsec = cd->afe_status_record_tv.tv_nsec;
	}
	return cd->afe_download_status;
}

static int thp_get_status_by_type(int type, int *status)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);

	if (status == NULL) {
		thp_log_err("status is null\n");
		return -EINVAL;
	}

	/*
	 * To avoid easy_wakeup_info.sleep_mode being changed
	 * during suspend, we assign cd->sleep_mode to
	 * easy_wakeup_info.sleep_mode in suspend.
	 * For TDDI, tp suspend must before lcd power off to
	 * make sure of easy_wakeup_info.sleep_mode be assigned.
	 */
	if ((type == TS_GESTURE_FUNCTION) &&
		(cd->easy_wakeup_info.sleep_mode == TS_GESTURE_MODE) &&
		cd->support_gesture_mode &&
		cd->lcd_gesture_mode_support) { /* TDDI need this */
		*status = GESTURE_MODE_OPEN;
		return NO_ERR;
	}

	return -EINVAL;
}

static int thp_provide_sn_to_lcd(char *sn_code, unsigned int len)
{
	int para_check_flag;
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);

	para_check_flag = (len > OEM_INFO_DATA_LENGTH) || (!sn_code) || (!cd);
	if (para_check_flag) {
		thp_log_err("%s invalid parameter\n", __func__);
		return -EINVAL;
	}

	if (cd->support_oem_info != THP_OEM_INFO_READ_FROM_TP) {
		thp_log_info("%s: not support read sn from tp\n", __func__);
		return -EINVAL;
	}
	strncpy(sn_code, cd->oem_info_data, len);
	return 0;
}

static int thp_sn_code_init(struct thp_core_data *cd)
{
	int ret;

	if (cd->support_oem_info != THP_OEM_INFO_READ_FROM_TP) {
		thp_log_info("%s: not support read sn from tp\n", __func__);
		return -EINVAL;
	}

	/* length of oem_info_data is 32 and THP_SN_CODE_LEN is 23 */
	if (cd->thp_dev->ops->get_sn_code) {
		ret = cd->thp_dev->ops->get_sn_code(cd->thp_dev,
			cd->oem_info_data, THP_SN_CODE_LEN);
	} else {
		thp_log_err("%s: get_sn_code is null\n", __func__);
		return -EINVAL;
	}

	if (ret) {
		thp_log_err("%s:get sn_code failed\n", __func__);
		return ret;
	}
	cd->oem_info_data[THP_SN_CODE_LEN] = '\0';

	return ret;
}
#endif

void thp_switch_shb_active_panel(u32 panel_id)
{
#ifdef CONFIG_HUAWEI_SHB_THP
	int ret;

	ret = send_thp_close_cmd();
	ret += send_active_panel_id(panel_id);
	ret += send_thp_open_cmd();
	if (ret < 0)
		thp_log_err("%s: ret = %d\n", __func__, ret);
#endif
}

#ifdef CONFIG_HUAWEI_SHB_THP
int thp_send_sync_info_to_ap(const char *head)
{
	struct thp_sync_info *rx = NULL;

	if (head == NULL) {
		thp_log_err("thp_sync_info, data is NULL\n");
		return -EINVAL;
	}
	rx = (struct thp_sync_info *)head;
	/* here we do something sending data to afe hal */
	thp_log_info("thp_sync_info, len:%d\n", rx->size);

	return 0;
}

static long thp_check_single_click(struct thp_core_data *cd)
{
	return cd->single_click_switch;
}

static long thp_check_double_click(struct thp_core_data *cd)
{
	return cd->double_click_switch;
}

static long thp_check_stylus_click(struct thp_core_data *cd)
{
	return thp_get_status(THP_STATUS_STYLUS, cd->panel_id) || cd->suspend_stylus3_status;
}

struct thp_feature_status_check {
	unsigned int cmd;
	long (*feature_check_fun)(struct thp_core_data *cd);
};

struct thp_feature_status_check thp_feature_check_table[] = {
	{ THP_SUB_EVENT_SINGLE_CLICK, thp_check_single_click },
	{ THP_SUB_EVENT_DOUBLE_CLICK, thp_check_double_click },
	{ THP_SUB_EVENT_STYLUS_SINGLE_CLICK_AND_PRESS, thp_check_stylus_click },
	{ THP_SUB_EVENT_STYLUS_SINGLE_CLICK, thp_check_stylus_click},
};

static int thp_feature_interception_check(struct thp_core_data *cd, char sub_event)
{
	int ret;

	for (int i = 0; i < ARRAY_SIZE(thp_feature_check_table); i++) {
		if (sub_event == thp_feature_check_table[i].cmd) {
			ret = thp_feature_check_table[i].feature_check_fun(cd);
			if (!ret) {
				thp_log_err("%s: check func status done : not support %u\n", __func__, ret);
				return -EINVAL;
			}
			return 0;
		}
	}
	return 0;
}

static int thp_report_single_click(struct thp_core_data *cd)
{
	__pm_wakeup_event(cd->thp_wake_lock, jiffies_to_msecs(HZ));
	thp_inputkey_report(TS_SINGLE_CLICK, cd->panel_id);
	thp_log_info("%s: report single click done\n", __func__);
	return 0;
}

static int thp_report_double_click(struct thp_core_data *cd)
{
	__pm_wakeup_event(cd->thp_wake_lock, jiffies_to_msecs(HZ));
	thp_inputkey_report(TS_DOUBLE_CLICK, cd->panel_id);
	thp_log_info("%s: report double click done\n", __func__);
	return 0;
}

static int thp_report_stylus_and_press(struct thp_core_data *cd)
{
	__pm_wakeup_event(cd->thp_wake_lock, jiffies_to_msecs(HZ));
	thp_input_pen_report(TS_STYLUS_WAKEUP_TO_MEMO, cd->panel_id);
	thp_log_info("%s: report thp_report_stylus_and_press done\n", __func__);
	return 0;
}

static int thp_report_stylus_single_click(struct thp_core_data *cd)
{
	__pm_wakeup_event(cd->thp_wake_lock, jiffies_to_msecs(HZ));
	thp_input_pen_report(TS_STYLUS_WAKEUP_SCREEN_ON, cd->panel_id);
	thp_log_info("%s: thp_report_stylus_single_click\n", __func__);
	return 0;
}

static int thp_shb_request_ap_reset(struct thp_core_data *cd)
{
	int value = 0;

	if (!cd->thp_shb_timeout_hard_reset)
		return 0;

	gpio_set_value(cd->gpios.rst_gpio, GPIO_LOW);
	value = gpio_get_value(cd->gpios.rst_gpio);
	thp_log_info("%s: get rst value : %d\n", __func__, value);
	mdelay(10); /* wait 10 ms */
	gpio_set_value(cd->gpios.rst_gpio, GPIO_HIGH);
	value = gpio_get_value(cd->gpios.rst_gpio);
	thp_log_info("%s: get rst value : %d\n", __func__, value);
	return 0;
}

static int thp_shb_process_done(struct thp_core_data *cd)
{
	cd->shb_waitq_flag = WAITQ_WAKEUP;
	wake_up_interruptible(&(cd->shb_waitq));
	thp_log_info("%s: shb process done\n", __func__);
	return 0;
}

struct thp_report_to_inputdev_group {
	unsigned int cmd;
	int (*report_to_inputdev)(struct thp_core_data *cd);
};

struct thp_report_to_inputdev_group thp_report_inputdev_table[] = {
	{ THP_SUB_EVENT_SINGLE_CLICK, thp_report_single_click },
	{ THP_SUB_EVENT_DOUBLE_CLICK, thp_report_double_click },
	{ THP_SUB_EVENT_STYLUS_SINGLE_CLICK_AND_PRESS, thp_report_stylus_and_press },
	{ THP_SUB_EVENT_STYLUS_SINGLE_CLICK, thp_report_stylus_single_click},
	{ THP_SUB_EVENT_SHB_PROCESS_DONE, thp_shb_process_done},
	{ THP_SUB_EVENT_REQUEST_AP_RESET, thp_shb_request_ap_reset},
};

static int thp_report_inputevent(struct thp_core_data *cd, char sub_event)
{
	int ret;

	for (int i = 0; i < ARRAY_SIZE(thp_report_inputdev_table); i++) {
		if (sub_event == thp_report_inputdev_table[i].cmd) {
			thp_log_info("%s: report event : %u\n", __func__, sub_event);
			return thp_report_inputdev_table[i].report_to_inputdev(cd);
		}
	}
	thp_log_err("%s: sub_event is invalid, event = %d\n",
		__func__, sub_event);
	return -EINVAL;
}

int thp_send_event_to_drv(const char *data)
{
	char sub_event;
	struct thp_core_data *cd = NULL;
	int ret;

	if (!data) {
		thp_log_err("%s: data is NULL\n", __func__);
		return -EINVAL;
	}

	cd = thp_get_core_data(data[1]);
	if (!atomic_read(&cd->register_flag)) {
		thp_log_err("%s: thp have not be registered\n", __func__);
		return -ENODEV;
	}
	sub_event = data[0];

	ret = thp_feature_interception_check(cd, sub_event);
	if (ret)
		return -EINVAL;

	ret = thp_report_inputevent(cd, sub_event);
	if (ret)
		return -EINVAL;
	thp_log_info("%s sub_event = %d\n", __func__, sub_event);
	return 0;
}
#endif

#if defined(CONFIG_TEE_TUI)
static void thp_tui_secos_exit(u32 panel_id);
#endif
int thp_spi_sync(struct spi_device *spi, struct spi_message *message)
{
	int ret;

	thp_log_debug("%s:enter\n", __func__);
	trace_touch(TOUCH_TRACE_SPI, TOUCH_TRACE_FUNC_IN, "thp");
	ret = spi_sync(spi, message);
	trace_touch(TOUCH_TRACE_SPI, TOUCH_TRACE_FUNC_OUT, "thp");
	thp_log_debug("%s:out\n", __func__);
	return ret;
}
EXPORT_SYMBOL(thp_spi_sync);

static int thp_is_valid_project_id(const char *id)
{
	int i;

	if ((id == NULL) || (*id == '\0'))
		return false;
	for (i = 0; i < THP_PROJECT_ID_LEN; i++) {
		if (!isascii(*id) || !isalnum(*id))
			return false;
		id++;
	}
	return true;
}

unsigned int thp_is_factory(void)
{
	unsigned int is_factory;

#if (defined(CONFIG_HUAWEI_THP_MTK) || defined(CONFIG_HUAWEI_THP_QCOM))
	is_factory = get_runmode_type();
	thp_log_debug("%s: is_factory = %d\n", __func__, is_factory);
#else
	is_factory = runmode_is_factory();
#endif
	return (!!is_factory);
}

int thp_project_id_provider(char *project_id,
	unsigned int len, u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	if (len < THP_PROJECT_ID_LEN) {
		thp_log_err("%s len is too small\n", __func__);
		return -EINVAL;
	}
	if ((project_id != NULL) && (cd != NULL)) {
		if (thp_is_valid_project_id(cd->project_id)) {
			if (!strncmp(cd->project_id, IC_THP_BOE_SYNA_RM_ELS,
				THP_PROJECT_ID_LEN)) {
				strncpy(project_id, IC_THP_BOE_SYNA_DW_ELS,
					THP_PROJECT_ID_LEN);
			} else {
				strncpy(project_id, cd->project_id,
					THP_PROJECT_ID_LEN);
			}
		} else {
			thp_log_debug("%s: invalid project id", __func__);
			return -EINVAL;
		}
	}
	return 0;
}
EXPORT_SYMBOL(thp_project_id_provider);

const int get_thp_unregister_ic_num(void)
{
	return thp_unregister_ic_num;
}
EXPORT_SYMBOL(get_thp_unregister_ic_num);

struct thp_core_data *thp_get_core_data(u32 panel_id)
{
	if (panel_id >= TOTAL_PANEL_NUM) {
		thp_log_err("%s: Invalid panel %u, use default panel 0\n", __func__, panel_id);
		WARN_ON(1);
		return g_thp_core[0];
	}
	return g_thp_core[panel_id];
}
EXPORT_SYMBOL(thp_get_core_data);

void thp_wake_up_frame_waitq(struct thp_core_data *cd)
{
	cd->frame_waitq_flag = WAITQ_WAKEUP;
	wake_up_interruptible(&(cd->frame_waitq));
}
static int thp_pinctrl_get_init(struct thp_device *tdev);

#define is_invaild_power_id(x) ((x) >= THP_POWER_ID_MAX)

static char *thp_power_name[THP_POWER_ID_MAX] = {
	"thp-iovdd",
	"thp-vcc",
	"thp-iovdd-s",
	"thp-vcc-s",
};

const char *thp_power_id2name(enum thp_power_id id)
{
	return !is_invaild_power_id(id) ? thp_power_name[id] : 0;
}

int thp_power_supply_get(enum thp_power_id power_id, u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	struct thp_power_supply *power = NULL;
	int ret;

	if (is_invaild_power_id(power_id)) {
		thp_log_err("%s: invalid power id %d", __func__, power_id);
		return -EINVAL;
	}

	power = &cd->thp_powers[power_id];
	if (power->type == THP_POWER_UNUSED)
		return 0;
	if (power->use_count) {
		power->use_count++;
		return 0;
	}
	switch (power->type) {
	case THP_POWER_LDO:
		power->regulator = regulator_get(&cd->sdev->dev,
			thp_power_id2name(power_id));
		if (IS_ERR_OR_NULL(power->regulator)) {
			thp_log_err("%s:fail to get %s\n", __func__,
				thp_power_id2name(power_id));
			return -ENODEV;
		}

		ret = regulator_set_voltage(power->regulator, power->ldo_value,
				power->ldo_value);
		if (ret) {
			regulator_put(power->regulator);
			thp_log_err("%s:fail to set %s valude %d\n", __func__,
					thp_power_id2name(power_id),
					power->ldo_value);
			return ret;
		}
		break;
	case THP_POWER_GPIO:
		ret = gpio_request(power->gpio, thp_power_id2name(power_id));
		if (ret) {
			thp_log_err("%s:request gpio %d for %s failed\n",
				__func__, power->gpio,
				thp_power_id2name(power_id));
			return ret;
		}
		break;

#if (!defined CONFIG_HUAWEI_THP_QCOM) && (!defined CONFIG_HUAWEI_THP_MTK)
	case THP_POWER_PMIC:
		thp_log_info("%s call %d,%d,%d\n", __func__,
			power->pmic_power.pmic_num, power->pmic_power.ldo_num,
			power->pmic_power.value);
		tp_pmic_ldo_set.pmic_num = power->pmic_power.pmic_num;
		tp_pmic_ldo_set.pmic_power_type = power->pmic_power.ldo_num;
		tp_pmic_ldo_set.pmic_power_voltage = power->pmic_power.value;
		break;
#endif

	default:
		thp_log_err("%s: invalid power type %d\n",
			__func__, power->type);
		break;
	}
	power->use_count++;
	return 0;
}
EXPORT_SYMBOL(thp_power_supply_get);

int thp_power_supply_put(enum thp_power_id power_id, u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	struct thp_power_supply *power = NULL;

	if (is_invaild_power_id(power_id)) {
		thp_log_err("%s: invalid power id %d", __func__, power_id);
		return -EINVAL;
	}

	power = &cd->thp_powers[power_id];
	if (power->type == THP_POWER_UNUSED)
		return 0;
	if ((--power->use_count) > 0)
		return 0;

	switch (power->type) {
	case THP_POWER_LDO:
		if (IS_ERR_OR_NULL(power->regulator)) {
			thp_log_err("%s:fail to get %s\n", __func__,
				thp_power_id2name(power_id));
			return -ENODEV;
		}
		regulator_put(power->regulator);
		break;
	case THP_POWER_GPIO:
		gpio_direction_output(power->gpio, 0);
		gpio_free(power->gpio);
		break;
	case THP_POWER_PMIC:
		thp_log_err("%s: power supply %d\n", __func__, power->type);
		break;

	default:
		thp_log_err("%s: invalid power type %d\n",
			__func__, power->type);
		break;
	}
	return 0;
}
EXPORT_SYMBOL(thp_power_supply_put);

void thp_power_lock(void)
{
	thp_log_debug("%s: get power lock", __func__);
	mutex_lock(&thp_power_mutex);
}
EXPORT_SYMBOL(thp_power_lock);

void thp_power_unlock(void)
{
	thp_log_debug("%s: power unlock", __func__);
	mutex_unlock(&thp_power_mutex);
}
EXPORT_SYMBOL(thp_power_unlock);

int thp_power_supply_ctrl(enum thp_power_id power_id,
	int status, unsigned int delay_ms, u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	struct thp_power_supply *power = NULL;
	int rc = 0;

	if (is_invaild_power_id(power_id)) {
		thp_log_err("%s: invalid power id %d", __func__, power_id);
		return -EINVAL;
	}

	power = &cd->thp_powers[power_id];
	if (power->type == THP_POWER_UNUSED)
		goto exit;

	thp_log_info("%s:power %s %s\n", __func__,
		thp_power_id2name(power_id), status ? "on" : "off");

	if (!power->use_count) {
		thp_log_err("%s:regulator %s not gotten yet\n", __func__,
				thp_power_id2name(power_id));
		return -ENODEV;
	}
	switch (power->type) {
	case THP_POWER_LDO:
		if (IS_ERR_OR_NULL(power->regulator)) {
			thp_log_err("%s:fail to get %s\n", __func__,
				thp_power_id2name(power_id));
			return -ENODEV;
		}
		rc = status ? regulator_enable(power->regulator) :
			regulator_disable(power->regulator);
		break;
	case THP_POWER_GPIO:
		gpio_direction_output(power->gpio, status ? 1 : 0);
		break;

#if (!defined CONFIG_HUAWEI_THP_QCOM) && (!defined CONFIG_HUAWEI_THP_MTK)
	case THP_POWER_PMIC:
		tp_pmic_ldo_set.pmic_power_state = (status ? 1 : 0);
		rc = hw_pmic_power_cfg(TP_PMIC_REQ, &tp_pmic_ldo_set);
		if (rc)
			thp_log_err("%s:pmic %s failed, %d\n", __func__,
				thp_power_id2name(power_id), rc);
		break;
#endif

	default:
		thp_log_err("%s: invalid power type %d\n",
			__func__, power->type);
		break;
	}
exit:
	if (delay_ms)
		mdelay(delay_ms);
	return rc;
}
EXPORT_SYMBOL(thp_power_supply_ctrl);

#define POWER_CONFIG_NAME_MAX 20
static void thp_paser_pmic_power(struct device_node *thp_node,
	struct thp_core_data *cd, int power_id)
{
	const char *power_name = NULL;
	char config_name[POWER_CONFIG_NAME_MAX] = {0};
	struct thp_power_supply *power = NULL;
	int rc;

	power_name = thp_power_id2name(power_id);
	power = &cd->thp_powers[power_id];
	snprintf(config_name, (POWER_CONFIG_NAME_MAX - 1),
		"%s-value", power_name);
	rc = of_property_read_u32(thp_node, config_name,
		&power->pmic_power.value);
	if (rc)
		thp_log_err("%s:failed to get %s\n",
			__func__, config_name);
	snprintf(config_name, (POWER_CONFIG_NAME_MAX - 1),
		"%s-ldo-num", power_name);
	rc = of_property_read_u32(thp_node, config_name,
		&power->pmic_power.ldo_num);
	if (rc)
		thp_log_err("%s:failed to get %s\n",
			__func__, config_name);
	snprintf(config_name, (POWER_CONFIG_NAME_MAX - 1),
		"%s-pmic-num", power_name);
	rc = of_property_read_u32(thp_node, config_name,
		&power->pmic_power.pmic_num);
	if (rc)
		thp_log_err("%s:failed to get %s\n",
			__func__, config_name);
	thp_log_info("%s: to get %d, %d,%d\n", __func__,
		power->pmic_power.ldo_num,
		power->pmic_power.pmic_num,
		power->pmic_power.value);
}

static int thp_parse_one_power(struct device_node *thp_node,
	struct thp_core_data *cd, int power_id)
{
	const char *power_name = NULL;
	char config_name[POWER_CONFIG_NAME_MAX] = {0};
	struct thp_power_supply *power = NULL;
	int rc;

	power_name = thp_power_id2name(power_id);
	power = &cd->thp_powers[power_id];

	rc = snprintf(config_name, POWER_CONFIG_NAME_MAX - 1,
		"%s-type", power_name);

	thp_log_info("%s:parse power: %s\n", __func__, config_name);

	rc = of_property_read_u32(thp_node, config_name, &power->type);
	if (rc || power->type == THP_POWER_UNUSED) {
		thp_log_info("%s: power %s type not config or 0, unused\n",
			__func__, config_name);
		return 0;
	}

	switch (power->type) {
	case THP_POWER_GPIO:
		snprintf(config_name, POWER_CONFIG_NAME_MAX - 1,
			"%s-gpio", power_name);
		power->gpio = of_get_named_gpio(thp_node, config_name, 0);
		if (!gpio_is_valid(power->gpio)) {
			thp_log_err("%s:failed to get %s\n",
				__func__, config_name);
			return -ENODEV;
		}
		break;
	case THP_POWER_LDO:
		snprintf(config_name, POWER_CONFIG_NAME_MAX - 1,
			"%s-value", power_name);
		rc = of_property_read_u32(thp_node, config_name,
				&power->ldo_value);
		if (rc) {
			thp_log_err("%s:failed to get %s\n",
				__func__, config_name);
			return rc;
		}
		break;
	case THP_POWER_PMIC:
		thp_paser_pmic_power(thp_node, cd, power_id);
		break;
	default:
		thp_log_err("%s: invaild power type %d", __func__, power->type);
		break;
	}

	return 0;
}

static int thp_parse_power_config(struct device_node *thp_node,
	struct thp_core_data *cd)
{
	int rc;
	int i;
	u32 value = 0;

	cd->not_control_power = 0;
	rc = of_property_read_u32(thp_node, "not_control_power",
		&value);
	if (!rc) {
		cd->not_control_power = value;
		thp_log_info("%s: not_control_power: %u\n",
			__func__, value);
		return 0;
	}
	for (i = 0; i < THP_POWER_ID_MAX; i++) {
		rc = thp_parse_one_power(thp_node, cd, i);
		if (rc)
			return rc;
	}

	return 0;
}

static void touch_driver_power_init(struct thp_device *tdev)
{
	struct thp_core_data *cd = tdev->thp_core;
	static int flag = 0;

	cd->power_cfg.common_power_flag = false;
	if (flag) {
		thp_log_info("%s: power is inited\n", __func__);
		return;
	}
	flag++;
	cd->power_cfg.common_power_flag = true;
	thp_power_supply_get(THP_VCC, cd->panel_id);
	thp_power_supply_get(THP_IOVDD, cd->panel_id);
	thp_power_supply_get(THP_VCC_S, cd->panel_id);
	thp_power_supply_get(THP_IOVDD_S, cd->panel_id);
}

static void thp_power_supply_ctrl_for_seq0(int status, u32 panel_id,
	unsigned int iovdd_delay_ms, unsigned int vcc_delay_ms)
{
	int ret;

	ret = thp_power_supply_ctrl(THP_IOVDD, status,
			iovdd_delay_ms, panel_id);
	if (ret)
		thp_log_err("%s:op IOVDD to status %d fail\n", __func__, status);
	ret = thp_power_supply_ctrl(THP_VCC, status,
		vcc_delay_ms, panel_id);
	if (ret)
		thp_log_err("%s:op VCC to status %d fail\n", __func__, status);
	ret = thp_power_supply_ctrl(THP_IOVDD_S, status,
		iovdd_delay_ms, panel_id);
	if (ret)
		thp_log_err("%s:op IOVDD_S to status %d fail\n", __func__, status);
	ret = thp_power_supply_ctrl(THP_VCC_S, status,
		vcc_delay_ms, panel_id);
	if (ret)
		thp_log_err("%s:op VCC_S to status %d fail\n", __func__, status);
	return;
}

static u32 thp_get_panel_for_common_power_ctrl(struct thp_core_data *cd)
{
	u32 i;
	u32 panel_index = 0;

	for (i = 0; i <= cd->dual_thp; i++) {
		if (thp_get_core_data(i)->power_cfg.common_power_flag)
			panel_index = i;
	}
	return panel_index;
}

static int touch_driver_power_on_for_seq0(struct thp_device *tdev)
{
	struct thp_core_data *cd = NULL;
	u32 panel_index;

	thp_log_info("%s:called\n", __func__);

	if (!tdev) {
	    thp_log_err("%s: invalid input\n", __func__);
	    return -EINVAL;
	}

	cd = tdev->thp_core;
	if (!cd->not_support_cs_control)
		gpio_set_value(tdev->gpios->cs_gpio, GPIO_HIGH);

#ifdef CONFIG_HUAWEI_THP_QCOM
	if (cd->support_control_cs_off &&
		(!IS_ERR_OR_NULL(cd->qcom_pinctrl.cs_high))) {
		pinctrl_select_state(tdev->thp_core->pctrl,
			tdev->thp_core->qcom_pinctrl.cs_high);
		thp_log_info("%s: cs to high\n", __func__);
	}
#endif

	panel_index = thp_get_panel_for_common_power_ctrl(cd);
	gpio_direction_output(tdev->gpios->rst_gpio, GPIO_LOW);
	if (!g_thp_common_power[cd->power_cfg.power_sequence].power_on_flag) {
		g_thp_common_power[cd->power_cfg.power_sequence].power_on_flag++;
		/* iovdd delay 7ms, vcc delay 12ms */
		thp_power_supply_ctrl_for_seq0(THP_POWER_ON, panel_index, 7, 12);
	}
	thp_log_info("%s pull up tp ic reset\n", __func__);
	gpio_direction_output(tdev->gpios->rst_gpio, GPIO_HIGH);
	thp_do_time_delay(tdev->timing_config.boot_reset_hi_delay_ms);
	return NO_ERR;
}

static int touch_driver_power_off_for_seq0(struct thp_device *tdev)
{
	struct thp_core_data *cd = NULL;
	u32 panel_index;

	thp_log_info("%s:called\n", __func__);
	if (!tdev) {
	    thp_log_err("%s: invalid input\n", __func__);
	    return -EINVAL;
	}
	cd = tdev->thp_core;
	panel_index = thp_get_panel_for_common_power_ctrl(cd);

	gpio_set_value(tdev->gpios->rst_gpio, GPIO_LOW);
	thp_do_time_delay(tdev->timing_config.suspend_reset_after_delay_ms);
	if (g_thp_common_power[cd->power_cfg.power_sequence].power_on_flag) {
		g_thp_common_power[cd->power_cfg.power_sequence].power_on_flag--;
		/* iovdd delay 0ms, vcc delay 0ms */
		thp_power_supply_ctrl_for_seq0(THP_POWER_OFF, panel_index, 0, 0);
	}
	if (!cd->not_support_cs_control)
		gpio_set_value(tdev->gpios->cs_gpio, GPIO_LOW);

#ifdef CONFIG_HUAWEI_THP_QCOM
	if (cd->support_control_cs_off &&
		(!IS_ERR_OR_NULL(cd->qcom_pinctrl.cs_low))) {
		pinctrl_select_state(tdev->thp_core->pctrl,
			tdev->thp_core->qcom_pinctrl.cs_low);
		thp_log_info("%s: cs to low\n", __func__);
	}
#endif
	return NO_ERR;
}

int is_valid_project_id(const char *id)
{
	while (*id != '\0') {
		if ((*id & BIT_MASK(PROJECTID_BIT_MASK_NUM)) || (!isalnum(*id)))
			return false;
		id++;
	}

	return true;
}
EXPORT_SYMBOL(is_valid_project_id);

#define GET_HWLOCK_FAIL 0
int thp_bus_lock(void)
{
	int ret;
	unsigned long time;
	unsigned long timeout;
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
	struct hwspinlock *hwlock = cd->hwspin_lock;

	mutex_lock(&cd->spi_mutex);
	if (!cd->use_hwlock)
		return 0;

	timeout = jiffies + msecs_to_jiffies(THP_GET_HARDWARE_TIMEOUT);

	do {
		ret = hwlock->bank->ops->trylock(hwlock);
		if (ret == GET_HWLOCK_FAIL) {
			time = jiffies;
			if (time_after(time, timeout)) {
				thp_log_err("%s:get hardware_mutex timeout\n",
					__func__);
				mutex_unlock(&cd->spi_mutex);
				return -ETIME;
			}
		}
	} while (ret == GET_HWLOCK_FAIL);

	return 0;
}
EXPORT_SYMBOL(thp_bus_lock);

void thp_bus_unlock(void)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
	struct hwspinlock *hwlock = cd->hwspin_lock;

	mutex_unlock(&cd->spi_mutex);
	if (cd->use_hwlock)
		hwlock->bank->ops->unlock(hwlock);
}
EXPORT_SYMBOL(thp_bus_unlock);

#define QUEUE_IS_FULL 1
#define QUEUE_IS_NOT_FULL 0
static struct thp_queue *thp_queue_init(struct thp_core_data *cd)
{
	int i;
	struct thp_queue *queue = NULL;

	thp_log_info("%s start\n", __func__);
	if (cd->thp_queue_buf_len)
		g_thp_queue_node_len = cd->thp_queue_buf_len;
	if (g_thp_queue_node_len > THP_MAX_FRAME_SIZE) {
		thp_log_err("%s thp queue node len is invalid\n", __func__);
		return NULL;
	}
	thp_log_info("%s:thp queue node len is %u\n",
		__func__, g_thp_queue_node_len);
	queue = kzalloc(sizeof(*queue), GFP_KERNEL);
	if (queue == NULL) {
		thp_log_err("%s queue malloc failed\n", __func__);
		return NULL;
	}
	g_queue_data_buf = vzalloc(g_thp_queue_node_len * THP_LIST_MAX_FRAMES);
	if (!g_queue_data_buf) {
		thp_log_err("%s g_queue_data_buf vzalloc failed\n", __func__);
		kfree(queue);
		queue = NULL;
		return NULL;
	}

	for (i = 0; i < THP_LIST_MAX_FRAMES; i++)
		queue->frame_data[i].buf = g_queue_data_buf +
			(i * g_thp_queue_node_len);

	queue->front = 0;
	queue->tail = 0;
	queue->size = THP_LIST_MAX_FRAMES;
	queue->flag = QUEUE_IS_NOT_FULL;
	return queue;
}

static bool thp_queue_is_empty(struct thp_queue *queue)
{
	if (queue == NULL) {
		thp_log_err("%s queue is NULL\n", __func__);
		return false;
	}
	if ((queue->flag == QUEUE_IS_NOT_FULL) &&
		(queue->front == queue->tail))
		return true;
	else
		return false;
}

static bool thp_queue_is_full(struct thp_queue *queue)
{
	if (queue == NULL) {
		thp_log_err("%s queue is NULL\n", __func__);
		return false;
	}
	if (queue->flag == QUEUE_IS_FULL)
		return true;
	else
		return false;
}

static bool thp_queue_dequeue(struct thp_queue *queue)
{
	if (queue == NULL) {
		thp_log_err("%s queue is NULL\n", __func__);
		return false;
	}
	thp_log_debug("%s in\n", __func__);

	if (thp_queue_is_empty(queue)) {
		thp_log_err("%s queue is empty\n", __func__);
		return false;
	}
	memset(queue->frame_data[queue->front].buf, 0, g_thp_queue_node_len);
	memset(&queue->frame_data[queue->front].tv, 0,
		sizeof(queue->frame_data[queue->front].tv));
	queue->front = ((queue->front + 1) < queue->size) ?
		(queue->front + 1) : 0;
	queue->flag = QUEUE_IS_NOT_FULL;
	return true;
}

static bool thp_queue_enqueue(struct thp_queue *queue,
	u8 *read_buf, unsigned int len, u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	if ((read_buf == NULL) || (queue == NULL)) {
		thp_log_err("%s read_buf is NULL\n", __func__);
		return false;
	}
	thp_log_debug("%s in\n", __func__);

	if (thp_queue_is_full(queue)) {
		if (!thp_queue_dequeue(queue)) {
			thp_log_err("%s dequeue failed\n", __func__);
			return false;
		}
	}
	if (len > g_thp_queue_node_len) {
		thp_log_debug("%s len is too big\n", __func__);
		len = g_thp_queue_node_len;
	}
	memcpy(queue->frame_data[queue->tail].buf, read_buf, len);
	if (cd->tp_time_sync_support)
		memcpy(&queue->frame_data[queue->tail].tv, &cd->report_cur_time, sizeof(cd->report_cur_time));
	else
		do_timetransfer(&queue->frame_data[queue->tail].tv);

	queue->tail = ((queue->tail + 1) < queue->size) ? (queue->tail + 1) : 0;
	if (queue->tail == queue->front)
		queue->flag = QUEUE_IS_FULL;
	return true;
}

static struct thp_queue_node *thp_queue_get_head(
	struct thp_queue *queue)
{
	if (queue == NULL) {
		thp_log_err("%s queue is NULL\n", __func__);
		return NULL;
	}
	if (thp_queue_is_empty(queue)) {
		thp_log_err("%s queue is empty\n", __func__);
		return NULL;
	}
	return &queue->frame_data[queue->front];
}

static void thp_queue_clear_all(struct thp_queue *queue)
{
	int i;

	if (queue == NULL) {
		thp_log_err("%s queue is NULL\n", __func__);
		return;
	}

	for (i = 0; i < THP_LIST_MAX_FRAMES; i++) {
		memset(queue->frame_data[i].buf, 0, g_thp_queue_node_len);
		memset(&queue->frame_data[i].tv, 0,
			sizeof(queue->frame_data[i].tv));
	}

	queue->front = 0;
	queue->tail = 0;
	queue->size = THP_LIST_MAX_FRAMES;
	queue->flag = QUEUE_IS_NOT_FULL;
}

static void thp_queue_free(struct thp_queue *queue)
{
	if (queue == NULL) {
		thp_log_err("%s queue is NULL\n", __func__);
		return;
	}
	vfree(g_queue_data_buf);
	g_queue_data_buf = NULL;
	kfree(queue);
	queue = NULL;
}

static int thp_setup_spi(struct thp_core_data *cd);
int thp_set_spi_max_speed(unsigned int speed, u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	int rc;

	cd->sdev->max_speed_hz = speed;

	thp_log_info("%s:set max_speed_hz %d\n", __func__, speed);
	rc = thp_bus_lock();
	if (rc) {
		thp_log_err("%s: get lock failed\n", __func__);
		return rc;
	}
	if (thp_setup_spi(cd)) {
		thp_log_err("%s: set spi speed fail\n", __func__);
		rc = -EIO;
	}
	thp_bus_unlock();
	return rc;
}
EXPORT_SYMBOL(thp_set_spi_max_speed);

/*
 * If getframe time out, check irq status.
 * If irq is disabled(depth != 0), enable irq.
 */
static void check_irq_status(unsigned int panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	struct irq_desc *desc = NULL;

	desc = irq_to_desc(cd->irq);
	if (!desc) {
		thp_log_err("%s:desc is null\n", __func__);
		return;
	}
	if (cd->need_check_irq_status && desc->depth != 0) {
		enable_irq(cd->irq);
#if defined(CONFIG_HUAWEI_DSM)
		thp_dmd_report(DSM_TPHOSTPROCESSING_DEV_STATUS_ERROR_NO,
			"%s, wait frame timed out\n", __func__);
#endif
		thp_log_info("%s:irq is disabled, need enable irq\n", __func__);
	}
}

static int thp_wait_frame_waitq(struct thp_core_data *cd)
{
	int t;

	cd->frame_waitq_flag = WAITQ_WAIT;

	/* if not use timeout */
	if (!cd->timeout) {
		t = wait_event_interruptible(cd->frame_waitq,
				(cd->frame_waitq_flag == WAITQ_WAKEUP));
		return 0;
	}

	/* if use timeout */
	t = wait_event_interruptible_timeout(cd->frame_waitq,
			(cd->frame_waitq_flag == WAITQ_WAKEUP),
			msecs_to_jiffies(cd->timeout));
	if (!is_tmo(t))
		return 0;

#if defined(CONFIG_HUAWEI_DSM)
	thp_log_err("%s: wait frame timed out, dmd code:%d\n",
			__func__, DSM_TPHOSTPROCESSING_DEV_STATUS_ERROR_NO);
#ifdef THP_TIMEOUT_DMD
	thp_dmd_report(DSM_TPHOSTPROCESSING_DEV_STATUS_ERROR_NO,
		"%s, wait frame timed out\n", __func__);
#endif
#endif
	thp_get_irq_info(cd->panel_id);
	check_irq_status(cd->panel_id);
	thp_get_polymeric_chip_status(cd->panel_id);
	g_thp_log_cfg = TEMP_OPEN_DEBUG_LOG;
	return -ETIMEDOUT;
}

int thp_set_status(int type, int status, u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	mutex_lock(&cd->status_mutex);
	status ? __set_bit(type, (unsigned long *)&cd->status) :
		__clear_bit(type, (unsigned long *)&cd->status);
	mutex_unlock(&cd->status_mutex);

	thp_mt_wrapper_wakeup_poll(panel_id);

	thp_log_info("%s:type=%d value=%d\n", __func__, type, status);
	return 0;
}

int thp_get_status(int type, u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	return test_bit(type, (unsigned long *)&cd->status);
}
EXPORT_SYMBOL(thp_get_status);

u32 thp_get_status_all(u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	return cd->status;
}

static void thp_clear_frame_buffer(struct thp_core_data *cd)
{
	struct thp_frame *temp = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	if (cd->use_thp_queue) {
		thp_queue_clear_all(cd->thp_queue);
	} else {
		if (list_empty(&cd->frame_list.list))
			return;
		list_for_each_safe(pos, n, &cd->frame_list.list) {
			temp = list_entry(pos, struct thp_frame, list);
			list_del(pos);
			kfree(temp);
			temp = NULL;
		}
	}
	cd->frame_count = 0;
}

static int thp_spi_transfer(struct thp_core_data *cd,
	char *tx_buf, char *rx_buf, unsigned int len, unsigned int lock_status)
{
	int rc;
	bool no_need_spi_communication_in_suspend;
	struct spi_message msg;
	struct spi_device *sdev = cd->sdev;
	struct spi_transfer xfer = {
		.tx_buf = tx_buf,
		.rx_buf = rx_buf,
		.len = len,
		.delay_usecs = cd->thp_dev->timing_config.spi_transfer_delay_us,
	};

	no_need_spi_communication_in_suspend = (cd->suspended) && (!cd->need_work_in_suspend) &&
		(!cd->need_spi_communication_in_suspend);
	if (no_need_spi_communication_in_suspend) {
		thp_log_err("%s - suspended\n", __func__);
		return 0;
	}

	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);
	if (lock_status == NEED_LOCK) {
		rc = thp_bus_lock();
		if (rc < 0) {
			thp_log_err("%s:get lock failed:%d\n", __func__, rc);
			return rc;
		}
	}
	if (!cd->not_support_cs_control) {
		if (cd->panel_id == MAIN_TOUCH_PANEL)
			thp_spi_cs_set(GPIO_HIGH);
		else if (cd->panel_id == SUB_TOUCH_PANEL)
			thp_spi_cs_set_sub_panel(GPIO_HIGH);
	}
	rc = thp_spi_sync(sdev, &msg);
	if (lock_status == NEED_LOCK)
		thp_bus_unlock();

	return rc;
}

/*
 * If irq is disabled/enabled, can not disable/enable again
 * disable - status 0; enable - status not 0
 */
void thp_set_irq_status(struct thp_core_data *cd, int status)
{
	if (!cd) {
		thp_log_err("%s: cd is nullptr\n", __func__);
		return;
	}
	mutex_lock(&cd->irq_mutex);
	if (cd->irq_enabled != (!!status)) {
		status ? enable_irq(cd->irq) : disable_irq(cd->irq);
		cd->irq_enabled = !!status;
		thp_log_info("%s: %s irq\n", __func__,
			status ? "enable" : "disable");
	}
	mutex_unlock(&cd->irq_mutex);
};
EXPORT_SYMBOL(thp_set_irq_status);

/*
 * This function is called for recording the system time when tp
 * receive the suspend cmd from lcd driver for proximity feature.
 */
static void thp_prox_suspend_record_time(struct thp_core_data *cd)
{
	do_timetransfer(&cd->tp_suspend_record_tv);
	thp_log_info(
		"[Proximity_feature] TP early suspend at %ld secs %ld us\n",
		cd->tp_suspend_record_tv.tv_sec,
		cd->tp_suspend_record_tv.tv_nsec);
}

static bool need_work_in_suspend_switch(struct thp_core_data *cd)
{
	bool result = false;
	unsigned int double_tap_status;
	unsigned int ap_gesture_status;

	if (!cd->use_ap_gesture)
		return (cd->easy_wakeup_info.sleep_mode == TS_GESTURE_MODE &&
			cd->support_gesture_mode && !cd->support_shb_thp &&
			cd->tsa_event_to_udfp != TAS_EVENT_TO_UDFP_MODE_TWO);

	double_tap_status = (cd->easy_wakeup_info.sleep_mode ==
		TS_GESTURE_MODE) && cd->support_gesture_mode;
	ap_gesture_status = thp_get_status(THP_STATUS_UDFP, cd->panel_id) ||
		cd->aod_touch_status;
	result = (double_tap_status || ap_gesture_status) &&
		!cd->support_shb_thp;
	return result;
}

#ifdef CONFIG_HUAWEI_SHB_THP
static int tp_ud_send_tp_ap_event(struct thp_core_data *cd)
{
	char switch_enable[SWITCH_ENABLE_LENGTH] = {1};
	int ret = 0;
	unsigned int gesture_status;
	unsigned int finger_status;
	unsigned int stylus_status;
	unsigned int flag_status;

	gesture_status = cd->easy_wakeup_info.sleep_mode;
	finger_status = thp_get_status(THP_STATUS_UDFP, cd->panel_id);
	stylus_status = thp_get_status(THP_STATUS_STYLUS, cd->panel_id) ||
		cd->suspend_stylus3_status;

	flag_status = (gesture_status == TS_GESTURE_MODE) || finger_status ||
		cd->ring_setting_switch || stylus_status || cd->aod_touch_status;

	if (cd->tsa_event_to_udfp == TAS_EVENT_TO_UDFP_MODE_TWO && flag_status) {
		thp_log_info("%s:switch_enable = %d\n", __func__, switch_enable[0]);
		ret = send_tp_ap_event(SWITCH_ENABLE_LENGTH, switch_enable,
			ST_CMD_TYPE_IRQ_SWITCH);
		thp_log_info("%s:tsa_event_to_udfp_switch_enable, ret = %d\n",
			__func__, ret);
	}
	return ret;
}
#endif

static int thp_suspend(struct thp_core_data *cd)
{
	if (cd->suspended) {
		thp_log_info("%s: already suspended, return\n", __func__);
		return 0;
	}
	cd->afe_download_status = false;
	cd->invalid_irq_num = 0; /* clear invalid irq count */
	if (cd->avoid_low_level_irq_storm)
		thp_set_irq_status(cd, THP_IRQ_DISABLE);
	if (g_thp_log_cfg == TEMP_OPEN_DEBUG_LOG)
		g_thp_log_cfg = DISABLE_DEBUG_LOG;
	mutex_lock(&cd->suspend_flag_mutex);
	cd->suspended = true;
	mutex_unlock(&cd->suspend_flag_mutex);
	if (cd->proximity_support == PROX_SUPPORT) {
		if (cd->need_work_in_suspend) {
			thp_log_info(
				"[Proximity_feature] %s: Enter prximity mode, no need suspend!\n",
				__func__);
			return 0;
		}
	}
	if (need_work_in_suspend_switch(cd)) {
#if (defined(CONFIG_HUAWEI_THP_MTK) || defined(CONFIG_HUAWEI_THP_QCOM))
		if (cd->need_enable_irq_wake) {
			cd->irq_wake_flag = true;
			if (enable_irq_wake(cd->irq)) // enable_irq_wake and disable_irq_wake must match
				thp_log_err("%s:irq wake fail\n", __func__);
		}
#endif
		if (cd->disable_irq_gesture_suspend)
			thp_set_irq_status(cd, THP_IRQ_DISABLE);
		cd->thp_dev->ops->suspend(cd->thp_dev);
		cd->work_status = SUSPEND_DONE;
		thp_set_irq_status(cd, THP_IRQ_ENABLE);
	} else {
#ifndef CONFIG_HUAWEI_THP_MTK
		if (cd->support_pinctrl == 1)
			thp_pinctrl_select_lowpower(cd->thp_dev);
#endif
		if (cd->open_count)
			thp_set_irq_status(cd, THP_IRQ_DISABLE);
#ifdef CONFIG_HUAWEI_SHB_THP
		tp_ud_send_tp_ap_event(cd);
#endif
		cd->thp_dev->ops->suspend(cd->thp_dev);
		cd->work_status = SUSPEND_DONE;
	}
	return 0;
}

static int thp_resume(struct thp_core_data *cd)
{
	if (!cd->suspended) {
		thp_log_info("%s: already resumed, return\n", __func__);
		return 0;
	}

	cd->work_status = BEFORE_RESUME;
	thp_log_info("%s: %s\n", __func__, cd->project_id);
	thp_log_info("%s: spi_sync_count_in_suspend = %d, get_get_frame_count_in_suspend = %d\n",
		__func__, spi_sync_count_in_suspend, get_frame_count_in_suspend);
	spi_sync_count_in_suspend = 0;
	get_frame_count_in_suspend = 0;
	/**
	 * The aod ready status can't be transferred to the sub screen after
	 * screenoff immediately fold. Therefore, the aod ready status of
	 * the sub screen is not reset when the screen is folding.
	 */
	if (!cd->not_change_aod_ready_status)
		cd->aod_window_ready_status = false;

	cd->thp_dev->ops->resume(cd->thp_dev);

	/* clear rawdata frame buffer list */
	mutex_lock(&cd->mutex_frame);
	thp_clear_frame_buffer(cd);
	mutex_unlock(&cd->mutex_frame);
#ifndef CONFIG_HUAWEI_THP_MTK
	if (cd->support_pinctrl == 1)
		thp_pinctrl_select_normal(cd->thp_dev);
#endif

	if (cd->proximity_support == PROX_SUPPORT) {
		g_thp_prox_enable = cd->prox_cache_enable;
		onetime_poweroff_done = false;
		cd->early_suspended = false;
		thp_log_info(
			"[Proximity_feature] %s: update g_thp_prox_enable to %d!\n",
			__func__, g_thp_prox_enable);
	}

#if (defined(CONFIG_HUAWEI_THP_MTK) || defined(CONFIG_HUAWEI_THP_QCOM))
	if (cd->need_enable_irq_wake && cd->irq_wake_flag) {
		cd->irq_wake_flag = false;
		if (disable_irq_wake(cd->irq)) // enable_irq_wake and disable_irq_wake must match
			thp_log_err("%s:disable irq wake fail\n", __func__);
	}
#endif

	cd->suspended = false;
	cd->work_status = RESUME_DONE;
	return 0;
}


static void thp_after_resume_work_fn(struct work_struct *work)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);

	if (cd->delay_work_for_pm) {
		if (cd->thp_dev->ops->after_resume)
			cd->thp_dev->ops->after_resume(cd->thp_dev);
		thp_set_status(THP_STATUS_POWER, THP_RESUME, cd->panel_id);
	}
	cd->suspend_resume_waitq_flag = WAITQ_WAKEUP;
	wake_up_interruptible(&(cd->suspend_resume_waitq));
	thp_log_info("%s: after resume end\n", __func__);
}

DECLARE_WORK(thp_after_resume_work, thp_after_resume_work_fn);

/*
 * Add some delay before screenoff to avoid quick resume-suspend TP-firmware
 * does not download success but thp have notified afe screenoff.
 * Use 'wait_event_interruptible_timeout' wait THP_AFE_STATUS_SCREEN_ON
 * notify and set enough timeout make sure of TP-firmware download success.
 * This solution will cause suspend be more slowly.
 */
static void thp_delay_before_screenoff(struct thp_core_data *cd)
{
	int rc;
	u32 suspend_delay_ms;

	if (!cd || !(cd->thp_dev)) {
		thp_log_err("%s: cd is null\n", __func__);
		return;
	}

	suspend_delay_ms = cd->thp_dev->timing_config.early_suspend_delay_ms;
	if ((atomic_read(&(cd->afe_screen_on_waitq_flag)) == WAITQ_WAKEUP) ||
		!suspend_delay_ms || cd->quickly_screen_on_off) {
		thp_log_info("%s, do not need wait\n", __func__);
		return;
	}

	thp_log_info("%s:wait afe screen on complete\n", __func__);
	rc = wait_event_interruptible_timeout(cd->afe_screen_on_waitq,
		(atomic_read(&(cd->afe_screen_on_waitq_flag)) == WAITQ_WAKEUP),
		msecs_to_jiffies(suspend_delay_ms));
	if (rc)
		return;
	/* if timeout and condition not true, rc is 0 need report DMD */
	atomic_set(&(cd->afe_screen_on_waitq_flag), WAITQ_WAKEUP);
#if defined(CONFIG_HUAWEI_DSM)
	thp_dmd_report(DSM_TPHOSTPROCESSING_DEV_GESTURE_EXP2,
		"%s, screen on %u ms, but fw not ready\n",
		__func__, suspend_delay_ms);
#endif
	thp_log_info("%s, screen on %u ms, but fw not ready\n",
		__func__, suspend_delay_ms);
}

#if ((defined CONFIG_LCD_KIT_DRIVER) || (defined CONFIG_HUAWEI_THP_UDP))
#define SUSPEND_WAIT_TIMEOUT 2000
static void thp_early_suspend(struct thp_core_data *cd)
{
	int rc;
	int status = 0;

	thp_log_info("%s: early suspend,%d\n", __func__,
		cd->suspend_resume_waitq_flag);
	/*
	 * to avoid easy_wakeup_info.sleep_mode being changed during suspend,
	 * assign cd->sleep_mode to easy_wakeup_info.sleep_mode once
	 */
	cd->easy_wakeup_info.sleep_mode = cd->sleep_mode;
	if (cd->multi_panel_index != SINGLE_TOUCH_PANEL)
		cd->easy_wakeup_info.aod_mode = cd->aod_touch_status;
	/*
	 * TDDI need make sure of firmware download complete,
	 * then lcd send 2810 to screen off,
	 * otherwise gesture mode will enter failed.
	 */
	if (cd->wait_afe_screen_on_support)
		thp_delay_before_screenoff(cd);
	if (cd->delay_work_for_pm) {
		if (cd->suspend_resume_waitq_flag != WAITQ_WAKEUP) {
			thp_log_info("%s:wait resume complete\n", __func__);
			rc =  wait_event_interruptible_timeout(
				cd->suspend_resume_waitq,
				(cd->suspend_resume_waitq_flag == WAITQ_WAKEUP),
				SUSPEND_WAIT_TIMEOUT);
			if (!rc)
				thp_log_err("%s:wait resume complete timeout\n",
					__func__);
		}
		thp_set_status(THP_STATUS_POWER, THP_SUSPEND, cd->panel_id);
		cd->suspend_resume_waitq_flag = WAITQ_WAIT;
	} else {
		if (cd->proximity_support != PROX_SUPPORT) {
			thp_log_info("%s: early suspend\n", __func__);
			if (cd->multi_panel_index == SINGLE_TOUCH_PANEL)
				thp_set_status(THP_STATUS_POWER, THP_SUSPEND, cd->panel_id);
			/*
			 * if multi screen and in quickly screen switch,
			 * not notify daemon
			 */
			else if (!cd->quickly_screen_on_off)
				thp_set_status(THP_STATUS_POWER, THP_SUSPEND, cd->panel_id);
			else
				cd->quickly_screen_on_off = false;
		} else {
			thp_prox_suspend_record_time(cd);

			if (thp_is_factory())
				status = is_pt_test_mode(cd->thp_dev);

			thp_log_info(
				"%s: early suspend! g_thp_prox_enable = %d %d\n",
				__func__, g_thp_prox_enable, status);

			if (!status)
				cd->need_work_in_suspend = g_thp_prox_enable;
			else
				cd->need_work_in_suspend = 0;

			cd->prox_cache_enable = g_thp_prox_enable;
			cd->early_suspended = true;
			if (cd->need_work_in_suspend)
				thp_set_status(THP_STATUS_AFE_PROXIMITY,
					THP_PROX_ENTER, cd->panel_id);
			else
				thp_set_status(THP_STATUS_POWER, THP_SUSPEND, cd->panel_id);
		}
	}
}

static void thp_after_resume(struct thp_core_data *cd)
{
	if (cd->delay_work_for_pm) {
		thp_log_info("%s: after resume called\n", __func__);
		schedule_work(&thp_after_resume_work);
	} else {
		if (cd->proximity_support != PROX_SUPPORT) {
			thp_log_info("%s: after resume\n", __func__);
			if (cd->multi_panel_index == SINGLE_TOUCH_PANEL)
				thp_set_status(THP_STATUS_POWER, THP_RESUME, cd->panel_id);
			/*
			 * if multi screen and in quickly screen switch,
			 * not notify daemon
			 */
			else if (!cd->quickly_screen_on_off)
				thp_set_status(THP_STATUS_POWER, THP_RESUME, cd->panel_id);
		} else {
			thp_log_info(
				"%s: after resume! need_work_in_suspend = %d\n",
				__func__, cd->need_work_in_suspend);
			if (cd->need_work_in_suspend)
				thp_set_status(THP_STATUS_AFE_PROXIMITY,
					THP_PROX_EXIT, cd->panel_id);
			else
				thp_set_status(THP_STATUS_POWER, THP_RESUME, cd->panel_id);
		}
	}
	if (cd->wait_afe_screen_on_support &&
		(atomic_read(&(cd->afe_screen_on_waitq_flag)) != WAITQ_WAIT))
		atomic_set(&(cd->afe_screen_on_waitq_flag), WAITQ_WAIT);
	if (cd->esd_para.support_esd_power_reset)
		atomic_set(&cd->esd_para.esd_suspend_status_check, 0);
}
#endif

static int thp_fb_notifier_call(struct notifier_block *nb,
	unsigned long action, void *data)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
	struct fb_event *event = data;
	int *blank = NULL;

	if (!cd->thp_dev)
		return 0;

	if (!event)
		return NOTIFY_OK;

	if (!event->data)
		return NOTIFY_OK;

	if (action != FB_EVENT_BLANK)
		return 0;

	blank = event->data;

	switch (*blank) {
	case FB_BLANK_UNBLANK:
		thp_resume(cd);
		thp_after_resume(cd);
		break;
	case FB_BLANK_POWERDOWN:
		thp_early_suspend(cd);
		thp_suspend(cd);
		break;
	default:
		break;
	}

	return 0;
}

static struct notifier_block thp_fb_notifier_block = {
	.notifier_call = thp_fb_notifier_call,
};

#if ((!defined CONFIG_LCD_KIT_DRIVER) && (!defined CONFIG_HUAWEI_THP_UDP))
static int thp_lcdkit_notifier_callback(struct notifier_block *self,
	unsigned long event, void *data)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
	unsigned long pm_type = event;

	thp_log_debug("%s: called by lcdkit, pm_type=%lu\n", __func__, pm_type);

	switch (pm_type) {
	case LCDKIT_TS_EARLY_SUSPEND:
		/*
		* TDDI need make sure of firmware download complete,
		* then lcd send 2810 to screen off,
		* otherwise gesture mode will enter failed.
		*/
		if (cd->wait_afe_screen_on_support)
			thp_delay_before_screenoff(cd);
		/*
		 * to avoid easy_wakeup_info.sleep_mode changed during suspend,
		 * assign cd->sleep_mode to easy_wakeup_info.sleep_mode once
		 */
		cd->easy_wakeup_info.sleep_mode = cd->sleep_mode;
		if (cd->proximity_support != PROX_SUPPORT) {
			thp_log_info("%s: early suspend\n", __func__);
			thp_set_status(THP_STATUS_POWER, THP_SUSPEND, cd->panel_id);
		} else {
			thp_prox_suspend_record_time(cd);
			thp_log_info("%s:early suspend!g_thp_prox_enable=%d\n",
				__func__, g_thp_prox_enable);
			cd->need_work_in_suspend = g_thp_prox_enable;
			cd->prox_cache_enable = g_thp_prox_enable;
			cd->early_suspended = true;
			if (cd->need_work_in_suspend)
				thp_set_status(THP_STATUS_AFE_PROXIMITY,
					THP_PROX_ENTER, cd->panel_id);
			else
				thp_set_status(THP_STATUS_POWER, THP_SUSPEND, cd->panel_id);
		}
		break;

	case LCDKIT_TS_SUSPEND_DEVICE:
		thp_log_info("%s: suspend\n", __func__);
		thp_clean_fingers(cd->panel_id);
		break;

	case LCDKIT_TS_BEFORE_SUSPEND:
		thp_log_info("%s: before suspend\n", __func__);
		thp_suspend(cd);
		break;

	case LCDKIT_TS_RESUME_DEVICE:
		thp_log_info("%s: resume\n", __func__);
		thp_resume(cd);
		break;

	case LCDKIT_TS_AFTER_RESUME:
		if (cd->proximity_support != PROX_SUPPORT) {
			thp_log_info("%s: after resume\n", __func__);
			thp_set_status(THP_STATUS_POWER, THP_RESUME, cd->panel_id);
		} else {
			thp_log_info(
				"%s:after resume!need_work_in_suspend=%d\n",
				__func__, cd->need_work_in_suspend);
			if (cd->need_work_in_suspend)
				thp_set_status(THP_STATUS_AFE_PROXIMITY,
				THP_PROX_EXIT, cd->panel_id);
			else
				thp_set_status(THP_STATUS_POWER, THP_RESUME, cd->panel_id);
		}
		if (cd->wait_afe_screen_on_support &&
		    (atomic_read(&(cd->afe_screen_on_waitq_flag)) != WAITQ_WAIT))
		    atomic_set(&(cd->afe_screen_on_waitq_flag), WAITQ_WAIT);
		break;
	default:
		break;
	}

	return 0;
}
#endif

#ifdef CONFIG_HUAWEI_SHB_THP
#define POWER_OFF 0
#define POWER_ON 1
static int send_thp_cmd_to_shb(unsigned char power_status, struct thp_core_data *cd)
{
	int ret = -EINVAL;

	if (cd == NULL) {
		thp_log_err("%s: have null ptr\n", __func__);
		return ret;
	}
	if (cd->support_shb_thp == 0) {
		thp_log_info("%s: not support shb\n", __func__);
		return ret;
	}
	switch (power_status) {
	case POWER_OFF:
		if (!cd->poweroff_function_status) {
			ret = send_thp_close_cmd();
			thp_log_info("%s: close shb thp, ret is %d\n",
				__func__, ret);
		} else {
			cd->shb_waitq_flag = WAITQ_WAIT;
			ret = send_thp_driver_status_cmd(power_status,
				cd->poweroff_function_status,
				ST_CMD_TYPE_SET_SCREEN_STATUS);
			thp_log_info(
				"%s: call poweroff_function_status = 0x%x\n",
				__func__, cd->poweroff_function_status);
		}
		break;
	case POWER_ON:
		if (!cd->poweroff_function_status) {
			ret = send_thp_open_cmd();
			thp_log_info("%s: open shb thp, ret is %d\n",
				__func__, ret);
		}
		cd->shb_waitq_flag = WAITQ_WAIT;
		ret = send_thp_driver_status_cmd(power_status,
			POWER_KEY_ON_CTRL, ST_CMD_TYPE_SET_SCREEN_STATUS);
		thp_log_info("%s: power on and send status\n", __func__);
		break;
	default:
		thp_log_err("%s: invaild power_status: %u\n",
			__func__, power_status);
	}
	if (ret)
		thp_log_err("%s: ret = %d\n", __func__, ret);
	return ret;
}
#endif

void thp_release_vote_of_polymeric_chip(struct thp_core_data *cd)
{
	if (!cd) {
		thp_log_err("%s:cd is NULL\n", __func__);
		return;
	}
#ifdef CONFIG_CONTEXTHUB_IO_DIE_STS
	if (!cd->support_polymeric_chip)
		return;
	mutex_lock(&polymeric_chip_mutex);
	thp_log_info("%s: thp vote count_in : %u\n",
		__func__, atomic_read(&count_vote_for_polymeric_chip));
	if (atomic_read(&count_vote_for_polymeric_chip) > HIGH_PERFORMANCE_STATE) {
		/* multiple screen release surplus vote */
		atomic_dec(&count_vote_for_polymeric_chip);
		thp_log_info("%s: thp vote count_out : %u\n",
			__func__, atomic_read(&count_vote_for_polymeric_chip));
		mutex_unlock(&polymeric_chip_mutex);
		return;
	}
	if (atomic_read(&count_vote_for_polymeric_chip) == HIGH_PERFORMANCE_STATE) {
		if (sts_normal_release(STS_USER_TP) != 0) {
			thp_log_err("%s: thp slow speed vote failed\n", __func__);
			thp_log_info("%s: thp vote count_out : %u\n",
				__func__, atomic_read(&count_vote_for_polymeric_chip));
			mutex_unlock(&polymeric_chip_mutex);
			return;
		}
		atomic_dec(&count_vote_for_polymeric_chip);
	}
	thp_log_info("%s: thp vote count_out : %u\n",
		__func__, atomic_read(&count_vote_for_polymeric_chip));
	mutex_unlock(&polymeric_chip_mutex);
#endif
}
EXPORT_SYMBOL(thp_release_vote_of_polymeric_chip);

void thp_request_vote_of_polymeric_chip(struct thp_core_data *cd)
{
	if (!cd) {
		thp_log_err("%s:cd is NULL\n", __func__);
		return;
	}
#ifdef CONFIG_CONTEXTHUB_IO_DIE_STS
	if (!cd->support_polymeric_chip)
		return;
	mutex_lock(&polymeric_chip_mutex);
	thp_log_info("%s: thp vote count_in : %u\n",
		__func__, atomic_read(&count_vote_for_polymeric_chip));
	if (!atomic_read(&count_vote_for_polymeric_chip)) {
		if (sts_normal_request(STS_USER_TP) != 0) {
			thp_log_err("%s: thp high performance vote failed\n", __func__);
			thp_log_info("%s: thp vote count_out : %u\n",
				__func__, atomic_read(&count_vote_for_polymeric_chip));
			mutex_unlock(&polymeric_chip_mutex);
			return;
		}
	}
	atomic_inc(&count_vote_for_polymeric_chip);
	thp_log_info("%s: thp vote count_out : %u\n",
		__func__, atomic_read(&count_vote_for_polymeric_chip));
	mutex_unlock(&polymeric_chip_mutex);
#endif
}
EXPORT_SYMBOL(thp_request_vote_of_polymeric_chip);

#if defined(CONFIG_LCD_KIT_DRIVER)
int thp_proc_screen_switch_cmd(enum lcd_kit_ts_pm_type pm_type, int timeout,
	int panel_index)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
	int ret = 0;

	if (!cd) {
		thp_log_err("%s: tp is not registered\n", __func__);
		return -ENODEV;
	}
	thp_log_info("%s: called, index:%d, pm_type:%u\n", __func__,
		panel_index, pm_type);

	switch (panel_index) {
	case MAIN_TOUCH_PANEL:
		ret = thp_power_control_notify(pm_type, timeout);
		break;
	case SUB_TOUCH_PANEL:
#ifdef CONFIG_HUAWEI_TS_KIT_3_0
		if (cd->multi_panel_index != SINGLE_TOUCH_PANEL) {
			if (pm_type == TS_SUSPEND_DEVICE)
				cd->ts_suspended = true;
			if (pm_type == TS_AFTER_RESUME)
				cd->ts_suspended = false;
		}
		ret  = ts_kit_multi_power_control_notify(pm_type, timeout,
			panel_index);
#endif
		break;
	default:
		thp_log_err("%s: invalid index: %d\n", __func__, panel_index);
		return -EINVAL;
	}
	if (ret)
		thp_log_err("%s:control notify error\n", __func__);
	return ret;
}

#define PM_TYPE_LIST_LEN 5
enum lcd_kit_ts_pm_type pm_type_list[PM_TYPE_LIST_LEN] = {
	TS_RESUME_DEVICE,
	TS_AFTER_RESUME,
	TS_EARLY_SUSPEND,
	TS_BEFORE_SUSPEND,
	TS_SUSPEND_DEVICE
};

static void thp_call_back_event_switch(struct thp_core_data *cd)
{
	unsigned int stylus3_status;

	stylus3_status = thp_get_status(THP_STATUS_STYLUS3, cd->panel_id);
	if (cd->pen_supported && stylus3_status) {
		memset(&cd->stylus3_callback_event, 0, sizeof(cd->stylus3_callback_event));
		complete(&cd->stylus3_callback_flag);
	}
}

static void multi_screen_switch_unfold_for_dual_thp(void)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
	struct thp_core_data *cd_extra = thp_get_core_data(SUB_TOUCH_PANEL);

	thp_power_control(cd_extra, TS_SCREEN_SWITCH, NO_SYNC_TIMEOUT);
	thp_call_back_event_switch(cd_extra);
	if (cd->suspended && cd_extra->suspended) {
		thp_log_info("%s: screen off unfold\n", __func__);
		thp_multi_resume(cd_extra->panel_id);
		wait_event_interruptible_timeout(cd->screen_waitq,
			(cd->screen_waitq_flag == WAITQ_WAKEUP),
			msecs_to_jiffies(200));
	}
	thp_suspend_thread(cd_extra->panel_id);
	thp_switch_shb_active_panel(MAIN_TOUCH_PANEL);
	thp_resume_thread(cd->panel_id);
	thp_log_info("%s: switch to SCREEN_UNFOLD\n", __func__);
}

static void multi_screen_switch_fold_for_dual_thp(void)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
	struct thp_core_data *cd_extra = thp_get_core_data(SUB_TOUCH_PANEL);
	bool aod_ready_tmp;

	thp_power_control(cd, TS_SCREEN_SWITCH, NO_SYNC_TIMEOUT);
	thp_call_back_event_switch(cd);
	if (cd->suspended && cd_extra->suspended) {
		aod_ready_tmp = cd->aod_window_ready_status;
		cd_extra->not_change_aod_ready_status = true;
		thp_log_info("%s: screen off fold\n", __func__);
		cd_extra->aod_notify_tp_power_status = cd->aod_notify_tp_power_status;
		cd->quickly_screen_on_off = true;
		cd_extra->quickly_screen_on_off = true;
		thp_multi_resume(cd->panel_id);
		thp_suspend_thread(cd->panel_id);
		thp_switch_shb_active_panel(SUB_TOUCH_PANEL);
		thp_resume_thread(cd_extra->panel_id);
		thp_suspend_thread(cd_extra->panel_id);
		cd_extra->aod_window_ready_status = aod_ready_tmp ?
			aod_ready_tmp : cd_extra->aod_window_ready_status;
		cd_extra->not_change_aod_ready_status = false;
		thp_log_info("%s: switch to SCREEN_FOLD\n", __func__);
		return;
	}
	thp_suspend_thread(cd->panel_id);
	thp_switch_shb_active_panel(SUB_TOUCH_PANEL);
	thp_resume_thread(cd_extra->panel_id);
	thp_log_info("%s: switch to SCREEN_FOLD\n", __func__);
}

static void multi_screen_switch_unfold(struct thp_core_data *cd)
{
	thp_proc_screen_switch_cmd(TS_SCREEN_SWITCH,
		SHORT_SYNC_TIMEOUT, SUB_TOUCH_PANEL);
	if (cd->ts_suspended && cd->suspended) {
		thp_log_info("%s: screen off unfold\n", __func__);
#ifdef CONFIG_HUAWEI_TS_KIT_3_0
		ts_kit_screen_status_notify(QUICKLY_SCREEN_ON_OFF);
#endif
		tskit_multi_resume();
		msleep(200);
	}
	tskit_multi_suspend();
	thp_resume_thread(cd->panel_id);
	thp_log_info("%s: switch to SCREEN_UNFOLD\n", __func__);
}

static void multi_screen_switch_fold(struct thp_core_data *cd)
{
	thp_proc_screen_switch_cmd(TS_SCREEN_SWITCH,
		NO_SYNC_TIMEOUT, MAIN_TOUCH_PANEL);

	if (cd->ts_suspended && cd->suspended) {
		thp_log_info("%s: screen off fold\n", __func__);
		cd->quickly_screen_on_off = true;
		thp_multi_resume(cd->panel_id);
		msleep(200);
	}
	thp_suspend_thread(cd->panel_id);
	tskit_multi_resume();
	thp_log_info("%s: switch to SCREEN_FOLD\n", __func__);
}

static int multi_screen_status_switch_for_thp_kit(u32 current_status)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
	struct thp_core_data *cd_extra = NULL;

#ifdef CONFIG_HUAWEI_TS_KIT_3_0
	ts_kit_screen_status_notify(current_status);
#endif
	if (cd->dual_thp)
		cd_extra = thp_get_core_data(SUB_TOUCH_PANEL);

	switch (current_status) {
	case SCREEN_OFF_FOLD:
	case SCREEN_FOLDED:
		cd->multi_screen_fold = true;
		thp_log_info("%s:screen fold, status = %u\n",
			__func__, current_status);
		if (!cd->dual_thp) {
			multi_screen_switch_fold(cd);
		} else {
			cd_extra->multi_screen_fold = false;
			multi_screen_switch_fold_for_dual_thp();
		}
		break;
	case SCREEN_UNFOLD:
		cd->multi_screen_fold = false;
		thp_log_info("%s:screen unfold, status = %u\n",
			__func__, current_status);
		if (!cd->dual_thp) {
			multi_screen_switch_unfold(cd);
		} else {
			cd_extra->multi_screen_fold = true;
			multi_screen_switch_unfold_for_dual_thp();
		}
		break;
	default:
		thp_log_info("%s:invalid status = %u\n",
			__func__, current_status);
		break;
	}
	return 0;
}


static int multi_screen_status_switch_for_multi_thp(u32 current_status)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
	struct thp_core_data *cd_extra = thp_get_core_data(SUB_TOUCH_PANEL);

	switch (current_status) {
	case SCREEN_OFF_FOLD:
	case SCREEN_FOLDED:
		cd->multi_screen_fold = true;
		thp_log_info("%s:screen fold, status = %u\n",
			__func__, current_status);
		cd_extra->multi_screen_fold = false;
		multi_screen_switch_fold_for_dual_thp();
		break;
	case SCREEN_UNFOLD:
		cd->multi_screen_fold = false;
		thp_log_info("%s:screen unfold, status = %u\n",
			__func__, current_status);
		cd_extra->multi_screen_fold = true;
		multi_screen_switch_unfold_for_dual_thp();
		break;
	default:
		thp_log_info("%s:invalid status = %u\n",
			__func__, current_status);
		break;
	}
	return 0;
}

static int thp_send_screen_power_off_cmd(u32 current_status)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);

#ifdef CONFIG_HUAWEI_TS_KIT_3_0
	ts_kit_screen_status_notify(current_status);
#endif
	if ((current_status == SCREEN_OFF_FOLD) ||
		(current_status == SCREEN_FOLDED))
		cd->multi_screen_fold = true;
	else if (current_status == SCREEN_UNFOLD)
		cd->multi_screen_fold = false;
	else
		thp_log_err("%s: invalid status: %u\n",
			__func__, current_status);
	thp_log_info("%s: power off mode, no need switch power status\n",
		__func__);
	return 0;
}

static void thp_send_screen_off_fold_cmd(u32 current_status)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
	int i;

#ifdef CONFIG_HUAWEI_TS_KIT_3_0
	/* before fold, nofity sub panel */
	ts_kit_screen_status_notify(current_status);
#endif
	cd->quickly_screen_on_off = true;
	thp_proc_screen_switch_cmd(TS_SCREEN_SWITCH,
		NO_SYNC_TIMEOUT, MAIN_TOUCH_PANEL);
	for (i = 0; i < PM_TYPE_LIST_LEN; i++) {
		if (pm_type_list[i] == TS_EARLY_SUSPEND)
			msleep(200); /* 200ms wait ic ready */

		thp_proc_screen_switch_cmd(pm_type_list[i],
			SHORT_SYNC_TIMEOUT, SUB_TOUCH_PANEL);
		thp_proc_screen_switch_cmd(pm_type_list[i],
			NO_SYNC_TIMEOUT, MAIN_TOUCH_PANEL);
	}
	cd->multi_screen_fold = true;
	thp_log_info("%s: switch to SCREEN_OFF_FOLD\n", __func__);
}

static void thp_send_screen_on_fold_cmd(u32 current_status)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
	int i;

#ifdef CONFIG_HUAWEI_TS_KIT_3_0
	/* before fold, nofity sub panel */
	ts_kit_screen_status_notify(current_status);
#endif
	if (cd->ts_suspended && cd->suspended) {
		cd->quickly_screen_on_off = true;
		thp_proc_screen_switch_cmd(TS_SCREEN_SWITCH,
			NO_SYNC_TIMEOUT, MAIN_TOUCH_PANEL);
		for (i = 0; i < PM_TYPE_LIST_LEN; i++) {
			if (pm_type_list[i] == TS_EARLY_SUSPEND)
				msleep(200); /* 200ms wait ic ready */

			thp_proc_screen_switch_cmd(pm_type_list[i],
				SHORT_SYNC_TIMEOUT, SUB_TOUCH_PANEL);
			thp_proc_screen_switch_cmd(pm_type_list[i],
				NO_SYNC_TIMEOUT, MAIN_TOUCH_PANEL);
		}
	}

	if (cd->ts_suspended && !cd->suspended)
		thp_proc_screen_switch_cmd(TS_SCREEN_SWITCH,
			NO_SYNC_TIMEOUT, MAIN_TOUCH_PANEL);
	/* finish fold, notify main panel and it will power off */
	cd->multi_screen_fold = true;
	thp_log_info("%s: switch to SCREEN_FOLDED\n", __func__);
}

static void thp_send_screen_unfold_cmd(u32 current_status)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
	int i;

	/* before unfold, nofity main panel */
	cd->multi_screen_fold = false;
	thp_proc_screen_switch_cmd(TS_SCREEN_SWITCH,
		SHORT_SYNC_TIMEOUT, SUB_TOUCH_PANEL);
	if (cd->ts_suspended && cd->suspended) {
		for (i = 0; i < PM_TYPE_LIST_LEN; i++) {
			if (pm_type_list[i] == TS_EARLY_SUSPEND)
				msleep(200); /* 200ms wait ic ready */
			thp_proc_screen_switch_cmd(pm_type_list[i],
				SHORT_SYNC_TIMEOUT, SUB_TOUCH_PANEL);
		}
	}
#ifdef CONFIG_HUAWEI_TS_KIT_3_0
	/* finish unfold , notify sub panel and it will power off */
	ts_kit_screen_status_notify(current_status);
#endif
	thp_log_info("%s: switch to SCREEN_UNFOLD\n", __func__);
}

int multi_screen_status_switch(u32 current_status)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);

	if (!cd) {
		thp_log_err("%s: tp is not registered\n", __func__);
		return -ENODEV;
	}
	/*
	* fold_scheme == 0 TET; fold_scheme == 1 for thp_kit situation;
	* fold_scheme == 2 for default dual_thp situation;
	*/
	if (cd->fold_scheme == 1)
		return multi_screen_status_switch_for_thp_kit(current_status);
	if (cd->fold_scheme == 2)
		return multi_screen_status_switch_for_multi_thp(current_status);
	if ((cd->sleep_mode == TS_POWER_OFF_MODE) &&
		(cd->aod_touch_status == 0))
		return thp_send_screen_power_off_cmd(current_status);
	if (current_status == SCREEN_OFF_FOLD)
		thp_send_screen_off_fold_cmd(current_status);
	else if (current_status == SCREEN_FOLDED)
		thp_send_screen_on_fold_cmd(current_status);
	else if (current_status == SCREEN_UNFOLD)
		thp_send_screen_unfold_cmd(current_status);
	return 0;
}

static void thp_easy_wake_mode_change(struct thp_core_data *cd)
{
	if (cd->sleep_mode != TS_POWER_OFF_MODE) {
		cd->sleep_mode_temp = cd->sleep_mode;
		cd->sleep_mode = TS_POWER_OFF_MODE;
	}
	if (cd->aod_touch_status != TS_TOUCH_AOD_CLOSE) {
		cd->aod_mode_temp = cd->aod_touch_status;
		cd->aod_touch_status = TS_TOUCH_AOD_CLOSE;
	}
	if (cd->suspend_stylus3_status != TS_TOUCH_STYLUS3_CLOSE) {
		cd->suspend_stylus3_status_temp = cd->suspend_stylus3_status;
		cd->suspend_stylus3_status = TS_TOUCH_STYLUS3_CLOSE;
	}
	thp_log_info("%s:panel %d enter TS_SCREEN_SWITCH mode\n", __func__, cd->panel_id);
}

#if (defined(CONFIG_HUAWEI_THP_MTK) || defined(CONFIG_HUAWEI_THP_QCOM))
static void thp_default_report_finger_up(void)
{
	struct thp_udfp_data udfp_data;
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);

	if (!cd->use_ap_gesture)
		return;

	memset(&udfp_data, 0, sizeof(udfp_data));
	udfp_data.tpud_data.tp_event = TP_EVENT_FINGER_UP;
	thp_log_info("%s:default send fingerup event to fp\n", __func__);
	send_event_to_fingerprint_ud(udfp_data);
}
#endif

static void exit_tui_before_suspend(struct thp_core_data *cd,
	enum lcd_kit_ts_pm_type pm_type)
{
#if defined(CONFIG_TEE_TUI)
	mutex_lock(&tui_mutex);
	if (cd->send_tui_exit_in_suspend &&
		tui_enable &&
		(pm_type == TS_EARLY_SUSPEND)) {
		thp_log_info("In TUI mode, need exit TUI before suspend\n");
		thp_tui_secos_exit(cd->panel_id);
	}
	mutex_unlock(&tui_mutex);
#endif
}

static void multi_panel_early_suspend(struct thp_core_data *cd)
{
	if ((cd->multi_panel_index != SINGLE_TOUCH_PANEL) &&
		(cd->screen_switch_flag || cd->multi_screen_fold)) {
		thp_log_info("switch flag %d, fold flag %d\n",
			cd->screen_switch_flag, cd->multi_screen_fold);
		thp_easy_wake_mode_change(cd);
		cd->screen_switch_flag = false;
	}
}

static void multi_panel_suspend_device(struct thp_core_data *cd)
{
	if (cd->multi_panel_index != SINGLE_TOUCH_PANEL) {
		if (cd->sleep_mode_temp != TS_POWER_OFF_MODE) {
			cd->sleep_mode = cd->sleep_mode_temp;
			cd->sleep_mode_temp = TS_POWER_OFF_MODE;
			thp_log_info("panel %u sleep_mode revert\n", cd->panel_id);
		}
		if (cd->aod_mode_temp != TS_TOUCH_AOD_CLOSE) {
			cd->aod_touch_status = cd->aod_mode_temp;
			cd->aod_mode_temp = TS_TOUCH_AOD_CLOSE;
			thp_log_info("panel %u aod_mode revert\n", cd->panel_id);
		}
		if (cd->suspend_stylus3_status_temp != TS_TOUCH_STYLUS3_CLOSE) {
			cd->suspend_stylus3_status = cd->suspend_stylus3_status_temp;
			cd->suspend_stylus3_status_temp = TS_TOUCH_STYLUS3_CLOSE;
			thp_log_info("panel %u stylus3_status revert\n", cd->panel_id);
		}
	}
}

static void ts_before_suspend(struct thp_core_data *cd)
{
#ifdef CONFIG_HUAWEI_SHB_THP
	int ret;
#endif

	if (cd->support_daemon_init_protect &&
		atomic_read(&(cd->fw_update_protect))) {
		thp_log_info("%s: suspend when fw update, return directly\n",
			__func__);
		thp_set_status(THP_STATUS_POWER, THP_RESUME, cd->panel_id);
		atomic_set(&(cd->resend_suspend_after_fw_update), 1);
		return;
	}
#if (defined(CONFIG_HUAWEI_THP_MTK) || defined(CONFIG_HUAWEI_THP_QCOM))
	thp_default_report_finger_up();
#endif
	thp_suspend(cd);
#ifdef CONFIG_HUAWEI_SHB_THP
	if (cd->support_shb_thp_app_switch) {
		ret = send_thp_cmd_to_shb(POWER_OFF, cd);
		if (ret)
			thp_log_err("%s: send_thp_cmd_to_shb fail\n",
				__func__);
		return;
	}
	if (cd->support_shb_thp) {
		thp_log_info(
			"%s call poweroff_function_status = 0x%x\n",
			__func__, cd->poweroff_function_status);
		/* 0: power off */
		ret = send_thp_driver_status_cmd(0,
			cd->poweroff_function_status,
			ST_CMD_TYPE_SET_SCREEN_STATUS);
		if (ret)
			thp_log_err("%s: send_thp_status_off fail\n",
				__func__);
	}
#endif
}

static void ts_resume_device(struct thp_core_data *cd)
{
#ifdef CONFIG_HUAWEI_SHB_THP
	int ret;
	char fp_event_to_udfp[UDFP_EVENT_DATA_LEN] = {0};
	char switch_enable[SWITCH_ENABLE_LENGTH] = {0};
#endif
#ifdef CONFIG_HUAWEI_THP_MTK
	struct thp_udfp_data udfp_data;
#endif

	if (cd->support_daemon_init_protect &&
		atomic_read(&(cd->fw_update_protect)) &&
		atomic_read(&(cd->resend_suspend_after_fw_update))) {
		atomic_set(&(cd->resend_suspend_after_fw_update), 0);
		thp_log_info("%s: resume when fw update, return directly\n",
			__func__);
		return;
	}
	thp_resume(cd);
#ifdef CONFIG_HUAWEI_SHB_THP
	if (cd->support_shb_thp_app_switch) {
		ret = send_thp_cmd_to_shb(POWER_ON, cd);
		if (ret)
			thp_log_err("%s: send_thp_cmd_to_shb fail\n",
				__func__);
		return;
	}
	if (cd->support_shb_thp) {
		/* 1: power on */
		ret = send_thp_driver_status_cmd(1, POWER_KEY_ON_CTRL,
			ST_CMD_TYPE_SET_SCREEN_STATUS);
		if (ret)
			thp_log_err("%s: send_thp_status_on fail\n",
				__func__);
	}
	if (cd->tsa_event_to_udfp) {
	/* fp_event_to_udfp[UDFP_EVENT_DATA_LEN - 1] = 1 is finger up */
		fp_event_to_udfp[UDFP_EVENT_DATA_LEN - 1] = 1;
		ret = send_tp_ap_event(UDFP_EVENT_DATA_LEN,
			fp_event_to_udfp,
			ST_CMD_TYPE_FINGERPRINT_EVENT);
		if (ret < 0)
			thp_log_err("%s:tsa_event notify fp err %d\n",
				__func__, ret);
		if (cd->tsa_event_to_udfp == TAS_EVENT_TO_UDFP_MODE_TWO) {
			ret = send_tp_ap_event(SWITCH_ENABLE_LENGTH, switch_enable,
				ST_CMD_TYPE_IRQ_SWITCH);
			thp_log_info("%s:tsa_event_to_udfp_switch_disable, ret = %d\n",
				__func__, ret);
		}
	}
#endif
#if (defined(CONFIG_HUAWEI_THP_MTK) || defined(CONFIG_HUAWEI_THP_QCOM))
	thp_default_report_finger_up();
#endif
}

static void second_power_off(struct thp_core_data *cd)
{
	if (cd->thp_dev && cd->thp_dev->ops &&
		cd->thp_dev->ops->second_poweroff)
		cd->thp_dev->ops->second_poweroff();
}

static void thp_wait_shb_process_done(struct thp_core_data *cd)
{
	if (!cd) {
		thp_log_err("%s:cd is NULL\n", __func__);
		return;
	}
#ifdef CONFIG_HUAWEI_SHB_THP
	if (cd->support_shb_thp_app_switch) {
		if (wait_event_interruptible_timeout(cd->shb_waitq,
			(cd->shb_waitq_flag == WAITQ_WAKEUP),
			msecs_to_jiffies(150)))
			return;
		thp_log_err("%s:wait shb power off time out\n", __func__);
	}
#endif
}

static int pm_type_switch(struct thp_core_data *cd,
	enum lcd_kit_ts_pm_type pm_type)
{
	exit_tui_before_suspend(cd, pm_type);
	thp_log_info("%s: pm_type=%d, panel_id = %u\n", __func__, pm_type, cd->panel_id);
	/* just for multi-screen switch call, not put in switch-case */
	if (cd->multi_panel_index != SINGLE_TOUCH_PANEL &&
		pm_type == TS_SCREEN_SWITCH) {
		cd->screen_switch_flag = true;
		return 0;
	}
	switch (pm_type) {
	case TS_EARLY_SUSPEND:
		multi_panel_early_suspend(cd);
		thp_early_suspend(cd);
		break;

	case TS_SUSPEND_DEVICE:
		multi_panel_suspend_device(cd);
		thp_log_info("%s: suspend\n", __func__);
		thp_clean_fingers(cd->panel_id);
		thp_release_vote_of_polymeric_chip(cd);
		cd->event_flag = false;
		break;

	case TS_BEFORE_SUSPEND:
		thp_log_info("%s: before suspend\n", __func__);
		ts_before_suspend(cd);
		break;

	case TS_RESUME_DEVICE:
		thp_log_info("%s: resume\n", __func__);
		thp_request_vote_of_polymeric_chip(cd);
		thp_wait_shb_process_done(cd);
		ts_resume_device(cd);
		break;

	case TS_AFTER_RESUME:
		thp_wait_shb_process_done(cd);
		thp_after_resume(cd);
		break;
	case TS_2ND_POWER_OFF:
		second_power_off(cd);
		break;
	default:
		break;
	}

	return 0;
}

void esd_reset_sync(enum lcd_kit_ts_pm_type pm_type, struct thp_core_data *cd)
{
	if (cd->esd_para.support_esd_power_reset) {
		if (pm_type == TS_EARLY_SUSPEND) {
			thp_log_info("%s: in\n", __func__);
			atomic_set(&cd->esd_para.esd_suspend_status_check, 1);
			wait_event_interruptible(cd->esd_para.esd_waitq,
				(cd->esd_para.esd_waitq_flag == WAITQ_WAKEUP));
		}
	}
}

int thp_power_control_notify(enum lcd_kit_ts_pm_type pm_type, int timeout)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
	struct thp_core_data *extra_cd = thp_get_core_data(SUB_TOUCH_PANEL);
	int flag = ((cd != NULL) && (cd->thp_dev == NULL) && (extra_cd != NULL) &&
		(extra_cd->thp_dev != NULL) && extra_cd->dual_thp && !lcd_product_type);

	if (flag) {
		thp_log_info("%s: only support panel %d\n",
			__func__, lcd_panel_index);
		cd = extra_cd;
	}
	if (!cd) {
		thp_log_info("%s: cd is null\n", __func__);
		return -EINVAL;
	}

	if (thp_is_factory() && cd->always_poweron_in_screenoff) {
		thp_log_info("%s screenoff cap testing, NO poweroff\n",
			__func__);
		return 0;
	}
	thp_log_debug("%s: called by lcdkit, pm_type=%d\n", __func__, pm_type);
	esd_reset_sync(pm_type, cd);
#if (defined(CONFIG_HUAWEI_THP_MTK) || defined(CONFIG_HUAWEI_THP_QCOM))
	/* If the AP side AOD display function is enabled, including
	 * click-display, all-day display, and fingerprint, the
	 * AOD controls the TP power status.
	 */
	if (cd->use_aod_power_ctrl_notify && cd->aod_notify_tp_power_status &&
		(cd->multi_panel_index == SINGLE_TOUCH_PANEL)) {
		thp_log_info("%s: use AOD notify, return\n", __func__);
		return 0;
	}
#endif
	return pm_type_switch(cd, pm_type);
}

static int thp_power_control(struct thp_core_data *cd,
	enum lcd_kit_ts_pm_type pm_type, int timeout)
{
	if (thp_is_factory() && cd->always_poweron_in_screenoff) {
		thp_log_info("%s screenoff cap testing, NO poweroff\n",
			__func__);
		return 0;
	}
	thp_log_debug("%s: called by lcdkit, pm_type=%d\n", __func__, pm_type);
	return pm_type_switch(cd, pm_type);
}

#define PM_STATUS_BIT1 2
#define PM_STATUS_BIT0 1
static void thp_set_multi_pm_status(enum lcd_kit_ts_pm_type pm_type,
	int panel_index)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);

	if (panel_index == MAIN_TOUCH_PANEL) {
		if (pm_type == TS_EARLY_SUSPEND)
			cd->current_pm_status &= ~PM_STATUS_BIT1;
		else if (pm_type == TS_RESUME_DEVICE)
			cd->current_pm_status |= PM_STATUS_BIT1;
	} else if (panel_index == SUB_TOUCH_PANEL) {
		if (pm_type == TS_EARLY_SUSPEND)
			cd->current_pm_status &= ~PM_STATUS_BIT0;
		else if (pm_type == TS_RESUME_DEVICE)
			cd->current_pm_status |= PM_STATUS_BIT0;
	}
}

int thp_multi_power_control_notify(enum lcd_kit_ts_pm_type pm_type, int timeout,
	int panel_index)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
	int ret;
	struct thp_cmd_node cmd;
	bool first_screen_switch_flag;
#ifdef CONFIG_HUAWEI_TS_KIT_3_0
	static int boot_screen_off_panel;
#endif

	if (!cd) {
		thp_log_err("%s: tp is not registered\n", __func__);
		return -ENODEV;
	}

	thp_power_lock();
	if (cd->dual_thp)
		cd = thp_get_core_data(panel_index);
	first_screen_switch_flag = cd->need_check_first_screen_switch && !is_first_screen_switch();
	if (first_screen_switch_flag) {
		thp_log_info("%s: first_screen_switch is false, return\n", __func__);
		ret = NO_ERR;
		goto exit;
	}

#ifdef CONFIG_HUAWEI_THP_QCOM
	/* If the AP side AOD display function is enabled, including
	 * click-display, all-day display, and fingerprint, the
	 * AOD controls the TP power status.
	 */
	if (cd->use_aod_power_ctrl_notify && cd->aod_notify_tp_power_status) {
		thp_log_info("%s: panel %d use AOD notify, return\n", __func__, panel_index);
		ret = NO_ERR;
		goto exit;
	}
#endif
	thp_log_info("%s: called, index:%d, pm_type:%u\n", __func__,
		panel_index, pm_type);
#ifdef CONFIG_HUAWEI_TS_KIT_3_0
	/*
	 * when boot firstly, lcd will notify tp to trun off the unused panel
	 * and we should change the unused panel to power_off state
	 */
	if (boot_screen_off_panel == 0) {
#ifndef CONFIG_HUAWEI_THP_QCOM
		if (!cd->fold_scheme) {
			if (panel_index == MAIN_TOUCH_PANEL)
				cd->multi_screen_fold = true;
			else
				ts_kit_screen_status_notify(SCREEN_UNFOLD);
		}
#endif
		boot_screen_off_panel = BOOT_SCREEN_OFF_DONE;
	}
#endif
	if ((pm_type == TS_AFTER_RESUME) ||
		(pm_type == TS_BEFORE_SUSPEND) ||
		(pm_type == TS_SUSPEND_DEVICE)) {
		thp_log_info("%s: ignore %d", __func__, pm_type);
		ret = NO_ERR;
		goto exit;
	}
	memset(&cmd, 0, sizeof(cmd));
	if (panel_index == MAIN_TOUCH_PANEL) {
		cmd.cmd_param.panel_id = MAIN_TOUCH_PANEL;
		if (pm_type == TS_RESUME_DEVICE)
			cmd.command = THP_MUTIL_RESUME_THREAD;
		else
			cmd.command = THP_MUTIL_SUSPEND_THREAD;
	} else if (panel_index == SUB_TOUCH_PANEL && !cd->dual_thp) {
		if (pm_type == TS_RESUME_DEVICE)
			cmd.command = TSKIT_MUTIL_RESUME_THREAD;
		else
			cmd.command = TSKIT_MUTIL_SUSPEND_THREAD;
	} else if (panel_index == SUB_TOUCH_PANEL && cd->dual_thp) {
		cmd.cmd_param.panel_id = SUB_TOUCH_PANEL;
		if (pm_type == TS_RESUME_DEVICE)
			cmd.command = THP_MUTIL_RESUME_THREAD;
		else
			cmd.command = THP_MUTIL_SUSPEND_THREAD;
	} else {
		thp_log_err("%s: invalid index: %d\n", __func__, panel_index);
		ret = -EINVAL;
		goto exit;
	}
	cmd.cmd_param.pub_params.dev = cd->thp_dev;
	cmd.cmd_param.thread_id = get_available_thread_id();
	ret = thp_put_one_cmd(&cmd, 0);
	if (ret)
		thp_log_err("%s: put cmd error :%d\n", __func__, ret);
exit:
	thp_power_unlock();
	return ret;
}

static void thp_multi_resume(u32 panel_id)
{
	int err;
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	if (!cd || !cd->thp_dev) {
		thp_log_err("%s: null pointer\n", __func__);
		return;
	}
	thp_log_info("%s: panel_id = %u\n", __func__, panel_id);
	if (cd->suspended == false) {
		thp_log_err("%s already resumed, return\n", __func__);
		return;
	}

	err = thp_power_control(cd, TS_RESUME_DEVICE, NO_SYNC_TIMEOUT);
	if (err)
		thp_log_err("%s: TS_RESUME_DEVICE fail\n", __func__);
	err = thp_power_control(cd, TS_AFTER_RESUME, NO_SYNC_TIMEOUT);
	if (err)
		thp_log_err("%s: TS_AFTER_RESUME fail\n", __func__);
	thp_set_multi_pm_status(TS_RESUME_DEVICE, panel_id);
}

static void resend_power_status_to_daemon(struct thp_core_data *cd)
{
	if (!cd->support_resend_status_to_daemon)
		return;
	thp_log_err("%s: panel_id = %u, need resend power status! \n", __func__, cd->panel_id);
	thp_set_status(THP_STATUS_POWER, THP_SUSPEND, cd->panel_id);
}

static void thp_multi_suspend(u32 panel_id)
{
	int err;
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	if (!cd || !cd->thp_dev) {
		thp_log_err("%s: null pointer\n", __func__);
		return;
	}
	thp_log_info("%s: panel_id = %u\n", __func__, panel_id);
	if (cd->suspended == true) {
		thp_log_err("%s already suspended, return\n", __func__);

		/* power_off come together with screen_fold, when lcd notification is slower than screen_off_fold cmd,
		send power status to daemon to make sure the state is consistent */
		resend_power_status_to_daemon(cd);
		return;
	}

	thp_set_multi_pm_status(TS_EARLY_SUSPEND, panel_id);
	err = thp_power_control(cd, TS_EARLY_SUSPEND, NO_SYNC_TIMEOUT);
	if (err)
		thp_log_err("%s: TS_EARLY_SUSPEND fail\n", __func__);
	err = thp_power_control(cd, TS_BEFORE_SUSPEND, NO_SYNC_TIMEOUT);
	if (err)
		thp_log_err("%s: TS_BEFORE_SUSPEND fail\n", __func__);
	err = thp_power_control(cd, TS_SUSPEND_DEVICE, NO_SYNC_TIMEOUT);
	if (err)
		thp_log_err("%s: TS_SUSPEND_DEVICE fail\n", __func__);
}

static void tskit_multi_resume(void)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
#ifdef CONFIG_HUAWEI_TS_KIT_3_0
	int err;
#endif

	if (cd->ts_suspended == false) {
		thp_log_err("%s resumed, return\n", __func__);
		return;
	}
	cd->ts_suspended = false;
#ifdef CONFIG_HUAWEI_TS_KIT_3_0
	err = ts_kit_multi_power_control_notify(TS_RESUME_DEVICE,
		SHORT_SYNC_TIMEOUT, SUB_TOUCH_PANEL);
	if (err)
		thp_log_err("%s: TS_RESUME_DEVICE fail\n", __func__);
	err = ts_kit_multi_power_control_notify(TS_AFTER_RESUME,
		SHORT_SYNC_TIMEOUT, SUB_TOUCH_PANEL);
	if (err)
		thp_log_err("%s: TS_AFTER_RESUME fail\n", __func__);
#endif
	thp_set_multi_pm_status(TS_RESUME_DEVICE, SUB_TOUCH_PANEL);
}

static void tskit_multi_suspend(void)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
#ifdef CONFIG_HUAWEI_TS_KIT_3_0
	int err;
#endif

	if (cd->ts_suspended == true) {
		thp_log_err("%s suspended, return\n", __func__);
		return;
	}
	cd->ts_suspended = true;
	thp_set_multi_pm_status(TS_EARLY_SUSPEND, SUB_TOUCH_PANEL);
#ifdef CONFIG_HUAWEI_TS_KIT_3_0
	err = ts_kit_multi_power_control_notify(TS_EARLY_SUSPEND,
		SHORT_SYNC_TIMEOUT, SUB_TOUCH_PANEL);
	if (err)
		thp_log_err("%s: TS_EARLY_SUSPEND fail\n", __func__);
	err = ts_kit_multi_power_control_notify(TS_BEFORE_SUSPEND,
		SHORT_SYNC_TIMEOUT, SUB_TOUCH_PANEL);
	if (err)
		thp_log_err("%s: TS_BEFORE_SUSPEND fail\n", __func__);
	err = ts_kit_multi_power_control_notify(TS_SUSPEND_DEVICE,
		SHORT_SYNC_TIMEOUT, SUB_TOUCH_PANEL);
	if (err)
		thp_log_err("%s: TS_SUSPEND_DEVICE fail\n", __func__);
#endif
}

static void thp_suspend_thread(u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	if (!cd || !cd->thp_dev) {
		thp_log_err("%s: null pointer\n", __func__);
		return;
	}
	if (cd->suspended == true) {
		thp_log_err("%s already suspended, return\n", __func__);
		return;
	}
	/* refresh multi panel tui resume and suspend flag */
	if (cd->multi_panel_index != SINGLE_TOUCH_PANEL)
		thp_set_multi_pm_status(TS_EARLY_SUSPEND, cd->panel_id);
	pm_type_switch(cd, TS_EARLY_SUSPEND);
	msleep(cd->suspend_delayms_early_to_before);
	pm_type_switch(cd, TS_BEFORE_SUSPEND);
	msleep(cd->suspend_delayms_before_to_later);
	pm_type_switch(cd, TS_SUSPEND_DEVICE);
}

static void thp_resume_thread(u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	if (!cd || !cd->thp_dev) {
		thp_log_err("%s: null pointer\n", __func__);
		return;
	}
	if (cd->suspended == false) {
		thp_log_err("%s already resumed, return\n", __func__);
		return;
	}
	pm_type_switch(cd, TS_RESUME_DEVICE);
	msleep(cd->resume_delayms_early_to_later);
	pm_type_switch(cd, TS_AFTER_RESUME);
	/* refresh multi panel tui resume and suspend flag */
	if (cd->multi_panel_index != SINGLE_TOUCH_PANEL)
		thp_set_multi_pm_status(TS_RESUME_DEVICE, cd->panel_id);
}

void aod_status_notifier(struct thp_core_data *cd, unsigned int status)
{
	static unsigned int last_status = INVALID_VALUE;
	struct thp_cmd_node cmd;

	thp_log_info("%s: status is %u\n", __func__, status);
	if (!cd) {
		thp_log_err("%s: have null ptr\n", __func__);
		return;
	}
	if ((cd->multi_panel_index == SINGLE_TOUCH_PANEL) &&
		(status == last_status)) {
		thp_log_info("%s: repeat event, dont handle!!\n", __func__);
		return;
	}
	memset(&cmd, 0, sizeof(cmd));
	switch (status) {
	case AOD_NOTIFY_TP_SUSPEND:
		cmd.command = TS_SUSPEND;
		cmd.cmd_param.panel_id = cd->panel_id;
		break;
	case AOD_NOTIFY_TP_RESUME:
		cmd.command = TS_RESUME;
		cmd.cmd_param.panel_id = cd->panel_id;
		break;
	default:
		cmd.command = TS_INVAILD_CMD;
		cmd.cmd_param.panel_id = cd->panel_id;
		break;
	}
	cmd.cmd_param.thread_id = get_available_thread_id();
	last_status = status;
	if (thp_put_one_cmd(&cmd, NO_SYNC_TIMEOUT))
		thp_log_err("%s: put cmd error\n", __func__);
}

void tskit_aod_status_notifier(unsigned int status)
{
	struct thp_cmd_node cmd;

	thp_log_info("%s: status is %u\n", __func__, status);
	memset(&cmd, 0, sizeof(cmd));
	switch (status) {
	case AOD_NOTIFY_TP_SUSPEND:
		cmd.command = TSKIT_MUTIL_SUSPEND_THREAD;
		break;
	case AOD_NOTIFY_TP_RESUME:
		cmd.command = TSKIT_MUTIL_RESUME_THREAD;
		break;
	default:
		cmd.command = TS_INVAILD_CMD;
		break;
	}
	if (thp_put_one_cmd(&cmd, NO_SYNC_TIMEOUT))
		thp_log_err("%s: put cmd error\n", __func__);
}
#endif

static int thp_open(struct inode *inode, struct file *filp)
{
	int ret;
	struct thp_core_data *cd = container_of((struct miscdevice *)filp->private_data,
		struct thp_core_data, thp_misc_device);

	thp_log_info("%s: called panel %u\n", __func__, cd->panel_id);

	if (cd->support_fake_panel && g_is_boot_proc) {
		thp_log_err("%s:reset count = %d\n", __func__, g_daemon_reset_count);
		if (g_daemon_reset_count != DAEMON_RESET_MAX_COUNT)
			g_daemon_reset_count++;
	}

	mutex_lock(&cd->thp_mutex);
	if (cd->open_count) {
		thp_log_err("%s: dev have be opened\n", __func__);
		mutex_unlock(&cd->thp_mutex);
		return -EBUSY;
	}

	cd->open_count++;
	mutex_unlock(&cd->thp_mutex);
	cd->reset_flag = 0; /* current isn't in reset status */
	cd->get_frame_block_flag = THP_GET_FRAME_BLOCK;

	cd->frame_size = THP_MAX_FRAME_SIZE;
#ifdef THP_NOVA_ONLY
	cd->frame_size = NT_MAX_FRAME_SIZE;
#endif
	cd->timeout = THP_DEFATULT_TIMEOUT_MS;

	/*
	 * Daemon default is 0
	 * setting to 1 will trigger daemon to init or restore the status
	 */
	__set_bit(THP_STATUS_WINDOW_UPDATE, (unsigned long *)&cd->status);
	__set_bit(THP_STATUS_TOUCH_SCENE, (unsigned long *)&cd->status);

	thp_log_info("%s: cd->status = 0x%x\n", __func__, cd->status);
	mutex_lock(&cd->mutex_frame);
	thp_clear_frame_buffer(cd);
	mutex_unlock(&cd->mutex_frame);
	/* restore spi config */
	ret = thp_set_spi_max_speed(cd->spi_config.max_speed_hz, cd->panel_id);
	if (ret)
		thp_log_err("%s: thp_set_spi_max_speed error\n", __func__);

	return 0;
}

static int thp_release(struct inode *inode, struct file *filp)
{
	struct thp_core_data *cd = container_of((struct miscdevice *)filp->private_data,
		struct thp_core_data, thp_misc_device);

	thp_log_info("%s: called, panel_id = %u\n", __func__, cd->panel_id);

	mutex_lock(&cd->thp_mutex);
	cd->open_count--;
	if (cd->open_count < 0) {
		thp_log_err("%s: abnormal release\n", __func__);
		cd->open_count = 0;
	}
	mutex_unlock(&cd->thp_mutex);

	thp_wake_up_frame_waitq(cd);
	if (!(cd->need_enable_irq_wake && cd->irq_wake_flag))
		thp_set_irq_status(cd, THP_IRQ_DISABLE);

	return 0;
}

static int thp_spi_sync_alloc_mem(struct thp_core_data *cd)
{
	if (cd->spi_sync_rx_buf || cd->spi_sync_tx_buf) {
		thp_log_debug("%s: has requested memory\n", __func__);
		return 0;
	}

	cd->spi_sync_rx_buf = kzalloc(THP_SYNC_DATA_MAX, GFP_KERNEL);
	if (!cd->spi_sync_rx_buf) {
		thp_log_err("%s:spi_sync_rx_buf request memory fail\n",
			__func__);
		goto exit;
	}
	cd->spi_sync_tx_buf = kzalloc(THP_SYNC_DATA_MAX, GFP_KERNEL);
	if (!cd->spi_sync_tx_buf) {
		thp_log_err("%s:spi_sync_tx_buf request memory fail\n",
			__func__);
		kfree(cd->spi_sync_rx_buf);
		cd->spi_sync_rx_buf = NULL;
		goto exit;
	}
	return 0;

exit:
	return -ENOMEM;
}

static unsigned int thp_get_spi_msg_lens(
	struct thp_ioctl_spi_xfer_data *spi_data, unsigned int xfer_num)
{
	int length = 0;
	int i;

	if (!spi_data || !xfer_num || (xfer_num > MAX_SPI_XFER_DATA_NUM)) {
		thp_log_err("%s:invalid input\n", __func__);
		return 0;
	}
	for (i = 0; i < xfer_num; i++) {
		if ((spi_data[i].len == 0) ||
			(spi_data[i].len > THP_MAX_FRAME_SIZE)) {
			thp_log_err("%s:spi_data[i].len invalid\n", __func__);
			return 0;
		}
		length += spi_data[i].len;
		thp_log_debug("%s: spi_data[i].len = %d\n",
			__func__, spi_data[i].len);
	}
	return length;
}

static long thp_ioctl_multiple_spi_xfer_sync(
	const void __user *data, unsigned int lock_status, u32 panel_id)
{
	struct thp_ioctl_spi_msg_package ioctl_spi_msg;
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	struct thp_ioctl_spi_xfer_data *ioctl_spi_xfer = NULL;
	struct spi_message msg;
	struct spi_transfer *xfer = NULL;
	int rc;
	int i;
	unsigned int current_speed = 0;
	u8 *tx_buf = NULL;
	u8 *rx_buf = NULL;
	unsigned int msg_len;

	memset(&ioctl_spi_msg, 0, sizeof(ioctl_spi_msg));
	if (cd->suspended || (!data)) {
		thp_log_info("%s:suspended or invalid data,return\n", __func__);
		return -EIO;
	}
#if defined(CONFIG_TEE_TUI)
	if (tui_enable) {
		thp_log_info("%s:TUI status,return!\n", __func__);
		return -EIO;
	}
#endif

	if (copy_from_user(&ioctl_spi_msg, data, sizeof(ioctl_spi_msg))) {
		thp_log_err("%s:Failed to copy spi data\n", __func__);
		return -EFAULT;
	}
	if ((ioctl_spi_msg.xfer_num > MAX_SPI_XFER_DATA_NUM) ||
		!ioctl_spi_msg.xfer_num) {
		thp_log_err("xfer_num:%d\n", ioctl_spi_msg.xfer_num);
		return -EINVAL;
	}

	ioctl_spi_xfer = kcalloc(ioctl_spi_msg.xfer_num,
			sizeof(*ioctl_spi_xfer), GFP_KERNEL);
	if (!ioctl_spi_xfer) {
		thp_log_err("%s:failed alloc memory for spi_xfer_data\n",
			__func__);
		return -EFAULT;
	}
	if (ioctl_spi_msg.xfer_data) {
		if (copy_from_user(ioctl_spi_xfer, ioctl_spi_msg.xfer_data,
			sizeof(*ioctl_spi_xfer) * ioctl_spi_msg.xfer_num)) {
			thp_log_err("%s:failed copy xfer_data\n", __func__);
			rc = -EINVAL;
			goto exit;
		}
	}
	msg_len = thp_get_spi_msg_lens(ioctl_spi_xfer, ioctl_spi_msg.xfer_num);
	if (msg_len > THP_MAX_FRAME_SIZE || !msg_len) {
		thp_log_err("%s:invalid msg len :%d\n", __func__, msg_len);
		rc = -EINVAL;
		goto exit;
	}

	xfer = kcalloc(ioctl_spi_msg.xfer_num, sizeof(*xfer), GFP_KERNEL);
	rx_buf = kzalloc(msg_len, GFP_KERNEL);
	tx_buf = kzalloc(msg_len, GFP_KERNEL);
	if (!rx_buf || !tx_buf || !xfer) {
		thp_log_err("%s:failed alloc buffer\n", __func__);
		rc = -EINVAL;
		goto exit;
	}

	spi_message_init(&msg);
	for (i = 0, msg_len = 0; i < ioctl_spi_msg.xfer_num; i++) {
		if (ioctl_spi_xfer[i].tx) {
			rc = copy_from_user(tx_buf + msg_len,
					ioctl_spi_xfer[i].tx,
					ioctl_spi_xfer[i].len);
			if (rc) {
				thp_log_err("%s:failed copy tx_buf:%d\n",
					__func__, rc);
				goto exit;
			}
		}
		xfer[i].tx_buf = tx_buf + msg_len;
		xfer[i].rx_buf = rx_buf + msg_len;
		xfer[i].len = ioctl_spi_xfer[i].len;
		xfer[i].cs_change = !!ioctl_spi_xfer[i].cs_change;
		xfer[i].delay_usecs = ioctl_spi_xfer[i].delay_usecs;

		thp_log_debug(
			"%s:%d, cs_change=%d,len =%d,delay_usecs=%d\n",
			__func__, i, ioctl_spi_xfer[i].cs_change,
			ioctl_spi_xfer[i].len, ioctl_spi_xfer[i].delay_usecs);
		spi_message_add_tail(&xfer[i], &msg);
		msg_len += ioctl_spi_xfer[i].len;
	}
	if (lock_status == NEED_LOCK) {
		rc = thp_bus_lock();
		if (rc) {
			thp_log_info("%s:failed get lock:%d", __func__, rc);
			goto exit;
		}
	}

	if (ioctl_spi_msg.speed_hz != 0) {
		thp_log_debug("%s change to 3.5k-> speed =%d\n",
			__func__, ioctl_spi_msg.speed_hz);
		current_speed = cd->sdev->max_speed_hz;
		cd->sdev->max_speed_hz = ioctl_spi_msg.speed_hz;
		rc = thp_setup_spi(cd);
		if (rc) {
			thp_log_err("%s:set max speed failed rc:%d",
				__func__, rc);
			if (lock_status == NEED_LOCK)
				thp_bus_unlock();
			goto exit;
		}
	}

	rc = thp_spi_sync(cd->sdev, &msg);
	if (rc) {
		thp_log_err("%s:failed sync msg:%d", __func__, rc);
		if (lock_status == NEED_LOCK)
			thp_bus_unlock();
		goto exit;
	}
	if (ioctl_spi_msg.speed_hz != 0) {
		thp_log_debug("%s current_speed-> %d\n",
			__func__, current_speed);
		cd->sdev->max_speed_hz = current_speed;
		rc = thp_setup_spi(cd);
		if (rc) {
			thp_log_err("%s:set max speed failed rc:%d",
				__func__, rc);
			if (lock_status == NEED_LOCK)
				thp_bus_unlock();
			goto exit;
		}
	}
	if (lock_status == NEED_LOCK)
		thp_bus_unlock();

	for (i = 0, msg_len = 0; i < ioctl_spi_msg.xfer_num; i++) {
		if (ioctl_spi_xfer[i].rx) {
			rc = copy_to_user(ioctl_spi_xfer[i].rx,
					rx_buf + msg_len,
					ioctl_spi_xfer[i].len);
			if (rc) {
				thp_log_err("%s:copy to rx buff fail:%d",
					__func__, rc);
				goto exit;
			}
		}
		msg_len += ioctl_spi_xfer[i].len;
	}
exit:
	kfree(ioctl_spi_xfer);
	kfree(xfer);
	kfree(rx_buf);
	kfree(tx_buf);
	return rc;
}

static void try_recovery_fake_panel_status(struct thp_core_data *cd,
	u8 *tx_buf, u8 *rx_buf, unsigned int size)
{
	if (cd->support_fake_panel
		&& g_daemon_reset_count == DAEMON_RESET_MAX_COUNT
		&& size > 5) {
		if (tx_buf[0] == 0xF1 && tx_buf[1] == 0x21 && tx_buf[2] == 0xE4
			&& tx_buf[3] == 0xFF && rx_buf[5] != 0xFC && rx_buf[5] != 0xBE) {
			thp_log_err("rx_buf[5] = 0x%x to 0xFC\n", rx_buf[5]);
#if defined(CONFIG_HUAWEI_DSM)
			if (g_is_boot_proc) // just report once on boot
				thp_dmd_report(DSM_TP_DEV_STATUS_ERROR_NO,
				"goodix fake panel, value is 0x%x\n", rx_buf[5]);
#endif
			rx_buf[5] = 0xFC;
		}
	}
}

#define MAX_IOCTL_COUNT_IN_SUSPEND 10
static long thp_spi_sync_common(const void __user *data,
	unsigned int lock_status, u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	struct thp_ioctl_spi_sync_data sync_data;
	struct thp_ioctl_spi_sync_data_compat sync_data_compat;
	int rc = 0;
	u8 *tx_buf = NULL;
	u8 *rx_buf = NULL;

	memset(&sync_data, 0, sizeof(sync_data));
	memset(&sync_data_compat, 0, sizeof(sync_data_compat));
	if (!data) {
		thp_log_err("%s: input parameter null\n", __func__);
		return -EINVAL;
	}

#if defined(CONFIG_TEE_TUI)
	if (tui_enable)
		return 0;
#endif

	if (cd->compat_flag == true) {
		if (copy_from_user(&sync_data_compat, data,
			sizeof(sync_data_compat))) {
			thp_log_err("Failed to copy_from_user\n");
			return -EFAULT;
		}
		sync_data.rx = compat_ptr(sync_data_compat.rx);
		sync_data.tx = compat_ptr(sync_data_compat.tx);
		sync_data.size = sync_data_compat.size;
	} else {
		if (copy_from_user(&sync_data, data,
			sizeof(sync_data))) {
			thp_log_err("Failed to copy_from_user\n");
			return -EFAULT;
		}
	}

	if (sync_data.size > THP_SYNC_DATA_MAX) {
		thp_log_err("sync_data.size out of range\n");
		return -EINVAL;
	}

	rc = thp_spi_sync_alloc_mem(cd);
	if (!rc) {
		rx_buf = cd->spi_sync_rx_buf;
		tx_buf = cd->spi_sync_tx_buf;
	} else {
		thp_log_err("%s:buf request memory fail\n", __func__);
		goto exit;
	}

	rc = copy_from_user(tx_buf, sync_data.tx, sync_data.size);
	if (rc) {
		thp_log_err("%s:copy in buff fail\n", __func__);
		goto exit;
	}
	if (lock_status == NEED_LOCK) {
		if (cd->thp_dev->ops->spi_transfer)
			rc = cd->thp_dev->ops->spi_transfer(tx_buf,
				rx_buf, sync_data.size);
		else
			rc = thp_spi_transfer(cd, tx_buf, rx_buf,
				sync_data.size, lock_status);
	} else {
		rc = thp_spi_transfer(cd, tx_buf, rx_buf,
			sync_data.size, lock_status);
	}
	if (rc) {
		thp_log_err("%s: transfer error, ret = %d\n", __func__, rc);
		goto exit;
	}

	try_recovery_fake_panel_status(cd, tx_buf, rx_buf, sync_data.size);

	if (sync_data.rx) {
		rc = copy_to_user(sync_data.rx, rx_buf, sync_data.size);
		if (rc) {
			thp_log_err("%s:copy out buff fail\n", __func__);
			goto exit;
		}
	}

exit:
	return rc;
}

static long thp_ioctl_spi_sync_in_suspend(const void __user *data,
	unsigned int lock_status, u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	int rc = 0;

	if ((!cd->need_spi_communication_in_suspend) || (!cd->suspended)) {
		thp_log_info("%s invalid called return\n", __func__);
		return 0;
	}
	thp_log_info("%s spi communication in panel %d suspend\n", __func__, panel_id);
	__pm_wakeup_event(cd->thp_wake_lock, jiffies_to_msecs(HZ));
	rc = thp_spi_sync_common(data, lock_status, panel_id);

	return rc;
}

#define MAX_IOCTL_COUNT_IN_SUSPEND 10
static long thp_ioctl_spi_sync(const void __user *data,
	unsigned int lock_status, u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	int rc = 0;

	thp_log_debug("%s: called\n", __func__);

	if (cd->suspended && (!cd->need_work_in_suspend)) {
		if (spi_sync_count_in_suspend < MAX_IOCTL_COUNT_IN_SUSPEND)
			thp_log_info("%s panel %d suspended return\n", __func__, panel_id);
		spi_sync_count_in_suspend++;
		return cd->spi_in_suspend_return_negative ? -EINVAL : 0;
	}
	rc = thp_spi_sync_common(data, lock_status, panel_id);
	return rc;
}

static long thp_ioctl_finish_notify(unsigned long arg, u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	unsigned long event_type = arg;
	int rc;
	struct thp_device_ops *ops = cd->thp_dev->ops;

	thp_log_info("%s: called\n", __func__);
	switch (event_type) {
	case THP_AFE_NOTIFY_FW_UPDATE:
		rc = ops->afe_notify ?
			ops->afe_notify(cd->thp_dev, event_type) : 0;
		break;
	default:
		thp_log_err("%s: illegal event type\n", __func__);
		rc = -EINVAL;
	}
	return rc;
}

static long thp_ioctl_get_frame_count(unsigned long arg, u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	u32 __user *frame_cnt = (u32 *)arg;

	if (cd->frame_count)
		thp_log_info("%s:frame_cnt=%d\n", __func__, cd->frame_count);

	if (frame_cnt == NULL) {
		thp_log_err("%s: input parameter null\n", __func__);
		return -EINVAL;
	}

	if (copy_to_user(frame_cnt, &cd->frame_count, sizeof(u32))) {
		thp_log_err("%s:copy frame_cnt failed\n", __func__);
		return -EFAULT;
	}

	return 0;
}

static long thp_ioctl_clear_frame_buffer(u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	if (cd->frame_count == 0)
		return 0;

	thp_log_info("%s called: panel %d\n", __func__, panel_id);
	mutex_lock(&cd->mutex_frame);
	thp_clear_frame_buffer(cd);
	mutex_unlock(&cd->mutex_frame);
	return 0;
}

static long thp_get_frame_from_thp_queue(
	struct thp_ioctl_get_frame_data *data, u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	struct thp_queue_node *temp_frame = NULL;
	long rc = 0;

	if (data->size > g_thp_queue_node_len) {
		thp_log_debug("%s data size too big, set to %u\n",
			__func__, g_thp_queue_node_len);
		data->size = g_thp_queue_node_len;
	}
	cd->frame_size = data->size;
	mutex_lock(&cd->suspend_flag_mutex);
	if (cd->suspended && (!cd->need_work_in_suspend)) {
		thp_log_info("%s: drv suspended\n", __func__);
		mutex_unlock(&cd->suspend_flag_mutex);
		return -EINVAL;
	}
	thp_set_irq_status(cd, THP_IRQ_ENABLE);
	mutex_unlock(&cd->suspend_flag_mutex);
	if (thp_queue_is_empty(cd->thp_queue) &&
		cd->get_frame_block_flag) {
		if (thp_wait_frame_waitq(cd))
			rc = -ETIMEDOUT;
	}
	mutex_lock(&cd->mutex_frame);
	if (thp_queue_is_empty(cd->thp_queue) == false) {
		temp_frame = thp_queue_get_head(cd->thp_queue);
		if (temp_frame == NULL) {
			thp_log_err("Failed to temp_frame is NULL\n");
			rc = -EFAULT;
			goto out;
		}
		if (data->buf == NULL) {
			thp_log_err("Failed to data buf is NULL\n");
			rc = -EFAULT;
			goto out;
		}
		if (copy_to_user(data->buf, temp_frame->buf,
			cd->frame_size)) {
			thp_log_err("Failed to copy_to_user\n");
			rc = -EFAULT;
			goto out;
		}
		if (data->tv == NULL) {
			thp_log_err("Failed to data tv is NULL\n");
			rc = -EFAULT;
			goto out;
		}
		if (copy_to_user(data->tv, &(temp_frame->tv),
			sizeof(temp_frame->tv))) {
			thp_log_err("Failed to copy_to_user tv\n");
			rc = -EFAULT;
			goto out;
		}
		if (!thp_queue_dequeue(cd->thp_queue))
			thp_log_err("%s queue remove failed\n", __func__);
		temp_frame = NULL;
		cd->frame_count--;
		rc = 0;
	} else {
		thp_log_err("%s:no frame\n", __func__);
		/*
		 * When wait timeout, try to get data.
		 * If timeout and no data, return -ETIMEDOUT
		 */
		if (rc != -ETIMEDOUT)
			rc = -ENODATA;
	}
out:
	mutex_unlock(&cd->mutex_frame);
	trace_touch(TOUCH_TRACE_GET_FRAME, TOUCH_TRACE_FUNC_OUT,
		rc ? "no frame" : "with frame");
	return rc;
}

static long thp_ioctl_get_frame(unsigned long arg, u32 panel_id)
{
	struct thp_ioctl_get_frame_data data;
	struct thp_ioctl_get_frame_data_compat data_compat;
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	void __user *argp = (void __user *)arg;
	long rc = 0;

	memset(&data, 0, sizeof(data));
	memset(&data_compat, 0, sizeof(data_compat));
	trace_touch(TOUCH_TRACE_GET_FRAME, TOUCH_TRACE_FUNC_IN, "thp");

	if (!arg) {
		thp_log_err("%s: input parameter null\n", __func__);
		return -EINVAL;
	}

	if (cd->suspended && (!cd->need_work_in_suspend)) {
		if (get_frame_count_in_suspend < MAX_IOCTL_COUNT_IN_SUSPEND)
			thp_log_info("%s: drv suspended\n", __func__);
		get_frame_count_in_suspend++;
		return -ETIMEDOUT;
	}

	if (cd->compat_flag == true) {
		if (copy_from_user(&data_compat, argp,
			sizeof(data_compat))) {
			thp_log_err("Failed to copy_from_user\n");
			return -EFAULT;
		}
		data.buf = compat_ptr(data_compat.buf);
		data.tv = compat_ptr(data_compat.tv);
		data.size = data_compat.size;
	} else {
		if (copy_from_user(&data, argp,
			sizeof(data))) {
			thp_log_err("Failed to copy_from_user\n");
			return -EFAULT;
		}
	}

	if (data.buf == 0 || data.size == 0 ||
		data.size > THP_MAX_FRAME_SIZE || data.tv == 0) {
		thp_log_err("%s:input buf invalid\n", __func__);
		return -EINVAL;
	}
	if (cd->use_thp_queue)
		return thp_get_frame_from_thp_queue(&data, cd->panel_id);

	cd->frame_size = data.size;
	mutex_lock(&cd->suspend_flag_mutex);
	if (cd->suspended && (!cd->need_work_in_suspend)) {
		thp_log_info("%s: drv suspended\n", __func__);
		mutex_unlock(&cd->suspend_flag_mutex);
		return -EINVAL;
	}
	thp_set_irq_status(cd, THP_IRQ_ENABLE);
	mutex_unlock(&cd->suspend_flag_mutex);
	if (list_empty(&cd->frame_list.list) && cd->get_frame_block_flag) {
		if (thp_wait_frame_waitq(cd))
			rc = -ETIMEDOUT;
	}

	mutex_lock(&cd->mutex_frame);
	if (!list_empty(&cd->frame_list.list)) {
		struct thp_frame *temp;

		temp = list_first_entry(&cd->frame_list.list,
				struct thp_frame, list);
		if (data.buf == NULL) {
			thp_log_err("Failed to copy_to_user()\n");
			rc = -EFAULT;
			goto out;
		}
		if (copy_to_user(data.buf, temp->frame, cd->frame_size)) {
			thp_log_err("Failed to copy_to_user()\n");
			rc = -EFAULT;
			goto out;
		}

		if (data.tv == NULL) {
			thp_log_err("Failed to copy_to_user()\n");
			rc = -EFAULT;
			goto out;
		}
		if (copy_to_user(data.tv, &(temp->tv),
					sizeof(struct timespec64))) {
			thp_log_err("Failed to copy_to_user()\n");
			rc = -EFAULT;
			goto out;
		}

		list_del(&temp->list);
		kfree(temp);
		cd->frame_count--;
		rc = 0;
	} else {
		thp_log_err("%s:no frame\n", __func__);
		/*
		 * When wait timeout, try to get data.
		 * If timeout and no data, return -ETIMEDOUT
		 */
		if (rc != -ETIMEDOUT)
			rc = -ENODATA;
	}
out:
	mutex_unlock(&cd->mutex_frame);
	trace_touch(TOUCH_TRACE_GET_FRAME, TOUCH_TRACE_FUNC_OUT,
		rc ? "no frame" : "with frame");
	return rc;
}

static long thp_ioctl_reset(unsigned long reset, u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);
#ifndef CONFIG_HUAWEI_THP_MTK
	int value;
#endif

	thp_log_info("%s:set panel %d reset status %lu\n", __func__, panel_id, reset);

#ifndef CONFIG_HUAWEI_THP_MTK
	gpio_set_value(cd->gpios.rst_gpio, !!reset);
	value = gpio_get_value(cd->gpios.rst_gpio);
	thp_log_info("%s: get rst value : %d\n", __func__, value);
#else
	if (cd->support_pinctrl == 0) {
		thp_log_info("%s: not support pinctrl\n", __func__);
		return 0;
	}

	if (!!reset)
		pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.reset_high);
	else
		pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.reset_low);
#endif

	if ((!!reset) && (cd->send_bt_status_to_fw) &&
		(!cd->enter_stylus3_mmi_test) &&
		(cd->thp_dev->ops->bt_handler)) {
		if (cd->thp_dev->ops->bt_handler(cd->thp_dev, true))
			thp_log_err("ioctl reset send bt status fail\n");
	}
	cd->frame_waitq_flag = WAITQ_WAIT;
	cd->reset_flag = !reset;
	thp_log_info("%s:out\n", __func__);
	return 0;
}

static long thp_ioctl_hw_lock_status(unsigned long arg, u32 panel_id)
{
	thp_log_err("%s: set hw lock status %lu from panel %u\n", __func__, arg, panel_id);
	return -EINVAL;
}

static long thp_ioctl_set_timeout(unsigned long arg, u32 panel_id)
{
	struct thp_core_data *ts = thp_get_core_data(panel_id);
	unsigned int timeout_ms = min_t(unsigned int, arg, THP_WAIT_MAX_TIME);

	thp_log_info("set panel %u wait time %d ms.(current %dms)\n", panel_id,
			timeout_ms, ts->timeout);

	if (timeout_ms != ts->timeout) {
		ts->timeout = timeout_ms;
		thp_wake_up_frame_waitq(ts);
	}

	return 0;
}

static long thp_ioctl_set_block(unsigned long arg, u32 panel_id)
{
	struct thp_core_data *ts = thp_get_core_data(panel_id);
	unsigned int block_flag = arg;

	if (block_flag)
		ts->get_frame_block_flag = THP_GET_FRAME_BLOCK;
	else
		ts->get_frame_block_flag = THP_GET_FRAME_NONBLOCK;

	thp_log_info("%s:set block %d\n", __func__, block_flag);

	// if received get block from daemon, ic reset is normal, no need to retry
	if (ts->support_fake_panel && g_is_boot_proc && block_flag) {
		g_is_boot_proc = false;
		thp_log_info("%s:is_boot_proc is false\n", __func__);
	}

	thp_wake_up_frame_waitq(ts);
	return 0;
}


static long thp_ioctl_set_irq(unsigned long arg, u32 panel_id)
{
	struct thp_core_data *ts = thp_get_core_data(panel_id);
	unsigned int irq_en = (unsigned int)arg;

	mutex_lock(&ts->suspend_flag_mutex);
	if (ts->suspended && (!ts->need_work_in_suspend)) {
		thp_log_info("%s: panel %u drv suspended\n", __func__, panel_id);
		mutex_unlock(&ts->suspend_flag_mutex);
		return -EINVAL;
	}
	thp_set_irq_status(ts, irq_en);
	mutex_unlock(&ts->suspend_flag_mutex);
	return 0;
}

static long thp_ioctl_get_irq_gpio_value(unsigned long arg, u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	u32 __user *out_value = (u32 *)arg;
	u32 value;

	value = gpio_get_value(cd->gpios.irq_gpio);
	if (copy_to_user(out_value, &value, sizeof(u32))) {
		thp_log_err("%s:copy value fail\n", __func__);
		return -EFAULT;
	}
	return 0;
}

static long thp_ioctl_set_spi_speed(unsigned long arg, u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	unsigned long max_speed_hz = arg;
	int rc;

	if ((max_speed_hz == cd->sdev->max_speed_hz) || cd->spi_speed_mode)
		return 0;
	rc = thp_set_spi_max_speed(max_speed_hz, panel_id);
	if (rc)
		thp_log_err("%s: failed to set spi speed\n", __func__);
	return rc;
}

static long thp_ioctl_spi_sync_ssl_bl(unsigned long arg, u32 panel_id)
{
	struct thp_ioctl_spi_sync_data sync_data;
	struct thp_core_data *cd = NULL;
	const void __user *data = (void __user *)arg;
	int rc = 0;
	u8 *tx_buf = NULL;
	u8 *rx_buf = NULL;

	memset(&sync_data, 0, sizeof(sync_data));
	thp_log_debug("%s: called\n", __func__);

	if (!data) {
		thp_log_err("%s: data null\n", __func__);
		return -EINVAL;
	}
	cd = thp_get_core_data(panel_id);
	if (cd == NULL) {
		thp_log_err("%s: thp get core data err\n", __func__);
		return -EINVAL;
	}
	if (cd->suspended)
		return 0;

#if defined(CONFIG_TEE_TUI)
	if (tui_enable)
		return 0;
#endif

	if (copy_from_user(&sync_data, data,
				sizeof(struct thp_ioctl_spi_sync_data))) {
		thp_log_err("Failed to copy_from_user()\n");
		return -EFAULT;
	}

	if (sync_data.size > THP_SYNC_DATA_MAX || sync_data.size == 0) {
		thp_log_err("sync_data.size out of range\n");
		return -EINVAL;
	}

	rx_buf = kzalloc(sync_data.size, GFP_KERNEL);
	tx_buf = kzalloc(sync_data.size, GFP_KERNEL);
	if (!rx_buf || !tx_buf) {
		thp_log_err("%s:buf request memory fail,sync_data.size = %d\n",
			__func__, sync_data.size);
		goto exit;
	}

	if (sync_data.tx) {
		rc = copy_from_user(tx_buf, sync_data.tx, sync_data.size);
		if (rc) {
			thp_log_err("%s:copy in buff fail\n", __func__);
			goto exit;
		}
	}

	if (cd) {
		if (cd->thp_dev->ops->spi_transfer_one_byte_bootloader)
			rc = cd->thp_dev->ops->spi_transfer_one_byte_bootloader(
				cd->thp_dev, tx_buf, rx_buf, sync_data.size);
		else
			rc = -EINVAL;
		if (rc) {
			thp_log_err("%s: transfer error, ret = %d\n",
				__func__, rc);
			goto exit;
		}
	}

	if (sync_data.rx) {
		rc = copy_to_user(sync_data.rx, rx_buf, sync_data.size);
		if (rc) {
			thp_log_err("%s:copy out buff fail\n", __func__);
			goto exit;
		}
	}

exit:

	kfree(rx_buf);
	kfree(tx_buf);

	return rc;
}

static void thp_wakeup_screenon_waitq(struct thp_core_data *cd)
{
	if (!cd) {
		thp_log_err("%s: cd is null\n", __func__);
		return;
	}

	if (atomic_read(&(cd->afe_screen_on_waitq_flag)) != WAITQ_WAKEUP) {
		atomic_set(&(cd->afe_screen_on_waitq_flag), WAITQ_WAKEUP);
		wake_up_interruptible(&(cd->afe_screen_on_waitq));
	} else {
		thp_log_info("afe set status screen on have done\n");
	}
}

#if defined CONFIG_LCD_KIT_DRIVER
static void afe_esd_reset(struct thp_core_data *cd)
{
	if (cd->esd_para.support_esd_power_reset) {
		if (atomic_read(&cd->esd_para.esd_suspend_status_check)) {
			thp_log_info("%s: power reset is already running\n", __func__);
			return;
		} else {
			cd->esd_para.esd_waitq_flag = WAITQ_WAIT;
			thp_log_err("%s: esd reset\n", __func__);
			thp_suspend_thread(cd->panel_id);
			msleep(cd->esd_para.esd_delay_time); /* 1s wait ic reset */
			thp_resume_thread(cd->panel_id);
			cd->esd_para.esd_waitq_flag = WAITQ_WAKEUP;
			wake_up_interruptible(&(cd->esd_para.esd_waitq));
		}
	}
}
#endif

static long thp_ioctl_set_afe_status(unsigned long arg, u32 panel_id)
{
	int rc = 0;
	struct thp_ioctl_set_afe_status set_afe_status;
	struct thp_core_data *cd = NULL;
	struct thp_device *tdev = NULL;
	const void __user *data = (void __user *)arg;

	thp_log_info("%s: called\n", __func__);
	if (!data) {
		thp_log_err("%s: data null\n", __func__);
		return -EINVAL;
	}
	cd = thp_get_core_data(panel_id);
	tdev = cd->thp_dev;
	if (copy_from_user(&set_afe_status, data,
				sizeof(struct thp_ioctl_set_afe_status))) {
		thp_log_err("Failed to copy_from_user()\n");
		return -EFAULT;
	}
	thp_log_info("%s ->%d,%d,%d\n", __func__, set_afe_status.type,
		set_afe_status.status, set_afe_status.parameter);

	switch (set_afe_status.type) {
	case THP_AFE_STATUS_FW_UPDATE:
		if (cd->thp_dev->ops->set_fw_update_mode != NULL)
			rc = cd->thp_dev->ops->set_fw_update_mode(cd->thp_dev,
				set_afe_status);
		else
			rc = -EINVAL;
		if (!rc) {
			if (set_afe_status.parameter == NORMAL_WORK_MODE) {
				is_fw_update = 0;
			} else if (set_afe_status.parameter ==
						FW_UPDATE_MODE) {
				is_fw_update = 1;
			} else {
				thp_log_err("%s->error mode\n",
					__func__);
				rc = -EINVAL;
			}
			thp_log_info("%s call is_fw_updating=%d\n",
				__func__, is_fw_update);
		}
		break;
	case THP_AFE_STATUS_SCREEN_ON:
		if (cd->wait_afe_screen_on_support)
			thp_wakeup_screenon_waitq(cd);
		if (cd->lcd_need_get_afe_status &&
			(set_afe_status.status == 1)) {
			do_timetransfer(&cd->afe_status_record_tv);
			cd->afe_download_status = true;
		}

		if ((cd->send_bt_status_to_fw) &&
			(cd->thp_dev->ops->bt_handler)) {
			if (cd->thp_dev->ops->bt_handler(cd->thp_dev, false))
				thp_log_err("send bt status to fw fail\n");
		}
		break;
#if defined CONFIG_LCD_KIT_DRIVER
	case THP_AFE_STATUS_ESD_RESET:
		afe_esd_reset(cd);
		break;
#endif
	default:
		thp_log_err("%s: illegal type\n", __func__);
		rc = -EINVAL;
		break;
	}
	return rc;
}

static long thp_ioctl_get_work_status(unsigned long arg, u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	u32 __user *work_status = (u32 *)(uintptr_t)arg;
	u32 status;

	if (cd == NULL) {
		thp_log_err("%s: thp cord data null\n", __func__);
		return -EINVAL;
	}
	if (work_status == NULL) {
		thp_log_err("%s: input parameter null\n", __func__);
		return -EINVAL;
	}
	if (cd->suspended == true)
		status = THP_WORK_STATUS_SUSPENDED;
	else
		status = THP_WORK_STATUS_RESUMED;

	if (copy_to_user(work_status, &status, sizeof(u32))) {
		thp_log_err("%s:get wort_status failed\n", __func__);
		return -EFAULT;
	}

	return 0;
}

static long thp_ioctl_cmd_spi_sync(unsigned long arg, u32 panel_id)
{
	return thp_ioctl_spi_sync((void __user *)arg, NEED_LOCK, panel_id);
}

static long thp_ioctl_cmd_clear_frame_buffer(unsigned long arg, u32 panel_id)
{
	return thp_ioctl_clear_frame_buffer(panel_id);
}

static long thp_ioctl_cmd_multiple_spi_xfer_sync(unsigned long arg, u32 panel_id)
{
	return thp_ioctl_multiple_spi_xfer_sync((void __user *)arg, NEED_LOCK, panel_id);
}

static long thp_ioctl_cmd_spi_sync_no_lock(unsigned long arg, u32 panel_id)
{
	return thp_ioctl_spi_sync((void __user *)arg, DONOT_NEED_LOCK, panel_id);
}

static long thp_ioctl_cmd_multiple_spi_xfer_sync_no_lock(unsigned long arg, u32 panel_id)
{
	return thp_ioctl_multiple_spi_xfer_sync((void __user *)arg, DONOT_NEED_LOCK, panel_id);
}

static long thp_ioctl_cmd_spi_sync_in_suspend(unsigned long arg, u32 panel_id)
{
	return thp_ioctl_spi_sync_in_suspend((void __user *)arg, NEED_LOCK, panel_id);
}

struct thp_ioctl_group {
	unsigned int cmd;
	long (*ioctl_func)(unsigned long arg, u32 panel_id);
};

struct thp_ioctl_group thp_ioctl_table[] = {
	{ THP_IOCTL_CMD_GET_FRAME_COMPAT, thp_ioctl_get_frame },
	{ THP_IOCTL_CMD_GET_FRAME, thp_ioctl_get_frame },
	{ THP_IOCTL_CMD_RESET, thp_ioctl_reset },
	{ THP_IOCTL_CMD_SET_TIMEOUT, thp_ioctl_set_timeout},
	{ THP_IOCTL_CMD_SPI_SYNC_COMPAT, thp_ioctl_cmd_spi_sync },
	{ THP_IOCTL_CMD_SPI_SYNC, thp_ioctl_cmd_spi_sync },
	{ THP_IOCTL_CMD_FINISH_NOTIFY, thp_ioctl_finish_notify },
	{ THP_IOCTL_CMD_SET_BLOCK, thp_ioctl_set_block },
	{ THP_IOCTL_CMD_SET_IRQ, thp_ioctl_set_irq },
	{ THP_IOCTL_CMD_GET_FRAME_COUNT, thp_ioctl_get_frame_count },
	{ THP_IOCTL_CMD_CLEAR_FRAME_BUFFER, thp_ioctl_cmd_clear_frame_buffer },
	{ THP_IOCTL_CMD_GET_IRQ_GPIO_VALUE, thp_ioctl_get_irq_gpio_value },
	{ THP_IOCTL_CMD_SET_SPI_SPEED, thp_ioctl_set_spi_speed },
	{ THP_IOCTL_CMD_SPI_SYNC_SSL_BL, thp_ioctl_spi_sync_ssl_bl },
	{ THP_IOCTL_CMD_SET_AFE_STATUS, thp_ioctl_set_afe_status },
	{ THP_IOCTL_CMD_MUILTIPLE_SPI_XFRE_SYNC, thp_ioctl_cmd_multiple_spi_xfer_sync },
	{ THP_IOCTL_CMD_HW_LOCK, thp_ioctl_hw_lock_status },
	{ THP_IOCTL_CMD_SPI_SYNC_NO_LOCK, thp_ioctl_cmd_spi_sync_no_lock },
	{ THP_IOCTL_CMD_MUILTIPLE_SPI_XFRE_SYNC_NO_LOCK, thp_ioctl_cmd_multiple_spi_xfer_sync_no_lock },
	{ THP_IOCTL_CMD_GET_WORK_STATUS, thp_ioctl_get_work_status },
	{ THP_IOCTL_CMD_SPI_SYNC_IN_SUSPEND, thp_ioctl_cmd_spi_sync_in_suspend },
};

static long thp_ioctl(struct file *filp, unsigned int cmd,
	unsigned long arg)
{
	long ret = 0;
	int i;
	struct thp_core_data *cd = container_of((struct miscdevice *)filp->private_data,
		struct thp_core_data, thp_misc_device);

	for (i = 0; i < ARRAY_SIZE(thp_ioctl_table); i++) {
		if (cmd == thp_ioctl_table[i].cmd) {
			ret = thp_ioctl_table[i].ioctl_func(arg, cd->panel_id);
			break;
		}
	}
	if (i == ARRAY_SIZE(thp_ioctl_table)) {
		thp_log_err("%s: cmd unknown, cmd = 0x%x\n", __func__, cmd);
		ret = 0;
	}

	return ret;
}

static long thp_compat_ioctl(struct file *filp, unsigned int cmd,
	unsigned long arg)
{
	long ret;
	struct thp_core_data *cd = container_of((struct miscdevice *)filp->private_data,
		struct thp_core_data, thp_misc_device);

	cd->compat_flag = true;
	ret = thp_ioctl(filp, cmd, arg);
	if (ret)
		thp_log_err("%s: ioctl err %ld\n", __func__, ret);

	return ret;
}

static const struct file_operations g_thp_fops = {
	.owner = THIS_MODULE,
	.open = thp_open,
	.release = thp_release,
	.unlocked_ioctl = thp_ioctl,
	.compat_ioctl = thp_compat_ioctl,
};

#if (defined(CONFIG_HUAWEI_THP_MTK) || defined(CONFIG_HUAWEI_THP_QCOM))
static void send_event_to_fingerprint_ud(struct thp_udfp_data udfp_data)
{
	struct ud_fp_ops *fp_ops = NULL;

	fp_ops = fp_get_ops();
	if (!fp_ops || !fp_ops->fp_irq_notify) {
		thp_log_err("%s: point is NULL!!\n", __func__);
		return;
	}
	if (fp_ops->fp_irq_notify(&udfp_data.tpud_data, sizeof(struct tp_to_udfp_data)))
		thp_log_err("%s: tpud event notify fp failed\n", __func__);
}

static void tp_event_dispatch(struct thp_udfp_data udfp_data, u32 panel_id)
{
	thp_log_info("%s: tp_event = %d, aod_event = %u, keyevent = %u\n",
		__func__, udfp_data.tpud_data.tp_event, udfp_data.aod_event,
		udfp_data.key_event);
	if (udfp_data.tpud_data.tp_event != TP_FP_EVENT_MAX) {
		send_event_to_fingerprint_ud(udfp_data);
		thp_udfp_event_to_aod(udfp_data, panel_id);
	}
	if (udfp_data.aod_event == AOD_VALID_EVENT)
		thp_aod_click_report(udfp_data, panel_id);
	if (udfp_data.key_event == TS_DOUBLE_CLICK)
		thp_inputkey_report(udfp_data.key_event, panel_id);
	if (udfp_data.key_event == TS_STYLUS_WAKEUP_TO_MEMO)
		thp_inputkey_report(udfp_data.key_event, panel_id);
}

static void gesture_event_process(struct thp_core_data *cd)
{
	int rc;
	struct thp_udfp_data udfp_data;

	memset(&udfp_data, 0, sizeof(udfp_data));
	if (!cd->thp_dev->ops->get_event_info) {
		thp_log_err("%s: ops is NULL\n", __func__);
		return;
	}
	rc = cd->thp_dev->ops->get_event_info(cd->thp_dev, &udfp_data);
	if (rc) {
		thp_log_err("%s: get event info fail, ret = %d\n",
			__func__, rc);
		return;
	}
	tp_event_dispatch(udfp_data, cd->panel_id);
}
#endif

static void thp_copy_frame_to_thp_queue(struct thp_core_data *cd)
{
	static int pre_frame_count = -1;

	mutex_lock(&(cd->mutex_frame));
	/* check for max limit */
	if (cd->frame_count >= THP_LIST_MAX_FRAMES) {
		if (cd->frame_count != pre_frame_count)
			thp_log_err("frame_lite buf full start,frame_count:%d\n",
				cd->frame_count);

		if (!thp_queue_dequeue(cd->thp_queue))
			thp_log_err("%s queue remove failed\n", __func__);
		pre_frame_count = cd->frame_count;
		cd->frame_count--;
	} else if (pre_frame_count >= THP_LIST_MAX_FRAMES) {
		thp_log_err(
			"%s:frame buf full exception restored,frame_count:%d\n",
			__func__, cd->frame_count);
		pre_frame_count = cd->frame_count;
	}

	if (!thp_queue_enqueue(cd->thp_queue, cd->frame_read_buf,
		cd->frame_size, cd->panel_id))
		thp_log_err("%s queue insert failed\n", __func__);
	cd->frame_count++;
	mutex_unlock(&(cd->mutex_frame));
}

static void thp_copy_frame(struct thp_core_data *cd)
{
	struct thp_frame *temp = NULL;
	static int pre_frame_count = -1;
	unsigned long len;

	mutex_lock(&(cd->mutex_frame));

	/* check for max limit */
	if (cd->frame_count >= THP_LIST_MAX_FRAMES) {
		if (cd->frame_count != pre_frame_count)
			thp_log_err("frame buf full start,frame_count:%d\n",
				cd->frame_count);

		temp = list_first_entry(&cd->frame_list.list,
				struct thp_frame, list);
		list_del(&temp->list);
		kfree(temp);
		pre_frame_count = cd->frame_count;
		cd->frame_count--;
	} else if (pre_frame_count >= THP_LIST_MAX_FRAMES) {
		thp_log_err(
			"%s:frame buf full exception restored,frame_count:%d\n",
			__func__, cd->frame_count);
		pre_frame_count = cd->frame_count;
	}

	temp = kzalloc(sizeof(struct thp_frame), GFP_KERNEL);
	if (!temp) {
		thp_log_err("%s:memory out\n", __func__);
		mutex_unlock(&(cd->mutex_frame));
		return;
	}

	if (cd->frame_size > sizeof(temp->frame)) {
		thp_log_err("%s: frame size is too large: %u\n",
			__func__, cd->frame_size);
		len = sizeof(temp->frame);
	} else {
		len = cd->frame_size;
	}
	memcpy(temp->frame, cd->frame_read_buf, len);
	do_timetransfer(&(temp->tv));
	list_add_tail(&(temp->list), &(cd->frame_list.list));
	cd->frame_count++;
	mutex_unlock(&(cd->mutex_frame));
}

/*
 * disable the interrupt if the interrupt is enabled,
 * which is only used in irq handler
 */
static void thp_disable_irq_in_irq_process(u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	int ret;

	/*
	 * Use mutex_trylock to avoid the irq process requesting lock failure,
	 * thus solving the problem that other process calls disable_irq
	 * process is blocked.
	 */
	ret = mutex_trylock(&cd->irq_mutex);
	if (ret) {
		if (cd->irq_enabled) {
			disable_irq_nosync(cd->irq);
			cd->irq_enabled = 0;
			thp_log_info("%s:disable irq to protect irq storm\n",
				__func__);
		}
		mutex_unlock(&cd->irq_mutex);
		return;
	}
	thp_log_info("%s:failed to try lock, only need ignore it\n", __func__);
}

static void protect_for_irq_storm(u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	struct timespec64 end_time;
	static struct timespec64 irq_storm_start_time;
	long delta_time;

	/*
	 * We should not try to disable irq when we
	 * need to handle the irq in screen off state,
	 * and add 3ms delay time to avoid irq storm.
	 */
	if (need_work_in_suspend_switch(cd)) {
		msleep(3);
		thp_log_err("%s:ignore the irq\n", __func__);
		return;
	}
	if (cd->invalid_irq_num == 0) {
		do_timetransfer(&irq_storm_start_time);
		return;
	}
	if (cd->invalid_irq_num == MAX_INVALID_IRQ_NUM) {
		memset(&end_time, 0, sizeof(end_time));
		cd->invalid_irq_num = 0;
		do_timetransfer(&end_time);
		/* multiply 1000000 to transfor second to us */
		delta_time = ((end_time.tv_sec - irq_storm_start_time.tv_sec) *
			1000000) + (end_time.tv_nsec -
			irq_storm_start_time.tv_nsec);
		/* divide 1000 to transfor sec to us to ms */
		delta_time /= 1000;
		thp_log_info("%s:delta_time = %ld ms\n", __func__, delta_time);
		if (delta_time <= (MAX_INVALID_IRQ_NUM / THP_IRQ_STORM_FREQ))
			thp_disable_irq_in_irq_process(panel_id);
	}
}

void thp_get_irq_info(u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	struct irq_desc *desc = NULL;
	int irq_value;

	desc = irq_to_desc(cd->irq);
	if (!desc) {
		thp_log_err("%s:desc is null\n", __func__);
		return;
	}
	irq_value = gpio_get_value(cd->gpios.irq_gpio);
	thp_log_info("%s:irq_value = %d, irq_num = %d\n", __func__, irq_value, cd->irq);
	thp_log_info("%s:status_use_accessors = 0x%x, depth = %d, irq_count = %d\n",
		__func__, desc->status_use_accessors, desc->depth, desc->irq_count);
}

static void thp_irq_thread_suspend_process(struct thp_core_data *cd)
{
	int rc;
	unsigned int gesture_wakeup_value = 0;
	if (need_work_in_suspend_switch(cd) &&
		(cd->work_status != RESUME_DONE)) {
		__pm_wakeup_event(cd->thp_wake_lock, jiffies_to_msecs(HZ));
		if (!cd->not_control_irq_in_irq_handler)
			disable_irq_nosync(cd->irq);
		thp_log_info("%s:ts gesture mode irq\n", __func__);
#if (defined(CONFIG_HUAWEI_THP_MTK) || defined(CONFIG_HUAWEI_THP_QCOM))
		if (cd->use_ap_gesture) {
			/* This function includes tpud, AOD click, doubletap */
			gesture_event_process(cd);
			goto exit;
		}
#endif
		if (cd->thp_dev->ops->chip_gesture_report) {
			rc = cd->thp_dev->ops->chip_gesture_report(cd->thp_dev,
				&gesture_wakeup_value);
			if (rc) {
				thp_log_err(
					"%s:gesture report failed this irq,rc = %d\n",
					__func__, rc);
				goto exit;
			}
		} else {
			thp_log_err("%s:gesture not support\n", __func__);
			goto exit;
		}
		thp_inputkey_report(gesture_wakeup_value, cd->panel_id);
		goto exit;
	}

	if (cd->invalid_irq_num < MAX_PRINT_LOG_INVALID_IRQ_NUM)
		thp_log_err("%s: ignore this irq\n", __func__);
	protect_for_irq_storm(cd->panel_id);
	cd->invalid_irq_num++;
	return;
exit:
	if (!cd->not_control_irq_in_irq_handler)
		enable_irq(cd->irq);
	trace_touch(TOUCH_TRACE_IRQ_BOTTOM, TOUCH_TRACE_FUNC_OUT, "thp");
}

static irqreturn_t irq_default_primary_handler(int irq, void *dev_id)
{
	struct timespec64 now;
	struct thp_core_data *cd = dev_id;

	now =  ktime_to_timespec64(ktime_get());
	cd->report_cur_time.tv_sec = now.tv_sec;
	cd->report_cur_time.tv_nsec = now.tv_nsec / 1000;
	return IRQ_WAKE_THREAD;
}

static irqreturn_t thp_irq_thread(int irq, void *dev_id)
{
	struct thp_core_data *cd = dev_id;
	u8 *read_buf = (u8 *)cd->frame_read_buf;
	int rc;

	trace_touch(TOUCH_TRACE_IRQ_BOTTOM, TOUCH_TRACE_FUNC_IN, "thp");
	irq_count_for_debug++;
	if (irq_count_for_debug >= MAX_SUPPORT_DEBUG_IRQ_NUM) {
		irq_count_for_debug = 0;
		g_thp_log_cfg = DISABLE_DEBUG_LOG;
	}
	thp_log_debug("%s:enter panel_id = %d\n", __func__, cd->panel_id);
	if (cd->reset_flag) {
		if (cd->avoid_low_level_irq_storm)
			thp_disable_irq_in_irq_process(cd->panel_id);
		thp_log_err("%s:reset state, ignore this irq\n", __func__);
		return IRQ_HANDLED;
	}

#if defined(CONFIG_TEE_TUI)
	if (tui_enable) {
		thp_log_err("%s:tui_mode, disable irq\n", __func__);
		thp_disable_irq_in_irq_process(cd->panel_id);
		return IRQ_HANDLED;
	}
#endif

	if (cd->suspended && (!cd->need_work_in_suspend)) {
		thp_irq_thread_suspend_process(cd);
		return IRQ_HANDLED;
	}
	if (!cd->not_control_irq_in_irq_handler)
		disable_irq_nosync(cd->irq);

	/* get frame */
	rc = cd->thp_dev->ops->get_frame(cd->thp_dev, read_buf, cd->frame_size);
	if (rc) {
		thp_log_err("%s: failed to read frame %d\n", __func__, rc);
		goto exit;
	}

	trace_touch(TOUCH_TRACE_DATA2ALGO, TOUCH_TRACE_FUNC_IN, "thp");
	if (cd->use_thp_queue)
		thp_copy_frame_to_thp_queue(cd);
	else
		thp_copy_frame(cd);
	thp_wake_up_frame_waitq(cd);
	trace_touch(TOUCH_TRACE_DATA2ALGO, TOUCH_TRACE_FUNC_OUT, "thp");

exit:
	if (!cd->not_control_irq_in_irq_handler)
		enable_irq(cd->irq);
	trace_touch(TOUCH_TRACE_IRQ_BOTTOM, TOUCH_TRACE_FUNC_OUT, "thp");
	thp_log_debug("%s:out\n", __func__);
	return IRQ_HANDLED;
}

void thp_spi_cs_set(u32 control)
{
	int rc = 0;
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
	struct thp_timing_config *timing = NULL;

	if (!cd || !cd->thp_dev) {
		thp_log_err("%s:no driver data", __func__);
		return;
	}

	timing = &cd->thp_dev->timing_config;
#ifndef CONFIG_HUAWEI_THP_MTK
	if (cd->not_support_cs_control)
		return;
	if (control == SSP_CHIP_SELECT) {
		rc = gpio_direction_output(cd->gpios.cs_gpio, control);
		ndelay(timing->spi_sync_cs_low_delay_ns);
		if (timing->spi_sync_cs_low_delay_us && (!is_fw_update))
			udelay(timing->spi_sync_cs_low_delay_us);
	} else {
		rc = gpio_direction_output(cd->gpios.cs_gpio, control);
		ndelay(timing->spi_sync_cs_hi_delay_ns);
	}
#else
	if (cd->support_pinctrl == 0) {
		thp_log_info("%s: not support pinctrl\n", __func__);
		return;
	}

	if (control == SSP_CHIP_SELECT) {
		pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.cs_low);
		ndelay(cd->thp_dev->timing_config.spi_sync_cs_low_delay_ns);
	} else {
		pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.cs_high);
		ndelay(cd->thp_dev->timing_config.spi_sync_cs_hi_delay_ns);
	}
#endif

	if (rc < 0)
		thp_log_err("%s:fail to set gpio cs", __func__);
}
EXPORT_SYMBOL(thp_spi_cs_set);

void thp_spi_cs_set_sub_panel(u32 control)
{
	int rc = 0;
	struct thp_core_data *cd = thp_get_core_data(SUB_TOUCH_PANEL);
	struct thp_timing_config *timing = NULL;

	if (!cd || !cd->thp_dev) {
		thp_log_err("%s:no driver data", __func__);
		return;
	}

	timing = &cd->thp_dev->timing_config;
#ifndef CONFIG_HUAWEI_THP_MTK
	if (cd->not_support_cs_control)
		return;
	if (control == SSP_CHIP_SELECT) {
		rc = gpio_direction_output(cd->gpios.cs_gpio, control);
		ndelay(timing->spi_sync_cs_low_delay_ns);
		if (timing->spi_sync_cs_low_delay_us && (!is_fw_update))
			udelay(timing->spi_sync_cs_low_delay_us);
	} else {
		rc = gpio_direction_output(cd->gpios.cs_gpio, control);
		ndelay(timing->spi_sync_cs_hi_delay_ns);
	}
#else
	if (cd->support_pinctrl == 0) {
		thp_log_info("%s: not support pinctrl\n", __func__);
		return;
	}

	if (control == SSP_CHIP_SELECT) {
		pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.cs_low);
		ndelay(cd->thp_dev->timing_config.spi_sync_cs_low_delay_ns);
	} else {
		pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.cs_high);
		ndelay(cd->thp_dev->timing_config.spi_sync_cs_hi_delay_ns);
	}
#endif

	if (rc < 0)
		thp_log_err("%s:fail to set gpio cs", __func__);
}
#ifdef CONFIG_BCI_BATTERY
static int thp_charger_detect_notifier_callback(struct notifier_block *self,
					unsigned long event, void *data)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
	int charger_stat_before = thp_get_status(THP_STATUS_CHARGER, cd->panel_id);
	int charger_state_new = charger_stat_before;

	thp_log_debug("%s called, charger type: %lu\n", __func__, event);

	switch (event) {
	case VCHRG_START_USB_CHARGING_EVENT:
	case VCHRG_START_AC_CHARGING_EVENT:
	case VCHRG_START_CHARGING_EVENT:
		charger_state_new = 1;
		break;
	case VCHRG_STOP_CHARGING_EVENT:
		charger_state_new = 0;
		break;
	default:
		break;
	}

	if (charger_stat_before != charger_state_new) {
		thp_log_info("%s, set new status: %d\n",
			__func__, charger_state_new);
		thp_set_status(THP_STATUS_CHARGER, charger_state_new, cd->panel_id);
		if (cd->dual_thp)
			thp_set_status(THP_STATUS_CHARGER, charger_state_new, SUB_TOUCH_PANEL);
	}

	return 0;
}

static void thp_bci_charger_notifier_register(struct thp_core_data *cd)
{
	int rc;
	static int flag = 0;

	if (flag) {
		thp_log_info("%s bci_register_notifier has be done\n", __func__);
		return;
	}
	flag++;

	cd->charger_detect_notify.notifier_call =
			thp_charger_detect_notifier_callback;
	rc = bci_register_notifier(&cd->charger_detect_notify, 1);
	if (rc < 0) {
		thp_log_err("%s:charger notifier register failed\n", __func__);
		cd->charger_detect_notify.notifier_call = NULL;
		flag--;
	} else {
		thp_log_info("%s:charger notifier register succ\n", __func__);
	}
}
#endif

#define THP_PROJECTID_LEN 9
#define THP_PROJECTID_PRODUCT_NAME_LEN 4
#define THP_PROJECTID_IC_NAME_LEN 2
#define THP_PROJECTID_VENDOR_NAME_LEN 3

struct thp_vendor {
	char *vendor_id;
	char *vendor_name;
};

struct thp_ic_name {
	char *ic_id;
	char *ic_name;
};

static struct thp_vendor thp_vendor_table[] = {
	{ "000", "ofilm" },
	{ "030", "mutto" },
	{ "080", "jdi" },
	{ "090", "samsung" },
	{ "100", "lg" },
	{ "101", "lg" },
	{ "102", "lg" },
	{ "110", "tianma" },
	{ "111", "tianma" },
	{ "112", "tianma" },
	{ "113", "tianma" },
	{ "120", "cmi" },
	{ "130", "boe" },
	{ "140", "ctc" },
	{ "160", "sharp" },
	{ "170", "auo" },
	{ "250", "txd" },
	{ "270", "tcl" },
	{ "320", "dpt" },
	{ "290", "vxn" },
};

static struct thp_ic_name thp_ic_table[] = {
	{ "32", "rohm" },
	{ "47", "rohm" },
	{ "49", "novatech" },
	{ "59", "himax" },
	{ "60", "himax" },
	{ "61", "himax" },
	{ "62", "synaptics" },
	{ "65", "novatech" },
	{ "66", "himax" },
	{ "68", "focaltech" },
	{ "69", "synaptics" },
	{ "71", "novatech" },
	{ "77", "novatech" },
	{ "78", "goodix" },
	{ "79", "ilitek" },
	{ "86", "synaptics" },
	{ "88", "novatech" },
	{ "91", "synaptics" },
	{ "96", "synaptics" },
	{ "9A", "focaltech" },
	{ "9B", "synaptics" },
	{ "9C", "novatech" },
	{ "9D", "focaltech" },
	{ "9I", "novatech" },
	{ "9J", "novatech" },
	{ "9L", "himax" },
	{ "9M", "novatech" },
	{ "9N", "focaltech" },
	{ "9S", "goodix" },
	{ "9Y", "novatech" },
	{ "A5", "synaptics" },
	{ "A7", "novatech" },
	{ "A8", "synaptics" },
	{ "A9", "focaltech" },
	{ "A6", "ilitek" },
	{ "AL", "goodix" },
	{ "AM", "ilitek" },
	{ "AQ", "ilitek" },
	{ "B1", "novatech" },
	{ "B6", "chipone" },
	{ "B0", "raydium" },
	{ "B2", "focaltech" },
};

static int thp_projectid_to_vender_name(const char *project_id,
	char **vendor_name, int project_id_len)
{
	char temp_buf[THP_PROJECTID_LEN + 1] = {'0'};
	int i;

	if (strlen(project_id) > project_id_len)
		thp_log_err("%s:project_id has a wrong length\n", __func__);
	strncpy(temp_buf, project_id + THP_PROJECTID_PRODUCT_NAME_LEN +
		THP_PROJECTID_IC_NAME_LEN, THP_PROJECTID_VENDOR_NAME_LEN);

	for (i = 0; i < ARRAY_SIZE(thp_vendor_table); i++) {
		if (!strncmp(thp_vendor_table[i].vendor_id, temp_buf,
			strlen(thp_vendor_table[i].vendor_id))) {
			*vendor_name = thp_vendor_table[i].vendor_name;
			return 0;
		}
	}

	return -ENODATA;
}

static int thp_projectid_to_ic_name(const char *project_id,
	char **ic, int project_id_len)
{
	char temp_buf[THP_PROJECTID_LEN + 1] = { '0' };
	int i;

	if (strlen(project_id) > project_id_len)
		thp_log_err("%s:project_id has a wrong length\n", __func__);
	strncpy(temp_buf, project_id + THP_PROJECTID_PRODUCT_NAME_LEN,
			THP_PROJECTID_IC_NAME_LEN);

	for (i = 0; i < ARRAY_SIZE(thp_ic_table); i++) {
		if (!strncmp(thp_ic_table[i].ic_id, temp_buf,
			strlen(thp_ic_table[i].ic_id))) {
			*ic = thp_ic_table[i].ic_name;
			return 0;
		}
	}

	return -ENODATA;
}

#ifndef CONFIG_MODULE_KO_TP
#if defined(CONFIG_LCD_KIT_DRIVER)
__attribute__((weak)) struct lcd_kit_ops *lcd_kit_get_ops(void)
{
	return 0;
}

__attribute__((weak)) int ts_kit_ops_register(struct ts_kit_ops *ops)
{
	return 0;
}

__attribute__((weak)) int ts_kit_ops_unregister(struct ts_kit_ops *ops)
{
	return 0;
}
#endif
#endif

static int parse_project_id_from_cmdline(struct thp_core_data *cd)
{
	char *pstr = NULL;
	char *dstr = NULL;
	uint32_t project_id_len;

	pstr = strstr(saved_command_line, "projectid=");
	if (!pstr) {
		thp_log_err("No fastboot projectid info\n");
		return -EINVAL;
	}
	pstr += strlen("projectid=");
	dstr = strstr(pstr, " ");
	if (!dstr) {
		thp_log_err("No find the projectid end\n");
		return -EINVAL;
	}

	project_id_len = (unsigned long)(dstr - pstr);
	thp_log_info("project_id_len is %d\n", project_id_len);
	thp_log_info("project_id: %s\n", pstr);
	if (project_id_len > THP_PROJECT_ID_LEN) {
		thp_log_info("project_id_len is %d\n", project_id_len);
		project_id_len = THP_PROJECT_ID_LEN;
	}

	strncpy(cd->project_id, pstr, project_id_len);
	thp_log_info("%s:get project id:%s\n", __func__, cd->project_id);

	return 0;
}

static int thp_init_chip_info(struct thp_core_data *cd)
{
	int rc = 0;

#ifndef CONFIG_MODULE_KO_TP
#if defined(CONFIG_LCD_KIT_DRIVER)
	struct lcd_kit_ops *tp_ops = lcd_kit_get_ops();
#endif
#endif

	if (cd->is_udp) {
#if (!defined CONFIG_HUAWEI_THP_QCOM) && (!defined CONFIG_HUAWEI_THP_MTK) || (defined CONFIG_HUAWEI_THP_UDP)
		rc = hostprocessing_get_project_id_for_udp(cd->project_id,
			THP_PROJECT_ID_LEN + 1);
#endif
	} else {
#ifndef CONFIG_HUAWEI_THP_UDP
#ifndef CONFIG_LCD_KIT_DRIVER
		rc = hostprocessing_get_project_id(cd->project_id,
			THP_PROJECT_ID_LEN + 1);
#else
#ifndef CONFIG_MODULE_KO_TP
		if (tp_ops && tp_ops->get_project_id) {
			rc = tp_ops->get_project_id(cd->project_id);
		} else {
			rc = -EINVAL;
			thp_log_err("%s:get lcd_kit_get_ops fail\n", __func__);
		}
#else
		if (cd->support_projectid_from_cmdline)
			parse_project_id_from_cmdline(cd);
#endif
#endif
#endif
	}
	if (rc)
		thp_log_err("%s:get project id form LCD fail\n", __func__);
	else
		thp_log_info("%s:project id :%s\n", __func__, cd->project_id);

	cd->project_id[THP_PROJECT_ID_LEN] = '\0';

	rc = thp_projectid_to_vender_name(cd->project_id,
			(char **)&cd->vendor_name, THP_PROJECT_ID_LEN + 1);
	if (rc)
		thp_log_info("%s:vendor name parse fail\n", __func__);

	rc = thp_projectid_to_ic_name(cd->project_id,
			(char **)&cd->ic_name, THP_PROJECT_ID_LEN + 1);
	if (rc)
		thp_log_info("%s:ic name parse fail\n", __func__);
	return rc;
}

#ifdef CONFIG_HUAWEI_THP_MTK
static int thp_mtk_pinctrl_get_init(struct thp_device *tdev)
{
	int ret;
	struct thp_core_data *cd = tdev->thp_core;

	cd->mtk_pinctrl.reset_high =
		pinctrl_lookup_state(cd->pctrl, PINCTRL_STATE_RESET_HIGH);
	if (IS_ERR_OR_NULL(cd->mtk_pinctrl.reset_high)) {
		thp_log_err("Can not lookup %s pinstate\n",
			PINCTRL_STATE_RESET_HIGH);
		ret = -EINVAL;
		return ret;
	}

	cd->mtk_pinctrl.reset_low =
		pinctrl_lookup_state(cd->pctrl, PINCTRL_STATE_RESET_LOW);
	if (IS_ERR_OR_NULL(cd->mtk_pinctrl.reset_low)) {
		thp_log_err("Can not lookup %s pinstate\n",
			PINCTRL_STATE_RESET_LOW);
		ret = -EINVAL;
		return ret;
	}

	cd->mtk_pinctrl.cs_high =
		pinctrl_lookup_state(cd->pctrl, PINCTRL_STATE_CS_HIGH);
	if (IS_ERR_OR_NULL(cd->mtk_pinctrl.cs_high)) {
		thp_log_err("Can not lookup %s pinstate\n",
			PINCTRL_STATE_CS_HIGH);
		ret = -EINVAL;
		return ret;
	}

	cd->mtk_pinctrl.cs_low =
		pinctrl_lookup_state(cd->pctrl, PINCTRL_STATE_CS_LOW);
	if (IS_ERR_OR_NULL(cd->mtk_pinctrl.cs_low)) {
		thp_log_err("Can not lookup %s pinstate\n",
			PINCTRL_STATE_CS_LOW);
		ret = -EINVAL;
		return ret;
	}

	cd->mtk_pinctrl.as_int =
		pinctrl_lookup_state(cd->pctrl, PINCTRL_STATE_AS_INT);
	if (IS_ERR_OR_NULL(cd->mtk_pinctrl.as_int)) {
		thp_log_err("Can not lookup %s pinstate\n",
			PINCTRL_STATE_AS_INT);
		ret = -EINVAL;
		return ret;
	}

	ret = pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.as_int);
	if (ret < 0)
		thp_log_err("set gpio as int failed\n");

	if (cd->change_spi_driving_force) {
		cd->mtk_pinctrl.spi_status = pinctrl_lookup_state(cd->pctrl,
			PINCTRL_STATE_SPI_STATUS);
		if (IS_ERR_OR_NULL(cd->mtk_pinctrl.spi_status)) {
			thp_log_err("Can not lookup %s pinstate\n",
				PINCTRL_STATE_SPI_STATUS);
			ret = -EINVAL;
			return ret;
		}
		ret = pinctrl_select_state(cd->pctrl,
			cd->mtk_pinctrl.spi_status);
		if (ret < 0)
			thp_log_err("change spi driving force failed\n");
	}
	return ret;
}
#endif

static int thp_setup_irq(struct thp_core_data *cd)
{
	int rc;
	int irq;
	unsigned long irq_flag_type;
	u32 current_trigger_mode;

	if (!cd) {
		thp_log_err("%s: thp_core_data is null\n", __func__);
		return -EINVAL;
	}

#ifndef CONFIG_HUAWEI_THP_MTK
	irq = gpio_to_irq(cd->gpios.irq_gpio);
#else
	irq = irq_of_parse_and_map(cd->thp_node, 0);
#endif

	/*
	 * IRQF_TRIGGER_RISING 0x00000001
	 * IRQF_TRIGGER_FALLING 0x00000002
	 * IRQF_TRIGGER_HIGH 0x00000004
	 * IRQF_TRIGGER_LOW 0x00000008
	 * IRQF_NO_SUSPEND 0x00004000
	 */
	current_trigger_mode = cd->irq_flag;
	thp_log_info("%s:panel id %d current_trigger_mode->0x%x\n",
		__func__, cd->panel_id, current_trigger_mode);

	irq_flag_type = IRQF_ONESHOT | current_trigger_mode;
#ifndef CONFIG_HUAWEI_THP_QCOM
	if (cd->support_gesture_mode || cd->use_ap_gesture)
		irq_flag_type = IRQF_ONESHOT | IRQF_NO_SUSPEND |
			current_trigger_mode;
#endif
	if (cd->tp_time_sync_support)
		rc = request_threaded_irq(irq, irq_default_primary_handler,
			thp_irq_thread, irq_flag_type,
			"thp", cd);
	else
		rc = request_threaded_irq(irq, NULL,
			thp_irq_thread, irq_flag_type,
			"thp", cd);
	if (rc) {
		thp_log_err("%s: request irq fail\n", __func__);
		return rc;
	}
	mutex_lock(&cd->irq_mutex);
	disable_irq(irq);
	cd->irq_enabled = false;
	mutex_unlock(&cd->irq_mutex);
	thp_log_info("%s: disable irq\n", __func__);
	cd->irq = irq;

	return 0;
}

static int thp_setup_gpio(struct thp_core_data *cd)
{
	int rc;

	thp_log_info("%s: called\n", __func__);

	rc = gpio_request(cd->gpios.rst_gpio, "thp_reset");
	if (rc) {
		thp_log_err("%s:gpio_request %d failed\n", __func__,
				cd->gpios.rst_gpio);
		return rc;
	}

	rc = gpio_request(cd->gpios.irq_gpio, "thp_int");
	if (rc) {
		thp_log_err("%s: irq gpio %d request failed\n", __func__,
				cd->gpios.irq_gpio);
		gpio_free(cd->gpios.rst_gpio);
		return rc;
	}
	rc = gpio_direction_input(cd->gpios.irq_gpio);
	if (rc)
		thp_log_info("%s:gpio_direction_input failed\n", __func__);

	if (!cd->not_support_cs_control) {
		rc = gpio_request(cd->gpios.cs_gpio, "thp_cs");
		if (rc) {
			thp_log_err("%s:gpio_request %d failed\n", __func__,
					cd->gpios.cs_gpio);
			gpio_free(cd->gpios.rst_gpio);
			gpio_free(cd->gpios.irq_gpio);
			return rc;
		}
		gpio_direction_output(cd->gpios.cs_gpio, GPIO_HIGH);
		thp_log_info("%s:set cs gpio %d deault hi\n", __func__,
			cd->gpios.cs_gpio);
	}
	return 0;
}

static void thp_free_gpio(struct thp_core_data *ts)
{
	gpio_free(ts->gpios.irq_gpio);
	gpio_free(ts->gpios.rst_gpio);
	/* no need free cs_gpio */
	if (!ts->not_support_cs_control)
		gpio_free(ts->gpios.cs_gpio);
}

static int thp_setup_spi(struct thp_core_data *cd)
{
	int rc;

#ifdef CONFIG_HUAWEI_THP_MTK
	cd->mtk_spi_config.cs_setuptime =
		cd->thp_dev->timing_config.spi_sync_cs_low_delay_ns;
#endif
	rc = spi_setup(cd->sdev);
	if (rc) {
		thp_log_err("%s: spi setup fail\n", __func__);
		return rc;
	}

	return 0;
}
int thp_set_spi_com_mode(struct thp_core_data *cd, u8 spi_com_mode)
{
	int rc;

	if (!cd) {
		thp_log_err("%s: tdev null\n", __func__);
		return -EINVAL;
	}

	if (spi_com_mode != SPI_DMA_MODE && spi_com_mode != SPI_POLLING_MODE) {
		thp_log_err("%s ->error mode\n", __func__);
		return -EINVAL;
	}
	cd->spi_config.pl022_spi_config.com_mode = spi_com_mode;
	cd->sdev->controller_data = &cd->spi_config.pl022_spi_config;
	rc = thp_setup_spi(cd);
	thp_log_info("%s rc->%d\n", __func__, rc);
	return rc;
}
#if defined(CONFIG_TEE_TUI)
static u32 thp_get_multi_pm_status(void)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);

	thp_log_info("%s current_pm_status:%u\n", __func__,
		cd->current_pm_status);
	return cd->current_pm_status;
}

__attribute__((weak)) int i2c_init_secos(struct i2c_adapter *adap)
{
	return 0;
}

__attribute__((weak)) int i2c_exit_secos(struct i2c_adapter *adap)
{
	return 0;
}

int spi_exit_secos(unsigned int spi_bus_id)
{
#ifdef CONFIG_HUAWEI_THP_MTK
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
	int rc;

	rc = thp_setup_spi(cd);
	thp_log_info("%s rc = %d\n", __func__, rc);
#endif
	return 0;
}

int spi_init_secos(unsigned int spi_bus_id)
{
	thp_log_info("%s enter, %u\n", __func__, spi_bus_id);
	return 0;
}

void thp_tui_secos_init(u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	int t;
#ifdef CONFIG_HUAWEI_SHB_THP
	int ret;
#endif

	if (!cd) {
		thp_log_err("%s: core not inited\n", __func__);
		return;
	}

	/* NOTICE: should not change this path unless ack daemon */
	thp_set_status(THP_STATUS_TUI, 1, cd->panel_id);
	cd->thp_ta_waitq_flag = WAITQ_WAIT;

	thp_log_info("%s: busid=%d. disable irq=%d\n",
		__func__, cd->spi_config.bus_id, cd->irq);
	t = wait_event_interruptible_timeout(cd->thp_ta_waitq,
		(cd->thp_ta_waitq_flag == WAITQ_WAKEUP), HZ);
	thp_log_info("%s: wake up finish\n", __func__);

#ifdef CONFIG_HUAWEI_SHB_THP
	if (cd->support_shb_thp) {
		if (cd->thp_dev && cd->thp_dev->ops &&
			cd->thp_dev->ops->switch_int_sh) {
			ret = cd->thp_dev->ops->switch_int_sh(cd->thp_dev);
			if (ret)
				thp_log_err("%s: switch to sh fail", __func__);
		}
		thp_set_irq_status(cd, THP_IRQ_DISABLE);
		ret = send_thp_driver_status_cmd(TP_SWITCH_ON, 0,
			ST_CMD_TYPE_SET_TUI_STATUS);
		if (ret)
			thp_log_err("%s: send thp tui on fail", __func__);
	} else {
#endif
		thp_set_irq_status(cd, THP_IRQ_DISABLE);
		if (cd->no_need_secos_bus_init == 0)
			spi_init_secos(cd->spi_config.bus_id);
#ifdef CONFIG_HUAWEI_SHB_THP
	}
#endif
	tui_enable = 1;
	thp_log_info("%s set tui_enable:%d\n", __func__, tui_enable);
	if (cd->multi_panel_index != SINGLE_TOUCH_PANEL)
		tp_tui_data[cd->panel_id].panel_in_tui = true;
}

void thp_tui_secos_exit(u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);
#ifdef CONFIG_HUAWEI_SHB_THP
	int ret;
#endif

	if (cd->send_tui_exit_in_suspend && (!tui_enable)) {
		thp_log_err("%s TUI has exit\n", __func__);
		return;
	}
	tui_enable = 0;
	thp_log_info("%s: busid=%u tui_enable=%u\n", __func__,
		cd->spi_config.bus_id, tui_enable);
  	if (cd->multi_panel_index != SINGLE_TOUCH_PANEL)
		tp_tui_data[cd->panel_id].panel_in_tui = false;
#ifdef CONFIG_HUAWEI_SHB_THP
	if (cd->support_shb_thp) {
		if (cd->thp_dev && cd->thp_dev->ops &&
			cd->thp_dev->ops->switch_int_ap) {
			ret = cd->thp_dev->ops->switch_int_ap(cd->thp_dev);
			if (ret)
				thp_log_err("%s: switch to ap fail", __func__);
			}
		cd->shb_waitq_flag = WAITQ_WAIT;
		ret = send_thp_driver_status_cmd(TP_SWITCH_OFF, 0,
			ST_CMD_TYPE_SET_TUI_STATUS);
		if (ret)
			thp_log_err("%s: send thp tui off fail", __func__);
	} else {
#endif
		if (cd->no_need_secos_bus_init == 0)
			spi_exit_secos(cd->spi_config.bus_id);
#ifdef CONFIG_HUAWEI_SHB_THP
	}
#endif
	mutex_lock(&cd->suspend_flag_mutex);
	if (cd->suspended && (!cd->need_work_in_suspend)) {
		thp_log_info("%s: drv suspended\n", __func__);
		mutex_unlock(&cd->suspend_flag_mutex);
		return;
	}
	thp_set_irq_status(cd, THP_IRQ_ENABLE);
	mutex_unlock(&cd->suspend_flag_mutex);
	thp_wait_shb_process_done(cd);
	thp_set_status(THP_STATUS_TUI, 0, cd->panel_id);
	mdelay(cd->thp_dev->timing_config.tui_exit_wait_time_ms);
	thp_log_info("%s end\n", __func__);
}

static int thp_tui_switch(void *data, int secure)
{
	thp_log_info("%s:tui secure is %d\n", __func__, secure);
	mutex_lock(&tui_mutex);
	if (secure)
		thp_tui_secos_init(MAIN_TOUCH_PANEL);
	else
		thp_tui_secos_exit(MAIN_TOUCH_PANEL);
	mutex_unlock(&tui_mutex);
	return 0;
}

static int thp_tui_switch_sub(void *data, int secure)
{
	thp_log_info("%s:tui secure is %d\n", __func__, secure);

	mutex_lock(&tui_mutex);
	if (secure)
		thp_tui_secos_init(SUB_TOUCH_PANEL);
	else
		thp_tui_secos_exit(SUB_TOUCH_PANEL);
	mutex_unlock(&tui_mutex);
	return 0;
}

static int multi_tp_tui_switch(void *data, int secure)
{
	int ret = 0;
	int panel_id;

	if (secure) {
		/* according to panel status to enter tui */
		switch (thp_get_multi_pm_status()) {
		case M_ON_S_OFF:
			panel_id = MAIN_TOUCH_PANEL;
			break;
		case M_OFF_S_ON:
			panel_id = SUB_TOUCH_PANEL;
			break;
		default:
			goto err;
		}
	} else {
		/* exit the panel which in tui mode */
		if (tp_tui_data[SUB_TOUCH_PANEL].panel_in_tui)
			panel_id = SUB_TOUCH_PANEL;
		else if (tp_tui_data[MAIN_TOUCH_PANEL].panel_in_tui)
			panel_id = MAIN_TOUCH_PANEL;
		else
			goto err;
	}

	memcpy(&thp_tui_info, &tp_tui_data[panel_id], sizeof(thp_tui_info));
	ret = tp_tui_data[panel_id].tui_drv_switch(data, secure);
	thp_log_info("%s:device_name: %s  ret:%d\n", __func__,
		thp_tui_info.project_id, ret);
	return ret;
err:
	thp_log_err("%s invalid mode\n", __func__);
	return ret;
}

#define IC_THP_SHB_TUI_COMMON_ID "shb_tui"
#define IC_THP_SHB_TUI_COMMON_ID_LEN 8

static void thp_tui_init(struct thp_core_data *cd)
{
	int rc;

	if (!cd) {
		thp_log_err("%s: core not inited\n", __func__);
		return;
	}
	cd->current_pm_status = M_ON_S_ON;
	tui_enable = 0;
	if (cd->support_shb_thp && cd->support_shb_thp_tui_common_id) {
#ifdef CONFIG_HUAWEI_SHB_THP
		rc = snprintf(thp_tui_info.project_id,
			IC_THP_SHB_TUI_COMMON_ID_LEN, IC_THP_SHB_TUI_COMMON_ID);
		if (rc < 0)
			thp_log_err("snprintf failed, rc: %d\n", rc);
#endif
		thp_log_info("%s thp_tui_info.project_id:%s\n",
			__func__, thp_tui_info.project_id);
	} else {
		strncpy(thp_tui_info.project_id, cd->project_id,
			THP_PROJECT_ID_LEN);
		thp_tui_info.project_id[THP_PROJECT_ID_LEN] = '\0';
		thp_log_info("%s thp_tui_info.project_id:%s\n",
			__func__, thp_tui_info.project_id);
	}
	thp_tui_info.frame_data_addr = cd->frame_data_addr;
	thp_log_info("%s thp_tui_info.get_frame_addr %d\n",
		__func__, thp_tui_info.frame_data_addr);
	memcpy(&tp_tui_data[MAIN_TOUCH_PANEL].tp_tui_normalized_data.thp_tui_data,
		&thp_tui_info, sizeof(thp_tui_info));
	if (cd->multi_panel_index == SINGLE_TOUCH_PANEL) {
		rc = register_tui_driver(thp_tui_switch, "tp", &thp_tui_info);
		if (rc != 0) {
			thp_log_err("%s reg thp_tui_switch fail: %d\n",
				__func__, rc);
			return;
		}
	} else {
		if (cd->multi_panel_index == MAIN_TOUCH_PANEL) {
			thp_log_err("%s register_tui_driver thp_tui_switch\n",
				__func__);
			tp_tui_data[MAIN_TOUCH_PANEL].tui_drv_switch =
				thp_tui_switch;
			thp_log_info("%s thp_tui_info.project_id:%s\n",
				__func__, thp_tui_info.project_id);
			rc = register_tui_driver(multi_tp_tui_switch,
				"tp", &thp_tui_info);
			if (rc != 0) {
				thp_log_err("%s reg thp_tui_switch fail: %d\n",
					__func__, rc);
				return;
			}
		}
	}
}

static void thp_tui_init_for_dual_thp(struct thp_core_data *cd)
{
	int rc;
	static int init_flag;

	cd->current_pm_status = M_ON_S_ON;
	tui_enable = 0;
	if (cd->support_shb_thp && cd->support_shb_thp_tui_common_id) {
#ifdef CONFIG_HUAWEI_SHB_THP
		rc = snprintf(thp_tui_info.project_id,
			IC_THP_SHB_TUI_COMMON_ID_LEN, IC_THP_SHB_TUI_COMMON_ID);
		if (rc < 0)
			thp_log_err("snprintf failed, rc: %d\n", rc);
#endif
		thp_log_info("%s thp_tui_info.project_id:%s\n",
			__func__, thp_tui_info.project_id);
	}

	memcpy(&tp_tui_data[cd->panel_id].tp_tui_normalized_data.thp_tui_data,
		&thp_tui_info, sizeof(thp_tui_info));
	tp_tui_data[cd->panel_id].tp_tui_normalized_data.thp_tui_data.frame_data_addr = cd->frame_data_addr;
	thp_log_info("%s thp_tui_info.get_frame_addr %d\n", __func__,
		tp_tui_data[cd->panel_id].tp_tui_normalized_data.thp_tui_data.frame_data_addr);

	if (cd->panel_id == MAIN_TOUCH_PANEL)
		tp_tui_data[cd->panel_id].tui_drv_switch = thp_tui_switch;
	else if (cd->panel_id == SUB_TOUCH_PANEL)
		tp_tui_data[cd->panel_id].tui_drv_switch = thp_tui_switch_sub;

	if (init_flag == 0) {
		thp_log_info("%s register thp_tui_info.project_id:%s\n", __func__,
			tp_tui_data[cd->panel_id].tp_tui_normalized_data.thp_tui_data.project_id);
		rc = register_tui_driver(multi_tp_tui_switch, "tp",
			&tp_tui_data[cd->panel_id].tp_tui_normalized_data.thp_tui_data);
		if (rc != 0) {
			thp_log_err("%s reg thp_tui_switch fail: %d\n", __func__, rc);
			return;
		}
		init_flag++;
	}
}
#endif

#ifdef CONFIG_HUAWEI_THP_QCOM
static void thp_qcom_pinctrl_get_init(struct thp_device *tdev)
{
	struct thp_core_data *cd = tdev->thp_core;

	cd->qcom_pinctrl.cs_high =
		pinctrl_lookup_state(cd->pctrl, PINCTRL_STATE_CS_HIGH);
	if (IS_ERR_OR_NULL(cd->qcom_pinctrl.cs_high))
		thp_log_err("Can not lookup %s pinstate\n", PINCTRL_STATE_CS_HIGH);

	cd->qcom_pinctrl.cs_low =
		pinctrl_lookup_state(cd->pctrl, PINCTRL_STATE_CS_LOW);
	if (IS_ERR_OR_NULL(cd->qcom_pinctrl.cs_low))
		thp_log_err("Can not lookup %s pinstate\n", PINCTRL_STATE_CS_LOW);
}
#endif

static int thp_pinctrl_get_init(struct thp_device *tdev)
{
	int ret = 0;

	tdev->thp_core->pctrl = devm_pinctrl_get(&tdev->sdev->dev);
	if (IS_ERR(tdev->thp_core->pctrl)) {
		thp_log_err("failed to devm pinctrl get\n");
		ret = -EINVAL;
		return ret;
	}

	tdev->thp_core->pins_default =
		pinctrl_lookup_state(tdev->thp_core->pctrl, "default");
	if (IS_ERR(tdev->thp_core->pins_default)) {
		thp_log_err("failed to pinctrl lookup state default\n");
		ret = -EINVAL;
		goto err_pinctrl_put;
	}

	tdev->thp_core->pins_idle =
			pinctrl_lookup_state(tdev->thp_core->pctrl, "idle");
	if (IS_ERR(tdev->thp_core->pins_idle)) {
		thp_log_err("failed to pinctrl lookup state idle\n");
		ret = -EINVAL;
		goto err_pinctrl_put;
	}

#ifdef CONFIG_HUAWEI_THP_QCOM
	thp_qcom_pinctrl_get_init(tdev);
#endif

#ifdef CONFIG_HUAWEI_THP_MTK
	ret = thp_mtk_pinctrl_get_init(tdev);
	if (ret < 0) {
		thp_log_err("%s: mtk pinctrl init failed\n", __func__);
		goto err_pinctrl_put;
	}
#endif

	return 0;

err_pinctrl_put:
	devm_pinctrl_put(tdev->thp_core->pctrl);
	return ret;
}

int thp_pinctrl_select_normal(struct thp_device *tdev)
{
	int retval;

	retval =
		pinctrl_select_state(tdev->thp_core->pctrl,
		tdev->thp_core->pins_default);
	if (retval < 0)
		thp_log_err("set iomux normal error, %d\n", retval);
	return retval;
}

int thp_pinctrl_select_lowpower(struct thp_device *tdev)
{
	int retval;

	retval = pinctrl_select_state(tdev->thp_core->pctrl,
		tdev->thp_core->pins_idle);
	if (retval < 0)
		thp_log_err("set iomux lowpower error, %d\n", retval);
	return retval;
}

const char *thp_get_vendor_name(u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	if (cd && cd->support_multi_panel_attach)
		return (cd->thp_dev && cd->thp_dev->multi_vendor_name) ?
			cd->thp_dev->multi_vendor_name : "null";

	return (cd && cd->thp_dev) ? cd->thp_dev->ic_name : 0;
}
EXPORT_SYMBOL(thp_get_vendor_name);

static int thp_project_in_tp(struct thp_core_data *cd)
{
	int ret = 0;
	unsigned int value = 0;
	int flag;

	flag = (cd->project_in_tp || cd->pid_need_modify_for_tsa) &&
		cd->thp_dev->ops->get_project_id;
	if (flag)
		ret = cd->thp_dev->ops->get_project_id(
						cd->thp_dev,
						cd->project_id,
						THP_PROJECT_ID_LEN);

	if (ret) {
		strncpy(
			cd->project_id,
			cd->project_id_dummy,
			THP_PROJECT_ID_LEN);
		thp_log_info("%s:get projectfail ,use dummy id:%s\n",
			__func__, cd->project_id);
	}

	/* projectid_from_panel_ver: open 1, close 0 */
	if (cd->projectid_from_panel_ver == 1) {
#if ((!defined CONFIG_LCD_KIT_DRIVER) && (!defined CONFIG_HUAWEI_THP_UDP))
		if (lcdkit_get_panel_version(&value))
			return ret;
#endif
		thp_log_info("%s SVX value from LCD is %u\n", __func__, value);
		value = value >> 4; /* right offset 4 bits */
		if (value == SV1_PANEL) {
			strncpy(cd->project_id, PROJECTID_SV1,
				THP_PROJECT_ID_LEN);
			thp_log_info("%s:project id SV1 is :%s\n", __func__,
				cd->project_id);
		} else if (value == SV2_PANEL) {
			strncpy(cd->project_id, PROJECTID_SV2,
				THP_PROJECT_ID_LEN);
			thp_log_info("%s:project id SV2 is :%s\n",
				__func__, cd->project_id);
		}
	}

	thp_log_info("%s:project id:%s\n", __func__, cd->project_id);
	return ret;
}

static int thp_edit_product_in_project_id(char *project_id,
	unsigned int project_id_length, const char *product)
{
	size_t len;

	if (product == NULL) {
		thp_log_err("%s:product is NULL\n", __func__);
		return -EINVAL;
	}
	len = strlen(product);

	thp_log_info("%s:product len is %zu\n", __func__, len);
	if ((len > project_id_length) || (len == 0))
		return -EINVAL;
	memcpy(project_id, product, len);

	return 0;
}

static int thp_project_id_mapping(struct thp_core_data *cd)
{
	size_t len;

	if ((cd->target_project_id == NULL) ||
		(cd->map_project_id == NULL)) {
		thp_log_err("%s: target_map_project_id is NULL\n", __func__);
		return -EINVAL;
	}
	len = strlen(cd->target_project_id);
	if ((len > THP_PROJECT_ID_LEN) || (len == 0))
		return -EINVAL;
	if (memcmp(cd->target_project_id, cd->project_id, len) == 0)
		memcpy(cd->project_id, cd->map_project_id, len);
	return 0;
}

static int thp_project_init(struct thp_core_data *cd)
{
	int ret = thp_project_in_tp(cd);

#if (!defined CONFIG_HUAWEI_THP_MTK) && (!defined CONFIG_HUAWEI_THP_QCOM)
	if (cd->edit_product_in_project_id &&
		strncmp(cd->project_id, cd->project_id_exception,
			THP_PROJECT_ID_LEN))
#else
	if (cd->edit_product_in_project_id)
#endif
		ret = thp_edit_product_in_project_id(cd->project_id,
			THP_PROJECT_ID_LEN, cd->product);
	if (cd->support_project_id_mapping)
		ret = thp_project_id_mapping(cd);
	thp_log_info("%s:edit_product:%d project id:%s ret:%d\n", __func__,
		cd->edit_product_in_project_id, cd->project_id, ret);
	if (ret != 0)
		return -EINVAL;

	return 0;
}

#if ((defined CONFIG_HUAWEI_THP_MTK) || (defined CONFIG_HUAWEI_THP_QCOM))

#ifndef CONFIG_MODULE_KO_TP
__attribute__((weak)) int power_event_bnc_register(unsigned int type,
	struct notifier_block *nb)
{
	return 0;
}

__attribute__((weak)) int power_event_bnc_unregister(unsigned int type,
	struct notifier_block *nb)
{
	return 0;
}
#endif

static int thp_ctrl_event_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
	int charger_state_before = thp_get_status(THP_STATUS_CHARGER, MAIN_TOUCH_PANEL);
	int charger_state_new = charger_state_before;

	thp_log_debug("%s char event %d\n", __func__, event);
	switch (event) {
	case POWER_NE_USB_CONNECT:
	case POWER_NE_WIRELESS_CONNECT:
		charger_state_new = CHARGER_CONNECT;
		break;
	case POWER_NE_USB_DISCONNECT:
	case POWER_NE_WIRELESS_DISCONNECT:
		charger_state_new = CHARGER_DISCONNECT;
		break;
	default:
		break;
	}
	if (charger_state_before != charger_state_new) {
		thp_log_info("%s, set new status: %d\n",
			__func__, charger_state_new);
		thp_set_status(THP_STATUS_CHARGER, charger_state_new, MAIN_TOUCH_PANEL);
		if (cd->dual_thp)
			thp_set_status(THP_STATUS_CHARGER, charger_state_new, SUB_TOUCH_PANEL);
	}
	return 0;
}

static void thp_nt_charger_notifier_register(struct thp_core_data *cd)
{
	int ret;
	static int flag = 0;

	if (!cd->supported_charger) {
		thp_log_info("%s No supported charger\n", __func__);
		return;
	}
	if (flag) {
		thp_log_info("%s power_event_nc_register has be done\n", __func__);
		return;
	}
	flag++;
	cd->charger_detect_notify.notifier_call = thp_ctrl_event_call;
	ret = power_event_bnc_register(POWER_BNT_CONNECT,
		&cd->charger_detect_notify);
	if (ret) {
		thp_log_err("%s power_event_nc_register failed\n", __func__);
		cd->charger_detect_notify.notifier_call = NULL;
		flag--;
	}
}
#endif

static int thp_misc_dev_register(struct thp_core_data *cd)
{
	int rc;

	cd->thp_misc_device.minor = MISC_DYNAMIC_MINOR;
	cd->thp_misc_device.fops = &g_thp_fops;
	if (cd->dual_thp == 0)
		cd->thp_misc_device.name = THP_MISC_DEVICE_NAME;
	else
		cd->thp_misc_device.name = misc_device_name[cd->panel_id];

	rc = misc_register(&cd->thp_misc_device);
	if (rc) {
		thp_log_err("%s: failed to register misc device\n", __func__);
		return rc;
	}
	return NO_ERR;
}

static int thp_core_init(struct thp_core_data *cd)
{
	int rc = 0;

	/* step 1 : init mutex */
	mutex_init(&cd->mutex_frame);
	mutex_init(&cd->irq_mutex);
	mutex_init(&cd->thp_mutex);
	mutex_init(&cd->status_mutex);
	mutex_init(&cd->suspend_flag_mutex);
#if (defined(CONFIG_HUAWEI_THP_MTK) || defined(CONFIG_HUAWEI_THP_QCOM))
	mutex_init(&cd->aod_power_ctrl_mutex);
#endif
	if (cd->support_gesture_mode)
		mutex_init(&cd->thp_wrong_touch_lock);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	cd->thp_wake_lock = wakeup_source_register(&cd->sdev->dev, "thp_wake_lock");
#else
	cd->thp_wake_lock = wakeup_source_register("thp_wake_lock");
#endif
	if (!cd->thp_wake_lock) {
		thp_log_err("%s: failed to init wakelock\n", __func__);
		goto err_out;
	}

	dev_set_drvdata(&cd->sdev->dev, cd);
	cd->ic_name = cd->thp_dev->ic_name;
	cd->prox_cache_enable = false;
	cd->need_work_in_suspend = false;
	g_thp_prox_enable = false;
	onetime_poweroff_done = false;
	atomic_inc(&count_vote_for_polymeric_chip);
	cd->work_status = RESUME_DONE;

#if defined(CONFIG_HUAWEI_DSM)
	if (cd->ic_name)
		dsm_thp.ic_name = cd->ic_name;
	if (strlen(cd->project_id))
		dsm_thp.module_name = cd->project_id;
	dsm_thp_dclient = dsm_register_client(&dsm_thp);
#endif

#if defined(CONFIG_LCD_KIT_DRIVER)
	rc = thp_sn_code_init(cd);
	if (rc)
		thp_log_err("%s: failed to get sn form tp ic\n", __func__);
#endif

	rc = thp_project_init(cd);
	if (rc)
		thp_log_err("%s: failed to get project id form tp ic\n",
			__func__);

	rc = thp_misc_dev_register(cd);
	if (rc)	{
		thp_log_err("%s: failed to register misc device\n", __func__);
		goto err_register_misc;
	}

	rc = thp_mt_wrapper_init(cd->panel_id);
	if (rc) {
		thp_log_err("%s: failed to init input_mt_wrapper\n", __func__);
		goto err_init_wrapper;
	}

	rc = thp_init_sysfs(cd);
	if (rc) {
		thp_log_err("%s: failed to create sysfs\n", __func__);
		goto err_init_sysfs;
	}

	rc = thp_setup_irq(cd);
	if (rc) {
		thp_log_err("%s: failed to set up irq\n", __func__);
		goto err_register_misc;
	}
#if ((!defined CONFIG_LCD_KIT_DRIVER) && (!defined CONFIG_HUAWEI_THP_UDP))
	cd->lcd_notify.notifier_call = thp_lcdkit_notifier_callback;
	rc = lcdkit_register_notifier(&cd->lcd_notify);
	if (rc) {
		thp_log_err("%s: failed to register fb_notifier: %d\n",
			__func__, rc);
		goto err_register_fb_notify;
	}
#endif

#if defined(CONFIG_LCD_KIT_DRIVER)
	rc = ts_kit_ops_register(&thp_ops);
	if (rc)
		thp_log_info("%s:ts_kit_ops_register fail\n", __func__);
#endif

#if ((defined CONFIG_HUAWEI_THP_MTK) || (defined CONFIG_HUAWEI_THP_QCOM))
	thp_nt_charger_notifier_register(cd);
#else
#ifdef CONFIG_BCI_BATTERY
	thp_bci_charger_notifier_register(cd);
#endif
#endif
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	set_hw_dev_flag(DEV_I2C_TOUCH_PANEL);
#endif

#if defined(CONFIG_TEE_TUI)
	if (!cd->dual_thp)
		thp_tui_init(cd);
	else
		thp_tui_init_for_dual_thp(cd);
#endif

#ifdef CONFIG_HUAWEI_SHB_THP
	if (cd->support_shb_thp_log) {
		if (thp_log_init())
			thp_log_err("%s: failed to init thp log thread\n",
				__func__);
	} else {
		thp_log_info("%s: sensorhub thp log is disabled\n", __func__);
	}
#endif
	atomic_set(&cd->register_flag, 1);
	atomic_set(&cd->esd_para.esd_suspend_status_check, 0);
	thp_set_status(THP_STATUS_POWER, 1, cd->panel_id);
	return 0;

err_init_sysfs:
	thp_mt_wrapper_exit(cd->panel_id);
err_init_wrapper:
	misc_deregister(&cd->thp_misc_device);
err_register_misc:

#if ((!defined CONFIG_LCD_KIT_DRIVER) && (!defined CONFIG_HUAWEI_THP_UDP))
	lcdkit_unregister_notifier(&cd->lcd_notify);
err_register_fb_notify:
#endif

#if defined(CONFIG_LCD_KIT_DRIVER)
	rc = ts_kit_ops_unregister(&thp_ops);
	if (rc)
		thp_log_info("%s:ts_kit_ops_register fail\n", __func__);
#endif
err_out:
	mutex_destroy(&cd->mutex_frame);
	mutex_destroy(&cd->irq_mutex);
	mutex_destroy(&cd->thp_mutex);
#ifdef CONFIG_HUAWEI_THP_MTK
	mutex_destroy(&cd->aod_power_ctrl_mutex);
#endif
	wakeup_source_unregister(cd->thp_wake_lock);
	cd->thp_wake_lock = NULL;
	if (cd->support_gesture_mode)
		mutex_destroy(&cd->thp_wrong_touch_lock);
	return rc;
}

static int thp_parse_test_config(struct device_node *test_node,
	struct thp_test_config *config)
{
	int rc;
	unsigned int value = 0;

	if (!test_node || !config) {
		thp_log_info("%s: input dev null\n", __func__);
		return -ENODEV;
	}

	rc = of_property_read_u32(test_node,
			"pt_station_test", &value);
	if (!rc) {
		config->pt_station_test = value;
		thp_log_info("%s:pt_test_flag %d\n",
			__func__, value);
	}

	return 0;
}

static struct device_node *thp_get_dev_node(struct thp_core_data *cd,
	struct thp_device *dev)
{
	struct device_node *dev_node = of_get_child_by_name(cd->thp_node,
						dev->ic_name);

	if (!dev_node && dev->dev_node_name)
		return of_get_child_by_name(cd->thp_node, dev->dev_node_name);

	return dev_node;
}

static void thp_chip_detect(struct thp_cmd_node *in_cmd)
{
	int ret;
	struct thp_device *dev = NULL;

	if (in_cmd == NULL) {
		thp_log_err("%s:input is NULL\n", __func__);
		return;
	}
	dev = in_cmd->cmd_param.pub_params.dev;
	ret = thp_register_dev(dev, in_cmd->cmd_param.panel_id);
	if (ret)
		thp_log_err("%s,register failed\n", __func__);
}

void thp_send_detect_cmd(struct thp_device *dev, u32 panel_id, int timeout)
{
	int error;
	struct thp_cmd_node cmd;

	thp_log_info("%s: called\n", __func__);
	if (dev == NULL) {
		thp_log_info("%s: input is invalid\n", __func__);
		return;
	}
	thp_unregister_ic_num++;
	thp_log_info("%s:thp_unregister_ic_num:%d",
		__func__, thp_unregister_ic_num);
	memset(&cmd, 0, sizeof(cmd));
	cmd.command = TS_CHIP_DETECT;
	cmd.cmd_param.pub_params.dev = dev;
	cmd.cmd_param.panel_id = panel_id;
	cmd.cmd_param.thread_id = panel_id;
	error = thp_put_one_cmd(&cmd, timeout);
	if (error)
		thp_log_err("%s: put cmd error :%d\n", __func__, error);
}
EXPORT_SYMBOL(thp_send_detect_cmd);

void thp_time_delay(unsigned int time)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);

	if (time == 0)
		return;
	if ((time < TIME_DELAY_MS_MAX) && cd->use_mdelay)
		mdelay(time);
	else
		thp_do_time_delay(time);
}
EXPORT_SYMBOL(thp_time_delay);

#ifndef CONFIG_HUAWEI_THP_MTK
#define SUPPORT_PINCTR 1
static int thp_pinctrl_init(struct thp_device *dev, u32 panel_id)
{
	int rc;
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	if (cd->support_pinctrl != SUPPORT_PINCTR)
		return 0;

	rc = thp_pinctrl_get_init(dev);
	if (rc) {
		thp_log_err("%s:pinctrl get init fail\n", __func__);
		return -EINVAL;
	}

	if (cd->pinctrl_init_enable) {
		rc = thp_pinctrl_select_normal(cd->thp_dev);
		if (rc) {
			thp_log_err("%s:select_normal fail\n", __func__);
			return -EINVAL;
		}
		thp_log_info("%s: select_normal sucessed\n", __func__);
	}
	return 0;
}
#endif

static int thp_check_tp_vendor(struct thp_device *dev)
{
	struct thp_core_data *cd = dev->thp_core;

	if (!cd->check_tp_vendor)
		return 0;

	if (cd->ic_name && dev->ic_name &&
			strcmp(cd->ic_name, dev->ic_name)) {
		thp_log_err("%s:mismatch lcd supply ic vendor\n",
					__func__);
		return -EINVAL;
	}
	return 0;
}

#define WIAT_THP_INIT_TIMEOUT 60
static int thp_check_register_status(struct thp_core_data *cd, struct thp_device *dev)
{
	int rc;

	/*
	 * 1. wait for thp probe done, wait time = 60s
	 * 2. only wait once when there are two or more IC drivers
	 * 3. if thp probe fail,  the wait will time out,need goto register_err
	 */
	if (!cd->boot_sync.thp_probe_done) {
		rc = wait_for_completion_timeout(&cd->boot_sync.thp_init_done, WIAT_THP_INIT_TIMEOUT * HZ);
		thp_log_info("%s:wait_for_completion_timeout ret = %lu\n", __func__, rc);
		if (!rc)
			goto register_err;
		cd->boot_sync.thp_probe_done++;
	}
	/* check device configed ot not */
	if (!thp_get_dev_node(cd, dev)) {
		thp_log_info("%s:%s not config in dts\n",
				__func__, dev->ic_name);
		goto register_err;
	}

	if (atomic_read(&cd->register_flag)) {
		thp_log_err("%s: thp have registerd\n", __func__);
		goto register_err;
	}

	if (!cd->project_in_tp && cd->ic_name && dev->ic_name &&
			strcmp(cd->ic_name, dev->ic_name)) {
		thp_log_err("%s:driver support ic mismatch connected device\n",
					__func__);
		goto register_err;
	}
	return 0;
register_err:
	return -EINVAL;
}

static int thp_dev_detect(struct thp_core_data *cd, struct thp_device *dev)
{
	int rc;

	rc = dev->ops->detect(dev);
	if (rc) {
		thp_log_err("%s: chip detect fail\n", __func__);
		return rc;
	}
	if (cd->dual_thp && !lcd_product_type) {
		if (cd->panel_id != lcd_panel_index) {
			dev->ops->suspend(dev);
			thp_log_info("%s: lcd only support one panel:%d\n",
				__func__, cd->panel_id);
			rc = -EINVAL;
			return rc;
		}
	}
#ifndef CONFIG_HUAWEI_THP_MTK
	if (!cd->support_control_cs_off) {
		rc = thp_pinctrl_init(dev, cd->panel_id);
		if (rc) {
			thp_log_err("%s: pinctrl init fail\n", __func__);
			return rc;
		}
	}
#endif
	rc = thp_core_init(cd);
	if (rc) {
		thp_log_err("%s: core init\n", __func__);
		return rc;
	}
	return rc;
}

int thp_register_dev(struct thp_device *dev, u32 panel_id)
{
	int rc = -EINVAL;
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	if ((dev == NULL) || (cd == NULL)) {
		thp_log_err("%s: input null\n", __func__);
		goto register_err;
	}
	thp_log_info("%s: called\n", __func__);

	rc = thp_check_register_status(cd, dev);
	if (rc)
		goto register_err;

	dev->thp_core = cd;
	dev->gpios = &cd->gpios;
	dev->sdev = cd->sdev;
	cd->thp_dev = dev;
	is_fw_update = 0;

	rc = thp_parse_timing_config(cd->thp_node, &dev->timing_config);
	if (rc) {
		thp_log_err("%s: timing config parse fail\n", __func__);
		goto register_err;
	}

	rc = thp_parse_test_config(cd->thp_node, &dev->test_config);
	if (rc) {
		thp_log_err("%s: special scene config parse fail\n", __func__);
		goto register_err;
	}

	rc = dev->ops->init(dev);
	if (rc) {
		thp_log_err("%s: dev init fail\n", __func__);
		goto dev_init_err;
	}
#ifdef CONFIG_HUAWEI_THP_QCOM
	if (cd->support_control_cs_off) {
		rc = thp_pinctrl_init(dev, cd->panel_id);
		if (rc) {
			thp_log_err("%s: spi dev init fail\n", __func__);
			goto dev_init_err;
		}
		pinctrl_select_state(dev->thp_core->pctrl,
			dev->thp_core->qcom_pinctrl.cs_high);
	}
#endif
#ifdef CONFIG_HUAWEI_THP_MTK
	if (cd->support_pinctrl == 1) {
		rc = thp_pinctrl_get_init(dev);
		if (rc) {
			thp_log_err("%s:pinctrl get init fail\n", __func__);
			goto dev_init_err;
		}
		pinctrl_select_state(dev->thp_core->pctrl,
			dev->thp_core->mtk_pinctrl.cs_high);
	}
#else
	rc = thp_setup_gpio(cd);
	if (rc) {
		thp_log_err("%s: spi dev init fail\n", __func__);
		goto dev_init_err;
	}
#endif

	rc = thp_setup_spi(cd);
	if (rc) {
		thp_log_err("%s: spi dev init fail\n", __func__);
		goto err;
	}

	rc = thp_check_tp_vendor(dev);
	if (rc) {
		thp_log_err("%s: not match lcd verndor,return\n", __func__);
		goto err;
	}
	if (cd->power_cfg.power_supply_mode) {
		thp_power_lock();
		touch_driver_power_init(dev);
		g_thp_common_power[cd->power_cfg.power_sequence].power_on(dev);
		thp_power_unlock();
	}
	rc = thp_dev_detect(cd, dev);
	if (rc)
		goto err;
	if (cd->fast_booting_solution) {
		thp_unregister_ic_num--;
		thp_log_info("%s:thp_unregister_ic_num :%d",
			__func__, thp_unregister_ic_num);
	}
	return 0;
err:
#ifndef CONFIG_HUAWEI_THP_MTK
	thp_free_gpio(cd);
#endif
dev_init_err:
	cd->thp_dev = 0;
register_err:
	if (cd && cd->fast_booting_solution) {
		thp_unregister_ic_num--;
		thp_log_info("%s:thp_unregister_ic_num :%d",
			__func__, thp_unregister_ic_num);
	}
	return rc;
}
EXPORT_SYMBOL(thp_register_dev);

#define THP_SUPPORT_PINCTRL "support_pinctrl"
int thp_parse_pinctrl_config(struct device_node *spi_cfg_node,
	struct thp_core_data *cd)
{
	int rc;
	unsigned int value = 0;

	if ((spi_cfg_node == NULL) || (cd == NULL)) {
		thp_log_info("%s: input null\n", __func__);
		return -ENODEV;
	}
	rc = of_property_read_u32(spi_cfg_node, THP_SUPPORT_PINCTRL, &value);
	if (rc == 0)
		cd->support_pinctrl = value;
	else
		cd->support_pinctrl = 0;
	thp_log_info("%s:support_pinctrl %d\n", __func__, value);

	rc = of_property_read_u32(spi_cfg_node, PINCTRL_INIT_ENABLE, &value);
	if (rc == 0)
		cd->pinctrl_init_enable = value;
	else
		cd->pinctrl_init_enable = 0;
	thp_log_info("%s:pinctrl_init_enable %u\n", __func__, value);

	return 0;
}

#ifdef CONFIG_HUAWEI_THP_QCOM
void thp_qcom_parse_spi_config(struct device_node *spi_cfg_node,
	struct thp_core_data *cd)
{
	unsigned int value = 0;
	int rc;

	rc = of_property_read_u32(spi_cfg_node, "spi_cs_clk_delay", &value);
	if (!rc) {
		cd->qcom_spi_config.spi_cs_clk_delay = value;
		thp_log_info("%s:spi_cs_clk_delay configed %u",
			__func__, value);
	}

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "spi_inter_words_delay", &value);
	if (!rc) {
		cd->qcom_spi_config.spi_inter_words_delay = value;
		thp_log_info("%s:spi_inter_words_delay configed %u\n",
			__func__, value);
	}
}
#endif

int thp_parse_spi_config(struct device_node *spi_cfg_node,
	struct thp_core_data *cd)
{
	int rc;
	unsigned int value;
	struct thp_spi_config *spi_config = NULL;
	struct pl022_config_chip *pl022_spi_config = NULL;

	if (!spi_cfg_node || !cd) {
		thp_log_info("%s: input null\n", __func__);
		return -ENODEV;
	}

	spi_config = &cd->spi_config;
	pl022_spi_config = &cd->spi_config.pl022_spi_config;

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "spi-max-frequency", &value);
	if (!rc) {
		spi_config->max_speed_hz = value;
		thp_log_info("%s:spi-max-frequency configed %d\n",
				__func__, value);
	}

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "spi-bus-id", &value);
	if (!rc) {
		spi_config->bus_id = (u8)value;
		thp_log_info("%s:spi-bus-id configed %d\n", __func__, value);
	}

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "spi-mode", &value);
	if (!rc) {
		spi_config->mode = value;
		thp_log_info("%s:spi-mode configed %d\n", __func__, value);
	}

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "bits-per-word", &value);
	if (!rc) {
		spi_config->bits_per_word = value;
		thp_log_info("%s:bits-per-word configed %d\n", __func__, value);
	}

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "pl022,interface", &value);
	if (!rc) {
		pl022_spi_config->iface = value;
		thp_log_info("%s: pl022,interface parsed\n", __func__);
	}
	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "pl022,com-mode", &value);
	if (!rc) {
		pl022_spi_config->com_mode = value;
		thp_log_info("%s:com_mode parsed\n", __func__);
	}
	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "pl022,rx-level-trig", &value);
	if (!rc) {
		pl022_spi_config->rx_lev_trig = value;
		thp_log_info("%s:rx-level-trig parsed\n", __func__);
	}

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "pl022,tx-level-trig", &value);
	if (!rc) {
		pl022_spi_config->tx_lev_trig = value;
		thp_log_info("%s:tx-level-trig parsed\n", __func__);
	}

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "pl022,ctrl-len", &value);
	if (!rc) {
		pl022_spi_config->ctrl_len = value;
		thp_log_info("%s:ctrl-len parsed\n", __func__);
	}

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "pl022,wait-state", &value);
	if (!rc) {
		pl022_spi_config->wait_state = value;
		thp_log_info("%s:wait-state parsed\n", __func__);
	}

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "pl022,duplex", &value);
	if (!rc) {
		pl022_spi_config->duplex = value;
		thp_log_info("%s:duplex parsed\n", __func__);
	}
#ifdef CONFIG_HUAWEI_THP_QCOM
	thp_qcom_parse_spi_config(spi_cfg_node, cd);
#endif
	if (cd->panel_id == MAIN_TOUCH_PANEL)
		cd->spi_config.pl022_spi_config.cs_control = thp_spi_cs_set;
	else if (cd->panel_id == SUB_TOUCH_PANEL)
		cd->spi_config.pl022_spi_config.cs_control = thp_spi_cs_set_sub_panel;
	cd->spi_config.pl022_spi_config.hierarchy = SSP_MASTER;

	if (!cd->spi_config.max_speed_hz)
		cd->spi_config.max_speed_hz = THP_SPI_SPEED_DEFAULT;
	if (!cd->spi_config.mode)
		cd->spi_config.mode = SPI_MODE_0;
	if (!cd->spi_config.bits_per_word)
	/* spi_config.bits_per_word default value is 8 */
		cd->spi_config.bits_per_word = 8;

#ifdef CONFIG_HUAWEI_THP_MTK
	/* tx ordering, 1-msb first send; 0-lsb first end */
	cd->mtk_spi_config.rx_mlsb = 1;
	/* rx ordering, 1-msb first send; 0-lsb first end */
	cd->mtk_spi_config.tx_mlsb = 1;
	/* control cs polarity, 0-active low; 1-active high */
	cd->mtk_spi_config.cs_pol = 0;
	/* control sample edge, 0-positive edge; 1-negative edge */
	cd->mtk_spi_config.sample_sel = 0;
	/* cs setup time, 0-default time */
	cd->mtk_spi_config.cs_setuptime = 0;
#endif
	cd->sdev->mode = spi_config->mode;
	cd->sdev->max_speed_hz = spi_config->max_speed_hz;
	cd->sdev->bits_per_word = spi_config->bits_per_word;
#if (!defined CONFIG_HUAWEI_THP_QCOM) && (!defined CONFIG_HUAWEI_THP_MTK)
	cd->sdev->controller_data = &spi_config->pl022_spi_config;
#endif
#ifdef CONFIG_HUAWEI_THP_MTK
	cd->sdev->controller_data = (void *)&(cd->mtk_spi_config);
#endif
#ifdef CONFIG_HUAWEI_THP_QCOM
	cd->sdev->controller_data = (void *)&(cd->qcom_spi_config);
#endif

	return 0;
}
EXPORT_SYMBOL(thp_parse_spi_config);

int thp_parse_timing_config(struct device_node *timing_cfg_node,
	struct thp_timing_config *timing)
{
	int rc;
	unsigned int value;

	if (!timing_cfg_node || !timing) {
		thp_log_info("%s: input null\n", __func__);
		return -ENODEV;
	}

	rc = of_property_read_u32(timing_cfg_node,
					"boot_reset_hi_delay_ms", &value);
	if (!rc) {
		timing->boot_reset_hi_delay_ms = value;
		thp_log_info("%s:boot_reset_hi_delay_ms configed %d\n",
				__func__, value);
	}

	timing->later_power_off_delay_ms = 0;
	rc = of_property_read_u32(timing_cfg_node,
		"later_power_off_delay_ms", &value);
	if (!rc) {
		timing->later_power_off_delay_ms = value;
		thp_log_info("%s:later_power_off_delay_ms configed %u\n",
			__func__, value);
	}

	rc = of_property_read_u32(timing_cfg_node,
					"boot_reset_low_delay_ms", &value);
	if (!rc) {
		timing->boot_reset_low_delay_ms = value;
		thp_log_info("%s:boot_reset_low_delay_ms configed %d\n",
				__func__, value);
	}

	rc = of_property_read_u32(timing_cfg_node,
					"boot_reset_after_delay_ms", &value);
	if (!rc) {
		timing->boot_reset_after_delay_ms = value;
		thp_log_info("%s:boot_reset_after_delay_ms configed %d\n",
				__func__, value);
	}

	rc = of_property_read_u32(timing_cfg_node,
					"resume_reset_after_delay_ms", &value);
	if (!rc) {
		timing->resume_reset_after_delay_ms = value;
		thp_log_info("%s:resume_reset_after_delay_ms configed %d\n",
				__func__, value);
	}

	rc = of_property_read_u32(timing_cfg_node,
					"suspend_reset_after_delay_ms", &value);
	if (!rc) {
		timing->suspend_reset_after_delay_ms = value;
		thp_log_info("%s:suspend_reset_after_delay configed_ms %d\n",
				__func__, value);
	}

	rc = of_property_read_u32(timing_cfg_node,
					"spi_sync_cs_hi_delay_ns", &value);
	if (!rc) {
		timing->spi_sync_cs_hi_delay_ns = value;
		thp_log_info("%s:spi_sync_cs_hi_delay_ns configed_ms %d\n",
				__func__, value);
	}

	rc = of_property_read_u32(timing_cfg_node,
					"spi_sync_cs_low_delay_ns", &value);
	if (!rc) {
		timing->spi_sync_cs_low_delay_ns = value;
		thp_log_info("%s:spi_sync_cs_low_delay_ns configed_ms %d\n",
				__func__, value);
	}
	rc = of_property_read_u32(timing_cfg_node,
					"spi_sync_cs_low_delay_us", &value);
	if (!rc) {
		timing->spi_sync_cs_low_delay_us = value;
		thp_log_info("%s:spi_sync_cs_low_delay_us = %d\n",
				__func__, value);
	} else {
		timing->spi_sync_cs_low_delay_us = 0;
	}

	rc = of_property_read_u32(timing_cfg_node,
		"boot_vcc_on_after_delay_ms", &value);
	if (!rc) {
		timing->boot_vcc_on_after_delay_ms = value;
		thp_log_info("%s:boot_vcc_on_after_delay_ms configed_ms %d\n",
			__func__, value);
	}
	rc = of_property_read_u32(timing_cfg_node,
		"boot_vddio_on_after_delay_ms", &value);
	if (!rc) {
		timing->boot_vddio_on_after_delay_ms = value;
		thp_log_info("%s:boot_vddio_on_after_delay_ms configed_ms %d\n",
			__func__, value);
	}
	rc = of_property_read_u32(timing_cfg_node,
		"spi_transfer_delay_us", &value);
	if (!rc) {
		timing->spi_transfer_delay_us = value;
		thp_log_info("%s:spi_transfer_delay_us = %d\n",
			__func__, value);
	} else {
		timing->spi_transfer_delay_us = 0;
	}
	if (!of_property_read_u32(timing_cfg_node,
		"early_suspend_delay_ms", &value)) {
		timing->early_suspend_delay_ms = value;
		thp_log_info("%s:early_suspend_delay_ms configed_ms %u\n",
			__func__, value);
	}
	timing->before_pen_clk_disable_delay_ms = 0;
	rc = of_property_read_u32(timing_cfg_node,
		"before_pen_clk_disable_delay_ms", &value);
	if (!rc) {
		timing->before_pen_clk_disable_delay_ms = value;
		thp_log_info("%s:before_pen_clk_disable_delay_ms configed_ms %d\n",
			__func__, value);
	}

	rc = of_property_read_u32(timing_cfg_node,
		"recovery_power_delay_ms", &value);
	if (!rc) {
		timing->recovery_power_delay_ms = value;
		thp_log_info("%s:recovery_power_delay_ms configed_ms %d\n",
			__func__, value);
	}

	timing->tui_exit_wait_time_ms = 0;
	rc = of_property_read_u32(timing_cfg_node, "tui_exit_wait_time_ms", &value);
	if (!rc) {
		timing->tui_exit_wait_time_ms = value;
		thp_log_info("%s:tui_exit_wait_time_ms configed_ms %d\n",
			__func__, value);
	}

	return 0;
}
EXPORT_SYMBOL(thp_parse_timing_config);

int thp_parse_trigger_config(struct device_node *thp_node,
	struct thp_core_data *cd)
{
	int rc;
	unsigned int value = 0;

	thp_log_debug("%s:Enter!\n", __func__);
	rc = of_property_read_u32(thp_node, "irq_flag", &value);
	if (!rc) {
		cd->irq_flag = value;
		thp_log_info("%s:cd->irq_flag %d\n", __func__, value);
	} else {
		cd->irq_flag = IRQF_TRIGGER_FALLING;
		thp_log_info("%s:cd->irq_flag defaule => %d\n",
			__func__, cd->irq_flag);
	}

	if ((cd->irq_flag & IRQF_TRIGGER_HIGH) ||
		(cd->irq_flag & IRQF_TRIGGER_LOW)) {
		value = 0;
		rc = of_property_read_u32(thp_node,
			"not_control_irq_in_irq_handler", &value);
		if (!rc)
			cd->not_control_irq_in_irq_handler = value;
		else
			cd->not_control_irq_in_irq_handler = 0;
		value = 0;
		rc = of_property_read_u32(thp_node, "avoid_low_level_irq_storm",
			&value);
		if (!rc)
			cd->avoid_low_level_irq_storm = value;
		else
			cd->avoid_low_level_irq_storm = 0;
	} else {
		cd->not_control_irq_in_irq_handler = 0;
		cd->avoid_low_level_irq_storm = 0;
	}
	thp_log_info("%s:not_control_irq_in_irq_handler: %u\n",
		__func__, cd->not_control_irq_in_irq_handler);
	thp_log_info("%s: avoid_low_level_irq_storm %u\n",
		__func__, cd->avoid_low_level_irq_storm);
	return 0;
}
EXPORT_SYMBOL(thp_parse_trigger_config);

static void thp_parse_pen_feature_config(
	struct device_node *thp_node, struct thp_core_data *cd)
{
	int rc;
	unsigned int value = 0;

	rc = of_property_read_u32(thp_node, "pen_supported", &value);
	if (!rc) {
		cd->pen_supported = value;
		thp_log_info("%s:pen_supported %u\n", __func__, value);
	}

	rc = of_property_read_u32(thp_node, "pen_mt_enable_flag",
		&value);
	if (!rc) {
		cd->pen_mt_enable_flag = value;
		thp_log_info("%s:pen_mt_enable_flag configed %u\n",
			__func__, value);
	}

	rc = of_property_read_u32(thp_node, "pen_change_protocol", &value);
	if (!rc) {
		cd->pen_change_protocol = value;
		thp_log_info("%s:pen_change_protocol %u\n", __func__, value);
	}

	rc = of_property_read_u32(thp_node, "send_bt_status_to_fw", &value);
	if (!rc) {
		cd->send_bt_status_to_fw = value;
		thp_log_info("%s: send_bt_status_to_fw %u\n", __func__,
			value);
	}

	rc = of_property_read_u32(thp_node, "send_adsorption_status_to_fw",
		&value);
	if (!rc) {
		cd->send_adsorption_status_to_fw = value;
		thp_log_info("%s: send_adsorption_status_to_fw %u\n", __func__,
			value);
	}

	rc = of_property_read_u32(thp_node,
		"support_stylus3_plam_suppression", &value);
	if (!rc) {
		cd->support_stylus3_plam_suppression = value;
		thp_log_info("%s: support_stylus3_plam_suppression %u\n",
			__func__, value);
	}
	rc = of_property_read_u32(thp_node, "send_stylus3_workmode_to_fw",
		&value);
	if (!rc) {
		cd->send_stylus3_workmode_to_fw = value;
		thp_log_info("%s: send_stylus3_workmode_to_fw %u\n", __func__,
			value);
	}
	rc = of_property_read_u32(thp_node, "unsupported_stylus", &value);
	if (!rc) {
		cd->unsupported_stylus = value;
		thp_log_info("%s: unsupported_stylus %u\n",
			__func__, value);
	}
	rc = of_property_read_u32(thp_node, "app_send_stylus3_status_to_fw", &value);
	if (!rc) {
		cd->app_send_stylus3_status_to_fw = value;
		thp_log_info("%s: app_send_stylus3_status_to_fw %u\n",
			__func__, value);
	}
}

static void thp_parse_multi_panel_config(
	struct device_node *thp_node, struct thp_core_data *cd)
{
	unsigned int value = 0;

	cd->multi_panel_index = SINGLE_TOUCH_PANEL;
	if (!of_property_read_u32(thp_node, "multi_panel_index", &value))
		cd->multi_panel_index = value;
	thp_log_info("%s: multi_panel_index %u\n", __func__,
		cd->multi_panel_index);

	cd->support_multi_panel_attach = 0;
	if (!of_property_read_u32(thp_node, "support_multi_panel_attach",
		&value))
		cd->support_multi_panel_attach = value;
	thp_log_info("%s: support_multi_panel_attach %u\n",
		__func__, cd->support_multi_panel_attach);

	cd->support_resend_status_to_daemon = 0;
	if (!of_property_read_u32(thp_node, "support_resend_status_to_daemon",
		&value))
		cd->support_resend_status_to_daemon = value;
	thp_log_info("%s: support_resend_status_to_daemon %u\n",
		__func__, cd->support_resend_status_to_daemon);

	cd->support_input_location = 0;
	if (!of_property_read_u32(thp_node, "support_input_location",
		&value))
		cd->support_input_location = value;
	thp_log_info("%s: support_input_location %u\n",
		__func__, cd->support_input_location);
}

static void  thp_parse_extra_feature_config(
	struct device_node *thp_node, struct thp_core_data *cd)
{
	int rc;
	unsigned int value = 0;

	rc = of_property_read_u32(thp_node, "support_factory_mode_extra_cmd",
		&value);
	if (!rc) {
		cd->support_factory_mode_extra_cmd = value;
		thp_log_info("%s: support_factory_mode_extra_cmd %u\n",
			__func__, value);
	}
	value = 0;
	rc = of_property_read_u32(thp_node, "support_no_cs_pinctrl_for_mtk",
		&value);
	if (!rc) {
		cd->not_support_cs_control = value;
	} else {
		value = 0;
		rc = of_property_read_u32(thp_node, "support_no_cs_for_qcom",
			&value);
		if (!rc)
			cd->not_support_cs_control = value;
	}
	thp_log_info("%s:not_support_cs_control %u\n",
		__func__, value);
	cd->support_control_cs_off = 0;
	rc = of_property_read_u32(thp_node, "support_control_cs_off", &value);
	if (!rc) {
		cd->support_control_cs_off = value;
		thp_log_info("%s:support_control_cs_off %u\n", __func__,
			value);
	}

	if (of_find_property(thp_node, "kirin-udp", NULL))
		cd->is_udp = true;
	else
		cd->is_udp = false;

	value = 0;
	rc = of_property_read_u32(thp_node, "use_fb_notify_to_sr", &value);
	if (!rc) {
		cd->use_fb_notify_to_sr = value;
		thp_log_info("%s: use_fb_notify_to_sr %u\n",
			__func__, value);
	}

	rc = of_property_read_u32(thp_node,
		"support_shb_thp_tui_common_id", &value);
	if (!rc) {
		cd->support_shb_thp_tui_common_id = value;
		thp_log_info("%s: support_shb_thp_tui_common_id %u\n",
			__func__, value);
	}

	rc = of_property_read_u32(thp_node,
		"thp_shb_timeout_hard_reset", &value);
	if (!rc) {
		cd->thp_shb_timeout_hard_reset = value;
		thp_log_info("%s: thp_shb_timeout_hard_reset %u\n",
			__func__, value);
	}

	rc = of_property_read_u32(thp_node, "support_diff_resolution",
		&value);
	if (!rc) {
		cd->support_diff_resolution = value;
		thp_log_info("%s: support_diff_resolution %u\n",
			__func__, value);
	}

	rc = of_property_read_u32(thp_node, "need_notify_to_roi_algo",
		&value);
	if (!rc) {
		cd->need_notify_to_roi_algo = value;
		thp_log_info("%s: need_notify_to_roi_algo %u\n",
			__func__, value);
	}

	rc = of_property_read_u32(thp_node, "support_daemon_init_protect",
		&value);
	if (!rc) {
		cd->support_daemon_init_protect = value;
		thp_log_info("%s: support_daemon_init_protect %u\n",
			__func__, value);
	}
	cd->support_reuse_ic_type = 0;
	rc = of_property_read_u32(thp_node, "support_reuse_ic_type",
		&value);
	if (!rc) {
		cd->support_reuse_ic_type = value;
		thp_log_info("%s: support_reuse_ic_type %u\n",
			__func__, value);
	}
	rc = of_property_read_u32(thp_node, "use_thp_queue",
		&value);
	if (!rc) {
		cd->use_thp_queue = value;
		thp_log_info("%s: use_thp_queue %u\n",
			__func__, value);
	}
	rc = of_property_read_u32(thp_node, "thp_queue_buf_len",
		&value);
	if (!rc) {
		cd->thp_queue_buf_len = value;
		thp_log_info("%s: thp_queue_buf_len %u\n",
			__func__, value);
	}
	rc = of_property_read_u32(thp_node, "supported_charger", &value);
	if (!rc) {
		cd->supported_charger = value;
		thp_log_info("%s:supported_charger %u\n", __func__, value);
	}
	rc = of_property_read_u32(thp_node, "tp_time_sync_support", &value);
	if (!rc) {
		cd->tp_time_sync_support = value;
		thp_log_info("%s:tp_time_sync_support %u\n", __func__, value);
	}
	rc = of_property_read_u32(thp_node, "support_phone_sleep_mode",
		&value);
	if (!rc) {
		cd->support_phone_sleep_mode = value;
		thp_log_info("%s: support_phone_sleep_mode %u\n",
			__func__, value);
	}
	/* for goodix detect */
	cd->not_support_ic_change_init_mode = 0;
	rc = of_property_read_u32(thp_node, "not_support_ic_change_init_mode",
		&value);
	if (!rc) {
		cd->not_support_ic_change_init_mode = value;
		thp_log_info("%s: not_support_ic_change_init_mode %u\n",
			__func__, value);
	}
	cd->aptouch_daemon_version = 2; /* 2 is daemon default version */
	rc = of_property_read_u32(thp_node, "aptouch_daemon_version",
		&value);
	if (!rc) {
		cd->aptouch_daemon_version = value;
		thp_log_info("%s: aptouch_daemon_version: %u\n",
			__func__, value);
	}
	cd->tp_assisted_ultrasonic_algo = 0;
	rc = of_property_read_u32(thp_node, "tp_assisted_ultrasonic_algo",
		&value);
	if (!rc) {
		cd->tp_assisted_ultrasonic_algo = value;
		thp_log_info("%s: tp_assisted_ultrasonic_algo: %u\n",
			__func__, value);
	}

	cd->need_spi_communication_in_suspend = 0;
	rc = of_property_read_u32(thp_node, "need_spi_communication_in_suspend",
		&value);
	if (!rc) {
		cd->need_spi_communication_in_suspend = value;
		thp_log_info("%s: need_spi_communication_in_suspend: %u\n",
			__func__, value);
	}
	thp_parse_multi_panel_config(thp_node, cd);
}

static void thp_parse_project_id_map_config(struct device_node *thp_node,
	struct thp_core_data *cd)
{
	unsigned int value;

	cd->support_project_id_mapping = 0;
	if (!of_property_read_u32(thp_node, "support_project_id_mapping",
		&value)) {
		cd->support_project_id_mapping = value;
		thp_log_info("%s: support_project_id_mapping %u\n", __func__,
			value);
	}
	if (!(cd->support_project_id_mapping))
		return;
	cd->target_project_id = NULL;
	cd->map_project_id = NULL;
	if (!of_property_read_string(thp_node, "target_project_id",
		(const char **)&cd->target_project_id))
		thp_log_info("%s: target_project_id %s\n", __func__,
			cd->target_project_id);
	if (!of_property_read_string(thp_node, "map_project_id",
		(const char **)&cd->map_project_id))
		thp_log_info("%s: map_project_id %s\n", __func__,
			cd->map_project_id);
}

static void thp_dts_parse_process_template(int rc, unsigned int value, unsigned int *target,
	const char *func_name, const char *log)
{
	if (!rc) {
		*target = value;
		thp_log_info("%s:%s: %u\n", func_name, log, value);
	}
}

int thp_parse_esd_reset_config(struct device_node *thp_node,
	struct thp_core_data *cd)
{
	int rc;
	unsigned int value = 0;
	rc = of_property_read_u32(thp_node, "support_esd_power_reset", &value);
	thp_dts_parse_process_template(rc, value, &(cd->esd_para.support_esd_power_reset),
		__func__, "support_esd_power_reset");

	rc = of_property_read_u32(thp_node, "esd_delay_time", &value);
	thp_dts_parse_process_template(rc, value, &(cd->esd_para.esd_delay_time),
	__func__, "esd_delay_time");

	return 0;
}

int thp_parse_feature_config(struct device_node *thp_node,
	struct thp_core_data *cd)
{
	int rc;
	unsigned int value = 0;

	thp_log_debug("%s:Enter!\n", __func__);

	rc = of_property_read_u32(thp_node, "self_control_power", &value);
	thp_dts_parse_process_template(rc, value, &(cd->self_control_power),
		__func__, "self_control_power");

	rc = of_property_read_u32(thp_node, "project_in_tp", &value);
	thp_dts_parse_process_template(rc, value, &(cd->project_in_tp),
		__func__, "project_in_tp");

	rc = of_property_read_u32(thp_node, "lcd_esd_event_upload", &value);
	thp_dts_parse_process_template(rc, value, &(cd->lcd_esd_event_upload),
		__func__, "lcd_esd_event_upload");

	cd->project_id_dummy = "dummy";
	rc = of_property_read_string(thp_node, "project_id_dummy",
		(const char **)&cd->project_id_dummy);
	if (!rc)
		thp_log_info("%s:project_id_dummy configed %s\n",
			__func__, cd->project_id_dummy);

	rc = of_property_read_u32(thp_node, "supported_func_indicater", &value);
	thp_dts_parse_process_template(rc, value, &(cd->supported_func_indicater),
		__func__, "supported_func_indicater");

	rc = of_property_read_u32(thp_node, "use_hwlock", &value);
	thp_dts_parse_process_template(rc, value, &(cd->use_hwlock),
		__func__, "use_hwlock");

	rc = of_property_read_u32(thp_node, "support_shb_thp", &value);
	thp_dts_parse_process_template(rc, value, &(cd->support_shb_thp),
		__func__, "support_shb_thp");

	rc = of_property_read_u32(thp_node, "support_shb_thp_log", &value);
	thp_dts_parse_process_template(rc, value, &(cd->support_shb_thp_log),
		__func__, "support_shb_thp_log");

	rc = of_property_read_u32(thp_node, "support_shb_thp_app_switch", &value);
	thp_dts_parse_process_template(rc, value, &(cd->support_shb_thp_app_switch),
		__func__, "support_shb_thp_app_switch");

	rc = of_property_read_u32(thp_node, "use_ap_sh_common_int", &value);
	thp_dts_parse_process_template(rc, value, &(cd->use_ap_sh_common_int),
		__func__, "use_ap_sh_common_int");

	rc = of_property_read_u32(thp_node, "delay_work_for_pm", &value);
	thp_dts_parse_process_template(rc, value, &(cd->delay_work_for_pm),
		__func__, "delay_work_for_pm");

	rc = of_property_read_u32(thp_node, "use_ap_gesture", &value);
	thp_dts_parse_process_template(rc, value, &(cd->use_ap_gesture),
		__func__, "use_ap_gesture");

	rc = of_property_read_u32(thp_node, "use_aod_power_ctrl_notify", &value);
	thp_dts_parse_process_template(rc, value, &(cd->use_aod_power_ctrl_notify),
		__func__, "use_aod_power_ctrl_notify");

	rc = of_property_read_u32(thp_node, "suspend_delayms_early_to_before", &value);
	thp_dts_parse_process_template(rc, value, &(cd->suspend_delayms_early_to_before),
		__func__, "suspend_delayms_early_to_before");

	rc = of_property_read_u32(thp_node, "suspend_delayms_before_to_later", &value);
	thp_dts_parse_process_template(rc, value, &(cd->suspend_delayms_before_to_later),
		__func__, "suspend_delayms_before_to_later");

	rc = of_property_read_u32(thp_node, "resume_delayms_early_to_later", &value);
	thp_dts_parse_process_template(rc, value, &(cd->resume_delayms_early_to_later),
		__func__, "resume_delayms_early_to_later");

	rc = of_property_read_u32(thp_node, "need_check_panel_from_aod", &value);
	thp_dts_parse_process_template(rc, value, &(cd->need_check_panel_from_aod),
		__func__, "need_check_panel_from_aod");

	rc = of_property_read_u32(thp_node, "support_gesture_mode", &value);
	if (!rc) {
		cd->support_gesture_mode = value;
		thp_log_info("%s:support_gesture_mode configed %u\n",
			__func__, value);
		rc = of_property_read_u32(thp_node, "gesture_from_sensorhub", &value);
		thp_dts_parse_process_template(rc, value, &(cd->gesture_from_sensorhub),
			__func__, "gesture_from_sensorhub");
	}

	rc = of_property_read_u32(thp_node, "aod_display_support", &value);
	thp_dts_parse_process_template(rc, value, &(cd->aod_display_support),
		__func__, "aod_display_support");

	rc = of_property_read_u32(thp_node, "tsa_event_to_udfp", &value);
	thp_dts_parse_process_template(rc, value, &(cd->tsa_event_to_udfp),
		__func__, "tsa_event_to_udfp");

	rc = of_property_read_u32(thp_node, "lcd_gesture_mode_support", &value);
	thp_dts_parse_process_template(rc, value, &(cd->lcd_gesture_mode_support),
		__func__, "lcd_gesture_mode_support");

	/*
	 * TDDI(TP powered by LCD) download fw in afe screen on,
	 * need wait interruptible to make sure of screen on done.
	 */
	rc = of_property_read_u32(thp_node, "wait_afe_screen_on_support", &value);
	thp_dts_parse_process_template(rc, value, &(cd->wait_afe_screen_on_support),
		__func__, "wait_afe_screen_on_support");

	rc = of_property_read_u32(thp_node, "need_check_irq_status", &value);
	thp_dts_parse_process_template(rc, value, &(cd->need_check_irq_status),
		__func__, "need_check_irq_status");

#if defined(CONFIG_TEE_TUI)
	rc = of_property_read_u32(thp_node, "send_tui_exit_in_suspend", &value);
	thp_dts_parse_process_template(rc, value, &(cd->send_tui_exit_in_suspend),
		__func__, "send_tui_exit_in_suspend");
	rc = of_property_read_u32(thp_node, "no_need_secos_bus_init", &value);
	thp_dts_parse_process_template(rc, value, &(cd->no_need_secos_bus_init),
		__func__, "no_need_secos_bus_init");
#endif

	rc = of_property_read_u32(thp_node, "support_ring_feature", &value);
	if (!rc) {
		cd->support_ring_feature = value;
		/* open or close ring switch by user, this is initial value */
		cd->ring_setting_switch = value;
		thp_log_info("%s:support_ring_feature configed %u\n",
			__func__, value);
	}

	rc = of_property_read_u32(thp_node, "support_ring_setting_switch",
		&value);
	thp_dts_parse_process_template(rc, value, &(cd->support_ring_setting_switch),
		__func__, "support_ring_setting_switch");

	rc = of_property_read_u32(thp_node, "support_fingerprint_switch",
		&value);
	thp_dts_parse_process_template(rc, value, &(cd->support_fingerprint_switch),
		__func__, "support_fingerprint_switch");

	rc = of_property_read_u32(thp_node, "support_extra_key_event_input",
		&value);
	thp_dts_parse_process_template(rc, value, &(cd->support_extra_key_event_input),
		__func__, "support_extra_key_event_input");
	rc = of_property_read_u32(thp_node, "hide_product_info_en", &value);
	thp_dts_parse_process_template(rc, value, &(cd->hide_product_info_en),
		__func__, "hide_product_info_en");
	rc = of_property_read_u32(thp_node, "support_oem_info", &value);
	thp_dts_parse_process_template(rc, value, &(cd->support_oem_info),
		__func__, "support_oem_info");

	rc = of_property_read_u32(thp_node, "projectid_from_panel_ver", &value);
	thp_dts_parse_process_template(rc, value, &(cd->projectid_from_panel_ver),
		__func__, "projectid_from_panel_ver");

	cd->support_vendor_ic_type = 0;
	rc = of_property_read_u32(thp_node, "support_vendor_ic_type", &value);
	thp_dts_parse_process_template(rc, value, &(cd->support_vendor_ic_type),
		__func__, "support_vendor_ic_type");

	rc = of_property_read_u32(thp_node, "suspend_no_reset", &value);
	thp_dts_parse_process_template(rc, value, &(cd->suspend_no_reset),
		__func__, "suspend_no_reset");

	cd->edit_product_in_project_id = 0;
	rc = of_property_read_u32(thp_node, "edit_product_in_project_id", &value);
	thp_dts_parse_process_template(rc, value, &(cd->edit_product_in_project_id),
		__func__, "edit_product_in_project_id");
	if (cd->edit_product_in_project_id) {
		cd->product = NULL;
		rc = of_property_read_string(thp_node, "product",
			(const char **)&cd->product);
		if (!rc)
			thp_log_info("%s:product configed %s\n", __func__,
				cd->product);
#if (!defined CONFIG_HUAWEI_THP_MTK) && (!defined CONFIG_HUAWEI_THP_QCOM)
		cd->project_id_exception = "dummy";
		rc = of_property_read_string(thp_node, "project_id_exception",
			(const char **)&cd->project_id_exception);
		if (!rc)
			thp_log_info("%s:project_id_exception configed %s\n",
				__func__, cd->project_id_exception);
#endif
	}
	cd->need_resume_reset = 0;
	rc = of_property_read_u32(thp_node, "need_resume_reset", &value);
	thp_dts_parse_process_template(rc, value, &(cd->need_resume_reset),
		__func__, "need_resume_reset");
	cd->disable_irq_gesture_suspend = 0;
	rc = of_property_read_u32(thp_node, "disable_irq_gesture_suspend",
		&value);
	thp_dts_parse_process_template(rc, value, &(cd->disable_irq_gesture_suspend),
		__func__, "disable_irq_gesture_suspend");

	cd->support_dual_chip_arch = 0;
	rc = of_property_read_u32(thp_node, "support_dual_chip_arch", &value);
	thp_dts_parse_process_template(rc, value, &(cd->support_dual_chip_arch),
		__func__, "support_dual_chip_arch");

	thp_parse_project_id_map_config(thp_node, cd);

#if (defined(CONFIG_HUAWEI_THP_MTK) || defined(CONFIG_HUAWEI_THP_QCOM))
	value = 0;
	cd->change_spi_driving_force = 0;
	rc = of_property_read_u32(thp_node, "change_spi_driving_force", &value);
	thp_dts_parse_process_template(rc, value, &(cd->change_spi_driving_force),
		__func__, "change_spi_driving_force");
	cd->need_enable_irq_wake = 0;
	rc = of_property_read_u32(thp_node, "need_enable_irq_wake", &value);
	thp_dts_parse_process_template(rc, value, &(cd->need_enable_irq_wake),
		__func__, "need_enable_irq_wake");
#endif
	cd->gesture_retry_times = 20; /* defult gesture retry times:20 */
	rc = of_property_read_u32(thp_node, "gesture_retry_times", &value);
	thp_dts_parse_process_template(rc, value, &(cd->gesture_retry_times),
		__func__, "gesture_retry_times");
	cd->lcd_need_get_afe_status = 0;
	rc = of_property_read_u32(thp_node, "lcd_need_get_afe_status", &value);
	thp_dts_parse_process_template(rc, value, &(cd->lcd_need_get_afe_status),
		__func__, "lcd_need_get_afe_status");

	rc = of_property_read_u32(thp_node,
		"support_report_interval_adjustment", &value);
	if (!rc) {
		cd->support_interval_adjustment = value;
		rc = of_property_read_u32(thp_node, "time_interval", &value);
		if (!rc) {
			cd->time_interval = value;
			thp_log_info("%s:support_adjustment %u\n",
				__func__, cd->time_interval);
		} else {
			cd->support_interval_adjustment = 0;
		}
		rc = of_property_read_u32(thp_node,
			"time_min_interval", &value);
		if (!rc) {
			cd->time_min_interval = value;
			thp_log_info("%s:support_adjustment %u\n",
				__func__, cd->time_min_interval);
		} else {
			cd->support_interval_adjustment = 0;
		}
	}
	rc = of_property_read_u32(thp_node, "support_screen_switch", &value);
	thp_dts_parse_process_template(rc, value, &(cd->support_screen_switch),
		__func__, "support_screen_switch");

	rc = of_property_read_u32(thp_node, "need_check_first_screen_switch", &value);
	thp_dts_parse_process_template(rc, value, &(cd->need_check_first_screen_switch),
		__func__, "need_check_first_screen_switch");

	rc = of_property_read_u32(thp_node,
		"iovdd_power_on_delay_ms", &value);
	if (!rc) {
		cd->iovdd_power_on_delay_ms = value;
		thp_log_info("%s:iovdd_power_on_delay_ms %u\n",
			__func__, cd->iovdd_power_on_delay_ms);
	} else {
		cd->iovdd_power_on_delay_ms = 1; /* default delay 1 ms */
	}

	cd->change_vendor_name = 0;
	rc = of_property_read_u32(thp_node, "change_vendor_name",
		&value);
	thp_dts_parse_process_template(rc, value, &(cd->change_vendor_name),
		__func__, "change_vendor_name");

	cd->thp_compatible_tskit = 0;
	rc = of_property_read_u32(thp_node, "thp_compatible_tskit",
		&value);
	thp_dts_parse_process_template(rc, value, &(cd->thp_compatible_tskit),
		__func__, "thp_compatible_tskit");

	cd->check_tp_vendor = 0;
	rc = of_property_read_u32(thp_node, "check_tp_vendor", &value);
	thp_dts_parse_process_template(rc, value, &(cd->check_tp_vendor),
		__func__, "check_tp_vendor");

	rc = of_property_read_u32(thp_node, "support_polymeric_chip", &value);
	thp_dts_parse_process_template(rc, value, &(cd->support_polymeric_chip),
		__func__, "support_polymeric_chip");

	cd->support_projectid_from_cmdline = 0;
	rc = of_property_read_u32(thp_node, "support_projectid_from_cmdline", &value);
	thp_dts_parse_process_template(rc, value, &(cd->support_projectid_from_cmdline),
		__func__, "support_projectid_from_cmdline");

	cd->double_click_use_ap_gesture = 0;
	rc = of_property_read_u32(thp_node, "double_click_use_ap_gesture", &value);
	thp_dts_parse_process_template(rc, value, &(cd->double_click_use_ap_gesture),
		__func__, "double_click_use_ap_gesture");

	thp_parse_esd_reset_config(thp_node, cd);

	return 0;
}
EXPORT_SYMBOL(thp_parse_feature_config);

int is_pt_test_mode(struct thp_device *tdev)
{
	int thp_pt_station_flag = 0;

#if defined(CONFIG_LCD_KIT_DRIVER)
#ifndef CONFIG_MODULE_KO_TP
	int ret;
	struct lcd_kit_ops *lcd_ops = lcd_kit_get_ops();

	if ((lcd_ops) && (lcd_ops->get_status_by_type)) {
		ret = lcd_ops->get_status_by_type(PT_STATION_TYPE,
			&thp_pt_station_flag);
		if (ret < 0) {
			thp_log_info("%s: get thp_pt_station_flag fail\n",
				__func__);
			return ret;
		}
	}
#endif
#else
	thp_pt_station_flag = g_tskit_pt_station_flag &&
			tdev->test_config.pt_station_test;
#endif

	thp_log_info("%s thp_pt_station_flag = %d\n", __func__,
		thp_pt_station_flag);

	return thp_pt_station_flag;
}
EXPORT_SYMBOL(is_pt_test_mode);

int is_tp_detected(void)
{
	int ret = TP_DETECT_SUCC;
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);

	if (!cd) {
		thp_log_err("%s: thp_core_data is not inited\n", __func__);
		return TP_DETECT_FAIL;
	}
	if (!atomic_read(&cd->register_flag))
		ret = TP_DETECT_FAIL;

	thp_log_info(
		"[Proximity_feature] %s : Check if tp is in place, ret = %d\n",
		__func__, ret);
	return ret;
}

/*
 * Here to count the period of time which is from suspend to a new
 * disable status, if the period is less than 1000ms then call lcdkit
 * power off, otherwise bypass the additional power off.
 */
static bool thp_prox_timeout_check(struct thp_core_data *cd)
{
	long delta_time;
	struct timespec64 tv;

	memset(&tv, 0, sizeof(tv));
	do_timetransfer(&tv);
	thp_log_info(
		"[Proximity_feature] check time at %ld seconds %ld us\n",
		tv.tv_sec, tv.tv_nsec);
	/* multiply 1000000 to transfor second to us */
	delta_time = (tv.tv_sec - cd->tp_suspend_record_tv.tv_sec) * 1000000 +
		(tv.tv_nsec - cd->tp_suspend_record_tv.tv_nsec);
	/* divide 1000 to transfor sec to us to ms */
	delta_time /= 1000;
	thp_log_info("[Proximity_feature] delta_time = %ld ms\n", delta_time);
	if (delta_time >= AFTER_SUSPEND_TIME)
		return true;
	else
		return false;
}

/*
 * After lcd driver complete the additional power down,calling this function
 * do something for matching current power status. Such as updating the
 * proximity switch status, sending the screen_off cmd to tp daemon, pulling
 * down the gpios and so on.
 */
#ifndef CONFIG_MODULE_KO_TP
static void thp_prox_add_suspend(struct thp_core_data *cd, bool enable)
{
	thp_log_info("[Proximity_feature] %s call enter\n", __func__);
	/* update the control status based on proximity switch */
	cd->need_work_in_suspend = enable;
	/* notify daemon to do screen off cmd */
	thp_set_status(THP_STATUS_POWER, THP_SUSPEND, cd->panel_id);
	/* notify daemon to do proximity off cmd */
	thp_set_status(THP_STATUS_AFE_PROXIMITY, THP_PROX_EXIT, cd->panel_id);
	/* pull down the gpio pin */
#ifndef CONFIG_HUAWEI_THP_MTK
	gpio_set_value(cd->thp_dev->gpios->rst_gpio, 0);
	if (!cd->not_support_cs_control)
		gpio_set_value(cd->thp_dev->gpios->cs_gpio, 0);
#else
	if (cd->reset_status_in_suspend_mode)
		thp_log_info("[Proximity_feature] %s second power down\n",
			__func__);
	else
		pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.reset_low);
	pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.cs_low);
#endif
	/* disable the irq */
	if (cd->open_count)
		thp_set_irq_status(cd, THP_IRQ_DISABLE);
	cd->work_status = SUSPEND_DONE;
	/* clean the fingers */
	thp_clean_fingers(cd->panel_id);
	thp_log_info("[Proximity_feature] %s call exit\n", __func__);
}
#endif
/*
 * In this function, differentiating lcdkit1.0 and lcdkit 3.0's interfaces,
 * and increasing some judgements, only meet these conditions then
 * the additional power off will be called.
 */
static void thp_prox_add_poweroff(struct thp_core_data *cd, bool enable)
{
#ifndef CONFIG_MODULE_KO_TP
#ifdef CONFIG_LCD_KIT_DRIVER
	struct lcd_kit_ops *tp_ops = lcd_kit_get_ops();
#endif
#endif
	if ((enable == false) && (onetime_poweroff_done == false)) {
		onetime_poweroff_done = true;
		if (thp_prox_timeout_check(cd)) {
			thp_log_info(
				"[Proximity_feature] timeout, bypass poweroff\n");
			return;
		}
#ifdef CONFIG_LCDKIT_DRIVER
		if (!lcdkit_proximity_poweroff())
			thp_prox_add_suspend(cd, enable);
#endif

#ifndef CONFIG_MODULE_KO_TP
#ifdef CONFIG_LCD_KIT_DRIVER
		if (tp_ops && tp_ops->proximity_power_off) {
			if (!tp_ops->proximity_power_off())
				thp_prox_add_suspend(cd, enable);
		} else {
			thp_log_err("[Proximity_feature] point is null\n");
		}
#endif
#endif
	}
}

/*
 * This function receive the proximity switch status and save it for
 *  controlling power operation or cmds transferring to daemon
 * (proximity_on or scrren_off).
 */
int thp_set_prox_switch_status(bool enable)
{
#if ((defined CONFIG_INPUTHUB_20) || (defined CONFIG_HUAWEI_PS_SENSOR) || \
	(defined CONFIG_HUAWEI_SENSORS_2_0))
	int ret;
	int report_value[PROX_VALUE_LEN] = {0};
#endif
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);

	if (!cd) {
		thp_log_err("%s: thp_core_data is not inited\n", __func__);
		return 0;
	}
	if (!atomic_read(&cd->register_flag))
		return 0;

	if (cd->proximity_support == PROX_SUPPORT) {
#if ((defined CONFIG_INPUTHUB_20) || (defined CONFIG_HUAWEI_PS_SENSOR) || \
	(defined CONFIG_HUAWEI_SENSORS_2_0))
		report_value[0] = AWAY_EVENT_VALUE;
		ret = thp_prox_event_report(report_value, PROX_EVENT_LEN);
		if (ret < 0)
			thp_log_info("%s: report event fail\n", __func__);
		thp_log_info(
			"[Proximity_feature] %s: default report [far] event!\n",
			__func__);
#endif
		thp_set_status(THP_STATUS_TOUCH_APPROACH, enable, cd->panel_id);
		if (cd->early_suspended == false) {
			g_thp_prox_enable = enable;
			thp_log_info(
				"[Proximity_feature] %s: 1.Update g_thp_prox_enable to %d in screen on!\n",
				__func__, g_thp_prox_enable);
		} else {
			cd->prox_cache_enable = enable;
			thp_log_info(
				"[Proximity_feature] %s: 2.Update prox_cache_enable to %d in screen off!\n",
				__func__, cd->prox_cache_enable);
			/*
			 * When disable proximity after suspend,
			 * call power off once.
			 */
			thp_prox_add_poweroff(cd, enable);
		}
		return 0;
	}
	thp_log_info(
		"[Proximity_feature] %s : Not support proximity feature!\n",
		__func__);
	return 0;
}

/*
 * This function is supplied for lcd driver to get the current proximity status
 * when lcdkit go to power off, and use this status to contorl power.
 */
bool thp_get_prox_switch_status(void)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);

	if (!cd) {
		thp_log_err("%s: thp_core_data is not inited\n", __func__);
		return 0;
	}
	if (cd->proximity_support == PROX_SUPPORT) {
		thp_log_info(
			"[Proximity_feature] %s: need_work_in_suspend = %d!\n",
			__func__, cd->need_work_in_suspend);
		return cd->need_work_in_suspend;
	}
	thp_log_info("[Proximity_feature] %s :Not support proximity feature!\n",
		__func__);
	return 0;
}

#ifndef CONFIG_HUAWEI_THP_MTK
static int thp_parse_gpio_config(struct device_node *thp_node,
	struct thp_core_data *cd)
{
	unsigned int value;

	value = of_get_named_gpio(thp_node, "irq_gpio", 0);
	thp_log_info("irq gpio_ = %d\n", value);
	if (!gpio_is_valid(value)) {
		thp_log_err("%s: get irq_gpio failed\n", __func__);
		return -EINVAL;;
	}
	cd->gpios.irq_gpio = value;

	value = of_get_named_gpio(thp_node, "rst_gpio", 0);
	thp_log_info("rst_gpio = %d\n", value);
	if (!gpio_is_valid(value)) {
		thp_log_err("%s: get rst_gpio failed\n", __func__);
		return -EINVAL;;
	}
	cd->gpios.rst_gpio = value;
	if (!cd->not_support_cs_control) {
		value = of_get_named_gpio(thp_node, "cs_gpio", 0);
		thp_log_info("cs_gpio = %d\n", value);
		if (!gpio_is_valid(value)) {
			thp_log_err("%s: get cs_gpio failed\n", __func__);
			return -EINVAL;;
		}
		cd->gpios.cs_gpio = value;
	}
	return 0;
}
#endif

static int thp_parse_config(struct thp_core_data *cd,
	struct device_node *thp_node)
{
	int rc;
	unsigned int value;

	if (!thp_node) {
		thp_log_err("%s:thp not config in dts, exit\n", __func__);
		return -ENODEV;
	}

	rc = thp_parse_spi_config(thp_node, cd);
	if (rc) {
		thp_log_err("%s: spi config parse fail\n", __func__);
		return rc;
	}

	cd->power_cfg.power_supply_mode = 0;
	rc = of_property_read_u32(thp_node, "power_supply_mode", &value);
	if (!rc) {
		cd->power_cfg.power_supply_mode = value;
		thp_log_info("%s:parsed success, power_supply_mode = %u\n",
			__func__, cd->power_cfg.power_supply_mode);
	} else {
		thp_log_info("%s:parsed failed, power_supply_mode = %u\n",
			__func__, cd->power_cfg.power_supply_mode);
	}

	if (cd->power_cfg.power_supply_mode) {
		cd->power_cfg.power_sequence = 0;
		rc = of_property_read_u32(thp_node, "power_sequence", &value);
		if (!rc) {
			cd->power_cfg.power_sequence = value;
			thp_log_info("%s:parsed success, power_sequence = %u\n",
				__func__, cd->power_cfg.power_sequence);
		} else {
			thp_log_info("%s:parsed success, power_sequence = %u\n",
				__func__, cd->power_cfg.power_sequence);
		}
	}

	rc = thp_parse_power_config(thp_node, cd);
	if (rc) {
		thp_log_err("%s: power config parse fail\n", __func__);
		return rc;
	}

	rc = thp_parse_pinctrl_config(thp_node, cd);
	if (rc) {
		thp_log_err("%s:pinctrl parse fail\n", __func__);
		return rc;
	}

	cd->irq_flag = IRQF_TRIGGER_FALLING;
	rc = of_property_read_u32(thp_node, "irq_flag", &value);
	if (!rc) {
		cd->irq_flag = value;
		thp_log_info("%s:irq_flag parsed\n", __func__);
	}
	cd->use_mdelay = 0;
	if (!of_property_read_u32(thp_node, "use_mdelay", &value)) {
		cd->use_mdelay = value;
		thp_log_info("%s:use_mdelay parsed:%u\n",
			__func__, cd->use_mdelay);
	}
	cd->proximity_support = PROX_NOT_SUPPORT;
	rc = of_property_read_u32(thp_node, "proximity_support", &value);
	if (!rc) {
		cd->proximity_support = value;
		thp_log_info("%s:parsed success, proximity_support = %u\n",
			__func__, cd->proximity_support);
	} else {
		thp_log_info("%s:parsed failed, proximity_support = %u\n",
			__func__, cd->proximity_support);
	}

	cd->platform_type = THP_PLATFORM_HISI;
	rc = of_property_read_u32(thp_node, "platform_type", &value);
	if (!rc) {
		cd->platform_type = value;
		thp_log_info("%s:parsed success, platform_type = %u\n",
			__func__, cd->platform_type);
	} else {
		thp_log_info("%s:parsed failed, platform_type = %u\n",
			__func__, cd->platform_type);
	}

#ifndef CONFIG_HUAWEI_THP_MTK
	rc = thp_parse_gpio_config(thp_node, cd);
	if (rc) {
		thp_log_info("%s:gpio config parsed failed\n",
			__func__);
		return rc;
	}
#endif
	thp_parse_feature_config(thp_node, cd);
	thp_parse_extra_feature_config(thp_node, cd);
	thp_parse_pen_feature_config(thp_node, cd);
	cd->thp_node = thp_node;
	return 0;
}

u32 get_available_thread_id(void)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
	u32 thread_id;

	for (thread_id = 0; thread_id <= cd->dual_thp; thread_id++) {
		if (thp_get_core_data(thread_id)->thp_dev) {
			thp_log_info("Use thread %u\n", thread_id);
			return thread_id;
		}
	}
	return 0;
}

static int thp_cmd_sync_allocate(struct thp_cmd_node *cmd, int timeout)
{
	struct thp_cmd_sync *sync = NULL;

	if (timeout == 0) {
		cmd->sync = NULL;
		return 0;
	}
	sync = kzalloc(sizeof(*sync), GFP_KERNEL);
	if (sync == NULL) {
		thp_log_err("failed to kzalloc completion\n");
		return -ENOMEM;
	}
	init_completion(&sync->done);
	atomic_set(&sync->timeout_flag, TS_NOT_TIMEOUT);
	cmd->sync = sync;
	return 0;
}

int thp_put_one_cmd(struct thp_cmd_node *cmd, int timeout)
{
	int error = -EIO;
	unsigned long flags;
	struct thp_cmd_queue *q = NULL;
	struct thp_core_data *cd = NULL;

	if (cmd == NULL) {
		thp_log_err("%s:null pointer\n", __func__);
		goto out;
	}
	cd = thp_get_core_data(cmd->cmd_param.thread_id);
	if ((!atomic_read(&cd->register_flag)) &&
		(cmd->command != TS_CHIP_DETECT)) {
		thp_log_err("%s: not initialize\n", __func__);
		goto out;
	}
	if (thp_cmd_sync_allocate(cmd, timeout)) {
		thp_log_debug("%s:allocate success\n", __func__);
		/*
		 * When the command execution timeout the memory occupied
		 * by sync will be released  in the command execution module,
		 * else the memory will be released after waiting for the
		 * command return normally.
		 */
		goto out;
	}
	q = &cd->queue;
	spin_lock_irqsave(&q->spin_lock, flags);
	smp_wmb(); /* Make sure queue has assigned correctly */
	if (q->cmd_count == q->queue_size) {
		spin_unlock_irqrestore(&q->spin_lock, flags);
		thp_log_err("%s:queue is full\n", __func__);
		WARN_ON(1);
		error = -EIO;
		goto free_sync;
	}
	memcpy(&q->ring_buff[q->wr_index], cmd, sizeof(*cmd));
	q->cmd_count++;
	q->wr_index++;
	q->wr_index %= q->queue_size;
	smp_mb(); /* Make sure queue is refreshed correctly */
	spin_unlock_irqrestore(&q->spin_lock, flags);
	thp_log_debug("%s:%d in ring buff\n", __func__, cmd->command);
	error = NO_ERR;
	wake_up_process(cd->thp_task); /* wakeup thp process */
	if (timeout && (cmd->sync != NULL) &&
		!(wait_for_completion_timeout(&cmd->sync->done,
			abs(timeout) * HZ))) {
		atomic_set(&cmd->sync->timeout_flag, TS_TIMEOUT);
		thp_log_err("%s:wait cmd respone timeout\n", __func__);
		error = -EBUSY;
		goto out;
	}
	smp_wmb(); /* Make sure code has been completed before function ends */
free_sync:
	kfree(cmd->sync);
	cmd->sync = NULL;
out:
	return error;
}

static int get_one_cmd(struct thp_cmd_node *cmd, u32 panel_id)
{
	unsigned long flags;
	int error = -EIO;
	struct thp_cmd_queue *q = NULL;
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	if (unlikely(!cmd)) {
		thp_log_err("%s:find null pointer\n", __func__);
		goto out;
	}

	q = &cd->queue;
	spin_lock_irqsave(&q->spin_lock, flags);
	smp_wmb(); /* Make sure queue has assigned correctly */
	if (!q->cmd_count) {
		thp_log_debug("%s: queue is empty\n", __func__);
		spin_unlock_irqrestore(&q->spin_lock, flags);
		goto out;
	}
	memcpy(cmd, &q->ring_buff[q->rd_index], sizeof(*cmd));
	q->cmd_count--;
	q->rd_index++;
	q->rd_index %= q->queue_size;
	smp_mb(); /* Make sure queue is refreshed correctly */
	spin_unlock_irqrestore(&q->spin_lock, flags);
	thp_log_debug("%s:%d from ring buff\n", __func__,
		cmd->command);
	error = NO_ERR;
out:
	return error;
}

static bool thp_task_continue(u32 panel_id)
{
	bool task_continue = true;
	unsigned long flags;
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	thp_log_debug("%s:prepare enter idle\n", __func__);
	while (task_continue) {
		if (unlikely(kthread_should_stop())) {
			task_continue = false;
			goto out;
		}
		spin_lock_irqsave(&cd->queue.spin_lock, flags);
		/*
		 * Make sure the memory and assignment are completed
		 * before updating the current process.
		 */
		smp_wmb();
		if (cd->queue.cmd_count) {
			set_current_state(TASK_RUNNING);
			thp_log_debug("%s:TASK_RUNNING\n", __func__);
			goto out_unlock;
		} else {
			set_current_state(TASK_INTERRUPTIBLE);
			thp_log_debug("%s:TASK_INTERRUPTIBLE\n", __func__);
			spin_unlock_irqrestore(&cd->queue.spin_lock, flags);
			schedule();
		}
	}

out_unlock:
	spin_unlock_irqrestore(&cd->queue.spin_lock, flags);
out:
	return task_continue;
}

static int thp_proc_command(struct thp_cmd_node *cmd)
{
	int error = NO_ERR;
	struct thp_cmd_sync *sync = NULL;
	struct thp_core_data *cd = thp_get_core_data(cmd->cmd_param.thread_id);
	struct thp_cmd_node *proc_cmd = cmd;
	struct thp_cmd_node *out_cmd = &cd->pang_cmd_buff;
	u32 panel_id = cmd->cmd_param.panel_id;

	if (!cmd) {
		thp_log_err("%s:invalid cmd\n", __func__);
		goto out;
	}
	sync = cmd->sync;
	/* discard timeout cmd */
	if (sync && (atomic_read(&sync->timeout_flag) == TS_TIMEOUT)) {
		kfree(sync);
		goto out;
	}
	while (true) {
		out_cmd->command = TS_INVAILD_CMD;
		switch (proc_cmd->command) {
		case TS_CHIP_DETECT:
			thp_chip_detect(proc_cmd);
			break;
#if defined CONFIG_LCD_KIT_DRIVER
		case TS_SUSPEND:
			thp_suspend_thread(cmd->cmd_param.panel_id);
			break;
		case TS_RESUME:
			thp_resume_thread(cmd->cmd_param.panel_id);
			break;
		case THP_MUTIL_RESUME_THREAD:
			cd = thp_get_core_data(panel_id);
			if (cd->multi_screen_fold == true)
				break;
			thp_multi_resume(panel_id);
			break;
		case THP_MUTIL_SUSPEND_THREAD:
			cd = thp_get_core_data(panel_id);
			if (cd->multi_screen_fold == true && cd->dual_thp)
				break;
			thp_multi_suspend(panel_id);
			break;
		case TSKIT_MUTIL_RESUME_THREAD:
			tskit_multi_resume();
			break;
		case TSKIT_MUTIL_SUSPEND_THREAD:
			tskit_multi_suspend();
			break;
		case TS_SCREEN_FOLD:
			error = multi_screen_status_switch(SCREEN_FOLDED);
			break;
		case TS_SCREEN_OFF_FOLD:
			error = multi_screen_status_switch(SCREEN_OFF_FOLD);
			break;
		case TS_SCREEN_UNFOLD:
			error = multi_screen_status_switch(SCREEN_UNFOLD);
			break;
#endif
		default:
			break;
		}

		thp_log_debug("%s:command :%d process result:%d\n",
			__func__, proc_cmd->command, error);
		if (out_cmd->command != TS_INVAILD_CMD) {
			thp_log_debug("%s:related command :%d  need process\n",
				__func__, out_cmd->command);
			/* ping - pang */
			swap(proc_cmd, out_cmd);
		} else {
			break;
		}
	}
	/* notify wait threads by completion */
	if (sync) {
		smp_mb(); /* Make sure current timeout_flag is up to date */
		thp_log_debug("%s:wakeup threads in waitqueue\n", __func__);
		if (atomic_read(&sync->timeout_flag) == TS_TIMEOUT)
			kfree(sync);
		else
			complete(&sync->done);
	}

out:
	return error;
}

static int thp_thread(void *p)
{
	struct thp_core_data *cd = (struct thp_core_data *)p;
	static const struct sched_param param = {
		/* The priority of thread scheduling is 99 */
		.sched_priority = 99,
	};
	/*
	 * Make sure buff is properly refreshed
	 * before the process is executed.
	 */
	smp_wmb();
	thp_log_info("%s: in\n", __func__);
	memset(&cd->ping_cmd_buff, 0, sizeof(cd->ping_cmd_buff));
	memset(&cd->pang_cmd_buff, 0, sizeof(cd->pang_cmd_buff));
	/*
	 * Make sure buff is properly refreshed
	 * before the process is executed.
	 */
	smp_mb();
	sched_setscheduler(current, SCHED_RR, &param);
	while (thp_task_continue(cd->panel_id)) {
		/* get one command */
		while (!get_one_cmd(&cd->ping_cmd_buff, cd->panel_id)) {
			thp_proc_command(&cd->ping_cmd_buff);
			memset(&cd->ping_cmd_buff, 0, sizeof(cd->ping_cmd_buff));
			memset(&cd->pang_cmd_buff, 0, sizeof(cd->pang_cmd_buff));
		}
	}
	thp_log_err("%s: stop\n", __func__);
	return NO_ERR;
}

static int thp_thread_init(struct thp_core_data *cd, u32 panel_id)
{
	cd->thp_task = kthread_create(thp_thread, cd, "ts_thread:%u", panel_id);
	if (IS_ERR(cd->thp_task)) {
		thp_log_err("%s: creat thread failed\n", __func__);
		return -ENODEV;
	}
	cd->queue.rd_index = 0;
	cd->queue.wr_index = 0;
	cd->queue.cmd_count = 0;
	cd->queue.queue_size = THP_CMD_QUEUE_SIZE;
	spin_lock_init(&cd->queue.spin_lock);
	smp_mb(); /* Make sure queue is initialized before wake up the task */
	wake_up_process(cd->thp_task);
	return 0;
}

#ifdef CONFIG_LCDKIT_DRIVER
static int thp_check_tp_driver_type(void)
{
	unsigned int driver_type;

	driver_type = lcdkit_get_tp_driver_type();
	thp_log_info("%s:enter driver_type = %u\n", __func__, driver_type);
	if (driver_type == THP)
		return 0;
	else
		return TP_DRIVER_TYPE_MATCH_FAIL;
}
#endif

static u32 thp_get_panel_id(struct device_node *thp_node)
{
	int rc;
	u32 value = 0;

	rc = of_property_read_u32(thp_node, "thp_panel_id", &value);
	if (!rc)
		thp_log_info("%s: panel_id = %u", __func__, value);
	else
		thp_log_info("%s: use default panel_id %u", __func__, value);
	return value;
}

static int thp_probe(struct spi_device *sdev)
{
	struct thp_core_data *thp_core = NULL;
	u8 *frame_read_buf = NULL;
	u32 panel_id;
	int rc;

	thp_log_info("%s: in\n", __func__);
	panel_id = thp_get_panel_id(sdev->dev.of_node);
	thp_core = thp_get_core_data(panel_id);
	if (!thp_core) {
		thp_log_err("%s: thp core data is null ptr\n", __func__);
		return -ENOMEM;
	}
#ifndef CONFIG_MODULE_KO_TP
#if (defined(CONFIG_LCD_KIT_DRIVER) && defined(CONFIG_HUAWEI_THP_QCOM))
	if (!lcd_kit_get_ops()) {
		thp_log_info("%s: lcdkit not init, need wait\n", __func__);
		return -EPROBE_DEFER;
	}
#endif
#endif

	frame_read_buf = kzalloc(THP_MAX_FRAME_SIZE, GFP_KERNEL);
	if (!frame_read_buf) {
		thp_log_err("%s: frame_read_buf out of memory\n", __func__);
		return -ENOMEM;
	}

	thp_core->frame_read_buf = frame_read_buf;
	thp_core->sdev = sdev;
	rc = thp_parse_config(thp_core, sdev->dev.of_node);
	if (rc) {
		thp_log_err("%s: parse dts fail\n", __func__);
		kfree(thp_core->frame_read_buf);
		return -EPROBE_DEFER;
	}
#ifdef CONFIG_LCDKIT_DRIVER
	if (thp_core->thp_compatible_tskit) {
		rc = thp_check_tp_driver_type();
		if (rc) {
			thp_log_err("%s: tp driver type is not match\n", __func__);
			kfree(thp_core->frame_read_buf);
			kfree(thp_core);
			return rc;
		}
	}
#endif
	if (thp_core->use_thp_queue) {
		thp_core->thp_queue = thp_queue_init(thp_core);
		if (thp_core->thp_queue == NULL) {
			thp_log_err("%s: kzalloc frame fail\n", __func__);
			kfree(thp_core->frame_read_buf);
			thp_core->frame_read_buf = NULL;
			return -ENOMEM;
		}
	}

	rc = thp_init_chip_info(thp_core);
	if (rc)
		thp_log_err("%s: chip info init fail\n", __func__);

	mutex_init(&thp_core->spi_mutex);
	thp_log_info("%s:use_hwlock = %d\n", __func__, thp_core->use_hwlock);
	if (thp_core->use_hwlock) {
		thp_core->hwspin_lock = hwspin_lock_request_specific(
			TP_HWSPIN_LOCK_CODE);
		if (!thp_core->hwspin_lock)
			thp_log_err("%s: hwspin_lock request failed\n",
				__func__);
	}

	atomic_set(&thp_core->register_flag, 0);
	INIT_LIST_HEAD(&thp_core->frame_list.list);
	init_waitqueue_head(&(thp_core->frame_waitq));
	init_waitqueue_head(&(thp_core->thp_ta_waitq));
	init_waitqueue_head(&(thp_core->thp_event_waitq));
	init_waitqueue_head(&(thp_core->suspend_resume_waitq));
	init_waitqueue_head(&(thp_core->screen_waitq));
	init_waitqueue_head(&(thp_core->shb_waitq));
	init_waitqueue_head(&(thp_core->esd_para.esd_waitq));
	thp_core->event_flag = false;
	thp_core->suspend_resume_waitq_flag = WAITQ_WAKEUP;
	thp_core->shb_waitq_flag = WAITQ_WAKEUP;
	thp_core->esd_para.esd_waitq_flag = WAITQ_WAKEUP;
	if (thp_core->wait_afe_screen_on_support) {
		init_waitqueue_head(&(thp_core->afe_screen_on_waitq));
		atomic_set(&(thp_core->afe_screen_on_waitq_flag), WAITQ_WAKEUP);
		thp_log_info("%s, init afe_screen_on_waitq done\n", __func__);
	}

	spi_set_drvdata(sdev, thp_core);
	if (thp_core->use_fb_notify_to_sr)
		fb_register_client(&thp_fb_notifier_block);

#if IS_ENABLED(CONFIG_HUAWEI_THP_PEN_CLK)
	/* dual thp probe twice, need add lock for the same dev register */
	mutex_lock(&penclk_mutex);
	rc = pen_clk_init();
	mutex_unlock(&penclk_mutex);
	thp_log_info("%s:pen_clk_init: rc = %d\n", __func__, rc);
#endif

	complete(&thp_core->boot_sync.thp_init_done);
	thp_log_info("%s:out, rc = %d\n", __func__, rc);

	return 0;
}

static int thp_remove(struct spi_device *sdev)
{
	struct thp_core_data *cd = spi_get_drvdata(sdev);
#if defined(CONFIG_LCD_KIT_DRIVER)
	int rc;
#endif
	thp_log_info("%s: in\n", __func__);

	if (cd->use_fb_notify_to_sr)
		fb_unregister_client(&thp_fb_notifier_block);

	if (cd->use_thp_queue) {
		mutex_lock(&cd->mutex_frame);
		thp_queue_free(cd->thp_queue);
		mutex_unlock(&cd->mutex_frame);
	}

	if (atomic_read(&cd->register_flag)) {
		thp_sysfs_release(cd);

#if ((defined THP_CHARGER_FB) && (!defined CONFIG_HUAWEI_THP_QCOM) && \
	(!defined CONFIG_HUAWEI_THP_MTK))
		if (cd->charger_detect_notify.notifier_call)
			chip_charger_type_notifier_unregister(
					&cd->charger_detect_notify);
#endif
#if ((defined THP_CHARGER_FB) && ((defined CONFIG_HUAWEI_THP_MTK) || \
	(defined CONFIG_HUAWEI_THP_QCOM)))
	if (cd->charger_detect_notify.notifier_call && cd->supported_charger) {
		rc = power_event_bnc_unregister(POWER_BNT_CONNECT,
			&cd->charger_detect_notify);
		if (rc)
			thp_log_err("%s:power_event_nc_unregister fail\n",
				__func__);
	}
#endif
#if ((!defined CONFIG_LCD_KIT_DRIVER) && (!defined CONFIG_HUAWEI_THP_UDP))
		lcdkit_unregister_notifier(&cd->lcd_notify);
#endif

#if defined(CONFIG_LCD_KIT_DRIVER)
	rc = ts_kit_ops_unregister(&thp_ops);
	if (rc)
		thp_log_info("%s:ts_kit_ops_register fail\n", __func__);
#endif

		misc_deregister(&cd->thp_misc_device);
		mutex_destroy(&cd->mutex_frame);
		thp_mt_wrapper_exit(cd->panel_id);
	}
#if IS_ENABLED(CONFIG_HUAWEI_THP_PEN_CLK)
	pen_clk_exit();
#endif
	kfree(cd->frame_read_buf);
	kfree(cd->spi_sync_rx_buf);
	cd->spi_sync_rx_buf = NULL;

	kfree(cd->spi_sync_tx_buf);
	cd->spi_sync_tx_buf = NULL;

	kfree(cd);
	cd = NULL;

#if defined(CONFIG_TEE_TUI)
	unregister_tui_driver("tp");
#endif
	return 0;
}

char *node_name[] = {
	"huawei,thp",
	"huawei,thp_sub_screen",
};

struct device_node *get_thp_node(u32 panel_id)
{
	struct device_node *thp_node = NULL;
	struct device_node *parent = NULL;
	char *status = NULL;
	int rc;

	do {
		thp_node = of_find_compatible_node(thp_node, NULL, node_name[panel_id]);
		if (!thp_node) {
			thp_log_info("can't find thp node\n");
			return NULL;
		}
		parent = of_get_parent(thp_node);
		rc = of_property_read_string(parent, "status", (const char **)&status);
		if (!rc && !strcmp(status, "ok"))
			break;
		thp_log_info("%s: find next thp node\n", __func__);
	} while (thp_node);
	return thp_node;
}
EXPORT_SYMBOL(get_thp_node);

static int thp_parse_module_init_dts(struct thp_core_data *cd, u32 panel_id)
{
	struct device_node *thp_node = NULL;
	u32 value;
	int rc;

	thp_node = get_thp_node(panel_id);
	if (!thp_node) {
		thp_log_info("%s: not support %s\n", __func__, node_name[panel_id]);
		return -EINVAL;
	}

	cd->fast_booting_solution = 0;
	rc = of_property_read_u32(thp_node, "fast_booting_solution", &value);
	if (!rc) {
		cd->fast_booting_solution = value;
		thp_log_info("%s:fast_booting_solution parsed:%d\n",
			__func__, cd->fast_booting_solution);
	}
	cd->dual_thp = 0;
	rc = of_property_read_u32(thp_node, "dual_thp", &value);
	if (!rc) {
		cd->dual_thp = value;
		thp_log_info("%s:dual thp parsed:%d\n",
			__func__, cd->dual_thp);
	}
	cd->fold_scheme = 2;
	rc = of_property_read_u32(thp_node, "fold_scheme", &value);
	if (!rc) {
		cd->fold_scheme = value;
		thp_log_info("%s:fold_scheme parsed:%d\n", __func__, cd->fold_scheme);
	}
	cd->panel_id = 0;
	rc = of_property_read_u32(thp_node, "thp_panel_id", &value);
	if (!rc) {
		cd->panel_id = value;
		thp_log_info("%s:panel_id parsed:%d\n",
			__func__, cd->panel_id);
	}
	cd->apply_spi_memory_in_dma = 0;
	rc = of_property_read_u32(thp_node, "apply_spi_memory_in_dma", &value);
	if (!rc) {
		cd->apply_spi_memory_in_dma = value;
		thp_log_info("%s:apply_spi_memory_in_dma parsed:%d\n",
			__func__, cd->apply_spi_memory_in_dma);
	}
	thp_get_lcd_product_type();
	return NO_ERR;
}

static const struct of_device_id g_thp_psoc_match_table[] = {
	{ .compatible = "huawei,thp", },
	{ },
};

static const struct of_device_id g_thp_psoc_match_table_sub[] = {
	{ .compatible = "huawei,thp_sub_screen", },
	{ },
};

static const struct spi_device_id g_thp_device_id[] = {
	{ THP_DEVICE_NAME, 0 },
	{ "huawei_thp_sub_screen", 0 },
	{ }
};
MODULE_DEVICE_TABLE(spi, g_thp_device_id);

static struct spi_driver g_thp_spi_driver[2] = {
	{
		.probe = thp_probe,
		.remove = thp_remove,
		.id_table = g_thp_device_id,
		.driver = {
			.name = THP_DEVICE_NAME,
			.owner = THIS_MODULE,
			.bus = &spi_bus_type,
			.of_match_table = g_thp_psoc_match_table,
		},
	},
	{
		.probe = thp_probe,
		.remove = thp_remove,
		.id_table = g_thp_device_id,
		.driver = {
			.name = "huawei_thp_sub_screen",
			.owner = THIS_MODULE,
			.bus = &spi_bus_type,
			.of_match_table = g_thp_psoc_match_table_sub,
		},
	}
};

static int thp_init(u32 panel_id)
{
	int rc;

	if (g_thp_core[panel_id]) {
		thp_log_info("%s: panel %d is already inited\n", __func__, panel_id);
		return NO_ERR;
	}
	g_thp_core[panel_id] = kzalloc(sizeof(struct thp_core_data), GFP_KERNEL);
	if (!g_thp_core[panel_id]) {
		thp_log_err("%s: thp_core out of memory\n", __func__);
		return -ENOMEM;
	}
	rc = thp_parse_module_init_dts(g_thp_core[panel_id], panel_id);
	if (rc) {
		rc = NO_ERR;
		goto err;
	}

	init_completion(&g_thp_core[panel_id]->boot_sync.thp_init_done);
	if (g_thp_core[panel_id]->fast_booting_solution) {
		thp_unregister_ic_num = 0;
		rc = thp_thread_init(g_thp_core[panel_id], panel_id);
		if (rc) {
			thp_log_err("%s: thp thread %d init fail, rc = %d\n", __func__, panel_id, rc);
			goto err;
		}
	}
	rc = spi_register_driver(&g_thp_spi_driver[panel_id]);
	if (rc != 0) {
		thp_log_info("%s: spi register driver fail, rc = %d\n", __func__, rc);
		goto err;
	}
	return rc;
err:
	kfree(g_thp_core[panel_id]);
	g_thp_core[panel_id] = NULL;
	return rc;
}

static int __init g_thp_module_init(void)
{
	int rc;
	u32 i;

	thp_log_info("%s: E\n", __func__);
	mutex_init(&thp_power_mutex);
	mutex_init(&polymeric_chip_mutex);
	mutex_init(&tui_mutex);
	mutex_init(&penclk_mutex);
	atomic_set(&count_vote_for_polymeric_chip, 0);
	for (i = 0; i < TOTAL_PANEL_NUM; i++) {
		rc = thp_init(i);
		if (rc) {
			thp_log_err("%s: panel %u fail\n", __func__, i);
			return rc;
		}
	}
	return rc;
}

static void __exit g_thp_module_exit(void)
{
	u32 i;
	u32 extra_thp_num = g_thp_core[0]->dual_thp;

	thp_log_info("%s: E\n", __func__);
	mutex_destroy(&thp_power_mutex);
	mutex_destroy(&tui_mutex);
	mutex_destroy(&penclk_mutex);
	for (i = 0; i <= extra_thp_num; i++)
		spi_unregister_driver(&g_thp_spi_driver[i]);
	thp_log_info("%s: X\n", __func__);
}

module_init(g_thp_module_init);
module_exit(g_thp_module_exit);

MODULE_AUTHOR("Huawei Device Company");
MODULE_DESCRIPTION("Huawei THP Driver");
MODULE_LICENSE("GPL");

