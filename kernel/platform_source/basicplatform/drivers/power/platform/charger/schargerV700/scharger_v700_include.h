#ifndef __SCHARGER_V700_INCLUDE_H__
#define __SCHARGER_V700_INCLUDE_H__

#ifdef CONFIG_HUAWEI_DSM
#include <chipset_common/hwpower/common_module/power_dsm.h>
#endif
#include "scharger_v700_dc.h"

/* delay 12s for dmd init */
#define OVP_DMD_WORK_DELAY      12000

/* Wait for vusb drop delay 1000ms  */
#define VDROPMIN_CHECK_WORK_DELAY 1000

#define DSM_BUFF_SIZE_MAX       256

struct scharger_buck *g_buck = NULL;
static int iin_set = CHG_ILIMIT_85;
static int force_set_term_flag = CHG_STAT_DISABLE;
static int is_weaksource = WEAKSOURCE_FALSE;

u8 charge_err_flag = 0;
u8 batfet_h_chg_err_flag = 0;
u8 sc_chg_err_flag = 0;
u8 lvc_chg_err_flag = 0;

#ifdef CONFIG_HUAWEI_DSM
int ovp2_ovp3_err_flag = 0;
#endif

#define ONE_BATTERY_DISCHARGE_ERR_COUNT 2

#define VUSB_6000MV                     6000
#define VUSB_3000MV                     3000
#define VUSB_4500MV                     4500
#define VOLTAGE_300MV                   300

#define ONE_BATTREY_DISCHARGE_WORK_DELAY    7200000
#define ONE_BATTERY_DISCHARGE_MONITOR_DELAY 300000

struct cv_info *g_cv_info = NULL;
#endif