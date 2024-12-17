#ifndef __SCHARGER_V700_BUCK_H__
#define __SCHARGER_V700_BUCK_H__

#include <soc_schargerV700_interface.h>

#include <linux/slab.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/regmap.h>
#include <linux/time.h>

#include <securec.h>
#include <linux/printk.h>
#include <linux/workqueue.h>
#include <huawei_platform/power/huawei_charger.h>
#include <linux/platform_drivers/usb/hisi_typec.h>
#include <platform_include/basicplatform/linux/power/platform/bci_battery.h>
#include <platform_include/basicplatform/linux/power/platform/coul/coul_drv.h>
#include <platform_include/basicplatform/linux/power/platform/coul/coul_merge_drv.h>
#include <linux/mfd/pmic_platform.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include "../../../../mfd/scharger_v700/scharger_v700_adc.h"
#include <pmic_interface.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/gpio.h>
#include <linux/of_irq.h>
#include "scharger_v700_watchdog.h"
#include <chipset_common/hwpower/common_module/power_log.h>
#include <chipset_common/hwpower/hardware_channel/power_sw.h>
#include "../../platform_source/basicplatform/drivers/mfd/scharger_v700/scharger_v700.h"
#include "../scharger_dynamic_cv.h"
#include "scharger_v700_regs.h"

struct param {
	int r_coul_uohm;
	int bat_comp_h;
	int bat_comp_l;
	int vclamp_h;
	int vclamp_l;
	int vusb_ovp;
	int vdrop_ovp;
};

struct ir_comp_tbl {
	int value;
	u16 reg;
	int mask;
	int shift;
	u8 reg_val;
};

enum chg_mode_state {
	NONE = 0,
	BUCK,
	LVC,
	SC,
	OTG_MODE,
	SOH_MODE
};

enum usb_mode_state {
	CHG_TYPE_NONE = 0,
	CHG_TYPE_SDP,
	CHG_TYPE_CDP,
	CHG_TYPE_DCP,
	CHG_TYPE_UNKNOWN,
	CHG_TYPE_LVC,
	CHG_TYPE_SC
};

struct scharger_buck {
	struct device *dev;
	struct regmap *regmap;
	int term_vol_mv;
	int buck_vbus_set;
	int input_current;
	struct delayed_work reverbst_work;
	struct delayed_work vdropmin_check_work;
	struct mutex otg_lock;
	struct mutex ibias_calc_lock;
	u8 batt_ovp_cnt_30s;
	int reverbst_cnt;
	unsigned int is_board_type; /* 0:sft 1:udp 2:asic */
	struct param param_dts;
	struct work_struct irq_work;
	int buck_irq;
	int others_irq;
	int ovp_psw_irq;
	u8 irq_state;
	enum chg_mode_state chg_mode;
	enum usb_mode_state usb_mode;
	struct power_sw_attr ovpsw_attr;
	int ovpsw_status;
	unsigned int scharger_version;
	struct notifier_block usb_nb;
#ifdef CONFIG_HUAWEI_DSM
	struct delayed_work ovp_dmd_work;
	struct delayed_work one_battery_discharge_work;
#endif
	bool boost_en_flag;
	int bat_sel;
};

struct cv_info {
	int cv_ori_val;
	int cv_new_set_val;
	int cv_trim_flag;
};

#define SINGLE_BATTERY 0
#define DUAL_BATTERY   1

#define BATT_L 0
#define BATT_H 1

#define CHG_ENABLE                      1
#define CHG_DISABLE                     0
#define DC_DISABLE                      0
#define BATFET_H_DISABLE                0

#define REVERBST_DELAY_ON               1000

#define FCP_ACK_RETRY_CYCLE             10
#define CHG_VBATT_SOFT_OVP_CNT          3
#define CHG_VBATT_SOFT_OVP_MAX          4600

#define chg_vbatt_cv_103(x)             ((x) * 103 / 100)

#define ILIMIT_RBOOST_CNT               10

#define R_UOHM_1000                     1000
#define R_UOHM_2000                     2000
#define R_UOHM_5000                     5000
#define R_UOHM_10000                    10000
#define R_UOHM_DEFAULT                  R_UOHM_2000 /* resisitance uohm */

#define BATTERY_H                       1
#define BATTERY_L                       0

#define CHG_BATFET_DIS                  0
#define CHG_BATFET_EN                   1

#define DBFET_BAT_H_OFF                 0
#define DBFET_BAT_L_OFF                 1

#define DBFET_INDEP_CTRL_MODE           0
#define DBFET_SINGLE_CTRL_MODE          1

#define SCHARGER_DIEID_LENGTH           8
#define SCHARGER_DIEID_BYTE7            63
#define SCHARGER_DIEID_BYTE6            62
#define SCHARGER_DIEID_BYTE5            61
#define SCHARGER_DIEID_BYTE4            60
#define SCHARGER_DIEID_BYTE3            59
#define SCHARGER_DIEID_BYTE2            58
#define SCHARGER_DIEID_BYTE1            57
#define SCHARGER_DIEID_BYTE0            56

#define CHARGE_ERR_FLAG                 0x1
#define SC_CHG_ERR_FLAG                 0x2
#define LVC_CHG_ERR_FLAG                0x4

#define DBFET_VUSB_DROP_OVP             200
#define VUSB_DROP_OVP_200MV             200
#define VUSB_DROP_OVP_300MV             300

/*
 * MINUTE_ADJUST_CV_0MV 0x7
 * MINUTE_ADJUST_CV_1MV 0x6
 * MINUTE_ADJUST_CV_2MV 0x5
 * MINUTE_ADJUST_CV_3MV 0x4
 * MINUTE_ADJUST_CV_4MV 0x3
 * MINUTE_ADJUST_CV_5MV 0x2
 * MINUTE_ADJUST_CV_6MV 0x1
 * MINUTE_ADJUST_CV_7MV 0x0
 */
#define MINUTE_ADJUST_CV_MAX 7

#define CV_TRIM_PROTECT      52
#define CV_TRIM_MAX          45

int scharger_set_term_enable(int enable);
int scharger_set_charge_current(int charge_current);
int scharger_set_input_current(int cin_limit);
int scharger_set_buck_mode_enable(u32 enable);
int scharger_set_charge_enable(int enable);
int scharger_set_terminal_current(int term_current);
int scharger_set_terminal_voltage(int charge_voltage);
int config_opt_param(int vbus_vol);
int dpm_init(void);
int set_fast_safe_timer(u32 safe_timer);
extern int get_gpio_int(void);
extern int scharger_set_watchdog_timer(int value);
extern int scharger_reset_watchdog_timer(void);
int scharger_get_register_head(char *buf, int size, void *dev_data);
int scharger_dump_register(char *buf, int size, void *dev_data);
int scharger_psw_enable(int enable);
int scharger_ovp3_enable(int enable);
unsigned int scharger_get_device_version(void);
u8 scharger_get_charge_err_flag(void);
int check_ovp2_state(u8 *ovp2_state);
enum usb_mode_state get_buck_charge_mode(void);
int set_vusb_ovp(int ov_vol);
int set_batfet_single_ctrl(int enable, int bat_index);
#endif
