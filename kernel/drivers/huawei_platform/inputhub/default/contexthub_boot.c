/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2020. All rights reserved.
 * Description: contexthub boot source file
 * Author: huangjisong
 * Create: 2013-3-10
 */

#include "contexthub_boot.h"
#ifdef CONFIG_INPUTHUB_30
#include <platform_include/smart/linux/iomcu_boot.h>
#endif
#include <linux/delay.h>
#include <linux/err.h>
#ifdef CONFIG_CONTEXTHUB_IDLE_32K
#include <linux/hisi/hisi_idle_sleep.h>
#endif
#include <platform_include/basicplatform/linux/ipc_rproc.h>
#include <linux/hisi/bsp_syscounter.h>
#include <linux/hisi/usb/chip_usb.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/of_address.h>
#ifdef CONFIG_COUL_DRV
#include <platform_include/basicplatform/linux/power/platform/coul/coul_drv.h>
#endif
#include <linux/regulator/consumer.h>
#include <linux/workqueue.h>
#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
#include <internal_security_interface.h>
#endif
#if defined(CONFIG_HUAWEI_OHOS_DSM) || defined(CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
#endif
/*
 * CONFIG_USE_CAMERA3_ARCH : the camera module build config
 * du to the sensor_1V8 by camera power chip
 */
#ifdef CONFIG_USE_CAMERA3_ARCH
#include <platform_include/camera/native/hw_extern_pmic.h>
#endif
#include <securec.h>

#include "contexthub_pm.h"
#include "contexthub_recovery.h"
#include "contexthub_route.h"
#include "mag_channel.h"
#include "motion_detect.h"
#include "sensor_config.h"
#include "sensor_feima.h"
#include "als/als_tp_color.h"

#ifdef CONFIG_USE_CAMERA3_ARCH
#define SENSOR_1V8_POWER
#endif
#ifndef CONFIG_INPUTHUB_30
int (*api_inputhub_mcu_recv) (const char *buf, unsigned int length) = 0;
#endif
#define POLL_TIMES 10
#define BUFFER_SIZE 1024
#define SLEEP_TIME 200

static int is_sensor_mcu_mode; /* mcu power mode: 0 power off;  1 power on */
#ifndef CONFIG_INPUTHUB_30
static struct notifier_block g_contexthub_boot_nb;
static int g_boot_iom3 = STARTUP_IOM3_CMD;
#endif
struct completion iom3_reboot;
struct regulator *sensorhub_vddio;
int sensor_power_pmic_flag = 0;
int sensor_power_init_finish = 0;
static u8 tplcd_manufacture;
static u8 tplcd_manufacture_curr; // get from /sys/class/graphics/fb0/lcd_model
static u8 g_lcd_info;
static u8 g_sec_lcd_info;

rproc_id_t ipc_ap_to_iom_mbx = IPC_AO_ACPU_IOM3_MBX1;
rproc_id_t ipc_ap_to_lpm_mbx = IPC_ACPU_LPM3_MBX_5;
rproc_id_t ipc_iom_to_ap_mbx = IPC_AO_IOM3_ACPU_MBX1;

static int sensorhub_reboot_reason_flag;

#ifdef CONFIG_CONTEXTHUB_IDLE_32K
#define PERI_USED_TIMEOUT (jiffies + HZ / 100)
#define LOWER_LIMIT 0
#define UPPER_LIMIT 255
#define borderline_upper_protect(a, b) (((a) == (b)) ? (a) : ((a) + 1))
#define borderline_lower_protect(a, b) (((a) == (b)) ? (a) : ((a) - 1))
struct timer_list peri_timer;
static unsigned int peri_used_t;
static unsigned int peri_used;
spinlock_t peri_lock;

static void peri_used_timeout(unsigned long data)
{
	unsigned long flags;

	spin_lock_irqsave(&peri_lock, flags);
	pr_debug("[%s]used[%d],t[%d]\n", __func__, peri_used, peri_used_t);
	if (peri_used == 0) {
		int ret = hisi_idle_sleep_vote(ID_IOMCU, 0);
		if (ret)
			pr_err("[%s]hisi_idle_sleep_vote err\n", __func__);
		peri_used_t = 0;
	}
	spin_unlock_irqrestore(&peri_lock, flags);
}
#endif

u8 get_tplcd_manufacture(void)
{
	return tplcd_manufacture;
}
u8 get_multi_lcd_info(int index)
{
	if (index == LCD_PANEL_1_OUTSIDE)
		return g_sec_lcd_info;
	return g_lcd_info;
}

u8 get_tplcd_manufacture_curr(void)
{
	return tplcd_manufacture_curr;
}

rproc_id_t get_ipc_ap_to_iom_mbx(void)
{
	return ipc_ap_to_iom_mbx;
}

rproc_id_t get_ipc_ap_to_lpm_mbx(void)
{
	return ipc_ap_to_lpm_mbx;
}

int get_sensorhub_reboot_reason_flag(void)
{
	return sensorhub_reboot_reason_flag;
}

void peri_used_request(void)
{
#ifdef CONFIG_CONTEXTHUB_IDLE_32K
	unsigned long flags;

	del_timer_sync(&peri_timer);
	spin_lock_irqsave(&peri_lock, flags);
	pr_debug("[%s]used[%d], t[%d]\n", __func__, peri_used, peri_used_t);
	if (peri_used != 0) {
		peri_used = borderline_upper_protect(peri_used, UPPER_LIMIT);
		spin_unlock_irqrestore(&peri_lock, flags);
		return;
	}

	if (peri_used_t == 0) {
		int ret = hisi_idle_sleep_vote(ID_IOMCU, 1);
		if (ret)pr_err("[%s]hisi_idle_sleep_vote err\n", __func__);
	}

	peri_used = 1;
	peri_used_t = 1;

	spin_unlock_irqrestore(&peri_lock, flags);
#endif
}

void peri_used_release(void)
{
#ifdef CONFIG_CONTEXTHUB_IDLE_32K
	unsigned long flags;

	spin_lock_irqsave(&peri_lock, flags);
	pr_debug("[%s]used[%d]\n", __func__, peri_used);
	peri_used = borderline_lower_protect(peri_used, LOWER_LIMIT);
	mod_timer(&peri_timer, PERI_USED_TIMEOUT);
	spin_unlock_irqrestore(&peri_lock, flags);
#endif
}

#if defined(CONFIG_HUAWEI_OHOS_DSM) || defined(CONFIG_HUAWEI_DSM)
struct dsm_client *shb_dclient;
struct dsm_client *inputhub_get_shb_dclient(void)
{
	return shb_dclient;
}
#endif

#ifndef SENSOR_1V8_POWER
static int hw_extern_pmic_query_state(int index, int *state)
{
	(void *)state;
	hwlog_err("the camera power cfg donot define, %d\n", index);
	return 1;
}
#endif

int get_sensor_mcu_mode(void)
{
	return is_sensor_mcu_mode;
}
EXPORT_SYMBOL(get_sensor_mcu_mode);

static void set_sensor_mcu_mode(int mode)
{
	is_sensor_mcu_mode = mode;
}

static bool recovery_mode_skip_load(void)
{
	int len = 0;
	struct device_node *recovery_node = NULL;
	const char *recovery_attr = NULL;

	if (!strstr(saved_command_line, "recovery_update=1"))
		return false;

	recovery_node = of_find_compatible_node(NULL, NULL, "hisilicon,recovery_iomcu_image_skip");
	if (!recovery_node)
		return false;

	recovery_attr = of_get_property(recovery_node, "status", &len);
	if (!recovery_attr)
		return false;

	if (strcmp(recovery_attr, "ok") != 0)
		return false;

	return true;
}

static bool inputhub_is_iomcu_bypass(void)
{
	struct device_node *node = NULL;
	unsigned int value;
	int ret;

	node = of_find_compatible_node(NULL, NULL, "hisilicon,contexthub_status");
	if (node == NULL) {
		pr_warn("%s: no contexthub status node.\n", __func__);
		return false;
	}

	ret = of_property_read_u32(node, "hardware_bypass", &value);
	if (ret != 0) {
		pr_warn("%s: read iomcu hardware_bypass fail, ret[%d]\n", __func__, ret);
		return false;
	}

	if (value == 1) {
		pr_warn("%s: iomcu hardware bypass\n", __func__);
		return true;
	}

	return false;
}

int is_sensorhub_disabled(void)
{
	int len = 0;
	struct device_node *sh_node = NULL;
	const char *sh_status = NULL;
	static int ret;
	static int once;

#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
	if (!get_ddr_dts_status()) {
		hwlog_err("%s:failed to get ddr dts info\n", __func__);
		ret = -EINVAL;
	}
#endif

	if (once)
		return ret;

	if (recovery_mode_skip_load()) {
		hwlog_err("%s: recovery update mode, do not start sensorhub\n", __func__);
		once = 1;
		ret = -1;
		return ret;
	}

	if (inputhub_is_iomcu_bypass()) {
		hwlog_err("%s: IOMCU bypass, do not start sensorhub\n", __func__);
		once = 1;
		ret = -1;
		return ret;
	}

	sh_node = of_find_compatible_node(NULL, NULL, "huawei,sensorhub_status");
	if (!sh_node) {
		hwlog_err("%s, can not find node sensorhub_status\n", __func__);
		return -1;
	}

	sh_status = of_get_property(sh_node, "status", &len);
	if (!sh_status) {
		hwlog_err("%s, can't find property status\n", __func__);
		return -1;
	}

	if (strstr(sh_status, "ok")) {
		hwlog_info("%s, sensorhub enabled!\n", __func__);
		ret = 0;
	} else {
		hwlog_info("%s, sensorhub disabled!\n", __func__);
		ret = -1;
	}
	once = 1;
	return ret;
}
EXPORT_SYMBOL(is_sensorhub_disabled);

#ifdef SENSOR_1V8_POWER
static int check_sensor_1v8_from_pmic(void)
{
	int tmp = 0;
	int i;
	int sensor_1v8_flag = 0;
	int state = 0;
	int sensor_1v8_ldo = 0;
	struct device_node *sensorhub_node = NULL;
	const char *sensor_1v8_from_pmic = NULL;

	sensorhub_node = of_find_compatible_node(NULL, NULL, "huawei,sensorhub");
	if (!sensorhub_node) {
		hwlog_err("%s, can't find node sensorhub\n", __func__);
		return 0;
	}
	sensor_1v8_from_pmic = of_get_property(sensorhub_node, "sensor_1v8_from_pmic", &tmp);
	if (!sensor_1v8_from_pmic) {
		hwlog_info("%s, can't find property sensor_1v8_from_pmic\n", __func__);
		return 1;
	}
	sensor_power_pmic_flag = 1;
	if (strstr(sensor_1v8_from_pmic, "yes")) {
		hwlog_info("%s, sensor_1v8 from pmic\n", __func__);
		if (of_property_read_u32(sensorhub_node,
			"sensor_1v8_ldo", &sensor_1v8_ldo)) {
			hwlog_err("%s, read sensor_1v8_ldo fail\n", __func__);
			return 0;
		} else {
			hwlog_info("%s, read sensor_1v8_ldo:%d succ\n",
				__func__, sensor_1v8_ldo);
			sensor_1v8_flag = 1;
		}
	} else {
		hwlog_info("%s, sensor_1v8 not from pmic\n", __func__);
		return 1;
	}

	if (sensor_1v8_flag) {
		for (i = 0; i < POLL_TIMES; i++) {
			tmp = hw_extern_pmic_query_state(sensor_1v8_ldo, &state);
			if (state) {
				hwlog_info("sensor_1V8 set high succ!\n");
				break;
			}
			msleep(SLEEP_TIME);
		}
		if (i == POLL_TIMES && state == 0) {
			hwlog_err("sensor_1V8 set high fail, ret:%d!\n", tmp);
			return 0;
		}
	}
	sensor_power_init_finish = 1;
	return 1;
}
#endif

int sensor_pmic_power_check(void)
{
	int ret;
	int state = 0;
	int result = SENSOR_POWER_STATE_OK;

	if (!sensor_power_init_finish || is_sensorhub_disabled()) {
		result = SENSOR_POWER_STATE_INIT_NOT_READY;
		goto out;
	}
	if (!sensor_power_pmic_flag) {
		result =  SENSOR_POWER_STATE_NOT_PMIC;
		goto out;
	}

	ret = hw_extern_pmic_query_state(1, &state);
	if (ret) {
		result = SENSOR_POWER_STATE_CHECK_ACTION_FAILED;
		goto out;
	}
	if (!state)
		result = SENSOR_POWER_STATE_CHECK_RESULT_FAILED;
out:
	hwlog_info("sensor check result:%d\n", result);
	return result;
}

static lcd_module lcd_info[] = {
	{ DTS_COMP_JDI_NT35695_CUT3_1, JDI_TPLCD },
	{ DTS_COMP_LG_ER69006A, LG_TPLCD },
	{ DTS_COMP_JDI_NT35695_CUT2_5, JDI_TPLCD },
	{ DTS_COMP_LG_ER69007, KNIGHT_BIEL_TPLCD },
	{ DTS_COMP_SHARP_NT35597, KNIGHT_BIEL_TPLCD },
	{ DTS_COMP_SHARP_NT35695, KNIGHT_BIEL_TPLCD },
	{ DTS_COMP_LG_ER69006_FHD, KNIGHT_BIEL_TPLCD },
	{ DTS_COMP_MIPI_BOE_ER69006, KNIGHT_LENS_TPLCD },
	{ DTS_COMP_BOE_OTM1906C, BOE_TPLCD },
	{ DTS_COMP_INX_OTM1906C, INX_TPLCD },
	{ DTS_COMP_EBBG_OTM1906C, EBBG_TPLCD },
	{ DTS_COMP_JDI_NT35695, JDI_TPLCD },
	{ DTS_COMP_LG_R69006, LG_TPLCD },
	{ DTS_COMP_SAMSUNG_S6E3HA3X02, SAMSUNG_TPLCD },
	{ DTS_COMP_LG_R69006_5P2, LG_TPLCD },
	{ DTS_COMP_SHARP_NT35695_5P2, SHARP_TPLCD },
	{ DTS_COMP_JDI_R63452_5P2, JDI_TPLCD },
	{ DTS_COMP_SAM_WQ_5P5, BIEL_TPLCD },
	{ DTS_COMP_SAM_FHD_5P5, VITAL_TPLCD },
	{ DTS_COMP_JDI_R63450_5P7, JDI_TPLCD },
	{ DTS_COMP_SHARP_DUKE_NT35597, SHARP_TPLCD },
	{ DTS_COMP_BOE_NT37700F, BOE_TPLCD },
	{ DTS_COMP_BOE_NT36672_6P26, BOE_TPLCD },
	{ DTS_COMP_LG_NT37280, LG_TPLCD },
	{ DTS_COMP_BOE_NT37700F_EXT, BOE_TPLCD },
	{ DTS_COMP_LG_NT37280_EXT, LG_TPLCD },
	{ DTS_COMP_SAMSUNG_EA8074, SAMSUNG_TPLCD },
	{ DTS_COMP_SAMSUNG_EA8076, SAMSUNG_TPLCD },
	{ DTS_COMP_SAMSUNG_EA8076_V2, SAMSUNG_TPLCD },
	{ DTS_COMP_BOE_NT37700F_TAH, BOE_TPLCD },
	{ DTS_COMP_BOE_NT37800ECO_TAH, BOE_TPLCD },
	{ DTS_COMP_HLK_AUO_OTM1901A, AUO_TPLCD },
	{ DTS_COMP_BOE_NT36682A, BOE_TPLCD },
	{ DTS_COMP_BOE_TD4320, BOE_TPLCD },
	{ DTS_COMP_INX_NT36682A, INX_TPLCD },
	{ DTS_COMP_TCL_NT36682A, TCL_TPLCD },
	{ DTS_COMP_TM_NT36682A, TM_TPLCD },
	{ DTS_COMP_TM_TD4320, TM_TPLCD },
	{ DTS_COMP_TM_TD4320_6P26, TM_TPLCD },
	{ DTS_COMP_TM_TD4330_6P26, TM_TPLCD },
	{ DTS_COMP_TM_NT36672A_6P26, TM_TPLCD },
	{ DTS_COMP_LG_TD4320_6P26, LG_TPLCD },
	{ DTS_COMP_CTC_FT8719_6P26, CTC_TPLCD },
	{ DTS_COMP_CTC_NT36672A_6P26, CTC_TPLCD },
	{ DTS_COMP_BOE_TD4321_6P26, BOE_TPLCD },
	{ DTS_COMP_AUO_NT36682A_6P72, AUO_TPLCD },
	{ DTS_COMP_AUO_OTM1901A_5P2_1080P_VIDEO_DEFAULT, AUO_TPLCD },
	{ DTS_COMP_BOE_NT36682A_6P57, BOE_TPLCD },
	{ DTS_COMP_BOE_TD4320_6P57, BOE_TPLCD },
	{ DTS_COMP_TCL_NT36682A_6P57, TCL_TPLCD },
	{ DTS_COMP_TCL_TD4320_6P57, TCL_TPLCD },
	{ DTS_COMP_TM_NT36682A_6P57, TM_TPLCD },
	{ DTS_COMP_TM_TD4320_6P57, TM_TPLCD },
	{ DTS_COMP_CSOT_NT36682A_6P5, CSOT_TPLCD },
	{ DTS_COMP_BOE_FT8719_6P5, BOE_TPLCD },
	{ DTS_COMP_TM_NT36682A_6P5, TM_TPLCD },
	{ DTS_COMP_BOE_TD4320_6P5, BOE_TPLCD },
	{ DTS_COMP_BOE_EW813P_6P57, BOE_TPLCD },
	{ DTS_COMP_BOE_NT37700P_6P57, BOE_TPLCD },
	{ DTS_COMP_VISI_NT37700C_6P57_ONCELL, VISI_TPLCD },
	{ DTS_COMP_VISI_NT37700C_6P57,  VISI_TPLCD },
	{ DTS_COMP_TCL_NT36682C_6P63, TCL_TPLCD },
	{ DTS_COMP_TM_NT36682C_6P63, TM_TPLCD },
	{ DTS_COMP_BOE_NT36682C_6P63, BOE_TPLCD },
	{ DTS_COMP_INX_NT36682C_6P63, INX_TPLCD },
	{ DTS_COMP_BOE_FT8720_6P63, BOE_TPLCD2 },
	{ DTS_COMP_SAMSUNG_EA8076_6P53, SAMSUNG_TPLCD },
	{ DTS_COMP_EDO_RM692D9, EDO_TPLCD },
};

static lcd_model lcd_model_info[] = {
	{ DTS_COMP_AUO_OTM1901A_5P2, AUO_TPLCD },
	{ DTS_COMP_AUO_TD4310_5P2, AUO_TPLCD },
	{ DTS_COMP_TM_FT8716_5P2, TM_TPLCD },
	{ DTS_COMP_EBBG_NT35596S_5P2, EBBG_TPLCD },
	{ DTS_COMP_JDI_ILI7807E_5P2, JDI_TPLCD },
	{ DTS_COMP_AUO_NT36682A_6P72, AUO_TPLCD },
	{ DTS_COMP_AUO_OTM1901A_5P2_1080P_VIDEO_DEFAULT, AUO_TPLCD },
	{ DTS_COMP_BOE_NT36682A_6P57, BOE_TPLCD },
	{ DTS_COMP_BOE_TD4320_6P57, BOE_TPLCD },
	{ DTS_COMP_TCL_NT36682A_6P57, TCL_TPLCD },
	{ DTS_COMP_TCL_TD4320_6P57, TCL_TPLCD },
	{ DTS_COMP_TM_NT36682A_6P57, TM_TPLCD },
	{ DTS_COMP_TM_TD4320_6P57, TM_TPLCD },
	{ DTS_COMP_CSOT_NT36682A_6P5, CSOT_TPLCD },
	{ DTS_COMP_BOE_FT8719_6P5, BOE_TPLCD },
	{ DTS_COMP_TM_NT36682A_6P5, TM_TPLCD },
	{ DTS_COMP_BOE_TD4320_6P5, BOE_TPLCD },
	{ DTS_COMP_BOE_EW813P_6P57, BOE_TPLCD },
	{ DTS_COMP_BOE_NT37700P_6P57, BOE_TPLCD },
	{ DTS_COMP_VISI_NT37700C_6P57_ONCELL, VISI_TPLCD },
	{ DTS_COMP_VISI_NT37700C_6P57,  VISI_TPLCD },
	{ DTS_COMP_TM_TD4321_6P59, TM_TPLCD },
	{ DTS_COMP_TCL_NT36682A_6P59, TCL_TPLCD },
	{ DTS_COMP_BOE_NT36682A_6P59, BOE_TPLCD },
	{ DTS_COMP_BOE_NT36672_6P26, BOE_TPLCD },
	{ DTS_COMP_LG_TD4320_6P26, LG_TPLCD },
	{ DTS_COMP_TM_TD4320_6P26, TM_TPLCD },
	{ DTS_COMP_TM_TD4330_6P26, TM_TPLCD },
	{ DTS_COMP_TM_NT36672A_6P26, TM_TPLCD },
	{ DTS_COMP_CTC_FT8719_6P26, CTC_TPLCD },
	{ DTS_COMP_CTC_NT36672A_6P26, CTC_TPLCD },
	{ DTS_COMP_BOE_RUIDING, BOE_RUIDING_TPLCD },
	{ DTS_COMP_BOE, BOE_TPLCD },
	{ DTS_COMP_BOE_B11, BOE_TPLCD },
	{ DTS_COMP_VISI, VISI_TPLCD },
	{ DTS_COMP_SAMSUNG_NAME, SAMSUNG_TPLCD },
	{ DTS_COMP_BOE_NAME, BOE_TPLCD },
	{ DTS_COMP_TM, TM_TPLCD },
	{ DTS_COMP_CSOT, CSOT_TPLCD },
	{ DTS_COMP_BOE_B12, BOE_TPLCD_B12 },
	{ DTS_COMP_SAMSUNG_NAME1, SAMSUNG_TPLCD },
	{ DTS_COMP_SAMSUNG, SAMSUNG_TPLCD },
	{ DTS_COMP_EDO_NAME, EDO_TPLCD },
	{ DTS_COMP_EDO, EDO_TPLCD },
};

const static lcd_panel_model g_lcd_panel_model_list[] = {
	{ "PCE_310 h01 0 13.2' 2880 x 1920", TS_PANEL_VXN },
	{ "XYAO_350 h01 0 11.0' 2560 x 1600", TS_PANEL_VXN },
};

uint16_t get_lcd_panel_model_list_size(void)
{
	return (uint16_t)ARRAY_SIZE(g_lcd_panel_model_list);
}

static lcd_model charllotte_lcd_info[] = {
	{ DTS_COMP_BOE, BOE_TPLCD },
	{ DTS_COMP_VISI, VISI_TPLCD },
	{ DTS_COMP_BOE_B12, BOE_TPLCD_B12 },
	{ DTS_COMP_CSOT, CSOT_TPLCD }
};
static int8_t get_lcd_info(uint8_t index)
{
	struct device_node *np = NULL;
	int ret;

	np = of_find_compatible_node(NULL, NULL, lcd_info[index].dts_comp_mipi);
	ret = of_device_is_available(np);
	if (np && ret)
		return lcd_info[index].tplcd;
	else
		return -1;
}

static int8_t get_lcd_model(const char *lcd_model, uint8_t index)
{
	if (!strncmp(lcd_model, lcd_model_info[index].dts_comp_lcd_model,
		strlen(lcd_model_info[index].dts_comp_lcd_model)))
		return lcd_model_info[index].tplcd;
	else
		return -1;
}

static int get_lcd_module(void)
{
	uint8_t index;
	int8_t tplcd;
	struct device_node *np = NULL;
	const char *lcd_model = NULL;

	for (index = 0; index < ARRAY_SIZE(lcd_info); index++) {
		tplcd = get_lcd_info(index);
		if (tplcd > 0)
			return tplcd;
	}
	np = of_find_compatible_node(NULL, NULL, "huawei,lcd_panel_type");
	if (!np) {
		hwlog_err("not find lcd_panel_type node\n");
		return -1;
	}
	if (of_property_read_string(np, "lcd_panel_type", &lcd_model)) {
		hwlog_err("not find lcd_model in dts\n");
		return -1;
	}
	hwlog_info("find lcd_panel_type suc in dts!!\n");

	for (index = 0; index < ARRAY_SIZE(lcd_model_info); index++) {
		tplcd = get_lcd_model(lcd_model, index);
		if (tplcd > 0)
			return tplcd;
	}

	hwlog_warn("sensor kernel failed to get lcd module\n");
	return -1;
}

static uint16_t find_lcd_manufacture(const char *panel_name)
{
	const int len = get_lcd_panel_model_list_size();
	int i = 0;
	for (i = 0; i < len; i++) {
		if ((strncmp(g_lcd_panel_model_list[i].panel_name, panel_name,
				strlen(g_lcd_panel_model_list[i].panel_name)) == 0)) {
			hwlog_err("match panel_name success, index = %d, manufacture = %d", i,
				g_lcd_panel_model_list[i].manufacture);
			return (uint16_t)g_lcd_panel_model_list[i].manufacture;
		}
	}
	return DEFAULT_TPLCD;
}

void init_tp_manufacture_curr(void)
{
	int ret = -1;
	char buf[LCD_PANEL_NAME_LEN + 1] = {'\0'};
	char path[128] = "/sys/class/graphics/fb0/lcd_model";
	struct file *fp = NULL;
	mm_segment_t old_fs;
	u8 lcd_manuf;

	tplcd_manufacture_curr = DEFAULT_TPLCD;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fp = filp_open(path, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		hwlog_err("FAIL TO OPEN %s FILE", path);
		set_fs(old_fs);
		return;
	}

	vfs_llseek(fp, 0L, SEEK_SET);
	ret = vfs_read(fp, buf, LCD_PANEL_NAME_LEN, &(fp->f_pos));
	if (ret < 0) {
		hwlog_err("%s:read file %s exception with ret %d.\n",
			__func__, path, ret);
		set_fs(old_fs);
		filp_close(fp, NULL);
		return;
	}
	filp_close(fp, NULL);

	set_fs(old_fs);

	lcd_manuf = find_lcd_manufacture(buf);
	if (lcd_manuf == DEFAULT_TPLCD)
		return;

	tplcd_manufacture_curr = lcd_manuf;
	return;
}

static int get_all_lcd_info(void)
{
	uint8_t index;
	struct device_node *np = NULL;
	const char *lcd_model = NULL;

	np = of_find_compatible_node(NULL, NULL, "huawei,lcd_panel_type");
	if (!np) {
		hwlog_err("not find lcd_panel_type node\n");
		return -1;
	}
	if (of_property_read_string(np, "lcd_panel_type", &lcd_model)) {
		hwlog_err("not find lcd_panel_type in dts\n");
	} else {
		g_lcd_info = -1;
		for (index = 0; index < ARRAY_SIZE(charllotte_lcd_info); index++) {
			if (!strncmp(lcd_model, charllotte_lcd_info[index].dts_comp_lcd_model,
				strlen(charllotte_lcd_info[index].dts_comp_lcd_model))) {
				g_lcd_info = charllotte_lcd_info[index].tplcd;
				break;
			}
		}
		hwlog_info("g_lcd_info=%d, lcd_model=%s\n", g_lcd_info, lcd_model);
	}
	if (of_property_read_string(np, "sec_lcd_panel_type", &lcd_model)) {
		hwlog_err("not find sec_lcd_panel_type in dts\n");
	} else {
		g_sec_lcd_info = -1;
		for (index = 0; index < ARRAY_SIZE(charllotte_lcd_info); index++) {
			if (!strncmp(lcd_model, charllotte_lcd_info[index].dts_comp_lcd_model,
				strlen(charllotte_lcd_info[index].dts_comp_lcd_model))) {
				g_sec_lcd_info = charllotte_lcd_info[index].tplcd;
				break;
			}
		}
		hwlog_info("g_sec_lcd_info=%d, lcd_model=%s\n", g_sec_lcd_info, lcd_model);
	}
	return 0;
}
#ifndef CONFIG_INPUTHUB_30
static int inputhub_mcu_recv(const char* buf, unsigned int length)
{
	if (atomic_read(&iom3_rec_state) == IOM3_RECOVERY_START) {
		hwlog_err("iom3 under recovery mode, ignore all recv data\n");
		return 0;
	}

	if (!api_inputhub_mcu_recv) {
		hwlog_err("---->error: api_inputhub_mcu_recv == NULL\n");
		return -1;
	} else {
		return api_inputhub_mcu_recv(buf, length);
	}
}

/* received data from mcu. */
static int mbox_recv_notifier(struct notifier_block *nb, unsigned long len,
	void *msg)
{
	inputhub_mcu_recv(msg, len * sizeof(int)); /* convert to bytes */
	return 0;
}

static int inputhub_mcu_connect(void)
{
	int ret;
	/* connect to inputhub_route */
	api_inputhub_mcu_recv = inputhub_route_recv_mcu_data;

	hwlog_info("----%s--->\n", __func__);

	g_contexthub_boot_nb.next = NULL;
	g_contexthub_boot_nb.notifier_call = mbox_recv_notifier;

	/* register the rx notify callback */
	ret = RPROC_MONITOR_REGISTER(ipc_iom_to_ap_mbx, &g_contexthub_boot_nb);
	if (ret)
		hwlog_info("%s:RPROC_MONITOR_REGISTER failed", __func__);

	return 0;
}
#endif

static int sensorhub_img_dump(int type, void* buff, int size)
{
	return 0;
}

#ifdef CONFIG_HISYSEVENT
struct error_no_map {
	int error_no;
	char *name;
};
 
int sensorhub_errorno_to_str(int error_no, char *str, int bufflen)
{
	int i;
	struct error_no_map sensorhub_error_no_map[] = {
		{921001058, "SHB_B2_BATTERY_COVER_STATE_ERR"},
		{921001059, "SHB_B2_ALL_SWITCHS_STATE_ERR"},
		{921001060, "SHB_B2_POWER_SUPPLY_LOGIC_ERR"},
		{936005002, "SHB_DDR_USAGE_INFO"},
		{936005007, "SHB_PERF_PARAM_INFO"},
		{936005008, "SHB_HIGH_LOAD_TASK_INFO"},
		{936005009, "SHB_AOD_STATIS"},
		{936005011, "SHB_B2_USAGE_INFO"},
		{926006002, "DSM_SHB_ERR_MCU_ALS"},
		{925201620, "IO_DIE_ERROR_INFO"},
		{926005003, "DSM_SHB_ERR_IOM7_DETECT_FAIL"},
		{926005007, "DSM_SHB_ERR_PS_OIL_POLLUTION"},
		{926005009, "DSM_SHB_ERR_ACCEL_DATA_UNCHANGE"},
		{926005010, "DSM_SHB_ERR_POSTURE_EXCEPTION"},
		{926006000, "DSM_SHB_ERR_GYRO_DATA_UNCHANGE"},
		{926006001, "DSM_SHB_ERR_MAG_DATA_UNCHANGE"},
		{926006005, "DSM_SHB_ERR_MCU_I2C_ERR"},
		{926006013, "DSM_SHB_ERR_MCU_PS"},
		{926005000, "DSM_SHB_ERR_IOM7_CFG_DATA"},
		{926005001, "DSM_SHB_ERR_IOM7_CMD"},
		{926005002, "DSM_SHB_ERR_IOM7_DYNLOAD"},
		{926005004, "DSM_SHB_ERR_IOM7_IPC_TIMEOUT"},
		{926005005, "DSM_SHB_ERR_IOM7_OTHER"},
		{926005006, "DSM_SHB_ERR_IOM7_READ"},
		{926005008, "DSM_SHB_ERR_IOM7_WDG"},
		{926004048, "DSM_SHB_ERR_TP_INIT_ERROR"},
		{926004049, "DSM_SHB_ERR_TP_I2C_ERROR"},
		{926004050, "DSM_SHB_ERR_TP_ESD_ERROR"},
		{926006003, "DSM_SHB_ERR_MCU_FUSION"},
		{926006004, "DSM_SHB_ERR_MCU_GYRO"},
		{926006006, "DSM_SHB_ERR_MCU_LABC"},
		{926006007, "DSM_SHB_ERR_MCU_LIGHT"},
		{926006008, "DSM_SHB_ERR_MCU_MAG"},
		{926006009, "DSM_SHB_ERR_MCU_MOTION"},
		{926006010, "DSM_SHB_ERR_MCU_OTHER"},
		{926006011, "DSM_SHB_ERR_MCU_PDR"},
		{926006012, "DSM_SHB_ERR_MCU_PEDOMETER"},
		{926006013, "DSM_SHB_ERR_MCU_PS"},
		{926006100, "DSM_SHB_ERR_MCU_SYS_MAIN_INIT"},
		{926006101, "DSM_SHB_ERR_MCU_SYS_SERVER_INIT"},
		{926006102, "DSM_SHB_ERR_MCU_TIMER_TIMEOUT"},
		{926006103, "DSM_SHB_ERR_MCU_ACC_PERIOD_ERROR"},
		{926006104, "DSM_SHB_ERR_MCU_CA"},
		{926006105, "DSM_SHB_ERR_SYSCOUNT"},
		{926007000, "DSM_SHB_ERR_GSENSOR_I2C_ERR"},
		{926007001, "DSM_SHB_ERR_GSENSOR_DATA_ABNORMAL"},
		{926007002, "DSM_SHB_ERR_GSENSOR_DATA_ALL_ZERO"},
		{926007003, "DSM_SHB_ERR_GSENSOR_DATA_NOT_UPDATE"},
		{926007004, "DSM_SHB_ERR_LIGHT_I2C_ERR"},
		{926007005, "DSM_SHB_ERR_LIGHT_IRQ_ERR"},
		{926007006, "DSM_SHB_ERR_LIGHT_THRESHOLD_ERR"},
		{926007007, "DSM_SHB_ERR_LIGHT_NO_IRQ"},
		{926007008, "DSM_SHB_ERR_LIGHT_ENABLE_ERR"},
		{926007009, "DSM_SHB_ERR_LIGHT_THRESHOLD_OUT"},
		{926007010, "DSM_SHB_ERR_LIGHT_POWER_ERR"},
		{926007011, "DSM_SHB_ERR_MAG_I2C_READ"},
		{926007012, "DSM_SHB_ERR_MAG_DATA_ABNORAML"},
		{926007013, "DSM_SHB_ERR_MAG_DATA_NOT_UPDATE"},
		{926007014, "DSM_SHB_ERR_MAG_SELFTEST_ERR"},
		{926007015, "DSM_SHB_ERR_SENSORSERVICE_EXIT_ERR"},
		{926007016, "DSM_SHB_ERR_SENSORSERVICE_DATA_UPDATE"},
		{926007017, "DSM_SHB_ERR_LAST"},
	};
	int error_no_map_len = sizeof(sensorhub_error_no_map) / sizeof(sensorhub_error_no_map[0]);

	for (i = 0; i < error_no_map_len; i++) {
		if (sensorhub_error_no_map[i].error_no == error_no) {
			if (strlen(sensorhub_error_no_map[i].name) >= bufflen) {
				hwlog_err("sensorhub_errorno_to_str name too long\n");
				return -1;
			}
			if (strncpy_s(str, bufflen, sensorhub_error_no_map[i].name, strlen(sensorhub_error_no_map[i].name)) != EOK) {
				hwlog_err("sensorhub_errorno_to_str strncpy fail\n");
				return -1;
			}
			str[bufflen - 1] = '\0';
			hwlog_info("sensorhub_errorno_to_str:%s\n", str);
			return 0;
		}
	}
	return -1;
}
#endif

#if defined(CONFIG_HUAWEI_OHOS_DSM) || defined(CONFIG_HUAWEI_DSM)
struct dsm_client_ops sensorhub_ops = {
	.poll_state = NULL,
	.dump_func = sensorhub_img_dump,
#ifdef CONFIG_HISYSEVENT
	.errorno_to_str = sensorhub_errorno_to_str,
#endif
};

struct dsm_dev dsm_sensorhub = {
#ifdef CONFIG_HISYSEVENT
	.name = "SENSORHUB",
#else
	.name = "dsm_sensorhub",
#endif
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = &sensorhub_ops,
	.buff_size = BUFFER_SIZE,
};

struct dsm_dev *get_dsm_sensorhub(void)
{
	return &dsm_sensorhub;
}
#endif
#ifndef CONFIG_INPUTHUB_30
static int boot_iom3(void)
{
	int ret;

	peri_used_request();
	ret = RPROC_ASYNC_SEND(ipc_ap_to_iom_mbx, (mbox_msg_t*)&g_boot_iom3, 1);
	peri_used_release();
	if (ret)
		hwlog_err("RPROC_ASYNC_SEND error in %s\n", __func__);
	return ret;
}
#endif
static void sensor_ldo24_setup(void)
{
	int ret;
	unsigned int time_reset;
	unsigned long new_sensor_jiffies;

	if (get_no_need_sensor_ldo24()) {
		hwlog_info("no_need set ldo24\n");
		return;
	}

	if (need_reset_io_power &&
		(sensorhub_reboot_reason_flag ==
		SENSOR_POWER_DO_RESET)) {
		new_sensor_jiffies = jiffies - get_sensor_jiffies();
		time_reset = jiffies_to_msecs(new_sensor_jiffies);
		if (time_reset < SENSOR_MAX_RESET_TIME_MS)
			msleep(SENSOR_MAX_RESET_TIME_MS - time_reset);

		if (need_set_3v_io_power) {
			ret = regulator_set_voltage(sensorhub_vddio,
				SENSOR_VOLTAGE_3V, SENSOR_VOLTAGE_3V);
			if (ret < 0)
				hwlog_err("failed to set ldo24 to 3V\n");
		}

		if (need_set_3_1v_io_power) {
			ret = regulator_set_voltage(sensorhub_vddio,
				SENSOR_VOLTAGE_3_1V, SENSOR_VOLTAGE_3_1V);
			if (ret < 0)
				hwlog_err("failed to set ldo24 to 3_1V\n");
		}

		if (need_set_3_2v_io_power) {
			ret = regulator_set_voltage(sensorhub_vddio,
				SENSOR_VOLTAGE_3_2V, SENSOR_VOLTAGE_3_2V);
			if (ret < 0)
				hwlog_err("failed to set ldo24 to 3_2V\n");
		}

		hwlog_info("time_of_vddio_power_reset %u\n", time_reset);
		ret = regulator_enable(sensorhub_vddio);
		if (ret < 0)
			hwlog_err("sensor vddio enable 2.85V\n");

		msleep(SENSOR_DETECT_AFTER_POWERON_TIME_MS);
	}
}

static int mcu_sys_ready_callback(const struct pkt_header *head)
{
	int ret;
#ifdef SENSOR_1V8_POWER
	int result;
#endif
	if (((pkt_sys_statuschange_req_t *) head)->status == ST_MINSYSREADY) {
		tplcd_manufacture = get_lcd_module();
		init_tp_manufacture_curr();
		get_all_lcd_info();
		hwlog_info("tplcd_manufacture=%d, curr=%d\n", tplcd_manufacture, tplcd_manufacture_curr);
#ifdef SENSOR_1V8_POWER
		result = check_sensor_1v8_from_pmic();
		if (!result)
			hwlog_err("check sensor_1V8 from pmic fail\n");
#endif
		hwlog_info("need_reset_io_power:%d reboot_reason_flag:%d\n",
				need_reset_io_power, sensorhub_reboot_reason_flag);
		sensor_ldo24_setup();
		ret = init_sensors_cfg_data_from_dts();
		if (ret)
			hwlog_err("get sensors cfg data from dts fail,ret=%d, use default config data!\n", ret);
		else
			hwlog_info("get sensors cfg data from dts success!\n");
	} else if (((pkt_sys_statuschange_req_t *) head)->status == ST_MCUREADY) {
		hwlog_info("mcu all ready!\n");
		ret = sensor_set_cfg_data();
		if (ret < 0)
			hwlog_err("sensor_chip_detect ret=%d\n", ret);
		ret = sensor_set_fw_load();
		if (ret < 0)
			hwlog_err("sensor fw dload err ret=%d\n", ret);
		ret = motion_set_cfg_data();
		if (ret < 0)
			hwlog_err("motion set cfg data err ret=%d\n", ret);
		unregister_mcu_event_notifier(TAG_SYS, CMD_SYS_STATUSCHANGE_REQ, mcu_sys_ready_callback);
		if (is_power_off_charging_posture() == 1) {
			posture_sensor_enable();
			hwlog_info("open pos sensor when shut_charge mode\n");
		}
#ifndef CONFIG_INPUTHUB_30
		atomic_set(&iom3_rec_state, IOM3_RECOVERY_IDLE);
#else
		set_iom3_rec_state(IOM3_RECOVERY_IDLE);
#endif
	} else {
		hwlog_info("other status\n");
	}
	return 0;
}

static void set_notifier(void)
{
	register_mcu_event_notifier(TAG_SYS,
		CMD_SYS_STATUSCHANGE_REQ,
		mcu_sys_ready_callback);
	register_mcu_event_notifier(TAG_SYS,
		CMD_SYS_STATUSCHANGE_REQ,
		iom3_rec_sys_callback);
	set_pm_notifier();
}

static void read_reboot_reason_cmdline(void)
{
	char reboot_reason_buf[SENSOR_REBOOT_REASON_MAX_LEN] = { 0 };
	char *pstr = NULL;
	char *dstr = NULL;
	bool checklen = false;

	pstr = strstr(saved_command_line, "reboot_reason=");
	if (!pstr) {
		pr_err("No fastboot reboot_reason info\n");
		return;
	}
	pstr += strlen("reboot_reason=");
	dstr = strstr(pstr, " ");
	if (!dstr) {
		pr_err("No find the reboot_reason end\n");
		return;
	}
	checklen = SENSOR_REBOOT_REASON_MAX_LEN > (unsigned long)(dstr - pstr);
	if (!checklen) {
		pr_err("overrun reboot_reason_buf\n");
		return;
	}
	memcpy(reboot_reason_buf, pstr, (unsigned long)(dstr - pstr));
	reboot_reason_buf[dstr - pstr] = '\0';

	if (!strcasecmp(reboot_reason_buf, "AP_S_COLDBOOT"))
		/* reboot flag */
		sensorhub_reboot_reason_flag = SENSOR_POWER_NO_RESET;
	else
		/* others */
		sensorhub_reboot_reason_flag = SENSOR_POWER_DO_RESET;
	hwlog_info("sensorhub get reboot reason:%s length:%d flag:%d\n",
		reboot_reason_buf,
		(int)strlen(reboot_reason_buf),
		sensorhub_reboot_reason_flag);
}

static u32 judge_system_cache(void)
{
	int len = 0;
	struct device_node *sh_swing_node = NULL;
	const char *sh_sc_status = NULL;
	struct device_node *soc_spec_node = NULL;
	const char *soc_spec_set = NULL;

	sh_swing_node = of_find_compatible_node(NULL, NULL, "hisilicon,igs-dev");
	if (!sh_swing_node) {
		hwlog_err("%s, no swing dev in DTS\n", __func__);
		return SC_INEXISTENCE;
	}

	sh_sc_status = of_get_property(sh_swing_node, "syscache_status", NULL);
	if (!sh_sc_status) {
		hwlog_err("%s, can't find property syscache_status\n", __func__);
		return SC_INEXISTENCE;
	} else if (strcmp(sh_sc_status, "ok")) {
		hwlog_err("%s, syscache is not enabled in DTS\n", __func__);
		return SC_INEXISTENCE;
	}

	soc_spec_node = of_find_compatible_node(NULL, NULL, "hisilicon, soc_spec");
	if (!soc_spec_node) {
		hwlog_err("%s, can not find node: soc_spec\n", __func__);
		/* no DTS node -> normal spec */
		return SC_EXISTENCE;
	}

	soc_spec_set = of_get_property(soc_spec_node, "soc_spec_set", &len);
	if (!soc_spec_set) {
		hwlog_err("%s, can't find property soc_spec_set\n", __func__);
		return SC_EXISTENCE;
	}

	if (!strcmp(soc_spec_set, "normal")) {
		hwlog_info("%s, normal chip\n", __func__);
		return SC_EXISTENCE;
	}

	hwlog_info("%s, chip without sc\n", __func__);
	return SC_INEXISTENCE;
}

static bool inputhub_is_swing_bypass(void)
{
#ifdef CONFIG_CONTEXTHUB_IGS
	struct device_node *node = NULL;
	unsigned int value;
	int ret;

	node = of_find_compatible_node(NULL, NULL, "hisilicon,igs-dev");
	if (node == NULL) {
		pr_warn("%s: no swing dev node.\n", __func__);
		return false;
	}

	ret = of_property_read_u32(node, "hardware_bypass", &value);
	if (ret != 0) {
		pr_warn("%s: read prop hardware_bypass fail, ret[%d]\n", __func__, ret);
		return false;
	}

	if (value == 1) {
		pr_warn("%s: swing hardware bypass\n", __func__);
		return true;
	}
	pr_warn("%s: swing hardware work well\n", __func__);
#endif

	return false;
}

static int write_defualt_config_info_to_sharemem(void)
{
	struct log_config *log = iomcu_config_get(IOMCU_CONFIG_USR_LOG, sizeof(struct log_config));
	log->log_buff_cb_backup.mutex = 0;
	log->log_level = INFO_LEVEL;

#ifdef CONFIG_COUL_DRV
	struct coul_config *coul = iomcu_config_get(IOMCU_CONFIG_USR_COUL, sizeof(struct coul_config));
	coul_drv_coulobmeter_cur_calibration(&coul->coul_info.c_offset_a,
		&coul->coul_info.c_offset_b);
	of_property_read_u32(of_find_compatible_node(NULL, NULL, "hisilicon,coul_core"), "r_coul_mohm",
		&coul->coul_info.r_coul_mohm);
#endif /* CONFIG_COUL_DRV */

	struct igs_config *igs = iomcu_config_get(IOMCU_CONFIG_USR_IGS, sizeof(struct igs_config));
	if (inputhub_is_swing_bypass())
		igs->igs_hardware_bypass = 1;

	igs->sc_flag = judge_system_cache();
	hwlog_info("igs_hardware_bypass = %u sc_flag = %u \n",
		igs->igs_hardware_bypass,
		igs->sc_flag);

	return 0;
}

void inputhub_init_before_boot(void)
{
	pr_info("%s: ++\n", __func__);
	read_tp_color_cmdline();
	read_reboot_reason_cmdline();
	sensorhub_io_driver_init();

	if (is_sensorhub_disabled())
		return;

#if defined(CONFIG_HUAWEI_OHOS_DSM) || defined(CONFIG_HUAWEI_DSM)
	shb_dclient = dsm_register_client(&dsm_sensorhub);
#endif
	init_completion(&iom3_reboot);
	recovery_init();
	sensor_redetect_init();
	inputhub_route_init();
	set_notifier();
	pr_info("%s: --\n", __func__);
}

void inputhub_init_after_boot(void)
{
	pr_info("%s: ++\n", __func__);
	set_sensor_mcu_mode(1);
	mag_current_notify();
	pr_info("%s: --\n", __func__);
}

#ifndef CONFIG_INPUTHUB_30
static void peri_used_init(void)
{
#ifdef CONFIG_CONTEXTHUB_IDLE_32K
	spin_lock_init(&peri_lock);
	setup_timer(&peri_timer, peri_used_timeout, 0);
#endif
}

static void get_swing_dev_dts_status(void)
{
	struct device_node *node = NULL;
	int temp = 0;

	node = of_find_compatible_node(NULL, NULL, "hisilicon,igs-dev");
	if (!node) {
		pr_warn("[%s] : no swing dev..\n", __func__);
		return;
	}

	if (!of_device_is_available(node)) {
		pr_warn("[%s] swing disabled..\n", __func__);
		return;
	}
	struct igs_config *igs = iomcu_config_get(IOMCU_CONFIG_USR_IGS, sizeof(struct igs_config));
	if (of_property_read_u32(node, "is_pll_on", &temp))
		igs->is_pll_on = 0;
	else
		igs->is_pll_on = (u32)temp;

	pr_info("[%s] pid 0x%x..\n", __func__, temp);

	return;
}

static int inputhub_mcu_init(void)
{
	int ret;

	pr_info("----hisi_inputhub_init--->\n");

	peri_used_init();
	if (write_defualt_config_info_to_sharemem())
		return -1;
	write_timestamp_base_to_sharemem();

	get_swing_dev_dts_status();

	inputhub_init_before_boot();

	if (is_sensorhub_disabled())
		return -1;

	inputhub_mcu_connect();
	ret = boot_iom3();
	if (ret)
		hwlog_err("%s boot sensorhub fail!ret %d.\n", __func__, ret);

	inputhub_init_after_boot();
	hwlog_info("----%s--->\n", __func__);
	return ret;
}

static void __exit inputhub_mcu_exit(void)
{
	inputhub_route_exit();
	RPROC_PUT(ipc_ap_to_iom_mbx);
	RPROC_PUT(ipc_iom_to_ap_mbx);
}
#else

static int inputhub_mcu_init(void)
{
	if (write_defualt_config_info_to_sharemem())
		return -1;

	hwlog_info("%s: done\n", __func__);
	return 0;
}

static void __exit inputhub_mcu_exit(void)
{
	inputhub_route_exit();
}


static int inputhub_boot_notifier(struct notifier_block *nb, unsigned long event, void *para)
{
	switch (event) {
	case IOMCU_BOOT_PRE:
		inputhub_init_before_boot();
		break;

	case IOMCU_BOOT_POST:
		inputhub_init_after_boot();
		break;

	default:
		break;
	}
	return NOTIFY_OK;
}

static struct notifier_block inputhub_boot_notif_block = {
	.notifier_call = inputhub_boot_notifier,
};

static int inputhhub_boot_init(void)
{
	int ret = register_iomcu_boot_notifier(&inputhub_boot_notif_block);
	if (ret != 0)
		hwlog_err("[%s], register_mcu_send_cmd_notifier failed\n", __func__);

	hwlog_info("[%s] done", __func__);

	return 0;
}

static void __exit inputhhub_boot_exit(void)
{
	(void)unregister_iomcu_boot_notifier(&inputhub_boot_notif_block);
}

device_initcall(inputhhub_boot_init);
module_exit(inputhhub_boot_exit);
#endif

late_initcall(inputhub_mcu_init);
module_exit(inputhub_mcu_exit);

MODULE_AUTHOR("Input Hub <smartphone@huawei.com>");
MODULE_DESCRIPTION("input hub bridge");
MODULE_LICENSE("GPL");
