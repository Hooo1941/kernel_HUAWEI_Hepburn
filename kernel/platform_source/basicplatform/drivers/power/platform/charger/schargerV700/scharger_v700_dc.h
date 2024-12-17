#ifndef __SCHARGER_V700_DC_H__
#define __SCHARGER_V700_DC_H__

#include <linux/device.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/regmap.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <securec.h>
#include <linux/printk.h>
#include <linux/workqueue.h>
#include <linux/kernel.h>

#include <soc_schargerV700_interface.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_device_id.h>
#include "../../../../mfd/scharger_v700/scharger_v700_adc.h"
#include "scharger_v700_buck.h"
#include "scharger_v700_watchdog.h"

#define REGISTER_RET_SIZE  4

#define BATT_NAME_SIZE_MAX 20

#define IBAT_ABNORMAL_TIME                  100

#define SCHG_LOGIC_CFG_REG_4    (REG_SCHG_LOGIC_CFG_REG_4_ADDR(REG_ANA_BASE))
#define DROPMIN_OVP_TIME_SHIFT  (4)
#define DROPMIN_OVP_TIME_MSK    (0x3 << DROPMIN_OVP_TIME_SHIFT)
#define DROPMIN_OVP_TIME_10MS   0x03
#define DROPMIN_OVP_TIME_128US  0x01
#define DROPMIN_OVP_TIME_32US   0x00

#define PSW_OVP_CFG_REG_3       (REG_PSW_OVP_CFG_REG_3_ADDR(REG_ANA_BASE))
#define OVP2_DROPOVP_EN_SHIFT   (0)
#define OVP2_DROPOVP_EN_MSK     (0x1 << OVP2_DROPOVP_EN_SHIFT)
#define OVP2_DROPOVP_ENABLE     1
#define OVP2_DROPOVP_DISABLE    0
#define OVP3_DROPOVP_ENABLE     1
#define OVP3_DROPOVP_DISABLE    0

#define CHG_CURRENT_1300MA          1300

extern int scharger_set_watchdog_timer(int value);
extern int scharger_set_watchdog_timer_ms(int ms);
extern int scharger_reset_watchdog_timer(void);
extern int scharger_set_buck_mode_enable(u32 enable);
extern int scharger_set_charge_enable(int enable);
extern int config_opt_param(int vbus_vol);
extern int set_fast_safe_timer(u32 safe_timer);
extern int dpm_init(void);
extern int scharger_set_term_enable(int enable);
extern int scharger_set_input_current(int cin_limit);
extern int scharger_set_charge_current(int charge_current);
extern int scharger_set_terminal_voltage(int charge_voltage);
extern int scharger_set_terminal_current(int term_current);
int scharger_lvc_enable(int enable);
int scharger_sc_enable(int enable);
enum chg_mode_state dc_get_direct_charge_mode(void);
void set_qsw2_rcp_protect(int enable);
enum chg_mode_state dc_get_direct_charge_mode(void);
#endif
