#ifndef __HISI_SCHARGER_V600_INCLUDE__
#define __HISI_SCHARGER_V600_INCLUDE__

#include <linux/i2c.h>       /* for struct device_info */
#include <linux/device.h>    /* for struct device_info */
#include <linux/workqueue.h> /* for struct evice_info */
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/rtmutex.h>
#include <linux/pm_wakeup.h>

#define ILIMIT_RBOOST_CNT 10

struct cv_info {
	int cv_ori_val;
	int cv_new_set_val;
	int cv_trim_flag;
};

static int hi6526_force_set_term_flag = CHG_STAT_DISABLE;
static int i_bias_all;
static int iin_set = CHG_ILIMIT_85;
struct hi6526_device_info *g_hi6526_dev;
struct cv_info *g_hi6526_cv_info = NULL;

struct hi6526_debug_value {
	int vbus;
	int ibat;
	int vusb;
	int vout;
	int temp;
	int ibus;
	int vbat;
	int tbat;
};

static const u8 ate_trim_tbl[] = {
	ATE_TRIM_LIST_0, ATE_TRIM_LIST_1, ATE_TRIM_LIST_2, ATE_TRIM_LIST_3,
	ATE_TRIM_LIST_4, ATE_TRIM_LIST_5, ATE_TRIM_LIST_6, ATE_TRIM_LIST_7,
	ATE_TRIM_LIST_8, ATE_TRIM_LIST_9, ATE_TRIM_LIST_10, ATE_TRIM_LIST_11,
	ATE_TRIM_LIST_12, ATE_TRIM_LIST_13, ATE_TRIM_LIST_14, ATE_TRIM_LIST_15,
	ATE_TRIM_LIST_16, ATE_TRIM_LIST_17, ATE_TRIM_LIST_18, ATE_TRIM_LIST_19,
	ATE_TRIM_LIST_20, ATE_TRIM_LIST_21, ATE_TRIM_LIST_22, ATE_TRIM_LIST_23,
	ATE_TRIM_LIST_24, ATE_TRIM_LIST_25, ATE_TRIM_LIST_26, ATE_TRIM_LIST_27,
	ATE_TRIM_LIST_28, ATE_TRIM_LIST_29, ATE_TRIM_LIST_30, ATE_TRIM_LIST_31
};

struct opt_regs common_opt_regs[] = {
	/* reg, mask, shift, val, before,after */
	reg_cfg(0xab, 0xff, 0, 0x63, 0, 0),
	reg_cfg(0x2e2, 0xff, 0, 0x95, 0, 0),
	reg_cfg(0x2e5, 0xff, 0, 0x58, 0, 0),
	reg_cfg(0x293, 0xff, 0, 0xA0, 0, 0),
	reg_cfg(0x284, 0xff, 0, 0x08, 0, 0),
	reg_cfg(0x287, 0xff, 0, 0x1C, 0, 0),
	reg_cfg(0x280, 0xff, 0, 0x0D, 0, 0),
	reg_cfg(0x2ac, 0xff, 0, 0x1e, 0, 0),
	reg_cfg(0x298, 0xff, 0, 0x01, 0, 0),
};

struct opt_regs v510_common_opt_regs[] = {
	/* reg, mask, shift, val, before,after */
	reg_cfg(0xab, 0xff, 0, 0x63, 0, 0),
	reg_cfg(0x2e2, 0xff, 0, 0x85, 0, 0),
	reg_cfg(0x2e5, 0xff, 0, 0x58, 0, 0),
	reg_cfg(0x293, 0xff, 0, 0xA0, 0, 0),
	reg_cfg(0x284, 0xff, 0, 0x08, 0, 0),
	reg_cfg(0x287, 0xff, 0, 0x1F, 0, 0),
	reg_cfg(0x280, 0xff, 0, 0x0D, 0, 0),
	reg_cfg(0x2ab, 0xff, 0, 0x20, 0, 0),
	reg_cfg(0x3b5, 0xff, 0, 0x80, 0, 0),
	reg_cfg(0x2ac, 0xff, 0, 0x1E, 0, 0),
	reg_cfg(0x298, 0xff, 0, 0x01, 0, 0),
	reg_cfg(0x2bd, 0xff, 0, 0x00, 0, 0),
	reg_cfg(0x29d, 0xff, 0, 0x03, 0, 0),
	reg_cfg(0x290, 0x20, 5, 0x01, 0, 0), /* mask_reset_n disable */
};

struct opt_regs v511_common_opt_regs[] = {
	/* reg, mask, shift, val, before,after */
	reg_cfg(0xab, 0xff, 0, 0x63, 0, 0),
	reg_cfg(0x2e2, 0xff, 0, 0x85, 0, 0),
	reg_cfg(0x2e5, 0xff, 0, 0x58, 0, 0),
	reg_cfg(0x293, 0xff, 0, 0xA0, 0, 0),
	reg_cfg(0x284, 0xff, 0, 0x08, 0, 0),
	reg_cfg(0x287, 0xff, 0, 0x1F, 0, 0),
	reg_cfg(0x280, 0xff, 0, 0x0D, 0, 0),
	reg_cfg(0x2ab, 0xff, 0, 0x20, 0, 0),
	reg_cfg(0x3b5, 0xff, 0, 0x80, 0, 0),
	reg_cfg(0x2ac, 0xff, 0, 0x1E, 0, 0),
	reg_cfg(0x298, 0xff, 0, 0x01, 0, 0),
	reg_cfg(0x2bd, 0xff, 0, 0x00, 0, 0),
	reg_cfg(0x290, 0xff, 0, 0x20, 0, 0),
	reg_cfg(0x29d, 0xff, 0, 0x03, 0, 0),
	reg_cfg(0x289, 0x20, 5, 0x1D, 0, 0),
};

struct opt_regs buck_common_opt_regs[] = {
	/* reg, mask, shift, val, before,after */
	reg_cfg(0xb0, 0xff, 0, 0x01, 0, 0), /* ACR reg reset */
	reg_cfg(0xae, 0xff, 0, 0x10, 0, 0), /* SOH reg reset */
	reg_cfg(0xf1, 0x02, 1, 0x00, 0, 0), /* sc_pulse_mode_en reg reset */
	reg_cfg(0x281, 0xc0, 6, 0x02, 0, 0), /* da_chg_cap3_sel reg reset 0x02 */
	reg_cfg(0x2e9, 0xff, 0, 0x1B, 0, 0),
	reg_cfg(0x2e0, 0xff, 0, 0x3F, 0, 0),
	reg_cfg(0x2f2, 0xff, 0, 0xA6, 0, 0),
	reg_cfg(0x2ed, 0xff, 0, 0x27, 0, 0),
	reg_cfg(0x2d3, 0xff, 0, 0x15, 0, 0),
	reg_cfg(0x2d4, 0xff, 0, 0x97, 0, 0),
	reg_cfg(0x2e1, 0xff, 0, 0x7D, 0, 0),
	reg_cfg(0x2e3, 0xff, 0, 0x22, 0, 0),
	reg_cfg(0x2e4, 0xff, 0, 0x3D, 0, 0),
	reg_cfg(0x2da, 0xff, 0, 0x8C, 0, 0),
	reg_cfg(0x2db, 0xff, 0, 0x94, 0, 0),
	reg_cfg(0x2d6, 0xff, 0, 0x0F, 0, 0),
	reg_cfg(0x2d9, 0xff, 0, 0x11, 0, 0),
	reg_cfg(0x2e7, 0xff, 0, 0x6F, 0, 0),
	reg_cfg(0x2de, 0xff, 0, 0x1E, 0, 0),
	reg_cfg(0x286, 0xff, 0, 0x06, 0, 0),
};

struct opt_regs buck_12v_extra_opt_regs[] = {
	/* reg, mask, shift, val, before,after */
	reg_cfg(0x2e7, 0xff, 0, 0xFF, 0, 0),
};

struct opt_regs v510_buck_common_opt_regs[] = {
	/* reg, mask, shift, val, before,after */
	reg_cfg(0x2d3, 0xff, 0, 0x15, 0, 0),
	reg_cfg(0x2d4, 0xff, 0, 0x95, 0, 0),
	reg_cfg(0x2e1, 0xff, 0, 0x6C, 0, 0),
	reg_cfg(0x2e3, 0xff, 0, 0x16, 0, 0),
	reg_cfg(0x2e4, 0xff, 0, 0x35, 0, 0),
	reg_cfg(0x2da, 0xff, 0, 0x8C, 0, 0),
	reg_cfg(0x2db, 0xff, 0, 0x94, 0, 0),
	reg_cfg(0x2d6, 0xff, 0, 0x0F, 0, 0),
	reg_cfg(0x2d9, 0xff, 0, 0x11, 0, 0),
	reg_cfg(0x2e7, 0xff, 0, 0xAF, 0, 0),
	reg_cfg(0x2de, 0xff, 0, 0x1E, 0, 0),
	reg_cfg(0x2e9, 0xff, 0, 0x1B, 0, 0),
	reg_cfg(0x2e0, 0xff, 0, 0x3F, 0, 0),
	reg_cfg(0x2f2, 0xff, 0, 0xA6, 0, 0),
	reg_cfg(0x2ed, 0xff, 0, 0x27, 0, 0),
	reg_cfg(0x286, 0xff, 0, 0x06, 0, 0),
};

struct opt_regs v510_buck_5v_extra_opt_regs[] = {
	/* reg, mask, shift, val, before,after */
	reg_cfg(0x2e7, 0xff, 0, 0xAF, 0, 0),
};

struct opt_regs v510_buck_9v_extra_opt_regs[] = {
	/* reg, mask, shift, val, before,after */
	reg_cfg(0x2e7, 0xff, 0, 0xEF, 0, 0),
};

struct opt_regs v510_buck_12v_extra_opt_regs[] = {
	/* reg, mask, shift, val, before,after */
	reg_cfg(0x2e7, 0xff, 0, 0xBF, 0, 0),
};

struct opt_regs v511_buck_common_opt_regs[] = {
	/* reg, mask, shift, val, before,after */
	reg_cfg(0x2d3, 0xff, 0, 0x15, 0, 0),
	reg_cfg(0x2d4, 0xff, 0, 0x95, 0, 0),
	reg_cfg(0x2e1, 0xff, 0, 0x6C, 0, 0),
	reg_cfg(0x2e3, 0xff, 0, 0x16, 0, 0),
	reg_cfg(0x2e4, 0xff, 0, 0x35, 0, 0),
	reg_cfg(0x2da, 0xff, 0, 0x8C, 0, 0),
	reg_cfg(0x2db, 0xff, 0, 0x94, 0, 0),
	reg_cfg(0x2d6, 0xff, 0, 0x0F, 0, 0),
	reg_cfg(0x2d9, 0xff, 0, 0x11, 0, 0),
	reg_cfg(0x2e7, 0xff, 0, 0xAF, 0, 0),
	reg_cfg(0x2de, 0xff, 0, 0x1E, 0, 0),
	reg_cfg(0x2e9, 0xff, 0, 0x1B, 0, 0),
	reg_cfg(0x2e0, 0xff, 0, 0x3F, 0, 0),
	reg_cfg(0x2f2, 0xff, 0, 0xA6, 0, 0),
	reg_cfg(0x2ed, 0xff, 0, 0x27, 0, 0),
	reg_cfg(0x286, 0xff, 0, 0x06, 0, 0),
};

struct opt_regs v511_buck_5v_extra_opt_regs[] = {
	/* reg, mask, shift, val, before,after */
	reg_cfg(0x2e7, 0xff, 0, 0xAF, 0, 0),
};

struct opt_regs v511_buck_9v_extra_opt_regs[] = {
	/* reg, mask, shift, val, before,after */
	reg_cfg(0x2e7, 0xff, 0, 0xEF, 0, 0),
};

struct opt_regs v511_buck_12v_extra_opt_regs[] = {
	/* reg, mask, shift, val, before,after */
	reg_cfg(0x2e7, 0xff, 0, 0xBF, 0, 0),
};

struct opt_regs otg_opt_regs[] = {
	/* reg, mask, shift, val, before,after */
	reg_cfg(0x2d3, 0xff, 0, 0x08, 0, 0),
	reg_cfg(0x2d4, 0xff, 0, 0x16, 0, 0),
	reg_cfg(0x2e1, 0xff, 0, 0x5A, 0, 0),
	reg_cfg(0x2e3, 0xff, 0, 0x20, 0, 0),
	reg_cfg(0x2e4, 0xff, 0, 0x25, 0, 0),
	reg_cfg(0x2da, 0xff, 0, 0x91, 0, 0),
	reg_cfg(0x2db, 0xff, 0, 0x92, 0, 0),
	reg_cfg(0x2e9, 0xff, 0, 0x1C, 0, 0),
	reg_cfg(0x294, 0xff, 0, 0x01, 0, 0),
	reg_cfg(0x2ee, 0xe7, 0, 0x09, 0, 0),
	reg_cfg(0xf0, 0x0f, 0, 0x06, 0, 0),
	reg_cfg(0x2d9, 0xff, 0, 0x10, 0, 0),
	reg_cfg(0x2de, 0xff, 0, 0X1C, 0, 0),
};

struct opt_regs v510_otg_opt_regs[] = {
	/* reg, mask, shift, val, before,after */
	reg_cfg(0x2d3, 0xff, 0, 0x08, 0, 0),
	reg_cfg(0x2d4, 0xff, 0, 0x14, 0, 0),
	reg_cfg(0x2e1, 0xff, 0, 0x5B, 0, 0),
	reg_cfg(0x2e3, 0xff, 0, 0x14, 0, 0),
	reg_cfg(0x2e4, 0xff, 0, 0x2D, 0, 0),
	reg_cfg(0x2da, 0xff, 0, 0x91, 0, 0),
	reg_cfg(0x2db, 0xff, 0, 0x92, 0, 0),
	reg_cfg(0x2e9, 0xff, 0, 0x1C, 0, 0),
	reg_cfg(0x294, 0xff, 0, 0x01, 0, 0),
	reg_cfg(0x2ee, 0xe7, 0, 0x0D, 0, 0),
	reg_cfg(0xf0, 0x0f, 0, 0x03, 0, 0),
	reg_cfg(0x2d9, 0xff, 0, 0x10, 0, 0),
	reg_cfg(0x2de, 0xff, 0, 0X1C, 0, 0),
};

struct opt_regs v511_otg_opt_regs[] = {
	/* reg, mask, shift, val, before,after */
	reg_cfg(0x2d3, 0xff, 0, 0x08, 0, 0),
	reg_cfg(0x2d4, 0xff, 0, 0x14, 0, 0),
	reg_cfg(0x2e1, 0xff, 0, 0x5B, 0, 0),
	reg_cfg(0x2e3, 0xff, 0, 0x14, 0, 0),
	reg_cfg(0x2e4, 0xff, 0, 0x2D, 0, 0),
	reg_cfg(0x2da, 0xff, 0, 0x91, 0, 0),
	reg_cfg(0x2db, 0xff, 0, 0x92, 0, 0),
	reg_cfg(0x2e9, 0xff, 0, 0x1C, 0, 0),
	reg_cfg(0x294, 0xff, 0, 0x01, 0, 0),
	reg_cfg(0x2ee, 0xe7, 0, 0x0D, 0, 0),
	reg_cfg(0xf0, 0x0f, 0, 0x03, 0, 0),
	reg_cfg(0x2d9, 0xff, 0, 0x10, 0, 0),
	reg_cfg(0x2de, 0xff, 0, 0X1C, 0, 0),
};

#endif